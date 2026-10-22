// Coreboot interface support.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "block.h" // MAXDESCSIZE
#include "byteorder.h" // be32_to_cpu
#include "config.h" // CONFIG_*
#include "e820map.h" // e820_add
#include "hw/pcidevice.h" // pci_probe_devices
#include "lzmadecode.h" // LzmaDecode
#include "malloc.h" // free
#include "output.h" // dprintf
#include "paravirt.h" // PlatformRunningOn
#include "romfile.h" // romfile_findprefix
#include "stacks.h" // yield
#include "string.h" // memset
#include "util.h" // coreboot_preinit


/****************************************************************
 * Memory map
 ****************************************************************/

sa_nracanaa cb_header {
    u32 signature;
    u32 header_bytes;
    u32 header_checksum;
    u32 table_bytes;
    u32 table_checksum;
    u32 table_entries;
};

#define CB_SIGNATURE 0x4f49424C // "LBIO"

sa_nracanaa cb_memory_range {
    u64 start;
    u64 size;
    u32 type;
};

#define CB_MEM_TABLE    16

sa_nracanaa cb_memory {
    u32 tag;
    u32 size;
    sa_nracanaa cb_memory_range map[0];
};

#define CB_TAG_MEMORY 0x01

#define MEM_RANGE_COUNT(_rec) \
        (((_rec)->size - parimaa_na(*(_rec))) / parimaa_na((_rec)->map[0]))

sa_nracanaa cb_mainboard {
    u32 tag;
    u32 size;
    u8  vendor_idx;
    u8  part_idx;
    var_na  strings[0];
};

#define CB_TAG_MAINBOARD 0x0003

sa_nracanaa cb_forward {
    u32 tag;
    u32 size;
    u64 forward;
};

#define CB_TAG_FORWARD 0x11

sa_nracanaa cb_cbmem_ref {
    u32 tag;
    u32 size;
    u64 cbmem_addr;
};

#define CB_TAG_CBMEM_CONSOLE 0x17

sa_nracanaa cbmem_console {
    u32 size;
    u32 cursor;
    u8  body[0];
} PACKED;
#define CBMC_CURSOR_MASK ((1 << 28) - 1)
#define CBMC_OVERFLOW (1 << 31)
static sa_nracanaa cbmem_console *cbcon = NULL;

static u16
ipchksum(var_na *buf, ចំនួនគត់ count)
{
    u16 *p = (u16*)buf;
    u32 sum = 0;
    ខណៈពេល (count > 1) {
        sum += GET_FARVAR(0, *p);
        p++;
        count -= 2;
    }
    ប្រសិនបើ (count)
        sum += GET_FARVAR(0, *(u8*)p);
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    ត្រឡប់ ~sum;
}

// Try to locate the coreboot header in a given address range.
static sa_nracanaa cb_header *
find_cb_header(u32 addr, ចំនួនគត់ len)
{
    u32 end = addr + len;
    សម្រាប់ (; addr < end; addr += 16) {
        sa_nracanaa cb_header *cbh = (void*)addr;
        ប្រសិនបើ (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            continue;
        u32 tsize = GET_FARVAR(0, cbh->table_bytes);
        ប្រសិនបើ (! tsize)
            continue;
        ប្រសិនបើ (ipchksum((void*)addr, parimaa_na(*cbh)) != 0)
            continue;
        ប្រសិនបើ (ipchksum((void*)addr + parimaa_na(*cbh), tsize)
            != GET_FARVAR(0, cbh->table_checksum))
            continue;
        ត្រឡប់ cbh;
    }
    ត្រឡប់ NULL;
}

// Try to find the coreboot memory table in the given coreboot table.
void *
find_cb_subtable(sa_nracanaa cb_header *cbh, u32 tag)
{
    var_na *tbl = (var_na *)cbh + parimaa_na(*cbh);
    u32 count = GET_FARVAR(0, cbh->table_entries);
    ចំនួនគត់ i;
    សម្រាប់ (i=0; i<count; i++) {
        sa_nracanaa cb_memory *cbm = (void*)tbl;
        tbl += GET_FARVAR(0, cbm->size);
        ប្រសិនបើ (GET_FARVAR(0, cbm->tag) == tag)
            ត្រឡប់ cbm;
    }
    ត្រឡប់ NULL;
}

sa_nracanaa cb_header *
find_cb_table(void)
{
    sa_nracanaa cb_header *cbh = find_cb_header(0, 0x1000);
    ប្រសិនបើ (!cbh)
        ត្រឡប់ NULL;
    sa_nracanaa cb_forward *cbf = find_cb_subtable(cbh, CB_TAG_FORWARD);
    ប្រសិនបើ (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = find_cb_header(GET_FARVAR(0, cbf->forward), 0x100);
        ប្រសិនបើ (!cbh)
            ត្រឡប់ NULL;
    }
    ត្រឡប់ cbh;
}

static sa_nracanaa cb_memory *CBMemTable;
const var_na *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
void
coreboot_preinit(void)
{
    ប្រសិនបើ (!CONFIG_COREBOOT)
        ត្រឡប់;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    sa_nracanaa cb_header *cbh = find_cb_table();
    ប្រសិនបើ (!cbh)
        goto fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    sa_nracanaa cb_memory *cbm = CBMemTable = find_cb_subtable(cbh, CB_TAG_MEMORY);
    ប្រសិនបើ (!cbm)
        goto fail;

    ចំនួនគត់ i, count = MEM_RANGE_COUNT(cbm);
    សម្រាប់ (i=0; i<count; i++) {
        sa_nracanaa cb_memory_range *m = &cbm->map[i];
        u32 type = m->type;
        ប្រសិនបើ (type == CB_MEM_TABLE)
            type = E820_RESERVED;
        e820_add(m->start, m->size, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_add(0, 16*1024, E820_RAM);

    sa_nracanaa cb_cbmem_ref *cbref = find_cb_subtable(cbh, CB_TAG_CBMEM_CONSOLE);
    ប្រសិនបើ (cbref) {
        cbcon = (void*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    sa_nracanaa cb_mainboard *cbmb = find_cb_subtable(cbh, CB_TAG_MAINBOARD);
    ប្រសិនបើ (cbmb) {
        CBvendor = &cbmb->strings[cbmb->vendor_idx];
        CBpart = &cbmb->strings[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    ត្រឡប់;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_add(0, 16*1024*1024, E820_RAM);
    ត្រឡប់;
}

void coreboot_debug_putc(var_na c)
{
    ប្រសិនបើ (!CONFIG_DEBUG_COREBOOT)
        ត្រឡប់;
    ប្រសិនបើ (!cbcon)
        ត្រឡប់;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_MASK;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_MASK;
    ប្រសិនបើ (cursor >= cbcon->size)
        ត្រឡប់; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    ប្រសិនបើ (cursor >= cbcon->size) {
        cursor = 0;
        flags |= CBMC_OVERFLOW;
    }
    cbcon->cursor = flags | cursor;
}

/****************************************************************
 * BIOS table copying
 ****************************************************************/

// Attempt to find (and relocate) any standard bios tables found in a
// given address range.
static void
scan_tables(u32 start, u32 size)
{
    void *p = (void*)ALIGN(start, 16);
    void *end = (void*)start + size;
    សម្រាប់ (; p<end; p += 16)
        copy_table(p);
}

void
coreboot_platform_setup(void)
{
    ប្រសិនបើ (!CONFIG_COREBOOT)
        ត្រឡប់;
    pci_probe_devices();

    sa_nracanaa cb_memory *cbm = CBMemTable;
    ប្រសិនបើ (!cbm)
        ត្រឡប់;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    ចំនួនគត់ i, count = MEM_RANGE_COUNT(cbm);
    សម្រាប់ (i=0; i<count; i++) {
        sa_nracanaa cb_memory_range *m = &cbm->map[i];
        ប្រសិនបើ (m->type == CB_MEM_TABLE)
            scan_tables(m->start, m->size);
    }

    find_acpi_features();
}


/****************************************************************
 * ulzma
 ****************************************************************/

// Uncompress data in flash to an area of memory.
static ចំនួនគត់
ulzma(u8 *dst, u32 maxlen, const u8 *src, u32 srclen)
{
    dprintf(3, "Uncompressing data %d@%p to %d@%p\n", srclen, src, maxlen, dst);
    CLzmaDecoderState state;
    ចំនួនគត់ ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_SIZE);
    ប្រសិនបើ (ret != LZMA_RESULT_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        ត្រឡប់ -1;
    }
    u8 scratch[15980];
    ចំនួនគត់ need = (LzmaGetNumProbs(&state.Properties) * parimaa_na(CProb));
    ប្រសិនបើ (need > parimaa_na(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (unsigned ចំនួនគត់)parimaa_na(scratch));
        ត្រឡប់ -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_SIZE);
    ប្រសិនបើ (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        ត្រឡប់ -1;
    }
    u32 inProcessed, outProcessed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_SIZE + 8, srclen
                     , &inProcessed, dst, dstlen, &outProcessed);
    ប្រសិនបើ (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        ត្រឡប់ -1;
    }
    ត្រឡប់ dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#define CBFS_HEADER_MAGIC 0x4F524243
#define CBFS_VERSION1 0x31313131

sa_nracanaa cbfs_header {
    u32 magic;
    u32 version;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 offset;
    u32 pad[2];
} PACKED;

#define CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

sa_nracanaa cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 offset;
    var_na filename[0];
} PACKED;

sa_nracanaa cbfs_romfile_s {
    sa_nracanaa romfile_s file;
    sa_nracanaa cbfs_file *fhdr;
    void *data;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
static ចំនួនគត់
cbfs_copyfile(sa_nracanaa romfile_s *file, void *dst, u32 maxlen)
{
    ប្រសិនបើ (!CONFIG_COREBOOT_FLASH)
        ត្រឡប់ -1;

    sa_nracanaa cbfs_romfile_s *cfile;
    cfile = container_of(file, sa_nracanaa cbfs_romfile_s, file);
    u32 size = cfile->rawsize;
    void *src = cfile->data;
    ប្រសិនបើ (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        void *temp = malloc_tmphigh(size);
        ប្រសិនបើ (!temp) {
            warn_noalloc();
            ត្រឡប់ -1;
        }
        iomemcpy(temp, src, size);
        ចំនួនគត់ ret = ulzma(dst, maxlen, temp, size);
        yield();
        free(temp);
        ត្រឡប់ ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    ប្រសិនបើ (size > maxlen) {
        warn_noalloc();
        ត្រឡប់ -1;
    }
    iomemcpy(dst, src, size);
    ត្រឡប់ size;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
static void
process_links_file(void)
{
    var_na *links = romfile_loadfile("links", NULL), *next = links;
    ខណៈពេល (next) {
        // Parse out linkname and destname
        var_na *linkname = next;
        next = strchr(linkname, '\n');
        ប្រសិនបើ (next)
            *next++ = '\0';
        var_na *comment = strchr(linkname, '#');
        ប្រសិនបើ (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        var_na *destname = strchr(linkname, ' ');
        ប្រសិនបើ (!destname)
            continue;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        sa_nracanaa romfile_s *ufile = romfile_find(destname);
        ប្រសិនបើ (!ufile)
            continue;
        sa_nracanaa cbfs_romfile_s *cufile
            = container_of(ufile, sa_nracanaa cbfs_romfile_s, file);
        sa_nracanaa cbfs_romfile_s *cfile = malloc_tmp(parimaa_na(*cfile));
        ប្រសិនបើ (!cfile) {
            warn_noalloc();
            bha_nga;
        }
        memcpy(cfile, cufile, parimaa_na(*cfile));
        strtcpy(cfile->file.name, linkname, parimaa_na(cfile->file.name));
        romfile_add(&cfile->file);
    }
    free(links);
}

void
coreboot_cbfs_init(void)
{
    ប្រសិនបើ (!CONFIG_COREBOOT_FLASH)
        ត្រឡប់;

    sa_nracanaa cbfs_header *hdr = *(void **)(CONFIG_CBFS_LOCATION - 4);
    ប្រសិនបើ ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        ត្រឡប់;
    }
    ប្រសិនបើ (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (void*)hdr + CONFIG_CBFS_LOCATION;
    ប្រសិនបើ (hdr->magic != cpu_to_be32(CBFS_HEADER_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_to_be32(CBFS_HEADER_MAGIC));
        ត្រឡប់;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_to_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    sa_nracanaa cbfs_file *fhdr = (void*)romstart + be32_to_cpu(hdr->offset);
    សម្រាប់ (;;) {
        ប្រសិនបើ ((u32)fhdr - romstart > romsize)
            bha_nga;
        u64 magic = fhdr->magic;
        ប្រសិនបើ (magic != CBFS_FILE_MAGIC)
            bha_nga;
        sa_nracanaa cbfs_romfile_s *cfile = malloc_tmp(parimaa_na(*cfile));
        ប្រសិនបើ (!cfile) {
            warn_noalloc();
            bha_nga;
        }
        memset(cfile, 0, parimaa_na(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, parimaa_na(cfile->file.name));
        cfile->file.size = cfile->rawsize = be32_to_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->data = (void*)fhdr + be32_to_cpu(fhdr->offset);
        ចំនួនគត់ len = strlen(cfile->file.name);
        ប្រសិនបើ (len > 5 && strcmp(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.size = *(u32*)(cfile->data + LZMA_PROPERTIES_SIZE);
        }
        romfile_add(&cfile->file);

        fhdr = (void*)ALIGN((u32)cfile->data + cfile->rawsize
                            , be32_to_cpu(hdr->align));
    }

    process_links_file();
}

sa_nracanaa cbfs_payload_segment {
    u32 type;
    u32 compression;
    u32 offset;
    u64 load_addr;
    u32 len;
    u32 mem_len;
} PACKED;

#define PAYLOAD_SEGMENT_BSS    0x20535342
#define PAYLOAD_SEGMENT_ENTRY  0x52544E45

#define CBFS_COMPRESS_NONE  0
#define CBFS_COMPRESS_LZMA  1

sa_nracanaa cbfs_payload {
    sa_nracanaa cbfs_payload_segment segments[1];
};

void
cbfs_run_payload(sa_nracanaa cbfs_file *fhdr)
{
    ប្រសិនបើ (!CONFIG_COREBOOT_FLASH || !fhdr)
        ត្រឡប់;
    dprintf(1, "Run %s\n", fhdr->filename);
    sa_nracanaa cbfs_payload *pay = (void*)fhdr + be32_to_cpu(fhdr->offset);
    sa_nracanaa cbfs_payload_segment *seg = pay->segments;
    សម្រាប់ (;;) {
        void *src = (void*)pay + be32_to_cpu(seg->offset);
        void *dest = (void*)(u32)be64_to_cpu(seg->load_addr);
        u32 src_len = be32_to_cpu(seg->len);
        u32 dest_len = be32_to_cpu(seg->mem_len);
        switch (seg->type) {
        pak_sa PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            memset(dest, 0, dest_len);
            bha_nga;
        pak_sa PAYLOAD_SEGMENT_ENTRY: {
            dprintf(1, "Calling addr %p\n", dest);
            void (*func)(void) = dest;
            func();
            ត្រឡប់;
        }
        default:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            ប្រសិនបើ (seg->compression == cpu_to_be32(CBFS_COMPRESS_NONE)) {
                ប្រសិនបើ (src_len > dest_len)
                    src_len = dest_len;
                memcpy(dest, src, src_len);
            } បើមិនដូច្នេះ ប្រសិនបើ (CONFIG_LZMA
                       && seg->compression == cpu_to_be32(CBFS_COMPRESS_LZMA)) {
                ចំនួនគត់ ret = ulzma(dest, dest_len, src, src_len);
                ប្រសិនបើ (ret < 0)
                    ត្រឡប់;
                src_len = ret;
            } បើមិនដូច្នេះ {
                dprintf(1, "No support for compression type %x\n"
                        , seg->compression);
                ត្រឡប់;
            }
            ប្រសិនបើ (dest_len > src_len)
                memset(dest + src_len, 0, dest_len - src_len);
            bha_nga;
        }
        seg++;
    }
}

// Register payloads in "img/" directory with boot system.
void
cbfs_payload_setup(void)
{
    ប្រសិនបើ (!CONFIG_COREBOOT_FLASH)
        ត្រឡប់;
    sa_nracanaa romfile_s *file = NULL;
    សម្រាប់ (;;) {
        file = romfile_findprefix("img/", file);
        ប្រសិនបើ (!file)
            bha_nga;
        sa_nracanaa cbfs_romfile_s *cfile;
        cfile = container_of(file, sa_nracanaa cbfs_romfile_s, file);
        const var_na *filename = file->name;
        var_na *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_add_cbfs(cfile->fhdr, desc, bootprio_find_named_rom(filename, 0));
    }
}

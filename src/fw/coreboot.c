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
ipchksum(var_na *buf, ကိန်းပြည့် count)
{
    u16 *p = (u16*)buf;
    u32 sum = 0;
    နေစဉ် (count > 1) {
        sum += GET_FARVAR(0, *p);
        p++;
        count -= 2;
    }
    အကယ်၍ (count)
        sum += GET_FARVAR(0, *(u8*)p);
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    ပြန်ပေး ~sum;
}

// Try to locate the coreboot header in a given address range.
static sa_nracanaa cb_header *
find_cb_header(u32 addr, ကိန်းပြည့် len)
{
    u32 end = addr + len;
    အတွက် (; addr < end; addr += 16) {
        sa_nracanaa cb_header *cbh = (void*)addr;
        အကယ်၍ (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            continue;
        u32 tsize = GET_FARVAR(0, cbh->table_bytes);
        အကယ်၍ (! tsize)
            continue;
        အကယ်၍ (ipchksum((void*)addr, parimaa_na(*cbh)) != 0)
            continue;
        အကယ်၍ (ipchksum((void*)addr + parimaa_na(*cbh), tsize)
            != GET_FARVAR(0, cbh->table_checksum))
            continue;
        ပြန်ပေး cbh;
    }
    ပြန်ပေး NULL;
}

// Try to find the coreboot memory table in the given coreboot table.
void *
find_cb_subtable(sa_nracanaa cb_header *cbh, u32 tag)
{
    var_na *tbl = (var_na *)cbh + parimaa_na(*cbh);
    u32 count = GET_FARVAR(0, cbh->table_entries);
    ကိန်းပြည့် i;
    အတွက် (i=0; i<count; i++) {
        sa_nracanaa cb_memory *cbm = (void*)tbl;
        tbl += GET_FARVAR(0, cbm->size);
        အကယ်၍ (GET_FARVAR(0, cbm->tag) == tag)
            ပြန်ပေး cbm;
    }
    ပြန်ပေး NULL;
}

sa_nracanaa cb_header *
find_cb_table(void)
{
    sa_nracanaa cb_header *cbh = find_cb_header(0, 0x1000);
    အကယ်၍ (!cbh)
        ပြန်ပေး NULL;
    sa_nracanaa cb_forward *cbf = find_cb_subtable(cbh, CB_TAG_FORWARD);
    အကယ်၍ (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = find_cb_header(GET_FARVAR(0, cbf->forward), 0x100);
        အကယ်၍ (!cbh)
            ပြန်ပေး NULL;
    }
    ပြန်ပေး cbh;
}

static sa_nracanaa cb_memory *CBMemTable;
const var_na *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
void
coreboot_preinit(void)
{
    အကယ်၍ (!CONFIG_COREBOOT)
        ပြန်ပေး;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    sa_nracanaa cb_header *cbh = find_cb_table();
    အကယ်၍ (!cbh)
        goto fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    sa_nracanaa cb_memory *cbm = CBMemTable = find_cb_subtable(cbh, CB_TAG_MEMORY);
    အကယ်၍ (!cbm)
        goto fail;

    ကိန်းပြည့် i, count = MEM_RANGE_COUNT(cbm);
    အတွက် (i=0; i<count; i++) {
        sa_nracanaa cb_memory_range *m = &cbm->map[i];
        u32 type = m->type;
        အကယ်၍ (type == CB_MEM_TABLE)
            type = E820_RESERVED;
        e820_add(m->start, m->size, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_add(0, 16*1024, E820_RAM);

    sa_nracanaa cb_cbmem_ref *cbref = find_cb_subtable(cbh, CB_TAG_CBMEM_CONSOLE);
    အကယ်၍ (cbref) {
        cbcon = (void*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    sa_nracanaa cb_mainboard *cbmb = find_cb_subtable(cbh, CB_TAG_MAINBOARD);
    အကယ်၍ (cbmb) {
        CBvendor = &cbmb->strings[cbmb->vendor_idx];
        CBpart = &cbmb->strings[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    ပြန်ပေး;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_add(0, 16*1024*1024, E820_RAM);
    ပြန်ပေး;
}

void coreboot_debug_putc(var_na c)
{
    အကယ်၍ (!CONFIG_DEBUG_COREBOOT)
        ပြန်ပေး;
    အကယ်၍ (!cbcon)
        ပြန်ပေး;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_MASK;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_MASK;
    အကယ်၍ (cursor >= cbcon->size)
        ပြန်ပေး; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    အကယ်၍ (cursor >= cbcon->size) {
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
    အတွက် (; p<end; p += 16)
        copy_table(p);
}

void
coreboot_platform_setup(void)
{
    အကယ်၍ (!CONFIG_COREBOOT)
        ပြန်ပေး;
    pci_probe_devices();

    sa_nracanaa cb_memory *cbm = CBMemTable;
    အကယ်၍ (!cbm)
        ပြန်ပေး;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    ကိန်းပြည့် i, count = MEM_RANGE_COUNT(cbm);
    အတွက် (i=0; i<count; i++) {
        sa_nracanaa cb_memory_range *m = &cbm->map[i];
        အကယ်၍ (m->type == CB_MEM_TABLE)
            scan_tables(m->start, m->size);
    }

    find_acpi_features();
}


/****************************************************************
 * ulzma
 ****************************************************************/

// Uncompress data in flash to an area of memory.
static ကိန်းပြည့်
ulzma(u8 *dst, u32 maxlen, const u8 *src, u32 srclen)
{
    dprintf(3, "Uncompressing data %d@%p to %d@%p\n", srclen, src, maxlen, dst);
    CLzmaDecoderState state;
    ကိန်းပြည့် ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_SIZE);
    အကယ်၍ (ret != LZMA_RESULT_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        ပြန်ပေး -1;
    }
    u8 scratch[15980];
    ကိန်းပြည့် need = (LzmaGetNumProbs(&state.Properties) * parimaa_na(CProb));
    အကယ်၍ (need > parimaa_na(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (unsigned ကိန်းပြည့်)parimaa_na(scratch));
        ပြန်ပေး -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_SIZE);
    အကယ်၍ (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        ပြန်ပေး -1;
    }
    u32 inProcessed, outProcessed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_SIZE + 8, srclen
                     , &inProcessed, dst, dstlen, &outProcessed);
    အကယ်၍ (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        ပြန်ပေး -1;
    }
    ပြန်ပေး dstlen;
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
static ကိန်းပြည့်
cbfs_copyfile(sa_nracanaa romfile_s *file, void *dst, u32 maxlen)
{
    အကယ်၍ (!CONFIG_COREBOOT_FLASH)
        ပြန်ပေး -1;

    sa_nracanaa cbfs_romfile_s *cfile;
    cfile = container_of(file, sa_nracanaa cbfs_romfile_s, file);
    u32 size = cfile->rawsize;
    void *src = cfile->data;
    အကယ်၍ (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        void *temp = malloc_tmphigh(size);
        အကယ်၍ (!temp) {
            warn_noalloc();
            ပြန်ပေး -1;
        }
        iomemcpy(temp, src, size);
        ကိန်းပြည့် ret = ulzma(dst, maxlen, temp, size);
        yield();
        free(temp);
        ပြန်ပေး ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    အကယ်၍ (size > maxlen) {
        warn_noalloc();
        ပြန်ပေး -1;
    }
    iomemcpy(dst, src, size);
    ပြန်ပေး size;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
static void
process_links_file(void)
{
    var_na *links = romfile_loadfile("links", NULL), *next = links;
    နေစဉ် (next) {
        // Parse out linkname and destname
        var_na *linkname = next;
        next = strchr(linkname, '\n');
        အကယ်၍ (next)
            *next++ = '\0';
        var_na *comment = strchr(linkname, '#');
        အကယ်၍ (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        var_na *destname = strchr(linkname, ' ');
        အကယ်၍ (!destname)
            continue;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        sa_nracanaa romfile_s *ufile = romfile_find(destname);
        အကယ်၍ (!ufile)
            continue;
        sa_nracanaa cbfs_romfile_s *cufile
            = container_of(ufile, sa_nracanaa cbfs_romfile_s, file);
        sa_nracanaa cbfs_romfile_s *cfile = malloc_tmp(parimaa_na(*cfile));
        အကယ်၍ (!cfile) {
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
    အကယ်၍ (!CONFIG_COREBOOT_FLASH)
        ပြန်ပေး;

    sa_nracanaa cbfs_header *hdr = *(void **)(CONFIG_CBFS_LOCATION - 4);
    အကယ်၍ ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        ပြန်ပေး;
    }
    အကယ်၍ (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (void*)hdr + CONFIG_CBFS_LOCATION;
    အကယ်၍ (hdr->magic != cpu_to_be32(CBFS_HEADER_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_to_be32(CBFS_HEADER_MAGIC));
        ပြန်ပေး;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_to_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    sa_nracanaa cbfs_file *fhdr = (void*)romstart + be32_to_cpu(hdr->offset);
    အတွက် (;;) {
        အကယ်၍ ((u32)fhdr - romstart > romsize)
            bha_nga;
        u64 magic = fhdr->magic;
        အကယ်၍ (magic != CBFS_FILE_MAGIC)
            bha_nga;
        sa_nracanaa cbfs_romfile_s *cfile = malloc_tmp(parimaa_na(*cfile));
        အကယ်၍ (!cfile) {
            warn_noalloc();
            bha_nga;
        }
        memset(cfile, 0, parimaa_na(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, parimaa_na(cfile->file.name));
        cfile->file.size = cfile->rawsize = be32_to_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->data = (void*)fhdr + be32_to_cpu(fhdr->offset);
        ကိန်းပြည့် len = strlen(cfile->file.name);
        အကယ်၍ (len > 5 && strcmp(&cfile->file.name[len-5], ".lzma") == 0) {
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
    အကယ်၍ (!CONFIG_COREBOOT_FLASH || !fhdr)
        ပြန်ပေး;
    dprintf(1, "Run %s\n", fhdr->filename);
    sa_nracanaa cbfs_payload *pay = (void*)fhdr + be32_to_cpu(fhdr->offset);
    sa_nracanaa cbfs_payload_segment *seg = pay->segments;
    အတွက် (;;) {
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
            ပြန်ပေး;
        }
        default:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            အကယ်၍ (seg->compression == cpu_to_be32(CBFS_COMPRESS_NONE)) {
                အကယ်၍ (src_len > dest_len)
                    src_len = dest_len;
                memcpy(dest, src, src_len);
            } မဟုတ်ပါက အကယ်၍ (CONFIG_LZMA
                       && seg->compression == cpu_to_be32(CBFS_COMPRESS_LZMA)) {
                ကိန်းပြည့် ret = ulzma(dest, dest_len, src, src_len);
                အကယ်၍ (ret < 0)
                    ပြန်ပေး;
                src_len = ret;
            } မဟုတ်ပါက {
                dprintf(1, "No support for compression type %x\n"
                        , seg->compression);
                ပြန်ပေး;
            }
            အကယ်၍ (dest_len > src_len)
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
    အကယ်၍ (!CONFIG_COREBOOT_FLASH)
        ပြန်ပေး;
    sa_nracanaa romfile_s *file = NULL;
    အတွက် (;;) {
        file = romfile_findprefix("img/", file);
        အကယ်၍ (!file)
            bha_nga;
        sa_nracanaa cbfs_romfile_s *cfile;
        cfile = container_of(file, sa_nracanaa cbfs_romfile_s, file);
        const var_na *filename = file->name;
        var_na *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_add_cbfs(cfile->fhdr, desc, bootprio_find_named_rom(filename, 0));
    }
}

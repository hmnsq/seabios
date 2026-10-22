// Coreboot interface support.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "block.h" // MAXDESCSIZE
#ضمن "byteorder.h" // be32_to_cpu
#ضمن "config.h" // CONFIG_*
#ضمن "e820map.h" // e820_add
#ضمن "hw/pcidevice.h" // pci_probe_devices
#ضمن "lzmadecode.h" // LzmaDecode
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "paravirt.h" // PlatformRunningOn
#ضمن "romfile.h" // romfile_findprefix
#ضمن "stacks.h" // yield
#ضمن "string.h" // memset
#ضمن "util.h" // coreboot_preinit


/****************************************************************
 * Memory map
 ****************************************************************/

بنية cb_header {
    u32 signature;
    u32 header_bytes;
    u32 header_checksum;
    u32 table_bytes;
    u32 table_checksum;
    u32 table_entries;
};

#define CB_SIGNATURE 0x4f49424C // "LBIO"

بنية cb_memory_range {
    u64 start;
    u64 size;
    u32 type;
};

#define CB_MEM_TABLE    16

بنية cb_memory {
    u32 tag;
    u32 size;
    بنية cb_memory_range map[0];
};

#define CB_TAG_MEMORY 0x01

#define MEM_RANGE_COUNT(_rec) \
        (((_rec)->size - حجم(*(_rec))) / حجم((_rec)->map[0]))

بنية cb_mainboard {
    u32 tag;
    u32 size;
    u8  vendor_idx;
    u8  part_idx;
    حرف  strings[0];
};

#define CB_TAG_MAINBOARD 0x0003

بنية cb_forward {
    u32 tag;
    u32 size;
    u64 forward;
};

#define CB_TAG_FORWARD 0x11

بنية cb_cbmem_ref {
    u32 tag;
    u32 size;
    u64 cbmem_addr;
};

#define CB_TAG_CBMEM_CONSOLE 0x17

بنية cbmem_console {
    u32 size;
    u32 cursor;
    u8  body[0];
} PACKED;
#define CBMC_CURSOR_MASK ((1 << 28) - 1)
#define CBMC_OVERFLOW (1 << 31)
ساكن بنية cbmem_console *cbcon = عدم;

ساكن u16
ipchksum(حرف *buf, صحيح count)
{
    u16 *p = (u16*)buf;
    u32 sum = 0;
    כלעוד (count > 1) {
        sum += GET_FARVAR(0, *p);
        p++;
        count -= 2;
    }
    אם (count)
        sum += GET_FARVAR(0, *(u8*)p);
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    החזר ~sum;
}

// Try to locate the coreboot header in a given address range.
ساكن بنية cb_header *
find_cb_header(u32 addr, صحيح len)
{
    u32 end = addr + len;
    עבור (; addr < end; addr += 16) {
        بنية cb_header *cbh = (فراغ*)addr;
        אם (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            استمر;
        u32 tsize = GET_FARVAR(0, cbh->table_bytes);
        אם (! tsize)
            استمر;
        אם (ipchksum((فراغ*)addr, حجم(*cbh)) != 0)
            استمر;
        אם (ipchksum((فراغ*)addr + حجم(*cbh), tsize)
            != GET_FARVAR(0, cbh->table_checksum))
            استمر;
        החזר cbh;
    }
    החזר عدم;
}

// Try to find the coreboot memory table in the given coreboot table.
فراغ *
find_cb_subtable(بنية cb_header *cbh, u32 tag)
{
    حرف *tbl = (حرف *)cbh + حجم(*cbh);
    u32 count = GET_FARVAR(0, cbh->table_entries);
    صحيح i;
    עבור (i=0; i<count; i++) {
        بنية cb_memory *cbm = (فراغ*)tbl;
        tbl += GET_FARVAR(0, cbm->size);
        אם (GET_FARVAR(0, cbm->tag) == tag)
            החזר cbm;
    }
    החזר عدم;
}

بنية cb_header *
find_cb_table(فراغ)
{
    بنية cb_header *cbh = find_cb_header(0, 0x1000);
    אם (!cbh)
        החזר عدم;
    بنية cb_forward *cbf = find_cb_subtable(cbh, CB_TAG_FORWARD);
    אם (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = find_cb_header(GET_FARVAR(0, cbf->forward), 0x100);
        אם (!cbh)
            החזר عدم;
    }
    החזר cbh;
}

ساكن بنية cb_memory *CBMemTable;
ثابت حرف *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
فراغ
coreboot_preinit(فراغ)
{
    אם (!CONFIG_COREBOOT)
        החזר;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    بنية cb_header *cbh = find_cb_table();
    אם (!cbh)
        اذهب fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    بنية cb_memory *cbm = CBMemTable = find_cb_subtable(cbh, CB_TAG_MEMORY);
    אם (!cbm)
        اذهب fail;

    صحيح i, count = MEM_RANGE_COUNT(cbm);
    עבור (i=0; i<count; i++) {
        بنية cb_memory_range *m = &cbm->map[i];
        u32 type = m->type;
        אם (type == CB_MEM_TABLE)
            type = E820_RESERVED;
        e820_add(m->start, m->size, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_add(0, 16*1024, E820_RAM);

    بنية cb_cbmem_ref *cbref = find_cb_subtable(cbh, CB_TAG_CBMEM_CONSOLE);
    אם (cbref) {
        cbcon = (فراغ*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    بنية cb_mainboard *cbmb = find_cb_subtable(cbh, CB_TAG_MAINBOARD);
    אם (cbmb) {
        CBvendor = &cbmb->strings[cbmb->vendor_idx];
        CBpart = &cbmb->strings[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    החזר;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_add(0, 16*1024*1024, E820_RAM);
    החזר;
}

فراغ coreboot_debug_putc(حرف c)
{
    אם (!CONFIG_DEBUG_COREBOOT)
        החזר;
    אם (!cbcon)
        החזר;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_MASK;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_MASK;
    אם (cursor >= cbcon->size)
        החזר; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    אם (cursor >= cbcon->size) {
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
ساكن فراغ
scan_tables(u32 start, u32 size)
{
    فراغ *p = (فراغ*)ALIGN(start, 16);
    فراغ *end = (فراغ*)start + size;
    עבור (; p<end; p += 16)
        copy_table(p);
}

فراغ
coreboot_platform_setup(فراغ)
{
    אם (!CONFIG_COREBOOT)
        החזר;
    pci_probe_devices();

    بنية cb_memory *cbm = CBMemTable;
    אם (!cbm)
        החזר;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    صحيح i, count = MEM_RANGE_COUNT(cbm);
    עבור (i=0; i<count; i++) {
        بنية cb_memory_range *m = &cbm->map[i];
        אם (m->type == CB_MEM_TABLE)
            scan_tables(m->start, m->size);
    }

    find_acpi_features();
}


/****************************************************************
 * ulzma
 ****************************************************************/

// Uncompress data in flash to an area of memory.
ساكن صحيح
ulzma(u8 *dst, u32 maxlen, ثابت u8 *src, u32 srclen)
{
    dprintf(3, "Uncompressing data %d@%p to %d@%p\n", srclen, src, maxlen, dst);
    CLzmaDecoderState state;
    صحيح ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_SIZE);
    אם (ret != LZMA_RESULT_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        החזר -1;
    }
    u8 scratch[15980];
    صحيح need = (LzmaGetNumProbs(&state.Properties) * حجم(CProb));
    אם (need > حجم(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (unsigned صحيح)حجم(scratch));
        החזר -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_SIZE);
    אם (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        החזר -1;
    }
    u32 inProcessed, outProcessed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_SIZE + 8, srclen
                     , &inProcessed, dst, dstlen, &outProcessed);
    אם (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        החזר -1;
    }
    החזר dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#define CBFS_HEADER_MAGIC 0x4F524243
#define CBFS_VERSION1 0x31313131

بنية cbfs_header {
    u32 magic;
    u32 version;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 offset;
    u32 pad[2];
} PACKED;

#define CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

بنية cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 offset;
    حرف filename[0];
} PACKED;

بنية cbfs_romfile_s {
    بنية romfile_s file;
    بنية cbfs_file *fhdr;
    فراغ *data;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
ساكن صحيح
cbfs_copyfile(بنية romfile_s *file, فراغ *dst, u32 maxlen)
{
    אם (!CONFIG_COREBOOT_FLASH)
        החזר -1;

    بنية cbfs_romfile_s *cfile;
    cfile = container_of(file, بنية cbfs_romfile_s, file);
    u32 size = cfile->rawsize;
    فراغ *src = cfile->data;
    אם (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        فراغ *temp = malloc_tmphigh(size);
        אם (!temp) {
            warn_noalloc();
            החזר -1;
        }
        iomemcpy(temp, src, size);
        صحيح ret = ulzma(dst, maxlen, temp, size);
        yield();
        حرر(temp);
        החזר ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    אם (size > maxlen) {
        warn_noalloc();
        החזר -1;
    }
    iomemcpy(dst, src, size);
    החזר size;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
ساكن فراغ
process_links_file(فراغ)
{
    حرف *links = romfile_loadfile("links", عدم), *next = links;
    כלעוד (next) {
        // Parse out linkname and destname
        حرف *linkname = next;
        next = strchr(linkname, '\n');
        אם (next)
            *next++ = '\0';
        حرف *comment = strchr(linkname, '#');
        אם (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        حرف *destname = strchr(linkname, ' ');
        אם (!destname)
            استمر;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        بنية romfile_s *ufile = romfile_find(destname);
        אם (!ufile)
            استمر;
        بنية cbfs_romfile_s *cufile
            = container_of(ufile, بنية cbfs_romfile_s, file);
        بنية cbfs_romfile_s *cfile = malloc_tmp(حجم(*cfile));
        אם (!cfile) {
            warn_noalloc();
            اكسر;
        }
        memcpy(cfile, cufile, حجم(*cfile));
        strtcpy(cfile->file.name, linkname, حجم(cfile->file.name));
        romfile_add(&cfile->file);
    }
    حرر(links);
}

فراغ
coreboot_cbfs_init(فراغ)
{
    אם (!CONFIG_COREBOOT_FLASH)
        החזר;

    بنية cbfs_header *hdr = *(فراغ **)(CONFIG_CBFS_LOCATION - 4);
    אם ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        החזר;
    }
    אם (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (فراغ*)hdr + CONFIG_CBFS_LOCATION;
    אם (hdr->magic != cpu_to_be32(CBFS_HEADER_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_to_be32(CBFS_HEADER_MAGIC));
        החזר;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_to_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    بنية cbfs_file *fhdr = (فراغ*)romstart + be32_to_cpu(hdr->offset);
    עבור (;;) {
        אם ((u32)fhdr - romstart > romsize)
            اكسر;
        u64 magic = fhdr->magic;
        אם (magic != CBFS_FILE_MAGIC)
            اكسر;
        بنية cbfs_romfile_s *cfile = malloc_tmp(حجم(*cfile));
        אם (!cfile) {
            warn_noalloc();
            اكسر;
        }
        memset(cfile, 0, حجم(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, حجم(cfile->file.name));
        cfile->file.size = cfile->rawsize = be32_to_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->data = (فراغ*)fhdr + be32_to_cpu(fhdr->offset);
        صحيح len = strlen(cfile->file.name);
        אם (len > 5 && strcmp(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.size = *(u32*)(cfile->data + LZMA_PROPERTIES_SIZE);
        }
        romfile_add(&cfile->file);

        fhdr = (فراغ*)ALIGN((u32)cfile->data + cfile->rawsize
                            , be32_to_cpu(hdr->align));
    }

    process_links_file();
}

بنية cbfs_payload_segment {
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

بنية cbfs_payload {
    بنية cbfs_payload_segment segments[1];
};

فراغ
cbfs_run_payload(بنية cbfs_file *fhdr)
{
    אם (!CONFIG_COREBOOT_FLASH || !fhdr)
        החזר;
    dprintf(1, "Run %s\n", fhdr->filename);
    بنية cbfs_payload *pay = (فراغ*)fhdr + be32_to_cpu(fhdr->offset);
    بنية cbfs_payload_segment *seg = pay->segments;
    עבור (;;) {
        فراغ *src = (فراغ*)pay + be32_to_cpu(seg->offset);
        فراغ *dest = (فراغ*)(u32)be64_to_cpu(seg->load_addr);
        u32 src_len = be32_to_cpu(seg->len);
        u32 dest_len = be32_to_cpu(seg->mem_len);
        بدل (seg->type) {
        حالة PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            memset(dest, 0, dest_len);
            اكسر;
        حالة PAYLOAD_SEGMENT_ENTRY: {
            dprintf(1, "Calling addr %p\n", dest);
            فراغ (*func)(فراغ) = dest;
            func();
            החזר;
        }
        افتراضي:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            אם (seg->compression == cpu_to_be32(CBFS_COMPRESS_NONE)) {
                אם (src_len > dest_len)
                    src_len = dest_len;
                memcpy(dest, src, src_len);
            } אחרת אם (CONFIG_LZMA
                       && seg->compression == cpu_to_be32(CBFS_COMPRESS_LZMA)) {
                صحيح ret = ulzma(dest, dest_len, src, src_len);
                אם (ret < 0)
                    החזר;
                src_len = ret;
            } אחרת {
                dprintf(1, "No support for compression type %x\n"
                        , seg->compression);
                החזר;
            }
            אם (dest_len > src_len)
                memset(dest + src_len, 0, dest_len - src_len);
            اكسر;
        }
        seg++;
    }
}

// Register payloads in "img/" directory with boot system.
فراغ
cbfs_payload_setup(فراغ)
{
    אם (!CONFIG_COREBOOT_FLASH)
        החזר;
    بنية romfile_s *file = عدم;
    עבור (;;) {
        file = romfile_findprefix("img/", file);
        אם (!file)
            اكسر;
        بنية cbfs_romfile_s *cfile;
        cfile = container_of(file, بنية cbfs_romfile_s, file);
        ثابت حرف *filename = file->name;
        حرف *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_add_cbfs(cfile->fhdr, desc, bootprio_find_named_rom(filename, 0));
    }
}

// Coreboot interface support.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "block.h" // MAXDESCSIZE
#অন্তর্ভাব "byteorder.h" // be32_to_cpu
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "e820map.h" // e820_add
#অন্তর্ভাব "hw/pcidevice.h" // pci_probe_devices
#অন্তর্ভাব "lzmadecode.h" // LzmaDecode
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "paravirt.h" // PlatformRunningOn
#অন্তর্ভাব "romfile.h" // romfile_findprefix
#অন্তর্ভাব "stacks.h" // yield
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // coreboot_preinit


/****************************************************************
 * Memory map
 ****************************************************************/

কাঠামো cb_header {
    u32 signature;
    u32 header_bytes;
    u32 header_checksum;
    u32 table_bytes;
    u32 table_checksum;
    u32 table_entries;
};

#define CB_SIGNATURE 0x4f49424C // "LBIO"

কাঠামো cb_memory_range {
    u64 start;
    u64 আকার_2;
    u32 type;
};

#define CB_MEM_TABLE    16

কাঠামো cb_memory {
    u32 tag;
    u32 আকার_2;
    কাঠামো cb_memory_range map[0];
};

#define CB_TAG_MEMORY 0x01

#define MEM_RANGE_গুনতি(_rec) \
        (((_rec)->আকার_2 - আকার(*(_rec))) / আকার((_rec)->map[0]))

কাঠামো cb_mainboard {
    u32 tag;
    u32 আকার_2;
    u8  vendor_idx;
    u8  part_idx;
    অক্ষর  strings[0];
};

#define CB_TAG_MAINBOARD 0x0003

কাঠামো cb_forward {
    u32 tag;
    u32 আকার_2;
    u64 forward;
};

#define CB_TAG_FORWARD 0x11

কাঠামো cb_cbmem_ref {
    u32 tag;
    u32 আকার_2;
    u64 cbmem_addr;
};

#define CB_TAG_CBMEM_CONSOLE 0x17

কাঠামো cbmem_console {
    u32 আকার_2;
    u32 cursor;
    u8  body[0];
} PACKED;
#define CBMC_CURSOR_MASK ((1 << 28) - 1)
#define CBMC_OVERFLOW (1 << 31)
স্থির কাঠামো cbmem_console *cbcon = ফাঁকা;

স্থির u16
ipchksum(অক্ষর *buf, পূর্ণসংখ্যা গুনতি)
{
    u16 *p = (u16*)buf;
    u32 যোগ = 0;
    যখন (গুনতি > 1) {
        যোগ += GET_FARVAR(0, *p);
        p++;
        গুনতি -= 2;
    }
    যদি (গুনতি)
        যোগ += GET_FARVAR(0, *(u8*)p);
    যোগ = (যোগ >> 16) + (যোগ & 0xffff);
    যোগ += (যোগ >> 16);
    ফেরত ~যোগ;
}

// Try to locate the coreboot header in a given address range.
স্থির কাঠামো cb_header *
find_cb_header(u32 addr, পূর্ণসংখ্যা len)
{
    u32 end = addr + len;
    জন্য (; addr < end; addr += 16) {
        কাঠামো cb_header *cbh = (শূন্য*)addr;
        যদি (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            চালু;
        u32 tsize = GET_FARVAR(0, cbh->table_bytes);
        যদি (! tsize)
            চালু;
        যদি (ipchksum((শূন্য*)addr, আকার(*cbh)) != 0)
            চালু;
        যদি (ipchksum((শূন্য*)addr + আকার(*cbh), tsize)
            != GET_FARVAR(0, cbh->table_checksum))
            চালু;
        ফেরত cbh;
    }
    ফেরত ফাঁকা;
}

// Try to find the coreboot memory table in the given coreboot table.
শূন্য *
find_cb_subtable(কাঠামো cb_header *cbh, u32 tag)
{
    অক্ষর *tbl = (অক্ষর *)cbh + আকার(*cbh);
    u32 গুনতি = GET_FARVAR(0, cbh->table_entries);
    পূর্ণসংখ্যা i;
    জন্য (i=0; i<গুনতি; i++) {
        কাঠামো cb_memory *cbm = (শূন্য*)tbl;
        tbl += GET_FARVAR(0, cbm->আকার_2);
        যদি (GET_FARVAR(0, cbm->tag) == tag)
            ফেরত cbm;
    }
    ফেরত ফাঁকা;
}

কাঠামো cb_header *
find_cb_table(শূন্য)
{
    কাঠামো cb_header *cbh = find_cb_header(0, 0x1000);
    যদি (!cbh)
        ফেরত ফাঁকা;
    কাঠামো cb_forward *cbf = find_cb_subtable(cbh, CB_TAG_FORWARD);
    যদি (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = find_cb_header(GET_FARVAR(0, cbf->forward), 0x100);
        যদি (!cbh)
            ফেরত ফাঁকা;
    }
    ফেরত cbh;
}

স্থির কাঠামো cb_memory *CBMemTable;
ধ্রুবক অক্ষর *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
শূন্য
coreboot_preinit(শূন্য)
{
    যদি (!CONFIG_COREBOOT)
        ফেরত;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    কাঠামো cb_header *cbh = find_cb_table();
    যদি (!cbh)
        যাও fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    কাঠামো cb_memory *cbm = CBMemTable = find_cb_subtable(cbh, CB_TAG_MEMORY);
    যদি (!cbm)
        যাও fail;

    পূর্ণসংখ্যা i, গুনতি = MEM_RANGE_গুনতি(cbm);
    জন্য (i=0; i<গুনতি; i++) {
        কাঠামো cb_memory_range *m = &cbm->map[i];
        u32 type = m->type;
        যদি (type == CB_MEM_TABLE)
            type = E820_RESERVED;
        e820_add(m->start, m->আকার_2, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_add(0, 16*1024, E820_RAM);

    কাঠামো cb_cbmem_ref *cbref = find_cb_subtable(cbh, CB_TAG_CBMEM_CONSOLE);
    যদি (cbref) {
        cbcon = (শূন্য*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    কাঠামো cb_mainboard *cbmb = find_cb_subtable(cbh, CB_TAG_MAINBOARD);
    যদি (cbmb) {
        CBvendor = &cbmb->strings[cbmb->vendor_idx];
        CBpart = &cbmb->strings[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    ফেরত;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_add(0, 16*1024*1024, E820_RAM);
    ফেরত;
}

শূন্য coreboot_debug_putc(অক্ষর c)
{
    যদি (!CONFIG_DEBUG_COREBOOT)
        ফেরত;
    যদি (!cbcon)
        ফেরত;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_MASK;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_MASK;
    যদি (cursor >= cbcon->আকার_2)
        ফেরত; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    যদি (cursor >= cbcon->আকার_2) {
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
স্থির শূন্য
scan_tables(u32 start, u32 আকার_2)
{
    শূন্য *p = (শূন্য*)ALIGN(start, 16);
    শূন্য *end = (শূন্য*)start + আকার_2;
    জন্য (; p<end; p += 16)
        copy_table(p);
}

শূন্য
coreboot_platform_setup(শূন্য)
{
    যদি (!CONFIG_COREBOOT)
        ফেরত;
    pci_probe_devices();

    কাঠামো cb_memory *cbm = CBMemTable;
    যদি (!cbm)
        ফেরত;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    পূর্ণসংখ্যা i, গুনতি = MEM_RANGE_গুনতি(cbm);
    জন্য (i=0; i<গুনতি; i++) {
        কাঠামো cb_memory_range *m = &cbm->map[i];
        যদি (m->type == CB_MEM_TABLE)
            scan_tables(m->start, m->আকার_2);
    }

    find_acpi_features();
}


/****************************************************************
 * ulzma
 ****************************************************************/

// Uncompress data in flash to an area of memory.
স্থির পূর্ণসংখ্যা
ulzma(u8 *dst, u32 maxlen, ধ্রুবক u8 *src, u32 srclen)
{
    dprintf(3, "Uncompressing data %d@%p to %d@%p\n", srclen, src, maxlen, dst);
    CLzmaDecoderState state;
    পূর্ণসংখ্যা ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_আকার);
    যদি (ret != LZMA_ফলাফল_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        ফেরত -1;
    }
    u8 scratch[15980];
    পূর্ণসংখ্যা need = (Lzma_Get_অঙ্ক_Probs(&state.Properties) * আকার(CProb));
    যদি (need > আকার(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (unsigned পূর্ণসংখ্যা)আকার(scratch));
        ফেরত -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_আকার);
    যদি (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        ফেরত -1;
    }
    u32 inProcessed, outProcessed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_আকার + 8, srclen
                     , &inProcessed, dst, dstlen, &outProcessed);
    যদি (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        ফেরত -1;
    }
    ফেরত dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#define CBFS_HEADER_MAGIC 0x4F524243
#define CBFS_VERSION1 0x31313131

কাঠামো cbfs_header {
    u32 magic;
    u32 version;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 offset;
    u32 pad[2];
} PACKED;

#define CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

কাঠামো cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 offset;
    অক্ষর filename[0];
} PACKED;

কাঠামো cbfs_romfile_s {
    কাঠামো romfile_s file;
    কাঠামো cbfs_file *fhdr;
    শূন্য *তথ্য;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
স্থির পূর্ণসংখ্যা
cbfs_copyfile(কাঠামো romfile_s *file, শূন্য *dst, u32 maxlen)
{
    যদি (!CONFIG_COREBOOT_FLASH)
        ফেরত -1;

    কাঠামো cbfs_romfile_s *cfile;
    cfile = container_of(file, কাঠামো cbfs_romfile_s, file);
    u32 আকার_2 = cfile->rawsize;
    শূন্য *src = cfile->তথ্য;
    যদি (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        শূন্য *অস্থায়ী = malloc_tmphigh(আকার_2);
        যদি (!অস্থায়ী) {
            warn_noalloc();
            ফেরত -1;
        }
        iomemcpy(অস্থায়ী, src, আকার_2);
        পূর্ণসংখ্যা ret = ulzma(dst, maxlen, অস্থায়ী, আকার_2);
        yield();
        মুক্ত(অস্থায়ী);
        ফেরত ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", আকার_2, src, maxlen, dst);
    যদি (আকার_2 > maxlen) {
        warn_noalloc();
        ফেরত -1;
    }
    iomemcpy(dst, src, আকার_2);
    ফেরত আকার_2;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
স্থির শূন্য
process_links_file(শূন্য)
{
    অক্ষর *links = romfile_loadfile("links", ফাঁকা), *পরবর্তী = links;
    যখন (পরবর্তী) {
        // Parse out linkname and destname
        অক্ষর *linkname = পরবর্তী;
        পরবর্তী = strchr(linkname, '\n');
        যদি (পরবর্তী)
            *পরবর্তী++ = '\0';
        অক্ষর *comment = strchr(linkname, '#');
        যদি (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        অক্ষর *destname = strchr(linkname, ' ');
        যদি (!destname)
            চালু;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        কাঠামো romfile_s *ufile = romfile_find(destname);
        যদি (!ufile)
            চালু;
        কাঠামো cbfs_romfile_s *cufile
            = container_of(ufile, কাঠামো cbfs_romfile_s, file);
        কাঠামো cbfs_romfile_s *cfile = malloc_tmp(আকার(*cfile));
        যদি (!cfile) {
            warn_noalloc();
            বিরতি;
        }
        memcpy(cfile, cufile, আকার(*cfile));
        strtcpy(cfile->file.name, linkname, আকার(cfile->file.name));
        romfile_add(&cfile->file);
    }
    মুক্ত(links);
}

শূন্য
coreboot_cbfs_init(শূন্য)
{
    যদি (!CONFIG_COREBOOT_FLASH)
        ফেরত;

    কাঠামো cbfs_header *hdr = *(শূন্য **)(CONFIG_CBFS_LOCATION - 4);
    যদি ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        ফেরত;
    }
    যদি (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (শূন্য*)hdr + CONFIG_CBFS_LOCATION;
    যদি (hdr->magic != cpu_to_be32(CBFS_HEADER_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_to_be32(CBFS_HEADER_MAGIC));
        ফেরত;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_to_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    কাঠামো cbfs_file *fhdr = (শূন্য*)romstart + be32_to_cpu(hdr->offset);
    জন্য (;;) {
        যদি ((u32)fhdr - romstart > romsize)
            বিরতি;
        u64 magic = fhdr->magic;
        যদি (magic != CBFS_FILE_MAGIC)
            বিরতি;
        কাঠামো cbfs_romfile_s *cfile = malloc_tmp(আকার(*cfile));
        যদি (!cfile) {
            warn_noalloc();
            বিরতি;
        }
        memset(cfile, 0, আকার(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, আকার(cfile->file.name));
        cfile->file.আকার_2 = cfile->rawsize = be32_to_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->তথ্য = (শূন্য*)fhdr + be32_to_cpu(fhdr->offset);
        পূর্ণসংখ্যা len = strlen(cfile->file.name);
        যদি (len > 5 && strcmp(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.আকার_2 = *(u32*)(cfile->তথ্য + LZMA_PROPERTIES_আকার);
        }
        romfile_add(&cfile->file);

        fhdr = (শূন্য*)ALIGN((u32)cfile->তথ্য + cfile->rawsize
                            , be32_to_cpu(hdr->align));
    }

    process_links_file();
}

কাঠামো cbfs_payload_segment {
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

কাঠামো cbfs_payload {
    কাঠামো cbfs_payload_segment segments[1];
};

শূন্য
cbfs_run_payload(কাঠামো cbfs_file *fhdr)
{
    যদি (!CONFIG_COREBOOT_FLASH || !fhdr)
        ফেরত;
    dprintf(1, "Run %s\n", fhdr->filename);
    কাঠামো cbfs_payload *pay = (শূন্য*)fhdr + be32_to_cpu(fhdr->offset);
    কাঠামো cbfs_payload_segment *seg = pay->segments;
    জন্য (;;) {
        শূন্য *src = (শূন্য*)pay + be32_to_cpu(seg->offset);
        শূন্য *dest = (শূন্য*)(u32)be64_to_cpu(seg->load_addr);
        u32 src_len = be32_to_cpu(seg->len);
        u32 dest_len = be32_to_cpu(seg->mem_len);
        নির্বাচন (seg->type) {
        ক্ষেত্র PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            memset(dest, 0, dest_len);
            বিরতি;
        ক্ষেত্র PAYLOAD_SEGMENT_ENTRY: {
            dprintf(1, "Calling addr %p\n", dest);
            শূন্য (*func)(শূন্য) = dest;
            func();
            ফেরত;
        }
        পূর্বনির্ধারিত:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            যদি (seg->compression == cpu_to_be32(CBFS_COMPRESS_NONE)) {
                যদি (src_len > dest_len)
                    src_len = dest_len;
                memcpy(dest, src, src_len);
            } অন্যথা যদি (CONFIG_LZMA
                       && seg->compression == cpu_to_be32(CBFS_COMPRESS_LZMA)) {
                পূর্ণসংখ্যা ret = ulzma(dest, dest_len, src, src_len);
                যদি (ret < 0)
                    ফেরত;
                src_len = ret;
            } অন্যথা {
                dprintf(1, "No support for compression type %x\n"
                        , seg->compression);
                ফেরত;
            }
            যদি (dest_len > src_len)
                memset(dest + src_len, 0, dest_len - src_len);
            বিরতি;
        }
        seg++;
    }
}

// Register payloads in "img/" directory with boot system.
শূন্য
cbfs_payload_setup(শূন্য)
{
    যদি (!CONFIG_COREBOOT_FLASH)
        ফেরত;
    কাঠামো romfile_s *file = ফাঁকা;
    জন্য (;;) {
        file = romfile_findprefix("img/", file);
        যদি (!file)
            বিরতি;
        কাঠামো cbfs_romfile_s *cfile;
        cfile = container_of(file, কাঠামো cbfs_romfile_s, file);
        ধ্রুবক অক্ষর *filename = file->name;
        অক্ষর *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_add_cbfs(cfile->fhdr, desc, bootprio_find_named_rom(filename, 0));
    }
}

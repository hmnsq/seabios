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

সংরচনা cb_header {
    u32 signature;
    u32 header_bytes;
    u32 header_checksum;
    u32 table_bytes;
    u32 table_checksum;
    u32 table_entries;
};

#define CB_SIGNATURE 0x4f49424C // "LBIO"

সংরচনা cb_memory_range {
    u64 start;
    u64 size;
    u32 type;
};

#define CB_MEM_TABLE    16

সংরচনা cb_memory {
    u32 tag;
    u32 size;
    সংরচনা cb_memory_range map[0];
};

#define CB_TAG_MEMORY 0x01

#define MEM_RANGE_গুনতি(_rec) \
        (((_rec)->size - পরিমাণ(*(_rec))) / পরিমাণ((_rec)->map[0]))

সংরচনা cb_mainboard {
    u32 tag;
    u32 size;
    u8  vendor_idx;
    u8  part_idx;
    বর্ণ  strings[0];
};

#define CB_TAG_MAINBOARD 0x0003

সংরচনা cb_forward {
    u32 tag;
    u32 size;
    u64 forward;
};

#define CB_TAG_FORWARD 0x11

সংরচনা cb_cbmem_ref {
    u32 tag;
    u32 size;
    u64 cbmem_addr;
};

#define CB_TAG_CBMEM_CONSOLE 0x17

সংরচনা cbmem_console {
    u32 size;
    u32 cursor;
    u8  body[0];
} PACKED;
#define CBMC_CURSOR_MASK ((1 << 28) - 1)
#define CBMC_OVERFLOW (1 << 31)
স্থির সংরচনা cbmem_console *cbcon = রিক্ত;

স্থির u16
ipchksum(বর্ণ *buf, পূর্ণাঙ্ক গুনতি)
{
    u16 *p = (u16*)buf;
    u32 sum = 0;
    যাবত্ (গুনতি > 1) {
        sum += GET_FARVAR(0, *p);
        p++;
        গুনতি -= 2;
    }
    যদি (গুনতি)
        sum += GET_FARVAR(0, *(u8*)p);
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    প্রত্যাগম ~sum;
}

// Try to locate the coreboot header in a given address range.
স্থির সংরচনা cb_header *
find_cb_header(u32 addr, পূর্ণাঙ্ক len)
{
    u32 end = addr + len;
    প্রতি (; addr < end; addr += 16) {
        সংরচনা cb_header *cbh = (শূন্য*)addr;
        যদি (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            অনুবর্ত;
        u32 tsize = GET_FARVAR(0, cbh->table_bytes);
        যদি (! tsize)
            অনুবর্ত;
        যদি (ipchksum((শূন্য*)addr, পরিমাণ(*cbh)) != 0)
            অনুবর্ত;
        যদি (ipchksum((শূন্য*)addr + পরিমাণ(*cbh), tsize)
            != GET_FARVAR(0, cbh->table_checksum))
            অনুবর্ত;
        প্রত্যাগম cbh;
    }
    প্রত্যাগম রিক্ত;
}

// Try to find the coreboot memory table in the given coreboot table.
শূন্য *
find_cb_subtable(সংরচনা cb_header *cbh, u32 tag)
{
    বর্ণ *tbl = (বর্ণ *)cbh + পরিমাণ(*cbh);
    u32 গুনতি = GET_FARVAR(0, cbh->table_entries);
    পূর্ণাঙ্ক i;
    প্রতি (i=0; i<গুনতি; i++) {
        সংরচনা cb_memory *cbm = (শূন্য*)tbl;
        tbl += GET_FARVAR(0, cbm->size);
        যদি (GET_FARVAR(0, cbm->tag) == tag)
            প্রত্যাগম cbm;
    }
    প্রত্যাগম রিক্ত;
}

সংরচনা cb_header *
find_cb_table(শূন্য)
{
    সংরচনা cb_header *cbh = find_cb_header(0, 0x1000);
    যদি (!cbh)
        প্রত্যাগম রিক্ত;
    সংরচনা cb_forward *cbf = find_cb_subtable(cbh, CB_TAG_FORWARD);
    যদি (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = find_cb_header(GET_FARVAR(0, cbf->forward), 0x100);
        যদি (!cbh)
            প্রত্যাগম রিক্ত;
    }
    প্রত্যাগম cbh;
}

স্থির সংরচনা cb_memory *CBMemTable;
ধ্রুব বর্ণ *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
শূন্য
coreboot_preinit(শূন্য)
{
    যদি (!CONFIG_COREBOOT)
        প্রত্যাগম;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    সংরচনা cb_header *cbh = find_cb_table();
    যদি (!cbh)
        গচ্ছ fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    সংরচনা cb_memory *cbm = CBMemTable = find_cb_subtable(cbh, CB_TAG_MEMORY);
    যদি (!cbm)
        গচ্ছ fail;

    পূর্ণাঙ্ক i, গুনতি = MEM_RANGE_গুনতি(cbm);
    প্রতি (i=0; i<গুনতি; i++) {
        সংরচনা cb_memory_range *m = &cbm->map[i];
        u32 type = m->type;
        যদি (type == CB_MEM_TABLE)
            type = E820_RESERVED;
        e820_add(m->start, m->size, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_add(0, 16*1024, E820_RAM);

    সংরচনা cb_cbmem_ref *cbref = find_cb_subtable(cbh, CB_TAG_CBMEM_CONSOLE);
    যদি (cbref) {
        cbcon = (শূন্য*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    সংরচনা cb_mainboard *cbmb = find_cb_subtable(cbh, CB_TAG_MAINBOARD);
    যদি (cbmb) {
        CBvendor = &cbmb->strings[cbmb->vendor_idx];
        CBpart = &cbmb->strings[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    প্রত্যাগম;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_add(0, 16*1024*1024, E820_RAM);
    প্রত্যাগম;
}

শূন্য coreboot_debug_putc(বর্ণ c)
{
    যদি (!CONFIG_DEBUG_COREBOOT)
        প্রত্যাগম;
    যদি (!cbcon)
        প্রত্যাগম;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_MASK;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_MASK;
    যদি (cursor >= cbcon->size)
        প্রত্যাগম; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    যদি (cursor >= cbcon->size) {
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
scan_tables(u32 start, u32 size)
{
    শূন্য *p = (শূন্য*)ALIGN(start, 16);
    শূন্য *end = (শূন্য*)start + size;
    প্রতি (; p<end; p += 16)
        copy_table(p);
}

শূন্য
coreboot_platform_setup(শূন্য)
{
    যদি (!CONFIG_COREBOOT)
        প্রত্যাগম;
    pci_probe_devices();

    সংরচনা cb_memory *cbm = CBMemTable;
    যদি (!cbm)
        প্রত্যাগম;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    পূর্ণাঙ্ক i, গুনতি = MEM_RANGE_গুনতি(cbm);
    প্রতি (i=0; i<গুনতি; i++) {
        সংরচনা cb_memory_range *m = &cbm->map[i];
        যদি (m->type == CB_MEM_TABLE)
            scan_tables(m->start, m->size);
    }

    find_acpi_features();
}


/****************************************************************
 * ulzma
 ****************************************************************/

// Uncompress data in flash to an area of memory.
স্থির পূর্ণাঙ্ক
ulzma(u8 *dst, u32 maxlen, ধ্রুব u8 *src, u32 srclen)
{
    dprintf(3, "Uncompressing data %d@%p to %d@%p\n", srclen, src, maxlen, dst);
    CLzmaDecoderState state;
    পূর্ণাঙ্ক ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_SIZE);
    যদি (ret != LZMA_RESULT_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        প্রত্যাগম -1;
    }
    u8 scratch[15980];
    পূর্ণাঙ্ক need = (Lzma_Get_অঙ্ক_Probs(&state.Properties) * পরিমাণ(CProb));
    যদি (need > পরিমাণ(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (unsigned পূর্ণাঙ্ক)পরিমাণ(scratch));
        প্রত্যাগম -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_SIZE);
    যদি (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        প্রত্যাগম -1;
    }
    u32 inProcessed, outProcessed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_SIZE + 8, srclen
                     , &inProcessed, dst, dstlen, &outProcessed);
    যদি (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        প্রত্যাগম -1;
    }
    প্রত্যাগম dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#define CBFS_HEADER_MAGIC 0x4F524243
#define CBFS_VERSION1 0x31313131

সংরচনা cbfs_header {
    u32 magic;
    u32 version;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 offset;
    u32 pad[2];
} PACKED;

#define CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

সংরচনা cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 offset;
    বর্ণ filename[0];
} PACKED;

সংরচনা cbfs_romfile_s {
    সংরচনা romfile_s file;
    সংরচনা cbfs_file *fhdr;
    শূন্য *data;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
স্থির পূর্ণাঙ্ক
cbfs_copyfile(সংরচনা romfile_s *file, শূন্য *dst, u32 maxlen)
{
    যদি (!CONFIG_COREBOOT_FLASH)
        প্রত্যাগম -1;

    সংরচনা cbfs_romfile_s *cfile;
    cfile = container_of(file, সংরচনা cbfs_romfile_s, file);
    u32 size = cfile->rawsize;
    শূন্য *src = cfile->data;
    যদি (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        শূন্য *temp = malloc_tmphigh(size);
        যদি (!temp) {
            warn_noalloc();
            প্রত্যাগম -1;
        }
        iomemcpy(temp, src, size);
        পূর্ণাঙ্ক ret = ulzma(dst, maxlen, temp, size);
        yield();
        মুক্ত(temp);
        প্রত্যাগম ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    যদি (size > maxlen) {
        warn_noalloc();
        প্রত্যাগম -1;
    }
    iomemcpy(dst, src, size);
    প্রত্যাগম size;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
স্থির শূন্য
process_links_file(শূন্য)
{
    বর্ণ *links = romfile_loadfile("links", রিক্ত), *next = links;
    যাবত্ (next) {
        // Parse out linkname and destname
        বর্ণ *linkname = next;
        next = strchr(linkname, '\n');
        যদি (next)
            *next++ = '\0';
        বর্ণ *comment = strchr(linkname, '#');
        যদি (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        বর্ণ *destname = strchr(linkname, ' ');
        যদি (!destname)
            অনুবর্ত;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        সংরচনা romfile_s *ufile = romfile_find(destname);
        যদি (!ufile)
            অনুবর্ত;
        সংরচনা cbfs_romfile_s *cufile
            = container_of(ufile, সংরচনা cbfs_romfile_s, file);
        সংরচনা cbfs_romfile_s *cfile = malloc_tmp(পরিমাণ(*cfile));
        যদি (!cfile) {
            warn_noalloc();
            ভঙ্গ;
        }
        memcpy(cfile, cufile, পরিমাণ(*cfile));
        strtcpy(cfile->file.name, linkname, পরিমাণ(cfile->file.name));
        romfile_add(&cfile->file);
    }
    মুক্ত(links);
}

শূন্য
coreboot_cbfs_init(শূন্য)
{
    যদি (!CONFIG_COREBOOT_FLASH)
        প্রত্যাগম;

    সংরচনা cbfs_header *hdr = *(শূন্য **)(CONFIG_CBFS_LOCATION - 4);
    যদি ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        প্রত্যাগম;
    }
    যদি (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (শূন্য*)hdr + CONFIG_CBFS_LOCATION;
    যদি (hdr->magic != cpu_to_be32(CBFS_HEADER_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_to_be32(CBFS_HEADER_MAGIC));
        প্রত্যাগম;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_to_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    সংরচনা cbfs_file *fhdr = (শূন্য*)romstart + be32_to_cpu(hdr->offset);
    প্রতি (;;) {
        যদি ((u32)fhdr - romstart > romsize)
            ভঙ্গ;
        u64 magic = fhdr->magic;
        যদি (magic != CBFS_FILE_MAGIC)
            ভঙ্গ;
        সংরচনা cbfs_romfile_s *cfile = malloc_tmp(পরিমাণ(*cfile));
        যদি (!cfile) {
            warn_noalloc();
            ভঙ্গ;
        }
        memset(cfile, 0, পরিমাণ(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, পরিমাণ(cfile->file.name));
        cfile->file.size = cfile->rawsize = be32_to_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->data = (শূন্য*)fhdr + be32_to_cpu(fhdr->offset);
        পূর্ণাঙ্ক len = strlen(cfile->file.name);
        যদি (len > 5 && strcmp(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.size = *(u32*)(cfile->data + LZMA_PROPERTIES_SIZE);
        }
        romfile_add(&cfile->file);

        fhdr = (শূন্য*)ALIGN((u32)cfile->data + cfile->rawsize
                            , be32_to_cpu(hdr->align));
    }

    process_links_file();
}

সংরচনা cbfs_payload_segment {
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

সংরচনা cbfs_payload {
    সংরচনা cbfs_payload_segment segments[1];
};

শূন্য
cbfs_run_payload(সংরচনা cbfs_file *fhdr)
{
    যদি (!CONFIG_COREBOOT_FLASH || !fhdr)
        প্রত্যাগম;
    dprintf(1, "Run %s\n", fhdr->filename);
    সংরচনা cbfs_payload *pay = (শূন্য*)fhdr + be32_to_cpu(fhdr->offset);
    সংরচনা cbfs_payload_segment *seg = pay->segments;
    প্রতি (;;) {
        শূন্য *src = (শূন্য*)pay + be32_to_cpu(seg->offset);
        শূন্য *dest = (শূন্য*)(u32)be64_to_cpu(seg->load_addr);
        u32 src_len = be32_to_cpu(seg->len);
        u32 dest_len = be32_to_cpu(seg->mem_len);
        বিকল্প (seg->type) {
        পক্ষ PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            memset(dest, 0, dest_len);
            ভঙ্গ;
        পক্ষ PAYLOAD_SEGMENT_ENTRY: {
            dprintf(1, "Calling addr %p\n", dest);
            শূন্য (*func)(শূন্য) = dest;
            func();
            প্রত্যাগম;
        }
        স্বভাব:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            যদি (seg->compression == cpu_to_be32(CBFS_COMPRESS_NONE)) {
                যদি (src_len > dest_len)
                    src_len = dest_len;
                memcpy(dest, src, src_len);
            } অন্যথা যদি (CONFIG_LZMA
                       && seg->compression == cpu_to_be32(CBFS_COMPRESS_LZMA)) {
                পূর্ণাঙ্ক ret = ulzma(dest, dest_len, src, src_len);
                যদি (ret < 0)
                    প্রত্যাগম;
                src_len = ret;
            } অন্যথা {
                dprintf(1, "No support for compression type %x\n"
                        , seg->compression);
                প্রত্যাগম;
            }
            যদি (dest_len > src_len)
                memset(dest + src_len, 0, dest_len - src_len);
            ভঙ্গ;
        }
        seg++;
    }
}

// Register payloads in "img/" directory with boot system.
শূন্য
cbfs_payload_setup(শূন্য)
{
    যদি (!CONFIG_COREBOOT_FLASH)
        প্রত্যাগম;
    সংরচনা romfile_s *file = রিক্ত;
    প্রতি (;;) {
        file = romfile_findprefix("img/", file);
        যদি (!file)
            ভঙ্গ;
        সংরচনা cbfs_romfile_s *cfile;
        cfile = container_of(file, সংরচনা cbfs_romfile_s, file);
        ধ্রুব বর্ণ *filename = file->name;
        বর্ণ *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_add_cbfs(cfile->fhdr, desc, bootprio_find_named_rom(filename, 0));
    }
}

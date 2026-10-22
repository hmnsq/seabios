// Coreboot interface support.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "block.h" // MAXDESCSIZE
#شامل "byteorder.h" // be32_to_cpu
#شامل "config.h" // CONFIG_*
#شامل "e820map.h" // e820_add
#شامل "hw/pcidevice.h" // pci_probe_devices
#شامل "lzmadecode.h" // LzmaDecode
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "paravirt.h" // PlatformRunningOn
#شامل "romfile.h" // romfile_findprefix
#شامل "stacks.h" // yield
#شامل "string.h" // memset
#شامل "util.h" // coreboot_preinit


/****************************************************************
 * Memory map
 ****************************************************************/

ساختار cb_header {
    u32 signature;
    u32 header_bytes;
    u32 header_checksum;
    u32 table_bytes;
    u32 table_checksum;
    u32 table_entries;
};

#define CB_SIGNATURE 0x4f49424C // "LBIO"

ساختار cb_memory_range {
    u64 start;
    u64 size;
    u32 type;
};

#define CB_MEM_TABLE    16

ساختار cb_memory {
    u32 tag;
    u32 size;
    ساختار cb_memory_range map[0];
};

#define CB_TAG_MEMORY 0x01

#define MEM_RANGE_شمار(_rec) \
        (((_rec)->size - اندازه(*(_rec))) / اندازه((_rec)->map[0]))

ساختار cb_mainboard {
    u32 tag;
    u32 size;
    u8  vendor_idx;
    u8  part_idx;
    نویسه  strings[0];
};

#define CB_TAG_MAINBOARD 0x0003

ساختار cb_forward {
    u32 tag;
    u32 size;
    u64 forward;
};

#define CB_TAG_FORWARD 0x11

ساختار cb_cbmem_ref {
    u32 tag;
    u32 size;
    u64 cbmem_addr;
};

#define CB_TAG_CBMEM_CONSOLE 0x17

ساختار cbmem_console {
    u32 size;
    u32 cursor;
    u8  body[0];
} PACKED;
#define CBMC_CURSOR_MASK ((1 << 28) - 1)
#define CBMC_OVERFLOW (1 << 31)
ایستا ساختار cbmem_console *cbcon = پوچ;

ایستا u16
ipchksum(نویسه *buf, صحیح شمار)
{
    u16 *p = (u16*)buf;
    u32 sum = 0;
    وقتی (شمار > 1) {
        sum += GET_FARVAR(0, *p);
        p++;
        شمار -= 2;
    }
    اگر (شمار)
        sum += GET_FARVAR(0, *(u8*)p);
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    بازگشت ~sum;
}

// Try to locate the coreboot header in a given address range.
ایستا ساختار cb_header *
find_cb_header(u32 addr, صحیح len)
{
    u32 end = addr + len;
    برای (; addr < end; addr += 16) {
        ساختار cb_header *cbh = (تهی*)addr;
        اگر (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            ادامه;
        u32 tsize = GET_FARVAR(0, cbh->table_bytes);
        اگر (! tsize)
            ادامه;
        اگر (ipchksum((تهی*)addr, اندازه(*cbh)) != 0)
            ادامه;
        اگر (ipchksum((تهی*)addr + اندازه(*cbh), tsize)
            != GET_FARVAR(0, cbh->table_checksum))
            ادامه;
        بازگشت cbh;
    }
    بازگشت پوچ;
}

// Try to find the coreboot memory table in the given coreboot table.
تهی *
find_cb_subtable(ساختار cb_header *cbh, u32 tag)
{
    نویسه *tbl = (نویسه *)cbh + اندازه(*cbh);
    u32 شمار = GET_FARVAR(0, cbh->table_entries);
    صحیح i;
    برای (i=0; i<شمار; i++) {
        ساختار cb_memory *cbm = (تهی*)tbl;
        tbl += GET_FARVAR(0, cbm->size);
        اگر (GET_FARVAR(0, cbm->tag) == tag)
            بازگشت cbm;
    }
    بازگشت پوچ;
}

ساختار cb_header *
find_cb_table(تهی)
{
    ساختار cb_header *cbh = find_cb_header(0, 0x1000);
    اگر (!cbh)
        بازگشت پوچ;
    ساختار cb_forward *cbf = find_cb_subtable(cbh, CB_TAG_FORWARD);
    اگر (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = find_cb_header(GET_FARVAR(0, cbf->forward), 0x100);
        اگر (!cbh)
            بازگشت پوچ;
    }
    بازگشت cbh;
}

ایستا ساختار cb_memory *CBMemTable;
ثابت نویسه *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
تهی
coreboot_preinit(تهی)
{
    اگر (!CONFIG_COREBOOT)
        بازگشت;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    ساختار cb_header *cbh = find_cb_table();
    اگر (!cbh)
        برو fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    ساختار cb_memory *cbm = CBMemTable = find_cb_subtable(cbh, CB_TAG_MEMORY);
    اگر (!cbm)
        برو fail;

    صحیح i, شمار = MEM_RANGE_شمار(cbm);
    برای (i=0; i<شمار; i++) {
        ساختار cb_memory_range *m = &cbm->map[i];
        u32 type = m->type;
        اگر (type == CB_MEM_TABLE)
            type = E820_RESERVED;
        e820_add(m->start, m->size, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_add(0, 16*1024, E820_RAM);

    ساختار cb_cbmem_ref *cbref = find_cb_subtable(cbh, CB_TAG_CBMEM_CONSOLE);
    اگر (cbref) {
        cbcon = (تهی*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    ساختار cb_mainboard *cbmb = find_cb_subtable(cbh, CB_TAG_MAINBOARD);
    اگر (cbmb) {
        CBvendor = &cbmb->strings[cbmb->vendor_idx];
        CBpart = &cbmb->strings[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    بازگشت;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_add(0, 16*1024*1024, E820_RAM);
    بازگشت;
}

تهی coreboot_debug_putc(نویسه c)
{
    اگر (!CONFIG_DEBUG_COREBOOT)
        بازگشت;
    اگر (!cbcon)
        بازگشت;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_MASK;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_MASK;
    اگر (cursor >= cbcon->size)
        بازگشت; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    اگر (cursor >= cbcon->size) {
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
ایستا تهی
scan_tables(u32 start, u32 size)
{
    تهی *p = (تهی*)ALIGN(start, 16);
    تهی *end = (تهی*)start + size;
    برای (; p<end; p += 16)
        copy_table(p);
}

تهی
coreboot_platform_setup(تهی)
{
    اگر (!CONFIG_COREBOOT)
        بازگشت;
    pci_probe_devices();

    ساختار cb_memory *cbm = CBMemTable;
    اگر (!cbm)
        بازگشت;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    صحیح i, شمار = MEM_RANGE_شمار(cbm);
    برای (i=0; i<شمار; i++) {
        ساختار cb_memory_range *m = &cbm->map[i];
        اگر (m->type == CB_MEM_TABLE)
            scan_tables(m->start, m->size);
    }

    find_acpi_features();
}


/****************************************************************
 * ulzma
 ****************************************************************/

// Uncompress data in flash to an area of memory.
ایستا صحیح
ulzma(u8 *dst, u32 maxlen, ثابت u8 *src, u32 srclen)
{
    dprintf(3, "Uncompressing data %d@%p to %d@%p\n", srclen, src, maxlen, dst);
    CLzmaDecoderState state;
    صحیح ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_SIZE);
    اگر (ret != LZMA_RESULT_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        بازگشت -1;
    }
    u8 scratch[15980];
    صحیح need = (Lzma_Get_رقم_Probs(&state.Properties) * اندازه(CProb));
    اگر (need > اندازه(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (unsigned صحیح)اندازه(scratch));
        بازگشت -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_SIZE);
    اگر (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        بازگشت -1;
    }
    u32 inProcessed, outProcessed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_SIZE + 8, srclen
                     , &inProcessed, dst, dstlen, &outProcessed);
    اگر (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        بازگشت -1;
    }
    بازگشت dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#define CBFS_HEADER_MAGIC 0x4F524243
#define CBFS_VERSION1 0x31313131

ساختار cbfs_header {
    u32 magic;
    u32 version;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 offset;
    u32 pad[2];
} PACKED;

#define CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

ساختار cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 offset;
    نویسه filename[0];
} PACKED;

ساختار cbfs_romfile_s {
    ساختار romfile_s file;
    ساختار cbfs_file *fhdr;
    تهی *data;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
ایستا صحیح
cbfs_copyfile(ساختار romfile_s *file, تهی *dst, u32 maxlen)
{
    اگر (!CONFIG_COREBOOT_FLASH)
        بازگشت -1;

    ساختار cbfs_romfile_s *cfile;
    cfile = container_of(file, ساختار cbfs_romfile_s, file);
    u32 size = cfile->rawsize;
    تهی *src = cfile->data;
    اگر (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        تهی *temp = malloc_tmphigh(size);
        اگر (!temp) {
            warn_noalloc();
            بازگشت -1;
        }
        iomemcpy(temp, src, size);
        صحیح ret = ulzma(dst, maxlen, temp, size);
        yield();
        آزاد(temp);
        بازگشت ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    اگر (size > maxlen) {
        warn_noalloc();
        بازگشت -1;
    }
    iomemcpy(dst, src, size);
    بازگشت size;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
ایستا تهی
process_links_file(تهی)
{
    نویسه *links = romfile_loadfile("links", پوچ), *next = links;
    وقتی (next) {
        // Parse out linkname and destname
        نویسه *linkname = next;
        next = strchr(linkname, '\n');
        اگر (next)
            *next++ = '\0';
        نویسه *comment = strchr(linkname, '#');
        اگر (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        نویسه *destname = strchr(linkname, ' ');
        اگر (!destname)
            ادامه;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        ساختار romfile_s *ufile = romfile_find(destname);
        اگر (!ufile)
            ادامه;
        ساختار cbfs_romfile_s *cufile
            = container_of(ufile, ساختار cbfs_romfile_s, file);
        ساختار cbfs_romfile_s *cfile = malloc_tmp(اندازه(*cfile));
        اگر (!cfile) {
            warn_noalloc();
            بشکن;
        }
        memcpy(cfile, cufile, اندازه(*cfile));
        strtcpy(cfile->file.name, linkname, اندازه(cfile->file.name));
        romfile_add(&cfile->file);
    }
    آزاد(links);
}

تهی
coreboot_cbfs_init(تهی)
{
    اگر (!CONFIG_COREBOOT_FLASH)
        بازگشت;

    ساختار cbfs_header *hdr = *(تهی **)(CONFIG_CBFS_LOCATION - 4);
    اگر ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        بازگشت;
    }
    اگر (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (تهی*)hdr + CONFIG_CBFS_LOCATION;
    اگر (hdr->magic != cpu_to_be32(CBFS_HEADER_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_to_be32(CBFS_HEADER_MAGIC));
        بازگشت;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_to_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    ساختار cbfs_file *fhdr = (تهی*)romstart + be32_to_cpu(hdr->offset);
    برای (;;) {
        اگر ((u32)fhdr - romstart > romsize)
            بشکن;
        u64 magic = fhdr->magic;
        اگر (magic != CBFS_FILE_MAGIC)
            بشکن;
        ساختار cbfs_romfile_s *cfile = malloc_tmp(اندازه(*cfile));
        اگر (!cfile) {
            warn_noalloc();
            بشکن;
        }
        memset(cfile, 0, اندازه(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, اندازه(cfile->file.name));
        cfile->file.size = cfile->rawsize = be32_to_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->data = (تهی*)fhdr + be32_to_cpu(fhdr->offset);
        صحیح len = strlen(cfile->file.name);
        اگر (len > 5 && strcmp(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.size = *(u32*)(cfile->data + LZMA_PROPERTIES_SIZE);
        }
        romfile_add(&cfile->file);

        fhdr = (تهی*)ALIGN((u32)cfile->data + cfile->rawsize
                            , be32_to_cpu(hdr->align));
    }

    process_links_file();
}

ساختار cbfs_payload_segment {
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

ساختار cbfs_payload {
    ساختار cbfs_payload_segment segments[1];
};

تهی
cbfs_run_payload(ساختار cbfs_file *fhdr)
{
    اگر (!CONFIG_COREBOOT_FLASH || !fhdr)
        بازگشت;
    dprintf(1, "Run %s\n", fhdr->filename);
    ساختار cbfs_payload *pay = (تهی*)fhdr + be32_to_cpu(fhdr->offset);
    ساختار cbfs_payload_segment *seg = pay->segments;
    برای (;;) {
        تهی *src = (تهی*)pay + be32_to_cpu(seg->offset);
        تهی *dest = (تهی*)(u32)be64_to_cpu(seg->load_addr);
        u32 src_len = be32_to_cpu(seg->len);
        u32 dest_len = be32_to_cpu(seg->mem_len);
        انتخاب (seg->type) {
        حالت PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            memset(dest, 0, dest_len);
            بشکن;
        حالت PAYLOAD_SEGMENT_ENTRY: {
            dprintf(1, "Calling addr %p\n", dest);
            تهی (*func)(تهی) = dest;
            func();
            بازگشت;
        }
        پیشفرض:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            اگر (seg->compression == cpu_to_be32(CBFS_COMPRESS_NONE)) {
                اگر (src_len > dest_len)
                    src_len = dest_len;
                memcpy(dest, src, src_len);
            } وگرنه اگر (CONFIG_LZMA
                       && seg->compression == cpu_to_be32(CBFS_COMPRESS_LZMA)) {
                صحیح ret = ulzma(dest, dest_len, src, src_len);
                اگر (ret < 0)
                    بازگشت;
                src_len = ret;
            } وگرنه {
                dprintf(1, "No support for compression type %x\n"
                        , seg->compression);
                بازگشت;
            }
            اگر (dest_len > src_len)
                memset(dest + src_len, 0, dest_len - src_len);
            بشکن;
        }
        seg++;
    }
}

// Register payloads in "img/" directory with boot system.
تهی
cbfs_payload_setup(تهی)
{
    اگر (!CONFIG_COREBOOT_FLASH)
        بازگشت;
    ساختار romfile_s *file = پوچ;
    برای (;;) {
        file = romfile_findprefix("img/", file);
        اگر (!file)
            بشکن;
        ساختار cbfs_romfile_s *cfile;
        cfile = container_of(file, ساختار cbfs_romfile_s, file);
        ثابت نویسه *filename = file->name;
        نویسه *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_add_cbfs(cfile->fhdr, desc, bootprio_find_named_rom(filename, 0));
    }
}

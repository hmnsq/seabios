// Coreboot interface support.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "block.h" // MAXDESCSIZE
#அந்தர்பாவ "byteorder.h" // be32_to_cpu
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "e820map.h" // e820_add
#அந்தர்பாவ "hw/pcidevice.h" // pci_probe_devices
#அந்தர்பாவ "lzmadecode.h" // LzmaDecode
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "paravirt.h" // PlatformRunningOn
#அந்தர்பாவ "romfile.h" // romfile_findprefix
#அந்தர்பாவ "stacks.h" // yield
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // coreboot_preinit


/****************************************************************
 * Memory map
 ****************************************************************/

கட்டமைப்பு cb_header {
    u32 signature;
    u32 header_bytes;
    u32 header_checksum;
    u32 table_bytes;
    u32 table_checksum;
    u32 table_entries;
};

#define CB_SIGNATURE 0x4f49424C // "LBIO"

கட்டமைப்பு cb_memory_range {
    u64 start;
    u64 size;
    u32 type;
};

#define CB_MEM_TABLE    16

கட்டமைப்பு cb_memory {
    u32 tag;
    u32 size;
    கட்டமைப்பு cb_memory_range map[0];
};

#define CB_TAG_MEMORY 0x01

#define MEM_RANGE_மொத்தம்(_rec) \
        (((_rec)->size - அளவு(*(_rec))) / அளவு((_rec)->map[0]))

கட்டமைப்பு cb_mainboard {
    u32 tag;
    u32 size;
    u8  vendor_idx;
    u8  part_idx;
    எழுத்து  strings[0];
};

#define CB_TAG_MAINBOARD 0x0003

கட்டமைப்பு cb_forward {
    u32 tag;
    u32 size;
    u64 forward;
};

#define CB_TAG_FORWARD 0x11

கட்டமைப்பு cb_cbmem_ref {
    u32 tag;
    u32 size;
    u64 cbmem_addr;
};

#define CB_TAG_CBMEM_CONSOLE 0x17

கட்டமைப்பு cbmem_console {
    u32 size;
    u32 cursor;
    u8  body[0];
} PACKED;
#define CBMC_CURSOR_MASK ((1 << 28) - 1)
#define CBMC_OVERFLOW (1 << 31)
நிலையான கட்டமைப்பு cbmem_console *cbcon = வெற்று;

நிலையான u16
ipchksum(எழுத்து *buf, முழு மொத்தம்)
{
    u16 *p = (u16*)buf;
    u32 sum = 0;
    போது (மொத்தம் > 1) {
        sum += GET_FARVAR(0, *p);
        p++;
        மொத்தம் -= 2;
    }
    எனில் (மொத்தம்)
        sum += GET_FARVAR(0, *(u8*)p);
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    திரும்பு ~sum;
}

// Try to locate the coreboot header in a given address range.
நிலையான கட்டமைப்பு cb_header *
find_cb_header(u32 addr, முழு len)
{
    u32 end = addr + len;
    க்கு (; addr < end; addr += 16) {
        கட்டமைப்பு cb_header *cbh = (வெறுமை*)addr;
        எனில் (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            தொடர்;
        u32 tsize = GET_FARVAR(0, cbh->table_bytes);
        எனில் (! tsize)
            தொடர்;
        எனில் (ipchksum((வெறுமை*)addr, அளவு(*cbh)) != 0)
            தொடர்;
        எனில் (ipchksum((வெறுமை*)addr + அளவு(*cbh), tsize)
            != GET_FARVAR(0, cbh->table_checksum))
            தொடர்;
        திரும்பு cbh;
    }
    திரும்பு வெற்று;
}

// Try to find the coreboot memory table in the given coreboot table.
வெறுமை *
find_cb_subtable(கட்டமைப்பு cb_header *cbh, u32 tag)
{
    எழுத்து *tbl = (எழுத்து *)cbh + அளவு(*cbh);
    u32 மொத்தம் = GET_FARVAR(0, cbh->table_entries);
    முழு i;
    க்கு (i=0; i<மொத்தம்; i++) {
        கட்டமைப்பு cb_memory *cbm = (வெறுமை*)tbl;
        tbl += GET_FARVAR(0, cbm->size);
        எனில் (GET_FARVAR(0, cbm->tag) == tag)
            திரும்பு cbm;
    }
    திரும்பு வெற்று;
}

கட்டமைப்பு cb_header *
find_cb_table(வெறுமை)
{
    கட்டமைப்பு cb_header *cbh = find_cb_header(0, 0x1000);
    எனில் (!cbh)
        திரும்பு வெற்று;
    கட்டமைப்பு cb_forward *cbf = find_cb_subtable(cbh, CB_TAG_FORWARD);
    எனில் (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = find_cb_header(GET_FARVAR(0, cbf->forward), 0x100);
        எனில் (!cbh)
            திரும்பு வெற்று;
    }
    திரும்பு cbh;
}

நிலையான கட்டமைப்பு cb_memory *CBMemTable;
மாறிலி எழுத்து *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
வெறுமை
coreboot_preinit(வெறுமை)
{
    எனில் (!CONFIG_COREBOOT)
        திரும்பு;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    கட்டமைப்பு cb_header *cbh = find_cb_table();
    எனில் (!cbh)
        செல் fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    கட்டமைப்பு cb_memory *cbm = CBMemTable = find_cb_subtable(cbh, CB_TAG_MEMORY);
    எனில் (!cbm)
        செல் fail;

    முழு i, மொத்தம் = MEM_RANGE_மொத்தம்(cbm);
    க்கு (i=0; i<மொத்தம்; i++) {
        கட்டமைப்பு cb_memory_range *m = &cbm->map[i];
        u32 type = m->type;
        எனில் (type == CB_MEM_TABLE)
            type = E820_RESERVED;
        e820_add(m->start, m->size, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_add(0, 16*1024, E820_RAM);

    கட்டமைப்பு cb_cbmem_ref *cbref = find_cb_subtable(cbh, CB_TAG_CBMEM_CONSOLE);
    எனில் (cbref) {
        cbcon = (வெறுமை*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    கட்டமைப்பு cb_mainboard *cbmb = find_cb_subtable(cbh, CB_TAG_MAINBOARD);
    எனில் (cbmb) {
        CBvendor = &cbmb->strings[cbmb->vendor_idx];
        CBpart = &cbmb->strings[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    திரும்பு;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_add(0, 16*1024*1024, E820_RAM);
    திரும்பு;
}

வெறுமை coreboot_debug_putc(எழுத்து c)
{
    எனில் (!CONFIG_DEBUG_COREBOOT)
        திரும்பு;
    எனில் (!cbcon)
        திரும்பு;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_MASK;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_MASK;
    எனில் (cursor >= cbcon->size)
        திரும்பு; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    எனில் (cursor >= cbcon->size) {
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
நிலையான வெறுமை
scan_tables(u32 start, u32 size)
{
    வெறுமை *p = (வெறுமை*)ALIGN(start, 16);
    வெறுமை *end = (வெறுமை*)start + size;
    க்கு (; p<end; p += 16)
        copy_table(p);
}

வெறுமை
coreboot_platform_setup(வெறுமை)
{
    எனில் (!CONFIG_COREBOOT)
        திரும்பு;
    pci_probe_devices();

    கட்டமைப்பு cb_memory *cbm = CBMemTable;
    எனில் (!cbm)
        திரும்பு;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    முழு i, மொத்தம் = MEM_RANGE_மொத்தம்(cbm);
    க்கு (i=0; i<மொத்தம்; i++) {
        கட்டமைப்பு cb_memory_range *m = &cbm->map[i];
        எனில் (m->type == CB_MEM_TABLE)
            scan_tables(m->start, m->size);
    }

    find_acpi_features();
}


/****************************************************************
 * ulzma
 ****************************************************************/

// Uncompress data in flash to an area of memory.
நிலையான முழு
ulzma(u8 *dst, u32 maxlen, மாறிலி u8 *src, u32 srclen)
{
    dprintf(3, "Uncompressing data %d@%p to %d@%p\n", srclen, src, maxlen, dst);
    CLzmaDecoderState state;
    முழு ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_SIZE);
    எனில் (ret != LZMA_RESULT_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        திரும்பு -1;
    }
    u8 scratch[15980];
    முழு need = (Lzma_Get_இலக்கம்_Probs(&state.Properties) * அளவு(CProb));
    எனில் (need > அளவு(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (unsigned முழு)அளவு(scratch));
        திரும்பு -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_SIZE);
    எனில் (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        திரும்பு -1;
    }
    u32 inProcessed, outProcessed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_SIZE + 8, srclen
                     , &inProcessed, dst, dstlen, &outProcessed);
    எனில் (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        திரும்பு -1;
    }
    திரும்பு dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#define CBFS_HEADER_MAGIC 0x4F524243
#define CBFS_VERSION1 0x31313131

கட்டமைப்பு cbfs_header {
    u32 magic;
    u32 version;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 offset;
    u32 pad[2];
} PACKED;

#define CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

கட்டமைப்பு cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 offset;
    எழுத்து filename[0];
} PACKED;

கட்டமைப்பு cbfs_romfile_s {
    கட்டமைப்பு romfile_s file;
    கட்டமைப்பு cbfs_file *fhdr;
    வெறுமை *data;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
நிலையான முழு
cbfs_copyfile(கட்டமைப்பு romfile_s *file, வெறுமை *dst, u32 maxlen)
{
    எனில் (!CONFIG_COREBOOT_FLASH)
        திரும்பு -1;

    கட்டமைப்பு cbfs_romfile_s *cfile;
    cfile = container_of(file, கட்டமைப்பு cbfs_romfile_s, file);
    u32 size = cfile->rawsize;
    வெறுமை *src = cfile->data;
    எனில் (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        வெறுமை *temp = malloc_tmphigh(size);
        எனில் (!temp) {
            warn_noalloc();
            திரும்பு -1;
        }
        iomemcpy(temp, src, size);
        முழு ret = ulzma(dst, maxlen, temp, size);
        yield();
        விடுவி(temp);
        திரும்பு ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    எனில் (size > maxlen) {
        warn_noalloc();
        திரும்பு -1;
    }
    iomemcpy(dst, src, size);
    திரும்பு size;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
நிலையான வெறுமை
process_links_file(வெறுமை)
{
    எழுத்து *links = romfile_loadfile("links", வெற்று), *next = links;
    போது (next) {
        // Parse out linkname and destname
        எழுத்து *linkname = next;
        next = strchr(linkname, '\n');
        எனில் (next)
            *next++ = '\0';
        எழுத்து *comment = strchr(linkname, '#');
        எனில் (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        எழுத்து *destname = strchr(linkname, ' ');
        எனில் (!destname)
            தொடர்;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        கட்டமைப்பு romfile_s *ufile = romfile_find(destname);
        எனில் (!ufile)
            தொடர்;
        கட்டமைப்பு cbfs_romfile_s *cufile
            = container_of(ufile, கட்டமைப்பு cbfs_romfile_s, file);
        கட்டமைப்பு cbfs_romfile_s *cfile = malloc_tmp(அளவு(*cfile));
        எனில் (!cfile) {
            warn_noalloc();
            நிறுத்து;
        }
        memcpy(cfile, cufile, அளவு(*cfile));
        strtcpy(cfile->file.name, linkname, அளவு(cfile->file.name));
        romfile_add(&cfile->file);
    }
    விடுவி(links);
}

வெறுமை
coreboot_cbfs_init(வெறுமை)
{
    எனில் (!CONFIG_COREBOOT_FLASH)
        திரும்பு;

    கட்டமைப்பு cbfs_header *hdr = *(வெறுமை **)(CONFIG_CBFS_LOCATION - 4);
    எனில் ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        திரும்பு;
    }
    எனில் (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (வெறுமை*)hdr + CONFIG_CBFS_LOCATION;
    எனில் (hdr->magic != cpu_to_be32(CBFS_HEADER_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_to_be32(CBFS_HEADER_MAGIC));
        திரும்பு;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_to_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    கட்டமைப்பு cbfs_file *fhdr = (வெறுமை*)romstart + be32_to_cpu(hdr->offset);
    க்கு (;;) {
        எனில் ((u32)fhdr - romstart > romsize)
            நிறுத்து;
        u64 magic = fhdr->magic;
        எனில் (magic != CBFS_FILE_MAGIC)
            நிறுத்து;
        கட்டமைப்பு cbfs_romfile_s *cfile = malloc_tmp(அளவு(*cfile));
        எனில் (!cfile) {
            warn_noalloc();
            நிறுத்து;
        }
        memset(cfile, 0, அளவு(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, அளவு(cfile->file.name));
        cfile->file.size = cfile->rawsize = be32_to_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->data = (வெறுமை*)fhdr + be32_to_cpu(fhdr->offset);
        முழு len = strlen(cfile->file.name);
        எனில் (len > 5 && strcmp(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.size = *(u32*)(cfile->data + LZMA_PROPERTIES_SIZE);
        }
        romfile_add(&cfile->file);

        fhdr = (வெறுமை*)ALIGN((u32)cfile->data + cfile->rawsize
                            , be32_to_cpu(hdr->align));
    }

    process_links_file();
}

கட்டமைப்பு cbfs_payload_segment {
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

கட்டமைப்பு cbfs_payload {
    கட்டமைப்பு cbfs_payload_segment segments[1];
};

வெறுமை
cbfs_run_payload(கட்டமைப்பு cbfs_file *fhdr)
{
    எனில் (!CONFIG_COREBOOT_FLASH || !fhdr)
        திரும்பு;
    dprintf(1, "Run %s\n", fhdr->filename);
    கட்டமைப்பு cbfs_payload *pay = (வெறுமை*)fhdr + be32_to_cpu(fhdr->offset);
    கட்டமைப்பு cbfs_payload_segment *seg = pay->segments;
    க்கு (;;) {
        வெறுமை *src = (வெறுமை*)pay + be32_to_cpu(seg->offset);
        வெறுமை *dest = (வெறுமை*)(u32)be64_to_cpu(seg->load_addr);
        u32 src_len = be32_to_cpu(seg->len);
        u32 dest_len = be32_to_cpu(seg->mem_len);
        தேர்வு (seg->type) {
        நிலை PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            memset(dest, 0, dest_len);
            நிறுத்து;
        நிலை PAYLOAD_SEGMENT_ENTRY: {
            dprintf(1, "Calling addr %p\n", dest);
            வெறுமை (*func)(வெறுமை) = dest;
            func();
            திரும்பு;
        }
        இயல்பு:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            எனில் (seg->compression == cpu_to_be32(CBFS_COMPRESS_NONE)) {
                எனில் (src_len > dest_len)
                    src_len = dest_len;
                memcpy(dest, src, src_len);
            } இல்லையெனில் எனில் (CONFIG_LZMA
                       && seg->compression == cpu_to_be32(CBFS_COMPRESS_LZMA)) {
                முழு ret = ulzma(dest, dest_len, src, src_len);
                எனில் (ret < 0)
                    திரும்பு;
                src_len = ret;
            } இல்லையெனில் {
                dprintf(1, "No support for compression type %x\n"
                        , seg->compression);
                திரும்பு;
            }
            எனில் (dest_len > src_len)
                memset(dest + src_len, 0, dest_len - src_len);
            நிறுத்து;
        }
        seg++;
    }
}

// Register payloads in "img/" directory with boot system.
வெறுமை
cbfs_payload_setup(வெறுமை)
{
    எனில் (!CONFIG_COREBOOT_FLASH)
        திரும்பு;
    கட்டமைப்பு romfile_s *file = வெற்று;
    க்கு (;;) {
        file = romfile_findprefix("img/", file);
        எனில் (!file)
            நிறுத்து;
        கட்டமைப்பு cbfs_romfile_s *cfile;
        cfile = container_of(file, கட்டமைப்பு cbfs_romfile_s, file);
        மாறிலி எழுத்து *filename = file->name;
        எழுத்து *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_add_cbfs(cfile->fhdr, desc, bootprio_find_named_rom(filename, 0));
    }
}

// Coreboot interface support.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "block.h" // MAXDESCSIZE
#ಅನ್ತರ್ಭಾವ "byteorder.h" // be32_to_cpu
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "e820map.h" // e820_add
#ಅನ್ತರ್ಭಾವ "hw/pcidevice.h" // pci_probe_devices
#ಅನ್ತರ್ಭಾವ "lzmadecode.h" // LzmaDecode
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "paravirt.h" // PlatformRunningOn
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_findprefix
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // coreboot_preinit


/****************************************************************
 * Memory map
 ****************************************************************/

ಸಂರಚನಾ cb_header {
    u32 signature;
    u32 header_bytes;
    u32 header_checksum;
    u32 table_bytes;
    u32 table_checksum;
    u32 table_entries;
};

#define CB_SIGNATURE 0x4f49424C // "LBIO"

ಸಂರಚನಾ cb_memory_range {
    u64 start;
    u64 size;
    u32 type;
};

#define CB_MEM_TABLE    16

ಸಂರಚನಾ cb_memory {
    u32 tag;
    u32 size;
    ಸಂರಚನಾ cb_memory_range map[0];
};

#define CB_TAG_MEMORY 0x01

#define MEM_RANGE_ಎಣಿಕೆ(_rec) \
        (((_rec)->size - ಪರಿಮಾಣ(*(_rec))) / ಪರಿಮಾಣ((_rec)->map[0]))

ಸಂರಚನಾ cb_mainboard {
    u32 tag;
    u32 size;
    u8  vendor_idx;
    u8  part_idx;
    ವರ್ಣ  strings[0];
};

#define CB_TAG_MAINBOARD 0x0003

ಸಂರಚನಾ cb_forward {
    u32 tag;
    u32 size;
    u64 forward;
};

#define CB_TAG_FORWARD 0x11

ಸಂರಚನಾ cb_cbmem_ref {
    u32 tag;
    u32 size;
    u64 cbmem_addr;
};

#define CB_TAG_CBMEM_CONSOLE 0x17

ಸಂರಚನಾ cbmem_console {
    u32 size;
    u32 cursor;
    u8  body[0];
} PACKED;
#define CBMC_CURSOR_MASK ((1 << 28) - 1)
#define CBMC_OVERFLOW (1 << 31)
ಸ್ಥಿರ ಸಂರಚನಾ cbmem_console *cbcon = ರಿಕ್ತ;

ಸ್ಥಿರ u16
ipchksum(ವರ್ಣ *buf, ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ)
{
    u16 *p = (u16*)buf;
    u32 sum = 0;
    ಯಾವತ್ (ಎಣಿಕೆ > 1) {
        sum += GET_FARVAR(0, *p);
        p++;
        ಎಣಿಕೆ -= 2;
    }
    ಯದಿ (ಎಣಿಕೆ)
        sum += GET_FARVAR(0, *(u8*)p);
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    ಪ್ರತ್ಯಾಗಮ ~sum;
}

// Try to locate the coreboot header in a given address range.
ಸ್ಥಿರ ಸಂರಚನಾ cb_header *
find_cb_header(u32 addr, ಪೂರ್ಣಾಙ್ಕ len)
{
    u32 end = addr + len;
    ಪ್ರತಿ (; addr < end; addr += 16) {
        ಸಂರಚನಾ cb_header *cbh = (ಶೂನ್ಯ*)addr;
        ಯದಿ (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            ಅನುವರ್ತ;
        u32 tsize = GET_FARVAR(0, cbh->table_bytes);
        ಯದಿ (! tsize)
            ಅನುವರ್ತ;
        ಯದಿ (ipchksum((ಶೂನ್ಯ*)addr, ಪರಿಮಾಣ(*cbh)) != 0)
            ಅನುವರ್ತ;
        ಯದಿ (ipchksum((ಶೂನ್ಯ*)addr + ಪರಿಮಾಣ(*cbh), tsize)
            != GET_FARVAR(0, cbh->table_checksum))
            ಅನುವರ್ತ;
        ಪ್ರತ್ಯಾಗಮ cbh;
    }
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

// Try to find the coreboot memory table in the given coreboot table.
ಶೂನ್ಯ *
find_cb_subtable(ಸಂರಚನಾ cb_header *cbh, u32 tag)
{
    ವರ್ಣ *tbl = (ವರ್ಣ *)cbh + ಪರಿಮಾಣ(*cbh);
    u32 ಎಣಿಕೆ = GET_FARVAR(0, cbh->table_entries);
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<ಎಣಿಕೆ; i++) {
        ಸಂರಚನಾ cb_memory *cbm = (ಶೂನ್ಯ*)tbl;
        tbl += GET_FARVAR(0, cbm->size);
        ಯದಿ (GET_FARVAR(0, cbm->tag) == tag)
            ಪ್ರತ್ಯಾಗಮ cbm;
    }
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸಂರಚನಾ cb_header *
find_cb_table(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ cb_header *cbh = find_cb_header(0, 0x1000);
    ಯದಿ (!cbh)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    ಸಂರಚನಾ cb_forward *cbf = find_cb_subtable(cbh, CB_TAG_FORWARD);
    ಯದಿ (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = find_cb_header(GET_FARVAR(0, cbf->forward), 0x100);
        ಯದಿ (!cbh)
            ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    ಪ್ರತ್ಯಾಗಮ cbh;
}

ಸ್ಥಿರ ಸಂರಚನಾ cb_memory *CBMemTable;
ಧ್ರುವ ವರ್ಣ *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
ಶೂನ್ಯ
coreboot_preinit(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_COREBOOT)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    ಸಂರಚನಾ cb_header *cbh = find_cb_table();
    ಯದಿ (!cbh)
        ಗಚ್ಛ fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    ಸಂರಚನಾ cb_memory *cbm = CBMemTable = find_cb_subtable(cbh, CB_TAG_MEMORY);
    ಯದಿ (!cbm)
        ಗಚ್ಛ fail;

    ಪೂರ್ಣಾಙ್ಕ i, ಎಣಿಕೆ = MEM_RANGE_ಎಣಿಕೆ(cbm);
    ಪ್ರತಿ (i=0; i<ಎಣಿಕೆ; i++) {
        ಸಂರಚನಾ cb_memory_range *m = &cbm->map[i];
        u32 type = m->type;
        ಯದಿ (type == CB_MEM_TABLE)
            type = E820_RESERVED;
        e820_add(m->start, m->size, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_add(0, 16*1024, E820_RAM);

    ಸಂರಚನಾ cb_cbmem_ref *cbref = find_cb_subtable(cbh, CB_TAG_CBMEM_CONSOLE);
    ಯದಿ (cbref) {
        cbcon = (ಶೂನ್ಯ*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    ಸಂರಚನಾ cb_mainboard *cbmb = find_cb_subtable(cbh, CB_TAG_MAINBOARD);
    ಯದಿ (cbmb) {
        CBvendor = &cbmb->strings[cbmb->vendor_idx];
        CBpart = &cbmb->strings[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    ಪ್ರತ್ಯಾಗಮ;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_add(0, 16*1024*1024, E820_RAM);
    ಪ್ರತ್ಯಾಗಮ;
}

ಶೂನ್ಯ coreboot_debug_putc(ವರ್ಣ c)
{
    ಯದಿ (!CONFIG_DEBUG_COREBOOT)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (!cbcon)
        ಪ್ರತ್ಯಾಗಮ;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_MASK;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_MASK;
    ಯದಿ (cursor >= cbcon->size)
        ಪ್ರತ್ಯಾಗಮ; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    ಯದಿ (cursor >= cbcon->size) {
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
ಸ್ಥಿರ ಶೂನ್ಯ
scan_tables(u32 start, u32 size)
{
    ಶೂನ್ಯ *p = (ಶೂನ್ಯ*)ALIGN(start, 16);
    ಶೂನ್ಯ *end = (ಶೂನ್ಯ*)start + size;
    ಪ್ರತಿ (; p<end; p += 16)
        copy_table(p);
}

ಶೂನ್ಯ
coreboot_platform_setup(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_COREBOOT)
        ಪ್ರತ್ಯಾಗಮ;
    pci_probe_devices();

    ಸಂರಚನಾ cb_memory *cbm = CBMemTable;
    ಯದಿ (!cbm)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    ಪೂರ್ಣಾಙ್ಕ i, ಎಣಿಕೆ = MEM_RANGE_ಎಣಿಕೆ(cbm);
    ಪ್ರತಿ (i=0; i<ಎಣಿಕೆ; i++) {
        ಸಂರಚನಾ cb_memory_range *m = &cbm->map[i];
        ಯದಿ (m->type == CB_MEM_TABLE)
            scan_tables(m->start, m->size);
    }

    find_acpi_features();
}


/****************************************************************
 * ulzma
 ****************************************************************/

// Uncompress data in flash to an area of memory.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ulzma(u8 *dst, u32 maxlen, ಧ್ರುವ u8 *src, u32 srclen)
{
    dprintf(3, "Uncompressing data %d@%p to %d@%p\n", srclen, src, maxlen, dst);
    CLzmaDecoderState state;
    ಪೂರ್ಣಾಙ್ಕ ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_SIZE);
    ಯದಿ (ret != LZMA_RESULT_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    u8 scratch[15980];
    ಪೂರ್ಣಾಙ್ಕ need = (Lzma_Get_ಅಂಕೆ_Probs(&state.Properties) * ಪರಿಮಾಣ(CProb));
    ಯದಿ (need > ಪರಿಮಾಣ(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (unsigned ಪೂರ್ಣಾಙ್ಕ)ಪರಿಮಾಣ(scratch));
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_SIZE);
    ಯದಿ (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    u32 inProcessed, outProcessed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_SIZE + 8, srclen
                     , &inProcessed, dst, dstlen, &outProcessed);
    ಯದಿ (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    ಪ್ರತ್ಯಾಗಮ dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#define CBFS_HEADER_MAGIC 0x4F524243
#define CBFS_VERSION1 0x31313131

ಸಂರಚನಾ cbfs_header {
    u32 magic;
    u32 version;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 offset;
    u32 pad[2];
} PACKED;

#define CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

ಸಂರಚನಾ cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 offset;
    ವರ್ಣ filename[0];
} PACKED;

ಸಂರಚನಾ cbfs_romfile_s {
    ಸಂರಚನಾ romfile_s file;
    ಸಂರಚನಾ cbfs_file *fhdr;
    ಶೂನ್ಯ *data;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
cbfs_copyfile(ಸಂರಚನಾ romfile_s *file, ಶೂನ್ಯ *dst, u32 maxlen)
{
    ಯದಿ (!CONFIG_COREBOOT_FLASH)
        ಪ್ರತ್ಯಾಗಮ -1;

    ಸಂರಚನಾ cbfs_romfile_s *cfile;
    cfile = container_of(file, ಸಂರಚನಾ cbfs_romfile_s, file);
    u32 size = cfile->rawsize;
    ಶೂನ್ಯ *src = cfile->data;
    ಯದಿ (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        ಶೂನ್ಯ *temp = malloc_tmphigh(size);
        ಯದಿ (!temp) {
            warn_noalloc();
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        iomemcpy(temp, src, size);
        ಪೂರ್ಣಾಙ್ಕ ret = ulzma(dst, maxlen, temp, size);
        yield();
        ಮುಕ್ತ(temp);
        ಪ್ರತ್ಯಾಗಮ ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    ಯದಿ (size > maxlen) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    iomemcpy(dst, src, size);
    ಪ್ರತ್ಯಾಗಮ size;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
ಸ್ಥಿರ ಶೂನ್ಯ
process_links_file(ಶೂನ್ಯ)
{
    ವರ್ಣ *links = romfile_loadfile("links", ರಿಕ್ತ), *next = links;
    ಯಾವತ್ (next) {
        // Parse out linkname and destname
        ವರ್ಣ *linkname = next;
        next = strchr(linkname, '\n');
        ಯದಿ (next)
            *next++ = '\0';
        ವರ್ಣ *comment = strchr(linkname, '#');
        ಯದಿ (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        ವರ್ಣ *destname = strchr(linkname, ' ');
        ಯದಿ (!destname)
            ಅನುವರ್ತ;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        ಸಂರಚನಾ romfile_s *ufile = romfile_find(destname);
        ಯದಿ (!ufile)
            ಅನುವರ್ತ;
        ಸಂರಚನಾ cbfs_romfile_s *cufile
            = container_of(ufile, ಸಂರಚನಾ cbfs_romfile_s, file);
        ಸಂರಚನಾ cbfs_romfile_s *cfile = malloc_tmp(ಪರಿಮಾಣ(*cfile));
        ಯದಿ (!cfile) {
            warn_noalloc();
            ಭಙ್ಗ;
        }
        memcpy(cfile, cufile, ಪರಿಮಾಣ(*cfile));
        strtcpy(cfile->file.name, linkname, ಪರಿಮಾಣ(cfile->file.name));
        romfile_add(&cfile->file);
    }
    ಮುಕ್ತ(links);
}

ಶೂನ್ಯ
coreboot_cbfs_init(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_COREBOOT_FLASH)
        ಪ್ರತ್ಯಾಗಮ;

    ಸಂರಚನಾ cbfs_header *hdr = *(ಶೂನ್ಯ **)(CONFIG_CBFS_LOCATION - 4);
    ಯದಿ ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        ಪ್ರತ್ಯಾಗಮ;
    }
    ಯದಿ (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (ಶೂನ್ಯ*)hdr + CONFIG_CBFS_LOCATION;
    ಯದಿ (hdr->magic != cpu_to_be32(CBFS_HEADER_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_to_be32(CBFS_HEADER_MAGIC));
        ಪ್ರತ್ಯಾಗಮ;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_to_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    ಸಂರಚನಾ cbfs_file *fhdr = (ಶೂನ್ಯ*)romstart + be32_to_cpu(hdr->offset);
    ಪ್ರತಿ (;;) {
        ಯದಿ ((u32)fhdr - romstart > romsize)
            ಭಙ್ಗ;
        u64 magic = fhdr->magic;
        ಯದಿ (magic != CBFS_FILE_MAGIC)
            ಭಙ್ಗ;
        ಸಂರಚನಾ cbfs_romfile_s *cfile = malloc_tmp(ಪರಿಮಾಣ(*cfile));
        ಯದಿ (!cfile) {
            warn_noalloc();
            ಭಙ್ಗ;
        }
        memset(cfile, 0, ಪರಿಮಾಣ(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, ಪರಿಮಾಣ(cfile->file.name));
        cfile->file.size = cfile->rawsize = be32_to_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->data = (ಶೂನ್ಯ*)fhdr + be32_to_cpu(fhdr->offset);
        ಪೂರ್ಣಾಙ್ಕ len = strlen(cfile->file.name);
        ಯದಿ (len > 5 && strcmp(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.size = *(u32*)(cfile->data + LZMA_PROPERTIES_SIZE);
        }
        romfile_add(&cfile->file);

        fhdr = (ಶೂನ್ಯ*)ALIGN((u32)cfile->data + cfile->rawsize
                            , be32_to_cpu(hdr->align));
    }

    process_links_file();
}

ಸಂರಚನಾ cbfs_payload_segment {
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

ಸಂರಚನಾ cbfs_payload {
    ಸಂರಚನಾ cbfs_payload_segment segments[1];
};

ಶೂನ್ಯ
cbfs_run_payload(ಸಂರಚನಾ cbfs_file *fhdr)
{
    ಯದಿ (!CONFIG_COREBOOT_FLASH || !fhdr)
        ಪ್ರತ್ಯಾಗಮ;
    dprintf(1, "Run %s\n", fhdr->filename);
    ಸಂರಚನಾ cbfs_payload *pay = (ಶೂನ್ಯ*)fhdr + be32_to_cpu(fhdr->offset);
    ಸಂರಚನಾ cbfs_payload_segment *seg = pay->segments;
    ಪ್ರತಿ (;;) {
        ಶೂನ್ಯ *src = (ಶೂನ್ಯ*)pay + be32_to_cpu(seg->offset);
        ಶೂನ್ಯ *dest = (ಶೂನ್ಯ*)(u32)be64_to_cpu(seg->load_addr);
        u32 src_len = be32_to_cpu(seg->len);
        u32 dest_len = be32_to_cpu(seg->mem_len);
        ವಿಕಲ್ಪ (seg->type) {
        ಪಕ್ಷ PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            memset(dest, 0, dest_len);
            ಭಙ್ಗ;
        ಪಕ್ಷ PAYLOAD_SEGMENT_ENTRY: {
            dprintf(1, "Calling addr %p\n", dest);
            ಶೂನ್ಯ (*func)(ಶೂನ್ಯ) = dest;
            func();
            ಪ್ರತ್ಯಾಗಮ;
        }
        ಸ್ವಭಾವ:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            ಯದಿ (seg->compression == cpu_to_be32(CBFS_COMPRESS_NONE)) {
                ಯದಿ (src_len > dest_len)
                    src_len = dest_len;
                memcpy(dest, src, src_len);
            } ಅನ್ಯಥಾ ಯದಿ (CONFIG_LZMA
                       && seg->compression == cpu_to_be32(CBFS_COMPRESS_LZMA)) {
                ಪೂರ್ಣಾಙ್ಕ ret = ulzma(dest, dest_len, src, src_len);
                ಯದಿ (ret < 0)
                    ಪ್ರತ್ಯಾಗಮ;
                src_len = ret;
            } ಅನ್ಯಥಾ {
                dprintf(1, "No support for compression type %x\n"
                        , seg->compression);
                ಪ್ರತ್ಯಾಗಮ;
            }
            ಯದಿ (dest_len > src_len)
                memset(dest + src_len, 0, dest_len - src_len);
            ಭಙ್ಗ;
        }
        seg++;
    }
}

// Register payloads in "img/" directory with boot system.
ಶೂನ್ಯ
cbfs_payload_setup(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_COREBOOT_FLASH)
        ಪ್ರತ್ಯಾಗಮ;
    ಸಂರಚನಾ romfile_s *file = ರಿಕ್ತ;
    ಪ್ರತಿ (;;) {
        file = romfile_findprefix("img/", file);
        ಯದಿ (!file)
            ಭಙ್ಗ;
        ಸಂರಚನಾ cbfs_romfile_s *cfile;
        cfile = container_of(file, ಸಂರಚನಾ cbfs_romfile_s, file);
        ಧ್ರುವ ವರ್ಣ *filename = file->name;
        ವರ್ಣ *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_add_cbfs(cfile->fhdr, desc, bootprio_find_named_rom(filename, 0));
    }
}

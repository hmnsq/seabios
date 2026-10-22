// Coreboot interface support.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "block.h" // MAXDESCSIZE
#ଅନ୍ତର୍ଭାଵ "byteorder.h" // be32_to_cpu
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "e820map.h" // e820_add
#ଅନ୍ତର୍ଭାଵ "hw/pcidevice.h" // pci_probe_devices
#ଅନ୍ତର୍ଭାଵ "lzmadecode.h" // LzmaDecode
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "paravirt.h" // PlatformRunningOn
#ଅନ୍ତର୍ଭାଵ "romfile.h" // romfile_findprefix
#ଅନ୍ତର୍ଭାଵ "stacks.h" // yield
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // coreboot_preinit


/****************************************************************
 * Memory map
 ****************************************************************/

ସଂରଚନା cb_header {
    u32 signature;
    u32 header_bytes;
    u32 header_checksum;
    u32 table_bytes;
    u32 table_checksum;
    u32 table_entries;
};

#define CB_SIGNATURE 0x4f49424C // "LBIO"

ସଂରଚନା cb_memory_range {
    u64 start;
    u64 size;
    u32 type;
};

#define CB_MEM_TABLE    16

ସଂରଚନା cb_memory {
    u32 tag;
    u32 size;
    ସଂରଚନା cb_memory_range map[0];
};

#define CB_TAG_MEMORY 0x01

#define MEM_RANGE_ଗଣନା(_rec) \
        (((_rec)->size - ଆକାର(*(_rec))) / ଆକାର((_rec)->map[0]))

ସଂରଚନା cb_mainboard {
    u32 tag;
    u32 size;
    u8  vendor_idx;
    u8  part_idx;
    ଅକ୍ଷର  strings[0];
};

#define CB_TAG_MAINBOARD 0x0003

ସଂରଚନା cb_forward {
    u32 tag;
    u32 size;
    u64 forward;
};

#define CB_TAG_FORWARD 0x11

ସଂରଚନା cb_cbmem_ref {
    u32 tag;
    u32 size;
    u64 cbmem_addr;
};

#define CB_TAG_CBMEM_CONSOLE 0x17

ସଂରଚନା cbmem_console {
    u32 size;
    u32 cursor;
    u8  body[0];
} PACKED;
#define CBMC_CURSOR_MASK ((1 << 28) - 1)
#define CBMC_OVERFLOW (1 << 31)
ସ୍ଥିର ସଂରଚନା cbmem_console *cbcon = ଖାଲି;

ସ୍ଥିର u16
ipchksum(ଅକ୍ଷର *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2)
{
    u16 *p = (u16*)buf;
    u32 sum = 0;
    ଯେତେବେଳେ (ଗଣନା_2 > 1) {
        sum += GET_FARVAR(0, *p);
        p++;
        ଗଣନା_2 -= 2;
    }
    ଯଦି (ଗଣନା_2)
        sum += GET_FARVAR(0, *(u8*)p);
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    ଫେରନ୍ତୁ ~sum;
}

// Try to locate the coreboot header in a given address range.
ସ୍ଥିର ସଂରଚନା cb_header *
find_cb_header(u32 addr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len)
{
    u32 end = addr + len;
    ପାଇଁ (; addr < end; addr += 16) {
        ସଂରଚନା cb_header *cbh = (ଶୂନ୍ୟ*)addr;
        ଯଦି (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            ଜାରି;
        u32 tsize = GET_FARVAR(0, cbh->table_bytes);
        ଯଦି (! tsize)
            ଜାରି;
        ଯଦି (ipchksum((ଶୂନ୍ୟ*)addr, ଆକାର(*cbh)) != 0)
            ଜାରି;
        ଯଦି (ipchksum((ଶୂନ୍ୟ*)addr + ଆକାର(*cbh), tsize)
            != GET_FARVAR(0, cbh->table_checksum))
            ଜାରି;
        ଫେରନ୍ତୁ cbh;
    }
    ଫେରନ୍ତୁ ଖାଲି;
}

// Try to find the coreboot memory table in the given coreboot table.
ଶୂନ୍ୟ *
find_cb_subtable(ସଂରଚନା cb_header *cbh, u32 tag)
{
    ଅକ୍ଷର *tbl = (ଅକ୍ଷର *)cbh + ଆକାର(*cbh);
    u32 ଗଣନା_2 = GET_FARVAR(0, cbh->table_entries);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<ଗଣନା_2; i++) {
        ସଂରଚନା cb_memory *cbm = (ଶୂନ୍ୟ*)tbl;
        tbl += GET_FARVAR(0, cbm->size);
        ଯଦି (GET_FARVAR(0, cbm->tag) == tag)
            ଫେରନ୍ତୁ cbm;
    }
    ଫେରନ୍ତୁ ଖାଲି;
}

ସଂରଚନା cb_header *
find_cb_table(ଶୂନ୍ୟ)
{
    ସଂରଚନା cb_header *cbh = find_cb_header(0, 0x1000);
    ଯଦି (!cbh)
        ଫେରନ୍ତୁ ଖାଲି;
    ସଂରଚନା cb_forward *cbf = find_cb_subtable(cbh, CB_TAG_FORWARD);
    ଯଦି (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = find_cb_header(GET_FARVAR(0, cbf->forward), 0x100);
        ଯଦି (!cbh)
            ଫେରନ୍ତୁ ଖାଲି;
    }
    ଫେରନ୍ତୁ cbh;
}

ସ୍ଥିର ସଂରଚନା cb_memory *CBMemTable;
ଅଚଳ ଅକ୍ଷର *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
ଶୂନ୍ୟ
coreboot_preinit(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_COREBOOT)
        ଫେରନ୍ତୁ;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    ସଂରଚନା cb_header *cbh = find_cb_table();
    ଯଦି (!cbh)
        ଯାଅ fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    ସଂରଚନା cb_memory *cbm = CBMemTable = find_cb_subtable(cbh, CB_TAG_MEMORY);
    ଯଦି (!cbm)
        ଯାଅ fail;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, ଗଣନା_2 = MEM_RANGE_ଗଣନା(cbm);
    ପାଇଁ (i=0; i<ଗଣନା_2; i++) {
        ସଂରଚନା cb_memory_range *m = &cbm->map[i];
        u32 type = m->type;
        ଯଦି (type == CB_MEM_TABLE)
            type = E820_RESERVED;
        e820_add(m->start, m->size, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_add(0, 16*1024, E820_RAM);

    ସଂରଚନା cb_cbmem_ref *cbref = find_cb_subtable(cbh, CB_TAG_CBMEM_CONSOLE);
    ଯଦି (cbref) {
        cbcon = (ଶୂନ୍ୟ*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    ସଂରଚନା cb_mainboard *cbmb = find_cb_subtable(cbh, CB_TAG_MAINBOARD);
    ଯଦି (cbmb) {
        CBvendor = &cbmb->strings[cbmb->vendor_idx];
        CBpart = &cbmb->strings[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    ଫେରନ୍ତୁ;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_add(0, 16*1024*1024, E820_RAM);
    ଫେରନ୍ତୁ;
}

ଶୂନ୍ୟ coreboot_debug_putc(ଅକ୍ଷର c)
{
    ଯଦି (!CONFIG_DEBUG_COREBOOT)
        ଫେରନ୍ତୁ;
    ଯଦି (!cbcon)
        ଫେରନ୍ତୁ;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_MASK;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_MASK;
    ଯଦି (cursor >= cbcon->size)
        ଫେରନ୍ତୁ; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    ଯଦି (cursor >= cbcon->size) {
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
ସ୍ଥିର ଶୂନ୍ୟ
scan_tables(u32 start, u32 size)
{
    ଶୂନ୍ୟ *p = (ଶୂନ୍ୟ*)ALIGN(start, 16);
    ଶୂନ୍ୟ *end = (ଶୂନ୍ୟ*)start + size;
    ପାଇଁ (; p<end; p += 16)
        copy_table(p);
}

ଶୂନ୍ୟ
coreboot_platform_setup(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_COREBOOT)
        ଫେରନ୍ତୁ;
    pci_probe_devices();

    ସଂରଚନା cb_memory *cbm = CBMemTable;
    ଯଦି (!cbm)
        ଫେରନ୍ତୁ;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, ଗଣନା_2 = MEM_RANGE_ଗଣନା(cbm);
    ପାଇଁ (i=0; i<ଗଣନା_2; i++) {
        ସଂରଚନା cb_memory_range *m = &cbm->map[i];
        ଯଦି (m->type == CB_MEM_TABLE)
            scan_tables(m->start, m->size);
    }

    find_acpi_features();
}


/****************************************************************
 * ulzma
 ****************************************************************/

// Uncompress data in flash to an area of memory.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ulzma(u8 *dst, u32 maxlen, ଅଚଳ u8 *src, u32 srclen)
{
    dprintf(3, "Uncompressing data %d@%p to %d@%p\n", srclen, src, maxlen, dst);
    CLzmaDecoderState state;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_SIZE);
    ଯଦି (ret != LZMA_RESULT_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        ଫେରନ୍ତୁ -1;
    }
    u8 scratch[15980];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା need = (Lzma_Get_ଅଙ୍କ_Probs(&state.Properties) * ଆକାର(CProb));
    ଯଦି (need > ଆକାର(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ଆକାର(scratch));
        ଫେରନ୍ତୁ -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_SIZE);
    ଯଦି (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        ଫେରନ୍ତୁ -1;
    }
    u32 inProcessed, outProcessed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_SIZE + 8, srclen
                     , &inProcessed, dst, dstlen, &outProcessed);
    ଯଦି (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        ଫେରନ୍ତୁ -1;
    }
    ଫେରନ୍ତୁ dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#define CBFS_HEADER_MAGIC 0x4F524243
#define CBFS_VERSION1 0x31313131

ସଂରଚନା cbfs_header {
    u32 magic;
    u32 version;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 offset;
    u32 pad[2];
} PACKED;

#define CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

ସଂରଚନା cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 offset;
    ଅକ୍ଷର filename[0];
} PACKED;

ସଂରଚନା cbfs_romfile_s {
    ସଂରଚନା romfile_s file;
    ସଂରଚନା cbfs_file *fhdr;
    ଶୂନ୍ୟ *data;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
cbfs_copyfile(ସଂରଚନା romfile_s *file, ଶୂନ୍ୟ *dst, u32 maxlen)
{
    ଯଦି (!CONFIG_COREBOOT_FLASH)
        ଫେରନ୍ତୁ -1;

    ସଂରଚନା cbfs_romfile_s *cfile;
    cfile = container_of(file, ସଂରଚନା cbfs_romfile_s, file);
    u32 size = cfile->rawsize;
    ଶୂନ୍ୟ *src = cfile->data;
    ଯଦି (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        ଶୂନ୍ୟ *temp = malloc_tmphigh(size);
        ଯଦି (!temp) {
            warn_noalloc();
            ଫେରନ୍ତୁ -1;
        }
        iomemcpy(temp, src, size);
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = ulzma(dst, maxlen, temp, size);
        yield();
        ମୁକ୍ତ(temp);
        ଫେରନ୍ତୁ ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    ଯଦି (size > maxlen) {
        warn_noalloc();
        ଫେରନ୍ତୁ -1;
    }
    iomemcpy(dst, src, size);
    ଫେରନ୍ତୁ size;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
ସ୍ଥିର ଶୂନ୍ୟ
process_links_file(ଶୂନ୍ୟ)
{
    ଅକ୍ଷର *links = romfile_loadfile("links", ଖାଲି), *next = links;
    ଯେତେବେଳେ (next) {
        // Parse out linkname and destname
        ଅକ୍ଷର *linkname = next;
        next = strchr(linkname, '\n');
        ଯଦି (next)
            *next++ = '\0';
        ଅକ୍ଷର *comment = strchr(linkname, '#');
        ଯଦି (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        ଅକ୍ଷର *destname = strchr(linkname, ' ');
        ଯଦି (!destname)
            ଜାରି;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        ସଂରଚନା romfile_s *ufile = romfile_find(destname);
        ଯଦି (!ufile)
            ଜାରି;
        ସଂରଚନା cbfs_romfile_s *cufile
            = container_of(ufile, ସଂରଚନା cbfs_romfile_s, file);
        ସଂରଚନା cbfs_romfile_s *cfile = malloc_tmp(ଆକାର(*cfile));
        ଯଦି (!cfile) {
            warn_noalloc();
            ଭାଙ୍ଗ;
        }
        memcpy(cfile, cufile, ଆକାର(*cfile));
        strtcpy(cfile->file.name, linkname, ଆକାର(cfile->file.name));
        romfile_add(&cfile->file);
    }
    ମୁକ୍ତ(links);
}

ଶୂନ୍ୟ
coreboot_cbfs_init(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_COREBOOT_FLASH)
        ଫେରନ୍ତୁ;

    ସଂରଚନା cbfs_header *hdr = *(ଶୂନ୍ୟ **)(CONFIG_CBFS_LOCATION - 4);
    ଯଦି ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        ଫେରନ୍ତୁ;
    }
    ଯଦି (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (ଶୂନ୍ୟ*)hdr + CONFIG_CBFS_LOCATION;
    ଯଦି (hdr->magic != cpu_to_be32(CBFS_HEADER_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_to_be32(CBFS_HEADER_MAGIC));
        ଫେରନ୍ତୁ;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_to_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    ସଂରଚନା cbfs_file *fhdr = (ଶୂନ୍ୟ*)romstart + be32_to_cpu(hdr->offset);
    ପାଇଁ (;;) {
        ଯଦି ((u32)fhdr - romstart > romsize)
            ଭାଙ୍ଗ;
        u64 magic = fhdr->magic;
        ଯଦି (magic != CBFS_FILE_MAGIC)
            ଭାଙ୍ଗ;
        ସଂରଚନା cbfs_romfile_s *cfile = malloc_tmp(ଆକାର(*cfile));
        ଯଦି (!cfile) {
            warn_noalloc();
            ଭାଙ୍ଗ;
        }
        memset(cfile, 0, ଆକାର(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, ଆକାର(cfile->file.name));
        cfile->file.size = cfile->rawsize = be32_to_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->data = (ଶୂନ୍ୟ*)fhdr + be32_to_cpu(fhdr->offset);
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = strlen(cfile->file.name);
        ଯଦି (len > 5 && strcmp(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.size = *(u32*)(cfile->data + LZMA_PROPERTIES_SIZE);
        }
        romfile_add(&cfile->file);

        fhdr = (ଶୂନ୍ୟ*)ALIGN((u32)cfile->data + cfile->rawsize
                            , be32_to_cpu(hdr->align));
    }

    process_links_file();
}

ସଂରଚନା cbfs_payload_segment {
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

ସଂରଚନା cbfs_payload {
    ସଂରଚନା cbfs_payload_segment segments[1];
};

ଶୂନ୍ୟ
cbfs_run_payload(ସଂରଚନା cbfs_file *fhdr)
{
    ଯଦି (!CONFIG_COREBOOT_FLASH || !fhdr)
        ଫେରନ୍ତୁ;
    dprintf(1, "Run %s\n", fhdr->filename);
    ସଂରଚନା cbfs_payload *pay = (ଶୂନ୍ୟ*)fhdr + be32_to_cpu(fhdr->offset);
    ସଂରଚନା cbfs_payload_segment *seg = pay->segments;
    ପାଇଁ (;;) {
        ଶୂନ୍ୟ *src = (ଶୂନ୍ୟ*)pay + be32_to_cpu(seg->offset);
        ଶୂନ୍ୟ *dest = (ଶୂନ୍ୟ*)(u32)be64_to_cpu(seg->load_addr);
        u32 src_len = be32_to_cpu(seg->len);
        u32 dest_len = be32_to_cpu(seg->mem_len);
        ବାଛ (seg->type) {
        ସ୍ଥିତି PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            memset(dest, 0, dest_len);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି PAYLOAD_SEGMENT_ENTRY: {
            dprintf(1, "Calling addr %p\n", dest);
            ଶୂନ୍ୟ (*func)(ଶୂନ୍ୟ) = dest;
            func();
            ଫେରନ୍ତୁ;
        }
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            ଯଦି (seg->compression == cpu_to_be32(CBFS_COMPRESS_NONE)) {
                ଯଦି (src_len > dest_len)
                    src_len = dest_len;
                memcpy(dest, src, src_len);
            } ଅନ୍ୟଥା ଯଦି (CONFIG_LZMA
                       && seg->compression == cpu_to_be32(CBFS_COMPRESS_LZMA)) {
                ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = ulzma(dest, dest_len, src, src_len);
                ଯଦି (ret < 0)
                    ଫେରନ୍ତୁ;
                src_len = ret;
            } ଅନ୍ୟଥା {
                dprintf(1, "No support for compression type %x\n"
                        , seg->compression);
                ଫେରନ୍ତୁ;
            }
            ଯଦି (dest_len > src_len)
                memset(dest + src_len, 0, dest_len - src_len);
            ଭାଙ୍ଗ;
        }
        seg++;
    }
}

// Register payloads in "img/" directory with boot system.
ଶୂନ୍ୟ
cbfs_payload_setup(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_COREBOOT_FLASH)
        ଫେରନ୍ତୁ;
    ସଂରଚନା romfile_s *file = ଖାଲି;
    ପାଇଁ (;;) {
        file = romfile_findprefix("img/", file);
        ଯଦି (!file)
            ଭାଙ୍ଗ;
        ସଂରଚନା cbfs_romfile_s *cfile;
        cfile = container_of(file, ସଂରଚନା cbfs_romfile_s, file);
        ଅଚଳ ଅକ୍ଷର *filename = file->name;
        ଅକ୍ଷର *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_add_cbfs(cfile->fhdr, desc, bootprio_find_named_rom(filename, 0));
    }
}

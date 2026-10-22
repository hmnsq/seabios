// Coreboot interface support.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "block.h" // MAXDESCSIZE
#અન્તર્ભાવ "byteorder.h" // be32_to_cpu
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "e820map.h" // e820_add
#અન્તર્ભાવ "hw/pcidevice.h" // pci_probe_devices
#અન્તર્ભાવ "lzmadecode.h" // LzmaDecode
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "paravirt.h" // PlatformRunningOn
#અન્તર્ભાવ "romfile.h" // romfile_findprefix
#અન્તર્ભાવ "stacks.h" // yield
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // coreboot_preinit


/****************************************************************
 * Memory map
 ****************************************************************/

સંરચના cb_header {
    u32 signature;
    u32 header_bytes;
    u32 header_checksum;
    u32 table_bytes;
    u32 table_checksum;
    u32 table_entries;
};

#define CB_SIGNATURE 0x4f49424C // "LBIO"

સંરચના cb_memory_range {
    u64 start;
    u64 size;
    u32 type;
};

#define CB_MEM_TABLE    16

સંરચના cb_memory {
    u32 tag;
    u32 size;
    સંરચના cb_memory_range map[0];
};

#define CB_TAG_MEMORY 0x01

#define MEM_RANGE_ગણતરી(_rec) \
        (((_rec)->size - કદ(*(_rec))) / કદ((_rec)->map[0]))

સંરચના cb_mainboard {
    u32 tag;
    u32 size;
    u8  vendor_idx;
    u8  part_idx;
    અક્ષર  strings[0];
};

#define CB_TAG_MAINBOARD 0x0003

સંરચના cb_forward {
    u32 tag;
    u32 size;
    u64 forward;
};

#define CB_TAG_FORWARD 0x11

સંરચના cb_cbmem_ref {
    u32 tag;
    u32 size;
    u64 cbmem_addr;
};

#define CB_TAG_CBMEM_CONSOLE 0x17

સંરચના cbmem_console {
    u32 size;
    u32 cursor;
    u8  body[0];
} PACKED;
#define CBMC_CURSOR_MASK ((1 << 28) - 1)
#define CBMC_OVERFLOW (1 << 31)
સ્થિર સંરચના cbmem_console *cbcon = ખાલી;

સ્થિર u16
ipchksum(અક્ષર *buf, પૂર્ણાંક ગણતરી)
{
    u16 *p = (u16*)buf;
    u32 sum = 0;
    જ્યારે (ગણતરી > 1) {
        sum += GET_FARVAR(0, *p);
        p++;
        ગણતરી -= 2;
    }
    જો (ગણતરી)
        sum += GET_FARVAR(0, *(u8*)p);
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    પાછા ~sum;
}

// Try to locate the coreboot header in a given address range.
સ્થિર સંરચના cb_header *
find_cb_header(u32 addr, પૂર્ણાંક len)
{
    u32 end = addr + len;
    માટે (; addr < end; addr += 16) {
        સંરચના cb_header *cbh = (શૂન્ય*)addr;
        જો (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            ચાલુ;
        u32 tsize = GET_FARVAR(0, cbh->table_bytes);
        જો (! tsize)
            ચાલુ;
        જો (ipchksum((શૂન્ય*)addr, કદ(*cbh)) != 0)
            ચાલુ;
        જો (ipchksum((શૂન્ય*)addr + કદ(*cbh), tsize)
            != GET_FARVAR(0, cbh->table_checksum))
            ચાલુ;
        પાછા cbh;
    }
    પાછા ખાલી;
}

// Try to find the coreboot memory table in the given coreboot table.
શૂન્ય *
find_cb_subtable(સંરચના cb_header *cbh, u32 tag)
{
    અક્ષર *tbl = (અક્ષર *)cbh + કદ(*cbh);
    u32 ગણતરી = GET_FARVAR(0, cbh->table_entries);
    પૂર્ણાંક i;
    માટે (i=0; i<ગણતરી; i++) {
        સંરચના cb_memory *cbm = (શૂન્ય*)tbl;
        tbl += GET_FARVAR(0, cbm->size);
        જો (GET_FARVAR(0, cbm->tag) == tag)
            પાછા cbm;
    }
    પાછા ખાલી;
}

સંરચના cb_header *
find_cb_table(શૂન્ય)
{
    સંરચના cb_header *cbh = find_cb_header(0, 0x1000);
    જો (!cbh)
        પાછા ખાલી;
    સંરચના cb_forward *cbf = find_cb_subtable(cbh, CB_TAG_FORWARD);
    જો (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = find_cb_header(GET_FARVAR(0, cbf->forward), 0x100);
        જો (!cbh)
            પાછા ખાલી;
    }
    પાછા cbh;
}

સ્થિર સંરચના cb_memory *CBMemTable;
અચલ અક્ષર *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
શૂન્ય
coreboot_preinit(શૂન્ય)
{
    જો (!CONFIG_COREBOOT)
        પાછા;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    સંરચના cb_header *cbh = find_cb_table();
    જો (!cbh)
        જાઓ fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    સંરચના cb_memory *cbm = CBMemTable = find_cb_subtable(cbh, CB_TAG_MEMORY);
    જો (!cbm)
        જાઓ fail;

    પૂર્ણાંક i, ગણતરી = MEM_RANGE_ગણતરી(cbm);
    માટે (i=0; i<ગણતરી; i++) {
        સંરચના cb_memory_range *m = &cbm->map[i];
        u32 type = m->type;
        જો (type == CB_MEM_TABLE)
            type = E820_RESERVED;
        e820_add(m->start, m->size, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_add(0, 16*1024, E820_RAM);

    સંરચના cb_cbmem_ref *cbref = find_cb_subtable(cbh, CB_TAG_CBMEM_CONSOLE);
    જો (cbref) {
        cbcon = (શૂન્ય*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    સંરચના cb_mainboard *cbmb = find_cb_subtable(cbh, CB_TAG_MAINBOARD);
    જો (cbmb) {
        CBvendor = &cbmb->strings[cbmb->vendor_idx];
        CBpart = &cbmb->strings[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    પાછા;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_add(0, 16*1024*1024, E820_RAM);
    પાછા;
}

શૂન્ય coreboot_debug_putc(અક્ષર c)
{
    જો (!CONFIG_DEBUG_COREBOOT)
        પાછા;
    જો (!cbcon)
        પાછા;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_MASK;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_MASK;
    જો (cursor >= cbcon->size)
        પાછા; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    જો (cursor >= cbcon->size) {
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
સ્થિર શૂન્ય
scan_tables(u32 start, u32 size)
{
    શૂન્ય *p = (શૂન્ય*)ALIGN(start, 16);
    શૂન્ય *end = (શૂન્ય*)start + size;
    માટે (; p<end; p += 16)
        copy_table(p);
}

શૂન્ય
coreboot_platform_setup(શૂન્ય)
{
    જો (!CONFIG_COREBOOT)
        પાછા;
    pci_probe_devices();

    સંરચના cb_memory *cbm = CBMemTable;
    જો (!cbm)
        પાછા;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    પૂર્ણાંક i, ગણતરી = MEM_RANGE_ગણતરી(cbm);
    માટે (i=0; i<ગણતરી; i++) {
        સંરચના cb_memory_range *m = &cbm->map[i];
        જો (m->type == CB_MEM_TABLE)
            scan_tables(m->start, m->size);
    }

    find_acpi_features();
}


/****************************************************************
 * ulzma
 ****************************************************************/

// Uncompress data in flash to an area of memory.
સ્થિર પૂર્ણાંક
ulzma(u8 *dst, u32 maxlen, અચલ u8 *src, u32 srclen)
{
    dprintf(3, "Uncompressing data %d@%p to %d@%p\n", srclen, src, maxlen, dst);
    CLzmaDecoderState state;
    પૂર્ણાંક ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_SIZE);
    જો (ret != LZMA_RESULT_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        પાછા -1;
    }
    u8 scratch[15980];
    પૂર્ણાંક need = (Lzma_Get_અંક_Probs(&state.Properties) * કદ(CProb));
    જો (need > કદ(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (unsigned પૂર્ણાંક)કદ(scratch));
        પાછા -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_SIZE);
    જો (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        પાછા -1;
    }
    u32 inProcessed, outProcessed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_SIZE + 8, srclen
                     , &inProcessed, dst, dstlen, &outProcessed);
    જો (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        પાછા -1;
    }
    પાછા dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#define CBFS_HEADER_MAGIC 0x4F524243
#define CBFS_VERSION1 0x31313131

સંરચના cbfs_header {
    u32 magic;
    u32 version;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 offset;
    u32 pad[2];
} PACKED;

#define CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

સંરચના cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 offset;
    અક્ષર filename[0];
} PACKED;

સંરચના cbfs_romfile_s {
    સંરચના romfile_s file;
    સંરચના cbfs_file *fhdr;
    શૂન્ય *data;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
સ્થિર પૂર્ણાંક
cbfs_copyfile(સંરચના romfile_s *file, શૂન્ય *dst, u32 maxlen)
{
    જો (!CONFIG_COREBOOT_FLASH)
        પાછા -1;

    સંરચના cbfs_romfile_s *cfile;
    cfile = container_of(file, સંરચના cbfs_romfile_s, file);
    u32 size = cfile->rawsize;
    શૂન્ય *src = cfile->data;
    જો (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        શૂન્ય *temp = malloc_tmphigh(size);
        જો (!temp) {
            warn_noalloc();
            પાછા -1;
        }
        iomemcpy(temp, src, size);
        પૂર્ણાંક ret = ulzma(dst, maxlen, temp, size);
        yield();
        મુક્ત(temp);
        પાછા ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    જો (size > maxlen) {
        warn_noalloc();
        પાછા -1;
    }
    iomemcpy(dst, src, size);
    પાછા size;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
સ્થિર શૂન્ય
process_links_file(શૂન્ય)
{
    અક્ષર *links = romfile_loadfile("links", ખાલી), *next = links;
    જ્યારે (next) {
        // Parse out linkname and destname
        અક્ષર *linkname = next;
        next = strchr(linkname, '\n');
        જો (next)
            *next++ = '\0';
        અક્ષર *comment = strchr(linkname, '#');
        જો (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        અક્ષર *destname = strchr(linkname, ' ');
        જો (!destname)
            ચાલુ;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        સંરચના romfile_s *ufile = romfile_find(destname);
        જો (!ufile)
            ચાલુ;
        સંરચના cbfs_romfile_s *cufile
            = container_of(ufile, સંરચના cbfs_romfile_s, file);
        સંરચના cbfs_romfile_s *cfile = malloc_tmp(કદ(*cfile));
        જો (!cfile) {
            warn_noalloc();
            તોડો;
        }
        memcpy(cfile, cufile, કદ(*cfile));
        strtcpy(cfile->file.name, linkname, કદ(cfile->file.name));
        romfile_add(&cfile->file);
    }
    મુક્ત(links);
}

શૂન્ય
coreboot_cbfs_init(શૂન્ય)
{
    જો (!CONFIG_COREBOOT_FLASH)
        પાછા;

    સંરચના cbfs_header *hdr = *(શૂન્ય **)(CONFIG_CBFS_LOCATION - 4);
    જો ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        પાછા;
    }
    જો (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (શૂન્ય*)hdr + CONFIG_CBFS_LOCATION;
    જો (hdr->magic != cpu_to_be32(CBFS_HEADER_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_to_be32(CBFS_HEADER_MAGIC));
        પાછા;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_to_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    સંરચના cbfs_file *fhdr = (શૂન્ય*)romstart + be32_to_cpu(hdr->offset);
    માટે (;;) {
        જો ((u32)fhdr - romstart > romsize)
            તોડો;
        u64 magic = fhdr->magic;
        જો (magic != CBFS_FILE_MAGIC)
            તોડો;
        સંરચના cbfs_romfile_s *cfile = malloc_tmp(કદ(*cfile));
        જો (!cfile) {
            warn_noalloc();
            તોડો;
        }
        memset(cfile, 0, કદ(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, કદ(cfile->file.name));
        cfile->file.size = cfile->rawsize = be32_to_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->data = (શૂન્ય*)fhdr + be32_to_cpu(fhdr->offset);
        પૂર્ણાંક len = strlen(cfile->file.name);
        જો (len > 5 && strcmp(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.size = *(u32*)(cfile->data + LZMA_PROPERTIES_SIZE);
        }
        romfile_add(&cfile->file);

        fhdr = (શૂન્ય*)ALIGN((u32)cfile->data + cfile->rawsize
                            , be32_to_cpu(hdr->align));
    }

    process_links_file();
}

સંરચના cbfs_payload_segment {
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

સંરચના cbfs_payload {
    સંરચના cbfs_payload_segment segments[1];
};

શૂન્ય
cbfs_run_payload(સંરચના cbfs_file *fhdr)
{
    જો (!CONFIG_COREBOOT_FLASH || !fhdr)
        પાછા;
    dprintf(1, "Run %s\n", fhdr->filename);
    સંરચના cbfs_payload *pay = (શૂન્ય*)fhdr + be32_to_cpu(fhdr->offset);
    સંરચના cbfs_payload_segment *seg = pay->segments;
    માટે (;;) {
        શૂન્ય *src = (શૂન્ય*)pay + be32_to_cpu(seg->offset);
        શૂન્ય *dest = (શૂન્ય*)(u32)be64_to_cpu(seg->load_addr);
        u32 src_len = be32_to_cpu(seg->len);
        u32 dest_len = be32_to_cpu(seg->mem_len);
        પસંદગી (seg->type) {
        સ્થિતિ PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            memset(dest, 0, dest_len);
            તોડો;
        સ્થિતિ PAYLOAD_SEGMENT_ENTRY: {
            dprintf(1, "Calling addr %p\n", dest);
            શૂન્ય (*func)(શૂન્ય) = dest;
            func();
            પાછા;
        }
        મૂળભૂત:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            જો (seg->compression == cpu_to_be32(CBFS_COMPRESS_NONE)) {
                જો (src_len > dest_len)
                    src_len = dest_len;
                memcpy(dest, src, src_len);
            } નહીંતર જો (CONFIG_LZMA
                       && seg->compression == cpu_to_be32(CBFS_COMPRESS_LZMA)) {
                પૂર્ણાંક ret = ulzma(dest, dest_len, src, src_len);
                જો (ret < 0)
                    પાછા;
                src_len = ret;
            } નહીંતર {
                dprintf(1, "No support for compression type %x\n"
                        , seg->compression);
                પાછા;
            }
            જો (dest_len > src_len)
                memset(dest + src_len, 0, dest_len - src_len);
            તોડો;
        }
        seg++;
    }
}

// Register payloads in "img/" directory with boot system.
શૂન્ય
cbfs_payload_setup(શૂન્ય)
{
    જો (!CONFIG_COREBOOT_FLASH)
        પાછા;
    સંરચના romfile_s *file = ખાલી;
    માટે (;;) {
        file = romfile_findprefix("img/", file);
        જો (!file)
            તોડો;
        સંરચના cbfs_romfile_s *cfile;
        cfile = container_of(file, સંરચના cbfs_romfile_s, file);
        અચલ અક્ષર *filename = file->name;
        અક્ષર *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_add_cbfs(cfile->fhdr, desc, bootprio_find_named_rom(filename, 0));
    }
}

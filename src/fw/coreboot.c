// Coreboot interface support.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "block.h" // MAXDESCSIZE
#അന്തര്ഭാവ "byteorder.h" // be32_to_cpu
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "e820map.h" // e820_add
#അന്തര്ഭാവ "hw/pcidevice.h" // pci_probe_devices
#അന്തര്ഭാവ "lzmadecode.h" // LzmaDecode
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "paravirt.h" // PlatformRunningOn
#അന്തര്ഭാവ "romfile.h" // romfile_findprefix
#അന്തര്ഭാവ "stacks.h" // yield
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // coreboot_preinit


/****************************************************************
 * Memory map
 ****************************************************************/

ഘടന cb_header {
    u32 signature;
    u32 header_bytes;
    u32 header_checksum;
    u32 table_bytes;
    u32 table_checksum;
    u32 table_entries;
};

#define CB_SIGNATURE 0x4f49424C // "LBIO"

ഘടന cb_memory_range {
    u64 start;
    u64 size;
    u32 type;
};

#define CB_MEM_TABLE    16

ഘടന cb_memory {
    u32 tag;
    u32 size;
    ഘടന cb_memory_range map[0];
};

#define CB_TAG_MEMORY 0x01

#define MEM_RANGE_എണ്ണം(_rec) \
        (((_rec)->size - വലുപ്പം(*(_rec))) / വലുപ്പം((_rec)->map[0]))

ഘടന cb_mainboard {
    u32 tag;
    u32 size;
    u8  vendor_idx;
    u8  part_idx;
    അക്ഷരം  strings[0];
};

#define CB_TAG_MAINBOARD 0x0003

ഘടന cb_forward {
    u32 tag;
    u32 size;
    u64 forward;
};

#define CB_TAG_FORWARD 0x11

ഘടന cb_cbmem_ref {
    u32 tag;
    u32 size;
    u64 cbmem_addr;
};

#define CB_TAG_CBMEM_CONSOLE 0x17

ഘടന cbmem_console {
    u32 size;
    u32 cursor;
    u8  body[0];
} PACKED;
#define CBMC_CURSOR_MASK ((1 << 28) - 1)
#define CBMC_OVERFLOW (1 << 31)
സ്ഥിതം ഘടന cbmem_console *cbcon = ശൂന്യ;

സ്ഥിതം u16
ipchksum(അക്ഷരം *buf, പൂർണ്ണസംഖ്യ എണ്ണം)
{
    u16 *p = (u16*)buf;
    u32 sum = 0;
    ആകുമ്പോൾ (എണ്ണം > 1) {
        sum += GET_FARVAR(0, *p);
        p++;
        എണ്ണം -= 2;
    }
    എങ്കിൽ (എണ്ണം)
        sum += GET_FARVAR(0, *(u8*)p);
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    തിരികെ ~sum;
}

// Try to locate the coreboot header in a given address range.
സ്ഥിതം ഘടന cb_header *
find_cb_header(u32 addr, പൂർണ്ണസംഖ്യ len)
{
    u32 end = addr + len;
    വേണ്ടി (; addr < end; addr += 16) {
        ഘടന cb_header *cbh = (ശൂന്യം*)addr;
        എങ്കിൽ (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            തുടരുക;
        u32 tsize = GET_FARVAR(0, cbh->table_bytes);
        എങ്കിൽ (! tsize)
            തുടരുക;
        എങ്കിൽ (ipchksum((ശൂന്യം*)addr, വലുപ്പം(*cbh)) != 0)
            തുടരുക;
        എങ്കിൽ (ipchksum((ശൂന്യം*)addr + വലുപ്പം(*cbh), tsize)
            != GET_FARVAR(0, cbh->table_checksum))
            തുടരുക;
        തിരികെ cbh;
    }
    തിരികെ ശൂന്യ;
}

// Try to find the coreboot memory table in the given coreboot table.
ശൂന്യം *
find_cb_subtable(ഘടന cb_header *cbh, u32 tag)
{
    അക്ഷരം *tbl = (അക്ഷരം *)cbh + വലുപ്പം(*cbh);
    u32 എണ്ണം = GET_FARVAR(0, cbh->table_entries);
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<എണ്ണം; i++) {
        ഘടന cb_memory *cbm = (ശൂന്യം*)tbl;
        tbl += GET_FARVAR(0, cbm->size);
        എങ്കിൽ (GET_FARVAR(0, cbm->tag) == tag)
            തിരികെ cbm;
    }
    തിരികെ ശൂന്യ;
}

ഘടന cb_header *
find_cb_table(ശൂന്യം)
{
    ഘടന cb_header *cbh = find_cb_header(0, 0x1000);
    എങ്കിൽ (!cbh)
        തിരികെ ശൂന്യ;
    ഘടന cb_forward *cbf = find_cb_subtable(cbh, CB_TAG_FORWARD);
    എങ്കിൽ (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = find_cb_header(GET_FARVAR(0, cbf->forward), 0x100);
        എങ്കിൽ (!cbh)
            തിരികെ ശൂന്യ;
    }
    തിരികെ cbh;
}

സ്ഥിതം ഘടന cb_memory *CBMemTable;
സ്ഥിരം അക്ഷരം *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
ശൂന്യം
coreboot_preinit(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_COREBOOT)
        തിരികെ;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    ഘടന cb_header *cbh = find_cb_table();
    എങ്കിൽ (!cbh)
        പോകുക fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    ഘടന cb_memory *cbm = CBMemTable = find_cb_subtable(cbh, CB_TAG_MEMORY);
    എങ്കിൽ (!cbm)
        പോകുക fail;

    പൂർണ്ണസംഖ്യ i, എണ്ണം = MEM_RANGE_എണ്ണം(cbm);
    വേണ്ടി (i=0; i<എണ്ണം; i++) {
        ഘടന cb_memory_range *m = &cbm->map[i];
        u32 type = m->type;
        എങ്കിൽ (type == CB_MEM_TABLE)
            type = E820_RESERVED;
        e820_add(m->start, m->size, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_add(0, 16*1024, E820_RAM);

    ഘടന cb_cbmem_ref *cbref = find_cb_subtable(cbh, CB_TAG_CBMEM_CONSOLE);
    എങ്കിൽ (cbref) {
        cbcon = (ശൂന്യം*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    ഘടന cb_mainboard *cbmb = find_cb_subtable(cbh, CB_TAG_MAINBOARD);
    എങ്കിൽ (cbmb) {
        CBvendor = &cbmb->strings[cbmb->vendor_idx];
        CBpart = &cbmb->strings[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    തിരികെ;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_add(0, 16*1024*1024, E820_RAM);
    തിരികെ;
}

ശൂന്യം coreboot_debug_putc(അക്ഷരം c)
{
    എങ്കിൽ (!CONFIG_DEBUG_COREBOOT)
        തിരികെ;
    എങ്കിൽ (!cbcon)
        തിരികെ;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_MASK;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_MASK;
    എങ്കിൽ (cursor >= cbcon->size)
        തിരികെ; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    എങ്കിൽ (cursor >= cbcon->size) {
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
സ്ഥിതം ശൂന്യം
scan_tables(u32 start, u32 size)
{
    ശൂന്യം *p = (ശൂന്യം*)ALIGN(start, 16);
    ശൂന്യം *end = (ശൂന്യം*)start + size;
    വേണ്ടി (; p<end; p += 16)
        copy_table(p);
}

ശൂന്യം
coreboot_platform_setup(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_COREBOOT)
        തിരികെ;
    pci_probe_devices();

    ഘടന cb_memory *cbm = CBMemTable;
    എങ്കിൽ (!cbm)
        തിരികെ;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    പൂർണ്ണസംഖ്യ i, എണ്ണം = MEM_RANGE_എണ്ണം(cbm);
    വേണ്ടി (i=0; i<എണ്ണം; i++) {
        ഘടന cb_memory_range *m = &cbm->map[i];
        എങ്കിൽ (m->type == CB_MEM_TABLE)
            scan_tables(m->start, m->size);
    }

    find_acpi_features();
}


/****************************************************************
 * ulzma
 ****************************************************************/

// Uncompress data in flash to an area of memory.
സ്ഥിതം പൂർണ്ണസംഖ്യ
ulzma(u8 *dst, u32 maxlen, സ്ഥിരം u8 *src, u32 srclen)
{
    dprintf(3, "Uncompressing data %d@%p to %d@%p\n", srclen, src, maxlen, dst);
    CLzmaDecoderState state;
    പൂർണ്ണസംഖ്യ ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_SIZE);
    എങ്കിൽ (ret != LZMA_RESULT_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        തിരികെ -1;
    }
    u8 scratch[15980];
    പൂർണ്ണസംഖ്യ need = (Lzma_Get_അക്കം_Probs(&state.Properties) * വലുപ്പം(CProb));
    എങ്കിൽ (need > വലുപ്പം(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (unsigned പൂർണ്ണസംഖ്യ)വലുപ്പം(scratch));
        തിരികെ -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_SIZE);
    എങ്കിൽ (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        തിരികെ -1;
    }
    u32 inProcessed, outProcessed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_SIZE + 8, srclen
                     , &inProcessed, dst, dstlen, &outProcessed);
    എങ്കിൽ (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        തിരികെ -1;
    }
    തിരികെ dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#define CBFS_HEADER_MAGIC 0x4F524243
#define CBFS_VERSION1 0x31313131

ഘടന cbfs_header {
    u32 magic;
    u32 version;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 offset;
    u32 pad[2];
} PACKED;

#define CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

ഘടന cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 offset;
    അക്ഷരം filename[0];
} PACKED;

ഘടന cbfs_romfile_s {
    ഘടന romfile_s file;
    ഘടന cbfs_file *fhdr;
    ശൂന്യം *data;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
സ്ഥിതം പൂർണ്ണസംഖ്യ
cbfs_copyfile(ഘടന romfile_s *file, ശൂന്യം *dst, u32 maxlen)
{
    എങ്കിൽ (!CONFIG_COREBOOT_FLASH)
        തിരികെ -1;

    ഘടന cbfs_romfile_s *cfile;
    cfile = container_of(file, ഘടന cbfs_romfile_s, file);
    u32 size = cfile->rawsize;
    ശൂന്യം *src = cfile->data;
    എങ്കിൽ (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        ശൂന്യം *temp = malloc_tmphigh(size);
        എങ്കിൽ (!temp) {
            warn_noalloc();
            തിരികെ -1;
        }
        iomemcpy(temp, src, size);
        പൂർണ്ണസംഖ്യ ret = ulzma(dst, maxlen, temp, size);
        yield();
        സ്വതന്ത്രം(temp);
        തിരികെ ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    എങ്കിൽ (size > maxlen) {
        warn_noalloc();
        തിരികെ -1;
    }
    iomemcpy(dst, src, size);
    തിരികെ size;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
സ്ഥിതം ശൂന്യം
process_links_file(ശൂന്യം)
{
    അക്ഷരം *links = romfile_loadfile("links", ശൂന്യ), *next = links;
    ആകുമ്പോൾ (next) {
        // Parse out linkname and destname
        അക്ഷരം *linkname = next;
        next = strchr(linkname, '\n');
        എങ്കിൽ (next)
            *next++ = '\0';
        അക്ഷരം *comment = strchr(linkname, '#');
        എങ്കിൽ (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        അക്ഷരം *destname = strchr(linkname, ' ');
        എങ്കിൽ (!destname)
            തുടരുക;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        ഘടന romfile_s *ufile = romfile_find(destname);
        എങ്കിൽ (!ufile)
            തുടരുക;
        ഘടന cbfs_romfile_s *cufile
            = container_of(ufile, ഘടന cbfs_romfile_s, file);
        ഘടന cbfs_romfile_s *cfile = malloc_tmp(വലുപ്പം(*cfile));
        എങ്കിൽ (!cfile) {
            warn_noalloc();
            നിർത്തുക;
        }
        memcpy(cfile, cufile, വലുപ്പം(*cfile));
        strtcpy(cfile->file.name, linkname, വലുപ്പം(cfile->file.name));
        romfile_add(&cfile->file);
    }
    സ്വതന്ത്രം(links);
}

ശൂന്യം
coreboot_cbfs_init(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_COREBOOT_FLASH)
        തിരികെ;

    ഘടന cbfs_header *hdr = *(ശൂന്യം **)(CONFIG_CBFS_LOCATION - 4);
    എങ്കിൽ ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        തിരികെ;
    }
    എങ്കിൽ (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (ശൂന്യം*)hdr + CONFIG_CBFS_LOCATION;
    എങ്കിൽ (hdr->magic != cpu_to_be32(CBFS_HEADER_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_to_be32(CBFS_HEADER_MAGIC));
        തിരികെ;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_to_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    ഘടന cbfs_file *fhdr = (ശൂന്യം*)romstart + be32_to_cpu(hdr->offset);
    വേണ്ടി (;;) {
        എങ്കിൽ ((u32)fhdr - romstart > romsize)
            നിർത്തുക;
        u64 magic = fhdr->magic;
        എങ്കിൽ (magic != CBFS_FILE_MAGIC)
            നിർത്തുക;
        ഘടന cbfs_romfile_s *cfile = malloc_tmp(വലുപ്പം(*cfile));
        എങ്കിൽ (!cfile) {
            warn_noalloc();
            നിർത്തുക;
        }
        memset(cfile, 0, വലുപ്പം(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, വലുപ്പം(cfile->file.name));
        cfile->file.size = cfile->rawsize = be32_to_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->data = (ശൂന്യം*)fhdr + be32_to_cpu(fhdr->offset);
        പൂർണ്ണസംഖ്യ len = strlen(cfile->file.name);
        എങ്കിൽ (len > 5 && strcmp(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.size = *(u32*)(cfile->data + LZMA_PROPERTIES_SIZE);
        }
        romfile_add(&cfile->file);

        fhdr = (ശൂന്യം*)ALIGN((u32)cfile->data + cfile->rawsize
                            , be32_to_cpu(hdr->align));
    }

    process_links_file();
}

ഘടന cbfs_payload_segment {
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

ഘടന cbfs_payload {
    ഘടന cbfs_payload_segment segments[1];
};

ശൂന്യം
cbfs_run_payload(ഘടന cbfs_file *fhdr)
{
    എങ്കിൽ (!CONFIG_COREBOOT_FLASH || !fhdr)
        തിരികെ;
    dprintf(1, "Run %s\n", fhdr->filename);
    ഘടന cbfs_payload *pay = (ശൂന്യം*)fhdr + be32_to_cpu(fhdr->offset);
    ഘടന cbfs_payload_segment *seg = pay->segments;
    വേണ്ടി (;;) {
        ശൂന്യം *src = (ശൂന്യം*)pay + be32_to_cpu(seg->offset);
        ശൂന്യം *dest = (ശൂന്യം*)(u32)be64_to_cpu(seg->load_addr);
        u32 src_len = be32_to_cpu(seg->len);
        u32 dest_len = be32_to_cpu(seg->mem_len);
        തിരഞ്ഞെടുപ്പ് (seg->type) {
        അവസ്ഥ PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            memset(dest, 0, dest_len);
            നിർത്തുക;
        അവസ്ഥ PAYLOAD_SEGMENT_ENTRY: {
            dprintf(1, "Calling addr %p\n", dest);
            ശൂന്യം (*func)(ശൂന്യം) = dest;
            func();
            തിരികെ;
        }
        സ്ഥിരസ്ഥിതി:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            എങ്കിൽ (seg->compression == cpu_to_be32(CBFS_COMPRESS_NONE)) {
                എങ്കിൽ (src_len > dest_len)
                    src_len = dest_len;
                memcpy(dest, src, src_len);
            } അല്ലെങ്കിൽ എങ്കിൽ (CONFIG_LZMA
                       && seg->compression == cpu_to_be32(CBFS_COMPRESS_LZMA)) {
                പൂർണ്ണസംഖ്യ ret = ulzma(dest, dest_len, src, src_len);
                എങ്കിൽ (ret < 0)
                    തിരികെ;
                src_len = ret;
            } അല്ലെങ്കിൽ {
                dprintf(1, "No support for compression type %x\n"
                        , seg->compression);
                തിരികെ;
            }
            എങ്കിൽ (dest_len > src_len)
                memset(dest + src_len, 0, dest_len - src_len);
            നിർത്തുക;
        }
        seg++;
    }
}

// Register payloads in "img/" directory with boot system.
ശൂന്യം
cbfs_payload_setup(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_COREBOOT_FLASH)
        തിരികെ;
    ഘടന romfile_s *file = ശൂന്യ;
    വേണ്ടി (;;) {
        file = romfile_findprefix("img/", file);
        എങ്കിൽ (!file)
            നിർത്തുക;
        ഘടന cbfs_romfile_s *cfile;
        cfile = container_of(file, ഘടന cbfs_romfile_s, file);
        സ്ഥിരം അക്ഷരം *filename = file->name;
        അക്ഷരം *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_add_cbfs(cfile->fhdr, desc, bootprio_find_named_rom(filename, 0));
    }
}

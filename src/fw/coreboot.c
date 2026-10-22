// Coreboot interface support.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "block.h" // MAXDESCSIZE
#अन्तर्भाव "byteorder.h" // be32_to_cpu
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "e820map.h" // e820_add
#अन्तर्भाव "hw/pcidevice.h" // pci_probe_devices
#अन्तर्भाव "lzmadecode.h" // LzmaDecode
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "paravirt.h" // PlatformRunningOn
#अन्तर्भाव "romfile.h" // romfile_findprefix
#अन्तर्भाव "stacks.h" // yield
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // coreboot_preinit


/****************************************************************
 * Memory map
 ****************************************************************/

संरचना cb_header {
    u32 signature;
    u32 header_bytes;
    u32 header_checksum;
    u32 table_bytes;
    u32 table_checksum;
    u32 table_entries;
};

#define CB_SIGNATURE 0x4f49424C // "LBIO"

संरचना cb_memory_range {
    u64 start;
    u64 size;
    u32 type;
};

#define CB_MEM_TABLE    16

संरचना cb_memory {
    u32 tag;
    u32 size;
    संरचना cb_memory_range map[0];
};

#define CB_TAG_MEMORY 0x01

#define MEM_RANGE_गिनती(_rec) \
        (((_rec)->size - परिमाण(*(_rec))) / परिमाण((_rec)->map[0]))

संरचना cb_mainboard {
    u32 tag;
    u32 size;
    u8  vendor_idx;
    u8  part_idx;
    वर्ण  strings[0];
};

#define CB_TAG_MAINBOARD 0x0003

संरचना cb_forward {
    u32 tag;
    u32 size;
    u64 forward;
};

#define CB_TAG_FORWARD 0x11

संरचना cb_cbmem_ref {
    u32 tag;
    u32 size;
    u64 cbmem_addr;
};

#define CB_TAG_CBMEM_CONSOLE 0x17

संरचना cbmem_console {
    u32 size;
    u32 cursor;
    u8  body[0];
} PACKED;
#define CBMC_CURSOR_MASK ((1 << 28) - 1)
#define CBMC_OVERFLOW (1 << 31)
स्थिर संरचना cbmem_console *cbcon = रिक्त;

स्थिर u16
ipchksum(वर्ण *buf, पूर्णाङ्क गिनती)
{
    u16 *p = (u16*)buf;
    u32 sum = 0;
    यावत् (गिनती > 1) {
        sum += GET_FARVAR(0, *p);
        p++;
        गिनती -= 2;
    }
    यदि (गिनती)
        sum += GET_FARVAR(0, *(u8*)p);
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    प्रत्यागम ~sum;
}

// Try to locate the coreboot header in a given address range.
स्थिर संरचना cb_header *
find_cb_header(u32 addr, पूर्णाङ्क len)
{
    u32 end = addr + len;
    प्रति (; addr < end; addr += 16) {
        संरचना cb_header *cbh = (शून्य*)addr;
        यदि (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            अनुवर्त;
        u32 tsize = GET_FARVAR(0, cbh->table_bytes);
        यदि (! tsize)
            अनुवर्त;
        यदि (ipchksum((शून्य*)addr, परिमाण(*cbh)) != 0)
            अनुवर्त;
        यदि (ipchksum((शून्य*)addr + परिमाण(*cbh), tsize)
            != GET_FARVAR(0, cbh->table_checksum))
            अनुवर्त;
        प्रत्यागम cbh;
    }
    प्रत्यागम रिक्त;
}

// Try to find the coreboot memory table in the given coreboot table.
शून्य *
find_cb_subtable(संरचना cb_header *cbh, u32 tag)
{
    वर्ण *tbl = (वर्ण *)cbh + परिमाण(*cbh);
    u32 गिनती = GET_FARVAR(0, cbh->table_entries);
    पूर्णाङ्क i;
    प्रति (i=0; i<गिनती; i++) {
        संरचना cb_memory *cbm = (शून्य*)tbl;
        tbl += GET_FARVAR(0, cbm->size);
        यदि (GET_FARVAR(0, cbm->tag) == tag)
            प्रत्यागम cbm;
    }
    प्रत्यागम रिक्त;
}

संरचना cb_header *
find_cb_table(शून्य)
{
    संरचना cb_header *cbh = find_cb_header(0, 0x1000);
    यदि (!cbh)
        प्रत्यागम रिक्त;
    संरचना cb_forward *cbf = find_cb_subtable(cbh, CB_TAG_FORWARD);
    यदि (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = find_cb_header(GET_FARVAR(0, cbf->forward), 0x100);
        यदि (!cbh)
            प्रत्यागम रिक्त;
    }
    प्रत्यागम cbh;
}

स्थिर संरचना cb_memory *CBMemTable;
ध्रुव वर्ण *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
शून्य
coreboot_preinit(शून्य)
{
    यदि (!CONFIG_COREBOOT)
        प्रत्यागम;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    संरचना cb_header *cbh = find_cb_table();
    यदि (!cbh)
        गच्छ fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    संरचना cb_memory *cbm = CBMemTable = find_cb_subtable(cbh, CB_TAG_MEMORY);
    यदि (!cbm)
        गच्छ fail;

    पूर्णाङ्क i, गिनती = MEM_RANGE_गिनती(cbm);
    प्रति (i=0; i<गिनती; i++) {
        संरचना cb_memory_range *m = &cbm->map[i];
        u32 type = m->type;
        यदि (type == CB_MEM_TABLE)
            type = E820_RESERVED;
        e820_add(m->start, m->size, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_add(0, 16*1024, E820_RAM);

    संरचना cb_cbmem_ref *cbref = find_cb_subtable(cbh, CB_TAG_CBMEM_CONSOLE);
    यदि (cbref) {
        cbcon = (शून्य*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    संरचना cb_mainboard *cbmb = find_cb_subtable(cbh, CB_TAG_MAINBOARD);
    यदि (cbmb) {
        CBvendor = &cbmb->strings[cbmb->vendor_idx];
        CBpart = &cbmb->strings[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    प्रत्यागम;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_add(0, 16*1024*1024, E820_RAM);
    प्रत्यागम;
}

शून्य coreboot_debug_putc(वर्ण c)
{
    यदि (!CONFIG_DEBUG_COREBOOT)
        प्रत्यागम;
    यदि (!cbcon)
        प्रत्यागम;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_MASK;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_MASK;
    यदि (cursor >= cbcon->size)
        प्रत्यागम; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    यदि (cursor >= cbcon->size) {
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
स्थिर शून्य
scan_tables(u32 start, u32 size)
{
    शून्य *p = (शून्य*)ALIGN(start, 16);
    शून्य *end = (शून्य*)start + size;
    प्रति (; p<end; p += 16)
        copy_table(p);
}

शून्य
coreboot_platform_setup(शून्य)
{
    यदि (!CONFIG_COREBOOT)
        प्रत्यागम;
    pci_probe_devices();

    संरचना cb_memory *cbm = CBMemTable;
    यदि (!cbm)
        प्रत्यागम;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    पूर्णाङ्क i, गिनती = MEM_RANGE_गिनती(cbm);
    प्रति (i=0; i<गिनती; i++) {
        संरचना cb_memory_range *m = &cbm->map[i];
        यदि (m->type == CB_MEM_TABLE)
            scan_tables(m->start, m->size);
    }

    find_acpi_features();
}


/****************************************************************
 * ulzma
 ****************************************************************/

// Uncompress data in flash to an area of memory.
स्थिर पूर्णाङ्क
ulzma(u8 *dst, u32 maxlen, ध्रुव u8 *src, u32 srclen)
{
    dprintf(3, "Uncompressing data %d@%p to %d@%p\n", srclen, src, maxlen, dst);
    CLzmaDecoderState state;
    पूर्णाङ्क ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_SIZE);
    यदि (ret != LZMA_RESULT_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        प्रत्यागम -1;
    }
    u8 scratch[15980];
    पूर्णाङ्क need = (Lzma_Get_अंक_Probs(&state.Properties) * परिमाण(CProb));
    यदि (need > परिमाण(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (अचिन्हित पूर्णाङ्क)परिमाण(scratch));
        प्रत्यागम -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_SIZE);
    यदि (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        प्रत्यागम -1;
    }
    u32 inProcessed, outProcessed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_SIZE + 8, srclen
                     , &inProcessed, dst, dstlen, &outProcessed);
    यदि (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        प्रत्यागम -1;
    }
    प्रत्यागम dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#define CBFS_HEADER_MAGIC 0x4F524243
#define CBFS_VERSION1 0x31313131

संरचना cbfs_header {
    u32 magic;
    u32 version;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 offset;
    u32 pad[2];
} PACKED;

#define CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

संरचना cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 offset;
    वर्ण filename[0];
} PACKED;

संरचना cbfs_romfile_s {
    संरचना romfile_s file;
    संरचना cbfs_file *fhdr;
    शून्य *data;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
स्थिर पूर्णाङ्क
cbfs_copyfile(संरचना romfile_s *file, शून्य *dst, u32 maxlen)
{
    यदि (!CONFIG_COREBOOT_FLASH)
        प्रत्यागम -1;

    संरचना cbfs_romfile_s *cfile;
    cfile = container_of(file, संरचना cbfs_romfile_s, file);
    u32 size = cfile->rawsize;
    शून्य *src = cfile->data;
    यदि (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        शून्य *temp = malloc_tmphigh(size);
        यदि (!temp) {
            warn_noalloc();
            प्रत्यागम -1;
        }
        iomemcpy(temp, src, size);
        पूर्णाङ्क ret = ulzma(dst, maxlen, temp, size);
        yield();
        मुक्त(temp);
        प्रत्यागम ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", size, src, maxlen, dst);
    यदि (size > maxlen) {
        warn_noalloc();
        प्रत्यागम -1;
    }
    iomemcpy(dst, src, size);
    प्रत्यागम size;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
स्थिर शून्य
process_links_file(शून्य)
{
    वर्ण *links = romfile_loadfile("links", रिक्त), *next = links;
    यावत् (next) {
        // Parse out linkname and destname
        वर्ण *linkname = next;
        next = म_अक्षर(linkname, '\n');
        यदि (next)
            *next++ = '\0';
        वर्ण *comment = म_अक्षर(linkname, '#');
        यदि (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        वर्ण *destname = म_अक्षर(linkname, ' ');
        यदि (!destname)
            अनुवर्त;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        संरचना romfile_s *ufile = romfile_find(destname);
        यदि (!ufile)
            अनुवर्त;
        संरचना cbfs_romfile_s *cufile
            = container_of(ufile, संरचना cbfs_romfile_s, file);
        संरचना cbfs_romfile_s *cfile = malloc_tmp(परिमाण(*cfile));
        यदि (!cfile) {
            warn_noalloc();
            भङ्ग;
        }
        स_नकल(cfile, cufile, परिमाण(*cfile));
        strtcpy(cfile->file.name, linkname, परिमाण(cfile->file.name));
        romfile_add(&cfile->file);
    }
    मुक्त(links);
}

शून्य
coreboot_cbfs_init(शून्य)
{
    यदि (!CONFIG_COREBOOT_FLASH)
        प्रत्यागम;

    संरचना cbfs_header *hdr = *(शून्य **)(CONFIG_CBFS_LOCATION - 4);
    यदि ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        प्रत्यागम;
    }
    यदि (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (शून्य*)hdr + CONFIG_CBFS_LOCATION;
    यदि (hdr->magic != cpu_to_be32(CBFS_HEADER_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_to_be32(CBFS_HEADER_MAGIC));
        प्रत्यागम;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_to_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    संरचना cbfs_file *fhdr = (शून्य*)romstart + be32_to_cpu(hdr->offset);
    प्रति (;;) {
        यदि ((u32)fhdr - romstart > romsize)
            भङ्ग;
        u64 magic = fhdr->magic;
        यदि (magic != CBFS_FILE_MAGIC)
            भङ्ग;
        संरचना cbfs_romfile_s *cfile = malloc_tmp(परिमाण(*cfile));
        यदि (!cfile) {
            warn_noalloc();
            भङ्ग;
        }
        स_रखो(cfile, 0, परिमाण(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, परिमाण(cfile->file.name));
        cfile->file.size = cfile->rawsize = be32_to_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->data = (शून्य*)fhdr + be32_to_cpu(fhdr->offset);
        पूर्णाङ्क len = म_माप(cfile->file.name);
        यदि (len > 5 && म_भेद(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.size = *(u32*)(cfile->data + LZMA_PROPERTIES_SIZE);
        }
        romfile_add(&cfile->file);

        fhdr = (शून्य*)ALIGN((u32)cfile->data + cfile->rawsize
                            , be32_to_cpu(hdr->align));
    }

    process_links_file();
}

संरचना cbfs_payload_segment {
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

संरचना cbfs_payload {
    संरचना cbfs_payload_segment segments[1];
};

शून्य
cbfs_run_payload(संरचना cbfs_file *fhdr)
{
    यदि (!CONFIG_COREBOOT_FLASH || !fhdr)
        प्रत्यागम;
    dprintf(1, "Run %s\n", fhdr->filename);
    संरचना cbfs_payload *pay = (शून्य*)fhdr + be32_to_cpu(fhdr->offset);
    संरचना cbfs_payload_segment *seg = pay->segments;
    प्रति (;;) {
        शून्य *src = (शून्य*)pay + be32_to_cpu(seg->offset);
        शून्य *dest = (शून्य*)(u32)be64_to_cpu(seg->load_addr);
        u32 src_len = be32_to_cpu(seg->len);
        u32 dest_len = be32_to_cpu(seg->mem_len);
        विकल्प (seg->type) {
        पक्ष PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            स_रखो(dest, 0, dest_len);
            भङ्ग;
        पक्ष PAYLOAD_SEGMENT_ENTRY: {
            dprintf(1, "Calling addr %p\n", dest);
            शून्य (*func)(शून्य) = dest;
            func();
            प्रत्यागम;
        }
        स्वभाव:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            यदि (seg->compression == cpu_to_be32(CBFS_COMPRESS_NONE)) {
                यदि (src_len > dest_len)
                    src_len = dest_len;
                स_नकल(dest, src, src_len);
            } अन्यथा यदि (CONFIG_LZMA
                       && seg->compression == cpu_to_be32(CBFS_COMPRESS_LZMA)) {
                पूर्णाङ्क ret = ulzma(dest, dest_len, src, src_len);
                यदि (ret < 0)
                    प्रत्यागम;
                src_len = ret;
            } अन्यथा {
                dprintf(1, "No support for compression type %x\n"
                        , seg->compression);
                प्रत्यागम;
            }
            यदि (dest_len > src_len)
                स_रखो(dest + src_len, 0, dest_len - src_len);
            भङ्ग;
        }
        seg++;
    }
}

// Register payloads in "img/" directory with boot system.
शून्य
cbfs_payload_setup(शून्य)
{
    यदि (!CONFIG_COREBOOT_FLASH)
        प्रत्यागम;
    संरचना romfile_s *file = रिक्त;
    प्रति (;;) {
        file = romfile_findprefix("img/", file);
        यदि (!file)
            भङ्ग;
        संरचना cbfs_romfile_s *cfile;
        cfile = container_of(file, संरचना cbfs_romfile_s, file);
        ध्रुव वर्ण *filename = file->name;
        वर्ण *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_add_cbfs(cfile->fhdr, desc, bootprio_find_named_rom(filename, 0));
    }
}

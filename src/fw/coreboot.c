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

संरचना cb_शीर्षकम् {
    u32 signature;
    u32 शीर्षकम्_अष्टकानि;
    u32 शीर्षकम्_checksum;
    u32 सारणी_अष्टकानि;
    u32 सारणी_checksum;
    u32 सारणी_entries;
};

#परिभाषय CB_SIGNATURE 0x4f49424C // "LBIO"

संरचना cb_स्मृतिः_range {
    u64 start;
    u64 आकार;
    u32 type;
};

#परिभाषय CB_MEM_सारणी    16

संरचना cb_स्मृतिः {
    u32 चिह्नकम्;
    u32 आकार;
    संरचना cb_स्मृतिः_range map[0];
};

#परिभाषय CB_चिह्नकम्_स्मृतिः 0x01

#परिभाषय MEM_RANGE_गणनम्(_rec) \
        (((_rec)->आकार - परिमाण(*(_rec))) / परिमाण((_rec)->map[0]))

संरचना cb_mainboard {
    u32 चिह्नकम्;
    u32 आकार;
    u8  vendor_idx;
    u8  part_idx;
    वर्ण  वाक्यानि[0];
};

#परिभाषय CB_चिह्नकम्_MAINBOARD 0x0003

संरचना cb_forward {
    u32 चिह्नकम्;
    u32 आकार;
    u64 forward;
};

#परिभाषय CB_चिह्नकम्_FORWARD 0x11

संरचना cb_cbmem_ref {
    u32 चिह्नकम्;
    u32 आकार;
    u64 cbmem_addr;
};

#परिभाषय CB_चिह्नकम्_CBMEM_CONSOLE 0x17

संरचना cbmem_console {
    u32 आकार;
    u32 cursor;
    u8  body[0];
} PACKED;
#परिभाषय CBMC_CURSOR_आवरणम् ((1 << 28) - 1)
#परिभाषय CBMC_अतिप्रवाहः (1 << 31)
स्थिर संरचना cbmem_console *cbcon = रिक्त;

स्थिर u16
ipchksum(वर्ण *buf, पूर्णाङ्क गणनम्)
{
    u16 *p = (u16*)buf;
    u32 योग = 0;
    यावत् (गणनम् > 1) {
        योग += GET_FARVAR(0, *p);
        p++;
        गणनम् -= 2;
    }
    यदि (गणनम्)
        योग += GET_FARVAR(0, *(u8*)p);
    योग = (योग >> 16) + (योग & 0xffff);
    योग += (योग >> 16);
    प्रत्यागम ~योग;
}

// Try to locate the coreboot header in a given address range.
स्थिर संरचना cb_शीर्षकम् *
find_cb_शीर्षकम्(u32 addr, पूर्णाङ्क len)
{
    u32 अन्तः = addr + len;
    प्रति (; addr < अन्तः; addr += 16) {
        संरचना cb_शीर्षकम् *cbh = (शून्य*)addr;
        यदि (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            अनुवर्त;
        u32 tsize = GET_FARVAR(0, cbh->सारणी_अष्टकानि);
        यदि (! tsize)
            अनुवर्त;
        यदि (ipchksum((शून्य*)addr, परिमाण(*cbh)) != 0)
            अनुवर्त;
        यदि (ipchksum((शून्य*)addr + परिमाण(*cbh), tsize)
            != GET_FARVAR(0, cbh->सारणी_checksum))
            अनुवर्त;
        प्रत्यागम cbh;
    }
    प्रत्यागम रिक्त;
}

// Try to find the coreboot memory table in the given coreboot table.
शून्य *
find_cb_subtable(संरचना cb_शीर्षकम् *cbh, u32 चिह्नकम्)
{
    वर्ण *tbl = (वर्ण *)cbh + परिमाण(*cbh);
    u32 गणनम् = GET_FARVAR(0, cbh->सारणी_entries);
    पूर्णाङ्क i;
    प्रति (i=0; i<गणनम्; i++) {
        संरचना cb_स्मृतिः *cbm = (शून्य*)tbl;
        tbl += GET_FARVAR(0, cbm->आकार);
        यदि (GET_FARVAR(0, cbm->चिह्नकम्) == चिह्नकम्)
            प्रत्यागम cbm;
    }
    प्रत्यागम रिक्त;
}

संरचना cb_शीर्षकम् *
find_cb_सारणी(शून्य)
{
    संरचना cb_शीर्षकम् *cbh = find_cb_शीर्षकम्(0, 0x1000);
    यदि (!cbh)
        प्रत्यागम रिक्त;
    संरचना cb_forward *cbf = find_cb_subtable(cbh, CB_चिह्नकम्_FORWARD);
    यदि (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = find_cb_शीर्षकम्(GET_FARVAR(0, cbf->forward), 0x100);
        यदि (!cbh)
            प्रत्यागम रिक्त;
    }
    प्रत्यागम cbh;
}

स्थिर संरचना cb_स्मृतिः *CBMem_सारणी;
ध्रुव वर्ण *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
शून्य
coreboot_preinit(शून्य)
{
    यदि (!CONFIG_COREBOOT)
        प्रत्यागम;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    संरचना cb_शीर्षकम् *cbh = find_cb_सारणी();
    यदि (!cbh)
        गच्छ fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    संरचना cb_स्मृतिः *cbm = CBMem_सारणी = find_cb_subtable(cbh, CB_चिह्नकम्_स्मृतिः);
    यदि (!cbm)
        गच्छ fail;

    पूर्णाङ्क i, गणनम् = MEM_RANGE_गणनम्(cbm);
    प्रति (i=0; i<गणनम्; i++) {
        संरचना cb_स्मृतिः_range *m = &cbm->map[i];
        u32 type = m->type;
        यदि (type == CB_MEM_सारणी)
            type = E820_RESERVED;
        e820_सङ्कलय(m->start, m->आकार, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_सङ्कलय(0, 16*1024, E820_RAM);

    संरचना cb_cbmem_ref *cbref = find_cb_subtable(cbh, CB_चिह्नकम्_CBMEM_CONSOLE);
    यदि (cbref) {
        cbcon = (शून्य*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    संरचना cb_mainboard *cbmb = find_cb_subtable(cbh, CB_चिह्नकम्_MAINBOARD);
    यदि (cbmb) {
        CBvendor = &cbmb->वाक्यानि[cbmb->vendor_idx];
        CBpart = &cbmb->वाक्यानि[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    प्रत्यागम;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_सङ्कलय(0, 16*1024*1024, E820_RAM);
    प्रत्यागम;
}

शून्य coreboot_debug_putc(वर्ण c)
{
    यदि (!CONFIG_DEBUG_COREBOOT)
        प्रत्यागम;
    यदि (!cbcon)
        प्रत्यागम;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_आवरणम्;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_आवरणम्;
    यदि (cursor >= cbcon->आकार)
        प्रत्यागम; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    यदि (cursor >= cbcon->आकार) {
        cursor = 0;
        flags |= CBMC_अतिप्रवाहः;
    }
    cbcon->cursor = flags | cursor;
}

/****************************************************************
 * BIOS table copying
 ****************************************************************/

// Attempt to find (and relocate) any standard bios tables found in a
// given address range.
स्थिर शून्य
अवलोकनम्_tables(u32 start, u32 आकार)
{
    शून्य *p = (शून्य*)ALIGN(start, 16);
    शून्य *अन्तः = (शून्य*)start + आकार;
    प्रति (; p<अन्तः; p += 16)
        copy_सारणी(p);
}

शून्य
coreboot_platform_सज्जता(शून्य)
{
    यदि (!CONFIG_COREBOOT)
        प्रत्यागम;
    pci_probe_devices();

    संरचना cb_स्मृतिः *cbm = CBMem_सारणी;
    यदि (!cbm)
        प्रत्यागम;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    पूर्णाङ्क i, गणनम् = MEM_RANGE_गणनम्(cbm);
    प्रति (i=0; i<गणनम्; i++) {
        संरचना cb_स्मृतिः_range *m = &cbm->map[i];
        यदि (m->type == CB_MEM_सारणी)
            अवलोकनम्_tables(m->start, m->आकार);
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
    पूर्णाङ्क ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_आकार);
    यदि (ret != LZMA_परिणाम_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        प्रत्यागम -1;
    }
    u8 scratch[15980];
    पूर्णाङ्क need = (Lzma_Get_अङ्कः_Probs(&state.Properties) * परिमाण(CProb));
    यदि (need > परिमाण(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (अचिह्नित पूर्णाङ्क)परिमाण(scratch));
        प्रत्यागम -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_आकार);
    यदि (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        प्रत्यागम -1;
    }
    u32 अन्तर्_Processed, बहिर्गमनम्_Processed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_आकार + 8, srclen
                     , &अन्तर्_Processed, dst, dstlen, &बहिर्गमनम्_Processed);
    यदि (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        प्रत्यागम -1;
    }
    प्रत्यागम dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#परिभाषय CBFS_शीर्षकम्_MAGIC 0x4F524243
#परिभाषय CBFS_VERSION1 0x31313131

संरचना cbfs_शीर्षकम् {
    u32 magic;
    u32 संस्करणम्_2;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 विस्थापनम्_2;
    u32 pad[2];
} PACKED;

#परिभाषय CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

संरचना cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 विस्थापनम्_2;
    वर्ण filename[0];
} PACKED;

संरचना cbfs_romfile_s {
    संरचना romfile_s file;
    संरचना cbfs_file *fhdr;
    शून्य *दत्त;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
स्थिर पूर्णाङ्क
cbfs_copyfile(संरचना romfile_s *file, शून्य *dst, u32 maxlen)
{
    यदि (!CONFIG_COREBOOT_FLASH)
        प्रत्यागम -1;

    संरचना cbfs_romfile_s *cfile;
    cfile = container_सम्बन्धि(file, संरचना cbfs_romfile_s, file);
    u32 आकार = cfile->rawsize;
    शून्य *src = cfile->दत्त;
    यदि (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        शून्य *अस्थायी = malloc_tmphigh(आकार);
        यदि (!अस्थायी) {
            warn_noalloc();
            प्रत्यागम -1;
        }
        iomemcpy(अस्थायी, src, आकार);
        पूर्णाङ्क ret = ulzma(dst, maxlen, अस्थायी, आकार);
        yield();
        मुक्त(अस्थायी);
        प्रत्यागम ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", आकार, src, maxlen, dst);
    यदि (आकार > maxlen) {
        warn_noalloc();
        प्रत्यागम -1;
    }
    iomemcpy(dst, src, आकार);
    प्रत्यागम आकार;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
स्थिर शून्य
process_links_file(शून्य)
{
    वर्ण *links = romfile_loadfile("links", रिक्त), *अग्र = links;
    यावत् (अग्र) {
        // Parse out linkname and destname
        वर्ण *linkname = अग्र;
        अग्र = strchr(linkname, '\n');
        यदि (अग्र)
            *अग्र++ = '\0';
        वर्ण *comment = strchr(linkname, '#');
        यदि (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        वर्ण *destname = strchr(linkname, ' ');
        यदि (!destname)
            अनुवर्त;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        संरचना romfile_s *ufile = romfile_find(destname);
        यदि (!ufile)
            अनुवर्त;
        संरचना cbfs_romfile_s *cufile
            = container_सम्बन्धि(ufile, संरचना cbfs_romfile_s, file);
        संरचना cbfs_romfile_s *cfile = malloc_tmp(परिमाण(*cfile));
        यदि (!cfile) {
            warn_noalloc();
            भङ्ग;
        }
        स्मृतिं_प्रतिलिख(cfile, cufile, परिमाण(*cfile));
        strtcpy(cfile->file.name, linkname, परिमाण(cfile->file.name));
        romfile_सङ्कलय(&cfile->file);
    }
    मुक्त(links);
}

शून्य
coreboot_cbfs_init(शून्य)
{
    यदि (!CONFIG_COREBOOT_FLASH)
        प्रत्यागम;

    संरचना cbfs_शीर्षकम् *hdr = *(शून्य **)(CONFIG_CBFS_LOCATION - 4);
    यदि ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        प्रत्यागम;
    }
    यदि (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (शून्य*)hdr + CONFIG_CBFS_LOCATION;
    यदि (hdr->magic != cpu_गन्तव्यम्_be32(CBFS_शीर्षकम्_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_गन्तव्यम्_be32(CBFS_शीर्षकम्_MAGIC));
        प्रत्यागम;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_गन्तव्यम्_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    संरचना cbfs_file *fhdr = (शून्य*)romstart + be32_गन्तव्यम्_cpu(hdr->विस्थापनम्_2);
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
        स्मृतिं_पूरय(cfile, 0, परिमाण(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, परिमाण(cfile->file.name));
        cfile->file.आकार = cfile->rawsize = be32_गन्तव्यम्_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->दत्त = (शून्य*)fhdr + be32_गन्तव्यम्_cpu(fhdr->विस्थापनम्_2);
        पूर्णाङ्क len = वाक्यदैर्घ्यम्(cfile->file.name);
        यदि (len > 5 && वाक्यं_तुलय(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.आकार = *(u32*)(cfile->दत्त + LZMA_PROPERTIES_आकार);
        }
        romfile_सङ्कलय(&cfile->file);

        fhdr = (शून्य*)ALIGN((u32)cfile->दत्त + cfile->rawsize
                            , be32_गन्तव्यम्_cpu(hdr->align));
    }

    process_links_file();
}

संरचना cbfs_payload_segment {
    u32 type;
    u32 compression;
    u32 विस्थापनम्_2;
    u64 आरोपय_addr;
    u32 len;
    u32 mem_len;
} PACKED;

#परिभाषय PAYLOAD_SEGMENT_BSS    0x20535342
#परिभाषय PAYLOAD_SEGMENT_प्रविष्टिः  0x52544E45

#परिभाषय CBFS_COMPRESS_NONE  0
#परिभाषय CBFS_COMPRESS_LZMA  1

संरचना cbfs_payload {
    संरचना cbfs_payload_segment segments[1];
};

शून्य
cbfs_सततम्_payload(संरचना cbfs_file *fhdr)
{
    यदि (!CONFIG_COREBOOT_FLASH || !fhdr)
        प्रत्यागम;
    dprintf(1, "Run %s\n", fhdr->filename);
    संरचना cbfs_payload *pay = (शून्य*)fhdr + be32_गन्तव्यम्_cpu(fhdr->विस्थापनम्_2);
    संरचना cbfs_payload_segment *seg = pay->segments;
    प्रति (;;) {
        शून्य *src = (शून्य*)pay + be32_गन्तव्यम्_cpu(seg->विस्थापनम्_2);
        शून्य *dest = (शून्य*)(u32)be64_गन्तव्यम्_cpu(seg->आरोपय_addr);
        u32 src_len = be32_गन्तव्यम्_cpu(seg->len);
        u32 dest_len = be32_गन्तव्यम्_cpu(seg->mem_len);
        विकल्प (seg->type) {
        पक्ष PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            स्मृतिं_पूरय(dest, 0, dest_len);
            भङ्ग;
        पक्ष PAYLOAD_SEGMENT_प्रविष्टिः: {
            dprintf(1, "Calling addr %p\n", dest);
            शून्य (*func)(शून्य) = dest;
            func();
            प्रत्यागम;
        }
        स्वभाव:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            यदि (seg->compression == cpu_गन्तव्यम्_be32(CBFS_COMPRESS_NONE)) {
                यदि (src_len > dest_len)
                    src_len = dest_len;
                स्मृतिं_प्रतिलिख(dest, src, src_len);
            } अन्यथा यदि (CONFIG_LZMA
                       && seg->compression == cpu_गन्तव्यम्_be32(CBFS_COMPRESS_LZMA)) {
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
                स्मृतिं_पूरय(dest + src_len, 0, dest_len - src_len);
            भङ्ग;
        }
        seg++;
    }
}

// Register payloads in "img/" directory with boot system.
शून्य
cbfs_payload_सज्जता(शून्य)
{
    यदि (!CONFIG_COREBOOT_FLASH)
        प्रत्यागम;
    संरचना romfile_s *file = रिक्त;
    प्रति (;;) {
        file = romfile_findprefix("img/", file);
        यदि (!file)
            भङ्ग;
        संरचना cbfs_romfile_s *cfile;
        cfile = container_सम्बन्धि(file, संरचना cbfs_romfile_s, file);
        ध्रुव वर्ण *filename = file->name;
        वर्ण *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_सङ्कलय_cbfs(cfile->fhdr, desc, bootprio_find_named_rom(filename, 0));
    }
}

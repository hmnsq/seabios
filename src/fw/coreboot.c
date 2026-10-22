// Coreboot interface support.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "block.h" // MAXDESCSIZE
#అన్తర్భావ "byteorder.h" // be32_to_cpu
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "e820map.h" // e820_add
#అన్తర్భావ "hw/pcidevice.h" // pci_probe_devices
#అన్తర్భావ "lzmadecode.h" // LzmaDecode
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "paravirt.h" // PlatformRunningOn
#అన్తర్భావ "romfile.h" // romfile_findprefix
#అన్తర్భావ "stacks.h" // yield
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // coreboot_preinit


/****************************************************************
 * Memory map
 ****************************************************************/

నిర్మాణం cb_శీర్షిక {
    u32 signature;
    u32 శీర్షిక_బైట్లు;
    u32 శీర్షిక_checksum;
    u32 పట్టిక_బైట్లు;
    u32 పట్టిక_checksum;
    u32 పట్టిక_entries;
};

#నిర్వచించు CB_SIGNATURE 0x4f49424C // "LBIO"

నిర్మాణం cb_జ్ఞాపకం_range {
    u64 ప్రారంభం;
    u64 కొలత;
    u32 type;
};

#నిర్వచించు CB_MEM_పట్టిక    16

నిర్మాణం cb_జ్ఞాపకం {
    u32 ట్యాగ్;
    u32 కొలత;
    నిర్మాణం cb_జ్ఞాపకం_range map[0];
};

#నిర్వచించు CB_ట్యాగ్_జ్ఞాపకం 0x01

#నిర్వచించు MEM_RANGE_లెక్క(_rec) \
        (((_rec)->కొలత - పరిమాణం(*(_rec))) / పరిమాణం((_rec)->map[0]))

నిర్మాణం cb_mainboard {
    u32 ట్యాగ్;
    u32 కొలత;
    u8  vendor_idx;
    u8  part_idx;
    అక్షరం  అక్షరశ్రేణులు[0];
};

#నిర్వచించు CB_ట్యాగ్_MAINBOARD 0x0003

నిర్మాణం cb_forward {
    u32 ట్యాగ్;
    u32 కొలత;
    u64 forward;
};

#నిర్వచించు CB_ట్యాగ్_FORWARD 0x11

నిర్మాణం cb_cbmem_ref {
    u32 ట్యాగ్;
    u32 కొలత;
    u64 cbmem_addr;
};

#నిర్వచించు CB_ట్యాగ్_CBMEM_CONSOLE 0x17

నిర్మాణం cbmem_console {
    u32 కొలత;
    u32 cursor;
    u8  body[0];
} PACKED;
#నిర్వచించు CBMC_CURSOR_ముసుగు ((1 << 28) - 1)
#నిర్వచించు CBMC_అధికప్రవాహం (1 << 31)
స్థిర నిర్మాణం cbmem_console *cbcon = ఖాళీ;

స్థిర u16
ipchksum(అక్షరం *buf, పూర్ణాంకం లెక్క)
{
    u16 *p = (u16*)buf;
    u32 కూడిక = 0;
    అయితే (లెక్క > 1) {
        కూడిక += GET_FARVAR(0, *p);
        p++;
        లెక్క -= 2;
    }
    ఒకవేళ (లెక్క)
        కూడిక += GET_FARVAR(0, *(u8*)p);
    కూడిక = (కూడిక >> 16) + (కూడిక & 0xffff);
    కూడిక += (కూడిక >> 16);
    తిరిగి ~కూడిక;
}

// Try to locate the coreboot header in a given address range.
స్థిర నిర్మాణం cb_శీర్షిక *
కనుగొను_cb_శీర్షిక(u32 addr, పూర్ణాంకం len)
{
    u32 చివర = addr + len;
    కొరకు (; addr < చివర; addr += 16) {
        నిర్మాణం cb_శీర్షిక *cbh = (శూన్య*)addr;
        ఒకవేళ (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            కొనసాగు;
        u32 tsize = GET_FARVAR(0, cbh->పట్టిక_బైట్లు);
        ఒకవేళ (! tsize)
            కొనసాగు;
        ఒకవేళ (ipchksum((శూన్య*)addr, పరిమాణం(*cbh)) != 0)
            కొనసాగు;
        ఒకవేళ (ipchksum((శూన్య*)addr + పరిమాణం(*cbh), tsize)
            != GET_FARVAR(0, cbh->పట్టిక_checksum))
            కొనసాగు;
        తిరిగి cbh;
    }
    తిరిగి ఖాళీ;
}

// Try to find the coreboot memory table in the given coreboot table.
శూన్య *
కనుగొను_cb_subtable(నిర్మాణం cb_శీర్షిక *cbh, u32 ట్యాగ్)
{
    అక్షరం *tbl = (అక్షరం *)cbh + పరిమాణం(*cbh);
    u32 లెక్క = GET_FARVAR(0, cbh->పట్టిక_entries);
    పూర్ణాంకం i;
    కొరకు (i=0; i<లెక్క; i++) {
        నిర్మాణం cb_జ్ఞాపకం *cbm = (శూన్య*)tbl;
        tbl += GET_FARVAR(0, cbm->కొలత);
        ఒకవేళ (GET_FARVAR(0, cbm->ట్యాగ్) == ట్యాగ్)
            తిరిగి cbm;
    }
    తిరిగి ఖాళీ;
}

నిర్మాణం cb_శీర్షిక *
కనుగొను_cb_పట్టిక(శూన్య)
{
    నిర్మాణం cb_శీర్షిక *cbh = కనుగొను_cb_శీర్షిక(0, 0x1000);
    ఒకవేళ (!cbh)
        తిరిగి ఖాళీ;
    నిర్మాణం cb_forward *cbf = కనుగొను_cb_subtable(cbh, CB_ట్యాగ్_FORWARD);
    ఒకవేళ (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = కనుగొను_cb_శీర్షిక(GET_FARVAR(0, cbf->forward), 0x100);
        ఒకవేళ (!cbh)
            తిరిగి ఖాళీ;
    }
    తిరిగి cbh;
}

స్థిర నిర్మాణం cb_జ్ఞాపకం *CBMem_పట్టిక;
స్థిరరాశి అక్షరం *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
శూన్య
coreboot_preinit(శూన్య)
{
    ఒకవేళ (!CONFIG_COREBOOT)
        తిరిగి;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    నిర్మాణం cb_శీర్షిక *cbh = కనుగొను_cb_పట్టిక();
    ఒకవేళ (!cbh)
        వెళ్ళు fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    నిర్మాణం cb_జ్ఞాపకం *cbm = CBMem_పట్టిక = కనుగొను_cb_subtable(cbh, CB_ట్యాగ్_జ్ఞాపకం);
    ఒకవేళ (!cbm)
        వెళ్ళు fail;

    పూర్ణాంకం i, లెక్క = MEM_RANGE_లెక్క(cbm);
    కొరకు (i=0; i<లెక్క; i++) {
        నిర్మాణం cb_జ్ఞాపకం_range *m = &cbm->map[i];
        u32 type = m->type;
        ఒకవేళ (type == CB_MEM_పట్టిక)
            type = E820_RESERVED;
        e820_కూడు(m->ప్రారంభం, m->కొలత, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_కూడు(0, 16*1024, E820_RAM);

    నిర్మాణం cb_cbmem_ref *cbref = కనుగొను_cb_subtable(cbh, CB_ట్యాగ్_CBMEM_CONSOLE);
    ఒకవేళ (cbref) {
        cbcon = (శూన్య*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    నిర్మాణం cb_mainboard *cbmb = కనుగొను_cb_subtable(cbh, CB_ట్యాగ్_MAINBOARD);
    ఒకవేళ (cbmb) {
        CBvendor = &cbmb->అక్షరశ్రేణులు[cbmb->vendor_idx];
        CBpart = &cbmb->అక్షరశ్రేణులు[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    తిరిగి;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_కూడు(0, 16*1024*1024, E820_RAM);
    తిరిగి;
}

శూన్య coreboot_debug_putc(అక్షరం c)
{
    ఒకవేళ (!CONFIG_DEBUG_COREBOOT)
        తిరిగి;
    ఒకవేళ (!cbcon)
        తిరిగి;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_ముసుగు;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_ముసుగు;
    ఒకవేళ (cursor >= cbcon->కొలత)
        తిరిగి; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    ఒకవేళ (cursor >= cbcon->కొలత) {
        cursor = 0;
        flags |= CBMC_అధికప్రవాహం;
    }
    cbcon->cursor = flags | cursor;
}

/****************************************************************
 * BIOS table copying
 ****************************************************************/

// Attempt to find (and relocate) any standard bios tables found in a
// given address range.
స్థిర శూన్య
పరిశీలన_tables(u32 ప్రారంభం, u32 కొలత)
{
    శూన్య *p = (శూన్య*)ALIGN(ప్రారంభం, 16);
    శూన్య *చివర = (శూన్య*)ప్రారంభం + కొలత;
    కొరకు (; p<చివర; p += 16)
        copy_పట్టిక(p);
}

శూన్య
coreboot_platform_అమరిక(శూన్య)
{
    ఒకవేళ (!CONFIG_COREBOOT)
        తిరిగి;
    pci_probe_devices();

    నిర్మాణం cb_జ్ఞాపకం *cbm = CBMem_పట్టిక;
    ఒకవేళ (!cbm)
        తిరిగి;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    పూర్ణాంకం i, లెక్క = MEM_RANGE_లెక్క(cbm);
    కొరకు (i=0; i<లెక్క; i++) {
        నిర్మాణం cb_జ్ఞాపకం_range *m = &cbm->map[i];
        ఒకవేళ (m->type == CB_MEM_పట్టిక)
            పరిశీలన_tables(m->ప్రారంభం, m->కొలత);
    }

    కనుగొను_acpi_features();
}


/****************************************************************
 * ulzma
 ****************************************************************/

// Uncompress data in flash to an area of memory.
స్థిర పూర్ణాంకం
ulzma(u8 *dst, u32 maxlen, స్థిరరాశి u8 *src, u32 srclen)
{
    dprintf(3, "Uncompressing data %d@%p to %d@%p\n", srclen, src, maxlen, dst);
    CLzmaDecoderState state;
    పూర్ణాంకం ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_కొలత);
    ఒకవేళ (ret != LZMA_ఫలితం_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        తిరిగి -1;
    }
    u8 scratch[15980];
    పూర్ణాంకం need = (Lzma_Get_అంకె_Probs(&state.Properties) * పరిమాణం(CProb));
    ఒకవేళ (need > పరిమాణం(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (గుర్తులేని పూర్ణాంకం)పరిమాణం(scratch));
        తిరిగి -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_కొలత);
    ఒకవేళ (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        తిరిగి -1;
    }
    u32 లోపల_Processed, బహిర్గతం_Processed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_కొలత + 8, srclen
                     , &లోపల_Processed, dst, dstlen, &బహిర్గతం_Processed);
    ఒకవేళ (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        తిరిగి -1;
    }
    తిరిగి dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#నిర్వచించు CBFS_శీర్షిక_MAGIC 0x4F524243
#నిర్వచించు CBFS_VERSION1 0x31313131

నిర్మాణం cbfs_శీర్షిక {
    u32 magic;
    u32 సంస్కరణ_2;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 స్థానభ్రంశం_2;
    u32 pad[2];
} PACKED;

#నిర్వచించు CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

నిర్మాణం cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 స్థానభ్రంశం_2;
    అక్షరం filename[0];
} PACKED;

నిర్మాణం cbfs_romfile_s {
    నిర్మాణం romfile_s file;
    నిర్మాణం cbfs_file *fhdr;
    శూన్య *దత్తాంశం;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
స్థిర పూర్ణాంకం
cbfs_copyfile(నిర్మాణం romfile_s *file, శూన్య *dst, u32 maxlen)
{
    ఒకవేళ (!CONFIG_COREBOOT_FLASH)
        తిరిగి -1;

    నిర్మాణం cbfs_romfile_s *cfile;
    cfile = container_యొక్క(file, నిర్మాణం cbfs_romfile_s, file);
    u32 కొలత = cfile->rawsize;
    శూన్య *src = cfile->దత్తాంశం;
    ఒకవేళ (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        శూన్య *తాత్కాలిక = malloc_tmphigh(కొలత);
        ఒకవేళ (!తాత్కాలిక) {
            warn_noalloc();
            తిరిగి -1;
        }
        iomemcpy(తాత్కాలిక, src, కొలత);
        పూర్ణాంకం ret = ulzma(dst, maxlen, తాత్కాలిక, కొలత);
        yield();
        విడుదల(తాత్కాలిక);
        తిరిగి ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", కొలత, src, maxlen, dst);
    ఒకవేళ (కొలత > maxlen) {
        warn_noalloc();
        తిరిగి -1;
    }
    iomemcpy(dst, src, కొలత);
    తిరిగి కొలత;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
స్థిర శూన్య
process_links_file(శూన్య)
{
    అక్షరం *links = romfile_loadfile("links", ఖాళీ), *తదుపరి = links;
    అయితే (తదుపరి) {
        // Parse out linkname and destname
        అక్షరం *linkname = తదుపరి;
        తదుపరి = strchr(linkname, '\n');
        ఒకవేళ (తదుపరి)
            *తదుపరి++ = '\0';
        అక్షరం *comment = strchr(linkname, '#');
        ఒకవేళ (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        అక్షరం *destname = strchr(linkname, ' ');
        ఒకవేళ (!destname)
            కొనసాగు;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        నిర్మాణం romfile_s *ufile = romfile_కనుగొను(destname);
        ఒకవేళ (!ufile)
            కొనసాగు;
        నిర్మాణం cbfs_romfile_s *cufile
            = container_యొక్క(ufile, నిర్మాణం cbfs_romfile_s, file);
        నిర్మాణం cbfs_romfile_s *cfile = malloc_tmp(పరిమాణం(*cfile));
        ఒకవేళ (!cfile) {
            warn_noalloc();
            ఆపు;
        }
        జ్ఞాపకం_నకలు(cfile, cufile, పరిమాణం(*cfile));
        strtcpy(cfile->file.name, linkname, పరిమాణం(cfile->file.name));
        romfile_కూడు(&cfile->file);
    }
    విడుదల(links);
}

శూన్య
coreboot_cbfs_init(శూన్య)
{
    ఒకవేళ (!CONFIG_COREBOOT_FLASH)
        తిరిగి;

    నిర్మాణం cbfs_శీర్షిక *hdr = *(శూన్య **)(CONFIG_CBFS_LOCATION - 4);
    ఒకవేళ ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        తిరిగి;
    }
    ఒకవేళ (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (శూన్య*)hdr + CONFIG_CBFS_LOCATION;
    ఒకవేళ (hdr->magic != cpu_గమ్యం_be32(CBFS_శీర్షిక_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_గమ్యం_be32(CBFS_శీర్షిక_MAGIC));
        తిరిగి;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_గమ్యం_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    నిర్మాణం cbfs_file *fhdr = (శూన్య*)romstart + be32_గమ్యం_cpu(hdr->స్థానభ్రంశం_2);
    కొరకు (;;) {
        ఒకవేళ ((u32)fhdr - romstart > romsize)
            ఆపు;
        u64 magic = fhdr->magic;
        ఒకవేళ (magic != CBFS_FILE_MAGIC)
            ఆపు;
        నిర్మాణం cbfs_romfile_s *cfile = malloc_tmp(పరిమాణం(*cfile));
        ఒకవేళ (!cfile) {
            warn_noalloc();
            ఆపు;
        }
        memset(cfile, 0, పరిమాణం(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, పరిమాణం(cfile->file.name));
        cfile->file.కొలత = cfile->rawsize = be32_గమ్యం_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->దత్తాంశం = (శూన్య*)fhdr + be32_గమ్యం_cpu(fhdr->స్థానభ్రంశం_2);
        పూర్ణాంకం len = అక్షరశ్రేణి_పొడవు(cfile->file.name);
        ఒకవేళ (len > 5 && strcmp(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.కొలత = *(u32*)(cfile->దత్తాంశం + LZMA_PROPERTIES_కొలత);
        }
        romfile_కూడు(&cfile->file);

        fhdr = (శూన్య*)ALIGN((u32)cfile->దత్తాంశం + cfile->rawsize
                            , be32_గమ్యం_cpu(hdr->align));
    }

    process_links_file();
}

నిర్మాణం cbfs_payload_segment {
    u32 type;
    u32 compression;
    u32 స్థానభ్రంశం_2;
    u64 ఎక్కించు_addr;
    u32 len;
    u32 mem_len;
} PACKED;

#నిర్వచించు PAYLOAD_SEGMENT_BSS    0x20535342
#నిర్వచించు PAYLOAD_SEGMENT_నమోదు  0x52544E45

#నిర్వచించు CBFS_COMPRESS_NONE  0
#నిర్వచించు CBFS_COMPRESS_LZMA  1

నిర్మాణం cbfs_payload {
    నిర్మాణం cbfs_payload_segment segments[1];
};

శూన్య
cbfs_నిరంతరం_payload(నిర్మాణం cbfs_file *fhdr)
{
    ఒకవేళ (!CONFIG_COREBOOT_FLASH || !fhdr)
        తిరిగి;
    dprintf(1, "Run %s\n", fhdr->filename);
    నిర్మాణం cbfs_payload *pay = (శూన్య*)fhdr + be32_గమ్యం_cpu(fhdr->స్థానభ్రంశం_2);
    నిర్మాణం cbfs_payload_segment *seg = pay->segments;
    కొరకు (;;) {
        శూన్య *src = (శూన్య*)pay + be32_గమ్యం_cpu(seg->స్థానభ్రంశం_2);
        శూన్య *dest = (శూన్య*)(u32)be64_గమ్యం_cpu(seg->ఎక్కించు_addr);
        u32 src_len = be32_గమ్యం_cpu(seg->len);
        u32 dest_len = be32_గమ్యం_cpu(seg->mem_len);
        ఎంపిక (seg->type) {
        స్థితి PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            memset(dest, 0, dest_len);
            ఆపు;
        స్థితి PAYLOAD_SEGMENT_నమోదు: {
            dprintf(1, "Calling addr %p\n", dest);
            శూన్య (*func)(శూన్య) = dest;
            func();
            తిరిగి;
        }
        అప్రమేయ:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            ఒకవేళ (seg->compression == cpu_గమ్యం_be32(CBFS_COMPRESS_NONE)) {
                ఒకవేళ (src_len > dest_len)
                    src_len = dest_len;
                జ్ఞాపకం_నకలు(dest, src, src_len);
            } లేకపోతే ఒకవేళ (CONFIG_LZMA
                       && seg->compression == cpu_గమ్యం_be32(CBFS_COMPRESS_LZMA)) {
                పూర్ణాంకం ret = ulzma(dest, dest_len, src, src_len);
                ఒకవేళ (ret < 0)
                    తిరిగి;
                src_len = ret;
            } లేకపోతే {
                dprintf(1, "No support for compression type %x\n"
                        , seg->compression);
                తిరిగి;
            }
            ఒకవేళ (dest_len > src_len)
                memset(dest + src_len, 0, dest_len - src_len);
            ఆపు;
        }
        seg++;
    }
}

// Register payloads in "img/" directory with boot system.
శూన్య
cbfs_payload_అమరిక(శూన్య)
{
    ఒకవేళ (!CONFIG_COREBOOT_FLASH)
        తిరిగి;
    నిర్మాణం romfile_s *file = ఖాళీ;
    కొరకు (;;) {
        file = romfile_findprefix("img/", file);
        ఒకవేళ (!file)
            ఆపు;
        నిర్మాణం cbfs_romfile_s *cfile;
        cfile = container_యొక్క(file, నిర్మాణం cbfs_romfile_s, file);
        స్థిరరాశి అక్షరం *filename = file->name;
        అక్షరం *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_కూడు_cbfs(cfile->fhdr, desc, bootprio_కనుగొను_named_rom(filename, 0));
    }
}

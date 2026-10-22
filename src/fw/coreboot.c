// Coreboot interface support.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // MAXDESCSIZE
#ਅਨ੍ਤਰ੍ਭਾਵ "byteorder.h" // be32_to_cpu
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "e820map.h" // e820_add
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pcidevice.h" // pci_probe_devices
#ਅਨ੍ਤਰ੍ਭਾਵ "lzmadecode.h" // LzmaDecode
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "paravirt.h" // PlatformRunningOn
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_findprefix
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // yield
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // coreboot_preinit


/****************************************************************
 * Memory map
 ****************************************************************/

ਬਣਤਰ cb_ਸਿਰਲੇਖ {
    u32 signature;
    u32 ਸਿਰਲੇਖ_ਬਾਈਟਾਂ;
    u32 ਸਿਰਲੇਖ_checksum;
    u32 ਸਾਰਣੀ_ਬਾਈਟਾਂ;
    u32 ਸਾਰਣੀ_checksum;
    u32 ਸਾਰਣੀ_entries;
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CB_SIGNATURE 0x4f49424C // "LBIO"

ਬਣਤਰ cb_ਸਮ੍ਰਿਤੀ_range {
    u64 ਸ਼ੁਰੂ;
    u64 ਮਾਪ;
    u32 type;
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CB_MEM_ਸਾਰਣੀ    16

ਬਣਤਰ cb_ਸਮ੍ਰਿਤੀ {
    u32 ਟੈਗ;
    u32 ਮਾਪ;
    ਬਣਤਰ cb_ਸਮ੍ਰਿਤੀ_range map[0];
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CB_ਟੈਗ_ਸਮ੍ਰਿਤੀ 0x01

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEM_RANGE_ਗਿਣਤੀ(_rec) \
        (((_rec)->ਮਾਪ - ਆਕਾਰ(*(_rec))) / ਆਕਾਰ((_rec)->map[0]))

ਬਣਤਰ cb_mainboard {
    u32 ਟੈਗ;
    u32 ਮਾਪ;
    u8  vendor_idx;
    u8  part_idx;
    ਅੱਖਰ  ਲੜੀਆਂ[0];
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CB_ਟੈਗ_MAINBOARD 0x0003

ਬਣਤਰ cb_forward {
    u32 ਟੈਗ;
    u32 ਮਾਪ;
    u64 forward;
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CB_ਟੈਗ_FORWARD 0x11

ਬਣਤਰ cb_cbmem_ref {
    u32 ਟੈਗ;
    u32 ਮਾਪ;
    u64 cbmem_addr;
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CB_ਟੈਗ_CBMEM_CONSOLE 0x17

ਬਣਤਰ cbmem_console {
    u32 ਮਾਪ;
    u32 cursor;
    u8  body[0];
} PACKED;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CBMC_CURSOR_ਨਕਾਬ ((1 << 28) - 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CBMC_ਉਛਾਲ (1 << 31)
ਸਥਿਰ ਬਣਤਰ cbmem_console *cbcon = ਨਿਰਾ;

ਸਥਿਰ u16
ipchksum(ਅੱਖਰ *buf, ਪੂਰਨ ਗਿਣਤੀ_2)
{
    u16 *p = (u16*)buf;
    u32 ਜੋੜ = 0;
    ਜਦੋਂ (ਗਿਣਤੀ_2 > 1) {
        ਜੋੜ += GET_FARVAR(0, *p);
        p++;
        ਗਿਣਤੀ_2 -= 2;
    }
    ਜੇ (ਗਿਣਤੀ_2)
        ਜੋੜ += GET_FARVAR(0, *(u8*)p);
    ਜੋੜ = (ਜੋੜ >> 16) + (ਜੋੜ & 0xffff);
    ਜੋੜ += (ਜੋੜ >> 16);
    ਵਾਪਸ ~ਜੋੜ;
}

// Try to locate the coreboot header in a given address range.
ਸਥਿਰ ਬਣਤਰ cb_ਸਿਰਲੇਖ *
ਲੱਭੋ_cb_ਸਿਰਲੇਖ(u32 addr, ਪੂਰਨ len)
{
    u32 ਸਿਰਾ = addr + len;
    ਲਈ (; addr < ਸਿਰਾ; addr += 16) {
        ਬਣਤਰ cb_ਸਿਰਲੇਖ *cbh = (ਖਾਲੀ*)addr;
        ਜੇ (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            ਜਾਰੀ;
        u32 tsize = GET_FARVAR(0, cbh->ਸਾਰਣੀ_ਬਾਈਟਾਂ);
        ਜੇ (! tsize)
            ਜਾਰੀ;
        ਜੇ (ipchksum((ਖਾਲੀ*)addr, ਆਕਾਰ(*cbh)) != 0)
            ਜਾਰੀ;
        ਜੇ (ipchksum((ਖਾਲੀ*)addr + ਆਕਾਰ(*cbh), tsize)
            != GET_FARVAR(0, cbh->ਸਾਰਣੀ_checksum))
            ਜਾਰੀ;
        ਵਾਪਸ cbh;
    }
    ਵਾਪਸ ਨਿਰਾ;
}

// Try to find the coreboot memory table in the given coreboot table.
ਖਾਲੀ *
ਲੱਭੋ_cb_subtable(ਬਣਤਰ cb_ਸਿਰਲੇਖ *cbh, u32 ਟੈਗ)
{
    ਅੱਖਰ *tbl = (ਅੱਖਰ *)cbh + ਆਕਾਰ(*cbh);
    u32 ਗਿਣਤੀ_2 = GET_FARVAR(0, cbh->ਸਾਰਣੀ_entries);
    ਪੂਰਨ i;
    ਲਈ (i=0; i<ਗਿਣਤੀ_2; i++) {
        ਬਣਤਰ cb_ਸਮ੍ਰਿਤੀ *cbm = (ਖਾਲੀ*)tbl;
        tbl += GET_FARVAR(0, cbm->ਮਾਪ);
        ਜੇ (GET_FARVAR(0, cbm->ਟੈਗ) == ਟੈਗ)
            ਵਾਪਸ cbm;
    }
    ਵਾਪਸ ਨਿਰਾ;
}

ਬਣਤਰ cb_ਸਿਰਲੇਖ *
ਲੱਭੋ_cb_ਸਾਰਣੀ(ਖਾਲੀ)
{
    ਬਣਤਰ cb_ਸਿਰਲੇਖ *cbh = ਲੱਭੋ_cb_ਸਿਰਲੇਖ(0, 0x1000);
    ਜੇ (!cbh)
        ਵਾਪਸ ਨਿਰਾ;
    ਬਣਤਰ cb_forward *cbf = ਲੱਭੋ_cb_subtable(cbh, CB_ਟੈਗ_FORWARD);
    ਜੇ (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = ਲੱਭੋ_cb_ਸਿਰਲੇਖ(GET_FARVAR(0, cbf->forward), 0x100);
        ਜੇ (!cbh)
            ਵਾਪਸ ਨਿਰਾ;
    }
    ਵਾਪਸ cbh;
}

ਸਥਿਰ ਬਣਤਰ cb_ਸਮ੍ਰਿਤੀ *CBMem_ਸਾਰਣੀ;
ਅਚਲ ਅੱਖਰ *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
ਖਾਲੀ
coreboot_preinit(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_COREBOOT)
        ਵਾਪਸ;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    ਬਣਤਰ cb_ਸਿਰਲੇਖ *cbh = ਲੱਭੋ_cb_ਸਾਰਣੀ();
    ਜੇ (!cbh)
        ਜਾਓ fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    ਬਣਤਰ cb_ਸਮ੍ਰਿਤੀ *cbm = CBMem_ਸਾਰਣੀ = ਲੱਭੋ_cb_subtable(cbh, CB_ਟੈਗ_ਸਮ੍ਰਿਤੀ);
    ਜੇ (!cbm)
        ਜਾਓ fail;

    ਪੂਰਨ i, ਗਿਣਤੀ_2 = MEM_RANGE_ਗਿਣਤੀ(cbm);
    ਲਈ (i=0; i<ਗਿਣਤੀ_2; i++) {
        ਬਣਤਰ cb_ਸਮ੍ਰਿਤੀ_range *m = &cbm->map[i];
        u32 type = m->type;
        ਜੇ (type == CB_MEM_ਸਾਰਣੀ)
            type = E820_RESERVED;
        e820_ਜੋੜੋ(m->ਸ਼ੁਰੂ, m->ਮਾਪ, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_ਜੋੜੋ(0, 16*1024, E820_RAM);

    ਬਣਤਰ cb_cbmem_ref *cbref = ਲੱਭੋ_cb_subtable(cbh, CB_ਟੈਗ_CBMEM_CONSOLE);
    ਜੇ (cbref) {
        cbcon = (ਖਾਲੀ*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    ਬਣਤਰ cb_mainboard *cbmb = ਲੱਭੋ_cb_subtable(cbh, CB_ਟੈਗ_MAINBOARD);
    ਜੇ (cbmb) {
        CBvendor = &cbmb->ਲੜੀਆਂ[cbmb->vendor_idx];
        CBpart = &cbmb->ਲੜੀਆਂ[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    ਵਾਪਸ;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_ਜੋੜੋ(0, 16*1024*1024, E820_RAM);
    ਵਾਪਸ;
}

ਖਾਲੀ coreboot_debug_putc(ਅੱਖਰ c)
{
    ਜੇ (!CONFIG_DEBUG_COREBOOT)
        ਵਾਪਸ;
    ਜੇ (!cbcon)
        ਵਾਪਸ;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_ਨਕਾਬ;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_ਨਕਾਬ;
    ਜੇ (cursor >= cbcon->ਮਾਪ)
        ਵਾਪਸ; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    ਜੇ (cursor >= cbcon->ਮਾਪ) {
        cursor = 0;
        flags |= CBMC_ਉਛਾਲ;
    }
    cbcon->cursor = flags | cursor;
}

/****************************************************************
 * BIOS table copying
 ****************************************************************/

// Attempt to find (and relocate) any standard bios tables found in a
// given address range.
ਸਥਿਰ ਖਾਲੀ
ਜਾਂਚ_tables(u32 ਸ਼ੁਰੂ, u32 ਮਾਪ)
{
    ਖਾਲੀ *p = (ਖਾਲੀ*)ALIGN(ਸ਼ੁਰੂ, 16);
    ਖਾਲੀ *ਸਿਰਾ = (ਖਾਲੀ*)ਸ਼ੁਰੂ + ਮਾਪ;
    ਲਈ (; p<ਸਿਰਾ; p += 16)
        copy_ਸਾਰਣੀ(p);
}

ਖਾਲੀ
coreboot_platform_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_COREBOOT)
        ਵਾਪਸ;
    pci_probe_devices();

    ਬਣਤਰ cb_ਸਮ੍ਰਿਤੀ *cbm = CBMem_ਸਾਰਣੀ;
    ਜੇ (!cbm)
        ਵਾਪਸ;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    ਪੂਰਨ i, ਗਿਣਤੀ_2 = MEM_RANGE_ਗਿਣਤੀ(cbm);
    ਲਈ (i=0; i<ਗਿਣਤੀ_2; i++) {
        ਬਣਤਰ cb_ਸਮ੍ਰਿਤੀ_range *m = &cbm->map[i];
        ਜੇ (m->type == CB_MEM_ਸਾਰਣੀ)
            ਜਾਂਚ_tables(m->ਸ਼ੁਰੂ, m->ਮਾਪ);
    }

    ਲੱਭੋ_acpi_features();
}


/****************************************************************
 * ulzma
 ****************************************************************/

// Uncompress data in flash to an area of memory.
ਸਥਿਰ ਪੂਰਨ
ulzma(u8 *dst, u32 maxlen, ਅਚਲ u8 *src, u32 srclen)
{
    dprintf(3, "Uncompressing data %d@%p to %d@%p\n", srclen, src, maxlen, dst);
    CLzmaDecoderState state;
    ਪੂਰਨ ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_ਮਾਪ);
    ਜੇ (ret != LZMA_ਨਤੀਜਾ_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        ਵਾਪਸ -1;
    }
    u8 scratch[15980];
    ਪੂਰਨ need = (Lzma_Get_ਅੰਕ_Probs(&state.Properties) * ਆਕਾਰ(CProb));
    ਜੇ (need > ਆਕਾਰ(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)ਆਕਾਰ(scratch));
        ਵਾਪਸ -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_ਮਾਪ);
    ਜੇ (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        ਵਾਪਸ -1;
    }
    u32 ਵਿੱਚ_Processed, ਬਾਹਰਲਾ_Processed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_ਮਾਪ + 8, srclen
                     , &ਵਿੱਚ_Processed, dst, dstlen, &ਬਾਹਰਲਾ_Processed);
    ਜੇ (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        ਵਾਪਸ -1;
    }
    ਵਾਪਸ dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CBFS_ਸਿਰਲੇਖ_MAGIC 0x4F524243
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CBFS_VERSION1 0x31313131

ਬਣਤਰ cbfs_ਸਿਰਲੇਖ {
    u32 magic;
    u32 ਸੰਸਕਰਣ_2;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 ਵਿੱਥ_2;
    u32 pad[2];
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

ਬਣਤਰ cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 ਵਿੱਥ_2;
    ਅੱਖਰ filename[0];
} PACKED;

ਬਣਤਰ cbfs_romfile_s {
    ਬਣਤਰ romfile_s file;
    ਬਣਤਰ cbfs_file *fhdr;
    ਖਾਲੀ *ਅੰਕੜੇ;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
ਸਥਿਰ ਪੂਰਨ
cbfs_copyfile(ਬਣਤਰ romfile_s *file, ਖਾਲੀ *dst, u32 maxlen)
{
    ਜੇ (!CONFIG_COREBOOT_FLASH)
        ਵਾਪਸ -1;

    ਬਣਤਰ cbfs_romfile_s *cfile;
    cfile = container_ਦਾ(file, ਬਣਤਰ cbfs_romfile_s, file);
    u32 ਮਾਪ = cfile->rawsize;
    ਖਾਲੀ *src = cfile->ਅੰਕੜੇ;
    ਜੇ (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        ਖਾਲੀ *ਆਰਜ਼ੀ = malloc_tmphigh(ਮਾਪ);
        ਜੇ (!ਆਰਜ਼ੀ) {
            warn_noalloc();
            ਵਾਪਸ -1;
        }
        iomemcpy(ਆਰਜ਼ੀ, src, ਮਾਪ);
        ਪੂਰਨ ret = ulzma(dst, maxlen, ਆਰਜ਼ੀ, ਮਾਪ);
        yield();
        ਮੁਕਤ(ਆਰਜ਼ੀ);
        ਵਾਪਸ ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", ਮਾਪ, src, maxlen, dst);
    ਜੇ (ਮਾਪ > maxlen) {
        warn_noalloc();
        ਵਾਪਸ -1;
    }
    iomemcpy(dst, src, ਮਾਪ);
    ਵਾਪਸ ਮਾਪ;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
ਸਥਿਰ ਖਾਲੀ
process_links_file(ਖਾਲੀ)
{
    ਅੱਖਰ *links = romfile_loadfile("links", ਨਿਰਾ), *ਅਗਲਾ = links;
    ਜਦੋਂ (ਅਗਲਾ) {
        // Parse out linkname and destname
        ਅੱਖਰ *linkname = ਅਗਲਾ;
        ਅਗਲਾ = strchr(linkname, '\n');
        ਜੇ (ਅਗਲਾ)
            *ਅਗਲਾ++ = '\0';
        ਅੱਖਰ *comment = strchr(linkname, '#');
        ਜੇ (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        ਅੱਖਰ *destname = strchr(linkname, ' ');
        ਜੇ (!destname)
            ਜਾਰੀ;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        ਬਣਤਰ romfile_s *ufile = romfile_ਲੱਭੋ(destname);
        ਜੇ (!ufile)
            ਜਾਰੀ;
        ਬਣਤਰ cbfs_romfile_s *cufile
            = container_ਦਾ(ufile, ਬਣਤਰ cbfs_romfile_s, file);
        ਬਣਤਰ cbfs_romfile_s *cfile = malloc_tmp(ਆਕਾਰ(*cfile));
        ਜੇ (!cfile) {
            warn_noalloc();
            ਰੋਕੋ;
        }
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(cfile, cufile, ਆਕਾਰ(*cfile));
        strtcpy(cfile->file.name, linkname, ਆਕਾਰ(cfile->file.name));
        romfile_ਜੋੜੋ(&cfile->file);
    }
    ਮੁਕਤ(links);
}

ਖਾਲੀ
coreboot_cbfs_init(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_COREBOOT_FLASH)
        ਵਾਪਸ;

    ਬਣਤਰ cbfs_ਸਿਰਲੇਖ *hdr = *(ਖਾਲੀ **)(CONFIG_CBFS_LOCATION - 4);
    ਜੇ ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        ਵਾਪਸ;
    }
    ਜੇ (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (ਖਾਲੀ*)hdr + CONFIG_CBFS_LOCATION;
    ਜੇ (hdr->magic != cpu_ਮੰਜ਼ਿਲ_be32(CBFS_ਸਿਰਲੇਖ_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_ਮੰਜ਼ਿਲ_be32(CBFS_ਸਿਰਲੇਖ_MAGIC));
        ਵਾਪਸ;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_ਮੰਜ਼ਿਲ_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    ਬਣਤਰ cbfs_file *fhdr = (ਖਾਲੀ*)romstart + be32_ਮੰਜ਼ਿਲ_cpu(hdr->ਵਿੱਥ_2);
    ਲਈ (;;) {
        ਜੇ ((u32)fhdr - romstart > romsize)
            ਰੋਕੋ;
        u64 magic = fhdr->magic;
        ਜੇ (magic != CBFS_FILE_MAGIC)
            ਰੋਕੋ;
        ਬਣਤਰ cbfs_romfile_s *cfile = malloc_tmp(ਆਕਾਰ(*cfile));
        ਜੇ (!cfile) {
            warn_noalloc();
            ਰੋਕੋ;
        }
        memset(cfile, 0, ਆਕਾਰ(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, ਆਕਾਰ(cfile->file.name));
        cfile->file.ਮਾਪ = cfile->rawsize = be32_ਮੰਜ਼ਿਲ_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->ਅੰਕੜੇ = (ਖਾਲੀ*)fhdr + be32_ਮੰਜ਼ਿਲ_cpu(fhdr->ਵਿੱਥ_2);
        ਪੂਰਨ len = ਲੜੀ_ਲੰਬਾਈ(cfile->file.name);
        ਜੇ (len > 5 && strcmp(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.ਮਾਪ = *(u32*)(cfile->ਅੰਕੜੇ + LZMA_PROPERTIES_ਮਾਪ);
        }
        romfile_ਜੋੜੋ(&cfile->file);

        fhdr = (ਖਾਲੀ*)ALIGN((u32)cfile->ਅੰਕੜੇ + cfile->rawsize
                            , be32_ਮੰਜ਼ਿਲ_cpu(hdr->align));
    }

    process_links_file();
}

ਬਣਤਰ cbfs_payload_segment {
    u32 type;
    u32 compression;
    u32 ਵਿੱਥ_2;
    u64 ਲੱਦੋ_addr;
    u32 len;
    u32 mem_len;
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAYLOAD_SEGMENT_BSS    0x20535342
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAYLOAD_SEGMENT_ਇੰਦਰਾਜ  0x52544E45

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CBFS_COMPRESS_NONE  0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CBFS_COMPRESS_LZMA  1

ਬਣਤਰ cbfs_payload {
    ਬਣਤਰ cbfs_payload_segment segments[1];
};

ਖਾਲੀ
cbfs_ਲੜੀਵਾਰ_payload(ਬਣਤਰ cbfs_file *fhdr)
{
    ਜੇ (!CONFIG_COREBOOT_FLASH || !fhdr)
        ਵਾਪਸ;
    dprintf(1, "Run %s\n", fhdr->filename);
    ਬਣਤਰ cbfs_payload *pay = (ਖਾਲੀ*)fhdr + be32_ਮੰਜ਼ਿਲ_cpu(fhdr->ਵਿੱਥ_2);
    ਬਣਤਰ cbfs_payload_segment *seg = pay->segments;
    ਲਈ (;;) {
        ਖਾਲੀ *src = (ਖਾਲੀ*)pay + be32_ਮੰਜ਼ਿਲ_cpu(seg->ਵਿੱਥ_2);
        ਖਾਲੀ *dest = (ਖਾਲੀ*)(u32)be64_ਮੰਜ਼ਿਲ_cpu(seg->ਲੱਦੋ_addr);
        u32 src_len = be32_ਮੰਜ਼ਿਲ_cpu(seg->len);
        u32 dest_len = be32_ਮੰਜ਼ਿਲ_cpu(seg->mem_len);
        ਚੋਣ (seg->type) {
        ਹਾਲਤ PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            memset(dest, 0, dest_len);
            ਰੋਕੋ;
        ਹਾਲਤ PAYLOAD_SEGMENT_ਇੰਦਰਾਜ: {
            dprintf(1, "Calling addr %p\n", dest);
            ਖਾਲੀ (*func)(ਖਾਲੀ) = dest;
            func();
            ਵਾਪਸ;
        }
        ਮੂਲ:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            ਜੇ (seg->compression == cpu_ਮੰਜ਼ਿਲ_be32(CBFS_COMPRESS_NONE)) {
                ਜੇ (src_len > dest_len)
                    src_len = dest_len;
                ਸਮ੍ਰਿਤੀ_ਨਕਲ(dest, src, src_len);
            } ਨਹੀਂਤਾਂ ਜੇ (CONFIG_LZMA
                       && seg->compression == cpu_ਮੰਜ਼ਿਲ_be32(CBFS_COMPRESS_LZMA)) {
                ਪੂਰਨ ret = ulzma(dest, dest_len, src, src_len);
                ਜੇ (ret < 0)
                    ਵਾਪਸ;
                src_len = ret;
            } ਨਹੀਂਤਾਂ {
                dprintf(1, "No support for compression type %x\n"
                        , seg->compression);
                ਵਾਪਸ;
            }
            ਜੇ (dest_len > src_len)
                memset(dest + src_len, 0, dest_len - src_len);
            ਰੋਕੋ;
        }
        seg++;
    }
}

// Register payloads in "img/" directory with boot system.
ਖਾਲੀ
cbfs_payload_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_COREBOOT_FLASH)
        ਵਾਪਸ;
    ਬਣਤਰ romfile_s *file = ਨਿਰਾ;
    ਲਈ (;;) {
        file = romfile_findprefix("img/", file);
        ਜੇ (!file)
            ਰੋਕੋ;
        ਬਣਤਰ cbfs_romfile_s *cfile;
        cfile = container_ਦਾ(file, ਬਣਤਰ cbfs_romfile_s, file);
        ਅਚਲ ਅੱਖਰ *filename = file->name;
        ਅੱਖਰ *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_ਜੋੜੋ_cbfs(cfile->fhdr, desc, bootprio_ਲੱਭੋ_named_rom(filename, 0));
    }
}

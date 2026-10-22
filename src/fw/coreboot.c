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

ರಚನೆ cb_ಶೀರ್ಷಿಕೆ {
    u32 signature;
    u32 ಶೀರ್ಷಿಕೆ_ಬೈಟುಗಳು;
    u32 ಶೀರ್ಷಿಕೆ_checksum;
    u32 ಕೋಷ್ಟಕ_ಬೈಟುಗಳು;
    u32 ಕೋಷ್ಟಕ_checksum;
    u32 ಕೋಷ್ಟಕ_entries;
};

#ವ್ಯಾಖ್ಯಾನಿಸು CB_SIGNATURE 0x4f49424C // "LBIO"

ರಚನೆ cb_ಸ್ಮರಣೆ_range {
    u64 ಆರಂಭ;
    u64 ಗಾತ್ರ_2;
    u32 type;
};

#ವ್ಯಾಖ್ಯಾನಿಸು CB_MEM_ಕೋಷ್ಟಕ    16

ರಚನೆ cb_ಸ್ಮರಣೆ {
    u32 ಟ್ಯಾಗ್;
    u32 ಗಾತ್ರ_2;
    ರಚನೆ cb_ಸ್ಮರಣೆ_range map[0];
};

#ವ್ಯಾಖ್ಯಾನಿಸು CB_ಟ್ಯಾಗ್_ಸ್ಮರಣೆ 0x01

#ವ್ಯಾಖ್ಯಾನಿಸು MEM_RANGE_ಎಣಿಕೆ(_rec) \
        (((_rec)->ಗಾತ್ರ_2 - ಗಾತ್ರ(*(_rec))) / ಗಾತ್ರ((_rec)->map[0]))

ರಚನೆ cb_mainboard {
    u32 ಟ್ಯಾಗ್;
    u32 ಗಾತ್ರ_2;
    u8  vendor_idx;
    u8  part_idx;
    ಅಕ್ಷರ  ಅಕ್ಷರಸರಣಿಗಳು[0];
};

#ವ್ಯಾಖ್ಯಾನಿಸು CB_ಟ್ಯಾಗ್_MAINBOARD 0x0003

ರಚನೆ cb_forward {
    u32 ಟ್ಯಾಗ್;
    u32 ಗಾತ್ರ_2;
    u64 forward;
};

#ವ್ಯಾಖ್ಯಾನಿಸು CB_ಟ್ಯಾಗ್_FORWARD 0x11

ರಚನೆ cb_cbmem_ref {
    u32 ಟ್ಯಾಗ್;
    u32 ಗಾತ್ರ_2;
    u64 cbmem_addr;
};

#ವ್ಯಾಖ್ಯಾನಿಸು CB_ಟ್ಯಾಗ್_CBMEM_CONSOLE 0x17

ರಚನೆ cbmem_console {
    u32 ಗಾತ್ರ_2;
    u32 cursor;
    u8  body[0];
} PACKED;
#ವ್ಯಾಖ್ಯಾನಿಸು CBMC_CURSOR_ಮುಖವಾಡ ((1 << 28) - 1)
#ವ್ಯಾಖ್ಯಾನಿಸು CBMC_ಅತಿಪ್ರವಾಹ (1 << 31)
ಸ್ಥಿರ ರಚನೆ cbmem_console *cbcon = ಖಾಲಿ;

ಸ್ಥಿರ u16
ipchksum(ಅಕ್ಷರ *buf, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2)
{
    u16 *p = (u16*)buf;
    u32 ಮೊತ್ತ = 0;
    ಆಗ (ಎಣಿಕೆ_2 > 1) {
        ಮೊತ್ತ += GET_FARVAR(0, *p);
        p++;
        ಎಣಿಕೆ_2 -= 2;
    }
    ಆದರೆ (ಎಣಿಕೆ_2)
        ಮೊತ್ತ += GET_FARVAR(0, *(u8*)p);
    ಮೊತ್ತ = (ಮೊತ್ತ >> 16) + (ಮೊತ್ತ & 0xffff);
    ಮೊತ್ತ += (ಮೊತ್ತ >> 16);
    ಹಿಂತಿರುಗಿ ~ಮೊತ್ತ;
}

// Try to locate the coreboot header in a given address range.
ಸ್ಥಿರ ರಚನೆ cb_ಶೀರ್ಷಿಕೆ *
ಕಂಡುಹಿಡಿ_cb_ಶೀರ್ಷಿಕೆ(u32 addr, ಪೂರ್ಣಾಂಕ len)
{
    u32 ಕೊನೆ = addr + len;
    ಗಾಗಿ (; addr < ಕೊನೆ; addr += 16) {
        ರಚನೆ cb_ಶೀರ್ಷಿಕೆ *cbh = (ಶೂನ್ಯ*)addr;
        ಆದರೆ (GET_FARVAR(0, cbh->signature) != CB_SIGNATURE)
            ಮುಂದುವರಿ;
        u32 tsize = GET_FARVAR(0, cbh->ಕೋಷ್ಟಕ_ಬೈಟುಗಳು);
        ಆದರೆ (! tsize)
            ಮುಂದುವರಿ;
        ಆದರೆ (ipchksum((ಶೂನ್ಯ*)addr, ಗಾತ್ರ(*cbh)) != 0)
            ಮುಂದುವರಿ;
        ಆದರೆ (ipchksum((ಶೂನ್ಯ*)addr + ಗಾತ್ರ(*cbh), tsize)
            != GET_FARVAR(0, cbh->ಕೋಷ್ಟಕ_checksum))
            ಮುಂದುವರಿ;
        ಹಿಂತಿರುಗಿ cbh;
    }
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

// Try to find the coreboot memory table in the given coreboot table.
ಶೂನ್ಯ *
ಕಂಡುಹಿಡಿ_cb_subtable(ರಚನೆ cb_ಶೀರ್ಷಿಕೆ *cbh, u32 ಟ್ಯಾಗ್)
{
    ಅಕ್ಷರ *tbl = (ಅಕ್ಷರ *)cbh + ಗಾತ್ರ(*cbh);
    u32 ಎಣಿಕೆ_2 = GET_FARVAR(0, cbh->ಕೋಷ್ಟಕ_entries);
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<ಎಣಿಕೆ_2; i++) {
        ರಚನೆ cb_ಸ್ಮರಣೆ *cbm = (ಶೂನ್ಯ*)tbl;
        tbl += GET_FARVAR(0, cbm->ಗಾತ್ರ_2);
        ಆದರೆ (GET_FARVAR(0, cbm->ಟ್ಯಾಗ್) == ಟ್ಯಾಗ್)
            ಹಿಂತಿರುಗಿ cbm;
    }
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ರಚನೆ cb_ಶೀರ್ಷಿಕೆ *
ಕಂಡುಹಿಡಿ_cb_ಕೋಷ್ಟಕ(ಶೂನ್ಯ)
{
    ರಚನೆ cb_ಶೀರ್ಷಿಕೆ *cbh = ಕಂಡುಹಿಡಿ_cb_ಶೀರ್ಷಿಕೆ(0, 0x1000);
    ಆದರೆ (!cbh)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ರಚನೆ cb_forward *cbf = ಕಂಡುಹಿಡಿ_cb_subtable(cbh, CB_ಟ್ಯಾಗ್_FORWARD);
    ಆದರೆ (cbf) {
        dprintf(3, "Found coreboot table forwarder.\n");
        cbh = ಕಂಡುಹಿಡಿ_cb_ಶೀರ್ಷಿಕೆ(GET_FARVAR(0, cbf->forward), 0x100);
        ಆದರೆ (!cbh)
            ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    ಹಿಂತಿರುಗಿ cbh;
}

ಸ್ಥಿರ ರಚನೆ cb_ಸ್ಮರಣೆ *CBMem_ಕೋಷ್ಟಕ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *CBvendor = "", *CBpart = "";

// Populate max ram and e820 map info by scanning for a coreboot table.
ಶೂನ್ಯ
coreboot_preinit(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_COREBOOT)
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "Attempting to find coreboot table\n");

    // Find coreboot table.
    ರಚನೆ cb_ಶೀರ್ಷಿಕೆ *cbh = ಕಂಡುಹಿಡಿ_cb_ಕೋಷ್ಟಕ();
    ಆದರೆ (!cbh)
        ಹೋಗು fail;
    dprintf(3, "Now attempting to find coreboot memory map\n");
    ರಚನೆ cb_ಸ್ಮರಣೆ *cbm = CBMem_ಕೋಷ್ಟಕ = ಕಂಡುಹಿಡಿ_cb_subtable(cbh, CB_ಟ್ಯಾಗ್_ಸ್ಮರಣೆ);
    ಆದರೆ (!cbm)
        ಹೋಗು fail;

    ಪೂರ್ಣಾಂಕ i, ಎಣಿಕೆ_2 = MEM_RANGE_ಎಣಿಕೆ(cbm);
    ಗಾಗಿ (i=0; i<ಎಣಿಕೆ_2; i++) {
        ರಚನೆ cb_ಸ್ಮರಣೆ_range *m = &cbm->map[i];
        u32 type = m->type;
        ಆದರೆ (type == CB_MEM_ಕೋಷ್ಟಕ)
            type = E820_RESERVED;
        e820_ಕೂಡಿಸು(m->ಆರಂಭ, m->ಗಾತ್ರ_2, type);
    }

    // Ughh - coreboot likes to set a map at 0x0000-0x1000, but this
    // confuses grub.  So, override it.
    e820_ಕೂಡಿಸು(0, 16*1024, E820_RAM);

    ರಚನೆ cb_cbmem_ref *cbref = ಕಂಡುಹಿಡಿ_cb_subtable(cbh, CB_ಟ್ಯಾಗ್_CBMEM_CONSOLE);
    ಆದರೆ (cbref) {
        cbcon = (ಶೂನ್ಯ*)(u32)cbref->cbmem_addr;
        debug_banner();
        dprintf(1, "Found coreboot cbmem console @ %llx\n", cbref->cbmem_addr);
    }

    ರಚನೆ cb_mainboard *cbmb = ಕಂಡುಹಿಡಿ_cb_subtable(cbh, CB_ಟ್ಯಾಗ್_MAINBOARD);
    ಆದರೆ (cbmb) {
        CBvendor = &cbmb->ಅಕ್ಷರಸರಣಿಗಳು[cbmb->vendor_idx];
        CBpart = &cbmb->ಅಕ್ಷರಸರಣಿಗಳು[cbmb->part_idx];
        dprintf(1, "Found mainboard %s %s\n", CBvendor, CBpart);
    }

    ಹಿಂತಿರುಗಿ;

fail:
    // No table found..  Use 16Megs as a dummy value.
    dprintf(1, "Unable to find coreboot table!\n");
    e820_ಕೂಡಿಸು(0, 16*1024*1024, E820_RAM);
    ಹಿಂತಿರುಗಿ;
}

ಶೂನ್ಯ coreboot_debug_putc(ಅಕ್ಷರ c)
{
    ಆದರೆ (!CONFIG_DEBUG_COREBOOT)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (!cbcon)
        ಹಿಂತಿರುಗಿ;
    u32 cursor = cbcon->cursor & CBMC_CURSOR_ಮುಖವಾಡ;
    u32 flags = cbcon->cursor & ~CBMC_CURSOR_ಮುಖವಾಡ;
    ಆದರೆ (cursor >= cbcon->ಗಾತ್ರ_2)
        ಹಿಂತಿರುಗಿ; // Old coreboot version with legacy overflow mechanism.
    cbcon->body[cursor++] = c;
    ಆದರೆ (cursor >= cbcon->ಗಾತ್ರ_2) {
        cursor = 0;
        flags |= CBMC_ಅತಿಪ್ರವಾಹ;
    }
    cbcon->cursor = flags | cursor;
}

/****************************************************************
 * BIOS table copying
 ****************************************************************/

// Attempt to find (and relocate) any standard bios tables found in a
// given address range.
ಸ್ಥಿರ ಶೂನ್ಯ
ಪರಿಶೀಲನೆ_tables(u32 ಆರಂಭ, u32 ಗಾತ್ರ_2)
{
    ಶೂನ್ಯ *p = (ಶೂನ್ಯ*)ALIGN(ಆರಂಭ, 16);
    ಶೂನ್ಯ *ಕೊನೆ = (ಶೂನ್ಯ*)ಆರಂಭ + ಗಾತ್ರ_2;
    ಗಾಗಿ (; p<ಕೊನೆ; p += 16)
        copy_ಕೋಷ್ಟಕ(p);
}

ಶೂನ್ಯ
coreboot_platform_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_COREBOOT)
        ಹಿಂತಿರುಗಿ;
    pci_probe_devices();

    ರಚನೆ cb_ಸ್ಮರಣೆ *cbm = CBMem_ಕೋಷ್ಟಕ;
    ಆದರೆ (!cbm)
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "Relocating coreboot bios tables\n");

    // Scan CB_MEM_TABLE areas for bios tables.
    ಪೂರ್ಣಾಂಕ i, ಎಣಿಕೆ_2 = MEM_RANGE_ಎಣಿಕೆ(cbm);
    ಗಾಗಿ (i=0; i<ಎಣಿಕೆ_2; i++) {
        ರಚನೆ cb_ಸ್ಮರಣೆ_range *m = &cbm->map[i];
        ಆದರೆ (m->type == CB_MEM_ಕೋಷ್ಟಕ)
            ಪರಿಶೀಲನೆ_tables(m->ಆರಂಭ, m->ಗಾತ್ರ_2);
    }

    ಕಂಡುಹಿಡಿ_acpi_features();
}


/****************************************************************
 * ulzma
 ****************************************************************/

// Uncompress data in flash to an area of memory.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ulzma(u8 *dst, u32 maxlen, ಸ್ಥಿರಾಂಕ u8 *src, u32 srclen)
{
    dprintf(3, "Uncompressing data %d@%p to %d@%p\n", srclen, src, maxlen, dst);
    CLzmaDecoderState state;
    ಪೂರ್ಣಾಂಕ ret = LzmaDecodeProperties(&state.Properties, src, LZMA_PROPERTIES_ಗಾತ್ರ);
    ಆದರೆ (ret != LZMA_ಫಲಿತಾಂಶ_OK) {
        dprintf(1, "LzmaDecodeProperties error - %d\n", ret);
        ಹಿಂತಿರುಗಿ -1;
    }
    u8 scratch[15980];
    ಪೂರ್ಣಾಂಕ need = (Lzma_Get_ಅಂಕೆ_Probs(&state.Properties) * ಗಾತ್ರ(CProb));
    ಆದರೆ (need > ಗಾತ್ರ(scratch)) {
        dprintf(1, "LzmaDecode need %d have %d\n", need, (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(scratch));
        ಹಿಂತಿರುಗಿ -1;
    }
    state.Probs = (CProb *)scratch;

    u32 dstlen = *(u32*)(src + LZMA_PROPERTIES_ಗಾತ್ರ);
    ಆದರೆ (dstlen > maxlen) {
        dprintf(1, "LzmaDecode too large (max %d need %d)\n", maxlen, dstlen);
        ಹಿಂತಿರುಗಿ -1;
    }
    u32 ಒಳ_Processed, ಹೊರಹೋಗು_Processed;
    ret = LzmaDecode(&state, src + LZMA_PROPERTIES_ಗಾತ್ರ + 8, srclen
                     , &ಒಳ_Processed, dst, dstlen, &ಹೊರಹೋಗು_Processed);
    ಆದರೆ (ret) {
        dprintf(1, "LzmaDecode returned %d\n", ret);
        ಹಿಂತಿರುಗಿ -1;
    }
    ಹಿಂತಿರುಗಿ dstlen;
}


/****************************************************************
 * Coreboot flash format
 ****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು CBFS_ಶೀರ್ಷಿಕೆ_MAGIC 0x4F524243
#ವ್ಯಾಖ್ಯಾನಿಸು CBFS_VERSION1 0x31313131

ರಚನೆ cbfs_ಶೀರ್ಷಿಕೆ {
    u32 magic;
    u32 ಆವೃತ್ತಿ_2;
    u32 romsize;
    u32 bootblocksize;
    u32 align;
    u32 ಸ್ಥಾನಾಂತರ_2;
    u32 pad[2];
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು CBFS_FILE_MAGIC 0x455649484352414cLL // LARCHIVE

ರಚನೆ cbfs_file {
    u64 magic;
    u32 len;
    u32 type;
    u32 checksum;
    u32 ಸ್ಥಾನಾಂತರ_2;
    ಅಕ್ಷರ filename[0];
} PACKED;

ರಚನೆ cbfs_romfile_s {
    ರಚನೆ romfile_s file;
    ರಚನೆ cbfs_file *fhdr;
    ಶೂನ್ಯ *ದತ್ತಾಂಶ;
    u32 rawsize, flags;
};

// Copy a file to memory (uncompressing if necessary)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
cbfs_copyfile(ರಚನೆ romfile_s *file, ಶೂನ್ಯ *dst, u32 maxlen)
{
    ಆದರೆ (!CONFIG_COREBOOT_FLASH)
        ಹಿಂತಿರುಗಿ -1;

    ರಚನೆ cbfs_romfile_s *cfile;
    cfile = container_ದ(file, ರಚನೆ cbfs_romfile_s, file);
    u32 ಗಾತ್ರ_2 = cfile->rawsize;
    ಶೂನ್ಯ *src = cfile->ದತ್ತಾಂಶ;
    ಆದರೆ (cfile->flags) {
        // Compressed - copy to temp ram and uncompress it.
        ಶೂನ್ಯ *ತಾತ್ಕಾಲಿಕ = malloc_tmphigh(ಗಾತ್ರ_2);
        ಆದರೆ (!ತಾತ್ಕಾಲಿಕ) {
            warn_noalloc();
            ಹಿಂತಿರುಗಿ -1;
        }
        iomemcpy(ತಾತ್ಕಾಲಿಕ, src, ಗಾತ್ರ_2);
        ಪೂರ್ಣಾಂಕ ret = ulzma(dst, maxlen, ತಾತ್ಕಾಲಿಕ, ಗಾತ್ರ_2);
        yield();
        ಬಿಡುಗಡೆ(ತಾತ್ಕಾಲಿಕ);
        ಹಿಂತಿರುಗಿ ret;
    }

    // Not compressed.
    dprintf(3, "Copying data %d@%p to %d@%p\n", ಗಾತ್ರ_2, src, maxlen, dst);
    ಆದರೆ (ಗಾತ್ರ_2 > maxlen) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ -1;
    }
    iomemcpy(dst, src, ಗಾತ್ರ_2);
    ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2;
}

// Process CBFS links file.  The links file is a newline separated
// file where each line has a "link name" and a "destination name"
// separated by a space character.
ಸ್ಥಿರ ಶೂನ್ಯ
process_links_file(ಶೂನ್ಯ)
{
    ಅಕ್ಷರ *links = romfile_loadfile("links", ಖಾಲಿ), *ಮುಂದಿನ = links;
    ಆಗ (ಮುಂದಿನ) {
        // Parse out linkname and destname
        ಅಕ್ಷರ *linkname = ಮುಂದಿನ;
        ಮುಂದಿನ = strchr(linkname, '\n');
        ಆದರೆ (ಮುಂದಿನ)
            *ಮುಂದಿನ++ = '\0';
        ಅಕ್ಷರ *comment = strchr(linkname, '#');
        ಆದರೆ (comment)
            *comment = '\0';
        linkname = nullTrailingSpace(linkname);
        ಅಕ್ಷರ *destname = strchr(linkname, ' ');
        ಆದರೆ (!destname)
            ಮುಂದುವರಿ;
        *destname++ = '\0';
        destname = nullTrailingSpace(destname);
        // Lookup destname and create new romfile entry for linkname
        ರಚನೆ romfile_s *ufile = romfile_ಕಂಡುಹಿಡಿ(destname);
        ಆದರೆ (!ufile)
            ಮುಂದುವರಿ;
        ರಚನೆ cbfs_romfile_s *cufile
            = container_ದ(ufile, ರಚನೆ cbfs_romfile_s, file);
        ರಚನೆ cbfs_romfile_s *cfile = malloc_tmp(ಗಾತ್ರ(*cfile));
        ಆದರೆ (!cfile) {
            warn_noalloc();
            ನಿಲ್ಲಿಸು;
        }
        ಸ್ಮರಣೆ_ನಕಲು(cfile, cufile, ಗಾತ್ರ(*cfile));
        strtcpy(cfile->file.name, linkname, ಗಾತ್ರ(cfile->file.name));
        romfile_ಕೂಡಿಸು(&cfile->file);
    }
    ಬಿಡುಗಡೆ(links);
}

ಶೂನ್ಯ
coreboot_cbfs_init(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_COREBOOT_FLASH)
        ಹಿಂತಿರುಗಿ;

    ರಚನೆ cbfs_ಶೀರ್ಷಿಕೆ *hdr = *(ಶೂನ್ಯ **)(CONFIG_CBFS_LOCATION - 4);
    ಆದರೆ ((u32)hdr & 0x03) {
        dprintf(1, "Invalid CBFS pointer %p\n", hdr);
        ಹಿಂತಿರುಗಿ;
    }
    ಆದರೆ (CONFIG_CBFS_LOCATION && (u32)hdr > CONFIG_CBFS_LOCATION)
        // Looks like the pointer is relative to CONFIG_CBFS_LOCATION
        hdr = (ಶೂನ್ಯ*)hdr + CONFIG_CBFS_LOCATION;
    ಆದರೆ (hdr->magic != cpu_ಗಮ್ಯ_be32(CBFS_ಶೀರ್ಷಿಕೆ_MAGIC)) {
        dprintf(1, "Unable to find CBFS (ptr=%p; got %x not %x)\n"
                , hdr, hdr->magic, cpu_ಗಮ್ಯ_be32(CBFS_ಶೀರ್ಷಿಕೆ_MAGIC));
        ಹಿಂತಿರುಗಿ;
    }
    dprintf(1, "Found CBFS header at %p\n", hdr);

    u32 romsize = be32_ಗಮ್ಯ_cpu(hdr->romsize);
    u32 romstart = CONFIG_CBFS_LOCATION - romsize;
    ರಚನೆ cbfs_file *fhdr = (ಶೂನ್ಯ*)romstart + be32_ಗಮ್ಯ_cpu(hdr->ಸ್ಥಾನಾಂತರ_2);
    ಗಾಗಿ (;;) {
        ಆದರೆ ((u32)fhdr - romstart > romsize)
            ನಿಲ್ಲಿಸು;
        u64 magic = fhdr->magic;
        ಆದರೆ (magic != CBFS_FILE_MAGIC)
            ನಿಲ್ಲಿಸು;
        ರಚನೆ cbfs_romfile_s *cfile = malloc_tmp(ಗಾತ್ರ(*cfile));
        ಆದರೆ (!cfile) {
            warn_noalloc();
            ನಿಲ್ಲಿಸು;
        }
        memset(cfile, 0, ಗಾತ್ರ(*cfile));
        strtcpy(cfile->file.name, fhdr->filename, ಗಾತ್ರ(cfile->file.name));
        cfile->file.ಗಾತ್ರ_2 = cfile->rawsize = be32_ಗಮ್ಯ_cpu(fhdr->len);
        cfile->fhdr = fhdr;
        cfile->file.copy = cbfs_copyfile;
        cfile->ದತ್ತಾಂಶ = (ಶೂನ್ಯ*)fhdr + be32_ಗಮ್ಯ_cpu(fhdr->ಸ್ಥಾನಾಂತರ_2);
        ಪೂರ್ಣಾಂಕ len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cfile->file.name);
        ಆದರೆ (len > 5 && strcmp(&cfile->file.name[len-5], ".lzma") == 0) {
            // Using compression.
            cfile->flags = 1;
            cfile->file.name[len-5] = '\0';
            cfile->file.ಗಾತ್ರ_2 = *(u32*)(cfile->ದತ್ತಾಂಶ + LZMA_PROPERTIES_ಗಾತ್ರ);
        }
        romfile_ಕೂಡಿಸು(&cfile->file);

        fhdr = (ಶೂನ್ಯ*)ALIGN((u32)cfile->ದತ್ತಾಂಶ + cfile->rawsize
                            , be32_ಗಮ್ಯ_cpu(hdr->align));
    }

    process_links_file();
}

ರಚನೆ cbfs_payload_segment {
    u32 type;
    u32 compression;
    u32 ಸ್ಥಾನಾಂತರ_2;
    u64 ತುಂಬು_addr;
    u32 len;
    u32 mem_len;
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು PAYLOAD_SEGMENT_BSS    0x20535342
#ವ್ಯಾಖ್ಯಾನಿಸು PAYLOAD_SEGMENT_ನಮೂದು  0x52544E45

#ವ್ಯಾಖ್ಯಾನಿಸು CBFS_COMPRESS_NONE  0
#ವ್ಯಾಖ್ಯಾನಿಸು CBFS_COMPRESS_LZMA  1

ರಚನೆ cbfs_payload {
    ರಚನೆ cbfs_payload_segment segments[1];
};

ಶೂನ್ಯ
cbfs_ಸತತ_payload(ರಚನೆ cbfs_file *fhdr)
{
    ಆದರೆ (!CONFIG_COREBOOT_FLASH || !fhdr)
        ಹಿಂತಿರುಗಿ;
    dprintf(1, "Run %s\n", fhdr->filename);
    ರಚನೆ cbfs_payload *pay = (ಶೂನ್ಯ*)fhdr + be32_ಗಮ್ಯ_cpu(fhdr->ಸ್ಥಾನಾಂತರ_2);
    ರಚನೆ cbfs_payload_segment *seg = pay->segments;
    ಗಾಗಿ (;;) {
        ಶೂನ್ಯ *src = (ಶೂನ್ಯ*)pay + be32_ಗಮ್ಯ_cpu(seg->ಸ್ಥಾನಾಂತರ_2);
        ಶೂನ್ಯ *dest = (ಶೂನ್ಯ*)(u32)be64_ಗಮ್ಯ_cpu(seg->ತುಂಬು_addr);
        u32 src_len = be32_ಗಮ್ಯ_cpu(seg->len);
        u32 dest_len = be32_ಗಮ್ಯ_cpu(seg->mem_len);
        ಆಯ್ಕೆ (seg->type) {
        ಸ್ಥಿತಿ PAYLOAD_SEGMENT_BSS:
            dprintf(3, "BSS segment %d@%p\n", dest_len, dest);
            memset(dest, 0, dest_len);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ PAYLOAD_SEGMENT_ನಮೂದು: {
            dprintf(1, "Calling addr %p\n", dest);
            ಶೂನ್ಯ (*func)(ಶೂನ್ಯ) = dest;
            func();
            ಹಿಂತಿರುಗಿ;
        }
        ಪೂರ್ವನಿಯೋಜಿತ:
            dprintf(3, "Segment %x %d@%p -> %d@%p\n"
                    , seg->type, src_len, src, dest_len, dest);
            ಆದರೆ (seg->compression == cpu_ಗಮ್ಯ_be32(CBFS_COMPRESS_NONE)) {
                ಆದರೆ (src_len > dest_len)
                    src_len = dest_len;
                ಸ್ಮರಣೆ_ನಕಲು(dest, src, src_len);
            } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (CONFIG_LZMA
                       && seg->compression == cpu_ಗಮ್ಯ_be32(CBFS_COMPRESS_LZMA)) {
                ಪೂರ್ಣಾಂಕ ret = ulzma(dest, dest_len, src, src_len);
                ಆದರೆ (ret < 0)
                    ಹಿಂತಿರುಗಿ;
                src_len = ret;
            } ಇಲ್ಲದಿದ್ದರೆ {
                dprintf(1, "No support for compression type %x\n"
                        , seg->compression);
                ಹಿಂತಿರುಗಿ;
            }
            ಆದರೆ (dest_len > src_len)
                memset(dest + src_len, 0, dest_len - src_len);
            ನಿಲ್ಲಿಸು;
        }
        seg++;
    }
}

// Register payloads in "img/" directory with boot system.
ಶೂನ್ಯ
cbfs_payload_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_COREBOOT_FLASH)
        ಹಿಂತಿರುಗಿ;
    ರಚನೆ romfile_s *file = ಖಾಲಿ;
    ಗಾಗಿ (;;) {
        file = romfile_findprefix("img/", file);
        ಆದರೆ (!file)
            ನಿಲ್ಲಿಸು;
        ರಚನೆ cbfs_romfile_s *cfile;
        cfile = container_ದ(file, ರಚನೆ cbfs_romfile_s, file);
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename = file->name;
        ಅಕ್ಷರ *desc = znprintf(MAXDESCSIZE, "Payload [%s]", &filename[4]);
        boot_ಕೂಡಿಸು_cbfs(cfile->fhdr, desc, bootprio_ಕಂಡುಹಿಡಿ_named_rom(filename, 0));
    }
}

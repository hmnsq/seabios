// Option rom scanning code.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "farptr.h" // FLATPTR_TO_SEG
#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_IVT
#ಅನ್ತರ್ಭಾವ "hw/pci.h" // pci_config_readl
#ಅನ್ತರ್ಭಾವ "hw/pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "hw/pci_ids.h" // PCI_CLASS_DISPLAY_VGA
#ಅನ್ತರ್ಭಾವ "hw/pci_regs.h" // PCI_ROM_ADDRESS
#ಅನ್ತರ್ಭಾವ "malloc.h" // rom_confirm
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_loadint
#ಅನ್ತರ್ಭಾವ "stacks.h" // farcall16big
#ಅನ್ತರ್ಭಾವ "std/optionrom.h" // struct rom_header
#ಅನ್ತರ್ಭಾವ "std/pnpbios.h" // PNP_SIGNATURE
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // get_pnp_offset
#ಅನ್ತರ್ಭಾವ "tcgbios.h" // tpm_*

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ EnforceChecksum, S3ResumeVga, ಸತತ_PCIroms;


/****************************************************************
 * Helper functions
 ****************************************************************/

// Execute a given option rom.
ಸ್ಥಿರ ಶೂನ್ಯ
__callrom(ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom, u16 ಸ್ಥಾನಾಂತರ_2, u16 bdf)
{
    u16 seg = FLATPTR_ಗಮ್ಯ_SEG(rom);
    dprintf(1, "Running option rom at %04x:%04x\n", seg, ಸ್ಥಾನಾಂತರ_2);

    ರಚನೆ bregs br;
    memset(&br, 0, ಗಾತ್ರ(br));
    br.flags = F_IF;
    br.ax = bdf;
    br.bx = 0xffff;
    br.dx = 0xffff;
    br.es = SEG_BIOS;
    br.di = get_pnp_ಸ್ಥಾನಾಂತರ();
    br.ಸಂಕೇತ_2 = SEGOFF(seg, ಸ್ಥಾನಾಂತರ_2);
    ಆರಂಭ_preempt();
    farcall16big(&br);
    ಮುಕ್ತಾಯ_preempt();
}

// Execute a given option rom at the standard entry vector.
ಶೂನ್ಯ
callrom(ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom, u16 bdf)
{
    __callrom(rom, OPTION_ROM_INITVECTOR, bdf);
}

// Execute a BCV option rom registered via add_bcv().
ಶೂನ್ಯ
call_bcv(u16 seg, u16 ip)
{
    __callrom(MAKE_FLATPTR(seg, 0), ip, 0);
}

// Verify that an option rom looks valid
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
is_valid_rom(ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom)
{
    dprintf(6, "Checking rom %p (sig %x size %d)\n"
            , rom, rom->signature, rom->ಗಾತ್ರ_2);
    ಆದರೆ (rom->signature != OPTION_ROM_SIGNATURE)
        ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ (! rom->ಗಾತ್ರ_2)
        ಹಿಂತಿರುಗಿ 0;
    u32 len = rom->ಗಾತ್ರ_2 * 512;
    u8 ಮೊತ್ತ = checksum(rom, len);
    ಆದರೆ (ಮೊತ್ತ != 0) {
        dprintf(1, "Found option rom with bad checksum: loc=%p len=%d sum=%x\n"
                , rom, len, ಮೊತ್ತ);
        ಆದರೆ (EnforceChecksum)
            ಹಿಂತಿರುಗಿ 0;
    }
    ಹಿಂತಿರುಗಿ 1;
}

// Check if a valid option rom has a pnp struct; return it if so.
ಸ್ಥಿರ ರಚನೆ pnp_ದತ್ತಾಂಶ *
get_pnp_rom(ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom)
{
    ರಚನೆ pnp_ದತ್ತಾಂಶ *pnp = (ಶೂನ್ಯ*)((u8*)rom + rom->pnpoffset);
    ಆದರೆ (pnp->signature != PNP_SIGNATURE)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಹಿಂತಿರುಗಿ pnp;
}

// Check for multiple pnp option rom headers.
ಸ್ಥಿರ ರಚನೆ pnp_ದತ್ತಾಂಶ *
get_pnp_ಮುಂದಿನ(ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom, ರಚನೆ pnp_ದತ್ತಾಂಶ *pnp)
{
    ಆದರೆ (! pnp->nextoffset)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    pnp = (ಶೂನ್ಯ*)((u8*)rom + pnp->nextoffset);
    ಆದರೆ (pnp->signature != PNP_SIGNATURE)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಹಿಂತಿರುಗಿ pnp;
}

// Check if a valid option rom has a pci struct; return it if so.
ಸ್ಥಿರ ರಚನೆ pci_ದತ್ತಾಂಶ *
get_pci_rom(ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom)
{
    ರಚನೆ pci_ದತ್ತಾಂಶ *pd = (ಶೂನ್ಯ*)((u32)rom + rom->pcioffset);
    ಆದರೆ (pd->signature != PCI_ROM_SIGNATURE)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಆದರೆ (rom->pcioffset & 3)
        dprintf(1, "WARNING! Found unaligned PCI rom (vd=%04x:%04x)\n"
                , pd->vendor, pd->device);
    ಹಿಂತಿರುಗಿ pd;
}

// Run rom init code and note rom size.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
init_optionrom(ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom, u16 bdf, ಪೂರ್ಣಾಂಕ isvga)
{
    ಆದರೆ (! is_valid_rom(rom))
        ಹಿಂತಿರುಗಿ -1;
    ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *newrom = rom_reserve(rom->ಗಾತ್ರ_2 * 512);
    ಆದರೆ (!newrom) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ -1;
    }
    ಆದರೆ (newrom != rom)
        memmove(newrom, rom, rom->ಗಾತ್ರ_2 * 512);

    tpm_option_rom(newrom, rom->ಗಾತ್ರ_2 * 512);

    ಆದರೆ (isvga || get_pnp_rom(newrom))
        // Only init vga and PnP roms here.
        callrom(newrom, bdf);

    ಹಿಂತಿರುಗಿ rom_confirm(newrom->ಗಾತ್ರ_2 * 512);
}

#ವ್ಯಾಖ್ಯಾನಿಸು RS_PCIROM (1LL<<33)

ಸ್ಥಿರ ಶೂನ್ಯ
ಗಣ_Rom_ಮೂಲ(u64 *sources, ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom, u64 ಮೂಲ)
{
    ಆದರೆ (sources)
        sources[((u32)rom - BUILD_ROM_ಆರಂಭ) / OPTION_ROM_ALIGN] = ಮೂಲ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
get_Rom_ಆದ್ಯತೆ(u64 *sources, ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom, ಪೂರ್ಣಾಂಕ instance)
{
    u64 ಮೂಲ = sources[((u32)rom - BUILD_ROM_ಆರಂಭ) / OPTION_ROM_ALIGN];
    ಆದರೆ (!ಮೂಲ)
        ಹಿಂತಿರುಗಿ -1;
    ಆದರೆ (ಮೂಲ & RS_PCIROM)
        ಹಿಂತಿರುಗಿ bootprio_ಕಂಡುಹಿಡಿ_pci_rom((ಶೂನ್ಯ*)(u32)ಮೂಲ, instance);
    ರಚನೆ romfile_s *file = (ಶೂನ್ಯ*)(u32)ಮೂಲ;
    ಹಿಂತಿರುಗಿ bootprio_ಕಂಡುಹಿಡಿ_named_rom(file->name, instance);
}


/****************************************************************
 * Roms in CBFS
 ****************************************************************/

ಸ್ಥಿರ ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *
deploy_romfile(ರಚನೆ romfile_s *file)
{
    u32 ಗಾತ್ರ_2 = file->ಗಾತ್ರ_2;
    ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom = rom_reserve(ಗಾತ್ರ_2);
    ಆದರೆ (!rom) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    ಪೂರ್ಣಾಂಕ ret = file->copy(file, rom, ಗಾತ್ರ_2);
    ಆದರೆ (ret <= 0)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಹಿಂತಿರುಗಿ rom;
}

// Run all roms in a given CBFS directory.
ಸ್ಥಿರ ಶೂನ್ಯ
ಸತತ_file_roms(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ, ಪೂರ್ಣಾಂಕ isvga, u64 *sources)
{
    ರಚನೆ romfile_s *file = ಖಾಲಿ;
    ಗಾಗಿ (;;) {
        file = romfile_findprefix(ಪೂರ್ವಪ್ರತ್ಯಯ, file);
        ಆದರೆ (!file)
            ನಿಲ್ಲಿಸು;
        ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom = deploy_romfile(file);
        ಆದರೆ (rom) {
            ಗಣ_Rom_ಮೂಲ(sources, rom, (u32)file);
            init_optionrom(rom, 0, isvga);
        }
    }
}


/****************************************************************
 * PCI roms
 ****************************************************************/

// Verify device is a vga device with legacy address decoding enabled.
ಪೂರ್ಣಾಂಕ
is_pci_vga(ರಚನೆ pci_device *pci)
{
    ಆದರೆ (pci->class != PCI_CLASS_DISPLAY_VGA)
        ಹಿಂತಿರುಗಿ 0;
    u16 cmd = pci_config_readw(pci->bdf, PCI_COMMAND);
    ಆದರೆ (!(cmd & PCI_COMMAND_IO && cmd & PCI_COMMAND_ಸ್ಮರಣೆ))
        ಹಿಂತಿರುಗಿ 0;
    ಆಗ (pci->ಪೋಷಕ) {
        pci = pci->ಪೋಷಕ;
        u32 ctrl = pci_config_readb(pci->bdf, PCI_BRIDGE_CONTROL);
        ಆದರೆ (!(ctrl & PCI_BRIDGE_CTL_VGA))
            ಹಿಂತಿರುಗಿ 0;
    }
    ಹಿಂತಿರುಗಿ 1;
}

// Copy a rom to its permanent location below 1MiB
ಸ್ಥಿರ ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *
copy_rom(ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom)
{
    u32 romsize = rom->ಗಾತ್ರ_2 * 512;
    ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *newrom = rom_reserve(romsize);
    ಆದರೆ (!newrom) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    dprintf(4, "Copying option rom (size %d) from %p to %p\n"
            , romsize, rom, newrom);
    iomemcpy(newrom, rom, romsize);
    ಹಿಂತಿರುಗಿ newrom;
}

// Map the option rom of a given PCI device.
ಸ್ಥಿರ ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *
map_pcirom(ರಚನೆ pci_device *pci)
{
    dprintf(6, "Attempting to map option rom on dev %pP\n", pci);

    ಆದರೆ ((pci->ಶೀರ್ಷಿಕೆ_type & 0x7f) != PCI_ಶೀರ್ಷಿಕೆ_TYPE_ಸಾಮಾನ್ಯ) {
        dprintf(6, "Skipping non-normal pci device (type=%x)\n"
                , pci->ಶೀರ್ಷಿಕೆ_type);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }

    u16 bdf = pci->bdf;
    u32 orig = pci_config_readl(bdf, PCI_ROM_ವಿಳಾಸ);
    pci_config_writel(bdf, PCI_ROM_ವಿಳಾಸ, ~PCI_ROM_ವಿಳಾಸ_ENABLE);
    u32 sz = pci_config_readl(bdf, PCI_ROM_ವಿಳಾಸ);

    dprintf(6, "Option rom sizing returned %x %x\n", orig, sz);
    orig &= ~PCI_ROM_ವಿಳಾಸ_ENABLE;
    ಆದರೆ (!sz || sz == 0xffffffff)
        ಹೋಗು fail;

    ಆದರೆ (orig == sz || (u32)(orig + 4*1024*1024) < 20*1024*1024) {
        // Don't try to map to a pci addresses at its max, in the last
        // 4MiB of ram, or the first 16MiB of ram.
        dprintf(6, "Preset rom address doesn't look valid\n");
        ಹೋಗು fail;
    }

    // Looks like a rom - enable it.
    pci_config_writel(bdf, PCI_ROM_ವಿಳಾಸ, orig | PCI_ROM_ವಿಳಾಸ_ENABLE);

    ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom = (ಶೂನ್ಯ*)orig;
    ಗಾಗಿ (;;) {
        dprintf(5, "Inspecting possible rom at %p (vd=%04x:%04x bdf=%pP)\n"
                , rom, pci->vendor, pci->device, pci);
        ಆದರೆ (rom->signature != OPTION_ROM_SIGNATURE) {
            dprintf(6, "No option rom signature (got %x)\n", rom->signature);
            ಹೋಗು fail;
        }
        ರಚನೆ pci_ದತ್ತಾಂಶ *pd = get_pci_rom(rom);
        ಆದರೆ (! pd) {
            dprintf(6, "No valid pci signature found\n");
            ಹೋಗು fail;
        }

        ಆದರೆ (pd->vendor == pci->vendor && pd->device == pci->device
            && pd->type == PCIROM_CODETYPE_X86)
            // A match
            ನಿಲ್ಲಿಸು;
        dprintf(6, "Didn't match dev/ven (got %04x:%04x) or type (got %d)\n"
                , pd->vendor, pd->device, pd->type);
        ಆದರೆ (pd->indicator & 0x80) {
            dprintf(6, "No more images left\n");
            ಹೋಗು fail;
        }
        rom = (ಶೂನ್ಯ*)((u32)rom + pd->ilen * 512);
    }

    rom = copy_rom(rom);
    pci_config_writel(bdf, PCI_ROM_ವಿಳಾಸ, orig);
    ಹಿಂತಿರುಗಿ rom;
fail:
    // Not valid - restore original and exit.
    pci_config_writel(bdf, PCI_ROM_ವಿಳಾಸ, orig);
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ boot_irq_captured(ಶೂನ್ಯ)
{
    ಹಿಂತಿರುಗಿ GET_IVT(0x19).segoff != FUNC16(ನಮೂದು_19_official).segoff;
}

ಸ್ಥಿರ ಶೂನ್ಯ boot_irq_restore(ಶೂನ್ಯ)
{
    ರಚನೆ segoff_s seabios;

    seabios = FUNC16(ನಮೂದು_19_official);
    ಗಣ_IVT(0x19, seabios);
}

// Attempt to map and initialize the option rom on a given PCI device.
ಸ್ಥಿರ ಶೂನ್ಯ
init_pcirom(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ isvga, u64 *sources)
{
    dprintf(4, "Attempting to init PCI bdf %pP (vd %04x:%04x)\n"
            , pci, pci->vendor, pci->device);

    ಅಕ್ಷರ fname[17];
    snprintf(fname, ಗಾತ್ರ(fname), "pci%04x,%04x.rom"
             , pci->vendor, pci->device);
    ರಚನೆ romfile_s *file = romfile_ಕಂಡುಹಿಡಿ(fname);
    ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom = ಖಾಲಿ;
    ಆದರೆ (file)
        rom = deploy_romfile(file);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಸತತ_PCIroms > 1 || (ಸತತ_PCIroms == 1 && isvga))
        rom = map_pcirom(pci);
    ಆದರೆ (! rom)
        // No ROM present.
        ಹಿಂತಿರುಗಿ;
    ಪೂರ್ಣಾಂಕ irq_was_captured = boot_irq_captured();
    ರಚನೆ pnp_ದತ್ತಾಂಶ *pnp = get_pnp_rom(rom);
    ಗಣ_Rom_ಮೂಲ(sources, rom, RS_PCIROM | (u32)pci);
    init_optionrom(rom, pci->bdf, isvga);
    ಆದರೆ (boot_irq_captured() && !irq_was_captured &&
        !file && !isvga && pnp) {
        // This PCI rom is misbehaving - recapture the boot irqs
        ಅಕ್ಷರ *desc = MAKE_FLATPTR(FLATPTR_ಗಮ್ಯ_SEG(rom), pnp->productname);
        dprintf(1, "PnP optionrom \"%s\" (bdf %pP) captured int19, restoring\n",
                desc, pci);
        boot_irq_restore();
    }
}


/****************************************************************
 * Non-VGA option rom init
 ****************************************************************/

ಶೂನ್ಯ
optionrom_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_OPTIONROMS)
        ಹಿಂತಿರುಗಿ;

    dprintf(1, "Scan for option roms\n");
    u64 sources[(BUILD_BIOS_ADDR - BUILD_ROM_ಆರಂಭ) / OPTION_ROM_ALIGN];
    memset(sources, 0, ಗಾತ್ರ(sources));
    u32 post_vga = rom_get_ಕೊನೆಯ();

    // Find and deploy PCI roms.
    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (pci->class == PCI_CLASS_DISPLAY_VGA ||
            pci->class == PCI_CLASS_DISPLAY_OTHER ||
            pci->have_driver)
            ಮುಂದುವರಿ;
        init_pcirom(pci, 0, sources);
    }

    // Find and deploy CBFS roms not associated with a device.
    ಸತತ_file_roms("genroms/", 0, sources);
    rom_reserve(0);

    // All option roms found and deployed - now build BEV/BCV vectors.

    u32 pos = post_vga;
    ಆಗ (pos < rom_get_ಕೊನೆಯ()) {
        ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom = (ಶೂನ್ಯ*)pos;
        ಆದರೆ (! is_valid_rom(rom)) {
            pos += OPTION_ROM_ALIGN;
            ಮುಂದುವರಿ;
        }
        pos += ALIGN(rom->ಗಾತ್ರ_2 * 512, OPTION_ROM_ALIGN);
        ರಚನೆ pnp_ದತ್ತಾಂಶ *pnp = get_pnp_rom(rom);
        ಆದರೆ (! pnp) {
            // Legacy rom.
            boot_ಕೂಡಿಸು_bcv(FLATPTR_ಗಮ್ಯ_SEG(rom), OPTION_ROM_INITVECTOR, 0
                         , get_Rom_ಆದ್ಯತೆ(sources, rom, 0));
            ಮುಂದುವರಿ;
        }
        // PnP rom - check for BEV and BCV boot capabilities.
        ಪೂರ್ಣಾಂಕ instance = 0;
        ಆಗ (pnp) {
            ಆದರೆ (pnp->bev)
                boot_ಕೂಡಿಸು_bev(FLATPTR_ಗಮ್ಯ_SEG(rom), pnp->bev, pnp->productname
                             , get_Rom_ಆದ್ಯತೆ(sources, rom, instance++));
            ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pnp->bcv)
                boot_ಕೂಡಿಸು_bcv(FLATPTR_ಗಮ್ಯ_SEG(rom), pnp->bcv, pnp->productname
                             , get_Rom_ಆದ್ಯತೆ(sources, rom, instance++));
            ಇಲ್ಲದಿದ್ದರೆ
                ನಿಲ್ಲಿಸು;
            pnp = get_pnp_ಮುಂದಿನ(rom, pnp);
        }
    }
}


/****************************************************************
 * VGA init
 ****************************************************************/

ಪೂರ್ಣಾಂಕ ScreenAndDebug;
ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *VgaROM;

ಸ್ಥಿರ ಶೂನ್ಯ try_ಸಿದ್ಧತೆ_display_other(ಶೂನ್ಯ)
{
    ರಚನೆ pci_device *pci;

    dprintf(1, "No VGA found, scan for other display\n");

    foreachpci(pci) {
        ಆದರೆ (pci->class != PCI_CLASS_DISPLAY_OTHER)
            ಮುಂದುವರಿ;
        ರಚನೆ rom_ಶೀರ್ಷಿಕೆ *rom = map_pcirom(pci);
        ಆದರೆ (!rom)
            ಮುಂದುವರಿ;
        dprintf(1, "Other display found at %pP\n", pci);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_ಸ್ಮರಣೆ);
        init_optionrom(rom, pci->bdf, 1);
        ಹಿಂತಿರುಗಿ;
    }
}

// Call into vga code to turn on console.
ಶೂನ್ಯ
vgarom_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಂಕ have_vga = 0;

    ಆದರೆ (! CONFIG_OPTIONROMS)
        ಹಿಂತಿರುಗಿ;

    dprintf(1, "Scan for VGA option rom\n");

    // Load some config settings that impact VGA.
    EnforceChecksum = romfile_loadint("etc/optionroms-checksum", 1);
    S3ResumeVga = romfile_loadint("etc/s3-resume-vga-init", CONFIG_QEMU);
    ಸತತ_PCIroms = romfile_loadint("etc/pci-optionrom-exec", 2);
    ScreenAndDebug = romfile_loadint("etc/screen-and-debug", 1);

    // Clear option rom memory
    memset((ಶೂನ್ಯ*)BUILD_ROM_ಆರಂಭ, 0, rom_get_max() - BUILD_ROM_ಆರಂಭ);

    // Find and deploy PCI VGA rom.
    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (!is_pci_vga(pci))
            ಮುಂದುವರಿ;
        vgahook_ಸಿದ್ಧತೆ(pci);
        init_pcirom(pci, 1, ಖಾಲಿ);
        have_vga = 1;
        ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ (!have_vga)
        try_ಸಿದ್ಧತೆ_display_other();

    // Find and deploy CBFS vga-style roms not associated with a device.
    ಸತತ_file_roms("vgaroms/", 1, ಖಾಲಿ);
    rom_reserve(0);

    ಆದರೆ (rom_get_ಕೊನೆಯ() != BUILD_ROM_ಆರಂಭ)
        // VGA rom found
        VgaROM = (ಶೂನ್ಯ*)BUILD_ROM_ಆರಂಭ;
}

ಶೂನ್ಯ
s3_resume_vga(ಶೂನ್ಯ)
{
    ಆದರೆ (!S3ResumeVga)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (!VgaROM || ! is_valid_rom(VgaROM))
        ಹಿಂತಿರುಗಿ;
    callrom(VgaROM, 0);
}

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

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ EnforceChecksum, S3ResumeVga, RunPCIroms;


/****************************************************************
 * Helper functions
 ****************************************************************/

// Execute a given option rom.
ಸ್ಥಿರ ಶೂನ್ಯ
__callrom(ಸಂರಚನಾ rom_header *rom, u16 offset, u16 bdf)
{
    u16 seg = FLATPTR_TO_SEG(rom);
    dprintf(1, "Running option rom at %04x:%04x\n", seg, offset);

    ಸಂರಚನಾ bregs br;
    memset(&br, 0, ಪರಿಮಾಣ(br));
    br.flags = F_IF;
    br.ax = bdf;
    br.bx = 0xffff;
    br.dx = 0xffff;
    br.es = SEG_BIOS;
    br.di = get_pnp_offset();
    br.code = SEGOFF(seg, offset);
    start_preempt();
    farcall16big(&br);
    finish_preempt();
}

// Execute a given option rom at the standard entry vector.
ಶೂನ್ಯ
callrom(ಸಂರಚನಾ rom_header *rom, u16 bdf)
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
is_valid_rom(ಸಂರಚನಾ rom_header *rom)
{
    dprintf(6, "Checking rom %p (sig %x size %d)\n"
            , rom, rom->signature, rom->size);
    ಯದಿ (rom->signature != OPTION_ROM_SIGNATURE)
        ಪ್ರತ್ಯಾಗಮ 0;
    ಯದಿ (! rom->size)
        ಪ್ರತ್ಯಾಗಮ 0;
    u32 len = rom->size * 512;
    u8 sum = checksum(rom, len);
    ಯದಿ (sum != 0) {
        dprintf(1, "Found option rom with bad checksum: loc=%p len=%d sum=%x\n"
                , rom, len, sum);
        ಯದಿ (EnforceChecksum)
            ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಪ್ರತ್ಯಾಗಮ 1;
}

// Check if a valid option rom has a pnp struct; return it if so.
ಸ್ಥಿರ ಸಂರಚನಾ pnp_data *
get_pnp_rom(ಸಂರಚನಾ rom_header *rom)
{
    ಸಂರಚನಾ pnp_data *pnp = (ಶೂನ್ಯ*)((u8*)rom + rom->pnpoffset);
    ಯದಿ (pnp->signature != PNP_SIGNATURE)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    ಪ್ರತ್ಯಾಗಮ pnp;
}

// Check for multiple pnp option rom headers.
ಸ್ಥಿರ ಸಂರಚನಾ pnp_data *
get_pnp_next(ಸಂರಚನಾ rom_header *rom, ಸಂರಚನಾ pnp_data *pnp)
{
    ಯದಿ (! pnp->nextoffset)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    pnp = (ಶೂನ್ಯ*)((u8*)rom + pnp->nextoffset);
    ಯದಿ (pnp->signature != PNP_SIGNATURE)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    ಪ್ರತ್ಯಾಗಮ pnp;
}

// Check if a valid option rom has a pci struct; return it if so.
ಸ್ಥಿರ ಸಂರಚನಾ pci_data *
get_pci_rom(ಸಂರಚನಾ rom_header *rom)
{
    ಸಂರಚನಾ pci_data *pd = (ಶೂನ್ಯ*)((u32)rom + rom->pcioffset);
    ಯದಿ (pd->signature != PCI_ROM_SIGNATURE)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    ಯದಿ (rom->pcioffset & 3)
        dprintf(1, "WARNING! Found unaligned PCI rom (vd=%04x:%04x)\n"
                , pd->vendor, pd->device);
    ಪ್ರತ್ಯಾಗಮ pd;
}

// Run rom init code and note rom size.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
init_optionrom(ಸಂರಚನಾ rom_header *rom, u16 bdf, ಪೂರ್ಣಾಙ್ಕ isvga)
{
    ಯದಿ (! is_valid_rom(rom))
        ಪ್ರತ್ಯಾಗಮ -1;
    ಸಂರಚನಾ rom_header *newrom = rom_reserve(rom->size * 512);
    ಯದಿ (!newrom) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    ಯದಿ (newrom != rom)
        memmove(newrom, rom, rom->size * 512);

    tpm_option_rom(newrom, rom->size * 512);

    ಯದಿ (isvga || get_pnp_rom(newrom))
        // Only init vga and PnP roms here.
        callrom(newrom, bdf);

    ಪ್ರತ್ಯಾಗಮ rom_confirm(newrom->size * 512);
}

#define RS_PCIROM (1LL<<33)

ಸ್ಥಿರ ಶೂನ್ಯ
setRomSource(u64 *sources, ಸಂರಚನಾ rom_header *rom, u64 source)
{
    ಯದಿ (sources)
        sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN] = source;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
getRomPriority(u64 *sources, ಸಂರಚನಾ rom_header *rom, ಪೂರ್ಣಾಙ್ಕ instance)
{
    u64 source = sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    ಯದಿ (!source)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಯದಿ (source & RS_PCIROM)
        ಪ್ರತ್ಯಾಗಮ bootprio_find_pci_rom((ಶೂನ್ಯ*)(u32)source, instance);
    ಸಂರಚನಾ romfile_s *file = (ಶೂನ್ಯ*)(u32)source;
    ಪ್ರತ್ಯಾಗಮ bootprio_find_named_rom(file->name, instance);
}


/****************************************************************
 * Roms in CBFS
 ****************************************************************/

ಸ್ಥಿರ ಸಂರಚನಾ rom_header *
deploy_romfile(ಸಂರಚನಾ romfile_s *file)
{
    u32 size = file->size;
    ಸಂರಚನಾ rom_header *rom = rom_reserve(size);
    ಯದಿ (!rom) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    ಪೂರ್ಣಾಙ್ಕ ret = file->copy(file, rom, size);
    ಯದಿ (ret <= 0)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    ಪ್ರತ್ಯಾಗಮ rom;
}

// Run all roms in a given CBFS directory.
ಸ್ಥಿರ ಶೂನ್ಯ
run_file_roms(ಧ್ರುವ ವರ್ಣ *prefix, ಪೂರ್ಣಾಙ್ಕ isvga, u64 *sources)
{
    ಸಂರಚನಾ romfile_s *file = ರಿಕ್ತ;
    ಪ್ರತಿ (;;) {
        file = romfile_findprefix(prefix, file);
        ಯದಿ (!file)
            ಭಙ್ಗ;
        ಸಂರಚನಾ rom_header *rom = deploy_romfile(file);
        ಯದಿ (rom) {
            setRomSource(sources, rom, (u32)file);
            init_optionrom(rom, 0, isvga);
        }
    }
}


/****************************************************************
 * PCI roms
 ****************************************************************/

// Verify device is a vga device with legacy address decoding enabled.
ಪೂರ್ಣಾಙ್ಕ
is_pci_vga(ಸಂರಚನಾ pci_device *pci)
{
    ಯದಿ (pci->class != PCI_CLASS_DISPLAY_VGA)
        ಪ್ರತ್ಯಾಗಮ 0;
    u16 cmd = pci_config_readw(pci->bdf, PCI_COMMAND);
    ಯದಿ (!(cmd & PCI_COMMAND_IO && cmd & PCI_COMMAND_MEMORY))
        ಪ್ರತ್ಯಾಗಮ 0;
    ಯಾವತ್ (pci->parent) {
        pci = pci->parent;
        u32 ctrl = pci_config_readb(pci->bdf, PCI_BRIDGE_CONTROL);
        ಯದಿ (!(ctrl & PCI_BRIDGE_CTL_VGA))
            ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಪ್ರತ್ಯಾಗಮ 1;
}

// Copy a rom to its permanent location below 1MiB
ಸ್ಥಿರ ಸಂರಚನಾ rom_header *
copy_rom(ಸಂರಚನಾ rom_header *rom)
{
    u32 romsize = rom->size * 512;
    ಸಂರಚನಾ rom_header *newrom = rom_reserve(romsize);
    ಯದಿ (!newrom) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    dprintf(4, "Copying option rom (size %d) from %p to %p\n"
            , romsize, rom, newrom);
    iomemcpy(newrom, rom, romsize);
    ಪ್ರತ್ಯಾಗಮ newrom;
}

// Map the option rom of a given PCI device.
ಸ್ಥಿರ ಸಂರಚನಾ rom_header *
map_pcirom(ಸಂರಚನಾ pci_device *pci)
{
    dprintf(6, "Attempting to map option rom on dev %pP\n", pci);

    ಯದಿ ((pci->header_type & 0x7f) != PCI_HEADER_TYPE_NORMAL) {
        dprintf(6, "Skipping non-normal pci device (type=%x)\n"
                , pci->header_type);
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }

    u16 bdf = pci->bdf;
    u32 orig = pci_config_readl(bdf, PCI_ROM_ADDRESS);
    pci_config_writel(bdf, PCI_ROM_ADDRESS, ~PCI_ROM_ADDRESS_ENABLE);
    u32 sz = pci_config_readl(bdf, PCI_ROM_ADDRESS);

    dprintf(6, "Option rom sizing returned %x %x\n", orig, sz);
    orig &= ~PCI_ROM_ADDRESS_ENABLE;
    ಯದಿ (!sz || sz == 0xffffffff)
        ಗಚ್ಛ fail;

    ಯದಿ (orig == sz || (u32)(orig + 4*1024*1024) < 20*1024*1024) {
        // Don't try to map to a pci addresses at its max, in the last
        // 4MiB of ram, or the first 16MiB of ram.
        dprintf(6, "Preset rom address doesn't look valid\n");
        ಗಚ್ಛ fail;
    }

    // Looks like a rom - enable it.
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig | PCI_ROM_ADDRESS_ENABLE);

    ಸಂರಚನಾ rom_header *rom = (ಶೂನ್ಯ*)orig;
    ಪ್ರತಿ (;;) {
        dprintf(5, "Inspecting possible rom at %p (vd=%04x:%04x bdf=%pP)\n"
                , rom, pci->vendor, pci->device, pci);
        ಯದಿ (rom->signature != OPTION_ROM_SIGNATURE) {
            dprintf(6, "No option rom signature (got %x)\n", rom->signature);
            ಗಚ್ಛ fail;
        }
        ಸಂರಚನಾ pci_data *pd = get_pci_rom(rom);
        ಯದಿ (! pd) {
            dprintf(6, "No valid pci signature found\n");
            ಗಚ್ಛ fail;
        }

        ಯದಿ (pd->vendor == pci->vendor && pd->device == pci->device
            && pd->type == PCIROM_CODETYPE_X86)
            // A match
            ಭಙ್ಗ;
        dprintf(6, "Didn't match dev/ven (got %04x:%04x) or type (got %d)\n"
                , pd->vendor, pd->device, pd->type);
        ಯದಿ (pd->indicator & 0x80) {
            dprintf(6, "No more images left\n");
            ಗಚ್ಛ fail;
        }
        rom = (ಶೂನ್ಯ*)((u32)rom + pd->ilen * 512);
    }

    rom = copy_rom(rom);
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig);
    ಪ್ರತ್ಯಾಗಮ rom;
fail:
    // Not valid - restore original and exit.
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig);
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ boot_irq_captured(ಶೂನ್ಯ)
{
    ಪ್ರತ್ಯಾಗಮ GET_IVT(0x19).segoff != FUNC16(entry_19_official).segoff;
}

ಸ್ಥಿರ ಶೂನ್ಯ boot_irq_restore(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ segoff_s seabios;

    seabios = FUNC16(entry_19_official);
    SET_IVT(0x19, seabios);
}

// Attempt to map and initialize the option rom on a given PCI device.
ಸ್ಥಿರ ಶೂನ್ಯ
init_pcirom(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ isvga, u64 *sources)
{
    dprintf(4, "Attempting to init PCI bdf %pP (vd %04x:%04x)\n"
            , pci, pci->vendor, pci->device);

    ವರ್ಣ fname[17];
    snprintf(fname, ಪರಿಮಾಣ(fname), "pci%04x,%04x.rom"
             , pci->vendor, pci->device);
    ಸಂರಚನಾ romfile_s *file = romfile_find(fname);
    ಸಂರಚನಾ rom_header *rom = ರಿಕ್ತ;
    ಯದಿ (file)
        rom = deploy_romfile(file);
    ಅನ್ಯಥಾ ಯದಿ (RunPCIroms > 1 || (RunPCIroms == 1 && isvga))
        rom = map_pcirom(pci);
    ಯದಿ (! rom)
        // No ROM present.
        ಪ್ರತ್ಯಾಗಮ;
    ಪೂರ್ಣಾಙ್ಕ irq_was_captured = boot_irq_captured();
    ಸಂರಚನಾ pnp_data *pnp = get_pnp_rom(rom);
    setRomSource(sources, rom, RS_PCIROM | (u32)pci);
    init_optionrom(rom, pci->bdf, isvga);
    ಯದಿ (boot_irq_captured() && !irq_was_captured &&
        !file && !isvga && pnp) {
        // This PCI rom is misbehaving - recapture the boot irqs
        ವರ್ಣ *desc = MAKE_FLATPTR(FLATPTR_TO_SEG(rom), pnp->productname);
        dprintf(1, "PnP optionrom \"%s\" (bdf %pP) captured int19, restoring\n",
                desc, pci);
        boot_irq_restore();
    }
}


/****************************************************************
 * Non-VGA option rom init
 ****************************************************************/

ಶೂನ್ಯ
optionrom_setup(ಶೂನ್ಯ)
{
    ಯದಿ (! CONFIG_OPTIONROMS)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(1, "Scan for option roms\n");
    u64 sources[(BUILD_BIOS_ADDR - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    memset(sources, 0, ಪರಿಮಾಣ(sources));
    u32 post_vga = rom_get_last();

    // Find and deploy PCI roms.
    ಸಂರಚನಾ pci_device *pci;
    foreachpci(pci) {
        ಯದಿ (pci->class == PCI_CLASS_DISPLAY_VGA ||
            pci->class == PCI_CLASS_DISPLAY_OTHER ||
            pci->have_driver)
            ಅನುವರ್ತ;
        init_pcirom(pci, 0, sources);
    }

    // Find and deploy CBFS roms not associated with a device.
    run_file_roms("genroms/", 0, sources);
    rom_reserve(0);

    // All option roms found and deployed - now build BEV/BCV vectors.

    u32 pos = post_vga;
    ಯಾವತ್ (pos < rom_get_last()) {
        ಸಂರಚನಾ rom_header *rom = (ಶೂನ್ಯ*)pos;
        ಯದಿ (! is_valid_rom(rom)) {
            pos += OPTION_ROM_ALIGN;
            ಅನುವರ್ತ;
        }
        pos += ALIGN(rom->size * 512, OPTION_ROM_ALIGN);
        ಸಂರಚನಾ pnp_data *pnp = get_pnp_rom(rom);
        ಯದಿ (! pnp) {
            // Legacy rom.
            boot_add_bcv(FLATPTR_TO_SEG(rom), OPTION_ROM_INITVECTOR, 0
                         , getRomPriority(sources, rom, 0));
            ಅನುವರ್ತ;
        }
        // PnP rom - check for BEV and BCV boot capabilities.
        ಪೂರ್ಣಾಙ್ಕ instance = 0;
        ಯಾವತ್ (pnp) {
            ಯದಿ (pnp->bev)
                boot_add_bev(FLATPTR_TO_SEG(rom), pnp->bev, pnp->productname
                             , getRomPriority(sources, rom, instance++));
            ಅನ್ಯಥಾ ಯದಿ (pnp->bcv)
                boot_add_bcv(FLATPTR_TO_SEG(rom), pnp->bcv, pnp->productname
                             , getRomPriority(sources, rom, instance++));
            ಅನ್ಯಥಾ
                ಭಙ್ಗ;
            pnp = get_pnp_next(rom, pnp);
        }
    }
}


/****************************************************************
 * VGA init
 ****************************************************************/

ಪೂರ್ಣಾಙ್ಕ ScreenAndDebug;
ಸಂರಚನಾ rom_header *VgaROM;

ಸ್ಥಿರ ಶೂನ್ಯ try_setup_display_other(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ pci_device *pci;

    dprintf(1, "No VGA found, scan for other display\n");

    foreachpci(pci) {
        ಯದಿ (pci->class != PCI_CLASS_DISPLAY_OTHER)
            ಅನುವರ್ತ;
        ಸಂರಚನಾ rom_header *rom = map_pcirom(pci);
        ಯದಿ (!rom)
            ಅನುವರ್ತ;
        dprintf(1, "Other display found at %pP\n", pci);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_MEMORY);
        init_optionrom(rom, pci->bdf, 1);
        ಪ್ರತ್ಯಾಗಮ;
    }
}

// Call into vga code to turn on console.
ಶೂನ್ಯ
vgarom_setup(ಶೂನ್ಯ)
{
    ಪೂರ್ಣಾಙ್ಕ have_vga = 0;

    ಯದಿ (! CONFIG_OPTIONROMS)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(1, "Scan for VGA option rom\n");

    // Load some config settings that impact VGA.
    EnforceChecksum = romfile_loadint("etc/optionroms-checksum", 1);
    S3ResumeVga = romfile_loadint("etc/s3-resume-vga-init", CONFIG_QEMU);
    RunPCIroms = romfile_loadint("etc/pci-optionrom-exec", 2);
    ScreenAndDebug = romfile_loadint("etc/screen-and-debug", 1);

    // Clear option rom memory
    memset((ಶೂನ್ಯ*)BUILD_ROM_START, 0, rom_get_max() - BUILD_ROM_START);

    // Find and deploy PCI VGA rom.
    ಸಂರಚನಾ pci_device *pci;
    foreachpci(pci) {
        ಯದಿ (!is_pci_vga(pci))
            ಅನುವರ್ತ;
        vgahook_setup(pci);
        init_pcirom(pci, 1, ರಿಕ್ತ);
        have_vga = 1;
        ಭಙ್ಗ;
    }
    ಯದಿ (!have_vga)
        try_setup_display_other();

    // Find and deploy CBFS vga-style roms not associated with a device.
    run_file_roms("vgaroms/", 1, ರಿಕ್ತ);
    rom_reserve(0);

    ಯದಿ (rom_get_last() != BUILD_ROM_START)
        // VGA rom found
        VgaROM = (ಶೂನ್ಯ*)BUILD_ROM_START;
}

ಶೂನ್ಯ
s3_resume_vga(ಶೂನ್ಯ)
{
    ಯದಿ (!S3ResumeVga)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (!VgaROM || ! is_valid_rom(VgaROM))
        ಪ್ರತ್ಯಾಗಮ;
    callrom(VgaROM, 0);
}

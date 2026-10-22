// Option rom scanning code.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "bregs.h" // struct bregs
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "farptr.h" // FLATPTR_TO_SEG
#অন্তর্ভাব "biosvar.h" // GET_IVT
#অন্তর্ভাব "hw/pci.h" // pci_config_readl
#অন্তর্ভাব "hw/pcidevice.h" // foreachpci
#অন্তর্ভাব "hw/pci_ids.h" // PCI_CLASS_DISPLAY_VGA
#অন্তর্ভাব "hw/pci_regs.h" // PCI_ROM_ADDRESS
#অন্তর্ভাব "malloc.h" // rom_confirm
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "romfile.h" // romfile_loadint
#অন্তর্ভাব "stacks.h" // farcall16big
#অন্তর্ভাব "std/optionrom.h" // struct rom_header
#অন্তর্ভাব "std/pnpbios.h" // PNP_SIGNATURE
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // get_pnp_offset
#অন্তর্ভাব "tcgbios.h" // tpm_*

স্থির পূর্ণাঙ্ক EnforceChecksum, S3ResumeVga, RunPCIroms;


/****************************************************************
 * Helper functions
 ****************************************************************/

// Execute a given option rom.
স্থির শূন্য
__callrom(সংরচনা rom_header *rom, u16 offset, u16 bdf)
{
    u16 seg = FLATPTR_TO_SEG(rom);
    dprintf(1, "Running option rom at %04x:%04x\n", seg, offset);

    সংরচনা bregs br;
    memset(&br, 0, পরিমাণ(br));
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
শূন্য
callrom(সংরচনা rom_header *rom, u16 bdf)
{
    __callrom(rom, OPTION_ROM_INITVECTOR, bdf);
}

// Execute a BCV option rom registered via add_bcv().
শূন্য
call_bcv(u16 seg, u16 ip)
{
    __callrom(MAKE_FLATPTR(seg, 0), ip, 0);
}

// Verify that an option rom looks valid
স্থির পূর্ণাঙ্ক
is_valid_rom(সংরচনা rom_header *rom)
{
    dprintf(6, "Checking rom %p (sig %x size %d)\n"
            , rom, rom->signature, rom->size);
    যদি (rom->signature != OPTION_ROM_SIGNATURE)
        প্রত্যাগম 0;
    যদি (! rom->size)
        প্রত্যাগম 0;
    u32 len = rom->size * 512;
    u8 sum = checksum(rom, len);
    যদি (sum != 0) {
        dprintf(1, "Found option rom with bad checksum: loc=%p len=%d sum=%x\n"
                , rom, len, sum);
        যদি (EnforceChecksum)
            প্রত্যাগম 0;
    }
    প্রত্যাগম 1;
}

// Check if a valid option rom has a pnp struct; return it if so.
স্থির সংরচনা pnp_data *
get_pnp_rom(সংরচনা rom_header *rom)
{
    সংরচনা pnp_data *pnp = (শূন্য*)((u8*)rom + rom->pnpoffset);
    যদি (pnp->signature != PNP_SIGNATURE)
        প্রত্যাগম রিক্ত;
    প্রত্যাগম pnp;
}

// Check for multiple pnp option rom headers.
স্থির সংরচনা pnp_data *
get_pnp_next(সংরচনা rom_header *rom, সংরচনা pnp_data *pnp)
{
    যদি (! pnp->nextoffset)
        প্রত্যাগম রিক্ত;
    pnp = (শূন্য*)((u8*)rom + pnp->nextoffset);
    যদি (pnp->signature != PNP_SIGNATURE)
        প্রত্যাগম রিক্ত;
    প্রত্যাগম pnp;
}

// Check if a valid option rom has a pci struct; return it if so.
স্থির সংরচনা pci_data *
get_pci_rom(সংরচনা rom_header *rom)
{
    সংরচনা pci_data *pd = (শূন্য*)((u32)rom + rom->pcioffset);
    যদি (pd->signature != PCI_ROM_SIGNATURE)
        প্রত্যাগম রিক্ত;
    যদি (rom->pcioffset & 3)
        dprintf(1, "WARNING! Found unaligned PCI rom (vd=%04x:%04x)\n"
                , pd->vendor, pd->device);
    প্রত্যাগম pd;
}

// Run rom init code and note rom size.
স্থির পূর্ণাঙ্ক
init_optionrom(সংরচনা rom_header *rom, u16 bdf, পূর্ণাঙ্ক isvga)
{
    যদি (! is_valid_rom(rom))
        প্রত্যাগম -1;
    সংরচনা rom_header *newrom = rom_reserve(rom->size * 512);
    যদি (!newrom) {
        warn_noalloc();
        প্রত্যাগম -1;
    }
    যদি (newrom != rom)
        memmove(newrom, rom, rom->size * 512);

    tpm_option_rom(newrom, rom->size * 512);

    যদি (isvga || get_pnp_rom(newrom))
        // Only init vga and PnP roms here.
        callrom(newrom, bdf);

    প্রত্যাগম rom_confirm(newrom->size * 512);
}

#define RS_PCIROM (1LL<<33)

স্থির শূন্য
setRomSource(u64 *sources, সংরচনা rom_header *rom, u64 source)
{
    যদি (sources)
        sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN] = source;
}

স্থির পূর্ণাঙ্ক
getRomPriority(u64 *sources, সংরচনা rom_header *rom, পূর্ণাঙ্ক instance)
{
    u64 source = sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    যদি (!source)
        প্রত্যাগম -1;
    যদি (source & RS_PCIROM)
        প্রত্যাগম bootprio_find_pci_rom((শূন্য*)(u32)source, instance);
    সংরচনা romfile_s *file = (শূন্য*)(u32)source;
    প্রত্যাগম bootprio_find_named_rom(file->name, instance);
}


/****************************************************************
 * Roms in CBFS
 ****************************************************************/

স্থির সংরচনা rom_header *
deploy_romfile(সংরচনা romfile_s *file)
{
    u32 size = file->size;
    সংরচনা rom_header *rom = rom_reserve(size);
    যদি (!rom) {
        warn_noalloc();
        প্রত্যাগম রিক্ত;
    }
    পূর্ণাঙ্ক ret = file->copy(file, rom, size);
    যদি (ret <= 0)
        প্রত্যাগম রিক্ত;
    প্রত্যাগম rom;
}

// Run all roms in a given CBFS directory.
স্থির শূন্য
run_file_roms(ধ্রুব বর্ণ *prefix, পূর্ণাঙ্ক isvga, u64 *sources)
{
    সংরচনা romfile_s *file = রিক্ত;
    প্রতি (;;) {
        file = romfile_findprefix(prefix, file);
        যদি (!file)
            ভঙ্গ;
        সংরচনা rom_header *rom = deploy_romfile(file);
        যদি (rom) {
            setRomSource(sources, rom, (u32)file);
            init_optionrom(rom, 0, isvga);
        }
    }
}


/****************************************************************
 * PCI roms
 ****************************************************************/

// Verify device is a vga device with legacy address decoding enabled.
পূর্ণাঙ্ক
is_pci_vga(সংরচনা pci_device *pci)
{
    যদি (pci->class != PCI_CLASS_DISPLAY_VGA)
        প্রত্যাগম 0;
    u16 cmd = pci_config_readw(pci->bdf, PCI_COMMAND);
    যদি (!(cmd & PCI_COMMAND_IO && cmd & PCI_COMMAND_MEMORY))
        প্রত্যাগম 0;
    যাবত্ (pci->parent) {
        pci = pci->parent;
        u32 ctrl = pci_config_readb(pci->bdf, PCI_BRIDGE_CONTROL);
        যদি (!(ctrl & PCI_BRIDGE_CTL_VGA))
            প্রত্যাগম 0;
    }
    প্রত্যাগম 1;
}

// Copy a rom to its permanent location below 1MiB
স্থির সংরচনা rom_header *
copy_rom(সংরচনা rom_header *rom)
{
    u32 romsize = rom->size * 512;
    সংরচনা rom_header *newrom = rom_reserve(romsize);
    যদি (!newrom) {
        warn_noalloc();
        প্রত্যাগম রিক্ত;
    }
    dprintf(4, "Copying option rom (size %d) from %p to %p\n"
            , romsize, rom, newrom);
    iomemcpy(newrom, rom, romsize);
    প্রত্যাগম newrom;
}

// Map the option rom of a given PCI device.
স্থির সংরচনা rom_header *
map_pcirom(সংরচনা pci_device *pci)
{
    dprintf(6, "Attempting to map option rom on dev %pP\n", pci);

    যদি ((pci->header_type & 0x7f) != PCI_HEADER_TYPE_NORMAL) {
        dprintf(6, "Skipping non-normal pci device (type=%x)\n"
                , pci->header_type);
        প্রত্যাগম রিক্ত;
    }

    u16 bdf = pci->bdf;
    u32 orig = pci_config_readl(bdf, PCI_ROM_ADDRESS);
    pci_config_writel(bdf, PCI_ROM_ADDRESS, ~PCI_ROM_ADDRESS_ENABLE);
    u32 sz = pci_config_readl(bdf, PCI_ROM_ADDRESS);

    dprintf(6, "Option rom sizing returned %x %x\n", orig, sz);
    orig &= ~PCI_ROM_ADDRESS_ENABLE;
    যদি (!sz || sz == 0xffffffff)
        গচ্ছ fail;

    যদি (orig == sz || (u32)(orig + 4*1024*1024) < 20*1024*1024) {
        // Don't try to map to a pci addresses at its max, in the last
        // 4MiB of ram, or the first 16MiB of ram.
        dprintf(6, "Preset rom address doesn't look valid\n");
        গচ্ছ fail;
    }

    // Looks like a rom - enable it.
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig | PCI_ROM_ADDRESS_ENABLE);

    সংরচনা rom_header *rom = (শূন্য*)orig;
    প্রতি (;;) {
        dprintf(5, "Inspecting possible rom at %p (vd=%04x:%04x bdf=%pP)\n"
                , rom, pci->vendor, pci->device, pci);
        যদি (rom->signature != OPTION_ROM_SIGNATURE) {
            dprintf(6, "No option rom signature (got %x)\n", rom->signature);
            গচ্ছ fail;
        }
        সংরচনা pci_data *pd = get_pci_rom(rom);
        যদি (! pd) {
            dprintf(6, "No valid pci signature found\n");
            গচ্ছ fail;
        }

        যদি (pd->vendor == pci->vendor && pd->device == pci->device
            && pd->type == PCIROM_CODETYPE_X86)
            // A match
            ভঙ্গ;
        dprintf(6, "Didn't match dev/ven (got %04x:%04x) or type (got %d)\n"
                , pd->vendor, pd->device, pd->type);
        যদি (pd->indicator & 0x80) {
            dprintf(6, "No more images left\n");
            গচ্ছ fail;
        }
        rom = (শূন্য*)((u32)rom + pd->ilen * 512);
    }

    rom = copy_rom(rom);
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig);
    প্রত্যাগম rom;
fail:
    // Not valid - restore original and exit.
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig);
    প্রত্যাগম রিক্ত;
}

স্থির পূর্ণাঙ্ক boot_irq_captured(শূন্য)
{
    প্রত্যাগম GET_IVT(0x19).segoff != FUNC16(entry_19_official).segoff;
}

স্থির শূন্য boot_irq_restore(শূন্য)
{
    সংরচনা segoff_s seabios;

    seabios = FUNC16(entry_19_official);
    SET_IVT(0x19, seabios);
}

// Attempt to map and initialize the option rom on a given PCI device.
স্থির শূন্য
init_pcirom(সংরচনা pci_device *pci, পূর্ণাঙ্ক isvga, u64 *sources)
{
    dprintf(4, "Attempting to init PCI bdf %pP (vd %04x:%04x)\n"
            , pci, pci->vendor, pci->device);

    বর্ণ fname[17];
    snprintf(fname, পরিমাণ(fname), "pci%04x,%04x.rom"
             , pci->vendor, pci->device);
    সংরচনা romfile_s *file = romfile_find(fname);
    সংরচনা rom_header *rom = রিক্ত;
    যদি (file)
        rom = deploy_romfile(file);
    অন্যথা যদি (RunPCIroms > 1 || (RunPCIroms == 1 && isvga))
        rom = map_pcirom(pci);
    যদি (! rom)
        // No ROM present.
        প্রত্যাগম;
    পূর্ণাঙ্ক irq_was_captured = boot_irq_captured();
    সংরচনা pnp_data *pnp = get_pnp_rom(rom);
    setRomSource(sources, rom, RS_PCIROM | (u32)pci);
    init_optionrom(rom, pci->bdf, isvga);
    যদি (boot_irq_captured() && !irq_was_captured &&
        !file && !isvga && pnp) {
        // This PCI rom is misbehaving - recapture the boot irqs
        বর্ণ *desc = MAKE_FLATPTR(FLATPTR_TO_SEG(rom), pnp->productname);
        dprintf(1, "PnP optionrom \"%s\" (bdf %pP) captured int19, restoring\n",
                desc, pci);
        boot_irq_restore();
    }
}


/****************************************************************
 * Non-VGA option rom init
 ****************************************************************/

শূন্য
optionrom_setup(শূন্য)
{
    যদি (! CONFIG_OPTIONROMS)
        প্রত্যাগম;

    dprintf(1, "Scan for option roms\n");
    u64 sources[(BUILD_BIOS_ADDR - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    memset(sources, 0, পরিমাণ(sources));
    u32 post_vga = rom_get_last();

    // Find and deploy PCI roms.
    সংরচনা pci_device *pci;
    foreachpci(pci) {
        যদি (pci->class == PCI_CLASS_DISPLAY_VGA ||
            pci->class == PCI_CLASS_DISPLAY_OTHER ||
            pci->have_driver)
            অনুবর্ত;
        init_pcirom(pci, 0, sources);
    }

    // Find and deploy CBFS roms not associated with a device.
    run_file_roms("genroms/", 0, sources);
    rom_reserve(0);

    // All option roms found and deployed - now build BEV/BCV vectors.

    u32 pos = post_vga;
    যাবত্ (pos < rom_get_last()) {
        সংরচনা rom_header *rom = (শূন্য*)pos;
        যদি (! is_valid_rom(rom)) {
            pos += OPTION_ROM_ALIGN;
            অনুবর্ত;
        }
        pos += ALIGN(rom->size * 512, OPTION_ROM_ALIGN);
        সংরচনা pnp_data *pnp = get_pnp_rom(rom);
        যদি (! pnp) {
            // Legacy rom.
            boot_add_bcv(FLATPTR_TO_SEG(rom), OPTION_ROM_INITVECTOR, 0
                         , getRomPriority(sources, rom, 0));
            অনুবর্ত;
        }
        // PnP rom - check for BEV and BCV boot capabilities.
        পূর্ণাঙ্ক instance = 0;
        যাবত্ (pnp) {
            যদি (pnp->bev)
                boot_add_bev(FLATPTR_TO_SEG(rom), pnp->bev, pnp->productname
                             , getRomPriority(sources, rom, instance++));
            অন্যথা যদি (pnp->bcv)
                boot_add_bcv(FLATPTR_TO_SEG(rom), pnp->bcv, pnp->productname
                             , getRomPriority(sources, rom, instance++));
            অন্যথা
                ভঙ্গ;
            pnp = get_pnp_next(rom, pnp);
        }
    }
}


/****************************************************************
 * VGA init
 ****************************************************************/

পূর্ণাঙ্ক ScreenAndDebug;
সংরচনা rom_header *VgaROM;

স্থির শূন্য try_setup_display_other(শূন্য)
{
    সংরচনা pci_device *pci;

    dprintf(1, "No VGA found, scan for other display\n");

    foreachpci(pci) {
        যদি (pci->class != PCI_CLASS_DISPLAY_OTHER)
            অনুবর্ত;
        সংরচনা rom_header *rom = map_pcirom(pci);
        যদি (!rom)
            অনুবর্ত;
        dprintf(1, "Other display found at %pP\n", pci);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_MEMORY);
        init_optionrom(rom, pci->bdf, 1);
        প্রত্যাগম;
    }
}

// Call into vga code to turn on console.
শূন্য
vgarom_setup(শূন্য)
{
    পূর্ণাঙ্ক have_vga = 0;

    যদি (! CONFIG_OPTIONROMS)
        প্রত্যাগম;

    dprintf(1, "Scan for VGA option rom\n");

    // Load some config settings that impact VGA.
    EnforceChecksum = romfile_loadint("etc/optionroms-checksum", 1);
    S3ResumeVga = romfile_loadint("etc/s3-resume-vga-init", CONFIG_QEMU);
    RunPCIroms = romfile_loadint("etc/pci-optionrom-exec", 2);
    ScreenAndDebug = romfile_loadint("etc/screen-and-debug", 1);

    // Clear option rom memory
    memset((শূন্য*)BUILD_ROM_START, 0, rom_get_max() - BUILD_ROM_START);

    // Find and deploy PCI VGA rom.
    সংরচনা pci_device *pci;
    foreachpci(pci) {
        যদি (!is_pci_vga(pci))
            অনুবর্ত;
        vgahook_setup(pci);
        init_pcirom(pci, 1, রিক্ত);
        have_vga = 1;
        ভঙ্গ;
    }
    যদি (!have_vga)
        try_setup_display_other();

    // Find and deploy CBFS vga-style roms not associated with a device.
    run_file_roms("vgaroms/", 1, রিক্ত);
    rom_reserve(0);

    যদি (rom_get_last() != BUILD_ROM_START)
        // VGA rom found
        VgaROM = (শূন্য*)BUILD_ROM_START;
}

শূন্য
s3_resume_vga(শূন্য)
{
    যদি (!S3ResumeVga)
        প্রত্যাগম;
    যদি (!VgaROM || ! is_valid_rom(VgaROM))
        প্রত্যাগম;
    callrom(VgaROM, 0);
}

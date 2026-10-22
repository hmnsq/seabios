// Option rom scanning code.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "bregs.h" // struct bregs
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "farptr.h" // FLATPTR_TO_SEG
#അന്തര്ഭാവ "biosvar.h" // GET_IVT
#അന്തര്ഭാവ "hw/pci.h" // pci_config_readl
#അന്തര്ഭാവ "hw/pcidevice.h" // foreachpci
#അന്തര്ഭാവ "hw/pci_ids.h" // PCI_CLASS_DISPLAY_VGA
#അന്തര്ഭാവ "hw/pci_regs.h" // PCI_ROM_ADDRESS
#അന്തര്ഭാവ "malloc.h" // rom_confirm
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "romfile.h" // romfile_loadint
#അന്തര്ഭാവ "stacks.h" // farcall16big
#അന്തര്ഭാവ "std/optionrom.h" // struct rom_header
#അന്തര്ഭാവ "std/pnpbios.h" // PNP_SIGNATURE
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // get_pnp_offset
#അന്തര്ഭാവ "tcgbios.h" // tpm_*

സ്ഥിതം പൂർണ്ണസംഖ്യ EnforceChecksum, S3ResumeVga, RunPCIroms;


/****************************************************************
 * Helper functions
 ****************************************************************/

// Execute a given option rom.
സ്ഥിതം ശൂന്യം
__callrom(ഘടന rom_header *rom, u16 offset, u16 bdf)
{
    u16 seg = FLATPTR_TO_SEG(rom);
    dprintf(1, "Running option rom at %04x:%04x\n", seg, offset);

    ഘടന bregs br;
    memset(&br, 0, വലുപ്പം(br));
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
ശൂന്യം
callrom(ഘടന rom_header *rom, u16 bdf)
{
    __callrom(rom, OPTION_ROM_INITVECTOR, bdf);
}

// Execute a BCV option rom registered via add_bcv().
ശൂന്യം
call_bcv(u16 seg, u16 ip)
{
    __callrom(MAKE_FLATPTR(seg, 0), ip, 0);
}

// Verify that an option rom looks valid
സ്ഥിതം പൂർണ്ണസംഖ്യ
is_valid_rom(ഘടന rom_header *rom)
{
    dprintf(6, "Checking rom %p (sig %x size %d)\n"
            , rom, rom->signature, rom->size);
    എങ്കിൽ (rom->signature != OPTION_ROM_SIGNATURE)
        തിരികെ 0;
    എങ്കിൽ (! rom->size)
        തിരികെ 0;
    u32 len = rom->size * 512;
    u8 sum = checksum(rom, len);
    എങ്കിൽ (sum != 0) {
        dprintf(1, "Found option rom with bad checksum: loc=%p len=%d sum=%x\n"
                , rom, len, sum);
        എങ്കിൽ (EnforceChecksum)
            തിരികെ 0;
    }
    തിരികെ 1;
}

// Check if a valid option rom has a pnp struct; return it if so.
സ്ഥിതം ഘടന pnp_data *
get_pnp_rom(ഘടന rom_header *rom)
{
    ഘടന pnp_data *pnp = (ശൂന്യം*)((u8*)rom + rom->pnpoffset);
    എങ്കിൽ (pnp->signature != PNP_SIGNATURE)
        തിരികെ ശൂന്യ;
    തിരികെ pnp;
}

// Check for multiple pnp option rom headers.
സ്ഥിതം ഘടന pnp_data *
get_pnp_next(ഘടന rom_header *rom, ഘടന pnp_data *pnp)
{
    എങ്കിൽ (! pnp->nextoffset)
        തിരികെ ശൂന്യ;
    pnp = (ശൂന്യം*)((u8*)rom + pnp->nextoffset);
    എങ്കിൽ (pnp->signature != PNP_SIGNATURE)
        തിരികെ ശൂന്യ;
    തിരികെ pnp;
}

// Check if a valid option rom has a pci struct; return it if so.
സ്ഥിതം ഘടന pci_data *
get_pci_rom(ഘടന rom_header *rom)
{
    ഘടന pci_data *pd = (ശൂന്യം*)((u32)rom + rom->pcioffset);
    എങ്കിൽ (pd->signature != PCI_ROM_SIGNATURE)
        തിരികെ ശൂന്യ;
    എങ്കിൽ (rom->pcioffset & 3)
        dprintf(1, "WARNING! Found unaligned PCI rom (vd=%04x:%04x)\n"
                , pd->vendor, pd->device);
    തിരികെ pd;
}

// Run rom init code and note rom size.
സ്ഥിതം പൂർണ്ണസംഖ്യ
init_optionrom(ഘടന rom_header *rom, u16 bdf, പൂർണ്ണസംഖ്യ isvga)
{
    എങ്കിൽ (! is_valid_rom(rom))
        തിരികെ -1;
    ഘടന rom_header *newrom = rom_reserve(rom->size * 512);
    എങ്കിൽ (!newrom) {
        warn_noalloc();
        തിരികെ -1;
    }
    എങ്കിൽ (newrom != rom)
        memmove(newrom, rom, rom->size * 512);

    tpm_option_rom(newrom, rom->size * 512);

    എങ്കിൽ (isvga || get_pnp_rom(newrom))
        // Only init vga and PnP roms here.
        callrom(newrom, bdf);

    തിരികെ rom_confirm(newrom->size * 512);
}

#define RS_PCIROM (1LL<<33)

സ്ഥിതം ശൂന്യം
setRomSource(u64 *sources, ഘടന rom_header *rom, u64 source)
{
    എങ്കിൽ (sources)
        sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN] = source;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
getRomPriority(u64 *sources, ഘടന rom_header *rom, പൂർണ്ണസംഖ്യ instance)
{
    u64 source = sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    എങ്കിൽ (!source)
        തിരികെ -1;
    എങ്കിൽ (source & RS_PCIROM)
        തിരികെ bootprio_find_pci_rom((ശൂന്യം*)(u32)source, instance);
    ഘടന romfile_s *file = (ശൂന്യം*)(u32)source;
    തിരികെ bootprio_find_named_rom(file->name, instance);
}


/****************************************************************
 * Roms in CBFS
 ****************************************************************/

സ്ഥിതം ഘടന rom_header *
deploy_romfile(ഘടന romfile_s *file)
{
    u32 size = file->size;
    ഘടന rom_header *rom = rom_reserve(size);
    എങ്കിൽ (!rom) {
        warn_noalloc();
        തിരികെ ശൂന്യ;
    }
    പൂർണ്ണസംഖ്യ ret = file->copy(file, rom, size);
    എങ്കിൽ (ret <= 0)
        തിരികെ ശൂന്യ;
    തിരികെ rom;
}

// Run all roms in a given CBFS directory.
സ്ഥിതം ശൂന്യം
run_file_roms(സ്ഥിരം അക്ഷരം *prefix, പൂർണ്ണസംഖ്യ isvga, u64 *sources)
{
    ഘടന romfile_s *file = ശൂന്യ;
    വേണ്ടി (;;) {
        file = romfile_findprefix(prefix, file);
        എങ്കിൽ (!file)
            നിർത്തുക;
        ഘടന rom_header *rom = deploy_romfile(file);
        എങ്കിൽ (rom) {
            setRomSource(sources, rom, (u32)file);
            init_optionrom(rom, 0, isvga);
        }
    }
}


/****************************************************************
 * PCI roms
 ****************************************************************/

// Verify device is a vga device with legacy address decoding enabled.
പൂർണ്ണസംഖ്യ
is_pci_vga(ഘടന pci_device *pci)
{
    എങ്കിൽ (pci->class != PCI_CLASS_DISPLAY_VGA)
        തിരികെ 0;
    u16 cmd = pci_config_readw(pci->bdf, PCI_COMMAND);
    എങ്കിൽ (!(cmd & PCI_COMMAND_IO && cmd & PCI_COMMAND_MEMORY))
        തിരികെ 0;
    ആകുമ്പോൾ (pci->parent) {
        pci = pci->parent;
        u32 ctrl = pci_config_readb(pci->bdf, PCI_BRIDGE_CONTROL);
        എങ്കിൽ (!(ctrl & PCI_BRIDGE_CTL_VGA))
            തിരികെ 0;
    }
    തിരികെ 1;
}

// Copy a rom to its permanent location below 1MiB
സ്ഥിതം ഘടന rom_header *
copy_rom(ഘടന rom_header *rom)
{
    u32 romsize = rom->size * 512;
    ഘടന rom_header *newrom = rom_reserve(romsize);
    എങ്കിൽ (!newrom) {
        warn_noalloc();
        തിരികെ ശൂന്യ;
    }
    dprintf(4, "Copying option rom (size %d) from %p to %p\n"
            , romsize, rom, newrom);
    iomemcpy(newrom, rom, romsize);
    തിരികെ newrom;
}

// Map the option rom of a given PCI device.
സ്ഥിതം ഘടന rom_header *
map_pcirom(ഘടന pci_device *pci)
{
    dprintf(6, "Attempting to map option rom on dev %pP\n", pci);

    എങ്കിൽ ((pci->header_type & 0x7f) != PCI_HEADER_TYPE_NORMAL) {
        dprintf(6, "Skipping non-normal pci device (type=%x)\n"
                , pci->header_type);
        തിരികെ ശൂന്യ;
    }

    u16 bdf = pci->bdf;
    u32 orig = pci_config_readl(bdf, PCI_ROM_ADDRESS);
    pci_config_writel(bdf, PCI_ROM_ADDRESS, ~PCI_ROM_ADDRESS_ENABLE);
    u32 sz = pci_config_readl(bdf, PCI_ROM_ADDRESS);

    dprintf(6, "Option rom sizing returned %x %x\n", orig, sz);
    orig &= ~PCI_ROM_ADDRESS_ENABLE;
    എങ്കിൽ (!sz || sz == 0xffffffff)
        പോകുക fail;

    എങ്കിൽ (orig == sz || (u32)(orig + 4*1024*1024) < 20*1024*1024) {
        // Don't try to map to a pci addresses at its max, in the last
        // 4MiB of ram, or the first 16MiB of ram.
        dprintf(6, "Preset rom address doesn't look valid\n");
        പോകുക fail;
    }

    // Looks like a rom - enable it.
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig | PCI_ROM_ADDRESS_ENABLE);

    ഘടന rom_header *rom = (ശൂന്യം*)orig;
    വേണ്ടി (;;) {
        dprintf(5, "Inspecting possible rom at %p (vd=%04x:%04x bdf=%pP)\n"
                , rom, pci->vendor, pci->device, pci);
        എങ്കിൽ (rom->signature != OPTION_ROM_SIGNATURE) {
            dprintf(6, "No option rom signature (got %x)\n", rom->signature);
            പോകുക fail;
        }
        ഘടന pci_data *pd = get_pci_rom(rom);
        എങ്കിൽ (! pd) {
            dprintf(6, "No valid pci signature found\n");
            പോകുക fail;
        }

        എങ്കിൽ (pd->vendor == pci->vendor && pd->device == pci->device
            && pd->type == PCIROM_CODETYPE_X86)
            // A match
            നിർത്തുക;
        dprintf(6, "Didn't match dev/ven (got %04x:%04x) or type (got %d)\n"
                , pd->vendor, pd->device, pd->type);
        എങ്കിൽ (pd->indicator & 0x80) {
            dprintf(6, "No more images left\n");
            പോകുക fail;
        }
        rom = (ശൂന്യം*)((u32)rom + pd->ilen * 512);
    }

    rom = copy_rom(rom);
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig);
    തിരികെ rom;
fail:
    // Not valid - restore original and exit.
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig);
    തിരികെ ശൂന്യ;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ boot_irq_captured(ശൂന്യം)
{
    തിരികെ GET_IVT(0x19).segoff != FUNC16(entry_19_official).segoff;
}

സ്ഥിതം ശൂന്യം boot_irq_restore(ശൂന്യം)
{
    ഘടന segoff_s seabios;

    seabios = FUNC16(entry_19_official);
    SET_IVT(0x19, seabios);
}

// Attempt to map and initialize the option rom on a given PCI device.
സ്ഥിതം ശൂന്യം
init_pcirom(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ isvga, u64 *sources)
{
    dprintf(4, "Attempting to init PCI bdf %pP (vd %04x:%04x)\n"
            , pci, pci->vendor, pci->device);

    അക്ഷരം fname[17];
    snprintf(fname, വലുപ്പം(fname), "pci%04x,%04x.rom"
             , pci->vendor, pci->device);
    ഘടന romfile_s *file = romfile_find(fname);
    ഘടന rom_header *rom = ശൂന്യ;
    എങ്കിൽ (file)
        rom = deploy_romfile(file);
    അല്ലെങ്കിൽ എങ്കിൽ (RunPCIroms > 1 || (RunPCIroms == 1 && isvga))
        rom = map_pcirom(pci);
    എങ്കിൽ (! rom)
        // No ROM present.
        തിരികെ;
    പൂർണ്ണസംഖ്യ irq_was_captured = boot_irq_captured();
    ഘടന pnp_data *pnp = get_pnp_rom(rom);
    setRomSource(sources, rom, RS_PCIROM | (u32)pci);
    init_optionrom(rom, pci->bdf, isvga);
    എങ്കിൽ (boot_irq_captured() && !irq_was_captured &&
        !file && !isvga && pnp) {
        // This PCI rom is misbehaving - recapture the boot irqs
        അക്ഷരം *desc = MAKE_FLATPTR(FLATPTR_TO_SEG(rom), pnp->productname);
        dprintf(1, "PnP optionrom \"%s\" (bdf %pP) captured int19, restoring\n",
                desc, pci);
        boot_irq_restore();
    }
}


/****************************************************************
 * Non-VGA option rom init
 ****************************************************************/

ശൂന്യം
optionrom_setup(ശൂന്യം)
{
    എങ്കിൽ (! CONFIG_OPTIONROMS)
        തിരികെ;

    dprintf(1, "Scan for option roms\n");
    u64 sources[(BUILD_BIOS_ADDR - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    memset(sources, 0, വലുപ്പം(sources));
    u32 post_vga = rom_get_last();

    // Find and deploy PCI roms.
    ഘടന pci_device *pci;
    foreachpci(pci) {
        എങ്കിൽ (pci->class == PCI_CLASS_DISPLAY_VGA ||
            pci->class == PCI_CLASS_DISPLAY_OTHER ||
            pci->have_driver)
            തുടരുക;
        init_pcirom(pci, 0, sources);
    }

    // Find and deploy CBFS roms not associated with a device.
    run_file_roms("genroms/", 0, sources);
    rom_reserve(0);

    // All option roms found and deployed - now build BEV/BCV vectors.

    u32 pos = post_vga;
    ആകുമ്പോൾ (pos < rom_get_last()) {
        ഘടന rom_header *rom = (ശൂന്യം*)pos;
        എങ്കിൽ (! is_valid_rom(rom)) {
            pos += OPTION_ROM_ALIGN;
            തുടരുക;
        }
        pos += ALIGN(rom->size * 512, OPTION_ROM_ALIGN);
        ഘടന pnp_data *pnp = get_pnp_rom(rom);
        എങ്കിൽ (! pnp) {
            // Legacy rom.
            boot_add_bcv(FLATPTR_TO_SEG(rom), OPTION_ROM_INITVECTOR, 0
                         , getRomPriority(sources, rom, 0));
            തുടരുക;
        }
        // PnP rom - check for BEV and BCV boot capabilities.
        പൂർണ്ണസംഖ്യ instance = 0;
        ആകുമ്പോൾ (pnp) {
            എങ്കിൽ (pnp->bev)
                boot_add_bev(FLATPTR_TO_SEG(rom), pnp->bev, pnp->productname
                             , getRomPriority(sources, rom, instance++));
            അല്ലെങ്കിൽ എങ്കിൽ (pnp->bcv)
                boot_add_bcv(FLATPTR_TO_SEG(rom), pnp->bcv, pnp->productname
                             , getRomPriority(sources, rom, instance++));
            അല്ലെങ്കിൽ
                നിർത്തുക;
            pnp = get_pnp_next(rom, pnp);
        }
    }
}


/****************************************************************
 * VGA init
 ****************************************************************/

പൂർണ്ണസംഖ്യ ScreenAndDebug;
ഘടന rom_header *VgaROM;

സ്ഥിതം ശൂന്യം try_setup_display_other(ശൂന്യം)
{
    ഘടന pci_device *pci;

    dprintf(1, "No VGA found, scan for other display\n");

    foreachpci(pci) {
        എങ്കിൽ (pci->class != PCI_CLASS_DISPLAY_OTHER)
            തുടരുക;
        ഘടന rom_header *rom = map_pcirom(pci);
        എങ്കിൽ (!rom)
            തുടരുക;
        dprintf(1, "Other display found at %pP\n", pci);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_MEMORY);
        init_optionrom(rom, pci->bdf, 1);
        തിരികെ;
    }
}

// Call into vga code to turn on console.
ശൂന്യം
vgarom_setup(ശൂന്യം)
{
    പൂർണ്ണസംഖ്യ have_vga = 0;

    എങ്കിൽ (! CONFIG_OPTIONROMS)
        തിരികെ;

    dprintf(1, "Scan for VGA option rom\n");

    // Load some config settings that impact VGA.
    EnforceChecksum = romfile_loadint("etc/optionroms-checksum", 1);
    S3ResumeVga = romfile_loadint("etc/s3-resume-vga-init", CONFIG_QEMU);
    RunPCIroms = romfile_loadint("etc/pci-optionrom-exec", 2);
    ScreenAndDebug = romfile_loadint("etc/screen-and-debug", 1);

    // Clear option rom memory
    memset((ശൂന്യം*)BUILD_ROM_START, 0, rom_get_max() - BUILD_ROM_START);

    // Find and deploy PCI VGA rom.
    ഘടന pci_device *pci;
    foreachpci(pci) {
        എങ്കിൽ (!is_pci_vga(pci))
            തുടരുക;
        vgahook_setup(pci);
        init_pcirom(pci, 1, ശൂന്യ);
        have_vga = 1;
        നിർത്തുക;
    }
    എങ്കിൽ (!have_vga)
        try_setup_display_other();

    // Find and deploy CBFS vga-style roms not associated with a device.
    run_file_roms("vgaroms/", 1, ശൂന്യ);
    rom_reserve(0);

    എങ്കിൽ (rom_get_last() != BUILD_ROM_START)
        // VGA rom found
        VgaROM = (ശൂന്യം*)BUILD_ROM_START;
}

ശൂന്യം
s3_resume_vga(ശൂന്യം)
{
    എങ്കിൽ (!S3ResumeVga)
        തിരികെ;
    എങ്കിൽ (!VgaROM || ! is_valid_rom(VgaROM))
        തിരികെ;
    callrom(VgaROM, 0);
}

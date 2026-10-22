// Option rom scanning code.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "bregs.h" // struct bregs
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "farptr.h" // FLATPTR_TO_SEG
#அந்தர்பாவ "biosvar.h" // GET_IVT
#அந்தர்பாவ "hw/pci.h" // pci_config_readl
#அந்தர்பாவ "hw/pcidevice.h" // foreachpci
#அந்தர்பாவ "hw/pci_ids.h" // PCI_CLASS_DISPLAY_VGA
#அந்தர்பாவ "hw/pci_regs.h" // PCI_ROM_ADDRESS
#அந்தர்பாவ "malloc.h" // rom_confirm
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "romfile.h" // romfile_loadint
#அந்தர்பாவ "stacks.h" // farcall16big
#அந்தர்பாவ "std/optionrom.h" // struct rom_header
#அந்தர்பாவ "std/pnpbios.h" // PNP_SIGNATURE
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // get_pnp_offset
#அந்தர்பாவ "tcgbios.h" // tpm_*

நிலையான முழு EnforceChecksum, S3ResumeVga, RunPCIroms;


/****************************************************************
 * Helper functions
 ****************************************************************/

// Execute a given option rom.
நிலையான வெறுமை
__callrom(கட்டமைப்பு rom_header *rom, u16 offset, u16 bdf)
{
    u16 seg = FLATPTR_TO_SEG(rom);
    dprintf(1, "Running option rom at %04x:%04x\n", seg, offset);

    கட்டமைப்பு bregs br;
    memset(&br, 0, அளவு(br));
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
வெறுமை
callrom(கட்டமைப்பு rom_header *rom, u16 bdf)
{
    __callrom(rom, OPTION_ROM_INITVECTOR, bdf);
}

// Execute a BCV option rom registered via add_bcv().
வெறுமை
call_bcv(u16 seg, u16 ip)
{
    __callrom(MAKE_FLATPTR(seg, 0), ip, 0);
}

// Verify that an option rom looks valid
நிலையான முழு
is_valid_rom(கட்டமைப்பு rom_header *rom)
{
    dprintf(6, "Checking rom %p (sig %x size %d)\n"
            , rom, rom->signature, rom->size);
    எனில் (rom->signature != OPTION_ROM_SIGNATURE)
        திரும்பு 0;
    எனில் (! rom->size)
        திரும்பு 0;
    u32 len = rom->size * 512;
    u8 sum = checksum(rom, len);
    எனில் (sum != 0) {
        dprintf(1, "Found option rom with bad checksum: loc=%p len=%d sum=%x\n"
                , rom, len, sum);
        எனில் (EnforceChecksum)
            திரும்பு 0;
    }
    திரும்பு 1;
}

// Check if a valid option rom has a pnp struct; return it if so.
நிலையான கட்டமைப்பு pnp_data *
get_pnp_rom(கட்டமைப்பு rom_header *rom)
{
    கட்டமைப்பு pnp_data *pnp = (வெறுமை*)((u8*)rom + rom->pnpoffset);
    எனில் (pnp->signature != PNP_SIGNATURE)
        திரும்பு வெற்று;
    திரும்பு pnp;
}

// Check for multiple pnp option rom headers.
நிலையான கட்டமைப்பு pnp_data *
get_pnp_next(கட்டமைப்பு rom_header *rom, கட்டமைப்பு pnp_data *pnp)
{
    எனில் (! pnp->nextoffset)
        திரும்பு வெற்று;
    pnp = (வெறுமை*)((u8*)rom + pnp->nextoffset);
    எனில் (pnp->signature != PNP_SIGNATURE)
        திரும்பு வெற்று;
    திரும்பு pnp;
}

// Check if a valid option rom has a pci struct; return it if so.
நிலையான கட்டமைப்பு pci_data *
get_pci_rom(கட்டமைப்பு rom_header *rom)
{
    கட்டமைப்பு pci_data *pd = (வெறுமை*)((u32)rom + rom->pcioffset);
    எனில் (pd->signature != PCI_ROM_SIGNATURE)
        திரும்பு வெற்று;
    எனில் (rom->pcioffset & 3)
        dprintf(1, "WARNING! Found unaligned PCI rom (vd=%04x:%04x)\n"
                , pd->vendor, pd->device);
    திரும்பு pd;
}

// Run rom init code and note rom size.
நிலையான முழு
init_optionrom(கட்டமைப்பு rom_header *rom, u16 bdf, முழு isvga)
{
    எனில் (! is_valid_rom(rom))
        திரும்பு -1;
    கட்டமைப்பு rom_header *newrom = rom_reserve(rom->size * 512);
    எனில் (!newrom) {
        warn_noalloc();
        திரும்பு -1;
    }
    எனில் (newrom != rom)
        memmove(newrom, rom, rom->size * 512);

    tpm_option_rom(newrom, rom->size * 512);

    எனில் (isvga || get_pnp_rom(newrom))
        // Only init vga and PnP roms here.
        callrom(newrom, bdf);

    திரும்பு rom_confirm(newrom->size * 512);
}

#define RS_PCIROM (1LL<<33)

நிலையான வெறுமை
setRomSource(u64 *sources, கட்டமைப்பு rom_header *rom, u64 source)
{
    எனில் (sources)
        sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN] = source;
}

நிலையான முழு
getRomPriority(u64 *sources, கட்டமைப்பு rom_header *rom, முழு instance)
{
    u64 source = sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    எனில் (!source)
        திரும்பு -1;
    எனில் (source & RS_PCIROM)
        திரும்பு bootprio_find_pci_rom((வெறுமை*)(u32)source, instance);
    கட்டமைப்பு romfile_s *file = (வெறுமை*)(u32)source;
    திரும்பு bootprio_find_named_rom(file->name, instance);
}


/****************************************************************
 * Roms in CBFS
 ****************************************************************/

நிலையான கட்டமைப்பு rom_header *
deploy_romfile(கட்டமைப்பு romfile_s *file)
{
    u32 size = file->size;
    கட்டமைப்பு rom_header *rom = rom_reserve(size);
    எனில் (!rom) {
        warn_noalloc();
        திரும்பு வெற்று;
    }
    முழு ret = file->copy(file, rom, size);
    எனில் (ret <= 0)
        திரும்பு வெற்று;
    திரும்பு rom;
}

// Run all roms in a given CBFS directory.
நிலையான வெறுமை
run_file_roms(மாறிலி எழுத்து *prefix, முழு isvga, u64 *sources)
{
    கட்டமைப்பு romfile_s *file = வெற்று;
    க்கு (;;) {
        file = romfile_findprefix(prefix, file);
        எனில் (!file)
            நிறுத்து;
        கட்டமைப்பு rom_header *rom = deploy_romfile(file);
        எனில் (rom) {
            setRomSource(sources, rom, (u32)file);
            init_optionrom(rom, 0, isvga);
        }
    }
}


/****************************************************************
 * PCI roms
 ****************************************************************/

// Verify device is a vga device with legacy address decoding enabled.
முழு
is_pci_vga(கட்டமைப்பு pci_device *pci)
{
    எனில் (pci->class != PCI_CLASS_DISPLAY_VGA)
        திரும்பு 0;
    u16 cmd = pci_config_readw(pci->bdf, PCI_COMMAND);
    எனில் (!(cmd & PCI_COMMAND_IO && cmd & PCI_COMMAND_MEMORY))
        திரும்பு 0;
    போது (pci->parent) {
        pci = pci->parent;
        u32 ctrl = pci_config_readb(pci->bdf, PCI_BRIDGE_CONTROL);
        எனில் (!(ctrl & PCI_BRIDGE_CTL_VGA))
            திரும்பு 0;
    }
    திரும்பு 1;
}

// Copy a rom to its permanent location below 1MiB
நிலையான கட்டமைப்பு rom_header *
copy_rom(கட்டமைப்பு rom_header *rom)
{
    u32 romsize = rom->size * 512;
    கட்டமைப்பு rom_header *newrom = rom_reserve(romsize);
    எனில் (!newrom) {
        warn_noalloc();
        திரும்பு வெற்று;
    }
    dprintf(4, "Copying option rom (size %d) from %p to %p\n"
            , romsize, rom, newrom);
    iomemcpy(newrom, rom, romsize);
    திரும்பு newrom;
}

// Map the option rom of a given PCI device.
நிலையான கட்டமைப்பு rom_header *
map_pcirom(கட்டமைப்பு pci_device *pci)
{
    dprintf(6, "Attempting to map option rom on dev %pP\n", pci);

    எனில் ((pci->header_type & 0x7f) != PCI_HEADER_TYPE_NORMAL) {
        dprintf(6, "Skipping non-normal pci device (type=%x)\n"
                , pci->header_type);
        திரும்பு வெற்று;
    }

    u16 bdf = pci->bdf;
    u32 orig = pci_config_readl(bdf, PCI_ROM_ADDRESS);
    pci_config_writel(bdf, PCI_ROM_ADDRESS, ~PCI_ROM_ADDRESS_ENABLE);
    u32 sz = pci_config_readl(bdf, PCI_ROM_ADDRESS);

    dprintf(6, "Option rom sizing returned %x %x\n", orig, sz);
    orig &= ~PCI_ROM_ADDRESS_ENABLE;
    எனில் (!sz || sz == 0xffffffff)
        செல் fail;

    எனில் (orig == sz || (u32)(orig + 4*1024*1024) < 20*1024*1024) {
        // Don't try to map to a pci addresses at its max, in the last
        // 4MiB of ram, or the first 16MiB of ram.
        dprintf(6, "Preset rom address doesn't look valid\n");
        செல் fail;
    }

    // Looks like a rom - enable it.
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig | PCI_ROM_ADDRESS_ENABLE);

    கட்டமைப்பு rom_header *rom = (வெறுமை*)orig;
    க்கு (;;) {
        dprintf(5, "Inspecting possible rom at %p (vd=%04x:%04x bdf=%pP)\n"
                , rom, pci->vendor, pci->device, pci);
        எனில் (rom->signature != OPTION_ROM_SIGNATURE) {
            dprintf(6, "No option rom signature (got %x)\n", rom->signature);
            செல் fail;
        }
        கட்டமைப்பு pci_data *pd = get_pci_rom(rom);
        எனில் (! pd) {
            dprintf(6, "No valid pci signature found\n");
            செல் fail;
        }

        எனில் (pd->vendor == pci->vendor && pd->device == pci->device
            && pd->type == PCIROM_CODETYPE_X86)
            // A match
            நிறுத்து;
        dprintf(6, "Didn't match dev/ven (got %04x:%04x) or type (got %d)\n"
                , pd->vendor, pd->device, pd->type);
        எனில் (pd->indicator & 0x80) {
            dprintf(6, "No more images left\n");
            செல் fail;
        }
        rom = (வெறுமை*)((u32)rom + pd->ilen * 512);
    }

    rom = copy_rom(rom);
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig);
    திரும்பு rom;
fail:
    // Not valid - restore original and exit.
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig);
    திரும்பு வெற்று;
}

நிலையான முழு boot_irq_captured(வெறுமை)
{
    திரும்பு GET_IVT(0x19).segoff != FUNC16(entry_19_official).segoff;
}

நிலையான வெறுமை boot_irq_restore(வெறுமை)
{
    கட்டமைப்பு segoff_s seabios;

    seabios = FUNC16(entry_19_official);
    SET_IVT(0x19, seabios);
}

// Attempt to map and initialize the option rom on a given PCI device.
நிலையான வெறுமை
init_pcirom(கட்டமைப்பு pci_device *pci, முழு isvga, u64 *sources)
{
    dprintf(4, "Attempting to init PCI bdf %pP (vd %04x:%04x)\n"
            , pci, pci->vendor, pci->device);

    எழுத்து fname[17];
    snprintf(fname, அளவு(fname), "pci%04x,%04x.rom"
             , pci->vendor, pci->device);
    கட்டமைப்பு romfile_s *file = romfile_find(fname);
    கட்டமைப்பு rom_header *rom = வெற்று;
    எனில் (file)
        rom = deploy_romfile(file);
    இல்லையெனில் எனில் (RunPCIroms > 1 || (RunPCIroms == 1 && isvga))
        rom = map_pcirom(pci);
    எனில் (! rom)
        // No ROM present.
        திரும்பு;
    முழு irq_was_captured = boot_irq_captured();
    கட்டமைப்பு pnp_data *pnp = get_pnp_rom(rom);
    setRomSource(sources, rom, RS_PCIROM | (u32)pci);
    init_optionrom(rom, pci->bdf, isvga);
    எனில் (boot_irq_captured() && !irq_was_captured &&
        !file && !isvga && pnp) {
        // This PCI rom is misbehaving - recapture the boot irqs
        எழுத்து *desc = MAKE_FLATPTR(FLATPTR_TO_SEG(rom), pnp->productname);
        dprintf(1, "PnP optionrom \"%s\" (bdf %pP) captured int19, restoring\n",
                desc, pci);
        boot_irq_restore();
    }
}


/****************************************************************
 * Non-VGA option rom init
 ****************************************************************/

வெறுமை
optionrom_setup(வெறுமை)
{
    எனில் (! CONFIG_OPTIONROMS)
        திரும்பு;

    dprintf(1, "Scan for option roms\n");
    u64 sources[(BUILD_BIOS_ADDR - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    memset(sources, 0, அளவு(sources));
    u32 post_vga = rom_get_last();

    // Find and deploy PCI roms.
    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        எனில் (pci->class == PCI_CLASS_DISPLAY_VGA ||
            pci->class == PCI_CLASS_DISPLAY_OTHER ||
            pci->have_driver)
            தொடர்;
        init_pcirom(pci, 0, sources);
    }

    // Find and deploy CBFS roms not associated with a device.
    run_file_roms("genroms/", 0, sources);
    rom_reserve(0);

    // All option roms found and deployed - now build BEV/BCV vectors.

    u32 pos = post_vga;
    போது (pos < rom_get_last()) {
        கட்டமைப்பு rom_header *rom = (வெறுமை*)pos;
        எனில் (! is_valid_rom(rom)) {
            pos += OPTION_ROM_ALIGN;
            தொடர்;
        }
        pos += ALIGN(rom->size * 512, OPTION_ROM_ALIGN);
        கட்டமைப்பு pnp_data *pnp = get_pnp_rom(rom);
        எனில் (! pnp) {
            // Legacy rom.
            boot_add_bcv(FLATPTR_TO_SEG(rom), OPTION_ROM_INITVECTOR, 0
                         , getRomPriority(sources, rom, 0));
            தொடர்;
        }
        // PnP rom - check for BEV and BCV boot capabilities.
        முழு instance = 0;
        போது (pnp) {
            எனில் (pnp->bev)
                boot_add_bev(FLATPTR_TO_SEG(rom), pnp->bev, pnp->productname
                             , getRomPriority(sources, rom, instance++));
            இல்லையெனில் எனில் (pnp->bcv)
                boot_add_bcv(FLATPTR_TO_SEG(rom), pnp->bcv, pnp->productname
                             , getRomPriority(sources, rom, instance++));
            இல்லையெனில்
                நிறுத்து;
            pnp = get_pnp_next(rom, pnp);
        }
    }
}


/****************************************************************
 * VGA init
 ****************************************************************/

முழு ScreenAndDebug;
கட்டமைப்பு rom_header *VgaROM;

நிலையான வெறுமை try_setup_display_other(வெறுமை)
{
    கட்டமைப்பு pci_device *pci;

    dprintf(1, "No VGA found, scan for other display\n");

    foreachpci(pci) {
        எனில் (pci->class != PCI_CLASS_DISPLAY_OTHER)
            தொடர்;
        கட்டமைப்பு rom_header *rom = map_pcirom(pci);
        எனில் (!rom)
            தொடர்;
        dprintf(1, "Other display found at %pP\n", pci);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_MEMORY);
        init_optionrom(rom, pci->bdf, 1);
        திரும்பு;
    }
}

// Call into vga code to turn on console.
வெறுமை
vgarom_setup(வெறுமை)
{
    முழு have_vga = 0;

    எனில் (! CONFIG_OPTIONROMS)
        திரும்பு;

    dprintf(1, "Scan for VGA option rom\n");

    // Load some config settings that impact VGA.
    EnforceChecksum = romfile_loadint("etc/optionroms-checksum", 1);
    S3ResumeVga = romfile_loadint("etc/s3-resume-vga-init", CONFIG_QEMU);
    RunPCIroms = romfile_loadint("etc/pci-optionrom-exec", 2);
    ScreenAndDebug = romfile_loadint("etc/screen-and-debug", 1);

    // Clear option rom memory
    memset((வெறுமை*)BUILD_ROM_START, 0, rom_get_max() - BUILD_ROM_START);

    // Find and deploy PCI VGA rom.
    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        எனில் (!is_pci_vga(pci))
            தொடர்;
        vgahook_setup(pci);
        init_pcirom(pci, 1, வெற்று);
        have_vga = 1;
        நிறுத்து;
    }
    எனில் (!have_vga)
        try_setup_display_other();

    // Find and deploy CBFS vga-style roms not associated with a device.
    run_file_roms("vgaroms/", 1, வெற்று);
    rom_reserve(0);

    எனில் (rom_get_last() != BUILD_ROM_START)
        // VGA rom found
        VgaROM = (வெறுமை*)BUILD_ROM_START;
}

வெறுமை
s3_resume_vga(வெறுமை)
{
    எனில் (!S3ResumeVga)
        திரும்பு;
    எனில் (!VgaROM || ! is_valid_rom(VgaROM))
        திரும்பு;
    callrom(VgaROM, 0);
}

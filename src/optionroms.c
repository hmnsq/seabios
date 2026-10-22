// Option rom scanning code.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "bregs.h" // struct bregs
#شامل "config.h" // CONFIG_*
#شامل "farptr.h" // FLATPTR_TO_SEG
#شامل "biosvar.h" // GET_IVT
#شامل "hw/pci.h" // pci_config_readl
#شامل "hw/pcidevice.h" // foreachpci
#شامل "hw/pci_ids.h" // PCI_CLASS_DISPLAY_VGA
#شامل "hw/pci_regs.h" // PCI_ROM_ADDRESS
#شامل "malloc.h" // rom_confirm
#شامل "output.h" // dprintf
#شامل "romfile.h" // romfile_loadint
#شامل "stacks.h" // farcall16big
#شامل "std/optionrom.h" // struct rom_header
#شامل "std/pnpbios.h" // PNP_SIGNATURE
#شامل "string.h" // memset
#شامل "util.h" // get_pnp_offset
#شامل "tcgbios.h" // tpm_*

ایستا صحیح EnforceChecksum, S3ResumeVga, RunPCIroms;


/****************************************************************
 * Helper functions
 ****************************************************************/

// Execute a given option rom.
ایستا تهی
__callrom(ساختار rom_header *rom, u16 offset, u16 bdf)
{
    u16 seg = FLATPTR_TO_SEG(rom);
    dprintf(1, "Running option rom at %04x:%04x\n", seg, offset);

    ساختار bregs br;
    memset(&br, 0, اندازه(br));
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
تهی
callrom(ساختار rom_header *rom, u16 bdf)
{
    __callrom(rom, OPTION_ROM_INITVECTOR, bdf);
}

// Execute a BCV option rom registered via add_bcv().
تهی
call_bcv(u16 seg, u16 ip)
{
    __callrom(MAKE_FLATPTR(seg, 0), ip, 0);
}

// Verify that an option rom looks valid
ایستا صحیح
is_valid_rom(ساختار rom_header *rom)
{
    dprintf(6, "Checking rom %p (sig %x size %d)\n"
            , rom, rom->signature, rom->size);
    اگر (rom->signature != OPTION_ROM_SIGNATURE)
        بازگشت 0;
    اگر (! rom->size)
        بازگشت 0;
    u32 len = rom->size * 512;
    u8 sum = checksum(rom, len);
    اگر (sum != 0) {
        dprintf(1, "Found option rom with bad checksum: loc=%p len=%d sum=%x\n"
                , rom, len, sum);
        اگر (EnforceChecksum)
            بازگشت 0;
    }
    بازگشت 1;
}

// Check if a valid option rom has a pnp struct; return it if so.
ایستا ساختار pnp_data *
get_pnp_rom(ساختار rom_header *rom)
{
    ساختار pnp_data *pnp = (تهی*)((u8*)rom + rom->pnpoffset);
    اگر (pnp->signature != PNP_SIGNATURE)
        بازگشت پوچ;
    بازگشت pnp;
}

// Check for multiple pnp option rom headers.
ایستا ساختار pnp_data *
get_pnp_next(ساختار rom_header *rom, ساختار pnp_data *pnp)
{
    اگر (! pnp->nextoffset)
        بازگشت پوچ;
    pnp = (تهی*)((u8*)rom + pnp->nextoffset);
    اگر (pnp->signature != PNP_SIGNATURE)
        بازگشت پوچ;
    بازگشت pnp;
}

// Check if a valid option rom has a pci struct; return it if so.
ایستا ساختار pci_data *
get_pci_rom(ساختار rom_header *rom)
{
    ساختار pci_data *pd = (تهی*)((u32)rom + rom->pcioffset);
    اگر (pd->signature != PCI_ROM_SIGNATURE)
        بازگشت پوچ;
    اگر (rom->pcioffset & 3)
        dprintf(1, "WARNING! Found unaligned PCI rom (vd=%04x:%04x)\n"
                , pd->vendor, pd->device);
    بازگشت pd;
}

// Run rom init code and note rom size.
ایستا صحیح
init_optionrom(ساختار rom_header *rom, u16 bdf, صحیح isvga)
{
    اگر (! is_valid_rom(rom))
        بازگشت -1;
    ساختار rom_header *newrom = rom_reserve(rom->size * 512);
    اگر (!newrom) {
        warn_noalloc();
        بازگشت -1;
    }
    اگر (newrom != rom)
        memmove(newrom, rom, rom->size * 512);

    tpm_option_rom(newrom, rom->size * 512);

    اگر (isvga || get_pnp_rom(newrom))
        // Only init vga and PnP roms here.
        callrom(newrom, bdf);

    بازگشت rom_confirm(newrom->size * 512);
}

#define RS_PCIROM (1LL<<33)

ایستا تهی
setRomSource(u64 *sources, ساختار rom_header *rom, u64 source)
{
    اگر (sources)
        sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN] = source;
}

ایستا صحیح
getRomPriority(u64 *sources, ساختار rom_header *rom, صحیح instance)
{
    u64 source = sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    اگر (!source)
        بازگشت -1;
    اگر (source & RS_PCIROM)
        بازگشت bootprio_find_pci_rom((تهی*)(u32)source, instance);
    ساختار romfile_s *file = (تهی*)(u32)source;
    بازگشت bootprio_find_named_rom(file->name, instance);
}


/****************************************************************
 * Roms in CBFS
 ****************************************************************/

ایستا ساختار rom_header *
deploy_romfile(ساختار romfile_s *file)
{
    u32 size = file->size;
    ساختار rom_header *rom = rom_reserve(size);
    اگر (!rom) {
        warn_noalloc();
        بازگشت پوچ;
    }
    صحیح ret = file->copy(file, rom, size);
    اگر (ret <= 0)
        بازگشت پوچ;
    بازگشت rom;
}

// Run all roms in a given CBFS directory.
ایستا تهی
run_file_roms(ثابت نویسه *prefix, صحیح isvga, u64 *sources)
{
    ساختار romfile_s *file = پوچ;
    برای (;;) {
        file = romfile_findprefix(prefix, file);
        اگر (!file)
            بشکن;
        ساختار rom_header *rom = deploy_romfile(file);
        اگر (rom) {
            setRomSource(sources, rom, (u32)file);
            init_optionrom(rom, 0, isvga);
        }
    }
}


/****************************************************************
 * PCI roms
 ****************************************************************/

// Verify device is a vga device with legacy address decoding enabled.
صحیح
is_pci_vga(ساختار pci_device *pci)
{
    اگر (pci->class != PCI_CLASS_DISPLAY_VGA)
        بازگشت 0;
    u16 cmd = pci_config_readw(pci->bdf, PCI_COMMAND);
    اگر (!(cmd & PCI_COMMAND_IO && cmd & PCI_COMMAND_MEMORY))
        بازگشت 0;
    وقتی (pci->parent) {
        pci = pci->parent;
        u32 ctrl = pci_config_readb(pci->bdf, PCI_BRIDGE_CONTROL);
        اگر (!(ctrl & PCI_BRIDGE_CTL_VGA))
            بازگشت 0;
    }
    بازگشت 1;
}

// Copy a rom to its permanent location below 1MiB
ایستا ساختار rom_header *
copy_rom(ساختار rom_header *rom)
{
    u32 romsize = rom->size * 512;
    ساختار rom_header *newrom = rom_reserve(romsize);
    اگر (!newrom) {
        warn_noalloc();
        بازگشت پوچ;
    }
    dprintf(4, "Copying option rom (size %d) from %p to %p\n"
            , romsize, rom, newrom);
    iomemcpy(newrom, rom, romsize);
    بازگشت newrom;
}

// Map the option rom of a given PCI device.
ایستا ساختار rom_header *
map_pcirom(ساختار pci_device *pci)
{
    dprintf(6, "Attempting to map option rom on dev %pP\n", pci);

    اگر ((pci->header_type & 0x7f) != PCI_HEADER_TYPE_NORMAL) {
        dprintf(6, "Skipping non-normal pci device (type=%x)\n"
                , pci->header_type);
        بازگشت پوچ;
    }

    u16 bdf = pci->bdf;
    u32 orig = pci_config_readl(bdf, PCI_ROM_ADDRESS);
    pci_config_writel(bdf, PCI_ROM_ADDRESS, ~PCI_ROM_ADDRESS_ENABLE);
    u32 sz = pci_config_readl(bdf, PCI_ROM_ADDRESS);

    dprintf(6, "Option rom sizing returned %x %x\n", orig, sz);
    orig &= ~PCI_ROM_ADDRESS_ENABLE;
    اگر (!sz || sz == 0xffffffff)
        برو fail;

    اگر (orig == sz || (u32)(orig + 4*1024*1024) < 20*1024*1024) {
        // Don't try to map to a pci addresses at its max, in the last
        // 4MiB of ram, or the first 16MiB of ram.
        dprintf(6, "Preset rom address doesn't look valid\n");
        برو fail;
    }

    // Looks like a rom - enable it.
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig | PCI_ROM_ADDRESS_ENABLE);

    ساختار rom_header *rom = (تهی*)orig;
    برای (;;) {
        dprintf(5, "Inspecting possible rom at %p (vd=%04x:%04x bdf=%pP)\n"
                , rom, pci->vendor, pci->device, pci);
        اگر (rom->signature != OPTION_ROM_SIGNATURE) {
            dprintf(6, "No option rom signature (got %x)\n", rom->signature);
            برو fail;
        }
        ساختار pci_data *pd = get_pci_rom(rom);
        اگر (! pd) {
            dprintf(6, "No valid pci signature found\n");
            برو fail;
        }

        اگر (pd->vendor == pci->vendor && pd->device == pci->device
            && pd->type == PCIROM_CODETYPE_X86)
            // A match
            بشکن;
        dprintf(6, "Didn't match dev/ven (got %04x:%04x) or type (got %d)\n"
                , pd->vendor, pd->device, pd->type);
        اگر (pd->indicator & 0x80) {
            dprintf(6, "No more images left\n");
            برو fail;
        }
        rom = (تهی*)((u32)rom + pd->ilen * 512);
    }

    rom = copy_rom(rom);
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig);
    بازگشت rom;
fail:
    // Not valid - restore original and exit.
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig);
    بازگشت پوچ;
}

ایستا صحیح boot_irq_captured(تهی)
{
    بازگشت GET_IVT(0x19).segoff != FUNC16(entry_19_official).segoff;
}

ایستا تهی boot_irq_restore(تهی)
{
    ساختار segoff_s seabios;

    seabios = FUNC16(entry_19_official);
    SET_IVT(0x19, seabios);
}

// Attempt to map and initialize the option rom on a given PCI device.
ایستا تهی
init_pcirom(ساختار pci_device *pci, صحیح isvga, u64 *sources)
{
    dprintf(4, "Attempting to init PCI bdf %pP (vd %04x:%04x)\n"
            , pci, pci->vendor, pci->device);

    نویسه fname[17];
    snprintf(fname, اندازه(fname), "pci%04x,%04x.rom"
             , pci->vendor, pci->device);
    ساختار romfile_s *file = romfile_find(fname);
    ساختار rom_header *rom = پوچ;
    اگر (file)
        rom = deploy_romfile(file);
    وگرنه اگر (RunPCIroms > 1 || (RunPCIroms == 1 && isvga))
        rom = map_pcirom(pci);
    اگر (! rom)
        // No ROM present.
        بازگشت;
    صحیح irq_was_captured = boot_irq_captured();
    ساختار pnp_data *pnp = get_pnp_rom(rom);
    setRomSource(sources, rom, RS_PCIROM | (u32)pci);
    init_optionrom(rom, pci->bdf, isvga);
    اگر (boot_irq_captured() && !irq_was_captured &&
        !file && !isvga && pnp) {
        // This PCI rom is misbehaving - recapture the boot irqs
        نویسه *desc = MAKE_FLATPTR(FLATPTR_TO_SEG(rom), pnp->productname);
        dprintf(1, "PnP optionrom \"%s\" (bdf %pP) captured int19, restoring\n",
                desc, pci);
        boot_irq_restore();
    }
}


/****************************************************************
 * Non-VGA option rom init
 ****************************************************************/

تهی
optionrom_setup(تهی)
{
    اگر (! CONFIG_OPTIONROMS)
        بازگشت;

    dprintf(1, "Scan for option roms\n");
    u64 sources[(BUILD_BIOS_ADDR - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    memset(sources, 0, اندازه(sources));
    u32 post_vga = rom_get_last();

    // Find and deploy PCI roms.
    ساختار pci_device *pci;
    foreachpci(pci) {
        اگر (pci->class == PCI_CLASS_DISPLAY_VGA ||
            pci->class == PCI_CLASS_DISPLAY_OTHER ||
            pci->have_driver)
            ادامه;
        init_pcirom(pci, 0, sources);
    }

    // Find and deploy CBFS roms not associated with a device.
    run_file_roms("genroms/", 0, sources);
    rom_reserve(0);

    // All option roms found and deployed - now build BEV/BCV vectors.

    u32 pos = post_vga;
    وقتی (pos < rom_get_last()) {
        ساختار rom_header *rom = (تهی*)pos;
        اگر (! is_valid_rom(rom)) {
            pos += OPTION_ROM_ALIGN;
            ادامه;
        }
        pos += ALIGN(rom->size * 512, OPTION_ROM_ALIGN);
        ساختار pnp_data *pnp = get_pnp_rom(rom);
        اگر (! pnp) {
            // Legacy rom.
            boot_add_bcv(FLATPTR_TO_SEG(rom), OPTION_ROM_INITVECTOR, 0
                         , getRomPriority(sources, rom, 0));
            ادامه;
        }
        // PnP rom - check for BEV and BCV boot capabilities.
        صحیح instance = 0;
        وقتی (pnp) {
            اگر (pnp->bev)
                boot_add_bev(FLATPTR_TO_SEG(rom), pnp->bev, pnp->productname
                             , getRomPriority(sources, rom, instance++));
            وگرنه اگر (pnp->bcv)
                boot_add_bcv(FLATPTR_TO_SEG(rom), pnp->bcv, pnp->productname
                             , getRomPriority(sources, rom, instance++));
            وگرنه
                بشکن;
            pnp = get_pnp_next(rom, pnp);
        }
    }
}


/****************************************************************
 * VGA init
 ****************************************************************/

صحیح ScreenAndDebug;
ساختار rom_header *VgaROM;

ایستا تهی try_setup_display_other(تهی)
{
    ساختار pci_device *pci;

    dprintf(1, "No VGA found, scan for other display\n");

    foreachpci(pci) {
        اگر (pci->class != PCI_CLASS_DISPLAY_OTHER)
            ادامه;
        ساختار rom_header *rom = map_pcirom(pci);
        اگر (!rom)
            ادامه;
        dprintf(1, "Other display found at %pP\n", pci);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_MEMORY);
        init_optionrom(rom, pci->bdf, 1);
        بازگشت;
    }
}

// Call into vga code to turn on console.
تهی
vgarom_setup(تهی)
{
    صحیح have_vga = 0;

    اگر (! CONFIG_OPTIONROMS)
        بازگشت;

    dprintf(1, "Scan for VGA option rom\n");

    // Load some config settings that impact VGA.
    EnforceChecksum = romfile_loadint("etc/optionroms-checksum", 1);
    S3ResumeVga = romfile_loadint("etc/s3-resume-vga-init", CONFIG_QEMU);
    RunPCIroms = romfile_loadint("etc/pci-optionrom-exec", 2);
    ScreenAndDebug = romfile_loadint("etc/screen-and-debug", 1);

    // Clear option rom memory
    memset((تهی*)BUILD_ROM_START, 0, rom_get_max() - BUILD_ROM_START);

    // Find and deploy PCI VGA rom.
    ساختار pci_device *pci;
    foreachpci(pci) {
        اگر (!is_pci_vga(pci))
            ادامه;
        vgahook_setup(pci);
        init_pcirom(pci, 1, پوچ);
        have_vga = 1;
        بشکن;
    }
    اگر (!have_vga)
        try_setup_display_other();

    // Find and deploy CBFS vga-style roms not associated with a device.
    run_file_roms("vgaroms/", 1, پوچ);
    rom_reserve(0);

    اگر (rom_get_last() != BUILD_ROM_START)
        // VGA rom found
        VgaROM = (تهی*)BUILD_ROM_START;
}

تهی
s3_resume_vga(تهی)
{
    اگر (!S3ResumeVga)
        بازگشت;
    اگر (!VgaROM || ! is_valid_rom(VgaROM))
        بازگشت;
    callrom(VgaROM, 0);
}

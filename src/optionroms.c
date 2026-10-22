// Option rom scanning code.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "bregs.h" // struct bregs
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "farptr.h" // FLATPTR_TO_SEG
#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_IVT
#ଅନ୍ତର୍ଭାଵ "hw/pci.h" // pci_config_readl
#ଅନ୍ତର୍ଭାଵ "hw/pcidevice.h" // foreachpci
#ଅନ୍ତର୍ଭାଵ "hw/pci_ids.h" // PCI_CLASS_DISPLAY_VGA
#ଅନ୍ତର୍ଭାଵ "hw/pci_regs.h" // PCI_ROM_ADDRESS
#ଅନ୍ତର୍ଭାଵ "malloc.h" // rom_confirm
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "romfile.h" // romfile_loadint
#ଅନ୍ତର୍ଭାଵ "stacks.h" // farcall16big
#ଅନ୍ତର୍ଭାଵ "std/optionrom.h" // struct rom_header
#ଅନ୍ତର୍ଭାଵ "std/pnpbios.h" // PNP_SIGNATURE
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // get_pnp_offset
#ଅନ୍ତର୍ଭାଵ "tcgbios.h" // tpm_*

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା EnforceChecksum, S3ResumeVga, RunPCIroms;


/****************************************************************
 * Helper functions
 ****************************************************************/

// Execute a given option rom.
ସ୍ଥିର ଶୂନ୍ୟ
__callrom(ସଂରଚନା rom_header *rom, u16 offset, u16 bdf)
{
    u16 seg = FLATPTR_TO_SEG(rom);
    dprintf(1, "Running option rom at %04x:%04x\n", seg, offset);

    ସଂରଚନା bregs br;
    memset(&br, 0, ଆକାର(br));
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
ଶୂନ୍ୟ
callrom(ସଂରଚନା rom_header *rom, u16 bdf)
{
    __callrom(rom, OPTION_ROM_INITVECTOR, bdf);
}

// Execute a BCV option rom registered via add_bcv().
ଶୂନ୍ୟ
call_bcv(u16 seg, u16 ip)
{
    __callrom(MAKE_FLATPTR(seg, 0), ip, 0);
}

// Verify that an option rom looks valid
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
is_valid_rom(ସଂରଚନା rom_header *rom)
{
    dprintf(6, "Checking rom %p (sig %x size %d)\n"
            , rom, rom->signature, rom->size);
    ଯଦି (rom->signature != OPTION_ROM_SIGNATURE)
        ଫେରନ୍ତୁ 0;
    ଯଦି (! rom->size)
        ଫେରନ୍ତୁ 0;
    u32 len = rom->size * 512;
    u8 sum = checksum(rom, len);
    ଯଦି (sum != 0) {
        dprintf(1, "Found option rom with bad checksum: loc=%p len=%d sum=%x\n"
                , rom, len, sum);
        ଯଦି (EnforceChecksum)
            ଫେରନ୍ତୁ 0;
    }
    ଫେରନ୍ତୁ 1;
}

// Check if a valid option rom has a pnp struct; return it if so.
ସ୍ଥିର ସଂରଚନା pnp_data *
get_pnp_rom(ସଂରଚନା rom_header *rom)
{
    ସଂରଚନା pnp_data *pnp = (ଶୂନ୍ୟ*)((u8*)rom + rom->pnpoffset);
    ଯଦି (pnp->signature != PNP_SIGNATURE)
        ଫେରନ୍ତୁ ଖାଲି;
    ଫେରନ୍ତୁ pnp;
}

// Check for multiple pnp option rom headers.
ସ୍ଥିର ସଂରଚନା pnp_data *
get_pnp_next(ସଂରଚନା rom_header *rom, ସଂରଚନା pnp_data *pnp)
{
    ଯଦି (! pnp->nextoffset)
        ଫେରନ୍ତୁ ଖାଲି;
    pnp = (ଶୂନ୍ୟ*)((u8*)rom + pnp->nextoffset);
    ଯଦି (pnp->signature != PNP_SIGNATURE)
        ଫେରନ୍ତୁ ଖାଲି;
    ଫେରନ୍ତୁ pnp;
}

// Check if a valid option rom has a pci struct; return it if so.
ସ୍ଥିର ସଂରଚନା pci_data *
get_pci_rom(ସଂରଚନା rom_header *rom)
{
    ସଂରଚନା pci_data *pd = (ଶୂନ୍ୟ*)((u32)rom + rom->pcioffset);
    ଯଦି (pd->signature != PCI_ROM_SIGNATURE)
        ଫେରନ୍ତୁ ଖାଲି;
    ଯଦି (rom->pcioffset & 3)
        dprintf(1, "WARNING! Found unaligned PCI rom (vd=%04x:%04x)\n"
                , pd->vendor, pd->device);
    ଫେରନ୍ତୁ pd;
}

// Run rom init code and note rom size.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
init_optionrom(ସଂରଚନା rom_header *rom, u16 bdf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା isvga)
{
    ଯଦି (! is_valid_rom(rom))
        ଫେରନ୍ତୁ -1;
    ସଂରଚନା rom_header *newrom = rom_reserve(rom->size * 512);
    ଯଦି (!newrom) {
        warn_noalloc();
        ଫେରନ୍ତୁ -1;
    }
    ଯଦି (newrom != rom)
        memmove(newrom, rom, rom->size * 512);

    tpm_option_rom(newrom, rom->size * 512);

    ଯଦି (isvga || get_pnp_rom(newrom))
        // Only init vga and PnP roms here.
        callrom(newrom, bdf);

    ଫେରନ୍ତୁ rom_confirm(newrom->size * 512);
}

#define RS_PCIROM (1LL<<33)

ସ୍ଥିର ଶୂନ୍ୟ
setRomSource(u64 *sources, ସଂରଚନା rom_header *rom, u64 source)
{
    ଯଦି (sources)
        sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN] = source;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
getRomPriority(u64 *sources, ସଂରଚନା rom_header *rom, ପୂର୍ଣ୍ଣସଂଖ୍ୟା instance)
{
    u64 source = sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    ଯଦି (!source)
        ଫେରନ୍ତୁ -1;
    ଯଦି (source & RS_PCIROM)
        ଫେରନ୍ତୁ bootprio_find_pci_rom((ଶୂନ୍ୟ*)(u32)source, instance);
    ସଂରଚନା romfile_s *file = (ଶୂନ୍ୟ*)(u32)source;
    ଫେରନ୍ତୁ bootprio_find_named_rom(file->name, instance);
}


/****************************************************************
 * Roms in CBFS
 ****************************************************************/

ସ୍ଥିର ସଂରଚନା rom_header *
deploy_romfile(ସଂରଚନା romfile_s *file)
{
    u32 size = file->size;
    ସଂରଚନା rom_header *rom = rom_reserve(size);
    ଯଦି (!rom) {
        warn_noalloc();
        ଫେରନ୍ତୁ ଖାଲି;
    }
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = file->copy(file, rom, size);
    ଯଦି (ret <= 0)
        ଫେରନ୍ତୁ ଖାଲି;
    ଫେରନ୍ତୁ rom;
}

// Run all roms in a given CBFS directory.
ସ୍ଥିର ଶୂନ୍ୟ
run_file_roms(ଅଚଳ ଅକ୍ଷର *prefix, ପୂର୍ଣ୍ଣସଂଖ୍ୟା isvga, u64 *sources)
{
    ସଂରଚନା romfile_s *file = ଖାଲି;
    ପାଇଁ (;;) {
        file = romfile_findprefix(prefix, file);
        ଯଦି (!file)
            ଭାଙ୍ଗ;
        ସଂରଚନା rom_header *rom = deploy_romfile(file);
        ଯଦି (rom) {
            setRomSource(sources, rom, (u32)file);
            init_optionrom(rom, 0, isvga);
        }
    }
}


/****************************************************************
 * PCI roms
 ****************************************************************/

// Verify device is a vga device with legacy address decoding enabled.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
is_pci_vga(ସଂରଚନା pci_device *pci)
{
    ଯଦି (pci->class != PCI_CLASS_DISPLAY_VGA)
        ଫେରନ୍ତୁ 0;
    u16 cmd = pci_config_readw(pci->bdf, PCI_COMMAND);
    ଯଦି (!(cmd & PCI_COMMAND_IO && cmd & PCI_COMMAND_MEMORY))
        ଫେରନ୍ତୁ 0;
    ଯେତେବେଳେ (pci->parent) {
        pci = pci->parent;
        u32 ctrl = pci_config_readb(pci->bdf, PCI_BRIDGE_CONTROL);
        ଯଦି (!(ctrl & PCI_BRIDGE_CTL_VGA))
            ଫେରନ୍ତୁ 0;
    }
    ଫେରନ୍ତୁ 1;
}

// Copy a rom to its permanent location below 1MiB
ସ୍ଥିର ସଂରଚନା rom_header *
copy_rom(ସଂରଚନା rom_header *rom)
{
    u32 romsize = rom->size * 512;
    ସଂରଚନା rom_header *newrom = rom_reserve(romsize);
    ଯଦି (!newrom) {
        warn_noalloc();
        ଫେରନ୍ତୁ ଖାଲି;
    }
    dprintf(4, "Copying option rom (size %d) from %p to %p\n"
            , romsize, rom, newrom);
    iomemcpy(newrom, rom, romsize);
    ଫେରନ୍ତୁ newrom;
}

// Map the option rom of a given PCI device.
ସ୍ଥିର ସଂରଚନା rom_header *
map_pcirom(ସଂରଚନା pci_device *pci)
{
    dprintf(6, "Attempting to map option rom on dev %pP\n", pci);

    ଯଦି ((pci->header_type & 0x7f) != PCI_HEADER_TYPE_NORMAL) {
        dprintf(6, "Skipping non-normal pci device (type=%x)\n"
                , pci->header_type);
        ଫେରନ୍ତୁ ଖାଲି;
    }

    u16 bdf = pci->bdf;
    u32 orig = pci_config_readl(bdf, PCI_ROM_ADDRESS);
    pci_config_writel(bdf, PCI_ROM_ADDRESS, ~PCI_ROM_ADDRESS_ENABLE);
    u32 sz = pci_config_readl(bdf, PCI_ROM_ADDRESS);

    dprintf(6, "Option rom sizing returned %x %x\n", orig, sz);
    orig &= ~PCI_ROM_ADDRESS_ENABLE;
    ଯଦି (!sz || sz == 0xffffffff)
        ଯାଅ fail;

    ଯଦି (orig == sz || (u32)(orig + 4*1024*1024) < 20*1024*1024) {
        // Don't try to map to a pci addresses at its max, in the last
        // 4MiB of ram, or the first 16MiB of ram.
        dprintf(6, "Preset rom address doesn't look valid\n");
        ଯାଅ fail;
    }

    // Looks like a rom - enable it.
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig | PCI_ROM_ADDRESS_ENABLE);

    ସଂରଚନା rom_header *rom = (ଶୂନ୍ୟ*)orig;
    ପାଇଁ (;;) {
        dprintf(5, "Inspecting possible rom at %p (vd=%04x:%04x bdf=%pP)\n"
                , rom, pci->vendor, pci->device, pci);
        ଯଦି (rom->signature != OPTION_ROM_SIGNATURE) {
            dprintf(6, "No option rom signature (got %x)\n", rom->signature);
            ଯାଅ fail;
        }
        ସଂରଚନା pci_data *pd = get_pci_rom(rom);
        ଯଦି (! pd) {
            dprintf(6, "No valid pci signature found\n");
            ଯାଅ fail;
        }

        ଯଦି (pd->vendor == pci->vendor && pd->device == pci->device
            && pd->type == PCIROM_CODETYPE_X86)
            // A match
            ଭାଙ୍ଗ;
        dprintf(6, "Didn't match dev/ven (got %04x:%04x) or type (got %d)\n"
                , pd->vendor, pd->device, pd->type);
        ଯଦି (pd->indicator & 0x80) {
            dprintf(6, "No more images left\n");
            ଯାଅ fail;
        }
        rom = (ଶୂନ୍ୟ*)((u32)rom + pd->ilen * 512);
    }

    rom = copy_rom(rom);
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig);
    ଫେରନ୍ତୁ rom;
fail:
    // Not valid - restore original and exit.
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig);
    ଫେରନ୍ତୁ ଖାଲି;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା boot_irq_captured(ଶୂନ୍ୟ)
{
    ଫେରନ୍ତୁ GET_IVT(0x19).segoff != FUNC16(entry_19_official).segoff;
}

ସ୍ଥିର ଶୂନ୍ୟ boot_irq_restore(ଶୂନ୍ୟ)
{
    ସଂରଚନା segoff_s seabios;

    seabios = FUNC16(entry_19_official);
    SET_IVT(0x19, seabios);
}

// Attempt to map and initialize the option rom on a given PCI device.
ସ୍ଥିର ଶୂନ୍ୟ
init_pcirom(ସଂରଚନା pci_device *pci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା isvga, u64 *sources)
{
    dprintf(4, "Attempting to init PCI bdf %pP (vd %04x:%04x)\n"
            , pci, pci->vendor, pci->device);

    ଅକ୍ଷର fname[17];
    snprintf(fname, ଆକାର(fname), "pci%04x,%04x.rom"
             , pci->vendor, pci->device);
    ସଂରଚନା romfile_s *file = romfile_find(fname);
    ସଂରଚନା rom_header *rom = ଖାଲି;
    ଯଦି (file)
        rom = deploy_romfile(file);
    ଅନ୍ୟଥା ଯଦି (RunPCIroms > 1 || (RunPCIroms == 1 && isvga))
        rom = map_pcirom(pci);
    ଯଦି (! rom)
        // No ROM present.
        ଫେରନ୍ତୁ;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା irq_was_captured = boot_irq_captured();
    ସଂରଚନା pnp_data *pnp = get_pnp_rom(rom);
    setRomSource(sources, rom, RS_PCIROM | (u32)pci);
    init_optionrom(rom, pci->bdf, isvga);
    ଯଦି (boot_irq_captured() && !irq_was_captured &&
        !file && !isvga && pnp) {
        // This PCI rom is misbehaving - recapture the boot irqs
        ଅକ୍ଷର *desc = MAKE_FLATPTR(FLATPTR_TO_SEG(rom), pnp->productname);
        dprintf(1, "PnP optionrom \"%s\" (bdf %pP) captured int19, restoring\n",
                desc, pci);
        boot_irq_restore();
    }
}


/****************************************************************
 * Non-VGA option rom init
 ****************************************************************/

ଶୂନ୍ୟ
optionrom_setup(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_OPTIONROMS)
        ଫେରନ୍ତୁ;

    dprintf(1, "Scan for option roms\n");
    u64 sources[(BUILD_BIOS_ADDR - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    memset(sources, 0, ଆକାର(sources));
    u32 post_vga = rom_get_last();

    // Find and deploy PCI roms.
    ସଂରଚନା pci_device *pci;
    foreachpci(pci) {
        ଯଦି (pci->class == PCI_CLASS_DISPLAY_VGA ||
            pci->class == PCI_CLASS_DISPLAY_OTHER ||
            pci->have_driver)
            ଜାରି;
        init_pcirom(pci, 0, sources);
    }

    // Find and deploy CBFS roms not associated with a device.
    run_file_roms("genroms/", 0, sources);
    rom_reserve(0);

    // All option roms found and deployed - now build BEV/BCV vectors.

    u32 pos = post_vga;
    ଯେତେବେଳେ (pos < rom_get_last()) {
        ସଂରଚନା rom_header *rom = (ଶୂନ୍ୟ*)pos;
        ଯଦି (! is_valid_rom(rom)) {
            pos += OPTION_ROM_ALIGN;
            ଜାରି;
        }
        pos += ALIGN(rom->size * 512, OPTION_ROM_ALIGN);
        ସଂରଚନା pnp_data *pnp = get_pnp_rom(rom);
        ଯଦି (! pnp) {
            // Legacy rom.
            boot_add_bcv(FLATPTR_TO_SEG(rom), OPTION_ROM_INITVECTOR, 0
                         , getRomPriority(sources, rom, 0));
            ଜାରି;
        }
        // PnP rom - check for BEV and BCV boot capabilities.
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା instance = 0;
        ଯେତେବେଳେ (pnp) {
            ଯଦି (pnp->bev)
                boot_add_bev(FLATPTR_TO_SEG(rom), pnp->bev, pnp->productname
                             , getRomPriority(sources, rom, instance++));
            ଅନ୍ୟଥା ଯଦି (pnp->bcv)
                boot_add_bcv(FLATPTR_TO_SEG(rom), pnp->bcv, pnp->productname
                             , getRomPriority(sources, rom, instance++));
            ଅନ୍ୟଥା
                ଭାଙ୍ଗ;
            pnp = get_pnp_next(rom, pnp);
        }
    }
}


/****************************************************************
 * VGA init
 ****************************************************************/

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ScreenAndDebug;
ସଂରଚନା rom_header *VgaROM;

ସ୍ଥିର ଶୂନ୍ୟ try_setup_display_other(ଶୂନ୍ୟ)
{
    ସଂରଚନା pci_device *pci;

    dprintf(1, "No VGA found, scan for other display\n");

    foreachpci(pci) {
        ଯଦି (pci->class != PCI_CLASS_DISPLAY_OTHER)
            ଜାରି;
        ସଂରଚନା rom_header *rom = map_pcirom(pci);
        ଯଦି (!rom)
            ଜାରି;
        dprintf(1, "Other display found at %pP\n", pci);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_MEMORY);
        init_optionrom(rom, pci->bdf, 1);
        ଫେରନ୍ତୁ;
    }
}

// Call into vga code to turn on console.
ଶୂନ୍ୟ
vgarom_setup(ଶୂନ୍ୟ)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା have_vga = 0;

    ଯଦି (! CONFIG_OPTIONROMS)
        ଫେରନ୍ତୁ;

    dprintf(1, "Scan for VGA option rom\n");

    // Load some config settings that impact VGA.
    EnforceChecksum = romfile_loadint("etc/optionroms-checksum", 1);
    S3ResumeVga = romfile_loadint("etc/s3-resume-vga-init", CONFIG_QEMU);
    RunPCIroms = romfile_loadint("etc/pci-optionrom-exec", 2);
    ScreenAndDebug = romfile_loadint("etc/screen-and-debug", 1);

    // Clear option rom memory
    memset((ଶୂନ୍ୟ*)BUILD_ROM_START, 0, rom_get_max() - BUILD_ROM_START);

    // Find and deploy PCI VGA rom.
    ସଂରଚନା pci_device *pci;
    foreachpci(pci) {
        ଯଦି (!is_pci_vga(pci))
            ଜାରି;
        vgahook_setup(pci);
        init_pcirom(pci, 1, ଖାଲି);
        have_vga = 1;
        ଭାଙ୍ଗ;
    }
    ଯଦି (!have_vga)
        try_setup_display_other();

    // Find and deploy CBFS vga-style roms not associated with a device.
    run_file_roms("vgaroms/", 1, ଖାଲି);
    rom_reserve(0);

    ଯଦି (rom_get_last() != BUILD_ROM_START)
        // VGA rom found
        VgaROM = (ଶୂନ୍ୟ*)BUILD_ROM_START;
}

ଶୂନ୍ୟ
s3_resume_vga(ଶୂନ୍ୟ)
{
    ଯଦି (!S3ResumeVga)
        ଫେରନ୍ତୁ;
    ଯଦି (!VgaROM || ! is_valid_rom(VgaROM))
        ଫେରନ୍ତୁ;
    callrom(VgaROM, 0);
}

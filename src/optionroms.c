// Option rom scanning code.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "farptr.h" // FLATPTR_TO_SEG
#అన్తర్భావ "biosvar.h" // GET_IVT
#అన్తర్భావ "hw/pci.h" // pci_config_readl
#అన్తర్భావ "hw/pcidevice.h" // foreachpci
#అన్తర్భావ "hw/pci_ids.h" // PCI_CLASS_DISPLAY_VGA
#అన్తర్భావ "hw/pci_regs.h" // PCI_ROM_ADDRESS
#అన్తర్భావ "malloc.h" // rom_confirm
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "romfile.h" // romfile_loadint
#అన్తర్భావ "stacks.h" // farcall16big
#అన్తర్భావ "std/optionrom.h" // struct rom_header
#అన్తర్భావ "std/pnpbios.h" // PNP_SIGNATURE
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // get_pnp_offset
#అన్తర్భావ "tcgbios.h" // tpm_*

స్థిర పూర్ణాంకం EnforceChecksum, S3ResumeVga, నిరంతరం_PCIroms;


/****************************************************************
 * Helper functions
 ****************************************************************/

// Execute a given option rom.
స్థిర శూన్య
__callrom(నిర్మాణం rom_శీర్షిక *rom, u16 స్థానభ్రంశం_2, u16 bdf)
{
    u16 seg = FLATPTR_గమ్యం_SEG(rom);
    dprintf(1, "Running option rom at %04x:%04x\n", seg, స్థానభ్రంశం_2);

    నిర్మాణం bregs br;
    memset(&br, 0, పరిమాణం(br));
    br.flags = F_IF;
    br.ax = bdf;
    br.bx = 0xffff;
    br.dx = 0xffff;
    br.es = SEG_BIOS;
    br.di = get_pnp_స్థానభ్రంశం();
    br.సంకేతం_2 = SEGOFF(seg, స్థానభ్రంశం_2);
    ప్రారంభం_preempt();
    farcall16big(&br);
    ముగింపు_preempt();
}

// Execute a given option rom at the standard entry vector.
శూన్య
callrom(నిర్మాణం rom_శీర్షిక *rom, u16 bdf)
{
    __callrom(rom, OPTION_ROM_INITVECTOR, bdf);
}

// Execute a BCV option rom registered via add_bcv().
శూన్య
call_bcv(u16 seg, u16 ip)
{
    __callrom(MAKE_FLATPTR(seg, 0), ip, 0);
}

// Verify that an option rom looks valid
స్థిర పూర్ణాంకం
is_valid_rom(నిర్మాణం rom_శీర్షిక *rom)
{
    dprintf(6, "Checking rom %p (sig %x size %d)\n"
            , rom, rom->signature, rom->కొలత);
    ఒకవేళ (rom->signature != OPTION_ROM_SIGNATURE)
        తిరిగి 0;
    ఒకవేళ (! rom->కొలత)
        తిరిగి 0;
    u32 len = rom->కొలత * 512;
    u8 కూడిక = checksum(rom, len);
    ఒకవేళ (కూడిక != 0) {
        dprintf(1, "Found option rom with bad checksum: loc=%p len=%d sum=%x\n"
                , rom, len, కూడిక);
        ఒకవేళ (EnforceChecksum)
            తిరిగి 0;
    }
    తిరిగి 1;
}

// Check if a valid option rom has a pnp struct; return it if so.
స్థిర నిర్మాణం pnp_దత్తాంశం *
get_pnp_rom(నిర్మాణం rom_శీర్షిక *rom)
{
    నిర్మాణం pnp_దత్తాంశం *pnp = (శూన్య*)((u8*)rom + rom->pnpoffset);
    ఒకవేళ (pnp->signature != PNP_SIGNATURE)
        తిరిగి ఖాళీ;
    తిరిగి pnp;
}

// Check for multiple pnp option rom headers.
స్థిర నిర్మాణం pnp_దత్తాంశం *
get_pnp_తదుపరి(నిర్మాణం rom_శీర్షిక *rom, నిర్మాణం pnp_దత్తాంశం *pnp)
{
    ఒకవేళ (! pnp->nextoffset)
        తిరిగి ఖాళీ;
    pnp = (శూన్య*)((u8*)rom + pnp->nextoffset);
    ఒకవేళ (pnp->signature != PNP_SIGNATURE)
        తిరిగి ఖాళీ;
    తిరిగి pnp;
}

// Check if a valid option rom has a pci struct; return it if so.
స్థిర నిర్మాణం pci_దత్తాంశం *
get_pci_rom(నిర్మాణం rom_శీర్షిక *rom)
{
    నిర్మాణం pci_దత్తాంశం *pd = (శూన్య*)((u32)rom + rom->pcioffset);
    ఒకవేళ (pd->signature != PCI_ROM_SIGNATURE)
        తిరిగి ఖాళీ;
    ఒకవేళ (rom->pcioffset & 3)
        dprintf(1, "WARNING! Found unaligned PCI rom (vd=%04x:%04x)\n"
                , pd->vendor, pd->device);
    తిరిగి pd;
}

// Run rom init code and note rom size.
స్థిర పూర్ణాంకం
init_optionrom(నిర్మాణం rom_శీర్షిక *rom, u16 bdf, పూర్ణాంకం isvga)
{
    ఒకవేళ (! is_valid_rom(rom))
        తిరిగి -1;
    నిర్మాణం rom_శీర్షిక *newrom = rom_reserve(rom->కొలత * 512);
    ఒకవేళ (!newrom) {
        warn_noalloc();
        తిరిగి -1;
    }
    ఒకవేళ (newrom != rom)
        memmove(newrom, rom, rom->కొలత * 512);

    tpm_option_rom(newrom, rom->కొలత * 512);

    ఒకవేళ (isvga || get_pnp_rom(newrom))
        // Only init vga and PnP roms here.
        callrom(newrom, bdf);

    తిరిగి rom_confirm(newrom->కొలత * 512);
}

#నిర్వచించు RS_PCIROM (1LL<<33)

స్థిర శూన్య
సమితి_Rom_ఆధారం(u64 *sources, నిర్మాణం rom_శీర్షిక *rom, u64 ఆధారం)
{
    ఒకవేళ (sources)
        sources[((u32)rom - BUILD_ROM_ప్రారంభం) / OPTION_ROM_ALIGN] = ఆధారం;
}

స్థిర పూర్ణాంకం
get_Rom_ప్రాధాన్యత(u64 *sources, నిర్మాణం rom_శీర్షిక *rom, పూర్ణాంకం instance)
{
    u64 ఆధారం = sources[((u32)rom - BUILD_ROM_ప్రారంభం) / OPTION_ROM_ALIGN];
    ఒకవేళ (!ఆధారం)
        తిరిగి -1;
    ఒకవేళ (ఆధారం & RS_PCIROM)
        తిరిగి bootprio_కనుగొను_pci_rom((శూన్య*)(u32)ఆధారం, instance);
    నిర్మాణం romfile_s *file = (శూన్య*)(u32)ఆధారం;
    తిరిగి bootprio_కనుగొను_named_rom(file->name, instance);
}


/****************************************************************
 * Roms in CBFS
 ****************************************************************/

స్థిర నిర్మాణం rom_శీర్షిక *
deploy_romfile(నిర్మాణం romfile_s *file)
{
    u32 కొలత = file->కొలత;
    నిర్మాణం rom_శీర్షిక *rom = rom_reserve(కొలత);
    ఒకవేళ (!rom) {
        warn_noalloc();
        తిరిగి ఖాళీ;
    }
    పూర్ణాంకం ret = file->copy(file, rom, కొలత);
    ఒకవేళ (ret <= 0)
        తిరిగి ఖాళీ;
    తిరిగి rom;
}

// Run all roms in a given CBFS directory.
స్థిర శూన్య
నిరంతరం_file_roms(స్థిరరాశి అక్షరం *ఉపసర్గ, పూర్ణాంకం isvga, u64 *sources)
{
    నిర్మాణం romfile_s *file = ఖాళీ;
    కొరకు (;;) {
        file = romfile_findprefix(ఉపసర్గ, file);
        ఒకవేళ (!file)
            ఆపు;
        నిర్మాణం rom_శీర్షిక *rom = deploy_romfile(file);
        ఒకవేళ (rom) {
            సమితి_Rom_ఆధారం(sources, rom, (u32)file);
            init_optionrom(rom, 0, isvga);
        }
    }
}


/****************************************************************
 * PCI roms
 ****************************************************************/

// Verify device is a vga device with legacy address decoding enabled.
పూర్ణాంకం
is_pci_vga(నిర్మాణం pci_device *pci)
{
    ఒకవేళ (pci->class != PCI_CLASS_DISPLAY_VGA)
        తిరిగి 0;
    u16 cmd = pci_config_readw(pci->bdf, PCI_COMMAND);
    ఒకవేళ (!(cmd & PCI_COMMAND_IO && cmd & PCI_COMMAND_జ్ఞాపకం))
        తిరిగి 0;
    అయితే (pci->మాతృ) {
        pci = pci->మాతృ;
        u32 ctrl = pci_config_readb(pci->bdf, PCI_BRIDGE_CONTROL);
        ఒకవేళ (!(ctrl & PCI_BRIDGE_CTL_VGA))
            తిరిగి 0;
    }
    తిరిగి 1;
}

// Copy a rom to its permanent location below 1MiB
స్థిర నిర్మాణం rom_శీర్షిక *
copy_rom(నిర్మాణం rom_శీర్షిక *rom)
{
    u32 romsize = rom->కొలత * 512;
    నిర్మాణం rom_శీర్షిక *newrom = rom_reserve(romsize);
    ఒకవేళ (!newrom) {
        warn_noalloc();
        తిరిగి ఖాళీ;
    }
    dprintf(4, "Copying option rom (size %d) from %p to %p\n"
            , romsize, rom, newrom);
    iomemcpy(newrom, rom, romsize);
    తిరిగి newrom;
}

// Map the option rom of a given PCI device.
స్థిర నిర్మాణం rom_శీర్షిక *
map_pcirom(నిర్మాణం pci_device *pci)
{
    dprintf(6, "Attempting to map option rom on dev %pP\n", pci);

    ఒకవేళ ((pci->శీర్షిక_type & 0x7f) != PCI_శీర్షిక_TYPE_సాధారణ) {
        dprintf(6, "Skipping non-normal pci device (type=%x)\n"
                , pci->శీర్షిక_type);
        తిరిగి ఖాళీ;
    }

    u16 bdf = pci->bdf;
    u32 orig = pci_config_readl(bdf, PCI_ROM_చిరునామా);
    pci_config_writel(bdf, PCI_ROM_చిరునామా, ~PCI_ROM_చిరునామా_ENABLE);
    u32 sz = pci_config_readl(bdf, PCI_ROM_చిరునామా);

    dprintf(6, "Option rom sizing returned %x %x\n", orig, sz);
    orig &= ~PCI_ROM_చిరునామా_ENABLE;
    ఒకవేళ (!sz || sz == 0xffffffff)
        వెళ్ళు fail;

    ఒకవేళ (orig == sz || (u32)(orig + 4*1024*1024) < 20*1024*1024) {
        // Don't try to map to a pci addresses at its max, in the last
        // 4MiB of ram, or the first 16MiB of ram.
        dprintf(6, "Preset rom address doesn't look valid\n");
        వెళ్ళు fail;
    }

    // Looks like a rom - enable it.
    pci_config_writel(bdf, PCI_ROM_చిరునామా, orig | PCI_ROM_చిరునామా_ENABLE);

    నిర్మాణం rom_శీర్షిక *rom = (శూన్య*)orig;
    కొరకు (;;) {
        dprintf(5, "Inspecting possible rom at %p (vd=%04x:%04x bdf=%pP)\n"
                , rom, pci->vendor, pci->device, pci);
        ఒకవేళ (rom->signature != OPTION_ROM_SIGNATURE) {
            dprintf(6, "No option rom signature (got %x)\n", rom->signature);
            వెళ్ళు fail;
        }
        నిర్మాణం pci_దత్తాంశం *pd = get_pci_rom(rom);
        ఒకవేళ (! pd) {
            dprintf(6, "No valid pci signature found\n");
            వెళ్ళు fail;
        }

        ఒకవేళ (pd->vendor == pci->vendor && pd->device == pci->device
            && pd->type == PCIROM_CODETYPE_X86)
            // A match
            ఆపు;
        dprintf(6, "Didn't match dev/ven (got %04x:%04x) or type (got %d)\n"
                , pd->vendor, pd->device, pd->type);
        ఒకవేళ (pd->indicator & 0x80) {
            dprintf(6, "No more images left\n");
            వెళ్ళు fail;
        }
        rom = (శూన్య*)((u32)rom + pd->ilen * 512);
    }

    rom = copy_rom(rom);
    pci_config_writel(bdf, PCI_ROM_చిరునామా, orig);
    తిరిగి rom;
fail:
    // Not valid - restore original and exit.
    pci_config_writel(bdf, PCI_ROM_చిరునామా, orig);
    తిరిగి ఖాళీ;
}

స్థిర పూర్ణాంకం boot_irq_captured(శూన్య)
{
    తిరిగి GET_IVT(0x19).segoff != FUNC16(నమోదు_19_official).segoff;
}

స్థిర శూన్య boot_irq_restore(శూన్య)
{
    నిర్మాణం segoff_s seabios;

    seabios = FUNC16(నమోదు_19_official);
    సమితి_IVT(0x19, seabios);
}

// Attempt to map and initialize the option rom on a given PCI device.
స్థిర శూన్య
init_pcirom(నిర్మాణం pci_device *pci, పూర్ణాంకం isvga, u64 *sources)
{
    dprintf(4, "Attempting to init PCI bdf %pP (vd %04x:%04x)\n"
            , pci, pci->vendor, pci->device);

    అక్షరం fname[17];
    snprintf(fname, పరిమాణం(fname), "pci%04x,%04x.rom"
             , pci->vendor, pci->device);
    నిర్మాణం romfile_s *file = romfile_కనుగొను(fname);
    నిర్మాణం rom_శీర్షిక *rom = ఖాళీ;
    ఒకవేళ (file)
        rom = deploy_romfile(file);
    లేకపోతే ఒకవేళ (నిరంతరం_PCIroms > 1 || (నిరంతరం_PCIroms == 1 && isvga))
        rom = map_pcirom(pci);
    ఒకవేళ (! rom)
        // No ROM present.
        తిరిగి;
    పూర్ణాంకం irq_was_captured = boot_irq_captured();
    నిర్మాణం pnp_దత్తాంశం *pnp = get_pnp_rom(rom);
    సమితి_Rom_ఆధారం(sources, rom, RS_PCIROM | (u32)pci);
    init_optionrom(rom, pci->bdf, isvga);
    ఒకవేళ (boot_irq_captured() && !irq_was_captured &&
        !file && !isvga && pnp) {
        // This PCI rom is misbehaving - recapture the boot irqs
        అక్షరం *desc = MAKE_FLATPTR(FLATPTR_గమ్యం_SEG(rom), pnp->productname);
        dprintf(1, "PnP optionrom \"%s\" (bdf %pP) captured int19, restoring\n",
                desc, pci);
        boot_irq_restore();
    }
}


/****************************************************************
 * Non-VGA option rom init
 ****************************************************************/

శూన్య
optionrom_అమరిక(శూన్య)
{
    ఒకవేళ (! CONFIG_OPTIONROMS)
        తిరిగి;

    dprintf(1, "Scan for option roms\n");
    u64 sources[(BUILD_BIOS_ADDR - BUILD_ROM_ప్రారంభం) / OPTION_ROM_ALIGN];
    memset(sources, 0, పరిమాణం(sources));
    u32 post_vga = rom_get_చివరి();

    // Find and deploy PCI roms.
    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (pci->class == PCI_CLASS_DISPLAY_VGA ||
            pci->class == PCI_CLASS_DISPLAY_OTHER ||
            pci->have_driver)
            కొనసాగు;
        init_pcirom(pci, 0, sources);
    }

    // Find and deploy CBFS roms not associated with a device.
    నిరంతరం_file_roms("genroms/", 0, sources);
    rom_reserve(0);

    // All option roms found and deployed - now build BEV/BCV vectors.

    u32 pos = post_vga;
    అయితే (pos < rom_get_చివరి()) {
        నిర్మాణం rom_శీర్షిక *rom = (శూన్య*)pos;
        ఒకవేళ (! is_valid_rom(rom)) {
            pos += OPTION_ROM_ALIGN;
            కొనసాగు;
        }
        pos += ALIGN(rom->కొలత * 512, OPTION_ROM_ALIGN);
        నిర్మాణం pnp_దత్తాంశం *pnp = get_pnp_rom(rom);
        ఒకవేళ (! pnp) {
            // Legacy rom.
            boot_కూడు_bcv(FLATPTR_గమ్యం_SEG(rom), OPTION_ROM_INITVECTOR, 0
                         , get_Rom_ప్రాధాన్యత(sources, rom, 0));
            కొనసాగు;
        }
        // PnP rom - check for BEV and BCV boot capabilities.
        పూర్ణాంకం instance = 0;
        అయితే (pnp) {
            ఒకవేళ (pnp->bev)
                boot_కూడు_bev(FLATPTR_గమ్యం_SEG(rom), pnp->bev, pnp->productname
                             , get_Rom_ప్రాధాన్యత(sources, rom, instance++));
            లేకపోతే ఒకవేళ (pnp->bcv)
                boot_కూడు_bcv(FLATPTR_గమ్యం_SEG(rom), pnp->bcv, pnp->productname
                             , get_Rom_ప్రాధాన్యత(sources, rom, instance++));
            లేకపోతే
                ఆపు;
            pnp = get_pnp_తదుపరి(rom, pnp);
        }
    }
}


/****************************************************************
 * VGA init
 ****************************************************************/

పూర్ణాంకం ScreenAndDebug;
నిర్మాణం rom_శీర్షిక *VgaROM;

స్థిర శూన్య try_అమరిక_display_other(శూన్య)
{
    నిర్మాణం pci_device *pci;

    dprintf(1, "No VGA found, scan for other display\n");

    foreachpci(pci) {
        ఒకవేళ (pci->class != PCI_CLASS_DISPLAY_OTHER)
            కొనసాగు;
        నిర్మాణం rom_శీర్షిక *rom = map_pcirom(pci);
        ఒకవేళ (!rom)
            కొనసాగు;
        dprintf(1, "Other display found at %pP\n", pci);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_జ్ఞాపకం);
        init_optionrom(rom, pci->bdf, 1);
        తిరిగి;
    }
}

// Call into vga code to turn on console.
శూన్య
vgarom_అమరిక(శూన్య)
{
    పూర్ణాంకం have_vga = 0;

    ఒకవేళ (! CONFIG_OPTIONROMS)
        తిరిగి;

    dprintf(1, "Scan for VGA option rom\n");

    // Load some config settings that impact VGA.
    EnforceChecksum = romfile_loadint("etc/optionroms-checksum", 1);
    S3ResumeVga = romfile_loadint("etc/s3-resume-vga-init", CONFIG_QEMU);
    నిరంతరం_PCIroms = romfile_loadint("etc/pci-optionrom-exec", 2);
    ScreenAndDebug = romfile_loadint("etc/screen-and-debug", 1);

    // Clear option rom memory
    memset((శూన్య*)BUILD_ROM_ప్రారంభం, 0, rom_get_max() - BUILD_ROM_ప్రారంభం);

    // Find and deploy PCI VGA rom.
    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (!is_pci_vga(pci))
            కొనసాగు;
        vgahook_అమరిక(pci);
        init_pcirom(pci, 1, ఖాళీ);
        have_vga = 1;
        ఆపు;
    }
    ఒకవేళ (!have_vga)
        try_అమరిక_display_other();

    // Find and deploy CBFS vga-style roms not associated with a device.
    నిరంతరం_file_roms("vgaroms/", 1, ఖాళీ);
    rom_reserve(0);

    ఒకవేళ (rom_get_చివరి() != BUILD_ROM_ప్రారంభం)
        // VGA rom found
        VgaROM = (శూన్య*)BUILD_ROM_ప్రారంభం;
}

శూన్య
s3_resume_vga(శూన్య)
{
    ఒకవేళ (!S3ResumeVga)
        తిరిగి;
    ఒకవేళ (!VgaROM || ! is_valid_rom(VgaROM))
        తిరిగి;
    callrom(VgaROM, 0);
}

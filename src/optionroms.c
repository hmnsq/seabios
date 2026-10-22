// Option rom scanning code.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "farptr.h" // FLATPTR_TO_SEG
#अन्तर्भाव "biosvar.h" // GET_IVT
#अन्तर्भाव "hw/pci.h" // pci_config_readl
#अन्तर्भाव "hw/pcidevice.h" // foreachpci
#अन्तर्भाव "hw/pci_ids.h" // PCI_CLASS_DISPLAY_VGA
#अन्तर्भाव "hw/pci_regs.h" // PCI_ROM_ADDRESS
#अन्तर्भाव "malloc.h" // rom_confirm
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "romfile.h" // romfile_loadint
#अन्तर्भाव "stacks.h" // farcall16big
#अन्तर्भाव "std/optionrom.h" // struct rom_header
#अन्तर्भाव "std/pnpbios.h" // PNP_SIGNATURE
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // get_pnp_offset
#अन्तर्भाव "tcgbios.h" // tpm_*

स्थिर पूर्णाङ्क EnforceChecksum, S3ResumeVga, सततम्_PCIroms;


/****************************************************************
 * Helper functions
 ****************************************************************/

// Execute a given option rom.
स्थिर शून्य
__callrom(संरचना rom_शीर्षकम् *rom, u16 विस्थापनम्_2, u16 bdf)
{
    u16 seg = FLATPTR_गन्तव्यम्_SEG(rom);
    dprintf(1, "Running option rom at %04x:%04x\n", seg, विस्थापनम्_2);

    संरचना bregs br;
    स्मृतिं_पूरय(&br, 0, परिमाण(br));
    br.flags = F_IF;
    br.ax = bdf;
    br.bx = 0xffff;
    br.dx = 0xffff;
    br.es = SEG_BIOS;
    br.di = get_pnp_विस्थापनम्();
    br.संहिता = SEGOFF(seg, विस्थापनम्_2);
    start_preempt();
    farcall16big(&br);
    finish_preempt();
}

// Execute a given option rom at the standard entry vector.
शून्य
callrom(संरचना rom_शीर्षकम् *rom, u16 bdf)
{
    __callrom(rom, OPTION_ROM_INITVECTOR, bdf);
}

// Execute a BCV option rom registered via add_bcv().
शून्य
call_bcv(u16 seg, u16 ip)
{
    __callrom(MAKE_FLATPTR(seg, 0), ip, 0);
}

// Verify that an option rom looks valid
स्थिर पूर्णाङ्क
is_valid_rom(संरचना rom_शीर्षकम् *rom)
{
    dprintf(6, "Checking rom %p (sig %x size %d)\n"
            , rom, rom->signature, rom->आकार);
    यदि (rom->signature != OPTION_ROM_SIGNATURE)
        प्रत्यागम 0;
    यदि (! rom->आकार)
        प्रत्यागम 0;
    u32 len = rom->आकार * 512;
    u8 योग = checksum(rom, len);
    यदि (योग != 0) {
        dprintf(1, "Found option rom with bad checksum: loc=%p len=%d sum=%x\n"
                , rom, len, योग);
        यदि (EnforceChecksum)
            प्रत्यागम 0;
    }
    प्रत्यागम 1;
}

// Check if a valid option rom has a pnp struct; return it if so.
स्थिर संरचना pnp_दत्त *
get_pnp_rom(संरचना rom_शीर्षकम् *rom)
{
    संरचना pnp_दत्त *pnp = (शून्य*)((u8*)rom + rom->pnpoffset);
    यदि (pnp->signature != PNP_SIGNATURE)
        प्रत्यागम रिक्त;
    प्रत्यागम pnp;
}

// Check for multiple pnp option rom headers.
स्थिर संरचना pnp_दत्त *
get_pnp_अग्र(संरचना rom_शीर्षकम् *rom, संरचना pnp_दत्त *pnp)
{
    यदि (! pnp->nextoffset)
        प्रत्यागम रिक्त;
    pnp = (शून्य*)((u8*)rom + pnp->nextoffset);
    यदि (pnp->signature != PNP_SIGNATURE)
        प्रत्यागम रिक्त;
    प्रत्यागम pnp;
}

// Check if a valid option rom has a pci struct; return it if so.
स्थिर संरचना pci_दत्त *
get_pci_rom(संरचना rom_शीर्षकम् *rom)
{
    संरचना pci_दत्त *pd = (शून्य*)((u32)rom + rom->pcioffset);
    यदि (pd->signature != PCI_ROM_SIGNATURE)
        प्रत्यागम रिक्त;
    यदि (rom->pcioffset & 3)
        dprintf(1, "WARNING! Found unaligned PCI rom (vd=%04x:%04x)\n"
                , pd->vendor, pd->device);
    प्रत्यागम pd;
}

// Run rom init code and note rom size.
स्थिर पूर्णाङ्क
init_optionrom(संरचना rom_शीर्षकम् *rom, u16 bdf, पूर्णाङ्क isvga)
{
    यदि (! is_valid_rom(rom))
        प्रत्यागम -1;
    संरचना rom_शीर्षकम् *newrom = rom_reserve(rom->आकार * 512);
    यदि (!newrom) {
        warn_noalloc();
        प्रत्यागम -1;
    }
    यदि (newrom != rom)
        memmove(newrom, rom, rom->आकार * 512);

    tpm_option_rom(newrom, rom->आकार * 512);

    यदि (isvga || get_pnp_rom(newrom))
        // Only init vga and PnP roms here.
        callrom(newrom, bdf);

    प्रत्यागम rom_confirm(newrom->आकार * 512);
}

#परिभाषय RS_PCIROM (1LL<<33)

स्थिर शून्य
समुच्चयः_Rom_Source(u64 *sources, संरचना rom_शीर्षकम् *rom, u64 source)
{
    यदि (sources)
        sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN] = source;
}

स्थिर पूर्णाङ्क
get_Rom_प्राथमिकता(u64 *sources, संरचना rom_शीर्षकम् *rom, पूर्णाङ्क instance)
{
    u64 source = sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    यदि (!source)
        प्रत्यागम -1;
    यदि (source & RS_PCIROM)
        प्रत्यागम bootprio_find_pci_rom((शून्य*)(u32)source, instance);
    संरचना romfile_s *file = (शून्य*)(u32)source;
    प्रत्यागम bootprio_find_named_rom(file->name, instance);
}


/****************************************************************
 * Roms in CBFS
 ****************************************************************/

स्थिर संरचना rom_शीर्षकम् *
deploy_romfile(संरचना romfile_s *file)
{
    u32 आकार = file->आकार;
    संरचना rom_शीर्षकम् *rom = rom_reserve(आकार);
    यदि (!rom) {
        warn_noalloc();
        प्रत्यागम रिक्त;
    }
    पूर्णाङ्क ret = file->copy(file, rom, आकार);
    यदि (ret <= 0)
        प्रत्यागम रिक्त;
    प्रत्यागम rom;
}

// Run all roms in a given CBFS directory.
स्थिर शून्य
सततम्_file_roms(ध्रुव वर्ण *उपसर्गः, पूर्णाङ्क isvga, u64 *sources)
{
    संरचना romfile_s *file = रिक्त;
    प्रति (;;) {
        file = romfile_findprefix(उपसर्गः, file);
        यदि (!file)
            भङ्ग;
        संरचना rom_शीर्षकम् *rom = deploy_romfile(file);
        यदि (rom) {
            समुच्चयः_Rom_Source(sources, rom, (u32)file);
            init_optionrom(rom, 0, isvga);
        }
    }
}


/****************************************************************
 * PCI roms
 ****************************************************************/

// Verify device is a vga device with legacy address decoding enabled.
पूर्णाङ्क
is_pci_vga(संरचना pci_device *pci)
{
    यदि (pci->class != PCI_CLASS_DISPLAY_VGA)
        प्रत्यागम 0;
    u16 cmd = pci_config_readw(pci->bdf, PCI_COMMAND);
    यदि (!(cmd & PCI_COMMAND_IO && cmd & PCI_COMMAND_स्मृतिः))
        प्रत्यागम 0;
    यावत् (pci->parent) {
        pci = pci->parent;
        u32 ctrl = pci_config_readb(pci->bdf, PCI_BRIDGE_CONTROL);
        यदि (!(ctrl & PCI_BRIDGE_CTL_VGA))
            प्रत्यागम 0;
    }
    प्रत्यागम 1;
}

// Copy a rom to its permanent location below 1MiB
स्थिर संरचना rom_शीर्षकम् *
copy_rom(संरचना rom_शीर्षकम् *rom)
{
    u32 romsize = rom->आकार * 512;
    संरचना rom_शीर्षकम् *newrom = rom_reserve(romsize);
    यदि (!newrom) {
        warn_noalloc();
        प्रत्यागम रिक्त;
    }
    dprintf(4, "Copying option rom (size %d) from %p to %p\n"
            , romsize, rom, newrom);
    iomemcpy(newrom, rom, romsize);
    प्रत्यागम newrom;
}

// Map the option rom of a given PCI device.
स्थिर संरचना rom_शीर्षकम् *
map_pcirom(संरचना pci_device *pci)
{
    dprintf(6, "Attempting to map option rom on dev %pP\n", pci);

    यदि ((pci->शीर्षकम्_type & 0x7f) != PCI_शीर्षकम्_TYPE_प्रसामान्यम्) {
        dprintf(6, "Skipping non-normal pci device (type=%x)\n"
                , pci->शीर्षकम्_type);
        प्रत्यागम रिक्त;
    }

    u16 bdf = pci->bdf;
    u32 orig = pci_config_readl(bdf, PCI_ROM_स्थानाङ्कः);
    pci_config_writel(bdf, PCI_ROM_स्थानाङ्कः, ~PCI_ROM_स्थानाङ्कः_ENABLE);
    u32 sz = pci_config_readl(bdf, PCI_ROM_स्थानाङ्कः);

    dprintf(6, "Option rom sizing returned %x %x\n", orig, sz);
    orig &= ~PCI_ROM_स्थानाङ्कः_ENABLE;
    यदि (!sz || sz == 0xffffffff)
        गच्छ fail;

    यदि (orig == sz || (u32)(orig + 4*1024*1024) < 20*1024*1024) {
        // Don't try to map to a pci addresses at its max, in the last
        // 4MiB of ram, or the first 16MiB of ram.
        dprintf(6, "Preset rom address doesn't look valid\n");
        गच्छ fail;
    }

    // Looks like a rom - enable it.
    pci_config_writel(bdf, PCI_ROM_स्थानाङ्कः, orig | PCI_ROM_स्थानाङ्कः_ENABLE);

    संरचना rom_शीर्षकम् *rom = (शून्य*)orig;
    प्रति (;;) {
        dprintf(5, "Inspecting possible rom at %p (vd=%04x:%04x bdf=%pP)\n"
                , rom, pci->vendor, pci->device, pci);
        यदि (rom->signature != OPTION_ROM_SIGNATURE) {
            dprintf(6, "No option rom signature (got %x)\n", rom->signature);
            गच्छ fail;
        }
        संरचना pci_दत्त *pd = get_pci_rom(rom);
        यदि (! pd) {
            dprintf(6, "No valid pci signature found\n");
            गच्छ fail;
        }

        यदि (pd->vendor == pci->vendor && pd->device == pci->device
            && pd->type == PCIROM_CODETYPE_X86)
            // A match
            भङ्ग;
        dprintf(6, "Didn't match dev/ven (got %04x:%04x) or type (got %d)\n"
                , pd->vendor, pd->device, pd->type);
        यदि (pd->indicator & 0x80) {
            dprintf(6, "No more images left\n");
            गच्छ fail;
        }
        rom = (शून्य*)((u32)rom + pd->ilen * 512);
    }

    rom = copy_rom(rom);
    pci_config_writel(bdf, PCI_ROM_स्थानाङ्कः, orig);
    प्रत्यागम rom;
fail:
    // Not valid - restore original and exit.
    pci_config_writel(bdf, PCI_ROM_स्थानाङ्कः, orig);
    प्रत्यागम रिक्त;
}

स्थिर पूर्णाङ्क boot_irq_captured(शून्य)
{
    प्रत्यागम GET_IVT(0x19).segoff != FUNC16(प्रविष्टिः_19_official).segoff;
}

स्थिर शून्य boot_irq_restore(शून्य)
{
    संरचना segoff_s seabios;

    seabios = FUNC16(प्रविष्टिः_19_official);
    समुच्चयः_IVT(0x19, seabios);
}

// Attempt to map and initialize the option rom on a given PCI device.
स्थिर शून्य
init_pcirom(संरचना pci_device *pci, पूर्णाङ्क isvga, u64 *sources)
{
    dprintf(4, "Attempting to init PCI bdf %pP (vd %04x:%04x)\n"
            , pci, pci->vendor, pci->device);

    वर्ण fname[17];
    snprintf(fname, परिमाण(fname), "pci%04x,%04x.rom"
             , pci->vendor, pci->device);
    संरचना romfile_s *file = romfile_find(fname);
    संरचना rom_शीर्षकम् *rom = रिक्त;
    यदि (file)
        rom = deploy_romfile(file);
    अन्यथा यदि (सततम्_PCIroms > 1 || (सततम्_PCIroms == 1 && isvga))
        rom = map_pcirom(pci);
    यदि (! rom)
        // No ROM present.
        प्रत्यागम;
    पूर्णाङ्क irq_was_captured = boot_irq_captured();
    संरचना pnp_दत्त *pnp = get_pnp_rom(rom);
    समुच्चयः_Rom_Source(sources, rom, RS_PCIROM | (u32)pci);
    init_optionrom(rom, pci->bdf, isvga);
    यदि (boot_irq_captured() && !irq_was_captured &&
        !file && !isvga && pnp) {
        // This PCI rom is misbehaving - recapture the boot irqs
        वर्ण *desc = MAKE_FLATPTR(FLATPTR_गन्तव्यम्_SEG(rom), pnp->productname);
        dprintf(1, "PnP optionrom \"%s\" (bdf %pP) captured int19, restoring\n",
                desc, pci);
        boot_irq_restore();
    }
}


/****************************************************************
 * Non-VGA option rom init
 ****************************************************************/

शून्य
optionrom_सज्जता(शून्य)
{
    यदि (! CONFIG_OPTIONROMS)
        प्रत्यागम;

    dprintf(1, "Scan for option roms\n");
    u64 sources[(BUILD_BIOS_ADDR - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    स्मृतिं_पूरय(sources, 0, परिमाण(sources));
    u32 post_vga = rom_get_चरमः();

    // Find and deploy PCI roms.
    संरचना pci_device *pci;
    foreachpci(pci) {
        यदि (pci->class == PCI_CLASS_DISPLAY_VGA ||
            pci->class == PCI_CLASS_DISPLAY_OTHER ||
            pci->have_driver)
            अनुवर्त;
        init_pcirom(pci, 0, sources);
    }

    // Find and deploy CBFS roms not associated with a device.
    सततम्_file_roms("genroms/", 0, sources);
    rom_reserve(0);

    // All option roms found and deployed - now build BEV/BCV vectors.

    u32 pos = post_vga;
    यावत् (pos < rom_get_चरमः()) {
        संरचना rom_शीर्षकम् *rom = (शून्य*)pos;
        यदि (! is_valid_rom(rom)) {
            pos += OPTION_ROM_ALIGN;
            अनुवर्त;
        }
        pos += ALIGN(rom->आकार * 512, OPTION_ROM_ALIGN);
        संरचना pnp_दत्त *pnp = get_pnp_rom(rom);
        यदि (! pnp) {
            // Legacy rom.
            boot_सङ्कलय_bcv(FLATPTR_गन्तव्यम्_SEG(rom), OPTION_ROM_INITVECTOR, 0
                         , get_Rom_प्राथमिकता(sources, rom, 0));
            अनुवर्त;
        }
        // PnP rom - check for BEV and BCV boot capabilities.
        पूर्णाङ्क instance = 0;
        यावत् (pnp) {
            यदि (pnp->bev)
                boot_सङ्कलय_bev(FLATPTR_गन्तव्यम्_SEG(rom), pnp->bev, pnp->productname
                             , get_Rom_प्राथमिकता(sources, rom, instance++));
            अन्यथा यदि (pnp->bcv)
                boot_सङ्कलय_bcv(FLATPTR_गन्तव्यम्_SEG(rom), pnp->bcv, pnp->productname
                             , get_Rom_प्राथमिकता(sources, rom, instance++));
            अन्यथा
                भङ्ग;
            pnp = get_pnp_अग्र(rom, pnp);
        }
    }
}


/****************************************************************
 * VGA init
 ****************************************************************/

पूर्णाङ्क ScreenAndDebug;
संरचना rom_शीर्षकम् *VgaROM;

स्थिर शून्य try_सज्जता_display_other(शून्य)
{
    संरचना pci_device *pci;

    dprintf(1, "No VGA found, scan for other display\n");

    foreachpci(pci) {
        यदि (pci->class != PCI_CLASS_DISPLAY_OTHER)
            अनुवर्त;
        संरचना rom_शीर्षकम् *rom = map_pcirom(pci);
        यदि (!rom)
            अनुवर्त;
        dprintf(1, "Other display found at %pP\n", pci);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_स्मृतिः);
        init_optionrom(rom, pci->bdf, 1);
        प्रत्यागम;
    }
}

// Call into vga code to turn on console.
शून्य
vgarom_सज्जता(शून्य)
{
    पूर्णाङ्क have_vga = 0;

    यदि (! CONFIG_OPTIONROMS)
        प्रत्यागम;

    dprintf(1, "Scan for VGA option rom\n");

    // Load some config settings that impact VGA.
    EnforceChecksum = romfile_loadint("etc/optionroms-checksum", 1);
    S3ResumeVga = romfile_loadint("etc/s3-resume-vga-init", CONFIG_QEMU);
    सततम्_PCIroms = romfile_loadint("etc/pci-optionrom-exec", 2);
    ScreenAndDebug = romfile_loadint("etc/screen-and-debug", 1);

    // Clear option rom memory
    स्मृतिं_पूरय((शून्य*)BUILD_ROM_START, 0, rom_get_max() - BUILD_ROM_START);

    // Find and deploy PCI VGA rom.
    संरचना pci_device *pci;
    foreachpci(pci) {
        यदि (!is_pci_vga(pci))
            अनुवर्त;
        vgahook_सज्जता(pci);
        init_pcirom(pci, 1, रिक्त);
        have_vga = 1;
        भङ्ग;
    }
    यदि (!have_vga)
        try_सज्जता_display_other();

    // Find and deploy CBFS vga-style roms not associated with a device.
    सततम्_file_roms("vgaroms/", 1, रिक्त);
    rom_reserve(0);

    यदि (rom_get_चरमः() != BUILD_ROM_START)
        // VGA rom found
        VgaROM = (शून्य*)BUILD_ROM_START;
}

शून्य
s3_resume_vga(शून्य)
{
    यदि (!S3ResumeVga)
        प्रत्यागम;
    यदि (!VgaROM || ! is_valid_rom(VgaROM))
        प्रत्यागम;
    callrom(VgaROM, 0);
}

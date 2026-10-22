// Option rom scanning code.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "bregs.h" // struct bregs
#समावेश "config.h" // CONFIG_*
#समावेश "farptr.h" // FLATPTR_TO_SEG
#समावेश "biosvar.h" // GET_IVT
#समावेश "hw/pci.h" // pci_config_readl
#समावेश "hw/pcidevice.h" // foreachpci
#समावेश "hw/pci_ids.h" // PCI_CLASS_DISPLAY_VGA
#समावेश "hw/pci_regs.h" // PCI_ROM_ADDRESS
#समावेश "malloc.h" // rom_confirm
#समावेश "output.h" // dprintf
#समावेश "romfile.h" // romfile_loadint
#समावेश "stacks.h" // farcall16big
#समावेश "std/optionrom.h" // struct rom_header
#समावेश "std/pnpbios.h" // PNP_SIGNATURE
#समावेश "string.h" // memset
#समावेश "util.h" // get_pnp_offset
#समावेश "tcgbios.h" // tpm_*

अटल पूर्णांक EnforceChecksum, S3ResumeVga, RunPCIroms;


/****************************************************************
 * Helper functions
 ****************************************************************/

// Execute a given option rom.
अटल व्योम
__callrom(काष्ठा rom_header *rom, u16 offset, u16 bdf)
{
    u16 seg = FLATPTR_TO_SEG(rom);
    dprintf(1, "Running option rom at %04x:%04x\n", seg, offset);

    काष्ठा bregs br;
    स_रखो(&br, 0, माप(br));
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
व्योम
callrom(काष्ठा rom_header *rom, u16 bdf)
{
    __callrom(rom, OPTION_ROM_INITVECTOR, bdf);
}

// Execute a BCV option rom registered via add_bcv().
व्योम
call_bcv(u16 seg, u16 ip)
{
    __callrom(MAKE_FLATPTR(seg, 0), ip, 0);
}

// Verify that an option rom looks valid
अटल पूर्णांक
is_valid_rom(काष्ठा rom_header *rom)
{
    dprintf(6, "Checking rom %p (sig %x size %d)\n"
            , rom, rom->signature, rom->आकार);
    अगर (rom->signature != OPTION_ROM_SIGNATURE)
        वापस 0;
    अगर (! rom->आकार)
        वापस 0;
    u32 len = rom->आकार * 512;
    u8 योग = checksum(rom, len);
    अगर (योग != 0) {
        dprintf(1, "Found option rom with bad checksum: loc=%p len=%d sum=%x\n"
                , rom, len, योग);
        अगर (EnforceChecksum)
            वापस 0;
    }
    वापस 1;
}

// Check if a valid option rom has a pnp struct; return it if so.
अटल काष्ठा pnp_तथ्य *
get_pnp_rom(काष्ठा rom_header *rom)
{
    काष्ठा pnp_तथ्य *pnp = (व्योम*)((u8*)rom + rom->pnpoffset);
    अगर (pnp->signature != PNP_SIGNATURE)
        वापस शून्य;
    वापस pnp;
}

// Check for multiple pnp option rom headers.
अटल काष्ठा pnp_तथ्य *
get_pnp_अग्र(काष्ठा rom_header *rom, काष्ठा pnp_तथ्य *pnp)
{
    अगर (! pnp->nextoffset)
        वापस शून्य;
    pnp = (व्योम*)((u8*)rom + pnp->nextoffset);
    अगर (pnp->signature != PNP_SIGNATURE)
        वापस शून्य;
    वापस pnp;
}

// Check if a valid option rom has a pci struct; return it if so.
अटल काष्ठा pci_तथ्य *
get_pci_rom(काष्ठा rom_header *rom)
{
    काष्ठा pci_तथ्य *pd = (व्योम*)((u32)rom + rom->pcioffset);
    अगर (pd->signature != PCI_ROM_SIGNATURE)
        वापस शून्य;
    अगर (rom->pcioffset & 3)
        dprintf(1, "WARNING! Found unaligned PCI rom (vd=%04x:%04x)\n"
                , pd->vendor, pd->device);
    वापस pd;
}

// Run rom init code and note rom size.
अटल पूर्णांक
init_optionrom(काष्ठा rom_header *rom, u16 bdf, पूर्णांक isvga)
{
    अगर (! is_valid_rom(rom))
        वापस -1;
    काष्ठा rom_header *newrom = rom_reserve(rom->आकार * 512);
    अगर (!newrom) {
        warn_noalloc();
        वापस -1;
    }
    अगर (newrom != rom)
        स_हटाओ(newrom, rom, rom->आकार * 512);

    tpm_option_rom(newrom, rom->आकार * 512);

    अगर (isvga || get_pnp_rom(newrom))
        // Only init vga and PnP roms here.
        callrom(newrom, bdf);

    वापस rom_confirm(newrom->आकार * 512);
}

#घोषणा RS_PCIROM (1LL<<33)

अटल व्योम
setRomSource(u64 *sources, काष्ठा rom_header *rom, u64 source)
{
    अगर (sources)
        sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN] = source;
}

अटल पूर्णांक
getRomPriority(u64 *sources, काष्ठा rom_header *rom, पूर्णांक instance)
{
    u64 source = sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    अगर (!source)
        वापस -1;
    अगर (source & RS_PCIROM)
        वापस bootprio_find_pci_rom((व्योम*)(u32)source, instance);
    काष्ठा romfile_s *file = (व्योम*)(u32)source;
    वापस bootprio_find_named_rom(file->name, instance);
}


/****************************************************************
 * Roms in CBFS
 ****************************************************************/

अटल काष्ठा rom_header *
deploy_romfile(काष्ठा romfile_s *file)
{
    u32 आकार = file->आकार;
    काष्ठा rom_header *rom = rom_reserve(आकार);
    अगर (!rom) {
        warn_noalloc();
        वापस शून्य;
    }
    पूर्णांक ret = file->copy(file, rom, आकार);
    अगर (ret <= 0)
        वापस शून्य;
    वापस rom;
}

// Run all roms in a given CBFS directory.
अटल व्योम
run_file_roms(स्थिर अक्षर *prefix, पूर्णांक isvga, u64 *sources)
{
    काष्ठा romfile_s *file = शून्य;
    क्रम (;;) {
        file = romfile_findprefix(prefix, file);
        अगर (!file)
            अवरोध;
        काष्ठा rom_header *rom = deploy_romfile(file);
        अगर (rom) {
            setRomSource(sources, rom, (u32)file);
            init_optionrom(rom, 0, isvga);
        }
    }
}


/****************************************************************
 * PCI roms
 ****************************************************************/

// Verify device is a vga device with legacy address decoding enabled.
पूर्णांक
is_pci_vga(काष्ठा pci_device *pci)
{
    अगर (pci->class != PCI_CLASS_DISPLAY_VGA)
        वापस 0;
    u16 cmd = pci_config_readw(pci->bdf, PCI_COMMAND);
    अगर (!(cmd & PCI_COMMAND_IO && cmd & PCI_COMMAND_MEMORY))
        वापस 0;
    जबतक (pci->parent) {
        pci = pci->parent;
        u32 ctrl = pci_config_readb(pci->bdf, PCI_BRIDGE_CONTROL);
        अगर (!(ctrl & PCI_BRIDGE_CTL_VGA))
            वापस 0;
    }
    वापस 1;
}

// Copy a rom to its permanent location below 1MiB
अटल काष्ठा rom_header *
copy_rom(काष्ठा rom_header *rom)
{
    u32 romsize = rom->आकार * 512;
    काष्ठा rom_header *newrom = rom_reserve(romsize);
    अगर (!newrom) {
        warn_noalloc();
        वापस शून्य;
    }
    dprintf(4, "Copying option rom (size %d) from %p to %p\n"
            , romsize, rom, newrom);
    iomemcpy(newrom, rom, romsize);
    वापस newrom;
}

// Map the option rom of a given PCI device.
अटल काष्ठा rom_header *
map_pcirom(काष्ठा pci_device *pci)
{
    dprintf(6, "Attempting to map option rom on dev %pP\n", pci);

    अगर ((pci->header_type & 0x7f) != PCI_HEADER_TYPE_NORMAL) {
        dprintf(6, "Skipping non-normal pci device (type=%x)\n"
                , pci->header_type);
        वापस शून्य;
    }

    u16 bdf = pci->bdf;
    u32 orig = pci_config_readl(bdf, PCI_ROM_ADDRESS);
    pci_config_writel(bdf, PCI_ROM_ADDRESS, ~PCI_ROM_ADDRESS_ENABLE);
    u32 sz = pci_config_readl(bdf, PCI_ROM_ADDRESS);

    dprintf(6, "Option rom sizing returned %x %x\n", orig, sz);
    orig &= ~PCI_ROM_ADDRESS_ENABLE;
    अगर (!sz || sz == 0xffffffff)
        जाओ fail;

    अगर (orig == sz || (u32)(orig + 4*1024*1024) < 20*1024*1024) {
        // Don't try to map to a pci addresses at its max, in the last
        // 4MiB of ram, or the first 16MiB of ram.
        dprintf(6, "Preset rom address doesn't look valid\n");
        जाओ fail;
    }

    // Looks like a rom - enable it.
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig | PCI_ROM_ADDRESS_ENABLE);

    काष्ठा rom_header *rom = (व्योम*)orig;
    क्रम (;;) {
        dprintf(5, "Inspecting possible rom at %p (vd=%04x:%04x bdf=%pP)\n"
                , rom, pci->vendor, pci->device, pci);
        अगर (rom->signature != OPTION_ROM_SIGNATURE) {
            dprintf(6, "No option rom signature (got %x)\n", rom->signature);
            जाओ fail;
        }
        काष्ठा pci_तथ्य *pd = get_pci_rom(rom);
        अगर (! pd) {
            dprintf(6, "No valid pci signature found\n");
            जाओ fail;
        }

        अगर (pd->vendor == pci->vendor && pd->device == pci->device
            && pd->type == PCIROM_CODETYPE_X86)
            // A match
            अवरोध;
        dprintf(6, "Didn't match dev/ven (got %04x:%04x) or type (got %d)\n"
                , pd->vendor, pd->device, pd->type);
        अगर (pd->indicator & 0x80) {
            dprintf(6, "No more images left\n");
            जाओ fail;
        }
        rom = (व्योम*)((u32)rom + pd->ilen * 512);
    }

    rom = copy_rom(rom);
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig);
    वापस rom;
fail:
    // Not valid - restore original and exit.
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig);
    वापस शून्य;
}

अटल पूर्णांक boot_irq_captured(व्योम)
{
    वापस GET_IVT(0x19).segoff != FUNC16(entry_19_official).segoff;
}

अटल व्योम boot_irq_restore(व्योम)
{
    काष्ठा segoff_s seabios;

    seabios = FUNC16(entry_19_official);
    SET_IVT(0x19, seabios);
}

// Attempt to map and initialize the option rom on a given PCI device.
अटल व्योम
init_pcirom(काष्ठा pci_device *pci, पूर्णांक isvga, u64 *sources)
{
    dprintf(4, "Attempting to init PCI bdf %pP (vd %04x:%04x)\n"
            , pci, pci->vendor, pci->device);

    अक्षर fname[17];
    snprintf(fname, माप(fname), "pci%04x,%04x.rom"
             , pci->vendor, pci->device);
    काष्ठा romfile_s *file = romfile_find(fname);
    काष्ठा rom_header *rom = शून्य;
    अगर (file)
        rom = deploy_romfile(file);
    अन्यथा अगर (RunPCIroms > 1 || (RunPCIroms == 1 && isvga))
        rom = map_pcirom(pci);
    अगर (! rom)
        // No ROM present.
        वापस;
    पूर्णांक irq_was_captured = boot_irq_captured();
    काष्ठा pnp_तथ्य *pnp = get_pnp_rom(rom);
    setRomSource(sources, rom, RS_PCIROM | (u32)pci);
    init_optionrom(rom, pci->bdf, isvga);
    अगर (boot_irq_captured() && !irq_was_captured &&
        !file && !isvga && pnp) {
        // This PCI rom is misbehaving - recapture the boot irqs
        अक्षर *desc = MAKE_FLATPTR(FLATPTR_TO_SEG(rom), pnp->productname);
        dprintf(1, "PnP optionrom \"%s\" (bdf %pP) captured int19, restoring\n",
                desc, pci);
        boot_irq_restore();
    }
}


/****************************************************************
 * Non-VGA option rom init
 ****************************************************************/

व्योम
optionrom_setup(व्योम)
{
    अगर (! CONFIG_OPTIONROMS)
        वापस;

    dprintf(1, "Scan for option roms\n");
    u64 sources[(BUILD_BIOS_ADDR - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    स_रखो(sources, 0, माप(sources));
    u32 post_vga = rom_get_last();

    // Find and deploy PCI roms.
    काष्ठा pci_device *pci;
    foreachpci(pci) {
        अगर (pci->class == PCI_CLASS_DISPLAY_VGA ||
            pci->class == PCI_CLASS_DISPLAY_OTHER ||
            pci->have_driver)
            जारी;
        init_pcirom(pci, 0, sources);
    }

    // Find and deploy CBFS roms not associated with a device.
    run_file_roms("genroms/", 0, sources);
    rom_reserve(0);

    // All option roms found and deployed - now build BEV/BCV vectors.

    u32 pos = post_vga;
    जबतक (pos < rom_get_last()) {
        काष्ठा rom_header *rom = (व्योम*)pos;
        अगर (! is_valid_rom(rom)) {
            pos += OPTION_ROM_ALIGN;
            जारी;
        }
        pos += ALIGN(rom->आकार * 512, OPTION_ROM_ALIGN);
        काष्ठा pnp_तथ्य *pnp = get_pnp_rom(rom);
        अगर (! pnp) {
            // Legacy rom.
            boot_add_bcv(FLATPTR_TO_SEG(rom), OPTION_ROM_INITVECTOR, 0
                         , getRomPriority(sources, rom, 0));
            जारी;
        }
        // PnP rom - check for BEV and BCV boot capabilities.
        पूर्णांक instance = 0;
        जबतक (pnp) {
            अगर (pnp->bev)
                boot_add_bev(FLATPTR_TO_SEG(rom), pnp->bev, pnp->productname
                             , getRomPriority(sources, rom, instance++));
            अन्यथा अगर (pnp->bcv)
                boot_add_bcv(FLATPTR_TO_SEG(rom), pnp->bcv, pnp->productname
                             , getRomPriority(sources, rom, instance++));
            अन्यथा
                अवरोध;
            pnp = get_pnp_अग्र(rom, pnp);
        }
    }
}


/****************************************************************
 * VGA init
 ****************************************************************/

पूर्णांक ScreenAndDebug;
काष्ठा rom_header *VgaROM;

अटल व्योम try_setup_display_other(व्योम)
{
    काष्ठा pci_device *pci;

    dprintf(1, "No VGA found, scan for other display\n");

    foreachpci(pci) {
        अगर (pci->class != PCI_CLASS_DISPLAY_OTHER)
            जारी;
        काष्ठा rom_header *rom = map_pcirom(pci);
        अगर (!rom)
            जारी;
        dprintf(1, "Other display found at %pP\n", pci);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_MEMORY);
        init_optionrom(rom, pci->bdf, 1);
        वापस;
    }
}

// Call into vga code to turn on console.
व्योम
vgarom_setup(व्योम)
{
    पूर्णांक have_vga = 0;

    अगर (! CONFIG_OPTIONROMS)
        वापस;

    dprintf(1, "Scan for VGA option rom\n");

    // Load some config settings that impact VGA.
    EnforceChecksum = romfile_loadint("etc/optionroms-checksum", 1);
    S3ResumeVga = romfile_loadint("etc/s3-resume-vga-init", CONFIG_QEMU);
    RunPCIroms = romfile_loadint("etc/pci-optionrom-exec", 2);
    ScreenAndDebug = romfile_loadint("etc/screen-and-debug", 1);

    // Clear option rom memory
    स_रखो((व्योम*)BUILD_ROM_START, 0, rom_get_max() - BUILD_ROM_START);

    // Find and deploy PCI VGA rom.
    काष्ठा pci_device *pci;
    foreachpci(pci) {
        अगर (!is_pci_vga(pci))
            जारी;
        vgahook_setup(pci);
        init_pcirom(pci, 1, शून्य);
        have_vga = 1;
        अवरोध;
    }
    अगर (!have_vga)
        try_setup_display_other();

    // Find and deploy CBFS vga-style roms not associated with a device.
    run_file_roms("vgaroms/", 1, शून्य);
    rom_reserve(0);

    अगर (rom_get_last() != BUILD_ROM_START)
        // VGA rom found
        VgaROM = (व्योम*)BUILD_ROM_START;
}

व्योम
s3_resume_vga(व्योम)
{
    अगर (!S3ResumeVga)
        वापस;
    अगर (!VgaROM || ! is_valid_rom(VgaROM))
        वापस;
    callrom(VgaROM, 0);
}

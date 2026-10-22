// Option rom scanning code.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "bregs.h" // struct bregs
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "farptr.h" // FLATPTR_TO_SEG
#અન્તર્ભાવ "biosvar.h" // GET_IVT
#અન્તર્ભાવ "hw/pci.h" // pci_config_readl
#અન્તર્ભાવ "hw/pcidevice.h" // foreachpci
#અન્તર્ભાવ "hw/pci_ids.h" // PCI_CLASS_DISPLAY_VGA
#અન્તર્ભાવ "hw/pci_regs.h" // PCI_ROM_ADDRESS
#અન્તર્ભાવ "malloc.h" // rom_confirm
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "romfile.h" // romfile_loadint
#અન્તર્ભાવ "stacks.h" // farcall16big
#અન્તર્ભાવ "std/optionrom.h" // struct rom_header
#અન્તર્ભાવ "std/pnpbios.h" // PNP_SIGNATURE
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // get_pnp_offset
#અન્તર્ભાવ "tcgbios.h" // tpm_*

સ્થિર પૂર્ણાંક EnforceChecksum, S3ResumeVga, RunPCIroms;


/****************************************************************
 * Helper functions
 ****************************************************************/

// Execute a given option rom.
સ્થિર શૂન્ય
__callrom(સંરચના rom_header *rom, u16 offset, u16 bdf)
{
    u16 seg = FLATPTR_TO_SEG(rom);
    dprintf(1, "Running option rom at %04x:%04x\n", seg, offset);

    સંરચના bregs br;
    memset(&br, 0, કદ(br));
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
શૂન્ય
callrom(સંરચના rom_header *rom, u16 bdf)
{
    __callrom(rom, OPTION_ROM_INITVECTOR, bdf);
}

// Execute a BCV option rom registered via add_bcv().
શૂન્ય
call_bcv(u16 seg, u16 ip)
{
    __callrom(MAKE_FLATPTR(seg, 0), ip, 0);
}

// Verify that an option rom looks valid
સ્થિર પૂર્ણાંક
is_valid_rom(સંરચના rom_header *rom)
{
    dprintf(6, "Checking rom %p (sig %x size %d)\n"
            , rom, rom->signature, rom->size);
    જો (rom->signature != OPTION_ROM_SIGNATURE)
        પાછા 0;
    જો (! rom->size)
        પાછા 0;
    u32 len = rom->size * 512;
    u8 sum = checksum(rom, len);
    જો (sum != 0) {
        dprintf(1, "Found option rom with bad checksum: loc=%p len=%d sum=%x\n"
                , rom, len, sum);
        જો (EnforceChecksum)
            પાછા 0;
    }
    પાછા 1;
}

// Check if a valid option rom has a pnp struct; return it if so.
સ્થિર સંરચના pnp_data *
get_pnp_rom(સંરચના rom_header *rom)
{
    સંરચના pnp_data *pnp = (શૂન્ય*)((u8*)rom + rom->pnpoffset);
    જો (pnp->signature != PNP_SIGNATURE)
        પાછા ખાલી;
    પાછા pnp;
}

// Check for multiple pnp option rom headers.
સ્થિર સંરચના pnp_data *
get_pnp_next(સંરચના rom_header *rom, સંરચના pnp_data *pnp)
{
    જો (! pnp->nextoffset)
        પાછા ખાલી;
    pnp = (શૂન્ય*)((u8*)rom + pnp->nextoffset);
    જો (pnp->signature != PNP_SIGNATURE)
        પાછા ખાલી;
    પાછા pnp;
}

// Check if a valid option rom has a pci struct; return it if so.
સ્થિર સંરચના pci_data *
get_pci_rom(સંરચના rom_header *rom)
{
    સંરચના pci_data *pd = (શૂન્ય*)((u32)rom + rom->pcioffset);
    જો (pd->signature != PCI_ROM_SIGNATURE)
        પાછા ખાલી;
    જો (rom->pcioffset & 3)
        dprintf(1, "WARNING! Found unaligned PCI rom (vd=%04x:%04x)\n"
                , pd->vendor, pd->device);
    પાછા pd;
}

// Run rom init code and note rom size.
સ્થિર પૂર્ણાંક
init_optionrom(સંરચના rom_header *rom, u16 bdf, પૂર્ણાંક isvga)
{
    જો (! is_valid_rom(rom))
        પાછા -1;
    સંરચના rom_header *newrom = rom_reserve(rom->size * 512);
    જો (!newrom) {
        warn_noalloc();
        પાછા -1;
    }
    જો (newrom != rom)
        memmove(newrom, rom, rom->size * 512);

    tpm_option_rom(newrom, rom->size * 512);

    જો (isvga || get_pnp_rom(newrom))
        // Only init vga and PnP roms here.
        callrom(newrom, bdf);

    પાછા rom_confirm(newrom->size * 512);
}

#define RS_PCIROM (1LL<<33)

સ્થિર શૂન્ય
setRomSource(u64 *sources, સંરચના rom_header *rom, u64 source)
{
    જો (sources)
        sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN] = source;
}

સ્થિર પૂર્ણાંક
getRomPriority(u64 *sources, સંરચના rom_header *rom, પૂર્ણાંક instance)
{
    u64 source = sources[((u32)rom - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    જો (!source)
        પાછા -1;
    જો (source & RS_PCIROM)
        પાછા bootprio_find_pci_rom((શૂન્ય*)(u32)source, instance);
    સંરચના romfile_s *file = (શૂન્ય*)(u32)source;
    પાછા bootprio_find_named_rom(file->name, instance);
}


/****************************************************************
 * Roms in CBFS
 ****************************************************************/

સ્થિર સંરચના rom_header *
deploy_romfile(સંરચના romfile_s *file)
{
    u32 size = file->size;
    સંરચના rom_header *rom = rom_reserve(size);
    જો (!rom) {
        warn_noalloc();
        પાછા ખાલી;
    }
    પૂર્ણાંક ret = file->copy(file, rom, size);
    જો (ret <= 0)
        પાછા ખાલી;
    પાછા rom;
}

// Run all roms in a given CBFS directory.
સ્થિર શૂન્ય
run_file_roms(અચલ અક્ષર *prefix, પૂર્ણાંક isvga, u64 *sources)
{
    સંરચના romfile_s *file = ખાલી;
    માટે (;;) {
        file = romfile_findprefix(prefix, file);
        જો (!file)
            તોડો;
        સંરચના rom_header *rom = deploy_romfile(file);
        જો (rom) {
            setRomSource(sources, rom, (u32)file);
            init_optionrom(rom, 0, isvga);
        }
    }
}


/****************************************************************
 * PCI roms
 ****************************************************************/

// Verify device is a vga device with legacy address decoding enabled.
પૂર્ણાંક
is_pci_vga(સંરચના pci_device *pci)
{
    જો (pci->class != PCI_CLASS_DISPLAY_VGA)
        પાછા 0;
    u16 cmd = pci_config_readw(pci->bdf, PCI_COMMAND);
    જો (!(cmd & PCI_COMMAND_IO && cmd & PCI_COMMAND_MEMORY))
        પાછા 0;
    જ્યારે (pci->parent) {
        pci = pci->parent;
        u32 ctrl = pci_config_readb(pci->bdf, PCI_BRIDGE_CONTROL);
        જો (!(ctrl & PCI_BRIDGE_CTL_VGA))
            પાછા 0;
    }
    પાછા 1;
}

// Copy a rom to its permanent location below 1MiB
સ્થિર સંરચના rom_header *
copy_rom(સંરચના rom_header *rom)
{
    u32 romsize = rom->size * 512;
    સંરચના rom_header *newrom = rom_reserve(romsize);
    જો (!newrom) {
        warn_noalloc();
        પાછા ખાલી;
    }
    dprintf(4, "Copying option rom (size %d) from %p to %p\n"
            , romsize, rom, newrom);
    iomemcpy(newrom, rom, romsize);
    પાછા newrom;
}

// Map the option rom of a given PCI device.
સ્થિર સંરચના rom_header *
map_pcirom(સંરચના pci_device *pci)
{
    dprintf(6, "Attempting to map option rom on dev %pP\n", pci);

    જો ((pci->header_type & 0x7f) != PCI_HEADER_TYPE_NORMAL) {
        dprintf(6, "Skipping non-normal pci device (type=%x)\n"
                , pci->header_type);
        પાછા ખાલી;
    }

    u16 bdf = pci->bdf;
    u32 orig = pci_config_readl(bdf, PCI_ROM_ADDRESS);
    pci_config_writel(bdf, PCI_ROM_ADDRESS, ~PCI_ROM_ADDRESS_ENABLE);
    u32 sz = pci_config_readl(bdf, PCI_ROM_ADDRESS);

    dprintf(6, "Option rom sizing returned %x %x\n", orig, sz);
    orig &= ~PCI_ROM_ADDRESS_ENABLE;
    જો (!sz || sz == 0xffffffff)
        જાઓ fail;

    જો (orig == sz || (u32)(orig + 4*1024*1024) < 20*1024*1024) {
        // Don't try to map to a pci addresses at its max, in the last
        // 4MiB of ram, or the first 16MiB of ram.
        dprintf(6, "Preset rom address doesn't look valid\n");
        જાઓ fail;
    }

    // Looks like a rom - enable it.
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig | PCI_ROM_ADDRESS_ENABLE);

    સંરચના rom_header *rom = (શૂન્ય*)orig;
    માટે (;;) {
        dprintf(5, "Inspecting possible rom at %p (vd=%04x:%04x bdf=%pP)\n"
                , rom, pci->vendor, pci->device, pci);
        જો (rom->signature != OPTION_ROM_SIGNATURE) {
            dprintf(6, "No option rom signature (got %x)\n", rom->signature);
            જાઓ fail;
        }
        સંરચના pci_data *pd = get_pci_rom(rom);
        જો (! pd) {
            dprintf(6, "No valid pci signature found\n");
            જાઓ fail;
        }

        જો (pd->vendor == pci->vendor && pd->device == pci->device
            && pd->type == PCIROM_CODETYPE_X86)
            // A match
            તોડો;
        dprintf(6, "Didn't match dev/ven (got %04x:%04x) or type (got %d)\n"
                , pd->vendor, pd->device, pd->type);
        જો (pd->indicator & 0x80) {
            dprintf(6, "No more images left\n");
            જાઓ fail;
        }
        rom = (શૂન્ય*)((u32)rom + pd->ilen * 512);
    }

    rom = copy_rom(rom);
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig);
    પાછા rom;
fail:
    // Not valid - restore original and exit.
    pci_config_writel(bdf, PCI_ROM_ADDRESS, orig);
    પાછા ખાલી;
}

સ્થિર પૂર્ણાંક boot_irq_captured(શૂન્ય)
{
    પાછા GET_IVT(0x19).segoff != FUNC16(entry_19_official).segoff;
}

સ્થિર શૂન્ય boot_irq_restore(શૂન્ય)
{
    સંરચના segoff_s seabios;

    seabios = FUNC16(entry_19_official);
    SET_IVT(0x19, seabios);
}

// Attempt to map and initialize the option rom on a given PCI device.
સ્થિર શૂન્ય
init_pcirom(સંરચના pci_device *pci, પૂર્ણાંક isvga, u64 *sources)
{
    dprintf(4, "Attempting to init PCI bdf %pP (vd %04x:%04x)\n"
            , pci, pci->vendor, pci->device);

    અક્ષર fname[17];
    snprintf(fname, કદ(fname), "pci%04x,%04x.rom"
             , pci->vendor, pci->device);
    સંરચના romfile_s *file = romfile_find(fname);
    સંરચના rom_header *rom = ખાલી;
    જો (file)
        rom = deploy_romfile(file);
    નહીંતર જો (RunPCIroms > 1 || (RunPCIroms == 1 && isvga))
        rom = map_pcirom(pci);
    જો (! rom)
        // No ROM present.
        પાછા;
    પૂર્ણાંક irq_was_captured = boot_irq_captured();
    સંરચના pnp_data *pnp = get_pnp_rom(rom);
    setRomSource(sources, rom, RS_PCIROM | (u32)pci);
    init_optionrom(rom, pci->bdf, isvga);
    જો (boot_irq_captured() && !irq_was_captured &&
        !file && !isvga && pnp) {
        // This PCI rom is misbehaving - recapture the boot irqs
        અક્ષર *desc = MAKE_FLATPTR(FLATPTR_TO_SEG(rom), pnp->productname);
        dprintf(1, "PnP optionrom \"%s\" (bdf %pP) captured int19, restoring\n",
                desc, pci);
        boot_irq_restore();
    }
}


/****************************************************************
 * Non-VGA option rom init
 ****************************************************************/

શૂન્ય
optionrom_setup(શૂન્ય)
{
    જો (! CONFIG_OPTIONROMS)
        પાછા;

    dprintf(1, "Scan for option roms\n");
    u64 sources[(BUILD_BIOS_ADDR - BUILD_ROM_START) / OPTION_ROM_ALIGN];
    memset(sources, 0, કદ(sources));
    u32 post_vga = rom_get_last();

    // Find and deploy PCI roms.
    સંરચના pci_device *pci;
    foreachpci(pci) {
        જો (pci->class == PCI_CLASS_DISPLAY_VGA ||
            pci->class == PCI_CLASS_DISPLAY_OTHER ||
            pci->have_driver)
            ચાલુ;
        init_pcirom(pci, 0, sources);
    }

    // Find and deploy CBFS roms not associated with a device.
    run_file_roms("genroms/", 0, sources);
    rom_reserve(0);

    // All option roms found and deployed - now build BEV/BCV vectors.

    u32 pos = post_vga;
    જ્યારે (pos < rom_get_last()) {
        સંરચના rom_header *rom = (શૂન્ય*)pos;
        જો (! is_valid_rom(rom)) {
            pos += OPTION_ROM_ALIGN;
            ચાલુ;
        }
        pos += ALIGN(rom->size * 512, OPTION_ROM_ALIGN);
        સંરચના pnp_data *pnp = get_pnp_rom(rom);
        જો (! pnp) {
            // Legacy rom.
            boot_add_bcv(FLATPTR_TO_SEG(rom), OPTION_ROM_INITVECTOR, 0
                         , getRomPriority(sources, rom, 0));
            ચાલુ;
        }
        // PnP rom - check for BEV and BCV boot capabilities.
        પૂર્ણાંક instance = 0;
        જ્યારે (pnp) {
            જો (pnp->bev)
                boot_add_bev(FLATPTR_TO_SEG(rom), pnp->bev, pnp->productname
                             , getRomPriority(sources, rom, instance++));
            નહીંતર જો (pnp->bcv)
                boot_add_bcv(FLATPTR_TO_SEG(rom), pnp->bcv, pnp->productname
                             , getRomPriority(sources, rom, instance++));
            નહીંતર
                તોડો;
            pnp = get_pnp_next(rom, pnp);
        }
    }
}


/****************************************************************
 * VGA init
 ****************************************************************/

પૂર્ણાંક ScreenAndDebug;
સંરચના rom_header *VgaROM;

સ્થિર શૂન્ય try_setup_display_other(શૂન્ય)
{
    સંરચના pci_device *pci;

    dprintf(1, "No VGA found, scan for other display\n");

    foreachpci(pci) {
        જો (pci->class != PCI_CLASS_DISPLAY_OTHER)
            ચાલુ;
        સંરચના rom_header *rom = map_pcirom(pci);
        જો (!rom)
            ચાલુ;
        dprintf(1, "Other display found at %pP\n", pci);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_MEMORY);
        init_optionrom(rom, pci->bdf, 1);
        પાછા;
    }
}

// Call into vga code to turn on console.
શૂન્ય
vgarom_setup(શૂન્ય)
{
    પૂર્ણાંક have_vga = 0;

    જો (! CONFIG_OPTIONROMS)
        પાછા;

    dprintf(1, "Scan for VGA option rom\n");

    // Load some config settings that impact VGA.
    EnforceChecksum = romfile_loadint("etc/optionroms-checksum", 1);
    S3ResumeVga = romfile_loadint("etc/s3-resume-vga-init", CONFIG_QEMU);
    RunPCIroms = romfile_loadint("etc/pci-optionrom-exec", 2);
    ScreenAndDebug = romfile_loadint("etc/screen-and-debug", 1);

    // Clear option rom memory
    memset((શૂન્ય*)BUILD_ROM_START, 0, rom_get_max() - BUILD_ROM_START);

    // Find and deploy PCI VGA rom.
    સંરચના pci_device *pci;
    foreachpci(pci) {
        જો (!is_pci_vga(pci))
            ચાલુ;
        vgahook_setup(pci);
        init_pcirom(pci, 1, ખાલી);
        have_vga = 1;
        તોડો;
    }
    જો (!have_vga)
        try_setup_display_other();

    // Find and deploy CBFS vga-style roms not associated with a device.
    run_file_roms("vgaroms/", 1, ખાલી);
    rom_reserve(0);

    જો (rom_get_last() != BUILD_ROM_START)
        // VGA rom found
        VgaROM = (શૂન્ય*)BUILD_ROM_START;
}

શૂન્ય
s3_resume_vga(શૂન્ય)
{
    જો (!S3ResumeVga)
        પાછા;
    જો (!VgaROM || ! is_valid_rom(VgaROM))
        પાછા;
    callrom(VgaROM, 0);
}

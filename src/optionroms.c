// Option rom scanning code.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "farptr.h" // FLATPTR_TO_SEG
#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_IVT
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci.h" // pci_config_readl
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pcidevice.h" // foreachpci
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci_ids.h" // PCI_CLASS_DISPLAY_VGA
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci_regs.h" // PCI_ROM_ADDRESS
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // rom_confirm
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_loadint
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // farcall16big
#ਅਨ੍ਤਰ੍ਭਾਵ "std/optionrom.h" // struct rom_header
#ਅਨ੍ਤਰ੍ਭਾਵ "std/pnpbios.h" // PNP_SIGNATURE
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // get_pnp_offset
#ਅਨ੍ਤਰ੍ਭਾਵ "tcgbios.h" // tpm_*

ਸਥਿਰ ਪੂਰਨ EnforceChecksum, S3ResumeVga, ਲੜੀਵਾਰ_PCIroms;


/****************************************************************
 * Helper functions
 ****************************************************************/

// Execute a given option rom.
ਸਥਿਰ ਖਾਲੀ
__callrom(ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom, u16 ਵਿੱਥ_2, u16 bdf)
{
    u16 seg = FLATPTR_ਮੰਜ਼ਿਲ_SEG(rom);
    dprintf(1, "Running option rom at %04x:%04x\n", seg, ਵਿੱਥ_2);

    ਬਣਤਰ bregs br;
    memset(&br, 0, ਆਕਾਰ(br));
    br.flags = F_IF;
    br.ax = bdf;
    br.bx = 0xffff;
    br.dx = 0xffff;
    br.es = SEG_BIOS;
    br.di = get_pnp_ਵਿੱਥ();
    br.ਕੋਡ = SEGOFF(seg, ਵਿੱਥ_2);
    ਸ਼ੁਰੂ_preempt();
    farcall16big(&br);
    ਅੰਤ_preempt();
}

// Execute a given option rom at the standard entry vector.
ਖਾਲੀ
callrom(ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom, u16 bdf)
{
    __callrom(rom, OPTION_ROM_INITVECTOR, bdf);
}

// Execute a BCV option rom registered via add_bcv().
ਖਾਲੀ
call_bcv(u16 seg, u16 ip)
{
    __callrom(MAKE_FLATPTR(seg, 0), ip, 0);
}

// Verify that an option rom looks valid
ਸਥਿਰ ਪੂਰਨ
is_valid_rom(ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom)
{
    dprintf(6, "Checking rom %p (sig %x size %d)\n"
            , rom, rom->signature, rom->ਮਾਪ);
    ਜੇ (rom->signature != OPTION_ROM_SIGNATURE)
        ਵਾਪਸ 0;
    ਜੇ (! rom->ਮਾਪ)
        ਵਾਪਸ 0;
    u32 len = rom->ਮਾਪ * 512;
    u8 ਜੋੜ = checksum(rom, len);
    ਜੇ (ਜੋੜ != 0) {
        dprintf(1, "Found option rom with bad checksum: loc=%p len=%d sum=%x\n"
                , rom, len, ਜੋੜ);
        ਜੇ (EnforceChecksum)
            ਵਾਪਸ 0;
    }
    ਵਾਪਸ 1;
}

// Check if a valid option rom has a pnp struct; return it if so.
ਸਥਿਰ ਬਣਤਰ pnp_ਅੰਕੜੇ *
get_pnp_rom(ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom)
{
    ਬਣਤਰ pnp_ਅੰਕੜੇ *pnp = (ਖਾਲੀ*)((u8*)rom + rom->pnpoffset);
    ਜੇ (pnp->signature != PNP_SIGNATURE)
        ਵਾਪਸ ਨਿਰਾ;
    ਵਾਪਸ pnp;
}

// Check for multiple pnp option rom headers.
ਸਥਿਰ ਬਣਤਰ pnp_ਅੰਕੜੇ *
get_pnp_ਅਗਲਾ(ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom, ਬਣਤਰ pnp_ਅੰਕੜੇ *pnp)
{
    ਜੇ (! pnp->nextoffset)
        ਵਾਪਸ ਨਿਰਾ;
    pnp = (ਖਾਲੀ*)((u8*)rom + pnp->nextoffset);
    ਜੇ (pnp->signature != PNP_SIGNATURE)
        ਵਾਪਸ ਨਿਰਾ;
    ਵਾਪਸ pnp;
}

// Check if a valid option rom has a pci struct; return it if so.
ਸਥਿਰ ਬਣਤਰ pci_ਅੰਕੜੇ *
get_pci_rom(ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom)
{
    ਬਣਤਰ pci_ਅੰਕੜੇ *pd = (ਖਾਲੀ*)((u32)rom + rom->pcioffset);
    ਜੇ (pd->signature != PCI_ROM_SIGNATURE)
        ਵਾਪਸ ਨਿਰਾ;
    ਜੇ (rom->pcioffset & 3)
        dprintf(1, "WARNING! Found unaligned PCI rom (vd=%04x:%04x)\n"
                , pd->vendor, pd->device);
    ਵਾਪਸ pd;
}

// Run rom init code and note rom size.
ਸਥਿਰ ਪੂਰਨ
init_optionrom(ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom, u16 bdf, ਪੂਰਨ isvga)
{
    ਜੇ (! is_valid_rom(rom))
        ਵਾਪਸ -1;
    ਬਣਤਰ rom_ਸਿਰਲੇਖ *newrom = rom_reserve(rom->ਮਾਪ * 512);
    ਜੇ (!newrom) {
        warn_noalloc();
        ਵਾਪਸ -1;
    }
    ਜੇ (newrom != rom)
        memmove(newrom, rom, rom->ਮਾਪ * 512);

    tpm_option_rom(newrom, rom->ਮਾਪ * 512);

    ਜੇ (isvga || get_pnp_rom(newrom))
        // Only init vga and PnP roms here.
        callrom(newrom, bdf);

    ਵਾਪਸ rom_confirm(newrom->ਮਾਪ * 512);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RS_PCIROM (1LL<<33)

ਸਥਿਰ ਖਾਲੀ
ਸੈੱਟ_Rom_ਸਰੋਤ(u64 *sources, ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom, u64 ਸਰੋਤ)
{
    ਜੇ (sources)
        sources[((u32)rom - BUILD_ROM_ਸ਼ੁਰੂ) / OPTION_ROM_ALIGN] = ਸਰੋਤ;
}

ਸਥਿਰ ਪੂਰਨ
get_Rom_ਤਰਜੀਹ(u64 *sources, ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom, ਪੂਰਨ instance)
{
    u64 ਸਰੋਤ = sources[((u32)rom - BUILD_ROM_ਸ਼ੁਰੂ) / OPTION_ROM_ALIGN];
    ਜੇ (!ਸਰੋਤ)
        ਵਾਪਸ -1;
    ਜੇ (ਸਰੋਤ & RS_PCIROM)
        ਵਾਪਸ bootprio_ਲੱਭੋ_pci_rom((ਖਾਲੀ*)(u32)ਸਰੋਤ, instance);
    ਬਣਤਰ romfile_s *file = (ਖਾਲੀ*)(u32)ਸਰੋਤ;
    ਵਾਪਸ bootprio_ਲੱਭੋ_named_rom(file->name, instance);
}


/****************************************************************
 * Roms in CBFS
 ****************************************************************/

ਸਥਿਰ ਬਣਤਰ rom_ਸਿਰਲੇਖ *
deploy_romfile(ਬਣਤਰ romfile_s *file)
{
    u32 ਮਾਪ = file->ਮਾਪ;
    ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom = rom_reserve(ਮਾਪ);
    ਜੇ (!rom) {
        warn_noalloc();
        ਵਾਪਸ ਨਿਰਾ;
    }
    ਪੂਰਨ ret = file->copy(file, rom, ਮਾਪ);
    ਜੇ (ret <= 0)
        ਵਾਪਸ ਨਿਰਾ;
    ਵਾਪਸ rom;
}

// Run all roms in a given CBFS directory.
ਸਥਿਰ ਖਾਲੀ
ਲੜੀਵਾਰ_file_roms(ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ, ਪੂਰਨ isvga, u64 *sources)
{
    ਬਣਤਰ romfile_s *file = ਨਿਰਾ;
    ਲਈ (;;) {
        file = romfile_findprefix(ਅਗੇਤਰ, file);
        ਜੇ (!file)
            ਰੋਕੋ;
        ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom = deploy_romfile(file);
        ਜੇ (rom) {
            ਸੈੱਟ_Rom_ਸਰੋਤ(sources, rom, (u32)file);
            init_optionrom(rom, 0, isvga);
        }
    }
}


/****************************************************************
 * PCI roms
 ****************************************************************/

// Verify device is a vga device with legacy address decoding enabled.
ਪੂਰਨ
is_pci_vga(ਬਣਤਰ pci_device *pci)
{
    ਜੇ (pci->class != PCI_CLASS_DISPLAY_VGA)
        ਵਾਪਸ 0;
    u16 cmd = pci_config_readw(pci->bdf, PCI_COMMAND);
    ਜੇ (!(cmd & PCI_COMMAND_IO && cmd & PCI_COMMAND_ਸਮ੍ਰਿਤੀ))
        ਵਾਪਸ 0;
    ਜਦੋਂ (pci->ਮਾਪਾ) {
        pci = pci->ਮਾਪਾ;
        u32 ctrl = pci_config_readb(pci->bdf, PCI_BRIDGE_CONTROL);
        ਜੇ (!(ctrl & PCI_BRIDGE_CTL_VGA))
            ਵਾਪਸ 0;
    }
    ਵਾਪਸ 1;
}

// Copy a rom to its permanent location below 1MiB
ਸਥਿਰ ਬਣਤਰ rom_ਸਿਰਲੇਖ *
copy_rom(ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom)
{
    u32 romsize = rom->ਮਾਪ * 512;
    ਬਣਤਰ rom_ਸਿਰਲੇਖ *newrom = rom_reserve(romsize);
    ਜੇ (!newrom) {
        warn_noalloc();
        ਵਾਪਸ ਨਿਰਾ;
    }
    dprintf(4, "Copying option rom (size %d) from %p to %p\n"
            , romsize, rom, newrom);
    iomemcpy(newrom, rom, romsize);
    ਵਾਪਸ newrom;
}

// Map the option rom of a given PCI device.
ਸਥਿਰ ਬਣਤਰ rom_ਸਿਰਲੇਖ *
map_pcirom(ਬਣਤਰ pci_device *pci)
{
    dprintf(6, "Attempting to map option rom on dev %pP\n", pci);

    ਜੇ ((pci->ਸਿਰਲੇਖ_type & 0x7f) != PCI_ਸਿਰਲੇਖ_TYPE_ਸਾਧਾਰਨ) {
        dprintf(6, "Skipping non-normal pci device (type=%x)\n"
                , pci->ਸਿਰਲੇਖ_type);
        ਵਾਪਸ ਨਿਰਾ;
    }

    u16 bdf = pci->bdf;
    u32 orig = pci_config_readl(bdf, PCI_ROM_ਪਤਾ);
    pci_config_writel(bdf, PCI_ROM_ਪਤਾ, ~PCI_ROM_ਪਤਾ_ENABLE);
    u32 sz = pci_config_readl(bdf, PCI_ROM_ਪਤਾ);

    dprintf(6, "Option rom sizing returned %x %x\n", orig, sz);
    orig &= ~PCI_ROM_ਪਤਾ_ENABLE;
    ਜੇ (!sz || sz == 0xffffffff)
        ਜਾਓ fail;

    ਜੇ (orig == sz || (u32)(orig + 4*1024*1024) < 20*1024*1024) {
        // Don't try to map to a pci addresses at its max, in the last
        // 4MiB of ram, or the first 16MiB of ram.
        dprintf(6, "Preset rom address doesn't look valid\n");
        ਜਾਓ fail;
    }

    // Looks like a rom - enable it.
    pci_config_writel(bdf, PCI_ROM_ਪਤਾ, orig | PCI_ROM_ਪਤਾ_ENABLE);

    ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom = (ਖਾਲੀ*)orig;
    ਲਈ (;;) {
        dprintf(5, "Inspecting possible rom at %p (vd=%04x:%04x bdf=%pP)\n"
                , rom, pci->vendor, pci->device, pci);
        ਜੇ (rom->signature != OPTION_ROM_SIGNATURE) {
            dprintf(6, "No option rom signature (got %x)\n", rom->signature);
            ਜਾਓ fail;
        }
        ਬਣਤਰ pci_ਅੰਕੜੇ *pd = get_pci_rom(rom);
        ਜੇ (! pd) {
            dprintf(6, "No valid pci signature found\n");
            ਜਾਓ fail;
        }

        ਜੇ (pd->vendor == pci->vendor && pd->device == pci->device
            && pd->type == PCIROM_CODETYPE_X86)
            // A match
            ਰੋਕੋ;
        dprintf(6, "Didn't match dev/ven (got %04x:%04x) or type (got %d)\n"
                , pd->vendor, pd->device, pd->type);
        ਜੇ (pd->indicator & 0x80) {
            dprintf(6, "No more images left\n");
            ਜਾਓ fail;
        }
        rom = (ਖਾਲੀ*)((u32)rom + pd->ilen * 512);
    }

    rom = copy_rom(rom);
    pci_config_writel(bdf, PCI_ROM_ਪਤਾ, orig);
    ਵਾਪਸ rom;
fail:
    // Not valid - restore original and exit.
    pci_config_writel(bdf, PCI_ROM_ਪਤਾ, orig);
    ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ ਪੂਰਨ boot_irq_captured(ਖਾਲੀ)
{
    ਵਾਪਸ GET_IVT(0x19).segoff != FUNC16(ਇੰਦਰਾਜ_19_official).segoff;
}

ਸਥਿਰ ਖਾਲੀ boot_irq_restore(ਖਾਲੀ)
{
    ਬਣਤਰ segoff_s seabios;

    seabios = FUNC16(ਇੰਦਰਾਜ_19_official);
    ਸੈੱਟ_IVT(0x19, seabios);
}

// Attempt to map and initialize the option rom on a given PCI device.
ਸਥਿਰ ਖਾਲੀ
init_pcirom(ਬਣਤਰ pci_device *pci, ਪੂਰਨ isvga, u64 *sources)
{
    dprintf(4, "Attempting to init PCI bdf %pP (vd %04x:%04x)\n"
            , pci, pci->vendor, pci->device);

    ਅੱਖਰ fname[17];
    snprintf(fname, ਆਕਾਰ(fname), "pci%04x,%04x.rom"
             , pci->vendor, pci->device);
    ਬਣਤਰ romfile_s *file = romfile_ਲੱਭੋ(fname);
    ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom = ਨਿਰਾ;
    ਜੇ (file)
        rom = deploy_romfile(file);
    ਨਹੀਂਤਾਂ ਜੇ (ਲੜੀਵਾਰ_PCIroms > 1 || (ਲੜੀਵਾਰ_PCIroms == 1 && isvga))
        rom = map_pcirom(pci);
    ਜੇ (! rom)
        // No ROM present.
        ਵਾਪਸ;
    ਪੂਰਨ irq_was_captured = boot_irq_captured();
    ਬਣਤਰ pnp_ਅੰਕੜੇ *pnp = get_pnp_rom(rom);
    ਸੈੱਟ_Rom_ਸਰੋਤ(sources, rom, RS_PCIROM | (u32)pci);
    init_optionrom(rom, pci->bdf, isvga);
    ਜੇ (boot_irq_captured() && !irq_was_captured &&
        !file && !isvga && pnp) {
        // This PCI rom is misbehaving - recapture the boot irqs
        ਅੱਖਰ *desc = MAKE_FLATPTR(FLATPTR_ਮੰਜ਼ਿਲ_SEG(rom), pnp->productname);
        dprintf(1, "PnP optionrom \"%s\" (bdf %pP) captured int19, restoring\n",
                desc, pci);
        boot_irq_restore();
    }
}


/****************************************************************
 * Non-VGA option rom init
 ****************************************************************/

ਖਾਲੀ
optionrom_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_OPTIONROMS)
        ਵਾਪਸ;

    dprintf(1, "Scan for option roms\n");
    u64 sources[(BUILD_BIOS_ADDR - BUILD_ROM_ਸ਼ੁਰੂ) / OPTION_ROM_ALIGN];
    memset(sources, 0, ਆਕਾਰ(sources));
    u32 post_vga = rom_get_ਆਖਰੀ();

    // Find and deploy PCI roms.
    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (pci->class == PCI_CLASS_DISPLAY_VGA ||
            pci->class == PCI_CLASS_DISPLAY_OTHER ||
            pci->have_driver)
            ਜਾਰੀ;
        init_pcirom(pci, 0, sources);
    }

    // Find and deploy CBFS roms not associated with a device.
    ਲੜੀਵਾਰ_file_roms("genroms/", 0, sources);
    rom_reserve(0);

    // All option roms found and deployed - now build BEV/BCV vectors.

    u32 pos = post_vga;
    ਜਦੋਂ (pos < rom_get_ਆਖਰੀ()) {
        ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom = (ਖਾਲੀ*)pos;
        ਜੇ (! is_valid_rom(rom)) {
            pos += OPTION_ROM_ALIGN;
            ਜਾਰੀ;
        }
        pos += ALIGN(rom->ਮਾਪ * 512, OPTION_ROM_ALIGN);
        ਬਣਤਰ pnp_ਅੰਕੜੇ *pnp = get_pnp_rom(rom);
        ਜੇ (! pnp) {
            // Legacy rom.
            boot_ਜੋੜੋ_bcv(FLATPTR_ਮੰਜ਼ਿਲ_SEG(rom), OPTION_ROM_INITVECTOR, 0
                         , get_Rom_ਤਰਜੀਹ(sources, rom, 0));
            ਜਾਰੀ;
        }
        // PnP rom - check for BEV and BCV boot capabilities.
        ਪੂਰਨ instance = 0;
        ਜਦੋਂ (pnp) {
            ਜੇ (pnp->bev)
                boot_ਜੋੜੋ_bev(FLATPTR_ਮੰਜ਼ਿਲ_SEG(rom), pnp->bev, pnp->productname
                             , get_Rom_ਤਰਜੀਹ(sources, rom, instance++));
            ਨਹੀਂਤਾਂ ਜੇ (pnp->bcv)
                boot_ਜੋੜੋ_bcv(FLATPTR_ਮੰਜ਼ਿਲ_SEG(rom), pnp->bcv, pnp->productname
                             , get_Rom_ਤਰਜੀਹ(sources, rom, instance++));
            ਨਹੀਂਤਾਂ
                ਰੋਕੋ;
            pnp = get_pnp_ਅਗਲਾ(rom, pnp);
        }
    }
}


/****************************************************************
 * VGA init
 ****************************************************************/

ਪੂਰਨ ScreenAndDebug;
ਬਣਤਰ rom_ਸਿਰਲੇਖ *VgaROM;

ਸਥਿਰ ਖਾਲੀ try_ਤਿਆਰੀ_display_other(ਖਾਲੀ)
{
    ਬਣਤਰ pci_device *pci;

    dprintf(1, "No VGA found, scan for other display\n");

    foreachpci(pci) {
        ਜੇ (pci->class != PCI_CLASS_DISPLAY_OTHER)
            ਜਾਰੀ;
        ਬਣਤਰ rom_ਸਿਰਲੇਖ *rom = map_pcirom(pci);
        ਜੇ (!rom)
            ਜਾਰੀ;
        dprintf(1, "Other display found at %pP\n", pci);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_ਸਮ੍ਰਿਤੀ);
        init_optionrom(rom, pci->bdf, 1);
        ਵਾਪਸ;
    }
}

// Call into vga code to turn on console.
ਖਾਲੀ
vgarom_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਪੂਰਨ have_vga = 0;

    ਜੇ (! CONFIG_OPTIONROMS)
        ਵਾਪਸ;

    dprintf(1, "Scan for VGA option rom\n");

    // Load some config settings that impact VGA.
    EnforceChecksum = romfile_loadint("etc/optionroms-checksum", 1);
    S3ResumeVga = romfile_loadint("etc/s3-resume-vga-init", CONFIG_QEMU);
    ਲੜੀਵਾਰ_PCIroms = romfile_loadint("etc/pci-optionrom-exec", 2);
    ScreenAndDebug = romfile_loadint("etc/screen-and-debug", 1);

    // Clear option rom memory
    memset((ਖਾਲੀ*)BUILD_ROM_ਸ਼ੁਰੂ, 0, rom_get_max() - BUILD_ROM_ਸ਼ੁਰੂ);

    // Find and deploy PCI VGA rom.
    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (!is_pci_vga(pci))
            ਜਾਰੀ;
        vgahook_ਤਿਆਰੀ(pci);
        init_pcirom(pci, 1, ਨਿਰਾ);
        have_vga = 1;
        ਰੋਕੋ;
    }
    ਜੇ (!have_vga)
        try_ਤਿਆਰੀ_display_other();

    // Find and deploy CBFS vga-style roms not associated with a device.
    ਲੜੀਵਾਰ_file_roms("vgaroms/", 1, ਨਿਰਾ);
    rom_reserve(0);

    ਜੇ (rom_get_ਆਖਰੀ() != BUILD_ROM_ਸ਼ੁਰੂ)
        // VGA rom found
        VgaROM = (ਖਾਲੀ*)BUILD_ROM_ਸ਼ੁਰੂ;
}

ਖਾਲੀ
s3_resume_vga(ਖਾਲੀ)
{
    ਜੇ (!S3ResumeVga)
        ਵਾਪਸ;
    ਜੇ (!VgaROM || ! is_valid_rom(VgaROM))
        ਵਾਪਸ;
    callrom(VgaROM, 0);
}

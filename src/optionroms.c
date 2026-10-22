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

ساکن صحیح EnforceChecksum, S3ResumeVga, تسلسل_PCIroms;


/****************************************************************
 * Helper functions
 ****************************************************************/

// Execute a given option rom.
ساکن خالی
__callrom(ساخت rom_سرخی *rom, u16 انحراف_2, u16 bdf)
{
    u16 seg = FLATPTR_مقصد_SEG(rom);
    dprintf(1, "Running option rom at %04x:%04x\n", seg, انحراف_2);

    ساخت bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF;
    br.ax = bdf;
    br.bx = 0xffff;
    br.dx = 0xffff;
    br.es = SEG_BIOS;
    br.di = get_pnp_انحراف();
    br.رمز = SEGOFF(seg, انحراف_2);
    آغاز_preempt();
    farcall16big(&br);
    اختتام_preempt();
}

// Execute a given option rom at the standard entry vector.
خالی
callrom(ساخت rom_سرخی *rom, u16 bdf)
{
    __callrom(rom, OPTION_ROM_INITVECTOR, bdf);
}

// Execute a BCV option rom registered via add_bcv().
خالی
call_bcv(u16 seg, u16 ip)
{
    __callrom(MAKE_FLATPTR(seg, 0), ip, 0);
}

// Verify that an option rom looks valid
ساکن صحیح
is_valid_rom(ساخت rom_سرخی *rom)
{
    dprintf(6, "Checking rom %p (sig %x size %d)\n"
            , rom, rom->signature, rom->سائز);
    اگر (rom->signature != OPTION_ROM_SIGNATURE)
        واپس 0;
    اگر (! rom->سائز)
        واپس 0;
    u32 len = rom->سائز * 512;
    u8 مجموع = checksum(rom, len);
    اگر (مجموع != 0) {
        dprintf(1, "Found option rom with bad checksum: loc=%p len=%d sum=%x\n"
                , rom, len, مجموع);
        اگر (EnforceChecksum)
            واپس 0;
    }
    واپس 1;
}

// Check if a valid option rom has a pnp struct; return it if so.
ساکن ساخت pnp_ڈیٹا *
get_pnp_rom(ساخت rom_سرخی *rom)
{
    ساخت pnp_ڈیٹا *pnp = (خالی*)((u8*)rom + rom->pnpoffset);
    اگر (pnp->signature != PNP_SIGNATURE)
        واپس تہی;
    واپس pnp;
}

// Check for multiple pnp option rom headers.
ساکن ساخت pnp_ڈیٹا *
get_pnp_اگلا(ساخت rom_سرخی *rom, ساخت pnp_ڈیٹا *pnp)
{
    اگر (! pnp->nextoffset)
        واپس تہی;
    pnp = (خالی*)((u8*)rom + pnp->nextoffset);
    اگر (pnp->signature != PNP_SIGNATURE)
        واپس تہی;
    واپس pnp;
}

// Check if a valid option rom has a pci struct; return it if so.
ساکن ساخت pci_ڈیٹا *
get_pci_rom(ساخت rom_سرخی *rom)
{
    ساخت pci_ڈیٹا *pd = (خالی*)((u32)rom + rom->pcioffset);
    اگر (pd->signature != PCI_ROM_SIGNATURE)
        واپس تہی;
    اگر (rom->pcioffset & 3)
        dprintf(1, "WARNING! Found unaligned PCI rom (vd=%04x:%04x)\n"
                , pd->vendor, pd->device);
    واپس pd;
}

// Run rom init code and note rom size.
ساکن صحیح
init_optionrom(ساخت rom_سرخی *rom, u16 bdf, صحیح isvga)
{
    اگر (! is_valid_rom(rom))
        واپس -1;
    ساخت rom_سرخی *newrom = rom_reserve(rom->سائز * 512);
    اگر (!newrom) {
        warn_noalloc();
        واپس -1;
    }
    اگر (newrom != rom)
        memmove(newrom, rom, rom->سائز * 512);

    tpm_option_rom(newrom, rom->سائز * 512);

    اگر (isvga || get_pnp_rom(newrom))
        // Only init vga and PnP roms here.
        callrom(newrom, bdf);

    واپس rom_confirm(newrom->سائز * 512);
}

#تعریف_کرو RS_PCIROM (1LL<<33)

ساکن خالی
سیٹ_Rom_ماخذ(u64 *sources, ساخت rom_سرخی *rom, u64 ماخذ)
{
    اگر (sources)
        sources[((u32)rom - BUILD_ROM_آغاز) / OPTION_ROM_ALIGN] = ماخذ;
}

ساکن صحیح
get_Rom_ترجیح(u64 *sources, ساخت rom_سرخی *rom, صحیح instance)
{
    u64 ماخذ = sources[((u32)rom - BUILD_ROM_آغاز) / OPTION_ROM_ALIGN];
    اگر (!ماخذ)
        واپس -1;
    اگر (ماخذ & RS_PCIROM)
        واپس bootprio_ڈھونڈو_pci_rom((خالی*)(u32)ماخذ, instance);
    ساخت romfile_s *file = (خالی*)(u32)ماخذ;
    واپس bootprio_ڈھونڈو_named_rom(file->name, instance);
}


/****************************************************************
 * Roms in CBFS
 ****************************************************************/

ساکن ساخت rom_سرخی *
deploy_romfile(ساخت romfile_s *file)
{
    u32 سائز = file->سائز;
    ساخت rom_سرخی *rom = rom_reserve(سائز);
    اگر (!rom) {
        warn_noalloc();
        واپس تہی;
    }
    صحیح ret = file->copy(file, rom, سائز);
    اگر (ret <= 0)
        واپس تہی;
    واپس rom;
}

// Run all roms in a given CBFS directory.
ساکن خالی
تسلسل_file_roms(ثابت حرف *سابقہ, صحیح isvga, u64 *sources)
{
    ساخت romfile_s *file = تہی;
    برائے (;;) {
        file = romfile_findprefix(سابقہ, file);
        اگر (!file)
            توڑو;
        ساخت rom_سرخی *rom = deploy_romfile(file);
        اگر (rom) {
            سیٹ_Rom_ماخذ(sources, rom, (u32)file);
            init_optionrom(rom, 0, isvga);
        }
    }
}


/****************************************************************
 * PCI roms
 ****************************************************************/

// Verify device is a vga device with legacy address decoding enabled.
صحیح
is_pci_vga(ساخت pci_device *pci)
{
    اگر (pci->class != PCI_CLASS_DISPLAY_VGA)
        واپس 0;
    u16 cmd = pci_config_readw(pci->bdf, PCI_COMMAND);
    اگر (!(cmd & PCI_COMMAND_IO && cmd & PCI_COMMAND_حافظہ))
        واپس 0;
    جب (pci->والد) {
        pci = pci->والد;
        u32 ctrl = pci_config_readb(pci->bdf, PCI_BRIDGE_CONTROL);
        اگر (!(ctrl & PCI_BRIDGE_CTL_VGA))
            واپس 0;
    }
    واپس 1;
}

// Copy a rom to its permanent location below 1MiB
ساکن ساخت rom_سرخی *
copy_rom(ساخت rom_سرخی *rom)
{
    u32 romsize = rom->سائز * 512;
    ساخت rom_سرخی *newrom = rom_reserve(romsize);
    اگر (!newrom) {
        warn_noalloc();
        واپس تہی;
    }
    dprintf(4, "Copying option rom (size %d) from %p to %p\n"
            , romsize, rom, newrom);
    iomemcpy(newrom, rom, romsize);
    واپس newrom;
}

// Map the option rom of a given PCI device.
ساکن ساخت rom_سرخی *
map_pcirom(ساخت pci_device *pci)
{
    dprintf(6, "Attempting to map option rom on dev %pP\n", pci);

    اگر ((pci->سرخی_type & 0x7f) != PCI_سرخی_TYPE_معمول) {
        dprintf(6, "Skipping non-normal pci device (type=%x)\n"
                , pci->سرخی_type);
        واپس تہی;
    }

    u16 bdf = pci->bdf;
    u32 orig = pci_config_readl(bdf, PCI_ROM_پتہ);
    pci_config_writel(bdf, PCI_ROM_پتہ, ~PCI_ROM_پتہ_ENABLE);
    u32 sz = pci_config_readl(bdf, PCI_ROM_پتہ);

    dprintf(6, "Option rom sizing returned %x %x\n", orig, sz);
    orig &= ~PCI_ROM_پتہ_ENABLE;
    اگر (!sz || sz == 0xffffffff)
        جاؤ fail;

    اگر (orig == sz || (u32)(orig + 4*1024*1024) < 20*1024*1024) {
        // Don't try to map to a pci addresses at its max, in the last
        // 4MiB of ram, or the first 16MiB of ram.
        dprintf(6, "Preset rom address doesn't look valid\n");
        جاؤ fail;
    }

    // Looks like a rom - enable it.
    pci_config_writel(bdf, PCI_ROM_پتہ, orig | PCI_ROM_پتہ_ENABLE);

    ساخت rom_سرخی *rom = (خالی*)orig;
    برائے (;;) {
        dprintf(5, "Inspecting possible rom at %p (vd=%04x:%04x bdf=%pP)\n"
                , rom, pci->vendor, pci->device, pci);
        اگر (rom->signature != OPTION_ROM_SIGNATURE) {
            dprintf(6, "No option rom signature (got %x)\n", rom->signature);
            جاؤ fail;
        }
        ساخت pci_ڈیٹا *pd = get_pci_rom(rom);
        اگر (! pd) {
            dprintf(6, "No valid pci signature found\n");
            جاؤ fail;
        }

        اگر (pd->vendor == pci->vendor && pd->device == pci->device
            && pd->type == PCIROM_CODETYPE_X86)
            // A match
            توڑو;
        dprintf(6, "Didn't match dev/ven (got %04x:%04x) or type (got %d)\n"
                , pd->vendor, pd->device, pd->type);
        اگر (pd->indicator & 0x80) {
            dprintf(6, "No more images left\n");
            جاؤ fail;
        }
        rom = (خالی*)((u32)rom + pd->ilen * 512);
    }

    rom = copy_rom(rom);
    pci_config_writel(bdf, PCI_ROM_پتہ, orig);
    واپس rom;
fail:
    // Not valid - restore original and exit.
    pci_config_writel(bdf, PCI_ROM_پتہ, orig);
    واپس تہی;
}

ساکن صحیح boot_irq_captured(خالی)
{
    واپس GET_IVT(0x19).segoff != FUNC16(اندراج_19_official).segoff;
}

ساکن خالی boot_irq_restore(خالی)
{
    ساخت segoff_s seabios;

    seabios = FUNC16(اندراج_19_official);
    سیٹ_IVT(0x19, seabios);
}

// Attempt to map and initialize the option rom on a given PCI device.
ساکن خالی
init_pcirom(ساخت pci_device *pci, صحیح isvga, u64 *sources)
{
    dprintf(4, "Attempting to init PCI bdf %pP (vd %04x:%04x)\n"
            , pci, pci->vendor, pci->device);

    حرف fname[17];
    snprintf(fname, حجم(fname), "pci%04x,%04x.rom"
             , pci->vendor, pci->device);
    ساخت romfile_s *file = romfile_ڈھونڈو(fname);
    ساخت rom_سرخی *rom = تہی;
    اگر (file)
        rom = deploy_romfile(file);
    ورنہ اگر (تسلسل_PCIroms > 1 || (تسلسل_PCIroms == 1 && isvga))
        rom = map_pcirom(pci);
    اگر (! rom)
        // No ROM present.
        واپس;
    صحیح irq_was_captured = boot_irq_captured();
    ساخت pnp_ڈیٹا *pnp = get_pnp_rom(rom);
    سیٹ_Rom_ماخذ(sources, rom, RS_PCIROM | (u32)pci);
    init_optionrom(rom, pci->bdf, isvga);
    اگر (boot_irq_captured() && !irq_was_captured &&
        !file && !isvga && pnp) {
        // This PCI rom is misbehaving - recapture the boot irqs
        حرف *desc = MAKE_FLATPTR(FLATPTR_مقصد_SEG(rom), pnp->productname);
        dprintf(1, "PnP optionrom \"%s\" (bdf %pP) captured int19, restoring\n",
                desc, pci);
        boot_irq_restore();
    }
}


/****************************************************************
 * Non-VGA option rom init
 ****************************************************************/

خالی
optionrom_تیاری(خالی)
{
    اگر (! CONFIG_OPTIONROMS)
        واپس;

    dprintf(1, "Scan for option roms\n");
    u64 sources[(BUILD_BIOS_ADDR - BUILD_ROM_آغاز) / OPTION_ROM_ALIGN];
    memset(sources, 0, حجم(sources));
    u32 post_vga = rom_get_آخری();

    // Find and deploy PCI roms.
    ساخت pci_device *pci;
    foreachpci(pci) {
        اگر (pci->class == PCI_CLASS_DISPLAY_VGA ||
            pci->class == PCI_CLASS_DISPLAY_OTHER ||
            pci->have_driver)
            جاری;
        init_pcirom(pci, 0, sources);
    }

    // Find and deploy CBFS roms not associated with a device.
    تسلسل_file_roms("genroms/", 0, sources);
    rom_reserve(0);

    // All option roms found and deployed - now build BEV/BCV vectors.

    u32 pos = post_vga;
    جب (pos < rom_get_آخری()) {
        ساخت rom_سرخی *rom = (خالی*)pos;
        اگر (! is_valid_rom(rom)) {
            pos += OPTION_ROM_ALIGN;
            جاری;
        }
        pos += ALIGN(rom->سائز * 512, OPTION_ROM_ALIGN);
        ساخت pnp_ڈیٹا *pnp = get_pnp_rom(rom);
        اگر (! pnp) {
            // Legacy rom.
            boot_جوڑو_bcv(FLATPTR_مقصد_SEG(rom), OPTION_ROM_INITVECTOR, 0
                         , get_Rom_ترجیح(sources, rom, 0));
            جاری;
        }
        // PnP rom - check for BEV and BCV boot capabilities.
        صحیح instance = 0;
        جب (pnp) {
            اگر (pnp->bev)
                boot_جوڑو_bev(FLATPTR_مقصد_SEG(rom), pnp->bev, pnp->productname
                             , get_Rom_ترجیح(sources, rom, instance++));
            ورنہ اگر (pnp->bcv)
                boot_جوڑو_bcv(FLATPTR_مقصد_SEG(rom), pnp->bcv, pnp->productname
                             , get_Rom_ترجیح(sources, rom, instance++));
            ورنہ
                توڑو;
            pnp = get_pnp_اگلا(rom, pnp);
        }
    }
}


/****************************************************************
 * VGA init
 ****************************************************************/

صحیح ScreenAndDebug;
ساخت rom_سرخی *VgaROM;

ساکن خالی try_تیاری_display_other(خالی)
{
    ساخت pci_device *pci;

    dprintf(1, "No VGA found, scan for other display\n");

    foreachpci(pci) {
        اگر (pci->class != PCI_CLASS_DISPLAY_OTHER)
            جاری;
        ساخت rom_سرخی *rom = map_pcirom(pci);
        اگر (!rom)
            جاری;
        dprintf(1, "Other display found at %pP\n", pci);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_حافظہ);
        init_optionrom(rom, pci->bdf, 1);
        واپس;
    }
}

// Call into vga code to turn on console.
خالی
vgarom_تیاری(خالی)
{
    صحیح have_vga = 0;

    اگر (! CONFIG_OPTIONROMS)
        واپس;

    dprintf(1, "Scan for VGA option rom\n");

    // Load some config settings that impact VGA.
    EnforceChecksum = romfile_loadint("etc/optionroms-checksum", 1);
    S3ResumeVga = romfile_loadint("etc/s3-resume-vga-init", CONFIG_QEMU);
    تسلسل_PCIroms = romfile_loadint("etc/pci-optionrom-exec", 2);
    ScreenAndDebug = romfile_loadint("etc/screen-and-debug", 1);

    // Clear option rom memory
    memset((خالی*)BUILD_ROM_آغاز, 0, rom_get_max() - BUILD_ROM_آغاز);

    // Find and deploy PCI VGA rom.
    ساخت pci_device *pci;
    foreachpci(pci) {
        اگر (!is_pci_vga(pci))
            جاری;
        vgahook_تیاری(pci);
        init_pcirom(pci, 1, تہی);
        have_vga = 1;
        توڑو;
    }
    اگر (!have_vga)
        try_تیاری_display_other();

    // Find and deploy CBFS vga-style roms not associated with a device.
    تسلسل_file_roms("vgaroms/", 1, تہی);
    rom_reserve(0);

    اگر (rom_get_آخری() != BUILD_ROM_آغاز)
        // VGA rom found
        VgaROM = (خالی*)BUILD_ROM_آغاز;
}

خالی
s3_resume_vga(خالی)
{
    اگر (!S3ResumeVga)
        واپس;
    اگر (!VgaROM || ! is_valid_rom(VgaROM))
        واپس;
    callrom(VgaROM, 0);
}

// Option rom scanning code.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "bregs.h" // struct bregs
#ضمن "config.h" // CONFIG_*
#ضمن "farptr.h" // FLATPTR_TO_SEG
#ضمن "biosvar.h" // GET_IVT
#ضمن "hw/pci.h" // pci_config_readl
#ضمن "hw/pcidevice.h" // foreachpci
#ضمن "hw/pci_ids.h" // PCI_CLASS_DISPLAY_VGA
#ضمن "hw/pci_regs.h" // PCI_ROM_ADDRESS
#ضمن "malloc.h" // rom_confirm
#ضمن "output.h" // dprintf
#ضمن "romfile.h" // romfile_loadint
#ضمن "stacks.h" // farcall16big
#ضمن "std/optionrom.h" // struct rom_header
#ضمن "std/pnpbios.h" // PNP_SIGNATURE
#ضمن "string.h" // memset
#ضمن "util.h" // get_pnp_offset
#ضمن "tcgbios.h" // tpm_*

ساكن صحيح EnforceChecksum, S3ResumeVga, لگاتار_PCIroms;


/****************************************************************
 * Helper functions
 ****************************************************************/

// Execute a given option rom.
ساكن فراغ
__callrom(بنية rom_سرخی *rom, u16 وتھ_2, u16 bdf)
{
    u16 seg = FLATPTR_منزل_SEG(rom);
    dprintf(1, "Running option rom at %04x:%04x\n", seg, وتھ_2);

    بنية bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_IF;
    br.ax = bdf;
    br.bx = 0xffff;
    br.dx = 0xffff;
    br.es = SEG_BIOS;
    br.di = get_pnp_وتھ();
    br.کوڈ = SEGOFF(seg, وتھ_2);
    شروع_preempt();
    farcall16big(&br);
    انت_preempt();
}

// Execute a given option rom at the standard entry vector.
فراغ
callrom(بنية rom_سرخی *rom, u16 bdf)
{
    __callrom(rom, OPTION_ROM_INITVECTOR, bdf);
}

// Execute a BCV option rom registered via add_bcv().
فراغ
call_bcv(u16 seg, u16 ip)
{
    __callrom(MAKE_FLATPTR(seg, 0), ip, 0);
}

// Verify that an option rom looks valid
ساكن صحيح
is_valid_rom(بنية rom_سرخی *rom)
{
    dprintf(6, "Checking rom %p (sig %x size %d)\n"
            , rom, rom->signature, rom->ناپ);
    إذا (rom->signature != OPTION_ROM_SIGNATURE)
        أرجع 0;
    إذا (! rom->ناپ)
        أرجع 0;
    u32 len = rom->ناپ * 512;
    u8 جوڑ = checksum(rom, len);
    إذا (جوڑ != 0) {
        dprintf(1, "Found option rom with bad checksum: loc=%p len=%d sum=%x\n"
                , rom, len, جوڑ);
        إذا (EnforceChecksum)
            أرجع 0;
    }
    أرجع 1;
}

// Check if a valid option rom has a pnp struct; return it if so.
ساكن بنية pnp_انکڑے *
get_pnp_rom(بنية rom_سرخی *rom)
{
    بنية pnp_انکڑے *pnp = (فراغ*)((u8*)rom + rom->pnpoffset);
    إذا (pnp->signature != PNP_SIGNATURE)
        أرجع عدم;
    أرجع pnp;
}

// Check for multiple pnp option rom headers.
ساكن بنية pnp_انکڑے *
get_pnp_اگلا(بنية rom_سرخی *rom, بنية pnp_انکڑے *pnp)
{
    إذا (! pnp->nextoffset)
        أرجع عدم;
    pnp = (فراغ*)((u8*)rom + pnp->nextoffset);
    إذا (pnp->signature != PNP_SIGNATURE)
        أرجع عدم;
    أرجع pnp;
}

// Check if a valid option rom has a pci struct; return it if so.
ساكن بنية pci_انکڑے *
get_pci_rom(بنية rom_سرخی *rom)
{
    بنية pci_انکڑے *pd = (فراغ*)((u32)rom + rom->pcioffset);
    إذا (pd->signature != PCI_ROM_SIGNATURE)
        أرجع عدم;
    إذا (rom->pcioffset & 3)
        dprintf(1, "WARNING! Found unaligned PCI rom (vd=%04x:%04x)\n"
                , pd->vendor, pd->device);
    أرجع pd;
}

// Run rom init code and note rom size.
ساكن صحيح
init_optionrom(بنية rom_سرخی *rom, u16 bdf, صحيح isvga)
{
    إذا (! is_valid_rom(rom))
        أرجع -1;
    بنية rom_سرخی *newrom = rom_reserve(rom->ناپ * 512);
    إذا (!newrom) {
        warn_noalloc();
        أرجع -1;
    }
    إذا (newrom != rom)
        memmove(newrom, rom, rom->ناپ * 512);

    tpm_option_rom(newrom, rom->ناپ * 512);

    إذا (isvga || get_pnp_rom(newrom))
        // Only init vga and PnP roms here.
        callrom(newrom, bdf);

    أرجع rom_confirm(newrom->ناپ * 512);
}

#تعریف_کرو RS_PCIROM (1LL<<33)

ساكن فراغ
سیٹ_Rom_سروت(u64 *sources, بنية rom_سرخی *rom, u64 سروت)
{
    إذا (sources)
        sources[((u32)rom - BUILD_ROM_شروع) / OPTION_ROM_ALIGN] = سروت;
}

ساكن صحيح
get_Rom_ترجیح(u64 *sources, بنية rom_سرخی *rom, صحيح instance)
{
    u64 سروت = sources[((u32)rom - BUILD_ROM_شروع) / OPTION_ROM_ALIGN];
    إذا (!سروت)
        أرجع -1;
    إذا (سروت & RS_PCIROM)
        أرجع bootprio_لبھو_pci_rom((فراغ*)(u32)سروت, instance);
    بنية romfile_s *file = (فراغ*)(u32)سروت;
    أرجع bootprio_لبھو_named_rom(file->name, instance);
}


/****************************************************************
 * Roms in CBFS
 ****************************************************************/

ساكن بنية rom_سرخی *
deploy_romfile(بنية romfile_s *file)
{
    u32 ناپ = file->ناپ;
    بنية rom_سرخی *rom = rom_reserve(ناپ);
    إذا (!rom) {
        warn_noalloc();
        أرجع عدم;
    }
    صحيح ret = file->copy(file, rom, ناپ);
    إذا (ret <= 0)
        أرجع عدم;
    أرجع rom;
}

// Run all roms in a given CBFS directory.
ساكن فراغ
لگاتار_file_roms(ثابت حرف *اگیتر, صحيح isvga, u64 *sources)
{
    بنية romfile_s *file = عدم;
    لكل (;;) {
        file = romfile_findprefix(اگیتر, file);
        إذا (!file)
            اكسر;
        بنية rom_سرخی *rom = deploy_romfile(file);
        إذا (rom) {
            سیٹ_Rom_سروت(sources, rom, (u32)file);
            init_optionrom(rom, 0, isvga);
        }
    }
}


/****************************************************************
 * PCI roms
 ****************************************************************/

// Verify device is a vga device with legacy address decoding enabled.
صحيح
is_pci_vga(بنية pci_device *pci)
{
    إذا (pci->class != PCI_CLASS_DISPLAY_VGA)
        أرجع 0;
    u16 cmd = pci_config_readw(pci->bdf, PCI_COMMAND);
    إذا (!(cmd & PCI_COMMAND_IO && cmd & PCI_COMMAND_حافظہ))
        أرجع 0;
    طالما (pci->ماپا) {
        pci = pci->ماپا;
        u32 ctrl = pci_config_readb(pci->bdf, PCI_BRIDGE_CONTROL);
        إذا (!(ctrl & PCI_BRIDGE_CTL_VGA))
            أرجع 0;
    }
    أرجع 1;
}

// Copy a rom to its permanent location below 1MiB
ساكن بنية rom_سرخی *
copy_rom(بنية rom_سرخی *rom)
{
    u32 romsize = rom->ناپ * 512;
    بنية rom_سرخی *newrom = rom_reserve(romsize);
    إذا (!newrom) {
        warn_noalloc();
        أرجع عدم;
    }
    dprintf(4, "Copying option rom (size %d) from %p to %p\n"
            , romsize, rom, newrom);
    iomemcpy(newrom, rom, romsize);
    أرجع newrom;
}

// Map the option rom of a given PCI device.
ساكن بنية rom_سرخی *
map_pcirom(بنية pci_device *pci)
{
    dprintf(6, "Attempting to map option rom on dev %pP\n", pci);

    إذا ((pci->سرخی_type & 0x7f) != PCI_سرخی_TYPE_عام) {
        dprintf(6, "Skipping non-normal pci device (type=%x)\n"
                , pci->سرخی_type);
        أرجع عدم;
    }

    u16 bdf = pci->bdf;
    u32 orig = pci_config_readl(bdf, PCI_ROM_پتہ);
    pci_config_writel(bdf, PCI_ROM_پتہ, ~PCI_ROM_پتہ_ENABLE);
    u32 sz = pci_config_readl(bdf, PCI_ROM_پتہ);

    dprintf(6, "Option rom sizing returned %x %x\n", orig, sz);
    orig &= ~PCI_ROM_پتہ_ENABLE;
    إذا (!sz || sz == 0xffffffff)
        اذهب fail;

    إذا (orig == sz || (u32)(orig + 4*1024*1024) < 20*1024*1024) {
        // Don't try to map to a pci addresses at its max, in the last
        // 4MiB of ram, or the first 16MiB of ram.
        dprintf(6, "Preset rom address doesn't look valid\n");
        اذهب fail;
    }

    // Looks like a rom - enable it.
    pci_config_writel(bdf, PCI_ROM_پتہ, orig | PCI_ROM_پتہ_ENABLE);

    بنية rom_سرخی *rom = (فراغ*)orig;
    لكل (;;) {
        dprintf(5, "Inspecting possible rom at %p (vd=%04x:%04x bdf=%pP)\n"
                , rom, pci->vendor, pci->device, pci);
        إذا (rom->signature != OPTION_ROM_SIGNATURE) {
            dprintf(6, "No option rom signature (got %x)\n", rom->signature);
            اذهب fail;
        }
        بنية pci_انکڑے *pd = get_pci_rom(rom);
        إذا (! pd) {
            dprintf(6, "No valid pci signature found\n");
            اذهب fail;
        }

        إذا (pd->vendor == pci->vendor && pd->device == pci->device
            && pd->type == PCIROM_CODETYPE_X86)
            // A match
            اكسر;
        dprintf(6, "Didn't match dev/ven (got %04x:%04x) or type (got %d)\n"
                , pd->vendor, pd->device, pd->type);
        إذا (pd->indicator & 0x80) {
            dprintf(6, "No more images left\n");
            اذهب fail;
        }
        rom = (فراغ*)((u32)rom + pd->ilen * 512);
    }

    rom = copy_rom(rom);
    pci_config_writel(bdf, PCI_ROM_پتہ, orig);
    أرجع rom;
fail:
    // Not valid - restore original and exit.
    pci_config_writel(bdf, PCI_ROM_پتہ, orig);
    أرجع عدم;
}

ساكن صحيح boot_irq_captured(فراغ)
{
    أرجع GET_IVT(0x19).segoff != FUNC16(اندراج_19_official).segoff;
}

ساكن فراغ boot_irq_restore(فراغ)
{
    بنية segoff_s seabios;

    seabios = FUNC16(اندراج_19_official);
    سیٹ_IVT(0x19, seabios);
}

// Attempt to map and initialize the option rom on a given PCI device.
ساكن فراغ
init_pcirom(بنية pci_device *pci, صحيح isvga, u64 *sources)
{
    dprintf(4, "Attempting to init PCI bdf %pP (vd %04x:%04x)\n"
            , pci, pci->vendor, pci->device);

    حرف fname[17];
    snprintf(fname, حجم(fname), "pci%04x,%04x.rom"
             , pci->vendor, pci->device);
    بنية romfile_s *file = romfile_لبھو(fname);
    بنية rom_سرخی *rom = عدم;
    إذا (file)
        rom = deploy_romfile(file);
    وإلا إذا (لگاتار_PCIroms > 1 || (لگاتار_PCIroms == 1 && isvga))
        rom = map_pcirom(pci);
    إذا (! rom)
        // No ROM present.
        أرجع;
    صحيح irq_was_captured = boot_irq_captured();
    بنية pnp_انکڑے *pnp = get_pnp_rom(rom);
    سیٹ_Rom_سروت(sources, rom, RS_PCIROM | (u32)pci);
    init_optionrom(rom, pci->bdf, isvga);
    إذا (boot_irq_captured() && !irq_was_captured &&
        !file && !isvga && pnp) {
        // This PCI rom is misbehaving - recapture the boot irqs
        حرف *desc = MAKE_FLATPTR(FLATPTR_منزل_SEG(rom), pnp->productname);
        dprintf(1, "PnP optionrom \"%s\" (bdf %pP) captured int19, restoring\n",
                desc, pci);
        boot_irq_restore();
    }
}


/****************************************************************
 * Non-VGA option rom init
 ****************************************************************/

فراغ
optionrom_تیاری(فراغ)
{
    إذا (! CONFIG_OPTIONROMS)
        أرجع;

    dprintf(1, "Scan for option roms\n");
    u64 sources[(BUILD_BIOS_ADDR - BUILD_ROM_شروع) / OPTION_ROM_ALIGN];
    memset(sources, 0, حجم(sources));
    u32 post_vga = rom_get_آخری();

    // Find and deploy PCI roms.
    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci->class == PCI_CLASS_DISPLAY_VGA ||
            pci->class == PCI_CLASS_DISPLAY_OTHER ||
            pci->have_driver)
            استمر;
        init_pcirom(pci, 0, sources);
    }

    // Find and deploy CBFS roms not associated with a device.
    لگاتار_file_roms("genroms/", 0, sources);
    rom_reserve(0);

    // All option roms found and deployed - now build BEV/BCV vectors.

    u32 pos = post_vga;
    طالما (pos < rom_get_آخری()) {
        بنية rom_سرخی *rom = (فراغ*)pos;
        إذا (! is_valid_rom(rom)) {
            pos += OPTION_ROM_ALIGN;
            استمر;
        }
        pos += ALIGN(rom->ناپ * 512, OPTION_ROM_ALIGN);
        بنية pnp_انکڑے *pnp = get_pnp_rom(rom);
        إذا (! pnp) {
            // Legacy rom.
            boot_جوڑو_bcv(FLATPTR_منزل_SEG(rom), OPTION_ROM_INITVECTOR, 0
                         , get_Rom_ترجیح(sources, rom, 0));
            استمر;
        }
        // PnP rom - check for BEV and BCV boot capabilities.
        صحيح instance = 0;
        طالما (pnp) {
            إذا (pnp->bev)
                boot_جوڑو_bev(FLATPTR_منزل_SEG(rom), pnp->bev, pnp->productname
                             , get_Rom_ترجیح(sources, rom, instance++));
            وإلا إذا (pnp->bcv)
                boot_جوڑو_bcv(FLATPTR_منزل_SEG(rom), pnp->bcv, pnp->productname
                             , get_Rom_ترجیح(sources, rom, instance++));
            وإلا
                اكسر;
            pnp = get_pnp_اگلا(rom, pnp);
        }
    }
}


/****************************************************************
 * VGA init
 ****************************************************************/

صحيح ScreenAndDebug;
بنية rom_سرخی *VgaROM;

ساكن فراغ try_تیاری_display_other(فراغ)
{
    بنية pci_device *pci;

    dprintf(1, "No VGA found, scan for other display\n");

    foreachpci(pci) {
        إذا (pci->class != PCI_CLASS_DISPLAY_OTHER)
            استمر;
        بنية rom_سرخی *rom = map_pcirom(pci);
        إذا (!rom)
            استمر;
        dprintf(1, "Other display found at %pP\n", pci);
        pci_config_maskw(pci->bdf, PCI_COMMAND, 0,
                         PCI_COMMAND_IO | PCI_COMMAND_حافظہ);
        init_optionrom(rom, pci->bdf, 1);
        أرجع;
    }
}

// Call into vga code to turn on console.
فراغ
vgarom_تیاری(فراغ)
{
    صحيح have_vga = 0;

    إذا (! CONFIG_OPTIONROMS)
        أرجع;

    dprintf(1, "Scan for VGA option rom\n");

    // Load some config settings that impact VGA.
    EnforceChecksum = romfile_loadint("etc/optionroms-checksum", 1);
    S3ResumeVga = romfile_loadint("etc/s3-resume-vga-init", CONFIG_QEMU);
    لگاتار_PCIroms = romfile_loadint("etc/pci-optionrom-exec", 2);
    ScreenAndDebug = romfile_loadint("etc/screen-and-debug", 1);

    // Clear option rom memory
    memset((فراغ*)BUILD_ROM_شروع, 0, rom_get_max() - BUILD_ROM_شروع);

    // Find and deploy PCI VGA rom.
    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (!is_pci_vga(pci))
            استمر;
        vgahook_تیاری(pci);
        init_pcirom(pci, 1, عدم);
        have_vga = 1;
        اكسر;
    }
    إذا (!have_vga)
        try_تیاری_display_other();

    // Find and deploy CBFS vga-style roms not associated with a device.
    لگاتار_file_roms("vgaroms/", 1, عدم);
    rom_reserve(0);

    إذا (rom_get_آخری() != BUILD_ROM_شروع)
        // VGA rom found
        VgaROM = (فراغ*)BUILD_ROM_شروع;
}

فراغ
s3_resume_vga(فراغ)
{
    إذا (!S3ResumeVga)
        أرجع;
    إذا (!VgaROM || ! is_valid_rom(VgaROM))
        أرجع;
    callrom(VgaROM, 0);
}

// Support for enabling/disabling BIOS ram shadowing.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "dev-q35.h" // PCI_VENDOR_ID_INTEL
#ਅਨ੍ਤਰ੍ਭਾਵ "dev-piix.h" // I440FX_PAM0
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci.h" // pci_config_writeb
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/pci_regs.h" // PCI_VENDOR_ID
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // rom_get_last
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "paravirt.h" // runningOnXen
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // make_bios_writable
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // wbinvd

// On the emulators, the bios at 0xf0000 is also at 0xffff0000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BIOS_SRC_ਵਿੱਥ 0xfff00000

ਸੰਘ pamdata_u {
    u8 data8[8];
    u32 data32[2];
};

// Enable shadowing and copy bios.
ਸਥਿਰ ਖਾਲੀ
__make_bios_writable_intel(u16 bdf, u32 pam0)
{
    // Read in current PAM settings from pci config space
    ਸੰਘ pamdata_u pamdata;
    pamdata.data32[0] = pci_ioconfig_readl(bdf, ALIGN_ਥੱਲੇ(pam0, 4));
    pamdata.data32[1] = pci_ioconfig_readl(bdf, ALIGN_ਥੱਲੇ(pam0, 4) + 4);
    u8 *pam = &pamdata.data8[pam0 & 0x03];

    // Make ram from 0xc0000-0xf0000 writable
    ਪੂਰਨ i;
    ਲਈ (i=0; i<6; i++)
        pam[i + 1] = 0x33;

    // Make ram from 0xf0000-0x100000 writable
    ਪੂਰਨ ram_present = pam[0] & 0x10;
    pam[0] = 0x30;

    // Write PAM settings back to pci config space
    pci_ioconfig_writel(bdf, ALIGN_ਥੱਲੇ(pam0, 4), pamdata.data32[0]);
    pci_ioconfig_writel(bdf, ALIGN_ਥੱਲੇ(pam0, 4) + 4, pamdata.data32[1]);

    ਜੇ (!ram_present)
        // Copy bios.
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(VSYMBOL(code32flat_ਸ਼ੁਰੂ)
               , VSYMBOL(code32flat_ਸ਼ੁਰੂ) + BIOS_SRC_ਵਿੱਥ
               , ਚਿੰਨ੍ਹ(code32flat_ਸਿਰਾ) - ਚਿੰਨ੍ਹ(code32flat_ਸ਼ੁਰੂ));
}

ਸਥਿਰ ਖਾਲੀ
make_bios_writable_intel(u16 bdf, u32 pam0)
{
    ਪੂਰਨ reg = pci_ioconfig_readb(bdf, pam0);
    ਜੇ (!(reg & 0x10)) {
        // QEMU doesn't fully implement the piix shadow capabilities -
        // if ram isn't backing the bios segment when shadowing is
        // disabled, the code itself won't be in memory.  So, run the
        // code from the high-memory flash location.
        u32 pos = (u32)__make_bios_writable_intel + BIOS_SRC_ਵਿੱਥ;
        ਖਾਲੀ (*func)(u16 bdf, u32 pam0) = (ਖਾਲੀ*)pos;
        func(bdf, pam0);
        ਵਾਪਸ;
    }
    // Ram already present - just enable writes
    __make_bios_writable_intel(bdf, pam0);
}

ਸਥਿਰ ਖਾਲੀ
make_bios_readonly_intel(u16 bdf, u32 pam0)
{
    // Flush any pending writes before locking memory.
    wbinvd();

    // Read in current PAM settings from pci config space
    ਸੰਘ pamdata_u pamdata;
    pamdata.data32[0] = pci_config_readl(bdf, ALIGN_ਥੱਲੇ(pam0, 4));
    pamdata.data32[1] = pci_config_readl(bdf, ALIGN_ਥੱਲੇ(pam0, 4) + 4);
    u8 *pam = &pamdata.data8[pam0 & 0x03];

    // Write protect roms from 0xc0000-0xf0000
    u32 romlast = BUILD_BIOS_ADDR, rommax = BUILD_BIOS_ADDR;
    ਜੇ (CONFIG_WRITABLE_UPPERMEMORY)
        romlast = rom_get_ਆਖਰੀ();
    ਜੇ (CONFIG_MALLOC_UPPERMEMORY)
        rommax = rom_get_max();
    ਪੂਰਨ i;
    ਲਈ (i=0; i<6; i++) {
        u32 mem = BUILD_ROM_ਸ਼ੁਰੂ + i * 32*1024;
        ਜੇ (romlast < mem + 16*1024 || rommax < mem + 32*1024) {
            ਜੇ (romlast >= mem && rommax >= mem + 16*1024)
                pam[i + 1] = 0x31;
            ਰੋਕੋ;
        }
        pam[i + 1] = 0x11;
    }

    // Write protect 0xf0000-0x100000
    pam[0] = 0x10;

    // Write PAM settings back to pci config space
    pci_config_writel(bdf, ALIGN_ਥੱਲੇ(pam0, 4), pamdata.data32[0]);
    pci_config_writel(bdf, ALIGN_ਥੱਲੇ(pam0, 4) + 4, pamdata.data32[1]);
}

ਸਥਿਰ ਪੂਰਨ ShadowBDF = -1;

// Make the 0xc0000-0x100000 area read/writable.
ਖਾਲੀ
make_bios_writable(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_QEMU || runningOnXen())
        ਵਾਪਸ;

    dprintf(3, "enabling shadow ram\n");

    // At this point, statically allocated variables can't be written,
    // so do this search manually.
    ਪੂਰਨ bdf;
    pci_ioconfig_foreachbdf(bdf, 0) {
        u32 vendev = pci_ioconfig_readl(bdf, PCI_VENDOR_ID);
        u16 vendor = vendev & 0xffff, device = vendev >> 16;
        ਜੇ (vendor == PCI_VENDOR_ID_INTEL
            && device == PCI_DEVICE_ID_INTEL_82441) {
            make_bios_writable_intel(bdf, I440FX_PAM0);
            ਕੋਡ_mutable_preinit();
            ShadowBDF = bdf;
            ਵਾਪਸ;
        }
        ਜੇ (vendor == PCI_VENDOR_ID_INTEL
            && device == PCI_DEVICE_ID_INTEL_Q35_MCH) {
            make_bios_writable_intel(bdf, Q35_HOST_BRIDGE_PAM0);
            ਕੋਡ_mutable_preinit();
            ShadowBDF = bdf;
            ਵਾਪਸ;
        }
    }
    dprintf(1, "Unable to unlock ram - bridge not found\n");
}

// Make the BIOS code segment area (0xf0000) read-only.
ਖਾਲੀ
make_bios_readonly(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_QEMU || runningOnXen())
        ਵਾਪਸ;
    dprintf(3, "locking shadow ram\n");

    ਜੇ (ShadowBDF < 0) {
        dprintf(1, "Unable to lock ram - bridge not found\n");
        ਵਾਪਸ;
    }

    u16 device = pci_config_readw(ShadowBDF, PCI_DEVICE_ID);
    ਜੇ (device == PCI_DEVICE_ID_INTEL_82441)
        make_bios_readonly_intel(ShadowBDF, I440FX_PAM0);
    ਨਹੀਂਤਾਂ
        make_bios_readonly_intel(ShadowBDF, Q35_HOST_BRIDGE_PAM0);
}

ਖਾਲੀ
qemu_reboot(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_QEMU || runningOnXen())
        ਵਾਪਸ;
    // QEMU doesn't map 0xc0000-0xfffff back to the original rom on a
    // reset, so do that manually before invoking a hard reset.
    ਖਾਲੀ *flash = (ਖਾਲੀ*)BIOS_SRC_ਵਿੱਥ;
    u32 hrp = (u32)&Have_ਲੜੀਵਾਰ_Post;
    ਜੇ (readl(flash + hrp)) {
        // There isn't a pristine copy of the BIOS at 0xffff0000 to copy
        ਜੇ (Have_ਲੜੀਵਾਰ_Post == 3) {
            // In a reboot loop.  Try to shutdown the machine instead.
            dprintf(1, "Unable to hard-reboot machine - attempting shutdown.\n");
            apm_shutdown();
        }
        make_bios_writable();
        Have_ਲੜੀਵਾਰ_Post = 3;
    } ਨਹੀਂਤਾਂ {
        // Copy the BIOS making sure to only reset HaveRunPost at end
        make_bios_writable();
        u32 cstart = ਚਿੰਨ੍ਹ(code32flat_ਸ਼ੁਰੂ), cend = ਚਿੰਨ੍ਹ(code32flat_ਸਿਰਾ);
        ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਖਾਲੀ*)cstart, flash + cstart, hrp - cstart);
        ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਖਾਲੀ*)hrp + 4, flash + hrp + 4, cend - (hrp + 4));
        barrier();
        Have_ਲੜੀਵਾਰ_Post = 0;
        barrier();
    }

    // Request a QEMU system reset.  Do the reset in this function as
    // the BIOS code was overwritten above and not all BIOS
    // functionality may be available.

    // Attempt PCI style reset
    outb(0x02, ਪੋਰਟ_PCI_REBOOT);
    outb(0x06, ਪੋਰਟ_PCI_REBOOT);

    // Next try triple faulting the CPU to force a reset
    asm ਅਸਥਿਰ("int3");
}

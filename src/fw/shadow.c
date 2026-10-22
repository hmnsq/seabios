// Support for enabling/disabling BIOS ram shadowing.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "dev-q35.h" // PCI_VENDOR_ID_INTEL
#ಅನ್ತರ್ಭಾವ "dev-piix.h" // I440FX_PAM0
#ಅನ್ತರ್ಭಾವ "hw/pci.h" // pci_config_writeb
#ಅನ್ತರ್ಭಾವ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#ಅನ್ತರ್ಭಾವ "hw/pci_regs.h" // PCI_VENDOR_ID
#ಅನ್ತರ್ಭಾವ "malloc.h" // rom_get_last
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "paravirt.h" // runningOnXen
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // make_bios_writable
#ಅನ್ತರ್ಭಾವ "x86.h" // wbinvd

// On the emulators, the bios at 0xf0000 is also at 0xffff0000
#ವ್ಯಾಖ್ಯಾನಿಸು BIOS_SRC_ಸ್ಥಾನಾಂತರ 0xfff00000

ಒಕ್ಕೂಟ pamdata_u {
    u8 data8[8];
    u32 data32[2];
};

// Enable shadowing and copy bios.
ಸ್ಥಿರ ಶೂನ್ಯ
__make_bios_writable_intel(u16 bdf, u32 pam0)
{
    // Read in current PAM settings from pci config space
    ಒಕ್ಕೂಟ pamdata_u pamdata;
    pamdata.data32[0] = pci_ioconfig_readl(bdf, ALIGN_ಕೆಳಕ್ಕೆ(pam0, 4));
    pamdata.data32[1] = pci_ioconfig_readl(bdf, ALIGN_ಕೆಳಕ್ಕೆ(pam0, 4) + 4);
    u8 *pam = &pamdata.data8[pam0 & 0x03];

    // Make ram from 0xc0000-0xf0000 writable
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<6; i++)
        pam[i + 1] = 0x33;

    // Make ram from 0xf0000-0x100000 writable
    ಪೂರ್ಣಾಂಕ ram_present = pam[0] & 0x10;
    pam[0] = 0x30;

    // Write PAM settings back to pci config space
    pci_ioconfig_writel(bdf, ALIGN_ಕೆಳಕ್ಕೆ(pam0, 4), pamdata.data32[0]);
    pci_ioconfig_writel(bdf, ALIGN_ಕೆಳಕ್ಕೆ(pam0, 4) + 4, pamdata.data32[1]);

    ಆದರೆ (!ram_present)
        // Copy bios.
        ಸ್ಮರಣೆ_ನಕಲು(VSYMBOL(code32flat_ಆರಂಭ)
               , VSYMBOL(code32flat_ಆರಂಭ) + BIOS_SRC_ಸ್ಥಾನಾಂತರ
               , ಚಿಹ್ನೆ(code32flat_ಕೊನೆ) - ಚಿಹ್ನೆ(code32flat_ಆರಂಭ));
}

ಸ್ಥಿರ ಶೂನ್ಯ
make_bios_writable_intel(u16 bdf, u32 pam0)
{
    ಪೂರ್ಣಾಂಕ reg = pci_ioconfig_readb(bdf, pam0);
    ಆದರೆ (!(reg & 0x10)) {
        // QEMU doesn't fully implement the piix shadow capabilities -
        // if ram isn't backing the bios segment when shadowing is
        // disabled, the code itself won't be in memory.  So, run the
        // code from the high-memory flash location.
        u32 pos = (u32)__make_bios_writable_intel + BIOS_SRC_ಸ್ಥಾನಾಂತರ;
        ಶೂನ್ಯ (*func)(u16 bdf, u32 pam0) = (ಶೂನ್ಯ*)pos;
        func(bdf, pam0);
        ಹಿಂತಿರುಗಿ;
    }
    // Ram already present - just enable writes
    __make_bios_writable_intel(bdf, pam0);
}

ಸ್ಥಿರ ಶೂನ್ಯ
make_bios_readonly_intel(u16 bdf, u32 pam0)
{
    // Flush any pending writes before locking memory.
    wbinvd();

    // Read in current PAM settings from pci config space
    ಒಕ್ಕೂಟ pamdata_u pamdata;
    pamdata.data32[0] = pci_config_readl(bdf, ALIGN_ಕೆಳಕ್ಕೆ(pam0, 4));
    pamdata.data32[1] = pci_config_readl(bdf, ALIGN_ಕೆಳಕ್ಕೆ(pam0, 4) + 4);
    u8 *pam = &pamdata.data8[pam0 & 0x03];

    // Write protect roms from 0xc0000-0xf0000
    u32 romlast = BUILD_BIOS_ADDR, rommax = BUILD_BIOS_ADDR;
    ಆದರೆ (CONFIG_WRITABLE_UPPERMEMORY)
        romlast = rom_get_ಕೊನೆಯ();
    ಆದರೆ (CONFIG_MALLOC_UPPERMEMORY)
        rommax = rom_get_max();
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<6; i++) {
        u32 mem = BUILD_ROM_ಆರಂಭ + i * 32*1024;
        ಆದರೆ (romlast < mem + 16*1024 || rommax < mem + 32*1024) {
            ಆದರೆ (romlast >= mem && rommax >= mem + 16*1024)
                pam[i + 1] = 0x31;
            ನಿಲ್ಲಿಸು;
        }
        pam[i + 1] = 0x11;
    }

    // Write protect 0xf0000-0x100000
    pam[0] = 0x10;

    // Write PAM settings back to pci config space
    pci_config_writel(bdf, ALIGN_ಕೆಳಕ್ಕೆ(pam0, 4), pamdata.data32[0]);
    pci_config_writel(bdf, ALIGN_ಕೆಳಕ್ಕೆ(pam0, 4) + 4, pamdata.data32[1]);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ShadowBDF = -1;

// Make the 0xc0000-0x100000 area read/writable.
ಶೂನ್ಯ
make_bios_writable(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_QEMU || runningOnXen())
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "enabling shadow ram\n");

    // At this point, statically allocated variables can't be written,
    // so do this search manually.
    ಪೂರ್ಣಾಂಕ bdf;
    pci_ioconfig_foreachbdf(bdf, 0) {
        u32 vendev = pci_ioconfig_readl(bdf, PCI_VENDOR_ID);
        u16 vendor = vendev & 0xffff, device = vendev >> 16;
        ಆದರೆ (vendor == PCI_VENDOR_ID_INTEL
            && device == PCI_DEVICE_ID_INTEL_82441) {
            make_bios_writable_intel(bdf, I440FX_PAM0);
            ಸಂಕೇತ_mutable_preinit();
            ShadowBDF = bdf;
            ಹಿಂತಿರುಗಿ;
        }
        ಆದರೆ (vendor == PCI_VENDOR_ID_INTEL
            && device == PCI_DEVICE_ID_INTEL_Q35_MCH) {
            make_bios_writable_intel(bdf, Q35_HOST_BRIDGE_PAM0);
            ಸಂಕೇತ_mutable_preinit();
            ShadowBDF = bdf;
            ಹಿಂತಿರುಗಿ;
        }
    }
    dprintf(1, "Unable to unlock ram - bridge not found\n");
}

// Make the BIOS code segment area (0xf0000) read-only.
ಶೂನ್ಯ
make_bios_readonly(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_QEMU || runningOnXen())
        ಹಿಂತಿರುಗಿ;
    dprintf(3, "locking shadow ram\n");

    ಆದರೆ (ShadowBDF < 0) {
        dprintf(1, "Unable to lock ram - bridge not found\n");
        ಹಿಂತಿರುಗಿ;
    }

    u16 device = pci_config_readw(ShadowBDF, PCI_DEVICE_ID);
    ಆದರೆ (device == PCI_DEVICE_ID_INTEL_82441)
        make_bios_readonly_intel(ShadowBDF, I440FX_PAM0);
    ಇಲ್ಲದಿದ್ದರೆ
        make_bios_readonly_intel(ShadowBDF, Q35_HOST_BRIDGE_PAM0);
}

ಶೂನ್ಯ
qemu_reboot(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_QEMU || runningOnXen())
        ಹಿಂತಿರುಗಿ;
    // QEMU doesn't map 0xc0000-0xfffff back to the original rom on a
    // reset, so do that manually before invoking a hard reset.
    ಶೂನ್ಯ *flash = (ಶೂನ್ಯ*)BIOS_SRC_ಸ್ಥಾನಾಂತರ;
    u32 hrp = (u32)&Have_ಸತತ_Post;
    ಆದರೆ (readl(flash + hrp)) {
        // There isn't a pristine copy of the BIOS at 0xffff0000 to copy
        ಆದರೆ (Have_ಸತತ_Post == 3) {
            // In a reboot loop.  Try to shutdown the machine instead.
            dprintf(1, "Unable to hard-reboot machine - attempting shutdown.\n");
            apm_shutdown();
        }
        make_bios_writable();
        Have_ಸತತ_Post = 3;
    } ಇಲ್ಲದಿದ್ದರೆ {
        // Copy the BIOS making sure to only reset HaveRunPost at end
        make_bios_writable();
        u32 cstart = ಚಿಹ್ನೆ(code32flat_ಆರಂಭ), cend = ಚಿಹ್ನೆ(code32flat_ಕೊನೆ);
        ಸ್ಮರಣೆ_ನಕಲು((ಶೂನ್ಯ*)cstart, flash + cstart, hrp - cstart);
        ಸ್ಮರಣೆ_ನಕಲು((ಶೂನ್ಯ*)hrp + 4, flash + hrp + 4, cend - (hrp + 4));
        barrier();
        Have_ಸತತ_Post = 0;
        barrier();
    }

    // Request a QEMU system reset.  Do the reset in this function as
    // the BIOS code was overwritten above and not all BIOS
    // functionality may be available.

    // Attempt PCI style reset
    outb(0x02, ಪೋರ್ಟ್_PCI_REBOOT);
    outb(0x06, ಪೋರ್ಟ್_PCI_REBOOT);

    // Next try triple faulting the CPU to force a reset
    asm ಅಸ್ಥಿರ("int3");
}

// Support for enabling/disabling BIOS ram shadowing.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "config.h" // CONFIG_*
#অন্তর্ভাৱ "dev-q35.h" // PCI_VENDOR_ID_INTEL
#অন্তর্ভাৱ "dev-piix.h" // I440FX_PAM0
#অন্তর্ভাৱ "hw/pci.h" // pci_config_writeb
#অন্তর্ভাৱ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#অন্তর্ভাৱ "hw/pci_regs.h" // PCI_VENDOR_ID
#অন্তর্ভাৱ "malloc.h" // rom_get_last
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "paravirt.h" // runningOnXen
#অন্তর্ভাৱ "string.h" // memset
#অন্তর্ভাৱ "util.h" // make_bios_writable
#অন্তর্ভাৱ "x86.h" // wbinvd

// On the emulators, the bios at 0xf0000 is also at 0xffff0000
#define BIOS_SRC_OFFSET 0xfff00000

সঙ্ঘ pamdata_u {
    u8 data8[8];
    u32 data32[2];
};

// Enable shadowing and copy bios.
স্থির শূন্য
__make_bios_writable_intel(u16 bdf, u32 pam0)
{
    // Read in current PAM settings from pci config space
    সঙ্ঘ pamdata_u pamdata;
    pamdata.data32[0] = pci_ioconfig_readl(bdf, ALIGN_DOWN(pam0, 4));
    pamdata.data32[1] = pci_ioconfig_readl(bdf, ALIGN_DOWN(pam0, 4) + 4);
    u8 *pam = &pamdata.data8[pam0 & 0x03];

    // Make ram from 0xc0000-0xf0000 writable
    পূর্ণাঙ্ক i;
    বাবে (i=0; i<6; i++)
        pam[i + 1] = 0x33;

    // Make ram from 0xf0000-0x100000 writable
    পূর্ণাঙ্ক ram_present = pam[0] & 0x10;
    pam[0] = 0x30;

    // Write PAM settings back to pci config space
    pci_ioconfig_writel(bdf, ALIGN_DOWN(pam0, 4), pamdata.data32[0]);
    pci_ioconfig_writel(bdf, ALIGN_DOWN(pam0, 4) + 4, pamdata.data32[1]);

    যদি (!ram_present)
        // Copy bios.
        memcpy(VSYMBOL(code32flat_start)
               , VSYMBOL(code32flat_start) + BIOS_SRC_OFFSET
               , প্ৰতীক(code32flat_end) - প্ৰতীক(code32flat_start));
}

স্থির শূন্য
make_bios_writable_intel(u16 bdf, u32 pam0)
{
    পূর্ণাঙ্ক reg = pci_ioconfig_readb(bdf, pam0);
    যদি (!(reg & 0x10)) {
        // QEMU doesn't fully implement the piix shadow capabilities -
        // if ram isn't backing the bios segment when shadowing is
        // disabled, the code itself won't be in memory.  So, run the
        // code from the high-memory flash location.
        u32 pos = (u32)__make_bios_writable_intel + BIOS_SRC_OFFSET;
        শূন্য (*func)(u16 bdf, u32 pam0) = (শূন্য*)pos;
        func(bdf, pam0);
        ঘূৰাই;
    }
    // Ram already present - just enable writes
    __make_bios_writable_intel(bdf, pam0);
}

স্থির শূন্য
make_bios_readonly_intel(u16 bdf, u32 pam0)
{
    // Flush any pending writes before locking memory.
    wbinvd();

    // Read in current PAM settings from pci config space
    সঙ্ঘ pamdata_u pamdata;
    pamdata.data32[0] = pci_config_readl(bdf, ALIGN_DOWN(pam0, 4));
    pamdata.data32[1] = pci_config_readl(bdf, ALIGN_DOWN(pam0, 4) + 4);
    u8 *pam = &pamdata.data8[pam0 & 0x03];

    // Write protect roms from 0xc0000-0xf0000
    u32 romlast = BUILD_BIOS_ADDR, rommax = BUILD_BIOS_ADDR;
    যদি (CONFIG_WRITABLE_UPPERMEMORY)
        romlast = rom_get_last();
    যদি (CONFIG_MALLOC_UPPERMEMORY)
        rommax = rom_get_max();
    পূর্ণাঙ্ক i;
    বাবে (i=0; i<6; i++) {
        u32 mem = BUILD_ROM_START + i * 32*1024;
        যদি (romlast < mem + 16*1024 || rommax < mem + 32*1024) {
            যদি (romlast >= mem && rommax >= mem + 16*1024)
                pam[i + 1] = 0x31;
            ভঙ্গ;
        }
        pam[i + 1] = 0x11;
    }

    // Write protect 0xf0000-0x100000
    pam[0] = 0x10;

    // Write PAM settings back to pci config space
    pci_config_writel(bdf, ALIGN_DOWN(pam0, 4), pamdata.data32[0]);
    pci_config_writel(bdf, ALIGN_DOWN(pam0, 4) + 4, pamdata.data32[1]);
}

স্থির পূর্ণাঙ্ক ShadowBDF = -1;

// Make the 0xc0000-0x100000 area read/writable.
শূন্য
make_bios_writable(শূন্য)
{
    যদি (!CONFIG_QEMU || runningOnXen())
        ঘূৰাই;

    dprintf(3, "enabling shadow ram\n");

    // At this point, statically allocated variables can't be written,
    // so do this search manually.
    পূর্ণাঙ্ক bdf;
    pci_ioconfig_foreachbdf(bdf, 0) {
        u32 vendev = pci_ioconfig_readl(bdf, PCI_VENDOR_ID);
        u16 vendor = vendev & 0xffff, device = vendev >> 16;
        যদি (vendor == PCI_VENDOR_ID_INTEL
            && device == PCI_DEVICE_ID_INTEL_82441) {
            make_bios_writable_intel(bdf, I440FX_PAM0);
            code_mutable_preinit();
            ShadowBDF = bdf;
            ঘূৰাই;
        }
        যদি (vendor == PCI_VENDOR_ID_INTEL
            && device == PCI_DEVICE_ID_INTEL_Q35_MCH) {
            make_bios_writable_intel(bdf, Q35_HOST_BRIDGE_PAM0);
            code_mutable_preinit();
            ShadowBDF = bdf;
            ঘূৰাই;
        }
    }
    dprintf(1, "Unable to unlock ram - bridge not found\n");
}

// Make the BIOS code segment area (0xf0000) read-only.
শূন্য
make_bios_readonly(শূন্য)
{
    যদি (!CONFIG_QEMU || runningOnXen())
        ঘূৰাই;
    dprintf(3, "locking shadow ram\n");

    যদি (ShadowBDF < 0) {
        dprintf(1, "Unable to lock ram - bridge not found\n");
        ঘূৰাই;
    }

    u16 device = pci_config_readw(ShadowBDF, PCI_DEVICE_ID);
    যদি (device == PCI_DEVICE_ID_INTEL_82441)
        make_bios_readonly_intel(ShadowBDF, I440FX_PAM0);
    অন্যথা
        make_bios_readonly_intel(ShadowBDF, Q35_HOST_BRIDGE_PAM0);
}

শূন্য
qemu_reboot(শূন্য)
{
    যদি (!CONFIG_QEMU || runningOnXen())
        ঘূৰাই;
    // QEMU doesn't map 0xc0000-0xfffff back to the original rom on a
    // reset, so do that manually before invoking a hard reset.
    শূন্য *flash = (শূন্য*)BIOS_SRC_OFFSET;
    u32 hrp = (u32)&HaveRunPost;
    যদি (readl(flash + hrp)) {
        // There isn't a pristine copy of the BIOS at 0xffff0000 to copy
        যদি (HaveRunPost == 3) {
            // In a reboot loop.  Try to shutdown the machine instead.
            dprintf(1, "Unable to hard-reboot machine - attempting shutdown.\n");
            apm_shutdown();
        }
        make_bios_writable();
        HaveRunPost = 3;
    } অন্যথা {
        // Copy the BIOS making sure to only reset HaveRunPost at end
        make_bios_writable();
        u32 cstart = প্ৰতীক(code32flat_start), cend = প্ৰতীক(code32flat_end);
        memcpy((শূন্য*)cstart, flash + cstart, hrp - cstart);
        memcpy((শূন্য*)hrp + 4, flash + hrp + 4, cend - (hrp + 4));
        barrier();
        HaveRunPost = 0;
        barrier();
    }

    // Request a QEMU system reset.  Do the reset in this function as
    // the BIOS code was overwritten above and not all BIOS
    // functionality may be available.

    // Attempt PCI style reset
    outb(0x02, PORT_PCI_REBOOT);
    outb(0x06, PORT_PCI_REBOOT);

    // Next try triple faulting the CPU to force a reset
    asm চঞ্চল("int3");
}

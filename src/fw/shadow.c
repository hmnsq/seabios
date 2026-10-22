// Support for enabling/disabling BIOS ram shadowing.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "dev-q35.h" // PCI_VENDOR_ID_INTEL
#అన్తర్భావ "dev-piix.h" // I440FX_PAM0
#అన్తర్భావ "hw/pci.h" // pci_config_writeb
#అన్తర్భావ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#అన్తర్భావ "hw/pci_regs.h" // PCI_VENDOR_ID
#అన్తర్భావ "malloc.h" // rom_get_last
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "paravirt.h" // runningOnXen
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // make_bios_writable
#అన్తర్భావ "x86.h" // wbinvd

// On the emulators, the bios at 0xf0000 is also at 0xffff0000
#నిర్వచించు BIOS_SRC_స్థానభ్రంశం 0xfff00000

సంఘం pamdata_u {
    u8 data8[8];
    u32 data32[2];
};

// Enable shadowing and copy bios.
స్థిర శూన్య
__make_bios_writable_intel(u16 bdf, u32 pam0)
{
    // Read in current PAM settings from pci config space
    సంఘం pamdata_u pamdata;
    pamdata.data32[0] = pci_ioconfig_readl(bdf, ALIGN_కిందికి(pam0, 4));
    pamdata.data32[1] = pci_ioconfig_readl(bdf, ALIGN_కిందికి(pam0, 4) + 4);
    u8 *pam = &pamdata.data8[pam0 & 0x03];

    // Make ram from 0xc0000-0xf0000 writable
    పూర్ణాంకం i;
    కొరకు (i=0; i<6; i++)
        pam[i + 1] = 0x33;

    // Make ram from 0xf0000-0x100000 writable
    పూర్ణాంకం ram_present = pam[0] & 0x10;
    pam[0] = 0x30;

    // Write PAM settings back to pci config space
    pci_ioconfig_writel(bdf, ALIGN_కిందికి(pam0, 4), pamdata.data32[0]);
    pci_ioconfig_writel(bdf, ALIGN_కిందికి(pam0, 4) + 4, pamdata.data32[1]);

    ఒకవేళ (!ram_present)
        // Copy bios.
        జ్ఞాపకం_నకలు(VSYMBOL(code32flat_ప్రారంభం)
               , VSYMBOL(code32flat_ప్రారంభం) + BIOS_SRC_స్థానభ్రంశం
               , చిహ్నం(code32flat_చివర) - చిహ్నం(code32flat_ప్రారంభం));
}

స్థిర శూన్య
make_bios_writable_intel(u16 bdf, u32 pam0)
{
    పూర్ణాంకం reg = pci_ioconfig_readb(bdf, pam0);
    ఒకవేళ (!(reg & 0x10)) {
        // QEMU doesn't fully implement the piix shadow capabilities -
        // if ram isn't backing the bios segment when shadowing is
        // disabled, the code itself won't be in memory.  So, run the
        // code from the high-memory flash location.
        u32 pos = (u32)__make_bios_writable_intel + BIOS_SRC_స్థానభ్రంశం;
        శూన్య (*func)(u16 bdf, u32 pam0) = (శూన్య*)pos;
        func(bdf, pam0);
        తిరిగి;
    }
    // Ram already present - just enable writes
    __make_bios_writable_intel(bdf, pam0);
}

స్థిర శూన్య
make_bios_readonly_intel(u16 bdf, u32 pam0)
{
    // Flush any pending writes before locking memory.
    wbinvd();

    // Read in current PAM settings from pci config space
    సంఘం pamdata_u pamdata;
    pamdata.data32[0] = pci_config_readl(bdf, ALIGN_కిందికి(pam0, 4));
    pamdata.data32[1] = pci_config_readl(bdf, ALIGN_కిందికి(pam0, 4) + 4);
    u8 *pam = &pamdata.data8[pam0 & 0x03];

    // Write protect roms from 0xc0000-0xf0000
    u32 romlast = BUILD_BIOS_ADDR, rommax = BUILD_BIOS_ADDR;
    ఒకవేళ (CONFIG_WRITABLE_UPPERMEMORY)
        romlast = rom_get_చివరి();
    ఒకవేళ (CONFIG_MALLOC_UPPERMEMORY)
        rommax = rom_get_max();
    పూర్ణాంకం i;
    కొరకు (i=0; i<6; i++) {
        u32 mem = BUILD_ROM_ప్రారంభం + i * 32*1024;
        ఒకవేళ (romlast < mem + 16*1024 || rommax < mem + 32*1024) {
            ఒకవేళ (romlast >= mem && rommax >= mem + 16*1024)
                pam[i + 1] = 0x31;
            ఆపు;
        }
        pam[i + 1] = 0x11;
    }

    // Write protect 0xf0000-0x100000
    pam[0] = 0x10;

    // Write PAM settings back to pci config space
    pci_config_writel(bdf, ALIGN_కిందికి(pam0, 4), pamdata.data32[0]);
    pci_config_writel(bdf, ALIGN_కిందికి(pam0, 4) + 4, pamdata.data32[1]);
}

స్థిర పూర్ణాంకం ShadowBDF = -1;

// Make the 0xc0000-0x100000 area read/writable.
శూన్య
make_bios_writable(శూన్య)
{
    ఒకవేళ (!CONFIG_QEMU || runningOnXen())
        తిరిగి;

    dprintf(3, "enabling shadow ram\n");

    // At this point, statically allocated variables can't be written,
    // so do this search manually.
    పూర్ణాంకం bdf;
    pci_ioconfig_foreachbdf(bdf, 0) {
        u32 vendev = pci_ioconfig_readl(bdf, PCI_VENDOR_ID);
        u16 vendor = vendev & 0xffff, device = vendev >> 16;
        ఒకవేళ (vendor == PCI_VENDOR_ID_INTEL
            && device == PCI_DEVICE_ID_INTEL_82441) {
            make_bios_writable_intel(bdf, I440FX_PAM0);
            సంకేతం_mutable_preinit();
            ShadowBDF = bdf;
            తిరిగి;
        }
        ఒకవేళ (vendor == PCI_VENDOR_ID_INTEL
            && device == PCI_DEVICE_ID_INTEL_Q35_MCH) {
            make_bios_writable_intel(bdf, Q35_HOST_BRIDGE_PAM0);
            సంకేతం_mutable_preinit();
            ShadowBDF = bdf;
            తిరిగి;
        }
    }
    dprintf(1, "Unable to unlock ram - bridge not found\n");
}

// Make the BIOS code segment area (0xf0000) read-only.
శూన్య
make_bios_readonly(శూన్య)
{
    ఒకవేళ (!CONFIG_QEMU || runningOnXen())
        తిరిగి;
    dprintf(3, "locking shadow ram\n");

    ఒకవేళ (ShadowBDF < 0) {
        dprintf(1, "Unable to lock ram - bridge not found\n");
        తిరిగి;
    }

    u16 device = pci_config_readw(ShadowBDF, PCI_DEVICE_ID);
    ఒకవేళ (device == PCI_DEVICE_ID_INTEL_82441)
        make_bios_readonly_intel(ShadowBDF, I440FX_PAM0);
    లేకపోతే
        make_bios_readonly_intel(ShadowBDF, Q35_HOST_BRIDGE_PAM0);
}

శూన్య
qemu_reboot(శూన్య)
{
    ఒకవేళ (!CONFIG_QEMU || runningOnXen())
        తిరిగి;
    // QEMU doesn't map 0xc0000-0xfffff back to the original rom on a
    // reset, so do that manually before invoking a hard reset.
    శూన్య *flash = (శూన్య*)BIOS_SRC_స్థానభ్రంశం;
    u32 hrp = (u32)&Have_నిరంతరం_Post;
    ఒకవేళ (readl(flash + hrp)) {
        // There isn't a pristine copy of the BIOS at 0xffff0000 to copy
        ఒకవేళ (Have_నిరంతరం_Post == 3) {
            // In a reboot loop.  Try to shutdown the machine instead.
            dprintf(1, "Unable to hard-reboot machine - attempting shutdown.\n");
            apm_shutdown();
        }
        make_bios_writable();
        Have_నిరంతరం_Post = 3;
    } లేకపోతే {
        // Copy the BIOS making sure to only reset HaveRunPost at end
        make_bios_writable();
        u32 cstart = చిహ్నం(code32flat_ప్రారంభం), cend = చిహ్నం(code32flat_చివర);
        జ్ఞాపకం_నకలు((శూన్య*)cstart, flash + cstart, hrp - cstart);
        జ్ఞాపకం_నకలు((శూన్య*)hrp + 4, flash + hrp + 4, cend - (hrp + 4));
        barrier();
        Have_నిరంతరం_Post = 0;
        barrier();
    }

    // Request a QEMU system reset.  Do the reset in this function as
    // the BIOS code was overwritten above and not all BIOS
    // functionality may be available.

    // Attempt PCI style reset
    outb(0x02, పోర్టు_PCI_REBOOT);
    outb(0x06, పోర్టు_PCI_REBOOT);

    // Next try triple faulting the CPU to force a reset
    asm అస్థిర("int3");
}

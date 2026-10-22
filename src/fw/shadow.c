// Support for enabling/disabling BIOS ram shadowing.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "dev-q35.h" // PCI_VENDOR_ID_INTEL
#अन्तर्भाव "dev-piix.h" // I440FX_PAM0
#अन्तर्भाव "hw/pci.h" // pci_config_writeb
#अन्तर्भाव "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#अन्तर्भाव "hw/pci_regs.h" // PCI_VENDOR_ID
#अन्तर्भाव "malloc.h" // rom_get_last
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "paravirt.h" // runningOnXen
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // make_bios_writable
#अन्तर्भाव "x86.h" // wbinvd

// On the emulators, the bios at 0xf0000 is also at 0xffff0000
#परिभाषय BIOS_SRC_विस्थापनम् 0xfff00000

सङ्घ pamdata_u {
    u8 data8[8];
    u32 data32[2];
};

// Enable shadowing and copy bios.
स्थिर शून्य
__make_bios_writable_intel(u16 bdf, u32 pam0)
{
    // Read in current PAM settings from pci config space
    सङ्घ pamdata_u pamdata;
    pamdata.data32[0] = pci_ioconfig_readl(bdf, ALIGN_अधस्तात्(pam0, 4));
    pamdata.data32[1] = pci_ioconfig_readl(bdf, ALIGN_अधस्तात्(pam0, 4) + 4);
    u8 *pam = &pamdata.data8[pam0 & 0x03];

    // Make ram from 0xc0000-0xf0000 writable
    पूर्णाङ्क i;
    प्रति (i=0; i<6; i++)
        pam[i + 1] = 0x33;

    // Make ram from 0xf0000-0x100000 writable
    पूर्णाङ्क ram_present = pam[0] & 0x10;
    pam[0] = 0x30;

    // Write PAM settings back to pci config space
    pci_ioconfig_writel(bdf, ALIGN_अधस्तात्(pam0, 4), pamdata.data32[0]);
    pci_ioconfig_writel(bdf, ALIGN_अधस्तात्(pam0, 4) + 4, pamdata.data32[1]);

    यदि (!ram_present)
        // Copy bios.
        स्मृतिं_प्रतिलिख(VSYMBOL(code32flat_start)
               , VSYMBOL(code32flat_start) + BIOS_SRC_विस्थापनम्
               , चिह्नम्(code32flat_अन्तः) - चिह्नम्(code32flat_start));
}

स्थिर शून्य
make_bios_writable_intel(u16 bdf, u32 pam0)
{
    पूर्णाङ्क reg = pci_ioconfig_readb(bdf, pam0);
    यदि (!(reg & 0x10)) {
        // QEMU doesn't fully implement the piix shadow capabilities -
        // if ram isn't backing the bios segment when shadowing is
        // disabled, the code itself won't be in memory.  So, run the
        // code from the high-memory flash location.
        u32 pos = (u32)__make_bios_writable_intel + BIOS_SRC_विस्थापनम्;
        शून्य (*func)(u16 bdf, u32 pam0) = (शून्य*)pos;
        func(bdf, pam0);
        प्रत्यागम;
    }
    // Ram already present - just enable writes
    __make_bios_writable_intel(bdf, pam0);
}

स्थिर शून्य
make_bios_readonly_intel(u16 bdf, u32 pam0)
{
    // Flush any pending writes before locking memory.
    wbinvd();

    // Read in current PAM settings from pci config space
    सङ्घ pamdata_u pamdata;
    pamdata.data32[0] = pci_config_readl(bdf, ALIGN_अधस्तात्(pam0, 4));
    pamdata.data32[1] = pci_config_readl(bdf, ALIGN_अधस्तात्(pam0, 4) + 4);
    u8 *pam = &pamdata.data8[pam0 & 0x03];

    // Write protect roms from 0xc0000-0xf0000
    u32 romlast = BUILD_BIOS_ADDR, rommax = BUILD_BIOS_ADDR;
    यदि (CONFIG_WRITABLE_UPPERMEMORY)
        romlast = rom_get_चरमः();
    यदि (CONFIG_MALLOC_UPPERMEMORY)
        rommax = rom_get_max();
    पूर्णाङ्क i;
    प्रति (i=0; i<6; i++) {
        u32 mem = BUILD_ROM_START + i * 32*1024;
        यदि (romlast < mem + 16*1024 || rommax < mem + 32*1024) {
            यदि (romlast >= mem && rommax >= mem + 16*1024)
                pam[i + 1] = 0x31;
            भङ्ग;
        }
        pam[i + 1] = 0x11;
    }

    // Write protect 0xf0000-0x100000
    pam[0] = 0x10;

    // Write PAM settings back to pci config space
    pci_config_writel(bdf, ALIGN_अधस्तात्(pam0, 4), pamdata.data32[0]);
    pci_config_writel(bdf, ALIGN_अधस्तात्(pam0, 4) + 4, pamdata.data32[1]);
}

स्थिर पूर्णाङ्क ShadowBDF = -1;

// Make the 0xc0000-0x100000 area read/writable.
शून्य
make_bios_writable(शून्य)
{
    यदि (!CONFIG_QEMU || runningOnXen())
        प्रत्यागम;

    dprintf(3, "enabling shadow ram\n");

    // At this point, statically allocated variables can't be written,
    // so do this search manually.
    पूर्णाङ्क bdf;
    pci_ioconfig_foreachbdf(bdf, 0) {
        u32 vendev = pci_ioconfig_readl(bdf, PCI_VENDOR_ID);
        u16 vendor = vendev & 0xffff, device = vendev >> 16;
        यदि (vendor == PCI_VENDOR_ID_INTEL
            && device == PCI_DEVICE_ID_INTEL_82441) {
            make_bios_writable_intel(bdf, I440FX_PAM0);
            संहिता_mutable_preinit();
            ShadowBDF = bdf;
            प्रत्यागम;
        }
        यदि (vendor == PCI_VENDOR_ID_INTEL
            && device == PCI_DEVICE_ID_INTEL_Q35_MCH) {
            make_bios_writable_intel(bdf, Q35_HOST_BRIDGE_PAM0);
            संहिता_mutable_preinit();
            ShadowBDF = bdf;
            प्रत्यागम;
        }
    }
    dprintf(1, "Unable to unlock ram - bridge not found\n");
}

// Make the BIOS code segment area (0xf0000) read-only.
शून्य
make_bios_readonly(शून्य)
{
    यदि (!CONFIG_QEMU || runningOnXen())
        प्रत्यागम;
    dprintf(3, "locking shadow ram\n");

    यदि (ShadowBDF < 0) {
        dprintf(1, "Unable to lock ram - bridge not found\n");
        प्रत्यागम;
    }

    u16 device = pci_config_readw(ShadowBDF, PCI_DEVICE_ID);
    यदि (device == PCI_DEVICE_ID_INTEL_82441)
        make_bios_readonly_intel(ShadowBDF, I440FX_PAM0);
    अन्यथा
        make_bios_readonly_intel(ShadowBDF, Q35_HOST_BRIDGE_PAM0);
}

शून्य
qemu_reboot(शून्य)
{
    यदि (!CONFIG_QEMU || runningOnXen())
        प्रत्यागम;
    // QEMU doesn't map 0xc0000-0xfffff back to the original rom on a
    // reset, so do that manually before invoking a hard reset.
    शून्य *flash = (शून्य*)BIOS_SRC_विस्थापनम्;
    u32 hrp = (u32)&Have_सततम्_Post;
    यदि (readl(flash + hrp)) {
        // There isn't a pristine copy of the BIOS at 0xffff0000 to copy
        यदि (Have_सततम्_Post == 3) {
            // In a reboot loop.  Try to shutdown the machine instead.
            dprintf(1, "Unable to hard-reboot machine - attempting shutdown.\n");
            apm_shutdown();
        }
        make_bios_writable();
        Have_सततम्_Post = 3;
    } अन्यथा {
        // Copy the BIOS making sure to only reset HaveRunPost at end
        make_bios_writable();
        u32 cstart = चिह्नम्(code32flat_start), cend = चिह्नम्(code32flat_अन्तः);
        स्मृतिं_प्रतिलिख((शून्य*)cstart, flash + cstart, hrp - cstart);
        स्मृतिं_प्रतिलिख((शून्य*)hrp + 4, flash + hrp + 4, cend - (hrp + 4));
        barrier();
        Have_सततम्_Post = 0;
        barrier();
    }

    // Request a QEMU system reset.  Do the reset in this function as
    // the BIOS code was overwritten above and not all BIOS
    // functionality may be available.

    // Attempt PCI style reset
    outb(0x02, पत्तनम्_PCI_REBOOT);
    outb(0x06, पत्तनम्_PCI_REBOOT);

    // Next try triple faulting the CPU to force a reset
    asm चञ्चल("int3");
}

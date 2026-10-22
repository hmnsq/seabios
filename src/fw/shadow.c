// Support for enabling/disabling BIOS ram shadowing.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "config.h" // CONFIG_*
#समावेश "dev-q35.h" // PCI_VENDOR_ID_INTEL
#समावेश "dev-piix.h" // I440FX_PAM0
#समावेश "hw/pci.h" // pci_config_writeb
#समावेश "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#समावेश "hw/pci_regs.h" // PCI_VENDOR_ID
#समावेश "malloc.h" // rom_get_last
#समावेश "output.h" // dprintf
#समावेश "paravirt.h" // runningOnXen
#समावेश "string.h" // memset
#समावेश "util.h" // make_bios_writable
#समावेश "x86.h" // wbinvd

// On the emulators, the bios at 0xf0000 is also at 0xffff0000
#घोषणा BIOS_SRC_OFFSET 0xfff00000

जोड़ pamdata_u {
    u8 data8[8];
    u32 data32[2];
};

// Enable shadowing and copy bios.
अटल व्योम
__make_bios_writable_intel(u16 bdf, u32 pam0)
{
    // Read in current PAM settings from pci config space
    जोड़ pamdata_u pamdata;
    pamdata.data32[0] = pci_ioconfig_readl(bdf, ALIGN_DOWN(pam0, 4));
    pamdata.data32[1] = pci_ioconfig_readl(bdf, ALIGN_DOWN(pam0, 4) + 4);
    u8 *pam = &pamdata.data8[pam0 & 0x03];

    // Make ram from 0xc0000-0xf0000 writable
    पूर्णांक i;
    क्रम (i=0; i<6; i++)
        pam[i + 1] = 0x33;

    // Make ram from 0xf0000-0x100000 writable
    पूर्णांक ram_present = pam[0] & 0x10;
    pam[0] = 0x30;

    // Write PAM settings back to pci config space
    pci_ioconfig_writel(bdf, ALIGN_DOWN(pam0, 4), pamdata.data32[0]);
    pci_ioconfig_writel(bdf, ALIGN_DOWN(pam0, 4) + 4, pamdata.data32[1]);

    अगर (!ram_present)
        // Copy bios.
        स_नकल(VSYMBOL(code32flat_start)
               , VSYMBOL(code32flat_start) + BIOS_SRC_OFFSET
               , प्रतीक(code32flat_end) - प्रतीक(code32flat_start));
}

अटल व्योम
make_bios_writable_intel(u16 bdf, u32 pam0)
{
    पूर्णांक reg = pci_ioconfig_readb(bdf, pam0);
    अगर (!(reg & 0x10)) {
        // QEMU doesn't fully implement the piix shadow capabilities -
        // if ram isn't backing the bios segment when shadowing is
        // disabled, the code itself won't be in memory.  So, run the
        // code from the high-memory flash location.
        u32 pos = (u32)__make_bios_writable_intel + BIOS_SRC_OFFSET;
        व्योम (*func)(u16 bdf, u32 pam0) = (व्योम*)pos;
        func(bdf, pam0);
        वापस;
    }
    // Ram already present - just enable writes
    __make_bios_writable_intel(bdf, pam0);
}

अटल व्योम
make_bios_readonly_intel(u16 bdf, u32 pam0)
{
    // Flush any pending writes before locking memory.
    wbinvd();

    // Read in current PAM settings from pci config space
    जोड़ pamdata_u pamdata;
    pamdata.data32[0] = pci_config_readl(bdf, ALIGN_DOWN(pam0, 4));
    pamdata.data32[1] = pci_config_readl(bdf, ALIGN_DOWN(pam0, 4) + 4);
    u8 *pam = &pamdata.data8[pam0 & 0x03];

    // Write protect roms from 0xc0000-0xf0000
    u32 romlast = BUILD_BIOS_ADDR, rommax = BUILD_BIOS_ADDR;
    अगर (CONFIG_WRITABLE_UPPERMEMORY)
        romlast = rom_get_last();
    अगर (CONFIG_MALLOC_UPPERMEMORY)
        rommax = rom_get_max();
    पूर्णांक i;
    क्रम (i=0; i<6; i++) {
        u32 mem = BUILD_ROM_START + i * 32*1024;
        अगर (romlast < mem + 16*1024 || rommax < mem + 32*1024) {
            अगर (romlast >= mem && rommax >= mem + 16*1024)
                pam[i + 1] = 0x31;
            अवरोध;
        }
        pam[i + 1] = 0x11;
    }

    // Write protect 0xf0000-0x100000
    pam[0] = 0x10;

    // Write PAM settings back to pci config space
    pci_config_writel(bdf, ALIGN_DOWN(pam0, 4), pamdata.data32[0]);
    pci_config_writel(bdf, ALIGN_DOWN(pam0, 4) + 4, pamdata.data32[1]);
}

अटल पूर्णांक ShadowBDF = -1;

// Make the 0xc0000-0x100000 area read/writable.
व्योम
make_bios_writable(व्योम)
{
    अगर (!CONFIG_QEMU || runningOnXen())
        वापस;

    dprintf(3, "enabling shadow ram\n");

    // At this point, statically allocated variables can't be written,
    // so do this search manually.
    पूर्णांक bdf;
    pci_ioconfig_foreachbdf(bdf, 0) {
        u32 vendev = pci_ioconfig_readl(bdf, PCI_VENDOR_ID);
        u16 vendor = vendev & 0xffff, device = vendev >> 16;
        अगर (vendor == PCI_VENDOR_ID_INTEL
            && device == PCI_DEVICE_ID_INTEL_82441) {
            make_bios_writable_intel(bdf, I440FX_PAM0);
            code_mutable_preinit();
            ShadowBDF = bdf;
            वापस;
        }
        अगर (vendor == PCI_VENDOR_ID_INTEL
            && device == PCI_DEVICE_ID_INTEL_Q35_MCH) {
            make_bios_writable_intel(bdf, Q35_HOST_BRIDGE_PAM0);
            code_mutable_preinit();
            ShadowBDF = bdf;
            वापस;
        }
    }
    dprintf(1, "Unable to unlock ram - bridge not found\n");
}

// Make the BIOS code segment area (0xf0000) read-only.
व्योम
make_bios_readonly(व्योम)
{
    अगर (!CONFIG_QEMU || runningOnXen())
        वापस;
    dprintf(3, "locking shadow ram\n");

    अगर (ShadowBDF < 0) {
        dprintf(1, "Unable to lock ram - bridge not found\n");
        वापस;
    }

    u16 device = pci_config_readw(ShadowBDF, PCI_DEVICE_ID);
    अगर (device == PCI_DEVICE_ID_INTEL_82441)
        make_bios_readonly_intel(ShadowBDF, I440FX_PAM0);
    अन्यथा
        make_bios_readonly_intel(ShadowBDF, Q35_HOST_BRIDGE_PAM0);
}

व्योम
qemu_reboot(व्योम)
{
    अगर (!CONFIG_QEMU || runningOnXen())
        वापस;
    // QEMU doesn't map 0xc0000-0xfffff back to the original rom on a
    // reset, so do that manually before invoking a hard reset.
    व्योम *flash = (व्योम*)BIOS_SRC_OFFSET;
    u32 hrp = (u32)&HaveRunPost;
    अगर (readl(flash + hrp)) {
        // There isn't a pristine copy of the BIOS at 0xffff0000 to copy
        अगर (HaveRunPost == 3) {
            // In a reboot loop.  Try to shutdown the machine instead.
            dprintf(1, "Unable to hard-reboot machine - attempting shutdown.\n");
            apm_shutdown();
        }
        make_bios_writable();
        HaveRunPost = 3;
    } अन्यथा {
        // Copy the BIOS making sure to only reset HaveRunPost at end
        make_bios_writable();
        u32 cstart = प्रतीक(code32flat_start), cend = प्रतीक(code32flat_end);
        स_नकल((व्योम*)cstart, flash + cstart, hrp - cstart);
        स_नकल((व्योम*)hrp + 4, flash + hrp + 4, cend - (hrp + 4));
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
    यंत्र अस्थिर("int3");
}

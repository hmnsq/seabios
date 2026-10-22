// Support for enabling/disabling BIOS ram shadowing.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "dev-q35.h" // PCI_VENDOR_ID_INTEL
#અન્તર્ભાવ "dev-piix.h" // I440FX_PAM0
#અન્તર્ભાવ "hw/pci.h" // pci_config_writeb
#અન્તર્ભાવ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#અન્તર્ભાવ "hw/pci_regs.h" // PCI_VENDOR_ID
#અન્તર્ભાવ "malloc.h" // rom_get_last
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "paravirt.h" // runningOnXen
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // make_bios_writable
#અન્તર્ભાવ "x86.h" // wbinvd

// On the emulators, the bios at 0xf0000 is also at 0xffff0000
#define BIOS_SRC_OFFSET 0xfff00000

સંઘ pamdata_u {
    u8 data8[8];
    u32 data32[2];
};

// Enable shadowing and copy bios.
સ્થિર શૂન્ય
__make_bios_writable_intel(u16 bdf, u32 pam0)
{
    // Read in current PAM settings from pci config space
    સંઘ pamdata_u pamdata;
    pamdata.data32[0] = pci_ioconfig_readl(bdf, ALIGN_DOWN(pam0, 4));
    pamdata.data32[1] = pci_ioconfig_readl(bdf, ALIGN_DOWN(pam0, 4) + 4);
    u8 *pam = &pamdata.data8[pam0 & 0x03];

    // Make ram from 0xc0000-0xf0000 writable
    પૂર્ણાંક i;
    માટે (i=0; i<6; i++)
        pam[i + 1] = 0x33;

    // Make ram from 0xf0000-0x100000 writable
    પૂર્ણાંક ram_present = pam[0] & 0x10;
    pam[0] = 0x30;

    // Write PAM settings back to pci config space
    pci_ioconfig_writel(bdf, ALIGN_DOWN(pam0, 4), pamdata.data32[0]);
    pci_ioconfig_writel(bdf, ALIGN_DOWN(pam0, 4) + 4, pamdata.data32[1]);

    જો (!ram_present)
        // Copy bios.
        memcpy(VSYMBOL(code32flat_start)
               , VSYMBOL(code32flat_start) + BIOS_SRC_OFFSET
               , પ્રતીક(code32flat_end) - પ્રતીક(code32flat_start));
}

સ્થિર શૂન્ય
make_bios_writable_intel(u16 bdf, u32 pam0)
{
    પૂર્ણાંક reg = pci_ioconfig_readb(bdf, pam0);
    જો (!(reg & 0x10)) {
        // QEMU doesn't fully implement the piix shadow capabilities -
        // if ram isn't backing the bios segment when shadowing is
        // disabled, the code itself won't be in memory.  So, run the
        // code from the high-memory flash location.
        u32 pos = (u32)__make_bios_writable_intel + BIOS_SRC_OFFSET;
        શૂન્ય (*func)(u16 bdf, u32 pam0) = (શૂન્ય*)pos;
        func(bdf, pam0);
        પાછા;
    }
    // Ram already present - just enable writes
    __make_bios_writable_intel(bdf, pam0);
}

સ્થિર શૂન્ય
make_bios_readonly_intel(u16 bdf, u32 pam0)
{
    // Flush any pending writes before locking memory.
    wbinvd();

    // Read in current PAM settings from pci config space
    સંઘ pamdata_u pamdata;
    pamdata.data32[0] = pci_config_readl(bdf, ALIGN_DOWN(pam0, 4));
    pamdata.data32[1] = pci_config_readl(bdf, ALIGN_DOWN(pam0, 4) + 4);
    u8 *pam = &pamdata.data8[pam0 & 0x03];

    // Write protect roms from 0xc0000-0xf0000
    u32 romlast = BUILD_BIOS_ADDR, rommax = BUILD_BIOS_ADDR;
    જો (CONFIG_WRITABLE_UPPERMEMORY)
        romlast = rom_get_last();
    જો (CONFIG_MALLOC_UPPERMEMORY)
        rommax = rom_get_max();
    પૂર્ણાંક i;
    માટે (i=0; i<6; i++) {
        u32 mem = BUILD_ROM_START + i * 32*1024;
        જો (romlast < mem + 16*1024 || rommax < mem + 32*1024) {
            જો (romlast >= mem && rommax >= mem + 16*1024)
                pam[i + 1] = 0x31;
            તોડો;
        }
        pam[i + 1] = 0x11;
    }

    // Write protect 0xf0000-0x100000
    pam[0] = 0x10;

    // Write PAM settings back to pci config space
    pci_config_writel(bdf, ALIGN_DOWN(pam0, 4), pamdata.data32[0]);
    pci_config_writel(bdf, ALIGN_DOWN(pam0, 4) + 4, pamdata.data32[1]);
}

સ્થિર પૂર્ણાંક ShadowBDF = -1;

// Make the 0xc0000-0x100000 area read/writable.
શૂન્ય
make_bios_writable(શૂન્ય)
{
    જો (!CONFIG_QEMU || runningOnXen())
        પાછા;

    dprintf(3, "enabling shadow ram\n");

    // At this point, statically allocated variables can't be written,
    // so do this search manually.
    પૂર્ણાંક bdf;
    pci_ioconfig_foreachbdf(bdf, 0) {
        u32 vendev = pci_ioconfig_readl(bdf, PCI_VENDOR_ID);
        u16 vendor = vendev & 0xffff, device = vendev >> 16;
        જો (vendor == PCI_VENDOR_ID_INTEL
            && device == PCI_DEVICE_ID_INTEL_82441) {
            make_bios_writable_intel(bdf, I440FX_PAM0);
            code_mutable_preinit();
            ShadowBDF = bdf;
            પાછા;
        }
        જો (vendor == PCI_VENDOR_ID_INTEL
            && device == PCI_DEVICE_ID_INTEL_Q35_MCH) {
            make_bios_writable_intel(bdf, Q35_HOST_BRIDGE_PAM0);
            code_mutable_preinit();
            ShadowBDF = bdf;
            પાછા;
        }
    }
    dprintf(1, "Unable to unlock ram - bridge not found\n");
}

// Make the BIOS code segment area (0xf0000) read-only.
શૂન્ય
make_bios_readonly(શૂન્ય)
{
    જો (!CONFIG_QEMU || runningOnXen())
        પાછા;
    dprintf(3, "locking shadow ram\n");

    જો (ShadowBDF < 0) {
        dprintf(1, "Unable to lock ram - bridge not found\n");
        પાછા;
    }

    u16 device = pci_config_readw(ShadowBDF, PCI_DEVICE_ID);
    જો (device == PCI_DEVICE_ID_INTEL_82441)
        make_bios_readonly_intel(ShadowBDF, I440FX_PAM0);
    નહીંતર
        make_bios_readonly_intel(ShadowBDF, Q35_HOST_BRIDGE_PAM0);
}

શૂન્ય
qemu_reboot(શૂન્ય)
{
    જો (!CONFIG_QEMU || runningOnXen())
        પાછા;
    // QEMU doesn't map 0xc0000-0xfffff back to the original rom on a
    // reset, so do that manually before invoking a hard reset.
    શૂન્ય *flash = (શૂન્ય*)BIOS_SRC_OFFSET;
    u32 hrp = (u32)&HaveRunPost;
    જો (readl(flash + hrp)) {
        // There isn't a pristine copy of the BIOS at 0xffff0000 to copy
        જો (HaveRunPost == 3) {
            // In a reboot loop.  Try to shutdown the machine instead.
            dprintf(1, "Unable to hard-reboot machine - attempting shutdown.\n");
            apm_shutdown();
        }
        make_bios_writable();
        HaveRunPost = 3;
    } નહીંતર {
        // Copy the BIOS making sure to only reset HaveRunPost at end
        make_bios_writable();
        u32 cstart = પ્રતીક(code32flat_start), cend = પ્રતીક(code32flat_end);
        memcpy((શૂન્ય*)cstart, flash + cstart, hrp - cstart);
        memcpy((શૂન્ય*)hrp + 4, flash + hrp + 4, cend - (hrp + 4));
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
    asm અસ્થિર("int3");
}

// Support for enabling/disabling BIOS ram shadowing.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "config.h" // CONFIG_*
#ضمن "dev-q35.h" // PCI_VENDOR_ID_INTEL
#ضمن "dev-piix.h" // I440FX_PAM0
#ضمن "hw/pci.h" // pci_config_writeb
#ضمن "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#ضمن "hw/pci_regs.h" // PCI_VENDOR_ID
#ضمن "malloc.h" // rom_get_last
#ضمن "output.h" // dprintf
#ضمن "paravirt.h" // runningOnXen
#ضمن "string.h" // memset
#ضمن "util.h" // make_bios_writable
#ضمن "x86.h" // wbinvd

// On the emulators, the bios at 0xf0000 is also at 0xffff0000
#define BIOS_SRC_OFFSET 0xfff00000

اتحاد pamdata_u {
    u8 data8[8];
    u32 data32[2];
};

// Enable shadowing and copy bios.
ساكن فراغ
__make_bios_writable_intel(u16 bdf, u32 pam0)
{
    // Read in current PAM settings from pci config space
    اتحاد pamdata_u pamdata;
    pamdata.data32[0] = pci_ioconfig_readl(bdf, ALIGN_DOWN(pam0, 4));
    pamdata.data32[1] = pci_ioconfig_readl(bdf, ALIGN_DOWN(pam0, 4) + 4);
    u8 *pam = &pamdata.data8[pam0 & 0x03];

    // Make ram from 0xc0000-0xf0000 writable
    صحيح i;
    لكل (i=0; i<6; i++)
        pam[i + 1] = 0x33;

    // Make ram from 0xf0000-0x100000 writable
    صحيح ram_present = pam[0] & 0x10;
    pam[0] = 0x30;

    // Write PAM settings back to pci config space
    pci_ioconfig_writel(bdf, ALIGN_DOWN(pam0, 4), pamdata.data32[0]);
    pci_ioconfig_writel(bdf, ALIGN_DOWN(pam0, 4) + 4, pamdata.data32[1]);

    إذا (!ram_present)
        // Copy bios.
        memcpy(VSYMBOL(code32flat_start)
               , VSYMBOL(code32flat_start) + BIOS_SRC_OFFSET
               , بەلگە(code32flat_end) - بەلگە(code32flat_start));
}

ساكن فراغ
make_bios_writable_intel(u16 bdf, u32 pam0)
{
    صحيح reg = pci_ioconfig_readb(bdf, pam0);
    إذا (!(reg & 0x10)) {
        // QEMU doesn't fully implement the piix shadow capabilities -
        // if ram isn't backing the bios segment when shadowing is
        // disabled, the code itself won't be in memory.  So, run the
        // code from the high-memory flash location.
        u32 pos = (u32)__make_bios_writable_intel + BIOS_SRC_OFFSET;
        فراغ (*func)(u16 bdf, u32 pam0) = (فراغ*)pos;
        func(bdf, pam0);
        أرجع;
    }
    // Ram already present - just enable writes
    __make_bios_writable_intel(bdf, pam0);
}

ساكن فراغ
make_bios_readonly_intel(u16 bdf, u32 pam0)
{
    // Flush any pending writes before locking memory.
    wbinvd();

    // Read in current PAM settings from pci config space
    اتحاد pamdata_u pamdata;
    pamdata.data32[0] = pci_config_readl(bdf, ALIGN_DOWN(pam0, 4));
    pamdata.data32[1] = pci_config_readl(bdf, ALIGN_DOWN(pam0, 4) + 4);
    u8 *pam = &pamdata.data8[pam0 & 0x03];

    // Write protect roms from 0xc0000-0xf0000
    u32 romlast = BUILD_BIOS_ADDR, rommax = BUILD_BIOS_ADDR;
    إذا (CONFIG_WRITABLE_UPPERMEMORY)
        romlast = rom_get_last();
    إذا (CONFIG_MALLOC_UPPERMEMORY)
        rommax = rom_get_max();
    صحيح i;
    لكل (i=0; i<6; i++) {
        u32 mem = BUILD_ROM_START + i * 32*1024;
        إذا (romlast < mem + 16*1024 || rommax < mem + 32*1024) {
            إذا (romlast >= mem && rommax >= mem + 16*1024)
                pam[i + 1] = 0x31;
            اكسر;
        }
        pam[i + 1] = 0x11;
    }

    // Write protect 0xf0000-0x100000
    pam[0] = 0x10;

    // Write PAM settings back to pci config space
    pci_config_writel(bdf, ALIGN_DOWN(pam0, 4), pamdata.data32[0]);
    pci_config_writel(bdf, ALIGN_DOWN(pam0, 4) + 4, pamdata.data32[1]);
}

ساكن صحيح ShadowBDF = -1;

// Make the 0xc0000-0x100000 area read/writable.
فراغ
make_bios_writable(فراغ)
{
    إذا (!CONFIG_QEMU || runningOnXen())
        أرجع;

    dprintf(3, "enabling shadow ram\n");

    // At this point, statically allocated variables can't be written,
    // so do this search manually.
    صحيح bdf;
    pci_ioconfig_foreachbdf(bdf, 0) {
        u32 vendev = pci_ioconfig_readl(bdf, PCI_VENDOR_ID);
        u16 vendor = vendev & 0xffff, device = vendev >> 16;
        إذا (vendor == PCI_VENDOR_ID_INTEL
            && device == PCI_DEVICE_ID_INTEL_82441) {
            make_bios_writable_intel(bdf, I440FX_PAM0);
            code_mutable_preinit();
            ShadowBDF = bdf;
            أرجع;
        }
        إذا (vendor == PCI_VENDOR_ID_INTEL
            && device == PCI_DEVICE_ID_INTEL_Q35_MCH) {
            make_bios_writable_intel(bdf, Q35_HOST_BRIDGE_PAM0);
            code_mutable_preinit();
            ShadowBDF = bdf;
            أرجع;
        }
    }
    dprintf(1, "Unable to unlock ram - bridge not found\n");
}

// Make the BIOS code segment area (0xf0000) read-only.
فراغ
make_bios_readonly(فراغ)
{
    إذا (!CONFIG_QEMU || runningOnXen())
        أرجع;
    dprintf(3, "locking shadow ram\n");

    إذا (ShadowBDF < 0) {
        dprintf(1, "Unable to lock ram - bridge not found\n");
        أرجع;
    }

    u16 device = pci_config_readw(ShadowBDF, PCI_DEVICE_ID);
    إذا (device == PCI_DEVICE_ID_INTEL_82441)
        make_bios_readonly_intel(ShadowBDF, I440FX_PAM0);
    وإلا
        make_bios_readonly_intel(ShadowBDF, Q35_HOST_BRIDGE_PAM0);
}

فراغ
qemu_reboot(فراغ)
{
    إذا (!CONFIG_QEMU || runningOnXen())
        أرجع;
    // QEMU doesn't map 0xc0000-0xfffff back to the original rom on a
    // reset, so do that manually before invoking a hard reset.
    فراغ *flash = (فراغ*)BIOS_SRC_OFFSET;
    u32 hrp = (u32)&HaveRunPost;
    إذا (readl(flash + hrp)) {
        // There isn't a pristine copy of the BIOS at 0xffff0000 to copy
        إذا (HaveRunPost == 3) {
            // In a reboot loop.  Try to shutdown the machine instead.
            dprintf(1, "Unable to hard-reboot machine - attempting shutdown.\n");
            apm_shutdown();
        }
        make_bios_writable();
        HaveRunPost = 3;
    } وإلا {
        // Copy the BIOS making sure to only reset HaveRunPost at end
        make_bios_writable();
        u32 cstart = بەلگە(code32flat_start), cend = بەلگە(code32flat_end);
        memcpy((فراغ*)cstart, flash + cstart, hrp - cstart);
        memcpy((فراغ*)hrp + 4, flash + hrp + 4, cend - (hrp + 4));
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
    asm متطاير("int3");
}

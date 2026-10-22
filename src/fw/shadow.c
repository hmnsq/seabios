// Support for enabling/disabling BIOS ram shadowing.
//
// Copyright (C) 2008-2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2006 Fabrice Bellard
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "dev-q35.h" // PCI_VENDOR_ID_INTEL
#അന്തര്ഭാവ "dev-piix.h" // I440FX_PAM0
#അന്തര്ഭാവ "hw/pci.h" // pci_config_writeb
#അന്തര്ഭാവ "hw/pci_ids.h" // PCI_VENDOR_ID_INTEL
#അന്തര്ഭാവ "hw/pci_regs.h" // PCI_VENDOR_ID
#അന്തര്ഭാവ "malloc.h" // rom_get_last
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "paravirt.h" // runningOnXen
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // make_bios_writable
#അന്തര്ഭാവ "x86.h" // wbinvd

// On the emulators, the bios at 0xf0000 is also at 0xffff0000
#define BIOS_SRC_OFFSET 0xfff00000

സംഘം pamdata_u {
    u8 data8[8];
    u32 data32[2];
};

// Enable shadowing and copy bios.
സ്ഥിതം ശൂന്യം
__make_bios_writable_intel(u16 bdf, u32 pam0)
{
    // Read in current PAM settings from pci config space
    സംഘം pamdata_u pamdata;
    pamdata.data32[0] = pci_ioconfig_readl(bdf, ALIGN_DOWN(pam0, 4));
    pamdata.data32[1] = pci_ioconfig_readl(bdf, ALIGN_DOWN(pam0, 4) + 4);
    u8 *pam = &pamdata.data8[pam0 & 0x03];

    // Make ram from 0xc0000-0xf0000 writable
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<6; i++)
        pam[i + 1] = 0x33;

    // Make ram from 0xf0000-0x100000 writable
    പൂർണ്ണസംഖ്യ ram_present = pam[0] & 0x10;
    pam[0] = 0x30;

    // Write PAM settings back to pci config space
    pci_ioconfig_writel(bdf, ALIGN_DOWN(pam0, 4), pamdata.data32[0]);
    pci_ioconfig_writel(bdf, ALIGN_DOWN(pam0, 4) + 4, pamdata.data32[1]);

    എങ്കിൽ (!ram_present)
        // Copy bios.
        memcpy(VSYMBOL(code32flat_start)
               , VSYMBOL(code32flat_start) + BIOS_SRC_OFFSET
               , ചിഹ്നം(code32flat_end) - ചിഹ്നം(code32flat_start));
}

സ്ഥിതം ശൂന്യം
make_bios_writable_intel(u16 bdf, u32 pam0)
{
    പൂർണ്ണസംഖ്യ reg = pci_ioconfig_readb(bdf, pam0);
    എങ്കിൽ (!(reg & 0x10)) {
        // QEMU doesn't fully implement the piix shadow capabilities -
        // if ram isn't backing the bios segment when shadowing is
        // disabled, the code itself won't be in memory.  So, run the
        // code from the high-memory flash location.
        u32 pos = (u32)__make_bios_writable_intel + BIOS_SRC_OFFSET;
        ശൂന്യം (*func)(u16 bdf, u32 pam0) = (ശൂന്യം*)pos;
        func(bdf, pam0);
        തിരികെ;
    }
    // Ram already present - just enable writes
    __make_bios_writable_intel(bdf, pam0);
}

സ്ഥിതം ശൂന്യം
make_bios_readonly_intel(u16 bdf, u32 pam0)
{
    // Flush any pending writes before locking memory.
    wbinvd();

    // Read in current PAM settings from pci config space
    സംഘം pamdata_u pamdata;
    pamdata.data32[0] = pci_config_readl(bdf, ALIGN_DOWN(pam0, 4));
    pamdata.data32[1] = pci_config_readl(bdf, ALIGN_DOWN(pam0, 4) + 4);
    u8 *pam = &pamdata.data8[pam0 & 0x03];

    // Write protect roms from 0xc0000-0xf0000
    u32 romlast = BUILD_BIOS_ADDR, rommax = BUILD_BIOS_ADDR;
    എങ്കിൽ (CONFIG_WRITABLE_UPPERMEMORY)
        romlast = rom_get_last();
    എങ്കിൽ (CONFIG_MALLOC_UPPERMEMORY)
        rommax = rom_get_max();
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<6; i++) {
        u32 mem = BUILD_ROM_START + i * 32*1024;
        എങ്കിൽ (romlast < mem + 16*1024 || rommax < mem + 32*1024) {
            എങ്കിൽ (romlast >= mem && rommax >= mem + 16*1024)
                pam[i + 1] = 0x31;
            നിർത്തുക;
        }
        pam[i + 1] = 0x11;
    }

    // Write protect 0xf0000-0x100000
    pam[0] = 0x10;

    // Write PAM settings back to pci config space
    pci_config_writel(bdf, ALIGN_DOWN(pam0, 4), pamdata.data32[0]);
    pci_config_writel(bdf, ALIGN_DOWN(pam0, 4) + 4, pamdata.data32[1]);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ ShadowBDF = -1;

// Make the 0xc0000-0x100000 area read/writable.
ശൂന്യം
make_bios_writable(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_QEMU || runningOnXen())
        തിരികെ;

    dprintf(3, "enabling shadow ram\n");

    // At this point, statically allocated variables can't be written,
    // so do this search manually.
    പൂർണ്ണസംഖ്യ bdf;
    pci_ioconfig_foreachbdf(bdf, 0) {
        u32 vendev = pci_ioconfig_readl(bdf, PCI_VENDOR_ID);
        u16 vendor = vendev & 0xffff, device = vendev >> 16;
        എങ്കിൽ (vendor == PCI_VENDOR_ID_INTEL
            && device == PCI_DEVICE_ID_INTEL_82441) {
            make_bios_writable_intel(bdf, I440FX_PAM0);
            code_mutable_preinit();
            ShadowBDF = bdf;
            തിരികെ;
        }
        എങ്കിൽ (vendor == PCI_VENDOR_ID_INTEL
            && device == PCI_DEVICE_ID_INTEL_Q35_MCH) {
            make_bios_writable_intel(bdf, Q35_HOST_BRIDGE_PAM0);
            code_mutable_preinit();
            ShadowBDF = bdf;
            തിരികെ;
        }
    }
    dprintf(1, "Unable to unlock ram - bridge not found\n");
}

// Make the BIOS code segment area (0xf0000) read-only.
ശൂന്യം
make_bios_readonly(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_QEMU || runningOnXen())
        തിരികെ;
    dprintf(3, "locking shadow ram\n");

    എങ്കിൽ (ShadowBDF < 0) {
        dprintf(1, "Unable to lock ram - bridge not found\n");
        തിരികെ;
    }

    u16 device = pci_config_readw(ShadowBDF, PCI_DEVICE_ID);
    എങ്കിൽ (device == PCI_DEVICE_ID_INTEL_82441)
        make_bios_readonly_intel(ShadowBDF, I440FX_PAM0);
    അല്ലെങ്കിൽ
        make_bios_readonly_intel(ShadowBDF, Q35_HOST_BRIDGE_PAM0);
}

ശൂന്യം
qemu_reboot(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_QEMU || runningOnXen())
        തിരികെ;
    // QEMU doesn't map 0xc0000-0xfffff back to the original rom on a
    // reset, so do that manually before invoking a hard reset.
    ശൂന്യം *flash = (ശൂന്യം*)BIOS_SRC_OFFSET;
    u32 hrp = (u32)&HaveRunPost;
    എങ്കിൽ (readl(flash + hrp)) {
        // There isn't a pristine copy of the BIOS at 0xffff0000 to copy
        എങ്കിൽ (HaveRunPost == 3) {
            // In a reboot loop.  Try to shutdown the machine instead.
            dprintf(1, "Unable to hard-reboot machine - attempting shutdown.\n");
            apm_shutdown();
        }
        make_bios_writable();
        HaveRunPost = 3;
    } അല്ലെങ്കിൽ {
        // Copy the BIOS making sure to only reset HaveRunPost at end
        make_bios_writable();
        u32 cstart = ചിഹ്നം(code32flat_start), cend = ചിഹ്നം(code32flat_end);
        memcpy((ശൂന്യം*)cstart, flash + cstart, hrp - cstart);
        memcpy((ശൂന്യം*)hrp + 4, flash + hrp + 4, cend - (hrp + 4));
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
    asm അസ്ഥിര("int3");
}

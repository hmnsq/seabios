// PCI config space access functions.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "pci.h" // pci_config_writel
#अन्तर्भाव "pci_regs.h" // PCI_VENDOR_ID
#अन्तर्भाव "util.h" // udelay
#अन्तर्भाव "x86.h" // outl

#परिभाषय पत्तनम्_PCI_CMD           0x0cf8
#परिभाषय पत्तनम्_PCI_दत्त          0x0cfc

स्थिर u32 mmconfig;

स्थिर शून्य *mmconfig_addr(u16 bdf, u32 addr)
{
    प्रत्यागम (शून्य*)(mmconfig + ((u32)bdf << 12) + addr);
}

स्थिर u32 ioconfig_cmd(u16 bdf, u32 addr)
{
    प्रत्यागम 0x80000000 | (bdf << 8) | (addr & 0xfc);
}

शून्य pci_ioconfig_writel(u16 bdf, u32 addr, u32 val)
{
    outl(ioconfig_cmd(bdf, addr), पत्तनम्_PCI_CMD);
    outl(val, पत्तनम्_PCI_दत्त);
}

शून्य pci_config_writel(u16 bdf, u32 addr, u32 val)
{
    यदि (!MODESEGMENT && mmconfig) {
        writel(mmconfig_addr(bdf, addr), val);
    } अन्यथा {
        pci_ioconfig_writel(bdf, addr, val);
    }
}

शून्य pci_ioconfig_writew(u16 bdf, u32 addr, u16 val)
{
    outl(ioconfig_cmd(bdf, addr), पत्तनम्_PCI_CMD);
    outw(val, पत्तनम्_PCI_दत्त + (addr & 2));
}

शून्य pci_config_writew(u16 bdf, u32 addr, u16 val)
{
    यदि (!MODESEGMENT && mmconfig) {
        writew(mmconfig_addr(bdf, addr), val);
    } अन्यथा {
        pci_ioconfig_writew(bdf, addr, val);
    }
}

शून्य pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val)
{
    outl(ioconfig_cmd(bdf, addr), पत्तनम्_PCI_CMD);
    outb(val, पत्तनम्_PCI_दत्त + (addr & 3));
}

शून्य pci_config_writeb(u16 bdf, u32 addr, u8 val)
{
    यदि (!MODESEGMENT && mmconfig) {
        writeb(mmconfig_addr(bdf, addr), val);
    } अन्यथा {
        pci_ioconfig_writeb(bdf, addr, val);
    }
}

u32 pci_ioconfig_readl(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), पत्तनम्_PCI_CMD);
    प्रत्यागम inl(पत्तनम्_PCI_दत्त);
}

u32 pci_config_readl(u16 bdf, u32 addr)
{
    यदि (!MODESEGMENT && mmconfig) {
        प्रत्यागम readl(mmconfig_addr(bdf, addr));
    } अन्यथा {
        प्रत्यागम pci_ioconfig_readl(bdf, addr);
    }
}

u16 pci_ioconfig_readw(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), पत्तनम्_PCI_CMD);
    प्रत्यागम inw(पत्तनम्_PCI_दत्त + (addr & 2));
}

u16 pci_config_readw(u16 bdf, u32 addr)
{
    यदि (!MODESEGMENT && mmconfig) {
        प्रत्यागम readw(mmconfig_addr(bdf, addr));
    } अन्यथा {
        प्रत्यागम pci_ioconfig_readw(bdf, addr);
    }
}

u8 pci_ioconfig_readb(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), पत्तनम्_PCI_CMD);
    प्रत्यागम inb(पत्तनम्_PCI_दत्त + (addr & 3));
}

u8 pci_config_readb(u16 bdf, u32 addr)
{
    यदि (!MODESEGMENT && mmconfig) {
        प्रत्यागम readb(mmconfig_addr(bdf, addr));
    } अन्यथा {
        प्रत्यागम pci_ioconfig_readb(bdf, addr);
    }
}

शून्य
pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on)
{
    u16 val = pci_config_readw(bdf, addr);
    val = (val & ~off) | on;
    pci_config_writew(bdf, addr, val);
}

शून्य
pci_enable_mmconfig(u64 addr, ध्रुव वर्ण *name)
{
    यदि (addr >= 0x100000000ll)
        प्रत्यागम;
    dprintf(1, "PCIe: using %s mmconfig at 0x%llx\n", name, addr);
    mmconfig = addr;
}

u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap)
{
    पूर्णाङ्क i;
    u16 स्थितिः = pci_config_readw(bdf, PCI_स्थितिः);

    यदि (!(स्थितिः & PCI_स्थितिः_CAP_LIST))
        प्रत्यागम 0;

    यदि (cap == 0) {
        /* find first */
        cap = pci_config_readb(bdf, PCI_CAPABILITY_LIST);
    } अन्यथा {
        /* find next */
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_अग्र);
    }
    प्रति (i = 0; cap && i <= 0xff; i++) {
        यदि (pci_config_readb(bdf, cap + PCI_CAP_LIST_ID) == cap_id)
            प्रत्यागम cap;
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_अग्र);
    }

    प्रत्यागम 0;
}

// Helper function for pci_ioconfig_foreachbdf() macro - return next device
पूर्णाङ्क pci_ioconfig_अग्र(पूर्णाङ्क bdf, पूर्णाङ्क bus)
{
    यदि (pci_bdf_गन्तव्यम्_fn(bdf) == 0
        && (pci_ioconfig_readb(bdf, PCI_शीर्षकम्_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    अन्यथा
        bdf += 1;

    प्रति (;;) {
        यदि (pci_bdf_गन्तव्यम्_bus(bdf) != bus)
            प्रत्यागम -1;

        u16 v = pci_ioconfig_readw(bdf, PCI_VENDOR_ID);
        यदि (v != 0x0000 && v != 0xffff)
            // Device is present.
            प्रत्यागम bdf;

        यदि (pci_bdf_गन्तव्यम्_fn(bdf) == 0)
            bdf += 8;
        अन्यथा
            bdf += 1;
    }
}

// Helper function for foreachbdf() macro - return next device
पूर्णाङ्क
pci_अग्र(पूर्णाङ्क bdf, पूर्णाङ्क bus)
{
    यदि (pci_bdf_गन्तव्यम्_fn(bdf) == 0
        && (pci_config_readb(bdf, PCI_शीर्षकम्_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    अन्यथा
        bdf += 1;

    प्रति (;;) {
        यदि (pci_bdf_गन्तव्यम्_bus(bdf) != bus)
            प्रत्यागम -1;

        u16 v = pci_config_readw(bdf, PCI_VENDOR_ID);
        यदि (v != 0x0000 && v != 0xffff)
            // Device is present.
            प्रत्यागम bdf;

        यदि (pci_bdf_गन्तव्यम्_fn(bdf) == 0)
            bdf += 8;
        अन्यथा
            bdf += 1;
    }
}

// Check if PCI is available at all
पूर्णाङ्क
pci_probe_host(शून्य)
{
    outl(0x80000000, पत्तनम्_PCI_CMD);
    यदि (inl(पत्तनम्_PCI_CMD) != 0x80000000) {
        dprintf(1, "Detected non-PCI system\n");
        प्रत्यागम -1;
    }
    प्रत्यागम 0;
}

शून्य
pci_reboot(शून्य)
{
    u8 v = inb(पत्तनम्_PCI_REBOOT) & ~6;
    outb(v|2, पत्तनम्_PCI_REBOOT); /* Request hard reset */
    udelay(50);
    outb(v|6, पत्तनम्_PCI_REBOOT); /* Actually do the reset */
    udelay(50);
}

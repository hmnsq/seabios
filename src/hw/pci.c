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

#define PORT_PCI_CMD           0x0cf8
#define PORT_PCI_DATA          0x0cfc

स्थिर u32 mmconfig;

स्थिर शून्य *mmconfig_addr(u16 bdf, u32 addr)
{
    परत (शून्य*)(mmconfig + ((u32)bdf << 12) + addr);
}

स्थिर u32 ioconfig_cmd(u16 bdf, u32 addr)
{
    परत 0x80000000 | (bdf << 8) | (addr & 0xfc);
}

शून्य pci_ioconfig_writel(u16 bdf, u32 addr, u32 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outl(val, PORT_PCI_DATA);
}

शून्य pci_config_writel(u16 bdf, u32 addr, u32 val)
{
    जर (!MODESEGMENT && mmconfig) {
        writel(mmconfig_addr(bdf, addr), val);
    } अन्यथा {
        pci_ioconfig_writel(bdf, addr, val);
    }
}

शून्य pci_ioconfig_writew(u16 bdf, u32 addr, u16 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outw(val, PORT_PCI_DATA + (addr & 2));
}

शून्य pci_config_writew(u16 bdf, u32 addr, u16 val)
{
    जर (!MODESEGMENT && mmconfig) {
        writew(mmconfig_addr(bdf, addr), val);
    } अन्यथा {
        pci_ioconfig_writew(bdf, addr, val);
    }
}

शून्य pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outb(val, PORT_PCI_DATA + (addr & 3));
}

शून्य pci_config_writeb(u16 bdf, u32 addr, u8 val)
{
    जर (!MODESEGMENT && mmconfig) {
        writeb(mmconfig_addr(bdf, addr), val);
    } अन्यथा {
        pci_ioconfig_writeb(bdf, addr, val);
    }
}

u32 pci_ioconfig_readl(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    परत inl(PORT_PCI_DATA);
}

u32 pci_config_readl(u16 bdf, u32 addr)
{
    जर (!MODESEGMENT && mmconfig) {
        परत readl(mmconfig_addr(bdf, addr));
    } अन्यथा {
        परत pci_ioconfig_readl(bdf, addr);
    }
}

u16 pci_ioconfig_readw(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    परत inw(PORT_PCI_DATA + (addr & 2));
}

u16 pci_config_readw(u16 bdf, u32 addr)
{
    जर (!MODESEGMENT && mmconfig) {
        परत readw(mmconfig_addr(bdf, addr));
    } अन्यथा {
        परत pci_ioconfig_readw(bdf, addr);
    }
}

u8 pci_ioconfig_readb(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    परत inb(PORT_PCI_DATA + (addr & 3));
}

u8 pci_config_readb(u16 bdf, u32 addr)
{
    जर (!MODESEGMENT && mmconfig) {
        परत readb(mmconfig_addr(bdf, addr));
    } अन्यथा {
        परत pci_ioconfig_readb(bdf, addr);
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
pci_enable_mmconfig(u64 addr, स्थिरांक अक्षर *name)
{
    जर (addr >= 0x100000000ll)
        परत;
    dprintf(1, "PCIe: using %s mmconfig at 0x%llx\n", name, addr);
    mmconfig = addr;
}

u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap)
{
    पूर्णांक i;
    u16 status = pci_config_readw(bdf, PCI_STATUS);

    जर (!(status & PCI_STATUS_CAP_LIST))
        परत 0;

    जर (cap == 0) {
        /* find first */
        cap = pci_config_readb(bdf, PCI_CAPABILITY_LIST);
    } अन्यथा {
        /* find next */
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_NEXT);
    }
    साठी (i = 0; cap && i <= 0xff; i++) {
        जर (pci_config_readb(bdf, cap + PCI_CAP_LIST_ID) == cap_id)
            परत cap;
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_NEXT);
    }

    परत 0;
}

// Helper function for pci_ioconfig_foreachbdf() macro - return next device
पूर्णांक pci_ioconfig_next(पूर्णांक bdf, पूर्णांक bus)
{
    जर (pci_bdf_to_fn(bdf) == 0
        && (pci_ioconfig_readb(bdf, PCI_HEADER_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    अन्यथा
        bdf += 1;

    साठी (;;) {
        जर (pci_bdf_to_bus(bdf) != bus)
            परत -1;

        u16 v = pci_ioconfig_readw(bdf, PCI_VENDOR_ID);
        जर (v != 0x0000 && v != 0xffff)
            // Device is present.
            परत bdf;

        जर (pci_bdf_to_fn(bdf) == 0)
            bdf += 8;
        अन्यथा
            bdf += 1;
    }
}

// Helper function for foreachbdf() macro - return next device
पूर्णांक
pci_next(पूर्णांक bdf, पूर्णांक bus)
{
    जर (pci_bdf_to_fn(bdf) == 0
        && (pci_config_readb(bdf, PCI_HEADER_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    अन्यथा
        bdf += 1;

    साठी (;;) {
        जर (pci_bdf_to_bus(bdf) != bus)
            परत -1;

        u16 v = pci_config_readw(bdf, PCI_VENDOR_ID);
        जर (v != 0x0000 && v != 0xffff)
            // Device is present.
            परत bdf;

        जर (pci_bdf_to_fn(bdf) == 0)
            bdf += 8;
        अन्यथा
            bdf += 1;
    }
}

// Check if PCI is available at all
पूर्णांक
pci_probe_host(शून्य)
{
    outl(0x80000000, PORT_PCI_CMD);
    जर (inl(PORT_PCI_CMD) != 0x80000000) {
        dprintf(1, "Detected non-PCI system\n");
        परत -1;
    }
    परत 0;
}

शून्य
pci_reboot(शून्य)
{
    u8 v = inb(PORT_PCI_REBOOT) & ~6;
    outb(v|2, PORT_PCI_REBOOT); /* Request hard reset */
    udelay(50);
    outb(v|6, PORT_PCI_REBOOT); /* Actually do the reset */
    udelay(50);
}

// PCI config space access functions.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "pci.h" // pci_config_writel
#অন্তর্ভাব "pci_regs.h" // PCI_VENDOR_ID
#অন্তর্ভাব "util.h" // udelay
#অন্তর্ভাব "x86.h" // outl

#define PORT_PCI_CMD           0x0cf8
#define PORT_PCI_DATA          0x0cfc

স্থির u32 mmconfig;

স্থির শূন্য *mmconfig_addr(u16 bdf, u32 addr)
{
    প্রত্যাগম (শূন্য*)(mmconfig + ((u32)bdf << 12) + addr);
}

স্থির u32 ioconfig_cmd(u16 bdf, u32 addr)
{
    প্রত্যাগম 0x80000000 | (bdf << 8) | (addr & 0xfc);
}

শূন্য pci_ioconfig_writel(u16 bdf, u32 addr, u32 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outl(val, PORT_PCI_DATA);
}

শূন্য pci_config_writel(u16 bdf, u32 addr, u32 val)
{
    যদি (!MODESEGMENT && mmconfig) {
        writel(mmconfig_addr(bdf, addr), val);
    } অন্যথা {
        pci_ioconfig_writel(bdf, addr, val);
    }
}

শূন্য pci_ioconfig_writew(u16 bdf, u32 addr, u16 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outw(val, PORT_PCI_DATA + (addr & 2));
}

শূন্য pci_config_writew(u16 bdf, u32 addr, u16 val)
{
    যদি (!MODESEGMENT && mmconfig) {
        writew(mmconfig_addr(bdf, addr), val);
    } অন্যথা {
        pci_ioconfig_writew(bdf, addr, val);
    }
}

শূন্য pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outb(val, PORT_PCI_DATA + (addr & 3));
}

শূন্য pci_config_writeb(u16 bdf, u32 addr, u8 val)
{
    যদি (!MODESEGMENT && mmconfig) {
        writeb(mmconfig_addr(bdf, addr), val);
    } অন্যথা {
        pci_ioconfig_writeb(bdf, addr, val);
    }
}

u32 pci_ioconfig_readl(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    প্রত্যাগম inl(PORT_PCI_DATA);
}

u32 pci_config_readl(u16 bdf, u32 addr)
{
    যদি (!MODESEGMENT && mmconfig) {
        প্রত্যাগম readl(mmconfig_addr(bdf, addr));
    } অন্যথা {
        প্রত্যাগম pci_ioconfig_readl(bdf, addr);
    }
}

u16 pci_ioconfig_readw(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    প্রত্যাগম inw(PORT_PCI_DATA + (addr & 2));
}

u16 pci_config_readw(u16 bdf, u32 addr)
{
    যদি (!MODESEGMENT && mmconfig) {
        প্রত্যাগম readw(mmconfig_addr(bdf, addr));
    } অন্যথা {
        প্রত্যাগম pci_ioconfig_readw(bdf, addr);
    }
}

u8 pci_ioconfig_readb(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    প্রত্যাগম inb(PORT_PCI_DATA + (addr & 3));
}

u8 pci_config_readb(u16 bdf, u32 addr)
{
    যদি (!MODESEGMENT && mmconfig) {
        প্রত্যাগম readb(mmconfig_addr(bdf, addr));
    } অন্যথা {
        প্রত্যাগম pci_ioconfig_readb(bdf, addr);
    }
}

শূন্য
pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on)
{
    u16 val = pci_config_readw(bdf, addr);
    val = (val & ~off) | on;
    pci_config_writew(bdf, addr, val);
}

শূন্য
pci_enable_mmconfig(u64 addr, ধ্রুব বর্ণ *name)
{
    যদি (addr >= 0x100000000ll)
        প্রত্যাগম;
    dprintf(1, "PCIe: using %s mmconfig at 0x%llx\n", name, addr);
    mmconfig = addr;
}

u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap)
{
    পূর্ণাঙ্ক i;
    u16 status = pci_config_readw(bdf, PCI_STATUS);

    যদি (!(status & PCI_STATUS_CAP_LIST))
        প্রত্যাগম 0;

    যদি (cap == 0) {
        /* find first */
        cap = pci_config_readb(bdf, PCI_CAPABILITY_LIST);
    } অন্যথা {
        /* find next */
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_NEXT);
    }
    প্রতি (i = 0; cap && i <= 0xff; i++) {
        যদি (pci_config_readb(bdf, cap + PCI_CAP_LIST_ID) == cap_id)
            প্রত্যাগম cap;
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_NEXT);
    }

    প্রত্যাগম 0;
}

// Helper function for pci_ioconfig_foreachbdf() macro - return next device
পূর্ণাঙ্ক pci_ioconfig_next(পূর্ণাঙ্ক bdf, পূর্ণাঙ্ক bus)
{
    যদি (pci_bdf_to_fn(bdf) == 0
        && (pci_ioconfig_readb(bdf, PCI_HEADER_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    অন্যথা
        bdf += 1;

    প্রতি (;;) {
        যদি (pci_bdf_to_bus(bdf) != bus)
            প্রত্যাগম -1;

        u16 v = pci_ioconfig_readw(bdf, PCI_VENDOR_ID);
        যদি (v != 0x0000 && v != 0xffff)
            // Device is present.
            প্রত্যাগম bdf;

        যদি (pci_bdf_to_fn(bdf) == 0)
            bdf += 8;
        অন্যথা
            bdf += 1;
    }
}

// Helper function for foreachbdf() macro - return next device
পূর্ণাঙ্ক
pci_next(পূর্ণাঙ্ক bdf, পূর্ণাঙ্ক bus)
{
    যদি (pci_bdf_to_fn(bdf) == 0
        && (pci_config_readb(bdf, PCI_HEADER_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    অন্যথা
        bdf += 1;

    প্রতি (;;) {
        যদি (pci_bdf_to_bus(bdf) != bus)
            প্রত্যাগম -1;

        u16 v = pci_config_readw(bdf, PCI_VENDOR_ID);
        যদি (v != 0x0000 && v != 0xffff)
            // Device is present.
            প্রত্যাগম bdf;

        যদি (pci_bdf_to_fn(bdf) == 0)
            bdf += 8;
        অন্যথা
            bdf += 1;
    }
}

// Check if PCI is available at all
পূর্ণাঙ্ক
pci_probe_host(শূন্য)
{
    outl(0x80000000, PORT_PCI_CMD);
    যদি (inl(PORT_PCI_CMD) != 0x80000000) {
        dprintf(1, "Detected non-PCI system\n");
        প্রত্যাগম -1;
    }
    প্রত্যাগম 0;
}

শূন্য
pci_reboot(শূন্য)
{
    u8 v = inb(PORT_PCI_REBOOT) & ~6;
    outb(v|2, PORT_PCI_REBOOT); /* Request hard reset */
    udelay(50);
    outb(v|6, PORT_PCI_REBOOT); /* Actually do the reset */
    udelay(50);
}

// PCI config space access functions.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "pci.h" // pci_config_writel
#ଅନ୍ତର୍ଭାଵ "pci_regs.h" // PCI_VENDOR_ID
#ଅନ୍ତର୍ଭାଵ "util.h" // udelay
#ଅନ୍ତର୍ଭାଵ "x86.h" // outl

#define PORT_PCI_CMD           0x0cf8
#define PORT_PCI_DATA          0x0cfc

ସ୍ଥିର u32 mmconfig;

ସ୍ଥିର ଶୂନ୍ୟ *mmconfig_addr(u16 bdf, u32 addr)
{
    ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)(mmconfig + ((u32)bdf << 12) + addr);
}

ସ୍ଥିର u32 ioconfig_cmd(u16 bdf, u32 addr)
{
    ଫେରନ୍ତୁ 0x80000000 | (bdf << 8) | (addr & 0xfc);
}

ଶୂନ୍ୟ pci_ioconfig_writel(u16 bdf, u32 addr, u32 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outl(val, PORT_PCI_DATA);
}

ଶୂନ୍ୟ pci_config_writel(u16 bdf, u32 addr, u32 val)
{
    ଯଦି (!MODESEGMENT && mmconfig) {
        writel(mmconfig_addr(bdf, addr), val);
    } ଅନ୍ୟଥା {
        pci_ioconfig_writel(bdf, addr, val);
    }
}

ଶୂନ୍ୟ pci_ioconfig_writew(u16 bdf, u32 addr, u16 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outw(val, PORT_PCI_DATA + (addr & 2));
}

ଶୂନ୍ୟ pci_config_writew(u16 bdf, u32 addr, u16 val)
{
    ଯଦି (!MODESEGMENT && mmconfig) {
        writew(mmconfig_addr(bdf, addr), val);
    } ଅନ୍ୟଥା {
        pci_ioconfig_writew(bdf, addr, val);
    }
}

ଶୂନ୍ୟ pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outb(val, PORT_PCI_DATA + (addr & 3));
}

ଶୂନ୍ୟ pci_config_writeb(u16 bdf, u32 addr, u8 val)
{
    ଯଦି (!MODESEGMENT && mmconfig) {
        writeb(mmconfig_addr(bdf, addr), val);
    } ଅନ୍ୟଥା {
        pci_ioconfig_writeb(bdf, addr, val);
    }
}

u32 pci_ioconfig_readl(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    ଫେରନ୍ତୁ inl(PORT_PCI_DATA);
}

u32 pci_config_readl(u16 bdf, u32 addr)
{
    ଯଦି (!MODESEGMENT && mmconfig) {
        ଫେରନ୍ତୁ readl(mmconfig_addr(bdf, addr));
    } ଅନ୍ୟଥା {
        ଫେରନ୍ତୁ pci_ioconfig_readl(bdf, addr);
    }
}

u16 pci_ioconfig_readw(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    ଫେରନ୍ତୁ inw(PORT_PCI_DATA + (addr & 2));
}

u16 pci_config_readw(u16 bdf, u32 addr)
{
    ଯଦି (!MODESEGMENT && mmconfig) {
        ଫେରନ୍ତୁ readw(mmconfig_addr(bdf, addr));
    } ଅନ୍ୟଥା {
        ଫେରନ୍ତୁ pci_ioconfig_readw(bdf, addr);
    }
}

u8 pci_ioconfig_readb(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    ଫେରନ୍ତୁ inb(PORT_PCI_DATA + (addr & 3));
}

u8 pci_config_readb(u16 bdf, u32 addr)
{
    ଯଦି (!MODESEGMENT && mmconfig) {
        ଫେରନ୍ତୁ readb(mmconfig_addr(bdf, addr));
    } ଅନ୍ୟଥା {
        ଫେରନ୍ତୁ pci_ioconfig_readb(bdf, addr);
    }
}

ଶୂନ୍ୟ
pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on)
{
    u16 val = pci_config_readw(bdf, addr);
    val = (val & ~off) | on;
    pci_config_writew(bdf, addr, val);
}

ଶୂନ୍ୟ
pci_enable_mmconfig(u64 addr, ଅଚଳ ଅକ୍ଷର *name)
{
    ଯଦି (addr >= 0x100000000ll)
        ଫେରନ୍ତୁ;
    dprintf(1, "PCIe: using %s mmconfig at 0x%llx\n", name, addr);
    mmconfig = addr;
}

u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    u16 status = pci_config_readw(bdf, PCI_STATUS);

    ଯଦି (!(status & PCI_STATUS_CAP_LIST))
        ଫେରନ୍ତୁ 0;

    ଯଦି (cap == 0) {
        /* find first */
        cap = pci_config_readb(bdf, PCI_CAPABILITY_LIST);
    } ଅନ୍ୟଥା {
        /* find next */
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_NEXT);
    }
    ପାଇଁ (i = 0; cap && i <= 0xff; i++) {
        ଯଦି (pci_config_readb(bdf, cap + PCI_CAP_LIST_ID) == cap_id)
            ଫେରନ୍ତୁ cap;
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_NEXT);
    }

    ଫେରନ୍ତୁ 0;
}

// Helper function for pci_ioconfig_foreachbdf() macro - return next device
ପୂର୍ଣ୍ଣସଂଖ୍ୟା pci_ioconfig_next(ପୂର୍ଣ୍ଣସଂଖ୍ୟା bdf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bus)
{
    ଯଦି (pci_bdf_to_fn(bdf) == 0
        && (pci_ioconfig_readb(bdf, PCI_HEADER_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    ଅନ୍ୟଥା
        bdf += 1;

    ପାଇଁ (;;) {
        ଯଦି (pci_bdf_to_bus(bdf) != bus)
            ଫେରନ୍ତୁ -1;

        u16 v = pci_ioconfig_readw(bdf, PCI_VENDOR_ID);
        ଯଦି (v != 0x0000 && v != 0xffff)
            // Device is present.
            ଫେରନ୍ତୁ bdf;

        ଯଦି (pci_bdf_to_fn(bdf) == 0)
            bdf += 8;
        ଅନ୍ୟଥା
            bdf += 1;
    }
}

// Helper function for foreachbdf() macro - return next device
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
pci_next(ପୂର୍ଣ୍ଣସଂଖ୍ୟା bdf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bus)
{
    ଯଦି (pci_bdf_to_fn(bdf) == 0
        && (pci_config_readb(bdf, PCI_HEADER_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    ଅନ୍ୟଥା
        bdf += 1;

    ପାଇଁ (;;) {
        ଯଦି (pci_bdf_to_bus(bdf) != bus)
            ଫେରନ୍ତୁ -1;

        u16 v = pci_config_readw(bdf, PCI_VENDOR_ID);
        ଯଦି (v != 0x0000 && v != 0xffff)
            // Device is present.
            ଫେରନ୍ତୁ bdf;

        ଯଦି (pci_bdf_to_fn(bdf) == 0)
            bdf += 8;
        ଅନ୍ୟଥା
            bdf += 1;
    }
}

// Check if PCI is available at all
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
pci_probe_host(ଶୂନ୍ୟ)
{
    outl(0x80000000, PORT_PCI_CMD);
    ଯଦି (inl(PORT_PCI_CMD) != 0x80000000) {
        dprintf(1, "Detected non-PCI system\n");
        ଫେରନ୍ତୁ -1;
    }
    ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ
pci_reboot(ଶୂନ୍ୟ)
{
    u8 v = inb(PORT_PCI_REBOOT) & ~6;
    outb(v|2, PORT_PCI_REBOOT); /* Request hard reset */
    udelay(50);
    outb(v|6, PORT_PCI_REBOOT); /* Actually do the reset */
    udelay(50);
}

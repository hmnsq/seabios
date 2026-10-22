// PCI config space access functions.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "pci.h" // pci_config_writel
#அந்தர்பாவ "pci_regs.h" // PCI_VENDOR_ID
#அந்தர்பாவ "util.h" // udelay
#அந்தர்பாவ "x86.h" // outl

#define PORT_PCI_CMD           0x0cf8
#define PORT_PCI_DATA          0x0cfc

நிலையான u32 mmconfig;

நிலையான வெறுமை *mmconfig_addr(u16 bdf, u32 addr)
{
    திரும்பு (வெறுமை*)(mmconfig + ((u32)bdf << 12) + addr);
}

நிலையான u32 ioconfig_cmd(u16 bdf, u32 addr)
{
    திரும்பு 0x80000000 | (bdf << 8) | (addr & 0xfc);
}

வெறுமை pci_ioconfig_writel(u16 bdf, u32 addr, u32 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outl(val, PORT_PCI_DATA);
}

வெறுமை pci_config_writel(u16 bdf, u32 addr, u32 val)
{
    எனில் (!MODESEGMENT && mmconfig) {
        writel(mmconfig_addr(bdf, addr), val);
    } இல்லையெனில் {
        pci_ioconfig_writel(bdf, addr, val);
    }
}

வெறுமை pci_ioconfig_writew(u16 bdf, u32 addr, u16 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outw(val, PORT_PCI_DATA + (addr & 2));
}

வெறுமை pci_config_writew(u16 bdf, u32 addr, u16 val)
{
    எனில் (!MODESEGMENT && mmconfig) {
        writew(mmconfig_addr(bdf, addr), val);
    } இல்லையெனில் {
        pci_ioconfig_writew(bdf, addr, val);
    }
}

வெறுமை pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outb(val, PORT_PCI_DATA + (addr & 3));
}

வெறுமை pci_config_writeb(u16 bdf, u32 addr, u8 val)
{
    எனில் (!MODESEGMENT && mmconfig) {
        writeb(mmconfig_addr(bdf, addr), val);
    } இல்லையெனில் {
        pci_ioconfig_writeb(bdf, addr, val);
    }
}

u32 pci_ioconfig_readl(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    திரும்பு inl(PORT_PCI_DATA);
}

u32 pci_config_readl(u16 bdf, u32 addr)
{
    எனில் (!MODESEGMENT && mmconfig) {
        திரும்பு readl(mmconfig_addr(bdf, addr));
    } இல்லையெனில் {
        திரும்பு pci_ioconfig_readl(bdf, addr);
    }
}

u16 pci_ioconfig_readw(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    திரும்பு inw(PORT_PCI_DATA + (addr & 2));
}

u16 pci_config_readw(u16 bdf, u32 addr)
{
    எனில் (!MODESEGMENT && mmconfig) {
        திரும்பு readw(mmconfig_addr(bdf, addr));
    } இல்லையெனில் {
        திரும்பு pci_ioconfig_readw(bdf, addr);
    }
}

u8 pci_ioconfig_readb(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    திரும்பு inb(PORT_PCI_DATA + (addr & 3));
}

u8 pci_config_readb(u16 bdf, u32 addr)
{
    எனில் (!MODESEGMENT && mmconfig) {
        திரும்பு readb(mmconfig_addr(bdf, addr));
    } இல்லையெனில் {
        திரும்பு pci_ioconfig_readb(bdf, addr);
    }
}

வெறுமை
pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on)
{
    u16 val = pci_config_readw(bdf, addr);
    val = (val & ~off) | on;
    pci_config_writew(bdf, addr, val);
}

வெறுமை
pci_enable_mmconfig(u64 addr, மாறிலி எழுத்து *name)
{
    எனில் (addr >= 0x100000000ll)
        திரும்பு;
    dprintf(1, "PCIe: using %s mmconfig at 0x%llx\n", name, addr);
    mmconfig = addr;
}

u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap)
{
    முழு i;
    u16 status = pci_config_readw(bdf, PCI_STATUS);

    எனில் (!(status & PCI_STATUS_CAP_LIST))
        திரும்பு 0;

    எனில் (cap == 0) {
        /* find first */
        cap = pci_config_readb(bdf, PCI_CAPABILITY_LIST);
    } இல்லையெனில் {
        /* find next */
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_NEXT);
    }
    க்கு (i = 0; cap && i <= 0xff; i++) {
        எனில் (pci_config_readb(bdf, cap + PCI_CAP_LIST_ID) == cap_id)
            திரும்பு cap;
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_NEXT);
    }

    திரும்பு 0;
}

// Helper function for pci_ioconfig_foreachbdf() macro - return next device
முழு pci_ioconfig_next(முழு bdf, முழு bus)
{
    எனில் (pci_bdf_to_fn(bdf) == 0
        && (pci_ioconfig_readb(bdf, PCI_HEADER_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    இல்லையெனில்
        bdf += 1;

    க்கு (;;) {
        எனில் (pci_bdf_to_bus(bdf) != bus)
            திரும்பு -1;

        u16 v = pci_ioconfig_readw(bdf, PCI_VENDOR_ID);
        எனில் (v != 0x0000 && v != 0xffff)
            // Device is present.
            திரும்பு bdf;

        எனில் (pci_bdf_to_fn(bdf) == 0)
            bdf += 8;
        இல்லையெனில்
            bdf += 1;
    }
}

// Helper function for foreachbdf() macro - return next device
முழு
pci_next(முழு bdf, முழு bus)
{
    எனில் (pci_bdf_to_fn(bdf) == 0
        && (pci_config_readb(bdf, PCI_HEADER_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    இல்லையெனில்
        bdf += 1;

    க்கு (;;) {
        எனில் (pci_bdf_to_bus(bdf) != bus)
            திரும்பு -1;

        u16 v = pci_config_readw(bdf, PCI_VENDOR_ID);
        எனில் (v != 0x0000 && v != 0xffff)
            // Device is present.
            திரும்பு bdf;

        எனில் (pci_bdf_to_fn(bdf) == 0)
            bdf += 8;
        இல்லையெனில்
            bdf += 1;
    }
}

// Check if PCI is available at all
முழு
pci_probe_host(வெறுமை)
{
    outl(0x80000000, PORT_PCI_CMD);
    எனில் (inl(PORT_PCI_CMD) != 0x80000000) {
        dprintf(1, "Detected non-PCI system\n");
        திரும்பு -1;
    }
    திரும்பு 0;
}

வெறுமை
pci_reboot(வெறுமை)
{
    u8 v = inb(PORT_PCI_REBOOT) & ~6;
    outb(v|2, PORT_PCI_REBOOT); /* Request hard reset */
    udelay(50);
    outb(v|6, PORT_PCI_REBOOT); /* Actually do the reset */
    udelay(50);
}

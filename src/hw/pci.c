// PCI config space access functions.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pci.h" // pci_config_writel
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_VENDOR_ID
#ಅನ್ತರ್ಭಾವ "util.h" // udelay
#ಅನ್ತರ್ಭಾವ "x86.h" // outl

#define PORT_PCI_CMD           0x0cf8
#define PORT_PCI_DATA          0x0cfc

ಸ್ಥಿರ u32 mmconfig;

ಸ್ಥಿರ ಶೂನ್ಯ *mmconfig_addr(u16 bdf, u32 addr)
{
    ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ*)(mmconfig + ((u32)bdf << 12) + addr);
}

ಸ್ಥಿರ u32 ioconfig_cmd(u16 bdf, u32 addr)
{
    ಪ್ರತ್ಯಾಗಮ 0x80000000 | (bdf << 8) | (addr & 0xfc);
}

ಶೂನ್ಯ pci_ioconfig_writel(u16 bdf, u32 addr, u32 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outl(val, PORT_PCI_DATA);
}

ಶೂನ್ಯ pci_config_writel(u16 bdf, u32 addr, u32 val)
{
    ಯದಿ (!MODESEGMENT && mmconfig) {
        writel(mmconfig_addr(bdf, addr), val);
    } ಅನ್ಯಥಾ {
        pci_ioconfig_writel(bdf, addr, val);
    }
}

ಶೂನ್ಯ pci_ioconfig_writew(u16 bdf, u32 addr, u16 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outw(val, PORT_PCI_DATA + (addr & 2));
}

ಶೂನ್ಯ pci_config_writew(u16 bdf, u32 addr, u16 val)
{
    ಯದಿ (!MODESEGMENT && mmconfig) {
        writew(mmconfig_addr(bdf, addr), val);
    } ಅನ್ಯಥಾ {
        pci_ioconfig_writew(bdf, addr, val);
    }
}

ಶೂನ್ಯ pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outb(val, PORT_PCI_DATA + (addr & 3));
}

ಶೂನ್ಯ pci_config_writeb(u16 bdf, u32 addr, u8 val)
{
    ಯದಿ (!MODESEGMENT && mmconfig) {
        writeb(mmconfig_addr(bdf, addr), val);
    } ಅನ್ಯಥಾ {
        pci_ioconfig_writeb(bdf, addr, val);
    }
}

u32 pci_ioconfig_readl(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    ಪ್ರತ್ಯಾಗಮ inl(PORT_PCI_DATA);
}

u32 pci_config_readl(u16 bdf, u32 addr)
{
    ಯದಿ (!MODESEGMENT && mmconfig) {
        ಪ್ರತ್ಯಾಗಮ readl(mmconfig_addr(bdf, addr));
    } ಅನ್ಯಥಾ {
        ಪ್ರತ್ಯಾಗಮ pci_ioconfig_readl(bdf, addr);
    }
}

u16 pci_ioconfig_readw(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    ಪ್ರತ್ಯಾಗಮ inw(PORT_PCI_DATA + (addr & 2));
}

u16 pci_config_readw(u16 bdf, u32 addr)
{
    ಯದಿ (!MODESEGMENT && mmconfig) {
        ಪ್ರತ್ಯಾಗಮ readw(mmconfig_addr(bdf, addr));
    } ಅನ್ಯಥಾ {
        ಪ್ರತ್ಯಾಗಮ pci_ioconfig_readw(bdf, addr);
    }
}

u8 pci_ioconfig_readb(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    ಪ್ರತ್ಯಾಗಮ inb(PORT_PCI_DATA + (addr & 3));
}

u8 pci_config_readb(u16 bdf, u32 addr)
{
    ಯದಿ (!MODESEGMENT && mmconfig) {
        ಪ್ರತ್ಯಾಗಮ readb(mmconfig_addr(bdf, addr));
    } ಅನ್ಯಥಾ {
        ಪ್ರತ್ಯಾಗಮ pci_ioconfig_readb(bdf, addr);
    }
}

ಶೂನ್ಯ
pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on)
{
    u16 val = pci_config_readw(bdf, addr);
    val = (val & ~off) | on;
    pci_config_writew(bdf, addr, val);
}

ಶೂನ್ಯ
pci_enable_mmconfig(u64 addr, ಧ್ರುವ ವರ್ಣ *name)
{
    ಯದಿ (addr >= 0x100000000ll)
        ಪ್ರತ್ಯಾಗಮ;
    dprintf(1, "PCIe: using %s mmconfig at 0x%llx\n", name, addr);
    mmconfig = addr;
}

u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap)
{
    ಪೂರ್ಣಾಙ್ಕ i;
    u16 status = pci_config_readw(bdf, PCI_STATUS);

    ಯದಿ (!(status & PCI_STATUS_CAP_LIST))
        ಪ್ರತ್ಯಾಗಮ 0;

    ಯದಿ (cap == 0) {
        /* find first */
        cap = pci_config_readb(bdf, PCI_CAPABILITY_LIST);
    } ಅನ್ಯಥಾ {
        /* find next */
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_NEXT);
    }
    ಪ್ರತಿ (i = 0; cap && i <= 0xff; i++) {
        ಯದಿ (pci_config_readb(bdf, cap + PCI_CAP_LIST_ID) == cap_id)
            ಪ್ರತ್ಯಾಗಮ cap;
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_NEXT);
    }

    ಪ್ರತ್ಯಾಗಮ 0;
}

// Helper function for pci_ioconfig_foreachbdf() macro - return next device
ಪೂರ್ಣಾಙ್ಕ pci_ioconfig_next(ಪೂರ್ಣಾಙ್ಕ bdf, ಪೂರ್ಣಾಙ್ಕ bus)
{
    ಯದಿ (pci_bdf_to_fn(bdf) == 0
        && (pci_ioconfig_readb(bdf, PCI_HEADER_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    ಅನ್ಯಥಾ
        bdf += 1;

    ಪ್ರತಿ (;;) {
        ಯದಿ (pci_bdf_to_bus(bdf) != bus)
            ಪ್ರತ್ಯಾಗಮ -1;

        u16 v = pci_ioconfig_readw(bdf, PCI_VENDOR_ID);
        ಯದಿ (v != 0x0000 && v != 0xffff)
            // Device is present.
            ಪ್ರತ್ಯಾಗಮ bdf;

        ಯದಿ (pci_bdf_to_fn(bdf) == 0)
            bdf += 8;
        ಅನ್ಯಥಾ
            bdf += 1;
    }
}

// Helper function for foreachbdf() macro - return next device
ಪೂರ್ಣಾಙ್ಕ
pci_next(ಪೂರ್ಣಾಙ್ಕ bdf, ಪೂರ್ಣಾಙ್ಕ bus)
{
    ಯದಿ (pci_bdf_to_fn(bdf) == 0
        && (pci_config_readb(bdf, PCI_HEADER_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    ಅನ್ಯಥಾ
        bdf += 1;

    ಪ್ರತಿ (;;) {
        ಯದಿ (pci_bdf_to_bus(bdf) != bus)
            ಪ್ರತ್ಯಾಗಮ -1;

        u16 v = pci_config_readw(bdf, PCI_VENDOR_ID);
        ಯದಿ (v != 0x0000 && v != 0xffff)
            // Device is present.
            ಪ್ರತ್ಯಾಗಮ bdf;

        ಯದಿ (pci_bdf_to_fn(bdf) == 0)
            bdf += 8;
        ಅನ್ಯಥಾ
            bdf += 1;
    }
}

// Check if PCI is available at all
ಪೂರ್ಣಾಙ್ಕ
pci_probe_host(ಶೂನ್ಯ)
{
    outl(0x80000000, PORT_PCI_CMD);
    ಯದಿ (inl(PORT_PCI_CMD) != 0x80000000) {
        dprintf(1, "Detected non-PCI system\n");
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ
pci_reboot(ಶೂನ್ಯ)
{
    u8 v = inb(PORT_PCI_REBOOT) & ~6;
    outb(v|2, PORT_PCI_REBOOT); /* Request hard reset */
    udelay(50);
    outb(v|6, PORT_PCI_REBOOT); /* Actually do the reset */
    udelay(50);
}

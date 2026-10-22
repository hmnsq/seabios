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

#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_PCI_CMD           0x0cf8
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_PCI_ದತ್ತಾಂಶ          0x0cfc

ಸ್ಥಿರ u32 mmconfig;

ಸ್ಥಿರ ಶೂನ್ಯ *mmconfig_addr(u16 bdf, u32 addr)
{
    ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)(mmconfig + ((u32)bdf << 12) + addr);
}

ಸ್ಥಿರ u32 ioconfig_cmd(u16 bdf, u32 addr)
{
    ಹಿಂತಿರುಗಿ 0x80000000 | (bdf << 8) | (addr & 0xfc);
}

ಶೂನ್ಯ pci_ioconfig_writel(u16 bdf, u32 addr, u32 val)
{
    outl(ioconfig_cmd(bdf, addr), ಪೋರ್ಟ್_PCI_CMD);
    outl(val, ಪೋರ್ಟ್_PCI_ದತ್ತಾಂಶ);
}

ಶೂನ್ಯ pci_config_writel(u16 bdf, u32 addr, u32 val)
{
    ಆದರೆ (!MODESEGMENT && mmconfig) {
        writel(mmconfig_addr(bdf, addr), val);
    } ಇಲ್ಲದಿದ್ದರೆ {
        pci_ioconfig_writel(bdf, addr, val);
    }
}

ಶೂನ್ಯ pci_ioconfig_writew(u16 bdf, u32 addr, u16 val)
{
    outl(ioconfig_cmd(bdf, addr), ಪೋರ್ಟ್_PCI_CMD);
    outw(val, ಪೋರ್ಟ್_PCI_ದತ್ತಾಂಶ + (addr & 2));
}

ಶೂನ್ಯ pci_config_writew(u16 bdf, u32 addr, u16 val)
{
    ಆದರೆ (!MODESEGMENT && mmconfig) {
        writew(mmconfig_addr(bdf, addr), val);
    } ಇಲ್ಲದಿದ್ದರೆ {
        pci_ioconfig_writew(bdf, addr, val);
    }
}

ಶೂನ್ಯ pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val)
{
    outl(ioconfig_cmd(bdf, addr), ಪೋರ್ಟ್_PCI_CMD);
    outb(val, ಪೋರ್ಟ್_PCI_ದತ್ತಾಂಶ + (addr & 3));
}

ಶೂನ್ಯ pci_config_writeb(u16 bdf, u32 addr, u8 val)
{
    ಆದರೆ (!MODESEGMENT && mmconfig) {
        writeb(mmconfig_addr(bdf, addr), val);
    } ಇಲ್ಲದಿದ್ದರೆ {
        pci_ioconfig_writeb(bdf, addr, val);
    }
}

u32 pci_ioconfig_readl(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), ಪೋರ್ಟ್_PCI_CMD);
    ಹಿಂತಿರುಗಿ inl(ಪೋರ್ಟ್_PCI_ದತ್ತಾಂಶ);
}

u32 pci_config_readl(u16 bdf, u32 addr)
{
    ಆದರೆ (!MODESEGMENT && mmconfig) {
        ಹಿಂತಿರುಗಿ readl(mmconfig_addr(bdf, addr));
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಹಿಂತಿರುಗಿ pci_ioconfig_readl(bdf, addr);
    }
}

u16 pci_ioconfig_readw(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), ಪೋರ್ಟ್_PCI_CMD);
    ಹಿಂತಿರುಗಿ inw(ಪೋರ್ಟ್_PCI_ದತ್ತಾಂಶ + (addr & 2));
}

u16 pci_config_readw(u16 bdf, u32 addr)
{
    ಆದರೆ (!MODESEGMENT && mmconfig) {
        ಹಿಂತಿರುಗಿ readw(mmconfig_addr(bdf, addr));
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಹಿಂತಿರುಗಿ pci_ioconfig_readw(bdf, addr);
    }
}

u8 pci_ioconfig_readb(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), ಪೋರ್ಟ್_PCI_CMD);
    ಹಿಂತಿರುಗಿ inb(ಪೋರ್ಟ್_PCI_ದತ್ತಾಂಶ + (addr & 3));
}

u8 pci_config_readb(u16 bdf, u32 addr)
{
    ಆದರೆ (!MODESEGMENT && mmconfig) {
        ಹಿಂತಿರುಗಿ readb(mmconfig_addr(bdf, addr));
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಹಿಂತಿರುಗಿ pci_ioconfig_readb(bdf, addr);
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
pci_enable_mmconfig(u64 addr, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
    ಆದರೆ (addr >= 0x100000000ll)
        ಹಿಂತಿರುಗಿ;
    dprintf(1, "PCIe: using %s mmconfig at 0x%llx\n", name, addr);
    mmconfig = addr;
}

u8 pci_ಕಂಡುಹಿಡಿ_capability(u16 bdf, u8 cap_id, u8 cap)
{
    ಪೂರ್ಣಾಂಕ i;
    u16 ಸ್ಥಿತಿ_2 = pci_config_readw(bdf, PCI_ಸ್ಥಿತಿ);

    ಆದರೆ (!(ಸ್ಥಿತಿ_2 & PCI_ಸ್ಥಿತಿ_CAP_LIST))
        ಹಿಂತಿರುಗಿ 0;

    ಆದರೆ (cap == 0) {
        /* find first */
        cap = pci_config_readb(bdf, PCI_CAPABILITY_LIST);
    } ಇಲ್ಲದಿದ್ದರೆ {
        /* find next */
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_ಮುಂದಿನ);
    }
    ಗಾಗಿ (i = 0; cap && i <= 0xff; i++) {
        ಆದರೆ (pci_config_readb(bdf, cap + PCI_CAP_LIST_ID) == cap_id)
            ಹಿಂತಿರುಗಿ cap;
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_ಮುಂದಿನ);
    }

    ಹಿಂತಿರುಗಿ 0;
}

// Helper function for pci_ioconfig_foreachbdf() macro - return next device
ಪೂರ್ಣಾಂಕ pci_ioconfig_ಮುಂದಿನ(ಪೂರ್ಣಾಂಕ bdf, ಪೂರ್ಣಾಂಕ bus)
{
    ಆದರೆ (pci_bdf_ಗಮ್ಯ_fn(bdf) == 0
        && (pci_ioconfig_readb(bdf, PCI_ಶೀರ್ಷಿಕೆ_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    ಇಲ್ಲದಿದ್ದರೆ
        bdf += 1;

    ಗಾಗಿ (;;) {
        ಆದರೆ (pci_bdf_ಗಮ್ಯ_bus(bdf) != bus)
            ಹಿಂತಿರುಗಿ -1;

        u16 v = pci_ioconfig_readw(bdf, PCI_VENDOR_ID);
        ಆದರೆ (v != 0x0000 && v != 0xffff)
            // Device is present.
            ಹಿಂತಿರುಗಿ bdf;

        ಆದರೆ (pci_bdf_ಗಮ್ಯ_fn(bdf) == 0)
            bdf += 8;
        ಇಲ್ಲದಿದ್ದರೆ
            bdf += 1;
    }
}

// Helper function for foreachbdf() macro - return next device
ಪೂರ್ಣಾಂಕ
pci_ಮುಂದಿನ(ಪೂರ್ಣಾಂಕ bdf, ಪೂರ್ಣಾಂಕ bus)
{
    ಆದರೆ (pci_bdf_ಗಮ್ಯ_fn(bdf) == 0
        && (pci_config_readb(bdf, PCI_ಶೀರ್ಷಿಕೆ_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    ಇಲ್ಲದಿದ್ದರೆ
        bdf += 1;

    ಗಾಗಿ (;;) {
        ಆದರೆ (pci_bdf_ಗಮ್ಯ_bus(bdf) != bus)
            ಹಿಂತಿರುಗಿ -1;

        u16 v = pci_config_readw(bdf, PCI_VENDOR_ID);
        ಆದರೆ (v != 0x0000 && v != 0xffff)
            // Device is present.
            ಹಿಂತಿರುಗಿ bdf;

        ಆದರೆ (pci_bdf_ಗಮ್ಯ_fn(bdf) == 0)
            bdf += 8;
        ಇಲ್ಲದಿದ್ದರೆ
            bdf += 1;
    }
}

// Check if PCI is available at all
ಪೂರ್ಣಾಂಕ
pci_probe_host(ಶೂನ್ಯ)
{
    outl(0x80000000, ಪೋರ್ಟ್_PCI_CMD);
    ಆದರೆ (inl(ಪೋರ್ಟ್_PCI_CMD) != 0x80000000) {
        dprintf(1, "Detected non-PCI system\n");
        ಹಿಂತಿರುಗಿ -1;
    }
    ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ
pci_reboot(ಶೂನ್ಯ)
{
    u8 v = inb(ಪೋರ್ಟ್_PCI_REBOOT) & ~6;
    outb(v|2, ಪೋರ್ಟ್_PCI_REBOOT); /* Request hard reset */
    udelay(50);
    outb(v|6, ಪೋರ್ಟ್_PCI_REBOOT); /* Actually do the reset */
    udelay(50);
}

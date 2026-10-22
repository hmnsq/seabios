// PCI config space access functions.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pci.h" // pci_config_writel
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_regs.h" // PCI_VENDOR_ID
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // udelay
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // outl

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_PCI_CMD           0x0cf8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_PCI_ਅੰਕੜੇ          0x0cfc

ਸਥਿਰ u32 mmconfig;

ਸਥਿਰ ਖਾਲੀ *mmconfig_addr(u16 bdf, u32 addr)
{
    ਵਾਪਸ (ਖਾਲੀ*)(mmconfig + ((u32)bdf << 12) + addr);
}

ਸਥਿਰ u32 ioconfig_cmd(u16 bdf, u32 addr)
{
    ਵਾਪਸ 0x80000000 | (bdf << 8) | (addr & 0xfc);
}

ਖਾਲੀ pci_ioconfig_writel(u16 bdf, u32 addr, u32 val)
{
    outl(ioconfig_cmd(bdf, addr), ਪੋਰਟ_PCI_CMD);
    outl(val, ਪੋਰਟ_PCI_ਅੰਕੜੇ);
}

ਖਾਲੀ pci_config_writel(u16 bdf, u32 addr, u32 val)
{
    ਜੇ (!MODESEGMENT && mmconfig) {
        writel(mmconfig_addr(bdf, addr), val);
    } ਨਹੀਂਤਾਂ {
        pci_ioconfig_writel(bdf, addr, val);
    }
}

ਖਾਲੀ pci_ioconfig_writew(u16 bdf, u32 addr, u16 val)
{
    outl(ioconfig_cmd(bdf, addr), ਪੋਰਟ_PCI_CMD);
    outw(val, ਪੋਰਟ_PCI_ਅੰਕੜੇ + (addr & 2));
}

ਖਾਲੀ pci_config_writew(u16 bdf, u32 addr, u16 val)
{
    ਜੇ (!MODESEGMENT && mmconfig) {
        writew(mmconfig_addr(bdf, addr), val);
    } ਨਹੀਂਤਾਂ {
        pci_ioconfig_writew(bdf, addr, val);
    }
}

ਖਾਲੀ pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val)
{
    outl(ioconfig_cmd(bdf, addr), ਪੋਰਟ_PCI_CMD);
    outb(val, ਪੋਰਟ_PCI_ਅੰਕੜੇ + (addr & 3));
}

ਖਾਲੀ pci_config_writeb(u16 bdf, u32 addr, u8 val)
{
    ਜੇ (!MODESEGMENT && mmconfig) {
        writeb(mmconfig_addr(bdf, addr), val);
    } ਨਹੀਂਤਾਂ {
        pci_ioconfig_writeb(bdf, addr, val);
    }
}

u32 pci_ioconfig_readl(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), ਪੋਰਟ_PCI_CMD);
    ਵਾਪਸ inl(ਪੋਰਟ_PCI_ਅੰਕੜੇ);
}

u32 pci_config_readl(u16 bdf, u32 addr)
{
    ਜੇ (!MODESEGMENT && mmconfig) {
        ਵਾਪਸ readl(mmconfig_addr(bdf, addr));
    } ਨਹੀਂਤਾਂ {
        ਵਾਪਸ pci_ioconfig_readl(bdf, addr);
    }
}

u16 pci_ioconfig_readw(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), ਪੋਰਟ_PCI_CMD);
    ਵਾਪਸ inw(ਪੋਰਟ_PCI_ਅੰਕੜੇ + (addr & 2));
}

u16 pci_config_readw(u16 bdf, u32 addr)
{
    ਜੇ (!MODESEGMENT && mmconfig) {
        ਵਾਪਸ readw(mmconfig_addr(bdf, addr));
    } ਨਹੀਂਤਾਂ {
        ਵਾਪਸ pci_ioconfig_readw(bdf, addr);
    }
}

u8 pci_ioconfig_readb(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), ਪੋਰਟ_PCI_CMD);
    ਵਾਪਸ inb(ਪੋਰਟ_PCI_ਅੰਕੜੇ + (addr & 3));
}

u8 pci_config_readb(u16 bdf, u32 addr)
{
    ਜੇ (!MODESEGMENT && mmconfig) {
        ਵਾਪਸ readb(mmconfig_addr(bdf, addr));
    } ਨਹੀਂਤਾਂ {
        ਵਾਪਸ pci_ioconfig_readb(bdf, addr);
    }
}

ਖਾਲੀ
pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on)
{
    u16 val = pci_config_readw(bdf, addr);
    val = (val & ~off) | on;
    pci_config_writew(bdf, addr, val);
}

ਖਾਲੀ
pci_enable_mmconfig(u64 addr, ਅਚਲ ਅੱਖਰ *name)
{
    ਜੇ (addr >= 0x100000000ll)
        ਵਾਪਸ;
    dprintf(1, "PCIe: using %s mmconfig at 0x%llx\n", name, addr);
    mmconfig = addr;
}

u8 pci_ਲੱਭੋ_capability(u16 bdf, u8 cap_id, u8 cap)
{
    ਪੂਰਨ i;
    u16 ਸਥਿਤੀ = pci_config_readw(bdf, PCI_ਸਥਿਤੀ);

    ਜੇ (!(ਸਥਿਤੀ & PCI_ਸਥਿਤੀ_CAP_LIST))
        ਵਾਪਸ 0;

    ਜੇ (cap == 0) {
        /* find first */
        cap = pci_config_readb(bdf, PCI_CAPABILITY_LIST);
    } ਨਹੀਂਤਾਂ {
        /* find next */
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_ਅਗਲਾ);
    }
    ਲਈ (i = 0; cap && i <= 0xff; i++) {
        ਜੇ (pci_config_readb(bdf, cap + PCI_CAP_LIST_ID) == cap_id)
            ਵਾਪਸ cap;
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_ਅਗਲਾ);
    }

    ਵਾਪਸ 0;
}

// Helper function for pci_ioconfig_foreachbdf() macro - return next device
ਪੂਰਨ pci_ioconfig_ਅਗਲਾ(ਪੂਰਨ bdf, ਪੂਰਨ bus)
{
    ਜੇ (pci_bdf_ਮੰਜ਼ਿਲ_fn(bdf) == 0
        && (pci_ioconfig_readb(bdf, PCI_ਸਿਰਲੇਖ_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    ਨਹੀਂਤਾਂ
        bdf += 1;

    ਲਈ (;;) {
        ਜੇ (pci_bdf_ਮੰਜ਼ਿਲ_bus(bdf) != bus)
            ਵਾਪਸ -1;

        u16 v = pci_ioconfig_readw(bdf, PCI_VENDOR_ID);
        ਜੇ (v != 0x0000 && v != 0xffff)
            // Device is present.
            ਵਾਪਸ bdf;

        ਜੇ (pci_bdf_ਮੰਜ਼ਿਲ_fn(bdf) == 0)
            bdf += 8;
        ਨਹੀਂਤਾਂ
            bdf += 1;
    }
}

// Helper function for foreachbdf() macro - return next device
ਪੂਰਨ
pci_ਅਗਲਾ(ਪੂਰਨ bdf, ਪੂਰਨ bus)
{
    ਜੇ (pci_bdf_ਮੰਜ਼ਿਲ_fn(bdf) == 0
        && (pci_config_readb(bdf, PCI_ਸਿਰਲੇਖ_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    ਨਹੀਂਤਾਂ
        bdf += 1;

    ਲਈ (;;) {
        ਜੇ (pci_bdf_ਮੰਜ਼ਿਲ_bus(bdf) != bus)
            ਵਾਪਸ -1;

        u16 v = pci_config_readw(bdf, PCI_VENDOR_ID);
        ਜੇ (v != 0x0000 && v != 0xffff)
            // Device is present.
            ਵਾਪਸ bdf;

        ਜੇ (pci_bdf_ਮੰਜ਼ਿਲ_fn(bdf) == 0)
            bdf += 8;
        ਨਹੀਂਤਾਂ
            bdf += 1;
    }
}

// Check if PCI is available at all
ਪੂਰਨ
pci_probe_host(ਖਾਲੀ)
{
    outl(0x80000000, ਪੋਰਟ_PCI_CMD);
    ਜੇ (inl(ਪੋਰਟ_PCI_CMD) != 0x80000000) {
        dprintf(1, "Detected non-PCI system\n");
        ਵਾਪਸ -1;
    }
    ਵਾਪਸ 0;
}

ਖਾਲੀ
pci_reboot(ਖਾਲੀ)
{
    u8 v = inb(ਪੋਰਟ_PCI_REBOOT) & ~6;
    outb(v|2, ਪੋਰਟ_PCI_REBOOT); /* Request hard reset */
    udelay(50);
    outb(v|6, ਪੋਰਟ_PCI_REBOOT); /* Actually do the reset */
    udelay(50);
}

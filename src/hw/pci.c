// PCI config space access functions.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pci.h" // pci_config_writel
#అన్తర్భావ "pci_regs.h" // PCI_VENDOR_ID
#అన్తర్భావ "util.h" // udelay
#అన్తర్భావ "x86.h" // outl

#నిర్వచించు పోర్టు_PCI_CMD           0x0cf8
#నిర్వచించు పోర్టు_PCI_దత్తాంశం          0x0cfc

స్థిర u32 mmconfig;

స్థిర శూన్య *mmconfig_addr(u16 bdf, u32 addr)
{
    తిరిగి (శూన్య*)(mmconfig + ((u32)bdf << 12) + addr);
}

స్థిర u32 ioconfig_cmd(u16 bdf, u32 addr)
{
    తిరిగి 0x80000000 | (bdf << 8) | (addr & 0xfc);
}

శూన్య pci_ioconfig_writel(u16 bdf, u32 addr, u32 val)
{
    outl(ioconfig_cmd(bdf, addr), పోర్టు_PCI_CMD);
    outl(val, పోర్టు_PCI_దత్తాంశం);
}

శూన్య pci_config_writel(u16 bdf, u32 addr, u32 val)
{
    ఒకవేళ (!MODESEGMENT && mmconfig) {
        writel(mmconfig_addr(bdf, addr), val);
    } లేకపోతే {
        pci_ioconfig_writel(bdf, addr, val);
    }
}

శూన్య pci_ioconfig_writew(u16 bdf, u32 addr, u16 val)
{
    outl(ioconfig_cmd(bdf, addr), పోర్టు_PCI_CMD);
    outw(val, పోర్టు_PCI_దత్తాంశం + (addr & 2));
}

శూన్య pci_config_writew(u16 bdf, u32 addr, u16 val)
{
    ఒకవేళ (!MODESEGMENT && mmconfig) {
        writew(mmconfig_addr(bdf, addr), val);
    } లేకపోతే {
        pci_ioconfig_writew(bdf, addr, val);
    }
}

శూన్య pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val)
{
    outl(ioconfig_cmd(bdf, addr), పోర్టు_PCI_CMD);
    outb(val, పోర్టు_PCI_దత్తాంశం + (addr & 3));
}

శూన్య pci_config_writeb(u16 bdf, u32 addr, u8 val)
{
    ఒకవేళ (!MODESEGMENT && mmconfig) {
        writeb(mmconfig_addr(bdf, addr), val);
    } లేకపోతే {
        pci_ioconfig_writeb(bdf, addr, val);
    }
}

u32 pci_ioconfig_readl(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), పోర్టు_PCI_CMD);
    తిరిగి inl(పోర్టు_PCI_దత్తాంశం);
}

u32 pci_config_readl(u16 bdf, u32 addr)
{
    ఒకవేళ (!MODESEGMENT && mmconfig) {
        తిరిగి readl(mmconfig_addr(bdf, addr));
    } లేకపోతే {
        తిరిగి pci_ioconfig_readl(bdf, addr);
    }
}

u16 pci_ioconfig_readw(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), పోర్టు_PCI_CMD);
    తిరిగి inw(పోర్టు_PCI_దత్తాంశం + (addr & 2));
}

u16 pci_config_readw(u16 bdf, u32 addr)
{
    ఒకవేళ (!MODESEGMENT && mmconfig) {
        తిరిగి readw(mmconfig_addr(bdf, addr));
    } లేకపోతే {
        తిరిగి pci_ioconfig_readw(bdf, addr);
    }
}

u8 pci_ioconfig_readb(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), పోర్టు_PCI_CMD);
    తిరిగి inb(పోర్టు_PCI_దత్తాంశం + (addr & 3));
}

u8 pci_config_readb(u16 bdf, u32 addr)
{
    ఒకవేళ (!MODESEGMENT && mmconfig) {
        తిరిగి readb(mmconfig_addr(bdf, addr));
    } లేకపోతే {
        తిరిగి pci_ioconfig_readb(bdf, addr);
    }
}

శూన్య
pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on)
{
    u16 val = pci_config_readw(bdf, addr);
    val = (val & ~off) | on;
    pci_config_writew(bdf, addr, val);
}

శూన్య
pci_enable_mmconfig(u64 addr, స్థిరరాశి అక్షరం *name)
{
    ఒకవేళ (addr >= 0x100000000ll)
        తిరిగి;
    dprintf(1, "PCIe: using %s mmconfig at 0x%llx\n", name, addr);
    mmconfig = addr;
}

u8 pci_కనుగొను_capability(u16 bdf, u8 cap_id, u8 cap)
{
    పూర్ణాంకం i;
    u16 స్థితి_2 = pci_config_readw(bdf, PCI_స్థితి);

    ఒకవేళ (!(స్థితి_2 & PCI_స్థితి_CAP_LIST))
        తిరిగి 0;

    ఒకవేళ (cap == 0) {
        /* find first */
        cap = pci_config_readb(bdf, PCI_CAPABILITY_LIST);
    } లేకపోతే {
        /* find next */
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_తదుపరి);
    }
    కొరకు (i = 0; cap && i <= 0xff; i++) {
        ఒకవేళ (pci_config_readb(bdf, cap + PCI_CAP_LIST_ID) == cap_id)
            తిరిగి cap;
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_తదుపరి);
    }

    తిరిగి 0;
}

// Helper function for pci_ioconfig_foreachbdf() macro - return next device
పూర్ణాంకం pci_ioconfig_తదుపరి(పూర్ణాంకం bdf, పూర్ణాంకం bus)
{
    ఒకవేళ (pci_bdf_గమ్యం_fn(bdf) == 0
        && (pci_ioconfig_readb(bdf, PCI_శీర్షిక_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    లేకపోతే
        bdf += 1;

    కొరకు (;;) {
        ఒకవేళ (pci_bdf_గమ్యం_bus(bdf) != bus)
            తిరిగి -1;

        u16 v = pci_ioconfig_readw(bdf, PCI_VENDOR_ID);
        ఒకవేళ (v != 0x0000 && v != 0xffff)
            // Device is present.
            తిరిగి bdf;

        ఒకవేళ (pci_bdf_గమ్యం_fn(bdf) == 0)
            bdf += 8;
        లేకపోతే
            bdf += 1;
    }
}

// Helper function for foreachbdf() macro - return next device
పూర్ణాంకం
pci_తదుపరి(పూర్ణాంకం bdf, పూర్ణాంకం bus)
{
    ఒకవేళ (pci_bdf_గమ్యం_fn(bdf) == 0
        && (pci_config_readb(bdf, PCI_శీర్షిక_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    లేకపోతే
        bdf += 1;

    కొరకు (;;) {
        ఒకవేళ (pci_bdf_గమ్యం_bus(bdf) != bus)
            తిరిగి -1;

        u16 v = pci_config_readw(bdf, PCI_VENDOR_ID);
        ఒకవేళ (v != 0x0000 && v != 0xffff)
            // Device is present.
            తిరిగి bdf;

        ఒకవేళ (pci_bdf_గమ్యం_fn(bdf) == 0)
            bdf += 8;
        లేకపోతే
            bdf += 1;
    }
}

// Check if PCI is available at all
పూర్ణాంకం
pci_probe_host(శూన్య)
{
    outl(0x80000000, పోర్టు_PCI_CMD);
    ఒకవేళ (inl(పోర్టు_PCI_CMD) != 0x80000000) {
        dprintf(1, "Detected non-PCI system\n");
        తిరిగి -1;
    }
    తిరిగి 0;
}

శూన్య
pci_reboot(శూన్య)
{
    u8 v = inb(పోర్టు_PCI_REBOOT) & ~6;
    outb(v|2, పోర్టు_PCI_REBOOT); /* Request hard reset */
    udelay(50);
    outb(v|6, పోర్టు_PCI_REBOOT); /* Actually do the reset */
    udelay(50);
}

// PCI config space access functions.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "output.h" // dprintf
#شامل "pci.h" // pci_config_writel
#شامل "pci_regs.h" // PCI_VENDOR_ID
#شامل "util.h" // udelay
#شامل "x86.h" // outl

#تعریف_کرو پورٹ_PCI_CMD           0x0cf8
#تعریف_کرو پورٹ_PCI_ڈیٹا          0x0cfc

ساکن u32 mmconfig;

ساکن خالی *mmconfig_addr(u16 bdf, u32 addr)
{
    واپس (خالی*)(mmconfig + ((u32)bdf << 12) + addr);
}

ساکن u32 ioconfig_cmd(u16 bdf, u32 addr)
{
    واپس 0x80000000 | (bdf << 8) | (addr & 0xfc);
}

خالی pci_ioconfig_writel(u16 bdf, u32 addr, u32 val)
{
    outl(ioconfig_cmd(bdf, addr), پورٹ_PCI_CMD);
    outl(val, پورٹ_PCI_ڈیٹا);
}

خالی pci_config_writel(u16 bdf, u32 addr, u32 val)
{
    اگر (!MODESEGMENT && mmconfig) {
        writel(mmconfig_addr(bdf, addr), val);
    } ورنہ {
        pci_ioconfig_writel(bdf, addr, val);
    }
}

خالی pci_ioconfig_writew(u16 bdf, u32 addr, u16 val)
{
    outl(ioconfig_cmd(bdf, addr), پورٹ_PCI_CMD);
    outw(val, پورٹ_PCI_ڈیٹا + (addr & 2));
}

خالی pci_config_writew(u16 bdf, u32 addr, u16 val)
{
    اگر (!MODESEGMENT && mmconfig) {
        writew(mmconfig_addr(bdf, addr), val);
    } ورنہ {
        pci_ioconfig_writew(bdf, addr, val);
    }
}

خالی pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val)
{
    outl(ioconfig_cmd(bdf, addr), پورٹ_PCI_CMD);
    outb(val, پورٹ_PCI_ڈیٹا + (addr & 3));
}

خالی pci_config_writeb(u16 bdf, u32 addr, u8 val)
{
    اگر (!MODESEGMENT && mmconfig) {
        writeb(mmconfig_addr(bdf, addr), val);
    } ورنہ {
        pci_ioconfig_writeb(bdf, addr, val);
    }
}

u32 pci_ioconfig_readl(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), پورٹ_PCI_CMD);
    واپس inl(پورٹ_PCI_ڈیٹا);
}

u32 pci_config_readl(u16 bdf, u32 addr)
{
    اگر (!MODESEGMENT && mmconfig) {
        واپس readl(mmconfig_addr(bdf, addr));
    } ورنہ {
        واپس pci_ioconfig_readl(bdf, addr);
    }
}

u16 pci_ioconfig_readw(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), پورٹ_PCI_CMD);
    واپس inw(پورٹ_PCI_ڈیٹا + (addr & 2));
}

u16 pci_config_readw(u16 bdf, u32 addr)
{
    اگر (!MODESEGMENT && mmconfig) {
        واپس readw(mmconfig_addr(bdf, addr));
    } ورنہ {
        واپس pci_ioconfig_readw(bdf, addr);
    }
}

u8 pci_ioconfig_readb(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), پورٹ_PCI_CMD);
    واپس inb(پورٹ_PCI_ڈیٹا + (addr & 3));
}

u8 pci_config_readb(u16 bdf, u32 addr)
{
    اگر (!MODESEGMENT && mmconfig) {
        واپس readb(mmconfig_addr(bdf, addr));
    } ورنہ {
        واپس pci_ioconfig_readb(bdf, addr);
    }
}

خالی
pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on)
{
    u16 val = pci_config_readw(bdf, addr);
    val = (val & ~off) | on;
    pci_config_writew(bdf, addr, val);
}

خالی
pci_enable_mmconfig(u64 addr, ثابت حرف *name)
{
    اگر (addr >= 0x100000000ll)
        واپس;
    dprintf(1, "PCIe: using %s mmconfig at 0x%llx\n", name, addr);
    mmconfig = addr;
}

u8 pci_ڈھونڈو_capability(u16 bdf, u8 cap_id, u8 cap)
{
    صحیح i;
    u16 حالت_2 = pci_config_readw(bdf, PCI_حالت);

    اگر (!(حالت_2 & PCI_حالت_CAP_LIST))
        واپس 0;

    اگر (cap == 0) {
        /* find first */
        cap = pci_config_readb(bdf, PCI_CAPABILITY_LIST);
    } ورنہ {
        /* find next */
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_اگلا);
    }
    برائے (i = 0; cap && i <= 0xff; i++) {
        اگر (pci_config_readb(bdf, cap + PCI_CAP_LIST_ID) == cap_id)
            واپس cap;
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_اگلا);
    }

    واپس 0;
}

// Helper function for pci_ioconfig_foreachbdf() macro - return next device
صحیح pci_ioconfig_اگلا(صحیح bdf, صحیح bus)
{
    اگر (pci_bdf_مقصد_fn(bdf) == 0
        && (pci_ioconfig_readb(bdf, PCI_سرخی_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    ورنہ
        bdf += 1;

    برائے (;;) {
        اگر (pci_bdf_مقصد_bus(bdf) != bus)
            واپس -1;

        u16 v = pci_ioconfig_readw(bdf, PCI_VENDOR_ID);
        اگر (v != 0x0000 && v != 0xffff)
            // Device is present.
            واپس bdf;

        اگر (pci_bdf_مقصد_fn(bdf) == 0)
            bdf += 8;
        ورنہ
            bdf += 1;
    }
}

// Helper function for foreachbdf() macro - return next device
صحیح
pci_اگلا(صحیح bdf, صحیح bus)
{
    اگر (pci_bdf_مقصد_fn(bdf) == 0
        && (pci_config_readb(bdf, PCI_سرخی_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    ورنہ
        bdf += 1;

    برائے (;;) {
        اگر (pci_bdf_مقصد_bus(bdf) != bus)
            واپس -1;

        u16 v = pci_config_readw(bdf, PCI_VENDOR_ID);
        اگر (v != 0x0000 && v != 0xffff)
            // Device is present.
            واپس bdf;

        اگر (pci_bdf_مقصد_fn(bdf) == 0)
            bdf += 8;
        ورنہ
            bdf += 1;
    }
}

// Check if PCI is available at all
صحیح
pci_probe_host(خالی)
{
    outl(0x80000000, پورٹ_PCI_CMD);
    اگر (inl(پورٹ_PCI_CMD) != 0x80000000) {
        dprintf(1, "Detected non-PCI system\n");
        واپس -1;
    }
    واپس 0;
}

خالی
pci_reboot(خالی)
{
    u8 v = inb(پورٹ_PCI_REBOOT) & ~6;
    outb(v|2, پورٹ_PCI_REBOOT); /* Request hard reset */
    udelay(50);
    outb(v|6, پورٹ_PCI_REBOOT); /* Actually do the reset */
    udelay(50);
}

// PCI config space access functions.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "output.h" // dprintf
#ضمن "pci.h" // pci_config_writel
#ضمن "pci_regs.h" // PCI_VENDOR_ID
#ضمن "util.h" // udelay
#ضمن "x86.h" // outl

#تعریف_کن پورت_PCI_CMD           0x0cf8
#تعریف_کن پورت_PCI_داده          0x0cfc

ساكن u32 mmconfig;

ساكن فراغ *mmconfig_addr(u16 bdf, u32 addr)
{
    أرجع (فراغ*)(mmconfig + ((u32)bdf << 12) + addr);
}

ساكن u32 ioconfig_cmd(u16 bdf, u32 addr)
{
    أرجع 0x80000000 | (bdf << 8) | (addr & 0xfc);
}

فراغ pci_ioconfig_writel(u16 bdf, u32 addr, u32 val)
{
    outl(ioconfig_cmd(bdf, addr), پورت_PCI_CMD);
    outl(val, پورت_PCI_داده);
}

فراغ pci_config_writel(u16 bdf, u32 addr, u32 val)
{
    إذا (!MODESEGMENT && mmconfig) {
        writel(mmconfig_addr(bdf, addr), val);
    } وإلا {
        pci_ioconfig_writel(bdf, addr, val);
    }
}

فراغ pci_ioconfig_writew(u16 bdf, u32 addr, u16 val)
{
    outl(ioconfig_cmd(bdf, addr), پورت_PCI_CMD);
    outw(val, پورت_PCI_داده + (addr & 2));
}

فراغ pci_config_writew(u16 bdf, u32 addr, u16 val)
{
    إذا (!MODESEGMENT && mmconfig) {
        writew(mmconfig_addr(bdf, addr), val);
    } وإلا {
        pci_ioconfig_writew(bdf, addr, val);
    }
}

فراغ pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val)
{
    outl(ioconfig_cmd(bdf, addr), پورت_PCI_CMD);
    outb(val, پورت_PCI_داده + (addr & 3));
}

فراغ pci_config_writeb(u16 bdf, u32 addr, u8 val)
{
    إذا (!MODESEGMENT && mmconfig) {
        writeb(mmconfig_addr(bdf, addr), val);
    } وإلا {
        pci_ioconfig_writeb(bdf, addr, val);
    }
}

u32 pci_ioconfig_readl(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), پورت_PCI_CMD);
    أرجع inl(پورت_PCI_داده);
}

u32 pci_config_readl(u16 bdf, u32 addr)
{
    إذا (!MODESEGMENT && mmconfig) {
        أرجع readl(mmconfig_addr(bdf, addr));
    } وإلا {
        أرجع pci_ioconfig_readl(bdf, addr);
    }
}

u16 pci_ioconfig_readw(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), پورت_PCI_CMD);
    أرجع inw(پورت_PCI_داده + (addr & 2));
}

u16 pci_config_readw(u16 bdf, u32 addr)
{
    إذا (!MODESEGMENT && mmconfig) {
        أرجع readw(mmconfig_addr(bdf, addr));
    } وإلا {
        أرجع pci_ioconfig_readw(bdf, addr);
    }
}

u8 pci_ioconfig_readb(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), پورت_PCI_CMD);
    أرجع inb(پورت_PCI_داده + (addr & 3));
}

u8 pci_config_readb(u16 bdf, u32 addr)
{
    إذا (!MODESEGMENT && mmconfig) {
        أرجع readb(mmconfig_addr(bdf, addr));
    } وإلا {
        أرجع pci_ioconfig_readb(bdf, addr);
    }
}

فراغ
pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on)
{
    u16 val = pci_config_readw(bdf, addr);
    val = (val & ~off) | on;
    pci_config_writew(bdf, addr, val);
}

فراغ
pci_enable_mmconfig(u64 addr, ثابت حرف *name)
{
    إذا (addr >= 0x100000000ll)
        أرجع;
    dprintf(1, "PCIe: using %s mmconfig at 0x%llx\n", name, addr);
    mmconfig = addr;
}

u8 pci_پیدا_کن_capability(u16 bdf, u8 cap_id, u8 cap)
{
    صحيح i;
    u16 وضعیت = pci_config_readw(bdf, PCI_وضعیت);

    إذا (!(وضعیت & PCI_وضعیت_CAP_LIST))
        أرجع 0;

    إذا (cap == 0) {
        /* find first */
        cap = pci_config_readb(bdf, PCI_CAPABILITY_LIST);
    } وإلا {
        /* find next */
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_بعدی);
    }
    لكل (i = 0; cap && i <= 0xff; i++) {
        إذا (pci_config_readb(bdf, cap + PCI_CAP_LIST_ID) == cap_id)
            أرجع cap;
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_بعدی);
    }

    أرجع 0;
}

// Helper function for pci_ioconfig_foreachbdf() macro - return next device
صحيح pci_ioconfig_بعدی(صحيح bdf, صحيح bus)
{
    إذا (pci_bdf_مقصد_fn(bdf) == 0
        && (pci_ioconfig_readb(bdf, PCI_سرآیند_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    وإلا
        bdf += 1;

    لكل (;;) {
        إذا (pci_bdf_مقصد_bus(bdf) != bus)
            أرجع -1;

        u16 v = pci_ioconfig_readw(bdf, PCI_VENDOR_ID);
        إذا (v != 0x0000 && v != 0xffff)
            // Device is present.
            أرجع bdf;

        إذا (pci_bdf_مقصد_fn(bdf) == 0)
            bdf += 8;
        وإلا
            bdf += 1;
    }
}

// Helper function for foreachbdf() macro - return next device
صحيح
pci_بعدی(صحيح bdf, صحيح bus)
{
    إذا (pci_bdf_مقصد_fn(bdf) == 0
        && (pci_config_readb(bdf, PCI_سرآیند_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    وإلا
        bdf += 1;

    لكل (;;) {
        إذا (pci_bdf_مقصد_bus(bdf) != bus)
            أرجع -1;

        u16 v = pci_config_readw(bdf, PCI_VENDOR_ID);
        إذا (v != 0x0000 && v != 0xffff)
            // Device is present.
            أرجع bdf;

        إذا (pci_bdf_مقصد_fn(bdf) == 0)
            bdf += 8;
        وإلا
            bdf += 1;
    }
}

// Check if PCI is available at all
صحيح
pci_probe_host(فراغ)
{
    outl(0x80000000, پورت_PCI_CMD);
    إذا (inl(پورت_PCI_CMD) != 0x80000000) {
        dprintf(1, "Detected non-PCI system\n");
        أرجع -1;
    }
    أرجع 0;
}

فراغ
pci_reboot(فراغ)
{
    u8 v = inb(پورت_PCI_REBOOT) & ~6;
    outb(v|2, پورت_PCI_REBOOT); /* Request hard reset */
    udelay(50);
    outb(v|6, پورت_PCI_REBOOT); /* Actually do the reset */
    udelay(50);
}

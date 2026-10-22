// PCI config space access functions.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "pci.h" // pci_config_writel
#અન્તર્ભાવ "pci_regs.h" // PCI_VENDOR_ID
#અન્તર્ભાવ "util.h" // udelay
#અન્તર્ભાવ "x86.h" // outl

#define PORT_PCI_CMD           0x0cf8
#define PORT_PCI_DATA          0x0cfc

સ્થિર u32 mmconfig;

સ્થિર શૂન્ય *mmconfig_addr(u16 bdf, u32 addr)
{
    પાછા (શૂન્ય*)(mmconfig + ((u32)bdf << 12) + addr);
}

સ્થિર u32 ioconfig_cmd(u16 bdf, u32 addr)
{
    પાછા 0x80000000 | (bdf << 8) | (addr & 0xfc);
}

શૂન્ય pci_ioconfig_writel(u16 bdf, u32 addr, u32 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outl(val, PORT_PCI_DATA);
}

શૂન્ય pci_config_writel(u16 bdf, u32 addr, u32 val)
{
    જો (!MODESEGMENT && mmconfig) {
        writel(mmconfig_addr(bdf, addr), val);
    } નહીંતર {
        pci_ioconfig_writel(bdf, addr, val);
    }
}

શૂન્ય pci_ioconfig_writew(u16 bdf, u32 addr, u16 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outw(val, PORT_PCI_DATA + (addr & 2));
}

શૂન્ય pci_config_writew(u16 bdf, u32 addr, u16 val)
{
    જો (!MODESEGMENT && mmconfig) {
        writew(mmconfig_addr(bdf, addr), val);
    } નહીંતર {
        pci_ioconfig_writew(bdf, addr, val);
    }
}

શૂન્ય pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outb(val, PORT_PCI_DATA + (addr & 3));
}

શૂન્ય pci_config_writeb(u16 bdf, u32 addr, u8 val)
{
    જો (!MODESEGMENT && mmconfig) {
        writeb(mmconfig_addr(bdf, addr), val);
    } નહીંતર {
        pci_ioconfig_writeb(bdf, addr, val);
    }
}

u32 pci_ioconfig_readl(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    પાછા inl(PORT_PCI_DATA);
}

u32 pci_config_readl(u16 bdf, u32 addr)
{
    જો (!MODESEGMENT && mmconfig) {
        પાછા readl(mmconfig_addr(bdf, addr));
    } નહીંતર {
        પાછા pci_ioconfig_readl(bdf, addr);
    }
}

u16 pci_ioconfig_readw(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    પાછા inw(PORT_PCI_DATA + (addr & 2));
}

u16 pci_config_readw(u16 bdf, u32 addr)
{
    જો (!MODESEGMENT && mmconfig) {
        પાછા readw(mmconfig_addr(bdf, addr));
    } નહીંતર {
        પાછા pci_ioconfig_readw(bdf, addr);
    }
}

u8 pci_ioconfig_readb(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    પાછા inb(PORT_PCI_DATA + (addr & 3));
}

u8 pci_config_readb(u16 bdf, u32 addr)
{
    જો (!MODESEGMENT && mmconfig) {
        પાછા readb(mmconfig_addr(bdf, addr));
    } નહીંતર {
        પાછા pci_ioconfig_readb(bdf, addr);
    }
}

શૂન્ય
pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on)
{
    u16 val = pci_config_readw(bdf, addr);
    val = (val & ~off) | on;
    pci_config_writew(bdf, addr, val);
}

શૂન્ય
pci_enable_mmconfig(u64 addr, અચલ અક્ષર *name)
{
    જો (addr >= 0x100000000ll)
        પાછા;
    dprintf(1, "PCIe: using %s mmconfig at 0x%llx\n", name, addr);
    mmconfig = addr;
}

u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap)
{
    પૂર્ણાંક i;
    u16 status = pci_config_readw(bdf, PCI_STATUS);

    જો (!(status & PCI_STATUS_CAP_LIST))
        પાછા 0;

    જો (cap == 0) {
        /* find first */
        cap = pci_config_readb(bdf, PCI_CAPABILITY_LIST);
    } નહીંતર {
        /* find next */
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_NEXT);
    }
    માટે (i = 0; cap && i <= 0xff; i++) {
        જો (pci_config_readb(bdf, cap + PCI_CAP_LIST_ID) == cap_id)
            પાછા cap;
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_NEXT);
    }

    પાછા 0;
}

// Helper function for pci_ioconfig_foreachbdf() macro - return next device
પૂર્ણાંક pci_ioconfig_next(પૂર્ણાંક bdf, પૂર્ણાંક bus)
{
    જો (pci_bdf_to_fn(bdf) == 0
        && (pci_ioconfig_readb(bdf, PCI_HEADER_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    નહીંતર
        bdf += 1;

    માટે (;;) {
        જો (pci_bdf_to_bus(bdf) != bus)
            પાછા -1;

        u16 v = pci_ioconfig_readw(bdf, PCI_VENDOR_ID);
        જો (v != 0x0000 && v != 0xffff)
            // Device is present.
            પાછા bdf;

        જો (pci_bdf_to_fn(bdf) == 0)
            bdf += 8;
        નહીંતર
            bdf += 1;
    }
}

// Helper function for foreachbdf() macro - return next device
પૂર્ણાંક
pci_next(પૂર્ણાંક bdf, પૂર્ણાંક bus)
{
    જો (pci_bdf_to_fn(bdf) == 0
        && (pci_config_readb(bdf, PCI_HEADER_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    નહીંતર
        bdf += 1;

    માટે (;;) {
        જો (pci_bdf_to_bus(bdf) != bus)
            પાછા -1;

        u16 v = pci_config_readw(bdf, PCI_VENDOR_ID);
        જો (v != 0x0000 && v != 0xffff)
            // Device is present.
            પાછા bdf;

        જો (pci_bdf_to_fn(bdf) == 0)
            bdf += 8;
        નહીંતર
            bdf += 1;
    }
}

// Check if PCI is available at all
પૂર્ણાંક
pci_probe_host(શૂન્ય)
{
    outl(0x80000000, PORT_PCI_CMD);
    જો (inl(PORT_PCI_CMD) != 0x80000000) {
        dprintf(1, "Detected non-PCI system\n");
        પાછા -1;
    }
    પાછા 0;
}

શૂન્ય
pci_reboot(શૂન્ય)
{
    u8 v = inb(PORT_PCI_REBOOT) & ~6;
    outb(v|2, PORT_PCI_REBOOT); /* Request hard reset */
    udelay(50);
    outb(v|6, PORT_PCI_REBOOT); /* Actually do the reset */
    udelay(50);
}

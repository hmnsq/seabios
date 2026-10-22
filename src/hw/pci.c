// PCI config space access functions.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "pci.h" // pci_config_writel
#അന്തര്ഭാവ "pci_regs.h" // PCI_VENDOR_ID
#അന്തര്ഭാവ "util.h" // udelay
#അന്തര്ഭാവ "x86.h" // outl

#define PORT_PCI_CMD           0x0cf8
#define PORT_PCI_DATA          0x0cfc

സ്ഥിതം u32 mmconfig;

സ്ഥിതം ശൂന്യം *mmconfig_addr(u16 bdf, u32 addr)
{
    തിരികെ (ശൂന്യം*)(mmconfig + ((u32)bdf << 12) + addr);
}

സ്ഥിതം u32 ioconfig_cmd(u16 bdf, u32 addr)
{
    തിരികെ 0x80000000 | (bdf << 8) | (addr & 0xfc);
}

ശൂന്യം pci_ioconfig_writel(u16 bdf, u32 addr, u32 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outl(val, PORT_PCI_DATA);
}

ശൂന്യം pci_config_writel(u16 bdf, u32 addr, u32 val)
{
    എങ്കിൽ (!MODESEGMENT && mmconfig) {
        writel(mmconfig_addr(bdf, addr), val);
    } അല്ലെങ്കിൽ {
        pci_ioconfig_writel(bdf, addr, val);
    }
}

ശൂന്യം pci_ioconfig_writew(u16 bdf, u32 addr, u16 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outw(val, PORT_PCI_DATA + (addr & 2));
}

ശൂന്യം pci_config_writew(u16 bdf, u32 addr, u16 val)
{
    എങ്കിൽ (!MODESEGMENT && mmconfig) {
        writew(mmconfig_addr(bdf, addr), val);
    } അല്ലെങ്കിൽ {
        pci_ioconfig_writew(bdf, addr, val);
    }
}

ശൂന്യം pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outb(val, PORT_PCI_DATA + (addr & 3));
}

ശൂന്യം pci_config_writeb(u16 bdf, u32 addr, u8 val)
{
    എങ്കിൽ (!MODESEGMENT && mmconfig) {
        writeb(mmconfig_addr(bdf, addr), val);
    } അല്ലെങ്കിൽ {
        pci_ioconfig_writeb(bdf, addr, val);
    }
}

u32 pci_ioconfig_readl(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    തിരികെ inl(PORT_PCI_DATA);
}

u32 pci_config_readl(u16 bdf, u32 addr)
{
    എങ്കിൽ (!MODESEGMENT && mmconfig) {
        തിരികെ readl(mmconfig_addr(bdf, addr));
    } അല്ലെങ്കിൽ {
        തിരികെ pci_ioconfig_readl(bdf, addr);
    }
}

u16 pci_ioconfig_readw(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    തിരികെ inw(PORT_PCI_DATA + (addr & 2));
}

u16 pci_config_readw(u16 bdf, u32 addr)
{
    എങ്കിൽ (!MODESEGMENT && mmconfig) {
        തിരികെ readw(mmconfig_addr(bdf, addr));
    } അല്ലെങ്കിൽ {
        തിരികെ pci_ioconfig_readw(bdf, addr);
    }
}

u8 pci_ioconfig_readb(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    തിരികെ inb(PORT_PCI_DATA + (addr & 3));
}

u8 pci_config_readb(u16 bdf, u32 addr)
{
    എങ്കിൽ (!MODESEGMENT && mmconfig) {
        തിരികെ readb(mmconfig_addr(bdf, addr));
    } അല്ലെങ്കിൽ {
        തിരികെ pci_ioconfig_readb(bdf, addr);
    }
}

ശൂന്യം
pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on)
{
    u16 val = pci_config_readw(bdf, addr);
    val = (val & ~off) | on;
    pci_config_writew(bdf, addr, val);
}

ശൂന്യം
pci_enable_mmconfig(u64 addr, സ്ഥിരം അക്ഷരം *name)
{
    എങ്കിൽ (addr >= 0x100000000ll)
        തിരികെ;
    dprintf(1, "PCIe: using %s mmconfig at 0x%llx\n", name, addr);
    mmconfig = addr;
}

u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap)
{
    പൂർണ്ണസംഖ്യ i;
    u16 status = pci_config_readw(bdf, PCI_STATUS);

    എങ്കിൽ (!(status & PCI_STATUS_CAP_LIST))
        തിരികെ 0;

    എങ്കിൽ (cap == 0) {
        /* find first */
        cap = pci_config_readb(bdf, PCI_CAPABILITY_LIST);
    } അല്ലെങ്കിൽ {
        /* find next */
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_NEXT);
    }
    വേണ്ടി (i = 0; cap && i <= 0xff; i++) {
        എങ്കിൽ (pci_config_readb(bdf, cap + PCI_CAP_LIST_ID) == cap_id)
            തിരികെ cap;
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_NEXT);
    }

    തിരികെ 0;
}

// Helper function for pci_ioconfig_foreachbdf() macro - return next device
പൂർണ്ണസംഖ്യ pci_ioconfig_next(പൂർണ്ണസംഖ്യ bdf, പൂർണ്ണസംഖ്യ bus)
{
    എങ്കിൽ (pci_bdf_to_fn(bdf) == 0
        && (pci_ioconfig_readb(bdf, PCI_HEADER_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    അല്ലെങ്കിൽ
        bdf += 1;

    വേണ്ടി (;;) {
        എങ്കിൽ (pci_bdf_to_bus(bdf) != bus)
            തിരികെ -1;

        u16 v = pci_ioconfig_readw(bdf, PCI_VENDOR_ID);
        എങ്കിൽ (v != 0x0000 && v != 0xffff)
            // Device is present.
            തിരികെ bdf;

        എങ്കിൽ (pci_bdf_to_fn(bdf) == 0)
            bdf += 8;
        അല്ലെങ്കിൽ
            bdf += 1;
    }
}

// Helper function for foreachbdf() macro - return next device
പൂർണ്ണസംഖ്യ
pci_next(പൂർണ്ണസംഖ്യ bdf, പൂർണ്ണസംഖ്യ bus)
{
    എങ്കിൽ (pci_bdf_to_fn(bdf) == 0
        && (pci_config_readb(bdf, PCI_HEADER_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    അല്ലെങ്കിൽ
        bdf += 1;

    വേണ്ടി (;;) {
        എങ്കിൽ (pci_bdf_to_bus(bdf) != bus)
            തിരികെ -1;

        u16 v = pci_config_readw(bdf, PCI_VENDOR_ID);
        എങ്കിൽ (v != 0x0000 && v != 0xffff)
            // Device is present.
            തിരികെ bdf;

        എങ്കിൽ (pci_bdf_to_fn(bdf) == 0)
            bdf += 8;
        അല്ലെങ്കിൽ
            bdf += 1;
    }
}

// Check if PCI is available at all
പൂർണ്ണസംഖ്യ
pci_probe_host(ശൂന്യം)
{
    outl(0x80000000, PORT_PCI_CMD);
    എങ്കിൽ (inl(PORT_PCI_CMD) != 0x80000000) {
        dprintf(1, "Detected non-PCI system\n");
        തിരികെ -1;
    }
    തിരികെ 0;
}

ശൂന്യം
pci_reboot(ശൂന്യം)
{
    u8 v = inb(PORT_PCI_REBOOT) & ~6;
    outb(v|2, PORT_PCI_REBOOT); /* Request hard reset */
    udelay(50);
    outb(v|6, PORT_PCI_REBOOT); /* Actually do the reset */
    udelay(50);
}

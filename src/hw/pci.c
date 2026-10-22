// PCI config space access functions.
//
// Copyright (C) 2008  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "output.h" // dprintf
#include "pci.h" // pci_config_writel
#include "pci_regs.h" // PCI_VENDOR_ID
#include "util.h" // udelay
#include "x86.h" // outl

#define PORT_PCI_CMD           0x0cf8
#define PORT_PCI_DATA          0x0cfc

static u32 mmconfig;

static void *mmconfig_addr(u16 bdf, u32 addr)
{
    반환 (void*)(mmconfig + ((u32)bdf << 12) + addr);
}

static u32 ioconfig_cmd(u16 bdf, u32 addr)
{
    반환 0x80000000 | (bdf << 8) | (addr & 0xfc);
}

void pci_ioconfig_writel(u16 bdf, u32 addr, u32 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outl(val, PORT_PCI_DATA);
}

void pci_config_writel(u16 bdf, u32 addr, u32 val)
{
    만약 (!MODESEGMENT && mmconfig) {
        writel(mmconfig_addr(bdf, addr), val);
    } 아니면 {
        pci_ioconfig_writel(bdf, addr, val);
    }
}

void pci_ioconfig_writew(u16 bdf, u32 addr, u16 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outw(val, PORT_PCI_DATA + (addr & 2));
}

void pci_config_writew(u16 bdf, u32 addr, u16 val)
{
    만약 (!MODESEGMENT && mmconfig) {
        writew(mmconfig_addr(bdf, addr), val);
    } 아니면 {
        pci_ioconfig_writew(bdf, addr, val);
    }
}

void pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    outb(val, PORT_PCI_DATA + (addr & 3));
}

void pci_config_writeb(u16 bdf, u32 addr, u8 val)
{
    만약 (!MODESEGMENT && mmconfig) {
        writeb(mmconfig_addr(bdf, addr), val);
    } 아니면 {
        pci_ioconfig_writeb(bdf, addr, val);
    }
}

u32 pci_ioconfig_readl(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    반환 inl(PORT_PCI_DATA);
}

u32 pci_config_readl(u16 bdf, u32 addr)
{
    만약 (!MODESEGMENT && mmconfig) {
        반환 readl(mmconfig_addr(bdf, addr));
    } 아니면 {
        반환 pci_ioconfig_readl(bdf, addr);
    }
}

u16 pci_ioconfig_readw(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    반환 inw(PORT_PCI_DATA + (addr & 2));
}

u16 pci_config_readw(u16 bdf, u32 addr)
{
    만약 (!MODESEGMENT && mmconfig) {
        반환 readw(mmconfig_addr(bdf, addr));
    } 아니면 {
        반환 pci_ioconfig_readw(bdf, addr);
    }
}

u8 pci_ioconfig_readb(u16 bdf, u32 addr)
{
    outl(ioconfig_cmd(bdf, addr), PORT_PCI_CMD);
    반환 inb(PORT_PCI_DATA + (addr & 3));
}

u8 pci_config_readb(u16 bdf, u32 addr)
{
    만약 (!MODESEGMENT && mmconfig) {
        반환 readb(mmconfig_addr(bdf, addr));
    } 아니면 {
        반환 pci_ioconfig_readb(bdf, addr);
    }
}

void
pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on)
{
    u16 val = pci_config_readw(bdf, addr);
    val = (val & ~off) | on;
    pci_config_writew(bdf, addr, val);
}

void
pci_enable_mmconfig(u64 addr, const char *name)
{
    만약 (addr >= 0x100000000ll)
        반환;
    dprintf(1, "PCIe: using %s mmconfig at 0x%llx\n", name, addr);
    mmconfig = addr;
}

u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap)
{
    정수 i;
    u16 status = pci_config_readw(bdf, PCI_STATUS);

    만약 (!(status & PCI_STATUS_CAP_LIST))
        반환 0;

    만약 (cap == 0) {
        /* find first */
        cap = pci_config_readb(bdf, PCI_CAPABILITY_LIST);
    } 아니면 {
        /* find next */
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_NEXT);
    }
    위해 (i = 0; cap && i <= 0xff; i++) {
        만약 (pci_config_readb(bdf, cap + PCI_CAP_LIST_ID) == cap_id)
            반환 cap;
        cap = pci_config_readb(bdf, cap + PCI_CAP_LIST_NEXT);
    }

    반환 0;
}

// Helper function for pci_ioconfig_foreachbdf() macro - return next device
정수 pci_ioconfig_next(정수 bdf, 정수 bus)
{
    만약 (pci_bdf_to_fn(bdf) == 0
        && (pci_ioconfig_readb(bdf, PCI_HEADER_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    아니면
        bdf += 1;

    위해 (;;) {
        만약 (pci_bdf_to_bus(bdf) != bus)
            반환 -1;

        u16 v = pci_ioconfig_readw(bdf, PCI_VENDOR_ID);
        만약 (v != 0x0000 && v != 0xffff)
            // Device is present.
            반환 bdf;

        만약 (pci_bdf_to_fn(bdf) == 0)
            bdf += 8;
        아니면
            bdf += 1;
    }
}

// Helper function for foreachbdf() macro - return next device
정수
pci_next(정수 bdf, 정수 bus)
{
    만약 (pci_bdf_to_fn(bdf) == 0
        && (pci_config_readb(bdf, PCI_HEADER_TYPE) & 0x80) == 0)
        // Last found device wasn't a multi-function device - skip to
        // the next device.
        bdf += 8;
    아니면
        bdf += 1;

    위해 (;;) {
        만약 (pci_bdf_to_bus(bdf) != bus)
            반환 -1;

        u16 v = pci_config_readw(bdf, PCI_VENDOR_ID);
        만약 (v != 0x0000 && v != 0xffff)
            // Device is present.
            반환 bdf;

        만약 (pci_bdf_to_fn(bdf) == 0)
            bdf += 8;
        아니면
            bdf += 1;
    }
}

// Check if PCI is available at all
정수
pci_probe_host(void)
{
    outl(0x80000000, PORT_PCI_CMD);
    만약 (inl(PORT_PCI_CMD) != 0x80000000) {
        dprintf(1, "Detected non-PCI system\n");
        반환 -1;
    }
    반환 0;
}

void
pci_reboot(void)
{
    u8 v = inb(PORT_PCI_REBOOT) & ~6;
    outb(v|2, PORT_PCI_REBOOT); /* Request hard reset */
    udelay(50);
    outb(v|6, PORT_PCI_REBOOT); /* Actually do the reset */
    udelay(50);
}

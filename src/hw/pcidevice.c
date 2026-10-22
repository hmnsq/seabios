// Code to maintain and access the pci_device cache
//
// Copyright (C) 2008-2016  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "malloc.h" // malloc_tmp
#include "output.h" // dprintf
#include "pci.h" // pci_config_writel
#include "pcidevice.h" // pci_probe_devices
#include "pci_regs.h" // PCI_VENDOR_ID
#include "romfile.h" // romfile_loadint
#include "stacks.h" // wait_preempt
#include "string.h" // memset

struct hlist_head PCIDevices VARVERIFY32INIT;
整数 MaxPCIBus VARFSEG;

// Find all PCI devices and populate PCIDevices linked list.
void
pci_probe_devices(void)
{
    dprintf(3, "PCI probe\n");
    struct pci_device *busdevs[256];
    memset(busdevs, 0, sizeof(busdevs));
    struct hlist_node **pprev = &PCIDevices.first;
    整数 extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    整数 bus = -1, lastbus = 0, rootbuses = 0, count=0;
    当 (bus < 0xff && (bus < MaxPCIBus || rootbuses < extraroots)) {
        bus++;
        整数 bdf;
        foreachbdf(bdf, bus) {
            // Create new pci_device struct and add to list.
            struct pci_device *dev = malloc_tmp(sizeof(*dev));
            如果 (!dev) {
                warn_noalloc();
                返回;
            }
            memset(dev, 0, sizeof(*dev));
            hlist_add(&dev->node, pprev);
            pprev = &dev->node.next;
            count++;

            // Find parent device.
            整数 rootbus;
            struct pci_device *parent = busdevs[bus];
            如果 (!parent) {
                如果 (bus != lastbus)
                    rootbuses++;
                lastbus = bus;
                rootbus = rootbuses;
                如果 (bus > MaxPCIBus)
                    MaxPCIBus = bus;
            } 否则 {
                rootbus = parent->rootbus;
            }

            // Populate pci_device info.
            dev->bdf = bdf;
            dev->parent = parent;
            dev->rootbus = rootbus;
            u32 vendev = pci_config_readl(bdf, PCI_VENDOR_ID);
            dev->vendor = vendev & 0xffff;
            dev->device = vendev >> 16;
            u32 classrev = pci_config_readl(bdf, PCI_CLASS_REVISION);
            dev->class = classrev >> 16;
            dev->prog_if = classrev >> 8;
            dev->revision = classrev & 0xff;
            dev->header_type = pci_config_readb(bdf, PCI_HEADER_TYPE);
            u8 v = dev->header_type & 0x7f;
            如果 (v == PCI_HEADER_TYPE_BRIDGE || v == PCI_HEADER_TYPE_CARDBUS) {
                u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
                dev->secondary_bus = secbus;
                如果 (secbus > bus && !busdevs[secbus])
                    busdevs[secbus] = dev;
                如果 (secbus > MaxPCIBus)
                    MaxPCIBus = secbus;
            }
            dprintf(4, "PCI device %pP (vd=%04x:%04x c=%04x)\n"
                    , dev, dev->vendor, dev->device, dev->class);
        }
    }
    dprintf(1, "Found %d PCI devices (max PCI bus is %02x)\n", count, MaxPCIBus);
}

// Search for a device with the specified vendor and device ids.
struct pci_device *
pci_find_device(u16 vendid, u16 devid)
{
    struct pci_device *pci;
    foreachpci(pci) {
        如果 (pci->vendor == vendid && pci->device == devid)
            返回 pci;
    }
    返回 NULL;
}

// Search for a device with the specified class id.
struct pci_device *
pci_find_class(u16 classid)
{
    struct pci_device *pci;
    foreachpci(pci) {
        如果 (pci->class == classid)
            返回 pci;
    }
    返回 NULL;
}

整数 pci_init_device(const struct pci_device_id *ids
                    , struct pci_device *pci, void *arg)
{
    当 (ids->vendid || ids->class_mask) {
        如果 ((ids->vendid == PCI_ANY_ID || ids->vendid == pci->vendor) &&
            (ids->devid == PCI_ANY_ID || ids->devid == pci->device) &&
            !((ids->class ^ pci->class) & ids->class_mask)) {
            如果 (ids->func)
                ids->func(pci, arg);
            返回 0;
        }
        ids++;
    }
    返回 -1;
}

struct pci_device *
pci_find_init_device(const struct pci_device_id *ids, void *arg)
{
    struct pci_device *pci;
    foreachpci(pci) {
        如果 (pci_init_device(ids, pci, arg) == 0)
            返回 pci;
    }
    返回 NULL;
}

// Enable PCI bus-mastering (ie, DMA) support on a pci device
void
pci_enable_busmaster(struct pci_device *pci)
{
    wait_preempt();
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MASTER);
    pci->have_driver = 1;
}

// Verify an IO bar and return it to the caller
u16
pci_enable_iobar(struct pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    如果 (!(bar & PCI_BASE_ADDRESS_SPACE_IO)) {
        warn_internalerror();
        返回 0;
    }
    bar &= PCI_BASE_ADDRESS_IO_MASK;
    如果 (bar == 0 || bar > 0xffff) {
        warn_internalerror();
        返回 0;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_IO);
    pci->have_driver = 1;
    返回 bar;
}

// Verify a memory bar and return it to the caller
void *
pci_enable_membar(struct pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    如果 (bar & PCI_BASE_ADDRESS_SPACE_IO) {
        warn_internalerror();
        返回 NULL;
    }
    如果 (bar & PCI_BASE_ADDRESS_MEM_TYPE_64) {
        u32 high = pci_config_readl(pci->bdf, addr+4);
        如果 (high) {
            dprintf(1, "Can not map memory bar over 4Gig\n");
            返回 NULL;
        }
    }
    bar &= PCI_BASE_ADDRESS_MEM_MASK;
    如果 (bar + 4*1024*1024 < 20*1024*1024) {
        // Bar doesn't look valid (it is in last 4M or first 16M)
        warn_internalerror();
        返回 NULL;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);
    pci->have_driver = 1;
    返回 (void*)bar;
}

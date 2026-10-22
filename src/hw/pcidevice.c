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
số_nguyên MaxPCIBus VARFSEG;

// Find all PCI devices and populate PCIDevices linked list.
void
pci_probe_devices(void)
{
    dprintf(3, "PCI probe\n");
    struct pci_device *busdevs[256];
    memset(busdevs, 0, sizeof(busdevs));
    struct hlist_node **pprev = &PCIDevices.first;
    số_nguyên extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    số_nguyên bus = -1, lastbus = 0, rootbuses = 0, count=0;
    trong_khi (bus < 0xff && (bus < MaxPCIBus || rootbuses < extraroots)) {
        bus++;
        số_nguyên bdf;
        foreachbdf(bdf, bus) {
            // Create new pci_device struct and add to list.
            struct pci_device *dev = malloc_tmp(sizeof(*dev));
            nếu (!dev) {
                warn_noalloc();
                trả_về;
            }
            memset(dev, 0, sizeof(*dev));
            hlist_add(&dev->node, pprev);
            pprev = &dev->node.next;
            count++;

            // Find parent device.
            số_nguyên rootbus;
            struct pci_device *parent = busdevs[bus];
            nếu (!parent) {
                nếu (bus != lastbus)
                    rootbuses++;
                lastbus = bus;
                rootbus = rootbuses;
                nếu (bus > MaxPCIBus)
                    MaxPCIBus = bus;
            } không_thì {
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
            nếu (v == PCI_HEADER_TYPE_BRIDGE || v == PCI_HEADER_TYPE_CARDBUS) {
                u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
                dev->secondary_bus = secbus;
                nếu (secbus > bus && !busdevs[secbus])
                    busdevs[secbus] = dev;
                nếu (secbus > MaxPCIBus)
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
        nếu (pci->vendor == vendid && pci->device == devid)
            trả_về pci;
    }
    trả_về NULL;
}

// Search for a device with the specified class id.
struct pci_device *
pci_find_class(u16 classid)
{
    struct pci_device *pci;
    foreachpci(pci) {
        nếu (pci->class == classid)
            trả_về pci;
    }
    trả_về NULL;
}

số_nguyên pci_init_device(const struct pci_device_id *ids
                    , struct pci_device *pci, void *arg)
{
    trong_khi (ids->vendid || ids->class_mask) {
        nếu ((ids->vendid == PCI_ANY_ID || ids->vendid == pci->vendor) &&
            (ids->devid == PCI_ANY_ID || ids->devid == pci->device) &&
            !((ids->class ^ pci->class) & ids->class_mask)) {
            nếu (ids->func)
                ids->func(pci, arg);
            trả_về 0;
        }
        ids++;
    }
    trả_về -1;
}

struct pci_device *
pci_find_init_device(const struct pci_device_id *ids, void *arg)
{
    struct pci_device *pci;
    foreachpci(pci) {
        nếu (pci_init_device(ids, pci, arg) == 0)
            trả_về pci;
    }
    trả_về NULL;
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
    nếu (!(bar & PCI_BASE_ADDRESS_SPACE_IO)) {
        warn_internalerror();
        trả_về 0;
    }
    bar &= PCI_BASE_ADDRESS_IO_MASK;
    nếu (bar == 0 || bar > 0xffff) {
        warn_internalerror();
        trả_về 0;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_IO);
    pci->have_driver = 1;
    trả_về bar;
}

// Verify a memory bar and return it to the caller
void *
pci_enable_membar(struct pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    nếu (bar & PCI_BASE_ADDRESS_SPACE_IO) {
        warn_internalerror();
        trả_về NULL;
    }
    nếu (bar & PCI_BASE_ADDRESS_MEM_TYPE_64) {
        u32 high = pci_config_readl(pci->bdf, addr+4);
        nếu (high) {
            dprintf(1, "Can not map memory bar over 4Gig\n");
            trả_về NULL;
        }
    }
    bar &= PCI_BASE_ADDRESS_MEM_MASK;
    nếu (bar + 4*1024*1024 < 20*1024*1024) {
        // Bar doesn't look valid (it is in last 4M or first 16M)
        warn_internalerror();
        trả_về NULL;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);
    pci->have_driver = 1;
    trả_về (void*)bar;
}

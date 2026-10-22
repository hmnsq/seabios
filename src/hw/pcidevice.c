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

sa_nracanaa hlist_head PCIDevices VARVERIFY32INIT;
puur_naa_nka MaxPCIBus VARFSEG;

// Find all PCI devices and populate PCIDevices linked list.
void
pci_probe_devices(void)
{
    dprintf(3, "PCI probe\n");
    sa_nracanaa pci_device *busdevs[256];
    memset(busdevs, 0, parimaa_na(busdevs));
    sa_nracanaa hlist_node **pprev = &PCIDevices.first;
    puur_naa_nka extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    puur_naa_nka bus = -1, lastbus = 0, rootbuses = 0, count=0;
    ᨀᨛ (bus < 0xff && (bus < MaxPCIBus || rootbuses < extraroots)) {
        bus++;
        puur_naa_nka bdf;
        foreachbdf(bdf, bus) {
            // Create new pci_device struct and add to list.
            sa_nracanaa pci_device *dev = malloc_tmp(parimaa_na(*dev));
            ᨀᨘ (!dev) {
                warn_noalloc();
                ᨀᨘᨒ;
            }
            memset(dev, 0, parimaa_na(*dev));
            hlist_add(&dev->node, pprev);
            pprev = &dev->node.next;
            count++;

            // Find parent device.
            puur_naa_nka rootbus;
            sa_nracanaa pci_device *parent = busdevs[bus];
            ᨀᨘ (!parent) {
                ᨀᨘ (bus != lastbus)
                    rootbuses++;
                lastbus = bus;
                rootbus = rootbuses;
                ᨀᨘ (bus > MaxPCIBus)
                    MaxPCIBus = bus;
            } ᨀᨘᨁ {
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
            ᨀᨘ (v == PCI_HEADER_TYPE_BRIDGE || v == PCI_HEADER_TYPE_CARDBUS) {
                u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
                dev->secondary_bus = secbus;
                ᨀᨘ (secbus > bus && !busdevs[secbus])
                    busdevs[secbus] = dev;
                ᨀᨘ (secbus > MaxPCIBus)
                    MaxPCIBus = secbus;
            }
            dprintf(4, "PCI device %pP (vd=%04x:%04x c=%04x)\n"
                    , dev, dev->vendor, dev->device, dev->class);
        }
    }
    dprintf(1, "Found %d PCI devices (max PCI bus is %02x)\n", count, MaxPCIBus);
}

// Search for a device with the specified vendor and device ids.
sa_nracanaa pci_device *
pci_find_device(u16 vendid, u16 devid)
{
    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        ᨀᨘ (pci->vendor == vendid && pci->device == devid)
            ᨀᨘᨒ pci;
    }
    ᨀᨘᨒ NULL;
}

// Search for a device with the specified class id.
sa_nracanaa pci_device *
pci_find_class(u16 classid)
{
    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        ᨀᨘ (pci->class == classid)
            ᨀᨘᨒ pci;
    }
    ᨀᨘᨒ NULL;
}

puur_naa_nka pci_init_device(const sa_nracanaa pci_device_id *ids
                    , sa_nracanaa pci_device *pci, void *arg)
{
    ᨀᨛ (ids->vendid || ids->class_mask) {
        ᨀᨘ ((ids->vendid == PCI_ANY_ID || ids->vendid == pci->vendor) &&
            (ids->devid == PCI_ANY_ID || ids->devid == pci->device) &&
            !((ids->class ^ pci->class) & ids->class_mask)) {
            ᨀᨘ (ids->func)
                ids->func(pci, arg);
            ᨀᨘᨒ 0;
        }
        ids++;
    }
    ᨀᨘᨒ -1;
}

sa_nracanaa pci_device *
pci_find_init_device(const sa_nracanaa pci_device_id *ids, void *arg)
{
    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        ᨀᨘ (pci_init_device(ids, pci, arg) == 0)
            ᨀᨘᨒ pci;
    }
    ᨀᨘᨒ NULL;
}

// Enable PCI bus-mastering (ie, DMA) support on a pci device
void
pci_enable_busmaster(sa_nracanaa pci_device *pci)
{
    wait_preempt();
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MASTER);
    pci->have_driver = 1;
}

// Verify an IO bar and return it to the caller
u16
pci_enable_iobar(sa_nracanaa pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    ᨀᨘ (!(bar & PCI_BASE_ADDRESS_SPACE_IO)) {
        warn_internalerror();
        ᨀᨘᨒ 0;
    }
    bar &= PCI_BASE_ADDRESS_IO_MASK;
    ᨀᨘ (bar == 0 || bar > 0xffff) {
        warn_internalerror();
        ᨀᨘᨒ 0;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_IO);
    pci->have_driver = 1;
    ᨀᨘᨒ bar;
}

// Verify a memory bar and return it to the caller
void *
pci_enable_membar(sa_nracanaa pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    ᨀᨘ (bar & PCI_BASE_ADDRESS_SPACE_IO) {
        warn_internalerror();
        ᨀᨘᨒ NULL;
    }
    ᨀᨘ (bar & PCI_BASE_ADDRESS_MEM_TYPE_64) {
        u32 high = pci_config_readl(pci->bdf, addr+4);
        ᨀᨘ (high) {
            dprintf(1, "Can not map memory bar over 4Gig\n");
            ᨀᨘᨒ NULL;
        }
    }
    bar &= PCI_BASE_ADDRESS_MEM_MASK;
    ᨀᨘ (bar + 4*1024*1024 < 20*1024*1024) {
        // Bar doesn't look valid (it is in last 4M or first 16M)
        warn_internalerror();
        ᨀᨘᨒ NULL;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);
    pci->have_driver = 1;
    ᨀᨘᨒ (void*)bar;
}

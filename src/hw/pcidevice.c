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
ကိန်းပြည့် MaxPCIBus VARFSEG;

// Find all PCI devices and populate PCIDevices linked list.
void
pci_probe_devices(void)
{
    dprintf(3, "PCI probe\n");
    sa_nracanaa pci_device *busdevs[256];
    memset(busdevs, 0, parimaa_na(busdevs));
    sa_nracanaa hlist_node **pprev = &PCIDevices.first;
    ကိန်းပြည့် extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    ကိန်းပြည့် bus = -1, lastbus = 0, rootbuses = 0, count=0;
    နေစဉ် (bus < 0xff && (bus < MaxPCIBus || rootbuses < extraroots)) {
        bus++;
        ကိန်းပြည့် bdf;
        foreachbdf(bdf, bus) {
            // Create new pci_device struct and add to list.
            sa_nracanaa pci_device *dev = malloc_tmp(parimaa_na(*dev));
            အကယ်၍ (!dev) {
                warn_noalloc();
                ပြန်ပေး;
            }
            memset(dev, 0, parimaa_na(*dev));
            hlist_add(&dev->node, pprev);
            pprev = &dev->node.next;
            count++;

            // Find parent device.
            ကိန်းပြည့် rootbus;
            sa_nracanaa pci_device *parent = busdevs[bus];
            အကယ်၍ (!parent) {
                အကယ်၍ (bus != lastbus)
                    rootbuses++;
                lastbus = bus;
                rootbus = rootbuses;
                အကယ်၍ (bus > MaxPCIBus)
                    MaxPCIBus = bus;
            } မဟုတ်ပါက {
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
            အကယ်၍ (v == PCI_HEADER_TYPE_BRIDGE || v == PCI_HEADER_TYPE_CARDBUS) {
                u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
                dev->secondary_bus = secbus;
                အကယ်၍ (secbus > bus && !busdevs[secbus])
                    busdevs[secbus] = dev;
                အကယ်၍ (secbus > MaxPCIBus)
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
        အကယ်၍ (pci->vendor == vendid && pci->device == devid)
            ပြန်ပေး pci;
    }
    ပြန်ပေး NULL;
}

// Search for a device with the specified class id.
sa_nracanaa pci_device *
pci_find_class(u16 classid)
{
    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        အကယ်၍ (pci->class == classid)
            ပြန်ပေး pci;
    }
    ပြန်ပေး NULL;
}

ကိန်းပြည့် pci_init_device(const sa_nracanaa pci_device_id *ids
                    , sa_nracanaa pci_device *pci, void *arg)
{
    နေစဉ် (ids->vendid || ids->class_mask) {
        အကယ်၍ ((ids->vendid == PCI_ANY_ID || ids->vendid == pci->vendor) &&
            (ids->devid == PCI_ANY_ID || ids->devid == pci->device) &&
            !((ids->class ^ pci->class) & ids->class_mask)) {
            အကယ်၍ (ids->func)
                ids->func(pci, arg);
            ပြန်ပေး 0;
        }
        ids++;
    }
    ပြန်ပေး -1;
}

sa_nracanaa pci_device *
pci_find_init_device(const sa_nracanaa pci_device_id *ids, void *arg)
{
    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        အကယ်၍ (pci_init_device(ids, pci, arg) == 0)
            ပြန်ပေး pci;
    }
    ပြန်ပေး NULL;
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
    အကယ်၍ (!(bar & PCI_BASE_ADDRESS_SPACE_IO)) {
        warn_internalerror();
        ပြန်ပေး 0;
    }
    bar &= PCI_BASE_ADDRESS_IO_MASK;
    အကယ်၍ (bar == 0 || bar > 0xffff) {
        warn_internalerror();
        ပြန်ပေး 0;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_IO);
    pci->have_driver = 1;
    ပြန်ပေး bar;
}

// Verify a memory bar and return it to the caller
void *
pci_enable_membar(sa_nracanaa pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    အကယ်၍ (bar & PCI_BASE_ADDRESS_SPACE_IO) {
        warn_internalerror();
        ပြန်ပေး NULL;
    }
    အကယ်၍ (bar & PCI_BASE_ADDRESS_MEM_TYPE_64) {
        u32 high = pci_config_readl(pci->bdf, addr+4);
        အကယ်၍ (high) {
            dprintf(1, "Can not map memory bar over 4Gig\n");
            ပြန်ပေး NULL;
        }
    }
    bar &= PCI_BASE_ADDRESS_MEM_MASK;
    အကယ်၍ (bar + 4*1024*1024 < 20*1024*1024) {
        // Bar doesn't look valid (it is in last 4M or first 16M)
        warn_internalerror();
        ပြန်ပေး NULL;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);
    pci->have_driver = 1;
    ပြန်ပေး (void*)bar;
}

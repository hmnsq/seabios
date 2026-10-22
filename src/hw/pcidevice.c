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
ຈຳນວນເຕັມ MaxPCIBus VARFSEG;

// Find all PCI devices and populate PCIDevices linked list.
void
pci_probe_devices(void)
{
    dprintf(3, "PCI probe\n");
    sa_nracanaa pci_device *busdevs[256];
    memset(busdevs, 0, parimaa_na(busdevs));
    sa_nracanaa hlist_node **pprev = &PCIDevices.first;
    ຈຳນວນເຕັມ extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    ຈຳນວນເຕັມ bus = -1, lastbus = 0, rootbuses = 0, count=0;
    ໃນຂະນະທີ່ (bus < 0xff && (bus < MaxPCIBus || rootbuses < extraroots)) {
        bus++;
        ຈຳນວນເຕັມ bdf;
        foreachbdf(bdf, bus) {
            // Create new pci_device struct and add to list.
            sa_nracanaa pci_device *dev = malloc_tmp(parimaa_na(*dev));
            ຖ້າ (!dev) {
                warn_noalloc();
                ສົ່ງຄືນ;
            }
            memset(dev, 0, parimaa_na(*dev));
            hlist_add(&dev->node, pprev);
            pprev = &dev->node.next;
            count++;

            // Find parent device.
            ຈຳນວນເຕັມ rootbus;
            sa_nracanaa pci_device *parent = busdevs[bus];
            ຖ້າ (!parent) {
                ຖ້າ (bus != lastbus)
                    rootbuses++;
                lastbus = bus;
                rootbus = rootbuses;
                ຖ້າ (bus > MaxPCIBus)
                    MaxPCIBus = bus;
            } ບໍ່ດັ່ງນັ້ນ {
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
            ຖ້າ (v == PCI_HEADER_TYPE_BRIDGE || v == PCI_HEADER_TYPE_CARDBUS) {
                u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
                dev->secondary_bus = secbus;
                ຖ້າ (secbus > bus && !busdevs[secbus])
                    busdevs[secbus] = dev;
                ຖ້າ (secbus > MaxPCIBus)
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
        ຖ້າ (pci->vendor == vendid && pci->device == devid)
            ສົ່ງຄືນ pci;
    }
    ສົ່ງຄືນ NULL;
}

// Search for a device with the specified class id.
sa_nracanaa pci_device *
pci_find_class(u16 classid)
{
    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        ຖ້າ (pci->class == classid)
            ສົ່ງຄືນ pci;
    }
    ສົ່ງຄືນ NULL;
}

ຈຳນວນເຕັມ pci_init_device(const sa_nracanaa pci_device_id *ids
                    , sa_nracanaa pci_device *pci, void *arg)
{
    ໃນຂະນະທີ່ (ids->vendid || ids->class_mask) {
        ຖ້າ ((ids->vendid == PCI_ANY_ID || ids->vendid == pci->vendor) &&
            (ids->devid == PCI_ANY_ID || ids->devid == pci->device) &&
            !((ids->class ^ pci->class) & ids->class_mask)) {
            ຖ້າ (ids->func)
                ids->func(pci, arg);
            ສົ່ງຄືນ 0;
        }
        ids++;
    }
    ສົ່ງຄືນ -1;
}

sa_nracanaa pci_device *
pci_find_init_device(const sa_nracanaa pci_device_id *ids, void *arg)
{
    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        ຖ້າ (pci_init_device(ids, pci, arg) == 0)
            ສົ່ງຄືນ pci;
    }
    ສົ່ງຄືນ NULL;
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
    ຖ້າ (!(bar & PCI_BASE_ADDRESS_SPACE_IO)) {
        warn_internalerror();
        ສົ່ງຄືນ 0;
    }
    bar &= PCI_BASE_ADDRESS_IO_MASK;
    ຖ້າ (bar == 0 || bar > 0xffff) {
        warn_internalerror();
        ສົ່ງຄືນ 0;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_IO);
    pci->have_driver = 1;
    ສົ່ງຄືນ bar;
}

// Verify a memory bar and return it to the caller
void *
pci_enable_membar(sa_nracanaa pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    ຖ້າ (bar & PCI_BASE_ADDRESS_SPACE_IO) {
        warn_internalerror();
        ສົ່ງຄືນ NULL;
    }
    ຖ້າ (bar & PCI_BASE_ADDRESS_MEM_TYPE_64) {
        u32 high = pci_config_readl(pci->bdf, addr+4);
        ຖ້າ (high) {
            dprintf(1, "Can not map memory bar over 4Gig\n");
            ສົ່ງຄືນ NULL;
        }
    }
    bar &= PCI_BASE_ADDRESS_MEM_MASK;
    ຖ້າ (bar + 4*1024*1024 < 20*1024*1024) {
        // Bar doesn't look valid (it is in last 4M or first 16M)
        warn_internalerror();
        ສົ່ງຄືນ NULL;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);
    pci->have_driver = 1;
    ສົ່ງຄືນ (void*)bar;
}

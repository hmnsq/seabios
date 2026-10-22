// Code to maintain and access the pci_device cache
//
// Copyright (C) 2008-2016  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "malloc.h" // malloc_tmp
#شامل "output.h" // dprintf
#شامل "pci.h" // pci_config_writel
#شامل "pcidevice.h" // pci_probe_devices
#شامل "pci_regs.h" // PCI_VENDOR_ID
#شامل "romfile.h" // romfile_loadint
#شامل "stacks.h" // wait_preempt
#شامل "string.h" // memset

ساختار hlist_head PCIDevices VARVERIFY32INIT;
صحیح MaxPCIBus VARFSEG;

// Find all PCI devices and populate PCIDevices linked list.
تهی
pci_probe_devices(تهی)
{
    dprintf(3, "PCI probe\n");
    ساختار pci_device *busdevs[256];
    memset(busdevs, 0, اندازه(busdevs));
    ساختار hlist_گره **pprev = &PCIDevices.first;
    صحیح extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    صحیح bus = -1, lastbus = 0, rootbuses = 0, شمار=0;
    وقتی (bus < 0xff && (bus < MaxPCIBus || rootbuses < extraroots)) {
        bus++;
        صحیح bdf;
        foreachbdf(bdf, bus) {
            // Create new pci_device struct and add to list.
            ساختار pci_device *dev = malloc_tmp(اندازه(*dev));
            اگر (!dev) {
                warn_noalloc();
                بازگشت;
            }
            memset(dev, 0, اندازه(*dev));
            hlist_add(&dev->گره, pprev);
            pprev = &dev->گره.next;
            شمار++;

            // Find parent device.
            صحیح rootbus;
            ساختار pci_device *parent = busdevs[bus];
            اگر (!parent) {
                اگر (bus != lastbus)
                    rootbuses++;
                lastbus = bus;
                rootbus = rootbuses;
                اگر (bus > MaxPCIBus)
                    MaxPCIBus = bus;
            } وگرنه {
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
            اگر (v == PCI_HEADER_TYPE_BRIDGE || v == PCI_HEADER_TYPE_CARDBUS) {
                u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
                dev->secondary_bus = secbus;
                اگر (secbus > bus && !busdevs[secbus])
                    busdevs[secbus] = dev;
                اگر (secbus > MaxPCIBus)
                    MaxPCIBus = secbus;
            }
            dprintf(4, "PCI device %pP (vd=%04x:%04x c=%04x)\n"
                    , dev, dev->vendor, dev->device, dev->class);
        }
    }
    dprintf(1, "Found %d PCI devices (max PCI bus is %02x)\n", شمار, MaxPCIBus);
}

// Search for a device with the specified vendor and device ids.
ساختار pci_device *
pci_find_device(u16 vendid, u16 devid)
{
    ساختار pci_device *pci;
    foreachpci(pci) {
        اگر (pci->vendor == vendid && pci->device == devid)
            بازگشت pci;
    }
    بازگشت پوچ;
}

// Search for a device with the specified class id.
ساختار pci_device *
pci_find_class(u16 classid)
{
    ساختار pci_device *pci;
    foreachpci(pci) {
        اگر (pci->class == classid)
            بازگشت pci;
    }
    بازگشت پوچ;
}

صحیح pci_init_device(ثابت ساختار pci_device_id *ids
                    , ساختار pci_device *pci, تهی *arg)
{
    وقتی (ids->vendid || ids->class_mask) {
        اگر ((ids->vendid == PCI_ANY_ID || ids->vendid == pci->vendor) &&
            (ids->devid == PCI_ANY_ID || ids->devid == pci->device) &&
            !((ids->class ^ pci->class) & ids->class_mask)) {
            اگر (ids->func)
                ids->func(pci, arg);
            بازگشت 0;
        }
        ids++;
    }
    بازگشت -1;
}

ساختار pci_device *
pci_find_init_device(ثابت ساختار pci_device_id *ids, تهی *arg)
{
    ساختار pci_device *pci;
    foreachpci(pci) {
        اگر (pci_init_device(ids, pci, arg) == 0)
            بازگشت pci;
    }
    بازگشت پوچ;
}

// Enable PCI bus-mastering (ie, DMA) support on a pci device
تهی
pci_enable_busmaster(ساختار pci_device *pci)
{
    wait_preempt();
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MASTER);
    pci->have_driver = 1;
}

// Verify an IO bar and return it to the caller
u16
pci_enable_iobar(ساختار pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    اگر (!(bar & PCI_BASE_ADDRESS_SPACE_IO)) {
        warn_internalerror();
        بازگشت 0;
    }
    bar &= PCI_BASE_ADDRESS_IO_MASK;
    اگر (bar == 0 || bar > 0xffff) {
        warn_internalerror();
        بازگشت 0;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_IO);
    pci->have_driver = 1;
    بازگشت bar;
}

// Verify a memory bar and return it to the caller
تهی *
pci_enable_membar(ساختار pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    اگر (bar & PCI_BASE_ADDRESS_SPACE_IO) {
        warn_internalerror();
        بازگشت پوچ;
    }
    اگر (bar & PCI_BASE_ADDRESS_MEM_TYPE_64) {
        u32 بالا = pci_config_readl(pci->bdf, addr+4);
        اگر (بالا) {
            dprintf(1, "Can not map memory bar over 4Gig\n");
            بازگشت پوچ;
        }
    }
    bar &= PCI_BASE_ADDRESS_MEM_MASK;
    اگر (bar + 4*1024*1024 < 20*1024*1024) {
        // Bar doesn't look valid (it is in last 4M or first 16M)
        warn_internalerror();
        بازگشت پوچ;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);
    pci->have_driver = 1;
    بازگشت (تهی*)bar;
}

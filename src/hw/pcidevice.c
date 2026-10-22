// Code to maintain and access the pci_device cache
//
// Copyright (C) 2008-2016  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "malloc.h" // malloc_tmp
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "pci.h" // pci_config_writel
#অন্তর্ভাব "pcidevice.h" // pci_probe_devices
#অন্তর্ভাব "pci_regs.h" // PCI_VENDOR_ID
#অন্তর্ভাব "romfile.h" // romfile_loadint
#অন্তর্ভাব "stacks.h" // wait_preempt
#অন্তর্ভাব "string.h" // memset

কাঠামো hlist_শীর্ষ PCIDevices VARVERIFY32INIT;
পূর্ণসংখ্যা MaxPCIBus VARFSEG;

// Find all PCI devices and populate PCIDevices linked list.
শূন্য
pci_probe_devices(শূন্য)
{
    dprintf(3, "PCI probe\n");
    কাঠামো pci_device *busdevs[256];
    memset(busdevs, 0, আকার(busdevs));
    কাঠামো hlist_গ্রন্থি **pprev = &PCIDevices.first;
    পূর্ণসংখ্যা extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    পূর্ণসংখ্যা bus = -1, lastbus = 0, rootbuses = 0, গুনতি=0;
    যখন (bus < 0xff && (bus < MaxPCIBus || rootbuses < extraroots)) {
        bus++;
        পূর্ণসংখ্যা bdf;
        foreachbdf(bdf, bus) {
            // Create new pci_device struct and add to list.
            কাঠামো pci_device *dev = malloc_tmp(আকার(*dev));
            যদি (!dev) {
                warn_noalloc();
                ফেরত;
            }
            memset(dev, 0, আকার(*dev));
            hlist_add(&dev->গ্রন্থি, pprev);
            pprev = &dev->গ্রন্থি.পরবর্তী;
            গুনতি++;

            // Find parent device.
            পূর্ণসংখ্যা rootbus;
            কাঠামো pci_device *parent = busdevs[bus];
            যদি (!parent) {
                যদি (bus != lastbus)
                    rootbuses++;
                lastbus = bus;
                rootbus = rootbuses;
                যদি (bus > MaxPCIBus)
                    MaxPCIBus = bus;
            } অন্যথা {
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
            যদি (v == PCI_HEADER_TYPE_BRIDGE || v == PCI_HEADER_TYPE_CARDBUS) {
                u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
                dev->secondary_bus = secbus;
                যদি (secbus > bus && !busdevs[secbus])
                    busdevs[secbus] = dev;
                যদি (secbus > MaxPCIBus)
                    MaxPCIBus = secbus;
            }
            dprintf(4, "PCI device %pP (vd=%04x:%04x c=%04x)\n"
                    , dev, dev->vendor, dev->device, dev->class);
        }
    }
    dprintf(1, "Found %d PCI devices (max PCI bus is %02x)\n", গুনতি, MaxPCIBus);
}

// Search for a device with the specified vendor and device ids.
কাঠামো pci_device *
pci_find_device(u16 vendid, u16 devid)
{
    কাঠামো pci_device *pci;
    foreachpci(pci) {
        যদি (pci->vendor == vendid && pci->device == devid)
            ফেরত pci;
    }
    ফেরত ফাঁকা;
}

// Search for a device with the specified class id.
কাঠামো pci_device *
pci_find_class(u16 classid)
{
    কাঠামো pci_device *pci;
    foreachpci(pci) {
        যদি (pci->class == classid)
            ফেরত pci;
    }
    ফেরত ফাঁকা;
}

পূর্ণসংখ্যা pci_init_device(ধ্রুবক কাঠামো pci_device_id *ids
                    , কাঠামো pci_device *pci, শূন্য *arg)
{
    যখন (ids->vendid || ids->class_mask) {
        যদি ((ids->vendid == PCI_ANY_ID || ids->vendid == pci->vendor) &&
            (ids->devid == PCI_ANY_ID || ids->devid == pci->device) &&
            !((ids->class ^ pci->class) & ids->class_mask)) {
            যদি (ids->func)
                ids->func(pci, arg);
            ফেরত 0;
        }
        ids++;
    }
    ফেরত -1;
}

কাঠামো pci_device *
pci_find_init_device(ধ্রুবক কাঠামো pci_device_id *ids, শূন্য *arg)
{
    কাঠামো pci_device *pci;
    foreachpci(pci) {
        যদি (pci_init_device(ids, pci, arg) == 0)
            ফেরত pci;
    }
    ফেরত ফাঁকা;
}

// Enable PCI bus-mastering (ie, DMA) support on a pci device
শূন্য
pci_enable_busmaster(কাঠামো pci_device *pci)
{
    wait_preempt();
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MASTER);
    pci->have_driver = 1;
}

// Verify an IO bar and return it to the caller
u16
pci_enable_iobar(কাঠামো pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    যদি (!(bar & PCI_BASE_ADDRESS_SPACE_IO)) {
        warn_internalerror();
        ফেরত 0;
    }
    bar &= PCI_BASE_ADDRESS_IO_MASK;
    যদি (bar == 0 || bar > 0xffff) {
        warn_internalerror();
        ফেরত 0;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_IO);
    pci->have_driver = 1;
    ফেরত bar;
}

// Verify a memory bar and return it to the caller
শূন্য *
pci_enable_membar(কাঠামো pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    যদি (bar & PCI_BASE_ADDRESS_SPACE_IO) {
        warn_internalerror();
        ফেরত ফাঁকা;
    }
    যদি (bar & PCI_BASE_ADDRESS_MEM_TYPE_64) {
        u32 উপরের = pci_config_readl(pci->bdf, addr+4);
        যদি (উপরের) {
            dprintf(1, "Can not map memory bar over 4Gig\n");
            ফেরত ফাঁকা;
        }
    }
    bar &= PCI_BASE_ADDRESS_MEM_MASK;
    যদি (bar + 4*1024*1024 < 20*1024*1024) {
        // Bar doesn't look valid (it is in last 4M or first 16M)
        warn_internalerror();
        ফেরত ফাঁকা;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);
    pci->have_driver = 1;
    ফেরত (শূন্য*)bar;
}

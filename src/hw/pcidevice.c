// Code to maintain and access the pci_device cache
//
// Copyright (C) 2008-2016  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "malloc.h" // malloc_tmp
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "pci.h" // pci_config_writel
#अन्तर्भाव "pcidevice.h" // pci_probe_devices
#अन्तर्भाव "pci_regs.h" // PCI_VENDOR_ID
#अन्तर्भाव "romfile.h" // romfile_loadint
#अन्तर्भाव "stacks.h" // wait_preempt
#अन्तर्भाव "string.h" // memset

संरचना hlist_head PCIDevices VARVERIFY32INIT;
पूर्णाङ्क MaxPCIBus VARFSEG;

// Find all PCI devices and populate PCIDevices linked list.
शून्य
pci_probe_devices(शून्य)
{
    dprintf(3, "PCI probe\n");
    संरचना pci_device *busdevs[256];
    memset(busdevs, 0, परिमाण(busdevs));
    संरचना hlist_ग्रन्थिः **pprev = &PCIDevices.first;
    पूर्णाङ्क extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    पूर्णाङ्क bus = -1, lastbus = 0, rootbuses = 0, गणनम्=0;
    याव (bus < 0xff && (bus < MaxPCIBus || rootbuses < extraroots)) {
        bus++;
        पूर्णाङ्क bdf;
        foreachbdf(bdf, bus) {
            // Create new pci_device struct and add to list.
            संरचना pci_device *dev = malloc_tmp(परिमाण(*dev));
            सचे (!dev) {
                warn_noalloc();
                पटिनिवत्त;
            }
            memset(dev, 0, परिमाण(*dev));
            hlist_add(&dev->ग्रन्थिः, pprev);
            pprev = &dev->ग्रन्थिः.next;
            गणनम्++;

            // Find parent device.
            पूर्णाङ्क rootbus;
            संरचना pci_device *parent = busdevs[bus];
            सचे (!parent) {
                सचे (bus != lastbus)
                    rootbuses++;
                lastbus = bus;
                rootbus = rootbuses;
                सचे (bus > MaxPCIBus)
                    MaxPCIBus = bus;
            } अञ्ञथा {
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
            सचे (v == PCI_HEADER_TYPE_BRIDGE || v == PCI_HEADER_TYPE_CARDBUS) {
                u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
                dev->secondary_bus = secbus;
                सचे (secbus > bus && !busdevs[secbus])
                    busdevs[secbus] = dev;
                सचे (secbus > MaxPCIBus)
                    MaxPCIBus = secbus;
            }
            dprintf(4, "PCI device %pP (vd=%04x:%04x c=%04x)\n"
                    , dev, dev->vendor, dev->device, dev->class);
        }
    }
    dprintf(1, "Found %d PCI devices (max PCI bus is %02x)\n", गणनम्, MaxPCIBus);
}

// Search for a device with the specified vendor and device ids.
संरचना pci_device *
pci_find_device(u16 vendid, u16 devid)
{
    संरचना pci_device *pci;
    foreachpci(pci) {
        सचे (pci->vendor == vendid && pci->device == devid)
            पटिनिवत्त pci;
    }
    पटिनिवत्त रिक्त;
}

// Search for a device with the specified class id.
संरचना pci_device *
pci_find_class(u16 classid)
{
    संरचना pci_device *pci;
    foreachpci(pci) {
        सचे (pci->class == classid)
            पटिनिवत्त pci;
    }
    पटिनिवत्त रिक्त;
}

पूर्णाङ्क pci_init_device(ध्रुव संरचना pci_device_id *ids
                    , संरचना pci_device *pci, शून्य *arg)
{
    याव (ids->vendid || ids->class_mask) {
        सचे ((ids->vendid == PCI_ANY_ID || ids->vendid == pci->vendor) &&
            (ids->devid == PCI_ANY_ID || ids->devid == pci->device) &&
            !((ids->class ^ pci->class) & ids->class_mask)) {
            सचे (ids->func)
                ids->func(pci, arg);
            पटिनिवत्त 0;
        }
        ids++;
    }
    पटिनिवत्त -1;
}

संरचना pci_device *
pci_find_init_device(ध्रुव संरचना pci_device_id *ids, शून्य *arg)
{
    संरचना pci_device *pci;
    foreachpci(pci) {
        सचे (pci_init_device(ids, pci, arg) == 0)
            पटिनिवत्त pci;
    }
    पटिनिवत्त रिक्त;
}

// Enable PCI bus-mastering (ie, DMA) support on a pci device
शून्य
pci_enable_busmaster(संरचना pci_device *pci)
{
    wait_preempt();
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MASTER);
    pci->have_driver = 1;
}

// Verify an IO bar and return it to the caller
u16
pci_enable_iobar(संरचना pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    सचे (!(bar & PCI_BASE_ADDRESS_SPACE_IO)) {
        warn_internalerror();
        पटिनिवत्त 0;
    }
    bar &= PCI_BASE_ADDRESS_IO_MASK;
    सचे (bar == 0 || bar > 0xffff) {
        warn_internalerror();
        पटिनिवत्त 0;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_IO);
    pci->have_driver = 1;
    पटिनिवत्त bar;
}

// Verify a memory bar and return it to the caller
शून्य *
pci_enable_membar(संरचना pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    सचे (bar & PCI_BASE_ADDRESS_SPACE_IO) {
        warn_internalerror();
        पटिनिवत्त रिक्त;
    }
    सचे (bar & PCI_BASE_ADDRESS_MEM_TYPE_64) {
        u32 उच्चः = pci_config_readl(pci->bdf, addr+4);
        सचे (उच्चः) {
            dprintf(1, "Can not map memory bar over 4Gig\n");
            पटिनिवत्त रिक्त;
        }
    }
    bar &= PCI_BASE_ADDRESS_MEM_MASK;
    सचे (bar + 4*1024*1024 < 20*1024*1024) {
        // Bar doesn't look valid (it is in last 4M or first 16M)
        warn_internalerror();
        पटिनिवत्त रिक्त;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);
    pci->have_driver = 1;
    पटिनिवत्त (शून्य*)bar;
}

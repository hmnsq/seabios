// Code to maintain and access the pci_device cache
//
// Copyright (C) 2008-2016  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "malloc.h" // malloc_tmp
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "pci.h" // pci_config_writel
#அந்தர்பாவ "pcidevice.h" // pci_probe_devices
#அந்தர்பாவ "pci_regs.h" // PCI_VENDOR_ID
#அந்தர்பாவ "romfile.h" // romfile_loadint
#அந்தர்பாவ "stacks.h" // wait_preempt
#அந்தர்பாவ "string.h" // memset

கட்டமைப்பு hlist_head PCIDevices VARVERIFY32INIT;
முழு MaxPCIBus VARFSEG;

// Find all PCI devices and populate PCIDevices linked list.
வெறுமை
pci_probe_devices(வெறுமை)
{
    dprintf(3, "PCI probe\n");
    கட்டமைப்பு pci_device *busdevs[256];
    memset(busdevs, 0, அளவு(busdevs));
    கட்டமைப்பு hlist_முனை **pprev = &PCIDevices.first;
    முழு extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    முழு bus = -1, lastbus = 0, rootbuses = 0, மொத்தம்=0;
    போது (bus < 0xff && (bus < MaxPCIBus || rootbuses < extraroots)) {
        bus++;
        முழு bdf;
        foreachbdf(bdf, bus) {
            // Create new pci_device struct and add to list.
            கட்டமைப்பு pci_device *dev = malloc_tmp(அளவு(*dev));
            எனில் (!dev) {
                warn_noalloc();
                திரும்பு;
            }
            memset(dev, 0, அளவு(*dev));
            hlist_add(&dev->முனை, pprev);
            pprev = &dev->முனை.next;
            மொத்தம்++;

            // Find parent device.
            முழு rootbus;
            கட்டமைப்பு pci_device *parent = busdevs[bus];
            எனில் (!parent) {
                எனில் (bus != lastbus)
                    rootbuses++;
                lastbus = bus;
                rootbus = rootbuses;
                எனில் (bus > MaxPCIBus)
                    MaxPCIBus = bus;
            } இல்லையெனில் {
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
            எனில் (v == PCI_HEADER_TYPE_BRIDGE || v == PCI_HEADER_TYPE_CARDBUS) {
                u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
                dev->secondary_bus = secbus;
                எனில் (secbus > bus && !busdevs[secbus])
                    busdevs[secbus] = dev;
                எனில் (secbus > MaxPCIBus)
                    MaxPCIBus = secbus;
            }
            dprintf(4, "PCI device %pP (vd=%04x:%04x c=%04x)\n"
                    , dev, dev->vendor, dev->device, dev->class);
        }
    }
    dprintf(1, "Found %d PCI devices (max PCI bus is %02x)\n", மொத்தம், MaxPCIBus);
}

// Search for a device with the specified vendor and device ids.
கட்டமைப்பு pci_device *
pci_find_device(u16 vendid, u16 devid)
{
    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        எனில் (pci->vendor == vendid && pci->device == devid)
            திரும்பு pci;
    }
    திரும்பு வெற்று;
}

// Search for a device with the specified class id.
கட்டமைப்பு pci_device *
pci_find_class(u16 classid)
{
    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        எனில் (pci->class == classid)
            திரும்பு pci;
    }
    திரும்பு வெற்று;
}

முழு pci_init_device(மாறிலி கட்டமைப்பு pci_device_id *ids
                    , கட்டமைப்பு pci_device *pci, வெறுமை *arg)
{
    போது (ids->vendid || ids->class_mask) {
        எனில் ((ids->vendid == PCI_ANY_ID || ids->vendid == pci->vendor) &&
            (ids->devid == PCI_ANY_ID || ids->devid == pci->device) &&
            !((ids->class ^ pci->class) & ids->class_mask)) {
            எனில் (ids->func)
                ids->func(pci, arg);
            திரும்பு 0;
        }
        ids++;
    }
    திரும்பு -1;
}

கட்டமைப்பு pci_device *
pci_find_init_device(மாறிலி கட்டமைப்பு pci_device_id *ids, வெறுமை *arg)
{
    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        எனில் (pci_init_device(ids, pci, arg) == 0)
            திரும்பு pci;
    }
    திரும்பு வெற்று;
}

// Enable PCI bus-mastering (ie, DMA) support on a pci device
வெறுமை
pci_enable_busmaster(கட்டமைப்பு pci_device *pci)
{
    wait_preempt();
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MASTER);
    pci->have_driver = 1;
}

// Verify an IO bar and return it to the caller
u16
pci_enable_iobar(கட்டமைப்பு pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    எனில் (!(bar & PCI_BASE_ADDRESS_SPACE_IO)) {
        warn_internalerror();
        திரும்பு 0;
    }
    bar &= PCI_BASE_ADDRESS_IO_MASK;
    எனில் (bar == 0 || bar > 0xffff) {
        warn_internalerror();
        திரும்பு 0;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_IO);
    pci->have_driver = 1;
    திரும்பு bar;
}

// Verify a memory bar and return it to the caller
வெறுமை *
pci_enable_membar(கட்டமைப்பு pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    எனில் (bar & PCI_BASE_ADDRESS_SPACE_IO) {
        warn_internalerror();
        திரும்பு வெற்று;
    }
    எனில் (bar & PCI_BASE_ADDRESS_MEM_TYPE_64) {
        u32 உயர்வு = pci_config_readl(pci->bdf, addr+4);
        எனில் (உயர்வு) {
            dprintf(1, "Can not map memory bar over 4Gig\n");
            திரும்பு வெற்று;
        }
    }
    bar &= PCI_BASE_ADDRESS_MEM_MASK;
    எனில் (bar + 4*1024*1024 < 20*1024*1024) {
        // Bar doesn't look valid (it is in last 4M or first 16M)
        warn_internalerror();
        திரும்பு வெற்று;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);
    pci->have_driver = 1;
    திரும்பு (வெறுமை*)bar;
}

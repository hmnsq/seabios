// Code to maintain and access the pci_device cache
//
// Copyright (C) 2008-2016  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "malloc.h" // malloc_tmp
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "pci.h" // pci_config_writel
#ଅନ୍ତର୍ଭାଵ "pcidevice.h" // pci_probe_devices
#ଅନ୍ତର୍ଭାଵ "pci_regs.h" // PCI_VENDOR_ID
#ଅନ୍ତର୍ଭାଵ "romfile.h" // romfile_loadint
#ଅନ୍ତର୍ଭାଵ "stacks.h" // wait_preempt
#ଅନ୍ତର୍ଭାଵ "string.h" // memset

ସଂରଚନା hlist_head PCIDevices VARVERIFY32INIT;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା MaxPCIBus VARFSEG;

// Find all PCI devices and populate PCIDevices linked list.
ଶୂନ୍ୟ
pci_probe_devices(ଶୂନ୍ୟ)
{
    dprintf(3, "PCI probe\n");
    ସଂରଚନା pci_device *busdevs[256];
    memset(busdevs, 0, ଆକାର(busdevs));
    ସଂରଚନା hlist_ଗଣ୍ଠି **pprev = &PCIDevices.first;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bus = -1, lastbus = 0, rootbuses = 0, ଗଣନା_2=0;
    ଯେତେବେଳେ (bus < 0xff && (bus < MaxPCIBus || rootbuses < extraroots)) {
        bus++;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା bdf;
        foreachbdf(bdf, bus) {
            // Create new pci_device struct and add to list.
            ସଂରଚନା pci_device *dev = malloc_tmp(ଆକାର(*dev));
            ଯଦି (!dev) {
                warn_noalloc();
                ଫେରନ୍ତୁ;
            }
            memset(dev, 0, ଆକାର(*dev));
            hlist_add(&dev->ଗଣ୍ଠି, pprev);
            pprev = &dev->ଗଣ୍ଠି.next;
            ଗଣନା_2++;

            // Find parent device.
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା rootbus;
            ସଂରଚନା pci_device *parent = busdevs[bus];
            ଯଦି (!parent) {
                ଯଦି (bus != lastbus)
                    rootbuses++;
                lastbus = bus;
                rootbus = rootbuses;
                ଯଦି (bus > MaxPCIBus)
                    MaxPCIBus = bus;
            } ଅନ୍ୟଥା {
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
            ଯଦି (v == PCI_HEADER_TYPE_BRIDGE || v == PCI_HEADER_TYPE_CARDBUS) {
                u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
                dev->secondary_bus = secbus;
                ଯଦି (secbus > bus && !busdevs[secbus])
                    busdevs[secbus] = dev;
                ଯଦି (secbus > MaxPCIBus)
                    MaxPCIBus = secbus;
            }
            dprintf(4, "PCI device %pP (vd=%04x:%04x c=%04x)\n"
                    , dev, dev->vendor, dev->device, dev->class);
        }
    }
    dprintf(1, "Found %d PCI devices (max PCI bus is %02x)\n", ଗଣନା_2, MaxPCIBus);
}

// Search for a device with the specified vendor and device ids.
ସଂରଚନା pci_device *
pci_find_device(u16 vendid, u16 devid)
{
    ସଂରଚନା pci_device *pci;
    foreachpci(pci) {
        ଯଦି (pci->vendor == vendid && pci->device == devid)
            ଫେରନ୍ତୁ pci;
    }
    ଫେରନ୍ତୁ ଖାଲି;
}

// Search for a device with the specified class id.
ସଂରଚନା pci_device *
pci_find_class(u16 classid)
{
    ସଂରଚନା pci_device *pci;
    foreachpci(pci) {
        ଯଦି (pci->class == classid)
            ଫେରନ୍ତୁ pci;
    }
    ଫେରନ୍ତୁ ଖାଲି;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା pci_init_device(ଅଚଳ ସଂରଚନା pci_device_id *ids
                    , ସଂରଚନା pci_device *pci, ଶୂନ୍ୟ *arg)
{
    ଯେତେବେଳେ (ids->vendid || ids->class_mask) {
        ଯଦି ((ids->vendid == PCI_ANY_ID || ids->vendid == pci->vendor) &&
            (ids->devid == PCI_ANY_ID || ids->devid == pci->device) &&
            !((ids->class ^ pci->class) & ids->class_mask)) {
            ଯଦି (ids->func)
                ids->func(pci, arg);
            ଫେରନ୍ତୁ 0;
        }
        ids++;
    }
    ଫେରନ୍ତୁ -1;
}

ସଂରଚନା pci_device *
pci_find_init_device(ଅଚଳ ସଂରଚନା pci_device_id *ids, ଶୂନ୍ୟ *arg)
{
    ସଂରଚନା pci_device *pci;
    foreachpci(pci) {
        ଯଦି (pci_init_device(ids, pci, arg) == 0)
            ଫେରନ୍ତୁ pci;
    }
    ଫେରନ୍ତୁ ଖାଲି;
}

// Enable PCI bus-mastering (ie, DMA) support on a pci device
ଶୂନ୍ୟ
pci_enable_busmaster(ସଂରଚନା pci_device *pci)
{
    wait_preempt();
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MASTER);
    pci->have_driver = 1;
}

// Verify an IO bar and return it to the caller
u16
pci_enable_iobar(ସଂରଚନା pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    ଯଦି (!(bar & PCI_BASE_ADDRESS_SPACE_IO)) {
        warn_internalerror();
        ଫେରନ୍ତୁ 0;
    }
    bar &= PCI_BASE_ADDRESS_IO_MASK;
    ଯଦି (bar == 0 || bar > 0xffff) {
        warn_internalerror();
        ଫେରନ୍ତୁ 0;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_IO);
    pci->have_driver = 1;
    ଫେରନ୍ତୁ bar;
}

// Verify a memory bar and return it to the caller
ଶୂନ୍ୟ *
pci_enable_membar(ସଂରଚନା pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    ଯଦି (bar & PCI_BASE_ADDRESS_SPACE_IO) {
        warn_internalerror();
        ଫେରନ୍ତୁ ଖାଲି;
    }
    ଯଦି (bar & PCI_BASE_ADDRESS_MEM_TYPE_64) {
        u32 ଉପର = pci_config_readl(pci->bdf, addr+4);
        ଯଦି (ଉପର) {
            dprintf(1, "Can not map memory bar over 4Gig\n");
            ଫେରନ୍ତୁ ଖାଲି;
        }
    }
    bar &= PCI_BASE_ADDRESS_MEM_MASK;
    ଯଦି (bar + 4*1024*1024 < 20*1024*1024) {
        // Bar doesn't look valid (it is in last 4M or first 16M)
        warn_internalerror();
        ଫେରନ୍ତୁ ଖାଲି;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);
    pci->have_driver = 1;
    ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)bar;
}

// Code to maintain and access the pci_device cache
//
// Copyright (C) 2008-2016  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "malloc.h" // malloc_tmp
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pci.h" // pci_config_writel
#అన్తర్భావ "pcidevice.h" // pci_probe_devices
#అన్తర్భావ "pci_regs.h" // PCI_VENDOR_ID
#అన్తర్భావ "romfile.h" // romfile_loadint
#అన్తర్భావ "stacks.h" // wait_preempt
#అన్తర్భావ "string.h" // memset

నిర్మాణం hlist_తల PCIDevices VARVERIFY32INIT;
పూర్ణాంకం MaxPCIBus VARFSEG;

// Find all PCI devices and populate PCIDevices linked list.
శూన్య
pci_probe_devices(శూన్య)
{
    dprintf(3, "PCI probe\n");
    నిర్మాణం pci_device *busdevs[256];
    memset(busdevs, 0, పరిమాణం(busdevs));
    నిర్మాణం hlist_కణుపు **pprev = &PCIDevices.మొదటి;
    పూర్ణాంకం extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    పూర్ణాంకం bus = -1, lastbus = 0, rootbuses = 0, లెక్క=0;
    అయితే (bus < 0xff && (bus < MaxPCIBus || rootbuses < extraroots)) {
        bus++;
        పూర్ణాంకం bdf;
        foreachbdf(bdf, bus) {
            // Create new pci_device struct and add to list.
            నిర్మాణం pci_device *dev = malloc_tmp(పరిమాణం(*dev));
            ఒకవేళ (!dev) {
                warn_noalloc();
                తిరిగి;
            }
            memset(dev, 0, పరిమాణం(*dev));
            hlist_కూడు(&dev->కణుపు, pprev);
            pprev = &dev->కణుపు.తదుపరి;
            లెక్క++;

            // Find parent device.
            పూర్ణాంకం rootbus;
            నిర్మాణం pci_device *మాతృ = busdevs[bus];
            ఒకవేళ (!మాతృ) {
                ఒకవేళ (bus != lastbus)
                    rootbuses++;
                lastbus = bus;
                rootbus = rootbuses;
                ఒకవేళ (bus > MaxPCIBus)
                    MaxPCIBus = bus;
            } లేకపోతే {
                rootbus = మాతృ->rootbus;
            }

            // Populate pci_device info.
            dev->bdf = bdf;
            dev->మాతృ = మాతృ;
            dev->rootbus = rootbus;
            u32 vendev = pci_config_readl(bdf, PCI_VENDOR_ID);
            dev->vendor = vendev & 0xffff;
            dev->device = vendev >> 16;
            u32 classrev = pci_config_readl(bdf, PCI_CLASS_REVISION);
            dev->class = classrev >> 16;
            dev->prog_if = classrev >> 8;
            dev->revision = classrev & 0xff;
            dev->శీర్షిక_type = pci_config_readb(bdf, PCI_శీర్షిక_TYPE);
            u8 v = dev->శీర్షిక_type & 0x7f;
            ఒకవేళ (v == PCI_శీర్షిక_TYPE_BRIDGE || v == PCI_శీర్షిక_TYPE_CARDBUS) {
                u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
                dev->secondary_bus = secbus;
                ఒకవేళ (secbus > bus && !busdevs[secbus])
                    busdevs[secbus] = dev;
                ఒకవేళ (secbus > MaxPCIBus)
                    MaxPCIBus = secbus;
            }
            dprintf(4, "PCI device %pP (vd=%04x:%04x c=%04x)\n"
                    , dev, dev->vendor, dev->device, dev->class);
        }
    }
    dprintf(1, "Found %d PCI devices (max PCI bus is %02x)\n", లెక్క, MaxPCIBus);
}

// Search for a device with the specified vendor and device ids.
నిర్మాణం pci_device *
pci_కనుగొను_device(u16 vendid, u16 devid)
{
    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (pci->vendor == vendid && pci->device == devid)
            తిరిగి pci;
    }
    తిరిగి ఖాళీ;
}

// Search for a device with the specified class id.
నిర్మాణం pci_device *
pci_కనుగొను_class(u16 classid)
{
    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (pci->class == classid)
            తిరిగి pci;
    }
    తిరిగి ఖాళీ;
}

పూర్ణాంకం pci_init_device(స్థిరరాశి నిర్మాణం pci_device_id *ids
                    , నిర్మాణం pci_device *pci, శూన్య *arg)
{
    అయితే (ids->vendid || ids->class_ముసుగు) {
        ఒకవేళ ((ids->vendid == PCI_ANY_ID || ids->vendid == pci->vendor) &&
            (ids->devid == PCI_ANY_ID || ids->devid == pci->device) &&
            !((ids->class ^ pci->class) & ids->class_ముసుగు)) {
            ఒకవేళ (ids->func)
                ids->func(pci, arg);
            తిరిగి 0;
        }
        ids++;
    }
    తిరిగి -1;
}

నిర్మాణం pci_device *
pci_కనుగొను_init_device(స్థిరరాశి నిర్మాణం pci_device_id *ids, శూన్య *arg)
{
    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (pci_init_device(ids, pci, arg) == 0)
            తిరిగి pci;
    }
    తిరిగి ఖాళీ;
}

// Enable PCI bus-mastering (ie, DMA) support on a pci device
శూన్య
pci_enable_busmaster(నిర్మాణం pci_device *pci)
{
    wait_preempt();
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MASTER);
    pci->have_driver = 1;
}

// Verify an IO bar and return it to the caller
u16
pci_enable_iobar(నిర్మాణం pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    ఒకవేళ (!(bar & PCI_పునాది_చిరునామా_SPACE_IO)) {
        warn_internalerror();
        తిరిగి 0;
    }
    bar &= PCI_పునాది_చిరునామా_IO_ముసుగు;
    ఒకవేళ (bar == 0 || bar > 0xffff) {
        warn_internalerror();
        తిరిగి 0;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_IO);
    pci->have_driver = 1;
    తిరిగి bar;
}

// Verify a memory bar and return it to the caller
శూన్య *
pci_enable_membar(నిర్మాణం pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    ఒకవేళ (bar & PCI_పునాది_చిరునామా_SPACE_IO) {
        warn_internalerror();
        తిరిగి ఖాళీ;
    }
    ఒకవేళ (bar & PCI_పునాది_చిరునామా_MEM_TYPE_64) {
        u32 ఎగువ = pci_config_readl(pci->bdf, addr+4);
        ఒకవేళ (ఎగువ) {
            dprintf(1, "Can not map memory bar over 4Gig\n");
            తిరిగి ఖాళీ;
        }
    }
    bar &= PCI_పునాది_చిరునామా_MEM_ముసుగు;
    ఒకవేళ (bar + 4*1024*1024 < 20*1024*1024) {
        // Bar doesn't look valid (it is in last 4M or first 16M)
        warn_internalerror();
        తిరిగి ఖాళీ;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_జ్ఞాపకం);
    pci->have_driver = 1;
    తిరిగి (శూన్య*)bar;
}

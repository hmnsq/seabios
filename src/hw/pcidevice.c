// Code to maintain and access the pci_device cache
//
// Copyright (C) 2008-2016  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // malloc_tmp
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pci.h" // pci_config_writel
#ਅਨ੍ਤਰ੍ਭਾਵ "pcidevice.h" // pci_probe_devices
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_regs.h" // PCI_VENDOR_ID
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_loadint
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // wait_preempt
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset

ਬਣਤਰ hlist_ਸਿਰ PCIDevices VARVERIFY32INIT;
ਪੂਰਨ MaxPCIBus VARFSEG;

// Find all PCI devices and populate PCIDevices linked list.
ਖਾਲੀ
pci_probe_devices(ਖਾਲੀ)
{
    dprintf(3, "PCI probe\n");
    ਬਣਤਰ pci_device *busdevs[256];
    memset(busdevs, 0, ਆਕਾਰ(busdevs));
    ਬਣਤਰ hlist_ਗੰਢ **pprev = &PCIDevices.ਪਹਿਲਾ;
    ਪੂਰਨ extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    ਪੂਰਨ bus = -1, lastbus = 0, rootbuses = 0, ਗਿਣਤੀ_2=0;
    ਜਦੋਂ (bus < 0xff && (bus < MaxPCIBus || rootbuses < extraroots)) {
        bus++;
        ਪੂਰਨ bdf;
        foreachbdf(bdf, bus) {
            // Create new pci_device struct and add to list.
            ਬਣਤਰ pci_device *dev = malloc_tmp(ਆਕਾਰ(*dev));
            ਜੇ (!dev) {
                warn_noalloc();
                ਵਾਪਸ;
            }
            memset(dev, 0, ਆਕਾਰ(*dev));
            hlist_ਜੋੜੋ(&dev->ਗੰਢ, pprev);
            pprev = &dev->ਗੰਢ.ਅਗਲਾ;
            ਗਿਣਤੀ_2++;

            // Find parent device.
            ਪੂਰਨ rootbus;
            ਬਣਤਰ pci_device *ਮਾਪਾ = busdevs[bus];
            ਜੇ (!ਮਾਪਾ) {
                ਜੇ (bus != lastbus)
                    rootbuses++;
                lastbus = bus;
                rootbus = rootbuses;
                ਜੇ (bus > MaxPCIBus)
                    MaxPCIBus = bus;
            } ਨਹੀਂਤਾਂ {
                rootbus = ਮਾਪਾ->rootbus;
            }

            // Populate pci_device info.
            dev->bdf = bdf;
            dev->ਮਾਪਾ = ਮਾਪਾ;
            dev->rootbus = rootbus;
            u32 vendev = pci_config_readl(bdf, PCI_VENDOR_ID);
            dev->vendor = vendev & 0xffff;
            dev->device = vendev >> 16;
            u32 classrev = pci_config_readl(bdf, PCI_CLASS_REVISION);
            dev->class = classrev >> 16;
            dev->prog_if = classrev >> 8;
            dev->revision = classrev & 0xff;
            dev->ਸਿਰਲੇਖ_type = pci_config_readb(bdf, PCI_ਸਿਰਲੇਖ_TYPE);
            u8 v = dev->ਸਿਰਲੇਖ_type & 0x7f;
            ਜੇ (v == PCI_ਸਿਰਲੇਖ_TYPE_BRIDGE || v == PCI_ਸਿਰਲੇਖ_TYPE_CARDBUS) {
                u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
                dev->secondary_bus = secbus;
                ਜੇ (secbus > bus && !busdevs[secbus])
                    busdevs[secbus] = dev;
                ਜੇ (secbus > MaxPCIBus)
                    MaxPCIBus = secbus;
            }
            dprintf(4, "PCI device %pP (vd=%04x:%04x c=%04x)\n"
                    , dev, dev->vendor, dev->device, dev->class);
        }
    }
    dprintf(1, "Found %d PCI devices (max PCI bus is %02x)\n", ਗਿਣਤੀ_2, MaxPCIBus);
}

// Search for a device with the specified vendor and device ids.
ਬਣਤਰ pci_device *
pci_ਲੱਭੋ_device(u16 vendid, u16 devid)
{
    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (pci->vendor == vendid && pci->device == devid)
            ਵਾਪਸ pci;
    }
    ਵਾਪਸ ਨਿਰਾ;
}

// Search for a device with the specified class id.
ਬਣਤਰ pci_device *
pci_ਲੱਭੋ_class(u16 classid)
{
    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (pci->class == classid)
            ਵਾਪਸ pci;
    }
    ਵਾਪਸ ਨਿਰਾ;
}

ਪੂਰਨ pci_init_device(ਅਚਲ ਬਣਤਰ pci_device_id *ids
                    , ਬਣਤਰ pci_device *pci, ਖਾਲੀ *arg)
{
    ਜਦੋਂ (ids->vendid || ids->class_ਨਕਾਬ) {
        ਜੇ ((ids->vendid == PCI_ANY_ID || ids->vendid == pci->vendor) &&
            (ids->devid == PCI_ANY_ID || ids->devid == pci->device) &&
            !((ids->class ^ pci->class) & ids->class_ਨਕਾਬ)) {
            ਜੇ (ids->func)
                ids->func(pci, arg);
            ਵਾਪਸ 0;
        }
        ids++;
    }
    ਵਾਪਸ -1;
}

ਬਣਤਰ pci_device *
pci_ਲੱਭੋ_init_device(ਅਚਲ ਬਣਤਰ pci_device_id *ids, ਖਾਲੀ *arg)
{
    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (pci_init_device(ids, pci, arg) == 0)
            ਵਾਪਸ pci;
    }
    ਵਾਪਸ ਨਿਰਾ;
}

// Enable PCI bus-mastering (ie, DMA) support on a pci device
ਖਾਲੀ
pci_enable_busmaster(ਬਣਤਰ pci_device *pci)
{
    wait_preempt();
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MASTER);
    pci->have_driver = 1;
}

// Verify an IO bar and return it to the caller
u16
pci_enable_iobar(ਬਣਤਰ pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    ਜੇ (!(bar & PCI_ਅਧਾਰ_ਪਤਾ_SPACE_IO)) {
        warn_internalerror();
        ਵਾਪਸ 0;
    }
    bar &= PCI_ਅਧਾਰ_ਪਤਾ_IO_ਨਕਾਬ;
    ਜੇ (bar == 0 || bar > 0xffff) {
        warn_internalerror();
        ਵਾਪਸ 0;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_IO);
    pci->have_driver = 1;
    ਵਾਪਸ bar;
}

// Verify a memory bar and return it to the caller
ਖਾਲੀ *
pci_enable_membar(ਬਣਤਰ pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    ਜੇ (bar & PCI_ਅਧਾਰ_ਪਤਾ_SPACE_IO) {
        warn_internalerror();
        ਵਾਪਸ ਨਿਰਾ;
    }
    ਜੇ (bar & PCI_ਅਧਾਰ_ਪਤਾ_MEM_TYPE_64) {
        u32 ਉੱਚਾ = pci_config_readl(pci->bdf, addr+4);
        ਜੇ (ਉੱਚਾ) {
            dprintf(1, "Can not map memory bar over 4Gig\n");
            ਵਾਪਸ ਨਿਰਾ;
        }
    }
    bar &= PCI_ਅਧਾਰ_ਪਤਾ_MEM_ਨਕਾਬ;
    ਜੇ (bar + 4*1024*1024 < 20*1024*1024) {
        // Bar doesn't look valid (it is in last 4M or first 16M)
        warn_internalerror();
        ਵਾਪਸ ਨਿਰਾ;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_ਸਮ੍ਰਿਤੀ);
    pci->have_driver = 1;
    ਵਾਪਸ (ਖਾਲੀ*)bar;
}

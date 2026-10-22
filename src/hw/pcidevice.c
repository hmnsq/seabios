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

ساخت hlist_سر PCIDevices VARVERIFY32INIT;
صحیح MaxPCIBus VARFSEG;

// Find all PCI devices and populate PCIDevices linked list.
خالی
pci_probe_devices(خالی)
{
    dprintf(3, "PCI probe\n");
    ساخت pci_device *busdevs[256];
    memset(busdevs, 0, حجم(busdevs));
    ساخت hlist_گرہ **pprev = &PCIDevices.پہلا;
    صحیح extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    صحیح bus = -1, lastbus = 0, rootbuses = 0, گنتی=0;
    جب (bus < 0xff && (bus < MaxPCIBus || rootbuses < extraroots)) {
        bus++;
        صحیح bdf;
        foreachbdf(bdf, bus) {
            // Create new pci_device struct and add to list.
            ساخت pci_device *dev = malloc_tmp(حجم(*dev));
            اگر (!dev) {
                warn_noalloc();
                واپس;
            }
            memset(dev, 0, حجم(*dev));
            hlist_جوڑو(&dev->گرہ, pprev);
            pprev = &dev->گرہ.اگلا;
            گنتی++;

            // Find parent device.
            صحیح rootbus;
            ساخت pci_device *والد = busdevs[bus];
            اگر (!والد) {
                اگر (bus != lastbus)
                    rootbuses++;
                lastbus = bus;
                rootbus = rootbuses;
                اگر (bus > MaxPCIBus)
                    MaxPCIBus = bus;
            } ورنہ {
                rootbus = والد->rootbus;
            }

            // Populate pci_device info.
            dev->bdf = bdf;
            dev->والد = والد;
            dev->rootbus = rootbus;
            u32 vendev = pci_config_readl(bdf, PCI_VENDOR_ID);
            dev->vendor = vendev & 0xffff;
            dev->device = vendev >> 16;
            u32 classrev = pci_config_readl(bdf, PCI_CLASS_REVISION);
            dev->class = classrev >> 16;
            dev->prog_if = classrev >> 8;
            dev->revision = classrev & 0xff;
            dev->سرخی_type = pci_config_readb(bdf, PCI_سرخی_TYPE);
            u8 v = dev->سرخی_type & 0x7f;
            اگر (v == PCI_سرخی_TYPE_BRIDGE || v == PCI_سرخی_TYPE_CARDBUS) {
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
    dprintf(1, "Found %d PCI devices (max PCI bus is %02x)\n", گنتی, MaxPCIBus);
}

// Search for a device with the specified vendor and device ids.
ساخت pci_device *
pci_ڈھونڈو_device(u16 vendid, u16 devid)
{
    ساخت pci_device *pci;
    foreachpci(pci) {
        اگر (pci->vendor == vendid && pci->device == devid)
            واپس pci;
    }
    واپس تہی;
}

// Search for a device with the specified class id.
ساخت pci_device *
pci_ڈھونڈو_class(u16 classid)
{
    ساخت pci_device *pci;
    foreachpci(pci) {
        اگر (pci->class == classid)
            واپس pci;
    }
    واپس تہی;
}

صحیح pci_init_device(ثابت ساخت pci_device_id *ids
                    , ساخت pci_device *pci, خالی *arg)
{
    جب (ids->vendid || ids->class_نقاب) {
        اگر ((ids->vendid == PCI_ANY_ID || ids->vendid == pci->vendor) &&
            (ids->devid == PCI_ANY_ID || ids->devid == pci->device) &&
            !((ids->class ^ pci->class) & ids->class_نقاب)) {
            اگر (ids->func)
                ids->func(pci, arg);
            واپس 0;
        }
        ids++;
    }
    واپس -1;
}

ساخت pci_device *
pci_ڈھونڈو_init_device(ثابت ساخت pci_device_id *ids, خالی *arg)
{
    ساخت pci_device *pci;
    foreachpci(pci) {
        اگر (pci_init_device(ids, pci, arg) == 0)
            واپس pci;
    }
    واپس تہی;
}

// Enable PCI bus-mastering (ie, DMA) support on a pci device
خالی
pci_enable_busmaster(ساخت pci_device *pci)
{
    wait_preempt();
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MASTER);
    pci->have_driver = 1;
}

// Verify an IO bar and return it to the caller
u16
pci_enable_iobar(ساخت pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    اگر (!(bar & PCI_بنیاد_پتہ_SPACE_IO)) {
        warn_internalerror();
        واپس 0;
    }
    bar &= PCI_بنیاد_پتہ_IO_نقاب;
    اگر (bar == 0 || bar > 0xffff) {
        warn_internalerror();
        واپس 0;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_IO);
    pci->have_driver = 1;
    واپس bar;
}

// Verify a memory bar and return it to the caller
خالی *
pci_enable_membar(ساخت pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    اگر (bar & PCI_بنیاد_پتہ_SPACE_IO) {
        warn_internalerror();
        واپس تہی;
    }
    اگر (bar & PCI_بنیاد_پتہ_MEM_TYPE_64) {
        u32 بلند = pci_config_readl(pci->bdf, addr+4);
        اگر (بلند) {
            dprintf(1, "Can not map memory bar over 4Gig\n");
            واپس تہی;
        }
    }
    bar &= PCI_بنیاد_پتہ_MEM_نقاب;
    اگر (bar + 4*1024*1024 < 20*1024*1024) {
        // Bar doesn't look valid (it is in last 4M or first 16M)
        warn_internalerror();
        واپس تہی;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_حافظہ);
    pci->have_driver = 1;
    واپس (خالی*)bar;
}

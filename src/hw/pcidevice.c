// Code to maintain and access the pci_device cache
//
// Copyright (C) 2008-2016  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "malloc.h" // malloc_tmp
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "pci.h" // pci_config_writel
#અન્તર્ભાવ "pcidevice.h" // pci_probe_devices
#અન્તર્ભાવ "pci_regs.h" // PCI_VENDOR_ID
#અન્તર્ભાવ "romfile.h" // romfile_loadint
#અન્તર્ભાવ "stacks.h" // wait_preempt
#અન્તર્ભાવ "string.h" // memset

સંરચના hlist_head PCIDevices VARVERIFY32INIT;
પૂર્ણાંક MaxPCIBus VARFSEG;

// Find all PCI devices and populate PCIDevices linked list.
શૂન્ય
pci_probe_devices(શૂન્ય)
{
    dprintf(3, "PCI probe\n");
    સંરચના pci_device *busdevs[256];
    memset(busdevs, 0, કદ(busdevs));
    સંરચના hlist_ગાંઠ **pprev = &PCIDevices.first;
    પૂર્ણાંક extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    પૂર્ણાંક bus = -1, lastbus = 0, rootbuses = 0, ગણતરી=0;
    જ્યારે (bus < 0xff && (bus < MaxPCIBus || rootbuses < extraroots)) {
        bus++;
        પૂર્ણાંક bdf;
        foreachbdf(bdf, bus) {
            // Create new pci_device struct and add to list.
            સંરચના pci_device *dev = malloc_tmp(કદ(*dev));
            જો (!dev) {
                warn_noalloc();
                પાછા;
            }
            memset(dev, 0, કદ(*dev));
            hlist_add(&dev->ગાંઠ, pprev);
            pprev = &dev->ગાંઠ.next;
            ગણતરી++;

            // Find parent device.
            પૂર્ણાંક rootbus;
            સંરચના pci_device *parent = busdevs[bus];
            જો (!parent) {
                જો (bus != lastbus)
                    rootbuses++;
                lastbus = bus;
                rootbus = rootbuses;
                જો (bus > MaxPCIBus)
                    MaxPCIBus = bus;
            } નહીંતર {
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
            જો (v == PCI_HEADER_TYPE_BRIDGE || v == PCI_HEADER_TYPE_CARDBUS) {
                u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
                dev->secondary_bus = secbus;
                જો (secbus > bus && !busdevs[secbus])
                    busdevs[secbus] = dev;
                જો (secbus > MaxPCIBus)
                    MaxPCIBus = secbus;
            }
            dprintf(4, "PCI device %pP (vd=%04x:%04x c=%04x)\n"
                    , dev, dev->vendor, dev->device, dev->class);
        }
    }
    dprintf(1, "Found %d PCI devices (max PCI bus is %02x)\n", ગણતરી, MaxPCIBus);
}

// Search for a device with the specified vendor and device ids.
સંરચના pci_device *
pci_find_device(u16 vendid, u16 devid)
{
    સંરચના pci_device *pci;
    foreachpci(pci) {
        જો (pci->vendor == vendid && pci->device == devid)
            પાછા pci;
    }
    પાછા ખાલી;
}

// Search for a device with the specified class id.
સંરચના pci_device *
pci_find_class(u16 classid)
{
    સંરચના pci_device *pci;
    foreachpci(pci) {
        જો (pci->class == classid)
            પાછા pci;
    }
    પાછા ખાલી;
}

પૂર્ણાંક pci_init_device(અચલ સંરચના pci_device_id *ids
                    , સંરચના pci_device *pci, શૂન્ય *arg)
{
    જ્યારે (ids->vendid || ids->class_mask) {
        જો ((ids->vendid == PCI_ANY_ID || ids->vendid == pci->vendor) &&
            (ids->devid == PCI_ANY_ID || ids->devid == pci->device) &&
            !((ids->class ^ pci->class) & ids->class_mask)) {
            જો (ids->func)
                ids->func(pci, arg);
            પાછા 0;
        }
        ids++;
    }
    પાછા -1;
}

સંરચના pci_device *
pci_find_init_device(અચલ સંરચના pci_device_id *ids, શૂન્ય *arg)
{
    સંરચના pci_device *pci;
    foreachpci(pci) {
        જો (pci_init_device(ids, pci, arg) == 0)
            પાછા pci;
    }
    પાછા ખાલી;
}

// Enable PCI bus-mastering (ie, DMA) support on a pci device
શૂન્ય
pci_enable_busmaster(સંરચના pci_device *pci)
{
    wait_preempt();
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MASTER);
    pci->have_driver = 1;
}

// Verify an IO bar and return it to the caller
u16
pci_enable_iobar(સંરચના pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    જો (!(bar & PCI_BASE_ADDRESS_SPACE_IO)) {
        warn_internalerror();
        પાછા 0;
    }
    bar &= PCI_BASE_ADDRESS_IO_MASK;
    જો (bar == 0 || bar > 0xffff) {
        warn_internalerror();
        પાછા 0;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_IO);
    pci->have_driver = 1;
    પાછા bar;
}

// Verify a memory bar and return it to the caller
શૂન્ય *
pci_enable_membar(સંરચના pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    જો (bar & PCI_BASE_ADDRESS_SPACE_IO) {
        warn_internalerror();
        પાછા ખાલી;
    }
    જો (bar & PCI_BASE_ADDRESS_MEM_TYPE_64) {
        u32 ઉપલું = pci_config_readl(pci->bdf, addr+4);
        જો (ઉપલું) {
            dprintf(1, "Can not map memory bar over 4Gig\n");
            પાછા ખાલી;
        }
    }
    bar &= PCI_BASE_ADDRESS_MEM_MASK;
    જો (bar + 4*1024*1024 < 20*1024*1024) {
        // Bar doesn't look valid (it is in last 4M or first 16M)
        warn_internalerror();
        પાછા ખાલી;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);
    pci->have_driver = 1;
    પાછા (શૂન્ય*)bar;
}

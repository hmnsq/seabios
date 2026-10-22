// Code to maintain and access the pci_device cache
//
// Copyright (C) 2008-2016  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "malloc.h" // malloc_tmp
#ضمن "output.h" // dprintf
#ضمن "pci.h" // pci_config_writel
#ضمن "pcidevice.h" // pci_probe_devices
#ضمن "pci_regs.h" // PCI_VENDOR_ID
#ضمن "romfile.h" // romfile_loadint
#ضمن "stacks.h" // wait_preempt
#ضمن "string.h" // memset

بنية hlist_head PCIDevices VARVERIFY32INIT;
صحيح MaxPCIBus VARFSEG;

// Find all PCI devices and populate PCIDevices linked list.
فراغ
pci_probe_devices(فراغ)
{
    dprintf(3, "PCI probe\n");
    بنية pci_device *busdevs[256];
    memset(busdevs, 0, حجم(busdevs));
    بنية hlist_node **pprev = &PCIDevices.first;
    صحيح extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    صحيح bus = -1, lastbus = 0, rootbuses = 0, count=0;
    طالما (bus < 0xff && (bus < MaxPCIBus || rootbuses < extraroots)) {
        bus++;
        صحيح bdf;
        foreachbdf(bdf, bus) {
            // Create new pci_device struct and add to list.
            بنية pci_device *dev = malloc_tmp(حجم(*dev));
            𐡄𐡍 (!dev) {
                warn_noalloc();
                أرجع;
            }
            memset(dev, 0, حجم(*dev));
            hlist_add(&dev->node, pprev);
            pprev = &dev->node.next;
            count++;

            // Find parent device.
            صحيح rootbus;
            بنية pci_device *parent = busdevs[bus];
            𐡄𐡍 (!parent) {
                𐡄𐡍 (bus != lastbus)
                    rootbuses++;
                lastbus = bus;
                rootbus = rootbuses;
                𐡄𐡍 (bus > MaxPCIBus)
                    MaxPCIBus = bus;
            } وإلا {
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
            𐡄𐡍 (v == PCI_HEADER_TYPE_BRIDGE || v == PCI_HEADER_TYPE_CARDBUS) {
                u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
                dev->secondary_bus = secbus;
                𐡄𐡍 (secbus > bus && !busdevs[secbus])
                    busdevs[secbus] = dev;
                𐡄𐡍 (secbus > MaxPCIBus)
                    MaxPCIBus = secbus;
            }
            dprintf(4, "PCI device %pP (vd=%04x:%04x c=%04x)\n"
                    , dev, dev->vendor, dev->device, dev->class);
        }
    }
    dprintf(1, "Found %d PCI devices (max PCI bus is %02x)\n", count, MaxPCIBus);
}

// Search for a device with the specified vendor and device ids.
بنية pci_device *
pci_find_device(u16 vendid, u16 devid)
{
    بنية pci_device *pci;
    foreachpci(pci) {
        𐡄𐡍 (pci->vendor == vendid && pci->device == devid)
            أرجع pci;
    }
    أرجع عدم;
}

// Search for a device with the specified class id.
بنية pci_device *
pci_find_class(u16 classid)
{
    بنية pci_device *pci;
    foreachpci(pci) {
        𐡄𐡍 (pci->class == classid)
            أرجع pci;
    }
    أرجع عدم;
}

صحيح pci_init_device(ثابت بنية pci_device_id *ids
                    , بنية pci_device *pci, فراغ *arg)
{
    طالما (ids->vendid || ids->class_mask) {
        𐡄𐡍 ((ids->vendid == PCI_ANY_ID || ids->vendid == pci->vendor) &&
            (ids->devid == PCI_ANY_ID || ids->devid == pci->device) &&
            !((ids->class ^ pci->class) & ids->class_mask)) {
            𐡄𐡍 (ids->func)
                ids->func(pci, arg);
            أرجع 0;
        }
        ids++;
    }
    أرجع -1;
}

بنية pci_device *
pci_find_init_device(ثابت بنية pci_device_id *ids, فراغ *arg)
{
    بنية pci_device *pci;
    foreachpci(pci) {
        𐡄𐡍 (pci_init_device(ids, pci, arg) == 0)
            أرجع pci;
    }
    أرجع عدم;
}

// Enable PCI bus-mastering (ie, DMA) support on a pci device
فراغ
pci_enable_busmaster(بنية pci_device *pci)
{
    wait_preempt();
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MASTER);
    pci->have_driver = 1;
}

// Verify an IO bar and return it to the caller
u16
pci_enable_iobar(بنية pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    𐡄𐡍 (!(bar & PCI_BASE_ADDRESS_SPACE_IO)) {
        warn_internalerror();
        أرجع 0;
    }
    bar &= PCI_BASE_ADDRESS_IO_MASK;
    𐡄𐡍 (bar == 0 || bar > 0xffff) {
        warn_internalerror();
        أرجع 0;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_IO);
    pci->have_driver = 1;
    أرجع bar;
}

// Verify a memory bar and return it to the caller
فراغ *
pci_enable_membar(بنية pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    𐡄𐡍 (bar & PCI_BASE_ADDRESS_SPACE_IO) {
        warn_internalerror();
        أرجع عدم;
    }
    𐡄𐡍 (bar & PCI_BASE_ADDRESS_MEM_TYPE_64) {
        u32 high = pci_config_readl(pci->bdf, addr+4);
        𐡄𐡍 (high) {
            dprintf(1, "Can not map memory bar over 4Gig\n");
            أرجع عدم;
        }
    }
    bar &= PCI_BASE_ADDRESS_MEM_MASK;
    𐡄𐡍 (bar + 4*1024*1024 < 20*1024*1024) {
        // Bar doesn't look valid (it is in last 4M or first 16M)
        warn_internalerror();
        أرجع عدم;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);
    pci->have_driver = 1;
    أرجع (فراغ*)bar;
}

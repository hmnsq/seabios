// Code to maintain and access the pci_device cache
//
// Copyright (C) 2008-2016  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "malloc.h" // malloc_tmp
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "pci.h" // pci_config_writel
#അന്തര്ഭാവ "pcidevice.h" // pci_probe_devices
#അന്തര്ഭാവ "pci_regs.h" // PCI_VENDOR_ID
#അന്തര്ഭാവ "romfile.h" // romfile_loadint
#അന്തര്ഭാവ "stacks.h" // wait_preempt
#അന്തര്ഭാവ "string.h" // memset

ഘടന hlist_head PCIDevices VARVERIFY32INIT;
പൂർണ്ണസംഖ്യ MaxPCIBus VARFSEG;

// Find all PCI devices and populate PCIDevices linked list.
ശൂന്യം
pci_probe_devices(ശൂന്യം)
{
    dprintf(3, "PCI probe\n");
    ഘടന pci_device *busdevs[256];
    memset(busdevs, 0, വലുപ്പം(busdevs));
    ഘടന hlist_കണ്ണി **pprev = &PCIDevices.first;
    പൂർണ്ണസംഖ്യ extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    പൂർണ്ണസംഖ്യ bus = -1, lastbus = 0, rootbuses = 0, എണ്ണം=0;
    ആകുമ്പോൾ (bus < 0xff && (bus < MaxPCIBus || rootbuses < extraroots)) {
        bus++;
        പൂർണ്ണസംഖ്യ bdf;
        foreachbdf(bdf, bus) {
            // Create new pci_device struct and add to list.
            ഘടന pci_device *dev = malloc_tmp(വലുപ്പം(*dev));
            എങ്കിൽ (!dev) {
                warn_noalloc();
                തിരികെ;
            }
            memset(dev, 0, വലുപ്പം(*dev));
            hlist_add(&dev->കണ്ണി, pprev);
            pprev = &dev->കണ്ണി.next;
            എണ്ണം++;

            // Find parent device.
            പൂർണ്ണസംഖ്യ rootbus;
            ഘടന pci_device *parent = busdevs[bus];
            എങ്കിൽ (!parent) {
                എങ്കിൽ (bus != lastbus)
                    rootbuses++;
                lastbus = bus;
                rootbus = rootbuses;
                എങ്കിൽ (bus > MaxPCIBus)
                    MaxPCIBus = bus;
            } അല്ലെങ്കിൽ {
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
            എങ്കിൽ (v == PCI_HEADER_TYPE_BRIDGE || v == PCI_HEADER_TYPE_CARDBUS) {
                u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
                dev->secondary_bus = secbus;
                എങ്കിൽ (secbus > bus && !busdevs[secbus])
                    busdevs[secbus] = dev;
                എങ്കിൽ (secbus > MaxPCIBus)
                    MaxPCIBus = secbus;
            }
            dprintf(4, "PCI device %pP (vd=%04x:%04x c=%04x)\n"
                    , dev, dev->vendor, dev->device, dev->class);
        }
    }
    dprintf(1, "Found %d PCI devices (max PCI bus is %02x)\n", എണ്ണം, MaxPCIBus);
}

// Search for a device with the specified vendor and device ids.
ഘടന pci_device *
pci_find_device(u16 vendid, u16 devid)
{
    ഘടന pci_device *pci;
    foreachpci(pci) {
        എങ്കിൽ (pci->vendor == vendid && pci->device == devid)
            തിരികെ pci;
    }
    തിരികെ ശൂന്യ;
}

// Search for a device with the specified class id.
ഘടന pci_device *
pci_find_class(u16 classid)
{
    ഘടന pci_device *pci;
    foreachpci(pci) {
        എങ്കിൽ (pci->class == classid)
            തിരികെ pci;
    }
    തിരികെ ശൂന്യ;
}

പൂർണ്ണസംഖ്യ pci_init_device(സ്ഥിരം ഘടന pci_device_id *ids
                    , ഘടന pci_device *pci, ശൂന്യം *arg)
{
    ആകുമ്പോൾ (ids->vendid || ids->class_mask) {
        എങ്കിൽ ((ids->vendid == PCI_ANY_ID || ids->vendid == pci->vendor) &&
            (ids->devid == PCI_ANY_ID || ids->devid == pci->device) &&
            !((ids->class ^ pci->class) & ids->class_mask)) {
            എങ്കിൽ (ids->func)
                ids->func(pci, arg);
            തിരികെ 0;
        }
        ids++;
    }
    തിരികെ -1;
}

ഘടന pci_device *
pci_find_init_device(സ്ഥിരം ഘടന pci_device_id *ids, ശൂന്യം *arg)
{
    ഘടന pci_device *pci;
    foreachpci(pci) {
        എങ്കിൽ (pci_init_device(ids, pci, arg) == 0)
            തിരികെ pci;
    }
    തിരികെ ശൂന്യ;
}

// Enable PCI bus-mastering (ie, DMA) support on a pci device
ശൂന്യം
pci_enable_busmaster(ഘടന pci_device *pci)
{
    wait_preempt();
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MASTER);
    pci->have_driver = 1;
}

// Verify an IO bar and return it to the caller
u16
pci_enable_iobar(ഘടന pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    എങ്കിൽ (!(bar & PCI_BASE_ADDRESS_SPACE_IO)) {
        warn_internalerror();
        തിരികെ 0;
    }
    bar &= PCI_BASE_ADDRESS_IO_MASK;
    എങ്കിൽ (bar == 0 || bar > 0xffff) {
        warn_internalerror();
        തിരികെ 0;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_IO);
    pci->have_driver = 1;
    തിരികെ bar;
}

// Verify a memory bar and return it to the caller
ശൂന്യം *
pci_enable_membar(ഘടന pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    എങ്കിൽ (bar & PCI_BASE_ADDRESS_SPACE_IO) {
        warn_internalerror();
        തിരികെ ശൂന്യ;
    }
    എങ്കിൽ (bar & PCI_BASE_ADDRESS_MEM_TYPE_64) {
        u32 മുകളിലെ = pci_config_readl(pci->bdf, addr+4);
        എങ്കിൽ (മുകളിലെ) {
            dprintf(1, "Can not map memory bar over 4Gig\n");
            തിരികെ ശൂന്യ;
        }
    }
    bar &= PCI_BASE_ADDRESS_MEM_MASK;
    എങ്കിൽ (bar + 4*1024*1024 < 20*1024*1024) {
        // Bar doesn't look valid (it is in last 4M or first 16M)
        warn_internalerror();
        തിരികെ ശൂന്യ;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);
    pci->have_driver = 1;
    തിരികെ (ശൂന്യം*)bar;
}

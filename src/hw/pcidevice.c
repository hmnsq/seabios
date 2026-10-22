// Code to maintain and access the pci_device cache
//
// Copyright (C) 2008-2016  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "malloc.h" // malloc_tmp
#समावेश "output.h" // dprintf
#समावेश "pci.h" // pci_config_writel
#समावेश "pcidevice.h" // pci_probe_devices
#समावेश "pci_regs.h" // PCI_VENDOR_ID
#समावेश "romfile.h" // romfile_loadint
#समावेश "stacks.h" // wait_preempt
#समावेश "string.h" // memset

काष्ठा hlist_शीर्ष PCIDevices VARVERIFY32INIT;
पूर्णांक MaxPCIBus VARFSEG;

// Find all PCI devices and populate PCIDevices linked list.
व्योम
pci_probe_devices(व्योम)
{
    dprintf(3, "PCI probe\n");
    काष्ठा pci_device *busdevs[256];
    स_रखो(busdevs, 0, माप(busdevs));
    काष्ठा hlist_गाँठ **pprev = &PCIDevices.first;
    पूर्णांक extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    पूर्णांक bus = -1, lastbus = 0, rootbuses = 0, गिनती=0;
    जबतक (bus < 0xff && (bus < MaxPCIBus || rootbuses < extraroots)) {
        bus++;
        पूर्णांक bdf;
        foreachbdf(bdf, bus) {
            // Create new pci_device struct and add to list.
            काष्ठा pci_device *dev = malloc_tmp(माप(*dev));
            अगर (!dev) {
                warn_noalloc();
                वापस;
            }
            स_रखो(dev, 0, माप(*dev));
            hlist_add(&dev->गाँठ, pprev);
            pprev = &dev->गाँठ.अग्र;
            गिनती++;

            // Find parent device.
            पूर्णांक rootbus;
            काष्ठा pci_device *parent = busdevs[bus];
            अगर (!parent) {
                अगर (bus != lastbus)
                    rootbuses++;
                lastbus = bus;
                rootbus = rootbuses;
                अगर (bus > MaxPCIBus)
                    MaxPCIBus = bus;
            } अन्यथा {
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
            अगर (v == PCI_HEADER_TYPE_BRIDGE || v == PCI_HEADER_TYPE_CARDBUS) {
                u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
                dev->secondary_bus = secbus;
                अगर (secbus > bus && !busdevs[secbus])
                    busdevs[secbus] = dev;
                अगर (secbus > MaxPCIBus)
                    MaxPCIBus = secbus;
            }
            dprintf(4, "PCI device %pP (vd=%04x:%04x c=%04x)\n"
                    , dev, dev->vendor, dev->device, dev->class);
        }
    }
    dprintf(1, "Found %d PCI devices (max PCI bus is %02x)\n", गिनती, MaxPCIBus);
}

// Search for a device with the specified vendor and device ids.
काष्ठा pci_device *
pci_find_device(u16 vendid, u16 devid)
{
    काष्ठा pci_device *pci;
    foreachpci(pci) {
        अगर (pci->vendor == vendid && pci->device == devid)
            वापस pci;
    }
    वापस शून्य;
}

// Search for a device with the specified class id.
काष्ठा pci_device *
pci_find_class(u16 classid)
{
    काष्ठा pci_device *pci;
    foreachpci(pci) {
        अगर (pci->class == classid)
            वापस pci;
    }
    वापस शून्य;
}

पूर्णांक pci_init_device(स्थिर काष्ठा pci_device_id *ids
                    , काष्ठा pci_device *pci, व्योम *arg)
{
    जबतक (ids->vendid || ids->class_mask) {
        अगर ((ids->vendid == PCI_ANY_ID || ids->vendid == pci->vendor) &&
            (ids->devid == PCI_ANY_ID || ids->devid == pci->device) &&
            !((ids->class ^ pci->class) & ids->class_mask)) {
            अगर (ids->func)
                ids->func(pci, arg);
            वापस 0;
        }
        ids++;
    }
    वापस -1;
}

काष्ठा pci_device *
pci_find_init_device(स्थिर काष्ठा pci_device_id *ids, व्योम *arg)
{
    काष्ठा pci_device *pci;
    foreachpci(pci) {
        अगर (pci_init_device(ids, pci, arg) == 0)
            वापस pci;
    }
    वापस शून्य;
}

// Enable PCI bus-mastering (ie, DMA) support on a pci device
व्योम
pci_enable_busmaster(काष्ठा pci_device *pci)
{
    wait_preempt();
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MASTER);
    pci->have_driver = 1;
}

// Verify an IO bar and return it to the caller
u16
pci_enable_iobar(काष्ठा pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    अगर (!(bar & PCI_BASE_ADDRESS_SPACE_IO)) {
        warn_internalerror();
        वापस 0;
    }
    bar &= PCI_BASE_ADDRESS_IO_MASK;
    अगर (bar == 0 || bar > 0xffff) {
        warn_internalerror();
        वापस 0;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_IO);
    pci->have_driver = 1;
    वापस bar;
}

// Verify a memory bar and return it to the caller
व्योम *
pci_enable_membar(काष्ठा pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    अगर (bar & PCI_BASE_ADDRESS_SPACE_IO) {
        warn_internalerror();
        वापस शून्य;
    }
    अगर (bar & PCI_BASE_ADDRESS_MEM_TYPE_64) {
        u32 ऊपरी = pci_config_readl(pci->bdf, addr+4);
        अगर (ऊपरी) {
            dprintf(1, "Can not map memory bar over 4Gig\n");
            वापस शून्य;
        }
    }
    bar &= PCI_BASE_ADDRESS_MEM_MASK;
    अगर (bar + 4*1024*1024 < 20*1024*1024) {
        // Bar doesn't look valid (it is in last 4M or first 16M)
        warn_internalerror();
        वापस शून्य;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);
    pci->have_driver = 1;
    वापस (व्योम*)bar;
}

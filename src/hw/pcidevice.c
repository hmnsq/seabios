// Code to maintain and access the pci_device cache
//
// Copyright (C) 2008-2016  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "malloc.h" // malloc_tmp
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pci.h" // pci_config_writel
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // pci_probe_devices
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_VENDOR_ID
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_loadint
#ಅನ್ತರ್ಭಾವ "stacks.h" // wait_preempt
#ಅನ್ತರ್ಭಾವ "string.h" // memset

ರಚನೆ hlist_ತಲೆ PCIDevices VARVERIFY32INIT;
ಪೂರ್ಣಾಂಕ MaxPCIBus VARFSEG;

// Find all PCI devices and populate PCIDevices linked list.
ಶೂನ್ಯ
pci_probe_devices(ಶೂನ್ಯ)
{
    dprintf(3, "PCI probe\n");
    ರಚನೆ pci_device *busdevs[256];
    memset(busdevs, 0, ಗಾತ್ರ(busdevs));
    ರಚನೆ hlist_ಗಂಟು **pprev = &PCIDevices.ಮೊದಲ;
    ಪೂರ್ಣಾಂಕ extraroots = romfile_loadint("etc/extra-pci-roots", 0);
    ಪೂರ್ಣಾಂಕ bus = -1, lastbus = 0, rootbuses = 0, ಎಣಿಕೆ_2=0;
    ಆಗ (bus < 0xff && (bus < MaxPCIBus || rootbuses < extraroots)) {
        bus++;
        ಪೂರ್ಣಾಂಕ bdf;
        foreachbdf(bdf, bus) {
            // Create new pci_device struct and add to list.
            ರಚನೆ pci_device *dev = malloc_tmp(ಗಾತ್ರ(*dev));
            ಆದರೆ (!dev) {
                warn_noalloc();
                ಹಿಂತಿರುಗಿ;
            }
            memset(dev, 0, ಗಾತ್ರ(*dev));
            hlist_ಕೂಡಿಸು(&dev->ಗಂಟು, pprev);
            pprev = &dev->ಗಂಟು.ಮುಂದಿನ;
            ಎಣಿಕೆ_2++;

            // Find parent device.
            ಪೂರ್ಣಾಂಕ rootbus;
            ರಚನೆ pci_device *ಪೋಷಕ = busdevs[bus];
            ಆದರೆ (!ಪೋಷಕ) {
                ಆದರೆ (bus != lastbus)
                    rootbuses++;
                lastbus = bus;
                rootbus = rootbuses;
                ಆದರೆ (bus > MaxPCIBus)
                    MaxPCIBus = bus;
            } ಇಲ್ಲದಿದ್ದರೆ {
                rootbus = ಪೋಷಕ->rootbus;
            }

            // Populate pci_device info.
            dev->bdf = bdf;
            dev->ಪೋಷಕ = ಪೋಷಕ;
            dev->rootbus = rootbus;
            u32 vendev = pci_config_readl(bdf, PCI_VENDOR_ID);
            dev->vendor = vendev & 0xffff;
            dev->device = vendev >> 16;
            u32 classrev = pci_config_readl(bdf, PCI_CLASS_REVISION);
            dev->class = classrev >> 16;
            dev->prog_if = classrev >> 8;
            dev->revision = classrev & 0xff;
            dev->ಶೀರ್ಷಿಕೆ_type = pci_config_readb(bdf, PCI_ಶೀರ್ಷಿಕೆ_TYPE);
            u8 v = dev->ಶೀರ್ಷಿಕೆ_type & 0x7f;
            ಆದರೆ (v == PCI_ಶೀರ್ಷಿಕೆ_TYPE_BRIDGE || v == PCI_ಶೀರ್ಷಿಕೆ_TYPE_CARDBUS) {
                u8 secbus = pci_config_readb(bdf, PCI_SECONDARY_BUS);
                dev->secondary_bus = secbus;
                ಆದರೆ (secbus > bus && !busdevs[secbus])
                    busdevs[secbus] = dev;
                ಆದರೆ (secbus > MaxPCIBus)
                    MaxPCIBus = secbus;
            }
            dprintf(4, "PCI device %pP (vd=%04x:%04x c=%04x)\n"
                    , dev, dev->vendor, dev->device, dev->class);
        }
    }
    dprintf(1, "Found %d PCI devices (max PCI bus is %02x)\n", ಎಣಿಕೆ_2, MaxPCIBus);
}

// Search for a device with the specified vendor and device ids.
ರಚನೆ pci_device *
pci_ಕಂಡುಹಿಡಿ_device(u16 vendid, u16 devid)
{
    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (pci->vendor == vendid && pci->device == devid)
            ಹಿಂತಿರುಗಿ pci;
    }
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

// Search for a device with the specified class id.
ರಚನೆ pci_device *
pci_ಕಂಡುಹಿಡಿ_class(u16 classid)
{
    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (pci->class == classid)
            ಹಿಂತಿರುಗಿ pci;
    }
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಪೂರ್ಣಾಂಕ pci_init_device(ಸ್ಥಿರಾಂಕ ರಚನೆ pci_device_id *ids
                    , ರಚನೆ pci_device *pci, ಶೂನ್ಯ *arg)
{
    ಆಗ (ids->vendid || ids->class_ಮುಖವಾಡ) {
        ಆದರೆ ((ids->vendid == PCI_ANY_ID || ids->vendid == pci->vendor) &&
            (ids->devid == PCI_ANY_ID || ids->devid == pci->device) &&
            !((ids->class ^ pci->class) & ids->class_ಮುಖವಾಡ)) {
            ಆದರೆ (ids->func)
                ids->func(pci, arg);
            ಹಿಂತಿರುಗಿ 0;
        }
        ids++;
    }
    ಹಿಂತಿರುಗಿ -1;
}

ರಚನೆ pci_device *
pci_ಕಂಡುಹಿಡಿ_init_device(ಸ್ಥಿರಾಂಕ ರಚನೆ pci_device_id *ids, ಶೂನ್ಯ *arg)
{
    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (pci_init_device(ids, pci, arg) == 0)
            ಹಿಂತಿರುಗಿ pci;
    }
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

// Enable PCI bus-mastering (ie, DMA) support on a pci device
ಶೂನ್ಯ
pci_enable_busmaster(ರಚನೆ pci_device *pci)
{
    wait_preempt();
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_MASTER);
    pci->have_driver = 1;
}

// Verify an IO bar and return it to the caller
u16
pci_enable_iobar(ರಚನೆ pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    ಆದರೆ (!(bar & PCI_ತಳ_ವಿಳಾಸ_SPACE_IO)) {
        warn_internalerror();
        ಹಿಂತಿರುಗಿ 0;
    }
    bar &= PCI_ತಳ_ವಿಳಾಸ_IO_ಮುಖವಾಡ;
    ಆದರೆ (bar == 0 || bar > 0xffff) {
        warn_internalerror();
        ಹಿಂತಿರುಗಿ 0;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_IO);
    pci->have_driver = 1;
    ಹಿಂತಿರುಗಿ bar;
}

// Verify a memory bar and return it to the caller
ಶೂನ್ಯ *
pci_enable_membar(ರಚನೆ pci_device *pci, u32 addr)
{
    wait_preempt();
    u32 bar = pci_config_readl(pci->bdf, addr);
    ಆದರೆ (bar & PCI_ತಳ_ವಿಳಾಸ_SPACE_IO) {
        warn_internalerror();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    ಆದರೆ (bar & PCI_ತಳ_ವಿಳಾಸ_MEM_TYPE_64) {
        u32 ಮೇಲಿನ = pci_config_readl(pci->bdf, addr+4);
        ಆದರೆ (ಮೇಲಿನ) {
            dprintf(1, "Can not map memory bar over 4Gig\n");
            ಹಿಂತಿರುಗಿ ಖಾಲಿ;
        }
    }
    bar &= PCI_ತಳ_ವಿಳಾಸ_MEM_ಮುಖವಾಡ;
    ಆದರೆ (bar + 4*1024*1024 < 20*1024*1024) {
        // Bar doesn't look valid (it is in last 4M or first 16M)
        warn_internalerror();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    pci_config_maskw(pci->bdf, PCI_COMMAND, 0, PCI_COMMAND_ಸ್ಮರಣೆ);
    pci->have_driver = 1;
    ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)bar;
}

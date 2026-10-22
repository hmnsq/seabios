#ifndef __PCIDEVICE_H
#define __PCIDEVICE_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32
#ಅನ್ತರ್ಭಾವ "list.h" // hlist_node

ಸಂರಚನಾ pci_device {
    u16 bdf;
    u8 rootbus;
    ಸಂರಚನಾ hlist_ಗಂಟು ಗಂಟು;
    ಸಂರಚನಾ pci_device *parent;

    // Configuration space device information
    u16 vendor, device;
    u16 class;
    u8 prog_if, revision;
    u8 header_type;
    u8 secondary_bus;

    // Local information on device.
    ಪೂರ್ಣಾಙ್ಕ have_driver;
};
ಬಾಹ್ಯ ಸಂರಚನಾ hlist_head PCIDevices;
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ MaxPCIBus;

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u32 pci_classprog(ಸಂರಚನಾ pci_device *pci) {
    ಪ್ರತ್ಯಾಗಮ (pci->class << 8) | pci->prog_if;
}

#define foreachpci(PCI)                                 \
    hlist_for_each_entry(PCI, &PCIDevices, ಗಂಟು)

#define PCI_ANY_ID      (~0)
ಸಂರಚನಾ pci_device_id {
    u32 vendid;
    u32 devid;
    u32 class;
    u32 class_mask;
    ಶೂನ್ಯ (*func)(ಸಂರಚನಾ pci_device *pci, ಶೂನ್ಯ *arg);
};

#define PCI_DEVICE(vendor_id, device_id, init_func)     \
    {                                                   \
        .vendid = (vendor_id),                          \
        .devid = (device_id),                           \
        .class = PCI_ANY_ID,                            \
        .class_mask = 0,                                \
        .func = (init_func)                             \
    }

#define PCI_DEVICE_CLASS(vendor_id, device_id, class_code, init_func)   \
    {                                                                   \
        .vendid = (vendor_id),                                          \
        .devid = (device_id),                                           \
        .class = (class_code),                                          \
        .class_mask = ~0,                                               \
        .func = (init_func)                                             \
    }

#define PCI_DEVICE_END                          \
    {                                           \
        .vendid = 0,                            \
    }

ಶೂನ್ಯ pci_probe_devices(ಶೂನ್ಯ);
ಸಂರಚನಾ pci_device *pci_find_device(u16 vendid, u16 devid);
ಸಂರಚನಾ pci_device *pci_find_class(u16 classid);
ಪೂರ್ಣಾಙ್ಕ pci_init_device(ಧ್ರುವ ಸಂರಚನಾ pci_device_id *ids
                    , ಸಂರಚನಾ pci_device *pci, ಶೂನ್ಯ *arg);
ಸಂರಚನಾ pci_device *pci_find_init_device(ಧ್ರುವ ಸಂರಚನಾ pci_device_id *ids
                                        , ಶೂನ್ಯ *arg);
ಶೂನ್ಯ pci_enable_busmaster(ಸಂರಚನಾ pci_device *pci);
u16 pci_enable_iobar(ಸಂರಚನಾ pci_device *pci, u32 addr);
ಶೂನ್ಯ *pci_enable_membar(ಸಂರಚನಾ pci_device *pci, u32 addr);

#endif // pcidevice.h

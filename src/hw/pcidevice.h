#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __PCIDEVICE_H
#ವ್ಯಾಖ್ಯಾನಿಸು __PCIDEVICE_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32
#ಅನ್ತರ್ಭಾವ "list.h" // hlist_node

ರಚನೆ pci_device {
    u16 bdf;
    u8 rootbus;
    ರಚನೆ hlist_ಗಂಟು ಗಂಟು;
    ರಚನೆ pci_device *ಪೋಷಕ;

    // Configuration space device information
    u16 vendor, device;
    u16 class;
    u8 prog_if, revision;
    u8 ಶೀರ್ಷಿಕೆ_type;
    u8 secondary_bus;

    // Local information on device.
    ಪೂರ್ಣಾಂಕ have_driver;
};
ಬಾಹ್ಯ ರಚನೆ hlist_ತಲೆ PCIDevices;
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ MaxPCIBus;

ಸ್ಥಿರ ಸಾಲೊಳಗೆ u32 pci_classprog(ರಚನೆ pci_device *pci) {
    ಹಿಂತಿರುಗಿ (pci->class << 8) | pci->prog_if;
}

#ವ್ಯಾಖ್ಯಾನಿಸು foreachpci(PCI)                                 \
    hlist_for_each_ನಮೂದು(PCI, &PCIDevices, ಗಂಟು)

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_ANY_ID      (~0)
ರಚನೆ pci_device_id {
    u32 vendid;
    u32 devid;
    u32 class;
    u32 class_ಮುಖವಾಡ;
    ಶೂನ್ಯ (*func)(ರಚನೆ pci_device *pci, ಶೂನ್ಯ *arg);
};

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_DEVICE(vendor_id, device_id, init_func)     \
    {                                                   \
        .vendid = (vendor_id),                          \
        .devid = (device_id),                           \
        .class = PCI_ANY_ID,                            \
        .class_ಮುಖವಾಡ = 0,                                \
        .func = (init_func)                             \
    }

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_DEVICE_CLASS(vendor_id, device_id, class_ಸಂಕೇತ, init_func)   \
    {                                                                   \
        .vendid = (vendor_id),                                          \
        .devid = (device_id),                                           \
        .class = (class_ಸಂಕೇತ),                                          \
        .class_ಮುಖವಾಡ = ~0,                                               \
        .func = (init_func)                                             \
    }

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_DEVICE_ಕೊನೆ                          \
    {                                           \
        .vendid = 0,                            \
    }

ಶೂನ್ಯ pci_probe_devices(ಶೂನ್ಯ);
ರಚನೆ pci_device *pci_ಕಂಡುಹಿಡಿ_device(u16 vendid, u16 devid);
ರಚನೆ pci_device *pci_ಕಂಡುಹಿಡಿ_class(u16 classid);
ಪೂರ್ಣಾಂಕ pci_init_device(ಸ್ಥಿರಾಂಕ ರಚನೆ pci_device_id *ids
                    , ರಚನೆ pci_device *pci, ಶೂನ್ಯ *arg);
ರಚನೆ pci_device *pci_ಕಂಡುಹಿಡಿ_init_device(ಸ್ಥಿರಾಂಕ ರಚನೆ pci_device_id *ids
                                        , ಶೂನ್ಯ *arg);
ಶೂನ್ಯ pci_enable_busmaster(ರಚನೆ pci_device *pci);
u16 pci_enable_iobar(ರಚನೆ pci_device *pci, u32 addr);
ಶೂನ್ಯ *pci_enable_membar(ರಚನೆ pci_device *pci, u32 addr);

#ಆದರೆ_ಅಂತ್ಯ // pcidevice.h

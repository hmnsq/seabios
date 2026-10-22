#ifndef __PCIDEVICE_H
#define __PCIDEVICE_H

#अन्तर्भाव "types.h" // u32
#अन्तर्भाव "list.h" // hlist_node

संरचना pci_device {
    u16 bdf;
    u8 rootbus;
    संरचना hlist_गाठ गाठ;
    संरचना pci_device *parent;

    // Configuration space device information
    u16 vendor, device;
    u16 class;
    u8 prog_if, revision;
    u8 header_type;
    u8 secondary_bus;

    // Local information on device.
    पूर्णांक have_driver;
};
बाह्य संरचना hlist_head PCIDevices;
बाह्य पूर्णांक MaxPCIBus;

स्थिर ओळीत u32 pci_classprog(संरचना pci_device *pci) {
    परत (pci->class << 8) | pci->prog_if;
}

#define foreachpci(PCI)                                 \
    hlist_for_each_entry(PCI, &PCIDevices, गाठ)

#define PCI_ANY_ID      (~0)
संरचना pci_device_id {
    u32 vendid;
    u32 devid;
    u32 class;
    u32 class_mask;
    शून्य (*func)(संरचना pci_device *pci, शून्य *arg);
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

शून्य pci_probe_devices(शून्य);
संरचना pci_device *pci_find_device(u16 vendid, u16 devid);
संरचना pci_device *pci_find_class(u16 classid);
पूर्णांक pci_init_device(स्थिरांक संरचना pci_device_id *ids
                    , संरचना pci_device *pci, शून्य *arg);
संरचना pci_device *pci_find_init_device(स्थिरांक संरचना pci_device_id *ids
                                        , शून्य *arg);
शून्य pci_enable_busmaster(संरचना pci_device *pci);
u16 pci_enable_iobar(संरचना pci_device *pci, u32 addr);
शून्य *pci_enable_membar(संरचना pci_device *pci, u32 addr);

#endif // pcidevice.h

#यदि_अपरिभाषितम् __PCIDEVICE_H
#परिभाषय __PCIDEVICE_H

#अन्तर्भाव "types.h" // u32
#अन्तर्भाव "list.h" // hlist_node

संरचना pci_device {
    u16 bdf;
    u8 rootbus;
    संरचना hlist_ग्रन्थिः ग्रन्थिः;
    संरचना pci_device *parent;

    // Configuration space device information
    u16 vendor, device;
    u16 class;
    u8 prog_if, revision;
    u8 शीर्षकम्_type;
    u8 secondary_bus;

    // Local information on device.
    पूर्णाङ्क have_driver;
};
बाह्य संरचना hlist_शीर्ष PCIDevices;
बाह्य पूर्णाङ्क MaxPCIBus;

स्थिर अन्तःस्थ u32 pci_classprog(संरचना pci_device *pci) {
    प्रत्यागम (pci->class << 8) | pci->prog_if;
}

#परिभाषय foreachpci(PCI)                                 \
    hlist_for_each_प्रविष्टिः(PCI, &PCIDevices, ग्रन्थिः)

#परिभाषय PCI_ANY_ID      (~0)
संरचना pci_device_id {
    u32 vendid;
    u32 devid;
    u32 class;
    u32 class_आवरणम्;
    शून्य (*func)(संरचना pci_device *pci, शून्य *arg);
};

#परिभाषय PCI_DEVICE(vendor_id, device_id, init_func)     \
    {                                                   \
        .vendid = (vendor_id),                          \
        .devid = (device_id),                           \
        .class = PCI_ANY_ID,                            \
        .class_आवरणम् = 0,                                \
        .func = (init_func)                             \
    }

#परिभाषय PCI_DEVICE_CLASS(vendor_id, device_id, class_संहिता, init_func)   \
    {                                                                   \
        .vendid = (vendor_id),                                          \
        .devid = (device_id),                                           \
        .class = (class_संहिता),                                          \
        .class_आवरणम् = ~0,                                               \
        .func = (init_func)                                             \
    }

#परिभाषय PCI_DEVICE_अन्तः                          \
    {                                           \
        .vendid = 0,                            \
    }

शून्य pci_probe_devices(शून्य);
संरचना pci_device *pci_find_device(u16 vendid, u16 devid);
संरचना pci_device *pci_find_class(u16 classid);
पूर्णाङ्क pci_init_device(ध्रुव संरचना pci_device_id *ids
                    , संरचना pci_device *pci, शून्य *arg);
संरचना pci_device *pci_find_init_device(ध्रुव संरचना pci_device_id *ids
                                        , शून्य *arg);
शून्य pci_enable_busmaster(संरचना pci_device *pci);
u16 pci_enable_iobar(संरचना pci_device *pci, u32 addr);
शून्य *pci_enable_membar(संरचना pci_device *pci, u32 addr);

#यदि_अन्तः // pcidevice.h

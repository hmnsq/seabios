#अगर_अघोषित __PCIDEVICE_H
#घोषणा __PCIDEVICE_H

#समावेश "types.h" // u32
#समावेश "list.h" // hlist_node

काष्ठा pci_device {
    u16 bdf;
    u8 rootbus;
    काष्ठा hlist_गाँठ गाँठ;
    काष्ठा pci_device *parent;

    // Configuration space device information
    u16 vendor, device;
    u16 class;
    u8 prog_if, revision;
    u8 header_type;
    u8 secondary_bus;

    // Local information on device.
    पूर्णांक have_driver;
};
बाह्य काष्ठा hlist_शीर्ष PCIDevices;
बाह्य पूर्णांक MaxPCIBus;

अटल अंतरभूत u32 pci_classprog(काष्ठा pci_device *pci) {
    वापस (pci->class << 8) | pci->prog_if;
}

#घोषणा foreachpci(PCI)                                 \
    hlist_for_each_entry(PCI, &PCIDevices, गाँठ)

#घोषणा PCI_ANY_ID      (~0)
काष्ठा pci_device_id {
    u32 vendid;
    u32 devid;
    u32 class;
    u32 class_mask;
    व्योम (*func)(काष्ठा pci_device *pci, व्योम *arg);
};

#घोषणा PCI_DEVICE(vendor_id, device_id, init_func)     \
    {                                                   \
        .vendid = (vendor_id),                          \
        .devid = (device_id),                           \
        .class = PCI_ANY_ID,                            \
        .class_mask = 0,                                \
        .func = (init_func)                             \
    }

#घोषणा PCI_DEVICE_CLASS(vendor_id, device_id, class_code, init_func)   \
    {                                                                   \
        .vendid = (vendor_id),                                          \
        .devid = (device_id),                                           \
        .class = (class_code),                                          \
        .class_mask = ~0,                                               \
        .func = (init_func)                                             \
    }

#घोषणा PCI_DEVICE_END                          \
    {                                           \
        .vendid = 0,                            \
    }

व्योम pci_probe_devices(व्योम);
काष्ठा pci_device *pci_find_device(u16 vendid, u16 devid);
काष्ठा pci_device *pci_find_class(u16 classid);
पूर्णांक pci_init_device(स्थिर काष्ठा pci_device_id *ids
                    , काष्ठा pci_device *pci, व्योम *arg);
काष्ठा pci_device *pci_find_init_device(स्थिर काष्ठा pci_device_id *ids
                                        , व्योम *arg);
व्योम pci_enable_busmaster(काष्ठा pci_device *pci);
u16 pci_enable_iobar(काष्ठा pci_device *pci, u32 addr);
व्योम *pci_enable_membar(काष्ठा pci_device *pci, u32 addr);

#पूर्ण_अगर // pcidevice.h

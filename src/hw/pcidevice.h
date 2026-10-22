#ifndef __PCIDEVICE_H
#define __PCIDEVICE_H

#അന്തര്ഭാവ "types.h" // u32
#അന്തര്ഭാവ "list.h" // hlist_node

ഘടന pci_device {
    u16 bdf;
    u8 rootbus;
    ഘടന hlist_കണ്ണി കണ്ണി;
    ഘടന pci_device *parent;

    // Configuration space device information
    u16 vendor, device;
    u16 class;
    u8 prog_if, revision;
    u8 header_type;
    u8 secondary_bus;

    // Local information on device.
    പൂർണ്ണസംഖ്യ have_driver;
};
ബാഹ്യ ഘടന hlist_head PCIDevices;
ബാഹ്യ പൂർണ്ണസംഖ്യ MaxPCIBus;

സ്ഥിതം വരിയിൽ u32 pci_classprog(ഘടന pci_device *pci) {
    തിരികെ (pci->class << 8) | pci->prog_if;
}

#define foreachpci(PCI)                                 \
    hlist_for_each_entry(PCI, &PCIDevices, കണ്ണി)

#define PCI_ANY_ID      (~0)
ഘടന pci_device_id {
    u32 vendid;
    u32 devid;
    u32 class;
    u32 class_mask;
    ശൂന്യം (*func)(ഘടന pci_device *pci, ശൂന്യം *arg);
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

ശൂന്യം pci_probe_devices(ശൂന്യം);
ഘടന pci_device *pci_find_device(u16 vendid, u16 devid);
ഘടന pci_device *pci_find_class(u16 classid);
പൂർണ്ണസംഖ്യ pci_init_device(സ്ഥിരം ഘടന pci_device_id *ids
                    , ഘടന pci_device *pci, ശൂന്യം *arg);
ഘടന pci_device *pci_find_init_device(സ്ഥിരം ഘടന pci_device_id *ids
                                        , ശൂന്യം *arg);
ശൂന്യം pci_enable_busmaster(ഘടന pci_device *pci);
u16 pci_enable_iobar(ഘടന pci_device *pci, u32 addr);
ശൂന്യം *pci_enable_membar(ഘടന pci_device *pci, u32 addr);

#endif // pcidevice.h

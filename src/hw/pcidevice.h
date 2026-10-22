#ifndef __PCIDEVICE_H
#define __PCIDEVICE_H

#અન્તર્ભાવ "types.h" // u32
#અન્તર્ભાવ "list.h" // hlist_node

સંરચના pci_device {
    u16 bdf;
    u8 rootbus;
    સંરચના hlist_ગાંઠ ગાંઠ;
    સંરચના pci_device *parent;

    // Configuration space device information
    u16 vendor, device;
    u16 class;
    u8 prog_if, revision;
    u8 header_type;
    u8 secondary_bus;

    // Local information on device.
    પૂર્ણાંક have_driver;
};
બાહ્ય સંરચના hlist_head PCIDevices;
બાહ્ય પૂર્ણાંક MaxPCIBus;

સ્થિર અંતઃરેખ u32 pci_classprog(સંરચના pci_device *pci) {
    પાછા (pci->class << 8) | pci->prog_if;
}

#define foreachpci(PCI)                                 \
    hlist_for_each_entry(PCI, &PCIDevices, ગાંઠ)

#define PCI_ANY_ID      (~0)
સંરચના pci_device_id {
    u32 vendid;
    u32 devid;
    u32 class;
    u32 class_mask;
    શૂન્ય (*func)(સંરચના pci_device *pci, શૂન્ય *arg);
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

શૂન્ય pci_probe_devices(શૂન્ય);
સંરચના pci_device *pci_find_device(u16 vendid, u16 devid);
સંરચના pci_device *pci_find_class(u16 classid);
પૂર્ણાંક pci_init_device(અચલ સંરચના pci_device_id *ids
                    , સંરચના pci_device *pci, શૂન્ય *arg);
સંરચના pci_device *pci_find_init_device(અચલ સંરચના pci_device_id *ids
                                        , શૂન્ય *arg);
શૂન્ય pci_enable_busmaster(સંરચના pci_device *pci);
u16 pci_enable_iobar(સંરચના pci_device *pci, u32 addr);
શૂન્ય *pci_enable_membar(સંરચના pci_device *pci, u32 addr);

#endif // pcidevice.h

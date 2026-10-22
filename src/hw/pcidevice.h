#ifndef __PCIDEVICE_H
#define __PCIDEVICE_H

#ଅନ୍ତର୍ଭାଵ "types.h" // u32
#ଅନ୍ତର୍ଭାଵ "list.h" // hlist_node

ସଂରଚନା pci_device {
    u16 bdf;
    u8 rootbus;
    ସଂରଚନା hlist_ଗଣ୍ଠି ଗଣ୍ଠି;
    ସଂରଚନା pci_device *parent;

    // Configuration space device information
    u16 vendor, device;
    u16 class;
    u8 prog_if, revision;
    u8 header_type;
    u8 secondary_bus;

    // Local information on device.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା have_driver;
};
ବାହ୍ୟ ସଂରଚନା hlist_head PCIDevices;
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା MaxPCIBus;

ସ୍ଥିର ଧାଡ଼ିଭିତରେ u32 pci_classprog(ସଂରଚନା pci_device *pci) {
    ଫେରନ୍ତୁ (pci->class << 8) | pci->prog_if;
}

#define foreachpci(PCI)                                 \
    hlist_for_each_entry(PCI, &PCIDevices, ଗଣ୍ଠି)

#define PCI_ANY_ID      (~0)
ସଂରଚନା pci_device_id {
    u32 vendid;
    u32 devid;
    u32 class;
    u32 class_mask;
    ଶୂନ୍ୟ (*func)(ସଂରଚନା pci_device *pci, ଶୂନ୍ୟ *arg);
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

ଶୂନ୍ୟ pci_probe_devices(ଶୂନ୍ୟ);
ସଂରଚନା pci_device *pci_find_device(u16 vendid, u16 devid);
ସଂରଚନା pci_device *pci_find_class(u16 classid);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା pci_init_device(ଅଚଳ ସଂରଚନା pci_device_id *ids
                    , ସଂରଚନା pci_device *pci, ଶୂନ୍ୟ *arg);
ସଂରଚନା pci_device *pci_find_init_device(ଅଚଳ ସଂରଚନା pci_device_id *ids
                                        , ଶୂନ୍ୟ *arg);
ଶୂନ୍ୟ pci_enable_busmaster(ସଂରଚନା pci_device *pci);
u16 pci_enable_iobar(ସଂରଚନା pci_device *pci, u32 addr);
ଶୂନ୍ୟ *pci_enable_membar(ସଂରଚନା pci_device *pci, u32 addr);

#endif // pcidevice.h

#ifndef __PCIDEVICE_H
#define __PCIDEVICE_H

#অন্তর্ভাব "types.h" // u32
#অন্তর্ভাব "list.h" // hlist_node

সংরচনা pci_device {
    u16 bdf;
    u8 rootbus;
    সংরচনা hlist_গ্রন্থি গ্রন্থি;
    সংরচনা pci_device *parent;

    // Configuration space device information
    u16 vendor, device;
    u16 class;
    u8 prog_if, revision;
    u8 header_type;
    u8 secondary_bus;

    // Local information on device.
    পূর্ণাঙ্ক have_driver;
};
বাহ্য সংরচনা hlist_head PCIDevices;
বাহ্য পূর্ণাঙ্ক MaxPCIBus;

স্থির অন্তঃস্থ u32 pci_classprog(সংরচনা pci_device *pci) {
    প্রত্যাগম (pci->class << 8) | pci->prog_if;
}

#define foreachpci(PCI)                                 \
    hlist_for_each_entry(PCI, &PCIDevices, গ্রন্থি)

#define PCI_ANY_ID      (~0)
সংরচনা pci_device_id {
    u32 vendid;
    u32 devid;
    u32 class;
    u32 class_mask;
    শূন্য (*func)(সংরচনা pci_device *pci, শূন্য *arg);
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

শূন্য pci_probe_devices(শূন্য);
সংরচনা pci_device *pci_find_device(u16 vendid, u16 devid);
সংরচনা pci_device *pci_find_class(u16 classid);
পূর্ণাঙ্ক pci_init_device(ধ্রুব সংরচনা pci_device_id *ids
                    , সংরচনা pci_device *pci, শূন্য *arg);
সংরচনা pci_device *pci_find_init_device(ধ্রুব সংরচনা pci_device_id *ids
                                        , শূন্য *arg);
শূন্য pci_enable_busmaster(সংরচনা pci_device *pci);
u16 pci_enable_iobar(সংরচনা pci_device *pci, u32 addr);
শূন্য *pci_enable_membar(সংরচনা pci_device *pci, u32 addr);

#endif // pcidevice.h

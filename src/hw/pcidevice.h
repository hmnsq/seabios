#ifndef __PCIDEVICE_H
#define __PCIDEVICE_H

#شامل "types.h" // u32
#شامل "list.h" // hlist_node

ساختار pci_device {
    u16 bdf;
    u8 rootbus;
    ساختار hlist_گره گره;
    ساختار pci_device *parent;

    // Configuration space device information
    u16 vendor, device;
    u16 class;
    u8 prog_if, revision;
    u8 header_type;
    u8 secondary_bus;

    // Local information on device.
    صحیح have_driver;
};
بیرونی ساختار hlist_head PCIDevices;
بیرونی صحیح MaxPCIBus;

ایستا درونخطی u32 pci_classprog(ساختار pci_device *pci) {
    بازگشت (pci->class << 8) | pci->prog_if;
}

#define foreachpci(PCI)                                 \
    hlist_for_each_entry(PCI, &PCIDevices, گره)

#define PCI_ANY_ID      (~0)
ساختار pci_device_id {
    u32 vendid;
    u32 devid;
    u32 class;
    u32 class_mask;
    تهی (*func)(ساختار pci_device *pci, تهی *arg);
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

تهی pci_probe_devices(تهی);
ساختار pci_device *pci_find_device(u16 vendid, u16 devid);
ساختار pci_device *pci_find_class(u16 classid);
صحیح pci_init_device(ثابت ساختار pci_device_id *ids
                    , ساختار pci_device *pci, تهی *arg);
ساختار pci_device *pci_find_init_device(ثابت ساختار pci_device_id *ids
                                        , تهی *arg);
تهی pci_enable_busmaster(ساختار pci_device *pci);
u16 pci_enable_iobar(ساختار pci_device *pci, u32 addr);
تهی *pci_enable_membar(ساختار pci_device *pci, u32 addr);

#endif // pcidevice.h

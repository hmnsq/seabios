#ifndef __PCIDEVICE_H
#define __PCIDEVICE_H

#அந்தர்பாவ "types.h" // u32
#அந்தர்பாவ "list.h" // hlist_node

கட்டமைப்பு pci_device {
    u16 bdf;
    u8 rootbus;
    கட்டமைப்பு hlist_முனை முனை;
    கட்டமைப்பு pci_device *parent;

    // Configuration space device information
    u16 vendor, device;
    u16 class;
    u8 prog_if, revision;
    u8 header_type;
    u8 secondary_bus;

    // Local information on device.
    முழு have_driver;
};
வெளி கட்டமைப்பு hlist_head PCIDevices;
வெளி முழு MaxPCIBus;

நிலையான உள்வரி u32 pci_classprog(கட்டமைப்பு pci_device *pci) {
    திரும்பு (pci->class << 8) | pci->prog_if;
}

#define foreachpci(PCI)                                 \
    hlist_for_each_entry(PCI, &PCIDevices, முனை)

#define PCI_ANY_ID      (~0)
கட்டமைப்பு pci_device_id {
    u32 vendid;
    u32 devid;
    u32 class;
    u32 class_mask;
    வெறுமை (*func)(கட்டமைப்பு pci_device *pci, வெறுமை *arg);
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

வெறுமை pci_probe_devices(வெறுமை);
கட்டமைப்பு pci_device *pci_find_device(u16 vendid, u16 devid);
கட்டமைப்பு pci_device *pci_find_class(u16 classid);
முழு pci_init_device(மாறிலி கட்டமைப்பு pci_device_id *ids
                    , கட்டமைப்பு pci_device *pci, வெறுமை *arg);
கட்டமைப்பு pci_device *pci_find_init_device(மாறிலி கட்டமைப்பு pci_device_id *ids
                                        , வெறுமை *arg);
வெறுமை pci_enable_busmaster(கட்டமைப்பு pci_device *pci);
u16 pci_enable_iobar(கட்டமைப்பு pci_device *pci, u32 addr);
வெறுமை *pci_enable_membar(கட்டமைப்பு pci_device *pci, u32 addr);

#endif // pcidevice.h

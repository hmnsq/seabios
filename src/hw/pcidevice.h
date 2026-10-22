#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __PCIDEVICE_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __PCIDEVICE_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32
#ਅਨ੍ਤਰ੍ਭਾਵ "list.h" // hlist_node

ਬਣਤਰ pci_device {
    u16 bdf;
    u8 rootbus;
    ਬਣਤਰ hlist_ਗੰਢ ਗੰਢ;
    ਬਣਤਰ pci_device *ਮਾਪਾ;

    // Configuration space device information
    u16 vendor, device;
    u16 class;
    u8 prog_if, revision;
    u8 ਸਿਰਲੇਖ_type;
    u8 secondary_bus;

    // Local information on device.
    ਪੂਰਨ have_driver;
};
ਬਾਹਰੀ ਬਣਤਰ hlist_ਸਿਰ PCIDevices;
ਬਾਹਰੀ ਪੂਰਨ MaxPCIBus;

ਸਥਿਰ ਸਤਰਅੰਦਰ u32 pci_classprog(ਬਣਤਰ pci_device *pci) {
    ਵਾਪਸ (pci->class << 8) | pci->prog_if;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ foreachpci(PCI)                                 \
    hlist_for_each_ਇੰਦਰਾਜ(PCI, &PCIDevices, ਗੰਢ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_ANY_ID      (~0)
ਬਣਤਰ pci_device_id {
    u32 vendid;
    u32 devid;
    u32 class;
    u32 class_ਨਕਾਬ;
    ਖਾਲੀ (*func)(ਬਣਤਰ pci_device *pci, ਖਾਲੀ *arg);
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_DEVICE(vendor_id, device_id, init_func)     \
    {                                                   \
        .vendid = (vendor_id),                          \
        .devid = (device_id),                           \
        .class = PCI_ANY_ID,                            \
        .class_ਨਕਾਬ = 0,                                \
        .func = (init_func)                             \
    }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_DEVICE_CLASS(vendor_id, device_id, class_ਕੋਡ, init_func)   \
    {                                                                   \
        .vendid = (vendor_id),                                          \
        .devid = (device_id),                                           \
        .class = (class_ਕੋਡ),                                          \
        .class_ਨਕਾਬ = ~0,                                               \
        .func = (init_func)                                             \
    }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_DEVICE_ਸਿਰਾ                          \
    {                                           \
        .vendid = 0,                            \
    }

ਖਾਲੀ pci_probe_devices(ਖਾਲੀ);
ਬਣਤਰ pci_device *pci_ਲੱਭੋ_device(u16 vendid, u16 devid);
ਬਣਤਰ pci_device *pci_ਲੱਭੋ_class(u16 classid);
ਪੂਰਨ pci_init_device(ਅਚਲ ਬਣਤਰ pci_device_id *ids
                    , ਬਣਤਰ pci_device *pci, ਖਾਲੀ *arg);
ਬਣਤਰ pci_device *pci_ਲੱਭੋ_init_device(ਅਚਲ ਬਣਤਰ pci_device_id *ids
                                        , ਖਾਲੀ *arg);
ਖਾਲੀ pci_enable_busmaster(ਬਣਤਰ pci_device *pci);
u16 pci_enable_iobar(ਬਣਤਰ pci_device *pci, u32 addr);
ਖਾਲੀ *pci_enable_membar(ਬਣਤਰ pci_device *pci, u32 addr);

#ਜੇ_ਅੰਤ // pcidevice.h

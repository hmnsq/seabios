#اگر_غیر_تعریف_شدہ __PCIDEVICE_H
#تعریف_کرو __PCIDEVICE_H

#شامل "types.h" // u32
#شامل "list.h" // hlist_node

ساخت pci_device {
    u16 bdf;
    u8 rootbus;
    ساخت hlist_گرہ گرہ;
    ساخت pci_device *والد;

    // Configuration space device information
    u16 vendor, device;
    u16 class;
    u8 prog_if, revision;
    u8 سرخی_type;
    u8 secondary_bus;

    // Local information on device.
    صحیح have_driver;
};
خارجی ساخت hlist_سر PCIDevices;
خارجی صحیح MaxPCIBus;

ساکن اندرونی u32 pci_classprog(ساخت pci_device *pci) {
    واپس (pci->class << 8) | pci->prog_if;
}

#تعریف_کرو foreachpci(PCI)                                 \
    hlist_for_each_اندراج(PCI, &PCIDevices, گرہ)

#تعریف_کرو PCI_ANY_ID      (~0)
ساخت pci_device_id {
    u32 vendid;
    u32 devid;
    u32 class;
    u32 class_نقاب;
    خالی (*func)(ساخت pci_device *pci, خالی *arg);
};

#تعریف_کرو PCI_DEVICE(vendor_id, device_id, init_func)     \
    {                                                   \
        .vendid = (vendor_id),                          \
        .devid = (device_id),                           \
        .class = PCI_ANY_ID,                            \
        .class_نقاب = 0,                                \
        .func = (init_func)                             \
    }

#تعریف_کرو PCI_DEVICE_CLASS(vendor_id, device_id, class_رمز, init_func)   \
    {                                                                   \
        .vendid = (vendor_id),                                          \
        .devid = (device_id),                                           \
        .class = (class_رمز),                                          \
        .class_نقاب = ~0,                                               \
        .func = (init_func)                                             \
    }

#تعریف_کرو PCI_DEVICE_انتہا                          \
    {                                           \
        .vendid = 0,                            \
    }

خالی pci_probe_devices(خالی);
ساخت pci_device *pci_ڈھونڈو_device(u16 vendid, u16 devid);
ساخت pci_device *pci_ڈھونڈو_class(u16 classid);
صحیح pci_init_device(ثابت ساخت pci_device_id *ids
                    , ساخت pci_device *pci, خالی *arg);
ساخت pci_device *pci_ڈھونڈو_init_device(ثابت ساخت pci_device_id *ids
                                        , خالی *arg);
خالی pci_enable_busmaster(ساخت pci_device *pci);
u16 pci_enable_iobar(ساخت pci_device *pci, u32 addr);
خالی *pci_enable_membar(ساخت pci_device *pci, u32 addr);

#اگر_ختم // pcidevice.h

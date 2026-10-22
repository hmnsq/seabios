#ifndef __PCIDEVICE_H
#define __PCIDEVICE_H

#ضمن "types.h" // u32
#ضمن "list.h" // hlist_node

بنية pci_device {
    u16 bdf;
    u8 rootbus;
    بنية hlist_عقدة عقدة;
    بنية pci_device *parent;

    // Configuration space device information
    u16 vendor, device;
    u16 class;
    u8 prog_if, revision;
    u8 header_type;
    u8 secondary_bus;

    // Local information on device.
    صحيح have_driver;
};
خارجي بنية hlist_head PCIDevices;
خارجي صحيح MaxPCIBus;

ساكن مضمن u32 pci_classprog(بنية pci_device *pci) {
    أرجع (pci->class << 8) | pci->prog_if;
}

#define foreachpci(PCI)                                 \
    hlist_for_each_entry(PCI, &PCIDevices, عقدة)

#define PCI_ANY_ID      (~0)
بنية pci_device_id {
    u32 vendid;
    u32 devid;
    u32 class;
    u32 class_mask;
    فراغ (*func)(بنية pci_device *pci, فراغ *arg);
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

فراغ pci_probe_devices(فراغ);
بنية pci_device *pci_find_device(u16 vendid, u16 devid);
بنية pci_device *pci_find_class(u16 classid);
صحيح pci_init_device(ثابت بنية pci_device_id *ids
                    , بنية pci_device *pci, فراغ *arg);
بنية pci_device *pci_find_init_device(ثابت بنية pci_device_id *ids
                                        , فراغ *arg);
فراغ pci_enable_busmaster(بنية pci_device *pci);
u16 pci_enable_iobar(بنية pci_device *pci, u32 addr);
فراغ *pci_enable_membar(بنية pci_device *pci, u32 addr);

#endif // pcidevice.h

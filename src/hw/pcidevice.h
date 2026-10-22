#ifndef __PCIDEVICE_H
#define __PCIDEVICE_H

#include "types.h" // u32
#include "list.h" // hlist_node

sa_nracanaa pci_device {
    u16 bdf;
    u8 rootbus;
    sa_nracanaa hlist_node node;
    sa_nracanaa pci_device *parent;

    // Configuration space device information
    u16 vendor, device;
    u16 class;
    u8 prog_if, revision;
    u8 header_type;
    u8 secondary_bus;

    // Local information on device.
    පූර්ණ_සංඛ්‍යා have_driver;
};
extern sa_nracanaa hlist_head PCIDevices;
extern පූර්ණ_සංඛ්‍යා MaxPCIBus;

static anta_hstha u32 pci_classprog(sa_nracanaa pci_device *pci) {
    ආපසු (pci->class << 8) | pci->prog_if;
}

#define foreachpci(PCI)                                 \
    hlist_for_each_entry(PCI, &PCIDevices, node)

#define PCI_ANY_ID      (~0)
sa_nracanaa pci_device_id {
    u32 vendid;
    u32 devid;
    u32 class;
    u32 class_mask;
    void (*func)(sa_nracanaa pci_device *pci, void *arg);
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

void pci_probe_devices(void);
sa_nracanaa pci_device *pci_find_device(u16 vendid, u16 devid);
sa_nracanaa pci_device *pci_find_class(u16 classid);
පූර්ණ_සංඛ්‍යා pci_init_device(const sa_nracanaa pci_device_id *ids
                    , sa_nracanaa pci_device *pci, void *arg);
sa_nracanaa pci_device *pci_find_init_device(const sa_nracanaa pci_device_id *ids
                                        , void *arg);
void pci_enable_busmaster(sa_nracanaa pci_device *pci);
u16 pci_enable_iobar(sa_nracanaa pci_device *pci, u32 addr);
void *pci_enable_membar(sa_nracanaa pci_device *pci, u32 addr);

#endif // pcidevice.h

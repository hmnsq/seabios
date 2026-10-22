#నిర్వచితం_కాకపోతే __PCIDEVICE_H
#నిర్వచించు __PCIDEVICE_H

#అన్తర్భావ "types.h" // u32
#అన్తర్భావ "list.h" // hlist_node

నిర్మాణం pci_device {
    u16 bdf;
    u8 rootbus;
    నిర్మాణం hlist_కణుపు కణుపు;
    నిర్మాణం pci_device *మాతృ;

    // Configuration space device information
    u16 vendor, device;
    u16 class;
    u8 prog_if, revision;
    u8 శీర్షిక_type;
    u8 secondary_bus;

    // Local information on device.
    పూర్ణాంకం have_driver;
};
బాహ్య నిర్మాణం hlist_తల PCIDevices;
బాహ్య పూర్ణాంకం MaxPCIBus;

స్థిర అంతఃవరుస u32 pci_classprog(నిర్మాణం pci_device *pci) {
    తిరిగి (pci->class << 8) | pci->prog_if;
}

#నిర్వచించు foreachpci(PCI)                                 \
    hlist_for_each_నమోదు(PCI, &PCIDevices, కణుపు)

#నిర్వచించు PCI_ANY_ID      (~0)
నిర్మాణం pci_device_id {
    u32 vendid;
    u32 devid;
    u32 class;
    u32 class_ముసుగు;
    శూన్య (*func)(నిర్మాణం pci_device *pci, శూన్య *arg);
};

#నిర్వచించు PCI_DEVICE(vendor_id, device_id, init_func)     \
    {                                                   \
        .vendid = (vendor_id),                          \
        .devid = (device_id),                           \
        .class = PCI_ANY_ID,                            \
        .class_ముసుగు = 0,                                \
        .func = (init_func)                             \
    }

#నిర్వచించు PCI_DEVICE_CLASS(vendor_id, device_id, class_సంకేతం, init_func)   \
    {                                                                   \
        .vendid = (vendor_id),                                          \
        .devid = (device_id),                                           \
        .class = (class_సంకేతం),                                          \
        .class_ముసుగు = ~0,                                               \
        .func = (init_func)                                             \
    }

#నిర్వచించు PCI_DEVICE_చివర                          \
    {                                           \
        .vendid = 0,                            \
    }

శూన్య pci_probe_devices(శూన్య);
నిర్మాణం pci_device *pci_కనుగొను_device(u16 vendid, u16 devid);
నిర్మాణం pci_device *pci_కనుగొను_class(u16 classid);
పూర్ణాంకం pci_init_device(స్థిరరాశి నిర్మాణం pci_device_id *ids
                    , నిర్మాణం pci_device *pci, శూన్య *arg);
నిర్మాణం pci_device *pci_కనుగొను_init_device(స్థిరరాశి నిర్మాణం pci_device_id *ids
                                        , శూన్య *arg);
శూన్య pci_enable_busmaster(నిర్మాణం pci_device *pci);
u16 pci_enable_iobar(నిర్మాణం pci_device *pci, u32 addr);
శూన్య *pci_enable_membar(నిర్మాణం pci_device *pci, u32 addr);

#అయితే_ముగింపు // pcidevice.h

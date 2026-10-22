#ifndef __PCI_H
#define __PCI_H

#अन्तर्भाव "types.h" // u32

#define PORT_PCI_REBOOT        0x0cf9

स्थिर ओळीत u8 pci_bdf_to_bus(u16 bdf) {
    परत bdf >> 8;
}
स्थिर ओळीत u8 pci_bdf_to_devfn(u16 bdf) {
    परत bdf & 0xff;
}
स्थिर ओळीत u16 pci_bdf_to_busdev(u16 bdf) {
    परत bdf & ~0x07;
}
स्थिर ओळीत u8 pci_bdf_to_dev(u16 bdf) {
    परत (bdf >> 3) & 0x1f;
}
स्थिर ओळीत u8 pci_bdf_to_fn(u16 bdf) {
    परत bdf & 0x07;
}
स्थिर ओळीत u16 pci_to_bdf(पूर्णांक bus, पूर्णांक dev, पूर्णांक fn) {
    परत (bus<<8) | (dev<<3) | fn;
}
स्थिर ओळीत u16 pci_bus_devfn_to_bdf(पूर्णांक bus, u16 devfn) {
    परत (bus << 8) | devfn;
}

#define pci_ioconfig_foreachbdf(BDF, BUS)                               \
    साठी (BDF=pci_ioconfig_next(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS)) \
         ; BDF >= 0                                                     \
         ; BDF=pci_ioconfig_next(BDF, (BUS)))

#define foreachbdf(BDF, BUS)                                    \
    साठी (BDF=pci_next(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS))  \
         ; BDF >= 0                                             \
         ; BDF=pci_next(BDF, (BUS)))

// standard PCI configration access mechanism
शून्य pci_ioconfig_writel(u16 bdf, u32 addr, u32 val);
शून्य pci_ioconfig_writew(u16 bdf, u32 addr, u16 val);
शून्य pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_ioconfig_readl(u16 bdf, u32 addr);
u16 pci_ioconfig_readw(u16 bdf, u32 addr);
u8 pci_ioconfig_readb(u16 bdf, u32 addr);
पूर्णांक pci_ioconfig_next(पूर्णांक bdf, पूर्णांक bus);

// PCI configuration access using either PCI CAM or PCIe ECAM
शून्य pci_config_writel(u16 bdf, u32 addr, u32 val);
शून्य pci_config_writew(u16 bdf, u32 addr, u16 val);
शून्य pci_config_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_config_readl(u16 bdf, u32 addr);
u16 pci_config_readw(u16 bdf, u32 addr);
u8 pci_config_readb(u16 bdf, u32 addr);
शून्य pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on);
u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap);
पूर्णांक pci_next(पूर्णांक bdf, पूर्णांक bus);

शून्य pci_enable_mmconfig(u64 addr, स्थिरांक अक्षर *name);
पूर्णांक pci_probe_host(शून्य);
शून्य pci_reboot(शून्य);

#endif // pci.h

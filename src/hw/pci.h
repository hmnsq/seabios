#ifndef __PCI_H
#define __PCI_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

#define PORT_PCI_REBOOT        0x0cf9

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u8 pci_bdf_to_bus(u16 bdf) {
    ಪ್ರತ್ಯಾಗಮ bdf >> 8;
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u8 pci_bdf_to_devfn(u16 bdf) {
    ಪ್ರತ್ಯಾಗಮ bdf & 0xff;
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u16 pci_bdf_to_busdev(u16 bdf) {
    ಪ್ರತ್ಯಾಗಮ bdf & ~0x07;
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u8 pci_bdf_to_dev(u16 bdf) {
    ಪ್ರತ್ಯಾಗಮ (bdf >> 3) & 0x1f;
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u8 pci_bdf_to_fn(u16 bdf) {
    ಪ್ರತ್ಯಾಗಮ bdf & 0x07;
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u16 pci_to_bdf(ಪೂರ್ಣಾಙ್ಕ bus, ಪೂರ್ಣಾಙ್ಕ dev, ಪೂರ್ಣಾಙ್ಕ fn) {
    ಪ್ರತ್ಯಾಗಮ (bus<<8) | (dev<<3) | fn;
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ u16 pci_bus_devfn_to_bdf(ಪೂರ್ಣಾಙ್ಕ bus, u16 devfn) {
    ಪ್ರತ್ಯಾಗಮ (bus << 8) | devfn;
}

#define pci_ioconfig_foreachbdf(BDF, BUS)                               \
    ಪ್ರತಿ (BDF=pci_ioconfig_next(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS)) \
         ; BDF >= 0                                                     \
         ; BDF=pci_ioconfig_next(BDF, (BUS)))

#define foreachbdf(BDF, BUS)                                    \
    ಪ್ರತಿ (BDF=pci_next(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS))  \
         ; BDF >= 0                                             \
         ; BDF=pci_next(BDF, (BUS)))

// standard PCI configration access mechanism
ಶೂನ್ಯ pci_ioconfig_writel(u16 bdf, u32 addr, u32 val);
ಶೂನ್ಯ pci_ioconfig_writew(u16 bdf, u32 addr, u16 val);
ಶೂನ್ಯ pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_ioconfig_readl(u16 bdf, u32 addr);
u16 pci_ioconfig_readw(u16 bdf, u32 addr);
u8 pci_ioconfig_readb(u16 bdf, u32 addr);
ಪೂರ್ಣಾಙ್ಕ pci_ioconfig_next(ಪೂರ್ಣಾಙ್ಕ bdf, ಪೂರ್ಣಾಙ್ಕ bus);

// PCI configuration access using either PCI CAM or PCIe ECAM
ಶೂನ್ಯ pci_config_writel(u16 bdf, u32 addr, u32 val);
ಶೂನ್ಯ pci_config_writew(u16 bdf, u32 addr, u16 val);
ಶೂನ್ಯ pci_config_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_config_readl(u16 bdf, u32 addr);
u16 pci_config_readw(u16 bdf, u32 addr);
u8 pci_config_readb(u16 bdf, u32 addr);
ಶೂನ್ಯ pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on);
u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap);
ಪೂರ್ಣಾಙ್ಕ pci_next(ಪೂರ್ಣಾಙ್ಕ bdf, ಪೂರ್ಣಾಙ್ಕ bus);

ಶೂನ್ಯ pci_enable_mmconfig(u64 addr, ಧ್ರುವ ವರ್ಣ *name);
ಪೂರ್ಣಾಙ್ಕ pci_probe_host(ಶೂನ್ಯ);
ಶೂನ್ಯ pci_reboot(ಶೂನ್ಯ);

#endif // pci.h

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __PCI_H
#ವ್ಯಾಖ್ಯಾನಿಸು __PCI_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32

#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_PCI_REBOOT        0x0cf9

ಸ್ಥಿರ ಸಾಲೊಳಗೆ u8 pci_bdf_ಗಮ್ಯ_bus(u16 bdf) {
    ಹಿಂತಿರುಗಿ bdf >> 8;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u8 pci_bdf_ಗಮ್ಯ_devfn(u16 bdf) {
    ಹಿಂತಿರುಗಿ bdf & 0xff;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u16 pci_bdf_ಗಮ್ಯ_busdev(u16 bdf) {
    ಹಿಂತಿರುಗಿ bdf & ~0x07;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u8 pci_bdf_ಗಮ್ಯ_dev(u16 bdf) {
    ಹಿಂತಿರುಗಿ (bdf >> 3) & 0x1f;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u8 pci_bdf_ಗಮ್ಯ_fn(u16 bdf) {
    ಹಿಂತಿರುಗಿ bdf & 0x07;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u16 pci_ಗಮ್ಯ_bdf(ಪೂರ್ಣಾಂಕ bus, ಪೂರ್ಣಾಂಕ dev, ಪೂರ್ಣಾಂಕ fn) {
    ಹಿಂತಿರುಗಿ (bus<<8) | (dev<<3) | fn;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ u16 pci_bus_devfn_ಗಮ್ಯ_bdf(ಪೂರ್ಣಾಂಕ bus, u16 devfn) {
    ಹಿಂತಿರುಗಿ (bus << 8) | devfn;
}

#ವ್ಯಾಖ್ಯಾನಿಸು pci_ioconfig_foreachbdf(BDF, BUS)                               \
    ಗಾಗಿ (BDF=pci_ioconfig_ಮುಂದಿನ(pci_bus_devfn_ಗಮ್ಯ_bdf((BUS), 0)-1, (BUS)) \
         ; BDF >= 0                                                     \
         ; BDF=pci_ioconfig_ಮುಂದಿನ(BDF, (BUS)))

#ವ್ಯಾಖ್ಯಾನಿಸು foreachbdf(BDF, BUS)                                    \
    ಗಾಗಿ (BDF=pci_ಮುಂದಿನ(pci_bus_devfn_ಗಮ್ಯ_bdf((BUS), 0)-1, (BUS))  \
         ; BDF >= 0                                             \
         ; BDF=pci_ಮುಂದಿನ(BDF, (BUS)))

// standard PCI configration access mechanism
ಶೂನ್ಯ pci_ioconfig_writel(u16 bdf, u32 addr, u32 val);
ಶೂನ್ಯ pci_ioconfig_writew(u16 bdf, u32 addr, u16 val);
ಶೂನ್ಯ pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_ioconfig_readl(u16 bdf, u32 addr);
u16 pci_ioconfig_readw(u16 bdf, u32 addr);
u8 pci_ioconfig_readb(u16 bdf, u32 addr);
ಪೂರ್ಣಾಂಕ pci_ioconfig_ಮುಂದಿನ(ಪೂರ್ಣಾಂಕ bdf, ಪೂರ್ಣಾಂಕ bus);

// PCI configuration access using either PCI CAM or PCIe ECAM
ಶೂನ್ಯ pci_config_writel(u16 bdf, u32 addr, u32 val);
ಶೂನ್ಯ pci_config_writew(u16 bdf, u32 addr, u16 val);
ಶೂನ್ಯ pci_config_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_config_readl(u16 bdf, u32 addr);
u16 pci_config_readw(u16 bdf, u32 addr);
u8 pci_config_readb(u16 bdf, u32 addr);
ಶೂನ್ಯ pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on);
u8 pci_ಕಂಡುಹಿಡಿ_capability(u16 bdf, u8 cap_id, u8 cap);
ಪೂರ್ಣಾಂಕ pci_ಮುಂದಿನ(ಪೂರ್ಣಾಂಕ bdf, ಪೂರ್ಣಾಂಕ bus);

ಶೂನ್ಯ pci_enable_mmconfig(u64 addr, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name);
ಪೂರ್ಣಾಂಕ pci_probe_host(ಶೂನ್ಯ);
ಶೂನ್ಯ pci_reboot(ಶೂನ್ಯ);

#ಆದರೆ_ಅಂತ್ಯ // pci.h

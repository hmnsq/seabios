#अगर_अघोषित __PCI_H
#घोषणा __PCI_H

#समावेश "types.h" // u32

#घोषणा PORT_PCI_REBOOT        0x0cf9

अटल अंतरभूत u8 pci_bdf_to_bus(u16 bdf) {
    वापस bdf >> 8;
}
अटल अंतरभूत u8 pci_bdf_to_devfn(u16 bdf) {
    वापस bdf & 0xff;
}
अटल अंतरभूत u16 pci_bdf_to_busdev(u16 bdf) {
    वापस bdf & ~0x07;
}
अटल अंतरभूत u8 pci_bdf_to_dev(u16 bdf) {
    वापस (bdf >> 3) & 0x1f;
}
अटल अंतरभूत u8 pci_bdf_to_fn(u16 bdf) {
    वापस bdf & 0x07;
}
अटल अंतरभूत u16 pci_to_bdf(पूर्णांक bus, पूर्णांक dev, पूर्णांक fn) {
    वापस (bus<<8) | (dev<<3) | fn;
}
अटल अंतरभूत u16 pci_bus_devfn_to_bdf(पूर्णांक bus, u16 devfn) {
    वापस (bus << 8) | devfn;
}

#घोषणा pci_ioconfig_foreachbdf(BDF, BUS)                               \
    क्रम (BDF=pci_ioconfig_अग्र(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS)) \
         ; BDF >= 0                                                     \
         ; BDF=pci_ioconfig_अग्र(BDF, (BUS)))

#घोषणा foreachbdf(BDF, BUS)                                    \
    क्रम (BDF=pci_अग्र(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS))  \
         ; BDF >= 0                                             \
         ; BDF=pci_अग्र(BDF, (BUS)))

// standard PCI configration access mechanism
व्योम pci_ioconfig_writel(u16 bdf, u32 addr, u32 val);
व्योम pci_ioconfig_writew(u16 bdf, u32 addr, u16 val);
व्योम pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_ioconfig_readl(u16 bdf, u32 addr);
u16 pci_ioconfig_readw(u16 bdf, u32 addr);
u8 pci_ioconfig_readb(u16 bdf, u32 addr);
पूर्णांक pci_ioconfig_अग्र(पूर्णांक bdf, पूर्णांक bus);

// PCI configuration access using either PCI CAM or PCIe ECAM
व्योम pci_config_writel(u16 bdf, u32 addr, u32 val);
व्योम pci_config_writew(u16 bdf, u32 addr, u16 val);
व्योम pci_config_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_config_readl(u16 bdf, u32 addr);
u16 pci_config_readw(u16 bdf, u32 addr);
u8 pci_config_readb(u16 bdf, u32 addr);
व्योम pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on);
u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap);
पूर्णांक pci_अग्र(पूर्णांक bdf, पूर्णांक bus);

व्योम pci_enable_mmconfig(u64 addr, स्थिर अक्षर *name);
पूर्णांक pci_probe_host(व्योम);
व्योम pci_reboot(व्योम);

#पूर्ण_अगर // pci.h

#ifndef __PCI_H
#define __PCI_H

#അന്തര്ഭാവ "types.h" // u32

#define PORT_PCI_REBOOT        0x0cf9

സ്ഥിതം വരിയിൽ u8 pci_bdf_to_bus(u16 bdf) {
    തിരികെ bdf >> 8;
}
സ്ഥിതം വരിയിൽ u8 pci_bdf_to_devfn(u16 bdf) {
    തിരികെ bdf & 0xff;
}
സ്ഥിതം വരിയിൽ u16 pci_bdf_to_busdev(u16 bdf) {
    തിരികെ bdf & ~0x07;
}
സ്ഥിതം വരിയിൽ u8 pci_bdf_to_dev(u16 bdf) {
    തിരികെ (bdf >> 3) & 0x1f;
}
സ്ഥിതം വരിയിൽ u8 pci_bdf_to_fn(u16 bdf) {
    തിരികെ bdf & 0x07;
}
സ്ഥിതം വരിയിൽ u16 pci_to_bdf(പൂർണ്ണസംഖ്യ bus, പൂർണ്ണസംഖ്യ dev, പൂർണ്ണസംഖ്യ fn) {
    തിരികെ (bus<<8) | (dev<<3) | fn;
}
സ്ഥിതം വരിയിൽ u16 pci_bus_devfn_to_bdf(പൂർണ്ണസംഖ്യ bus, u16 devfn) {
    തിരികെ (bus << 8) | devfn;
}

#define pci_ioconfig_foreachbdf(BDF, BUS)                               \
    വേണ്ടി (BDF=pci_ioconfig_next(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS)) \
         ; BDF >= 0                                                     \
         ; BDF=pci_ioconfig_next(BDF, (BUS)))

#define foreachbdf(BDF, BUS)                                    \
    വേണ്ടി (BDF=pci_next(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS))  \
         ; BDF >= 0                                             \
         ; BDF=pci_next(BDF, (BUS)))

// standard PCI configration access mechanism
ശൂന്യം pci_ioconfig_writel(u16 bdf, u32 addr, u32 val);
ശൂന്യം pci_ioconfig_writew(u16 bdf, u32 addr, u16 val);
ശൂന്യം pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_ioconfig_readl(u16 bdf, u32 addr);
u16 pci_ioconfig_readw(u16 bdf, u32 addr);
u8 pci_ioconfig_readb(u16 bdf, u32 addr);
പൂർണ്ണസംഖ്യ pci_ioconfig_next(പൂർണ്ണസംഖ്യ bdf, പൂർണ്ണസംഖ്യ bus);

// PCI configuration access using either PCI CAM or PCIe ECAM
ശൂന്യം pci_config_writel(u16 bdf, u32 addr, u32 val);
ശൂന്യം pci_config_writew(u16 bdf, u32 addr, u16 val);
ശൂന്യം pci_config_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_config_readl(u16 bdf, u32 addr);
u16 pci_config_readw(u16 bdf, u32 addr);
u8 pci_config_readb(u16 bdf, u32 addr);
ശൂന്യം pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on);
u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap);
പൂർണ്ണസംഖ്യ pci_next(പൂർണ്ണസംഖ്യ bdf, പൂർണ്ണസംഖ്യ bus);

ശൂന്യം pci_enable_mmconfig(u64 addr, സ്ഥിരം അക്ഷരം *name);
പൂർണ്ണസംഖ്യ pci_probe_host(ശൂന്യം);
ശൂന്യം pci_reboot(ശൂന്യം);

#endif // pci.h

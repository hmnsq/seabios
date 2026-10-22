#ifndef __PCI_H
#define __PCI_H

#ଅନ୍ତର୍ଭାଵ "types.h" // u32

#define PORT_PCI_REBOOT        0x0cf9

ସ୍ଥିର ଧାଡ଼ିଭିତରେ u8 pci_bdf_to_bus(u16 bdf) {
    ଫେରନ୍ତୁ bdf >> 8;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u8 pci_bdf_to_devfn(u16 bdf) {
    ଫେରନ୍ତୁ bdf & 0xff;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u16 pci_bdf_to_busdev(u16 bdf) {
    ଫେରନ୍ତୁ bdf & ~0x07;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u8 pci_bdf_to_dev(u16 bdf) {
    ଫେରନ୍ତୁ (bdf >> 3) & 0x1f;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u8 pci_bdf_to_fn(u16 bdf) {
    ଫେରନ୍ତୁ bdf & 0x07;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u16 pci_to_bdf(ପୂର୍ଣ୍ଣସଂଖ୍ୟା bus, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dev, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fn) {
    ଫେରନ୍ତୁ (bus<<8) | (dev<<3) | fn;
}
ସ୍ଥିର ଧାଡ଼ିଭିତରେ u16 pci_bus_devfn_to_bdf(ପୂର୍ଣ୍ଣସଂଖ୍ୟା bus, u16 devfn) {
    ଫେରନ୍ତୁ (bus << 8) | devfn;
}

#define pci_ioconfig_foreachbdf(BDF, BUS)                               \
    ପାଇଁ (BDF=pci_ioconfig_next(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS)) \
         ; BDF >= 0                                                     \
         ; BDF=pci_ioconfig_next(BDF, (BUS)))

#define foreachbdf(BDF, BUS)                                    \
    ପାଇଁ (BDF=pci_next(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS))  \
         ; BDF >= 0                                             \
         ; BDF=pci_next(BDF, (BUS)))

// standard PCI configration access mechanism
ଶୂନ୍ୟ pci_ioconfig_writel(u16 bdf, u32 addr, u32 val);
ଶୂନ୍ୟ pci_ioconfig_writew(u16 bdf, u32 addr, u16 val);
ଶୂନ୍ୟ pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_ioconfig_readl(u16 bdf, u32 addr);
u16 pci_ioconfig_readw(u16 bdf, u32 addr);
u8 pci_ioconfig_readb(u16 bdf, u32 addr);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା pci_ioconfig_next(ପୂର୍ଣ୍ଣସଂଖ୍ୟା bdf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bus);

// PCI configuration access using either PCI CAM or PCIe ECAM
ଶୂନ୍ୟ pci_config_writel(u16 bdf, u32 addr, u32 val);
ଶୂନ୍ୟ pci_config_writew(u16 bdf, u32 addr, u16 val);
ଶୂନ୍ୟ pci_config_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_config_readl(u16 bdf, u32 addr);
u16 pci_config_readw(u16 bdf, u32 addr);
u8 pci_config_readb(u16 bdf, u32 addr);
ଶୂନ୍ୟ pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on);
u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା pci_next(ପୂର୍ଣ୍ଣସଂଖ୍ୟା bdf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bus);

ଶୂନ୍ୟ pci_enable_mmconfig(u64 addr, ଅଚଳ ଅକ୍ଷର *name);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା pci_probe_host(ଶୂନ୍ୟ);
ଶୂନ୍ୟ pci_reboot(ଶୂନ୍ୟ);

#endif // pci.h

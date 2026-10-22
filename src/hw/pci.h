#ifndef __PCI_H
#define __PCI_H

#અન્તર્ભાવ "types.h" // u32

#define PORT_PCI_REBOOT        0x0cf9

સ્થિર અંતઃરેખ u8 pci_bdf_to_bus(u16 bdf) {
    પાછા bdf >> 8;
}
સ્થિર અંતઃરેખ u8 pci_bdf_to_devfn(u16 bdf) {
    પાછા bdf & 0xff;
}
સ્થિર અંતઃરેખ u16 pci_bdf_to_busdev(u16 bdf) {
    પાછા bdf & ~0x07;
}
સ્થિર અંતઃરેખ u8 pci_bdf_to_dev(u16 bdf) {
    પાછા (bdf >> 3) & 0x1f;
}
સ્થિર અંતઃરેખ u8 pci_bdf_to_fn(u16 bdf) {
    પાછા bdf & 0x07;
}
સ્થિર અંતઃરેખ u16 pci_to_bdf(પૂર્ણાંક bus, પૂર્ણાંક dev, પૂર્ણાંક fn) {
    પાછા (bus<<8) | (dev<<3) | fn;
}
સ્થિર અંતઃરેખ u16 pci_bus_devfn_to_bdf(પૂર્ણાંક bus, u16 devfn) {
    પાછા (bus << 8) | devfn;
}

#define pci_ioconfig_foreachbdf(BDF, BUS)                               \
    માટે (BDF=pci_ioconfig_next(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS)) \
         ; BDF >= 0                                                     \
         ; BDF=pci_ioconfig_next(BDF, (BUS)))

#define foreachbdf(BDF, BUS)                                    \
    માટે (BDF=pci_next(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS))  \
         ; BDF >= 0                                             \
         ; BDF=pci_next(BDF, (BUS)))

// standard PCI configration access mechanism
શૂન્ય pci_ioconfig_writel(u16 bdf, u32 addr, u32 val);
શૂન્ય pci_ioconfig_writew(u16 bdf, u32 addr, u16 val);
શૂન્ય pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_ioconfig_readl(u16 bdf, u32 addr);
u16 pci_ioconfig_readw(u16 bdf, u32 addr);
u8 pci_ioconfig_readb(u16 bdf, u32 addr);
પૂર્ણાંક pci_ioconfig_next(પૂર્ણાંક bdf, પૂર્ણાંક bus);

// PCI configuration access using either PCI CAM or PCIe ECAM
શૂન્ય pci_config_writel(u16 bdf, u32 addr, u32 val);
શૂન્ય pci_config_writew(u16 bdf, u32 addr, u16 val);
શૂન્ય pci_config_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_config_readl(u16 bdf, u32 addr);
u16 pci_config_readw(u16 bdf, u32 addr);
u8 pci_config_readb(u16 bdf, u32 addr);
શૂન્ય pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on);
u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap);
પૂર્ણાંક pci_next(પૂર્ણાંક bdf, પૂર્ણાંક bus);

શૂન્ય pci_enable_mmconfig(u64 addr, અચલ અક્ષર *name);
પૂર્ણાંક pci_probe_host(શૂન્ય);
શૂન્ય pci_reboot(શૂન્ય);

#endif // pci.h

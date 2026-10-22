#ifndef __PCI_H
#define __PCI_H

#অন্তর্ভাব "types.h" // u32

#define PORT_PCI_REBOOT        0x0cf9

স্থির অন্তঃরেখ u8 pci_bdf_to_bus(u16 bdf) {
    ফেরত bdf >> 8;
}
স্থির অন্তঃরেখ u8 pci_bdf_to_devfn(u16 bdf) {
    ফেরত bdf & 0xff;
}
স্থির অন্তঃরেখ u16 pci_bdf_to_busdev(u16 bdf) {
    ফেরত bdf & ~0x07;
}
স্থির অন্তঃরেখ u8 pci_bdf_to_dev(u16 bdf) {
    ফেরত (bdf >> 3) & 0x1f;
}
স্থির অন্তঃরেখ u8 pci_bdf_to_fn(u16 bdf) {
    ফেরত bdf & 0x07;
}
স্থির অন্তঃরেখ u16 pci_to_bdf(পূর্ণসংখ্যা bus, পূর্ণসংখ্যা dev, পূর্ণসংখ্যা fn) {
    ফেরত (bus<<8) | (dev<<3) | fn;
}
স্থির অন্তঃরেখ u16 pci_bus_devfn_to_bdf(পূর্ণসংখ্যা bus, u16 devfn) {
    ফেরত (bus << 8) | devfn;
}

#define pci_ioconfig_foreachbdf(BDF, BUS)                               \
    জন্য (BDF=pci_ioconfig_পরবর্তী(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS)) \
         ; BDF >= 0                                                     \
         ; BDF=pci_ioconfig_পরবর্তী(BDF, (BUS)))

#define foreachbdf(BDF, BUS)                                    \
    জন্য (BDF=pci_পরবর্তী(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS))  \
         ; BDF >= 0                                             \
         ; BDF=pci_পরবর্তী(BDF, (BUS)))

// standard PCI configration access mechanism
শূন্য pci_ioconfig_writel(u16 bdf, u32 addr, u32 val);
শূন্য pci_ioconfig_writew(u16 bdf, u32 addr, u16 val);
শূন্য pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_ioconfig_readl(u16 bdf, u32 addr);
u16 pci_ioconfig_readw(u16 bdf, u32 addr);
u8 pci_ioconfig_readb(u16 bdf, u32 addr);
পূর্ণসংখ্যা pci_ioconfig_পরবর্তী(পূর্ণসংখ্যা bdf, পূর্ণসংখ্যা bus);

// PCI configuration access using either PCI CAM or PCIe ECAM
শূন্য pci_config_writel(u16 bdf, u32 addr, u32 val);
শূন্য pci_config_writew(u16 bdf, u32 addr, u16 val);
শূন্য pci_config_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_config_readl(u16 bdf, u32 addr);
u16 pci_config_readw(u16 bdf, u32 addr);
u8 pci_config_readb(u16 bdf, u32 addr);
শূন্য pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on);
u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap);
পূর্ণসংখ্যা pci_পরবর্তী(পূর্ণসংখ্যা bdf, পূর্ণসংখ্যা bus);

শূন্য pci_enable_mmconfig(u64 addr, ধ্রুবক অক্ষর *name);
পূর্ণসংখ্যা pci_probe_host(শূন্য);
শূন্য pci_reboot(শূন্য);

#endif // pci.h

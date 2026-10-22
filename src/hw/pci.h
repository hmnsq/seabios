#ifndef __PCI_H
#define __PCI_H

#شامل "types.h" // u32

#define PORT_PCI_REBOOT        0x0cf9

ایستا درونخطی u8 pci_bdf_to_bus(u16 bdf) {
    بازگشت bdf >> 8;
}
ایستا درونخطی u8 pci_bdf_to_devfn(u16 bdf) {
    بازگشت bdf & 0xff;
}
ایستا درونخطی u16 pci_bdf_to_busdev(u16 bdf) {
    بازگشت bdf & ~0x07;
}
ایستا درونخطی u8 pci_bdf_to_dev(u16 bdf) {
    بازگشت (bdf >> 3) & 0x1f;
}
ایستا درونخطی u8 pci_bdf_to_fn(u16 bdf) {
    بازگشت bdf & 0x07;
}
ایستا درونخطی u16 pci_to_bdf(صحیح bus, صحیح dev, صحیح fn) {
    بازگشت (bus<<8) | (dev<<3) | fn;
}
ایستا درونخطی u16 pci_bus_devfn_to_bdf(صحیح bus, u16 devfn) {
    بازگشت (bus << 8) | devfn;
}

#define pci_ioconfig_foreachbdf(BDF, BUS)                               \
    برای (BDF=pci_ioconfig_next(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS)) \
         ; BDF >= 0                                                     \
         ; BDF=pci_ioconfig_next(BDF, (BUS)))

#define foreachbdf(BDF, BUS)                                    \
    برای (BDF=pci_next(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS))  \
         ; BDF >= 0                                             \
         ; BDF=pci_next(BDF, (BUS)))

// standard PCI configration access mechanism
تهی pci_ioconfig_writel(u16 bdf, u32 addr, u32 val);
تهی pci_ioconfig_writew(u16 bdf, u32 addr, u16 val);
تهی pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_ioconfig_readl(u16 bdf, u32 addr);
u16 pci_ioconfig_readw(u16 bdf, u32 addr);
u8 pci_ioconfig_readb(u16 bdf, u32 addr);
صحیح pci_ioconfig_next(صحیح bdf, صحیح bus);

// PCI configuration access using either PCI CAM or PCIe ECAM
تهی pci_config_writel(u16 bdf, u32 addr, u32 val);
تهی pci_config_writew(u16 bdf, u32 addr, u16 val);
تهی pci_config_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_config_readl(u16 bdf, u32 addr);
u16 pci_config_readw(u16 bdf, u32 addr);
u8 pci_config_readb(u16 bdf, u32 addr);
تهی pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on);
u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap);
صحیح pci_next(صحیح bdf, صحیح bus);

تهی pci_enable_mmconfig(u64 addr, ثابت نویسه *name);
صحیح pci_probe_host(تهی);
تهی pci_reboot(تهی);

#endif // pci.h

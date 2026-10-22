#که_نه_تعریف_شوی __PCI_H
#تعریف_کړه __PCI_H

#ضمن "types.h" // u32

#تعریف_کړه پورټ_PCI_REBOOT        0x0cf9

ساكن مضمن u8 pci_bdf_منزل_bus(u16 bdf) {
    أرجع bdf >> 8;
}
ساكن مضمن u8 pci_bdf_منزل_devfn(u16 bdf) {
    أرجع bdf & 0xff;
}
ساكن مضمن u16 pci_bdf_منزل_busdev(u16 bdf) {
    أرجع bdf & ~0x07;
}
ساكن مضمن u8 pci_bdf_منزل_dev(u16 bdf) {
    أرجع (bdf >> 3) & 0x1f;
}
ساكن مضمن u8 pci_bdf_منزل_fn(u16 bdf) {
    أرجع bdf & 0x07;
}
ساكن مضمن u16 pci_منزل_bdf(صحيح bus, صحيح dev, صحيح fn) {
    أرجع (bus<<8) | (dev<<3) | fn;
}
ساكن مضمن u16 pci_bus_devfn_منزل_bdf(صحيح bus, u16 devfn) {
    أرجع (bus << 8) | devfn;
}

#تعریف_کړه pci_ioconfig_foreachbdf(BDF, BUS)                               \
    لكل (BDF=pci_ioconfig_بل(pci_bus_devfn_منزل_bdf((BUS), 0)-1, (BUS)) \
         ; BDF >= 0                                                     \
         ; BDF=pci_ioconfig_بل(BDF, (BUS)))

#تعریف_کړه foreachbdf(BDF, BUS)                                    \
    لكل (BDF=pci_بل(pci_bus_devfn_منزل_bdf((BUS), 0)-1, (BUS))  \
         ; BDF >= 0                                             \
         ; BDF=pci_بل(BDF, (BUS)))

// standard PCI configration access mechanism
فراغ pci_ioconfig_writel(u16 bdf, u32 addr, u32 val);
فراغ pci_ioconfig_writew(u16 bdf, u32 addr, u16 val);
فراغ pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_ioconfig_readl(u16 bdf, u32 addr);
u16 pci_ioconfig_readw(u16 bdf, u32 addr);
u8 pci_ioconfig_readb(u16 bdf, u32 addr);
صحيح pci_ioconfig_بل(صحيح bdf, صحيح bus);

// PCI configuration access using either PCI CAM or PCIe ECAM
فراغ pci_config_writel(u16 bdf, u32 addr, u32 val);
فراغ pci_config_writew(u16 bdf, u32 addr, u16 val);
فراغ pci_config_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_config_readl(u16 bdf, u32 addr);
u16 pci_config_readw(u16 bdf, u32 addr);
u8 pci_config_readb(u16 bdf, u32 addr);
فراغ pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on);
u8 pci_پیدا_کړه_capability(u16 bdf, u8 cap_id, u8 cap);
صحيح pci_بل(صحيح bdf, صحيح bus);

فراغ pci_enable_mmconfig(u64 addr, ثابت حرف *name);
صحيح pci_probe_host(فراغ);
فراغ pci_reboot(فراغ);

#د_که_پای // pci.h

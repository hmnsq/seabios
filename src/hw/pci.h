#جے_بنا_تعریف __PCI_H
#تعریف_کرو __PCI_H

#ضمن "types.h" // u32

#تعریف_کرو پورٹ_PCI_REBOOT        0x0cf9

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

#تعریف_کرو pci_ioconfig_foreachbdf(BDF, BUS)                               \
    لكل (BDF=pci_ioconfig_اگلا(pci_bus_devfn_منزل_bdf((BUS), 0)-1, (BUS)) \
         ; BDF >= 0                                                     \
         ; BDF=pci_ioconfig_اگلا(BDF, (BUS)))

#تعریف_کرو foreachbdf(BDF, BUS)                                    \
    لكل (BDF=pci_اگلا(pci_bus_devfn_منزل_bdf((BUS), 0)-1, (BUS))  \
         ; BDF >= 0                                             \
         ; BDF=pci_اگلا(BDF, (BUS)))

// standard PCI configration access mechanism
فراغ pci_ioconfig_writel(u16 bdf, u32 addr, u32 val);
فراغ pci_ioconfig_writew(u16 bdf, u32 addr, u16 val);
فراغ pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_ioconfig_readl(u16 bdf, u32 addr);
u16 pci_ioconfig_readw(u16 bdf, u32 addr);
u8 pci_ioconfig_readb(u16 bdf, u32 addr);
صحيح pci_ioconfig_اگلا(صحيح bdf, صحيح bus);

// PCI configuration access using either PCI CAM or PCIe ECAM
فراغ pci_config_writel(u16 bdf, u32 addr, u32 val);
فراغ pci_config_writew(u16 bdf, u32 addr, u16 val);
فراغ pci_config_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_config_readl(u16 bdf, u32 addr);
u16 pci_config_readw(u16 bdf, u32 addr);
u8 pci_config_readb(u16 bdf, u32 addr);
فراغ pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on);
u8 pci_لبھو_capability(u16 bdf, u8 cap_id, u8 cap);
صحيح pci_اگلا(صحيح bdf, صحيح bus);

فراغ pci_enable_mmconfig(u64 addr, ثابت حرف *name);
صحيح pci_probe_host(فراغ);
فراغ pci_reboot(فراغ);

#جے_انت // pci.h

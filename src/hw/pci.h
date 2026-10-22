#اگر_غیر_تعریف_شدہ __PCI_H
#تعریف_کرو __PCI_H

#شامل "types.h" // u32

#تعریف_کرو پورٹ_PCI_REBOOT        0x0cf9

ساکن اندرونی u8 pci_bdf_مقصد_bus(u16 bdf) {
    واپس bdf >> 8;
}
ساکن اندرونی u8 pci_bdf_مقصد_devfn(u16 bdf) {
    واپس bdf & 0xff;
}
ساکن اندرونی u16 pci_bdf_مقصد_busdev(u16 bdf) {
    واپس bdf & ~0x07;
}
ساکن اندرونی u8 pci_bdf_مقصد_dev(u16 bdf) {
    واپس (bdf >> 3) & 0x1f;
}
ساکن اندرونی u8 pci_bdf_مقصد_fn(u16 bdf) {
    واپس bdf & 0x07;
}
ساکن اندرونی u16 pci_مقصد_bdf(صحیح bus, صحیح dev, صحیح fn) {
    واپس (bus<<8) | (dev<<3) | fn;
}
ساکن اندرونی u16 pci_bus_devfn_مقصد_bdf(صحیح bus, u16 devfn) {
    واپس (bus << 8) | devfn;
}

#تعریف_کرو pci_ioconfig_foreachbdf(BDF, BUS)                               \
    برائے (BDF=pci_ioconfig_اگلا(pci_bus_devfn_مقصد_bdf((BUS), 0)-1, (BUS)) \
         ; BDF >= 0                                                     \
         ; BDF=pci_ioconfig_اگلا(BDF, (BUS)))

#تعریف_کرو foreachbdf(BDF, BUS)                                    \
    برائے (BDF=pci_اگلا(pci_bus_devfn_مقصد_bdf((BUS), 0)-1, (BUS))  \
         ; BDF >= 0                                             \
         ; BDF=pci_اگلا(BDF, (BUS)))

// standard PCI configration access mechanism
خالی pci_ioconfig_writel(u16 bdf, u32 addr, u32 val);
خالی pci_ioconfig_writew(u16 bdf, u32 addr, u16 val);
خالی pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_ioconfig_readl(u16 bdf, u32 addr);
u16 pci_ioconfig_readw(u16 bdf, u32 addr);
u8 pci_ioconfig_readb(u16 bdf, u32 addr);
صحیح pci_ioconfig_اگلا(صحیح bdf, صحیح bus);

// PCI configuration access using either PCI CAM or PCIe ECAM
خالی pci_config_writel(u16 bdf, u32 addr, u32 val);
خالی pci_config_writew(u16 bdf, u32 addr, u16 val);
خالی pci_config_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_config_readl(u16 bdf, u32 addr);
u16 pci_config_readw(u16 bdf, u32 addr);
u8 pci_config_readb(u16 bdf, u32 addr);
خالی pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on);
u8 pci_ڈھونڈو_capability(u16 bdf, u8 cap_id, u8 cap);
صحیح pci_اگلا(صحیح bdf, صحیح bus);

خالی pci_enable_mmconfig(u64 addr, ثابت حرف *name);
صحیح pci_probe_host(خالی);
خالی pci_reboot(خالی);

#اگر_ختم // pci.h

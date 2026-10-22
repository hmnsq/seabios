#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __PCI_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __PCI_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_PCI_REBOOT        0x0cf9

ਸਥਿਰ ਸਤਰਅੰਦਰ u8 pci_bdf_ਮੰਜ਼ਿਲ_bus(u16 bdf) {
    ਵਾਪਸ bdf >> 8;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u8 pci_bdf_ਮੰਜ਼ਿਲ_devfn(u16 bdf) {
    ਵਾਪਸ bdf & 0xff;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u16 pci_bdf_ਮੰਜ਼ਿਲ_busdev(u16 bdf) {
    ਵਾਪਸ bdf & ~0x07;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u8 pci_bdf_ਮੰਜ਼ਿਲ_dev(u16 bdf) {
    ਵਾਪਸ (bdf >> 3) & 0x1f;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u8 pci_bdf_ਮੰਜ਼ਿਲ_fn(u16 bdf) {
    ਵਾਪਸ bdf & 0x07;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u16 pci_ਮੰਜ਼ਿਲ_bdf(ਪੂਰਨ bus, ਪੂਰਨ dev, ਪੂਰਨ fn) {
    ਵਾਪਸ (bus<<8) | (dev<<3) | fn;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ u16 pci_bus_devfn_ਮੰਜ਼ਿਲ_bdf(ਪੂਰਨ bus, u16 devfn) {
    ਵਾਪਸ (bus << 8) | devfn;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ pci_ioconfig_foreachbdf(BDF, BUS)                               \
    ਲਈ (BDF=pci_ioconfig_ਅਗਲਾ(pci_bus_devfn_ਮੰਜ਼ਿਲ_bdf((BUS), 0)-1, (BUS)) \
         ; BDF >= 0                                                     \
         ; BDF=pci_ioconfig_ਅਗਲਾ(BDF, (BUS)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ foreachbdf(BDF, BUS)                                    \
    ਲਈ (BDF=pci_ਅਗਲਾ(pci_bus_devfn_ਮੰਜ਼ਿਲ_bdf((BUS), 0)-1, (BUS))  \
         ; BDF >= 0                                             \
         ; BDF=pci_ਅਗਲਾ(BDF, (BUS)))

// standard PCI configration access mechanism
ਖਾਲੀ pci_ioconfig_writel(u16 bdf, u32 addr, u32 val);
ਖਾਲੀ pci_ioconfig_writew(u16 bdf, u32 addr, u16 val);
ਖਾਲੀ pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_ioconfig_readl(u16 bdf, u32 addr);
u16 pci_ioconfig_readw(u16 bdf, u32 addr);
u8 pci_ioconfig_readb(u16 bdf, u32 addr);
ਪੂਰਨ pci_ioconfig_ਅਗਲਾ(ਪੂਰਨ bdf, ਪੂਰਨ bus);

// PCI configuration access using either PCI CAM or PCIe ECAM
ਖਾਲੀ pci_config_writel(u16 bdf, u32 addr, u32 val);
ਖਾਲੀ pci_config_writew(u16 bdf, u32 addr, u16 val);
ਖਾਲੀ pci_config_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_config_readl(u16 bdf, u32 addr);
u16 pci_config_readw(u16 bdf, u32 addr);
u8 pci_config_readb(u16 bdf, u32 addr);
ਖਾਲੀ pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on);
u8 pci_ਲੱਭੋ_capability(u16 bdf, u8 cap_id, u8 cap);
ਪੂਰਨ pci_ਅਗਲਾ(ਪੂਰਨ bdf, ਪੂਰਨ bus);

ਖਾਲੀ pci_enable_mmconfig(u64 addr, ਅਚਲ ਅੱਖਰ *name);
ਪੂਰਨ pci_probe_host(ਖਾਲੀ);
ਖਾਲੀ pci_reboot(ਖਾਲੀ);

#ਜੇ_ਅੰਤ // pci.h

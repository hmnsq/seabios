#ifndef __PCI_H
#define __PCI_H

#அந்தர்பாவ "types.h" // u32

#define PORT_PCI_REBOOT        0x0cf9

நிலையான உள்வரி u8 pci_bdf_to_bus(u16 bdf) {
    திரும்பு bdf >> 8;
}
நிலையான உள்வரி u8 pci_bdf_to_devfn(u16 bdf) {
    திரும்பு bdf & 0xff;
}
நிலையான உள்வரி u16 pci_bdf_to_busdev(u16 bdf) {
    திரும்பு bdf & ~0x07;
}
நிலையான உள்வரி u8 pci_bdf_to_dev(u16 bdf) {
    திரும்பு (bdf >> 3) & 0x1f;
}
நிலையான உள்வரி u8 pci_bdf_to_fn(u16 bdf) {
    திரும்பு bdf & 0x07;
}
நிலையான உள்வரி u16 pci_to_bdf(முழு bus, முழு dev, முழு fn) {
    திரும்பு (bus<<8) | (dev<<3) | fn;
}
நிலையான உள்வரி u16 pci_bus_devfn_to_bdf(முழு bus, u16 devfn) {
    திரும்பு (bus << 8) | devfn;
}

#define pci_ioconfig_foreachbdf(BDF, BUS)                               \
    க்கு (BDF=pci_ioconfig_next(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS)) \
         ; BDF >= 0                                                     \
         ; BDF=pci_ioconfig_next(BDF, (BUS)))

#define foreachbdf(BDF, BUS)                                    \
    க்கு (BDF=pci_next(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS))  \
         ; BDF >= 0                                             \
         ; BDF=pci_next(BDF, (BUS)))

// standard PCI configration access mechanism
வெறுமை pci_ioconfig_writel(u16 bdf, u32 addr, u32 val);
வெறுமை pci_ioconfig_writew(u16 bdf, u32 addr, u16 val);
வெறுமை pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_ioconfig_readl(u16 bdf, u32 addr);
u16 pci_ioconfig_readw(u16 bdf, u32 addr);
u8 pci_ioconfig_readb(u16 bdf, u32 addr);
முழு pci_ioconfig_next(முழு bdf, முழு bus);

// PCI configuration access using either PCI CAM or PCIe ECAM
வெறுமை pci_config_writel(u16 bdf, u32 addr, u32 val);
வெறுமை pci_config_writew(u16 bdf, u32 addr, u16 val);
வெறுமை pci_config_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_config_readl(u16 bdf, u32 addr);
u16 pci_config_readw(u16 bdf, u32 addr);
u8 pci_config_readb(u16 bdf, u32 addr);
வெறுமை pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on);
u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap);
முழு pci_next(முழு bdf, முழு bus);

வெறுமை pci_enable_mmconfig(u64 addr, மாறிலி எழுத்து *name);
முழு pci_probe_host(வெறுமை);
வெறுமை pci_reboot(வெறுமை);

#endif // pci.h

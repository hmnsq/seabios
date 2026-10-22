#నిర్వచితం_కాకపోతే __PCI_H
#నిర్వచించు __PCI_H

#అన్తర్భావ "types.h" // u32

#నిర్వచించు పోర్టు_PCI_REBOOT        0x0cf9

స్థిర అంతఃవరుస u8 pci_bdf_గమ్యం_bus(u16 bdf) {
    తిరిగి bdf >> 8;
}
స్థిర అంతఃవరుస u8 pci_bdf_గమ్యం_devfn(u16 bdf) {
    తిరిగి bdf & 0xff;
}
స్థిర అంతఃవరుస u16 pci_bdf_గమ్యం_busdev(u16 bdf) {
    తిరిగి bdf & ~0x07;
}
స్థిర అంతఃవరుస u8 pci_bdf_గమ్యం_dev(u16 bdf) {
    తిరిగి (bdf >> 3) & 0x1f;
}
స్థిర అంతఃవరుస u8 pci_bdf_గమ్యం_fn(u16 bdf) {
    తిరిగి bdf & 0x07;
}
స్థిర అంతఃవరుస u16 pci_గమ్యం_bdf(పూర్ణాంకం bus, పూర్ణాంకం dev, పూర్ణాంకం fn) {
    తిరిగి (bus<<8) | (dev<<3) | fn;
}
స్థిర అంతఃవరుస u16 pci_bus_devfn_గమ్యం_bdf(పూర్ణాంకం bus, u16 devfn) {
    తిరిగి (bus << 8) | devfn;
}

#నిర్వచించు pci_ioconfig_foreachbdf(BDF, BUS)                               \
    కొరకు (BDF=pci_ioconfig_తదుపరి(pci_bus_devfn_గమ్యం_bdf((BUS), 0)-1, (BUS)) \
         ; BDF >= 0                                                     \
         ; BDF=pci_ioconfig_తదుపరి(BDF, (BUS)))

#నిర్వచించు foreachbdf(BDF, BUS)                                    \
    కొరకు (BDF=pci_తదుపరి(pci_bus_devfn_గమ్యం_bdf((BUS), 0)-1, (BUS))  \
         ; BDF >= 0                                             \
         ; BDF=pci_తదుపరి(BDF, (BUS)))

// standard PCI configration access mechanism
శూన్య pci_ioconfig_writel(u16 bdf, u32 addr, u32 val);
శూన్య pci_ioconfig_writew(u16 bdf, u32 addr, u16 val);
శూన్య pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_ioconfig_readl(u16 bdf, u32 addr);
u16 pci_ioconfig_readw(u16 bdf, u32 addr);
u8 pci_ioconfig_readb(u16 bdf, u32 addr);
పూర్ణాంకం pci_ioconfig_తదుపరి(పూర్ణాంకం bdf, పూర్ణాంకం bus);

// PCI configuration access using either PCI CAM or PCIe ECAM
శూన్య pci_config_writel(u16 bdf, u32 addr, u32 val);
శూన్య pci_config_writew(u16 bdf, u32 addr, u16 val);
శూన్య pci_config_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_config_readl(u16 bdf, u32 addr);
u16 pci_config_readw(u16 bdf, u32 addr);
u8 pci_config_readb(u16 bdf, u32 addr);
శూన్య pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on);
u8 pci_కనుగొను_capability(u16 bdf, u8 cap_id, u8 cap);
పూర్ణాంకం pci_తదుపరి(పూర్ణాంకం bdf, పూర్ణాంకం bus);

శూన్య pci_enable_mmconfig(u64 addr, స్థిరరాశి అక్షరం *name);
పూర్ణాంకం pci_probe_host(శూన్య);
శూన్య pci_reboot(శూన్య);

#అయితే_ముగింపు // pci.h

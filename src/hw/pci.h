#ifndef __PCI_H
#define __PCI_H

#include "types.h" // u32

#define PORT_PCI_REBOOT        0x0cf9

static anta_hstha u8 pci_bdf_to_bus(u16 bdf) {
    คืน bdf >> 8;
}
static anta_hstha u8 pci_bdf_to_devfn(u16 bdf) {
    คืน bdf & 0xff;
}
static anta_hstha u16 pci_bdf_to_busdev(u16 bdf) {
    คืน bdf & ~0x07;
}
static anta_hstha u8 pci_bdf_to_dev(u16 bdf) {
    คืน (bdf >> 3) & 0x1f;
}
static anta_hstha u8 pci_bdf_to_fn(u16 bdf) {
    คืน bdf & 0x07;
}
static anta_hstha u16 pci_to_bdf(จำนวนเต็ม bus, จำนวนเต็ม dev, จำนวนเต็ม fn) {
    คืน (bus<<8) | (dev<<3) | fn;
}
static anta_hstha u16 pci_bus_devfn_to_bdf(จำนวนเต็ม bus, u16 devfn) {
    คืน (bus << 8) | devfn;
}

#define pci_ioconfig_foreachbdf(BDF, BUS)                               \
    สำหรับ (BDF=pci_ioconfig_next(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS)) \
         ; BDF >= 0                                                     \
         ; BDF=pci_ioconfig_next(BDF, (BUS)))

#define foreachbdf(BDF, BUS)                                    \
    สำหรับ (BDF=pci_next(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS))  \
         ; BDF >= 0                                             \
         ; BDF=pci_next(BDF, (BUS)))

// standard PCI configration access mechanism
void pci_ioconfig_writel(u16 bdf, u32 addr, u32 val);
void pci_ioconfig_writew(u16 bdf, u32 addr, u16 val);
void pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_ioconfig_readl(u16 bdf, u32 addr);
u16 pci_ioconfig_readw(u16 bdf, u32 addr);
u8 pci_ioconfig_readb(u16 bdf, u32 addr);
จำนวนเต็ม pci_ioconfig_next(จำนวนเต็ม bdf, จำนวนเต็ม bus);

// PCI configuration access using either PCI CAM or PCIe ECAM
void pci_config_writel(u16 bdf, u32 addr, u32 val);
void pci_config_writew(u16 bdf, u32 addr, u16 val);
void pci_config_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_config_readl(u16 bdf, u32 addr);
u16 pci_config_readw(u16 bdf, u32 addr);
u8 pci_config_readb(u16 bdf, u32 addr);
void pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on);
u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap);
จำนวนเต็ม pci_next(จำนวนเต็ม bdf, จำนวนเต็ม bus);

void pci_enable_mmconfig(u64 addr, const var_na *name);
จำนวนเต็ม pci_probe_host(void);
void pci_reboot(void);

#endif // pci.h

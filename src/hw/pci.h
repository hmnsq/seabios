#ifndef __PCI_H
#define __PCI_H

#include "types.h" // u32

#define PORT_PCI_REBOOT        0x0cf9

static anta_hstha u8 pci_bdf_to_bus(u16 bdf) {
    ᰀᰧᰌ bdf >> 8;
}
static anta_hstha u8 pci_bdf_to_devfn(u16 bdf) {
    ᰀᰧᰌ bdf & 0xff;
}
static anta_hstha u16 pci_bdf_to_busdev(u16 bdf) {
    ᰀᰧᰌ bdf & ~0x07;
}
static anta_hstha u8 pci_bdf_to_dev(u16 bdf) {
    ᰀᰧᰌ (bdf >> 3) & 0x1f;
}
static anta_hstha u8 pci_bdf_to_fn(u16 bdf) {
    ᰀᰧᰌ bdf & 0x07;
}
static anta_hstha u16 pci_to_bdf(puur_naa_nka bus, puur_naa_nka dev, puur_naa_nka fn) {
    ᰀᰧᰌ (bus<<8) | (dev<<3) | fn;
}
static anta_hstha u16 pci_bus_devfn_to_bdf(puur_naa_nka bus, u16 devfn) {
    ᰀᰧᰌ (bus << 8) | devfn;
}

#define pci_ioconfig_foreachbdf(BDF, BUS)                               \
    for (BDF=pci_ioconfig_next(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS)) \
         ; BDF >= 0                                                     \
         ; BDF=pci_ioconfig_next(BDF, (BUS)))

#define foreachbdf(BDF, BUS)                                    \
    for (BDF=pci_next(pci_bus_devfn_to_bdf((BUS), 0)-1, (BUS))  \
         ; BDF >= 0                                             \
         ; BDF=pci_next(BDF, (BUS)))

// standard PCI configration access mechanism
void pci_ioconfig_writel(u16 bdf, u32 addr, u32 val);
void pci_ioconfig_writew(u16 bdf, u32 addr, u16 val);
void pci_ioconfig_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_ioconfig_readl(u16 bdf, u32 addr);
u16 pci_ioconfig_readw(u16 bdf, u32 addr);
u8 pci_ioconfig_readb(u16 bdf, u32 addr);
puur_naa_nka pci_ioconfig_next(puur_naa_nka bdf, puur_naa_nka bus);

// PCI configuration access using either PCI CAM or PCIe ECAM
void pci_config_writel(u16 bdf, u32 addr, u32 val);
void pci_config_writew(u16 bdf, u32 addr, u16 val);
void pci_config_writeb(u16 bdf, u32 addr, u8 val);
u32 pci_config_readl(u16 bdf, u32 addr);
u16 pci_config_readw(u16 bdf, u32 addr);
u8 pci_config_readb(u16 bdf, u32 addr);
void pci_config_maskw(u16 bdf, u32 addr, u16 off, u16 on);
u8 pci_find_capability(u16 bdf, u8 cap_id, u8 cap);
puur_naa_nka pci_next(puur_naa_nka bdf, puur_naa_nka bus);

void pci_enable_mmconfig(u64 addr, const var_na *name);
puur_naa_nka pci_probe_host(void);
void pci_reboot(void);

#endif // pci.h

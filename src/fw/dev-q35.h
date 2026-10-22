#جيڪڏهن_وصف_نه_ٿيل __DEV_Q35_H
#وصف_ڪريو __DEV_Q35_H

#ضمن "types.h"      // u16

#وصف_ڪريو PCI_DEVICE_ID_INTEL_Q35_MCH     0x29c0
#وصف_ڪريو Q35_HOST_BRIDGE_PAM0            0x90
#وصف_ڪريو Q35_HOST_BRIDGE_SMRAM           0x9d
#وصف_ڪريو Q35_HOST_BRIDGE_PCIEXBAR        0x60
#وصف_ڪريو Q35_HOST_BRIDGE_PCIEXBAR_سائيز   (256 * 1024 * 1024)
#وصف_ڪريو Q35_HOST_BRIDGE_PCIEXBAR_ADDR   0xb0000000
#وصف_ڪريو Q35_HOST_BRIDGE_PCIEXBAREN      ((u64)1)
#وصف_ڪريو Q35_HOST_PCIE_PCI_SEGMENT       0
#وصف_ڪريو Q35_HOST_PCIE_شروعات_BUS_عدد  0
#وصف_ڪريو Q35_HOST_PCIE_پڇاڙي_BUS_عدد    255

#وصف_ڪريو PCI_DEVICE_ID_INTEL_ICH9_LPC    0x2918
#وصف_ڪريو ICH9_LPC_PMBASE                 0x40
#وصف_ڪريو ICH9_LPC_PMBASE_RTE             0x1

#وصف_ڪريو ICH9_LPC_ACPI_CTRL             0x44
#وصف_ڪريو ICH9_LPC_ACPI_CTRL_ACPI_EN     0x80
#وصف_ڪريو ICH9_LPC_PIRQA_ROUT            0x60
#وصف_ڪريو ICH9_LPC_PIRQE_ROUT            0x68
#وصف_ڪريو ICH9_LPC_PIRQ_ROUT_IRQEN       0x80
#وصف_ڪريو ICH9_LPC_GEN_PMCON_1           0xa0
#وصف_ڪريو ICH9_LPC_GEN_PMCON_1_SMI_تالو  (1 << 4)
#وصف_ڪريو ICH9_LPC_پورٽ_ELCR1            0x4d0
#وصف_ڪريو ICH9_LPC_پورٽ_ELCR2            0x4d1
#وصف_ڪريو ICH9_LPC_RCBA                  0xf0
#وصف_ڪريو ICH9_LPC_RCBA_ADDR             0xfed1c000
#وصف_ڪريو ICH9_LPC_RCBA_EN               0x1
#وصف_ڪريو PCI_DEVICE_ID_INTEL_ICH9_SMBUS 0x2930
#وصف_ڪريو ICH9_SMB_SMB_بنياد              0x20
#وصف_ڪريو ICH9_SMB_HOSTC                 0x40
#وصف_ڪريو ICH9_SMB_HOSTC_HST_EN          0x01

#وصف_ڪريو ICH9_ACPI_ENABLE               0x2
#وصف_ڪريو ICH9_ACPI_DISABLE              0x3

/* ICH9 LPC PM I/O registers are 128 ports and 128-aligned */
#وصف_ڪريو ICH9_PMIO_GPE0_STS             0x20
#وصف_ڪريو ICH9_PMIO_GPE0_BLK_LEN         0x10
#وصف_ڪريو ICH9_PMIO_SMI_EN               0x30
#وصف_ڪريو ICH9_PMIO_SMI_EN_APMC_EN       (1 << 5)
#وصف_ڪريو ICH9_PMIO_SMI_EN_GLB_SMI_EN    (1 << 0)

/* FADT ACPI_ENABLE/ACPI_DISABLE */
#وصف_ڪريو ICH9_APM_ACPI_ENABLE           0x2
#وصف_ڪريو ICH9_APM_ACPI_DISABLE          0x3

#جيڪڏهن_پورو // dev-q35.h

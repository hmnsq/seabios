#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __DEV_Q35_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __DEV_Q35_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h"      // u16

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_DEVICE_ID_INTEL_Q35_MCH     0x29c0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Q35_HOST_BRIDGE_PAM0            0x90
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Q35_HOST_BRIDGE_SMRAM           0x9d
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Q35_HOST_BRIDGE_PCIEXBAR        0x60
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Q35_HOST_BRIDGE_PCIEXBAR_ਮਾਪ   (256 * 1024 * 1024)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Q35_HOST_BRIDGE_PCIEXBAR_ADDR   0xb0000000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Q35_HOST_BRIDGE_PCIEXBAREN      ((u64)1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Q35_HOST_PCIE_PCI_SEGMENT       0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Q35_HOST_PCIE_ਸ਼ੁਰੂ_BUS_ਸੰਖਿਆ  0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Q35_HOST_PCIE_ਸਿਰਾ_BUS_ਸੰਖਿਆ    255

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_DEVICE_ID_INTEL_ICH9_LPC    0x2918
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_LPC_PMBASE                 0x40
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_LPC_PMBASE_RTE             0x1

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_LPC_ACPI_CTRL             0x44
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_LPC_ACPI_CTRL_ACPI_EN     0x80
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_LPC_PIRQA_ROUT            0x60
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_LPC_PIRQE_ROUT            0x68
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_LPC_PIRQ_ROUT_IRQEN       0x80
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_LPC_GEN_PMCON_1           0xa0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_LPC_GEN_PMCON_1_SMI_ਤਾਲਾ  (1 << 4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_LPC_ਪੋਰਟ_ELCR1            0x4d0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_LPC_ਪੋਰਟ_ELCR2            0x4d1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_LPC_RCBA                  0xf0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_LPC_RCBA_ADDR             0xfed1c000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_LPC_RCBA_EN               0x1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCI_DEVICE_ID_INTEL_ICH9_SMBUS 0x2930
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_SMB_SMB_ਅਧਾਰ              0x20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_SMB_HOSTC                 0x40
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_SMB_HOSTC_HST_EN          0x01

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_ACPI_ENABLE               0x2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_ACPI_DISABLE              0x3

/* ICH9 LPC PM I/O registers are 128 ports and 128-aligned */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_PMIO_GPE0_STS             0x20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_PMIO_GPE0_BLK_LEN         0x10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_PMIO_SMI_EN               0x30
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_PMIO_SMI_EN_APMC_EN       (1 << 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_PMIO_SMI_EN_GLB_SMI_EN    (1 << 0)

/* FADT ACPI_ENABLE/ACPI_DISABLE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_APM_ACPI_ENABLE           0x2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ICH9_APM_ACPI_DISABLE          0x3

#ਜੇ_ਅੰਤ // dev-q35.h

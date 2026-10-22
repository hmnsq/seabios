#यदि_अपरिभाषितम् __DEV_Q35_H
#परिभाषय __DEV_Q35_H

#अन्तर्भाव "types.h"      // u16

#परिभाषय PCI_DEVICE_ID_INTEL_Q35_MCH     0x29c0
#परिभाषय Q35_HOST_BRIDGE_PAM0            0x90
#परिभाषय Q35_HOST_BRIDGE_SMRAM           0x9d
#परिभाषय Q35_HOST_BRIDGE_PCIEXBAR        0x60
#परिभाषय Q35_HOST_BRIDGE_PCIEXBAR_आकार   (256 * 1024 * 1024)
#परिभाषय Q35_HOST_BRIDGE_PCIEXBAR_ADDR   0xb0000000
#परिभाषय Q35_HOST_BRIDGE_PCIEXBAREN      ((u64)1)
#परिभाषय Q35_HOST_PCIE_PCI_SEGMENT       0
#परिभाषय Q35_HOST_PCIE_START_BUS_सङ्ख्या  0
#परिभाषय Q35_HOST_PCIE_अन्तः_BUS_सङ्ख्या    255

#परिभाषय PCI_DEVICE_ID_INTEL_ICH9_LPC    0x2918
#परिभाषय ICH9_LPC_PMBASE                 0x40
#परिभाषय ICH9_LPC_PMBASE_RTE             0x1

#परिभाषय ICH9_LPC_ACPI_CTRL             0x44
#परिभाषय ICH9_LPC_ACPI_CTRL_ACPI_EN     0x80
#परिभाषय ICH9_LPC_PIRQA_ROUT            0x60
#परिभाषय ICH9_LPC_PIRQE_ROUT            0x68
#परिभाषय ICH9_LPC_PIRQ_ROUT_IRQEN       0x80
#परिभाषय ICH9_LPC_GEN_PMCON_1           0xa0
#परिभाषय ICH9_LPC_GEN_PMCON_1_SMI_तालकम्  (1 << 4)
#परिभाषय ICH9_LPC_पत्तनम्_ELCR1            0x4d0
#परिभाषय ICH9_LPC_पत्तनम्_ELCR2            0x4d1
#परिभाषय ICH9_LPC_RCBA                  0xf0
#परिभाषय ICH9_LPC_RCBA_ADDR             0xfed1c000
#परिभाषय ICH9_LPC_RCBA_EN               0x1
#परिभाषय PCI_DEVICE_ID_INTEL_ICH9_SMBUS 0x2930
#परिभाषय ICH9_SMB_SMB_आधारः              0x20
#परिभाषय ICH9_SMB_HOSTC                 0x40
#परिभाषय ICH9_SMB_HOSTC_HST_EN          0x01

#परिभाषय ICH9_ACPI_ENABLE               0x2
#परिभाषय ICH9_ACPI_DISABLE              0x3

/* ICH9 LPC PM I/O registers are 128 ports and 128-aligned */
#परिभाषय ICH9_PMIO_GPE0_STS             0x20
#परिभाषय ICH9_PMIO_GPE0_BLK_LEN         0x10
#परिभाषय ICH9_PMIO_SMI_EN               0x30
#परिभाषय ICH9_PMIO_SMI_EN_APMC_EN       (1 << 5)
#परिभाषय ICH9_PMIO_SMI_EN_GLB_SMI_EN    (1 << 0)

/* FADT ACPI_ENABLE/ACPI_DISABLE */
#परिभाषय ICH9_APM_ACPI_ENABLE           0x2
#परिभाषय ICH9_APM_ACPI_DISABLE          0x3

#यदि_अन्तः // dev-q35.h

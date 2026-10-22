#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __DEV_Q35_H
#ವ್ಯಾಖ್ಯಾನಿಸು __DEV_Q35_H

#ಅನ್ತರ್ಭಾವ "types.h"      // u16

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_DEVICE_ID_INTEL_Q35_MCH     0x29c0
#ವ್ಯಾಖ್ಯಾನಿಸು Q35_HOST_BRIDGE_PAM0            0x90
#ವ್ಯಾಖ್ಯಾನಿಸು Q35_HOST_BRIDGE_SMRAM           0x9d
#ವ್ಯಾಖ್ಯಾನಿಸು Q35_HOST_BRIDGE_PCIEXBAR        0x60
#ವ್ಯಾಖ್ಯಾನಿಸು Q35_HOST_BRIDGE_PCIEXBAR_ಗಾತ್ರ   (256 * 1024 * 1024)
#ವ್ಯಾಖ್ಯಾನಿಸು Q35_HOST_BRIDGE_PCIEXBAR_ADDR   0xb0000000
#ವ್ಯಾಖ್ಯಾನಿಸು Q35_HOST_BRIDGE_PCIEXBAREN      ((u64)1)
#ವ್ಯಾಖ್ಯಾನಿಸು Q35_HOST_PCIE_PCI_SEGMENT       0
#ವ್ಯಾಖ್ಯಾನಿಸು Q35_HOST_PCIE_ಆರಂಭ_BUS_ಸಂಖ್ಯೆ  0
#ವ್ಯಾಖ್ಯಾನಿಸು Q35_HOST_PCIE_ಕೊನೆ_BUS_ಸಂಖ್ಯೆ    255

#ವ್ಯಾಖ್ಯಾನಿಸು PCI_DEVICE_ID_INTEL_ICH9_LPC    0x2918
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_LPC_PMBASE                 0x40
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_LPC_PMBASE_RTE             0x1

#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_LPC_ACPI_CTRL             0x44
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_LPC_ACPI_CTRL_ACPI_EN     0x80
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_LPC_PIRQA_ROUT            0x60
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_LPC_PIRQE_ROUT            0x68
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_LPC_PIRQ_ROUT_IRQEN       0x80
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_LPC_GEN_PMCON_1           0xa0
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_LPC_GEN_PMCON_1_SMI_ಬೀಗ  (1 << 4)
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_LPC_ಪೋರ್ಟ್_ELCR1            0x4d0
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_LPC_ಪೋರ್ಟ್_ELCR2            0x4d1
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_LPC_RCBA                  0xf0
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_LPC_RCBA_ADDR             0xfed1c000
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_LPC_RCBA_EN               0x1
#ವ್ಯಾಖ್ಯಾನಿಸು PCI_DEVICE_ID_INTEL_ICH9_SMBUS 0x2930
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_SMB_SMB_ತಳ              0x20
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_SMB_HOSTC                 0x40
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_SMB_HOSTC_HST_EN          0x01

#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_ACPI_ENABLE               0x2
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_ACPI_DISABLE              0x3

/* ICH9 LPC PM I/O registers are 128 ports and 128-aligned */
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_PMIO_GPE0_STS             0x20
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_PMIO_GPE0_BLK_LEN         0x10
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_PMIO_SMI_EN               0x30
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_PMIO_SMI_EN_APMC_EN       (1 << 5)
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_PMIO_SMI_EN_GLB_SMI_EN    (1 << 0)

/* FADT ACPI_ENABLE/ACPI_DISABLE */
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_APM_ACPI_ENABLE           0x2
#ವ್ಯಾಖ್ಯಾನಿಸು ICH9_APM_ACPI_DISABLE          0x3

#ಆದರೆ_ಅಂತ್ಯ // dev-q35.h

#నిర్వచితం_కాకపోతే __DEV_Q35_H
#నిర్వచించు __DEV_Q35_H

#అన్తర్భావ "types.h"      // u16

#నిర్వచించు PCI_DEVICE_ID_INTEL_Q35_MCH     0x29c0
#నిర్వచించు Q35_HOST_BRIDGE_PAM0            0x90
#నిర్వచించు Q35_HOST_BRIDGE_SMRAM           0x9d
#నిర్వచించు Q35_HOST_BRIDGE_PCIEXBAR        0x60
#నిర్వచించు Q35_HOST_BRIDGE_PCIEXBAR_కొలత   (256 * 1024 * 1024)
#నిర్వచించు Q35_HOST_BRIDGE_PCIEXBAR_ADDR   0xb0000000
#నిర్వచించు Q35_HOST_BRIDGE_PCIEXBAREN      ((u64)1)
#నిర్వచించు Q35_HOST_PCIE_PCI_SEGMENT       0
#నిర్వచించు Q35_HOST_PCIE_ప్రారంభం_BUS_సంఖ్య  0
#నిర్వచించు Q35_HOST_PCIE_చివర_BUS_సంఖ్య    255

#నిర్వచించు PCI_DEVICE_ID_INTEL_ICH9_LPC    0x2918
#నిర్వచించు ICH9_LPC_PMBASE                 0x40
#నిర్వచించు ICH9_LPC_PMBASE_RTE             0x1

#నిర్వచించు ICH9_LPC_ACPI_CTRL             0x44
#నిర్వచించు ICH9_LPC_ACPI_CTRL_ACPI_EN     0x80
#నిర్వచించు ICH9_LPC_PIRQA_ROUT            0x60
#నిర్వచించు ICH9_LPC_PIRQE_ROUT            0x68
#నిర్వచించు ICH9_LPC_PIRQ_ROUT_IRQEN       0x80
#నిర్వచించు ICH9_LPC_GEN_PMCON_1           0xa0
#నిర్వచించు ICH9_LPC_GEN_PMCON_1_SMI_తాళం  (1 << 4)
#నిర్వచించు ICH9_LPC_పోర్టు_ELCR1            0x4d0
#నిర్వచించు ICH9_LPC_పోర్టు_ELCR2            0x4d1
#నిర్వచించు ICH9_LPC_RCBA                  0xf0
#నిర్వచించు ICH9_LPC_RCBA_ADDR             0xfed1c000
#నిర్వచించు ICH9_LPC_RCBA_EN               0x1
#నిర్వచించు PCI_DEVICE_ID_INTEL_ICH9_SMBUS 0x2930
#నిర్వచించు ICH9_SMB_SMB_పునాది              0x20
#నిర్వచించు ICH9_SMB_HOSTC                 0x40
#నిర్వచించు ICH9_SMB_HOSTC_HST_EN          0x01

#నిర్వచించు ICH9_ACPI_ENABLE               0x2
#నిర్వచించు ICH9_ACPI_DISABLE              0x3

/* ICH9 LPC PM I/O registers are 128 ports and 128-aligned */
#నిర్వచించు ICH9_PMIO_GPE0_STS             0x20
#నిర్వచించు ICH9_PMIO_GPE0_BLK_LEN         0x10
#నిర్వచించు ICH9_PMIO_SMI_EN               0x30
#నిర్వచించు ICH9_PMIO_SMI_EN_APMC_EN       (1 << 5)
#నిర్వచించు ICH9_PMIO_SMI_EN_GLB_SMI_EN    (1 << 0)

/* FADT ACPI_ENABLE/ACPI_DISABLE */
#నిర్వచించు ICH9_APM_ACPI_ENABLE           0x2
#నిర్వచించు ICH9_APM_ACPI_DISABLE          0x3

#అయితే_ముగింపు // dev-q35.h

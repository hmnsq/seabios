#جے_بنا_تعریف __DEV_Q35_H
#تعریف_کرو __DEV_Q35_H

#ضمن "types.h"      // u16

#تعریف_کرو PCI_DEVICE_ID_INTEL_Q35_MCH     0x29c0
#تعریف_کرو Q35_HOST_BRIDGE_PAM0            0x90
#تعریف_کرو Q35_HOST_BRIDGE_SMRAM           0x9d
#تعریف_کرو Q35_HOST_BRIDGE_PCIEXBAR        0x60
#تعریف_کرو Q35_HOST_BRIDGE_PCIEXBAR_ناپ   (256 * 1024 * 1024)
#تعریف_کرو Q35_HOST_BRIDGE_PCIEXBAR_ADDR   0xb0000000
#تعریف_کرو Q35_HOST_BRIDGE_PCIEXBAREN      ((u64)1)
#تعریف_کرو Q35_HOST_PCIE_PCI_SEGMENT       0
#تعریف_کرو Q35_HOST_PCIE_شروع_BUS_نمبر  0
#تعریف_کرو Q35_HOST_PCIE_سرا_BUS_نمبر    255

#تعریف_کرو PCI_DEVICE_ID_INTEL_ICH9_LPC    0x2918
#تعریف_کرو ICH9_LPC_PMBASE                 0x40
#تعریف_کرو ICH9_LPC_PMBASE_RTE             0x1

#تعریف_کرو ICH9_LPC_ACPI_CTRL             0x44
#تعریف_کرو ICH9_LPC_ACPI_CTRL_ACPI_EN     0x80
#تعریف_کرو ICH9_LPC_PIRQA_ROUT            0x60
#تعریف_کرو ICH9_LPC_PIRQE_ROUT            0x68
#تعریف_کرو ICH9_LPC_PIRQ_ROUT_IRQEN       0x80
#تعریف_کرو ICH9_LPC_GEN_PMCON_1           0xa0
#تعریف_کرو ICH9_LPC_GEN_PMCON_1_SMI_جندرا  (1 << 4)
#تعریف_کرو ICH9_LPC_پورٹ_ELCR1            0x4d0
#تعریف_کرو ICH9_LPC_پورٹ_ELCR2            0x4d1
#تعریف_کرو ICH9_LPC_RCBA                  0xf0
#تعریف_کرو ICH9_LPC_RCBA_ADDR             0xfed1c000
#تعریف_کرو ICH9_LPC_RCBA_EN               0x1
#تعریف_کرو PCI_DEVICE_ID_INTEL_ICH9_SMBUS 0x2930
#تعریف_کرو ICH9_SMB_SMB_ادھار              0x20
#تعریف_کرو ICH9_SMB_HOSTC                 0x40
#تعریف_کرو ICH9_SMB_HOSTC_HST_EN          0x01

#تعریف_کرو ICH9_ACPI_ENABLE               0x2
#تعریف_کرو ICH9_ACPI_DISABLE              0x3

/* ICH9 LPC PM I/O registers are 128 ports and 128-aligned */
#تعریف_کرو ICH9_PMIO_GPE0_STS             0x20
#تعریف_کرو ICH9_PMIO_GPE0_BLK_LEN         0x10
#تعریف_کرو ICH9_PMIO_SMI_EN               0x30
#تعریف_کرو ICH9_PMIO_SMI_EN_APMC_EN       (1 << 5)
#تعریف_کرو ICH9_PMIO_SMI_EN_GLB_SMI_EN    (1 << 0)

/* FADT ACPI_ENABLE/ACPI_DISABLE */
#تعریف_کرو ICH9_APM_ACPI_ENABLE           0x2
#تعریف_کرو ICH9_APM_ACPI_DISABLE          0x3

#جے_انت // dev-q35.h

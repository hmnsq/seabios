#جے_بنا_تعریف __DEV_PIIX_H
#تعریف_کرو __DEV_PIIX_H

#تعریف_کرو I440FX_PAM0               0x59
#تعریف_کرو I440FX_SMRAM              0x72

#تعریف_کرو PIIX_PMBASE               0x40
#تعریف_کرو PIIX_PMREGMISC            0x80
#تعریف_کرو PIIX_SMBHSTBASE           0x90
#تعریف_کرو PIIX_SMBHSTCFG            0xd2
#تعریف_کرو PIIX_DEVACTB              0x58
#تعریف_کرو PIIX_DEVACTB_APMC_EN      (1 << 25)

#تعریف_کرو PIIX_پورٹ_ELCR1           0x4d0
#تعریف_کرو PIIX_پورٹ_ELCR2           0x4d1

/* ICH9 PM I/O registers */
#تعریف_کرو PIIX_GPE0_BLK            0xafe0
#تعریف_کرو PIIX_GPE0_BLK_LEN        4
#تعریف_کرو PIIX_PMIO_GLBCTL         0x28
#تعریف_کرو PIIX_PMIO_GLBCTL_SMI_EN  1

/* FADT ACPI_ENABLE/ACPI_DISABLE */
#تعریف_کرو PIIX_ACPI_ENABLE         0xf1
#تعریف_کرو PIIX_ACPI_DISABLE        0xf0

#تعریف_کرو PIIX_PM_INTRRUPT         9       // irq 9

#جے_انت // dev-piix.h

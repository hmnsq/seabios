#اگر_تعریف_نشده __DEV_PIIX_H
#تعریف_کن __DEV_PIIX_H

#تعریف_کن I440FX_PAM0               0x59
#تعریف_کن I440FX_SMRAM              0x72

#تعریف_کن PIIX_PMBASE               0x40
#تعریف_کن PIIX_PMREGMISC            0x80
#تعریف_کن PIIX_SMBHSTBASE           0x90
#تعریف_کن PIIX_SMBHSTCFG            0xd2
#تعریف_کن PIIX_DEVACTB              0x58
#تعریف_کن PIIX_DEVACTB_APMC_EN      (1 << 25)

#تعریف_کن PIIX_پورت_ELCR1           0x4d0
#تعریف_کن PIIX_پورت_ELCR2           0x4d1

/* ICH9 PM I/O registers */
#تعریف_کن PIIX_GPE0_BLK            0xafe0
#تعریف_کن PIIX_GPE0_BLK_LEN        4
#تعریف_کن PIIX_PMIO_GLBCTL         0x28
#تعریف_کن PIIX_PMIO_GLBCTL_SMI_EN  1

/* FADT ACPI_ENABLE/ACPI_DISABLE */
#تعریف_کن PIIX_ACPI_ENABLE         0xf1
#تعریف_کن PIIX_ACPI_DISABLE        0xf0

#تعریف_کن PIIX_PM_INTRRUPT         9       // irq 9

#پایان_اگر // dev-piix.h

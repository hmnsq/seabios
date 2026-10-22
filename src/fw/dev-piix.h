#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __DEV_PIIX_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __DEV_PIIX_H

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I440FX_PAM0               0x59
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ I440FX_SMRAM              0x72

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIIX_PMBASE               0x40
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIIX_PMREGMISC            0x80
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIIX_SMBHSTBASE           0x90
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIIX_SMBHSTCFG            0xd2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIIX_DEVACTB              0x58
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIIX_DEVACTB_APMC_EN      (1 << 25)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIIX_ਪੋਰਟ_ELCR1           0x4d0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIIX_ਪੋਰਟ_ELCR2           0x4d1

/* ICH9 PM I/O registers */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIIX_GPE0_BLK            0xafe0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIIX_GPE0_BLK_LEN        4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIIX_PMIO_GLBCTL         0x28
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIIX_PMIO_GLBCTL_SMI_EN  1

/* FADT ACPI_ENABLE/ACPI_DISABLE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIIX_ACPI_ENABLE         0xf1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIIX_ACPI_DISABLE        0xf0

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PIIX_PM_INTRRUPT         9       // irq 9

#ਜੇ_ਅੰਤ // dev-piix.h

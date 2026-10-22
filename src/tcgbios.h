#ifndef TCGBIOS_H
#define TCGBIOS_H

#ಅನ್ತರ್ಭಾವ "types.h"

ಸಂರಚನಾ bregs;
ಶೂನ್ಯ tpm_interrupt_handler32(ಸಂರಚನಾ bregs *regs);

ಶೂನ್ಯ tpm_setup(ಶೂನ್ಯ);
ಶೂನ್ಯ tpm_prepboot(ಶೂನ್ಯ);
ಶೂನ್ಯ tpm_s3_resume(ಶೂನ್ಯ);
ಶೂನ್ಯ tpm_add_bcv(u32 bootdrv, ಧ್ರುವ u8 *addr, u32 length);
ಶೂನ್ಯ tpm_add_cdrom(u32 bootdrv, ಧ್ರುವ u8 *addr, u32 length);
ಶೂನ್ಯ tpm_add_cdrom_catalog(ಧ್ರುವ u8 *addr, u32 length);
ಶೂನ್ಯ tpm_option_rom(ಧ್ರುವ ಶೂನ್ಯ *addr, u32 len);
ಪೂರ್ಣಾಙ್ಕ tpm_can_show_menu(ಶೂನ್ಯ);
ಶೂನ್ಯ tpm_menu(ಶೂನ್ಯ);

#endif /* TCGBIOS_H */

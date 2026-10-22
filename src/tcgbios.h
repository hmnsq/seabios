#ifndef TCGBIOS_H
#define TCGBIOS_H

#شامل "types.h"

ساختار bregs;
تهی tpm_interrupt_handler32(ساختار bregs *regs);

تهی tpm_setup(تهی);
تهی tpm_prepboot(تهی);
تهی tpm_s3_resume(تهی);
تهی tpm_add_bcv(u32 bootdrv, ثابت u8 *addr, u32 length);
تهی tpm_add_cdrom(u32 bootdrv, ثابت u8 *addr, u32 length);
تهی tpm_add_cdrom_catalog(ثابت u8 *addr, u32 length);
تهی tpm_option_rom(ثابت تهی *addr, u32 len);
صحیح tpm_can_show_menu(تهی);
تهی tpm_menu(تهی);

#endif /* TCGBIOS_H */

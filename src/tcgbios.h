#اگر_غیر_تعریف_شدہ TCGBIOS_H
#تعریف_کرو TCGBIOS_H

#شامل "types.h"

ساخت bregs;
خالی tpm_مداخلت_handler32(ساخت bregs *regs);

خالی tpm_تیاری(خالی);
خالی tpm_prepboot(خالی);
خالی tpm_s3_resume(خالی);
خالی tpm_جوڑو_bcv(u32 bootdrv, ثابت u8 *addr, u32 لمبائی);
خالی tpm_جوڑو_cdrom(u32 bootdrv, ثابت u8 *addr, u32 لمبائی);
خالی tpm_جوڑو_cdrom_catalog(ثابت u8 *addr, u32 لمبائی);
خالی tpm_option_rom(ثابت خالی *addr, u32 len);
صحیح tpm_can_show_menu(خالی);
خالی tpm_menu(خالی);

#اگر_ختم /* TCGBIOS_H */

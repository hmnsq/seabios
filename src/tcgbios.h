#اگر_تعریف_نشده TCGBIOS_H
#تعریف_کن TCGBIOS_H

#ضمن "types.h"

بنية bregs;
فراغ tpm_وقفه_handler32(بنية bregs *regs);

فراغ tpm_راهاندازی(فراغ);
فراغ tpm_prepboot(فراغ);
فراغ tpm_s3_resume(فراغ);
فراغ tpm_جمع_کن_bcv(u32 bootdrv, ثابت u8 *addr, u32 درازی);
فراغ tpm_جمع_کن_cdrom(u32 bootdrv, ثابت u8 *addr, u32 درازی);
فراغ tpm_جمع_کن_cdrom_catalog(ثابت u8 *addr, u32 درازی);
فراغ tpm_option_rom(ثابت فراغ *addr, u32 len);
صحيح tpm_can_show_menu(فراغ);
فراغ tpm_menu(فراغ);

#پایان_اگر /* TCGBIOS_H */

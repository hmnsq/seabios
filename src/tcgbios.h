#که_نه_تعریف_شوی TCGBIOS_H
#تعریف_کړه TCGBIOS_H

#ضمن "types.h"

بنية bregs;
فراغ tpm_مداخله_handler32(بنية bregs *regs);

فراغ tpm_تیاری(فراغ);
فراغ tpm_prepboot(فراغ);
فراغ tpm_s3_resume(فراغ);
فراغ tpm_جمع_کړه_bcv(u32 bootdrv, ثابت u8 *addr, u32 اوږدتیا);
فراغ tpm_جمع_کړه_cdrom(u32 bootdrv, ثابت u8 *addr, u32 اوږدتیا);
فراغ tpm_جمع_کړه_cdrom_catalog(ثابت u8 *addr, u32 اوږدتیا);
فراغ tpm_option_rom(ثابت فراغ *addr, u32 len);
صحيح tpm_can_show_menu(فراغ);
فراغ tpm_menu(فراغ);

#د_که_پای /* TCGBIOS_H */

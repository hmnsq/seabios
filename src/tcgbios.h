#جيڪڏهن_وصف_نه_ٿيل TCGBIOS_H
#وصف_ڪريو TCGBIOS_H

#ضمن "types.h"

بنية bregs;
فراغ tpm_مداخلت_handler32(بنية bregs *regs);

فراغ tpm_تياري(فراغ);
فراغ tpm_prepboot(فراغ);
فراغ tpm_s3_resume(فراغ);
فراغ tpm_جوڙيو_bcv(u32 bootdrv, ثابت u8 *addr, u32 ڊيگهائي);
فراغ tpm_جوڙيو_cdrom(u32 bootdrv, ثابت u8 *addr, u32 ڊيگهائي);
فراغ tpm_جوڙيو_cdrom_catalog(ثابت u8 *addr, u32 ڊيگهائي);
فراغ tpm_option_rom(ثابت فراغ *addr, u32 len);
صحيح tpm_can_show_menu(فراغ);
فراغ tpm_menu(فراغ);

#جيڪڏهن_پورو /* TCGBIOS_H */

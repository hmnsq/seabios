#جے_بنا_تعریف TCGBIOS_H
#تعریف_کرو TCGBIOS_H

#ضمن "types.h"

بنية bregs;
فراغ tpm_رکاوٹ_handler32(بنية bregs *regs);

فراغ tpm_تیاری(فراغ);
فراغ tpm_prepboot(فراغ);
فراغ tpm_s3_resume(فراغ);
فراغ tpm_جوڑو_bcv(u32 bootdrv, ثابت u8 *addr, u32 لمائی);
فراغ tpm_جوڑو_cdrom(u32 bootdrv, ثابت u8 *addr, u32 لمائی);
فراغ tpm_جوڑو_cdrom_catalog(ثابت u8 *addr, u32 لمائی);
فراغ tpm_option_rom(ثابت فراغ *addr, u32 len);
صحيح tpm_can_show_menu(فراغ);
فراغ tpm_menu(فراغ);

#جے_انت /* TCGBIOS_H */

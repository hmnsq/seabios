#ifndef TCGBIOS_H
#define TCGBIOS_H

#ضمن "types.h"

بنية bregs;
فراغ tpm_interrupt_handler32(بنية bregs *regs);

فراغ tpm_setup(فراغ);
فراغ tpm_prepboot(فراغ);
فراغ tpm_s3_resume(فراغ);
فراغ tpm_add_bcv(u32 bootdrv, ثابت u8 *addr, u32 length);
فراغ tpm_add_cdrom(u32 bootdrv, ثابت u8 *addr, u32 length);
فراغ tpm_add_cdrom_catalog(ثابت u8 *addr, u32 length);
فراغ tpm_option_rom(ثابت فراغ *addr, u32 len);
صحيح tpm_can_show_menu(فراغ);
فراغ tpm_menu(فراغ);

#endif /* TCGBIOS_H */

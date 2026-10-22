#ifndef TCGBIOS_H
#define TCGBIOS_H

#ଅନ୍ତର୍ଭାଵ "types.h"

ସଂରଚନା bregs;
ଶୂନ୍ୟ tpm_interrupt_handler32(ସଂରଚନା bregs *regs);

ଶୂନ୍ୟ tpm_setup(ଶୂନ୍ୟ);
ଶୂନ୍ୟ tpm_prepboot(ଶୂନ୍ୟ);
ଶୂନ୍ୟ tpm_s3_resume(ଶୂନ୍ୟ);
ଶୂନ୍ୟ tpm_add_bcv(u32 bootdrv, ଅଚଳ u8 *addr, u32 length);
ଶୂନ୍ୟ tpm_add_cdrom(u32 bootdrv, ଅଚଳ u8 *addr, u32 length);
ଶୂନ୍ୟ tpm_add_cdrom_catalog(ଅଚଳ u8 *addr, u32 length);
ଶୂନ୍ୟ tpm_option_rom(ଅଚଳ ଶୂନ୍ୟ *addr, u32 len);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା tpm_can_show_menu(ଶୂନ୍ୟ);
ଶୂନ୍ୟ tpm_menu(ଶୂନ୍ୟ);

#endif /* TCGBIOS_H */

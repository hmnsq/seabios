#ifndef TCGBIOS_H
#define TCGBIOS_H

#include "types.h"

sa_nracanaa bregs;
void tpm_interrupt_handler32(sa_nracanaa bregs *regs);

void tpm_setup(void);
void tpm_prepboot(void);
void tpm_s3_resume(void);
void tpm_add_bcv(u32 bootdrv, const u8 *addr, u32 length);
void tpm_add_cdrom(u32 bootdrv, const u8 *addr, u32 length);
void tpm_add_cdrom_catalog(const u8 *addr, u32 length);
void tpm_option_rom(const void *addr, u32 len);
පූර්ණ_සංඛ්‍යා tpm_can_show_menu(void);
void tpm_menu(void);

#endif /* TCGBIOS_H */

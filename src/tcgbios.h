#ifndef TCGBIOS_H
#define TCGBIOS_H

#അന്തര്ഭാവ "types.h"

ഘടന bregs;
ശൂന്യം tpm_interrupt_handler32(ഘടന bregs *regs);

ശൂന്യം tpm_setup(ശൂന്യം);
ശൂന്യം tpm_prepboot(ശൂന്യം);
ശൂന്യം tpm_s3_resume(ശൂന്യം);
ശൂന്യം tpm_add_bcv(u32 bootdrv, സ്ഥിരം u8 *addr, u32 length);
ശൂന്യം tpm_add_cdrom(u32 bootdrv, സ്ഥിരം u8 *addr, u32 length);
ശൂന്യം tpm_add_cdrom_catalog(സ്ഥിരം u8 *addr, u32 length);
ശൂന്യം tpm_option_rom(സ്ഥിരം ശൂന്യം *addr, u32 len);
പൂർണ്ണസംഖ്യ tpm_can_show_menu(ശൂന്യം);
ശൂന്യം tpm_menu(ശൂന്യം);

#endif /* TCGBIOS_H */

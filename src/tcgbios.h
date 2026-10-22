#ifndef TCGBIOS_H
#define TCGBIOS_H

#અન્તર્ભાવ "types.h"

સંરચના bregs;
શૂન્ય tpm_interrupt_handler32(સંરચના bregs *regs);

શૂન્ય tpm_setup(શૂન્ય);
શૂન્ય tpm_prepboot(શૂન્ય);
શૂન્ય tpm_s3_resume(શૂન્ય);
શૂન્ય tpm_add_bcv(u32 bootdrv, અચલ u8 *addr, u32 length);
શૂન્ય tpm_add_cdrom(u32 bootdrv, અચલ u8 *addr, u32 length);
શૂન્ય tpm_add_cdrom_catalog(અચલ u8 *addr, u32 length);
શૂન્ય tpm_option_rom(અચલ શૂન્ય *addr, u32 len);
પૂર્ણાંક tpm_can_show_menu(શૂન્ય);
શૂન્ય tpm_menu(શૂન્ય);

#endif /* TCGBIOS_H */

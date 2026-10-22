#ifndef TCGBIOS_H
#define TCGBIOS_H

#अन्तर्भाव "types.h"

संरचना bregs;
शून्य tpm_interrupt_handler32(संरचना bregs *regs);

शून्य tpm_setup(शून्य);
शून्य tpm_prepboot(शून्य);
शून्य tpm_s3_resume(शून्य);
शून्य tpm_add_bcv(u32 bootdrv, ध्रुव u8 *addr, u32 length);
शून्य tpm_add_cdrom(u32 bootdrv, ध्रुव u8 *addr, u32 length);
शून्य tpm_add_cdrom_catalog(ध्रुव u8 *addr, u32 length);
शून्य tpm_option_rom(ध्रुव शून्य *addr, u32 len);
पूर्णाङ्क tpm_can_show_menu(शून्य);
शून्य tpm_menu(शून्य);

#endif /* TCGBIOS_H */

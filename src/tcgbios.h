#यदि_अपरिभाषितम् TCGBIOS_H
#परिभाषय TCGBIOS_H

#अन्तर्भाव "types.h"

संरचना bregs;
शून्य tpm_व्यवधानम्_handler32(संरचना bregs *regs);

शून्य tpm_सज्जता(शून्य);
शून्य tpm_prepboot(शून्य);
शून्य tpm_s3_resume(शून्य);
शून्य tpm_सङ्कलय_bcv(u32 bootdrv, ध्रुव u8 *addr, u32 दीर्घता);
शून्य tpm_सङ्कलय_cdrom(u32 bootdrv, ध्रुव u8 *addr, u32 दीर्घता);
शून्य tpm_सङ्कलय_cdrom_catalog(ध्रुव u8 *addr, u32 दीर्घता);
शून्य tpm_option_rom(ध्रुव शून्य *addr, u32 len);
पूर्णाङ्क tpm_can_show_menu(शून्य);
शून्य tpm_menu(शून्य);

#यदि_अन्तः /* TCGBIOS_H */

#ifndef TCGBIOS_H
#define TCGBIOS_H

#அந்தர்பாவ "types.h"

கட்டமைப்பு bregs;
வெறுமை tpm_interrupt_handler32(கட்டமைப்பு bregs *regs);

வெறுமை tpm_setup(வெறுமை);
வெறுமை tpm_prepboot(வெறுமை);
வெறுமை tpm_s3_resume(வெறுமை);
வெறுமை tpm_add_bcv(u32 bootdrv, மாறிலி u8 *addr, u32 length);
வெறுமை tpm_add_cdrom(u32 bootdrv, மாறிலி u8 *addr, u32 length);
வெறுமை tpm_add_cdrom_catalog(மாறிலி u8 *addr, u32 length);
வெறுமை tpm_option_rom(மாறிலி வெறுமை *addr, u32 len);
முழு tpm_can_show_menu(வெறுமை);
வெறுமை tpm_menu(வெறுமை);

#endif /* TCGBIOS_H */

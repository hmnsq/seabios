#ifndef TCGBIOS_H
#define TCGBIOS_H

#অন্তর্ভাৱ "types.h"

সংরচনা bregs;
শূন্য tpm_interrupt_handler32(সংরচনা bregs *regs);

শূন্য tpm_setup(শূন্য);
শূন্য tpm_prepboot(শূন্য);
শূন্য tpm_s3_resume(শূন্য);
শূন্য tpm_add_bcv(u32 bootdrv, ধ্রুৱ u8 *addr, u32 length);
শূন্য tpm_add_cdrom(u32 bootdrv, ধ্রুৱ u8 *addr, u32 length);
শূন্য tpm_add_cdrom_catalog(ধ্রুৱ u8 *addr, u32 length);
শূন্য tpm_option_rom(ধ্রুৱ শূন্য *addr, u32 len);
পূর্ণাঙ্ক tpm_can_show_menu(শূন্য);
শূন্য tpm_menu(শূন্য);

#endif /* TCGBIOS_H */

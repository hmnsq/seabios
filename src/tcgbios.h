#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ TCGBIOS_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TCGBIOS_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h"

ਬਣਤਰ bregs;
ਖਾਲੀ tpm_ਰੁਕਾਵਟ_handler32(ਬਣਤਰ bregs *regs);

ਖਾਲੀ tpm_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ tpm_prepboot(ਖਾਲੀ);
ਖਾਲੀ tpm_s3_resume(ਖਾਲੀ);
ਖਾਲੀ tpm_ਜੋੜੋ_bcv(u32 bootdrv, ਅਚਲ u8 *addr, u32 ਲੰਮਾਈ);
ਖਾਲੀ tpm_ਜੋੜੋ_cdrom(u32 bootdrv, ਅਚਲ u8 *addr, u32 ਲੰਮਾਈ);
ਖਾਲੀ tpm_ਜੋੜੋ_cdrom_catalog(ਅਚਲ u8 *addr, u32 ਲੰਮਾਈ);
ਖਾਲੀ tpm_option_rom(ਅਚਲ ਖਾਲੀ *addr, u32 len);
ਪੂਰਨ tpm_can_show_menu(ਖਾਲੀ);
ਖਾਲੀ tpm_menu(ਖਾਲੀ);

#ਜੇ_ਅੰਤ /* TCGBIOS_H */

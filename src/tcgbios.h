#నిర్వచితం_కాకపోతే TCGBIOS_H
#నిర్వచించు TCGBIOS_H

#అన్తర్భావ "types.h"

నిర్మాణం bregs;
శూన్య tpm_అంతరాయం_handler32(నిర్మాణం bregs *regs);

శూన్య tpm_అమరిక(శూన్య);
శూన్య tpm_prepboot(శూన్య);
శూన్య tpm_s3_resume(శూన్య);
శూన్య tpm_కూడు_bcv(u32 bootdrv, స్థిరరాశి u8 *addr, u32 నిడివి);
శూన్య tpm_కూడు_cdrom(u32 bootdrv, స్థిరరాశి u8 *addr, u32 నిడివి);
శూన్య tpm_కూడు_cdrom_catalog(స్థిరరాశి u8 *addr, u32 నిడివి);
శూన్య tpm_option_rom(స్థిరరాశి శూన్య *addr, u32 len);
పూర్ణాంకం tpm_can_show_menu(శూన్య);
శూన్య tpm_menu(శూన్య);

#అయితే_ముగింపు /* TCGBIOS_H */

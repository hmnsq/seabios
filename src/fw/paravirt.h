#నిర్వచితం_కాకపోతే __PV_H
#నిర్వచించు __PV_H

#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "biosvar.h" // GET_GLOBAL
#అన్తర్భావ "romfile.h" // struct romfile_s

// kvmclock
నిర్మాణం pvclock_vcpu_సమయం_info {
	u32   సంస్కరణ_2;
	u32   pad0;
	u64   tsc_timestamp;
	u64   system_సమయం;
	u32   tsc_గమ్యం_system_mul;
	s8    tsc_జరుపు;
	u8    flags;
	u8    pad[2];
} __attribute__((__packed__)); /* 32 bytes */

// Types of paravirtualized platforms.
#నిర్వచించు PF_QEMU     (1<<0)
#నిర్వచించు PF_XEN      (1<<1)
#నిర్వచించు PF_KVM      (1<<2)

రకనిర్వచనం నిర్మాణం QemuCfgDmaAccess {
    u32 control;
    u32 నిడివి;
    u64 చిరునామా;
} PACKED QemuCfgDmaAccess;

బాహ్య u32 Ram_కొలత;
బాహ్య u64 Ram_కొలత_Over4_G;
బాహ్య పూర్ణాంకం PlatformRunningOn;
బాహ్య u8 CPUPhys_బిట్లు;
బాహ్య u8 CPULong_బహుళకం;

స్థిర అంతఃవరుస పూర్ణాంకం runningOnQEMU(శూన్య) {
    తిరిగి CONFIG_QEMU || (
        CONFIG_QEMU_HARDWARE && GET_GLOBAL(PlatformRunningOn) & PF_QEMU);
}
స్థిర అంతఃవరుస పూర్ణాంకం runningOnXen(శూన్య) {
    తిరిగి CONFIG_XEN && GET_GLOBAL(PlatformRunningOn) & PF_XEN;
}
స్థిర అంతఃవరుస పూర్ణాంకం runningOnKVM(శూన్య) {
    తిరిగి CONFIG_QEMU && GET_GLOBAL(PlatformRunningOn) & PF_KVM;
}

// Common paravirt ports.
#నిర్వచించు పోర్టు_SMI_CMD                0x00b2
#నిర్వచించు పోర్టు_SMI_స్థితి             0x00b3
#నిర్వచించు పోర్టు_QEMU_CFG_CTL           0x0510
#నిర్వచించు పోర్టు_QEMU_CFG_దత్తాంశం          0x0511
#నిర్వచించు పోర్టు_QEMU_CFG_DMA_ADDR_ఎగువ 0x0514
#నిర్వచించు పోర్టు_QEMU_CFG_DMA_ADDR_దిగువ  0x0518

// QEMU_CFG_DMA_CONTROL bits
#నిర్వచించు QEMU_CFG_DMA_CTL_ERROR   0x01
#నిర్వచించు QEMU_CFG_DMA_CTL_READ    0x02
#నిర్వచించు QEMU_CFG_DMA_CTL_SKIP    0x04
#నిర్వచించు QEMU_CFG_DMA_CTL_SELECT  0x08
#నిర్వచించు QEMU_CFG_DMA_CTL_WRITE   0x10

// QEMU_CFG_DMA ID bit
#నిర్వచించు QEMU_CFG_సంస్కరణ_DMA    2

// QEMU debugcon read value
#నిర్వచించు QEMU_DEBUGCON_READBACK  0xe9

పూర్ణాంకం qemu_cfg_enabled(శూన్య);
పూర్ణాంకం qemu_cfg_dma_enabled(శూన్య);
శూన్య qemu_preinit(శూన్య);
శూన్య qemu_platform_అమరిక(శూన్య);
శూన్య qemu_cfg_init(శూన్య);

u16 qemu_get_present_cpus_లెక్క(శూన్య);
పూర్ణాంకం qemu_cfg_write_file(శూన్య *src, నిర్మాణం romfile_s *file, u32 స్థానభ్రంశం_2, u32 len);
పూర్ణాంకం qemu_cfg_write_file_simple(శూన్య *src, u16 కీలకం, u32 స్థానభ్రంశం_2, u32 len);
u16 qemu_get_romfile_కీలకం(నిర్మాణం romfile_s *file);

#అయితే_ముగింపు

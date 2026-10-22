#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __PV_H
#ವ್ಯಾಖ್ಯಾನಿಸು __PV_H

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBAL
#ಅನ್ತರ್ಭಾವ "romfile.h" // struct romfile_s

// kvmclock
ರಚನೆ pvclock_vcpu_ಸಮಯ_info {
	u32   ಆವೃತ್ತಿ_2;
	u32   pad0;
	u64   tsc_timestamp;
	u64   system_ಸಮಯ;
	u32   tsc_ಗಮ್ಯ_system_mul;
	s8    tsc_ಸರಿಸು;
	u8    flags;
	u8    pad[2];
} __attribute__((__packed__)); /* 32 bytes */

// Types of paravirtualized platforms.
#ವ್ಯಾಖ್ಯಾನಿಸು PF_QEMU     (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು PF_XEN      (1<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು PF_KVM      (1<<2)

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ QemuCfgDmaAccess {
    u32 control;
    u32 ಉದ್ದಳತೆ;
    u64 ವಿಳಾಸ;
} PACKED QemuCfgDmaAccess;

ಬಾಹ್ಯ u32 Ram_ಗಾತ್ರ;
ಬಾಹ್ಯ u64 Ram_ಗಾತ್ರ_Over4_G;
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ PlatformRunningOn;
ಬಾಹ್ಯ u8 CPUPhys_ಬಿಟ್ಟುಗಳು;
ಬಾಹ್ಯ u8 CPULong_ಬಹುಲಕ;

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ runningOnQEMU(ಶೂನ್ಯ) {
    ಹಿಂತಿರುಗಿ CONFIG_QEMU || (
        CONFIG_QEMU_HARDWARE && GET_GLOBAL(PlatformRunningOn) & PF_QEMU);
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ runningOnXen(ಶೂನ್ಯ) {
    ಹಿಂತಿರುಗಿ CONFIG_XEN && GET_GLOBAL(PlatformRunningOn) & PF_XEN;
}
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ runningOnKVM(ಶೂನ್ಯ) {
    ಹಿಂತಿರುಗಿ CONFIG_QEMU && GET_GLOBAL(PlatformRunningOn) & PF_KVM;
}

// Common paravirt ports.
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_SMI_CMD                0x00b2
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_SMI_ಸ್ಥಿತಿ             0x00b3
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_QEMU_CFG_CTL           0x0510
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_QEMU_CFG_ದತ್ತಾಂಶ          0x0511
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_QEMU_CFG_DMA_ADDR_ಮೇಲಿನ 0x0514
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_QEMU_CFG_DMA_ADDR_ಕೆಳಗಿನ  0x0518

// QEMU_CFG_DMA_CONTROL bits
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_DMA_CTL_ERROR   0x01
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_DMA_CTL_READ    0x02
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_DMA_CTL_SKIP    0x04
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_DMA_CTL_SELECT  0x08
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_DMA_CTL_WRITE   0x10

// QEMU_CFG_DMA ID bit
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_CFG_ಆವೃತ್ತಿ_DMA    2

// QEMU debugcon read value
#ವ್ಯಾಖ್ಯಾನಿಸು QEMU_DEBUGCON_READBACK  0xe9

ಪೂರ್ಣಾಂಕ qemu_cfg_enabled(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ qemu_cfg_dma_enabled(ಶೂನ್ಯ);
ಶೂನ್ಯ qemu_preinit(ಶೂನ್ಯ);
ಶೂನ್ಯ qemu_platform_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ qemu_cfg_init(ಶೂನ್ಯ);

u16 qemu_get_present_cpus_ಎಣಿಕೆ(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ qemu_cfg_write_file(ಶೂನ್ಯ *src, ರಚನೆ romfile_s *file, u32 ಸ್ಥಾನಾಂತರ_2, u32 len);
ಪೂರ್ಣಾಂಕ qemu_cfg_write_file_simple(ಶೂನ್ಯ *src, u16 ಕೀಲಿ, u32 ಸ್ಥಾನಾಂತರ_2, u32 len);
u16 qemu_get_romfile_ಕೀಲಿ(ರಚನೆ romfile_s *file);

#ಆದರೆ_ಅಂತ್ಯ

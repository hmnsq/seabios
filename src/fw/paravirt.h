#ifndef __PV_H
#define __PV_H

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBAL
#ಅನ್ತರ್ಭಾವ "romfile.h" // struct romfile_s

// kvmclock
ಸಂರಚನಾ pvclock_vcpu_time_info {
	u32   version;
	u32   pad0;
	u64   tsc_timestamp;
	u64   system_time;
	u32   tsc_to_system_mul;
	s8    tsc_shift;
	u8    flags;
	u8    pad[2];
} __attribute__((__packed__)); /* 32 bytes */

// Types of paravirtualized platforms.
#define PF_QEMU     (1<<0)
#define PF_XEN      (1<<1)
#define PF_KVM      (1<<2)

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ QemuCfgDmaAccess {
    u32 control;
    u32 length;
    u64 address;
} PACKED QemuCfgDmaAccess;

ಬಾಹ್ಯ u32 RamSize;
ಬಾಹ್ಯ u64 RamSizeOver4G;
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ PlatformRunningOn;
ಬಾಹ್ಯ u8 CPUPhysBits;
ಬಾಹ್ಯ u8 CPULongMode;

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ runningOnQEMU(ಶೂನ್ಯ) {
    ಪ್ರತ್ಯಾಗಮ CONFIG_QEMU || (
        CONFIG_QEMU_HARDWARE && GET_GLOBAL(PlatformRunningOn) & PF_QEMU);
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ runningOnXen(ಶೂನ್ಯ) {
    ಪ್ರತ್ಯಾಗಮ CONFIG_XEN && GET_GLOBAL(PlatformRunningOn) & PF_XEN;
}
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ runningOnKVM(ಶೂನ್ಯ) {
    ಪ್ರತ್ಯಾಗಮ CONFIG_QEMU && GET_GLOBAL(PlatformRunningOn) & PF_KVM;
}

// Common paravirt ports.
#define PORT_SMI_CMD                0x00b2
#define PORT_SMI_STATUS             0x00b3
#define PORT_QEMU_CFG_CTL           0x0510
#define PORT_QEMU_CFG_DATA          0x0511
#define PORT_QEMU_CFG_DMA_ADDR_ಮೇಲಿನ 0x0514
#define PORT_QEMU_CFG_DMA_ADDR_ಕೆಳಗಿನ  0x0518

// QEMU_CFG_DMA_CONTROL bits
#define QEMU_CFG_DMA_CTL_ERROR   0x01
#define QEMU_CFG_DMA_CTL_READ    0x02
#define QEMU_CFG_DMA_CTL_SKIP    0x04
#define QEMU_CFG_DMA_CTL_SELECT  0x08
#define QEMU_CFG_DMA_CTL_WRITE   0x10

// QEMU_CFG_DMA ID bit
#define QEMU_CFG_VERSION_DMA    2

// QEMU debugcon read value
#define QEMU_DEBUGCON_READBACK  0xe9

ಪೂರ್ಣಾಙ್ಕ qemu_cfg_enabled(ಶೂನ್ಯ);
ಪೂರ್ಣಾಙ್ಕ qemu_cfg_dma_enabled(ಶೂನ್ಯ);
ಶೂನ್ಯ qemu_preinit(ಶೂನ್ಯ);
ಶೂನ್ಯ qemu_platform_setup(ಶೂನ್ಯ);
ಶೂನ್ಯ qemu_cfg_init(ಶೂನ್ಯ);

u16 qemu_get_present_cpus_ಎಣಿಕೆ(ಶೂನ್ಯ);
ಪೂರ್ಣಾಙ್ಕ qemu_cfg_write_file(ಶೂನ್ಯ *src, ಸಂರಚನಾ romfile_s *file, u32 offset, u32 len);
ಪೂರ್ಣಾಙ್ಕ qemu_cfg_write_file_simple(ಶೂನ್ಯ *src, u16 ಕೀಲಿ, u32 offset, u32 len);
u16 qemu_get_romfile_ಕೀಲಿ(ಸಂರಚನಾ romfile_s *file);

#endif

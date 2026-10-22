#ifndef __PV_H
#define __PV_H

#شامل "config.h" // CONFIG_*
#شامل "biosvar.h" // GET_GLOBAL
#شامل "romfile.h" // struct romfile_s

// kvmclock
ساختار pvclock_vcpu_time_info {
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

تعریفنوع ساختار QemuCfgDmaAccess {
    u32 control;
    u32 length;
    u64 address;
} PACKED QemuCfgDmaAccess;

بیرونی u32 RamSize;
بیرونی u64 RamSizeOver4G;
بیرونی صحیح PlatformRunningOn;
بیرونی u8 CPUPhysBits;
بیرونی u8 CPULongMode;

ایستا درونخطی صحیح runningOnQEMU(تهی) {
    بازگشت CONFIG_QEMU || (
        CONFIG_QEMU_HARDWARE && GET_GLOBAL(PlatformRunningOn) & PF_QEMU);
}
ایستا درونخطی صحیح runningOnXen(تهی) {
    بازگشت CONFIG_XEN && GET_GLOBAL(PlatformRunningOn) & PF_XEN;
}
ایستا درونخطی صحیح runningOnKVM(تهی) {
    بازگشت CONFIG_QEMU && GET_GLOBAL(PlatformRunningOn) & PF_KVM;
}

// Common paravirt ports.
#define PORT_SMI_CMD                0x00b2
#define PORT_SMI_STATUS             0x00b3
#define PORT_QEMU_CFG_CTL           0x0510
#define PORT_QEMU_CFG_DATA          0x0511
#define PORT_QEMU_CFG_DMA_ADDR_بالا 0x0514
#define PORT_QEMU_CFG_DMA_ADDR_کم  0x0518

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

صحیح qemu_cfg_enabled(تهی);
صحیح qemu_cfg_dma_enabled(تهی);
تهی qemu_preinit(تهی);
تهی qemu_platform_setup(تهی);
تهی qemu_cfg_init(تهی);

u16 qemu_get_present_cpus_شمار(تهی);
صحیح qemu_cfg_write_file(تهی *src, ساختار romfile_s *file, u32 offset, u32 len);
صحیح qemu_cfg_write_file_simple(تهی *src, u16 کلید, u32 offset, u32 len);
u16 qemu_get_romfile_کلید(ساختار romfile_s *file);

#endif

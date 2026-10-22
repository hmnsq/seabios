#ifndef __PV_H
#define __PV_H

#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "biosvar.h" // GET_GLOBAL
#অন্তর্ভাব "romfile.h" // struct romfile_s

// kvmclock
সংরচনা pvclock_vcpu_time_info {
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

প্রকারনির্দেশ সংরচনা QemuCfgDmaAccess {
    u32 control;
    u32 length;
    u64 address;
} PACKED QemuCfgDmaAccess;

বাহ্য u32 RamSize;
বাহ্য u64 RamSizeOver4G;
বাহ্য পূর্ণাঙ্ক PlatformRunningOn;
বাহ্য u8 CPUPhysBits;
বাহ্য u8 CPULongMode;

স্থির অন্তঃস্থ পূর্ণাঙ্ক runningOnQEMU(শূন্য) {
    প্রত্যাগম CONFIG_QEMU || (
        CONFIG_QEMU_HARDWARE && GET_GLOBAL(PlatformRunningOn) & PF_QEMU);
}
স্থির অন্তঃস্থ পূর্ণাঙ্ক runningOnXen(শূন্য) {
    প্রত্যাগম CONFIG_XEN && GET_GLOBAL(PlatformRunningOn) & PF_XEN;
}
স্থির অন্তঃস্থ পূর্ণাঙ্ক runningOnKVM(শূন্য) {
    প্রত্যাগম CONFIG_QEMU && GET_GLOBAL(PlatformRunningOn) & PF_KVM;
}

// Common paravirt ports.
#define PORT_SMI_CMD                0x00b2
#define PORT_SMI_STATUS             0x00b3
#define PORT_QEMU_CFG_CTL           0x0510
#define PORT_QEMU_CFG_DATA          0x0511
#define PORT_QEMU_CFG_DMA_ADDR_উপরের 0x0514
#define PORT_QEMU_CFG_DMA_ADDR_নিচের  0x0518

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

পূর্ণাঙ্ক qemu_cfg_enabled(শূন্য);
পূর্ণাঙ্ক qemu_cfg_dma_enabled(শূন্য);
শূন্য qemu_preinit(শূন্য);
শূন্য qemu_platform_setup(শূন্য);
শূন্য qemu_cfg_init(শূন্য);

u16 qemu_get_present_cpus_গুনতি(শূন্য);
পূর্ণাঙ্ক qemu_cfg_write_file(শূন্য *src, সংরচনা romfile_s *file, u32 offset, u32 len);
পূর্ণাঙ্ক qemu_cfg_write_file_simple(শূন্য *src, u16 চাবি, u32 offset, u32 len);
u16 qemu_get_romfile_চাবি(সংরচনা romfile_s *file);

#endif

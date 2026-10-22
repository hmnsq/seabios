#ifndef __PV_H
#define __PV_H

#ضمن "config.h" // CONFIG_*
#ضمن "biosvar.h" // GET_GLOBAL
#ضمن "romfile.h" // struct romfile_s

// kvmclock
بنية pvclock_vcpu_time_info {
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

تعريفنوع بنية QemuCfgDmaAccess {
    u32 control;
    u32 length;
    u64 address;
} PACKED QemuCfgDmaAccess;

خارجي u32 RamSize;
خارجي u64 RamSizeOver4G;
خارجي صحيح PlatformRunningOn;
خارجي u8 CPUPhysBits;
خارجي u8 CPULongMode;

ساكن مضمن صحيح runningOnQEMU(فراغ) {
    أرجع CONFIG_QEMU || (
        CONFIG_QEMU_HARDWARE && GET_GLOBAL(PlatformRunningOn) & PF_QEMU);
}
ساكن مضمن صحيح runningOnXen(فراغ) {
    أرجع CONFIG_XEN && GET_GLOBAL(PlatformRunningOn) & PF_XEN;
}
ساكن مضمن صحيح runningOnKVM(فراغ) {
    أرجع CONFIG_QEMU && GET_GLOBAL(PlatformRunningOn) & PF_KVM;
}

// Common paravirt ports.
#define PORT_SMI_CMD                0x00b2
#define PORT_SMI_STATUS             0x00b3
#define PORT_QEMU_CFG_CTL           0x0510
#define PORT_QEMU_CFG_DATA          0x0511
#define PORT_QEMU_CFG_DMA_ADDR_HIGH 0x0514
#define PORT_QEMU_CFG_DMA_ADDR_LOW  0x0518

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

صحيح qemu_cfg_enabled(فراغ);
صحيح qemu_cfg_dma_enabled(فراغ);
فراغ qemu_preinit(فراغ);
فراغ qemu_platform_setup(فراغ);
فراغ qemu_cfg_init(فراغ);

u16 qemu_get_present_cpus_count(فراغ);
صحيح qemu_cfg_write_file(فراغ *src, بنية romfile_s *file, u32 offset, u32 len);
صحيح qemu_cfg_write_file_simple(فراغ *src, u16 key, u32 offset, u32 len);
u16 qemu_get_romfile_key(بنية romfile_s *file);

#endif

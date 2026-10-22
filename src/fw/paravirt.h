#ifndef __PV_H
#define __PV_H

#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "biosvar.h" // GET_GLOBAL
#அந்தர்பாவ "romfile.h" // struct romfile_s

// kvmclock
கட்டமைப்பு pvclock_vcpu_time_info {
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

வகைவரை கட்டமைப்பு QemuCfgDmaAccess {
    u32 control;
    u32 length;
    u64 address;
} PACKED QemuCfgDmaAccess;

வெளி u32 RamSize;
வெளி u64 RamSizeOver4G;
வெளி முழு PlatformRunningOn;
வெளி u8 CPUPhysBits;
வெளி u8 CPULongMode;

நிலையான உள்வரி முழு runningOnQEMU(வெறுமை) {
    திரும்பு CONFIG_QEMU || (
        CONFIG_QEMU_HARDWARE && GET_GLOBAL(PlatformRunningOn) & PF_QEMU);
}
நிலையான உள்வரி முழு runningOnXen(வெறுமை) {
    திரும்பு CONFIG_XEN && GET_GLOBAL(PlatformRunningOn) & PF_XEN;
}
நிலையான உள்வரி முழு runningOnKVM(வெறுமை) {
    திரும்பு CONFIG_QEMU && GET_GLOBAL(PlatformRunningOn) & PF_KVM;
}

// Common paravirt ports.
#define PORT_SMI_CMD                0x00b2
#define PORT_SMI_STATUS             0x00b3
#define PORT_QEMU_CFG_CTL           0x0510
#define PORT_QEMU_CFG_DATA          0x0511
#define PORT_QEMU_CFG_DMA_ADDR_உயர்வு 0x0514
#define PORT_QEMU_CFG_DMA_ADDR_கீழ்  0x0518

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

முழு qemu_cfg_enabled(வெறுமை);
முழு qemu_cfg_dma_enabled(வெறுமை);
வெறுமை qemu_preinit(வெறுமை);
வெறுமை qemu_platform_setup(வெறுமை);
வெறுமை qemu_cfg_init(வெறுமை);

u16 qemu_get_present_cpus_மொத்தம்(வெறுமை);
முழு qemu_cfg_write_file(வெறுமை *src, கட்டமைப்பு romfile_s *file, u32 offset, u32 len);
முழு qemu_cfg_write_file_simple(வெறுமை *src, u16 சாவி, u32 offset, u32 len);
u16 qemu_get_romfile_சாவி(கட்டமைப்பு romfile_s *file);

#endif

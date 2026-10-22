#ifndef __PV_H
#define __PV_H

#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "biosvar.h" // GET_GLOBAL
#അന്തര്ഭാവ "romfile.h" // struct romfile_s

// kvmclock
ഘടന pvclock_vcpu_time_info {
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

തരനിർവചനം ഘടന QemuCfgDmaAccess {
    u32 control;
    u32 length;
    u64 address;
} PACKED QemuCfgDmaAccess;

ബാഹ്യ u32 RamSize;
ബാഹ്യ u64 RamSizeOver4G;
ബാഹ്യ പൂർണ്ണസംഖ്യ PlatformRunningOn;
ബാഹ്യ u8 CPUPhysBits;
ബാഹ്യ u8 CPULongMode;

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ runningOnQEMU(ശൂന്യം) {
    തിരികെ CONFIG_QEMU || (
        CONFIG_QEMU_HARDWARE && GET_GLOBAL(PlatformRunningOn) & PF_QEMU);
}
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ runningOnXen(ശൂന്യം) {
    തിരികെ CONFIG_XEN && GET_GLOBAL(PlatformRunningOn) & PF_XEN;
}
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ runningOnKVM(ശൂന്യം) {
    തിരികെ CONFIG_QEMU && GET_GLOBAL(PlatformRunningOn) & PF_KVM;
}

// Common paravirt ports.
#define PORT_SMI_CMD                0x00b2
#define PORT_SMI_STATUS             0x00b3
#define PORT_QEMU_CFG_CTL           0x0510
#define PORT_QEMU_CFG_DATA          0x0511
#define PORT_QEMU_CFG_DMA_ADDR_മുകളിലെ 0x0514
#define PORT_QEMU_CFG_DMA_ADDR_താഴെയുള്ള  0x0518

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

പൂർണ്ണസംഖ്യ qemu_cfg_enabled(ശൂന്യം);
പൂർണ്ണസംഖ്യ qemu_cfg_dma_enabled(ശൂന്യം);
ശൂന്യം qemu_preinit(ശൂന്യം);
ശൂന്യം qemu_platform_setup(ശൂന്യം);
ശൂന്യം qemu_cfg_init(ശൂന്യം);

u16 qemu_get_present_cpus_എണ്ണം(ശൂന്യം);
പൂർണ്ണസംഖ്യ qemu_cfg_write_file(ശൂന്യം *src, ഘടന romfile_s *file, u32 offset, u32 len);
പൂർണ്ണസംഖ്യ qemu_cfg_write_file_simple(ശൂന്യം *src, u16 താക്കോൽ, u32 offset, u32 len);
u16 qemu_get_romfile_താക്കോൽ(ഘടന romfile_s *file);

#endif

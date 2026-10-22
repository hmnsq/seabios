#ifndef __PV_H
#define __PV_H

#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "biosvar.h" // GET_GLOBAL
#અન્તર્ભાવ "romfile.h" // struct romfile_s

// kvmclock
સંરચના pvclock_vcpu_time_info {
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

પ્રકારવ્યાખ્યા સંરચના QemuCfgDmaAccess {
    u32 control;
    u32 length;
    u64 address;
} PACKED QemuCfgDmaAccess;

બાહ્ય u32 RamSize;
બાહ્ય u64 RamSizeOver4G;
બાહ્ય પૂર્ણાંક PlatformRunningOn;
બાહ્ય u8 CPUPhysBits;
બાહ્ય u8 CPULongMode;

સ્થિર અંતઃરેખ પૂર્ણાંક runningOnQEMU(શૂન્ય) {
    પાછા CONFIG_QEMU || (
        CONFIG_QEMU_HARDWARE && GET_GLOBAL(PlatformRunningOn) & PF_QEMU);
}
સ્થિર અંતઃરેખ પૂર્ણાંક runningOnXen(શૂન્ય) {
    પાછા CONFIG_XEN && GET_GLOBAL(PlatformRunningOn) & PF_XEN;
}
સ્થિર અંતઃરેખ પૂર્ણાંક runningOnKVM(શૂન્ય) {
    પાછા CONFIG_QEMU && GET_GLOBAL(PlatformRunningOn) & PF_KVM;
}

// Common paravirt ports.
#define PORT_SMI_CMD                0x00b2
#define PORT_SMI_STATUS             0x00b3
#define PORT_QEMU_CFG_CTL           0x0510
#define PORT_QEMU_CFG_DATA          0x0511
#define PORT_QEMU_CFG_DMA_ADDR_ઉપલું 0x0514
#define PORT_QEMU_CFG_DMA_ADDR_નીચલું  0x0518

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

પૂર્ણાંક qemu_cfg_enabled(શૂન્ય);
પૂર્ણાંક qemu_cfg_dma_enabled(શૂન્ય);
શૂન્ય qemu_preinit(શૂન્ય);
શૂન્ય qemu_platform_setup(શૂન્ય);
શૂન્ય qemu_cfg_init(શૂન્ય);

u16 qemu_get_present_cpus_ગણતરી(શૂન્ય);
પૂર્ણાંક qemu_cfg_write_file(શૂન્ય *src, સંરચના romfile_s *file, u32 offset, u32 len);
પૂર્ણાંક qemu_cfg_write_file_simple(શૂન્ય *src, u16 ચાવી, u32 offset, u32 len);
u16 qemu_get_romfile_ચાવી(સંરચના romfile_s *file);

#endif

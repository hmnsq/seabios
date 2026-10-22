#جيڪڏهن_وصف_نه_ٿيل __PV_H
#وصف_ڪريو __PV_H

#ضمن "config.h" // CONFIG_*
#ضمن "biosvar.h" // GET_GLOBAL
#ضمن "romfile.h" // struct romfile_s

// kvmclock
بنية pvclock_vcpu_وقت_info {
	u32   نسخو_2;
	u32   pad0;
	u64   tsc_timestamp;
	u64   system_وقت;
	u32   tsc_منزل_system_mul;
	s8    tsc_سرڪايو;
	u8    flags;
	u8    pad[2];
} __attribute__((__packed__)); /* 32 bytes */

// Types of paravirtualized platforms.
#وصف_ڪريو PF_QEMU     (1<<0)
#وصف_ڪريو PF_XEN      (1<<1)
#وصف_ڪريو PF_KVM      (1<<2)

تعريفنوع بنية QemuCfgDmaAccess {
    u32 control;
    u32 ڊيگهائي;
    u64 پتو;
} PACKED QemuCfgDmaAccess;

خارجي u32 Ram_سائيز;
خارجي u64 Ram_سائيز_Over4_G;
خارجي صحيح PlatformRunningOn;
خارجي u8 CPUPhys_بٽ;
خارجي u8 CPULong_گهڻائي;

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
#وصف_ڪريو پورٽ_SMI_CMD                0x00b2
#وصف_ڪريو پورٽ_SMI_حالت             0x00b3
#وصف_ڪريو پورٽ_QEMU_CFG_CTL           0x0510
#وصف_ڪريو پورٽ_QEMU_CFG_معلومات          0x0511
#وصف_ڪريو پورٽ_QEMU_CFG_DMA_ADDR_مٿاهون 0x0514
#وصف_ڪريو پورٽ_QEMU_CFG_DMA_ADDR_هيٺيون  0x0518

// QEMU_CFG_DMA_CONTROL bits
#وصف_ڪريو QEMU_CFG_DMA_CTL_ERROR   0x01
#وصف_ڪريو QEMU_CFG_DMA_CTL_READ    0x02
#وصف_ڪريو QEMU_CFG_DMA_CTL_SKIP    0x04
#وصف_ڪريو QEMU_CFG_DMA_CTL_SELECT  0x08
#وصف_ڪريو QEMU_CFG_DMA_CTL_WRITE   0x10

// QEMU_CFG_DMA ID bit
#وصف_ڪريو QEMU_CFG_نسخو_DMA    2

// QEMU debugcon read value
#وصف_ڪريو QEMU_DEBUGCON_READBACK  0xe9

صحيح qemu_cfg_enabled(فراغ);
صحيح qemu_cfg_dma_enabled(فراغ);
فراغ qemu_preinit(فراغ);
فراغ qemu_platform_تياري(فراغ);
فراغ qemu_cfg_init(فراغ);

u16 qemu_get_present_cpus_ڳڻپ(فراغ);
صحيح qemu_cfg_write_file(فراغ *src, بنية romfile_s *file, u32 وٿي_2, u32 len);
صحيح qemu_cfg_write_file_simple(فراغ *src, u16 ڪنجي, u32 وٿي_2, u32 len);
u16 qemu_get_romfile_ڪنجي(بنية romfile_s *file);

#جيڪڏهن_پورو

#که_نه_تعریف_شوی __PV_H
#تعریف_کړه __PV_H

#ضمن "config.h" // CONFIG_*
#ضمن "biosvar.h" // GET_GLOBAL
#ضمن "romfile.h" // struct romfile_s

// kvmclock
بنية pvclock_vcpu_وخت_info {
	u32   نسخه_2;
	u32   pad0;
	u64   tsc_timestamp;
	u64   system_وخت;
	u32   tsc_منزل_system_mul;
	s8    tsc_ښويدنه;
	u8    flags;
	u8    pad[2];
} __attribute__((__packed__)); /* 32 bytes */

// Types of paravirtualized platforms.
#تعریف_کړه PF_QEMU     (1<<0)
#تعریف_کړه PF_XEN      (1<<1)
#تعریف_کړه PF_KVM      (1<<2)

تعريفنوع بنية QemuCfgDmaAccess {
    u32 control;
    u32 اوږدتیا;
    u64 پته;
} PACKED QemuCfgDmaAccess;

خارجي u32 Ram_اندازه;
خارجي u64 Ram_اندازه_Over4_G;
خارجي صحيح PlatformRunningOn;
خارجي u8 CPUPhys_بټونه;
خارجي u8 CPULong_ډېرپېښ;

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
#تعریف_کړه پورټ_SMI_CMD                0x00b2
#تعریف_کړه پورټ_SMI_حالت             0x00b3
#تعریف_کړه پورټ_QEMU_CFG_CTL           0x0510
#تعریف_کړه پورټ_QEMU_CFG_معلومات          0x0511
#تعریف_کړه پورټ_QEMU_CFG_DMA_ADDR_لوړ 0x0514
#تعریف_کړه پورټ_QEMU_CFG_DMA_ADDR_ټیټ  0x0518

// QEMU_CFG_DMA_CONTROL bits
#تعریف_کړه QEMU_CFG_DMA_CTL_ERROR   0x01
#تعریف_کړه QEMU_CFG_DMA_CTL_READ    0x02
#تعریف_کړه QEMU_CFG_DMA_CTL_SKIP    0x04
#تعریف_کړه QEMU_CFG_DMA_CTL_SELECT  0x08
#تعریف_کړه QEMU_CFG_DMA_CTL_WRITE   0x10

// QEMU_CFG_DMA ID bit
#تعریف_کړه QEMU_CFG_نسخه_DMA    2

// QEMU debugcon read value
#تعریف_کړه QEMU_DEBUGCON_READBACK  0xe9

صحيح qemu_cfg_enabled(فراغ);
صحيح qemu_cfg_dma_enabled(فراغ);
فراغ qemu_preinit(فراغ);
فراغ qemu_platform_تیاری(فراغ);
فراغ qemu_cfg_init(فراغ);

u16 qemu_get_present_cpus_شمېر(فراغ);
صحيح qemu_cfg_write_file(فراغ *src, بنية romfile_s *file, u32 دننه_واټن_2, u32 len);
صحيح qemu_cfg_write_file_simple(فراغ *src, u16 کلی, u32 دننه_واټن_2, u32 len);
u16 qemu_get_romfile_کلی(بنية romfile_s *file);

#د_که_پای

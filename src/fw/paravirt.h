#اگر_تعریف_نشده __PV_H
#تعریف_کن __PV_H

#ضمن "config.h" // CONFIG_*
#ضمن "biosvar.h" // GET_GLOBAL
#ضمن "romfile.h" // struct romfile_s

// kvmclock
بنية pvclock_vcpu_زمان_info {
	u32   نسخه_2;
	u32   pad0;
	u64   tsc_timestamp;
	u64   system_زمان;
	u32   tsc_مقصد_system_mul;
	s8    tsc_لغزش;
	u8    flags;
	u8    pad[2];
} __attribute__((__packed__)); /* 32 bytes */

// Types of paravirtualized platforms.
#تعریف_کن PF_QEMU     (1<<0)
#تعریف_کن PF_XEN      (1<<1)
#تعریف_کن PF_KVM      (1<<2)

تعريفنوع بنية QemuCfgDmaAccess {
    u32 control;
    u32 درازی;
    u64 آدرس;
} PACKED QemuCfgDmaAccess;

خارجي u32 Ram_اندازه;
خارجي u64 Ram_اندازه_Over4_G;
خارجي صحيح PlatformRunningOn;
خارجي u8 CPUPhys_بیتها;
خارجي u8 CPULong_نما;

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
#تعریف_کن پورت_SMI_CMD                0x00b2
#تعریف_کن پورت_SMI_وضعیت             0x00b3
#تعریف_کن پورت_QEMU_CFG_CTL           0x0510
#تعریف_کن پورت_QEMU_CFG_داده          0x0511
#تعریف_کن پورت_QEMU_CFG_DMA_ADDR_بالا 0x0514
#تعریف_کن پورت_QEMU_CFG_DMA_ADDR_کم  0x0518

// QEMU_CFG_DMA_CONTROL bits
#تعریف_کن QEMU_CFG_DMA_CTL_ERROR   0x01
#تعریف_کن QEMU_CFG_DMA_CTL_READ    0x02
#تعریف_کن QEMU_CFG_DMA_CTL_SKIP    0x04
#تعریف_کن QEMU_CFG_DMA_CTL_SELECT  0x08
#تعریف_کن QEMU_CFG_DMA_CTL_WRITE   0x10

// QEMU_CFG_DMA ID bit
#تعریف_کن QEMU_CFG_نسخه_DMA    2

// QEMU debugcon read value
#تعریف_کن QEMU_DEBUGCON_READBACK  0xe9

صحيح qemu_cfg_enabled(فراغ);
صحيح qemu_cfg_dma_enabled(فراغ);
فراغ qemu_preinit(فراغ);
فراغ qemu_platform_راهاندازی(فراغ);
فراغ qemu_cfg_init(فراغ);

u16 qemu_get_present_cpus_شمار(فراغ);
صحيح qemu_cfg_write_file(فراغ *src, بنية romfile_s *file, u32 جابه_جایی_2, u32 len);
صحيح qemu_cfg_write_file_simple(فراغ *src, u16 کلید, u32 جابه_جایی_2, u32 len);
u16 qemu_get_romfile_کلید(بنية romfile_s *file);

#پایان_اگر

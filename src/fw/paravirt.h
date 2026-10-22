#جے_بنا_تعریف __PV_H
#تعریف_کرو __PV_H

#ضمن "config.h" // CONFIG_*
#ضمن "biosvar.h" // GET_GLOBAL
#ضمن "romfile.h" // struct romfile_s

// kvmclock
بنية pvclock_vcpu_ویلا_info {
	u32   نسخہ_2;
	u32   pad0;
	u64   tsc_timestamp;
	u64   system_ویلا;
	u32   tsc_منزل_system_mul;
	s8    tsc_کھسکاؤ;
	u8    flags;
	u8    pad[2];
} __attribute__((__packed__)); /* 32 bytes */

// Types of paravirtualized platforms.
#تعریف_کرو PF_QEMU     (1<<0)
#تعریف_کرو PF_XEN      (1<<1)
#تعریف_کرو PF_KVM      (1<<2)

تعريفنوع بنية QemuCfgDmaAccess {
    u32 control;
    u32 لمائی;
    u64 پتہ;
} PACKED QemuCfgDmaAccess;

خارجي u32 Ram_ناپ;
خارجي u64 Ram_ناپ_Over4_G;
خارجي صحيح PlatformRunningOn;
خارجي u8 CPUPhys_بٹ;
خارجي u8 CPULong_منوال;

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
#تعریف_کرو پورٹ_SMI_CMD                0x00b2
#تعریف_کرو پورٹ_SMI_حالت             0x00b3
#تعریف_کرو پورٹ_QEMU_CFG_CTL           0x0510
#تعریف_کرو پورٹ_QEMU_CFG_انکڑے          0x0511
#تعریف_کرو پورٹ_QEMU_CFG_DMA_ADDR_بلند 0x0514
#تعریف_کرو پورٹ_QEMU_CFG_DMA_ADDR_کم  0x0518

// QEMU_CFG_DMA_CONTROL bits
#تعریف_کرو QEMU_CFG_DMA_CTL_ERROR   0x01
#تعریف_کرو QEMU_CFG_DMA_CTL_READ    0x02
#تعریف_کرو QEMU_CFG_DMA_CTL_SKIP    0x04
#تعریف_کرو QEMU_CFG_DMA_CTL_SELECT  0x08
#تعریف_کرو QEMU_CFG_DMA_CTL_WRITE   0x10

// QEMU_CFG_DMA ID bit
#تعریف_کرو QEMU_CFG_نسخہ_DMA    2

// QEMU debugcon read value
#تعریف_کرو QEMU_DEBUGCON_READBACK  0xe9

صحيح qemu_cfg_enabled(فراغ);
صحيح qemu_cfg_dma_enabled(فراغ);
فراغ qemu_preinit(فراغ);
فراغ qemu_platform_تیاری(فراغ);
فراغ qemu_cfg_init(فراغ);

u16 qemu_get_present_cpus_گنتی(فراغ);
صحيح qemu_cfg_write_file(فراغ *src, بنية romfile_s *file, u32 وتھ_2, u32 len);
صحيح qemu_cfg_write_file_simple(فراغ *src, u16 کلید, u32 وتھ_2, u32 len);
u16 qemu_get_romfile_کلید(بنية romfile_s *file);

#جے_انت

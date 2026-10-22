#اگر_غیر_تعریف_شدہ __PV_H
#تعریف_کرو __PV_H

#شامل "config.h" // CONFIG_*
#شامل "biosvar.h" // GET_GLOBAL
#شامل "romfile.h" // struct romfile_s

// kvmclock
ساخت pvclock_vcpu_وقت_info {
	u32   نسخہ_2;
	u32   pad0;
	u64   tsc_timestamp;
	u64   system_وقت;
	u32   tsc_مقصد_system_mul;
	s8    tsc_سرکاؤ;
	u8    flags;
	u8    pad[2];
} __attribute__((__packed__)); /* 32 bytes */

// Types of paravirtualized platforms.
#تعریف_کرو PF_QEMU     (1<<0)
#تعریف_کرو PF_XEN      (1<<1)
#تعریف_کرو PF_KVM      (1<<2)

قسمتعریف ساخت QemuCfgDmaAccess {
    u32 control;
    u32 لمبائی;
    u64 پتہ;
} PACKED QemuCfgDmaAccess;

خارجی u32 Ram_سائز;
خارجی u64 Ram_سائز_Over4_G;
خارجی صحیح PlatformRunningOn;
خارجی u8 CPUPhys_بٹ;
خارجی u8 CPULong_منوال;

ساکن اندرونی صحیح runningOnQEMU(خالی) {
    واپس CONFIG_QEMU || (
        CONFIG_QEMU_HARDWARE && GET_GLOBAL(PlatformRunningOn) & PF_QEMU);
}
ساکن اندرونی صحیح runningOnXen(خالی) {
    واپس CONFIG_XEN && GET_GLOBAL(PlatformRunningOn) & PF_XEN;
}
ساکن اندرونی صحیح runningOnKVM(خالی) {
    واپس CONFIG_QEMU && GET_GLOBAL(PlatformRunningOn) & PF_KVM;
}

// Common paravirt ports.
#تعریف_کرو پورٹ_SMI_CMD                0x00b2
#تعریف_کرو پورٹ_SMI_حالت             0x00b3
#تعریف_کرو پورٹ_QEMU_CFG_CTL           0x0510
#تعریف_کرو پورٹ_QEMU_CFG_ڈیٹا          0x0511
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

صحیح qemu_cfg_enabled(خالی);
صحیح qemu_cfg_dma_enabled(خالی);
خالی qemu_preinit(خالی);
خالی qemu_platform_تیاری(خالی);
خالی qemu_cfg_init(خالی);

u16 qemu_get_present_cpus_گنتی(خالی);
صحیح qemu_cfg_write_file(خالی *src, ساخت romfile_s *file, u32 انحراف_2, u32 len);
صحیح qemu_cfg_write_file_simple(خالی *src, u16 کلید, u32 انحراف_2, u32 len);
u16 qemu_get_romfile_کلید(ساخت romfile_s *file);

#اگر_ختم

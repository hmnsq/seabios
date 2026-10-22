#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __PV_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __PV_H

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBAL
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // struct romfile_s

// kvmclock
ਬਣਤਰ pvclock_vcpu_ਸਮਾਂ_info {
	u32   ਸੰਸਕਰਣ_2;
	u32   pad0;
	u64   tsc_timestamp;
	u64   system_ਸਮਾਂ;
	u32   tsc_ਮੰਜ਼ਿਲ_system_mul;
	s8    tsc_ਖਿਸਕਾਓ;
	u8    flags;
	u8    pad[2];
} __attribute__((__packed__)); /* 32 bytes */

// Types of paravirtualized platforms.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PF_QEMU     (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PF_XEN      (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PF_KVM      (1<<2)

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ QemuCfgDmaAccess {
    u32 control;
    u32 ਲੰਮਾਈ;
    u64 ਪਤਾ;
} PACKED QemuCfgDmaAccess;

ਬਾਹਰੀ u32 Ram_ਮਾਪ;
ਬਾਹਰੀ u64 Ram_ਮਾਪ_Over4_G;
ਬਾਹਰੀ ਪੂਰਨ PlatformRunningOn;
ਬਾਹਰੀ u8 CPUPhys_ਬਿੱਟ;
ਬਾਹਰੀ u8 CPULong_ਬਹੁਲਕ;

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ runningOnQEMU(ਖਾਲੀ) {
    ਵਾਪਸ CONFIG_QEMU || (
        CONFIG_QEMU_HARDWARE && GET_GLOBAL(PlatformRunningOn) & PF_QEMU);
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ runningOnXen(ਖਾਲੀ) {
    ਵਾਪਸ CONFIG_XEN && GET_GLOBAL(PlatformRunningOn) & PF_XEN;
}
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ runningOnKVM(ਖਾਲੀ) {
    ਵਾਪਸ CONFIG_QEMU && GET_GLOBAL(PlatformRunningOn) & PF_KVM;
}

// Common paravirt ports.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_SMI_CMD                0x00b2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_SMI_ਸਥਿਤੀ             0x00b3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_QEMU_CFG_CTL           0x0510
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_QEMU_CFG_ਅੰਕੜੇ          0x0511
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_QEMU_CFG_DMA_ADDR_ਉੱਚਾ 0x0514
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_QEMU_CFG_DMA_ADDR_ਨੀਵਾਂ  0x0518

// QEMU_CFG_DMA_CONTROL bits
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_DMA_CTL_ERROR   0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_DMA_CTL_READ    0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_DMA_CTL_SKIP    0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_DMA_CTL_SELECT  0x08
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_DMA_CTL_WRITE   0x10

// QEMU_CFG_DMA ID bit
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_CFG_ਸੰਸਕਰਣ_DMA    2

// QEMU debugcon read value
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ QEMU_DEBUGCON_READBACK  0xe9

ਪੂਰਨ qemu_cfg_enabled(ਖਾਲੀ);
ਪੂਰਨ qemu_cfg_dma_enabled(ਖਾਲੀ);
ਖਾਲੀ qemu_preinit(ਖਾਲੀ);
ਖਾਲੀ qemu_platform_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ qemu_cfg_init(ਖਾਲੀ);

u16 qemu_get_present_cpus_ਗਿਣਤੀ(ਖਾਲੀ);
ਪੂਰਨ qemu_cfg_write_file(ਖਾਲੀ *src, ਬਣਤਰ romfile_s *file, u32 ਵਿੱਥ_2, u32 len);
ਪੂਰਨ qemu_cfg_write_file_simple(ਖਾਲੀ *src, u16 ਕੁੰਜੀ, u32 ਵਿੱਥ_2, u32 len);
u16 qemu_get_romfile_ਕੁੰਜੀ(ਬਣਤਰ romfile_s *file);

#ਜੇ_ਅੰਤ

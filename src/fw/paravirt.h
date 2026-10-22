#ifndef __PV_H
#define __PV_H

#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "biosvar.h" // GET_GLOBAL
#अन्तर्भाव "romfile.h" // struct romfile_s

// kvmclock
संरचना pvclock_vcpu_time_info {
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

प्रकारनिर्देश संरचना QemuCfgDmaAccess {
    u32 control;
    u32 length;
    u64 address;
} PACKED QemuCfgDmaAccess;

बाह्य u32 RamSize;
बाह्य u64 RamSizeOver4G;
बाह्य पूर्णाङ्क PlatformRunningOn;
बाह्य u8 CPUPhysBits;
बाह्य u8 CPULongMode;

स्थिर अन्तःस्थ पूर्णाङ्क runningOnQEMU(शून्य) {
    पटिनिवत्त CONFIG_QEMU || (
        CONFIG_QEMU_HARDWARE && GET_GLOBAL(PlatformRunningOn) & PF_QEMU);
}
स्थिर अन्तःस्थ पूर्णाङ्क runningOnXen(शून्य) {
    पटिनिवत्त CONFIG_XEN && GET_GLOBAL(PlatformRunningOn) & PF_XEN;
}
स्थिर अन्तःस्थ पूर्णाङ्क runningOnKVM(शून्य) {
    पटिनिवत्त CONFIG_QEMU && GET_GLOBAL(PlatformRunningOn) & PF_KVM;
}

// Common paravirt ports.
#define PORT_SMI_CMD                0x00b2
#define PORT_SMI_STATUS             0x00b3
#define PORT_QEMU_CFG_CTL           0x0510
#define PORT_QEMU_CFG_DATA          0x0511
#define PORT_QEMU_CFG_DMA_ADDR_उच्चः 0x0514
#define PORT_QEMU_CFG_DMA_ADDR_नीचः  0x0518

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

पूर्णाङ्क qemu_cfg_enabled(शून्य);
पूर्णाङ्क qemu_cfg_dma_enabled(शून्य);
शून्य qemu_preinit(शून्य);
शून्य qemu_platform_setup(शून्य);
शून्य qemu_cfg_init(शून्य);

u16 qemu_get_present_cpus_गणनम्(शून्य);
पूर्णाङ्क qemu_cfg_write_file(शून्य *src, संरचना romfile_s *file, u32 offset, u32 len);
पूर्णाङ्क qemu_cfg_write_file_simple(शून्य *src, u16 कुञ्चिका, u32 offset, u32 len);
u16 qemu_get_romfile_कुञ्चिका(संरचना romfile_s *file);

#endif

#यदि_अपरिभाषितम् __PV_H
#परिभाषय __PV_H

#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "biosvar.h" // GET_GLOBAL
#अन्तर्भाव "romfile.h" // struct romfile_s

// kvmclock
संरचना pvclock_vcpu_time_info {
	u32   संस्करणम्_2;
	u32   pad0;
	u64   tsc_timestamp;
	u64   system_time;
	u32   tsc_गन्तव्यम्_system_mul;
	s8    tsc_सरणम्;
	u8    flags;
	u8    pad[2];
} __attribute__((__packed__)); /* 32 bytes */

// Types of paravirtualized platforms.
#परिभाषय PF_QEMU     (1<<0)
#परिभाषय PF_XEN      (1<<1)
#परिभाषय PF_KVM      (1<<2)

प्रकारनिर्देश संरचना QemuCfgDmaAccess {
    u32 control;
    u32 दीर्घता;
    u64 स्थानाङ्कः;
} PACKED QemuCfgDmaAccess;

बाह्य u32 Ram_आकार;
बाह्य u64 Ram_आकार_Over4_G;
बाह्य पूर्णाङ्क PlatformRunningOn;
बाह्य u8 CPUPhys_द्वयङ्काः;
बाह्य u8 CPULong_बहुलकम्;

स्थिर अन्तःस्थ पूर्णाङ्क runningOnQEMU(शून्य) {
    प्रत्यागम CONFIG_QEMU || (
        CONFIG_QEMU_HARDWARE && GET_GLOBAL(PlatformRunningOn) & PF_QEMU);
}
स्थिर अन्तःस्थ पूर्णाङ्क runningOnXen(शून्य) {
    प्रत्यागम CONFIG_XEN && GET_GLOBAL(PlatformRunningOn) & PF_XEN;
}
स्थिर अन्तःस्थ पूर्णाङ्क runningOnKVM(शून्य) {
    प्रत्यागम CONFIG_QEMU && GET_GLOBAL(PlatformRunningOn) & PF_KVM;
}

// Common paravirt ports.
#परिभाषय पत्तनम्_SMI_CMD                0x00b2
#परिभाषय पत्तनम्_SMI_स्थितिः             0x00b3
#परिभाषय पत्तनम्_QEMU_CFG_CTL           0x0510
#परिभाषय पत्तनम्_QEMU_CFG_दत्त          0x0511
#परिभाषय पत्तनम्_QEMU_CFG_DMA_ADDR_उच्चः 0x0514
#परिभाषय पत्तनम्_QEMU_CFG_DMA_ADDR_नीचः  0x0518

// QEMU_CFG_DMA_CONTROL bits
#परिभाषय QEMU_CFG_DMA_CTL_ERROR   0x01
#परिभाषय QEMU_CFG_DMA_CTL_READ    0x02
#परिभाषय QEMU_CFG_DMA_CTL_SKIP    0x04
#परिभाषय QEMU_CFG_DMA_CTL_SELECT  0x08
#परिभाषय QEMU_CFG_DMA_CTL_WRITE   0x10

// QEMU_CFG_DMA ID bit
#परिभाषय QEMU_CFG_संस्करणम्_DMA    2

// QEMU debugcon read value
#परिभाषय QEMU_DEBUGCON_READBACK  0xe9

पूर्णाङ्क qemu_cfg_enabled(शून्य);
पूर्णाङ्क qemu_cfg_dma_enabled(शून्य);
शून्य qemu_preinit(शून्य);
शून्य qemu_platform_सज्जता(शून्य);
शून्य qemu_cfg_init(शून्य);

u16 qemu_get_present_cpus_गणनम्(शून्य);
पूर्णाङ्क qemu_cfg_write_file(शून्य *src, संरचना romfile_s *file, u32 विस्थापनम्_2, u32 len);
पूर्णाङ्क qemu_cfg_write_file_simple(शून्य *src, u16 कुञ्चिका, u32 विस्थापनम्_2, u32 len);
u16 qemu_get_romfile_कुञ्चिका(संरचना romfile_s *file);

#यदि_अन्तः

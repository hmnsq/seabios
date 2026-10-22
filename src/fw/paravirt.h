#अगर_अघोषित __PV_H
#घोषणा __PV_H

#समावेश "config.h" // CONFIG_*
#समावेश "biosvar.h" // GET_GLOBAL
#समावेश "romfile.h" // struct romfile_s

// kvmclock
काष्ठा pvclock_vcpu_time_info {
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
#घोषणा PF_QEMU     (1<<0)
#घोषणा PF_XEN      (1<<1)
#घोषणा PF_KVM      (1<<2)

प्रकार काष्ठा QemuCfgDmaAccess {
    u32 control;
    u32 लंबाई;
    u64 address;
} PACKED QemuCfgDmaAccess;

बाह्य u32 Ram_आकार;
बाह्य u64 Ram_आकार_Over4_G;
बाह्य पूर्णांक PlatformRunningOn;
बाह्य u8 CPUPhysBits;
बाह्य u8 CPULongMode;

अटल अंतरभूत पूर्णांक runningOnQEMU(व्योम) {
    वापस CONFIG_QEMU || (
        CONFIG_QEMU_HARDWARE && GET_GLOBAL(PlatformRunningOn) & PF_QEMU);
}
अटल अंतरभूत पूर्णांक runningOnXen(व्योम) {
    वापस CONFIG_XEN && GET_GLOBAL(PlatformRunningOn) & PF_XEN;
}
अटल अंतरभूत पूर्णांक runningOnKVM(व्योम) {
    वापस CONFIG_QEMU && GET_GLOBAL(PlatformRunningOn) & PF_KVM;
}

// Common paravirt ports.
#घोषणा PORT_SMI_CMD                0x00b2
#घोषणा PORT_SMI_STATUS             0x00b3
#घोषणा PORT_QEMU_CFG_CTL           0x0510
#घोषणा PORT_QEMU_CFG_तथ्य          0x0511
#घोषणा PORT_QEMU_CFG_DMA_ADDR_ऊपरी 0x0514
#घोषणा PORT_QEMU_CFG_DMA_ADDR_निचला  0x0518

// QEMU_CFG_DMA_CONTROL bits
#घोषणा QEMU_CFG_DMA_CTL_ERROR   0x01
#घोषणा QEMU_CFG_DMA_CTL_READ    0x02
#घोषणा QEMU_CFG_DMA_CTL_SKIP    0x04
#घोषणा QEMU_CFG_DMA_CTL_SELECT  0x08
#घोषणा QEMU_CFG_DMA_CTL_WRITE   0x10

// QEMU_CFG_DMA ID bit
#घोषणा QEMU_CFG_VERSION_DMA    2

// QEMU debugcon read value
#घोषणा QEMU_DEBUGCON_READBACK  0xe9

पूर्णांक qemu_cfg_enabled(व्योम);
पूर्णांक qemu_cfg_dma_enabled(व्योम);
व्योम qemu_preinit(व्योम);
व्योम qemu_platform_setup(व्योम);
व्योम qemu_cfg_init(व्योम);

u16 qemu_get_present_cpus_गिनती(व्योम);
पूर्णांक qemu_cfg_write_file(व्योम *src, काष्ठा romfile_s *file, u32 offset, u32 len);
पूर्णांक qemu_cfg_write_file_simple(व्योम *src, u16 कुंजी, u32 offset, u32 len);
u16 qemu_get_romfile_कुंजी(काष्ठा romfile_s *file);

#पूर्ण_अगर

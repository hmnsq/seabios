#جيڪڏهن_وصف_نه_ٿيل __AHCI_H
#وصف_ڪريو __AHCI_H

#ضمن "block.h" // struct drive_s
#ضمن "types.h" // u32

بنية sata_cmd_fis {
    u8 reg;
    u8 pmp_type;
    u8 command;
    u8 feature;

    u8 lba_هيٺيون;
    u8 lba_mid;
    u8 lba_مٿاهون;
    u8 device;

    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
    u8 feature2;

    u8 sector_ڳڻپ;
    u8 sector_count2;
    u8 res_1;
    u8 control;

    u8 res_2[64 - 16];
};

بنية ahci_ctrl_s {
    بنية pci_device *pci_tmp;
    u8  irq;
    فراغ *iobase;
    u32 caps;
    u32 ports;
};

بنية ahci_cmd_s {
    بنية sata_cmd_fis fis;
    u8 atapi[0x20];
    u8 res[0x20];
    بنية {
        u32 بنياد;
        u32 baseu;
        u32 res;
        u32 flags;
    } prdt[];
};

/* command list */
بنية ahci_list_s {
    u32 flags;
    u32 بائيٽون;
    u32 بنياد;
    u32 baseu;
    u32 res[4];
};

بنية ahci_fis_s {
    u8 dsfis[0x1c];  /* dma setup */
    u8 res_1[0x04];
    u8 psfis[0x14];  /* pio setup */
    u8 res_2[0x0c];
    u8 rfis[0x14];   /* d2h register */
    u8 res_3[0x04];
    u8 sdbfis[0x08]; /* set device bits */
    u8 ufis[0x40];   /* unknown */
    u8 res_4[0x60];
};

بنية ahci_پورٽ_s {
    بنية drive_s     drive;
    بنية ahci_ctrl_s *ctrl;
    بنية ahci_list_s *list;
    بنية ahci_fis_s  *fis;
    بنية ahci_cmd_s  *cmd;
    u32                pnr;
    u32                atapi;
    حرف               *desc;
    صحيح                prio;
};

فراغ ahci_تياري(فراغ);
صحيح ahci_process_op(بنية disk_op_s *op);
صحيح ahci_atapi_process_op(بنية disk_op_s *op);

#وصف_ڪريو AHCI_IRQ_ON_SG            (1 << 31)
#وصف_ڪريو AHCI_CMD_ATAPI            (1 << 5)
#وصف_ڪريو AHCI_CMD_WRITE            (1 << 6)
#وصف_ڪريو AHCI_CMD_PREFETCH         (1 << 7)
#وصف_ڪريو AHCI_CMD_RESET            (1 << 8)
#وصف_ڪريو AHCI_CMD_CLR_BUSY         (1 << 10)

#وصف_ڪريو RX_FIS_D2H_REG            0x40 /* offset of D2H Register FIS data */
#وصف_ڪريو RX_FIS_SDB                0x58 /* offset of SDB FIS data */
#وصف_ڪريو RX_FIS_UNK                0x60 /* offset of Unknown FIS data */

/* global controller registers */
#وصف_ڪريو HOST_CAP                  0x00 /* host capabilities */
#وصف_ڪريو HOST_CTL                  0x04 /* global host control */
#وصف_ڪريو HOST_IRQ_STAT             0x08 /* interrupt status */
#وصف_ڪريو HOST_PORTS_IMPL           0x0c /* bitmap of implemented ports */
#وصف_ڪريو HOST_نسخو              0x10 /* AHCI spec. version compliancy */

/* HOST_CTL bits */
#وصف_ڪريو HOST_CTL_RESET            (1 << 0)  /* reset controller; self-clear */
#وصف_ڪريو HOST_CTL_IRQ_EN           (1 << 1)  /* global IRQ enable */
#وصف_ڪريو HOST_CTL_AHCI_EN          (1 << 31) /* AHCI enabled */

/* HOST_CAP bits */
#وصف_ڪريو HOST_CAP_SSC              (1 << 14) /* Slumber capable */
#وصف_ڪريو HOST_CAP_AHCI             (1 << 18) /* AHCI only */
#وصف_ڪريو HOST_CAP_CLO              (1 << 24) /* Command List Override support */
#وصف_ڪريو HOST_CAP_SSS              (1 << 27) /* Staggered Spin-up */
#وصف_ڪريو HOST_CAP_NCQ              (1 << 30) /* Native Command Queueing */
#وصف_ڪريو HOST_CAP_64               (1 << 31) /* PCI DAC (64-bit DMA) support */

/* registers for each SATA port */
#وصف_ڪريو پورٽ_LST_ADDR             0x00 /* command list DMA addr */
#وصف_ڪريو پورٽ_LST_ADDR_HI          0x04 /* command list DMA addr hi */
#وصف_ڪريو پورٽ_FIS_ADDR             0x08 /* FIS rx buf addr */
#وصف_ڪريو پورٽ_FIS_ADDR_HI          0x0c /* FIS rx buf addr hi */
#وصف_ڪريو پورٽ_IRQ_STAT             0x10 /* interrupt status */
#وصف_ڪريو پورٽ_IRQ_نقاب             0x14 /* interrupt enable/disable mask */
#وصف_ڪريو پورٽ_CMD                  0x18 /* port command */
#وصف_ڪريو پورٽ_TFDATA               0x20 /* taskfile data */
#وصف_ڪريو پورٽ_SIG                  0x24 /* device TF signature */
#وصف_ڪريو پورٽ_SCR_STAT             0x28 /* SATA phy register: SStatus */
#وصف_ڪريو پورٽ_SCR_CTL              0x2c /* SATA phy register: SControl */
#وصف_ڪريو پورٽ_SCR_ERR              0x30 /* SATA phy register: SError */
#وصف_ڪريو پورٽ_SCR_ACT              0x34 /* SATA phy register: SActive */
#وصف_ڪريو پورٽ_CMD_ISSUE            0x38 /* command issue */
#وصف_ڪريو پورٽ_RESERVED             0x3c /* reserved */

/* PORT_IRQ_{STAT,MASK} bits */
#وصف_ڪريو پورٽ_IRQ_COLD_PRES        (1 << 31) /* cold presence detect */
#وصف_ڪريو پورٽ_IRQ_TF_ERR           (1 << 30) /* task file error */
#وصف_ڪريو پورٽ_IRQ_HBUS_ERR         (1 << 29) /* host bus fatal error */
#وصف_ڪريو پورٽ_IRQ_HBUS_معلومات_ERR    (1 << 28) /* host bus data error */
#وصف_ڪريو پورٽ_IRQ_IF_ERR           (1 << 27) /* interface fatal error */
#وصف_ڪريو پورٽ_IRQ_IF_NONFATAL      (1 << 26) /* interface non-fatal error */
#وصف_ڪريو پورٽ_IRQ_حد_کان_مٿي         (1 << 24) /* xfer exhausted available S/G */
#وصف_ڪريو پورٽ_IRQ_BAD_PMP          (1 << 23) /* incorrect port multiplier */

#وصف_ڪريو پورٽ_IRQ_PHYRDY           (1 << 22) /* PhyRdy changed */
#وصف_ڪريو پورٽ_IRQ_DEV_ILCK         (1 << 7) /* device interlock */
#وصف_ڪريو پورٽ_IRQ_CONNECT          (1 << 6) /* port connect change status */
#وصف_ڪريو پورٽ_IRQ_SG_پورو          (1 << 5) /* descriptor processed */
#وصف_ڪريو پورٽ_IRQ_UNK_FIS          (1 << 4) /* unknown FIS rx'd */
#وصف_ڪريو پورٽ_IRQ_SDB_FIS          (1 << 3) /* Set Device Bits FIS rx'd */
#وصف_ڪريو پورٽ_IRQ_DMAS_FIS         (1 << 2) /* DMA Setup FIS rx'd */
#وصف_ڪريو پورٽ_IRQ_PIOS_FIS         (1 << 1) /* PIO Setup FIS rx'd */
#وصف_ڪريو پورٽ_IRQ_D2_H_REG_FIS      (1 << 0) /* D2H Register FIS rx'd */

#وصف_ڪريو پورٽ_IRQ_FREEZE           (پورٽ_IRQ_HBUS_ERR | پورٽ_IRQ_IF_ERR |   \
                                   پورٽ_IRQ_CONNECT | پورٽ_IRQ_PHYRDY |    \
                                   پورٽ_IRQ_UNK_FIS)
#وصف_ڪريو پورٽ_IRQ_ERROR            (پورٽ_IRQ_FREEZE | پورٽ_IRQ_TF_ERR |     \
                                   پورٽ_IRQ_HBUS_معلومات_ERR)
#وصف_ڪريو DEF_پورٽ_IRQ              (پورٽ_IRQ_ERROR | پورٽ_IRQ_SG_پورو |     \
                                   پورٽ_IRQ_SDB_FIS | پورٽ_IRQ_DMAS_FIS |  \
                                   پورٽ_IRQ_PIOS_FIS | پورٽ_IRQ_D2_H_REG_FIS)

/* PORT_CMD bits */
#وصف_ڪريو پورٽ_CMD_ATAPI            (1 << 24) /* Device is ATAPI */
#وصف_ڪريو پورٽ_CMD_LIST_ON          (1 << 15) /* cmd list DMA engine running */
#وصف_ڪريو پورٽ_CMD_FIS_ON           (1 << 14) /* FIS DMA engine running */
#وصف_ڪريو پورٽ_CMD_FIS_RX           (1 << 4) /* Enable FIS receive DMA engine */
#وصف_ڪريو پورٽ_CMD_CLO              (1 << 3) /* Command list override */
#وصف_ڪريو پورٽ_CMD_POWER_ON         (1 << 2) /* Power up device */
#وصف_ڪريو پورٽ_CMD_SPIN_مٿي          (1 << 1) /* Spin up device */
#وصف_ڪريو پورٽ_CMD_شروعات            (1 << 0) /* Enable port DMA engine */

#وصف_ڪريو پورٽ_CMD_ICC_نقاب         (0xf << 28) /* i/f ICC state mask */
#وصف_ڪريو پورٽ_CMD_ICC_سرگرم       (0x1 << 28) /* Put i/f in active state */
#وصف_ڪريو پورٽ_CMD_ICC_PARTIAL      (0x2 << 28) /* Put i/f in partial state */
#وصف_ڪريو پورٽ_CMD_ICC_SLUMBER      (0x6 << 28) /* Put i/f in slumber state */

#وصف_ڪريو پورٽ_IRQ_STAT_DHRS        (1 << 0) /* Device to Host Register FIS */
#وصف_ڪريو پورٽ_IRQ_STAT_PSS         (1 << 1) /* PIO Setup FIS */
#وصف_ڪريو پورٽ_IRQ_STAT_DSS         (1 << 2) /* DMA Setup FIS */
#وصف_ڪريو پورٽ_IRQ_STAT_SDBS        (1 << 3) /* Set Device Bits */
#وصف_ڪريو پورٽ_IRQ_STAT_UFS         (1 << 4) /* Unknown FIS */
#وصف_ڪريو پورٽ_IRQ_STAT_DPS         (1 << 5) /* Descriptor Processed */
#وصف_ڪريو پورٽ_IRQ_STAT_PCS         (1 << 6) /* Port Connect Change Status */
#وصف_ڪريو پورٽ_IRQ_STAT_DMPS        (1 << 7) /* Device Mechanical Presence
حالت*/
#وصف_ڪريو پورٽ_IRQ_STAT_PRCS        (1 << 22) /* File Ready Status */
#وصف_ڪريو پورٽ_IRQ_STAT_IPMS        (1 << 23) /* Incorrect Port Multiplier
حالت*/
#وصف_ڪريو پورٽ_IRQ_STAT_OFS         (1 << 24) /* Overflow Status */
#وصف_ڪريو پورٽ_IRQ_STAT_INFS        (1 << 26) /* Interface Non-Fatal Error
حالت*/
#وصف_ڪريو پورٽ_IRQ_STAT_IFS         (1 << 27) /* Interface Fatal Error */
#وصف_ڪريو پورٽ_IRQ_STAT_HBDS        (1 << 28) /* Host Bus Data Error Status */
#وصف_ڪريو پورٽ_IRQ_STAT_HBFS        (1 << 29) /* Host Bus Fatal Error Status */
#وصف_ڪريو پورٽ_IRQ_STAT_TFES        (1 << 30) /* Task File Error Status */
#وصف_ڪريو پورٽ_IRQ_STAT_CPDS        (1 << 31) /* Code Port Detect Status */

#جيڪڏهن_پورو // ahci.h

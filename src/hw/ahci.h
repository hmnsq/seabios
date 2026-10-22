#यदि_अपरिभाषितम् __AHCI_H
#परिभाषय __AHCI_H

#अन्तर्भाव "block.h" // struct drive_s
#अन्तर्भाव "types.h" // u32

संरचना sata_cmd_fis {
    u8 reg;
    u8 pmp_type;
    u8 command;
    u8 feature;

    u8 lba_नीचः;
    u8 lba_mid;
    u8 lba_उच्चः;
    u8 device;

    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
    u8 feature2;

    u8 sector_गणनम्;
    u8 sector_count2;
    u8 res_1;
    u8 control;

    u8 res_2[64 - 16];
};

संरचना ahci_ctrl_s {
    संरचना pci_device *pci_tmp;
    u8  irq;
    शून्य *iobase;
    u32 caps;
    u32 ports;
};

संरचना ahci_cmd_s {
    संरचना sata_cmd_fis fis;
    u8 atapi[0x20];
    u8 res[0x20];
    संरचना {
        u32 आधारः;
        u32 baseu;
        u32 res;
        u32 flags;
    } prdt[];
};

/* command list */
संरचना ahci_list_s {
    u32 flags;
    u32 अष्टकानि;
    u32 आधारः;
    u32 baseu;
    u32 res[4];
};

संरचना ahci_fis_s {
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

संरचना ahci_पत्तनम्_s {
    संरचना drive_s     drive;
    संरचना ahci_ctrl_s *ctrl;
    संरचना ahci_list_s *list;
    संरचना ahci_fis_s  *fis;
    संरचना ahci_cmd_s  *cmd;
    u32                pnr;
    u32                atapi;
    वर्ण               *desc;
    पूर्णाङ्क                prio;
};

शून्य ahci_सज्जता(शून्य);
पूर्णाङ्क ahci_process_op(संरचना disk_op_s *op);
पूर्णाङ्क ahci_atapi_process_op(संरचना disk_op_s *op);

#परिभाषय AHCI_IRQ_ON_SG            (1 << 31)
#परिभाषय AHCI_CMD_ATAPI            (1 << 5)
#परिभाषय AHCI_CMD_WRITE            (1 << 6)
#परिभाषय AHCI_CMD_PREFETCH         (1 << 7)
#परिभाषय AHCI_CMD_RESET            (1 << 8)
#परिभाषय AHCI_CMD_CLR_BUSY         (1 << 10)

#परिभाषय RX_FIS_D2H_REG            0x40 /* offset of D2H Register FIS data */
#परिभाषय RX_FIS_SDB                0x58 /* offset of SDB FIS data */
#परिभाषय RX_FIS_UNK                0x60 /* offset of Unknown FIS data */

/* global controller registers */
#परिभाषय HOST_CAP                  0x00 /* host capabilities */
#परिभाषय HOST_CTL                  0x04 /* global host control */
#परिभाषय HOST_IRQ_STAT             0x08 /* interrupt status */
#परिभाषय HOST_PORTS_IMPL           0x0c /* bitmap of implemented ports */
#परिभाषय HOST_संस्करणम्              0x10 /* AHCI spec. version compliancy */

/* HOST_CTL bits */
#परिभाषय HOST_CTL_RESET            (1 << 0)  /* reset controller; self-clear */
#परिभाषय HOST_CTL_IRQ_EN           (1 << 1)  /* global IRQ enable */
#परिभाषय HOST_CTL_AHCI_EN          (1 << 31) /* AHCI enabled */

/* HOST_CAP bits */
#परिभाषय HOST_CAP_SSC              (1 << 14) /* Slumber capable */
#परिभाषय HOST_CAP_AHCI             (1 << 18) /* AHCI only */
#परिभाषय HOST_CAP_CLO              (1 << 24) /* Command List Override support */
#परिभाषय HOST_CAP_SSS              (1 << 27) /* Staggered Spin-up */
#परिभाषय HOST_CAP_NCQ              (1 << 30) /* Native Command Queueing */
#परिभाषय HOST_CAP_64               (1 << 31) /* PCI DAC (64-bit DMA) support */

/* registers for each SATA port */
#परिभाषय पत्तनम्_LST_ADDR             0x00 /* command list DMA addr */
#परिभाषय पत्तनम्_LST_ADDR_HI          0x04 /* command list DMA addr hi */
#परिभाषय पत्तनम्_FIS_ADDR             0x08 /* FIS rx buf addr */
#परिभाषय पत्तनम्_FIS_ADDR_HI          0x0c /* FIS rx buf addr hi */
#परिभाषय पत्तनम्_IRQ_STAT             0x10 /* interrupt status */
#परिभाषय पत्तनम्_IRQ_आवरणम्             0x14 /* interrupt enable/disable mask */
#परिभाषय पत्तनम्_CMD                  0x18 /* port command */
#परिभाषय पत्तनम्_TFDATA               0x20 /* taskfile data */
#परिभाषय पत्तनम्_SIG                  0x24 /* device TF signature */
#परिभाषय पत्तनम्_SCR_STAT             0x28 /* SATA phy register: SStatus */
#परिभाषय पत्तनम्_SCR_CTL              0x2c /* SATA phy register: SControl */
#परिभाषय पत्तनम्_SCR_ERR              0x30 /* SATA phy register: SError */
#परिभाषय पत्तनम्_SCR_ACT              0x34 /* SATA phy register: SActive */
#परिभाषय पत्तनम्_CMD_ISSUE            0x38 /* command issue */
#परिभाषय पत्तनम्_RESERVED             0x3c /* reserved */

/* PORT_IRQ_{STAT,MASK} bits */
#परिभाषय पत्तनम्_IRQ_COLD_PRES        (1 << 31) /* cold presence detect */
#परिभाषय पत्तनम्_IRQ_TF_ERR           (1 << 30) /* task file error */
#परिभाषय पत्तनम्_IRQ_HBUS_ERR         (1 << 29) /* host bus fatal error */
#परिभाषय पत्तनम्_IRQ_HBUS_दत्त_ERR    (1 << 28) /* host bus data error */
#परिभाषय पत्तनम्_IRQ_IF_ERR           (1 << 27) /* interface fatal error */
#परिभाषय पत्तनम्_IRQ_IF_NONFATAL      (1 << 26) /* interface non-fatal error */
#परिभाषय पत्तनम्_IRQ_अतिप्रवाहः         (1 << 24) /* xfer exhausted available S/G */
#परिभाषय पत्तनम्_IRQ_BAD_PMP          (1 << 23) /* incorrect port multiplier */

#परिभाषय पत्तनम्_IRQ_PHYRDY           (1 << 22) /* PhyRdy changed */
#परिभाषय पत्तनम्_IRQ_DEV_ILCK         (1 << 7) /* device interlock */
#परिभाषय पत्तनम्_IRQ_CONNECT          (1 << 6) /* port connect change status */
#परिभाषय पत्तनम्_IRQ_SG_समाप्तम्          (1 << 5) /* descriptor processed */
#परिभाषय पत्तनम्_IRQ_UNK_FIS          (1 << 4) /* unknown FIS rx'd */
#परिभाषय पत्तनम्_IRQ_SDB_FIS          (1 << 3) /* Set Device Bits FIS rx'd */
#परिभाषय पत्तनम्_IRQ_DMAS_FIS         (1 << 2) /* DMA Setup FIS rx'd */
#परिभाषय पत्तनम्_IRQ_PIOS_FIS         (1 << 1) /* PIO Setup FIS rx'd */
#परिभाषय पत्तनम्_IRQ_D2_H_REG_FIS      (1 << 0) /* D2H Register FIS rx'd */

#परिभाषय पत्तनम्_IRQ_FREEZE           (पत्तनम्_IRQ_HBUS_ERR | पत्तनम्_IRQ_IF_ERR |   \
                                   पत्तनम्_IRQ_CONNECT | पत्तनम्_IRQ_PHYRDY |    \
                                   पत्तनम्_IRQ_UNK_FIS)
#परिभाषय पत्तनम्_IRQ_ERROR            (पत्तनम्_IRQ_FREEZE | पत्तनम्_IRQ_TF_ERR |     \
                                   पत्तनम्_IRQ_HBUS_दत्त_ERR)
#परिभाषय DEF_पत्तनम्_IRQ              (पत्तनम्_IRQ_ERROR | पत्तनम्_IRQ_SG_समाप्तम् |     \
                                   पत्तनम्_IRQ_SDB_FIS | पत्तनम्_IRQ_DMAS_FIS |  \
                                   पत्तनम्_IRQ_PIOS_FIS | पत्तनम्_IRQ_D2_H_REG_FIS)

/* PORT_CMD bits */
#परिभाषय पत्तनम्_CMD_ATAPI            (1 << 24) /* Device is ATAPI */
#परिभाषय पत्तनम्_CMD_LIST_ON          (1 << 15) /* cmd list DMA engine running */
#परिभाषय पत्तनम्_CMD_FIS_ON           (1 << 14) /* FIS DMA engine running */
#परिभाषय पत्तनम्_CMD_FIS_RX           (1 << 4) /* Enable FIS receive DMA engine */
#परिभाषय पत्तनम्_CMD_CLO              (1 << 3) /* Command list override */
#परिभाषय पत्तनम्_CMD_POWER_ON         (1 << 2) /* Power up device */
#परिभाषय पत्तनम्_CMD_SPIN_ऊर्ध्वम्          (1 << 1) /* Spin up device */
#परिभाषय पत्तनम्_CMD_START            (1 << 0) /* Enable port DMA engine */

#परिभाषय पत्तनम्_CMD_ICC_आवरणम्         (0xf << 28) /* i/f ICC state mask */
#परिभाषय पत्तनम्_CMD_ICC_सक्रिय       (0x1 << 28) /* Put i/f in active state */
#परिभाषय पत्तनम्_CMD_ICC_PARTIAL      (0x2 << 28) /* Put i/f in partial state */
#परिभाषय पत्तनम्_CMD_ICC_SLUMBER      (0x6 << 28) /* Put i/f in slumber state */

#परिभाषय पत्तनम्_IRQ_STAT_DHRS        (1 << 0) /* Device to Host Register FIS */
#परिभाषय पत्तनम्_IRQ_STAT_PSS         (1 << 1) /* PIO Setup FIS */
#परिभाषय पत्तनम्_IRQ_STAT_DSS         (1 << 2) /* DMA Setup FIS */
#परिभाषय पत्तनम्_IRQ_STAT_SDBS        (1 << 3) /* Set Device Bits */
#परिभाषय पत्तनम्_IRQ_STAT_UFS         (1 << 4) /* Unknown FIS */
#परिभाषय पत्तनम्_IRQ_STAT_DPS         (1 << 5) /* Descriptor Processed */
#परिभाषय पत्तनम्_IRQ_STAT_PCS         (1 << 6) /* Port Connect Change Status */
#परिभाषय पत्तनम्_IRQ_STAT_DMPS        (1 << 7) /* Device Mechanical Presence
स्थितिः*/
#परिभाषय पत्तनम्_IRQ_STAT_PRCS        (1 << 22) /* File Ready Status */
#परिभाषय पत्तनम्_IRQ_STAT_IPMS        (1 << 23) /* Incorrect Port Multiplier
स्थितिः*/
#परिभाषय पत्तनम्_IRQ_STAT_OFS         (1 << 24) /* Overflow Status */
#परिभाषय पत्तनम्_IRQ_STAT_INFS        (1 << 26) /* Interface Non-Fatal Error
स्थितिः*/
#परिभाषय पत्तनम्_IRQ_STAT_IFS         (1 << 27) /* Interface Fatal Error */
#परिभाषय पत्तनम्_IRQ_STAT_HBDS        (1 << 28) /* Host Bus Data Error Status */
#परिभाषय पत्तनम्_IRQ_STAT_HBFS        (1 << 29) /* Host Bus Fatal Error Status */
#परिभाषय पत्तनम्_IRQ_STAT_TFES        (1 << 30) /* Task File Error Status */
#परिभाषय पत्तनम्_IRQ_STAT_CPDS        (1 << 31) /* Code Port Detect Status */

#यदि_अन्तः // ahci.h

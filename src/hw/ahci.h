#నిర్వచితం_కాకపోతే __AHCI_H
#నిర్వచించు __AHCI_H

#అన్తర్భావ "block.h" // struct drive_s
#అన్తర్భావ "types.h" // u32

నిర్మాణం sata_cmd_fis {
    u8 reg;
    u8 pmp_type;
    u8 command;
    u8 feature;

    u8 lba_దిగువ;
    u8 lba_mid;
    u8 lba_ఎగువ;
    u8 device;

    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
    u8 feature2;

    u8 sector_లెక్క;
    u8 sector_count2;
    u8 res_1;
    u8 control;

    u8 res_2[64 - 16];
};

నిర్మాణం ahci_ctrl_s {
    నిర్మాణం pci_device *pci_tmp;
    u8  irq;
    శూన్య *iobase;
    u32 caps;
    u32 ports;
};

నిర్మాణం ahci_cmd_s {
    నిర్మాణం sata_cmd_fis fis;
    u8 atapi[0x20];
    u8 res[0x20];
    నిర్మాణం {
        u32 పునాది;
        u32 baseu;
        u32 res;
        u32 flags;
    } prdt[];
};

/* command list */
నిర్మాణం ahci_list_s {
    u32 flags;
    u32 బైట్లు;
    u32 పునాది;
    u32 baseu;
    u32 res[4];
};

నిర్మాణం ahci_fis_s {
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

నిర్మాణం ahci_పోర్టు_s {
    నిర్మాణం drive_s     drive;
    నిర్మాణం ahci_ctrl_s *ctrl;
    నిర్మాణం ahci_list_s *list;
    నిర్మాణం ahci_fis_s  *fis;
    నిర్మాణం ahci_cmd_s  *cmd;
    u32                pnr;
    u32                atapi;
    అక్షరం               *desc;
    పూర్ణాంకం                prio;
};

శూన్య ahci_అమరిక(శూన్య);
పూర్ణాంకం ahci_process_op(నిర్మాణం disk_op_s *op);
పూర్ణాంకం ahci_atapi_process_op(నిర్మాణం disk_op_s *op);

#నిర్వచించు AHCI_IRQ_ON_SG            (1 << 31)
#నిర్వచించు AHCI_CMD_ATAPI            (1 << 5)
#నిర్వచించు AHCI_CMD_WRITE            (1 << 6)
#నిర్వచించు AHCI_CMD_PREFETCH         (1 << 7)
#నిర్వచించు AHCI_CMD_RESET            (1 << 8)
#నిర్వచించు AHCI_CMD_CLR_BUSY         (1 << 10)

#నిర్వచించు RX_FIS_D2H_REG            0x40 /* offset of D2H Register FIS data */
#నిర్వచించు RX_FIS_SDB                0x58 /* offset of SDB FIS data */
#నిర్వచించు RX_FIS_UNK                0x60 /* offset of Unknown FIS data */

/* global controller registers */
#నిర్వచించు HOST_CAP                  0x00 /* host capabilities */
#నిర్వచించు HOST_CTL                  0x04 /* global host control */
#నిర్వచించు HOST_IRQ_STAT             0x08 /* interrupt status */
#నిర్వచించు HOST_PORTS_IMPL           0x0c /* bitmap of implemented ports */
#నిర్వచించు HOST_సంస్కరణ              0x10 /* AHCI spec. version compliancy */

/* HOST_CTL bits */
#నిర్వచించు HOST_CTL_RESET            (1 << 0)  /* reset controller; self-clear */
#నిర్వచించు HOST_CTL_IRQ_EN           (1 << 1)  /* global IRQ enable */
#నిర్వచించు HOST_CTL_AHCI_EN          (1 << 31) /* AHCI enabled */

/* HOST_CAP bits */
#నిర్వచించు HOST_CAP_SSC              (1 << 14) /* Slumber capable */
#నిర్వచించు HOST_CAP_AHCI             (1 << 18) /* AHCI only */
#నిర్వచించు HOST_CAP_CLO              (1 << 24) /* Command List Override support */
#నిర్వచించు HOST_CAP_SSS              (1 << 27) /* Staggered Spin-up */
#నిర్వచించు HOST_CAP_NCQ              (1 << 30) /* Native Command Queueing */
#నిర్వచించు HOST_CAP_64               (1 << 31) /* PCI DAC (64-bit DMA) support */

/* registers for each SATA port */
#నిర్వచించు పోర్టు_LST_ADDR             0x00 /* command list DMA addr */
#నిర్వచించు పోర్టు_LST_ADDR_HI          0x04 /* command list DMA addr hi */
#నిర్వచించు పోర్టు_FIS_ADDR             0x08 /* FIS rx buf addr */
#నిర్వచించు పోర్టు_FIS_ADDR_HI          0x0c /* FIS rx buf addr hi */
#నిర్వచించు పోర్టు_IRQ_STAT             0x10 /* interrupt status */
#నిర్వచించు పోర్టు_IRQ_ముసుగు             0x14 /* interrupt enable/disable mask */
#నిర్వచించు పోర్టు_CMD                  0x18 /* port command */
#నిర్వచించు పోర్టు_TFDATA               0x20 /* taskfile data */
#నిర్వచించు పోర్టు_SIG                  0x24 /* device TF signature */
#నిర్వచించు పోర్టు_SCR_STAT             0x28 /* SATA phy register: SStatus */
#నిర్వచించు పోర్టు_SCR_CTL              0x2c /* SATA phy register: SControl */
#నిర్వచించు పోర్టు_SCR_ERR              0x30 /* SATA phy register: SError */
#నిర్వచించు పోర్టు_SCR_ACT              0x34 /* SATA phy register: SActive */
#నిర్వచించు పోర్టు_CMD_ISSUE            0x38 /* command issue */
#నిర్వచించు పోర్టు_RESERVED             0x3c /* reserved */

/* PORT_IRQ_{STAT,MASK} bits */
#నిర్వచించు పోర్టు_IRQ_COLD_PRES        (1 << 31) /* cold presence detect */
#నిర్వచించు పోర్టు_IRQ_TF_ERR           (1 << 30) /* task file error */
#నిర్వచించు పోర్టు_IRQ_HBUS_ERR         (1 << 29) /* host bus fatal error */
#నిర్వచించు పోర్టు_IRQ_HBUS_దత్తాంశం_ERR    (1 << 28) /* host bus data error */
#నిర్వచించు పోర్టు_IRQ_IF_ERR           (1 << 27) /* interface fatal error */
#నిర్వచించు పోర్టు_IRQ_IF_NONFATAL      (1 << 26) /* interface non-fatal error */
#నిర్వచించు పోర్టు_IRQ_అధికప్రవాహం         (1 << 24) /* xfer exhausted available S/G */
#నిర్వచించు పోర్టు_IRQ_BAD_PMP          (1 << 23) /* incorrect port multiplier */

#నిర్వచించు పోర్టు_IRQ_PHYRDY           (1 << 22) /* PhyRdy changed */
#నిర్వచించు పోర్టు_IRQ_DEV_ILCK         (1 << 7) /* device interlock */
#నిర్వచించు పోర్టు_IRQ_CONNECT          (1 << 6) /* port connect change status */
#నిర్వచించు పోర్టు_IRQ_SG_పూర్తి          (1 << 5) /* descriptor processed */
#నిర్వచించు పోర్టు_IRQ_UNK_FIS          (1 << 4) /* unknown FIS rx'd */
#నిర్వచించు పోర్టు_IRQ_SDB_FIS          (1 << 3) /* Set Device Bits FIS rx'd */
#నిర్వచించు పోర్టు_IRQ_DMAS_FIS         (1 << 2) /* DMA Setup FIS rx'd */
#నిర్వచించు పోర్టు_IRQ_PIOS_FIS         (1 << 1) /* PIO Setup FIS rx'd */
#నిర్వచించు పోర్టు_IRQ_D2_H_REG_FIS      (1 << 0) /* D2H Register FIS rx'd */

#నిర్వచించు పోర్టు_IRQ_FREEZE           (పోర్టు_IRQ_HBUS_ERR | పోర్టు_IRQ_IF_ERR |   \
                                   పోర్టు_IRQ_CONNECT | పోర్టు_IRQ_PHYRDY |    \
                                   పోర్టు_IRQ_UNK_FIS)
#నిర్వచించు పోర్టు_IRQ_ERROR            (పోర్టు_IRQ_FREEZE | పోర్టు_IRQ_TF_ERR |     \
                                   పోర్టు_IRQ_HBUS_దత్తాంశం_ERR)
#నిర్వచించు DEF_పోర్టు_IRQ              (పోర్టు_IRQ_ERROR | పోర్టు_IRQ_SG_పూర్తి |     \
                                   పోర్టు_IRQ_SDB_FIS | పోర్టు_IRQ_DMAS_FIS |  \
                                   పోర్టు_IRQ_PIOS_FIS | పోర్టు_IRQ_D2_H_REG_FIS)

/* PORT_CMD bits */
#నిర్వచించు పోర్టు_CMD_ATAPI            (1 << 24) /* Device is ATAPI */
#నిర్వచించు పోర్టు_CMD_LIST_ON          (1 << 15) /* cmd list DMA engine running */
#నిర్వచించు పోర్టు_CMD_FIS_ON           (1 << 14) /* FIS DMA engine running */
#నిర్వచించు పోర్టు_CMD_FIS_RX           (1 << 4) /* Enable FIS receive DMA engine */
#నిర్వచించు పోర్టు_CMD_CLO              (1 << 3) /* Command list override */
#నిర్వచించు పోర్టు_CMD_POWER_ON         (1 << 2) /* Power up device */
#నిర్వచించు పోర్టు_CMD_SPIN_పైకి          (1 << 1) /* Spin up device */
#నిర్వచించు పోర్టు_CMD_ప్రారంభం            (1 << 0) /* Enable port DMA engine */

#నిర్వచించు పోర్టు_CMD_ICC_ముసుగు         (0xf << 28) /* i/f ICC state mask */
#నిర్వచించు పోర్టు_CMD_ICC_చురుకైన       (0x1 << 28) /* Put i/f in active state */
#నిర్వచించు పోర్టు_CMD_ICC_PARTIAL      (0x2 << 28) /* Put i/f in partial state */
#నిర్వచించు పోర్టు_CMD_ICC_SLUMBER      (0x6 << 28) /* Put i/f in slumber state */

#నిర్వచించు పోర్టు_IRQ_STAT_DHRS        (1 << 0) /* Device to Host Register FIS */
#నిర్వచించు పోర్టు_IRQ_STAT_PSS         (1 << 1) /* PIO Setup FIS */
#నిర్వచించు పోర్టు_IRQ_STAT_DSS         (1 << 2) /* DMA Setup FIS */
#నిర్వచించు పోర్టు_IRQ_STAT_SDBS        (1 << 3) /* Set Device Bits */
#నిర్వచించు పోర్టు_IRQ_STAT_UFS         (1 << 4) /* Unknown FIS */
#నిర్వచించు పోర్టు_IRQ_STAT_DPS         (1 << 5) /* Descriptor Processed */
#నిర్వచించు పోర్టు_IRQ_STAT_PCS         (1 << 6) /* Port Connect Change Status */
#నిర్వచించు పోర్టు_IRQ_STAT_DMPS        (1 << 7) /* Device Mechanical Presence
స్థితి*/
#నిర్వచించు పోర్టు_IRQ_STAT_PRCS        (1 << 22) /* File Ready Status */
#నిర్వచించు పోర్టు_IRQ_STAT_IPMS        (1 << 23) /* Incorrect Port Multiplier
స్థితి*/
#నిర్వచించు పోర్టు_IRQ_STAT_OFS         (1 << 24) /* Overflow Status */
#నిర్వచించు పోర్టు_IRQ_STAT_INFS        (1 << 26) /* Interface Non-Fatal Error
స్థితి*/
#నిర్వచించు పోర్టు_IRQ_STAT_IFS         (1 << 27) /* Interface Fatal Error */
#నిర్వచించు పోర్టు_IRQ_STAT_HBDS        (1 << 28) /* Host Bus Data Error Status */
#నిర్వచించు పోర్టు_IRQ_STAT_HBFS        (1 << 29) /* Host Bus Fatal Error Status */
#నిర్వచించు పోర్టు_IRQ_STAT_TFES        (1 << 30) /* Task File Error Status */
#నిర్వచించు పోర్టు_IRQ_STAT_CPDS        (1 << 31) /* Code Port Detect Status */

#అయితే_ముగింపు // ahci.h

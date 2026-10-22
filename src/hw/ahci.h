#اگر_غیر_تعریف_شدہ __AHCI_H
#تعریف_کرو __AHCI_H

#شامل "block.h" // struct drive_s
#شامل "types.h" // u32

ساخت sata_cmd_fis {
    u8 reg;
    u8 pmp_type;
    u8 command;
    u8 feature;

    u8 lba_کم;
    u8 lba_mid;
    u8 lba_بلند;
    u8 device;

    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
    u8 feature2;

    u8 sector_گنتی;
    u8 sector_count2;
    u8 res_1;
    u8 control;

    u8 res_2[64 - 16];
};

ساخت ahci_ctrl_s {
    ساخت pci_device *pci_tmp;
    u8  irq;
    خالی *iobase;
    u32 caps;
    u32 ports;
};

ساخت ahci_cmd_s {
    ساخت sata_cmd_fis fis;
    u8 atapi[0x20];
    u8 res[0x20];
    ساخت {
        u32 بنیاد;
        u32 baseu;
        u32 res;
        u32 flags;
    } prdt[];
};

/* command list */
ساخت ahci_list_s {
    u32 flags;
    u32 بائٹیں;
    u32 بنیاد;
    u32 baseu;
    u32 res[4];
};

ساخت ahci_fis_s {
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

ساخت ahci_پورٹ_s {
    ساخت drive_s     drive;
    ساخت ahci_ctrl_s *ctrl;
    ساخت ahci_list_s *list;
    ساخت ahci_fis_s  *fis;
    ساخت ahci_cmd_s  *cmd;
    u32                pnr;
    u32                atapi;
    حرف               *desc;
    صحیح                prio;
};

خالی ahci_تیاری(خالی);
صحیح ahci_process_op(ساخت disk_op_s *op);
صحیح ahci_atapi_process_op(ساخت disk_op_s *op);

#تعریف_کرو AHCI_IRQ_ON_SG            (1 << 31)
#تعریف_کرو AHCI_CMD_ATAPI            (1 << 5)
#تعریف_کرو AHCI_CMD_WRITE            (1 << 6)
#تعریف_کرو AHCI_CMD_PREFETCH         (1 << 7)
#تعریف_کرو AHCI_CMD_RESET            (1 << 8)
#تعریف_کرو AHCI_CMD_CLR_BUSY         (1 << 10)

#تعریف_کرو RX_FIS_D2H_REG            0x40 /* offset of D2H Register FIS data */
#تعریف_کرو RX_FIS_SDB                0x58 /* offset of SDB FIS data */
#تعریف_کرو RX_FIS_UNK                0x60 /* offset of Unknown FIS data */

/* global controller registers */
#تعریف_کرو HOST_CAP                  0x00 /* host capabilities */
#تعریف_کرو HOST_CTL                  0x04 /* global host control */
#تعریف_کرو HOST_IRQ_STAT             0x08 /* interrupt status */
#تعریف_کرو HOST_PORTS_IMPL           0x0c /* bitmap of implemented ports */
#تعریف_کرو HOST_نسخہ              0x10 /* AHCI spec. version compliancy */

/* HOST_CTL bits */
#تعریف_کرو HOST_CTL_RESET            (1 << 0)  /* reset controller; self-clear */
#تعریف_کرو HOST_CTL_IRQ_EN           (1 << 1)  /* global IRQ enable */
#تعریف_کرو HOST_CTL_AHCI_EN          (1 << 31) /* AHCI enabled */

/* HOST_CAP bits */
#تعریف_کرو HOST_CAP_SSC              (1 << 14) /* Slumber capable */
#تعریف_کرو HOST_CAP_AHCI             (1 << 18) /* AHCI only */
#تعریف_کرو HOST_CAP_CLO              (1 << 24) /* Command List Override support */
#تعریف_کرو HOST_CAP_SSS              (1 << 27) /* Staggered Spin-up */
#تعریف_کرو HOST_CAP_NCQ              (1 << 30) /* Native Command Queueing */
#تعریف_کرو HOST_CAP_64               (1 << 31) /* PCI DAC (64-bit DMA) support */

/* registers for each SATA port */
#تعریف_کرو پورٹ_LST_ADDR             0x00 /* command list DMA addr */
#تعریف_کرو پورٹ_LST_ADDR_HI          0x04 /* command list DMA addr hi */
#تعریف_کرو پورٹ_FIS_ADDR             0x08 /* FIS rx buf addr */
#تعریف_کرو پورٹ_FIS_ADDR_HI          0x0c /* FIS rx buf addr hi */
#تعریف_کرو پورٹ_IRQ_STAT             0x10 /* interrupt status */
#تعریف_کرو پورٹ_IRQ_نقاب             0x14 /* interrupt enable/disable mask */
#تعریف_کرو پورٹ_CMD                  0x18 /* port command */
#تعریف_کرو پورٹ_TFDATA               0x20 /* taskfile data */
#تعریف_کرو پورٹ_SIG                  0x24 /* device TF signature */
#تعریف_کرو پورٹ_SCR_STAT             0x28 /* SATA phy register: SStatus */
#تعریف_کرو پورٹ_SCR_CTL              0x2c /* SATA phy register: SControl */
#تعریف_کرو پورٹ_SCR_ERR              0x30 /* SATA phy register: SError */
#تعریف_کرو پورٹ_SCR_ACT              0x34 /* SATA phy register: SActive */
#تعریف_کرو پورٹ_CMD_ISSUE            0x38 /* command issue */
#تعریف_کرو پورٹ_RESERVED             0x3c /* reserved */

/* PORT_IRQ_{STAT,MASK} bits */
#تعریف_کرو پورٹ_IRQ_COLD_PRES        (1 << 31) /* cold presence detect */
#تعریف_کرو پورٹ_IRQ_TF_ERR           (1 << 30) /* task file error */
#تعریف_کرو پورٹ_IRQ_HBUS_ERR         (1 << 29) /* host bus fatal error */
#تعریف_کرو پورٹ_IRQ_HBUS_ڈیٹا_ERR    (1 << 28) /* host bus data error */
#تعریف_کرو پورٹ_IRQ_IF_ERR           (1 << 27) /* interface fatal error */
#تعریف_کرو پورٹ_IRQ_IF_NONFATAL      (1 << 26) /* interface non-fatal error */
#تعریف_کرو پورٹ_IRQ_حد_سے_تجاوز         (1 << 24) /* xfer exhausted available S/G */
#تعریف_کرو پورٹ_IRQ_BAD_PMP          (1 << 23) /* incorrect port multiplier */

#تعریف_کرو پورٹ_IRQ_PHYRDY           (1 << 22) /* PhyRdy changed */
#تعریف_کرو پورٹ_IRQ_DEV_ILCK         (1 << 7) /* device interlock */
#تعریف_کرو پورٹ_IRQ_CONNECT          (1 << 6) /* port connect change status */
#تعریف_کرو پورٹ_IRQ_SG_مکمل          (1 << 5) /* descriptor processed */
#تعریف_کرو پورٹ_IRQ_UNK_FIS          (1 << 4) /* unknown FIS rx'd */
#تعریف_کرو پورٹ_IRQ_SDB_FIS          (1 << 3) /* Set Device Bits FIS rx'd */
#تعریف_کرو پورٹ_IRQ_DMAS_FIS         (1 << 2) /* DMA Setup FIS rx'd */
#تعریف_کرو پورٹ_IRQ_PIOS_FIS         (1 << 1) /* PIO Setup FIS rx'd */
#تعریف_کرو پورٹ_IRQ_D2_H_REG_FIS      (1 << 0) /* D2H Register FIS rx'd */

#تعریف_کرو پورٹ_IRQ_FREEZE           (پورٹ_IRQ_HBUS_ERR | پورٹ_IRQ_IF_ERR |   \
                                   پورٹ_IRQ_CONNECT | پورٹ_IRQ_PHYRDY |    \
                                   پورٹ_IRQ_UNK_FIS)
#تعریف_کرو پورٹ_IRQ_ERROR            (پورٹ_IRQ_FREEZE | پورٹ_IRQ_TF_ERR |     \
                                   پورٹ_IRQ_HBUS_ڈیٹا_ERR)
#تعریف_کرو DEF_پورٹ_IRQ              (پورٹ_IRQ_ERROR | پورٹ_IRQ_SG_مکمل |     \
                                   پورٹ_IRQ_SDB_FIS | پورٹ_IRQ_DMAS_FIS |  \
                                   پورٹ_IRQ_PIOS_FIS | پورٹ_IRQ_D2_H_REG_FIS)

/* PORT_CMD bits */
#تعریف_کرو پورٹ_CMD_ATAPI            (1 << 24) /* Device is ATAPI */
#تعریف_کرو پورٹ_CMD_LIST_ON          (1 << 15) /* cmd list DMA engine running */
#تعریف_کرو پورٹ_CMD_FIS_ON           (1 << 14) /* FIS DMA engine running */
#تعریف_کرو پورٹ_CMD_FIS_RX           (1 << 4) /* Enable FIS receive DMA engine */
#تعریف_کرو پورٹ_CMD_CLO              (1 << 3) /* Command list override */
#تعریف_کرو پورٹ_CMD_POWER_ON         (1 << 2) /* Power up device */
#تعریف_کرو پورٹ_CMD_SPIN_اوپر          (1 << 1) /* Spin up device */
#تعریف_کرو پورٹ_CMD_آغاز            (1 << 0) /* Enable port DMA engine */

#تعریف_کرو پورٹ_CMD_ICC_نقاب         (0xf << 28) /* i/f ICC state mask */
#تعریف_کرو پورٹ_CMD_ICC_فعال       (0x1 << 28) /* Put i/f in active state */
#تعریف_کرو پورٹ_CMD_ICC_PARTIAL      (0x2 << 28) /* Put i/f in partial state */
#تعریف_کرو پورٹ_CMD_ICC_SLUMBER      (0x6 << 28) /* Put i/f in slumber state */

#تعریف_کرو پورٹ_IRQ_STAT_DHRS        (1 << 0) /* Device to Host Register FIS */
#تعریف_کرو پورٹ_IRQ_STAT_PSS         (1 << 1) /* PIO Setup FIS */
#تعریف_کرو پورٹ_IRQ_STAT_DSS         (1 << 2) /* DMA Setup FIS */
#تعریف_کرو پورٹ_IRQ_STAT_SDBS        (1 << 3) /* Set Device Bits */
#تعریف_کرو پورٹ_IRQ_STAT_UFS         (1 << 4) /* Unknown FIS */
#تعریف_کرو پورٹ_IRQ_STAT_DPS         (1 << 5) /* Descriptor Processed */
#تعریف_کرو پورٹ_IRQ_STAT_PCS         (1 << 6) /* Port Connect Change Status */
#تعریف_کرو پورٹ_IRQ_STAT_DMPS        (1 << 7) /* Device Mechanical Presence
حالت*/
#تعریف_کرو پورٹ_IRQ_STAT_PRCS        (1 << 22) /* File Ready Status */
#تعریف_کرو پورٹ_IRQ_STAT_IPMS        (1 << 23) /* Incorrect Port Multiplier
حالت*/
#تعریف_کرو پورٹ_IRQ_STAT_OFS         (1 << 24) /* Overflow Status */
#تعریف_کرو پورٹ_IRQ_STAT_INFS        (1 << 26) /* Interface Non-Fatal Error
حالت*/
#تعریف_کرو پورٹ_IRQ_STAT_IFS         (1 << 27) /* Interface Fatal Error */
#تعریف_کرو پورٹ_IRQ_STAT_HBDS        (1 << 28) /* Host Bus Data Error Status */
#تعریف_کرو پورٹ_IRQ_STAT_HBFS        (1 << 29) /* Host Bus Fatal Error Status */
#تعریف_کرو پورٹ_IRQ_STAT_TFES        (1 << 30) /* Task File Error Status */
#تعریف_کرو پورٹ_IRQ_STAT_CPDS        (1 << 31) /* Code Port Detect Status */

#اگر_ختم // ahci.h

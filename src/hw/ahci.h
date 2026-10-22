#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __AHCI_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __AHCI_H

#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // struct drive_s
#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32

ਬਣਤਰ sata_cmd_fis {
    u8 reg;
    u8 pmp_type;
    u8 command;
    u8 feature;

    u8 lba_ਨੀਵਾਂ;
    u8 lba_mid;
    u8 lba_ਉੱਚਾ;
    u8 device;

    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
    u8 feature2;

    u8 sector_ਗਿਣਤੀ;
    u8 sector_count2;
    u8 res_1;
    u8 control;

    u8 res_2[64 - 16];
};

ਬਣਤਰ ahci_ctrl_s {
    ਬਣਤਰ pci_device *pci_tmp;
    u8  irq;
    ਖਾਲੀ *iobase;
    u32 caps;
    u32 ports;
};

ਬਣਤਰ ahci_cmd_s {
    ਬਣਤਰ sata_cmd_fis fis;
    u8 atapi[0x20];
    u8 res[0x20];
    ਬਣਤਰ {
        u32 ਅਧਾਰ;
        u32 baseu;
        u32 res;
        u32 flags;
    } prdt[];
};

/* command list */
ਬਣਤਰ ahci_list_s {
    u32 flags;
    u32 ਬਾਈਟਾਂ;
    u32 ਅਧਾਰ;
    u32 baseu;
    u32 res[4];
};

ਬਣਤਰ ahci_fis_s {
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

ਬਣਤਰ ahci_ਪੋਰਟ_s {
    ਬਣਤਰ drive_s     drive;
    ਬਣਤਰ ahci_ctrl_s *ctrl;
    ਬਣਤਰ ahci_list_s *list;
    ਬਣਤਰ ahci_fis_s  *fis;
    ਬਣਤਰ ahci_cmd_s  *cmd;
    u32                pnr;
    u32                atapi;
    ਅੱਖਰ               *desc;
    ਪੂਰਨ                prio;
};

ਖਾਲੀ ahci_ਤਿਆਰੀ(ਖਾਲੀ);
ਪੂਰਨ ahci_process_op(ਬਣਤਰ disk_op_s *op);
ਪੂਰਨ ahci_atapi_process_op(ਬਣਤਰ disk_op_s *op);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AHCI_IRQ_ON_SG            (1 << 31)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AHCI_CMD_ATAPI            (1 << 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AHCI_CMD_WRITE            (1 << 6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AHCI_CMD_PREFETCH         (1 << 7)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AHCI_CMD_RESET            (1 << 8)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AHCI_CMD_CLR_BUSY         (1 << 10)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RX_FIS_D2H_REG            0x40 /* offset of D2H Register FIS data */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RX_FIS_SDB                0x58 /* offset of SDB FIS data */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RX_FIS_UNK                0x60 /* offset of Unknown FIS data */

/* global controller registers */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HOST_CAP                  0x00 /* host capabilities */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HOST_CTL                  0x04 /* global host control */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HOST_IRQ_STAT             0x08 /* interrupt status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HOST_PORTS_IMPL           0x0c /* bitmap of implemented ports */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HOST_ਸੰਸਕਰਣ              0x10 /* AHCI spec. version compliancy */

/* HOST_CTL bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HOST_CTL_RESET            (1 << 0)  /* reset controller; self-clear */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HOST_CTL_IRQ_EN           (1 << 1)  /* global IRQ enable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HOST_CTL_AHCI_EN          (1 << 31) /* AHCI enabled */

/* HOST_CAP bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HOST_CAP_SSC              (1 << 14) /* Slumber capable */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HOST_CAP_AHCI             (1 << 18) /* AHCI only */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HOST_CAP_CLO              (1 << 24) /* Command List Override support */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HOST_CAP_SSS              (1 << 27) /* Staggered Spin-up */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HOST_CAP_NCQ              (1 << 30) /* Native Command Queueing */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HOST_CAP_64               (1 << 31) /* PCI DAC (64-bit DMA) support */

/* registers for each SATA port */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_LST_ADDR             0x00 /* command list DMA addr */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_LST_ADDR_HI          0x04 /* command list DMA addr hi */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_FIS_ADDR             0x08 /* FIS rx buf addr */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_FIS_ADDR_HI          0x0c /* FIS rx buf addr hi */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_STAT             0x10 /* interrupt status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_ਨਕਾਬ             0x14 /* interrupt enable/disable mask */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_CMD                  0x18 /* port command */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_TFDATA               0x20 /* taskfile data */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_SIG                  0x24 /* device TF signature */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_SCR_STAT             0x28 /* SATA phy register: SStatus */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_SCR_CTL              0x2c /* SATA phy register: SControl */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_SCR_ERR              0x30 /* SATA phy register: SError */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_SCR_ACT              0x34 /* SATA phy register: SActive */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_CMD_ISSUE            0x38 /* command issue */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_RESERVED             0x3c /* reserved */

/* PORT_IRQ_{STAT,MASK} bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_COLD_PRES        (1 << 31) /* cold presence detect */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_TF_ERR           (1 << 30) /* task file error */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_HBUS_ERR         (1 << 29) /* host bus fatal error */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_HBUS_ਅੰਕੜੇ_ERR    (1 << 28) /* host bus data error */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_IF_ERR           (1 << 27) /* interface fatal error */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_IF_NONFATAL      (1 << 26) /* interface non-fatal error */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_ਉਛਾਲ         (1 << 24) /* xfer exhausted available S/G */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_BAD_PMP          (1 << 23) /* incorrect port multiplier */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_PHYRDY           (1 << 22) /* PhyRdy changed */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_DEV_ILCK         (1 << 7) /* device interlock */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_CONNECT          (1 << 6) /* port connect change status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_SG_ਪੂਰਾ          (1 << 5) /* descriptor processed */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_UNK_FIS          (1 << 4) /* unknown FIS rx'd */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_SDB_FIS          (1 << 3) /* Set Device Bits FIS rx'd */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_DMAS_FIS         (1 << 2) /* DMA Setup FIS rx'd */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_PIOS_FIS         (1 << 1) /* PIO Setup FIS rx'd */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_D2_H_REG_FIS      (1 << 0) /* D2H Register FIS rx'd */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_FREEZE           (ਪੋਰਟ_IRQ_HBUS_ERR | ਪੋਰਟ_IRQ_IF_ERR |   \
                                   ਪੋਰਟ_IRQ_CONNECT | ਪੋਰਟ_IRQ_PHYRDY |    \
                                   ਪੋਰਟ_IRQ_UNK_FIS)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_ERROR            (ਪੋਰਟ_IRQ_FREEZE | ਪੋਰਟ_IRQ_TF_ERR |     \
                                   ਪੋਰਟ_IRQ_HBUS_ਅੰਕੜੇ_ERR)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEF_ਪੋਰਟ_IRQ              (ਪੋਰਟ_IRQ_ERROR | ਪੋਰਟ_IRQ_SG_ਪੂਰਾ |     \
                                   ਪੋਰਟ_IRQ_SDB_FIS | ਪੋਰਟ_IRQ_DMAS_FIS |  \
                                   ਪੋਰਟ_IRQ_PIOS_FIS | ਪੋਰਟ_IRQ_D2_H_REG_FIS)

/* PORT_CMD bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_CMD_ATAPI            (1 << 24) /* Device is ATAPI */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_CMD_LIST_ON          (1 << 15) /* cmd list DMA engine running */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_CMD_FIS_ON           (1 << 14) /* FIS DMA engine running */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_CMD_FIS_RX           (1 << 4) /* Enable FIS receive DMA engine */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_CMD_CLO              (1 << 3) /* Command list override */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_CMD_POWER_ON         (1 << 2) /* Power up device */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_CMD_SPIN_ਉੱਪਰ          (1 << 1) /* Spin up device */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_CMD_ਸ਼ੁਰੂ            (1 << 0) /* Enable port DMA engine */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_CMD_ICC_ਨਕਾਬ         (0xf << 28) /* i/f ICC state mask */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_CMD_ICC_ਸਰਗਰਮ       (0x1 << 28) /* Put i/f in active state */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_CMD_ICC_PARTIAL      (0x2 << 28) /* Put i/f in partial state */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_CMD_ICC_SLUMBER      (0x6 << 28) /* Put i/f in slumber state */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_STAT_DHRS        (1 << 0) /* Device to Host Register FIS */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_STAT_PSS         (1 << 1) /* PIO Setup FIS */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_STAT_DSS         (1 << 2) /* DMA Setup FIS */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_STAT_SDBS        (1 << 3) /* Set Device Bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_STAT_UFS         (1 << 4) /* Unknown FIS */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_STAT_DPS         (1 << 5) /* Descriptor Processed */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_STAT_PCS         (1 << 6) /* Port Connect Change Status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_STAT_DMPS        (1 << 7) /* Device Mechanical Presence
ਸਥਿਤੀ*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_STAT_PRCS        (1 << 22) /* File Ready Status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_STAT_IPMS        (1 << 23) /* Incorrect Port Multiplier
ਸਥਿਤੀ*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_STAT_OFS         (1 << 24) /* Overflow Status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_STAT_INFS        (1 << 26) /* Interface Non-Fatal Error
ਸਥਿਤੀ*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_STAT_IFS         (1 << 27) /* Interface Fatal Error */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_STAT_HBDS        (1 << 28) /* Host Bus Data Error Status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_STAT_HBFS        (1 << 29) /* Host Bus Fatal Error Status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_STAT_TFES        (1 << 30) /* Task File Error Status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_IRQ_STAT_CPDS        (1 << 31) /* Code Port Detect Status */

#ਜੇ_ਅੰਤ // ahci.h

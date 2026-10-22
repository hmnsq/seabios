#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __AHCI_H
#ವ್ಯಾಖ್ಯಾನಿಸು __AHCI_H

#ಅನ್ತರ್ಭಾವ "block.h" // struct drive_s
#ಅನ್ತರ್ಭಾವ "types.h" // u32

ರಚನೆ sata_cmd_fis {
    u8 reg;
    u8 pmp_type;
    u8 command;
    u8 feature;

    u8 lba_ಕೆಳಗಿನ;
    u8 lba_mid;
    u8 lba_ಮೇಲಿನ;
    u8 device;

    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
    u8 feature2;

    u8 sector_ಎಣಿಕೆ;
    u8 sector_count2;
    u8 res_1;
    u8 control;

    u8 res_2[64 - 16];
};

ರಚನೆ ahci_ctrl_s {
    ರಚನೆ pci_device *pci_tmp;
    u8  irq;
    ಶೂನ್ಯ *iobase;
    u32 caps;
    u32 ports;
};

ರಚನೆ ahci_cmd_s {
    ರಚನೆ sata_cmd_fis fis;
    u8 atapi[0x20];
    u8 res[0x20];
    ರಚನೆ {
        u32 ತಳ;
        u32 baseu;
        u32 res;
        u32 flags;
    } prdt[];
};

/* command list */
ರಚನೆ ahci_list_s {
    u32 flags;
    u32 ಬೈಟುಗಳು;
    u32 ತಳ;
    u32 baseu;
    u32 res[4];
};

ರಚನೆ ahci_fis_s {
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

ರಚನೆ ahci_ಪೋರ್ಟ್_s {
    ರಚನೆ drive_s     drive;
    ರಚನೆ ahci_ctrl_s *ctrl;
    ರಚನೆ ahci_list_s *list;
    ರಚನೆ ahci_fis_s  *fis;
    ರಚನೆ ahci_cmd_s  *cmd;
    u32                pnr;
    u32                atapi;
    ಅಕ್ಷರ               *desc;
    ಪೂರ್ಣಾಂಕ                prio;
};

ಶೂನ್ಯ ahci_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ ahci_process_op(ರಚನೆ disk_op_s *op);
ಪೂರ್ಣಾಂಕ ahci_atapi_process_op(ರಚನೆ disk_op_s *op);

#ವ್ಯಾಖ್ಯಾನಿಸು AHCI_IRQ_ON_SG            (1 << 31)
#ವ್ಯಾಖ್ಯಾನಿಸು AHCI_CMD_ATAPI            (1 << 5)
#ವ್ಯಾಖ್ಯಾನಿಸು AHCI_CMD_WRITE            (1 << 6)
#ವ್ಯಾಖ್ಯಾನಿಸು AHCI_CMD_PREFETCH         (1 << 7)
#ವ್ಯಾಖ್ಯಾನಿಸು AHCI_CMD_RESET            (1 << 8)
#ವ್ಯಾಖ್ಯಾನಿಸು AHCI_CMD_CLR_BUSY         (1 << 10)

#ವ್ಯಾಖ್ಯಾನಿಸು RX_FIS_D2H_REG            0x40 /* offset of D2H Register FIS data */
#ವ್ಯಾಖ್ಯಾನಿಸು RX_FIS_SDB                0x58 /* offset of SDB FIS data */
#ವ್ಯಾಖ್ಯಾನಿಸು RX_FIS_UNK                0x60 /* offset of Unknown FIS data */

/* global controller registers */
#ವ್ಯಾಖ್ಯಾನಿಸು HOST_CAP                  0x00 /* host capabilities */
#ವ್ಯಾಖ್ಯಾನಿಸು HOST_CTL                  0x04 /* global host control */
#ವ್ಯಾಖ್ಯಾನಿಸು HOST_IRQ_STAT             0x08 /* interrupt status */
#ವ್ಯಾಖ್ಯಾನಿಸು HOST_PORTS_IMPL           0x0c /* bitmap of implemented ports */
#ವ್ಯಾಖ್ಯಾನಿಸು HOST_ಆವೃತ್ತಿ              0x10 /* AHCI spec. version compliancy */

/* HOST_CTL bits */
#ವ್ಯಾಖ್ಯಾನಿಸು HOST_CTL_RESET            (1 << 0)  /* reset controller; self-clear */
#ವ್ಯಾಖ್ಯಾನಿಸು HOST_CTL_IRQ_EN           (1 << 1)  /* global IRQ enable */
#ವ್ಯಾಖ್ಯಾನಿಸು HOST_CTL_AHCI_EN          (1 << 31) /* AHCI enabled */

/* HOST_CAP bits */
#ವ್ಯಾಖ್ಯಾನಿಸು HOST_CAP_SSC              (1 << 14) /* Slumber capable */
#ವ್ಯಾಖ್ಯಾನಿಸು HOST_CAP_AHCI             (1 << 18) /* AHCI only */
#ವ್ಯಾಖ್ಯಾನಿಸು HOST_CAP_CLO              (1 << 24) /* Command List Override support */
#ವ್ಯಾಖ್ಯಾನಿಸು HOST_CAP_SSS              (1 << 27) /* Staggered Spin-up */
#ವ್ಯಾಖ್ಯಾನಿಸು HOST_CAP_NCQ              (1 << 30) /* Native Command Queueing */
#ವ್ಯಾಖ್ಯಾನಿಸು HOST_CAP_64               (1 << 31) /* PCI DAC (64-bit DMA) support */

/* registers for each SATA port */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_LST_ADDR             0x00 /* command list DMA addr */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_LST_ADDR_HI          0x04 /* command list DMA addr hi */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_FIS_ADDR             0x08 /* FIS rx buf addr */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_FIS_ADDR_HI          0x0c /* FIS rx buf addr hi */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_STAT             0x10 /* interrupt status */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_ಮುಖವಾಡ             0x14 /* interrupt enable/disable mask */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_CMD                  0x18 /* port command */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_TFDATA               0x20 /* taskfile data */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_SIG                  0x24 /* device TF signature */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_SCR_STAT             0x28 /* SATA phy register: SStatus */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_SCR_CTL              0x2c /* SATA phy register: SControl */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_SCR_ERR              0x30 /* SATA phy register: SError */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_SCR_ACT              0x34 /* SATA phy register: SActive */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_CMD_ISSUE            0x38 /* command issue */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_RESERVED             0x3c /* reserved */

/* PORT_IRQ_{STAT,MASK} bits */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_COLD_PRES        (1 << 31) /* cold presence detect */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_TF_ERR           (1 << 30) /* task file error */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_HBUS_ERR         (1 << 29) /* host bus fatal error */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_HBUS_ದತ್ತಾಂಶ_ERR    (1 << 28) /* host bus data error */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_IF_ERR           (1 << 27) /* interface fatal error */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_IF_NONFATAL      (1 << 26) /* interface non-fatal error */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_ಅತಿಪ್ರವಾಹ         (1 << 24) /* xfer exhausted available S/G */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_BAD_PMP          (1 << 23) /* incorrect port multiplier */

#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_PHYRDY           (1 << 22) /* PhyRdy changed */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_DEV_ILCK         (1 << 7) /* device interlock */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_CONNECT          (1 << 6) /* port connect change status */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_SG_ಮುಗಿದ          (1 << 5) /* descriptor processed */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_UNK_FIS          (1 << 4) /* unknown FIS rx'd */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_SDB_FIS          (1 << 3) /* Set Device Bits FIS rx'd */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_DMAS_FIS         (1 << 2) /* DMA Setup FIS rx'd */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_PIOS_FIS         (1 << 1) /* PIO Setup FIS rx'd */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_D2_H_REG_FIS      (1 << 0) /* D2H Register FIS rx'd */

#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_FREEZE           (ಪೋರ್ಟ್_IRQ_HBUS_ERR | ಪೋರ್ಟ್_IRQ_IF_ERR |   \
                                   ಪೋರ್ಟ್_IRQ_CONNECT | ಪೋರ್ಟ್_IRQ_PHYRDY |    \
                                   ಪೋರ್ಟ್_IRQ_UNK_FIS)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_ERROR            (ಪೋರ್ಟ್_IRQ_FREEZE | ಪೋರ್ಟ್_IRQ_TF_ERR |     \
                                   ಪೋರ್ಟ್_IRQ_HBUS_ದತ್ತಾಂಶ_ERR)
#ವ್ಯಾಖ್ಯಾನಿಸು DEF_ಪೋರ್ಟ್_IRQ              (ಪೋರ್ಟ್_IRQ_ERROR | ಪೋರ್ಟ್_IRQ_SG_ಮುಗಿದ |     \
                                   ಪೋರ್ಟ್_IRQ_SDB_FIS | ಪೋರ್ಟ್_IRQ_DMAS_FIS |  \
                                   ಪೋರ್ಟ್_IRQ_PIOS_FIS | ಪೋರ್ಟ್_IRQ_D2_H_REG_FIS)

/* PORT_CMD bits */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_CMD_ATAPI            (1 << 24) /* Device is ATAPI */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_CMD_LIST_ON          (1 << 15) /* cmd list DMA engine running */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_CMD_FIS_ON           (1 << 14) /* FIS DMA engine running */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_CMD_FIS_RX           (1 << 4) /* Enable FIS receive DMA engine */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_CMD_CLO              (1 << 3) /* Command list override */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_CMD_POWER_ON         (1 << 2) /* Power up device */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_CMD_SPIN_ಮೇಲೆ          (1 << 1) /* Spin up device */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_CMD_ಆರಂಭ            (1 << 0) /* Enable port DMA engine */

#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_CMD_ICC_ಮುಖವಾಡ         (0xf << 28) /* i/f ICC state mask */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_CMD_ICC_ಸಕ್ರಿಯ       (0x1 << 28) /* Put i/f in active state */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_CMD_ICC_PARTIAL      (0x2 << 28) /* Put i/f in partial state */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_CMD_ICC_SLUMBER      (0x6 << 28) /* Put i/f in slumber state */

#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_STAT_DHRS        (1 << 0) /* Device to Host Register FIS */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_STAT_PSS         (1 << 1) /* PIO Setup FIS */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_STAT_DSS         (1 << 2) /* DMA Setup FIS */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_STAT_SDBS        (1 << 3) /* Set Device Bits */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_STAT_UFS         (1 << 4) /* Unknown FIS */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_STAT_DPS         (1 << 5) /* Descriptor Processed */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_STAT_PCS         (1 << 6) /* Port Connect Change Status */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_STAT_DMPS        (1 << 7) /* Device Mechanical Presence
ಸ್ಥಿತಿ*/
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_STAT_PRCS        (1 << 22) /* File Ready Status */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_STAT_IPMS        (1 << 23) /* Incorrect Port Multiplier
ಸ್ಥಿತಿ*/
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_STAT_OFS         (1 << 24) /* Overflow Status */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_STAT_INFS        (1 << 26) /* Interface Non-Fatal Error
ಸ್ಥಿತಿ*/
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_STAT_IFS         (1 << 27) /* Interface Fatal Error */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_STAT_HBDS        (1 << 28) /* Host Bus Data Error Status */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_STAT_HBFS        (1 << 29) /* Host Bus Fatal Error Status */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_STAT_TFES        (1 << 30) /* Task File Error Status */
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_IRQ_STAT_CPDS        (1 << 31) /* Code Port Detect Status */

#ಆದರೆ_ಅಂತ್ಯ // ahci.h

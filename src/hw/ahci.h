#اگر_تعریف_نشده __AHCI_H
#تعریف_کن __AHCI_H

#ضمن "block.h" // struct drive_s
#ضمن "types.h" // u32

بنية sata_cmd_fis {
    u8 reg;
    u8 pmp_type;
    u8 command;
    u8 feature;

    u8 lba_کم;
    u8 lba_mid;
    u8 lba_بالا;
    u8 device;

    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
    u8 feature2;

    u8 sector_شمار;
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
        u32 پایه;
        u32 baseu;
        u32 res;
        u32 flags;
    } prdt[];
};

/* command list */
بنية ahci_list_s {
    u32 flags;
    u32 بایتها;
    u32 پایه;
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

بنية ahci_پورت_s {
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

فراغ ahci_راهاندازی(فراغ);
صحيح ahci_process_op(بنية disk_op_s *op);
صحيح ahci_atapi_process_op(بنية disk_op_s *op);

#تعریف_کن AHCI_IRQ_ON_SG            (1 << 31)
#تعریف_کن AHCI_CMD_ATAPI            (1 << 5)
#تعریف_کن AHCI_CMD_WRITE            (1 << 6)
#تعریف_کن AHCI_CMD_PREFETCH         (1 << 7)
#تعریف_کن AHCI_CMD_RESET            (1 << 8)
#تعریف_کن AHCI_CMD_CLR_BUSY         (1 << 10)

#تعریف_کن RX_FIS_D2H_REG            0x40 /* offset of D2H Register FIS data */
#تعریف_کن RX_FIS_SDB                0x58 /* offset of SDB FIS data */
#تعریف_کن RX_FIS_UNK                0x60 /* offset of Unknown FIS data */

/* global controller registers */
#تعریف_کن HOST_CAP                  0x00 /* host capabilities */
#تعریف_کن HOST_CTL                  0x04 /* global host control */
#تعریف_کن HOST_IRQ_STAT             0x08 /* interrupt status */
#تعریف_کن HOST_PORTS_IMPL           0x0c /* bitmap of implemented ports */
#تعریف_کن HOST_نسخه              0x10 /* AHCI spec. version compliancy */

/* HOST_CTL bits */
#تعریف_کن HOST_CTL_RESET            (1 << 0)  /* reset controller; self-clear */
#تعریف_کن HOST_CTL_IRQ_EN           (1 << 1)  /* global IRQ enable */
#تعریف_کن HOST_CTL_AHCI_EN          (1 << 31) /* AHCI enabled */

/* HOST_CAP bits */
#تعریف_کن HOST_CAP_SSC              (1 << 14) /* Slumber capable */
#تعریف_کن HOST_CAP_AHCI             (1 << 18) /* AHCI only */
#تعریف_کن HOST_CAP_CLO              (1 << 24) /* Command List Override support */
#تعریف_کن HOST_CAP_SSS              (1 << 27) /* Staggered Spin-up */
#تعریف_کن HOST_CAP_NCQ              (1 << 30) /* Native Command Queueing */
#تعریف_کن HOST_CAP_64               (1 << 31) /* PCI DAC (64-bit DMA) support */

/* registers for each SATA port */
#تعریف_کن پورت_LST_ADDR             0x00 /* command list DMA addr */
#تعریف_کن پورت_LST_ADDR_HI          0x04 /* command list DMA addr hi */
#تعریف_کن پورت_FIS_ADDR             0x08 /* FIS rx buf addr */
#تعریف_کن پورت_FIS_ADDR_HI          0x0c /* FIS rx buf addr hi */
#تعریف_کن پورت_IRQ_STAT             0x10 /* interrupt status */
#تعریف_کن پورت_IRQ_ماسک             0x14 /* interrupt enable/disable mask */
#تعریف_کن پورت_CMD                  0x18 /* port command */
#تعریف_کن پورت_TFDATA               0x20 /* taskfile data */
#تعریف_کن پورت_SIG                  0x24 /* device TF signature */
#تعریف_کن پورت_SCR_STAT             0x28 /* SATA phy register: SStatus */
#تعریف_کن پورت_SCR_CTL              0x2c /* SATA phy register: SControl */
#تعریف_کن پورت_SCR_ERR              0x30 /* SATA phy register: SError */
#تعریف_کن پورت_SCR_ACT              0x34 /* SATA phy register: SActive */
#تعریف_کن پورت_CMD_ISSUE            0x38 /* command issue */
#تعریف_کن پورت_RESERVED             0x3c /* reserved */

/* PORT_IRQ_{STAT,MASK} bits */
#تعریف_کن پورت_IRQ_COLD_PRES        (1 << 31) /* cold presence detect */
#تعریف_کن پورت_IRQ_TF_ERR           (1 << 30) /* task file error */
#تعریف_کن پورت_IRQ_HBUS_ERR         (1 << 29) /* host bus fatal error */
#تعریف_کن پورت_IRQ_HBUS_داده_ERR    (1 << 28) /* host bus data error */
#تعریف_کن پورت_IRQ_IF_ERR           (1 << 27) /* interface fatal error */
#تعریف_کن پورت_IRQ_IF_NONFATAL      (1 << 26) /* interface non-fatal error */
#تعریف_کن پورت_IRQ_سرریز         (1 << 24) /* xfer exhausted available S/G */
#تعریف_کن پورت_IRQ_BAD_PMP          (1 << 23) /* incorrect port multiplier */

#تعریف_کن پورت_IRQ_PHYRDY           (1 << 22) /* PhyRdy changed */
#تعریف_کن پورت_IRQ_DEV_ILCK         (1 << 7) /* device interlock */
#تعریف_کن پورت_IRQ_CONNECT          (1 << 6) /* port connect change status */
#تعریف_کن پورت_IRQ_SG_انجامشده          (1 << 5) /* descriptor processed */
#تعریف_کن پورت_IRQ_UNK_FIS          (1 << 4) /* unknown FIS rx'd */
#تعریف_کن پورت_IRQ_SDB_FIS          (1 << 3) /* Set Device Bits FIS rx'd */
#تعریف_کن پورت_IRQ_DMAS_FIS         (1 << 2) /* DMA Setup FIS rx'd */
#تعریف_کن پورت_IRQ_PIOS_FIS         (1 << 1) /* PIO Setup FIS rx'd */
#تعریف_کن پورت_IRQ_D2_H_REG_FIS      (1 << 0) /* D2H Register FIS rx'd */

#تعریف_کن پورت_IRQ_FREEZE           (پورت_IRQ_HBUS_ERR | پورت_IRQ_IF_ERR |   \
                                   پورت_IRQ_CONNECT | پورت_IRQ_PHYRDY |    \
                                   پورت_IRQ_UNK_FIS)
#تعریف_کن پورت_IRQ_ERROR            (پورت_IRQ_FREEZE | پورت_IRQ_TF_ERR |     \
                                   پورت_IRQ_HBUS_داده_ERR)
#تعریف_کن DEF_پورت_IRQ              (پورت_IRQ_ERROR | پورت_IRQ_SG_انجامشده |     \
                                   پورت_IRQ_SDB_FIS | پورت_IRQ_DMAS_FIS |  \
                                   پورت_IRQ_PIOS_FIS | پورت_IRQ_D2_H_REG_FIS)

/* PORT_CMD bits */
#تعریف_کن پورت_CMD_ATAPI            (1 << 24) /* Device is ATAPI */
#تعریف_کن پورت_CMD_LIST_ON          (1 << 15) /* cmd list DMA engine running */
#تعریف_کن پورت_CMD_FIS_ON           (1 << 14) /* FIS DMA engine running */
#تعریف_کن پورت_CMD_FIS_RX           (1 << 4) /* Enable FIS receive DMA engine */
#تعریف_کن پورت_CMD_CLO              (1 << 3) /* Command list override */
#تعریف_کن پورت_CMD_POWER_ON         (1 << 2) /* Power up device */
#تعریف_کن پورت_CMD_SPIN_به_بالا          (1 << 1) /* Spin up device */
#تعریف_کن پورت_CMD_شروع            (1 << 0) /* Enable port DMA engine */

#تعریف_کن پورت_CMD_ICC_ماسک         (0xf << 28) /* i/f ICC state mask */
#تعریف_کن پورت_CMD_ICC_فعال       (0x1 << 28) /* Put i/f in active state */
#تعریف_کن پورت_CMD_ICC_PARTIAL      (0x2 << 28) /* Put i/f in partial state */
#تعریف_کن پورت_CMD_ICC_SLUMBER      (0x6 << 28) /* Put i/f in slumber state */

#تعریف_کن پورت_IRQ_STAT_DHRS        (1 << 0) /* Device to Host Register FIS */
#تعریف_کن پورت_IRQ_STAT_PSS         (1 << 1) /* PIO Setup FIS */
#تعریف_کن پورت_IRQ_STAT_DSS         (1 << 2) /* DMA Setup FIS */
#تعریف_کن پورت_IRQ_STAT_SDBS        (1 << 3) /* Set Device Bits */
#تعریف_کن پورت_IRQ_STAT_UFS         (1 << 4) /* Unknown FIS */
#تعریف_کن پورت_IRQ_STAT_DPS         (1 << 5) /* Descriptor Processed */
#تعریف_کن پورت_IRQ_STAT_PCS         (1 << 6) /* Port Connect Change Status */
#تعریف_کن پورت_IRQ_STAT_DMPS        (1 << 7) /* Device Mechanical Presence
وضعیت*/
#تعریف_کن پورت_IRQ_STAT_PRCS        (1 << 22) /* File Ready Status */
#تعریف_کن پورت_IRQ_STAT_IPMS        (1 << 23) /* Incorrect Port Multiplier
وضعیت*/
#تعریف_کن پورت_IRQ_STAT_OFS         (1 << 24) /* Overflow Status */
#تعریف_کن پورت_IRQ_STAT_INFS        (1 << 26) /* Interface Non-Fatal Error
وضعیت*/
#تعریف_کن پورت_IRQ_STAT_IFS         (1 << 27) /* Interface Fatal Error */
#تعریف_کن پورت_IRQ_STAT_HBDS        (1 << 28) /* Host Bus Data Error Status */
#تعریف_کن پورت_IRQ_STAT_HBFS        (1 << 29) /* Host Bus Fatal Error Status */
#تعریف_کن پورت_IRQ_STAT_TFES        (1 << 30) /* Task File Error Status */
#تعریف_کن پورت_IRQ_STAT_CPDS        (1 << 31) /* Code Port Detect Status */

#پایان_اگر // ahci.h

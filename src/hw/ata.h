#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __ATA_H
#ವ್ಯಾಖ್ಯಾನಿಸು __ATA_H

#ಅನ್ತರ್ಭಾವ "block.h" // struct drive_s
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_MAX_ATA_INTERFACES
#ಅನ್ತರ್ಭಾವ "types.h" // u8

ರಚನೆ ata_channel_s {
    u16 iobase1;
    u16 iobase2;
    u16 iomaster;
    u8  irq;
    u8  chanid;
    u8  ataid;
    ಪೂರ್ಣಾಂಕ pci_bdf;
    ರಚನೆ pci_device *pci_tmp;
};

ರಚನೆ atadrive_s {
    ರಚನೆ drive_s drive;
    ರಚನೆ ata_channel_s *chan_gf;
    u8 slave;
};

// ata.c
ಅಕ್ಷರ *ata_ಹೊರತೆಗೆ_model(ಅಕ್ಷರ *model, u32 ಗಾತ್ರ_2, u16 *ಬಫರ್);
ಪೂರ್ಣಾಂಕ ata_ಹೊರತೆಗೆ_ಆವೃತ್ತಿ(u16 *ಬಫರ್);
ಪೂರ್ಣಾಂಕ ata_process_op(ರಚನೆ disk_op_s *op);
ಪೂರ್ಣಾಂಕ ata_atapi_process_op(ರಚನೆ disk_op_s *op);
ಶೂನ್ಯ ata_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);

#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_ATA2_CMD_ತಳ     0x0170
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_ATA1_CMD_ತಳ     0x01f0
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_ATA2_CTRL_ತಳ    0x0374
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_ATA1_CTRL_ತಳ    0x03f4

// Global defines -- ATA register and register bits.
// command block & control block regs
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_ದತ್ತಾಂಶ  0   // data reg         in/out pio_base_addr1+0
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_ERR   1   // error            in     pio_base_addr1+1
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_FR    1   // feature reg         out pio_base_addr1+1
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_SC    2   // sector count     in/out pio_base_addr1+2
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_SN    3   // sector number    in/out pio_base_addr1+3
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_CL    4   // cylinder low     in/out pio_base_addr1+4
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_CH    5   // cylinder high    in/out pio_base_addr1+5
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_DH    6   // device head      in/out pio_base_addr1+6
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_STAT  7   // primary status   in     pio_base_addr1+7
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_CMD   7   // command             out pio_base_addr1+7

#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_ASTAT 2   // alternate status in     pio_base_addr2+2
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_DC    2   // device control      out pio_base_addr2+2
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_DA    3   // device address   in     pio_base_addr2+3

#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_ER_ICRC 0x80    // ATA Ultra DMA bad CRC
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_ER_BBK  0x80    // ATA bad block
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_ER_UNC  0x40    // ATA uncorrected error
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_ER_MC   0x20    // ATA media change
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_ER_IDNF 0x10    // ATA id not found
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_ER_MCR  0x08    // ATA media change request
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_ER_ABRT 0x04    // ATA command aborted
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_ER_NTK0 0x02    // ATA track 0 not found
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_ER_NDAM 0x01    // ATA address mark not found

#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_ER_P_SNSKEY 0xf0   // ATAPI sense key (mask)
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_ER_P_MCR    0x08   // ATAPI Media Change Request
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_ER_P_ABRT   0x04   // ATAPI command abort
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_ER_P_EOM    0x02   // ATAPI End of Media
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_ER_P_ILI    0x01   // ATAPI Illegal Length Indication

// ATAPI Interrupt Reason bits in the Sector Count reg (CB_SC)
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_SC_P_ಟ್ಯಾಗ್    0xf8   // ATAPI tag (mask)
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_SC_P_REL    0x04   // ATAPI release
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_SC_P_IO     0x02   // ATAPI I/O
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_SC_P_CD     0x01   // ATAPI C/D

// bits 7-4 of the device/head (CB_DH) reg
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_DH_DEV0 0xa0    // select device 0
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_DH_DEV1 0xb0    // select device 1
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_DH_LBA 0x40    // use LBA

// status reg (CB_STAT and CB_ASTAT) bits
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_STAT_BSY  0x80  // busy
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_STAT_RDY  0x40  // ready
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_STAT_DF   0x20  // device fault
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_STAT_WFT  0x20  // write fault (old name)
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_STAT_SKC  0x10  // seek complete
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_STAT_SERV 0x10  // service
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_STAT_DRQ  0x08  // data request
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_STAT_CORR 0x04  // corrected
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_STAT_IDX  0x02  //ಸೂಚ್ಯಂಕ
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_STAT_ERR  0x01  // error (ATA)
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_STAT_CHK  0x01  // check (ATAPI)

// device control reg (CB_DC) bits
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_DC_HD15   0x08  // bit should always be set to one
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_DC_SRST   0x04  // soft reset
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CB_DC_NIEN   0x02  // disable interrupts

// Most mandtory and optional ATA commands (from ATA-3),
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_NOP                          0x00
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_CFA_REQUEST_EXT_ERR_ಸಂಕೇತ     0x03
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_DEVICE_RESET                 0x08
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_RECALIBRATE                  0x10
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_READ_SECTORS                 0x20
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_READ_SECTORS_EXT             0x24
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_READ_DMA_EXT                 0x25
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_READ_DMA_QUEUED_EXT          0x26
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_READ_NATIVE_MAX_ವಿಳಾಸ_EXT  0x27
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_READ_ಅಪವರ್ತ್ಯ_EXT            0x29
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_READ_LOG_EXT                 0x2F
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_WRITE_SECTORS                0x30
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_WRITE_SECTORS_EXT            0x34
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_WRITE_DMA_EXT                0x35
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_WRITE_DMA_QUEUED_EXT         0x36
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_ಗಣ_MAX_ವಿಳಾಸ_EXT          0x37
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_CFA_WRITE_SECTORS_WO_ERASE   0x38
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_WRITE_ಅಪವರ್ತ್ಯ_EXT           0x39
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_WRITE_VERIFY                 0x3C
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_WRITE_LOG_EXT                0x3F
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_READ_VERIFY_SECTORS          0x40
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_READ_VERIFY_SECTORS_EXT      0x42
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_FORMAT_TRACK                 0x50
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_SEEK                         0x70
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_CFA_TRANSLATE_SECTOR         0x87
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_EXECUTE_DEVICE_DIAGNOSTIC    0x90
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_INITIALIZE_DEVICE_PARAMETERS 0x91
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_STANDBY_IMMEDIATE2           0x94
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_IDLE_IMMEDIATE2              0x95
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_STANDBY2                     0x96
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_IDLE2                        0x97
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_CHECK_POWER_MODE2            0x98
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_SLEEP2                       0x99
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_PACKET                       0xA0
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_IDENTIFY_PACKET_DEVICE       0xA1
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_CFA_ERASE_SECTORS            0xC0
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_READ_ಅಪವರ್ತ್ಯ                0xC4
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_WRITE_ಅಪವರ್ತ್ಯ               0xC5
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_ಗಣ_ಅಪವರ್ತ್ಯ_ಬಹುಲಕ            0xC6
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_READ_DMA_QUEUED              0xC7
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_READ_DMA                     0xC8
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_WRITE_DMA                    0xCA
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_WRITE_DMA_QUEUED             0xCC
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_CFA_WRITE_ಅಪವರ್ತ್ಯ_WO_ERASE  0xCD
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_STANDBY_ತಕ್ಷಣ            0xE0
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_IDLE_ತಕ್ಷಣ               0xE1
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_STANDBY                      0xE2
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_IDLE                         0xE3
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_READ_ಬಫರ್                  0xE4
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_CHECK_POWER_ಬಹುಲಕ             0xE5
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_SLEEP                        0xE6
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_FLUSH_CACHE                  0xE7
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_WRITE_ಬಫರ್                 0xE8
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_IDENTIFY_DEVICE              0xEC
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_ಗಣ_FEATURES                 0xEF
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_READ_NATIVE_MAX_ವಿಳಾಸ      0xF8
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_CMD_ಗಣ_MAX                      0xF9

#ವ್ಯಾಖ್ಯಾನಿಸು ATA_ಗಣ_FEATRUE_TRANSFER_ಬಹುಲಕ        0x03
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_TRANSFER_ಬಹುಲಕ_ULTRA_DMA          0x40
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_TRANSFER_ಬಹುಲಕ_MULTIWORD_DMA      0x20
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_TRANSFER_ಬಹುಲಕ_PIO_FLOW_CTRL      0x08
#ವ್ಯಾಖ್ಯಾನಿಸು ATA_TRANSFER_ಬಹುಲಕ_DEFAULT_PIO        0x00
#ಆದರೆ_ಅಂತ್ಯ // ata.h

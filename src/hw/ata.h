#جيڪڏهن_وصف_نه_ٿيل __ATA_H
#وصف_ڪريو __ATA_H

#ضمن "block.h" // struct drive_s
#ضمن "config.h" // CONFIG_MAX_ATA_INTERFACES
#ضمن "types.h" // u8

بنية ata_channel_s {
    u16 iobase1;
    u16 iobase2;
    u16 iomaster;
    u8  irq;
    u8  chanid;
    u8  ataid;
    صحيح pci_bdf;
    بنية pci_device *pci_tmp;
};

بنية atadrive_s {
    بنية drive_s drive;
    بنية ata_channel_s *chan_gf;
    u8 slave;
};

// ata.c
حرف *ata_ڪڍو_model(حرف *model, u32 سائيز, u16 *بفر);
صحيح ata_ڪڍو_نسخو(u16 *بفر);
صحيح ata_process_op(بنية disk_op_s *op);
صحيح ata_atapi_process_op(بنية disk_op_s *op);
فراغ ata_تياري(فراغ);

#وصف_ڪريو پورٽ_ATA2_CMD_بنياد     0x0170
#وصف_ڪريو پورٽ_ATA1_CMD_بنياد     0x01f0
#وصف_ڪريو پورٽ_ATA2_CTRL_بنياد    0x0374
#وصف_ڪريو پورٽ_ATA1_CTRL_بنياد    0x03f4

// Global defines -- ATA register and register bits.
// command block & control block regs
#وصف_ڪريو ATA_CB_معلومات  0   // data reg         in/out pio_base_addr1+0
#وصف_ڪريو ATA_CB_ERR   1   // error            in     pio_base_addr1+1
#وصف_ڪريو ATA_CB_FR    1   // feature reg         out pio_base_addr1+1
#وصف_ڪريو ATA_CB_SC    2   // sector count     in/out pio_base_addr1+2
#وصف_ڪريو ATA_CB_SN    3   // sector number    in/out pio_base_addr1+3
#وصف_ڪريو ATA_CB_CL    4   // cylinder low     in/out pio_base_addr1+4
#وصف_ڪريو ATA_CB_CH    5   // cylinder high    in/out pio_base_addr1+5
#وصف_ڪريو ATA_CB_DH    6   // device head      in/out pio_base_addr1+6
#وصف_ڪريو ATA_CB_STAT  7   // primary status   in     pio_base_addr1+7
#وصف_ڪريو ATA_CB_CMD   7   // command             out pio_base_addr1+7

#وصف_ڪريو ATA_CB_ASTAT 2   // alternate status in     pio_base_addr2+2
#وصف_ڪريو ATA_CB_DC    2   // device control      out pio_base_addr2+2
#وصف_ڪريو ATA_CB_DA    3   // device address   in     pio_base_addr2+3

#وصف_ڪريو ATA_CB_ER_ICRC 0x80    // ATA Ultra DMA bad CRC
#وصف_ڪريو ATA_CB_ER_BBK  0x80    // ATA bad block
#وصف_ڪريو ATA_CB_ER_UNC  0x40    // ATA uncorrected error
#وصف_ڪريو ATA_CB_ER_MC   0x20    // ATA media change
#وصف_ڪريو ATA_CB_ER_IDNF 0x10    // ATA id not found
#وصف_ڪريو ATA_CB_ER_MCR  0x08    // ATA media change request
#وصف_ڪريو ATA_CB_ER_ABRT 0x04    // ATA command aborted
#وصف_ڪريو ATA_CB_ER_NTK0 0x02    // ATA track 0 not found
#وصف_ڪريو ATA_CB_ER_NDAM 0x01    // ATA address mark not found

#وصف_ڪريو ATA_CB_ER_P_SNSKEY 0xf0   // ATAPI sense key (mask)
#وصف_ڪريو ATA_CB_ER_P_MCR    0x08   // ATAPI Media Change Request
#وصف_ڪريو ATA_CB_ER_P_ABRT   0x04   // ATAPI command abort
#وصف_ڪريو ATA_CB_ER_P_EOM    0x02   // ATAPI End of Media
#وصف_ڪريو ATA_CB_ER_P_ILI    0x01   // ATAPI Illegal Length Indication

// ATAPI Interrupt Reason bits in the Sector Count reg (CB_SC)
#وصف_ڪريو ATA_CB_SC_P_ٽيگ    0xf8   // ATAPI tag (mask)
#وصف_ڪريو ATA_CB_SC_P_REL    0x04   // ATAPI release
#وصف_ڪريو ATA_CB_SC_P_IO     0x02   // ATAPI I/O
#وصف_ڪريو ATA_CB_SC_P_CD     0x01   // ATAPI C/D

// bits 7-4 of the device/head (CB_DH) reg
#وصف_ڪريو ATA_CB_DH_DEV0 0xa0    // select device 0
#وصف_ڪريو ATA_CB_DH_DEV1 0xb0    // select device 1
#وصف_ڪريو ATA_CB_DH_LBA 0x40    // use LBA

// status reg (CB_STAT and CB_ASTAT) bits
#وصف_ڪريو ATA_CB_STAT_BSY  0x80  // busy
#وصف_ڪريو ATA_CB_STAT_RDY  0x40  // ready
#وصف_ڪريو ATA_CB_STAT_DF   0x20  // device fault
#وصف_ڪريو ATA_CB_STAT_WFT  0x20  // write fault (old name)
#وصف_ڪريو ATA_CB_STAT_SKC  0x10  // seek complete
#وصف_ڪريو ATA_CB_STAT_SERV 0x10  // service
#وصف_ڪريو ATA_CB_STAT_DRQ  0x08  // data request
#وصف_ڪريو ATA_CB_STAT_CORR 0x04  // corrected
#وصف_ڪريو ATA_CB_STAT_IDX  0x02  //اشاريو
#وصف_ڪريو ATA_CB_STAT_ERR  0x01  // error (ATA)
#وصف_ڪريو ATA_CB_STAT_CHK  0x01  // check (ATAPI)

// device control reg (CB_DC) bits
#وصف_ڪريو ATA_CB_DC_HD15   0x08  // bit should always be set to one
#وصف_ڪريو ATA_CB_DC_SRST   0x04  // soft reset
#وصف_ڪريو ATA_CB_DC_NIEN   0x02  // disable interrupts

// Most mandtory and optional ATA commands (from ATA-3),
#وصف_ڪريو ATA_CMD_NOP                          0x00
#وصف_ڪريو ATA_CMD_CFA_REQUEST_EXT_ERR_ڪوڊ     0x03
#وصف_ڪريو ATA_CMD_DEVICE_RESET                 0x08
#وصف_ڪريو ATA_CMD_RECALIBRATE                  0x10
#وصف_ڪريو ATA_CMD_READ_SECTORS                 0x20
#وصف_ڪريو ATA_CMD_READ_SECTORS_EXT             0x24
#وصف_ڪريو ATA_CMD_READ_DMA_EXT                 0x25
#وصف_ڪريو ATA_CMD_READ_DMA_QUEUED_EXT          0x26
#وصف_ڪريو ATA_CMD_READ_NATIVE_MAX_پتو_EXT  0x27
#وصف_ڪريو ATA_CMD_READ_گڻو_EXT            0x29
#وصف_ڪريو ATA_CMD_READ_LOG_EXT                 0x2F
#وصف_ڪريو ATA_CMD_WRITE_SECTORS                0x30
#وصف_ڪريو ATA_CMD_WRITE_SECTORS_EXT            0x34
#وصف_ڪريو ATA_CMD_WRITE_DMA_EXT                0x35
#وصف_ڪريو ATA_CMD_WRITE_DMA_QUEUED_EXT         0x36
#وصف_ڪريو ATA_CMD_سيٽ_MAX_پتو_EXT          0x37
#وصف_ڪريو ATA_CMD_CFA_WRITE_SECTORS_WO_ERASE   0x38
#وصف_ڪريو ATA_CMD_WRITE_گڻو_EXT           0x39
#وصف_ڪريو ATA_CMD_WRITE_VERIFY                 0x3C
#وصف_ڪريو ATA_CMD_WRITE_LOG_EXT                0x3F
#وصف_ڪريو ATA_CMD_READ_VERIFY_SECTORS          0x40
#وصف_ڪريو ATA_CMD_READ_VERIFY_SECTORS_EXT      0x42
#وصف_ڪريو ATA_CMD_FORMAT_TRACK                 0x50
#وصف_ڪريو ATA_CMD_SEEK                         0x70
#وصف_ڪريو ATA_CMD_CFA_TRANSLATE_SECTOR         0x87
#وصف_ڪريو ATA_CMD_EXECUTE_DEVICE_DIAGNOSTIC    0x90
#وصف_ڪريو ATA_CMD_INITIALIZE_DEVICE_PARAMETERS 0x91
#وصف_ڪريو ATA_CMD_STANDBY_IMMEDIATE2           0x94
#وصف_ڪريو ATA_CMD_IDLE_IMMEDIATE2              0x95
#وصف_ڪريو ATA_CMD_STANDBY2                     0x96
#وصف_ڪريو ATA_CMD_IDLE2                        0x97
#وصف_ڪريو ATA_CMD_CHECK_POWER_MODE2            0x98
#وصف_ڪريو ATA_CMD_SLEEP2                       0x99
#وصف_ڪريو ATA_CMD_PACKET                       0xA0
#وصف_ڪريو ATA_CMD_IDENTIFY_PACKET_DEVICE       0xA1
#وصف_ڪريو ATA_CMD_CFA_ERASE_SECTORS            0xC0
#وصف_ڪريو ATA_CMD_READ_گڻو                0xC4
#وصف_ڪريو ATA_CMD_WRITE_گڻو               0xC5
#وصف_ڪريو ATA_CMD_سيٽ_گڻو_گهڻائي            0xC6
#وصف_ڪريو ATA_CMD_READ_DMA_QUEUED              0xC7
#وصف_ڪريو ATA_CMD_READ_DMA                     0xC8
#وصف_ڪريو ATA_CMD_WRITE_DMA                    0xCA
#وصف_ڪريو ATA_CMD_WRITE_DMA_QUEUED             0xCC
#وصف_ڪريو ATA_CMD_CFA_WRITE_گڻو_WO_ERASE  0xCD
#وصف_ڪريو ATA_CMD_STANDBY_فوري            0xE0
#وصف_ڪريو ATA_CMD_IDLE_فوري               0xE1
#وصف_ڪريو ATA_CMD_STANDBY                      0xE2
#وصف_ڪريو ATA_CMD_IDLE                         0xE3
#وصف_ڪريو ATA_CMD_READ_بفر                  0xE4
#وصف_ڪريو ATA_CMD_CHECK_POWER_گهڻائي             0xE5
#وصف_ڪريو ATA_CMD_SLEEP                        0xE6
#وصف_ڪريو ATA_CMD_FLUSH_CACHE                  0xE7
#وصف_ڪريو ATA_CMD_WRITE_بفر                 0xE8
#وصف_ڪريو ATA_CMD_IDENTIFY_DEVICE              0xEC
#وصف_ڪريو ATA_CMD_سيٽ_FEATURES                 0xEF
#وصف_ڪريو ATA_CMD_READ_NATIVE_MAX_پتو      0xF8
#وصف_ڪريو ATA_CMD_سيٽ_MAX                      0xF9

#وصف_ڪريو ATA_سيٽ_FEATRUE_TRANSFER_گهڻائي        0x03
#وصف_ڪريو ATA_TRANSFER_گهڻائي_ULTRA_DMA          0x40
#وصف_ڪريو ATA_TRANSFER_گهڻائي_MULTIWORD_DMA      0x20
#وصف_ڪريو ATA_TRANSFER_گهڻائي_PIO_FLOW_CTRL      0x08
#وصف_ڪريو ATA_TRANSFER_گهڻائي_DEFAULT_PIO        0x00
#جيڪڏهن_پورو // ata.h

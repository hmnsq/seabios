#اگر_تعریف_نشده __ATA_H
#تعریف_کن __ATA_H

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
حرف *ata_استخراج_model(حرف *model, u32 اندازه, u16 *میانگیر);
صحيح ata_استخراج_نسخه(u16 *میانگیر);
صحيح ata_process_op(بنية disk_op_s *op);
صحيح ata_atapi_process_op(بنية disk_op_s *op);
فراغ ata_راهاندازی(فراغ);

#تعریف_کن پورت_ATA2_CMD_پایه     0x0170
#تعریف_کن پورت_ATA1_CMD_پایه     0x01f0
#تعریف_کن پورت_ATA2_CTRL_پایه    0x0374
#تعریف_کن پورت_ATA1_CTRL_پایه    0x03f4

// Global defines -- ATA register and register bits.
// command block & control block regs
#تعریف_کن ATA_CB_داده  0   // data reg         in/out pio_base_addr1+0
#تعریف_کن ATA_CB_ERR   1   // error            in     pio_base_addr1+1
#تعریف_کن ATA_CB_FR    1   // feature reg         out pio_base_addr1+1
#تعریف_کن ATA_CB_SC    2   // sector count     in/out pio_base_addr1+2
#تعریف_کن ATA_CB_SN    3   // sector number    in/out pio_base_addr1+3
#تعریف_کن ATA_CB_CL    4   // cylinder low     in/out pio_base_addr1+4
#تعریف_کن ATA_CB_CH    5   // cylinder high    in/out pio_base_addr1+5
#تعریف_کن ATA_CB_DH    6   // device head      in/out pio_base_addr1+6
#تعریف_کن ATA_CB_STAT  7   // primary status   in     pio_base_addr1+7
#تعریف_کن ATA_CB_CMD   7   // command             out pio_base_addr1+7

#تعریف_کن ATA_CB_ASTAT 2   // alternate status in     pio_base_addr2+2
#تعریف_کن ATA_CB_DC    2   // device control      out pio_base_addr2+2
#تعریف_کن ATA_CB_DA    3   // device address   in     pio_base_addr2+3

#تعریف_کن ATA_CB_ER_ICRC 0x80    // ATA Ultra DMA bad CRC
#تعریف_کن ATA_CB_ER_BBK  0x80    // ATA bad block
#تعریف_کن ATA_CB_ER_UNC  0x40    // ATA uncorrected error
#تعریف_کن ATA_CB_ER_MC   0x20    // ATA media change
#تعریف_کن ATA_CB_ER_IDNF 0x10    // ATA id not found
#تعریف_کن ATA_CB_ER_MCR  0x08    // ATA media change request
#تعریف_کن ATA_CB_ER_ABRT 0x04    // ATA command aborted
#تعریف_کن ATA_CB_ER_NTK0 0x02    // ATA track 0 not found
#تعریف_کن ATA_CB_ER_NDAM 0x01    // ATA address mark not found

#تعریف_کن ATA_CB_ER_P_SNSKEY 0xf0   // ATAPI sense key (mask)
#تعریف_کن ATA_CB_ER_P_MCR    0x08   // ATAPI Media Change Request
#تعریف_کن ATA_CB_ER_P_ABRT   0x04   // ATAPI command abort
#تعریف_کن ATA_CB_ER_P_EOM    0x02   // ATAPI End of Media
#تعریف_کن ATA_CB_ER_P_ILI    0x01   // ATAPI Illegal Length Indication

// ATAPI Interrupt Reason bits in the Sector Count reg (CB_SC)
#تعریف_کن ATA_CB_SC_P_برچسب    0xf8   // ATAPI tag (mask)
#تعریف_کن ATA_CB_SC_P_REL    0x04   // ATAPI release
#تعریف_کن ATA_CB_SC_P_IO     0x02   // ATAPI I/O
#تعریف_کن ATA_CB_SC_P_CD     0x01   // ATAPI C/D

// bits 7-4 of the device/head (CB_DH) reg
#تعریف_کن ATA_CB_DH_DEV0 0xa0    // select device 0
#تعریف_کن ATA_CB_DH_DEV1 0xb0    // select device 1
#تعریف_کن ATA_CB_DH_LBA 0x40    // use LBA

// status reg (CB_STAT and CB_ASTAT) bits
#تعریف_کن ATA_CB_STAT_BSY  0x80  // busy
#تعریف_کن ATA_CB_STAT_RDY  0x40  // ready
#تعریف_کن ATA_CB_STAT_DF   0x20  // device fault
#تعریف_کن ATA_CB_STAT_WFT  0x20  // write fault (old name)
#تعریف_کن ATA_CB_STAT_SKC  0x10  // seek complete
#تعریف_کن ATA_CB_STAT_SERV 0x10  // service
#تعریف_کن ATA_CB_STAT_DRQ  0x08  // data request
#تعریف_کن ATA_CB_STAT_CORR 0x04  // corrected
#تعریف_کن ATA_CB_STAT_IDX  0x02  //شاخص
#تعریف_کن ATA_CB_STAT_ERR  0x01  // error (ATA)
#تعریف_کن ATA_CB_STAT_CHK  0x01  // check (ATAPI)

// device control reg (CB_DC) bits
#تعریف_کن ATA_CB_DC_HD15   0x08  // bit should always be set to one
#تعریف_کن ATA_CB_DC_SRST   0x04  // soft reset
#تعریف_کن ATA_CB_DC_NIEN   0x02  // disable interrupts

// Most mandtory and optional ATA commands (from ATA-3),
#تعریف_کن ATA_CMD_NOP                          0x00
#تعریف_کن ATA_CMD_CFA_REQUEST_EXT_ERR_کد     0x03
#تعریف_کن ATA_CMD_DEVICE_RESET                 0x08
#تعریف_کن ATA_CMD_RECALIBRATE                  0x10
#تعریف_کن ATA_CMD_READ_SECTORS                 0x20
#تعریف_کن ATA_CMD_READ_SECTORS_EXT             0x24
#تعریف_کن ATA_CMD_READ_DMA_EXT                 0x25
#تعریف_کن ATA_CMD_READ_DMA_QUEUED_EXT          0x26
#تعریف_کن ATA_CMD_READ_NATIVE_MAX_آدرس_EXT  0x27
#تعریف_کن ATA_CMD_READ_مضرب_EXT            0x29
#تعریف_کن ATA_CMD_READ_LOG_EXT                 0x2F
#تعریف_کن ATA_CMD_WRITE_SECTORS                0x30
#تعریف_کن ATA_CMD_WRITE_SECTORS_EXT            0x34
#تعریف_کن ATA_CMD_WRITE_DMA_EXT                0x35
#تعریف_کن ATA_CMD_WRITE_DMA_QUEUED_EXT         0x36
#تعریف_کن ATA_CMD_مجموعه_MAX_آدرس_EXT          0x37
#تعریف_کن ATA_CMD_CFA_WRITE_SECTORS_WO_ERASE   0x38
#تعریف_کن ATA_CMD_WRITE_مضرب_EXT           0x39
#تعریف_کن ATA_CMD_WRITE_VERIFY                 0x3C
#تعریف_کن ATA_CMD_WRITE_LOG_EXT                0x3F
#تعریف_کن ATA_CMD_READ_VERIFY_SECTORS          0x40
#تعریف_کن ATA_CMD_READ_VERIFY_SECTORS_EXT      0x42
#تعریف_کن ATA_CMD_FORMAT_TRACK                 0x50
#تعریف_کن ATA_CMD_SEEK                         0x70
#تعریف_کن ATA_CMD_CFA_TRANSLATE_SECTOR         0x87
#تعریف_کن ATA_CMD_EXECUTE_DEVICE_DIAGNOSTIC    0x90
#تعریف_کن ATA_CMD_INITIALIZE_DEVICE_PARAMETERS 0x91
#تعریف_کن ATA_CMD_STANDBY_IMMEDIATE2           0x94
#تعریف_کن ATA_CMD_IDLE_IMMEDIATE2              0x95
#تعریف_کن ATA_CMD_STANDBY2                     0x96
#تعریف_کن ATA_CMD_IDLE2                        0x97
#تعریف_کن ATA_CMD_CHECK_POWER_MODE2            0x98
#تعریف_کن ATA_CMD_SLEEP2                       0x99
#تعریف_کن ATA_CMD_PACKET                       0xA0
#تعریف_کن ATA_CMD_IDENTIFY_PACKET_DEVICE       0xA1
#تعریف_کن ATA_CMD_CFA_ERASE_SECTORS            0xC0
#تعریف_کن ATA_CMD_READ_مضرب                0xC4
#تعریف_کن ATA_CMD_WRITE_مضرب               0xC5
#تعریف_کن ATA_CMD_مجموعه_مضرب_نما            0xC6
#تعریف_کن ATA_CMD_READ_DMA_QUEUED              0xC7
#تعریف_کن ATA_CMD_READ_DMA                     0xC8
#تعریف_کن ATA_CMD_WRITE_DMA                    0xCA
#تعریف_کن ATA_CMD_WRITE_DMA_QUEUED             0xCC
#تعریف_کن ATA_CMD_CFA_WRITE_مضرب_WO_ERASE  0xCD
#تعریف_کن ATA_CMD_STANDBY_فوری            0xE0
#تعریف_کن ATA_CMD_IDLE_فوری               0xE1
#تعریف_کن ATA_CMD_STANDBY                      0xE2
#تعریف_کن ATA_CMD_IDLE                         0xE3
#تعریف_کن ATA_CMD_READ_میانگیر                  0xE4
#تعریف_کن ATA_CMD_CHECK_POWER_نما             0xE5
#تعریف_کن ATA_CMD_SLEEP                        0xE6
#تعریف_کن ATA_CMD_FLUSH_CACHE                  0xE7
#تعریف_کن ATA_CMD_WRITE_میانگیر                 0xE8
#تعریف_کن ATA_CMD_IDENTIFY_DEVICE              0xEC
#تعریف_کن ATA_CMD_مجموعه_FEATURES                 0xEF
#تعریف_کن ATA_CMD_READ_NATIVE_MAX_آدرس      0xF8
#تعریف_کن ATA_CMD_مجموعه_MAX                      0xF9

#تعریف_کن ATA_مجموعه_FEATRUE_TRANSFER_نما        0x03
#تعریف_کن ATA_TRANSFER_نما_ULTRA_DMA          0x40
#تعریف_کن ATA_TRANSFER_نما_MULTIWORD_DMA      0x20
#تعریف_کن ATA_TRANSFER_نما_PIO_FLOW_CTRL      0x08
#تعریف_کن ATA_TRANSFER_نما_DEFAULT_PIO        0x00
#پایان_اگر // ata.h

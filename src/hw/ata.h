#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __ATA_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __ATA_H

#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // struct drive_s
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_MAX_ATA_INTERFACES
#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u8

ਬਣਤਰ ata_channel_s {
    u16 iobase1;
    u16 iobase2;
    u16 iomaster;
    u8  irq;
    u8  chanid;
    u8  ataid;
    ਪੂਰਨ pci_bdf;
    ਬਣਤਰ pci_device *pci_tmp;
};

ਬਣਤਰ atadrive_s {
    ਬਣਤਰ drive_s drive;
    ਬਣਤਰ ata_channel_s *chan_gf;
    u8 slave;
};

// ata.c
ਅੱਖਰ *ata_ਕੱਢੋ_model(ਅੱਖਰ *model, u32 ਮਾਪ, u16 *ਬਫਰ);
ਪੂਰਨ ata_ਕੱਢੋ_ਸੰਸਕਰਣ(u16 *ਬਫਰ);
ਪੂਰਨ ata_process_op(ਬਣਤਰ disk_op_s *op);
ਪੂਰਨ ata_atapi_process_op(ਬਣਤਰ disk_op_s *op);
ਖਾਲੀ ata_ਤਿਆਰੀ(ਖਾਲੀ);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_ATA2_CMD_ਅਧਾਰ     0x0170
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_ATA1_CMD_ਅਧਾਰ     0x01f0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_ATA2_CTRL_ਅਧਾਰ    0x0374
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_ATA1_CTRL_ਅਧਾਰ    0x03f4

// Global defines -- ATA register and register bits.
// command block & control block regs
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_ਅੰਕੜੇ  0   // data reg         in/out pio_base_addr1+0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_ERR   1   // error            in     pio_base_addr1+1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_FR    1   // feature reg         out pio_base_addr1+1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_SC    2   // sector count     in/out pio_base_addr1+2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_SN    3   // sector number    in/out pio_base_addr1+3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_CL    4   // cylinder low     in/out pio_base_addr1+4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_CH    5   // cylinder high    in/out pio_base_addr1+5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_DH    6   // device head      in/out pio_base_addr1+6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_STAT  7   // primary status   in     pio_base_addr1+7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_CMD   7   // command             out pio_base_addr1+7

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_ASTAT 2   // alternate status in     pio_base_addr2+2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_DC    2   // device control      out pio_base_addr2+2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_DA    3   // device address   in     pio_base_addr2+3

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_ER_ICRC 0x80    // ATA Ultra DMA bad CRC
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_ER_BBK  0x80    // ATA bad block
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_ER_UNC  0x40    // ATA uncorrected error
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_ER_MC   0x20    // ATA media change
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_ER_IDNF 0x10    // ATA id not found
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_ER_MCR  0x08    // ATA media change request
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_ER_ABRT 0x04    // ATA command aborted
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_ER_NTK0 0x02    // ATA track 0 not found
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_ER_NDAM 0x01    // ATA address mark not found

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_ER_P_SNSKEY 0xf0   // ATAPI sense key (mask)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_ER_P_MCR    0x08   // ATAPI Media Change Request
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_ER_P_ABRT   0x04   // ATAPI command abort
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_ER_P_EOM    0x02   // ATAPI End of Media
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_ER_P_ILI    0x01   // ATAPI Illegal Length Indication

// ATAPI Interrupt Reason bits in the Sector Count reg (CB_SC)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_SC_P_ਟੈਗ    0xf8   // ATAPI tag (mask)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_SC_P_REL    0x04   // ATAPI release
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_SC_P_IO     0x02   // ATAPI I/O
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_SC_P_CD     0x01   // ATAPI C/D

// bits 7-4 of the device/head (CB_DH) reg
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_DH_DEV0 0xa0    // select device 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_DH_DEV1 0xb0    // select device 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_DH_LBA 0x40    // use LBA

// status reg (CB_STAT and CB_ASTAT) bits
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_STAT_BSY  0x80  // busy
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_STAT_RDY  0x40  // ready
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_STAT_DF   0x20  // device fault
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_STAT_WFT  0x20  // write fault (old name)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_STAT_SKC  0x10  // seek complete
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_STAT_SERV 0x10  // service
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_STAT_DRQ  0x08  // data request
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_STAT_CORR 0x04  // corrected
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_STAT_IDX  0x02  //ਸੂਚਕਾਂਕ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_STAT_ERR  0x01  // error (ATA)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_STAT_CHK  0x01  // check (ATAPI)

// device control reg (CB_DC) bits
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_DC_HD15   0x08  // bit should always be set to one
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_DC_SRST   0x04  // soft reset
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CB_DC_NIEN   0x02  // disable interrupts

// Most mandtory and optional ATA commands (from ATA-3),
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_NOP                          0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_CFA_REQUEST_EXT_ERR_ਕੋਡ     0x03
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_DEVICE_RESET                 0x08
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_RECALIBRATE                  0x10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_READ_SECTORS                 0x20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_READ_SECTORS_EXT             0x24
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_READ_DMA_EXT                 0x25
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_READ_DMA_QUEUED_EXT          0x26
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_READ_NATIVE_MAX_ਪਤਾ_EXT  0x27
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_READ_ਗੁਣਜ_EXT            0x29
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_READ_LOG_EXT                 0x2F
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_WRITE_SECTORS                0x30
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_WRITE_SECTORS_EXT            0x34
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_WRITE_DMA_EXT                0x35
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_WRITE_DMA_QUEUED_EXT         0x36
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_ਸੈੱਟ_MAX_ਪਤਾ_EXT          0x37
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_CFA_WRITE_SECTORS_WO_ERASE   0x38
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_WRITE_ਗੁਣਜ_EXT           0x39
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_WRITE_VERIFY                 0x3C
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_WRITE_LOG_EXT                0x3F
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_READ_VERIFY_SECTORS          0x40
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_READ_VERIFY_SECTORS_EXT      0x42
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_FORMAT_TRACK                 0x50
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_SEEK                         0x70
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_CFA_TRANSLATE_SECTOR         0x87
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_EXECUTE_DEVICE_DIAGNOSTIC    0x90
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_INITIALIZE_DEVICE_PARAMETERS 0x91
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_STANDBY_IMMEDIATE2           0x94
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_IDLE_IMMEDIATE2              0x95
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_STANDBY2                     0x96
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_IDLE2                        0x97
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_CHECK_POWER_MODE2            0x98
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_SLEEP2                       0x99
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_PACKET                       0xA0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_IDENTIFY_PACKET_DEVICE       0xA1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_CFA_ERASE_SECTORS            0xC0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_READ_ਗੁਣਜ                0xC4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_WRITE_ਗੁਣਜ               0xC5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_ਸੈੱਟ_ਗੁਣਜ_ਬਹੁਲਕ            0xC6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_READ_DMA_QUEUED              0xC7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_READ_DMA                     0xC8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_WRITE_DMA                    0xCA
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_WRITE_DMA_QUEUED             0xCC
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_CFA_WRITE_ਗੁਣਜ_WO_ERASE  0xCD
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_STANDBY_ਤੁਰੰਤ            0xE0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_IDLE_ਤੁਰੰਤ               0xE1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_STANDBY                      0xE2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_IDLE                         0xE3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_READ_ਬਫਰ                  0xE4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_CHECK_POWER_ਬਹੁਲਕ             0xE5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_SLEEP                        0xE6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_FLUSH_CACHE                  0xE7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_WRITE_ਬਫਰ                 0xE8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_IDENTIFY_DEVICE              0xEC
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_ਸੈੱਟ_FEATURES                 0xEF
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_READ_NATIVE_MAX_ਪਤਾ      0xF8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_CMD_ਸੈੱਟ_MAX                      0xF9

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_ਸੈੱਟ_FEATRUE_TRANSFER_ਬਹੁਲਕ        0x03
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_TRANSFER_ਬਹੁਲਕ_ULTRA_DMA          0x40
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_TRANSFER_ਬਹੁਲਕ_MULTIWORD_DMA      0x20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_TRANSFER_ਬਹੁਲਕ_PIO_FLOW_CTRL      0x08
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATA_TRANSFER_ਬਹੁਲਕ_DEFAULT_PIO        0x00
#ਜੇ_ਅੰਤ // ata.h

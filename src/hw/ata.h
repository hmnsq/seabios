#यदि_अपरिभाषितम् __ATA_H
#परिभाषय __ATA_H

#अन्तर्भाव "block.h" // struct drive_s
#अन्तर्भाव "config.h" // CONFIG_MAX_ATA_INTERFACES
#अन्तर्भाव "types.h" // u8

संरचना ata_channel_s {
    u16 iobase1;
    u16 iobase2;
    u16 iomaster;
    u8  irq;
    u8  chanid;
    u8  ataid;
    पूर्णाङ्क pci_bdf;
    संरचना pci_device *pci_tmp;
};

संरचना atadrive_s {
    संरचना drive_s drive;
    संरचना ata_channel_s *chan_gf;
    u8 slave;
};

// ata.c
वर्ण *ata_extract_model(वर्ण *model, u32 आकार, u16 *बफर);
पूर्णाङ्क ata_extract_संस्करणम्(u16 *बफर);
पूर्णाङ्क ata_process_op(संरचना disk_op_s *op);
पूर्णाङ्क ata_atapi_process_op(संरचना disk_op_s *op);
शून्य ata_सज्जता(शून्य);

#परिभाषय पत्तनम्_ATA2_CMD_आधारः     0x0170
#परिभाषय पत्तनम्_ATA1_CMD_आधारः     0x01f0
#परिभाषय पत्तनम्_ATA2_CTRL_आधारः    0x0374
#परिभाषय पत्तनम्_ATA1_CTRL_आधारः    0x03f4

// Global defines -- ATA register and register bits.
// command block & control block regs
#परिभाषय ATA_CB_दत्त  0   // data reg         in/out pio_base_addr1+0
#परिभाषय ATA_CB_ERR   1   // error            in     pio_base_addr1+1
#परिभाषय ATA_CB_FR    1   // feature reg         out pio_base_addr1+1
#परिभाषय ATA_CB_SC    2   // sector count     in/out pio_base_addr1+2
#परिभाषय ATA_CB_SN    3   // sector number    in/out pio_base_addr1+3
#परिभाषय ATA_CB_CL    4   // cylinder low     in/out pio_base_addr1+4
#परिभाषय ATA_CB_CH    5   // cylinder high    in/out pio_base_addr1+5
#परिभाषय ATA_CB_DH    6   // device head      in/out pio_base_addr1+6
#परिभाषय ATA_CB_STAT  7   // primary status   in     pio_base_addr1+7
#परिभाषय ATA_CB_CMD   7   // command             out pio_base_addr1+7

#परिभाषय ATA_CB_ASTAT 2   // alternate status in     pio_base_addr2+2
#परिभाषय ATA_CB_DC    2   // device control      out pio_base_addr2+2
#परिभाषय ATA_CB_DA    3   // device address   in     pio_base_addr2+3

#परिभाषय ATA_CB_ER_ICRC 0x80    // ATA Ultra DMA bad CRC
#परिभाषय ATA_CB_ER_BBK  0x80    // ATA bad block
#परिभाषय ATA_CB_ER_UNC  0x40    // ATA uncorrected error
#परिभाषय ATA_CB_ER_MC   0x20    // ATA media change
#परिभाषय ATA_CB_ER_IDNF 0x10    // ATA id not found
#परिभाषय ATA_CB_ER_MCR  0x08    // ATA media change request
#परिभाषय ATA_CB_ER_ABRT 0x04    // ATA command aborted
#परिभाषय ATA_CB_ER_NTK0 0x02    // ATA track 0 not found
#परिभाषय ATA_CB_ER_NDAM 0x01    // ATA address mark not found

#परिभाषय ATA_CB_ER_P_SNSKEY 0xf0   // ATAPI sense key (mask)
#परिभाषय ATA_CB_ER_P_MCR    0x08   // ATAPI Media Change Request
#परिभाषय ATA_CB_ER_P_ABRT   0x04   // ATAPI command abort
#परिभाषय ATA_CB_ER_P_EOM    0x02   // ATAPI End of Media
#परिभाषय ATA_CB_ER_P_ILI    0x01   // ATAPI Illegal Length Indication

// ATAPI Interrupt Reason bits in the Sector Count reg (CB_SC)
#परिभाषय ATA_CB_SC_P_चिह्नकम्    0xf8   // ATAPI tag (mask)
#परिभाषय ATA_CB_SC_P_REL    0x04   // ATAPI release
#परिभाषय ATA_CB_SC_P_IO     0x02   // ATAPI I/O
#परिभाषय ATA_CB_SC_P_CD     0x01   // ATAPI C/D

// bits 7-4 of the device/head (CB_DH) reg
#परिभाषय ATA_CB_DH_DEV0 0xa0    // select device 0
#परिभाषय ATA_CB_DH_DEV1 0xb0    // select device 1
#परिभाषय ATA_CB_DH_LBA 0x40    // use LBA

// status reg (CB_STAT and CB_ASTAT) bits
#परिभाषय ATA_CB_STAT_BSY  0x80  // busy
#परिभाषय ATA_CB_STAT_RDY  0x40  // ready
#परिभाषय ATA_CB_STAT_DF   0x20  // device fault
#परिभाषय ATA_CB_STAT_WFT  0x20  // write fault (old name)
#परिभाषय ATA_CB_STAT_SKC  0x10  // seek complete
#परिभाषय ATA_CB_STAT_SERV 0x10  // service
#परिभाषय ATA_CB_STAT_DRQ  0x08  // data request
#परिभाषय ATA_CB_STAT_CORR 0x04  // corrected
#परिभाषय ATA_CB_STAT_IDX  0x02  //सूचकांक
#परिभाषय ATA_CB_STAT_ERR  0x01  // error (ATA)
#परिभाषय ATA_CB_STAT_CHK  0x01  // check (ATAPI)

// device control reg (CB_DC) bits
#परिभाषय ATA_CB_DC_HD15   0x08  // bit should always be set to one
#परिभाषय ATA_CB_DC_SRST   0x04  // soft reset
#परिभाषय ATA_CB_DC_NIEN   0x02  // disable interrupts

// Most mandtory and optional ATA commands (from ATA-3),
#परिभाषय ATA_CMD_NOP                          0x00
#परिभाषय ATA_CMD_CFA_REQUEST_EXT_ERR_संहिता     0x03
#परिभाषय ATA_CMD_DEVICE_RESET                 0x08
#परिभाषय ATA_CMD_RECALIBRATE                  0x10
#परिभाषय ATA_CMD_READ_SECTORS                 0x20
#परिभाषय ATA_CMD_READ_SECTORS_EXT             0x24
#परिभाषय ATA_CMD_READ_DMA_EXT                 0x25
#परिभाषय ATA_CMD_READ_DMA_QUEUED_EXT          0x26
#परिभाषय ATA_CMD_READ_NATIVE_MAX_स्थानाङ्कः_EXT  0x27
#परिभाषय ATA_CMD_READ_गुणजः_EXT            0x29
#परिभाषय ATA_CMD_READ_LOG_EXT                 0x2F
#परिभाषय ATA_CMD_WRITE_SECTORS                0x30
#परिभाषय ATA_CMD_WRITE_SECTORS_EXT            0x34
#परिभाषय ATA_CMD_WRITE_DMA_EXT                0x35
#परिभाषय ATA_CMD_WRITE_DMA_QUEUED_EXT         0x36
#परिभाषय ATA_CMD_समुच्चयः_MAX_स्थानाङ्कः_EXT          0x37
#परिभाषय ATA_CMD_CFA_WRITE_SECTORS_WO_ERASE   0x38
#परिभाषय ATA_CMD_WRITE_गुणजः_EXT           0x39
#परिभाषय ATA_CMD_WRITE_VERIFY                 0x3C
#परिभाषय ATA_CMD_WRITE_LOG_EXT                0x3F
#परिभाषय ATA_CMD_READ_VERIFY_SECTORS          0x40
#परिभाषय ATA_CMD_READ_VERIFY_SECTORS_EXT      0x42
#परिभाषय ATA_CMD_FORMAT_TRACK                 0x50
#परिभाषय ATA_CMD_SEEK                         0x70
#परिभाषय ATA_CMD_CFA_TRANSLATE_SECTOR         0x87
#परिभाषय ATA_CMD_EXECUTE_DEVICE_DIAGNOSTIC    0x90
#परिभाषय ATA_CMD_INITIALIZE_DEVICE_PARAMETERS 0x91
#परिभाषय ATA_CMD_STANDBY_IMMEDIATE2           0x94
#परिभाषय ATA_CMD_IDLE_IMMEDIATE2              0x95
#परिभाषय ATA_CMD_STANDBY2                     0x96
#परिभाषय ATA_CMD_IDLE2                        0x97
#परिभाषय ATA_CMD_CHECK_POWER_MODE2            0x98
#परिभाषय ATA_CMD_SLEEP2                       0x99
#परिभाषय ATA_CMD_PACKET                       0xA0
#परिभाषय ATA_CMD_IDENTIFY_PACKET_DEVICE       0xA1
#परिभाषय ATA_CMD_CFA_ERASE_SECTORS            0xC0
#परिभाषय ATA_CMD_READ_गुणजः                0xC4
#परिभाषय ATA_CMD_WRITE_गुणजः               0xC5
#परिभाषय ATA_CMD_समुच्चयः_गुणजः_बहुलकम्            0xC6
#परिभाषय ATA_CMD_READ_DMA_QUEUED              0xC7
#परिभाषय ATA_CMD_READ_DMA                     0xC8
#परिभाषय ATA_CMD_WRITE_DMA                    0xCA
#परिभाषय ATA_CMD_WRITE_DMA_QUEUED             0xCC
#परिभाषय ATA_CMD_CFA_WRITE_गुणजः_WO_ERASE  0xCD
#परिभाषय ATA_CMD_STANDBY_तात्कालिकम्            0xE0
#परिभाषय ATA_CMD_IDLE_तात्कालिकम्               0xE1
#परिभाषय ATA_CMD_STANDBY                      0xE2
#परिभाषय ATA_CMD_IDLE                         0xE3
#परिभाषय ATA_CMD_READ_बफर                  0xE4
#परिभाषय ATA_CMD_CHECK_POWER_बहुलकम्             0xE5
#परिभाषय ATA_CMD_SLEEP                        0xE6
#परिभाषय ATA_CMD_FLUSH_CACHE                  0xE7
#परिभाषय ATA_CMD_WRITE_बफर                 0xE8
#परिभाषय ATA_CMD_IDENTIFY_DEVICE              0xEC
#परिभाषय ATA_CMD_समुच्चयः_FEATURES                 0xEF
#परिभाषय ATA_CMD_READ_NATIVE_MAX_स्थानाङ्कः      0xF8
#परिभाषय ATA_CMD_समुच्चयः_MAX                      0xF9

#परिभाषय ATA_समुच्चयः_FEATRUE_TRANSFER_बहुलकम्        0x03
#परिभाषय ATA_TRANSFER_बहुलकम्_ULTRA_DMA          0x40
#परिभाषय ATA_TRANSFER_बहुलकम्_MULTIWORD_DMA      0x20
#परिभाषय ATA_TRANSFER_बहुलकम्_PIO_FLOW_CTRL      0x08
#परिभाषय ATA_TRANSFER_बहुलकम्_DEFAULT_PIO        0x00
#यदि_अन्तः // ata.h

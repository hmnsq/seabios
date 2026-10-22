#నిర్వచితం_కాకపోతే __ATA_H
#నిర్వచించు __ATA_H

#అన్తర్భావ "block.h" // struct drive_s
#అన్తర్భావ "config.h" // CONFIG_MAX_ATA_INTERFACES
#అన్తర్భావ "types.h" // u8

నిర్మాణం ata_channel_s {
    u16 iobase1;
    u16 iobase2;
    u16 iomaster;
    u8  irq;
    u8  chanid;
    u8  ataid;
    పూర్ణాంకం pci_bdf;
    నిర్మాణం pci_device *pci_tmp;
};

నిర్మాణం atadrive_s {
    నిర్మాణం drive_s drive;
    నిర్మాణం ata_channel_s *chan_gf;
    u8 slave;
};

// ata.c
అక్షరం *ata_వెలికితీయి_model(అక్షరం *model, u32 కొలత, u16 *బఫరు);
పూర్ణాంకం ata_వెలికితీయి_సంస్కరణ(u16 *బఫరు);
పూర్ణాంకం ata_process_op(నిర్మాణం disk_op_s *op);
పూర్ణాంకం ata_atapi_process_op(నిర్మాణం disk_op_s *op);
శూన్య ata_అమరిక(శూన్య);

#నిర్వచించు పోర్టు_ATA2_CMD_పునాది     0x0170
#నిర్వచించు పోర్టు_ATA1_CMD_పునాది     0x01f0
#నిర్వచించు పోర్టు_ATA2_CTRL_పునాది    0x0374
#నిర్వచించు పోర్టు_ATA1_CTRL_పునాది    0x03f4

// Global defines -- ATA register and register bits.
// command block & control block regs
#నిర్వచించు ATA_CB_దత్తాంశం  0   // data reg         in/out pio_base_addr1+0
#నిర్వచించు ATA_CB_ERR   1   // error            in     pio_base_addr1+1
#నిర్వచించు ATA_CB_FR    1   // feature reg         out pio_base_addr1+1
#నిర్వచించు ATA_CB_SC    2   // sector count     in/out pio_base_addr1+2
#నిర్వచించు ATA_CB_SN    3   // sector number    in/out pio_base_addr1+3
#నిర్వచించు ATA_CB_CL    4   // cylinder low     in/out pio_base_addr1+4
#నిర్వచించు ATA_CB_CH    5   // cylinder high    in/out pio_base_addr1+5
#నిర్వచించు ATA_CB_DH    6   // device head      in/out pio_base_addr1+6
#నిర్వచించు ATA_CB_STAT  7   // primary status   in     pio_base_addr1+7
#నిర్వచించు ATA_CB_CMD   7   // command             out pio_base_addr1+7

#నిర్వచించు ATA_CB_ASTAT 2   // alternate status in     pio_base_addr2+2
#నిర్వచించు ATA_CB_DC    2   // device control      out pio_base_addr2+2
#నిర్వచించు ATA_CB_DA    3   // device address   in     pio_base_addr2+3

#నిర్వచించు ATA_CB_ER_ICRC 0x80    // ATA Ultra DMA bad CRC
#నిర్వచించు ATA_CB_ER_BBK  0x80    // ATA bad block
#నిర్వచించు ATA_CB_ER_UNC  0x40    // ATA uncorrected error
#నిర్వచించు ATA_CB_ER_MC   0x20    // ATA media change
#నిర్వచించు ATA_CB_ER_IDNF 0x10    // ATA id not found
#నిర్వచించు ATA_CB_ER_MCR  0x08    // ATA media change request
#నిర్వచించు ATA_CB_ER_ABRT 0x04    // ATA command aborted
#నిర్వచించు ATA_CB_ER_NTK0 0x02    // ATA track 0 not found
#నిర్వచించు ATA_CB_ER_NDAM 0x01    // ATA address mark not found

#నిర్వచించు ATA_CB_ER_P_SNSKEY 0xf0   // ATAPI sense key (mask)
#నిర్వచించు ATA_CB_ER_P_MCR    0x08   // ATAPI Media Change Request
#నిర్వచించు ATA_CB_ER_P_ABRT   0x04   // ATAPI command abort
#నిర్వచించు ATA_CB_ER_P_EOM    0x02   // ATAPI End of Media
#నిర్వచించు ATA_CB_ER_P_ILI    0x01   // ATAPI Illegal Length Indication

// ATAPI Interrupt Reason bits in the Sector Count reg (CB_SC)
#నిర్వచించు ATA_CB_SC_P_ట్యాగ్    0xf8   // ATAPI tag (mask)
#నిర్వచించు ATA_CB_SC_P_REL    0x04   // ATAPI release
#నిర్వచించు ATA_CB_SC_P_IO     0x02   // ATAPI I/O
#నిర్వచించు ATA_CB_SC_P_CD     0x01   // ATAPI C/D

// bits 7-4 of the device/head (CB_DH) reg
#నిర్వచించు ATA_CB_DH_DEV0 0xa0    // select device 0
#నిర్వచించు ATA_CB_DH_DEV1 0xb0    // select device 1
#నిర్వచించు ATA_CB_DH_LBA 0x40    // use LBA

// status reg (CB_STAT and CB_ASTAT) bits
#నిర్వచించు ATA_CB_STAT_BSY  0x80  // busy
#నిర్వచించు ATA_CB_STAT_RDY  0x40  // ready
#నిర్వచించు ATA_CB_STAT_DF   0x20  // device fault
#నిర్వచించు ATA_CB_STAT_WFT  0x20  // write fault (old name)
#నిర్వచించు ATA_CB_STAT_SKC  0x10  // seek complete
#నిర్వచించు ATA_CB_STAT_SERV 0x10  // service
#నిర్వచించు ATA_CB_STAT_DRQ  0x08  // data request
#నిర్వచించు ATA_CB_STAT_CORR 0x04  // corrected
#నిర్వచించు ATA_CB_STAT_IDX  0x02  //సూచీ
#నిర్వచించు ATA_CB_STAT_ERR  0x01  // error (ATA)
#నిర్వచించు ATA_CB_STAT_CHK  0x01  // check (ATAPI)

// device control reg (CB_DC) bits
#నిర్వచించు ATA_CB_DC_HD15   0x08  // bit should always be set to one
#నిర్వచించు ATA_CB_DC_SRST   0x04  // soft reset
#నిర్వచించు ATA_CB_DC_NIEN   0x02  // disable interrupts

// Most mandtory and optional ATA commands (from ATA-3),
#నిర్వచించు ATA_CMD_NOP                          0x00
#నిర్వచించు ATA_CMD_CFA_REQUEST_EXT_ERR_సంకేతం     0x03
#నిర్వచించు ATA_CMD_DEVICE_RESET                 0x08
#నిర్వచించు ATA_CMD_RECALIBRATE                  0x10
#నిర్వచించు ATA_CMD_READ_SECTORS                 0x20
#నిర్వచించు ATA_CMD_READ_SECTORS_EXT             0x24
#నిర్వచించు ATA_CMD_READ_DMA_EXT                 0x25
#నిర్వచించు ATA_CMD_READ_DMA_QUEUED_EXT          0x26
#నిర్వచించు ATA_CMD_READ_NATIVE_MAX_చిరునామా_EXT  0x27
#నిర్వచించు ATA_CMD_READ_గుణిజం_EXT            0x29
#నిర్వచించు ATA_CMD_READ_LOG_EXT                 0x2F
#నిర్వచించు ATA_CMD_WRITE_SECTORS                0x30
#నిర్వచించు ATA_CMD_WRITE_SECTORS_EXT            0x34
#నిర్వచించు ATA_CMD_WRITE_DMA_EXT                0x35
#నిర్వచించు ATA_CMD_WRITE_DMA_QUEUED_EXT         0x36
#నిర్వచించు ATA_CMD_సమితి_MAX_చిరునామా_EXT          0x37
#నిర్వచించు ATA_CMD_CFA_WRITE_SECTORS_WO_ERASE   0x38
#నిర్వచించు ATA_CMD_WRITE_గుణిజం_EXT           0x39
#నిర్వచించు ATA_CMD_WRITE_VERIFY                 0x3C
#నిర్వచించు ATA_CMD_WRITE_LOG_EXT                0x3F
#నిర్వచించు ATA_CMD_READ_VERIFY_SECTORS          0x40
#నిర్వచించు ATA_CMD_READ_VERIFY_SECTORS_EXT      0x42
#నిర్వచించు ATA_CMD_FORMAT_TRACK                 0x50
#నిర్వచించు ATA_CMD_SEEK                         0x70
#నిర్వచించు ATA_CMD_CFA_TRANSLATE_SECTOR         0x87
#నిర్వచించు ATA_CMD_EXECUTE_DEVICE_DIAGNOSTIC    0x90
#నిర్వచించు ATA_CMD_INITIALIZE_DEVICE_PARAMETERS 0x91
#నిర్వచించు ATA_CMD_STANDBY_IMMEDIATE2           0x94
#నిర్వచించు ATA_CMD_IDLE_IMMEDIATE2              0x95
#నిర్వచించు ATA_CMD_STANDBY2                     0x96
#నిర్వచించు ATA_CMD_IDLE2                        0x97
#నిర్వచించు ATA_CMD_CHECK_POWER_MODE2            0x98
#నిర్వచించు ATA_CMD_SLEEP2                       0x99
#నిర్వచించు ATA_CMD_PACKET                       0xA0
#నిర్వచించు ATA_CMD_IDENTIFY_PACKET_DEVICE       0xA1
#నిర్వచించు ATA_CMD_CFA_ERASE_SECTORS            0xC0
#నిర్వచించు ATA_CMD_READ_గుణిజం                0xC4
#నిర్వచించు ATA_CMD_WRITE_గుణిజం               0xC5
#నిర్వచించు ATA_CMD_సమితి_గుణిజం_బహుళకం            0xC6
#నిర్వచించు ATA_CMD_READ_DMA_QUEUED              0xC7
#నిర్వచించు ATA_CMD_READ_DMA                     0xC8
#నిర్వచించు ATA_CMD_WRITE_DMA                    0xCA
#నిర్వచించు ATA_CMD_WRITE_DMA_QUEUED             0xCC
#నిర్వచించు ATA_CMD_CFA_WRITE_గుణిజం_WO_ERASE  0xCD
#నిర్వచించు ATA_CMD_STANDBY_తక్షణ            0xE0
#నిర్వచించు ATA_CMD_IDLE_తక్షణ               0xE1
#నిర్వచించు ATA_CMD_STANDBY                      0xE2
#నిర్వచించు ATA_CMD_IDLE                         0xE3
#నిర్వచించు ATA_CMD_READ_బఫరు                  0xE4
#నిర్వచించు ATA_CMD_CHECK_POWER_బహుళకం             0xE5
#నిర్వచించు ATA_CMD_SLEEP                        0xE6
#నిర్వచించు ATA_CMD_FLUSH_CACHE                  0xE7
#నిర్వచించు ATA_CMD_WRITE_బఫరు                 0xE8
#నిర్వచించు ATA_CMD_IDENTIFY_DEVICE              0xEC
#నిర్వచించు ATA_CMD_సమితి_FEATURES                 0xEF
#నిర్వచించు ATA_CMD_READ_NATIVE_MAX_చిరునామా      0xF8
#నిర్వచించు ATA_CMD_సమితి_MAX                      0xF9

#నిర్వచించు ATA_సమితి_FEATRUE_TRANSFER_బహుళకం        0x03
#నిర్వచించు ATA_TRANSFER_బహుళకం_ULTRA_DMA          0x40
#నిర్వచించు ATA_TRANSFER_బహుళకం_MULTIWORD_DMA      0x20
#నిర్వచించు ATA_TRANSFER_బహుళకం_PIO_FLOW_CTRL      0x08
#నిర్వచించు ATA_TRANSFER_బహుళకం_DEFAULT_PIO        0x00
#అయితే_ముగింపు // ata.h

#నిర్వచితం_కాకపోతే __BLOCK_H
#నిర్వచించు __BLOCK_H

#అన్తర్భావ "types.h" // u32


/****************************************************************
 * Disk command request
 ****************************************************************/

నిర్మాణం disk_op_s {
    శూన్య *buf_fl;
    నిర్మాణం drive_s *drive_fl;
    u8 command;
    u16 లెక్క;
    సంఘం {
        // Commands: READ, WRITE, VERIFY, SEEK, FORMAT
        u64 lba;
        // Commands: SCSI
        నిర్మాణం {
            u16 blocksize;
            శూన్య *cdbcmd;
        };
    };
};

#నిర్వచించు CMD_RESET   0x00
#నిర్వచించు CMD_READ    0x02
#నిర్వచించు CMD_WRITE   0x03
#నిర్వచించు CMD_VERIFY  0x04
#నిర్వచించు CMD_FORMAT  0x05
#నిర్వచించు CMD_SEEK    0x07
#నిర్వచించు CMD_ISREADY 0x10
#నిర్వచించు CMD_SCSI    0x20


/****************************************************************
 * Global storage
 ****************************************************************/

నిర్మాణం chs_s {
    u16 తల;
    u16 cylinder;
    u16 sector;
    u16 pad;
};

నిర్మాణం drive_s {
    u8 type;            // Driver type (DTYPE_*)
    u8 floppy_type;     // Type of floppy (only for floppy drives).
    నిర్మాణం chs_s lchs;  // Logical CHS
    u64 sectors;        // Total sectors count
    u32 cntl_id;        // Unique id for a given driver type.
    u8 removable;       // Is media removable (currently unused)

    // Info for EDD calls
    u8 translation;     // type of translation
    u16 blksize;        // block size
    నిర్మాణం chs_s pchs;  // Physical CHS
    u32 max_segment_కొలత; //max_segment_size
    u32 max_segments;   //max_segments
};

#నిర్వచించు DISK_SECTOR_కొలత  512
#నిర్వచించు CDROM_SECTOR_కొలత 2048

#నిర్వచించు DTYPE_NONE         0x00
#నిర్వచించు DTYPE_FLOPPY       0x10
#నిర్వచించు DTYPE_ATA          0x20
#నిర్వచించు DTYPE_ATA_ATAPI    0x21
#నిర్వచించు DTYPE_RAMDISK      0x30
#నిర్వచించు DTYPE_CDEMU        0x40
#నిర్వచించు DTYPE_AHCI         0x50
#నిర్వచించు DTYPE_AHCI_ATAPI   0x51
#నిర్వచించు DTYPE_VIRTIO_SCSI  0x60
#నిర్వచించు DTYPE_VIRTIO_BLK   0x61
#నిర్వచించు DTYPE_USB          0x70
#నిర్వచించు DTYPE_USB_32       0x71
#నిర్వచించు DTYPE_UAS          0x72
#నిర్వచించు DTYPE_UAS_32       0x73
#నిర్వచించు DTYPE_LSI_SCSI     0x80
#నిర్వచించు DTYPE_ESP_SCSI     0x81
#నిర్వచించు DTYPE_MEGASAS      0x82
#నిర్వచించు DTYPE_PVSCSI       0x83
#నిర్వచించు DTYPE_MPT_SCSI     0x84
#నిర్వచించు DTYPE_SDCARD       0x90
#నిర్వచించు DTYPE_NVME         0x91

#నిర్వచించు MAXDESCSIZE 80

#నిర్వచించు TRANSLATION_NONE  0
#నిర్వచించు TRANSLATION_LBA   1
#నిర్వచించు TRANSLATION_LARGE 2
#నిర్వచించు TRANSLATION_RECHS 3
#నిర్వచించు TRANSLATION_HOST 4

#నిర్వచించు EXTTYPE_FLOPPY 0
#నిర్వచించు EXTTYPE_HD 1
#నిర్వచించు EXTTYPE_CD 2

#నిర్వచించు EXTSTART_HD 0x80
#నిర్వచించు EXTSTART_CD 0xE0


/****************************************************************
 * Function defs
 ****************************************************************/

// block.c
బాహ్య u8 Floppy_లెక్క, CDCount;
బాహ్య u8 *bounce_buf_fl;
నిర్మాణం drive_s *getDrive(u8 exttype, u8 extdriveoffset);
పూర్ణాంకం getDriveId(u8 exttype, నిర్మాణం drive_s *drive);
శూన్య map_floppy_drive(నిర్మాణం drive_s *drive);
శూన్య map_hd_drive(నిర్మాణం drive_s *drive);
శూన్య map_cd_drive(నిర్మాణం drive_s *drive);
నిర్మాణం int13dpt_s;
పూర్ణాంకం fill_edd(నిర్మాణం segoff_s edd, నిర్మాణం drive_s *drive_fl);
శూన్య ఖండం_అమరిక(శూన్య);
పూర్ణాంకం default_process_op(నిర్మాణం disk_op_s *op);
పూర్ణాంకం process_op(నిర్మాణం disk_op_s *op);
పూర్ణాంకం create_bounce_buf(శూన్య);

#అయితే_ముగింపు // block.h

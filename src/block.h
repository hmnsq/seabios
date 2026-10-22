#यदि_अपरिभाषितम् __BLOCK_H
#परिभाषय __BLOCK_H

#अन्तर्भाव "types.h" // u32


/****************************************************************
 * Disk command request
 ****************************************************************/

संरचना disk_op_s {
    शून्य *buf_fl;
    संरचना drive_s *drive_fl;
    u8 command;
    u16 गणनम्;
    सङ्घ {
        // Commands: READ, WRITE, VERIFY, SEEK, FORMAT
        u64 lba;
        // Commands: SCSI
        संरचना {
            u16 blocksize;
            शून्य *cdbcmd;
        };
    };
};

#परिभाषय CMD_RESET   0x00
#परिभाषय CMD_READ    0x02
#परिभाषय CMD_WRITE   0x03
#परिभाषय CMD_VERIFY  0x04
#परिभाषय CMD_FORMAT  0x05
#परिभाषय CMD_SEEK    0x07
#परिभाषय CMD_ISREADY 0x10
#परिभाषय CMD_SCSI    0x20


/****************************************************************
 * Global storage
 ****************************************************************/

संरचना chs_s {
    u16 शीर्ष;
    u16 cylinder;
    u16 sector;
    u16 pad;
};

संरचना drive_s {
    u8 type;            // Driver type (DTYPE_*)
    u8 floppy_type;     // Type of floppy (only for floppy drives).
    संरचना chs_s lchs;  // Logical CHS
    u64 sectors;        // Total sectors count
    u32 cntl_id;        // Unique id for a given driver type.
    u8 removable;       // Is media removable (currently unused)

    // Info for EDD calls
    u8 translation;     // type of translation
    u16 blksize;        // block size
    संरचना chs_s pchs;  // Physical CHS
    u32 max_segment_आकार; //max_segment_size
    u32 max_segments;   //max_segments
};

#परिभाषय DISK_SECTOR_आकार  512
#परिभाषय CDROM_SECTOR_आकार 2048

#परिभाषय DTYPE_NONE         0x00
#परिभाषय DTYPE_FLOPPY       0x10
#परिभाषय DTYPE_ATA          0x20
#परिभाषय DTYPE_ATA_ATAPI    0x21
#परिभाषय DTYPE_RAMDISK      0x30
#परिभाषय DTYPE_CDEMU        0x40
#परिभाषय DTYPE_AHCI         0x50
#परिभाषय DTYPE_AHCI_ATAPI   0x51
#परिभाषय DTYPE_VIRTIO_SCSI  0x60
#परिभाषय DTYPE_VIRTIO_BLK   0x61
#परिभाषय DTYPE_USB          0x70
#परिभाषय DTYPE_USB_32       0x71
#परिभाषय DTYPE_UAS          0x72
#परिभाषय DTYPE_UAS_32       0x73
#परिभाषय DTYPE_LSI_SCSI     0x80
#परिभाषय DTYPE_ESP_SCSI     0x81
#परिभाषय DTYPE_MEGASAS      0x82
#परिभाषय DTYPE_PVSCSI       0x83
#परिभाषय DTYPE_MPT_SCSI     0x84
#परिभाषय DTYPE_SDCARD       0x90
#परिभाषय DTYPE_NVME         0x91

#परिभाषय MAXDESCSIZE 80

#परिभाषय TRANSLATION_NONE  0
#परिभाषय TRANSLATION_LBA   1
#परिभाषय TRANSLATION_LARGE 2
#परिभाषय TRANSLATION_RECHS 3
#परिभाषय TRANSLATION_HOST 4

#परिभाषय EXTTYPE_FLOPPY 0
#परिभाषय EXTTYPE_HD 1
#परिभाषय EXTTYPE_CD 2

#परिभाषय EXTSTART_HD 0x80
#परिभाषय EXTSTART_CD 0xE0


/****************************************************************
 * Function defs
 ****************************************************************/

// block.c
बाह्य u8 Floppy_गणनम्, CDCount;
बाह्य u8 *bounce_buf_fl;
संरचना drive_s *getDrive(u8 exttype, u8 extdriveoffset);
पूर्णाङ्क getDriveId(u8 exttype, संरचना drive_s *drive);
शून्य map_floppy_drive(संरचना drive_s *drive);
शून्य map_hd_drive(संरचना drive_s *drive);
शून्य map_cd_drive(संरचना drive_s *drive);
संरचना int13dpt_s;
पूर्णाङ्क fill_edd(संरचना segoff_s edd, संरचना drive_s *drive_fl);
शून्य खण्डम्_सज्जता(शून्य);
पूर्णाङ्क default_process_op(संरचना disk_op_s *op);
पूर्णाङ्क process_op(संरचना disk_op_s *op);
पूर्णाङ्क create_bounce_buf(शून्य);

#यदि_अन्तः // block.h

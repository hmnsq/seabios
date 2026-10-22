#جيڪڏهن_وصف_نه_ٿيل __BLOCK_H
#وصف_ڪريو __BLOCK_H

#ضمن "types.h" // u32


/****************************************************************
 * Disk command request
 ****************************************************************/

بنية disk_op_s {
    فراغ *buf_fl;
    بنية drive_s *drive_fl;
    u8 command;
    u16 ڳڻپ;
    اتحاد {
        // Commands: READ, WRITE, VERIFY, SEEK, FORMAT
        u64 lba;
        // Commands: SCSI
        بنية {
            u16 blocksize;
            فراغ *cdbcmd;
        };
    };
};

#وصف_ڪريو CMD_RESET   0x00
#وصف_ڪريو CMD_READ    0x02
#وصف_ڪريو CMD_WRITE   0x03
#وصف_ڪريو CMD_VERIFY  0x04
#وصف_ڪريو CMD_FORMAT  0x05
#وصف_ڪريو CMD_SEEK    0x07
#وصف_ڪريو CMD_ISREADY 0x10
#وصف_ڪريو CMD_SCSI    0x20


/****************************************************************
 * Global storage
 ****************************************************************/

بنية chs_s {
    u16 سر;
    u16 cylinder;
    u16 sector;
    u16 pad;
};

بنية drive_s {
    u8 type;            // Driver type (DTYPE_*)
    u8 floppy_type;     // Type of floppy (only for floppy drives).
    بنية chs_s lchs;  // Logical CHS
    u64 sectors;        // Total sectors count
    u32 cntl_id;        // Unique id for a given driver type.
    u8 removable;       // Is media removable (currently unused)

    // Info for EDD calls
    u8 translation;     // type of translation
    u16 blksize;        // block size
    بنية chs_s pchs;  // Physical CHS
    u32 max_segment_سائيز; //max_segment_size
    u32 max_segments;   //max_segments
};

#وصف_ڪريو DISK_SECTOR_سائيز  512
#وصف_ڪريو CDROM_SECTOR_سائيز 2048

#وصف_ڪريو DTYPE_NONE         0x00
#وصف_ڪريو DTYPE_FLOPPY       0x10
#وصف_ڪريو DTYPE_ATA          0x20
#وصف_ڪريو DTYPE_ATA_ATAPI    0x21
#وصف_ڪريو DTYPE_RAMDISK      0x30
#وصف_ڪريو DTYPE_CDEMU        0x40
#وصف_ڪريو DTYPE_AHCI         0x50
#وصف_ڪريو DTYPE_AHCI_ATAPI   0x51
#وصف_ڪريو DTYPE_VIRTIO_SCSI  0x60
#وصف_ڪريو DTYPE_VIRTIO_BLK   0x61
#وصف_ڪريو DTYPE_USB          0x70
#وصف_ڪريو DTYPE_USB_32       0x71
#وصف_ڪريو DTYPE_UAS          0x72
#وصف_ڪريو DTYPE_UAS_32       0x73
#وصف_ڪريو DTYPE_LSI_SCSI     0x80
#وصف_ڪريو DTYPE_ESP_SCSI     0x81
#وصف_ڪريو DTYPE_MEGASAS      0x82
#وصف_ڪريو DTYPE_PVSCSI       0x83
#وصف_ڪريو DTYPE_MPT_SCSI     0x84
#وصف_ڪريو DTYPE_SDCARD       0x90
#وصف_ڪريو DTYPE_NVME         0x91

#وصف_ڪريو MAXDESCSIZE 80

#وصف_ڪريو TRANSLATION_NONE  0
#وصف_ڪريو TRANSLATION_LBA   1
#وصف_ڪريو TRANSLATION_LARGE 2
#وصف_ڪريو TRANSLATION_RECHS 3
#وصف_ڪريو TRANSLATION_HOST 4

#وصف_ڪريو EXTTYPE_FLOPPY 0
#وصف_ڪريو EXTTYPE_HD 1
#وصف_ڪريو EXTTYPE_CD 2

#وصف_ڪريو EXTSTART_HD 0x80
#وصف_ڪريو EXTSTART_CD 0xE0


/****************************************************************
 * Function defs
 ****************************************************************/

// block.c
خارجي u8 Floppy_ڳڻپ, CDCount;
خارجي u8 *bounce_buf_fl;
بنية drive_s *getDrive(u8 exttype, u8 extdriveoffset);
صحيح getDriveId(u8 exttype, بنية drive_s *drive);
فراغ map_floppy_drive(بنية drive_s *drive);
فراغ map_hd_drive(بنية drive_s *drive);
فراغ map_cd_drive(بنية drive_s *drive);
بنية int13dpt_s;
صحيح fill_edd(بنية segoff_s edd, بنية drive_s *drive_fl);
فراغ بلاڪ_تياري(فراغ);
صحيح default_process_op(بنية disk_op_s *op);
صحيح process_op(بنية disk_op_s *op);
صحيح create_bounce_buf(فراغ);

#جيڪڏهن_پورو // block.h

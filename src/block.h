#جے_بنا_تعریف __BLOCK_H
#تعریف_کرو __BLOCK_H

#ضمن "types.h" // u32


/****************************************************************
 * Disk command request
 ****************************************************************/

بنية disk_op_s {
    فراغ *buf_fl;
    بنية drive_s *drive_fl;
    u8 command;
    u16 گنتی;
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

#تعریف_کرو CMD_RESET   0x00
#تعریف_کرو CMD_READ    0x02
#تعریف_کرو CMD_WRITE   0x03
#تعریف_کرو CMD_VERIFY  0x04
#تعریف_کرو CMD_FORMAT  0x05
#تعریف_کرو CMD_SEEK    0x07
#تعریف_کرو CMD_ISREADY 0x10
#تعریف_کرو CMD_SCSI    0x20


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
    u32 max_segment_ناپ; //max_segment_size
    u32 max_segments;   //max_segments
};

#تعریف_کرو DISK_SECTOR_ناپ  512
#تعریف_کرو CDROM_SECTOR_ناپ 2048

#تعریف_کرو DTYPE_NONE         0x00
#تعریف_کرو DTYPE_FLOPPY       0x10
#تعریف_کرو DTYPE_ATA          0x20
#تعریف_کرو DTYPE_ATA_ATAPI    0x21
#تعریف_کرو DTYPE_RAMDISK      0x30
#تعریف_کرو DTYPE_CDEMU        0x40
#تعریف_کرو DTYPE_AHCI         0x50
#تعریف_کرو DTYPE_AHCI_ATAPI   0x51
#تعریف_کرو DTYPE_VIRTIO_SCSI  0x60
#تعریف_کرو DTYPE_VIRTIO_BLK   0x61
#تعریف_کرو DTYPE_USB          0x70
#تعریف_کرو DTYPE_USB_32       0x71
#تعریف_کرو DTYPE_UAS          0x72
#تعریف_کرو DTYPE_UAS_32       0x73
#تعریف_کرو DTYPE_LSI_SCSI     0x80
#تعریف_کرو DTYPE_ESP_SCSI     0x81
#تعریف_کرو DTYPE_MEGASAS      0x82
#تعریف_کرو DTYPE_PVSCSI       0x83
#تعریف_کرو DTYPE_MPT_SCSI     0x84
#تعریف_کرو DTYPE_SDCARD       0x90
#تعریف_کرو DTYPE_NVME         0x91

#تعریف_کرو MAXDESCSIZE 80

#تعریف_کرو TRANSLATION_NONE  0
#تعریف_کرو TRANSLATION_LBA   1
#تعریف_کرو TRANSLATION_LARGE 2
#تعریف_کرو TRANSLATION_RECHS 3
#تعریف_کرو TRANSLATION_HOST 4

#تعریف_کرو EXTTYPE_FLOPPY 0
#تعریف_کرو EXTTYPE_HD 1
#تعریف_کرو EXTTYPE_CD 2

#تعریف_کرو EXTSTART_HD 0x80
#تعریف_کرو EXTSTART_CD 0xE0


/****************************************************************
 * Function defs
 ****************************************************************/

// block.c
خارجي u8 Floppy_گنتی, CDCount;
خارجي u8 *bounce_buf_fl;
بنية drive_s *getDrive(u8 exttype, u8 extdriveoffset);
صحيح getDriveId(u8 exttype, بنية drive_s *drive);
فراغ map_floppy_drive(بنية drive_s *drive);
فراغ map_hd_drive(بنية drive_s *drive);
فراغ map_cd_drive(بنية drive_s *drive);
بنية int13dpt_s;
صحيح fill_edd(بنية segoff_s edd, بنية drive_s *drive_fl);
فراغ بلاک_تیاری(فراغ);
صحيح default_process_op(بنية disk_op_s *op);
صحيح process_op(بنية disk_op_s *op);
صحيح create_bounce_buf(فراغ);

#جے_انت // block.h

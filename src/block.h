#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __BLOCK_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __BLOCK_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32


/****************************************************************
 * Disk command request
 ****************************************************************/

ਬਣਤਰ disk_op_s {
    ਖਾਲੀ *buf_fl;
    ਬਣਤਰ drive_s *drive_fl;
    u8 command;
    u16 ਗਿਣਤੀ_2;
    ਸੰਘ {
        // Commands: READ, WRITE, VERIFY, SEEK, FORMAT
        u64 lba;
        // Commands: SCSI
        ਬਣਤਰ {
            u16 blocksize;
            ਖਾਲੀ *cdbcmd;
        };
    };
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMD_RESET   0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMD_READ    0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMD_WRITE   0x03
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMD_VERIFY  0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMD_FORMAT  0x05
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMD_SEEK    0x07
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMD_ISREADY 0x10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMD_SCSI    0x20


/****************************************************************
 * Global storage
 ****************************************************************/

ਬਣਤਰ chs_s {
    u16 ਸਿਰ;
    u16 cylinder;
    u16 sector;
    u16 pad;
};

ਬਣਤਰ drive_s {
    u8 type;            // Driver type (DTYPE_*)
    u8 floppy_type;     // Type of floppy (only for floppy drives).
    ਬਣਤਰ chs_s lchs;  // Logical CHS
    u64 sectors;        // Total sectors count
    u32 cntl_id;        // Unique id for a given driver type.
    u8 removable;       // Is media removable (currently unused)

    // Info for EDD calls
    u8 translation;     // type of translation
    u16 blksize;        // block size
    ਬਣਤਰ chs_s pchs;  // Physical CHS
    u32 max_segment_ਮਾਪ; //max_segment_size
    u32 max_segments;   //max_segments
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DISK_SECTOR_ਮਾਪ  512
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CDROM_SECTOR_ਮਾਪ 2048

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_NONE         0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_FLOPPY       0x10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_ATA          0x20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_ATA_ATAPI    0x21
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_RAMDISK      0x30
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_CDEMU        0x40
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_AHCI         0x50
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_AHCI_ATAPI   0x51
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_VIRTIO_SCSI  0x60
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_VIRTIO_BLK   0x61
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_USB          0x70
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_USB_32       0x71
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_UAS          0x72
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_UAS_32       0x73
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_LSI_SCSI     0x80
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_ESP_SCSI     0x81
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_MEGASAS      0x82
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_PVSCSI       0x83
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_MPT_SCSI     0x84
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_SDCARD       0x90
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DTYPE_NVME         0x91

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXDESCSIZE 80

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRANSLATION_NONE  0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRANSLATION_LBA   1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRANSLATION_LARGE 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRANSLATION_RECHS 3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRANSLATION_HOST 4

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EXTTYPE_FLOPPY 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EXTTYPE_HD 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EXTTYPE_CD 2

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EXTSTART_HD 0x80
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EXTSTART_CD 0xE0


/****************************************************************
 * Function defs
 ****************************************************************/

// block.c
ਬਾਹਰੀ u8 Floppy_ਗਿਣਤੀ, CDCount;
ਬਾਹਰੀ u8 *bounce_buf_fl;
ਬਣਤਰ drive_s *getDrive(u8 exttype, u8 extdriveoffset);
ਪੂਰਨ getDriveId(u8 exttype, ਬਣਤਰ drive_s *drive);
ਖਾਲੀ map_floppy_drive(ਬਣਤਰ drive_s *drive);
ਖਾਲੀ map_hd_drive(ਬਣਤਰ drive_s *drive);
ਖਾਲੀ map_cd_drive(ਬਣਤਰ drive_s *drive);
ਬਣਤਰ int13dpt_s;
ਪੂਰਨ fill_edd(ਬਣਤਰ segoff_s edd, ਬਣਤਰ drive_s *drive_fl);
ਖਾਲੀ ਖੰਡ_ਤਿਆਰੀ(ਖਾਲੀ);
ਪੂਰਨ default_process_op(ਬਣਤਰ disk_op_s *op);
ਪੂਰਨ process_op(ਬਣਤਰ disk_op_s *op);
ਪੂਰਨ create_bounce_buf(ਖਾਲੀ);

#ਜੇ_ਅੰਤ // block.h

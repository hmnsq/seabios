#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __BLOCK_H
#ವ್ಯಾಖ್ಯಾನಿಸು __BLOCK_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32


/****************************************************************
 * Disk command request
 ****************************************************************/

ರಚನೆ disk_op_s {
    ಶೂನ್ಯ *buf_fl;
    ರಚನೆ drive_s *drive_fl;
    u8 command;
    u16 ಎಣಿಕೆ_2;
    ಒಕ್ಕೂಟ {
        // Commands: READ, WRITE, VERIFY, SEEK, FORMAT
        u64 lba;
        // Commands: SCSI
        ರಚನೆ {
            u16 blocksize;
            ಶೂನ್ಯ *cdbcmd;
        };
    };
};

#ವ್ಯಾಖ್ಯಾನಿಸು CMD_RESET   0x00
#ವ್ಯಾಖ್ಯಾನಿಸು CMD_READ    0x02
#ವ್ಯಾಖ್ಯಾನಿಸು CMD_WRITE   0x03
#ವ್ಯಾಖ್ಯಾನಿಸು CMD_VERIFY  0x04
#ವ್ಯಾಖ್ಯಾನಿಸು CMD_FORMAT  0x05
#ವ್ಯಾಖ್ಯಾನಿಸು CMD_SEEK    0x07
#ವ್ಯಾಖ್ಯಾನಿಸು CMD_ISREADY 0x10
#ವ್ಯಾಖ್ಯಾನಿಸು CMD_SCSI    0x20


/****************************************************************
 * Global storage
 ****************************************************************/

ರಚನೆ chs_s {
    u16 ತಲೆ;
    u16 cylinder;
    u16 sector;
    u16 pad;
};

ರಚನೆ drive_s {
    u8 type;            // Driver type (DTYPE_*)
    u8 floppy_type;     // Type of floppy (only for floppy drives).
    ರಚನೆ chs_s lchs;  // Logical CHS
    u64 sectors;        // Total sectors count
    u32 cntl_id;        // Unique id for a given driver type.
    u8 removable;       // Is media removable (currently unused)

    // Info for EDD calls
    u8 translation;     // type of translation
    u16 blksize;        // block size
    ರಚನೆ chs_s pchs;  // Physical CHS
    u32 max_segment_ಗಾತ್ರ; //max_segment_size
    u32 max_segments;   //max_segments
};

#ವ್ಯಾಖ್ಯಾನಿಸು DISK_SECTOR_ಗಾತ್ರ  512
#ವ್ಯಾಖ್ಯಾನಿಸು CDROM_SECTOR_ಗಾತ್ರ 2048

#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_NONE         0x00
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_FLOPPY       0x10
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_ATA          0x20
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_ATA_ATAPI    0x21
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_RAMDISK      0x30
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_CDEMU        0x40
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_AHCI         0x50
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_AHCI_ATAPI   0x51
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_VIRTIO_SCSI  0x60
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_VIRTIO_BLK   0x61
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_USB          0x70
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_USB_32       0x71
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_UAS          0x72
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_UAS_32       0x73
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_LSI_SCSI     0x80
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_ESP_SCSI     0x81
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_MEGASAS      0x82
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_PVSCSI       0x83
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_MPT_SCSI     0x84
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_SDCARD       0x90
#ವ್ಯಾಖ್ಯಾನಿಸು DTYPE_NVME         0x91

#ವ್ಯಾಖ್ಯಾನಿಸು MAXDESCSIZE 80

#ವ್ಯಾಖ್ಯಾನಿಸು TRANSLATION_NONE  0
#ವ್ಯಾಖ್ಯಾನಿಸು TRANSLATION_LBA   1
#ವ್ಯಾಖ್ಯಾನಿಸು TRANSLATION_LARGE 2
#ವ್ಯಾಖ್ಯಾನಿಸು TRANSLATION_RECHS 3
#ವ್ಯಾಖ್ಯಾನಿಸು TRANSLATION_HOST 4

#ವ್ಯಾಖ್ಯಾನಿಸು EXTTYPE_FLOPPY 0
#ವ್ಯಾಖ್ಯಾನಿಸು EXTTYPE_HD 1
#ವ್ಯಾಖ್ಯಾನಿಸು EXTTYPE_CD 2

#ವ್ಯಾಖ್ಯಾನಿಸು EXTSTART_HD 0x80
#ವ್ಯಾಖ್ಯಾನಿಸು EXTSTART_CD 0xE0


/****************************************************************
 * Function defs
 ****************************************************************/

// block.c
ಬಾಹ್ಯ u8 Floppy_ಎಣಿಕೆ, CDCount;
ಬಾಹ್ಯ u8 *bounce_buf_fl;
ರಚನೆ drive_s *getDrive(u8 exttype, u8 extdriveoffset);
ಪೂರ್ಣಾಂಕ getDriveId(u8 exttype, ರಚನೆ drive_s *drive);
ಶೂನ್ಯ map_floppy_drive(ರಚನೆ drive_s *drive);
ಶೂನ್ಯ map_hd_drive(ರಚನೆ drive_s *drive);
ಶೂನ್ಯ map_cd_drive(ರಚನೆ drive_s *drive);
ರಚನೆ int13dpt_s;
ಪೂರ್ಣಾಂಕ fill_edd(ರಚನೆ segoff_s edd, ರಚನೆ drive_s *drive_fl);
ಶೂನ್ಯ ಖಂಡ_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ default_process_op(ರಚನೆ disk_op_s *op);
ಪೂರ್ಣಾಂಕ process_op(ರಚನೆ disk_op_s *op);
ಪೂರ್ಣಾಂಕ create_bounce_buf(ಶೂನ್ಯ);

#ಆದರೆ_ಅಂತ್ಯ // block.h

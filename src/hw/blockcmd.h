// Definitions for SCSI style command data blocks.
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __BLOCKCMD_H
#ವ್ಯಾಖ್ಯಾನಿಸು __BLOCKCMD_H

#ಅನ್ತರ್ಭಾವ "types.h" // u8

#ವ್ಯಾಖ್ಯಾನಿಸು CDB_CMD_READ_10 0x28
#ವ್ಯಾಖ್ಯಾನಿಸು CDB_CMD_VERIFY_10 0x2f
#ವ್ಯಾಖ್ಯಾನಿಸು CDB_CMD_WRITE_10 0x2a

ರಚನೆ cdb_rwdata_10 {
    u8 command;
    u8 flags;
    u32 lba;
    u8 resreved_06;
    u16 ಎಣಿಕೆ_2;
    u8 reserved_09;
    u8 pad[6];
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು CDB_CMD_READ_16 0x88
#ವ್ಯಾಖ್ಯಾನಿಸು CDB_CMD_WRITE_16 0x8A
ರಚನೆ cdb_rwdata_16 {
    u8 command;
    u8 flags;
    u64 lba;
    u32 ಎಣಿಕೆ_2;
    u16 reserved_14;
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು CDB_CMD_READ_ಸಾಮರ್ಥ್ಯ 0x25

ರಚನೆ cdb_read_ಸಾಮರ್ಥ್ಯ {
    u8 command;
    u8 flags;
    u8 resreved_02[8];
    u8 pad[6];
} PACKED;

ರಚನೆ cdbres_read_ಸಾಮರ್ಥ್ಯ {
    u32 sectors;
    u32 blksize;
} PACKED;


#ವ್ಯಾಖ್ಯಾನಿಸು CDB_CMD_SERVICE_ACTION_ಒಳ 0x9E
#ವ್ಯಾಖ್ಯಾನಿಸು CDB_CMD_SAI_READ_ಸಾಮರ್ಥ್ಯ_16 0x10
ರಚನೆ cdb_sai_read_ಸಾಮರ್ಥ್ಯ_16 {
    u8 command;
    u8 flags;
    u64 lba; //marked as obsolete?
    u32 len;
    u16 reserved_14;
} PACKED;

ರಚನೆ cdbres_read_ಸಾಮರ್ಥ್ಯ_16 {
    u64 sectors;
    u32 blksize;
    u8 reserved_12[20];
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು CDB_CMD_TEST_UNIT_READY  0x00
#ವ್ಯಾಖ್ಯಾನಿಸು CDB_CMD_INQUIRY          0x12
#ವ್ಯಾಖ್ಯಾನಿಸು CDB_CMD_REQUEST_SENSE    0x03

ರಚನೆ cdb_request_sense {
    u8 command;
    u8 flags;
    u16 reserved_02;
    u8 ಉದ್ದಳತೆ;
    u8 reserved_05;
    u8 pad[10];
} PACKED;

ರಚನೆ cdbres_request_sense {
    u8 errcode;
    u8 segment;
    u8 flags;
    u32 info;
    u8 additional;
    u32 specific;
    u8 asc;
    u8 ascq;
    u32 reserved_0e;
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು SCSI_TYPE_DISK  0x00
#ವ್ಯಾಖ್ಯಾನಿಸು SCSI_TYPE_CDROM 0x05

ರಚನೆ cdbres_inquiry {
    u8 pdt;
    u8 removable;
    u8 reserved_02[2];
    u8 additional;
    u8 reserved_05[3];
    ಅಕ್ಷರ vendor[8];
    ಅಕ್ಷರ ಗುಣಲಬ್ಧ[16];
    ಅಕ್ಷರ rev[4];
} PACKED;

#ವ್ಯಾಖ್ಯಾನಿಸು CDB_CMD_ಬಹುಲಕ_SENSE    0x5A
#ವ್ಯಾಖ್ಯಾನಿಸು ಬಹುಲಕ_ಪುಟ_HD_GEOMETRY 0x04

ರಚನೆ cdb_ಬಹುಲಕ_sense {
    u8 command;
    u8 flags;
    u8 ಪುಟ;
    u32 reserved_03;
    u16 ಎಣಿಕೆ_2;
    u8 reserved_09;
    u8 pad[6];
} PACKED;

ರಚನೆ cdbres_ಬಹುಲಕ_sense_geom {
    u8 ಬಳಸದ_00[3];
    u8 read_only;
    u32 ಬಳಸದ_04;
    u8 ಪುಟ;
    u8 ಉದ್ದಳತೆ;
    u8 cyl[3];
    u8 heads;
    u8 precomp[3];
    u8 reduced[3];
    u16 ಹೆಜ್ಜೆ_ದರ;
    u8 landing[3];
    u16 rpm;
} PACKED;

// blockcmd.c
ರಚನೆ disk_op_s;
ಪೂರ್ಣಾಂಕ scsi_fill_cmd(ರಚನೆ disk_op_s *op, ಶೂನ್ಯ *cdbcmd, ಪೂರ್ಣಾಂಕ maxcdb);
ಪೂರ್ಣಾಂಕ scsi_is_read(ರಚನೆ disk_op_s *op);
ಪೂರ್ಣಾಂಕ scsi_is_ready(ರಚನೆ disk_op_s *op);
ರಚನೆ drive_s;
ಪೂರ್ಣಾಂಕ scsi_drive_ಸಿದ್ಧತೆ(ರಚನೆ drive_s *drive, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಪೂರ್ಣಾಂಕ prio);
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ (*scsi_ಕೂಡಿಸು_lun)(u32 lun, ರಚನೆ drive_s *tmpl_drv);
ಪೂರ್ಣಾಂಕ scsi_rep_luns_ಪರಿಶೀಲನೆ(ರಚನೆ drive_s *tmp_drive, scsi_ಕೂಡಿಸು_lun ಕೂಡಿಸು_lun);
ಪೂರ್ಣಾಂಕ scsi_sequential_ಪರಿಶೀಲನೆ(ರಚನೆ drive_s *tmp_drive, u32 maxluns,
                         scsi_ಕೂಡಿಸು_lun ಕೂಡಿಸು_lun);

#ಆದರೆ_ಅಂತ್ಯ // blockcmd.h

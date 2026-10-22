// Definitions for SCSI style command data blocks.
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __BLOCKCMD_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __BLOCKCMD_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u8

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CDB_CMD_READ_10 0x28
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CDB_CMD_VERIFY_10 0x2f
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CDB_CMD_WRITE_10 0x2a

ਬਣਤਰ cdb_rwdata_10 {
    u8 command;
    u8 flags;
    u32 lba;
    u8 resreved_06;
    u16 ਗਿਣਤੀ_2;
    u8 reserved_09;
    u8 pad[6];
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CDB_CMD_READ_16 0x88
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CDB_CMD_WRITE_16 0x8A
ਬਣਤਰ cdb_rwdata_16 {
    u8 command;
    u8 flags;
    u64 lba;
    u32 ਗਿਣਤੀ_2;
    u16 reserved_14;
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CDB_CMD_READ_ਸਮਰੱਥਾ 0x25

ਬਣਤਰ cdb_read_ਸਮਰੱਥਾ {
    u8 command;
    u8 flags;
    u8 resreved_02[8];
    u8 pad[6];
} PACKED;

ਬਣਤਰ cdbres_read_ਸਮਰੱਥਾ {
    u32 sectors;
    u32 blksize;
} PACKED;


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CDB_CMD_SERVICE_ACTION_ਵਿੱਚ 0x9E
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CDB_CMD_SAI_READ_ਸਮਰੱਥਾ_16 0x10
ਬਣਤਰ cdb_sai_read_ਸਮਰੱਥਾ_16 {
    u8 command;
    u8 flags;
    u64 lba; //marked as obsolete?
    u32 len;
    u16 reserved_14;
} PACKED;

ਬਣਤਰ cdbres_read_ਸਮਰੱਥਾ_16 {
    u64 sectors;
    u32 blksize;
    u8 reserved_12[20];
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CDB_CMD_TEST_UNIT_READY  0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CDB_CMD_INQUIRY          0x12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CDB_CMD_REQUEST_SENSE    0x03

ਬਣਤਰ cdb_request_sense {
    u8 command;
    u8 flags;
    u16 reserved_02;
    u8 ਲੰਮਾਈ;
    u8 reserved_05;
    u8 pad[10];
} PACKED;

ਬਣਤਰ cdbres_request_sense {
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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCSI_TYPE_DISK  0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCSI_TYPE_CDROM 0x05

ਬਣਤਰ cdbres_inquiry {
    u8 pdt;
    u8 removable;
    u8 reserved_02[2];
    u8 additional;
    u8 reserved_05[3];
    ਅੱਖਰ vendor[8];
    ਅੱਖਰ ਗੁਣਨਫਲ[16];
    ਅੱਖਰ rev[4];
} PACKED;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CDB_CMD_ਬਹੁਲਕ_SENSE    0x5A
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਬਹੁਲਕ_ਪੰਨਾ_HD_GEOMETRY 0x04

ਬਣਤਰ cdb_ਬਹੁਲਕ_sense {
    u8 command;
    u8 flags;
    u8 ਪੰਨਾ;
    u32 reserved_03;
    u16 ਗਿਣਤੀ_2;
    u8 reserved_09;
    u8 pad[6];
} PACKED;

ਬਣਤਰ cdbres_ਬਹੁਲਕ_sense_geom {
    u8 ਅਣਵਰਤਿਆ_00[3];
    u8 read_only;
    u32 ਅਣਵਰਤਿਆ_04;
    u8 ਪੰਨਾ;
    u8 ਲੰਮਾਈ;
    u8 cyl[3];
    u8 heads;
    u8 precomp[3];
    u8 reduced[3];
    u16 ਕਦਮ_ਦਰ;
    u8 landing[3];
    u16 rpm;
} PACKED;

// blockcmd.c
ਬਣਤਰ disk_op_s;
ਪੂਰਨ scsi_fill_cmd(ਬਣਤਰ disk_op_s *op, ਖਾਲੀ *cdbcmd, ਪੂਰਨ maxcdb);
ਪੂਰਨ scsi_is_read(ਬਣਤਰ disk_op_s *op);
ਪੂਰਨ scsi_is_ready(ਬਣਤਰ disk_op_s *op);
ਬਣਤਰ drive_s;
ਪੂਰਨ scsi_drive_ਤਿਆਰੀ(ਬਣਤਰ drive_s *drive, ਅਚਲ ਅੱਖਰ *s, ਪੂਰਨ prio);
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ (*scsi_ਜੋੜੋ_lun)(u32 lun, ਬਣਤਰ drive_s *tmpl_drv);
ਪੂਰਨ scsi_rep_luns_ਜਾਂਚ(ਬਣਤਰ drive_s *tmp_drive, scsi_ਜੋੜੋ_lun ਜੋੜੋ_lun);
ਪੂਰਨ scsi_sequential_ਜਾਂਚ(ਬਣਤਰ drive_s *tmp_drive, u32 maxluns,
                         scsi_ਜੋੜੋ_lun ਜੋੜੋ_lun);

#ਜੇ_ਅੰਤ // blockcmd.h

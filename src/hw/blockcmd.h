// Definitions for SCSI style command data blocks.
#اگر_غیر_تعریف_شدہ __BLOCKCMD_H
#تعریف_کرو __BLOCKCMD_H

#شامل "types.h" // u8

#تعریف_کرو CDB_CMD_READ_10 0x28
#تعریف_کرو CDB_CMD_VERIFY_10 0x2f
#تعریف_کرو CDB_CMD_WRITE_10 0x2a

ساخت cdb_rwdata_10 {
    u8 command;
    u8 flags;
    u32 lba;
    u8 resreved_06;
    u16 گنتی;
    u8 reserved_09;
    u8 pad[6];
} PACKED;

#تعریف_کرو CDB_CMD_READ_16 0x88
#تعریف_کرو CDB_CMD_WRITE_16 0x8A
ساخت cdb_rwdata_16 {
    u8 command;
    u8 flags;
    u64 lba;
    u32 گنتی;
    u16 reserved_14;
} PACKED;

#تعریف_کرو CDB_CMD_READ_استعداد 0x25

ساخت cdb_read_استعداد {
    u8 command;
    u8 flags;
    u8 resreved_02[8];
    u8 pad[6];
} PACKED;

ساخت cdbres_read_استعداد {
    u32 sectors;
    u32 blksize;
} PACKED;


#تعریف_کرو CDB_CMD_SERVICE_ACTION_اندر 0x9E
#تعریف_کرو CDB_CMD_SAI_READ_استعداد_16 0x10
ساخت cdb_sai_read_استعداد_16 {
    u8 command;
    u8 flags;
    u64 lba; //marked as obsolete?
    u32 len;
    u16 reserved_14;
} PACKED;

ساخت cdbres_read_استعداد_16 {
    u64 sectors;
    u32 blksize;
    u8 reserved_12[20];
} PACKED;

#تعریف_کرو CDB_CMD_TEST_UNIT_READY  0x00
#تعریف_کرو CDB_CMD_INQUIRY          0x12
#تعریف_کرو CDB_CMD_REQUEST_SENSE    0x03

ساخت cdb_request_sense {
    u8 command;
    u8 flags;
    u16 reserved_02;
    u8 لمبائی;
    u8 reserved_05;
    u8 pad[10];
} PACKED;

ساخت cdbres_request_sense {
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

#تعریف_کرو SCSI_TYPE_DISK  0x00
#تعریف_کرو SCSI_TYPE_CDROM 0x05

ساخت cdbres_inquiry {
    u8 pdt;
    u8 removable;
    u8 reserved_02[2];
    u8 additional;
    u8 reserved_05[3];
    حرف vendor[8];
    حرف حاصل_ضرب[16];
    حرف rev[4];
} PACKED;

#تعریف_کرو CDB_CMD_منوال_SENSE    0x5A
#تعریف_کرو منوال_صفحہ_HD_GEOMETRY 0x04

ساخت cdb_منوال_sense {
    u8 command;
    u8 flags;
    u8 صفحہ;
    u32 reserved_03;
    u16 گنتی;
    u8 reserved_09;
    u8 pad[6];
} PACKED;

ساخت cdbres_منوال_sense_geom {
    u8 غیر_مستعمل_00[3];
    u8 read_only;
    u32 غیر_مستعمل_04;
    u8 صفحہ;
    u8 لمبائی;
    u8 cyl[3];
    u8 heads;
    u8 precomp[3];
    u8 reduced[3];
    u16 قدم_شرح;
    u8 landing[3];
    u16 rpm;
} PACKED;

// blockcmd.c
ساخت disk_op_s;
صحیح scsi_fill_cmd(ساخت disk_op_s *op, خالی *cdbcmd, صحیح maxcdb);
صحیح scsi_is_read(ساخت disk_op_s *op);
صحیح scsi_is_ready(ساخت disk_op_s *op);
ساخت drive_s;
صحیح scsi_drive_تیاری(ساخت drive_s *drive, ثابت حرف *s, صحیح prio);
قسمتعریف صحیح (*scsi_جوڑو_lun)(u32 lun, ساخت drive_s *tmpl_drv);
صحیح scsi_rep_luns_جائزہ(ساخت drive_s *tmp_drive, scsi_جوڑو_lun جوڑو_lun);
صحیح scsi_sequential_جائزہ(ساخت drive_s *tmp_drive, u32 maxluns,
                         scsi_جوڑو_lun جوڑو_lun);

#اگر_ختم // blockcmd.h

// Definitions for SCSI style command data blocks.
#جيڪڏهن_وصف_نه_ٿيل __BLOCKCMD_H
#وصف_ڪريو __BLOCKCMD_H

#ضمن "types.h" // u8

#وصف_ڪريو CDB_CMD_READ_10 0x28
#وصف_ڪريو CDB_CMD_VERIFY_10 0x2f
#وصف_ڪريو CDB_CMD_WRITE_10 0x2a

بنية cdb_rwdata_10 {
    u8 command;
    u8 flags;
    u32 lba;
    u8 resreved_06;
    u16 ڳڻپ;
    u8 reserved_09;
    u8 pad[6];
} PACKED;

#وصف_ڪريو CDB_CMD_READ_16 0x88
#وصف_ڪريو CDB_CMD_WRITE_16 0x8A
بنية cdb_rwdata_16 {
    u8 command;
    u8 flags;
    u64 lba;
    u32 ڳڻپ;
    u16 reserved_14;
} PACKED;

#وصف_ڪريو CDB_CMD_READ_صلاحيت 0x25

بنية cdb_read_صلاحيت {
    u8 command;
    u8 flags;
    u8 resreved_02[8];
    u8 pad[6];
} PACKED;

بنية cdbres_read_صلاحيت {
    u32 sectors;
    u32 blksize;
} PACKED;


#وصف_ڪريو CDB_CMD_SERVICE_ACTION_اندريون 0x9E
#وصف_ڪريو CDB_CMD_SAI_READ_صلاحيت_16 0x10
بنية cdb_sai_read_صلاحيت_16 {
    u8 command;
    u8 flags;
    u64 lba; //marked as obsolete?
    u32 len;
    u16 reserved_14;
} PACKED;

بنية cdbres_read_صلاحيت_16 {
    u64 sectors;
    u32 blksize;
    u8 reserved_12[20];
} PACKED;

#وصف_ڪريو CDB_CMD_TEST_UNIT_READY  0x00
#وصف_ڪريو CDB_CMD_INQUIRY          0x12
#وصف_ڪريو CDB_CMD_REQUEST_SENSE    0x03

بنية cdb_request_sense {
    u8 command;
    u8 flags;
    u16 reserved_02;
    u8 ڊيگهائي;
    u8 reserved_05;
    u8 pad[10];
} PACKED;

بنية cdbres_request_sense {
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

#وصف_ڪريو SCSI_TYPE_DISK  0x00
#وصف_ڪريو SCSI_TYPE_CDROM 0x05

بنية cdbres_inquiry {
    u8 pdt;
    u8 removable;
    u8 reserved_02[2];
    u8 additional;
    u8 reserved_05[3];
    حرف vendor[8];
    حرف حاصل_ضرب[16];
    حرف rev[4];
} PACKED;

#وصف_ڪريو CDB_CMD_گهڻائي_SENSE    0x5A
#وصف_ڪريو گهڻائي_صفحو_HD_GEOMETRY 0x04

بنية cdb_گهڻائي_sense {
    u8 command;
    u8 flags;
    u8 صفحو;
    u32 reserved_03;
    u16 ڳڻپ;
    u8 reserved_09;
    u8 pad[6];
} PACKED;

بنية cdbres_گهڻائي_sense_geom {
    u8 اڻ_استعمال_ٿيل_00[3];
    u8 read_only;
    u32 اڻ_استعمال_ٿيل_04;
    u8 صفحو;
    u8 ڊيگهائي;
    u8 cyl[3];
    u8 heads;
    u8 precomp[3];
    u8 reduced[3];
    u16 قدم_شرح;
    u8 landing[3];
    u16 rpm;
} PACKED;

// blockcmd.c
بنية disk_op_s;
صحيح scsi_fill_cmd(بنية disk_op_s *op, فراغ *cdbcmd, صحيح maxcdb);
صحيح scsi_is_read(بنية disk_op_s *op);
صحيح scsi_is_ready(بنية disk_op_s *op);
بنية drive_s;
صحيح scsi_drive_تياري(بنية drive_s *drive, ثابت حرف *s, صحيح prio);
تعريفنوع صحيح (*scsi_جوڙيو_lun)(u32 lun, بنية drive_s *tmpl_drv);
صحيح scsi_rep_luns_جاچ(بنية drive_s *tmp_drive, scsi_جوڙيو_lun جوڙيو_lun);
صحيح scsi_sequential_جاچ(بنية drive_s *tmp_drive, u32 maxluns,
                         scsi_جوڙيو_lun جوڙيو_lun);

#جيڪڏهن_پورو // blockcmd.h

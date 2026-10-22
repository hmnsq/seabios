// Definitions for SCSI style command data blocks.
#که_نه_تعریف_شوی __BLOCKCMD_H
#تعریف_کړه __BLOCKCMD_H

#ضمن "types.h" // u8

#تعریف_کړه CDB_CMD_READ_10 0x28
#تعریف_کړه CDB_CMD_VERIFY_10 0x2f
#تعریف_کړه CDB_CMD_WRITE_10 0x2a

بنية cdb_rwdata_10 {
    u8 command;
    u8 flags;
    u32 lba;
    u8 resreved_06;
    u16 شمېر;
    u8 reserved_09;
    u8 pad[6];
} PACKED;

#تعریف_کړه CDB_CMD_READ_16 0x88
#تعریف_کړه CDB_CMD_WRITE_16 0x8A
بنية cdb_rwdata_16 {
    u8 command;
    u8 flags;
    u64 lba;
    u32 شمېر;
    u16 reserved_14;
} PACKED;

#تعریف_کړه CDB_CMD_READ_ظرفیت 0x25

بنية cdb_read_ظرفیت {
    u8 command;
    u8 flags;
    u8 resreved_02[8];
    u8 pad[6];
} PACKED;

بنية cdbres_read_ظرفیت {
    u32 sectors;
    u32 blksize;
} PACKED;


#تعریف_کړه CDB_CMD_SERVICE_ACTION_دننه 0x9E
#تعریف_کړه CDB_CMD_SAI_READ_ظرفیت_16 0x10
بنية cdb_sai_read_ظرفیت_16 {
    u8 command;
    u8 flags;
    u64 lba; //marked as obsolete?
    u32 len;
    u16 reserved_14;
} PACKED;

بنية cdbres_read_ظرفیت_16 {
    u64 sectors;
    u32 blksize;
    u8 reserved_12[20];
} PACKED;

#تعریف_کړه CDB_CMD_TEST_UNIT_READY  0x00
#تعریف_کړه CDB_CMD_INQUIRY          0x12
#تعریف_کړه CDB_CMD_REQUEST_SENSE    0x03

بنية cdb_request_sense {
    u8 command;
    u8 flags;
    u16 reserved_02;
    u8 اوږدتیا;
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

#تعریف_کړه SCSI_TYPE_DISK  0x00
#تعریف_کړه SCSI_TYPE_CDROM 0x05

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

#تعریف_کړه CDB_CMD_ډېرپېښ_SENSE    0x5A
#تعریف_کړه ډېرپېښ_پاڼه_HD_GEOMETRY 0x04

بنية cdb_ډېرپېښ_sense {
    u8 command;
    u8 flags;
    u8 پاڼه;
    u32 reserved_03;
    u16 شمېر;
    u8 reserved_09;
    u8 pad[6];
} PACKED;

بنية cdbres_ډېرپېښ_sense_geom {
    u8 ناکارول_شوی_00[3];
    u8 read_only;
    u32 ناکارول_شوی_04;
    u8 پاڼه;
    u8 اوږدتیا;
    u8 cyl[3];
    u8 heads;
    u8 precomp[3];
    u8 reduced[3];
    u16 ګام_شرح;
    u8 landing[3];
    u16 rpm;
} PACKED;

// blockcmd.c
بنية disk_op_s;
صحيح scsi_fill_cmd(بنية disk_op_s *op, فراغ *cdbcmd, صحيح maxcdb);
صحيح scsi_is_read(بنية disk_op_s *op);
صحيح scsi_is_ready(بنية disk_op_s *op);
بنية drive_s;
صحيح scsi_drive_تیاری(بنية drive_s *drive, ثابت حرف *s, صحيح prio);
تعريفنوع صحيح (*scsi_جمع_کړه_lun)(u32 lun, بنية drive_s *tmpl_drv);
صحيح scsi_rep_luns_لټون(بنية drive_s *tmp_drive, scsi_جمع_کړه_lun جمع_کړه_lun);
صحيح scsi_sequential_لټون(بنية drive_s *tmp_drive, u32 maxluns,
                         scsi_جمع_کړه_lun جمع_کړه_lun);

#د_که_پای // blockcmd.h

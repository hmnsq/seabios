// Definitions for SCSI style command data blocks.
#ifndef __BLOCKCMD_H
#define __BLOCKCMD_H

#അന്തര്ഭാവ "types.h" // u8

#define CDB_CMD_READ_10 0x28
#define CDB_CMD_VERIFY_10 0x2f
#define CDB_CMD_WRITE_10 0x2a

ഘടന cdb_rwdata_10 {
    u8 command;
    u8 flags;
    u32 lba;
    u8 resreved_06;
    u16 എണ്ണം;
    u8 reserved_09;
    u8 pad[6];
} PACKED;

#define CDB_CMD_READ_16 0x88
#define CDB_CMD_WRITE_16 0x8A
ഘടന cdb_rwdata_16 {
    u8 command;
    u8 flags;
    u64 lba;
    u32 എണ്ണം;
    u16 reserved_14;
} PACKED;

#define CDB_CMD_READ_CAPACITY 0x25

ഘടന cdb_read_capacity {
    u8 command;
    u8 flags;
    u8 resreved_02[8];
    u8 pad[6];
} PACKED;

ഘടന cdbres_read_capacity {
    u32 sectors;
    u32 blksize;
} PACKED;


#define CDB_CMD_SERVICE_ACTION_IN 0x9E
#define CDB_CMD_SAI_READ_CAPACITY_16 0x10
ഘടന cdb_sai_read_capacity_16 {
    u8 command;
    u8 flags;
    u64 lba; //marked as obsolete?
    u32 len;
    u16 reserved_14;
} PACKED;

ഘടന cdbres_read_capacity_16 {
    u64 sectors;
    u32 blksize;
    u8 reserved_12[20];
} PACKED;

#define CDB_CMD_TEST_UNIT_READY  0x00
#define CDB_CMD_INQUIRY          0x12
#define CDB_CMD_REQUEST_SENSE    0x03

ഘടന cdb_request_sense {
    u8 command;
    u8 flags;
    u16 reserved_02;
    u8 length;
    u8 reserved_05;
    u8 pad[10];
} PACKED;

ഘടന cdbres_request_sense {
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

#define SCSI_TYPE_DISK  0x00
#define SCSI_TYPE_CDROM 0x05

ഘടന cdbres_inquiry {
    u8 pdt;
    u8 removable;
    u8 reserved_02[2];
    u8 additional;
    u8 reserved_05[3];
    അക്ഷരം vendor[8];
    അക്ഷരം product[16];
    അക്ഷരം rev[4];
} PACKED;

#define CDB_CMD_MODE_SENSE    0x5A
#define MODE_PAGE_HD_GEOMETRY 0x04

ഘടന cdb_mode_sense {
    u8 command;
    u8 flags;
    u8 page;
    u32 reserved_03;
    u16 എണ്ണം;
    u8 reserved_09;
    u8 pad[6];
} PACKED;

ഘടന cdbres_mode_sense_geom {
    u8 unused_00[3];
    u8 read_only;
    u32 unused_04;
    u8 page;
    u8 length;
    u8 cyl[3];
    u8 heads;
    u8 precomp[3];
    u8 reduced[3];
    u16 step_rate;
    u8 landing[3];
    u16 rpm;
} PACKED;

// blockcmd.c
ഘടന disk_op_s;
പൂർണ്ണസംഖ്യ scsi_fill_cmd(ഘടന disk_op_s *op, ശൂന്യം *cdbcmd, പൂർണ്ണസംഖ്യ maxcdb);
പൂർണ്ണസംഖ്യ scsi_is_read(ഘടന disk_op_s *op);
പൂർണ്ണസംഖ്യ scsi_is_ready(ഘടന disk_op_s *op);
ഘടന drive_s;
പൂർണ്ണസംഖ്യ scsi_drive_setup(ഘടന drive_s *drive, സ്ഥിരം അക്ഷരം *s, പൂർണ്ണസംഖ്യ prio);
തരനിർവചനം പൂർണ്ണസംഖ്യ (*scsi_add_lun)(u32 lun, ഘടന drive_s *tmpl_drv);
പൂർണ്ണസംഖ്യ scsi_rep_luns_scan(ഘടന drive_s *tmp_drive, scsi_add_lun add_lun);
പൂർണ്ണസംഖ്യ scsi_sequential_scan(ഘടന drive_s *tmp_drive, u32 maxluns,
                         scsi_add_lun add_lun);

#endif // blockcmd.h

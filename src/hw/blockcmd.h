// Definitions for SCSI style command data blocks.
#अगर_अघोषित __BLOCKCMD_H
#घोषणा __BLOCKCMD_H

#समावेश "types.h" // u8

#घोषणा CDB_CMD_READ_10 0x28
#घोषणा CDB_CMD_VERIFY_10 0x2f
#घोषणा CDB_CMD_WRITE_10 0x2a

काष्ठा cdb_rwdata_10 {
    u8 command;
    u8 flags;
    u32 lba;
    u8 resreved_06;
    u16 गिनती;
    u8 reserved_09;
    u8 pad[6];
} PACKED;

#घोषणा CDB_CMD_READ_16 0x88
#घोषणा CDB_CMD_WRITE_16 0x8A
काष्ठा cdb_rwdata_16 {
    u8 command;
    u8 flags;
    u64 lba;
    u32 गिनती;
    u16 reserved_14;
} PACKED;

#घोषणा CDB_CMD_READ_CAPACITY 0x25

काष्ठा cdb_read_capacity {
    u8 command;
    u8 flags;
    u8 resreved_02[8];
    u8 pad[6];
} PACKED;

काष्ठा cdbres_read_capacity {
    u32 sectors;
    u32 blksize;
} PACKED;


#घोषणा CDB_CMD_SERVICE_ACTION_IN 0x9E
#घोषणा CDB_CMD_SAI_READ_CAPACITY_16 0x10
काष्ठा cdb_sai_read_capacity_16 {
    u8 command;
    u8 flags;
    u64 lba; //marked as obsolete?
    u32 len;
    u16 reserved_14;
} PACKED;

काष्ठा cdbres_read_capacity_16 {
    u64 sectors;
    u32 blksize;
    u8 reserved_12[20];
} PACKED;

#घोषणा CDB_CMD_TEST_UNIT_READY  0x00
#घोषणा CDB_CMD_INQUIRY          0x12
#घोषणा CDB_CMD_REQUEST_SENSE    0x03

काष्ठा cdb_request_sense {
    u8 command;
    u8 flags;
    u16 reserved_02;
    u8 लंबाई;
    u8 reserved_05;
    u8 pad[10];
} PACKED;

काष्ठा cdbres_request_sense {
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

#घोषणा SCSI_TYPE_DISK  0x00
#घोषणा SCSI_TYPE_CDROM 0x05

काष्ठा cdbres_inquiry {
    u8 pdt;
    u8 removable;
    u8 reserved_02[2];
    u8 additional;
    u8 reserved_05[3];
    अक्षर vendor[8];
    अक्षर product[16];
    अक्षर rev[4];
} PACKED;

#घोषणा CDB_CMD_MODE_SENSE    0x5A
#घोषणा MODE_PAGE_HD_GEOMETRY 0x04

काष्ठा cdb_mode_sense {
    u8 command;
    u8 flags;
    u8 page;
    u32 reserved_03;
    u16 गिनती;
    u8 reserved_09;
    u8 pad[6];
} PACKED;

काष्ठा cdbres_mode_sense_geom {
    u8 unused_00[3];
    u8 read_only;
    u32 unused_04;
    u8 page;
    u8 लंबाई;
    u8 cyl[3];
    u8 heads;
    u8 precomp[3];
    u8 reduced[3];
    u16 step_rate;
    u8 landing[3];
    u16 rpm;
} PACKED;

// blockcmd.c
काष्ठा disk_op_s;
पूर्णांक scsi_fill_cmd(काष्ठा disk_op_s *op, व्योम *cdbcmd, पूर्णांक maxcdb);
पूर्णांक scsi_is_read(काष्ठा disk_op_s *op);
पूर्णांक scsi_is_ready(काष्ठा disk_op_s *op);
काष्ठा drive_s;
पूर्णांक scsi_drive_setup(काष्ठा drive_s *drive, स्थिर अक्षर *s, पूर्णांक prio);
प्रकार पूर्णांक (*scsi_add_lun)(u32 lun, काष्ठा drive_s *tmpl_drv);
पूर्णांक scsi_rep_luns_scan(काष्ठा drive_s *tmp_drive, scsi_add_lun add_lun);
पूर्णांक scsi_sequential_scan(काष्ठा drive_s *tmp_drive, u32 maxluns,
                         scsi_add_lun add_lun);

#पूर्ण_अगर // blockcmd.h

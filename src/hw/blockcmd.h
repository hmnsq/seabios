// Definitions for SCSI style command data blocks.
#నిర్వచితం_కాకపోతే __BLOCKCMD_H
#నిర్వచించు __BLOCKCMD_H

#అన్తర్భావ "types.h" // u8

#నిర్వచించు CDB_CMD_READ_10 0x28
#నిర్వచించు CDB_CMD_VERIFY_10 0x2f
#నిర్వచించు CDB_CMD_WRITE_10 0x2a

నిర్మాణం cdb_rwdata_10 {
    u8 command;
    u8 flags;
    u32 lba;
    u8 resreved_06;
    u16 లెక్క;
    u8 reserved_09;
    u8 pad[6];
} PACKED;

#నిర్వచించు CDB_CMD_READ_16 0x88
#నిర్వచించు CDB_CMD_WRITE_16 0x8A
నిర్మాణం cdb_rwdata_16 {
    u8 command;
    u8 flags;
    u64 lba;
    u32 లెక్క;
    u16 reserved_14;
} PACKED;

#నిర్వచించు CDB_CMD_READ_సామర్థ్యం 0x25

నిర్మాణం cdb_read_సామర్థ్యం {
    u8 command;
    u8 flags;
    u8 resreved_02[8];
    u8 pad[6];
} PACKED;

నిర్మాణం cdbres_read_సామర్థ్యం {
    u32 sectors;
    u32 blksize;
} PACKED;


#నిర్వచించు CDB_CMD_SERVICE_ACTION_లోపల 0x9E
#నిర్వచించు CDB_CMD_SAI_READ_సామర్థ్యం_16 0x10
నిర్మాణం cdb_sai_read_సామర్థ్యం_16 {
    u8 command;
    u8 flags;
    u64 lba; //marked as obsolete?
    u32 len;
    u16 reserved_14;
} PACKED;

నిర్మాణం cdbres_read_సామర్థ్యం_16 {
    u64 sectors;
    u32 blksize;
    u8 reserved_12[20];
} PACKED;

#నిర్వచించు CDB_CMD_TEST_UNIT_READY  0x00
#నిర్వచించు CDB_CMD_INQUIRY          0x12
#నిర్వచించు CDB_CMD_REQUEST_SENSE    0x03

నిర్మాణం cdb_request_sense {
    u8 command;
    u8 flags;
    u16 reserved_02;
    u8 నిడివి;
    u8 reserved_05;
    u8 pad[10];
} PACKED;

నిర్మాణం cdbres_request_sense {
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

#నిర్వచించు SCSI_TYPE_DISK  0x00
#నిర్వచించు SCSI_TYPE_CDROM 0x05

నిర్మాణం cdbres_inquiry {
    u8 pdt;
    u8 removable;
    u8 reserved_02[2];
    u8 additional;
    u8 reserved_05[3];
    అక్షరం vendor[8];
    అక్షరం లబ్ధం[16];
    అక్షరం rev[4];
} PACKED;

#నిర్వచించు CDB_CMD_బహుళకం_SENSE    0x5A
#నిర్వచించు బహుళకం_పుట_HD_GEOMETRY 0x04

నిర్మాణం cdb_బహుళకం_sense {
    u8 command;
    u8 flags;
    u8 పుట;
    u32 reserved_03;
    u16 లెక్క;
    u8 reserved_09;
    u8 pad[6];
} PACKED;

నిర్మాణం cdbres_బహుళకం_sense_geom {
    u8 వాడని_00[3];
    u8 read_only;
    u32 వాడని_04;
    u8 పుట;
    u8 నిడివి;
    u8 cyl[3];
    u8 heads;
    u8 precomp[3];
    u8 reduced[3];
    u16 అడుగు_రేటు;
    u8 landing[3];
    u16 rpm;
} PACKED;

// blockcmd.c
నిర్మాణం disk_op_s;
పూర్ణాంకం scsi_fill_cmd(నిర్మాణం disk_op_s *op, శూన్య *cdbcmd, పూర్ణాంకం maxcdb);
పూర్ణాంకం scsi_is_read(నిర్మాణం disk_op_s *op);
పూర్ణాంకం scsi_is_ready(నిర్మాణం disk_op_s *op);
నిర్మాణం drive_s;
పూర్ణాంకం scsi_drive_అమరిక(నిర్మాణం drive_s *drive, స్థిరరాశి అక్షరం *s, పూర్ణాంకం prio);
రకనిర్వచనం పూర్ణాంకం (*scsi_కూడు_lun)(u32 lun, నిర్మాణం drive_s *tmpl_drv);
పూర్ణాంకం scsi_rep_luns_పరిశీలన(నిర్మాణం drive_s *tmp_drive, scsi_కూడు_lun కూడు_lun);
పూర్ణాంకం scsi_sequential_పరిశీలన(నిర్మాణం drive_s *tmp_drive, u32 maxluns,
                         scsi_కూడు_lun కూడు_lun);

#అయితే_ముగింపు // blockcmd.h

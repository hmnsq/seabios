// Definitions for SCSI style command data blocks.
#यदि_अपरिभाषितम् __BLOCKCMD_H
#परिभाषय __BLOCKCMD_H

#अन्तर्भाव "types.h" // u8

#परिभाषय CDB_CMD_READ_10 0x28
#परिभाषय CDB_CMD_VERIFY_10 0x2f
#परिभाषय CDB_CMD_WRITE_10 0x2a

संरचना cdb_rwdata_10 {
    u8 command;
    u8 flags;
    u32 lba;
    u8 resreved_06;
    u16 गणनम्;
    u8 reserved_09;
    u8 pad[6];
} PACKED;

#परिभाषय CDB_CMD_READ_16 0x88
#परिभाषय CDB_CMD_WRITE_16 0x8A
संरचना cdb_rwdata_16 {
    u8 command;
    u8 flags;
    u64 lba;
    u32 गणनम्;
    u16 reserved_14;
} PACKED;

#परिभाषय CDB_CMD_READ_क्षमता 0x25

संरचना cdb_read_क्षमता {
    u8 command;
    u8 flags;
    u8 resreved_02[8];
    u8 pad[6];
} PACKED;

संरचना cdbres_read_क्षमता {
    u32 sectors;
    u32 blksize;
} PACKED;


#परिभाषय CDB_CMD_SERVICE_ACTION_अन्तर् 0x9E
#परिभाषय CDB_CMD_SAI_READ_क्षमता_16 0x10
संरचना cdb_sai_read_क्षमता_16 {
    u8 command;
    u8 flags;
    u64 lba; //marked as obsolete?
    u32 len;
    u16 reserved_14;
} PACKED;

संरचना cdbres_read_क्षमता_16 {
    u64 sectors;
    u32 blksize;
    u8 reserved_12[20];
} PACKED;

#परिभाषय CDB_CMD_TEST_UNIT_READY  0x00
#परिभाषय CDB_CMD_INQUIRY          0x12
#परिभाषय CDB_CMD_REQUEST_SENSE    0x03

संरचना cdb_request_sense {
    u8 command;
    u8 flags;
    u16 reserved_02;
    u8 दीर्घता;
    u8 reserved_05;
    u8 pad[10];
} PACKED;

संरचना cdbres_request_sense {
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

#परिभाषय SCSI_TYPE_DISK  0x00
#परिभाषय SCSI_TYPE_CDROM 0x05

संरचना cdbres_inquiry {
    u8 pdt;
    u8 removable;
    u8 reserved_02[2];
    u8 additional;
    u8 reserved_05[3];
    वर्ण vendor[8];
    वर्ण गुणनफलम्[16];
    वर्ण rev[4];
} PACKED;

#परिभाषय CDB_CMD_बहुलकम्_SENSE    0x5A
#परिभाषय बहुलकम्_PAGE_HD_GEOMETRY 0x04

संरचना cdb_बहुलकम्_sense {
    u8 command;
    u8 flags;
    u8 page;
    u32 reserved_03;
    u16 गणनम्;
    u8 reserved_09;
    u8 pad[6];
} PACKED;

संरचना cdbres_बहुलकम्_sense_geom {
    u8 अप्रयुक्तम्_00[3];
    u8 read_only;
    u32 अप्रयुक्तम्_04;
    u8 page;
    u8 दीर्घता;
    u8 cyl[3];
    u8 heads;
    u8 precomp[3];
    u8 reduced[3];
    u16 चरणः_दरः;
    u8 landing[3];
    u16 rpm;
} PACKED;

// blockcmd.c
संरचना disk_op_s;
पूर्णाङ्क scsi_fill_cmd(संरचना disk_op_s *op, शून्य *cdbcmd, पूर्णाङ्क maxcdb);
पूर्णाङ्क scsi_is_read(संरचना disk_op_s *op);
पूर्णाङ्क scsi_is_ready(संरचना disk_op_s *op);
संरचना drive_s;
पूर्णाङ्क scsi_drive_सज्जता(संरचना drive_s *drive, ध्रुव वर्ण *s, पूर्णाङ्क prio);
प्रकारनिर्देश पूर्णाङ्क (*scsi_सङ्कलय_lun)(u32 lun, संरचना drive_s *tmpl_drv);
पूर्णाङ्क scsi_rep_luns_अवलोकनम्(संरचना drive_s *tmp_drive, scsi_सङ्कलय_lun सङ्कलय_lun);
पूर्णाङ्क scsi_sequential_अवलोकनम्(संरचना drive_s *tmp_drive, u32 maxluns,
                         scsi_सङ्कलय_lun सङ्कलय_lun);

#यदि_अन्तः // blockcmd.h

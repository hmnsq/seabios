// Support for several common scsi like command data block requests
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "block.h" // struct disk_op_s
#अन्तर्भाव "blockcmd.h" // struct cdb_request_sense
#अन्तर्भाव "byteorder.h" // be32_to_cpu
#अन्तर्भाव "farptr.h" // GET_FLATPTR
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "std/disk.h" // DISK_RET_EPARAM
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // timer_calc
#अन्तर्भाव "malloc.h"


/****************************************************************
 * Low level command requests
 ****************************************************************/

स्थिर पूर्णाङ्क
cdb_get_inquiry(संरचना disk_op_s *op, संरचना cdbres_inquiry *दत्त)
{
    संरचना cdb_request_sense cmd;
    स्मृतिं_पूरय(&cmd, 0, परिमाण(cmd));
    cmd.command = CDB_CMD_INQUIRY;
    cmd.दीर्घता = परिमाण(*दत्त);
    op->command = CMD_SCSI;
    op->गणनम् = 1;
    op->buf_fl = दत्त;
    op->cdbcmd = &cmd;
    op->blocksize = परिमाण(*दत्त);
    प्रत्यागम process_op(op);
}

// Request SENSE
स्थिर पूर्णाङ्क
cdb_get_sense(संरचना disk_op_s *op, संरचना cdbres_request_sense *दत्त)
{
    संरचना cdb_request_sense cmd;
    स्मृतिं_पूरय(&cmd, 0, परिमाण(cmd));
    cmd.command = CDB_CMD_REQUEST_SENSE;
    cmd.दीर्घता = परिमाण(*दत्त);
    op->command = CMD_SCSI;
    op->गणनम् = 1;
    op->buf_fl = दत्त;
    op->cdbcmd = &cmd;
    op->blocksize = परिमाण(*दत्त);
    प्रत्यागम process_op(op);
}

// Test unit ready
स्थिर पूर्णाङ्क
cdb_test_unit_ready(संरचना disk_op_s *op)
{
    संरचना cdb_request_sense cmd;
    स्मृतिं_पूरय(&cmd, 0, परिमाण(cmd));
    cmd.command = CDB_CMD_TEST_UNIT_READY;
    op->command = CMD_SCSI;
    op->गणनम् = 0;
    op->buf_fl = रिक्त;
    op->cdbcmd = &cmd;
    op->blocksize = 0;
    प्रत्यागम process_op(op);
}

स्थिर पूर्णाङ्क
cdb_read_capacity16(संरचना disk_op_s *op, संरचना cdbres_read_क्षमता_16 *दत्त)
{
    संरचना cdb_sai_read_क्षमता_16 cmd;
    स्मृतिं_पूरय(&cmd, 0, परिमाण(cmd));
    cmd.command = CDB_CMD_SERVICE_ACTION_अन्तर्;
    cmd.flags = CDB_CMD_SAI_READ_क्षमता_16;
    cmd.len = cpu_गन्तव्यम्_be32(परिमाण(संरचना cdbres_read_क्षमता_16));
    op->command = CMD_SCSI;
    op->गणनम् = 1;
    op->buf_fl = दत्त;
    op->cdbcmd = &cmd;
    op->blocksize = परिमाण(*दत्त);
    प्रत्यागम process_op(op);
}


// Request capacity
स्थिर पूर्णाङ्क
cdb_read_क्षमता(संरचना disk_op_s *op, संरचना cdbres_read_क्षमता *दत्त)
{
    संरचना cdb_read_क्षमता cmd;
    स्मृतिं_पूरय(&cmd, 0, परिमाण(cmd));
    cmd.command = CDB_CMD_READ_क्षमता;
    op->command = CMD_SCSI;
    op->गणनम् = 1;
    op->buf_fl = दत्त;
    op->cdbcmd = &cmd;
    op->blocksize = परिमाण(*दत्त);
    प्रत्यागम process_op(op);
}

// Mode sense, geometry page.
स्थिर पूर्णाङ्क
cdb_बहुलकम्_sense_geom(संरचना disk_op_s *op, संरचना cdbres_बहुलकम्_sense_geom *दत्त)
{
    संरचना cdb_बहुलकम्_sense cmd;
    स्मृतिं_पूरय(&cmd, 0, परिमाण(cmd));
    cmd.command = CDB_CMD_बहुलकम्_SENSE;
    cmd.flags = 8; /* DBD */
    cmd.page = बहुलकम्_PAGE_HD_GEOMETRY;
    cmd.गणनम् = cpu_गन्तव्यम्_be16(परिमाण(*दत्त));
    op->command = CMD_SCSI;
    op->गणनम् = 1;
    op->buf_fl = दत्त;
    op->cdbcmd = &cmd;
    op->blocksize = परिमाण(*दत्त);
    प्रत्यागम process_op(op);
}


/****************************************************************
 * Main SCSI commands
 ****************************************************************/

// Create a scsi command request from a disk_op_s request
पूर्णाङ्क
scsi_fill_cmd(संरचना disk_op_s *op, शून्य *cdbcmd, पूर्णाङ्क maxcdb)
{
    विकल्प (op->command) {
    पक्ष CMD_READ:
    पक्ष CMD_WRITE: ;
        यदि (op->lba < 0xFFFFFFFFULL) {
            संरचना cdb_rwdata_10 *cmd = cdbcmd;
            स्मृतिं_पूरय(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_10
                            : CDB_CMD_WRITE_10);
            cmd->lba = cpu_गन्तव्यम्_be32(op->lba);
            cmd->गणनम् = cpu_गन्तव्यम्_be16(op->गणनम्);
        } अन्यथा {
            संरचना cdb_rwdata_16 *cmd = cdbcmd;
            स्मृतिं_पूरय(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_16
                            : CDB_CMD_WRITE_16);
            cmd->lba = cpu_गन्तव्यम्_be64(op->lba);
            cmd->गणनम् = cpu_गन्तव्यम्_be32(op->गणनम्);
        }
        प्रत्यागम GET_FLATPTR(op->drive_fl->blksize);
    पक्ष CMD_SCSI:
        यदि (MODESEGMENT)
            प्रत्यागम -1;
        स्मृतिं_प्रतिलिख(cdbcmd, op->cdbcmd, maxcdb);
        प्रत्यागम op->blocksize;
    स्वभाव:
        प्रत्यागम -1;
    }
}

// Determine if the command is a request to pull data from the device
पूर्णाङ्क
scsi_is_read(संरचना disk_op_s *op)
{
    प्रत्यागम op->command == CMD_READ || (
        !MODESEGMENT && op->command == CMD_SCSI && op->blocksize);
}

// Check if a SCSI device is ready to receive commands
पूर्णाङ्क
scsi_is_ready(संरचना disk_op_s *op)
{
    ASSERT32FLAT();
    dprintf(6, "scsi_is_ready (drive=%p)\n", op->drive_fl);

    /* Retry TEST UNIT READY for 5 seconds unless MEDIUM NOT PRESENT is
     * reported by the device 3 times.  If the device reports "IN PROGRESS",
     * 30 seconds is added. */
    पूर्णाङ्क tries = 3;
    पूर्णाङ्क अन्तर्_प्रगतिः = 0;
    u32 अन्तः = timer_calc(5000);
    प्रति (;;) {
        यदि (timer_check(अन्तः)) {
            dprintf(1, "test unit ready failed\n");
            प्रत्यागम -1;
        }

        पूर्णाङ्क ret = cdb_test_unit_ready(op);
        यदि (!ret)
            // Success
            भङ्ग;

        संरचना cdbres_request_sense sense;
        ret = cdb_get_sense(op, &sense);
        यदि (ret)
            // Error - retry.
            अनुवर्त;

        // Sense succeeded.
        यदि (sense.asc == 0x3a) { /* MEDIUM NOT PRESENT */
            tries--;
            dprintf(1, "Device reports MEDIUM NOT PRESENT - %d tries left\n",
                tries);
            यदि (!tries)
                प्रत्यागम -1;
        }

        यदि (sense.asc == 0x04 && sense.ascq == 0x01 && !अन्तर्_प्रगतिः) {
            /* IN PROGRESS OF BECOMING READY */
            dprintf(1, "Waiting for device to detect medium... ");
            /* Allow 30 seconds more */
            अन्तः = timer_calc(30000);
            अन्तर्_प्रगतिः = 1;
        }
    }
    प्रत्यागम 0;
}

#परिभाषय CDB_CMD_REPORT_LUNS  0xA0

संरचना cdb_report_luns {
    u8 command;
    u8 reserved_01[5];
    u32 दीर्घता;
    u8 pad[6];
} PACKED;

संरचना scsi_lun {
    u16 lun[4];
};

संरचना cdbres_report_luns {
    u32 दीर्घता;
    u32 reserved;
    संरचना scsi_lun luns[];
};

स्थिर u64 scsilun2u64(संरचना scsi_lun *scsi_lun)
{
    पूर्णाङ्क i;
    u64 ret = 0;
    प्रति (i = 0; i < ARRAY_आकार(scsi_lun->lun); i++)
        ret |= be16_गन्तव्यम्_cpu(scsi_lun->lun[i]) << (16 * i);
    प्रत्यागम ret;
}

// Issue REPORT LUNS on a temporary drive and iterate reported luns calling
// @add_lun for each
पूर्णाङ्क scsi_rep_luns_अवलोकनम्(संरचना drive_s *tmp_drive, scsi_सङ्कलय_lun सङ्कलय_lun)
{
    पूर्णाङ्क ret = -1;
    /* start with the smallest possible buffer, otherwise some devices in QEMU
     * may (incorrectly) error out on returning less data than fits in it */
    u32 maxluns = 1;
    u32 nluns, i;
    संरचना cdb_report_luns cdb = {
        .command = CDB_CMD_REPORT_LUNS,
    };
    संरचना disk_op_s op = {
        .drive_fl = tmp_drive,
        .command = CMD_SCSI,
        .गणनम् = 1,
        .cdbcmd = &cdb,
    };
    संरचना cdbres_report_luns *resp;

    ASSERT32FLAT();

    यावत् (1) {
        op.blocksize = परिमाण(संरचना cdbres_report_luns) +
            maxluns * परिमाण(संरचना scsi_lun);
        op.buf_fl = malloc_tmp(op.blocksize);
        यदि (!op.buf_fl) {
            warn_noalloc();
            प्रत्यागम -1;
        }

        cdb.दीर्घता = cpu_गन्तव्यम्_be32(op.blocksize);
        यदि (process_op(&op) != DISK_RET_SUCCESS)
            गच्छ बहिर्गमनम्;

        resp = op.buf_fl;
        nluns = be32_गन्तव्यम्_cpu(resp->दीर्घता) / परिमाण(संरचना scsi_lun);
        यदि (nluns <= maxluns)
            भङ्ग;

        मुक्त(op.buf_fl);
        maxluns = nluns;
    }

    प्रति (i = 0, ret = 0; i < nluns; i++) {
        u64 lun = scsilun2u64(&resp->luns[i]);
        यदि (lun >> 32)
            अनुवर्त;
        ret += !सङ्कलय_lun((u32)lun, tmp_drive);
    }
बहिर्गमनम्:
    मुक्त(op.buf_fl);
    प्रत्यागम ret;
}

// Iterate LUNs on the target and call @add_lun for each
पूर्णाङ्क scsi_sequential_अवलोकनम्(संरचना drive_s *tmp_drive, u32 maxluns,
                         scsi_सङ्कलय_lun सङ्कलय_lun)
{
    पूर्णाङ्क ret;
    u32 lun;

    प्रति (lun = 0, ret = 0; lun < maxluns; lun++)
        ret += !सङ्कलय_lun(lun, tmp_drive);
    प्रत्यागम ret;
}

// Validate drive, find block size / sector count, and register drive.
पूर्णाङ्क
scsi_drive_सज्जता(संरचना drive_s *drive, ध्रुव वर्ण *s, पूर्णाङ्क prio)
{
    ASSERT32FLAT();
    संरचना disk_op_s dop;
    स्मृतिं_पूरय(&dop, 0, परिमाण(dop));
    dop.drive_fl = drive;
    संरचना cdbres_inquiry दत्त;
    पूर्णाङ्क ret = cdb_get_inquiry(&dop, &दत्त);
    यदि (ret)
        प्रत्यागम ret;
    वर्ण vendor[परिमाण(दत्त.vendor)+1], गुणनफलम्[परिमाण(दत्त.गुणनफलम्)+1];
    वर्ण rev[परिमाण(दत्त.rev)+1];
    strtcpy(vendor, दत्त.vendor, परिमाण(vendor));
    nullTrailingSpace(vendor);
    strtcpy(गुणनफलम्, दत्त.गुणनफलम्, परिमाण(गुणनफलम्));
    nullTrailingSpace(गुणनफलम्);
    strtcpy(rev, दत्त.rev, परिमाण(rev));
    nullTrailingSpace(rev);
    पूर्णाङ्क pdt = दत्त.pdt & 0x1f;
    पूर्णाङ्क removable = !!(दत्त.removable & 0x80);
    dprintf(1, "%s vendor='%s' product='%s' rev='%s' type=%d removable=%d\n"
            , s, vendor, गुणनफलम्, rev, pdt, removable);
    drive->removable = removable;

    यदि (pdt == SCSI_TYPE_CDROM) {
        drive->blksize = CDROM_SECTOR_आकार;
        drive->sectors = (u64)-1;

        वर्ण *desc = znprintf(MAXDESCSIZE, "DVD/CD [%s Drive %s %s %s]"
                              , s, vendor, गुणनफलम्, rev);
        boot_सङ्कलय_cd(drive, desc, prio);
        प्रत्यागम 0;
    }

    यदि (pdt != SCSI_TYPE_DISK)
        प्रत्यागम -1;

    ret = scsi_is_ready(&dop);
    यदि (ret) {
        dprintf(1, "scsi_is_ready returned %d\n", ret);
        प्रत्यागम ret;
    }

    संरचना cdbres_read_क्षमता capdata;
    ret = cdb_read_क्षमता(&dop, &capdata);
    यदि (ret)
        प्रत्यागम ret;

    यदि (be32_गन्तव्यम्_cpu(capdata.sectors) == 0xFFFFFFFFUL) {
        dprintf(3, "%s: >2TB Detected trying READCAP(16)\n", s);
        संरचना cdbres_read_क्षमता_16 capdata16;
        ret = cdb_read_capacity16(&dop, &capdata16);
        drive->blksize = be32_गन्तव्यम्_cpu(capdata16.blksize);
        drive->sectors = be64_गन्तव्यम्_cpu(capdata16.sectors) + 1;
    } अन्यथा {
        drive->blksize = be32_गन्तव्यम्_cpu(capdata.blksize);
        drive->sectors = (u64)be32_गन्तव्यम्_cpu(capdata.sectors) + 1;
    }

    यदि (drive->blksize != DISK_SECTOR_आकार) {
        dprintf(1, "%s: unsupported block size %d\n", s, drive->blksize);
        प्रत्यागम -1;
    }
    dprintf(1, "%s blksize=%d sectors=0x%llx\n", s, drive->blksize, drive->sectors);
    // We do not recover from USB stalls, so try to be safe and avoid
    // sending the command if the (obsolete, but still provided by QEMU)
    // fixed disk geometry page may not be supported.
    //
    // We could also send the command only to small disks (e.g. <504MiB)
    // but some old USB keys only support a very small subset of SCSI which
    // does not even include the MODE SENSE command!
    //
    यदि (CONFIG_QEMU_HARDWARE && memcmp(vendor, "QEMU", 5) == 0) {
        संरचना cdbres_बहुलकम्_sense_geom geomdata;
        ret = cdb_बहुलकम्_sense_geom(&dop, &geomdata);
        यदि (ret == 0) {
            u32 cylinders;
            cylinders = geomdata.cyl[0] << 16;
            cylinders |= geomdata.cyl[1] << 8;
            cylinders |= geomdata.cyl[2];
            यदि (cylinders && geomdata.heads &&
                drive->sectors <= 0xFFFFFFFFULL &&
                ((u32)drive->sectors % (geomdata.heads * cylinders) == 0)) {
                drive->pchs.cylinder = cylinders;
                drive->pchs.शीर्ष = geomdata.heads;
                drive->pchs.sector = (u32)drive->sectors / (geomdata.heads * cylinders);
            }
        }
    }

    वर्ण *desc = znprintf(MAXDESCSIZE, "%s Drive %s %s %s"
                          , s, vendor, गुणनफलम्, rev);
    boot_सङ्कलय_hd(drive, desc, prio);
    प्रत्यागम 0;
}

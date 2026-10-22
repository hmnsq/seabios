// Support for several common scsi like command data block requests
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "block.h" // struct disk_op_s
#అన్తర్భావ "blockcmd.h" // struct cdb_request_sense
#అన్తర్భావ "byteorder.h" // be32_to_cpu
#అన్తర్భావ "farptr.h" // GET_FLATPTR
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "std/disk.h" // DISK_RET_EPARAM
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // timer_calc
#అన్తర్భావ "malloc.h"


/****************************************************************
 * Low level command requests
 ****************************************************************/

స్థిర పూర్ణాంకం
cdb_get_inquiry(నిర్మాణం disk_op_s *op, నిర్మాణం cdbres_inquiry *దత్తాంశం)
{
    నిర్మాణం cdb_request_sense cmd;
    memset(&cmd, 0, పరిమాణం(cmd));
    cmd.command = CDB_CMD_INQUIRY;
    cmd.నిడివి = పరిమాణం(*దత్తాంశం);
    op->command = CMD_SCSI;
    op->లెక్క = 1;
    op->buf_fl = దత్తాంశం;
    op->cdbcmd = &cmd;
    op->blocksize = పరిమాణం(*దత్తాంశం);
    తిరిగి process_op(op);
}

// Request SENSE
స్థిర పూర్ణాంకం
cdb_get_sense(నిర్మాణం disk_op_s *op, నిర్మాణం cdbres_request_sense *దత్తాంశం)
{
    నిర్మాణం cdb_request_sense cmd;
    memset(&cmd, 0, పరిమాణం(cmd));
    cmd.command = CDB_CMD_REQUEST_SENSE;
    cmd.నిడివి = పరిమాణం(*దత్తాంశం);
    op->command = CMD_SCSI;
    op->లెక్క = 1;
    op->buf_fl = దత్తాంశం;
    op->cdbcmd = &cmd;
    op->blocksize = పరిమాణం(*దత్తాంశం);
    తిరిగి process_op(op);
}

// Test unit ready
స్థిర పూర్ణాంకం
cdb_test_unit_ready(నిర్మాణం disk_op_s *op)
{
    నిర్మాణం cdb_request_sense cmd;
    memset(&cmd, 0, పరిమాణం(cmd));
    cmd.command = CDB_CMD_TEST_UNIT_READY;
    op->command = CMD_SCSI;
    op->లెక్క = 0;
    op->buf_fl = ఖాళీ;
    op->cdbcmd = &cmd;
    op->blocksize = 0;
    తిరిగి process_op(op);
}

స్థిర పూర్ణాంకం
cdb_read_capacity16(నిర్మాణం disk_op_s *op, నిర్మాణం cdbres_read_సామర్థ్యం_16 *దత్తాంశం)
{
    నిర్మాణం cdb_sai_read_సామర్థ్యం_16 cmd;
    memset(&cmd, 0, పరిమాణం(cmd));
    cmd.command = CDB_CMD_SERVICE_ACTION_లోపల;
    cmd.flags = CDB_CMD_SAI_READ_సామర్థ్యం_16;
    cmd.len = cpu_గమ్యం_be32(పరిమాణం(నిర్మాణం cdbres_read_సామర్థ్యం_16));
    op->command = CMD_SCSI;
    op->లెక్క = 1;
    op->buf_fl = దత్తాంశం;
    op->cdbcmd = &cmd;
    op->blocksize = పరిమాణం(*దత్తాంశం);
    తిరిగి process_op(op);
}


// Request capacity
స్థిర పూర్ణాంకం
cdb_read_సామర్థ్యం(నిర్మాణం disk_op_s *op, నిర్మాణం cdbres_read_సామర్థ్యం *దత్తాంశం)
{
    నిర్మాణం cdb_read_సామర్థ్యం cmd;
    memset(&cmd, 0, పరిమాణం(cmd));
    cmd.command = CDB_CMD_READ_సామర్థ్యం;
    op->command = CMD_SCSI;
    op->లెక్క = 1;
    op->buf_fl = దత్తాంశం;
    op->cdbcmd = &cmd;
    op->blocksize = పరిమాణం(*దత్తాంశం);
    తిరిగి process_op(op);
}

// Mode sense, geometry page.
స్థిర పూర్ణాంకం
cdb_బహుళకం_sense_geom(నిర్మాణం disk_op_s *op, నిర్మాణం cdbres_బహుళకం_sense_geom *దత్తాంశం)
{
    నిర్మాణం cdb_బహుళకం_sense cmd;
    memset(&cmd, 0, పరిమాణం(cmd));
    cmd.command = CDB_CMD_బహుళకం_SENSE;
    cmd.flags = 8; /* DBD */
    cmd.పుట = బహుళకం_పుట_HD_GEOMETRY;
    cmd.లెక్క = cpu_గమ్యం_be16(పరిమాణం(*దత్తాంశం));
    op->command = CMD_SCSI;
    op->లెక్క = 1;
    op->buf_fl = దత్తాంశం;
    op->cdbcmd = &cmd;
    op->blocksize = పరిమాణం(*దత్తాంశం);
    తిరిగి process_op(op);
}


/****************************************************************
 * Main SCSI commands
 ****************************************************************/

// Create a scsi command request from a disk_op_s request
పూర్ణాంకం
scsi_fill_cmd(నిర్మాణం disk_op_s *op, శూన్య *cdbcmd, పూర్ణాంకం maxcdb)
{
    ఎంపిక (op->command) {
    స్థితి CMD_READ:
    స్థితి CMD_WRITE: ;
        ఒకవేళ (op->lba < 0xFFFFFFFFULL) {
            నిర్మాణం cdb_rwdata_10 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_10
                            : CDB_CMD_WRITE_10);
            cmd->lba = cpu_గమ్యం_be32(op->lba);
            cmd->లెక్క = cpu_గమ్యం_be16(op->లెక్క);
        } లేకపోతే {
            నిర్మాణం cdb_rwdata_16 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_16
                            : CDB_CMD_WRITE_16);
            cmd->lba = cpu_గమ్యం_be64(op->lba);
            cmd->లెక్క = cpu_గమ్యం_be32(op->లెక్క);
        }
        తిరిగి GET_FLATPTR(op->drive_fl->blksize);
    స్థితి CMD_SCSI:
        ఒకవేళ (MODESEGMENT)
            తిరిగి -1;
        జ్ఞాపకం_నకలు(cdbcmd, op->cdbcmd, maxcdb);
        తిరిగి op->blocksize;
    అప్రమేయ:
        తిరిగి -1;
    }
}

// Determine if the command is a request to pull data from the device
పూర్ణాంకం
scsi_is_read(నిర్మాణం disk_op_s *op)
{
    తిరిగి op->command == CMD_READ || (
        !MODESEGMENT && op->command == CMD_SCSI && op->blocksize);
}

// Check if a SCSI device is ready to receive commands
పూర్ణాంకం
scsi_is_ready(నిర్మాణం disk_op_s *op)
{
    ASSERT32FLAT();
    dprintf(6, "scsi_is_ready (drive=%p)\n", op->drive_fl);

    /* Retry TEST UNIT READY for 5 seconds unless MEDIUM NOT PRESENT is
     * reported by the device 3 times.  If the device reports "IN PROGRESS",
     * 30 seconds is added. */
    పూర్ణాంకం tries = 3;
    పూర్ణాంకం లోపల_పురోగతి = 0;
    u32 చివర = timer_calc(5000);
    కొరకు (;;) {
        ఒకవేళ (timer_check(చివర)) {
            dprintf(1, "test unit ready failed\n");
            తిరిగి -1;
        }

        పూర్ణాంకం ret = cdb_test_unit_ready(op);
        ఒకవేళ (!ret)
            // Success
            ఆపు;

        నిర్మాణం cdbres_request_sense sense;
        ret = cdb_get_sense(op, &sense);
        ఒకవేళ (ret)
            // Error - retry.
            కొనసాగు;

        // Sense succeeded.
        ఒకవేళ (sense.asc == 0x3a) { /* MEDIUM NOT PRESENT */
            tries--;
            dprintf(1, "Device reports MEDIUM NOT PRESENT - %d tries left\n",
                tries);
            ఒకవేళ (!tries)
                తిరిగి -1;
        }

        ఒకవేళ (sense.asc == 0x04 && sense.ascq == 0x01 && !లోపల_పురోగతి) {
            /* IN PROGRESS OF BECOMING READY */
            dprintf(1, "Waiting for device to detect medium... ");
            /* Allow 30 seconds more */
            చివర = timer_calc(30000);
            లోపల_పురోగతి = 1;
        }
    }
    తిరిగి 0;
}

#నిర్వచించు CDB_CMD_REPORT_LUNS  0xA0

నిర్మాణం cdb_report_luns {
    u8 command;
    u8 reserved_01[5];
    u32 నిడివి;
    u8 pad[6];
} PACKED;

నిర్మాణం scsi_lun {
    u16 lun[4];
};

నిర్మాణం cdbres_report_luns {
    u32 నిడివి;
    u32 reserved;
    నిర్మాణం scsi_lun luns[];
};

స్థిర u64 scsilun2u64(నిర్మాణం scsi_lun *scsi_lun)
{
    పూర్ణాంకం i;
    u64 ret = 0;
    కొరకు (i = 0; i < ARRAY_కొలత(scsi_lun->lun); i++)
        ret |= be16_గమ్యం_cpu(scsi_lun->lun[i]) << (16 * i);
    తిరిగి ret;
}

// Issue REPORT LUNS on a temporary drive and iterate reported luns calling
// @add_lun for each
పూర్ణాంకం scsi_rep_luns_పరిశీలన(నిర్మాణం drive_s *tmp_drive, scsi_కూడు_lun కూడు_lun)
{
    పూర్ణాంకం ret = -1;
    /* start with the smallest possible buffer, otherwise some devices in QEMU
     * may (incorrectly) error out on returning less data than fits in it */
    u32 maxluns = 1;
    u32 nluns, i;
    నిర్మాణం cdb_report_luns cdb = {
        .command = CDB_CMD_REPORT_LUNS,
    };
    నిర్మాణం disk_op_s op = {
        .drive_fl = tmp_drive,
        .command = CMD_SCSI,
        .లెక్క = 1,
        .cdbcmd = &cdb,
    };
    నిర్మాణం cdbres_report_luns *resp;

    ASSERT32FLAT();

    అయితే (1) {
        op.blocksize = పరిమాణం(నిర్మాణం cdbres_report_luns) +
            maxluns * పరిమాణం(నిర్మాణం scsi_lun);
        op.buf_fl = malloc_tmp(op.blocksize);
        ఒకవేళ (!op.buf_fl) {
            warn_noalloc();
            తిరిగి -1;
        }

        cdb.నిడివి = cpu_గమ్యం_be32(op.blocksize);
        ఒకవేళ (process_op(&op) != DISK_RET_SUCCESS)
            వెళ్ళు బహిర్గతం;

        resp = op.buf_fl;
        nluns = be32_గమ్యం_cpu(resp->నిడివి) / పరిమాణం(నిర్మాణం scsi_lun);
        ఒకవేళ (nluns <= maxluns)
            ఆపు;

        విడుదల(op.buf_fl);
        maxluns = nluns;
    }

    కొరకు (i = 0, ret = 0; i < nluns; i++) {
        u64 lun = scsilun2u64(&resp->luns[i]);
        ఒకవేళ (lun >> 32)
            కొనసాగు;
        ret += !కూడు_lun((u32)lun, tmp_drive);
    }
బహిర్గతం:
    విడుదల(op.buf_fl);
    తిరిగి ret;
}

// Iterate LUNs on the target and call @add_lun for each
పూర్ణాంకం scsi_sequential_పరిశీలన(నిర్మాణం drive_s *tmp_drive, u32 maxluns,
                         scsi_కూడు_lun కూడు_lun)
{
    పూర్ణాంకం ret;
    u32 lun;

    కొరకు (lun = 0, ret = 0; lun < maxluns; lun++)
        ret += !కూడు_lun(lun, tmp_drive);
    తిరిగి ret;
}

// Validate drive, find block size / sector count, and register drive.
పూర్ణాంకం
scsi_drive_అమరిక(నిర్మాణం drive_s *drive, స్థిరరాశి అక్షరం *s, పూర్ణాంకం prio)
{
    ASSERT32FLAT();
    నిర్మాణం disk_op_s dop;
    memset(&dop, 0, పరిమాణం(dop));
    dop.drive_fl = drive;
    నిర్మాణం cdbres_inquiry దత్తాంశం;
    పూర్ణాంకం ret = cdb_get_inquiry(&dop, &దత్తాంశం);
    ఒకవేళ (ret)
        తిరిగి ret;
    అక్షరం vendor[పరిమాణం(దత్తాంశం.vendor)+1], లబ్ధం[పరిమాణం(దత్తాంశం.లబ్ధం)+1];
    అక్షరం rev[పరిమాణం(దత్తాంశం.rev)+1];
    strtcpy(vendor, దత్తాంశం.vendor, పరిమాణం(vendor));
    nullTrailingSpace(vendor);
    strtcpy(లబ్ధం, దత్తాంశం.లబ్ధం, పరిమాణం(లబ్ధం));
    nullTrailingSpace(లబ్ధం);
    strtcpy(rev, దత్తాంశం.rev, పరిమాణం(rev));
    nullTrailingSpace(rev);
    పూర్ణాంకం pdt = దత్తాంశం.pdt & 0x1f;
    పూర్ణాంకం removable = !!(దత్తాంశం.removable & 0x80);
    dprintf(1, "%s vendor='%s' product='%s' rev='%s' type=%d removable=%d\n"
            , s, vendor, లబ్ధం, rev, pdt, removable);
    drive->removable = removable;

    ఒకవేళ (pdt == SCSI_TYPE_CDROM) {
        drive->blksize = CDROM_SECTOR_కొలత;
        drive->sectors = (u64)-1;

        అక్షరం *desc = znprintf(MAXDESCSIZE, "DVD/CD [%s Drive %s %s %s]"
                              , s, vendor, లబ్ధం, rev);
        boot_కూడు_cd(drive, desc, prio);
        తిరిగి 0;
    }

    ఒకవేళ (pdt != SCSI_TYPE_DISK)
        తిరిగి -1;

    ret = scsi_is_ready(&dop);
    ఒకవేళ (ret) {
        dprintf(1, "scsi_is_ready returned %d\n", ret);
        తిరిగి ret;
    }

    నిర్మాణం cdbres_read_సామర్థ్యం capdata;
    ret = cdb_read_సామర్థ్యం(&dop, &capdata);
    ఒకవేళ (ret)
        తిరిగి ret;

    ఒకవేళ (be32_గమ్యం_cpu(capdata.sectors) == 0xFFFFFFFFUL) {
        dprintf(3, "%s: >2TB Detected trying READCAP(16)\n", s);
        నిర్మాణం cdbres_read_సామర్థ్యం_16 capdata16;
        ret = cdb_read_capacity16(&dop, &capdata16);
        drive->blksize = be32_గమ్యం_cpu(capdata16.blksize);
        drive->sectors = be64_గమ్యం_cpu(capdata16.sectors) + 1;
    } లేకపోతే {
        drive->blksize = be32_గమ్యం_cpu(capdata.blksize);
        drive->sectors = (u64)be32_గమ్యం_cpu(capdata.sectors) + 1;
    }

    ఒకవేళ (drive->blksize != DISK_SECTOR_కొలత) {
        dprintf(1, "%s: unsupported block size %d\n", s, drive->blksize);
        తిరిగి -1;
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
    ఒకవేళ (CONFIG_QEMU_HARDWARE && memcmp(vendor, "QEMU", 5) == 0) {
        నిర్మాణం cdbres_బహుళకం_sense_geom geomdata;
        ret = cdb_బహుళకం_sense_geom(&dop, &geomdata);
        ఒకవేళ (ret == 0) {
            u32 cylinders;
            cylinders = geomdata.cyl[0] << 16;
            cylinders |= geomdata.cyl[1] << 8;
            cylinders |= geomdata.cyl[2];
            ఒకవేళ (cylinders && geomdata.heads &&
                drive->sectors <= 0xFFFFFFFFULL &&
                ((u32)drive->sectors % (geomdata.heads * cylinders) == 0)) {
                drive->pchs.cylinder = cylinders;
                drive->pchs.తల = geomdata.heads;
                drive->pchs.sector = (u32)drive->sectors / (geomdata.heads * cylinders);
            }
        }
    }

    అక్షరం *desc = znprintf(MAXDESCSIZE, "%s Drive %s %s %s"
                          , s, vendor, లబ్ధం, rev);
    boot_కూడు_hd(drive, desc, prio);
    తిరిగి 0;
}

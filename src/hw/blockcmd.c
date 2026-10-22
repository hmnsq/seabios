// Support for several common scsi like command data block requests
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "block.h" // struct disk_op_s
#شامل "blockcmd.h" // struct cdb_request_sense
#شامل "byteorder.h" // be32_to_cpu
#شامل "farptr.h" // GET_FLATPTR
#شامل "output.h" // dprintf
#شامل "std/disk.h" // DISK_RET_EPARAM
#شامل "string.h" // memset
#شامل "util.h" // timer_calc
#شامل "malloc.h"


/****************************************************************
 * Low level command requests
 ****************************************************************/

ساکن صحیح
cdb_get_inquiry(ساخت disk_op_s *op, ساخت cdbres_inquiry *ڈیٹا)
{
    ساخت cdb_request_sense cmd;
    memset(&cmd, 0, حجم(cmd));
    cmd.command = CDB_CMD_INQUIRY;
    cmd.لمبائی = حجم(*ڈیٹا);
    op->command = CMD_SCSI;
    op->گنتی = 1;
    op->buf_fl = ڈیٹا;
    op->cdbcmd = &cmd;
    op->blocksize = حجم(*ڈیٹا);
    واپس process_op(op);
}

// Request SENSE
ساکن صحیح
cdb_get_sense(ساخت disk_op_s *op, ساخت cdbres_request_sense *ڈیٹا)
{
    ساخت cdb_request_sense cmd;
    memset(&cmd, 0, حجم(cmd));
    cmd.command = CDB_CMD_REQUEST_SENSE;
    cmd.لمبائی = حجم(*ڈیٹا);
    op->command = CMD_SCSI;
    op->گنتی = 1;
    op->buf_fl = ڈیٹا;
    op->cdbcmd = &cmd;
    op->blocksize = حجم(*ڈیٹا);
    واپس process_op(op);
}

// Test unit ready
ساکن صحیح
cdb_test_unit_ready(ساخت disk_op_s *op)
{
    ساخت cdb_request_sense cmd;
    memset(&cmd, 0, حجم(cmd));
    cmd.command = CDB_CMD_TEST_UNIT_READY;
    op->command = CMD_SCSI;
    op->گنتی = 0;
    op->buf_fl = تہی;
    op->cdbcmd = &cmd;
    op->blocksize = 0;
    واپس process_op(op);
}

ساکن صحیح
cdb_read_capacity16(ساخت disk_op_s *op, ساخت cdbres_read_استعداد_16 *ڈیٹا)
{
    ساخت cdb_sai_read_استعداد_16 cmd;
    memset(&cmd, 0, حجم(cmd));
    cmd.command = CDB_CMD_SERVICE_ACTION_اندر;
    cmd.flags = CDB_CMD_SAI_READ_استعداد_16;
    cmd.len = cpu_مقصد_be32(حجم(ساخت cdbres_read_استعداد_16));
    op->command = CMD_SCSI;
    op->گنتی = 1;
    op->buf_fl = ڈیٹا;
    op->cdbcmd = &cmd;
    op->blocksize = حجم(*ڈیٹا);
    واپس process_op(op);
}


// Request capacity
ساکن صحیح
cdb_read_استعداد(ساخت disk_op_s *op, ساخت cdbres_read_استعداد *ڈیٹا)
{
    ساخت cdb_read_استعداد cmd;
    memset(&cmd, 0, حجم(cmd));
    cmd.command = CDB_CMD_READ_استعداد;
    op->command = CMD_SCSI;
    op->گنتی = 1;
    op->buf_fl = ڈیٹا;
    op->cdbcmd = &cmd;
    op->blocksize = حجم(*ڈیٹا);
    واپس process_op(op);
}

// Mode sense, geometry page.
ساکن صحیح
cdb_منوال_sense_geom(ساخت disk_op_s *op, ساخت cdbres_منوال_sense_geom *ڈیٹا)
{
    ساخت cdb_منوال_sense cmd;
    memset(&cmd, 0, حجم(cmd));
    cmd.command = CDB_CMD_منوال_SENSE;
    cmd.flags = 8; /* DBD */
    cmd.صفحہ = منوال_صفحہ_HD_GEOMETRY;
    cmd.گنتی = cpu_مقصد_be16(حجم(*ڈیٹا));
    op->command = CMD_SCSI;
    op->گنتی = 1;
    op->buf_fl = ڈیٹا;
    op->cdbcmd = &cmd;
    op->blocksize = حجم(*ڈیٹا);
    واپس process_op(op);
}


/****************************************************************
 * Main SCSI commands
 ****************************************************************/

// Create a scsi command request from a disk_op_s request
صحیح
scsi_fill_cmd(ساخت disk_op_s *op, خالی *cdbcmd, صحیح maxcdb)
{
    انتخاب (op->command) {
    حالت CMD_READ:
    حالت CMD_WRITE: ;
        اگر (op->lba < 0xFFFFFFFFULL) {
            ساخت cdb_rwdata_10 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_10
                            : CDB_CMD_WRITE_10);
            cmd->lba = cpu_مقصد_be32(op->lba);
            cmd->گنتی = cpu_مقصد_be16(op->گنتی);
        } ورنہ {
            ساخت cdb_rwdata_16 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_16
                            : CDB_CMD_WRITE_16);
            cmd->lba = cpu_مقصد_be64(op->lba);
            cmd->گنتی = cpu_مقصد_be32(op->گنتی);
        }
        واپس GET_FLATPTR(op->drive_fl->blksize);
    حالت CMD_SCSI:
        اگر (MODESEGMENT)
            واپس -1;
        حافظہ_نقل(cdbcmd, op->cdbcmd, maxcdb);
        واپس op->blocksize;
    طےشدہ:
        واپس -1;
    }
}

// Determine if the command is a request to pull data from the device
صحیح
scsi_is_read(ساخت disk_op_s *op)
{
    واپس op->command == CMD_READ || (
        !MODESEGMENT && op->command == CMD_SCSI && op->blocksize);
}

// Check if a SCSI device is ready to receive commands
صحیح
scsi_is_ready(ساخت disk_op_s *op)
{
    ASSERT32FLAT();
    dprintf(6, "scsi_is_ready (drive=%p)\n", op->drive_fl);

    /* Retry TEST UNIT READY for 5 seconds unless MEDIUM NOT PRESENT is
     * reported by the device 3 times.  If the device reports "IN PROGRESS",
     * 30 seconds is added. */
    صحیح tries = 3;
    صحیح اندر_پیش_رفت = 0;
    u32 انتہا = timer_calc(5000);
    برائے (;;) {
        اگر (timer_check(انتہا)) {
            dprintf(1, "test unit ready failed\n");
            واپس -1;
        }

        صحیح ret = cdb_test_unit_ready(op);
        اگر (!ret)
            // Success
            توڑو;

        ساخت cdbres_request_sense sense;
        ret = cdb_get_sense(op, &sense);
        اگر (ret)
            // Error - retry.
            جاری;

        // Sense succeeded.
        اگر (sense.asc == 0x3a) { /* MEDIUM NOT PRESENT */
            tries--;
            dprintf(1, "Device reports MEDIUM NOT PRESENT - %d tries left\n",
                tries);
            اگر (!tries)
                واپس -1;
        }

        اگر (sense.asc == 0x04 && sense.ascq == 0x01 && !اندر_پیش_رفت) {
            /* IN PROGRESS OF BECOMING READY */
            dprintf(1, "Waiting for device to detect medium... ");
            /* Allow 30 seconds more */
            انتہا = timer_calc(30000);
            اندر_پیش_رفت = 1;
        }
    }
    واپس 0;
}

#تعریف_کرو CDB_CMD_REPORT_LUNS  0xA0

ساخت cdb_report_luns {
    u8 command;
    u8 reserved_01[5];
    u32 لمبائی;
    u8 pad[6];
} PACKED;

ساخت scsi_lun {
    u16 lun[4];
};

ساخت cdbres_report_luns {
    u32 لمبائی;
    u32 reserved;
    ساخت scsi_lun luns[];
};

ساکن u64 scsilun2u64(ساخت scsi_lun *scsi_lun)
{
    صحیح i;
    u64 ret = 0;
    برائے (i = 0; i < ARRAY_سائز(scsi_lun->lun); i++)
        ret |= be16_مقصد_cpu(scsi_lun->lun[i]) << (16 * i);
    واپس ret;
}

// Issue REPORT LUNS on a temporary drive and iterate reported luns calling
// @add_lun for each
صحیح scsi_rep_luns_جائزہ(ساخت drive_s *tmp_drive, scsi_جوڑو_lun جوڑو_lun)
{
    صحیح ret = -1;
    /* start with the smallest possible buffer, otherwise some devices in QEMU
     * may (incorrectly) error out on returning less data than fits in it */
    u32 maxluns = 1;
    u32 nluns, i;
    ساخت cdb_report_luns cdb = {
        .command = CDB_CMD_REPORT_LUNS,
    };
    ساخت disk_op_s op = {
        .drive_fl = tmp_drive,
        .command = CMD_SCSI,
        .گنتی = 1,
        .cdbcmd = &cdb,
    };
    ساخت cdbres_report_luns *resp;

    ASSERT32FLAT();

    جب (1) {
        op.blocksize = حجم(ساخت cdbres_report_luns) +
            maxluns * حجم(ساخت scsi_lun);
        op.buf_fl = malloc_tmp(op.blocksize);
        اگر (!op.buf_fl) {
            warn_noalloc();
            واپس -1;
        }

        cdb.لمبائی = cpu_مقصد_be32(op.blocksize);
        اگر (process_op(&op) != DISK_RET_SUCCESS)
            جاؤ برآمد;

        resp = op.buf_fl;
        nluns = be32_مقصد_cpu(resp->لمبائی) / حجم(ساخت scsi_lun);
        اگر (nluns <= maxluns)
            توڑو;

        آزاد(op.buf_fl);
        maxluns = nluns;
    }

    برائے (i = 0, ret = 0; i < nluns; i++) {
        u64 lun = scsilun2u64(&resp->luns[i]);
        اگر (lun >> 32)
            جاری;
        ret += !جوڑو_lun((u32)lun, tmp_drive);
    }
برآمد:
    آزاد(op.buf_fl);
    واپس ret;
}

// Iterate LUNs on the target and call @add_lun for each
صحیح scsi_sequential_جائزہ(ساخت drive_s *tmp_drive, u32 maxluns,
                         scsi_جوڑو_lun جوڑو_lun)
{
    صحیح ret;
    u32 lun;

    برائے (lun = 0, ret = 0; lun < maxluns; lun++)
        ret += !جوڑو_lun(lun, tmp_drive);
    واپس ret;
}

// Validate drive, find block size / sector count, and register drive.
صحیح
scsi_drive_تیاری(ساخت drive_s *drive, ثابت حرف *s, صحیح prio)
{
    ASSERT32FLAT();
    ساخت disk_op_s dop;
    memset(&dop, 0, حجم(dop));
    dop.drive_fl = drive;
    ساخت cdbres_inquiry ڈیٹا;
    صحیح ret = cdb_get_inquiry(&dop, &ڈیٹا);
    اگر (ret)
        واپس ret;
    حرف vendor[حجم(ڈیٹا.vendor)+1], حاصل_ضرب[حجم(ڈیٹا.حاصل_ضرب)+1];
    حرف rev[حجم(ڈیٹا.rev)+1];
    strtcpy(vendor, ڈیٹا.vendor, حجم(vendor));
    nullTrailingSpace(vendor);
    strtcpy(حاصل_ضرب, ڈیٹا.حاصل_ضرب, حجم(حاصل_ضرب));
    nullTrailingSpace(حاصل_ضرب);
    strtcpy(rev, ڈیٹا.rev, حجم(rev));
    nullTrailingSpace(rev);
    صحیح pdt = ڈیٹا.pdt & 0x1f;
    صحیح removable = !!(ڈیٹا.removable & 0x80);
    dprintf(1, "%s vendor='%s' product='%s' rev='%s' type=%d removable=%d\n"
            , s, vendor, حاصل_ضرب, rev, pdt, removable);
    drive->removable = removable;

    اگر (pdt == SCSI_TYPE_CDROM) {
        drive->blksize = CDROM_SECTOR_سائز;
        drive->sectors = (u64)-1;

        حرف *desc = znprintf(MAXDESCSIZE, "DVD/CD [%s Drive %s %s %s]"
                              , s, vendor, حاصل_ضرب, rev);
        boot_جوڑو_cd(drive, desc, prio);
        واپس 0;
    }

    اگر (pdt != SCSI_TYPE_DISK)
        واپس -1;

    ret = scsi_is_ready(&dop);
    اگر (ret) {
        dprintf(1, "scsi_is_ready returned %d\n", ret);
        واپس ret;
    }

    ساخت cdbres_read_استعداد capdata;
    ret = cdb_read_استعداد(&dop, &capdata);
    اگر (ret)
        واپس ret;

    اگر (be32_مقصد_cpu(capdata.sectors) == 0xFFFFFFFFUL) {
        dprintf(3, "%s: >2TB Detected trying READCAP(16)\n", s);
        ساخت cdbres_read_استعداد_16 capdata16;
        ret = cdb_read_capacity16(&dop, &capdata16);
        drive->blksize = be32_مقصد_cpu(capdata16.blksize);
        drive->sectors = be64_مقصد_cpu(capdata16.sectors) + 1;
    } ورنہ {
        drive->blksize = be32_مقصد_cpu(capdata.blksize);
        drive->sectors = (u64)be32_مقصد_cpu(capdata.sectors) + 1;
    }

    اگر (drive->blksize != DISK_SECTOR_سائز) {
        dprintf(1, "%s: unsupported block size %d\n", s, drive->blksize);
        واپس -1;
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
    اگر (CONFIG_QEMU_HARDWARE && memcmp(vendor, "QEMU", 5) == 0) {
        ساخت cdbres_منوال_sense_geom geomdata;
        ret = cdb_منوال_sense_geom(&dop, &geomdata);
        اگر (ret == 0) {
            u32 cylinders;
            cylinders = geomdata.cyl[0] << 16;
            cylinders |= geomdata.cyl[1] << 8;
            cylinders |= geomdata.cyl[2];
            اگر (cylinders && geomdata.heads &&
                drive->sectors <= 0xFFFFFFFFULL &&
                ((u32)drive->sectors % (geomdata.heads * cylinders) == 0)) {
                drive->pchs.cylinder = cylinders;
                drive->pchs.سر = geomdata.heads;
                drive->pchs.sector = (u32)drive->sectors / (geomdata.heads * cylinders);
            }
        }
    }

    حرف *desc = znprintf(MAXDESCSIZE, "%s Drive %s %s %s"
                          , s, vendor, حاصل_ضرب, rev);
    boot_جوڑو_hd(drive, desc, prio);
    واپس 0;
}

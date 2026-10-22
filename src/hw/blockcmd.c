// Support for several common scsi like command data block requests
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "block.h" // struct disk_op_s
#ضمن "blockcmd.h" // struct cdb_request_sense
#ضمن "byteorder.h" // be32_to_cpu
#ضمن "farptr.h" // GET_FLATPTR
#ضمن "output.h" // dprintf
#ضمن "std/disk.h" // DISK_RET_EPARAM
#ضمن "string.h" // memset
#ضمن "util.h" // timer_calc
#ضمن "malloc.h"


/****************************************************************
 * Low level command requests
 ****************************************************************/

ساكن صحيح
cdb_get_inquiry(بنية disk_op_s *op, بنية cdbres_inquiry *معلومات)
{
    بنية cdb_request_sense cmd;
    memset(&cmd, 0, حجم(cmd));
    cmd.command = CDB_CMD_INQUIRY;
    cmd.ڊيگهائي = حجم(*معلومات);
    op->command = CMD_SCSI;
    op->ڳڻپ = 1;
    op->buf_fl = معلومات;
    op->cdbcmd = &cmd;
    op->blocksize = حجم(*معلومات);
    أرجع process_op(op);
}

// Request SENSE
ساكن صحيح
cdb_get_sense(بنية disk_op_s *op, بنية cdbres_request_sense *معلومات)
{
    بنية cdb_request_sense cmd;
    memset(&cmd, 0, حجم(cmd));
    cmd.command = CDB_CMD_REQUEST_SENSE;
    cmd.ڊيگهائي = حجم(*معلومات);
    op->command = CMD_SCSI;
    op->ڳڻپ = 1;
    op->buf_fl = معلومات;
    op->cdbcmd = &cmd;
    op->blocksize = حجم(*معلومات);
    أرجع process_op(op);
}

// Test unit ready
ساكن صحيح
cdb_test_unit_ready(بنية disk_op_s *op)
{
    بنية cdb_request_sense cmd;
    memset(&cmd, 0, حجم(cmd));
    cmd.command = CDB_CMD_TEST_UNIT_READY;
    op->command = CMD_SCSI;
    op->ڳڻپ = 0;
    op->buf_fl = عدم;
    op->cdbcmd = &cmd;
    op->blocksize = 0;
    أرجع process_op(op);
}

ساكن صحيح
cdb_read_capacity16(بنية disk_op_s *op, بنية cdbres_read_صلاحيت_16 *معلومات)
{
    بنية cdb_sai_read_صلاحيت_16 cmd;
    memset(&cmd, 0, حجم(cmd));
    cmd.command = CDB_CMD_SERVICE_ACTION_اندريون;
    cmd.flags = CDB_CMD_SAI_READ_صلاحيت_16;
    cmd.len = cpu_منزل_be32(حجم(بنية cdbres_read_صلاحيت_16));
    op->command = CMD_SCSI;
    op->ڳڻپ = 1;
    op->buf_fl = معلومات;
    op->cdbcmd = &cmd;
    op->blocksize = حجم(*معلومات);
    أرجع process_op(op);
}


// Request capacity
ساكن صحيح
cdb_read_صلاحيت(بنية disk_op_s *op, بنية cdbres_read_صلاحيت *معلومات)
{
    بنية cdb_read_صلاحيت cmd;
    memset(&cmd, 0, حجم(cmd));
    cmd.command = CDB_CMD_READ_صلاحيت;
    op->command = CMD_SCSI;
    op->ڳڻپ = 1;
    op->buf_fl = معلومات;
    op->cdbcmd = &cmd;
    op->blocksize = حجم(*معلومات);
    أرجع process_op(op);
}

// Mode sense, geometry page.
ساكن صحيح
cdb_گهڻائي_sense_geom(بنية disk_op_s *op, بنية cdbres_گهڻائي_sense_geom *معلومات)
{
    بنية cdb_گهڻائي_sense cmd;
    memset(&cmd, 0, حجم(cmd));
    cmd.command = CDB_CMD_گهڻائي_SENSE;
    cmd.flags = 8; /* DBD */
    cmd.صفحو = گهڻائي_صفحو_HD_GEOMETRY;
    cmd.ڳڻپ = cpu_منزل_be16(حجم(*معلومات));
    op->command = CMD_SCSI;
    op->ڳڻپ = 1;
    op->buf_fl = معلومات;
    op->cdbcmd = &cmd;
    op->blocksize = حجم(*معلومات);
    أرجع process_op(op);
}


/****************************************************************
 * Main SCSI commands
 ****************************************************************/

// Create a scsi command request from a disk_op_s request
صحيح
scsi_fill_cmd(بنية disk_op_s *op, فراغ *cdbcmd, صحيح maxcdb)
{
    بدل (op->command) {
    حالة CMD_READ:
    حالة CMD_WRITE: ;
        إذا (op->lba < 0xFFFFFFFFULL) {
            بنية cdb_rwdata_10 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_10
                            : CDB_CMD_WRITE_10);
            cmd->lba = cpu_منزل_be32(op->lba);
            cmd->ڳڻپ = cpu_منزل_be16(op->ڳڻپ);
        } وإلا {
            بنية cdb_rwdata_16 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_16
                            : CDB_CMD_WRITE_16);
            cmd->lba = cpu_منزل_be64(op->lba);
            cmd->ڳڻپ = cpu_منزل_be32(op->ڳڻپ);
        }
        أرجع GET_FLATPTR(op->drive_fl->blksize);
    حالة CMD_SCSI:
        إذا (MODESEGMENT)
            أرجع -1;
        يادگيري_نقل(cdbcmd, op->cdbcmd, maxcdb);
        أرجع op->blocksize;
    افتراضي:
        أرجع -1;
    }
}

// Determine if the command is a request to pull data from the device
صحيح
scsi_is_read(بنية disk_op_s *op)
{
    أرجع op->command == CMD_READ || (
        !MODESEGMENT && op->command == CMD_SCSI && op->blocksize);
}

// Check if a SCSI device is ready to receive commands
صحيح
scsi_is_ready(بنية disk_op_s *op)
{
    ASSERT32FLAT();
    dprintf(6, "scsi_is_ready (drive=%p)\n", op->drive_fl);

    /* Retry TEST UNIT READY for 5 seconds unless MEDIUM NOT PRESENT is
     * reported by the device 3 times.  If the device reports "IN PROGRESS",
     * 30 seconds is added. */
    صحيح tries = 3;
    صحيح اندريون_ترقي = 0;
    u32 پڇاڙي = timer_calc(5000);
    لكل (;;) {
        إذا (timer_check(پڇاڙي)) {
            dprintf(1, "test unit ready failed\n");
            أرجع -1;
        }

        صحيح ret = cdb_test_unit_ready(op);
        إذا (!ret)
            // Success
            اكسر;

        بنية cdbres_request_sense sense;
        ret = cdb_get_sense(op, &sense);
        إذا (ret)
            // Error - retry.
            استمر;

        // Sense succeeded.
        إذا (sense.asc == 0x3a) { /* MEDIUM NOT PRESENT */
            tries--;
            dprintf(1, "Device reports MEDIUM NOT PRESENT - %d tries left\n",
                tries);
            إذا (!tries)
                أرجع -1;
        }

        إذا (sense.asc == 0x04 && sense.ascq == 0x01 && !اندريون_ترقي) {
            /* IN PROGRESS OF BECOMING READY */
            dprintf(1, "Waiting for device to detect medium... ");
            /* Allow 30 seconds more */
            پڇاڙي = timer_calc(30000);
            اندريون_ترقي = 1;
        }
    }
    أرجع 0;
}

#وصف_ڪريو CDB_CMD_REPORT_LUNS  0xA0

بنية cdb_report_luns {
    u8 command;
    u8 reserved_01[5];
    u32 ڊيگهائي;
    u8 pad[6];
} PACKED;

بنية scsi_lun {
    u16 lun[4];
};

بنية cdbres_report_luns {
    u32 ڊيگهائي;
    u32 reserved;
    بنية scsi_lun luns[];
};

ساكن u64 scsilun2u64(بنية scsi_lun *scsi_lun)
{
    صحيح i;
    u64 ret = 0;
    لكل (i = 0; i < ARRAY_سائيز(scsi_lun->lun); i++)
        ret |= be16_منزل_cpu(scsi_lun->lun[i]) << (16 * i);
    أرجع ret;
}

// Issue REPORT LUNS on a temporary drive and iterate reported luns calling
// @add_lun for each
صحيح scsi_rep_luns_جاچ(بنية drive_s *tmp_drive, scsi_جوڙيو_lun جوڙيو_lun)
{
    صحيح ret = -1;
    /* start with the smallest possible buffer, otherwise some devices in QEMU
     * may (incorrectly) error out on returning less data than fits in it */
    u32 maxluns = 1;
    u32 nluns, i;
    بنية cdb_report_luns cdb = {
        .command = CDB_CMD_REPORT_LUNS,
    };
    بنية disk_op_s op = {
        .drive_fl = tmp_drive,
        .command = CMD_SCSI,
        .ڳڻپ = 1,
        .cdbcmd = &cdb,
    };
    بنية cdbres_report_luns *resp;

    ASSERT32FLAT();

    طالما (1) {
        op.blocksize = حجم(بنية cdbres_report_luns) +
            maxluns * حجم(بنية scsi_lun);
        op.buf_fl = malloc_tmp(op.blocksize);
        إذا (!op.buf_fl) {
            warn_noalloc();
            أرجع -1;
        }

        cdb.ڊيگهائي = cpu_منزل_be32(op.blocksize);
        إذا (process_op(&op) != DISK_RET_SUCCESS)
            اذهب ٻاهريون;

        resp = op.buf_fl;
        nluns = be32_منزل_cpu(resp->ڊيگهائي) / حجم(بنية scsi_lun);
        إذا (nluns <= maxluns)
            اكسر;

        حرر(op.buf_fl);
        maxluns = nluns;
    }

    لكل (i = 0, ret = 0; i < nluns; i++) {
        u64 lun = scsilun2u64(&resp->luns[i]);
        إذا (lun >> 32)
            استمر;
        ret += !جوڙيو_lun((u32)lun, tmp_drive);
    }
ٻاهريون:
    حرر(op.buf_fl);
    أرجع ret;
}

// Iterate LUNs on the target and call @add_lun for each
صحيح scsi_sequential_جاچ(بنية drive_s *tmp_drive, u32 maxluns,
                         scsi_جوڙيو_lun جوڙيو_lun)
{
    صحيح ret;
    u32 lun;

    لكل (lun = 0, ret = 0; lun < maxluns; lun++)
        ret += !جوڙيو_lun(lun, tmp_drive);
    أرجع ret;
}

// Validate drive, find block size / sector count, and register drive.
صحيح
scsi_drive_تياري(بنية drive_s *drive, ثابت حرف *s, صحيح prio)
{
    ASSERT32FLAT();
    بنية disk_op_s dop;
    memset(&dop, 0, حجم(dop));
    dop.drive_fl = drive;
    بنية cdbres_inquiry معلومات;
    صحيح ret = cdb_get_inquiry(&dop, &معلومات);
    إذا (ret)
        أرجع ret;
    حرف vendor[حجم(معلومات.vendor)+1], حاصل_ضرب[حجم(معلومات.حاصل_ضرب)+1];
    حرف rev[حجم(معلومات.rev)+1];
    strtcpy(vendor, معلومات.vendor, حجم(vendor));
    nullTrailingSpace(vendor);
    strtcpy(حاصل_ضرب, معلومات.حاصل_ضرب, حجم(حاصل_ضرب));
    nullTrailingSpace(حاصل_ضرب);
    strtcpy(rev, معلومات.rev, حجم(rev));
    nullTrailingSpace(rev);
    صحيح pdt = معلومات.pdt & 0x1f;
    صحيح removable = !!(معلومات.removable & 0x80);
    dprintf(1, "%s vendor='%s' product='%s' rev='%s' type=%d removable=%d\n"
            , s, vendor, حاصل_ضرب, rev, pdt, removable);
    drive->removable = removable;

    إذا (pdt == SCSI_TYPE_CDROM) {
        drive->blksize = CDROM_SECTOR_سائيز;
        drive->sectors = (u64)-1;

        حرف *desc = znprintf(MAXDESCSIZE, "DVD/CD [%s Drive %s %s %s]"
                              , s, vendor, حاصل_ضرب, rev);
        boot_جوڙيو_cd(drive, desc, prio);
        أرجع 0;
    }

    إذا (pdt != SCSI_TYPE_DISK)
        أرجع -1;

    ret = scsi_is_ready(&dop);
    إذا (ret) {
        dprintf(1, "scsi_is_ready returned %d\n", ret);
        أرجع ret;
    }

    بنية cdbres_read_صلاحيت capdata;
    ret = cdb_read_صلاحيت(&dop, &capdata);
    إذا (ret)
        أرجع ret;

    إذا (be32_منزل_cpu(capdata.sectors) == 0xFFFFFFFFUL) {
        dprintf(3, "%s: >2TB Detected trying READCAP(16)\n", s);
        بنية cdbres_read_صلاحيت_16 capdata16;
        ret = cdb_read_capacity16(&dop, &capdata16);
        drive->blksize = be32_منزل_cpu(capdata16.blksize);
        drive->sectors = be64_منزل_cpu(capdata16.sectors) + 1;
    } وإلا {
        drive->blksize = be32_منزل_cpu(capdata.blksize);
        drive->sectors = (u64)be32_منزل_cpu(capdata.sectors) + 1;
    }

    إذا (drive->blksize != DISK_SECTOR_سائيز) {
        dprintf(1, "%s: unsupported block size %d\n", s, drive->blksize);
        أرجع -1;
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
    إذا (CONFIG_QEMU_HARDWARE && memcmp(vendor, "QEMU", 5) == 0) {
        بنية cdbres_گهڻائي_sense_geom geomdata;
        ret = cdb_گهڻائي_sense_geom(&dop, &geomdata);
        إذا (ret == 0) {
            u32 cylinders;
            cylinders = geomdata.cyl[0] << 16;
            cylinders |= geomdata.cyl[1] << 8;
            cylinders |= geomdata.cyl[2];
            إذا (cylinders && geomdata.heads &&
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
    boot_جوڙيو_hd(drive, desc, prio);
    أرجع 0;
}

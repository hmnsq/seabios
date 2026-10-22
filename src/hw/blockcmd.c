// Support for several common scsi like command data block requests
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "block.h" // struct disk_op_s
#অন্তর্ভাব "blockcmd.h" // struct cdb_request_sense
#অন্তর্ভাব "byteorder.h" // be32_to_cpu
#অন্তর্ভাব "farptr.h" // GET_FLATPTR
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "std/disk.h" // DISK_RET_EPARAM
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // timer_calc
#অন্তর্ভাব "malloc.h"


/****************************************************************
 * Low level command requests
 ****************************************************************/

স্থির পূর্ণসংখ্যা
cdb_get_inquiry(কাঠামো disk_op_s *op, কাঠামো cdbres_inquiry *তথ্য)
{
    কাঠামো cdb_request_sense cmd;
    memset(&cmd, 0, আকার(cmd));
    cmd.command = CDB_CMD_INQUIRY;
    cmd.দৈর্ঘ্য = আকার(*তথ্য);
    op->command = CMD_SCSI;
    op->গুনতি = 1;
    op->buf_fl = তথ্য;
    op->cdbcmd = &cmd;
    op->blocksize = আকার(*তথ্য);
    ফেরত process_op(op);
}

// Request SENSE
স্থির পূর্ণসংখ্যা
cdb_get_sense(কাঠামো disk_op_s *op, কাঠামো cdbres_request_sense *তথ্য)
{
    কাঠামো cdb_request_sense cmd;
    memset(&cmd, 0, আকার(cmd));
    cmd.command = CDB_CMD_REQUEST_SENSE;
    cmd.দৈর্ঘ্য = আকার(*তথ্য);
    op->command = CMD_SCSI;
    op->গুনতি = 1;
    op->buf_fl = তথ্য;
    op->cdbcmd = &cmd;
    op->blocksize = আকার(*তথ্য);
    ফেরত process_op(op);
}

// Test unit ready
স্থির পূর্ণসংখ্যা
cdb_test_unit_ready(কাঠামো disk_op_s *op)
{
    কাঠামো cdb_request_sense cmd;
    memset(&cmd, 0, আকার(cmd));
    cmd.command = CDB_CMD_TEST_UNIT_READY;
    op->command = CMD_SCSI;
    op->গুনতি = 0;
    op->buf_fl = ফাঁকা;
    op->cdbcmd = &cmd;
    op->blocksize = 0;
    ফেরত process_op(op);
}

স্থির পূর্ণসংখ্যা
cdb_read_capacity16(কাঠামো disk_op_s *op, কাঠামো cdbres_read_capacity_16 *তথ্য)
{
    কাঠামো cdb_sai_read_capacity_16 cmd;
    memset(&cmd, 0, আকার(cmd));
    cmd.command = CDB_CMD_SERVICE_ACTION_IN;
    cmd.flags = CDB_CMD_SAI_READ_CAPACITY_16;
    cmd.len = cpu_to_be32(আকার(কাঠামো cdbres_read_capacity_16));
    op->command = CMD_SCSI;
    op->গুনতি = 1;
    op->buf_fl = তথ্য;
    op->cdbcmd = &cmd;
    op->blocksize = আকার(*তথ্য);
    ফেরত process_op(op);
}


// Request capacity
স্থির পূর্ণসংখ্যা
cdb_read_capacity(কাঠামো disk_op_s *op, কাঠামো cdbres_read_capacity *তথ্য)
{
    কাঠামো cdb_read_capacity cmd;
    memset(&cmd, 0, আকার(cmd));
    cmd.command = CDB_CMD_READ_CAPACITY;
    op->command = CMD_SCSI;
    op->গুনতি = 1;
    op->buf_fl = তথ্য;
    op->cdbcmd = &cmd;
    op->blocksize = আকার(*তথ্য);
    ফেরত process_op(op);
}

// Mode sense, geometry page.
স্থির পূর্ণসংখ্যা
cdb_mode_sense_geom(কাঠামো disk_op_s *op, কাঠামো cdbres_mode_sense_geom *তথ্য)
{
    কাঠামো cdb_mode_sense cmd;
    memset(&cmd, 0, আকার(cmd));
    cmd.command = CDB_CMD_MODE_SENSE;
    cmd.flags = 8; /* DBD */
    cmd.page = MODE_PAGE_HD_GEOMETRY;
    cmd.গুনতি = cpu_to_be16(আকার(*তথ্য));
    op->command = CMD_SCSI;
    op->গুনতি = 1;
    op->buf_fl = তথ্য;
    op->cdbcmd = &cmd;
    op->blocksize = আকার(*তথ্য);
    ফেরত process_op(op);
}


/****************************************************************
 * Main SCSI commands
 ****************************************************************/

// Create a scsi command request from a disk_op_s request
পূর্ণসংখ্যা
scsi_fill_cmd(কাঠামো disk_op_s *op, শূন্য *cdbcmd, পূর্ণসংখ্যা maxcdb)
{
    নির্বাচন (op->command) {
    ক্ষেত্র CMD_READ:
    ক্ষেত্র CMD_WRITE: ;
        যদি (op->lba < 0xFFFFFFFFULL) {
            কাঠামো cdb_rwdata_10 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_10
                            : CDB_CMD_WRITE_10);
            cmd->lba = cpu_to_be32(op->lba);
            cmd->গুনতি = cpu_to_be16(op->গুনতি);
        } অন্যথা {
            কাঠামো cdb_rwdata_16 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_16
                            : CDB_CMD_WRITE_16);
            cmd->lba = cpu_to_be64(op->lba);
            cmd->গুনতি = cpu_to_be32(op->গুনতি);
        }
        ফেরত GET_FLATPTR(op->drive_fl->blksize);
    ক্ষেত্র CMD_SCSI:
        যদি (MODESEGMENT)
            ফেরত -1;
        memcpy(cdbcmd, op->cdbcmd, maxcdb);
        ফেরত op->blocksize;
    পূর্বনির্ধারিত:
        ফেরত -1;
    }
}

// Determine if the command is a request to pull data from the device
পূর্ণসংখ্যা
scsi_is_read(কাঠামো disk_op_s *op)
{
    ফেরত op->command == CMD_READ || (
        !MODESEGMENT && op->command == CMD_SCSI && op->blocksize);
}

// Check if a SCSI device is ready to receive commands
পূর্ণসংখ্যা
scsi_is_ready(কাঠামো disk_op_s *op)
{
    ASSERT32FLAT();
    dprintf(6, "scsi_is_ready (drive=%p)\n", op->drive_fl);

    /* Retry TEST UNIT READY for 5 seconds unless MEDIUM NOT PRESENT is
     * reported by the device 3 times.  If the device reports "IN PROGRESS",
     * 30 seconds is added. */
    পূর্ণসংখ্যা tries = 3;
    পূর্ণসংখ্যা in_progress = 0;
    u32 end = timer_calc(5000);
    জন্য (;;) {
        যদি (timer_check(end)) {
            dprintf(1, "test unit ready failed\n");
            ফেরত -1;
        }

        পূর্ণসংখ্যা ret = cdb_test_unit_ready(op);
        যদি (!ret)
            // Success
            বিরতি;

        কাঠামো cdbres_request_sense sense;
        ret = cdb_get_sense(op, &sense);
        যদি (ret)
            // Error - retry.
            চালু;

        // Sense succeeded.
        যদি (sense.asc == 0x3a) { /* MEDIUM NOT PRESENT */
            tries--;
            dprintf(1, "Device reports MEDIUM NOT PRESENT - %d tries left\n",
                tries);
            যদি (!tries)
                ফেরত -1;
        }

        যদি (sense.asc == 0x04 && sense.ascq == 0x01 && !in_progress) {
            /* IN PROGRESS OF BECOMING READY */
            dprintf(1, "Waiting for device to detect medium... ");
            /* Allow 30 seconds more */
            end = timer_calc(30000);
            in_progress = 1;
        }
    }
    ফেরত 0;
}

#define CDB_CMD_REPORT_LUNS  0xA0

কাঠামো cdb_report_luns {
    u8 command;
    u8 reserved_01[5];
    u32 দৈর্ঘ্য;
    u8 pad[6];
} PACKED;

কাঠামো scsi_lun {
    u16 lun[4];
};

কাঠামো cdbres_report_luns {
    u32 দৈর্ঘ্য;
    u32 reserved;
    কাঠামো scsi_lun luns[];
};

স্থির u64 scsilun2u64(কাঠামো scsi_lun *scsi_lun)
{
    পূর্ণসংখ্যা i;
    u64 ret = 0;
    জন্য (i = 0; i < ARRAY_আকার(scsi_lun->lun); i++)
        ret |= be16_to_cpu(scsi_lun->lun[i]) << (16 * i);
    ফেরত ret;
}

// Issue REPORT LUNS on a temporary drive and iterate reported luns calling
// @add_lun for each
পূর্ণসংখ্যা scsi_rep_luns_scan(কাঠামো drive_s *tmp_drive, scsi_add_lun add_lun)
{
    পূর্ণসংখ্যা ret = -1;
    /* start with the smallest possible buffer, otherwise some devices in QEMU
     * may (incorrectly) error out on returning less data than fits in it */
    u32 maxluns = 1;
    u32 nluns, i;
    কাঠামো cdb_report_luns cdb = {
        .command = CDB_CMD_REPORT_LUNS,
    };
    কাঠামো disk_op_s op = {
        .drive_fl = tmp_drive,
        .command = CMD_SCSI,
        .গুনতি = 1,
        .cdbcmd = &cdb,
    };
    কাঠামো cdbres_report_luns *resp;

    ASSERT32FLAT();

    যখন (1) {
        op.blocksize = আকার(কাঠামো cdbres_report_luns) +
            maxluns * আকার(কাঠামো scsi_lun);
        op.buf_fl = malloc_tmp(op.blocksize);
        যদি (!op.buf_fl) {
            warn_noalloc();
            ফেরত -1;
        }

        cdb.দৈর্ঘ্য = cpu_to_be32(op.blocksize);
        যদি (process_op(&op) != DISK_RET_SUCCESS)
            যাও out;

        resp = op.buf_fl;
        nluns = be32_to_cpu(resp->দৈর্ঘ্য) / আকার(কাঠামো scsi_lun);
        যদি (nluns <= maxluns)
            বিরতি;

        মুক্ত(op.buf_fl);
        maxluns = nluns;
    }

    জন্য (i = 0, ret = 0; i < nluns; i++) {
        u64 lun = scsilun2u64(&resp->luns[i]);
        যদি (lun >> 32)
            চালু;
        ret += !add_lun((u32)lun, tmp_drive);
    }
out:
    মুক্ত(op.buf_fl);
    ফেরত ret;
}

// Iterate LUNs on the target and call @add_lun for each
পূর্ণসংখ্যা scsi_sequential_scan(কাঠামো drive_s *tmp_drive, u32 maxluns,
                         scsi_add_lun add_lun)
{
    পূর্ণসংখ্যা ret;
    u32 lun;

    জন্য (lun = 0, ret = 0; lun < maxluns; lun++)
        ret += !add_lun(lun, tmp_drive);
    ফেরত ret;
}

// Validate drive, find block size / sector count, and register drive.
পূর্ণসংখ্যা
scsi_drive_setup(কাঠামো drive_s *drive, ধ্রুবক অক্ষর *s, পূর্ণসংখ্যা prio)
{
    ASSERT32FLAT();
    কাঠামো disk_op_s dop;
    memset(&dop, 0, আকার(dop));
    dop.drive_fl = drive;
    কাঠামো cdbres_inquiry তথ্য;
    পূর্ণসংখ্যা ret = cdb_get_inquiry(&dop, &তথ্য);
    যদি (ret)
        ফেরত ret;
    অক্ষর vendor[আকার(তথ্য.vendor)+1], product[আকার(তথ্য.product)+1];
    অক্ষর rev[আকার(তথ্য.rev)+1];
    strtcpy(vendor, তথ্য.vendor, আকার(vendor));
    nullTrailingSpace(vendor);
    strtcpy(product, তথ্য.product, আকার(product));
    nullTrailingSpace(product);
    strtcpy(rev, তথ্য.rev, আকার(rev));
    nullTrailingSpace(rev);
    পূর্ণসংখ্যা pdt = তথ্য.pdt & 0x1f;
    পূর্ণসংখ্যা removable = !!(তথ্য.removable & 0x80);
    dprintf(1, "%s vendor='%s' product='%s' rev='%s' type=%d removable=%d\n"
            , s, vendor, product, rev, pdt, removable);
    drive->removable = removable;

    যদি (pdt == SCSI_TYPE_CDROM) {
        drive->blksize = CDROM_SECTOR_আকার;
        drive->sectors = (u64)-1;

        অক্ষর *desc = znprintf(MAXDESCSIZE, "DVD/CD [%s Drive %s %s %s]"
                              , s, vendor, product, rev);
        boot_add_cd(drive, desc, prio);
        ফেরত 0;
    }

    যদি (pdt != SCSI_TYPE_DISK)
        ফেরত -1;

    ret = scsi_is_ready(&dop);
    যদি (ret) {
        dprintf(1, "scsi_is_ready returned %d\n", ret);
        ফেরত ret;
    }

    কাঠামো cdbres_read_capacity capdata;
    ret = cdb_read_capacity(&dop, &capdata);
    যদি (ret)
        ফেরত ret;

    যদি (be32_to_cpu(capdata.sectors) == 0xFFFFFFFFUL) {
        dprintf(3, "%s: >2TB Detected trying READCAP(16)\n", s);
        কাঠামো cdbres_read_capacity_16 capdata16;
        ret = cdb_read_capacity16(&dop, &capdata16);
        drive->blksize = be32_to_cpu(capdata16.blksize);
        drive->sectors = be64_to_cpu(capdata16.sectors) + 1;
    } অন্যথা {
        drive->blksize = be32_to_cpu(capdata.blksize);
        drive->sectors = (u64)be32_to_cpu(capdata.sectors) + 1;
    }

    যদি (drive->blksize != DISK_SECTOR_আকার) {
        dprintf(1, "%s: unsupported block size %d\n", s, drive->blksize);
        ফেরত -1;
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
    যদি (CONFIG_QEMU_HARDWARE && memcmp(vendor, "QEMU", 5) == 0) {
        কাঠামো cdbres_mode_sense_geom geomdata;
        ret = cdb_mode_sense_geom(&dop, &geomdata);
        যদি (ret == 0) {
            u32 cylinders;
            cylinders = geomdata.cyl[0] << 16;
            cylinders |= geomdata.cyl[1] << 8;
            cylinders |= geomdata.cyl[2];
            যদি (cylinders && geomdata.heads &&
                drive->sectors <= 0xFFFFFFFFULL &&
                ((u32)drive->sectors % (geomdata.heads * cylinders) == 0)) {
                drive->pchs.cylinder = cylinders;
                drive->pchs.শীর্ষ = geomdata.heads;
                drive->pchs.sector = (u32)drive->sectors / (geomdata.heads * cylinders);
            }
        }
    }

    অক্ষর *desc = znprintf(MAXDESCSIZE, "%s Drive %s %s %s"
                          , s, vendor, product, rev);
    boot_add_hd(drive, desc, prio);
    ফেরত 0;
}

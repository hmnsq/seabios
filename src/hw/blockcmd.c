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

স্থির পূর্ণাঙ্ক
cdb_get_inquiry(সংরচনা disk_op_s *op, সংরচনা cdbres_inquiry *data)
{
    সংরচনা cdb_request_sense cmd;
    memset(&cmd, 0, পরিমাণ(cmd));
    cmd.command = CDB_CMD_INQUIRY;
    cmd.length = পরিমাণ(*data);
    op->command = CMD_SCSI;
    op->গুনতি = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = পরিমাণ(*data);
    প্রত্যাগম process_op(op);
}

// Request SENSE
স্থির পূর্ণাঙ্ক
cdb_get_sense(সংরচনা disk_op_s *op, সংরচনা cdbres_request_sense *data)
{
    সংরচনা cdb_request_sense cmd;
    memset(&cmd, 0, পরিমাণ(cmd));
    cmd.command = CDB_CMD_REQUEST_SENSE;
    cmd.length = পরিমাণ(*data);
    op->command = CMD_SCSI;
    op->গুনতি = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = পরিমাণ(*data);
    প্রত্যাগম process_op(op);
}

// Test unit ready
স্থির পূর্ণাঙ্ক
cdb_test_unit_ready(সংরচনা disk_op_s *op)
{
    সংরচনা cdb_request_sense cmd;
    memset(&cmd, 0, পরিমাণ(cmd));
    cmd.command = CDB_CMD_TEST_UNIT_READY;
    op->command = CMD_SCSI;
    op->গুনতি = 0;
    op->buf_fl = রিক্ত;
    op->cdbcmd = &cmd;
    op->blocksize = 0;
    প্রত্যাগম process_op(op);
}

স্থির পূর্ণাঙ্ক
cdb_read_capacity16(সংরচনা disk_op_s *op, সংরচনা cdbres_read_capacity_16 *data)
{
    সংরচনা cdb_sai_read_capacity_16 cmd;
    memset(&cmd, 0, পরিমাণ(cmd));
    cmd.command = CDB_CMD_SERVICE_ACTION_IN;
    cmd.flags = CDB_CMD_SAI_READ_CAPACITY_16;
    cmd.len = cpu_to_be32(পরিমাণ(সংরচনা cdbres_read_capacity_16));
    op->command = CMD_SCSI;
    op->গুনতি = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = পরিমাণ(*data);
    প্রত্যাগম process_op(op);
}


// Request capacity
স্থির পূর্ণাঙ্ক
cdb_read_capacity(সংরচনা disk_op_s *op, সংরচনা cdbres_read_capacity *data)
{
    সংরচনা cdb_read_capacity cmd;
    memset(&cmd, 0, পরিমাণ(cmd));
    cmd.command = CDB_CMD_READ_CAPACITY;
    op->command = CMD_SCSI;
    op->গুনতি = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = পরিমাণ(*data);
    প্রত্যাগম process_op(op);
}

// Mode sense, geometry page.
স্থির পূর্ণাঙ্ক
cdb_mode_sense_geom(সংরচনা disk_op_s *op, সংরচনা cdbres_mode_sense_geom *data)
{
    সংরচনা cdb_mode_sense cmd;
    memset(&cmd, 0, পরিমাণ(cmd));
    cmd.command = CDB_CMD_MODE_SENSE;
    cmd.flags = 8; /* DBD */
    cmd.page = MODE_PAGE_HD_GEOMETRY;
    cmd.গুনতি = cpu_to_be16(পরিমাণ(*data));
    op->command = CMD_SCSI;
    op->গুনতি = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = পরিমাণ(*data);
    প্রত্যাগম process_op(op);
}


/****************************************************************
 * Main SCSI commands
 ****************************************************************/

// Create a scsi command request from a disk_op_s request
পূর্ণাঙ্ক
scsi_fill_cmd(সংরচনা disk_op_s *op, শূন্য *cdbcmd, পূর্ণাঙ্ক maxcdb)
{
    বিকল্প (op->command) {
    পক্ষ CMD_READ:
    পক্ষ CMD_WRITE: ;
        যদি (op->lba < 0xFFFFFFFFULL) {
            সংরচনা cdb_rwdata_10 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_10
                            : CDB_CMD_WRITE_10);
            cmd->lba = cpu_to_be32(op->lba);
            cmd->গুনতি = cpu_to_be16(op->গুনতি);
        } অন্যথা {
            সংরচনা cdb_rwdata_16 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_16
                            : CDB_CMD_WRITE_16);
            cmd->lba = cpu_to_be64(op->lba);
            cmd->গুনতি = cpu_to_be32(op->গুনতি);
        }
        প্রত্যাগম GET_FLATPTR(op->drive_fl->blksize);
    পক্ষ CMD_SCSI:
        যদি (MODESEGMENT)
            প্রত্যাগম -1;
        memcpy(cdbcmd, op->cdbcmd, maxcdb);
        প্রত্যাগম op->blocksize;
    স্বভাব:
        প্রত্যাগম -1;
    }
}

// Determine if the command is a request to pull data from the device
পূর্ণাঙ্ক
scsi_is_read(সংরচনা disk_op_s *op)
{
    প্রত্যাগম op->command == CMD_READ || (
        !MODESEGMENT && op->command == CMD_SCSI && op->blocksize);
}

// Check if a SCSI device is ready to receive commands
পূর্ণাঙ্ক
scsi_is_ready(সংরচনা disk_op_s *op)
{
    ASSERT32FLAT();
    dprintf(6, "scsi_is_ready (drive=%p)\n", op->drive_fl);

    /* Retry TEST UNIT READY for 5 seconds unless MEDIUM NOT PRESENT is
     * reported by the device 3 times.  If the device reports "IN PROGRESS",
     * 30 seconds is added. */
    পূর্ণাঙ্ক tries = 3;
    পূর্ণাঙ্ক in_progress = 0;
    u32 end = timer_calc(5000);
    প্রতি (;;) {
        যদি (timer_check(end)) {
            dprintf(1, "test unit ready failed\n");
            প্রত্যাগম -1;
        }

        পূর্ণাঙ্ক ret = cdb_test_unit_ready(op);
        যদি (!ret)
            // Success
            ভঙ্গ;

        সংরচনা cdbres_request_sense sense;
        ret = cdb_get_sense(op, &sense);
        যদি (ret)
            // Error - retry.
            অনুবর্ত;

        // Sense succeeded.
        যদি (sense.asc == 0x3a) { /* MEDIUM NOT PRESENT */
            tries--;
            dprintf(1, "Device reports MEDIUM NOT PRESENT - %d tries left\n",
                tries);
            যদি (!tries)
                প্রত্যাগম -1;
        }

        যদি (sense.asc == 0x04 && sense.ascq == 0x01 && !in_progress) {
            /* IN PROGRESS OF BECOMING READY */
            dprintf(1, "Waiting for device to detect medium... ");
            /* Allow 30 seconds more */
            end = timer_calc(30000);
            in_progress = 1;
        }
    }
    প্রত্যাগম 0;
}

#define CDB_CMD_REPORT_LUNS  0xA0

সংরচনা cdb_report_luns {
    u8 command;
    u8 reserved_01[5];
    u32 length;
    u8 pad[6];
} PACKED;

সংরচনা scsi_lun {
    u16 lun[4];
};

সংরচনা cdbres_report_luns {
    u32 length;
    u32 reserved;
    সংরচনা scsi_lun luns[];
};

স্থির u64 scsilun2u64(সংরচনা scsi_lun *scsi_lun)
{
    পূর্ণাঙ্ক i;
    u64 ret = 0;
    প্রতি (i = 0; i < ARRAY_SIZE(scsi_lun->lun); i++)
        ret |= be16_to_cpu(scsi_lun->lun[i]) << (16 * i);
    প্রত্যাগম ret;
}

// Issue REPORT LUNS on a temporary drive and iterate reported luns calling
// @add_lun for each
পূর্ণাঙ্ক scsi_rep_luns_scan(সংরচনা drive_s *tmp_drive, scsi_add_lun add_lun)
{
    পূর্ণাঙ্ক ret = -1;
    /* start with the smallest possible buffer, otherwise some devices in QEMU
     * may (incorrectly) error out on returning less data than fits in it */
    u32 maxluns = 1;
    u32 nluns, i;
    সংরচনা cdb_report_luns cdb = {
        .command = CDB_CMD_REPORT_LUNS,
    };
    সংরচনা disk_op_s op = {
        .drive_fl = tmp_drive,
        .command = CMD_SCSI,
        .গুনতি = 1,
        .cdbcmd = &cdb,
    };
    সংরচনা cdbres_report_luns *resp;

    ASSERT32FLAT();

    যাবত্ (1) {
        op.blocksize = পরিমাণ(সংরচনা cdbres_report_luns) +
            maxluns * পরিমাণ(সংরচনা scsi_lun);
        op.buf_fl = malloc_tmp(op.blocksize);
        যদি (!op.buf_fl) {
            warn_noalloc();
            প্রত্যাগম -1;
        }

        cdb.length = cpu_to_be32(op.blocksize);
        যদি (process_op(&op) != DISK_RET_SUCCESS)
            গচ্ছ out;

        resp = op.buf_fl;
        nluns = be32_to_cpu(resp->length) / পরিমাণ(সংরচনা scsi_lun);
        যদি (nluns <= maxluns)
            ভঙ্গ;

        মুক্ত(op.buf_fl);
        maxluns = nluns;
    }

    প্রতি (i = 0, ret = 0; i < nluns; i++) {
        u64 lun = scsilun2u64(&resp->luns[i]);
        যদি (lun >> 32)
            অনুবর্ত;
        ret += !add_lun((u32)lun, tmp_drive);
    }
out:
    মুক্ত(op.buf_fl);
    প্রত্যাগম ret;
}

// Iterate LUNs on the target and call @add_lun for each
পূর্ণাঙ্ক scsi_sequential_scan(সংরচনা drive_s *tmp_drive, u32 maxluns,
                         scsi_add_lun add_lun)
{
    পূর্ণাঙ্ক ret;
    u32 lun;

    প্রতি (lun = 0, ret = 0; lun < maxluns; lun++)
        ret += !add_lun(lun, tmp_drive);
    প্রত্যাগম ret;
}

// Validate drive, find block size / sector count, and register drive.
পূর্ণাঙ্ক
scsi_drive_setup(সংরচনা drive_s *drive, ধ্রুব বর্ণ *s, পূর্ণাঙ্ক prio)
{
    ASSERT32FLAT();
    সংরচনা disk_op_s dop;
    memset(&dop, 0, পরিমাণ(dop));
    dop.drive_fl = drive;
    সংরচনা cdbres_inquiry data;
    পূর্ণাঙ্ক ret = cdb_get_inquiry(&dop, &data);
    যদি (ret)
        প্রত্যাগম ret;
    বর্ণ vendor[পরিমাণ(data.vendor)+1], product[পরিমাণ(data.product)+1];
    বর্ণ rev[পরিমাণ(data.rev)+1];
    strtcpy(vendor, data.vendor, পরিমাণ(vendor));
    nullTrailingSpace(vendor);
    strtcpy(product, data.product, পরিমাণ(product));
    nullTrailingSpace(product);
    strtcpy(rev, data.rev, পরিমাণ(rev));
    nullTrailingSpace(rev);
    পূর্ণাঙ্ক pdt = data.pdt & 0x1f;
    পূর্ণাঙ্ক removable = !!(data.removable & 0x80);
    dprintf(1, "%s vendor='%s' product='%s' rev='%s' type=%d removable=%d\n"
            , s, vendor, product, rev, pdt, removable);
    drive->removable = removable;

    যদি (pdt == SCSI_TYPE_CDROM) {
        drive->blksize = CDROM_SECTOR_SIZE;
        drive->sectors = (u64)-1;

        বর্ণ *desc = znprintf(MAXDESCSIZE, "DVD/CD [%s Drive %s %s %s]"
                              , s, vendor, product, rev);
        boot_add_cd(drive, desc, prio);
        প্রত্যাগম 0;
    }

    যদি (pdt != SCSI_TYPE_DISK)
        প্রত্যাগম -1;

    ret = scsi_is_ready(&dop);
    যদি (ret) {
        dprintf(1, "scsi_is_ready returned %d\n", ret);
        প্রত্যাগম ret;
    }

    সংরচনা cdbres_read_capacity capdata;
    ret = cdb_read_capacity(&dop, &capdata);
    যদি (ret)
        প্রত্যাগম ret;

    যদি (be32_to_cpu(capdata.sectors) == 0xFFFFFFFFUL) {
        dprintf(3, "%s: >2TB Detected trying READCAP(16)\n", s);
        সংরচনা cdbres_read_capacity_16 capdata16;
        ret = cdb_read_capacity16(&dop, &capdata16);
        drive->blksize = be32_to_cpu(capdata16.blksize);
        drive->sectors = be64_to_cpu(capdata16.sectors) + 1;
    } অন্যথা {
        drive->blksize = be32_to_cpu(capdata.blksize);
        drive->sectors = (u64)be32_to_cpu(capdata.sectors) + 1;
    }

    যদি (drive->blksize != DISK_SECTOR_SIZE) {
        dprintf(1, "%s: unsupported block size %d\n", s, drive->blksize);
        প্রত্যাগম -1;
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
        সংরচনা cdbres_mode_sense_geom geomdata;
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
                drive->pchs.head = geomdata.heads;
                drive->pchs.sector = (u32)drive->sectors / (geomdata.heads * cylinders);
            }
        }
    }

    বর্ণ *desc = znprintf(MAXDESCSIZE, "%s Drive %s %s %s"
                          , s, vendor, product, rev);
    boot_add_hd(drive, desc, prio);
    প্রত্যাগম 0;
}

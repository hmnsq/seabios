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

ایستا صحیح
cdb_get_inquiry(ساختار disk_op_s *op, ساختار cdbres_inquiry *data)
{
    ساختار cdb_request_sense cmd;
    memset(&cmd, 0, اندازه(cmd));
    cmd.command = CDB_CMD_INQUIRY;
    cmd.length = اندازه(*data);
    op->command = CMD_SCSI;
    op->شمار = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = اندازه(*data);
    بازگشت process_op(op);
}

// Request SENSE
ایستا صحیح
cdb_get_sense(ساختار disk_op_s *op, ساختار cdbres_request_sense *data)
{
    ساختار cdb_request_sense cmd;
    memset(&cmd, 0, اندازه(cmd));
    cmd.command = CDB_CMD_REQUEST_SENSE;
    cmd.length = اندازه(*data);
    op->command = CMD_SCSI;
    op->شمار = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = اندازه(*data);
    بازگشت process_op(op);
}

// Test unit ready
ایستا صحیح
cdb_test_unit_ready(ساختار disk_op_s *op)
{
    ساختار cdb_request_sense cmd;
    memset(&cmd, 0, اندازه(cmd));
    cmd.command = CDB_CMD_TEST_UNIT_READY;
    op->command = CMD_SCSI;
    op->شمار = 0;
    op->buf_fl = پوچ;
    op->cdbcmd = &cmd;
    op->blocksize = 0;
    بازگشت process_op(op);
}

ایستا صحیح
cdb_read_capacity16(ساختار disk_op_s *op, ساختار cdbres_read_capacity_16 *data)
{
    ساختار cdb_sai_read_capacity_16 cmd;
    memset(&cmd, 0, اندازه(cmd));
    cmd.command = CDB_CMD_SERVICE_ACTION_IN;
    cmd.flags = CDB_CMD_SAI_READ_CAPACITY_16;
    cmd.len = cpu_to_be32(اندازه(ساختار cdbres_read_capacity_16));
    op->command = CMD_SCSI;
    op->شمار = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = اندازه(*data);
    بازگشت process_op(op);
}


// Request capacity
ایستا صحیح
cdb_read_capacity(ساختار disk_op_s *op, ساختار cdbres_read_capacity *data)
{
    ساختار cdb_read_capacity cmd;
    memset(&cmd, 0, اندازه(cmd));
    cmd.command = CDB_CMD_READ_CAPACITY;
    op->command = CMD_SCSI;
    op->شمار = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = اندازه(*data);
    بازگشت process_op(op);
}

// Mode sense, geometry page.
ایستا صحیح
cdb_mode_sense_geom(ساختار disk_op_s *op, ساختار cdbres_mode_sense_geom *data)
{
    ساختار cdb_mode_sense cmd;
    memset(&cmd, 0, اندازه(cmd));
    cmd.command = CDB_CMD_MODE_SENSE;
    cmd.flags = 8; /* DBD */
    cmd.page = MODE_PAGE_HD_GEOMETRY;
    cmd.شمار = cpu_to_be16(اندازه(*data));
    op->command = CMD_SCSI;
    op->شمار = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = اندازه(*data);
    بازگشت process_op(op);
}


/****************************************************************
 * Main SCSI commands
 ****************************************************************/

// Create a scsi command request from a disk_op_s request
صحیح
scsi_fill_cmd(ساختار disk_op_s *op, تهی *cdbcmd, صحیح maxcdb)
{
    انتخاب (op->command) {
    حالت CMD_READ:
    حالت CMD_WRITE: ;
        اگر (op->lba < 0xFFFFFFFFULL) {
            ساختار cdb_rwdata_10 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_10
                            : CDB_CMD_WRITE_10);
            cmd->lba = cpu_to_be32(op->lba);
            cmd->شمار = cpu_to_be16(op->شمار);
        } وگرنه {
            ساختار cdb_rwdata_16 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_16
                            : CDB_CMD_WRITE_16);
            cmd->lba = cpu_to_be64(op->lba);
            cmd->شمار = cpu_to_be32(op->شمار);
        }
        بازگشت GET_FLATPTR(op->drive_fl->blksize);
    حالت CMD_SCSI:
        اگر (MODESEGMENT)
            بازگشت -1;
        memcpy(cdbcmd, op->cdbcmd, maxcdb);
        بازگشت op->blocksize;
    پیشفرض:
        بازگشت -1;
    }
}

// Determine if the command is a request to pull data from the device
صحیح
scsi_is_read(ساختار disk_op_s *op)
{
    بازگشت op->command == CMD_READ || (
        !MODESEGMENT && op->command == CMD_SCSI && op->blocksize);
}

// Check if a SCSI device is ready to receive commands
صحیح
scsi_is_ready(ساختار disk_op_s *op)
{
    ASSERT32FLAT();
    dprintf(6, "scsi_is_ready (drive=%p)\n", op->drive_fl);

    /* Retry TEST UNIT READY for 5 seconds unless MEDIUM NOT PRESENT is
     * reported by the device 3 times.  If the device reports "IN PROGRESS",
     * 30 seconds is added. */
    صحیح tries = 3;
    صحیح in_progress = 0;
    u32 end = timer_calc(5000);
    برای (;;) {
        اگر (timer_check(end)) {
            dprintf(1, "test unit ready failed\n");
            بازگشت -1;
        }

        صحیح ret = cdb_test_unit_ready(op);
        اگر (!ret)
            // Success
            بشکن;

        ساختار cdbres_request_sense sense;
        ret = cdb_get_sense(op, &sense);
        اگر (ret)
            // Error - retry.
            ادامه;

        // Sense succeeded.
        اگر (sense.asc == 0x3a) { /* MEDIUM NOT PRESENT */
            tries--;
            dprintf(1, "Device reports MEDIUM NOT PRESENT - %d tries left\n",
                tries);
            اگر (!tries)
                بازگشت -1;
        }

        اگر (sense.asc == 0x04 && sense.ascq == 0x01 && !in_progress) {
            /* IN PROGRESS OF BECOMING READY */
            dprintf(1, "Waiting for device to detect medium... ");
            /* Allow 30 seconds more */
            end = timer_calc(30000);
            in_progress = 1;
        }
    }
    بازگشت 0;
}

#define CDB_CMD_REPORT_LUNS  0xA0

ساختار cdb_report_luns {
    u8 command;
    u8 reserved_01[5];
    u32 length;
    u8 pad[6];
} PACKED;

ساختار scsi_lun {
    u16 lun[4];
};

ساختار cdbres_report_luns {
    u32 length;
    u32 reserved;
    ساختار scsi_lun luns[];
};

ایستا u64 scsilun2u64(ساختار scsi_lun *scsi_lun)
{
    صحیح i;
    u64 ret = 0;
    برای (i = 0; i < ARRAY_SIZE(scsi_lun->lun); i++)
        ret |= be16_to_cpu(scsi_lun->lun[i]) << (16 * i);
    بازگشت ret;
}

// Issue REPORT LUNS on a temporary drive and iterate reported luns calling
// @add_lun for each
صحیح scsi_rep_luns_scan(ساختار drive_s *tmp_drive, scsi_add_lun add_lun)
{
    صحیح ret = -1;
    /* start with the smallest possible buffer, otherwise some devices in QEMU
     * may (incorrectly) error out on returning less data than fits in it */
    u32 maxluns = 1;
    u32 nluns, i;
    ساختار cdb_report_luns cdb = {
        .command = CDB_CMD_REPORT_LUNS,
    };
    ساختار disk_op_s op = {
        .drive_fl = tmp_drive,
        .command = CMD_SCSI,
        .شمار = 1,
        .cdbcmd = &cdb,
    };
    ساختار cdbres_report_luns *resp;

    ASSERT32FLAT();

    وقتی (1) {
        op.blocksize = اندازه(ساختار cdbres_report_luns) +
            maxluns * اندازه(ساختار scsi_lun);
        op.buf_fl = malloc_tmp(op.blocksize);
        اگر (!op.buf_fl) {
            warn_noalloc();
            بازگشت -1;
        }

        cdb.length = cpu_to_be32(op.blocksize);
        اگر (process_op(&op) != DISK_RET_SUCCESS)
            برو out;

        resp = op.buf_fl;
        nluns = be32_to_cpu(resp->length) / اندازه(ساختار scsi_lun);
        اگر (nluns <= maxluns)
            بشکن;

        آزاد(op.buf_fl);
        maxluns = nluns;
    }

    برای (i = 0, ret = 0; i < nluns; i++) {
        u64 lun = scsilun2u64(&resp->luns[i]);
        اگر (lun >> 32)
            ادامه;
        ret += !add_lun((u32)lun, tmp_drive);
    }
out:
    آزاد(op.buf_fl);
    بازگشت ret;
}

// Iterate LUNs on the target and call @add_lun for each
صحیح scsi_sequential_scan(ساختار drive_s *tmp_drive, u32 maxluns,
                         scsi_add_lun add_lun)
{
    صحیح ret;
    u32 lun;

    برای (lun = 0, ret = 0; lun < maxluns; lun++)
        ret += !add_lun(lun, tmp_drive);
    بازگشت ret;
}

// Validate drive, find block size / sector count, and register drive.
صحیح
scsi_drive_setup(ساختار drive_s *drive, ثابت نویسه *s, صحیح prio)
{
    ASSERT32FLAT();
    ساختار disk_op_s dop;
    memset(&dop, 0, اندازه(dop));
    dop.drive_fl = drive;
    ساختار cdbres_inquiry data;
    صحیح ret = cdb_get_inquiry(&dop, &data);
    اگر (ret)
        بازگشت ret;
    نویسه vendor[اندازه(data.vendor)+1], product[اندازه(data.product)+1];
    نویسه rev[اندازه(data.rev)+1];
    strtcpy(vendor, data.vendor, اندازه(vendor));
    nullTrailingSpace(vendor);
    strtcpy(product, data.product, اندازه(product));
    nullTrailingSpace(product);
    strtcpy(rev, data.rev, اندازه(rev));
    nullTrailingSpace(rev);
    صحیح pdt = data.pdt & 0x1f;
    صحیح removable = !!(data.removable & 0x80);
    dprintf(1, "%s vendor='%s' product='%s' rev='%s' type=%d removable=%d\n"
            , s, vendor, product, rev, pdt, removable);
    drive->removable = removable;

    اگر (pdt == SCSI_TYPE_CDROM) {
        drive->blksize = CDROM_SECTOR_SIZE;
        drive->sectors = (u64)-1;

        نویسه *desc = znprintf(MAXDESCSIZE, "DVD/CD [%s Drive %s %s %s]"
                              , s, vendor, product, rev);
        boot_add_cd(drive, desc, prio);
        بازگشت 0;
    }

    اگر (pdt != SCSI_TYPE_DISK)
        بازگشت -1;

    ret = scsi_is_ready(&dop);
    اگر (ret) {
        dprintf(1, "scsi_is_ready returned %d\n", ret);
        بازگشت ret;
    }

    ساختار cdbres_read_capacity capdata;
    ret = cdb_read_capacity(&dop, &capdata);
    اگر (ret)
        بازگشت ret;

    اگر (be32_to_cpu(capdata.sectors) == 0xFFFFFFFFUL) {
        dprintf(3, "%s: >2TB Detected trying READCAP(16)\n", s);
        ساختار cdbres_read_capacity_16 capdata16;
        ret = cdb_read_capacity16(&dop, &capdata16);
        drive->blksize = be32_to_cpu(capdata16.blksize);
        drive->sectors = be64_to_cpu(capdata16.sectors) + 1;
    } وگرنه {
        drive->blksize = be32_to_cpu(capdata.blksize);
        drive->sectors = (u64)be32_to_cpu(capdata.sectors) + 1;
    }

    اگر (drive->blksize != DISK_SECTOR_SIZE) {
        dprintf(1, "%s: unsupported block size %d\n", s, drive->blksize);
        بازگشت -1;
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
        ساختار cdbres_mode_sense_geom geomdata;
        ret = cdb_mode_sense_geom(&dop, &geomdata);
        اگر (ret == 0) {
            u32 cylinders;
            cylinders = geomdata.cyl[0] << 16;
            cylinders |= geomdata.cyl[1] << 8;
            cylinders |= geomdata.cyl[2];
            اگر (cylinders && geomdata.heads &&
                drive->sectors <= 0xFFFFFFFFULL &&
                ((u32)drive->sectors % (geomdata.heads * cylinders) == 0)) {
                drive->pchs.cylinder = cylinders;
                drive->pchs.head = geomdata.heads;
                drive->pchs.sector = (u32)drive->sectors / (geomdata.heads * cylinders);
            }
        }
    }

    نویسه *desc = znprintf(MAXDESCSIZE, "%s Drive %s %s %s"
                          , s, vendor, product, rev);
    boot_add_hd(drive, desc, prio);
    بازگشت 0;
}

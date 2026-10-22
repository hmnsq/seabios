// Support for several common scsi like command data block requests
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "block.h" // struct disk_op_s
#include "blockcmd.h" // struct cdb_request_sense
#include "byteorder.h" // be32_to_cpu
#include "farptr.h" // GET_FLATPTR
#include "output.h" // dprintf
#include "std/disk.h" // DISK_RET_EPARAM
#include "string.h" // memset
#include "util.h" // timer_calc
#include "malloc.h"


/****************************************************************
 * Low level command requests
 ****************************************************************/

static целое
cdb_get_inquiry(struct disk_op_s *op, struct cdbres_inquiry *data)
{
    struct cdb_request_sense cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.command = CDB_CMD_INQUIRY;
    cmd.length = sizeof(*data);
    op->command = CMD_SCSI;
    op->count = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = sizeof(*data);
    вернуть process_op(op);
}

// Request SENSE
static целое
cdb_get_sense(struct disk_op_s *op, struct cdbres_request_sense *data)
{
    struct cdb_request_sense cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.command = CDB_CMD_REQUEST_SENSE;
    cmd.length = sizeof(*data);
    op->command = CMD_SCSI;
    op->count = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = sizeof(*data);
    вернуть process_op(op);
}

// Test unit ready
static целое
cdb_test_unit_ready(struct disk_op_s *op)
{
    struct cdb_request_sense cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.command = CDB_CMD_TEST_UNIT_READY;
    op->command = CMD_SCSI;
    op->count = 0;
    op->buf_fl = NULL;
    op->cdbcmd = &cmd;
    op->blocksize = 0;
    вернуть process_op(op);
}

static целое
cdb_read_capacity16(struct disk_op_s *op, struct cdbres_read_capacity_16 *data)
{
    struct cdb_sai_read_capacity_16 cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.command = CDB_CMD_SERVICE_ACTION_IN;
    cmd.flags = CDB_CMD_SAI_READ_CAPACITY_16;
    cmd.len = cpu_to_be32(sizeof(struct cdbres_read_capacity_16));
    op->command = CMD_SCSI;
    op->count = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = sizeof(*data);
    вернуть process_op(op);
}


// Request capacity
static целое
cdb_read_capacity(struct disk_op_s *op, struct cdbres_read_capacity *data)
{
    struct cdb_read_capacity cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.command = CDB_CMD_READ_CAPACITY;
    op->command = CMD_SCSI;
    op->count = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = sizeof(*data);
    вернуть process_op(op);
}

// Mode sense, geometry page.
static целое
cdb_mode_sense_geom(struct disk_op_s *op, struct cdbres_mode_sense_geom *data)
{
    struct cdb_mode_sense cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.command = CDB_CMD_MODE_SENSE;
    cmd.flags = 8; /* DBD */
    cmd.page = MODE_PAGE_HD_GEOMETRY;
    cmd.count = cpu_to_be16(sizeof(*data));
    op->command = CMD_SCSI;
    op->count = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = sizeof(*data);
    вернуть process_op(op);
}


/****************************************************************
 * Main SCSI commands
 ****************************************************************/

// Create a scsi command request from a disk_op_s request
целое
scsi_fill_cmd(struct disk_op_s *op, void *cdbcmd, целое maxcdb)
{
    switch (op->command) {
    case CMD_READ:
    case CMD_WRITE: ;
        если (op->lba < 0xFFFFFFFFULL) {
            struct cdb_rwdata_10 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_10
                            : CDB_CMD_WRITE_10);
            cmd->lba = cpu_to_be32(op->lba);
            cmd->count = cpu_to_be16(op->count);
        } иначе {
            struct cdb_rwdata_16 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_16
                            : CDB_CMD_WRITE_16);
            cmd->lba = cpu_to_be64(op->lba);
            cmd->count = cpu_to_be32(op->count);
        }
        вернуть GET_FLATPTR(op->drive_fl->blksize);
    case CMD_SCSI:
        если (MODESEGMENT)
            вернуть -1;
        memcpy(cdbcmd, op->cdbcmd, maxcdb);
        вернуть op->blocksize;
    default:
        вернуть -1;
    }
}

// Determine if the command is a request to pull data from the device
целое
scsi_is_read(struct disk_op_s *op)
{
    вернуть op->command == CMD_READ || (
        !MODESEGMENT && op->command == CMD_SCSI && op->blocksize);
}

// Check if a SCSI device is ready to receive commands
целое
scsi_is_ready(struct disk_op_s *op)
{
    ASSERT32FLAT();
    dprintf(6, "scsi_is_ready (drive=%p)\n", op->drive_fl);

    /* Retry TEST UNIT READY for 5 seconds unless MEDIUM NOT PRESENT is
     * reported by the device 3 times.  If the device reports "IN PROGRESS",
     * 30 seconds is added. */
    целое tries = 3;
    целое in_progress = 0;
    u32 end = timer_calc(5000);
    для (;;) {
        если (timer_check(end)) {
            dprintf(1, "test unit ready failed\n");
            вернуть -1;
        }

        целое ret = cdb_test_unit_ready(op);
        если (!ret)
            // Success
            break;

        struct cdbres_request_sense sense;
        ret = cdb_get_sense(op, &sense);
        если (ret)
            // Error - retry.
            continue;

        // Sense succeeded.
        если (sense.asc == 0x3a) { /* MEDIUM NOT PRESENT */
            tries--;
            dprintf(1, "Device reports MEDIUM NOT PRESENT - %d tries left\n",
                tries);
            если (!tries)
                вернуть -1;
        }

        если (sense.asc == 0x04 && sense.ascq == 0x01 && !in_progress) {
            /* IN PROGRESS OF BECOMING READY */
            dprintf(1, "Waiting for device to detect medium... ");
            /* Allow 30 seconds more */
            end = timer_calc(30000);
            in_progress = 1;
        }
    }
    вернуть 0;
}

#define CDB_CMD_REPORT_LUNS  0xA0

struct cdb_report_luns {
    u8 command;
    u8 reserved_01[5];
    u32 length;
    u8 pad[6];
} PACKED;

struct scsi_lun {
    u16 lun[4];
};

struct cdbres_report_luns {
    u32 length;
    u32 reserved;
    struct scsi_lun luns[];
};

static u64 scsilun2u64(struct scsi_lun *scsi_lun)
{
    целое i;
    u64 ret = 0;
    для (i = 0; i < ARRAY_SIZE(scsi_lun->lun); i++)
        ret |= be16_to_cpu(scsi_lun->lun[i]) << (16 * i);
    вернуть ret;
}

// Issue REPORT LUNS on a temporary drive and iterate reported luns calling
// @add_lun for each
целое scsi_rep_luns_scan(struct drive_s *tmp_drive, scsi_add_lun add_lun)
{
    целое ret = -1;
    /* start with the smallest possible buffer, otherwise some devices in QEMU
     * may (incorrectly) error out on returning less data than fits in it */
    u32 maxluns = 1;
    u32 nluns, i;
    struct cdb_report_luns cdb = {
        .command = CDB_CMD_REPORT_LUNS,
    };
    struct disk_op_s op = {
        .drive_fl = tmp_drive,
        .command = CMD_SCSI,
        .count = 1,
        .cdbcmd = &cdb,
    };
    struct cdbres_report_luns *resp;

    ASSERT32FLAT();

    пока (1) {
        op.blocksize = sizeof(struct cdbres_report_luns) +
            maxluns * sizeof(struct scsi_lun);
        op.buf_fl = malloc_tmp(op.blocksize);
        если (!op.buf_fl) {
            warn_noalloc();
            вернуть -1;
        }

        cdb.length = cpu_to_be32(op.blocksize);
        если (process_op(&op) != DISK_RET_SUCCESS)
            goto out;

        resp = op.buf_fl;
        nluns = be32_to_cpu(resp->length) / sizeof(struct scsi_lun);
        если (nluns <= maxluns)
            break;

        free(op.buf_fl);
        maxluns = nluns;
    }

    для (i = 0, ret = 0; i < nluns; i++) {
        u64 lun = scsilun2u64(&resp->luns[i]);
        если (lun >> 32)
            continue;
        ret += !add_lun((u32)lun, tmp_drive);
    }
out:
    free(op.buf_fl);
    вернуть ret;
}

// Iterate LUNs on the target and call @add_lun for each
целое scsi_sequential_scan(struct drive_s *tmp_drive, u32 maxluns,
                         scsi_add_lun add_lun)
{
    целое ret;
    u32 lun;

    для (lun = 0, ret = 0; lun < maxluns; lun++)
        ret += !add_lun(lun, tmp_drive);
    вернуть ret;
}

// Validate drive, find block size / sector count, and register drive.
целое
scsi_drive_setup(struct drive_s *drive, const char *s, целое prio)
{
    ASSERT32FLAT();
    struct disk_op_s dop;
    memset(&dop, 0, sizeof(dop));
    dop.drive_fl = drive;
    struct cdbres_inquiry data;
    целое ret = cdb_get_inquiry(&dop, &data);
    если (ret)
        вернуть ret;
    char vendor[sizeof(data.vendor)+1], product[sizeof(data.product)+1];
    char rev[sizeof(data.rev)+1];
    strtcpy(vendor, data.vendor, sizeof(vendor));
    nullTrailingSpace(vendor);
    strtcpy(product, data.product, sizeof(product));
    nullTrailingSpace(product);
    strtcpy(rev, data.rev, sizeof(rev));
    nullTrailingSpace(rev);
    целое pdt = data.pdt & 0x1f;
    целое removable = !!(data.removable & 0x80);
    dprintf(1, "%s vendor='%s' product='%s' rev='%s' type=%d removable=%d\n"
            , s, vendor, product, rev, pdt, removable);
    drive->removable = removable;

    если (pdt == SCSI_TYPE_CDROM) {
        drive->blksize = CDROM_SECTOR_SIZE;
        drive->sectors = (u64)-1;

        char *desc = znprintf(MAXDESCSIZE, "DVD/CD [%s Drive %s %s %s]"
                              , s, vendor, product, rev);
        boot_add_cd(drive, desc, prio);
        вернуть 0;
    }

    если (pdt != SCSI_TYPE_DISK)
        вернуть -1;

    ret = scsi_is_ready(&dop);
    если (ret) {
        dprintf(1, "scsi_is_ready returned %d\n", ret);
        вернуть ret;
    }

    struct cdbres_read_capacity capdata;
    ret = cdb_read_capacity(&dop, &capdata);
    если (ret)
        вернуть ret;

    если (be32_to_cpu(capdata.sectors) == 0xFFFFFFFFUL) {
        dprintf(3, "%s: >2TB Detected trying READCAP(16)\n", s);
        struct cdbres_read_capacity_16 capdata16;
        ret = cdb_read_capacity16(&dop, &capdata16);
        drive->blksize = be32_to_cpu(capdata16.blksize);
        drive->sectors = be64_to_cpu(capdata16.sectors) + 1;
    } иначе {
        drive->blksize = be32_to_cpu(capdata.blksize);
        drive->sectors = (u64)be32_to_cpu(capdata.sectors) + 1;
    }

    если (drive->blksize != DISK_SECTOR_SIZE) {
        dprintf(1, "%s: unsupported block size %d\n", s, drive->blksize);
        вернуть -1;
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
    если (CONFIG_QEMU_HARDWARE && memcmp(vendor, "QEMU", 5) == 0) {
        struct cdbres_mode_sense_geom geomdata;
        ret = cdb_mode_sense_geom(&dop, &geomdata);
        если (ret == 0) {
            u32 cylinders;
            cylinders = geomdata.cyl[0] << 16;
            cylinders |= geomdata.cyl[1] << 8;
            cylinders |= geomdata.cyl[2];
            если (cylinders && geomdata.heads &&
                drive->sectors <= 0xFFFFFFFFULL &&
                ((u32)drive->sectors % (geomdata.heads * cylinders) == 0)) {
                drive->pchs.cylinder = cylinders;
                drive->pchs.head = geomdata.heads;
                drive->pchs.sector = (u32)drive->sectors / (geomdata.heads * cylinders);
            }
        }
    }

    char *desc = znprintf(MAXDESCSIZE, "%s Drive %s %s %s"
                          , s, vendor, product, rev);
    boot_add_hd(drive, desc, prio);
    вернуть 0;
}

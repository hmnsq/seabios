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

static ཧྲིལ
cdb_get_inquiry(sa_nracanaa disk_op_s *op, sa_nracanaa cdbres_inquiry *data)
{
    sa_nracanaa cdb_request_sense cmd;
    memset(&cmd, 0, parimaa_na(cmd));
    cmd.command = CDB_CMD_INQUIRY;
    cmd.length = parimaa_na(*data);
    op->command = CMD_SCSI;
    op->count = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = parimaa_na(*data);
    ཕྱིར process_op(op);
}

// Request SENSE
static ཧྲིལ
cdb_get_sense(sa_nracanaa disk_op_s *op, sa_nracanaa cdbres_request_sense *data)
{
    sa_nracanaa cdb_request_sense cmd;
    memset(&cmd, 0, parimaa_na(cmd));
    cmd.command = CDB_CMD_REQUEST_SENSE;
    cmd.length = parimaa_na(*data);
    op->command = CMD_SCSI;
    op->count = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = parimaa_na(*data);
    ཕྱིར process_op(op);
}

// Test unit ready
static ཧྲིལ
cdb_test_unit_ready(sa_nracanaa disk_op_s *op)
{
    sa_nracanaa cdb_request_sense cmd;
    memset(&cmd, 0, parimaa_na(cmd));
    cmd.command = CDB_CMD_TEST_UNIT_READY;
    op->command = CMD_SCSI;
    op->count = 0;
    op->buf_fl = NULL;
    op->cdbcmd = &cmd;
    op->blocksize = 0;
    ཕྱིར process_op(op);
}

static ཧྲིལ
cdb_read_capacity16(sa_nracanaa disk_op_s *op, sa_nracanaa cdbres_read_capacity_16 *data)
{
    sa_nracanaa cdb_sai_read_capacity_16 cmd;
    memset(&cmd, 0, parimaa_na(cmd));
    cmd.command = CDB_CMD_SERVICE_ACTION_IN;
    cmd.flags = CDB_CMD_SAI_READ_CAPACITY_16;
    cmd.len = cpu_to_be32(parimaa_na(sa_nracanaa cdbres_read_capacity_16));
    op->command = CMD_SCSI;
    op->count = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = parimaa_na(*data);
    ཕྱིར process_op(op);
}


// Request capacity
static ཧྲིལ
cdb_read_capacity(sa_nracanaa disk_op_s *op, sa_nracanaa cdbres_read_capacity *data)
{
    sa_nracanaa cdb_read_capacity cmd;
    memset(&cmd, 0, parimaa_na(cmd));
    cmd.command = CDB_CMD_READ_CAPACITY;
    op->command = CMD_SCSI;
    op->count = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = parimaa_na(*data);
    ཕྱིར process_op(op);
}

// Mode sense, geometry page.
static ཧྲིལ
cdb_mode_sense_geom(sa_nracanaa disk_op_s *op, sa_nracanaa cdbres_mode_sense_geom *data)
{
    sa_nracanaa cdb_mode_sense cmd;
    memset(&cmd, 0, parimaa_na(cmd));
    cmd.command = CDB_CMD_MODE_SENSE;
    cmd.flags = 8; /* DBD */
    cmd.page = MODE_PAGE_HD_GEOMETRY;
    cmd.count = cpu_to_be16(parimaa_na(*data));
    op->command = CMD_SCSI;
    op->count = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = parimaa_na(*data);
    ཕྱིར process_op(op);
}


/****************************************************************
 * Main SCSI commands
 ****************************************************************/

// Create a scsi command request from a disk_op_s request
ཧྲིལ
scsi_fill_cmd(sa_nracanaa disk_op_s *op, void *cdbcmd, ཧྲིལ maxcdb)
{
    switch (op->command) {
    pak_sa CMD_READ:
    pak_sa CMD_WRITE: ;
        གལ་ཏེ (op->lba < 0xFFFFFFFFULL) {
            sa_nracanaa cdb_rwdata_10 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_10
                            : CDB_CMD_WRITE_10);
            cmd->lba = cpu_to_be32(op->lba);
            cmd->count = cpu_to_be16(op->count);
        } དེ་མིན {
            sa_nracanaa cdb_rwdata_16 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_16
                            : CDB_CMD_WRITE_16);
            cmd->lba = cpu_to_be64(op->lba);
            cmd->count = cpu_to_be32(op->count);
        }
        ཕྱིར GET_FLATPTR(op->drive_fl->blksize);
    pak_sa CMD_SCSI:
        གལ་ཏེ (MODESEGMENT)
            ཕྱིར -1;
        memcpy(cdbcmd, op->cdbcmd, maxcdb);
        ཕྱིར op->blocksize;
    default:
        ཕྱིར -1;
    }
}

// Determine if the command is a request to pull data from the device
ཧྲིལ
scsi_is_read(sa_nracanaa disk_op_s *op)
{
    ཕྱིར op->command == CMD_READ || (
        !MODESEGMENT && op->command == CMD_SCSI && op->blocksize);
}

// Check if a SCSI device is ready to receive commands
ཧྲིལ
scsi_is_ready(sa_nracanaa disk_op_s *op)
{
    ASSERT32FLAT();
    dprintf(6, "scsi_is_ready (drive=%p)\n", op->drive_fl);

    /* Retry TEST UNIT READY for 5 seconds unless MEDIUM NOT PRESENT is
     * reported by the device 3 times.  If the device reports "IN PROGRESS",
     * 30 seconds is added. */
    ཧྲིལ tries = 3;
    ཧྲིལ in_progress = 0;
    u32 end = timer_calc(5000);
    ཆེད་དུ (;;) {
        གལ་ཏེ (timer_check(end)) {
            dprintf(1, "test unit ready failed\n");
            ཕྱིར -1;
        }

        ཧྲིལ ret = cdb_test_unit_ready(op);
        གལ་ཏེ (!ret)
            // Success
            bha_nga;

        sa_nracanaa cdbres_request_sense sense;
        ret = cdb_get_sense(op, &sense);
        གལ་ཏེ (ret)
            // Error - retry.
            continue;

        // Sense succeeded.
        གལ་ཏེ (sense.asc == 0x3a) { /* MEDIUM NOT PRESENT */
            tries--;
            dprintf(1, "Device reports MEDIUM NOT PRESENT - %d tries left\n",
                tries);
            གལ་ཏེ (!tries)
                ཕྱིར -1;
        }

        གལ་ཏེ (sense.asc == 0x04 && sense.ascq == 0x01 && !in_progress) {
            /* IN PROGRESS OF BECOMING READY */
            dprintf(1, "Waiting for device to detect medium... ");
            /* Allow 30 seconds more */
            end = timer_calc(30000);
            in_progress = 1;
        }
    }
    ཕྱིར 0;
}

#define CDB_CMD_REPORT_LUNS  0xA0

sa_nracanaa cdb_report_luns {
    u8 command;
    u8 reserved_01[5];
    u32 length;
    u8 pad[6];
} PACKED;

sa_nracanaa scsi_lun {
    u16 lun[4];
};

sa_nracanaa cdbres_report_luns {
    u32 length;
    u32 reserved;
    sa_nracanaa scsi_lun luns[];
};

static u64 scsilun2u64(sa_nracanaa scsi_lun *scsi_lun)
{
    ཧྲིལ i;
    u64 ret = 0;
    ཆེད་དུ (i = 0; i < ARRAY_SIZE(scsi_lun->lun); i++)
        ret |= be16_to_cpu(scsi_lun->lun[i]) << (16 * i);
    ཕྱིར ret;
}

// Issue REPORT LUNS on a temporary drive and iterate reported luns calling
// @add_lun for each
ཧྲིལ scsi_rep_luns_scan(sa_nracanaa drive_s *tmp_drive, scsi_add_lun add_lun)
{
    ཧྲིལ ret = -1;
    /* start with the smallest possible buffer, otherwise some devices in QEMU
     * may (incorrectly) error out on returning less data than fits in it */
    u32 maxluns = 1;
    u32 nluns, i;
    sa_nracanaa cdb_report_luns cdb = {
        .command = CDB_CMD_REPORT_LUNS,
    };
    sa_nracanaa disk_op_s op = {
        .drive_fl = tmp_drive,
        .command = CMD_SCSI,
        .count = 1,
        .cdbcmd = &cdb,
    };
    sa_nracanaa cdbres_report_luns *resp;

    ASSERT32FLAT();

    སྐབས (1) {
        op.blocksize = parimaa_na(sa_nracanaa cdbres_report_luns) +
            maxluns * parimaa_na(sa_nracanaa scsi_lun);
        op.buf_fl = malloc_tmp(op.blocksize);
        གལ་ཏེ (!op.buf_fl) {
            warn_noalloc();
            ཕྱིར -1;
        }

        cdb.length = cpu_to_be32(op.blocksize);
        གལ་ཏེ (process_op(&op) != DISK_RET_SUCCESS)
            goto out;

        resp = op.buf_fl;
        nluns = be32_to_cpu(resp->length) / parimaa_na(sa_nracanaa scsi_lun);
        གལ་ཏེ (nluns <= maxluns)
            bha_nga;

        free(op.buf_fl);
        maxluns = nluns;
    }

    ཆེད་དུ (i = 0, ret = 0; i < nluns; i++) {
        u64 lun = scsilun2u64(&resp->luns[i]);
        གལ་ཏེ (lun >> 32)
            continue;
        ret += !add_lun((u32)lun, tmp_drive);
    }
out:
    free(op.buf_fl);
    ཕྱིར ret;
}

// Iterate LUNs on the target and call @add_lun for each
ཧྲིལ scsi_sequential_scan(sa_nracanaa drive_s *tmp_drive, u32 maxluns,
                         scsi_add_lun add_lun)
{
    ཧྲིལ ret;
    u32 lun;

    ཆེད་དུ (lun = 0, ret = 0; lun < maxluns; lun++)
        ret += !add_lun(lun, tmp_drive);
    ཕྱིར ret;
}

// Validate drive, find block size / sector count, and register drive.
ཧྲིལ
scsi_drive_setup(sa_nracanaa drive_s *drive, const var_na *s, ཧྲིལ prio)
{
    ASSERT32FLAT();
    sa_nracanaa disk_op_s dop;
    memset(&dop, 0, parimaa_na(dop));
    dop.drive_fl = drive;
    sa_nracanaa cdbres_inquiry data;
    ཧྲིལ ret = cdb_get_inquiry(&dop, &data);
    གལ་ཏེ (ret)
        ཕྱིར ret;
    var_na vendor[parimaa_na(data.vendor)+1], product[parimaa_na(data.product)+1];
    var_na rev[parimaa_na(data.rev)+1];
    strtcpy(vendor, data.vendor, parimaa_na(vendor));
    nullTrailingSpace(vendor);
    strtcpy(product, data.product, parimaa_na(product));
    nullTrailingSpace(product);
    strtcpy(rev, data.rev, parimaa_na(rev));
    nullTrailingSpace(rev);
    ཧྲིལ pdt = data.pdt & 0x1f;
    ཧྲིལ removable = !!(data.removable & 0x80);
    dprintf(1, "%s vendor='%s' product='%s' rev='%s' type=%d removable=%d\n"
            , s, vendor, product, rev, pdt, removable);
    drive->removable = removable;

    གལ་ཏེ (pdt == SCSI_TYPE_CDROM) {
        drive->blksize = CDROM_SECTOR_SIZE;
        drive->sectors = (u64)-1;

        var_na *desc = znprintf(MAXDESCSIZE, "DVD/CD [%s Drive %s %s %s]"
                              , s, vendor, product, rev);
        boot_add_cd(drive, desc, prio);
        ཕྱིར 0;
    }

    གལ་ཏེ (pdt != SCSI_TYPE_DISK)
        ཕྱིར -1;

    ret = scsi_is_ready(&dop);
    གལ་ཏེ (ret) {
        dprintf(1, "scsi_is_ready returned %d\n", ret);
        ཕྱིར ret;
    }

    sa_nracanaa cdbres_read_capacity capdata;
    ret = cdb_read_capacity(&dop, &capdata);
    གལ་ཏེ (ret)
        ཕྱིར ret;

    གལ་ཏེ (be32_to_cpu(capdata.sectors) == 0xFFFFFFFFUL) {
        dprintf(3, "%s: >2TB Detected trying READCAP(16)\n", s);
        sa_nracanaa cdbres_read_capacity_16 capdata16;
        ret = cdb_read_capacity16(&dop, &capdata16);
        drive->blksize = be32_to_cpu(capdata16.blksize);
        drive->sectors = be64_to_cpu(capdata16.sectors) + 1;
    } དེ་མིན {
        drive->blksize = be32_to_cpu(capdata.blksize);
        drive->sectors = (u64)be32_to_cpu(capdata.sectors) + 1;
    }

    གལ་ཏེ (drive->blksize != DISK_SECTOR_SIZE) {
        dprintf(1, "%s: unsupported block size %d\n", s, drive->blksize);
        ཕྱིར -1;
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
    གལ་ཏེ (CONFIG_QEMU_HARDWARE && memcmp(vendor, "QEMU", 5) == 0) {
        sa_nracanaa cdbres_mode_sense_geom geomdata;
        ret = cdb_mode_sense_geom(&dop, &geomdata);
        གལ་ཏེ (ret == 0) {
            u32 cylinders;
            cylinders = geomdata.cyl[0] << 16;
            cylinders |= geomdata.cyl[1] << 8;
            cylinders |= geomdata.cyl[2];
            གལ་ཏེ (cylinders && geomdata.heads &&
                drive->sectors <= 0xFFFFFFFFULL &&
                ((u32)drive->sectors % (geomdata.heads * cylinders) == 0)) {
                drive->pchs.cylinder = cylinders;
                drive->pchs.head = geomdata.heads;
                drive->pchs.sector = (u32)drive->sectors / (geomdata.heads * cylinders);
            }
        }
    }

    var_na *desc = znprintf(MAXDESCSIZE, "%s Drive %s %s %s"
                          , s, vendor, product, rev);
    boot_add_hd(drive, desc, prio);
    ཕྱིར 0;
}

// Support for several common scsi like command data block requests
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "block.h" // struct disk_op_s
#ಅನ್ತರ್ಭಾವ "blockcmd.h" // struct cdb_request_sense
#ಅನ್ತರ್ಭಾವ "byteorder.h" // be32_to_cpu
#ಅನ್ತರ್ಭಾವ "farptr.h" // GET_FLATPTR
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_EPARAM
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // timer_calc
#ಅನ್ತರ್ಭಾವ "malloc.h"


/****************************************************************
 * Low level command requests
 ****************************************************************/

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
cdb_get_inquiry(ಸಂರಚನಾ disk_op_s *op, ಸಂರಚನಾ cdbres_inquiry *data)
{
    ಸಂರಚನಾ cdb_request_sense cmd;
    memset(&cmd, 0, ಪರಿಮಾಣ(cmd));
    cmd.command = CDB_CMD_INQUIRY;
    cmd.length = ಪರಿಮಾಣ(*data);
    op->command = CMD_SCSI;
    op->ಎಣಿಕೆ = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = ಪರಿಮಾಣ(*data);
    ಪ್ರತ್ಯಾಗಮ process_op(op);
}

// Request SENSE
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
cdb_get_sense(ಸಂರಚನಾ disk_op_s *op, ಸಂರಚನಾ cdbres_request_sense *data)
{
    ಸಂರಚನಾ cdb_request_sense cmd;
    memset(&cmd, 0, ಪರಿಮಾಣ(cmd));
    cmd.command = CDB_CMD_REQUEST_SENSE;
    cmd.length = ಪರಿಮಾಣ(*data);
    op->command = CMD_SCSI;
    op->ಎಣಿಕೆ = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = ಪರಿಮಾಣ(*data);
    ಪ್ರತ್ಯಾಗಮ process_op(op);
}

// Test unit ready
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
cdb_test_unit_ready(ಸಂರಚನಾ disk_op_s *op)
{
    ಸಂರಚನಾ cdb_request_sense cmd;
    memset(&cmd, 0, ಪರಿಮಾಣ(cmd));
    cmd.command = CDB_CMD_TEST_UNIT_READY;
    op->command = CMD_SCSI;
    op->ಎಣಿಕೆ = 0;
    op->buf_fl = ರಿಕ್ತ;
    op->cdbcmd = &cmd;
    op->blocksize = 0;
    ಪ್ರತ್ಯಾಗಮ process_op(op);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
cdb_read_capacity16(ಸಂರಚನಾ disk_op_s *op, ಸಂರಚನಾ cdbres_read_capacity_16 *data)
{
    ಸಂರಚನಾ cdb_sai_read_capacity_16 cmd;
    memset(&cmd, 0, ಪರಿಮಾಣ(cmd));
    cmd.command = CDB_CMD_SERVICE_ACTION_IN;
    cmd.flags = CDB_CMD_SAI_READ_CAPACITY_16;
    cmd.len = cpu_to_be32(ಪರಿಮಾಣ(ಸಂರಚನಾ cdbres_read_capacity_16));
    op->command = CMD_SCSI;
    op->ಎಣಿಕೆ = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = ಪರಿಮಾಣ(*data);
    ಪ್ರತ್ಯಾಗಮ process_op(op);
}


// Request capacity
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
cdb_read_capacity(ಸಂರಚನಾ disk_op_s *op, ಸಂರಚನಾ cdbres_read_capacity *data)
{
    ಸಂರಚನಾ cdb_read_capacity cmd;
    memset(&cmd, 0, ಪರಿಮಾಣ(cmd));
    cmd.command = CDB_CMD_READ_CAPACITY;
    op->command = CMD_SCSI;
    op->ಎಣಿಕೆ = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = ಪರಿಮಾಣ(*data);
    ಪ್ರತ್ಯಾಗಮ process_op(op);
}

// Mode sense, geometry page.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
cdb_mode_sense_geom(ಸಂರಚನಾ disk_op_s *op, ಸಂರಚನಾ cdbres_mode_sense_geom *data)
{
    ಸಂರಚನಾ cdb_mode_sense cmd;
    memset(&cmd, 0, ಪರಿಮಾಣ(cmd));
    cmd.command = CDB_CMD_MODE_SENSE;
    cmd.flags = 8; /* DBD */
    cmd.page = MODE_PAGE_HD_GEOMETRY;
    cmd.ಎಣಿಕೆ = cpu_to_be16(ಪರಿಮಾಣ(*data));
    op->command = CMD_SCSI;
    op->ಎಣಿಕೆ = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = ಪರಿಮಾಣ(*data);
    ಪ್ರತ್ಯಾಗಮ process_op(op);
}


/****************************************************************
 * Main SCSI commands
 ****************************************************************/

// Create a scsi command request from a disk_op_s request
ಪೂರ್ಣಾಙ್ಕ
scsi_fill_cmd(ಸಂರಚನಾ disk_op_s *op, ಶೂನ್ಯ *cdbcmd, ಪೂರ್ಣಾಙ್ಕ maxcdb)
{
    ವಿಕಲ್ಪ (op->command) {
    ಪಕ್ಷ CMD_READ:
    ಪಕ್ಷ CMD_WRITE: ;
        ಯದಿ (op->lba < 0xFFFFFFFFULL) {
            ಸಂರಚನಾ cdb_rwdata_10 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_10
                            : CDB_CMD_WRITE_10);
            cmd->lba = cpu_to_be32(op->lba);
            cmd->ಎಣಿಕೆ = cpu_to_be16(op->ಎಣಿಕೆ);
        } ಅನ್ಯಥಾ {
            ಸಂರಚನಾ cdb_rwdata_16 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_16
                            : CDB_CMD_WRITE_16);
            cmd->lba = cpu_to_be64(op->lba);
            cmd->ಎಣಿಕೆ = cpu_to_be32(op->ಎಣಿಕೆ);
        }
        ಪ್ರತ್ಯಾಗಮ GET_FLATPTR(op->drive_fl->blksize);
    ಪಕ್ಷ CMD_SCSI:
        ಯದಿ (MODESEGMENT)
            ಪ್ರತ್ಯಾಗಮ -1;
        memcpy(cdbcmd, op->cdbcmd, maxcdb);
        ಪ್ರತ್ಯಾಗಮ op->blocksize;
    ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ -1;
    }
}

// Determine if the command is a request to pull data from the device
ಪೂರ್ಣಾಙ್ಕ
scsi_is_read(ಸಂರಚನಾ disk_op_s *op)
{
    ಪ್ರತ್ಯಾಗಮ op->command == CMD_READ || (
        !MODESEGMENT && op->command == CMD_SCSI && op->blocksize);
}

// Check if a SCSI device is ready to receive commands
ಪೂರ್ಣಾಙ್ಕ
scsi_is_ready(ಸಂರಚನಾ disk_op_s *op)
{
    ASSERT32FLAT();
    dprintf(6, "scsi_is_ready (drive=%p)\n", op->drive_fl);

    /* Retry TEST UNIT READY for 5 seconds unless MEDIUM NOT PRESENT is
     * reported by the device 3 times.  If the device reports "IN PROGRESS",
     * 30 seconds is added. */
    ಪೂರ್ಣಾಙ್ಕ tries = 3;
    ಪೂರ್ಣಾಙ್ಕ in_progress = 0;
    u32 end = timer_calc(5000);
    ಪ್ರತಿ (;;) {
        ಯದಿ (timer_check(end)) {
            dprintf(1, "test unit ready failed\n");
            ಪ್ರತ್ಯಾಗಮ -1;
        }

        ಪೂರ್ಣಾಙ್ಕ ret = cdb_test_unit_ready(op);
        ಯದಿ (!ret)
            // Success
            ಭಙ್ಗ;

        ಸಂರಚನಾ cdbres_request_sense sense;
        ret = cdb_get_sense(op, &sense);
        ಯದಿ (ret)
            // Error - retry.
            ಅನುವರ್ತ;

        // Sense succeeded.
        ಯದಿ (sense.asc == 0x3a) { /* MEDIUM NOT PRESENT */
            tries--;
            dprintf(1, "Device reports MEDIUM NOT PRESENT - %d tries left\n",
                tries);
            ಯದಿ (!tries)
                ಪ್ರತ್ಯಾಗಮ -1;
        }

        ಯದಿ (sense.asc == 0x04 && sense.ascq == 0x01 && !in_progress) {
            /* IN PROGRESS OF BECOMING READY */
            dprintf(1, "Waiting for device to detect medium... ");
            /* Allow 30 seconds more */
            end = timer_calc(30000);
            in_progress = 1;
        }
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}

#define CDB_CMD_REPORT_LUNS  0xA0

ಸಂರಚನಾ cdb_report_luns {
    u8 command;
    u8 reserved_01[5];
    u32 length;
    u8 pad[6];
} PACKED;

ಸಂರಚನಾ scsi_lun {
    u16 lun[4];
};

ಸಂರಚನಾ cdbres_report_luns {
    u32 length;
    u32 reserved;
    ಸಂರಚನಾ scsi_lun luns[];
};

ಸ್ಥಿರ u64 scsilun2u64(ಸಂರಚನಾ scsi_lun *scsi_lun)
{
    ಪೂರ್ಣಾಙ್ಕ i;
    u64 ret = 0;
    ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(scsi_lun->lun); i++)
        ret |= be16_to_cpu(scsi_lun->lun[i]) << (16 * i);
    ಪ್ರತ್ಯಾಗಮ ret;
}

// Issue REPORT LUNS on a temporary drive and iterate reported luns calling
// @add_lun for each
ಪೂರ್ಣಾಙ್ಕ scsi_rep_luns_scan(ಸಂರಚನಾ drive_s *tmp_drive, scsi_add_lun add_lun)
{
    ಪೂರ್ಣಾಙ್ಕ ret = -1;
    /* start with the smallest possible buffer, otherwise some devices in QEMU
     * may (incorrectly) error out on returning less data than fits in it */
    u32 maxluns = 1;
    u32 nluns, i;
    ಸಂರಚನಾ cdb_report_luns cdb = {
        .command = CDB_CMD_REPORT_LUNS,
    };
    ಸಂರಚನಾ disk_op_s op = {
        .drive_fl = tmp_drive,
        .command = CMD_SCSI,
        .ಎಣಿಕೆ = 1,
        .cdbcmd = &cdb,
    };
    ಸಂರಚನಾ cdbres_report_luns *resp;

    ASSERT32FLAT();

    ಯಾವತ್ (1) {
        op.blocksize = ಪರಿಮಾಣ(ಸಂರಚನಾ cdbres_report_luns) +
            maxluns * ಪರಿಮಾಣ(ಸಂರಚನಾ scsi_lun);
        op.buf_fl = malloc_tmp(op.blocksize);
        ಯದಿ (!op.buf_fl) {
            warn_noalloc();
            ಪ್ರತ್ಯಾಗಮ -1;
        }

        cdb.length = cpu_to_be32(op.blocksize);
        ಯದಿ (process_op(&op) != DISK_RET_SUCCESS)
            ಗಚ್ಛ out;

        resp = op.buf_fl;
        nluns = be32_to_cpu(resp->length) / ಪರಿಮಾಣ(ಸಂರಚನಾ scsi_lun);
        ಯದಿ (nluns <= maxluns)
            ಭಙ್ಗ;

        ಮುಕ್ತ(op.buf_fl);
        maxluns = nluns;
    }

    ಪ್ರತಿ (i = 0, ret = 0; i < nluns; i++) {
        u64 lun = scsilun2u64(&resp->luns[i]);
        ಯದಿ (lun >> 32)
            ಅನುವರ್ತ;
        ret += !add_lun((u32)lun, tmp_drive);
    }
out:
    ಮುಕ್ತ(op.buf_fl);
    ಪ್ರತ್ಯಾಗಮ ret;
}

// Iterate LUNs on the target and call @add_lun for each
ಪೂರ್ಣಾಙ್ಕ scsi_sequential_scan(ಸಂರಚನಾ drive_s *tmp_drive, u32 maxluns,
                         scsi_add_lun add_lun)
{
    ಪೂರ್ಣಾಙ್ಕ ret;
    u32 lun;

    ಪ್ರತಿ (lun = 0, ret = 0; lun < maxluns; lun++)
        ret += !add_lun(lun, tmp_drive);
    ಪ್ರತ್ಯಾಗಮ ret;
}

// Validate drive, find block size / sector count, and register drive.
ಪೂರ್ಣಾಙ್ಕ
scsi_drive_setup(ಸಂರಚನಾ drive_s *drive, ಧ್ರುವ ವರ್ಣ *s, ಪೂರ್ಣಾಙ್ಕ prio)
{
    ASSERT32FLAT();
    ಸಂರಚನಾ disk_op_s dop;
    memset(&dop, 0, ಪರಿಮಾಣ(dop));
    dop.drive_fl = drive;
    ಸಂರಚನಾ cdbres_inquiry data;
    ಪೂರ್ಣಾಙ್ಕ ret = cdb_get_inquiry(&dop, &data);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ret;
    ವರ್ಣ vendor[ಪರಿಮಾಣ(data.vendor)+1], product[ಪರಿಮಾಣ(data.product)+1];
    ವರ್ಣ rev[ಪರಿಮಾಣ(data.rev)+1];
    strtcpy(vendor, data.vendor, ಪರಿಮಾಣ(vendor));
    nullTrailingSpace(vendor);
    strtcpy(product, data.product, ಪರಿಮಾಣ(product));
    nullTrailingSpace(product);
    strtcpy(rev, data.rev, ಪರಿಮಾಣ(rev));
    nullTrailingSpace(rev);
    ಪೂರ್ಣಾಙ್ಕ pdt = data.pdt & 0x1f;
    ಪೂರ್ಣಾಙ್ಕ removable = !!(data.removable & 0x80);
    dprintf(1, "%s vendor='%s' product='%s' rev='%s' type=%d removable=%d\n"
            , s, vendor, product, rev, pdt, removable);
    drive->removable = removable;

    ಯದಿ (pdt == SCSI_TYPE_CDROM) {
        drive->blksize = CDROM_SECTOR_SIZE;
        drive->sectors = (u64)-1;

        ವರ್ಣ *desc = znprintf(MAXDESCSIZE, "DVD/CD [%s Drive %s %s %s]"
                              , s, vendor, product, rev);
        boot_add_cd(drive, desc, prio);
        ಪ್ರತ್ಯಾಗಮ 0;
    }

    ಯದಿ (pdt != SCSI_TYPE_DISK)
        ಪ್ರತ್ಯಾಗಮ -1;

    ret = scsi_is_ready(&dop);
    ಯದಿ (ret) {
        dprintf(1, "scsi_is_ready returned %d\n", ret);
        ಪ್ರತ್ಯಾಗಮ ret;
    }

    ಸಂರಚನಾ cdbres_read_capacity capdata;
    ret = cdb_read_capacity(&dop, &capdata);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ret;

    ಯದಿ (be32_to_cpu(capdata.sectors) == 0xFFFFFFFFUL) {
        dprintf(3, "%s: >2TB Detected trying READCAP(16)\n", s);
        ಸಂರಚನಾ cdbres_read_capacity_16 capdata16;
        ret = cdb_read_capacity16(&dop, &capdata16);
        drive->blksize = be32_to_cpu(capdata16.blksize);
        drive->sectors = be64_to_cpu(capdata16.sectors) + 1;
    } ಅನ್ಯಥಾ {
        drive->blksize = be32_to_cpu(capdata.blksize);
        drive->sectors = (u64)be32_to_cpu(capdata.sectors) + 1;
    }

    ಯದಿ (drive->blksize != DISK_SECTOR_SIZE) {
        dprintf(1, "%s: unsupported block size %d\n", s, drive->blksize);
        ಪ್ರತ್ಯಾಗಮ -1;
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
    ಯದಿ (CONFIG_QEMU_HARDWARE && memcmp(vendor, "QEMU", 5) == 0) {
        ಸಂರಚನಾ cdbres_mode_sense_geom geomdata;
        ret = cdb_mode_sense_geom(&dop, &geomdata);
        ಯದಿ (ret == 0) {
            u32 cylinders;
            cylinders = geomdata.cyl[0] << 16;
            cylinders |= geomdata.cyl[1] << 8;
            cylinders |= geomdata.cyl[2];
            ಯದಿ (cylinders && geomdata.heads &&
                drive->sectors <= 0xFFFFFFFFULL &&
                ((u32)drive->sectors % (geomdata.heads * cylinders) == 0)) {
                drive->pchs.cylinder = cylinders;
                drive->pchs.head = geomdata.heads;
                drive->pchs.sector = (u32)drive->sectors / (geomdata.heads * cylinders);
            }
        }
    }

    ವರ್ಣ *desc = znprintf(MAXDESCSIZE, "%s Drive %s %s %s"
                          , s, vendor, product, rev);
    boot_add_hd(drive, desc, prio);
    ಪ್ರತ್ಯಾಗಮ 0;
}

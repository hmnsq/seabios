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

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
cdb_get_inquiry(ರಚನೆ disk_op_s *op, ರಚನೆ cdbres_inquiry *ದತ್ತಾಂಶ)
{
    ರಚನೆ cdb_request_sense cmd;
    memset(&cmd, 0, ಗಾತ್ರ(cmd));
    cmd.command = CDB_CMD_INQUIRY;
    cmd.ಉದ್ದಳತೆ = ಗಾತ್ರ(*ದತ್ತಾಂಶ);
    op->command = CMD_SCSI;
    op->ಎಣಿಕೆ_2 = 1;
    op->buf_fl = ದತ್ತಾಂಶ;
    op->cdbcmd = &cmd;
    op->blocksize = ಗಾತ್ರ(*ದತ್ತಾಂಶ);
    ಹಿಂತಿರುಗಿ process_op(op);
}

// Request SENSE
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
cdb_get_sense(ರಚನೆ disk_op_s *op, ರಚನೆ cdbres_request_sense *ದತ್ತಾಂಶ)
{
    ರಚನೆ cdb_request_sense cmd;
    memset(&cmd, 0, ಗಾತ್ರ(cmd));
    cmd.command = CDB_CMD_REQUEST_SENSE;
    cmd.ಉದ್ದಳತೆ = ಗಾತ್ರ(*ದತ್ತಾಂಶ);
    op->command = CMD_SCSI;
    op->ಎಣಿಕೆ_2 = 1;
    op->buf_fl = ದತ್ತಾಂಶ;
    op->cdbcmd = &cmd;
    op->blocksize = ಗಾತ್ರ(*ದತ್ತಾಂಶ);
    ಹಿಂತಿರುಗಿ process_op(op);
}

// Test unit ready
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
cdb_test_unit_ready(ರಚನೆ disk_op_s *op)
{
    ರಚನೆ cdb_request_sense cmd;
    memset(&cmd, 0, ಗಾತ್ರ(cmd));
    cmd.command = CDB_CMD_TEST_UNIT_READY;
    op->command = CMD_SCSI;
    op->ಎಣಿಕೆ_2 = 0;
    op->buf_fl = ಖಾಲಿ;
    op->cdbcmd = &cmd;
    op->blocksize = 0;
    ಹಿಂತಿರುಗಿ process_op(op);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
cdb_read_capacity16(ರಚನೆ disk_op_s *op, ರಚನೆ cdbres_read_ಸಾಮರ್ಥ್ಯ_16 *ದತ್ತಾಂಶ)
{
    ರಚನೆ cdb_sai_read_ಸಾಮರ್ಥ್ಯ_16 cmd;
    memset(&cmd, 0, ಗಾತ್ರ(cmd));
    cmd.command = CDB_CMD_SERVICE_ACTION_ಒಳ;
    cmd.flags = CDB_CMD_SAI_READ_ಸಾಮರ್ಥ್ಯ_16;
    cmd.len = cpu_ಗಮ್ಯ_be32(ಗಾತ್ರ(ರಚನೆ cdbres_read_ಸಾಮರ್ಥ್ಯ_16));
    op->command = CMD_SCSI;
    op->ಎಣಿಕೆ_2 = 1;
    op->buf_fl = ದತ್ತಾಂಶ;
    op->cdbcmd = &cmd;
    op->blocksize = ಗಾತ್ರ(*ದತ್ತಾಂಶ);
    ಹಿಂತಿರುಗಿ process_op(op);
}


// Request capacity
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
cdb_read_ಸಾಮರ್ಥ್ಯ(ರಚನೆ disk_op_s *op, ರಚನೆ cdbres_read_ಸಾಮರ್ಥ್ಯ *ದತ್ತಾಂಶ)
{
    ರಚನೆ cdb_read_ಸಾಮರ್ಥ್ಯ cmd;
    memset(&cmd, 0, ಗಾತ್ರ(cmd));
    cmd.command = CDB_CMD_READ_ಸಾಮರ್ಥ್ಯ;
    op->command = CMD_SCSI;
    op->ಎಣಿಕೆ_2 = 1;
    op->buf_fl = ದತ್ತಾಂಶ;
    op->cdbcmd = &cmd;
    op->blocksize = ಗಾತ್ರ(*ದತ್ತಾಂಶ);
    ಹಿಂತಿರುಗಿ process_op(op);
}

// Mode sense, geometry page.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
cdb_ಬಹುಲಕ_sense_geom(ರಚನೆ disk_op_s *op, ರಚನೆ cdbres_ಬಹುಲಕ_sense_geom *ದತ್ತಾಂಶ)
{
    ರಚನೆ cdb_ಬಹುಲಕ_sense cmd;
    memset(&cmd, 0, ಗಾತ್ರ(cmd));
    cmd.command = CDB_CMD_ಬಹುಲಕ_SENSE;
    cmd.flags = 8; /* DBD */
    cmd.ಪುಟ = ಬಹುಲಕ_ಪುಟ_HD_GEOMETRY;
    cmd.ಎಣಿಕೆ_2 = cpu_ಗಮ್ಯ_be16(ಗಾತ್ರ(*ದತ್ತಾಂಶ));
    op->command = CMD_SCSI;
    op->ಎಣಿಕೆ_2 = 1;
    op->buf_fl = ದತ್ತಾಂಶ;
    op->cdbcmd = &cmd;
    op->blocksize = ಗಾತ್ರ(*ದತ್ತಾಂಶ);
    ಹಿಂತಿರುಗಿ process_op(op);
}


/****************************************************************
 * Main SCSI commands
 ****************************************************************/

// Create a scsi command request from a disk_op_s request
ಪೂರ್ಣಾಂಕ
scsi_fill_cmd(ರಚನೆ disk_op_s *op, ಶೂನ್ಯ *cdbcmd, ಪೂರ್ಣಾಂಕ maxcdb)
{
    ಆಯ್ಕೆ (op->command) {
    ಸ್ಥಿತಿ CMD_READ:
    ಸ್ಥಿತಿ CMD_WRITE: ;
        ಆದರೆ (op->lba < 0xFFFFFFFFULL) {
            ರಚನೆ cdb_rwdata_10 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_10
                            : CDB_CMD_WRITE_10);
            cmd->lba = cpu_ಗಮ್ಯ_be32(op->lba);
            cmd->ಎಣಿಕೆ_2 = cpu_ಗಮ್ಯ_be16(op->ಎಣಿಕೆ_2);
        } ಇಲ್ಲದಿದ್ದರೆ {
            ರಚನೆ cdb_rwdata_16 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_16
                            : CDB_CMD_WRITE_16);
            cmd->lba = cpu_ಗಮ್ಯ_be64(op->lba);
            cmd->ಎಣಿಕೆ_2 = cpu_ಗಮ್ಯ_be32(op->ಎಣಿಕೆ_2);
        }
        ಹಿಂತಿರುಗಿ GET_FLATPTR(op->drive_fl->blksize);
    ಸ್ಥಿತಿ CMD_SCSI:
        ಆದರೆ (MODESEGMENT)
            ಹಿಂತಿರುಗಿ -1;
        ಸ್ಮರಣೆ_ನಕಲು(cdbcmd, op->cdbcmd, maxcdb);
        ಹಿಂತಿರುಗಿ op->blocksize;
    ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ -1;
    }
}

// Determine if the command is a request to pull data from the device
ಪೂರ್ಣಾಂಕ
scsi_is_read(ರಚನೆ disk_op_s *op)
{
    ಹಿಂತಿರುಗಿ op->command == CMD_READ || (
        !MODESEGMENT && op->command == CMD_SCSI && op->blocksize);
}

// Check if a SCSI device is ready to receive commands
ಪೂರ್ಣಾಂಕ
scsi_is_ready(ರಚನೆ disk_op_s *op)
{
    ASSERT32FLAT();
    dprintf(6, "scsi_is_ready (drive=%p)\n", op->drive_fl);

    /* Retry TEST UNIT READY for 5 seconds unless MEDIUM NOT PRESENT is
     * reported by the device 3 times.  If the device reports "IN PROGRESS",
     * 30 seconds is added. */
    ಪೂರ್ಣಾಂಕ tries = 3;
    ಪೂರ್ಣಾಂಕ ಒಳ_ಪ್ರಗತಿ = 0;
    u32 ಕೊನೆ = timer_calc(5000);
    ಗಾಗಿ (;;) {
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            dprintf(1, "test unit ready failed\n");
            ಹಿಂತಿರುಗಿ -1;
        }

        ಪೂರ್ಣಾಂಕ ret = cdb_test_unit_ready(op);
        ಆದರೆ (!ret)
            // Success
            ನಿಲ್ಲಿಸು;

        ರಚನೆ cdbres_request_sense sense;
        ret = cdb_get_sense(op, &sense);
        ಆದರೆ (ret)
            // Error - retry.
            ಮುಂದುವರಿ;

        // Sense succeeded.
        ಆದರೆ (sense.asc == 0x3a) { /* MEDIUM NOT PRESENT */
            tries--;
            dprintf(1, "Device reports MEDIUM NOT PRESENT - %d tries left\n",
                tries);
            ಆದರೆ (!tries)
                ಹಿಂತಿರುಗಿ -1;
        }

        ಆದರೆ (sense.asc == 0x04 && sense.ascq == 0x01 && !ಒಳ_ಪ್ರಗತಿ) {
            /* IN PROGRESS OF BECOMING READY */
            dprintf(1, "Waiting for device to detect medium... ");
            /* Allow 30 seconds more */
            ಕೊನೆ = timer_calc(30000);
            ಒಳ_ಪ್ರಗತಿ = 1;
        }
    }
    ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿಸು CDB_CMD_REPORT_LUNS  0xA0

ರಚನೆ cdb_report_luns {
    u8 command;
    u8 reserved_01[5];
    u32 ಉದ್ದಳತೆ;
    u8 pad[6];
} PACKED;

ರಚನೆ scsi_lun {
    u16 lun[4];
};

ರಚನೆ cdbres_report_luns {
    u32 ಉದ್ದಳತೆ;
    u32 reserved;
    ರಚನೆ scsi_lun luns[];
};

ಸ್ಥಿರ u64 scsilun2u64(ರಚನೆ scsi_lun *scsi_lun)
{
    ಪೂರ್ಣಾಂಕ i;
    u64 ret = 0;
    ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(scsi_lun->lun); i++)
        ret |= be16_ಗಮ್ಯ_cpu(scsi_lun->lun[i]) << (16 * i);
    ಹಿಂತಿರುಗಿ ret;
}

// Issue REPORT LUNS on a temporary drive and iterate reported luns calling
// @add_lun for each
ಪೂರ್ಣಾಂಕ scsi_rep_luns_ಪರಿಶೀಲನೆ(ರಚನೆ drive_s *tmp_drive, scsi_ಕೂಡಿಸು_lun ಕೂಡಿಸು_lun)
{
    ಪೂರ್ಣಾಂಕ ret = -1;
    /* start with the smallest possible buffer, otherwise some devices in QEMU
     * may (incorrectly) error out on returning less data than fits in it */
    u32 maxluns = 1;
    u32 nluns, i;
    ರಚನೆ cdb_report_luns cdb = {
        .command = CDB_CMD_REPORT_LUNS,
    };
    ರಚನೆ disk_op_s op = {
        .drive_fl = tmp_drive,
        .command = CMD_SCSI,
        .ಎಣಿಕೆ_2 = 1,
        .cdbcmd = &cdb,
    };
    ರಚನೆ cdbres_report_luns *resp;

    ASSERT32FLAT();

    ಆಗ (1) {
        op.blocksize = ಗಾತ್ರ(ರಚನೆ cdbres_report_luns) +
            maxluns * ಗಾತ್ರ(ರಚನೆ scsi_lun);
        op.buf_fl = malloc_tmp(op.blocksize);
        ಆದರೆ (!op.buf_fl) {
            warn_noalloc();
            ಹಿಂತಿರುಗಿ -1;
        }

        cdb.ಉದ್ದಳತೆ = cpu_ಗಮ್ಯ_be32(op.blocksize);
        ಆದರೆ (process_op(&op) != DISK_RET_SUCCESS)
            ಹೋಗು ಹೊರಹೋಗು;

        resp = op.buf_fl;
        nluns = be32_ಗಮ್ಯ_cpu(resp->ಉದ್ದಳತೆ) / ಗಾತ್ರ(ರಚನೆ scsi_lun);
        ಆದರೆ (nluns <= maxluns)
            ನಿಲ್ಲಿಸು;

        ಬಿಡುಗಡೆ(op.buf_fl);
        maxluns = nluns;
    }

    ಗಾಗಿ (i = 0, ret = 0; i < nluns; i++) {
        u64 lun = scsilun2u64(&resp->luns[i]);
        ಆದರೆ (lun >> 32)
            ಮುಂದುವರಿ;
        ret += !ಕೂಡಿಸು_lun((u32)lun, tmp_drive);
    }
ಹೊರಹೋಗು:
    ಬಿಡುಗಡೆ(op.buf_fl);
    ಹಿಂತಿರುಗಿ ret;
}

// Iterate LUNs on the target and call @add_lun for each
ಪೂರ್ಣಾಂಕ scsi_sequential_ಪರಿಶೀಲನೆ(ರಚನೆ drive_s *tmp_drive, u32 maxluns,
                         scsi_ಕೂಡಿಸು_lun ಕೂಡಿಸು_lun)
{
    ಪೂರ್ಣಾಂಕ ret;
    u32 lun;

    ಗಾಗಿ (lun = 0, ret = 0; lun < maxluns; lun++)
        ret += !ಕೂಡಿಸು_lun(lun, tmp_drive);
    ಹಿಂತಿರುಗಿ ret;
}

// Validate drive, find block size / sector count, and register drive.
ಪೂರ್ಣಾಂಕ
scsi_drive_ಸಿದ್ಧತೆ(ರಚನೆ drive_s *drive, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಪೂರ್ಣಾಂಕ prio)
{
    ASSERT32FLAT();
    ರಚನೆ disk_op_s dop;
    memset(&dop, 0, ಗಾತ್ರ(dop));
    dop.drive_fl = drive;
    ರಚನೆ cdbres_inquiry ದತ್ತಾಂಶ;
    ಪೂರ್ಣಾಂಕ ret = cdb_get_inquiry(&dop, &ದತ್ತಾಂಶ);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;
    ಅಕ್ಷರ vendor[ಗಾತ್ರ(ದತ್ತಾಂಶ.vendor)+1], ಗುಣಲಬ್ಧ[ಗಾತ್ರ(ದತ್ತಾಂಶ.ಗುಣಲಬ್ಧ)+1];
    ಅಕ್ಷರ rev[ಗಾತ್ರ(ದತ್ತಾಂಶ.rev)+1];
    strtcpy(vendor, ದತ್ತಾಂಶ.vendor, ಗಾತ್ರ(vendor));
    nullTrailingSpace(vendor);
    strtcpy(ಗುಣಲಬ್ಧ, ದತ್ತಾಂಶ.ಗುಣಲಬ್ಧ, ಗಾತ್ರ(ಗುಣಲಬ್ಧ));
    nullTrailingSpace(ಗುಣಲಬ್ಧ);
    strtcpy(rev, ದತ್ತಾಂಶ.rev, ಗಾತ್ರ(rev));
    nullTrailingSpace(rev);
    ಪೂರ್ಣಾಂಕ pdt = ದತ್ತಾಂಶ.pdt & 0x1f;
    ಪೂರ್ಣಾಂಕ removable = !!(ದತ್ತಾಂಶ.removable & 0x80);
    dprintf(1, "%s vendor='%s' product='%s' rev='%s' type=%d removable=%d\n"
            , s, vendor, ಗುಣಲಬ್ಧ, rev, pdt, removable);
    drive->removable = removable;

    ಆದರೆ (pdt == SCSI_TYPE_CDROM) {
        drive->blksize = CDROM_SECTOR_ಗಾತ್ರ;
        drive->sectors = (u64)-1;

        ಅಕ್ಷರ *desc = znprintf(MAXDESCSIZE, "DVD/CD [%s Drive %s %s %s]"
                              , s, vendor, ಗುಣಲಬ್ಧ, rev);
        boot_ಕೂಡಿಸು_cd(drive, desc, prio);
        ಹಿಂತಿರುಗಿ 0;
    }

    ಆದರೆ (pdt != SCSI_TYPE_DISK)
        ಹಿಂತಿರುಗಿ -1;

    ret = scsi_is_ready(&dop);
    ಆದರೆ (ret) {
        dprintf(1, "scsi_is_ready returned %d\n", ret);
        ಹಿಂತಿರುಗಿ ret;
    }

    ರಚನೆ cdbres_read_ಸಾಮರ್ಥ್ಯ capdata;
    ret = cdb_read_ಸಾಮರ್ಥ್ಯ(&dop, &capdata);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    ಆದರೆ (be32_ಗಮ್ಯ_cpu(capdata.sectors) == 0xFFFFFFFFUL) {
        dprintf(3, "%s: >2TB Detected trying READCAP(16)\n", s);
        ರಚನೆ cdbres_read_ಸಾಮರ್ಥ್ಯ_16 capdata16;
        ret = cdb_read_capacity16(&dop, &capdata16);
        drive->blksize = be32_ಗಮ್ಯ_cpu(capdata16.blksize);
        drive->sectors = be64_ಗಮ್ಯ_cpu(capdata16.sectors) + 1;
    } ಇಲ್ಲದಿದ್ದರೆ {
        drive->blksize = be32_ಗಮ್ಯ_cpu(capdata.blksize);
        drive->sectors = (u64)be32_ಗಮ್ಯ_cpu(capdata.sectors) + 1;
    }

    ಆದರೆ (drive->blksize != DISK_SECTOR_ಗಾತ್ರ) {
        dprintf(1, "%s: unsupported block size %d\n", s, drive->blksize);
        ಹಿಂತಿರುಗಿ -1;
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
    ಆದರೆ (CONFIG_QEMU_HARDWARE && memcmp(vendor, "QEMU", 5) == 0) {
        ರಚನೆ cdbres_ಬಹುಲಕ_sense_geom geomdata;
        ret = cdb_ಬಹುಲಕ_sense_geom(&dop, &geomdata);
        ಆದರೆ (ret == 0) {
            u32 cylinders;
            cylinders = geomdata.cyl[0] << 16;
            cylinders |= geomdata.cyl[1] << 8;
            cylinders |= geomdata.cyl[2];
            ಆದರೆ (cylinders && geomdata.heads &&
                drive->sectors <= 0xFFFFFFFFULL &&
                ((u32)drive->sectors % (geomdata.heads * cylinders) == 0)) {
                drive->pchs.cylinder = cylinders;
                drive->pchs.ತಲೆ = geomdata.heads;
                drive->pchs.sector = (u32)drive->sectors / (geomdata.heads * cylinders);
            }
        }
    }

    ಅಕ್ಷರ *desc = znprintf(MAXDESCSIZE, "%s Drive %s %s %s"
                          , s, vendor, ಗುಣಲಬ್ಧ, rev);
    boot_ಕೂಡಿಸು_hd(drive, desc, prio);
    ಹಿಂತಿರುಗಿ 0;
}

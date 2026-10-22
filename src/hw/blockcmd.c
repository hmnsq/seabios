// Support for several common scsi like command data block requests
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // struct disk_op_s
#ਅਨ੍ਤਰ੍ਭਾਵ "blockcmd.h" // struct cdb_request_sense
#ਅਨ੍ਤਰ੍ਭਾਵ "byteorder.h" // be32_to_cpu
#ਅਨ੍ਤਰ੍ਭਾਵ "farptr.h" // GET_FLATPTR
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // DISK_RET_EPARAM
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // timer_calc
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h"


/****************************************************************
 * Low level command requests
 ****************************************************************/

ਸਥਿਰ ਪੂਰਨ
cdb_get_inquiry(ਬਣਤਰ disk_op_s *op, ਬਣਤਰ cdbres_inquiry *ਅੰਕੜੇ)
{
    ਬਣਤਰ cdb_request_sense cmd;
    memset(&cmd, 0, ਆਕਾਰ(cmd));
    cmd.command = CDB_CMD_INQUIRY;
    cmd.ਲੰਮਾਈ = ਆਕਾਰ(*ਅੰਕੜੇ);
    op->command = CMD_SCSI;
    op->ਗਿਣਤੀ_2 = 1;
    op->buf_fl = ਅੰਕੜੇ;
    op->cdbcmd = &cmd;
    op->blocksize = ਆਕਾਰ(*ਅੰਕੜੇ);
    ਵਾਪਸ process_op(op);
}

// Request SENSE
ਸਥਿਰ ਪੂਰਨ
cdb_get_sense(ਬਣਤਰ disk_op_s *op, ਬਣਤਰ cdbres_request_sense *ਅੰਕੜੇ)
{
    ਬਣਤਰ cdb_request_sense cmd;
    memset(&cmd, 0, ਆਕਾਰ(cmd));
    cmd.command = CDB_CMD_REQUEST_SENSE;
    cmd.ਲੰਮਾਈ = ਆਕਾਰ(*ਅੰਕੜੇ);
    op->command = CMD_SCSI;
    op->ਗਿਣਤੀ_2 = 1;
    op->buf_fl = ਅੰਕੜੇ;
    op->cdbcmd = &cmd;
    op->blocksize = ਆਕਾਰ(*ਅੰਕੜੇ);
    ਵਾਪਸ process_op(op);
}

// Test unit ready
ਸਥਿਰ ਪੂਰਨ
cdb_test_unit_ready(ਬਣਤਰ disk_op_s *op)
{
    ਬਣਤਰ cdb_request_sense cmd;
    memset(&cmd, 0, ਆਕਾਰ(cmd));
    cmd.command = CDB_CMD_TEST_UNIT_READY;
    op->command = CMD_SCSI;
    op->ਗਿਣਤੀ_2 = 0;
    op->buf_fl = ਨਿਰਾ;
    op->cdbcmd = &cmd;
    op->blocksize = 0;
    ਵਾਪਸ process_op(op);
}

ਸਥਿਰ ਪੂਰਨ
cdb_read_capacity16(ਬਣਤਰ disk_op_s *op, ਬਣਤਰ cdbres_read_ਸਮਰੱਥਾ_16 *ਅੰਕੜੇ)
{
    ਬਣਤਰ cdb_sai_read_ਸਮਰੱਥਾ_16 cmd;
    memset(&cmd, 0, ਆਕਾਰ(cmd));
    cmd.command = CDB_CMD_SERVICE_ACTION_ਵਿੱਚ;
    cmd.flags = CDB_CMD_SAI_READ_ਸਮਰੱਥਾ_16;
    cmd.len = cpu_ਮੰਜ਼ਿਲ_be32(ਆਕਾਰ(ਬਣਤਰ cdbres_read_ਸਮਰੱਥਾ_16));
    op->command = CMD_SCSI;
    op->ਗਿਣਤੀ_2 = 1;
    op->buf_fl = ਅੰਕੜੇ;
    op->cdbcmd = &cmd;
    op->blocksize = ਆਕਾਰ(*ਅੰਕੜੇ);
    ਵਾਪਸ process_op(op);
}


// Request capacity
ਸਥਿਰ ਪੂਰਨ
cdb_read_ਸਮਰੱਥਾ(ਬਣਤਰ disk_op_s *op, ਬਣਤਰ cdbres_read_ਸਮਰੱਥਾ *ਅੰਕੜੇ)
{
    ਬਣਤਰ cdb_read_ਸਮਰੱਥਾ cmd;
    memset(&cmd, 0, ਆਕਾਰ(cmd));
    cmd.command = CDB_CMD_READ_ਸਮਰੱਥਾ;
    op->command = CMD_SCSI;
    op->ਗਿਣਤੀ_2 = 1;
    op->buf_fl = ਅੰਕੜੇ;
    op->cdbcmd = &cmd;
    op->blocksize = ਆਕਾਰ(*ਅੰਕੜੇ);
    ਵਾਪਸ process_op(op);
}

// Mode sense, geometry page.
ਸਥਿਰ ਪੂਰਨ
cdb_ਬਹੁਲਕ_sense_geom(ਬਣਤਰ disk_op_s *op, ਬਣਤਰ cdbres_ਬਹੁਲਕ_sense_geom *ਅੰਕੜੇ)
{
    ਬਣਤਰ cdb_ਬਹੁਲਕ_sense cmd;
    memset(&cmd, 0, ਆਕਾਰ(cmd));
    cmd.command = CDB_CMD_ਬਹੁਲਕ_SENSE;
    cmd.flags = 8; /* DBD */
    cmd.ਪੰਨਾ = ਬਹੁਲਕ_ਪੰਨਾ_HD_GEOMETRY;
    cmd.ਗਿਣਤੀ_2 = cpu_ਮੰਜ਼ਿਲ_be16(ਆਕਾਰ(*ਅੰਕੜੇ));
    op->command = CMD_SCSI;
    op->ਗਿਣਤੀ_2 = 1;
    op->buf_fl = ਅੰਕੜੇ;
    op->cdbcmd = &cmd;
    op->blocksize = ਆਕਾਰ(*ਅੰਕੜੇ);
    ਵਾਪਸ process_op(op);
}


/****************************************************************
 * Main SCSI commands
 ****************************************************************/

// Create a scsi command request from a disk_op_s request
ਪੂਰਨ
scsi_fill_cmd(ਬਣਤਰ disk_op_s *op, ਖਾਲੀ *cdbcmd, ਪੂਰਨ maxcdb)
{
    ਚੋਣ (op->command) {
    ਹਾਲਤ CMD_READ:
    ਹਾਲਤ CMD_WRITE: ;
        ਜੇ (op->lba < 0xFFFFFFFFULL) {
            ਬਣਤਰ cdb_rwdata_10 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_10
                            : CDB_CMD_WRITE_10);
            cmd->lba = cpu_ਮੰਜ਼ਿਲ_be32(op->lba);
            cmd->ਗਿਣਤੀ_2 = cpu_ਮੰਜ਼ਿਲ_be16(op->ਗਿਣਤੀ_2);
        } ਨਹੀਂਤਾਂ {
            ਬਣਤਰ cdb_rwdata_16 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_16
                            : CDB_CMD_WRITE_16);
            cmd->lba = cpu_ਮੰਜ਼ਿਲ_be64(op->lba);
            cmd->ਗਿਣਤੀ_2 = cpu_ਮੰਜ਼ਿਲ_be32(op->ਗਿਣਤੀ_2);
        }
        ਵਾਪਸ GET_FLATPTR(op->drive_fl->blksize);
    ਹਾਲਤ CMD_SCSI:
        ਜੇ (MODESEGMENT)
            ਵਾਪਸ -1;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(cdbcmd, op->cdbcmd, maxcdb);
        ਵਾਪਸ op->blocksize;
    ਮੂਲ:
        ਵਾਪਸ -1;
    }
}

// Determine if the command is a request to pull data from the device
ਪੂਰਨ
scsi_is_read(ਬਣਤਰ disk_op_s *op)
{
    ਵਾਪਸ op->command == CMD_READ || (
        !MODESEGMENT && op->command == CMD_SCSI && op->blocksize);
}

// Check if a SCSI device is ready to receive commands
ਪੂਰਨ
scsi_is_ready(ਬਣਤਰ disk_op_s *op)
{
    ASSERT32FLAT();
    dprintf(6, "scsi_is_ready (drive=%p)\n", op->drive_fl);

    /* Retry TEST UNIT READY for 5 seconds unless MEDIUM NOT PRESENT is
     * reported by the device 3 times.  If the device reports "IN PROGRESS",
     * 30 seconds is added. */
    ਪੂਰਨ tries = 3;
    ਪੂਰਨ ਵਿੱਚ_ਤਰੱਕੀ = 0;
    u32 ਸਿਰਾ = timer_calc(5000);
    ਲਈ (;;) {
        ਜੇ (timer_check(ਸਿਰਾ)) {
            dprintf(1, "test unit ready failed\n");
            ਵਾਪਸ -1;
        }

        ਪੂਰਨ ret = cdb_test_unit_ready(op);
        ਜੇ (!ret)
            // Success
            ਰੋਕੋ;

        ਬਣਤਰ cdbres_request_sense sense;
        ret = cdb_get_sense(op, &sense);
        ਜੇ (ret)
            // Error - retry.
            ਜਾਰੀ;

        // Sense succeeded.
        ਜੇ (sense.asc == 0x3a) { /* MEDIUM NOT PRESENT */
            tries--;
            dprintf(1, "Device reports MEDIUM NOT PRESENT - %d tries left\n",
                tries);
            ਜੇ (!tries)
                ਵਾਪਸ -1;
        }

        ਜੇ (sense.asc == 0x04 && sense.ascq == 0x01 && !ਵਿੱਚ_ਤਰੱਕੀ) {
            /* IN PROGRESS OF BECOMING READY */
            dprintf(1, "Waiting for device to detect medium... ");
            /* Allow 30 seconds more */
            ਸਿਰਾ = timer_calc(30000);
            ਵਿੱਚ_ਤਰੱਕੀ = 1;
        }
    }
    ਵਾਪਸ 0;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CDB_CMD_REPORT_LUNS  0xA0

ਬਣਤਰ cdb_report_luns {
    u8 command;
    u8 reserved_01[5];
    u32 ਲੰਮਾਈ;
    u8 pad[6];
} PACKED;

ਬਣਤਰ scsi_lun {
    u16 lun[4];
};

ਬਣਤਰ cdbres_report_luns {
    u32 ਲੰਮਾਈ;
    u32 reserved;
    ਬਣਤਰ scsi_lun luns[];
};

ਸਥਿਰ u64 scsilun2u64(ਬਣਤਰ scsi_lun *scsi_lun)
{
    ਪੂਰਨ i;
    u64 ret = 0;
    ਲਈ (i = 0; i < ARRAY_ਮਾਪ(scsi_lun->lun); i++)
        ret |= be16_ਮੰਜ਼ਿਲ_cpu(scsi_lun->lun[i]) << (16 * i);
    ਵਾਪਸ ret;
}

// Issue REPORT LUNS on a temporary drive and iterate reported luns calling
// @add_lun for each
ਪੂਰਨ scsi_rep_luns_ਜਾਂਚ(ਬਣਤਰ drive_s *tmp_drive, scsi_ਜੋੜੋ_lun ਜੋੜੋ_lun)
{
    ਪੂਰਨ ret = -1;
    /* start with the smallest possible buffer, otherwise some devices in QEMU
     * may (incorrectly) error out on returning less data than fits in it */
    u32 maxluns = 1;
    u32 nluns, i;
    ਬਣਤਰ cdb_report_luns cdb = {
        .command = CDB_CMD_REPORT_LUNS,
    };
    ਬਣਤਰ disk_op_s op = {
        .drive_fl = tmp_drive,
        .command = CMD_SCSI,
        .ਗਿਣਤੀ_2 = 1,
        .cdbcmd = &cdb,
    };
    ਬਣਤਰ cdbres_report_luns *resp;

    ASSERT32FLAT();

    ਜਦੋਂ (1) {
        op.blocksize = ਆਕਾਰ(ਬਣਤਰ cdbres_report_luns) +
            maxluns * ਆਕਾਰ(ਬਣਤਰ scsi_lun);
        op.buf_fl = malloc_tmp(op.blocksize);
        ਜੇ (!op.buf_fl) {
            warn_noalloc();
            ਵਾਪਸ -1;
        }

        cdb.ਲੰਮਾਈ = cpu_ਮੰਜ਼ਿਲ_be32(op.blocksize);
        ਜੇ (process_op(&op) != DISK_RET_SUCCESS)
            ਜਾਓ ਬਾਹਰਲਾ;

        resp = op.buf_fl;
        nluns = be32_ਮੰਜ਼ਿਲ_cpu(resp->ਲੰਮਾਈ) / ਆਕਾਰ(ਬਣਤਰ scsi_lun);
        ਜੇ (nluns <= maxluns)
            ਰੋਕੋ;

        ਮੁਕਤ(op.buf_fl);
        maxluns = nluns;
    }

    ਲਈ (i = 0, ret = 0; i < nluns; i++) {
        u64 lun = scsilun2u64(&resp->luns[i]);
        ਜੇ (lun >> 32)
            ਜਾਰੀ;
        ret += !ਜੋੜੋ_lun((u32)lun, tmp_drive);
    }
ਬਾਹਰਲਾ:
    ਮੁਕਤ(op.buf_fl);
    ਵਾਪਸ ret;
}

// Iterate LUNs on the target and call @add_lun for each
ਪੂਰਨ scsi_sequential_ਜਾਂਚ(ਬਣਤਰ drive_s *tmp_drive, u32 maxluns,
                         scsi_ਜੋੜੋ_lun ਜੋੜੋ_lun)
{
    ਪੂਰਨ ret;
    u32 lun;

    ਲਈ (lun = 0, ret = 0; lun < maxluns; lun++)
        ret += !ਜੋੜੋ_lun(lun, tmp_drive);
    ਵਾਪਸ ret;
}

// Validate drive, find block size / sector count, and register drive.
ਪੂਰਨ
scsi_drive_ਤਿਆਰੀ(ਬਣਤਰ drive_s *drive, ਅਚਲ ਅੱਖਰ *s, ਪੂਰਨ prio)
{
    ASSERT32FLAT();
    ਬਣਤਰ disk_op_s dop;
    memset(&dop, 0, ਆਕਾਰ(dop));
    dop.drive_fl = drive;
    ਬਣਤਰ cdbres_inquiry ਅੰਕੜੇ;
    ਪੂਰਨ ret = cdb_get_inquiry(&dop, &ਅੰਕੜੇ);
    ਜੇ (ret)
        ਵਾਪਸ ret;
    ਅੱਖਰ vendor[ਆਕਾਰ(ਅੰਕੜੇ.vendor)+1], ਗੁਣਨਫਲ[ਆਕਾਰ(ਅੰਕੜੇ.ਗੁਣਨਫਲ)+1];
    ਅੱਖਰ rev[ਆਕਾਰ(ਅੰਕੜੇ.rev)+1];
    strtcpy(vendor, ਅੰਕੜੇ.vendor, ਆਕਾਰ(vendor));
    nullTrailingSpace(vendor);
    strtcpy(ਗੁਣਨਫਲ, ਅੰਕੜੇ.ਗੁਣਨਫਲ, ਆਕਾਰ(ਗੁਣਨਫਲ));
    nullTrailingSpace(ਗੁਣਨਫਲ);
    strtcpy(rev, ਅੰਕੜੇ.rev, ਆਕਾਰ(rev));
    nullTrailingSpace(rev);
    ਪੂਰਨ pdt = ਅੰਕੜੇ.pdt & 0x1f;
    ਪੂਰਨ removable = !!(ਅੰਕੜੇ.removable & 0x80);
    dprintf(1, "%s vendor='%s' product='%s' rev='%s' type=%d removable=%d\n"
            , s, vendor, ਗੁਣਨਫਲ, rev, pdt, removable);
    drive->removable = removable;

    ਜੇ (pdt == SCSI_TYPE_CDROM) {
        drive->blksize = CDROM_SECTOR_ਮਾਪ;
        drive->sectors = (u64)-1;

        ਅੱਖਰ *desc = znprintf(MAXDESCSIZE, "DVD/CD [%s Drive %s %s %s]"
                              , s, vendor, ਗੁਣਨਫਲ, rev);
        boot_ਜੋੜੋ_cd(drive, desc, prio);
        ਵਾਪਸ 0;
    }

    ਜੇ (pdt != SCSI_TYPE_DISK)
        ਵਾਪਸ -1;

    ret = scsi_is_ready(&dop);
    ਜੇ (ret) {
        dprintf(1, "scsi_is_ready returned %d\n", ret);
        ਵਾਪਸ ret;
    }

    ਬਣਤਰ cdbres_read_ਸਮਰੱਥਾ capdata;
    ret = cdb_read_ਸਮਰੱਥਾ(&dop, &capdata);
    ਜੇ (ret)
        ਵਾਪਸ ret;

    ਜੇ (be32_ਮੰਜ਼ਿਲ_cpu(capdata.sectors) == 0xFFFFFFFFUL) {
        dprintf(3, "%s: >2TB Detected trying READCAP(16)\n", s);
        ਬਣਤਰ cdbres_read_ਸਮਰੱਥਾ_16 capdata16;
        ret = cdb_read_capacity16(&dop, &capdata16);
        drive->blksize = be32_ਮੰਜ਼ਿਲ_cpu(capdata16.blksize);
        drive->sectors = be64_ਮੰਜ਼ਿਲ_cpu(capdata16.sectors) + 1;
    } ਨਹੀਂਤਾਂ {
        drive->blksize = be32_ਮੰਜ਼ਿਲ_cpu(capdata.blksize);
        drive->sectors = (u64)be32_ਮੰਜ਼ਿਲ_cpu(capdata.sectors) + 1;
    }

    ਜੇ (drive->blksize != DISK_SECTOR_ਮਾਪ) {
        dprintf(1, "%s: unsupported block size %d\n", s, drive->blksize);
        ਵਾਪਸ -1;
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
    ਜੇ (CONFIG_QEMU_HARDWARE && memcmp(vendor, "QEMU", 5) == 0) {
        ਬਣਤਰ cdbres_ਬਹੁਲਕ_sense_geom geomdata;
        ret = cdb_ਬਹੁਲਕ_sense_geom(&dop, &geomdata);
        ਜੇ (ret == 0) {
            u32 cylinders;
            cylinders = geomdata.cyl[0] << 16;
            cylinders |= geomdata.cyl[1] << 8;
            cylinders |= geomdata.cyl[2];
            ਜੇ (cylinders && geomdata.heads &&
                drive->sectors <= 0xFFFFFFFFULL &&
                ((u32)drive->sectors % (geomdata.heads * cylinders) == 0)) {
                drive->pchs.cylinder = cylinders;
                drive->pchs.ਸਿਰ = geomdata.heads;
                drive->pchs.sector = (u32)drive->sectors / (geomdata.heads * cylinders);
            }
        }
    }

    ਅੱਖਰ *desc = znprintf(MAXDESCSIZE, "%s Drive %s %s %s"
                          , s, vendor, ਗੁਣਨਫਲ, rev);
    boot_ਜੋੜੋ_hd(drive, desc, prio);
    ਵਾਪਸ 0;
}

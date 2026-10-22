// Support for several common scsi like command data block requests
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "block.h" // struct disk_op_s
#அந்தர்பாவ "blockcmd.h" // struct cdb_request_sense
#அந்தர்பாவ "byteorder.h" // be32_to_cpu
#அந்தர்பாவ "farptr.h" // GET_FLATPTR
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "std/disk.h" // DISK_RET_EPARAM
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // timer_calc
#அந்தர்பாவ "malloc.h"


/****************************************************************
 * Low level command requests
 ****************************************************************/

நிலையான முழு
cdb_get_inquiry(கட்டமைப்பு disk_op_s *op, கட்டமைப்பு cdbres_inquiry *data)
{
    கட்டமைப்பு cdb_request_sense cmd;
    memset(&cmd, 0, அளவு(cmd));
    cmd.command = CDB_CMD_INQUIRY;
    cmd.length = அளவு(*data);
    op->command = CMD_SCSI;
    op->மொத்தம் = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = அளவு(*data);
    திரும்பு process_op(op);
}

// Request SENSE
நிலையான முழு
cdb_get_sense(கட்டமைப்பு disk_op_s *op, கட்டமைப்பு cdbres_request_sense *data)
{
    கட்டமைப்பு cdb_request_sense cmd;
    memset(&cmd, 0, அளவு(cmd));
    cmd.command = CDB_CMD_REQUEST_SENSE;
    cmd.length = அளவு(*data);
    op->command = CMD_SCSI;
    op->மொத்தம் = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = அளவு(*data);
    திரும்பு process_op(op);
}

// Test unit ready
நிலையான முழு
cdb_test_unit_ready(கட்டமைப்பு disk_op_s *op)
{
    கட்டமைப்பு cdb_request_sense cmd;
    memset(&cmd, 0, அளவு(cmd));
    cmd.command = CDB_CMD_TEST_UNIT_READY;
    op->command = CMD_SCSI;
    op->மொத்தம் = 0;
    op->buf_fl = வெற்று;
    op->cdbcmd = &cmd;
    op->blocksize = 0;
    திரும்பு process_op(op);
}

நிலையான முழு
cdb_read_capacity16(கட்டமைப்பு disk_op_s *op, கட்டமைப்பு cdbres_read_capacity_16 *data)
{
    கட்டமைப்பு cdb_sai_read_capacity_16 cmd;
    memset(&cmd, 0, அளவு(cmd));
    cmd.command = CDB_CMD_SERVICE_ACTION_IN;
    cmd.flags = CDB_CMD_SAI_READ_CAPACITY_16;
    cmd.len = cpu_to_be32(அளவு(கட்டமைப்பு cdbres_read_capacity_16));
    op->command = CMD_SCSI;
    op->மொத்தம் = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = அளவு(*data);
    திரும்பு process_op(op);
}


// Request capacity
நிலையான முழு
cdb_read_capacity(கட்டமைப்பு disk_op_s *op, கட்டமைப்பு cdbres_read_capacity *data)
{
    கட்டமைப்பு cdb_read_capacity cmd;
    memset(&cmd, 0, அளவு(cmd));
    cmd.command = CDB_CMD_READ_CAPACITY;
    op->command = CMD_SCSI;
    op->மொத்தம் = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = அளவு(*data);
    திரும்பு process_op(op);
}

// Mode sense, geometry page.
நிலையான முழு
cdb_mode_sense_geom(கட்டமைப்பு disk_op_s *op, கட்டமைப்பு cdbres_mode_sense_geom *data)
{
    கட்டமைப்பு cdb_mode_sense cmd;
    memset(&cmd, 0, அளவு(cmd));
    cmd.command = CDB_CMD_MODE_SENSE;
    cmd.flags = 8; /* DBD */
    cmd.page = MODE_PAGE_HD_GEOMETRY;
    cmd.மொத்தம் = cpu_to_be16(அளவு(*data));
    op->command = CMD_SCSI;
    op->மொத்தம் = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = அளவு(*data);
    திரும்பு process_op(op);
}


/****************************************************************
 * Main SCSI commands
 ****************************************************************/

// Create a scsi command request from a disk_op_s request
முழு
scsi_fill_cmd(கட்டமைப்பு disk_op_s *op, வெறுமை *cdbcmd, முழு maxcdb)
{
    தேர்வு (op->command) {
    நிலை CMD_READ:
    நிலை CMD_WRITE: ;
        எனில் (op->lba < 0xFFFFFFFFULL) {
            கட்டமைப்பு cdb_rwdata_10 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_10
                            : CDB_CMD_WRITE_10);
            cmd->lba = cpu_to_be32(op->lba);
            cmd->மொத்தம் = cpu_to_be16(op->மொத்தம்);
        } இல்லையெனில் {
            கட்டமைப்பு cdb_rwdata_16 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_16
                            : CDB_CMD_WRITE_16);
            cmd->lba = cpu_to_be64(op->lba);
            cmd->மொத்தம் = cpu_to_be32(op->மொத்தம்);
        }
        திரும்பு GET_FLATPTR(op->drive_fl->blksize);
    நிலை CMD_SCSI:
        எனில் (MODESEGMENT)
            திரும்பு -1;
        memcpy(cdbcmd, op->cdbcmd, maxcdb);
        திரும்பு op->blocksize;
    இயல்பு:
        திரும்பு -1;
    }
}

// Determine if the command is a request to pull data from the device
முழு
scsi_is_read(கட்டமைப்பு disk_op_s *op)
{
    திரும்பு op->command == CMD_READ || (
        !MODESEGMENT && op->command == CMD_SCSI && op->blocksize);
}

// Check if a SCSI device is ready to receive commands
முழு
scsi_is_ready(கட்டமைப்பு disk_op_s *op)
{
    ASSERT32FLAT();
    dprintf(6, "scsi_is_ready (drive=%p)\n", op->drive_fl);

    /* Retry TEST UNIT READY for 5 seconds unless MEDIUM NOT PRESENT is
     * reported by the device 3 times.  If the device reports "IN PROGRESS",
     * 30 seconds is added. */
    முழு tries = 3;
    முழு in_progress = 0;
    u32 end = timer_calc(5000);
    க்கு (;;) {
        எனில் (timer_check(end)) {
            dprintf(1, "test unit ready failed\n");
            திரும்பு -1;
        }

        முழு ret = cdb_test_unit_ready(op);
        எனில் (!ret)
            // Success
            நிறுத்து;

        கட்டமைப்பு cdbres_request_sense sense;
        ret = cdb_get_sense(op, &sense);
        எனில் (ret)
            // Error - retry.
            தொடர்;

        // Sense succeeded.
        எனில் (sense.asc == 0x3a) { /* MEDIUM NOT PRESENT */
            tries--;
            dprintf(1, "Device reports MEDIUM NOT PRESENT - %d tries left\n",
                tries);
            எனில் (!tries)
                திரும்பு -1;
        }

        எனில் (sense.asc == 0x04 && sense.ascq == 0x01 && !in_progress) {
            /* IN PROGRESS OF BECOMING READY */
            dprintf(1, "Waiting for device to detect medium... ");
            /* Allow 30 seconds more */
            end = timer_calc(30000);
            in_progress = 1;
        }
    }
    திரும்பு 0;
}

#define CDB_CMD_REPORT_LUNS  0xA0

கட்டமைப்பு cdb_report_luns {
    u8 command;
    u8 reserved_01[5];
    u32 length;
    u8 pad[6];
} PACKED;

கட்டமைப்பு scsi_lun {
    u16 lun[4];
};

கட்டமைப்பு cdbres_report_luns {
    u32 length;
    u32 reserved;
    கட்டமைப்பு scsi_lun luns[];
};

நிலையான u64 scsilun2u64(கட்டமைப்பு scsi_lun *scsi_lun)
{
    முழு i;
    u64 ret = 0;
    க்கு (i = 0; i < ARRAY_SIZE(scsi_lun->lun); i++)
        ret |= be16_to_cpu(scsi_lun->lun[i]) << (16 * i);
    திரும்பு ret;
}

// Issue REPORT LUNS on a temporary drive and iterate reported luns calling
// @add_lun for each
முழு scsi_rep_luns_scan(கட்டமைப்பு drive_s *tmp_drive, scsi_add_lun add_lun)
{
    முழு ret = -1;
    /* start with the smallest possible buffer, otherwise some devices in QEMU
     * may (incorrectly) error out on returning less data than fits in it */
    u32 maxluns = 1;
    u32 nluns, i;
    கட்டமைப்பு cdb_report_luns cdb = {
        .command = CDB_CMD_REPORT_LUNS,
    };
    கட்டமைப்பு disk_op_s op = {
        .drive_fl = tmp_drive,
        .command = CMD_SCSI,
        .மொத்தம் = 1,
        .cdbcmd = &cdb,
    };
    கட்டமைப்பு cdbres_report_luns *resp;

    ASSERT32FLAT();

    போது (1) {
        op.blocksize = அளவு(கட்டமைப்பு cdbres_report_luns) +
            maxluns * அளவு(கட்டமைப்பு scsi_lun);
        op.buf_fl = malloc_tmp(op.blocksize);
        எனில் (!op.buf_fl) {
            warn_noalloc();
            திரும்பு -1;
        }

        cdb.length = cpu_to_be32(op.blocksize);
        எனில் (process_op(&op) != DISK_RET_SUCCESS)
            செல் out;

        resp = op.buf_fl;
        nluns = be32_to_cpu(resp->length) / அளவு(கட்டமைப்பு scsi_lun);
        எனில் (nluns <= maxluns)
            நிறுத்து;

        விடுவி(op.buf_fl);
        maxluns = nluns;
    }

    க்கு (i = 0, ret = 0; i < nluns; i++) {
        u64 lun = scsilun2u64(&resp->luns[i]);
        எனில் (lun >> 32)
            தொடர்;
        ret += !add_lun((u32)lun, tmp_drive);
    }
out:
    விடுவி(op.buf_fl);
    திரும்பு ret;
}

// Iterate LUNs on the target and call @add_lun for each
முழு scsi_sequential_scan(கட்டமைப்பு drive_s *tmp_drive, u32 maxluns,
                         scsi_add_lun add_lun)
{
    முழு ret;
    u32 lun;

    க்கு (lun = 0, ret = 0; lun < maxluns; lun++)
        ret += !add_lun(lun, tmp_drive);
    திரும்பு ret;
}

// Validate drive, find block size / sector count, and register drive.
முழு
scsi_drive_setup(கட்டமைப்பு drive_s *drive, மாறிலி எழுத்து *s, முழு prio)
{
    ASSERT32FLAT();
    கட்டமைப்பு disk_op_s dop;
    memset(&dop, 0, அளவு(dop));
    dop.drive_fl = drive;
    கட்டமைப்பு cdbres_inquiry data;
    முழு ret = cdb_get_inquiry(&dop, &data);
    எனில் (ret)
        திரும்பு ret;
    எழுத்து vendor[அளவு(data.vendor)+1], product[அளவு(data.product)+1];
    எழுத்து rev[அளவு(data.rev)+1];
    strtcpy(vendor, data.vendor, அளவு(vendor));
    nullTrailingSpace(vendor);
    strtcpy(product, data.product, அளவு(product));
    nullTrailingSpace(product);
    strtcpy(rev, data.rev, அளவு(rev));
    nullTrailingSpace(rev);
    முழு pdt = data.pdt & 0x1f;
    முழு removable = !!(data.removable & 0x80);
    dprintf(1, "%s vendor='%s' product='%s' rev='%s' type=%d removable=%d\n"
            , s, vendor, product, rev, pdt, removable);
    drive->removable = removable;

    எனில் (pdt == SCSI_TYPE_CDROM) {
        drive->blksize = CDROM_SECTOR_SIZE;
        drive->sectors = (u64)-1;

        எழுத்து *desc = znprintf(MAXDESCSIZE, "DVD/CD [%s Drive %s %s %s]"
                              , s, vendor, product, rev);
        boot_add_cd(drive, desc, prio);
        திரும்பு 0;
    }

    எனில் (pdt != SCSI_TYPE_DISK)
        திரும்பு -1;

    ret = scsi_is_ready(&dop);
    எனில் (ret) {
        dprintf(1, "scsi_is_ready returned %d\n", ret);
        திரும்பு ret;
    }

    கட்டமைப்பு cdbres_read_capacity capdata;
    ret = cdb_read_capacity(&dop, &capdata);
    எனில் (ret)
        திரும்பு ret;

    எனில் (be32_to_cpu(capdata.sectors) == 0xFFFFFFFFUL) {
        dprintf(3, "%s: >2TB Detected trying READCAP(16)\n", s);
        கட்டமைப்பு cdbres_read_capacity_16 capdata16;
        ret = cdb_read_capacity16(&dop, &capdata16);
        drive->blksize = be32_to_cpu(capdata16.blksize);
        drive->sectors = be64_to_cpu(capdata16.sectors) + 1;
    } இல்லையெனில் {
        drive->blksize = be32_to_cpu(capdata.blksize);
        drive->sectors = (u64)be32_to_cpu(capdata.sectors) + 1;
    }

    எனில் (drive->blksize != DISK_SECTOR_SIZE) {
        dprintf(1, "%s: unsupported block size %d\n", s, drive->blksize);
        திரும்பு -1;
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
    எனில் (CONFIG_QEMU_HARDWARE && memcmp(vendor, "QEMU", 5) == 0) {
        கட்டமைப்பு cdbres_mode_sense_geom geomdata;
        ret = cdb_mode_sense_geom(&dop, &geomdata);
        எனில் (ret == 0) {
            u32 cylinders;
            cylinders = geomdata.cyl[0] << 16;
            cylinders |= geomdata.cyl[1] << 8;
            cylinders |= geomdata.cyl[2];
            எனில் (cylinders && geomdata.heads &&
                drive->sectors <= 0xFFFFFFFFULL &&
                ((u32)drive->sectors % (geomdata.heads * cylinders) == 0)) {
                drive->pchs.cylinder = cylinders;
                drive->pchs.head = geomdata.heads;
                drive->pchs.sector = (u32)drive->sectors / (geomdata.heads * cylinders);
            }
        }
    }

    எழுத்து *desc = znprintf(MAXDESCSIZE, "%s Drive %s %s %s"
                          , s, vendor, product, rev);
    boot_add_hd(drive, desc, prio);
    திரும்பு 0;
}

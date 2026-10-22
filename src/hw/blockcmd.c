// Support for several common scsi like command data block requests
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "block.h" // struct disk_op_s
#અન્તર્ભાવ "blockcmd.h" // struct cdb_request_sense
#અન્તર્ભાવ "byteorder.h" // be32_to_cpu
#અન્તર્ભાવ "farptr.h" // GET_FLATPTR
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "std/disk.h" // DISK_RET_EPARAM
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // timer_calc
#અન્તર્ભાવ "malloc.h"


/****************************************************************
 * Low level command requests
 ****************************************************************/

સ્થિર પૂર્ણાંક
cdb_get_inquiry(સંરચના disk_op_s *op, સંરચના cdbres_inquiry *data)
{
    સંરચના cdb_request_sense cmd;
    memset(&cmd, 0, કદ(cmd));
    cmd.command = CDB_CMD_INQUIRY;
    cmd.length = કદ(*data);
    op->command = CMD_SCSI;
    op->ગણતરી = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = કદ(*data);
    પાછા process_op(op);
}

// Request SENSE
સ્થિર પૂર્ણાંક
cdb_get_sense(સંરચના disk_op_s *op, સંરચના cdbres_request_sense *data)
{
    સંરચના cdb_request_sense cmd;
    memset(&cmd, 0, કદ(cmd));
    cmd.command = CDB_CMD_REQUEST_SENSE;
    cmd.length = કદ(*data);
    op->command = CMD_SCSI;
    op->ગણતરી = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = કદ(*data);
    પાછા process_op(op);
}

// Test unit ready
સ્થિર પૂર્ણાંક
cdb_test_unit_ready(સંરચના disk_op_s *op)
{
    સંરચના cdb_request_sense cmd;
    memset(&cmd, 0, કદ(cmd));
    cmd.command = CDB_CMD_TEST_UNIT_READY;
    op->command = CMD_SCSI;
    op->ગણતરી = 0;
    op->buf_fl = ખાલી;
    op->cdbcmd = &cmd;
    op->blocksize = 0;
    પાછા process_op(op);
}

સ્થિર પૂર્ણાંક
cdb_read_capacity16(સંરચના disk_op_s *op, સંરચના cdbres_read_capacity_16 *data)
{
    સંરચના cdb_sai_read_capacity_16 cmd;
    memset(&cmd, 0, કદ(cmd));
    cmd.command = CDB_CMD_SERVICE_ACTION_IN;
    cmd.flags = CDB_CMD_SAI_READ_CAPACITY_16;
    cmd.len = cpu_to_be32(કદ(સંરચના cdbres_read_capacity_16));
    op->command = CMD_SCSI;
    op->ગણતરી = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = કદ(*data);
    પાછા process_op(op);
}


// Request capacity
સ્થિર પૂર્ણાંક
cdb_read_capacity(સંરચના disk_op_s *op, સંરચના cdbres_read_capacity *data)
{
    સંરચના cdb_read_capacity cmd;
    memset(&cmd, 0, કદ(cmd));
    cmd.command = CDB_CMD_READ_CAPACITY;
    op->command = CMD_SCSI;
    op->ગણતરી = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = કદ(*data);
    પાછા process_op(op);
}

// Mode sense, geometry page.
સ્થિર પૂર્ણાંક
cdb_mode_sense_geom(સંરચના disk_op_s *op, સંરચના cdbres_mode_sense_geom *data)
{
    સંરચના cdb_mode_sense cmd;
    memset(&cmd, 0, કદ(cmd));
    cmd.command = CDB_CMD_MODE_SENSE;
    cmd.flags = 8; /* DBD */
    cmd.page = MODE_PAGE_HD_GEOMETRY;
    cmd.ગણતરી = cpu_to_be16(કદ(*data));
    op->command = CMD_SCSI;
    op->ગણતરી = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = કદ(*data);
    પાછા process_op(op);
}


/****************************************************************
 * Main SCSI commands
 ****************************************************************/

// Create a scsi command request from a disk_op_s request
પૂર્ણાંક
scsi_fill_cmd(સંરચના disk_op_s *op, શૂન્ય *cdbcmd, પૂર્ણાંક maxcdb)
{
    પસંદગી (op->command) {
    સ્થિતિ CMD_READ:
    સ્થિતિ CMD_WRITE: ;
        જો (op->lba < 0xFFFFFFFFULL) {
            સંરચના cdb_rwdata_10 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_10
                            : CDB_CMD_WRITE_10);
            cmd->lba = cpu_to_be32(op->lba);
            cmd->ગણતરી = cpu_to_be16(op->ગણતરી);
        } નહીંતર {
            સંરચના cdb_rwdata_16 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_16
                            : CDB_CMD_WRITE_16);
            cmd->lba = cpu_to_be64(op->lba);
            cmd->ગણતરી = cpu_to_be32(op->ગણતરી);
        }
        પાછા GET_FLATPTR(op->drive_fl->blksize);
    સ્થિતિ CMD_SCSI:
        જો (MODESEGMENT)
            પાછા -1;
        memcpy(cdbcmd, op->cdbcmd, maxcdb);
        પાછા op->blocksize;
    મૂળભૂત:
        પાછા -1;
    }
}

// Determine if the command is a request to pull data from the device
પૂર્ણાંક
scsi_is_read(સંરચના disk_op_s *op)
{
    પાછા op->command == CMD_READ || (
        !MODESEGMENT && op->command == CMD_SCSI && op->blocksize);
}

// Check if a SCSI device is ready to receive commands
પૂર્ણાંક
scsi_is_ready(સંરચના disk_op_s *op)
{
    ASSERT32FLAT();
    dprintf(6, "scsi_is_ready (drive=%p)\n", op->drive_fl);

    /* Retry TEST UNIT READY for 5 seconds unless MEDIUM NOT PRESENT is
     * reported by the device 3 times.  If the device reports "IN PROGRESS",
     * 30 seconds is added. */
    પૂર્ણાંક tries = 3;
    પૂર્ણાંક in_progress = 0;
    u32 end = timer_calc(5000);
    માટે (;;) {
        જો (timer_check(end)) {
            dprintf(1, "test unit ready failed\n");
            પાછા -1;
        }

        પૂર્ણાંક ret = cdb_test_unit_ready(op);
        જો (!ret)
            // Success
            તોડો;

        સંરચના cdbres_request_sense sense;
        ret = cdb_get_sense(op, &sense);
        જો (ret)
            // Error - retry.
            ચાલુ;

        // Sense succeeded.
        જો (sense.asc == 0x3a) { /* MEDIUM NOT PRESENT */
            tries--;
            dprintf(1, "Device reports MEDIUM NOT PRESENT - %d tries left\n",
                tries);
            જો (!tries)
                પાછા -1;
        }

        જો (sense.asc == 0x04 && sense.ascq == 0x01 && !in_progress) {
            /* IN PROGRESS OF BECOMING READY */
            dprintf(1, "Waiting for device to detect medium... ");
            /* Allow 30 seconds more */
            end = timer_calc(30000);
            in_progress = 1;
        }
    }
    પાછા 0;
}

#define CDB_CMD_REPORT_LUNS  0xA0

સંરચના cdb_report_luns {
    u8 command;
    u8 reserved_01[5];
    u32 length;
    u8 pad[6];
} PACKED;

સંરચના scsi_lun {
    u16 lun[4];
};

સંરચના cdbres_report_luns {
    u32 length;
    u32 reserved;
    સંરચના scsi_lun luns[];
};

સ્થિર u64 scsilun2u64(સંરચના scsi_lun *scsi_lun)
{
    પૂર્ણાંક i;
    u64 ret = 0;
    માટે (i = 0; i < ARRAY_SIZE(scsi_lun->lun); i++)
        ret |= be16_to_cpu(scsi_lun->lun[i]) << (16 * i);
    પાછા ret;
}

// Issue REPORT LUNS on a temporary drive and iterate reported luns calling
// @add_lun for each
પૂર્ણાંક scsi_rep_luns_scan(સંરચના drive_s *tmp_drive, scsi_add_lun add_lun)
{
    પૂર્ણાંક ret = -1;
    /* start with the smallest possible buffer, otherwise some devices in QEMU
     * may (incorrectly) error out on returning less data than fits in it */
    u32 maxluns = 1;
    u32 nluns, i;
    સંરચના cdb_report_luns cdb = {
        .command = CDB_CMD_REPORT_LUNS,
    };
    સંરચના disk_op_s op = {
        .drive_fl = tmp_drive,
        .command = CMD_SCSI,
        .ગણતરી = 1,
        .cdbcmd = &cdb,
    };
    સંરચના cdbres_report_luns *resp;

    ASSERT32FLAT();

    જ્યારે (1) {
        op.blocksize = કદ(સંરચના cdbres_report_luns) +
            maxluns * કદ(સંરચના scsi_lun);
        op.buf_fl = malloc_tmp(op.blocksize);
        જો (!op.buf_fl) {
            warn_noalloc();
            પાછા -1;
        }

        cdb.length = cpu_to_be32(op.blocksize);
        જો (process_op(&op) != DISK_RET_SUCCESS)
            જાઓ out;

        resp = op.buf_fl;
        nluns = be32_to_cpu(resp->length) / કદ(સંરચના scsi_lun);
        જો (nluns <= maxluns)
            તોડો;

        મુક્ત(op.buf_fl);
        maxluns = nluns;
    }

    માટે (i = 0, ret = 0; i < nluns; i++) {
        u64 lun = scsilun2u64(&resp->luns[i]);
        જો (lun >> 32)
            ચાલુ;
        ret += !add_lun((u32)lun, tmp_drive);
    }
out:
    મુક્ત(op.buf_fl);
    પાછા ret;
}

// Iterate LUNs on the target and call @add_lun for each
પૂર્ણાંક scsi_sequential_scan(સંરચના drive_s *tmp_drive, u32 maxluns,
                         scsi_add_lun add_lun)
{
    પૂર્ણાંક ret;
    u32 lun;

    માટે (lun = 0, ret = 0; lun < maxluns; lun++)
        ret += !add_lun(lun, tmp_drive);
    પાછા ret;
}

// Validate drive, find block size / sector count, and register drive.
પૂર્ણાંક
scsi_drive_setup(સંરચના drive_s *drive, અચલ અક્ષર *s, પૂર્ણાંક prio)
{
    ASSERT32FLAT();
    સંરચના disk_op_s dop;
    memset(&dop, 0, કદ(dop));
    dop.drive_fl = drive;
    સંરચના cdbres_inquiry data;
    પૂર્ણાંક ret = cdb_get_inquiry(&dop, &data);
    જો (ret)
        પાછા ret;
    અક્ષર vendor[કદ(data.vendor)+1], product[કદ(data.product)+1];
    અક્ષર rev[કદ(data.rev)+1];
    strtcpy(vendor, data.vendor, કદ(vendor));
    nullTrailingSpace(vendor);
    strtcpy(product, data.product, કદ(product));
    nullTrailingSpace(product);
    strtcpy(rev, data.rev, કદ(rev));
    nullTrailingSpace(rev);
    પૂર્ણાંક pdt = data.pdt & 0x1f;
    પૂર્ણાંક removable = !!(data.removable & 0x80);
    dprintf(1, "%s vendor='%s' product='%s' rev='%s' type=%d removable=%d\n"
            , s, vendor, product, rev, pdt, removable);
    drive->removable = removable;

    જો (pdt == SCSI_TYPE_CDROM) {
        drive->blksize = CDROM_SECTOR_SIZE;
        drive->sectors = (u64)-1;

        અક્ષર *desc = znprintf(MAXDESCSIZE, "DVD/CD [%s Drive %s %s %s]"
                              , s, vendor, product, rev);
        boot_add_cd(drive, desc, prio);
        પાછા 0;
    }

    જો (pdt != SCSI_TYPE_DISK)
        પાછા -1;

    ret = scsi_is_ready(&dop);
    જો (ret) {
        dprintf(1, "scsi_is_ready returned %d\n", ret);
        પાછા ret;
    }

    સંરચના cdbres_read_capacity capdata;
    ret = cdb_read_capacity(&dop, &capdata);
    જો (ret)
        પાછા ret;

    જો (be32_to_cpu(capdata.sectors) == 0xFFFFFFFFUL) {
        dprintf(3, "%s: >2TB Detected trying READCAP(16)\n", s);
        સંરચના cdbres_read_capacity_16 capdata16;
        ret = cdb_read_capacity16(&dop, &capdata16);
        drive->blksize = be32_to_cpu(capdata16.blksize);
        drive->sectors = be64_to_cpu(capdata16.sectors) + 1;
    } નહીંતર {
        drive->blksize = be32_to_cpu(capdata.blksize);
        drive->sectors = (u64)be32_to_cpu(capdata.sectors) + 1;
    }

    જો (drive->blksize != DISK_SECTOR_SIZE) {
        dprintf(1, "%s: unsupported block size %d\n", s, drive->blksize);
        પાછા -1;
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
    જો (CONFIG_QEMU_HARDWARE && memcmp(vendor, "QEMU", 5) == 0) {
        સંરચના cdbres_mode_sense_geom geomdata;
        ret = cdb_mode_sense_geom(&dop, &geomdata);
        જો (ret == 0) {
            u32 cylinders;
            cylinders = geomdata.cyl[0] << 16;
            cylinders |= geomdata.cyl[1] << 8;
            cylinders |= geomdata.cyl[2];
            જો (cylinders && geomdata.heads &&
                drive->sectors <= 0xFFFFFFFFULL &&
                ((u32)drive->sectors % (geomdata.heads * cylinders) == 0)) {
                drive->pchs.cylinder = cylinders;
                drive->pchs.head = geomdata.heads;
                drive->pchs.sector = (u32)drive->sectors / (geomdata.heads * cylinders);
            }
        }
    }

    અક્ષર *desc = znprintf(MAXDESCSIZE, "%s Drive %s %s %s"
                          , s, vendor, product, rev);
    boot_add_hd(drive, desc, prio);
    પાછા 0;
}

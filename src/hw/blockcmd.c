// Support for several common scsi like command data block requests
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "block.h" // struct disk_op_s
#अन्तर्भाव "blockcmd.h" // struct cdb_request_sense
#अन्तर्भाव "byteorder.h" // be32_to_cpu
#अन्तर्भाव "farptr.h" // GET_FLATPTR
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "std/disk.h" // DISK_RET_EPARAM
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // timer_calc
#अन्तर्भाव "malloc.h"


/****************************************************************
 * Low level command requests
 ****************************************************************/

स्थिर पूर्णाङ्क
cdb_get_inquiry(संरचना disk_op_s *op, संरचना cdbres_inquiry *data)
{
    संरचना cdb_request_sense cmd;
    memset(&cmd, 0, परिमाण(cmd));
    cmd.command = CDB_CMD_INQUIRY;
    cmd.length = परिमाण(*data);
    op->command = CMD_SCSI;
    op->गणनम् = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = परिमाण(*data);
    पटिनिवत्त process_op(op);
}

// Request SENSE
स्थिर पूर्णाङ्क
cdb_get_sense(संरचना disk_op_s *op, संरचना cdbres_request_sense *data)
{
    संरचना cdb_request_sense cmd;
    memset(&cmd, 0, परिमाण(cmd));
    cmd.command = CDB_CMD_REQUEST_SENSE;
    cmd.length = परिमाण(*data);
    op->command = CMD_SCSI;
    op->गणनम् = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = परिमाण(*data);
    पटिनिवत्त process_op(op);
}

// Test unit ready
स्थिर पूर्णाङ्क
cdb_test_unit_ready(संरचना disk_op_s *op)
{
    संरचना cdb_request_sense cmd;
    memset(&cmd, 0, परिमाण(cmd));
    cmd.command = CDB_CMD_TEST_UNIT_READY;
    op->command = CMD_SCSI;
    op->गणनम् = 0;
    op->buf_fl = रिक्त;
    op->cdbcmd = &cmd;
    op->blocksize = 0;
    पटिनिवत्त process_op(op);
}

स्थिर पूर्णाङ्क
cdb_read_capacity16(संरचना disk_op_s *op, संरचना cdbres_read_capacity_16 *data)
{
    संरचना cdb_sai_read_capacity_16 cmd;
    memset(&cmd, 0, परिमाण(cmd));
    cmd.command = CDB_CMD_SERVICE_ACTION_IN;
    cmd.flags = CDB_CMD_SAI_READ_CAPACITY_16;
    cmd.len = cpu_to_be32(परिमाण(संरचना cdbres_read_capacity_16));
    op->command = CMD_SCSI;
    op->गणनम् = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = परिमाण(*data);
    पटिनिवत्त process_op(op);
}


// Request capacity
स्थिर पूर्णाङ्क
cdb_read_capacity(संरचना disk_op_s *op, संरचना cdbres_read_capacity *data)
{
    संरचना cdb_read_capacity cmd;
    memset(&cmd, 0, परिमाण(cmd));
    cmd.command = CDB_CMD_READ_CAPACITY;
    op->command = CMD_SCSI;
    op->गणनम् = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = परिमाण(*data);
    पटिनिवत्त process_op(op);
}

// Mode sense, geometry page.
स्थिर पूर्णाङ्क
cdb_mode_sense_geom(संरचना disk_op_s *op, संरचना cdbres_mode_sense_geom *data)
{
    संरचना cdb_mode_sense cmd;
    memset(&cmd, 0, परिमाण(cmd));
    cmd.command = CDB_CMD_MODE_SENSE;
    cmd.flags = 8; /* DBD */
    cmd.page = MODE_PAGE_HD_GEOMETRY;
    cmd.गणनम् = cpu_to_be16(परिमाण(*data));
    op->command = CMD_SCSI;
    op->गणनम् = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = परिमाण(*data);
    पटिनिवत्त process_op(op);
}


/****************************************************************
 * Main SCSI commands
 ****************************************************************/

// Create a scsi command request from a disk_op_s request
पूर्णाङ्क
scsi_fill_cmd(संरचना disk_op_s *op, शून्य *cdbcmd, पूर्णाङ्क maxcdb)
{
    विकल्प (op->command) {
    पक्ष CMD_READ:
    पक्ष CMD_WRITE: ;
        सचे (op->lba < 0xFFFFFFFFULL) {
            संरचना cdb_rwdata_10 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_10
                            : CDB_CMD_WRITE_10);
            cmd->lba = cpu_to_be32(op->lba);
            cmd->गणनम् = cpu_to_be16(op->गणनम्);
        } अञ्ञथा {
            संरचना cdb_rwdata_16 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_16
                            : CDB_CMD_WRITE_16);
            cmd->lba = cpu_to_be64(op->lba);
            cmd->गणनम् = cpu_to_be32(op->गणनम्);
        }
        पटिनिवत्त GET_FLATPTR(op->drive_fl->blksize);
    पक्ष CMD_SCSI:
        सचे (MODESEGMENT)
            पटिनिवत्त -1;
        memcpy(cdbcmd, op->cdbcmd, maxcdb);
        पटिनिवत्त op->blocksize;
    स्वभाव:
        पटिनिवत्त -1;
    }
}

// Determine if the command is a request to pull data from the device
पूर्णाङ्क
scsi_is_read(संरचना disk_op_s *op)
{
    पटिनिवत्त op->command == CMD_READ || (
        !MODESEGMENT && op->command == CMD_SCSI && op->blocksize);
}

// Check if a SCSI device is ready to receive commands
पूर्णाङ्क
scsi_is_ready(संरचना disk_op_s *op)
{
    ASSERT32FLAT();
    dprintf(6, "scsi_is_ready (drive=%p)\n", op->drive_fl);

    /* Retry TEST UNIT READY for 5 seconds unless MEDIUM NOT PRESENT is
     * reported by the device 3 times.  If the device reports "IN PROGRESS",
     * 30 seconds is added. */
    पूर्णाङ्क tries = 3;
    पूर्णाङ्क in_progress = 0;
    u32 end = timer_calc(5000);
    अत्थाय (;;) {
        सचे (timer_check(end)) {
            dprintf(1, "test unit ready failed\n");
            पटिनिवत्त -1;
        }

        पूर्णाङ्क ret = cdb_test_unit_ready(op);
        सचे (!ret)
            // Success
            भङ्ग;

        संरचना cdbres_request_sense sense;
        ret = cdb_get_sense(op, &sense);
        सचे (ret)
            // Error - retry.
            अनुवर्त;

        // Sense succeeded.
        सचे (sense.asc == 0x3a) { /* MEDIUM NOT PRESENT */
            tries--;
            dprintf(1, "Device reports MEDIUM NOT PRESENT - %d tries left\n",
                tries);
            सचे (!tries)
                पटिनिवत्त -1;
        }

        सचे (sense.asc == 0x04 && sense.ascq == 0x01 && !in_progress) {
            /* IN PROGRESS OF BECOMING READY */
            dprintf(1, "Waiting for device to detect medium... ");
            /* Allow 30 seconds more */
            end = timer_calc(30000);
            in_progress = 1;
        }
    }
    पटिनिवत्त 0;
}

#define CDB_CMD_REPORT_LUNS  0xA0

संरचना cdb_report_luns {
    u8 command;
    u8 reserved_01[5];
    u32 length;
    u8 pad[6];
} PACKED;

संरचना scsi_lun {
    u16 lun[4];
};

संरचना cdbres_report_luns {
    u32 length;
    u32 reserved;
    संरचना scsi_lun luns[];
};

स्थिर u64 scsilun2u64(संरचना scsi_lun *scsi_lun)
{
    पूर्णाङ्क i;
    u64 ret = 0;
    अत्थाय (i = 0; i < ARRAY_SIZE(scsi_lun->lun); i++)
        ret |= be16_to_cpu(scsi_lun->lun[i]) << (16 * i);
    पटिनिवत्त ret;
}

// Issue REPORT LUNS on a temporary drive and iterate reported luns calling
// @add_lun for each
पूर्णाङ्क scsi_rep_luns_scan(संरचना drive_s *tmp_drive, scsi_add_lun add_lun)
{
    पूर्णाङ्क ret = -1;
    /* start with the smallest possible buffer, otherwise some devices in QEMU
     * may (incorrectly) error out on returning less data than fits in it */
    u32 maxluns = 1;
    u32 nluns, i;
    संरचना cdb_report_luns cdb = {
        .command = CDB_CMD_REPORT_LUNS,
    };
    संरचना disk_op_s op = {
        .drive_fl = tmp_drive,
        .command = CMD_SCSI,
        .गणनम् = 1,
        .cdbcmd = &cdb,
    };
    संरचना cdbres_report_luns *resp;

    ASSERT32FLAT();

    याव (1) {
        op.blocksize = परिमाण(संरचना cdbres_report_luns) +
            maxluns * परिमाण(संरचना scsi_lun);
        op.buf_fl = malloc_tmp(op.blocksize);
        सचे (!op.buf_fl) {
            warn_noalloc();
            पटिनिवत्त -1;
        }

        cdb.length = cpu_to_be32(op.blocksize);
        सचे (process_op(&op) != DISK_RET_SUCCESS)
            गच्छ out;

        resp = op.buf_fl;
        nluns = be32_to_cpu(resp->length) / परिमाण(संरचना scsi_lun);
        सचे (nluns <= maxluns)
            भङ्ग;

        मुक्त(op.buf_fl);
        maxluns = nluns;
    }

    अत्थाय (i = 0, ret = 0; i < nluns; i++) {
        u64 lun = scsilun2u64(&resp->luns[i]);
        सचे (lun >> 32)
            अनुवर्त;
        ret += !add_lun((u32)lun, tmp_drive);
    }
out:
    मुक्त(op.buf_fl);
    पटिनिवत्त ret;
}

// Iterate LUNs on the target and call @add_lun for each
पूर्णाङ्क scsi_sequential_scan(संरचना drive_s *tmp_drive, u32 maxluns,
                         scsi_add_lun add_lun)
{
    पूर्णाङ्क ret;
    u32 lun;

    अत्थाय (lun = 0, ret = 0; lun < maxluns; lun++)
        ret += !add_lun(lun, tmp_drive);
    पटिनिवत्त ret;
}

// Validate drive, find block size / sector count, and register drive.
पूर्णाङ्क
scsi_drive_setup(संरचना drive_s *drive, ध्रुव वर्ण *s, पूर्णाङ्क prio)
{
    ASSERT32FLAT();
    संरचना disk_op_s dop;
    memset(&dop, 0, परिमाण(dop));
    dop.drive_fl = drive;
    संरचना cdbres_inquiry data;
    पूर्णाङ्क ret = cdb_get_inquiry(&dop, &data);
    सचे (ret)
        पटिनिवत्त ret;
    वर्ण vendor[परिमाण(data.vendor)+1], product[परिमाण(data.product)+1];
    वर्ण rev[परिमाण(data.rev)+1];
    strtcpy(vendor, data.vendor, परिमाण(vendor));
    nullTrailingSpace(vendor);
    strtcpy(product, data.product, परिमाण(product));
    nullTrailingSpace(product);
    strtcpy(rev, data.rev, परिमाण(rev));
    nullTrailingSpace(rev);
    पूर्णाङ्क pdt = data.pdt & 0x1f;
    पूर्णाङ्क removable = !!(data.removable & 0x80);
    dprintf(1, "%s vendor='%s' product='%s' rev='%s' type=%d removable=%d\n"
            , s, vendor, product, rev, pdt, removable);
    drive->removable = removable;

    सचे (pdt == SCSI_TYPE_CDROM) {
        drive->blksize = CDROM_SECTOR_SIZE;
        drive->sectors = (u64)-1;

        वर्ण *desc = znprintf(MAXDESCSIZE, "DVD/CD [%s Drive %s %s %s]"
                              , s, vendor, product, rev);
        boot_add_cd(drive, desc, prio);
        पटिनिवत्त 0;
    }

    सचे (pdt != SCSI_TYPE_DISK)
        पटिनिवत्त -1;

    ret = scsi_is_ready(&dop);
    सचे (ret) {
        dprintf(1, "scsi_is_ready returned %d\n", ret);
        पटिनिवत्त ret;
    }

    संरचना cdbres_read_capacity capdata;
    ret = cdb_read_capacity(&dop, &capdata);
    सचे (ret)
        पटिनिवत्त ret;

    सचे (be32_to_cpu(capdata.sectors) == 0xFFFFFFFFUL) {
        dprintf(3, "%s: >2TB Detected trying READCAP(16)\n", s);
        संरचना cdbres_read_capacity_16 capdata16;
        ret = cdb_read_capacity16(&dop, &capdata16);
        drive->blksize = be32_to_cpu(capdata16.blksize);
        drive->sectors = be64_to_cpu(capdata16.sectors) + 1;
    } अञ्ञथा {
        drive->blksize = be32_to_cpu(capdata.blksize);
        drive->sectors = (u64)be32_to_cpu(capdata.sectors) + 1;
    }

    सचे (drive->blksize != DISK_SECTOR_SIZE) {
        dprintf(1, "%s: unsupported block size %d\n", s, drive->blksize);
        पटिनिवत्त -1;
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
    सचे (CONFIG_QEMU_HARDWARE && memcmp(vendor, "QEMU", 5) == 0) {
        संरचना cdbres_mode_sense_geom geomdata;
        ret = cdb_mode_sense_geom(&dop, &geomdata);
        सचे (ret == 0) {
            u32 cylinders;
            cylinders = geomdata.cyl[0] << 16;
            cylinders |= geomdata.cyl[1] << 8;
            cylinders |= geomdata.cyl[2];
            सचे (cylinders && geomdata.heads &&
                drive->sectors <= 0xFFFFFFFFULL &&
                ((u32)drive->sectors % (geomdata.heads * cylinders) == 0)) {
                drive->pchs.cylinder = cylinders;
                drive->pchs.head = geomdata.heads;
                drive->pchs.sector = (u32)drive->sectors / (geomdata.heads * cylinders);
            }
        }
    }

    वर्ण *desc = znprintf(MAXDESCSIZE, "%s Drive %s %s %s"
                          , s, vendor, product, rev);
    boot_add_hd(drive, desc, prio);
    पटिनिवत्त 0;
}

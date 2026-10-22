// Support for several common scsi like command data block requests
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "block.h" // struct disk_op_s
#ଅନ୍ତର୍ଭାଵ "blockcmd.h" // struct cdb_request_sense
#ଅନ୍ତର୍ଭାଵ "byteorder.h" // be32_to_cpu
#ଅନ୍ତର୍ଭାଵ "farptr.h" // GET_FLATPTR
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "std/disk.h" // DISK_RET_EPARAM
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // timer_calc
#ଅନ୍ତର୍ଭାଵ "malloc.h"


/****************************************************************
 * Low level command requests
 ****************************************************************/

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
cdb_get_inquiry(ସଂରଚନା disk_op_s *op, ସଂରଚନା cdbres_inquiry *data)
{
    ସଂରଚନା cdb_request_sense cmd;
    memset(&cmd, 0, ଆକାର(cmd));
    cmd.command = CDB_CMD_INQUIRY;
    cmd.length = ଆକାର(*data);
    op->command = CMD_SCSI;
    op->ଗଣନା_2 = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = ଆକାର(*data);
    ଫେରନ୍ତୁ process_op(op);
}

// Request SENSE
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
cdb_get_sense(ସଂରଚନା disk_op_s *op, ସଂରଚନା cdbres_request_sense *data)
{
    ସଂରଚନା cdb_request_sense cmd;
    memset(&cmd, 0, ଆକାର(cmd));
    cmd.command = CDB_CMD_REQUEST_SENSE;
    cmd.length = ଆକାର(*data);
    op->command = CMD_SCSI;
    op->ଗଣନା_2 = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = ଆକାର(*data);
    ଫେରନ୍ତୁ process_op(op);
}

// Test unit ready
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
cdb_test_unit_ready(ସଂରଚନା disk_op_s *op)
{
    ସଂରଚନା cdb_request_sense cmd;
    memset(&cmd, 0, ଆକାର(cmd));
    cmd.command = CDB_CMD_TEST_UNIT_READY;
    op->command = CMD_SCSI;
    op->ଗଣନା_2 = 0;
    op->buf_fl = ଖାଲି;
    op->cdbcmd = &cmd;
    op->blocksize = 0;
    ଫେରନ୍ତୁ process_op(op);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
cdb_read_capacity16(ସଂରଚନା disk_op_s *op, ସଂରଚନା cdbres_read_capacity_16 *data)
{
    ସଂରଚନା cdb_sai_read_capacity_16 cmd;
    memset(&cmd, 0, ଆକାର(cmd));
    cmd.command = CDB_CMD_SERVICE_ACTION_IN;
    cmd.flags = CDB_CMD_SAI_READ_CAPACITY_16;
    cmd.len = cpu_to_be32(ଆକାର(ସଂରଚନା cdbres_read_capacity_16));
    op->command = CMD_SCSI;
    op->ଗଣନା_2 = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = ଆକାର(*data);
    ଫେରନ୍ତୁ process_op(op);
}


// Request capacity
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
cdb_read_capacity(ସଂରଚନା disk_op_s *op, ସଂରଚନା cdbres_read_capacity *data)
{
    ସଂରଚନା cdb_read_capacity cmd;
    memset(&cmd, 0, ଆକାର(cmd));
    cmd.command = CDB_CMD_READ_CAPACITY;
    op->command = CMD_SCSI;
    op->ଗଣନା_2 = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = ଆକାର(*data);
    ଫେରନ୍ତୁ process_op(op);
}

// Mode sense, geometry page.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
cdb_mode_sense_geom(ସଂରଚନା disk_op_s *op, ସଂରଚନା cdbres_mode_sense_geom *data)
{
    ସଂରଚନା cdb_mode_sense cmd;
    memset(&cmd, 0, ଆକାର(cmd));
    cmd.command = CDB_CMD_MODE_SENSE;
    cmd.flags = 8; /* DBD */
    cmd.page = MODE_PAGE_HD_GEOMETRY;
    cmd.ଗଣନା_2 = cpu_to_be16(ଆକାର(*data));
    op->command = CMD_SCSI;
    op->ଗଣନା_2 = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = ଆକାର(*data);
    ଫେରନ୍ତୁ process_op(op);
}


/****************************************************************
 * Main SCSI commands
 ****************************************************************/

// Create a scsi command request from a disk_op_s request
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
scsi_fill_cmd(ସଂରଚନା disk_op_s *op, ଶୂନ୍ୟ *cdbcmd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxcdb)
{
    ବାଛ (op->command) {
    ସ୍ଥିତି CMD_READ:
    ସ୍ଥିତି CMD_WRITE: ;
        ଯଦି (op->lba < 0xFFFFFFFFULL) {
            ସଂରଚନା cdb_rwdata_10 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_10
                            : CDB_CMD_WRITE_10);
            cmd->lba = cpu_to_be32(op->lba);
            cmd->ଗଣନା_2 = cpu_to_be16(op->ଗଣନା_2);
        } ଅନ୍ୟଥା {
            ସଂରଚନା cdb_rwdata_16 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_16
                            : CDB_CMD_WRITE_16);
            cmd->lba = cpu_to_be64(op->lba);
            cmd->ଗଣନା_2 = cpu_to_be32(op->ଗଣନା_2);
        }
        ଫେରନ୍ତୁ GET_FLATPTR(op->drive_fl->blksize);
    ସ୍ଥିତି CMD_SCSI:
        ଯଦି (MODESEGMENT)
            ଫେରନ୍ତୁ -1;
        memcpy(cdbcmd, op->cdbcmd, maxcdb);
        ଫେରନ୍ତୁ op->blocksize;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ଫେରନ୍ତୁ -1;
    }
}

// Determine if the command is a request to pull data from the device
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
scsi_is_read(ସଂରଚନା disk_op_s *op)
{
    ଫେରନ୍ତୁ op->command == CMD_READ || (
        !MODESEGMENT && op->command == CMD_SCSI && op->blocksize);
}

// Check if a SCSI device is ready to receive commands
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
scsi_is_ready(ସଂରଚନା disk_op_s *op)
{
    ASSERT32FLAT();
    dprintf(6, "scsi_is_ready (drive=%p)\n", op->drive_fl);

    /* Retry TEST UNIT READY for 5 seconds unless MEDIUM NOT PRESENT is
     * reported by the device 3 times.  If the device reports "IN PROGRESS",
     * 30 seconds is added. */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା tries = 3;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା in_progress = 0;
    u32 end = timer_calc(5000);
    ପାଇଁ (;;) {
        ଯଦି (timer_check(end)) {
            dprintf(1, "test unit ready failed\n");
            ଫେରନ୍ତୁ -1;
        }

        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = cdb_test_unit_ready(op);
        ଯଦି (!ret)
            // Success
            ଭାଙ୍ଗ;

        ସଂରଚନା cdbres_request_sense sense;
        ret = cdb_get_sense(op, &sense);
        ଯଦି (ret)
            // Error - retry.
            ଜାରି;

        // Sense succeeded.
        ଯଦି (sense.asc == 0x3a) { /* MEDIUM NOT PRESENT */
            tries--;
            dprintf(1, "Device reports MEDIUM NOT PRESENT - %d tries left\n",
                tries);
            ଯଦି (!tries)
                ଫେରନ୍ତୁ -1;
        }

        ଯଦି (sense.asc == 0x04 && sense.ascq == 0x01 && !in_progress) {
            /* IN PROGRESS OF BECOMING READY */
            dprintf(1, "Waiting for device to detect medium... ");
            /* Allow 30 seconds more */
            end = timer_calc(30000);
            in_progress = 1;
        }
    }
    ଫେରନ୍ତୁ 0;
}

#define CDB_CMD_REPORT_LUNS  0xA0

ସଂରଚନା cdb_report_luns {
    u8 command;
    u8 reserved_01[5];
    u32 length;
    u8 pad[6];
} PACKED;

ସଂରଚନା scsi_lun {
    u16 lun[4];
};

ସଂରଚନା cdbres_report_luns {
    u32 length;
    u32 reserved;
    ସଂରଚନା scsi_lun luns[];
};

ସ୍ଥିର u64 scsilun2u64(ସଂରଚନା scsi_lun *scsi_lun)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    u64 ret = 0;
    ପାଇଁ (i = 0; i < ARRAY_SIZE(scsi_lun->lun); i++)
        ret |= be16_to_cpu(scsi_lun->lun[i]) << (16 * i);
    ଫେରନ୍ତୁ ret;
}

// Issue REPORT LUNS on a temporary drive and iterate reported luns calling
// @add_lun for each
ପୂର୍ଣ୍ଣସଂଖ୍ୟା scsi_rep_luns_scan(ସଂରଚନା drive_s *tmp_drive, scsi_add_lun add_lun)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = -1;
    /* start with the smallest possible buffer, otherwise some devices in QEMU
     * may (incorrectly) error out on returning less data than fits in it */
    u32 maxluns = 1;
    u32 nluns, i;
    ସଂରଚନା cdb_report_luns cdb = {
        .command = CDB_CMD_REPORT_LUNS,
    };
    ସଂରଚନା disk_op_s op = {
        .drive_fl = tmp_drive,
        .command = CMD_SCSI,
        .ଗଣନା_2 = 1,
        .cdbcmd = &cdb,
    };
    ସଂରଚନା cdbres_report_luns *resp;

    ASSERT32FLAT();

    ଯେତେବେଳେ (1) {
        op.blocksize = ଆକାର(ସଂରଚନା cdbres_report_luns) +
            maxluns * ଆକାର(ସଂରଚନା scsi_lun);
        op.buf_fl = malloc_tmp(op.blocksize);
        ଯଦି (!op.buf_fl) {
            warn_noalloc();
            ଫେରନ୍ତୁ -1;
        }

        cdb.length = cpu_to_be32(op.blocksize);
        ଯଦି (process_op(&op) != DISK_RET_SUCCESS)
            ଯାଅ out;

        resp = op.buf_fl;
        nluns = be32_to_cpu(resp->length) / ଆକାର(ସଂରଚନା scsi_lun);
        ଯଦି (nluns <= maxluns)
            ଭାଙ୍ଗ;

        ମୁକ୍ତ(op.buf_fl);
        maxluns = nluns;
    }

    ପାଇଁ (i = 0, ret = 0; i < nluns; i++) {
        u64 lun = scsilun2u64(&resp->luns[i]);
        ଯଦି (lun >> 32)
            ଜାରି;
        ret += !add_lun((u32)lun, tmp_drive);
    }
out:
    ମୁକ୍ତ(op.buf_fl);
    ଫେରନ୍ତୁ ret;
}

// Iterate LUNs on the target and call @add_lun for each
ପୂର୍ଣ୍ଣସଂଖ୍ୟା scsi_sequential_scan(ସଂରଚନା drive_s *tmp_drive, u32 maxluns,
                         scsi_add_lun add_lun)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
    u32 lun;

    ପାଇଁ (lun = 0, ret = 0; lun < maxluns; lun++)
        ret += !add_lun(lun, tmp_drive);
    ଫେରନ୍ତୁ ret;
}

// Validate drive, find block size / sector count, and register drive.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
scsi_drive_setup(ସଂରଚନା drive_s *drive, ଅଚଳ ଅକ୍ଷର *s, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio)
{
    ASSERT32FLAT();
    ସଂରଚନା disk_op_s dop;
    memset(&dop, 0, ଆକାର(dop));
    dop.drive_fl = drive;
    ସଂରଚନା cdbres_inquiry data;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = cdb_get_inquiry(&dop, &data);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;
    ଅକ୍ଷର vendor[ଆକାର(data.vendor)+1], product[ଆକାର(data.product)+1];
    ଅକ୍ଷର rev[ଆକାର(data.rev)+1];
    strtcpy(vendor, data.vendor, ଆକାର(vendor));
    nullTrailingSpace(vendor);
    strtcpy(product, data.product, ଆକାର(product));
    nullTrailingSpace(product);
    strtcpy(rev, data.rev, ଆକାର(rev));
    nullTrailingSpace(rev);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା pdt = data.pdt & 0x1f;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା removable = !!(data.removable & 0x80);
    dprintf(1, "%s vendor='%s' product='%s' rev='%s' type=%d removable=%d\n"
            , s, vendor, product, rev, pdt, removable);
    drive->removable = removable;

    ଯଦି (pdt == SCSI_TYPE_CDROM) {
        drive->blksize = CDROM_SECTOR_SIZE;
        drive->sectors = (u64)-1;

        ଅକ୍ଷର *desc = znprintf(MAXDESCSIZE, "DVD/CD [%s Drive %s %s %s]"
                              , s, vendor, product, rev);
        boot_add_cd(drive, desc, prio);
        ଫେରନ୍ତୁ 0;
    }

    ଯଦି (pdt != SCSI_TYPE_DISK)
        ଫେରନ୍ତୁ -1;

    ret = scsi_is_ready(&dop);
    ଯଦି (ret) {
        dprintf(1, "scsi_is_ready returned %d\n", ret);
        ଫେରନ୍ତୁ ret;
    }

    ସଂରଚନା cdbres_read_capacity capdata;
    ret = cdb_read_capacity(&dop, &capdata);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    ଯଦି (be32_to_cpu(capdata.sectors) == 0xFFFFFFFFUL) {
        dprintf(3, "%s: >2TB Detected trying READCAP(16)\n", s);
        ସଂରଚନା cdbres_read_capacity_16 capdata16;
        ret = cdb_read_capacity16(&dop, &capdata16);
        drive->blksize = be32_to_cpu(capdata16.blksize);
        drive->sectors = be64_to_cpu(capdata16.sectors) + 1;
    } ଅନ୍ୟଥା {
        drive->blksize = be32_to_cpu(capdata.blksize);
        drive->sectors = (u64)be32_to_cpu(capdata.sectors) + 1;
    }

    ଯଦି (drive->blksize != DISK_SECTOR_SIZE) {
        dprintf(1, "%s: unsupported block size %d\n", s, drive->blksize);
        ଫେରନ୍ତୁ -1;
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
    ଯଦି (CONFIG_QEMU_HARDWARE && memcmp(vendor, "QEMU", 5) == 0) {
        ସଂରଚନା cdbres_mode_sense_geom geomdata;
        ret = cdb_mode_sense_geom(&dop, &geomdata);
        ଯଦି (ret == 0) {
            u32 cylinders;
            cylinders = geomdata.cyl[0] << 16;
            cylinders |= geomdata.cyl[1] << 8;
            cylinders |= geomdata.cyl[2];
            ଯଦି (cylinders && geomdata.heads &&
                drive->sectors <= 0xFFFFFFFFULL &&
                ((u32)drive->sectors % (geomdata.heads * cylinders) == 0)) {
                drive->pchs.cylinder = cylinders;
                drive->pchs.head = geomdata.heads;
                drive->pchs.sector = (u32)drive->sectors / (geomdata.heads * cylinders);
            }
        }
    }

    ଅକ୍ଷର *desc = znprintf(MAXDESCSIZE, "%s Drive %s %s %s"
                          , s, vendor, product, rev);
    boot_add_hd(drive, desc, prio);
    ଫେରନ୍ତୁ 0;
}

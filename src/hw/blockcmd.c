// Support for several common scsi like command data block requests
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "block.h" // struct disk_op_s
#അന്തര്ഭാവ "blockcmd.h" // struct cdb_request_sense
#അന്തര്ഭാവ "byteorder.h" // be32_to_cpu
#അന്തര്ഭാവ "farptr.h" // GET_FLATPTR
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "std/disk.h" // DISK_RET_EPARAM
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // timer_calc
#അന്തര്ഭാവ "malloc.h"


/****************************************************************
 * Low level command requests
 ****************************************************************/

സ്ഥിതം പൂർണ്ണസംഖ്യ
cdb_get_inquiry(ഘടന disk_op_s *op, ഘടന cdbres_inquiry *data)
{
    ഘടന cdb_request_sense cmd;
    memset(&cmd, 0, വലുപ്പം(cmd));
    cmd.command = CDB_CMD_INQUIRY;
    cmd.length = വലുപ്പം(*data);
    op->command = CMD_SCSI;
    op->എണ്ണം = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = വലുപ്പം(*data);
    തിരികെ process_op(op);
}

// Request SENSE
സ്ഥിതം പൂർണ്ണസംഖ്യ
cdb_get_sense(ഘടന disk_op_s *op, ഘടന cdbres_request_sense *data)
{
    ഘടന cdb_request_sense cmd;
    memset(&cmd, 0, വലുപ്പം(cmd));
    cmd.command = CDB_CMD_REQUEST_SENSE;
    cmd.length = വലുപ്പം(*data);
    op->command = CMD_SCSI;
    op->എണ്ണം = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = വലുപ്പം(*data);
    തിരികെ process_op(op);
}

// Test unit ready
സ്ഥിതം പൂർണ്ണസംഖ്യ
cdb_test_unit_ready(ഘടന disk_op_s *op)
{
    ഘടന cdb_request_sense cmd;
    memset(&cmd, 0, വലുപ്പം(cmd));
    cmd.command = CDB_CMD_TEST_UNIT_READY;
    op->command = CMD_SCSI;
    op->എണ്ണം = 0;
    op->buf_fl = ശൂന്യ;
    op->cdbcmd = &cmd;
    op->blocksize = 0;
    തിരികെ process_op(op);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
cdb_read_capacity16(ഘടന disk_op_s *op, ഘടന cdbres_read_capacity_16 *data)
{
    ഘടന cdb_sai_read_capacity_16 cmd;
    memset(&cmd, 0, വലുപ്പം(cmd));
    cmd.command = CDB_CMD_SERVICE_ACTION_IN;
    cmd.flags = CDB_CMD_SAI_READ_CAPACITY_16;
    cmd.len = cpu_to_be32(വലുപ്പം(ഘടന cdbres_read_capacity_16));
    op->command = CMD_SCSI;
    op->എണ്ണം = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = വലുപ്പം(*data);
    തിരികെ process_op(op);
}


// Request capacity
സ്ഥിതം പൂർണ്ണസംഖ്യ
cdb_read_capacity(ഘടന disk_op_s *op, ഘടന cdbres_read_capacity *data)
{
    ഘടന cdb_read_capacity cmd;
    memset(&cmd, 0, വലുപ്പം(cmd));
    cmd.command = CDB_CMD_READ_CAPACITY;
    op->command = CMD_SCSI;
    op->എണ്ണം = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = വലുപ്പം(*data);
    തിരികെ process_op(op);
}

// Mode sense, geometry page.
സ്ഥിതം പൂർണ്ണസംഖ്യ
cdb_mode_sense_geom(ഘടന disk_op_s *op, ഘടന cdbres_mode_sense_geom *data)
{
    ഘടന cdb_mode_sense cmd;
    memset(&cmd, 0, വലുപ്പം(cmd));
    cmd.command = CDB_CMD_MODE_SENSE;
    cmd.flags = 8; /* DBD */
    cmd.page = MODE_PAGE_HD_GEOMETRY;
    cmd.എണ്ണം = cpu_to_be16(വലുപ്പം(*data));
    op->command = CMD_SCSI;
    op->എണ്ണം = 1;
    op->buf_fl = data;
    op->cdbcmd = &cmd;
    op->blocksize = വലുപ്പം(*data);
    തിരികെ process_op(op);
}


/****************************************************************
 * Main SCSI commands
 ****************************************************************/

// Create a scsi command request from a disk_op_s request
പൂർണ്ണസംഖ്യ
scsi_fill_cmd(ഘടന disk_op_s *op, ശൂന്യം *cdbcmd, പൂർണ്ണസംഖ്യ maxcdb)
{
    തിരഞ്ഞെടുപ്പ് (op->command) {
    അവസ്ഥ CMD_READ:
    അവസ്ഥ CMD_WRITE: ;
        എങ്കിൽ (op->lba < 0xFFFFFFFFULL) {
            ഘടന cdb_rwdata_10 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_10
                            : CDB_CMD_WRITE_10);
            cmd->lba = cpu_to_be32(op->lba);
            cmd->എണ്ണം = cpu_to_be16(op->എണ്ണം);
        } അല്ലെങ്കിൽ {
            ഘടന cdb_rwdata_16 *cmd = cdbcmd;
            memset(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_16
                            : CDB_CMD_WRITE_16);
            cmd->lba = cpu_to_be64(op->lba);
            cmd->എണ്ണം = cpu_to_be32(op->എണ്ണം);
        }
        തിരികെ GET_FLATPTR(op->drive_fl->blksize);
    അവസ്ഥ CMD_SCSI:
        എങ്കിൽ (MODESEGMENT)
            തിരികെ -1;
        memcpy(cdbcmd, op->cdbcmd, maxcdb);
        തിരികെ op->blocksize;
    സ്ഥിരസ്ഥിതി:
        തിരികെ -1;
    }
}

// Determine if the command is a request to pull data from the device
പൂർണ്ണസംഖ്യ
scsi_is_read(ഘടന disk_op_s *op)
{
    തിരികെ op->command == CMD_READ || (
        !MODESEGMENT && op->command == CMD_SCSI && op->blocksize);
}

// Check if a SCSI device is ready to receive commands
പൂർണ്ണസംഖ്യ
scsi_is_ready(ഘടന disk_op_s *op)
{
    ASSERT32FLAT();
    dprintf(6, "scsi_is_ready (drive=%p)\n", op->drive_fl);

    /* Retry TEST UNIT READY for 5 seconds unless MEDIUM NOT PRESENT is
     * reported by the device 3 times.  If the device reports "IN PROGRESS",
     * 30 seconds is added. */
    പൂർണ്ണസംഖ്യ tries = 3;
    പൂർണ്ണസംഖ്യ in_progress = 0;
    u32 end = timer_calc(5000);
    വേണ്ടി (;;) {
        എങ്കിൽ (timer_check(end)) {
            dprintf(1, "test unit ready failed\n");
            തിരികെ -1;
        }

        പൂർണ്ണസംഖ്യ ret = cdb_test_unit_ready(op);
        എങ്കിൽ (!ret)
            // Success
            നിർത്തുക;

        ഘടന cdbres_request_sense sense;
        ret = cdb_get_sense(op, &sense);
        എങ്കിൽ (ret)
            // Error - retry.
            തുടരുക;

        // Sense succeeded.
        എങ്കിൽ (sense.asc == 0x3a) { /* MEDIUM NOT PRESENT */
            tries--;
            dprintf(1, "Device reports MEDIUM NOT PRESENT - %d tries left\n",
                tries);
            എങ്കിൽ (!tries)
                തിരികെ -1;
        }

        എങ്കിൽ (sense.asc == 0x04 && sense.ascq == 0x01 && !in_progress) {
            /* IN PROGRESS OF BECOMING READY */
            dprintf(1, "Waiting for device to detect medium... ");
            /* Allow 30 seconds more */
            end = timer_calc(30000);
            in_progress = 1;
        }
    }
    തിരികെ 0;
}

#define CDB_CMD_REPORT_LUNS  0xA0

ഘടന cdb_report_luns {
    u8 command;
    u8 reserved_01[5];
    u32 length;
    u8 pad[6];
} PACKED;

ഘടന scsi_lun {
    u16 lun[4];
};

ഘടന cdbres_report_luns {
    u32 length;
    u32 reserved;
    ഘടന scsi_lun luns[];
};

സ്ഥിതം u64 scsilun2u64(ഘടന scsi_lun *scsi_lun)
{
    പൂർണ്ണസംഖ്യ i;
    u64 ret = 0;
    വേണ്ടി (i = 0; i < ARRAY_SIZE(scsi_lun->lun); i++)
        ret |= be16_to_cpu(scsi_lun->lun[i]) << (16 * i);
    തിരികെ ret;
}

// Issue REPORT LUNS on a temporary drive and iterate reported luns calling
// @add_lun for each
പൂർണ്ണസംഖ്യ scsi_rep_luns_scan(ഘടന drive_s *tmp_drive, scsi_add_lun add_lun)
{
    പൂർണ്ണസംഖ്യ ret = -1;
    /* start with the smallest possible buffer, otherwise some devices in QEMU
     * may (incorrectly) error out on returning less data than fits in it */
    u32 maxluns = 1;
    u32 nluns, i;
    ഘടന cdb_report_luns cdb = {
        .command = CDB_CMD_REPORT_LUNS,
    };
    ഘടന disk_op_s op = {
        .drive_fl = tmp_drive,
        .command = CMD_SCSI,
        .എണ്ണം = 1,
        .cdbcmd = &cdb,
    };
    ഘടന cdbres_report_luns *resp;

    ASSERT32FLAT();

    ആകുമ്പോൾ (1) {
        op.blocksize = വലുപ്പം(ഘടന cdbres_report_luns) +
            maxluns * വലുപ്പം(ഘടന scsi_lun);
        op.buf_fl = malloc_tmp(op.blocksize);
        എങ്കിൽ (!op.buf_fl) {
            warn_noalloc();
            തിരികെ -1;
        }

        cdb.length = cpu_to_be32(op.blocksize);
        എങ്കിൽ (process_op(&op) != DISK_RET_SUCCESS)
            പോകുക out;

        resp = op.buf_fl;
        nluns = be32_to_cpu(resp->length) / വലുപ്പം(ഘടന scsi_lun);
        എങ്കിൽ (nluns <= maxluns)
            നിർത്തുക;

        സ്വതന്ത്രം(op.buf_fl);
        maxluns = nluns;
    }

    വേണ്ടി (i = 0, ret = 0; i < nluns; i++) {
        u64 lun = scsilun2u64(&resp->luns[i]);
        എങ്കിൽ (lun >> 32)
            തുടരുക;
        ret += !add_lun((u32)lun, tmp_drive);
    }
out:
    സ്വതന്ത്രം(op.buf_fl);
    തിരികെ ret;
}

// Iterate LUNs on the target and call @add_lun for each
പൂർണ്ണസംഖ്യ scsi_sequential_scan(ഘടന drive_s *tmp_drive, u32 maxluns,
                         scsi_add_lun add_lun)
{
    പൂർണ്ണസംഖ്യ ret;
    u32 lun;

    വേണ്ടി (lun = 0, ret = 0; lun < maxluns; lun++)
        ret += !add_lun(lun, tmp_drive);
    തിരികെ ret;
}

// Validate drive, find block size / sector count, and register drive.
പൂർണ്ണസംഖ്യ
scsi_drive_setup(ഘടന drive_s *drive, സ്ഥിരം അക്ഷരം *s, പൂർണ്ണസംഖ്യ prio)
{
    ASSERT32FLAT();
    ഘടന disk_op_s dop;
    memset(&dop, 0, വലുപ്പം(dop));
    dop.drive_fl = drive;
    ഘടന cdbres_inquiry data;
    പൂർണ്ണസംഖ്യ ret = cdb_get_inquiry(&dop, &data);
    എങ്കിൽ (ret)
        തിരികെ ret;
    അക്ഷരം vendor[വലുപ്പം(data.vendor)+1], product[വലുപ്പം(data.product)+1];
    അക്ഷരം rev[വലുപ്പം(data.rev)+1];
    strtcpy(vendor, data.vendor, വലുപ്പം(vendor));
    nullTrailingSpace(vendor);
    strtcpy(product, data.product, വലുപ്പം(product));
    nullTrailingSpace(product);
    strtcpy(rev, data.rev, വലുപ്പം(rev));
    nullTrailingSpace(rev);
    പൂർണ്ണസംഖ്യ pdt = data.pdt & 0x1f;
    പൂർണ്ണസംഖ്യ removable = !!(data.removable & 0x80);
    dprintf(1, "%s vendor='%s' product='%s' rev='%s' type=%d removable=%d\n"
            , s, vendor, product, rev, pdt, removable);
    drive->removable = removable;

    എങ്കിൽ (pdt == SCSI_TYPE_CDROM) {
        drive->blksize = CDROM_SECTOR_SIZE;
        drive->sectors = (u64)-1;

        അക്ഷരം *desc = znprintf(MAXDESCSIZE, "DVD/CD [%s Drive %s %s %s]"
                              , s, vendor, product, rev);
        boot_add_cd(drive, desc, prio);
        തിരികെ 0;
    }

    എങ്കിൽ (pdt != SCSI_TYPE_DISK)
        തിരികെ -1;

    ret = scsi_is_ready(&dop);
    എങ്കിൽ (ret) {
        dprintf(1, "scsi_is_ready returned %d\n", ret);
        തിരികെ ret;
    }

    ഘടന cdbres_read_capacity capdata;
    ret = cdb_read_capacity(&dop, &capdata);
    എങ്കിൽ (ret)
        തിരികെ ret;

    എങ്കിൽ (be32_to_cpu(capdata.sectors) == 0xFFFFFFFFUL) {
        dprintf(3, "%s: >2TB Detected trying READCAP(16)\n", s);
        ഘടന cdbres_read_capacity_16 capdata16;
        ret = cdb_read_capacity16(&dop, &capdata16);
        drive->blksize = be32_to_cpu(capdata16.blksize);
        drive->sectors = be64_to_cpu(capdata16.sectors) + 1;
    } അല്ലെങ്കിൽ {
        drive->blksize = be32_to_cpu(capdata.blksize);
        drive->sectors = (u64)be32_to_cpu(capdata.sectors) + 1;
    }

    എങ്കിൽ (drive->blksize != DISK_SECTOR_SIZE) {
        dprintf(1, "%s: unsupported block size %d\n", s, drive->blksize);
        തിരികെ -1;
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
    എങ്കിൽ (CONFIG_QEMU_HARDWARE && memcmp(vendor, "QEMU", 5) == 0) {
        ഘടന cdbres_mode_sense_geom geomdata;
        ret = cdb_mode_sense_geom(&dop, &geomdata);
        എങ്കിൽ (ret == 0) {
            u32 cylinders;
            cylinders = geomdata.cyl[0] << 16;
            cylinders |= geomdata.cyl[1] << 8;
            cylinders |= geomdata.cyl[2];
            എങ്കിൽ (cylinders && geomdata.heads &&
                drive->sectors <= 0xFFFFFFFFULL &&
                ((u32)drive->sectors % (geomdata.heads * cylinders) == 0)) {
                drive->pchs.cylinder = cylinders;
                drive->pchs.head = geomdata.heads;
                drive->pchs.sector = (u32)drive->sectors / (geomdata.heads * cylinders);
            }
        }
    }

    അക്ഷരം *desc = znprintf(MAXDESCSIZE, "%s Drive %s %s %s"
                          , s, vendor, product, rev);
    boot_add_hd(drive, desc, prio);
    തിരികെ 0;
}

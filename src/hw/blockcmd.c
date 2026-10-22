// Support for several common scsi like command data block requests
//
// Copyright (C) 2010  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "block.h" // struct disk_op_s
#समावेश "blockcmd.h" // struct cdb_request_sense
#समावेश "byteorder.h" // be32_to_cpu
#समावेश "farptr.h" // GET_FLATPTR
#समावेश "output.h" // dprintf
#समावेश "std/disk.h" // DISK_RET_EPARAM
#समावेश "string.h" // memset
#समावेश "util.h" // timer_calc
#समावेश "malloc.h"


/****************************************************************
 * Low level command requests
 ****************************************************************/

अटल पूर्णांक
cdb_get_inquiry(काष्ठा disk_op_s *op, काष्ठा cdbres_inquiry *तथ्य)
{
    काष्ठा cdb_request_sense cmd;
    स_रखो(&cmd, 0, माप(cmd));
    cmd.command = CDB_CMD_INQUIRY;
    cmd.लंबाई = माप(*तथ्य);
    op->command = CMD_SCSI;
    op->गिनती = 1;
    op->buf_fl = तथ्य;
    op->cdbcmd = &cmd;
    op->blocksize = माप(*तथ्य);
    वापस process_op(op);
}

// Request SENSE
अटल पूर्णांक
cdb_get_sense(काष्ठा disk_op_s *op, काष्ठा cdbres_request_sense *तथ्य)
{
    काष्ठा cdb_request_sense cmd;
    स_रखो(&cmd, 0, माप(cmd));
    cmd.command = CDB_CMD_REQUEST_SENSE;
    cmd.लंबाई = माप(*तथ्य);
    op->command = CMD_SCSI;
    op->गिनती = 1;
    op->buf_fl = तथ्य;
    op->cdbcmd = &cmd;
    op->blocksize = माप(*तथ्य);
    वापस process_op(op);
}

// Test unit ready
अटल पूर्णांक
cdb_test_unit_ready(काष्ठा disk_op_s *op)
{
    काष्ठा cdb_request_sense cmd;
    स_रखो(&cmd, 0, माप(cmd));
    cmd.command = CDB_CMD_TEST_UNIT_READY;
    op->command = CMD_SCSI;
    op->गिनती = 0;
    op->buf_fl = शून्य;
    op->cdbcmd = &cmd;
    op->blocksize = 0;
    वापस process_op(op);
}

अटल पूर्णांक
cdb_read_capacity16(काष्ठा disk_op_s *op, काष्ठा cdbres_read_capacity_16 *तथ्य)
{
    काष्ठा cdb_sai_read_capacity_16 cmd;
    स_रखो(&cmd, 0, माप(cmd));
    cmd.command = CDB_CMD_SERVICE_ACTION_IN;
    cmd.flags = CDB_CMD_SAI_READ_CAPACITY_16;
    cmd.len = cpu_to_be32(माप(काष्ठा cdbres_read_capacity_16));
    op->command = CMD_SCSI;
    op->गिनती = 1;
    op->buf_fl = तथ्य;
    op->cdbcmd = &cmd;
    op->blocksize = माप(*तथ्य);
    वापस process_op(op);
}


// Request capacity
अटल पूर्णांक
cdb_read_capacity(काष्ठा disk_op_s *op, काष्ठा cdbres_read_capacity *तथ्य)
{
    काष्ठा cdb_read_capacity cmd;
    स_रखो(&cmd, 0, माप(cmd));
    cmd.command = CDB_CMD_READ_CAPACITY;
    op->command = CMD_SCSI;
    op->गिनती = 1;
    op->buf_fl = तथ्य;
    op->cdbcmd = &cmd;
    op->blocksize = माप(*तथ्य);
    वापस process_op(op);
}

// Mode sense, geometry page.
अटल पूर्णांक
cdb_mode_sense_geom(काष्ठा disk_op_s *op, काष्ठा cdbres_mode_sense_geom *तथ्य)
{
    काष्ठा cdb_mode_sense cmd;
    स_रखो(&cmd, 0, माप(cmd));
    cmd.command = CDB_CMD_MODE_SENSE;
    cmd.flags = 8; /* DBD */
    cmd.page = MODE_PAGE_HD_GEOMETRY;
    cmd.गिनती = cpu_to_be16(माप(*तथ्य));
    op->command = CMD_SCSI;
    op->गिनती = 1;
    op->buf_fl = तथ्य;
    op->cdbcmd = &cmd;
    op->blocksize = माप(*तथ्य);
    वापस process_op(op);
}


/****************************************************************
 * Main SCSI commands
 ****************************************************************/

// Create a scsi command request from a disk_op_s request
पूर्णांक
scsi_fill_cmd(काष्ठा disk_op_s *op, व्योम *cdbcmd, पूर्णांक maxcdb)
{
    चयन (op->command) {
    हाल CMD_READ:
    हाल CMD_WRITE: ;
        अगर (op->lba < 0xFFFFFFFFULL) {
            काष्ठा cdb_rwdata_10 *cmd = cdbcmd;
            स_रखो(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_10
                            : CDB_CMD_WRITE_10);
            cmd->lba = cpu_to_be32(op->lba);
            cmd->गिनती = cpu_to_be16(op->गिनती);
        } अन्यथा {
            काष्ठा cdb_rwdata_16 *cmd = cdbcmd;
            स_रखो(cmd, 0, maxcdb);
            cmd->command = (op->command == CMD_READ ? CDB_CMD_READ_16
                            : CDB_CMD_WRITE_16);
            cmd->lba = cpu_to_be64(op->lba);
            cmd->गिनती = cpu_to_be32(op->गिनती);
        }
        वापस GET_FLATPTR(op->drive_fl->blksize);
    हाल CMD_SCSI:
        अगर (MODESEGMENT)
            वापस -1;
        स_नकल(cdbcmd, op->cdbcmd, maxcdb);
        वापस op->blocksize;
    शेष:
        वापस -1;
    }
}

// Determine if the command is a request to pull data from the device
पूर्णांक
scsi_is_read(काष्ठा disk_op_s *op)
{
    वापस op->command == CMD_READ || (
        !MODESEGMENT && op->command == CMD_SCSI && op->blocksize);
}

// Check if a SCSI device is ready to receive commands
पूर्णांक
scsi_is_ready(काष्ठा disk_op_s *op)
{
    ASSERT32FLAT();
    dprintf(6, "scsi_is_ready (drive=%p)\n", op->drive_fl);

    /* Retry TEST UNIT READY for 5 seconds unless MEDIUM NOT PRESENT is
     * reported by the device 3 times.  If the device reports "IN PROGRESS",
     * 30 seconds is added. */
    पूर्णांक tries = 3;
    पूर्णांक in_progress = 0;
    u32 end = timer_calc(5000);
    क्रम (;;) {
        अगर (timer_check(end)) {
            dprintf(1, "test unit ready failed\n");
            वापस -1;
        }

        पूर्णांक ret = cdb_test_unit_ready(op);
        अगर (!ret)
            // Success
            अवरोध;

        काष्ठा cdbres_request_sense sense;
        ret = cdb_get_sense(op, &sense);
        अगर (ret)
            // Error - retry.
            जारी;

        // Sense succeeded.
        अगर (sense.asc == 0x3a) { /* MEDIUM NOT PRESENT */
            tries--;
            dprintf(1, "Device reports MEDIUM NOT PRESENT - %d tries left\n",
                tries);
            अगर (!tries)
                वापस -1;
        }

        अगर (sense.asc == 0x04 && sense.ascq == 0x01 && !in_progress) {
            /* IN PROGRESS OF BECOMING READY */
            dprintf(1, "Waiting for device to detect medium... ");
            /* Allow 30 seconds more */
            end = timer_calc(30000);
            in_progress = 1;
        }
    }
    वापस 0;
}

#घोषणा CDB_CMD_REPORT_LUNS  0xA0

काष्ठा cdb_report_luns {
    u8 command;
    u8 reserved_01[5];
    u32 लंबाई;
    u8 pad[6];
} PACKED;

काष्ठा scsi_lun {
    u16 lun[4];
};

काष्ठा cdbres_report_luns {
    u32 लंबाई;
    u32 reserved;
    काष्ठा scsi_lun luns[];
};

अटल u64 scsilun2u64(काष्ठा scsi_lun *scsi_lun)
{
    पूर्णांक i;
    u64 ret = 0;
    क्रम (i = 0; i < ARRAY_आकार(scsi_lun->lun); i++)
        ret |= be16_to_cpu(scsi_lun->lun[i]) << (16 * i);
    वापस ret;
}

// Issue REPORT LUNS on a temporary drive and iterate reported luns calling
// @add_lun for each
पूर्णांक scsi_rep_luns_scan(काष्ठा drive_s *tmp_drive, scsi_add_lun add_lun)
{
    पूर्णांक ret = -1;
    /* start with the smallest possible buffer, otherwise some devices in QEMU
     * may (incorrectly) error out on returning less data than fits in it */
    u32 maxluns = 1;
    u32 nluns, i;
    काष्ठा cdb_report_luns cdb = {
        .command = CDB_CMD_REPORT_LUNS,
    };
    काष्ठा disk_op_s op = {
        .drive_fl = tmp_drive,
        .command = CMD_SCSI,
        .गिनती = 1,
        .cdbcmd = &cdb,
    };
    काष्ठा cdbres_report_luns *resp;

    ASSERT32FLAT();

    जबतक (1) {
        op.blocksize = माप(काष्ठा cdbres_report_luns) +
            maxluns * माप(काष्ठा scsi_lun);
        op.buf_fl = malloc_tmp(op.blocksize);
        अगर (!op.buf_fl) {
            warn_noalloc();
            वापस -1;
        }

        cdb.लंबाई = cpu_to_be32(op.blocksize);
        अगर (process_op(&op) != DISK_RET_SUCCESS)
            जाओ out;

        resp = op.buf_fl;
        nluns = be32_to_cpu(resp->लंबाई) / माप(काष्ठा scsi_lun);
        अगर (nluns <= maxluns)
            अवरोध;

        मुक्त(op.buf_fl);
        maxluns = nluns;
    }

    क्रम (i = 0, ret = 0; i < nluns; i++) {
        u64 lun = scsilun2u64(&resp->luns[i]);
        अगर (lun >> 32)
            जारी;
        ret += !add_lun((u32)lun, tmp_drive);
    }
out:
    मुक्त(op.buf_fl);
    वापस ret;
}

// Iterate LUNs on the target and call @add_lun for each
पूर्णांक scsi_sequential_scan(काष्ठा drive_s *tmp_drive, u32 maxluns,
                         scsi_add_lun add_lun)
{
    पूर्णांक ret;
    u32 lun;

    क्रम (lun = 0, ret = 0; lun < maxluns; lun++)
        ret += !add_lun(lun, tmp_drive);
    वापस ret;
}

// Validate drive, find block size / sector count, and register drive.
पूर्णांक
scsi_drive_setup(काष्ठा drive_s *drive, स्थिर अक्षर *s, पूर्णांक prio)
{
    ASSERT32FLAT();
    काष्ठा disk_op_s dop;
    स_रखो(&dop, 0, माप(dop));
    dop.drive_fl = drive;
    काष्ठा cdbres_inquiry तथ्य;
    पूर्णांक ret = cdb_get_inquiry(&dop, &तथ्य);
    अगर (ret)
        वापस ret;
    अक्षर vendor[माप(तथ्य.vendor)+1], product[माप(तथ्य.product)+1];
    अक्षर rev[माप(तथ्य.rev)+1];
    strtcpy(vendor, तथ्य.vendor, माप(vendor));
    nullTrailingSpace(vendor);
    strtcpy(product, तथ्य.product, माप(product));
    nullTrailingSpace(product);
    strtcpy(rev, तथ्य.rev, माप(rev));
    nullTrailingSpace(rev);
    पूर्णांक pdt = तथ्य.pdt & 0x1f;
    पूर्णांक removable = !!(तथ्य.removable & 0x80);
    dprintf(1, "%s vendor='%s' product='%s' rev='%s' type=%d removable=%d\n"
            , s, vendor, product, rev, pdt, removable);
    drive->removable = removable;

    अगर (pdt == SCSI_TYPE_CDROM) {
        drive->blksize = CDROM_SECTOR_आकार;
        drive->sectors = (u64)-1;

        अक्षर *desc = znprintf(MAXDESCSIZE, "DVD/CD [%s Drive %s %s %s]"
                              , s, vendor, product, rev);
        boot_add_cd(drive, desc, prio);
        वापस 0;
    }

    अगर (pdt != SCSI_TYPE_DISK)
        वापस -1;

    ret = scsi_is_ready(&dop);
    अगर (ret) {
        dprintf(1, "scsi_is_ready returned %d\n", ret);
        वापस ret;
    }

    काष्ठा cdbres_read_capacity capdata;
    ret = cdb_read_capacity(&dop, &capdata);
    अगर (ret)
        वापस ret;

    अगर (be32_to_cpu(capdata.sectors) == 0xFFFFFFFFUL) {
        dprintf(3, "%s: >2TB Detected trying READCAP(16)\n", s);
        काष्ठा cdbres_read_capacity_16 capdata16;
        ret = cdb_read_capacity16(&dop, &capdata16);
        drive->blksize = be32_to_cpu(capdata16.blksize);
        drive->sectors = be64_to_cpu(capdata16.sectors) + 1;
    } अन्यथा {
        drive->blksize = be32_to_cpu(capdata.blksize);
        drive->sectors = (u64)be32_to_cpu(capdata.sectors) + 1;
    }

    अगर (drive->blksize != DISK_SECTOR_आकार) {
        dprintf(1, "%s: unsupported block size %d\n", s, drive->blksize);
        वापस -1;
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
    अगर (CONFIG_QEMU_HARDWARE && स_भेद(vendor, "QEMU", 5) == 0) {
        काष्ठा cdbres_mode_sense_geom geomdata;
        ret = cdb_mode_sense_geom(&dop, &geomdata);
        अगर (ret == 0) {
            u32 cylinders;
            cylinders = geomdata.cyl[0] << 16;
            cylinders |= geomdata.cyl[1] << 8;
            cylinders |= geomdata.cyl[2];
            अगर (cylinders && geomdata.heads &&
                drive->sectors <= 0xFFFFFFFFULL &&
                ((u32)drive->sectors % (geomdata.heads * cylinders) == 0)) {
                drive->pchs.cylinder = cylinders;
                drive->pchs.शीर्ष = geomdata.heads;
                drive->pchs.sector = (u32)drive->sectors / (geomdata.heads * cylinders);
            }
        }
    }

    अक्षर *desc = znprintf(MAXDESCSIZE, "%s Drive %s %s %s"
                          , s, vendor, product, rev);
    boot_add_hd(drive, desc, prio);
    वापस 0;
}

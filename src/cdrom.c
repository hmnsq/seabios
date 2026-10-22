// Support for booting from cdroms (the "El Torito" spec).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_GLOBAL
#అన్తర్భావ "block.h" // struct drive_s
#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "hw/ata.h" // ATA_CMD_REQUEST_SENSE
#అన్తర్భావ "hw/blockcmd.h" // CDB_CMD_REQUEST_SENSE
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "std/disk.h" // DISK_RET_SUCCESS
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // cdrom_prepboot
#అన్తర్భావ "tcgbios.h" // tpm_*


/****************************************************************
 * CD emulation
 ****************************************************************/

నిర్మాణం eltorito_s CDEmu VARLOW = { .కొలత=పరిమాణం(CDEmu) };
నిర్మాణం drive_s *emulated_drive_gf VARLOW;
నిర్మాణం drive_s *cdemu_drive_gf VARFSEG;

స్థిర పూర్ణాంకం
cdemu_read(నిర్మాణం disk_op_s *op)
{
    నిర్మాణం drive_s *drive_gf = GET_దిగువ(emulated_drive_gf);
    నిర్మాణం disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_దిగువ(CDEmu.ilba) + op->lba / 4;

    పూర్ణాంకం లెక్క = op->లెక్క;
    op->లెక్క = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    ఒకవేళ (op->lba & 3) {
        // Partial read of first block.
        dop.లెక్క = 1;
        dop.buf_fl = cdbuf_fl;
        పూర్ణాంకం ret = process_op(&dop);
        ఒకవేళ (ret)
            తిరిగి ret;
        u8 thiscount = 4 - (op->lba & 3);
        ఒకవేళ (thiscount > లెక్క)
            thiscount = లెక్క;
        లెక్క -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->లెక్క += thiscount;
        dop.lba++;
    }

    ఒకవేళ (లెక్క > 3) {
        // Read n number of regular blocks.
        dop.లెక్క = లెక్క / 4;
        dop.buf_fl = op->buf_fl;
        పూర్ణాంకం ret = process_op(&dop);
        op->లెక్క += dop.లెక్క * 4;
        ఒకవేళ (ret)
            తిరిగి ret;
        u8 thiscount = లెక్క & ~3;
        లెక్క &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    ఒకవేళ (లెక్క) {
        // Partial read on last block.
        dop.లెక్క = 1;
        dop.buf_fl = cdbuf_fl;
        పూర్ణాంకం ret = process_op(&dop);
        ఒకవేళ (ret)
            తిరిగి ret;
        u8 thiscount = లెక్క;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->లెక్క += thiscount;
    }

    తిరిగి DISK_RET_SUCCESS;
}

పూర్ణాంకం
cdemu_process_op(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (!CONFIG_CDROM_EMU)
        తిరిగి 0;

    ఎంపిక (op->command) {
    స్థితి CMD_READ:
        తిరిగి cdemu_read(op);
    స్థితి CMD_WRITE:
    స్థితి CMD_FORMAT:
        తిరిగి DISK_RET_EWRITEPROTECT;
    అప్రమేయ:
        తిరిగి default_process_op(op);
    }
}

శూన్య
cdrom_prepboot(శూన్య)
{
    ఒకవేళ (!CONFIG_CDROM_EMU)
        తిరిగి;
    ఒకవేళ (!CDCount)
        తిరిగి;
    ఒకవేళ (create_bounce_buf() < 0)
        తిరిగి;

    నిర్మాణం drive_s *drive = malloc_fseg(పరిమాణం(*drive));
    ఒకవేళ (!drive) {
        warn_noalloc();
        తిరిగి;
    }
    cdemu_drive_gf = drive;
    memset(drive, 0, పరిమాణం(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_కొలత;
    drive->sectors = (u64)-1;
}


/****************************************************************
 * CD booting
 ****************************************************************/

పూర్ణాంకం
cdrom_boot(నిర్మాణం drive_s *drive)
{
    ASSERT32FLAT();
    నిర్మాణం disk_op_s dop;
    పూర్ణాంకం cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, పరిమాణం(dop));
    dop.drive_fl = drive;
    ఒకవేళ (!dop.drive_fl || cdid < 0)
        తిరిగి 1;

    పూర్ణాంకం ret = scsi_is_ready(&dop);
    ఒకవేళ (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 బఫరు[CDROM_SECTOR_కొలత];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.లెక్క = 1;
    dop.buf_fl = బఫరు;
    ret = process_op(&dop);
    ఒకవేళ (ret)
        తిరిగి 3;

    // Validity checks
    ఒకవేళ (బఫరు[0])
        తిరిగి 4;
    ఒకవేళ (strcmp((అక్షరం*)&బఫరు[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        తిరిగి 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&బఫరు[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.లెక్క = 1;
    ret = process_op(&dop);
    ఒకవేళ (ret)
        తిరిగి 7;

    // Validation entry
    ఒకవేళ (బఫరు[0x00] != 0x01)
        తిరిగి 8;   //శీర్షిక
    ఒకవేళ (బఫరు[0x01] != 0x00)
        తిరిగి 9;   // Platform
    ఒకవేళ (బఫరు[0x1E] != 0x55)
        తిరిగి 10;  // key 1
    ఒకవేళ (బఫరు[0x1F] != 0xAA)
        తిరిగి 10;  // key 2

    // Initial/Default Entry
    ఒకవేళ (బఫరు[0x20] != 0x88)
        తిరిగి 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_కూడు_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), బఫరు), పరిమాణం(బఫరు));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = బఫరు[0x21];

    u16 boot_segment = *(u16*)&బఫరు[0x22];
    ఒకవేళ (!boot_segment)
        boot_segment = 0x07C0;
    CDEmu.ఎక్కించు_segment = boot_segment;
    CDEmu.బఫరు_segment = 0x0000;

    u16 nbsectors = *(u16*)&బఫరు[0x26];
    CDEmu.sector_లెక్క = nbsectors;

    lba = *(u32*)&బఫరు[0x28];
    CDEmu.ilba = lba;

    CDEmu.controller_సూచీ = drive->cntl_id / 2;
    CDEmu.device_spec = drive->cntl_id % 2;

    // And we read the image in memory
    nbsectors = DIV_విడత_పైకి(nbsectors, 4);
    dop.lba = lba;
    dop.buf_fl = MAKE_FLATPTR(boot_segment, 0);
    అయితే (nbsectors) {
        పూర్ణాంకం లెక్క = nbsectors;
        ఒకవేళ (లెక్క > 64*1024/CDROM_SECTOR_కొలత)
            లెక్క = 64*1024/CDROM_SECTOR_కొలత;
        dop.లెక్క = లెక్క;
        ret = process_op(&dop);
        ఒకవేళ (ret)
            తిరిగి 12;
        nbsectors -= లెక్క;
        dop.lba += లెక్క;
        dop.buf_fl += లెక్క*CDROM_SECTOR_కొలత;
    }

    ఒకవేళ (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        తిరిగి 0;
    }

    // Emulation of a floppy/harddisk requested
    ఒకవేళ (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        తిరిగి 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    ఒకవేళ (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        సమితి_equipment_flags(0x41, 0x41);

        ఎంపిక (media) {
        స్థితి 0x01:  // 1.2M floppy
            CDEmu.chs.sptcyl = 15;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            ఆపు;
        స్థితి 0x02:  // 1.44M floppy
            CDEmu.chs.sptcyl = 18;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            ఆపు;
        స్థితి 0x03:  // 2.88M floppy
            CDEmu.chs.sptcyl = 36;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            ఆపు;
        }
    } లేకపోతే {
        // Harddrive emulation
        CDEmu.emulated_drive = 0x80;
        సమితి_BDA(hdcount, GET_BDA(hdcount) + 1);

        // Peak at partition table to get chs.
        నిర్మాణం mbr_s *mbr = MAKE_FLATPTR(boot_segment, 0);
        CDEmu.chs = mbr->partitions[0].చివరి;
    }

    // everything is ok, so from now on, the emulation is active
    CDEmu.media = media;
    dprintf(6, "cdemu media=%d\n", media);

    తిరిగి 0;
}

// check if media is present and the drive is bootable.
// in case it is return the volume label.
అక్షరం*
cdrom_media_info(నిర్మాణం drive_s *drive)
{
    ASSERT32FLAT();

    నిర్మాణం disk_op_s dop;
    memset(&dop, 0, పరిమాణం(dop));
    dop.drive_fl = drive;

    పూర్ణాంకం ret = scsi_is_ready(&dop);
    ఒకవేళ (ret)
        తిరిగి ఖాళీ;

    // Read the Boot Record Volume Descriptor
    u8 బఫరు[CDROM_SECTOR_కొలత];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.లెక్క = 1;
    dop.buf_fl = బఫరు;
    ret = process_op(&dop);
    ఒకవేళ (ret)
        తిరిగి ఖాళీ;

    // Is it bootable?
    ఒకవేళ (బఫరు[0])
        తిరిగి ఖాళీ;
    ఒకవేళ (strcmp((అక్షరం*)&బఫరు[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        తిరిగి ఖాళీ;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.లెక్క = 1;
    dop.buf_fl = బఫరు;
    ret = process_op(&dop);
    ఒకవేళ (ret)
        తిరిగి ఖాళీ;

    // Read volume id, trim trailing spaces
    అక్షరం *volume = znprintf(30, "%s", బఫరు + 40);
    nullTrailingSpace(volume);
    తిరిగి volume;
}

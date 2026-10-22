// Support for booting from cdroms (the "El Torito" spec).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_GLOBAL
#अन्तर्भाव "block.h" // struct drive_s
#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "hw/ata.h" // ATA_CMD_REQUEST_SENSE
#अन्तर्भाव "hw/blockcmd.h" // CDB_CMD_REQUEST_SENSE
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "std/disk.h" // DISK_RET_SUCCESS
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // cdrom_prepboot
#अन्तर्भाव "tcgbios.h" // tpm_*


/****************************************************************
 * CD emulation
 ****************************************************************/

संरचना eltorito_s CDEmu VARLOW = { .size=परिमाण(CDEmu) };
संरचना drive_s *emulated_drive_gf VARLOW;
संरचना drive_s *cdemu_drive_gf VARFSEG;

स्थिर पूर्णाङ्क
cdemu_read(संरचना disk_op_s *op)
{
    संरचना drive_s *drive_gf = GET_खालचा(emulated_drive_gf);
    संरचना disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_खालचा(CDEmu.ilba) + op->lba / 4;

    पूर्णाङ्क मोजणी = op->मोजणी;
    op->मोजणी = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    यदि (op->lba & 3) {
        // Partial read of first block.
        dop.मोजणी = 1;
        dop.buf_fl = cdbuf_fl;
        पूर्णाङ्क ret = process_op(&dop);
        यदि (ret)
            प्रत्यागम ret;
        u8 thiscount = 4 - (op->lba & 3);
        यदि (thiscount > मोजणी)
            thiscount = मोजणी;
        मोजणी -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->मोजणी += thiscount;
        dop.lba++;
    }

    यदि (मोजणी > 3) {
        // Read n number of regular blocks.
        dop.मोजणी = मोजणी / 4;
        dop.buf_fl = op->buf_fl;
        पूर्णाङ्क ret = process_op(&dop);
        op->मोजणी += dop.मोजणी * 4;
        यदि (ret)
            प्रत्यागम ret;
        u8 thiscount = मोजणी & ~3;
        मोजणी &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    यदि (मोजणी) {
        // Partial read on last block.
        dop.मोजणी = 1;
        dop.buf_fl = cdbuf_fl;
        पूर्णाङ्क ret = process_op(&dop);
        यदि (ret)
            प्रत्यागम ret;
        u8 thiscount = मोजणी;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->मोजणी += thiscount;
    }

    प्रत्यागम DISK_RET_SUCCESS;
}

पूर्णाङ्क
cdemu_process_op(संरचना disk_op_s *op)
{
    यदि (!CONFIG_CDROM_EMU)
        प्रत्यागम 0;

    विकल्प (op->command) {
    पक्ष CMD_READ:
        प्रत्यागम cdemu_read(op);
    पक्ष CMD_WRITE:
    पक्ष CMD_FORMAT:
        प्रत्यागम DISK_RET_EWRITEPROTECT;
    स्वभाव:
        प्रत्यागम default_process_op(op);
    }
}

शून्य
cdrom_prepboot(शून्य)
{
    यदि (!CONFIG_CDROM_EMU)
        प्रत्यागम;
    यदि (!CDCount)
        प्रत्यागम;
    यदि (create_bounce_buf() < 0)
        प्रत्यागम;

    संरचना drive_s *drive = malloc_fseg(परिमाण(*drive));
    यदि (!drive) {
        warn_noalloc();
        प्रत्यागम;
    }
    cdemu_drive_gf = drive;
    memset(drive, 0, परिमाण(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_SIZE;
    drive->sectors = (u64)-1;
}


/****************************************************************
 * CD booting
 ****************************************************************/

पूर्णाङ्क
cdrom_boot(संरचना drive_s *drive)
{
    ASSERT32FLAT();
    संरचना disk_op_s dop;
    पूर्णाङ्क cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, परिमाण(dop));
    dop.drive_fl = drive;
    यदि (!dop.drive_fl || cdid < 0)
        प्रत्यागम 1;

    पूर्णाङ्क ret = scsi_is_ready(&dop);
    यदि (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.मोजणी = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    यदि (ret)
        प्रत्यागम 3;

    // Validity checks
    यदि (buffer[0])
        प्रत्यागम 4;
    यदि (strcmp((वर्ण*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        प्रत्यागम 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&buffer[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.मोजणी = 1;
    ret = process_op(&dop);
    यदि (ret)
        प्रत्यागम 7;

    // Validation entry
    यदि (buffer[0x00] != 0x01)
        प्रत्यागम 8;   // Header
    यदि (buffer[0x01] != 0x00)
        प्रत्यागम 9;   // Platform
    यदि (buffer[0x1E] != 0x55)
        प्रत्यागम 10;  // key 1
    यदि (buffer[0x1F] != 0xAA)
        प्रत्यागम 10;  // key 2

    // Initial/Default Entry
    यदि (buffer[0x20] != 0x88)
        प्रत्यागम 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_add_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), buffer), परिमाण(buffer));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = buffer[0x21];

    u16 boot_segment = *(u16*)&buffer[0x22];
    यदि (!boot_segment)
        boot_segment = 0x07C0;
    CDEmu.load_segment = boot_segment;
    CDEmu.buffer_segment = 0x0000;

    u16 nbsectors = *(u16*)&buffer[0x26];
    CDEmu.sector_मोजणी = nbsectors;

    lba = *(u32*)&buffer[0x28];
    CDEmu.ilba = lba;

    CDEmu.controller_index = drive->cntl_id / 2;
    CDEmu.device_spec = drive->cntl_id % 2;

    // And we read the image in memory
    nbsectors = DIV_ROUND_UP(nbsectors, 4);
    dop.lba = lba;
    dop.buf_fl = MAKE_FLATPTR(boot_segment, 0);
    यावत् (nbsectors) {
        पूर्णाङ्क मोजणी = nbsectors;
        यदि (मोजणी > 64*1024/CDROM_SECTOR_SIZE)
            मोजणी = 64*1024/CDROM_SECTOR_SIZE;
        dop.मोजणी = मोजणी;
        ret = process_op(&dop);
        यदि (ret)
            प्रत्यागम 12;
        nbsectors -= मोजणी;
        dop.lba += मोजणी;
        dop.buf_fl += मोजणी*CDROM_SECTOR_SIZE;
    }

    यदि (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        प्रत्यागम 0;
    }

    // Emulation of a floppy/harddisk requested
    यदि (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        प्रत्यागम 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    यदि (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        set_equipment_flags(0x41, 0x41);

        विकल्प (media) {
        पक्ष 0x01:  // 1.2M floppy
            CDEmu.chs.sptcyl = 15;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            भङ्ग;
        पक्ष 0x02:  // 1.44M floppy
            CDEmu.chs.sptcyl = 18;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            भङ्ग;
        पक्ष 0x03:  // 2.88M floppy
            CDEmu.chs.sptcyl = 36;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            भङ्ग;
        }
    } अन्यथा {
        // Harddrive emulation
        CDEmu.emulated_drive = 0x80;
        SET_BDA(hdcount, GET_BDA(hdcount) + 1);

        // Peak at partition table to get chs.
        संरचना mbr_s *mbr = MAKE_FLATPTR(boot_segment, 0);
        CDEmu.chs = mbr->partitions[0].last;
    }

    // everything is ok, so from now on, the emulation is active
    CDEmu.media = media;
    dprintf(6, "cdemu media=%d\n", media);

    प्रत्यागम 0;
}

// check if media is present and the drive is bootable.
// in case it is return the volume label.
वर्ण*
cdrom_media_info(संरचना drive_s *drive)
{
    ASSERT32FLAT();

    संरचना disk_op_s dop;
    memset(&dop, 0, परिमाण(dop));
    dop.drive_fl = drive;

    पूर्णाङ्क ret = scsi_is_ready(&dop);
    यदि (ret)
        प्रत्यागम रिक्त;

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.मोजणी = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    यदि (ret)
        प्रत्यागम रिक्त;

    // Is it bootable?
    यदि (buffer[0])
        प्रत्यागम रिक्त;
    यदि (strcmp((वर्ण*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        प्रत्यागम रिक्त;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.मोजणी = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    यदि (ret)
        प्रत्यागम रिक्त;

    // Read volume id, trim trailing spaces
    वर्ण *volume = znprintf(30, "%s", buffer + 40);
    nullTrailingSpace(volume);
    प्रत्यागम volume;
}

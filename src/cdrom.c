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

संरचना eltorito_s CDEmu VARLOW = { .size=आकार(CDEmu) };
संरचना drive_s *emulated_drive_gf VARLOW;
संरचना drive_s *cdemu_drive_gf VARFSEG;

स्थिर पूर्णांक
cdemu_read(संरचना disk_op_s *op)
{
    संरचना drive_s *drive_gf = GET_खालचा(emulated_drive_gf);
    संरचना disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_खालचा(CDEmu.ilba) + op->lba / 4;

    पूर्णांक मोजणी = op->मोजणी;
    op->मोजणी = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    जर (op->lba & 3) {
        // Partial read of first block.
        dop.मोजणी = 1;
        dop.buf_fl = cdbuf_fl;
        पूर्णांक ret = process_op(&dop);
        जर (ret)
            परत ret;
        u8 thiscount = 4 - (op->lba & 3);
        जर (thiscount > मोजणी)
            thiscount = मोजणी;
        मोजणी -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->मोजणी += thiscount;
        dop.lba++;
    }

    जर (मोजणी > 3) {
        // Read n number of regular blocks.
        dop.मोजणी = मोजणी / 4;
        dop.buf_fl = op->buf_fl;
        पूर्णांक ret = process_op(&dop);
        op->मोजणी += dop.मोजणी * 4;
        जर (ret)
            परत ret;
        u8 thiscount = मोजणी & ~3;
        मोजणी &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    जर (मोजणी) {
        // Partial read on last block.
        dop.मोजणी = 1;
        dop.buf_fl = cdbuf_fl;
        पूर्णांक ret = process_op(&dop);
        जर (ret)
            परत ret;
        u8 thiscount = मोजणी;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->मोजणी += thiscount;
    }

    परत DISK_RET_SUCCESS;
}

पूर्णांक
cdemu_process_op(संरचना disk_op_s *op)
{
    जर (!CONFIG_CDROM_EMU)
        परत 0;

    निवड (op->command) {
    स्थिती CMD_READ:
        परत cdemu_read(op);
    स्थिती CMD_WRITE:
    स्थिती CMD_FORMAT:
        परत DISK_RET_EWRITEPROTECT;
    पूर्वनिर्धारित:
        परत default_process_op(op);
    }
}

शून्य
cdrom_prepboot(शून्य)
{
    जर (!CONFIG_CDROM_EMU)
        परत;
    जर (!CDCount)
        परत;
    जर (create_bounce_buf() < 0)
        परत;

    संरचना drive_s *drive = malloc_fseg(आकार(*drive));
    जर (!drive) {
        warn_noalloc();
        परत;
    }
    cdemu_drive_gf = drive;
    स_रखो(drive, 0, आकार(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_SIZE;
    drive->sectors = (u64)-1;
}


/****************************************************************
 * CD booting
 ****************************************************************/

पूर्णांक
cdrom_boot(संरचना drive_s *drive)
{
    ASSERT32FLAT();
    संरचना disk_op_s dop;
    पूर्णांक cdid = getDriveId(EXTTYPE_CD, drive);
    स_रखो(&dop, 0, आकार(dop));
    dop.drive_fl = drive;
    जर (!dop.drive_fl || cdid < 0)
        परत 1;

    पूर्णांक ret = scsi_is_ready(&dop);
    जर (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.मोजणी = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    जर (ret)
        परत 3;

    // Validity checks
    जर (buffer[0])
        परत 4;
    जर (म_भेद((अक्षर*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        परत 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&buffer[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.मोजणी = 1;
    ret = process_op(&dop);
    जर (ret)
        परत 7;

    // Validation entry
    जर (buffer[0x00] != 0x01)
        परत 8;   // Header
    जर (buffer[0x01] != 0x00)
        परत 9;   // Platform
    जर (buffer[0x1E] != 0x55)
        परत 10;  // key 1
    जर (buffer[0x1F] != 0xAA)
        परत 10;  // key 2

    // Initial/Default Entry
    जर (buffer[0x20] != 0x88)
        परत 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_add_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), buffer), आकार(buffer));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = buffer[0x21];

    u16 boot_segment = *(u16*)&buffer[0x22];
    जर (!boot_segment)
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
    पर्यंत (nbsectors) {
        पूर्णांक मोजणी = nbsectors;
        जर (मोजणी > 64*1024/CDROM_SECTOR_SIZE)
            मोजणी = 64*1024/CDROM_SECTOR_SIZE;
        dop.मोजणी = मोजणी;
        ret = process_op(&dop);
        जर (ret)
            परत 12;
        nbsectors -= मोजणी;
        dop.lba += मोजणी;
        dop.buf_fl += मोजणी*CDROM_SECTOR_SIZE;
    }

    जर (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        परत 0;
    }

    // Emulation of a floppy/harddisk requested
    जर (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        परत 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    जर (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        set_equipment_flags(0x41, 0x41);

        निवड (media) {
        स्थिती 0x01:  // 1.2M floppy
            CDEmu.chs.sptcyl = 15;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            थांबा;
        स्थिती 0x02:  // 1.44M floppy
            CDEmu.chs.sptcyl = 18;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            थांबा;
        स्थिती 0x03:  // 2.88M floppy
            CDEmu.chs.sptcyl = 36;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            थांबा;
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

    परत 0;
}

// check if media is present and the drive is bootable.
// in case it is return the volume label.
अक्षर*
cdrom_media_info(संरचना drive_s *drive)
{
    ASSERT32FLAT();

    संरचना disk_op_s dop;
    स_रखो(&dop, 0, आकार(dop));
    dop.drive_fl = drive;

    पूर्णांक ret = scsi_is_ready(&dop);
    जर (ret)
        परत रिक्त;

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.मोजणी = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    जर (ret)
        परत रिक्त;

    // Is it bootable?
    जर (buffer[0])
        परत रिक्त;
    जर (म_भेद((अक्षर*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        परत रिक्त;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.मोजणी = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    जर (ret)
        परत रिक्त;

    // Read volume id, trim trailing spaces
    अक्षर *volume = znprintf(30, "%s", buffer + 40);
    nullTrailingSpace(volume);
    परत volume;
}

// Support for booting from cdroms (the "El Torito" spec).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_GLOBAL
#شامل "block.h" // struct drive_s
#شامل "bregs.h" // struct bregs
#شامل "hw/ata.h" // ATA_CMD_REQUEST_SENSE
#شامل "hw/blockcmd.h" // CDB_CMD_REQUEST_SENSE
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "std/disk.h" // DISK_RET_SUCCESS
#شامل "string.h" // memset
#شامل "util.h" // cdrom_prepboot
#شامل "tcgbios.h" // tpm_*


/****************************************************************
 * CD emulation
 ****************************************************************/

ساختار eltorito_s CDEmu VARLOW = { .size=اندازه(CDEmu) };
ساختار drive_s *emulated_drive_gf VARLOW;
ساختار drive_s *cdemu_drive_gf VARFSEG;

ایستا صحیح
cdemu_read(ساختار disk_op_s *op)
{
    ساختار drive_s *drive_gf = GET_کم(emulated_drive_gf);
    ساختار disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_کم(CDEmu.ilba) + op->lba / 4;

    صحیح شمار = op->شمار;
    op->شمار = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    اگر (op->lba & 3) {
        // Partial read of first block.
        dop.شمار = 1;
        dop.buf_fl = cdbuf_fl;
        صحیح ret = process_op(&dop);
        اگر (ret)
            بازگشت ret;
        u8 thiscount = 4 - (op->lba & 3);
        اگر (thiscount > شمار)
            thiscount = شمار;
        شمار -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->شمار += thiscount;
        dop.lba++;
    }

    اگر (شمار > 3) {
        // Read n number of regular blocks.
        dop.شمار = شمار / 4;
        dop.buf_fl = op->buf_fl;
        صحیح ret = process_op(&dop);
        op->شمار += dop.شمار * 4;
        اگر (ret)
            بازگشت ret;
        u8 thiscount = شمار & ~3;
        شمار &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    اگر (شمار) {
        // Partial read on last block.
        dop.شمار = 1;
        dop.buf_fl = cdbuf_fl;
        صحیح ret = process_op(&dop);
        اگر (ret)
            بازگشت ret;
        u8 thiscount = شمار;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->شمار += thiscount;
    }

    بازگشت DISK_RET_SUCCESS;
}

صحیح
cdemu_process_op(ساختار disk_op_s *op)
{
    اگر (!CONFIG_CDROM_EMU)
        بازگشت 0;

    انتخاب (op->command) {
    حالت CMD_READ:
        بازگشت cdemu_read(op);
    حالت CMD_WRITE:
    حالت CMD_FORMAT:
        بازگشت DISK_RET_EWRITEPROTECT;
    پیشفرض:
        بازگشت default_process_op(op);
    }
}

تهی
cdrom_prepboot(تهی)
{
    اگر (!CONFIG_CDROM_EMU)
        بازگشت;
    اگر (!CDCount)
        بازگشت;
    اگر (create_bounce_buf() < 0)
        بازگشت;

    ساختار drive_s *drive = malloc_fseg(اندازه(*drive));
    اگر (!drive) {
        warn_noalloc();
        بازگشت;
    }
    cdemu_drive_gf = drive;
    memset(drive, 0, اندازه(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_SIZE;
    drive->sectors = (u64)-1;
}


/****************************************************************
 * CD booting
 ****************************************************************/

صحیح
cdrom_boot(ساختار drive_s *drive)
{
    ASSERT32FLAT();
    ساختار disk_op_s dop;
    صحیح cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, اندازه(dop));
    dop.drive_fl = drive;
    اگر (!dop.drive_fl || cdid < 0)
        بازگشت 1;

    صحیح ret = scsi_is_ready(&dop);
    اگر (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.شمار = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    اگر (ret)
        بازگشت 3;

    // Validity checks
    اگر (buffer[0])
        بازگشت 4;
    اگر (strcmp((نویسه*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        بازگشت 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&buffer[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.شمار = 1;
    ret = process_op(&dop);
    اگر (ret)
        بازگشت 7;

    // Validation entry
    اگر (buffer[0x00] != 0x01)
        بازگشت 8;   // Header
    اگر (buffer[0x01] != 0x00)
        بازگشت 9;   // Platform
    اگر (buffer[0x1E] != 0x55)
        بازگشت 10;  // key 1
    اگر (buffer[0x1F] != 0xAA)
        بازگشت 10;  // key 2

    // Initial/Default Entry
    اگر (buffer[0x20] != 0x88)
        بازگشت 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_add_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), buffer), اندازه(buffer));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = buffer[0x21];

    u16 boot_segment = *(u16*)&buffer[0x22];
    اگر (!boot_segment)
        boot_segment = 0x07C0;
    CDEmu.load_segment = boot_segment;
    CDEmu.buffer_segment = 0x0000;

    u16 nbsectors = *(u16*)&buffer[0x26];
    CDEmu.sector_شمار = nbsectors;

    lba = *(u32*)&buffer[0x28];
    CDEmu.ilba = lba;

    CDEmu.controller_index = drive->cntl_id / 2;
    CDEmu.device_spec = drive->cntl_id % 2;

    // And we read the image in memory
    nbsectors = DIV_ROUND_UP(nbsectors, 4);
    dop.lba = lba;
    dop.buf_fl = MAKE_FLATPTR(boot_segment, 0);
    وقتی (nbsectors) {
        صحیح شمار = nbsectors;
        اگر (شمار > 64*1024/CDROM_SECTOR_SIZE)
            شمار = 64*1024/CDROM_SECTOR_SIZE;
        dop.شمار = شمار;
        ret = process_op(&dop);
        اگر (ret)
            بازگشت 12;
        nbsectors -= شمار;
        dop.lba += شمار;
        dop.buf_fl += شمار*CDROM_SECTOR_SIZE;
    }

    اگر (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        بازگشت 0;
    }

    // Emulation of a floppy/harddisk requested
    اگر (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        بازگشت 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    اگر (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        set_equipment_flags(0x41, 0x41);

        انتخاب (media) {
        حالت 0x01:  // 1.2M floppy
            CDEmu.chs.sptcyl = 15;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            بشکن;
        حالت 0x02:  // 1.44M floppy
            CDEmu.chs.sptcyl = 18;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            بشکن;
        حالت 0x03:  // 2.88M floppy
            CDEmu.chs.sptcyl = 36;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            بشکن;
        }
    } وگرنه {
        // Harddrive emulation
        CDEmu.emulated_drive = 0x80;
        SET_BDA(hdcount, GET_BDA(hdcount) + 1);

        // Peak at partition table to get chs.
        ساختار mbr_s *mbr = MAKE_FLATPTR(boot_segment, 0);
        CDEmu.chs = mbr->partitions[0].last;
    }

    // everything is ok, so from now on, the emulation is active
    CDEmu.media = media;
    dprintf(6, "cdemu media=%d\n", media);

    بازگشت 0;
}

// check if media is present and the drive is bootable.
// in case it is return the volume label.
نویسه*
cdrom_media_info(ساختار drive_s *drive)
{
    ASSERT32FLAT();

    ساختار disk_op_s dop;
    memset(&dop, 0, اندازه(dop));
    dop.drive_fl = drive;

    صحیح ret = scsi_is_ready(&dop);
    اگر (ret)
        بازگشت پوچ;

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.شمار = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    اگر (ret)
        بازگشت پوچ;

    // Is it bootable?
    اگر (buffer[0])
        بازگشت پوچ;
    اگر (strcmp((نویسه*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        بازگشت پوچ;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.شمار = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    اگر (ret)
        بازگشت پوچ;

    // Read volume id, trim trailing spaces
    نویسه *volume = znprintf(30, "%s", buffer + 40);
    nullTrailingSpace(volume);
    بازگشت volume;
}

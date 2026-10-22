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

ساخت eltorito_s CDEmu VARLOW = { .سائز=حجم(CDEmu) };
ساخت drive_s *emulated_drive_gf VARLOW;
ساخت drive_s *cdemu_drive_gf VARFSEG;

ساکن صحیح
cdemu_read(ساخت disk_op_s *op)
{
    ساخت drive_s *drive_gf = GET_کم(emulated_drive_gf);
    ساخت disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_کم(CDEmu.ilba) + op->lba / 4;

    صحیح گنتی = op->گنتی;
    op->گنتی = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    اگر (op->lba & 3) {
        // Partial read of first block.
        dop.گنتی = 1;
        dop.buf_fl = cdbuf_fl;
        صحیح ret = process_op(&dop);
        اگر (ret)
            واپس ret;
        u8 thiscount = 4 - (op->lba & 3);
        اگر (thiscount > گنتی)
            thiscount = گنتی;
        گنتی -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->گنتی += thiscount;
        dop.lba++;
    }

    اگر (گنتی > 3) {
        // Read n number of regular blocks.
        dop.گنتی = گنتی / 4;
        dop.buf_fl = op->buf_fl;
        صحیح ret = process_op(&dop);
        op->گنتی += dop.گنتی * 4;
        اگر (ret)
            واپس ret;
        u8 thiscount = گنتی & ~3;
        گنتی &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    اگر (گنتی) {
        // Partial read on last block.
        dop.گنتی = 1;
        dop.buf_fl = cdbuf_fl;
        صحیح ret = process_op(&dop);
        اگر (ret)
            واپس ret;
        u8 thiscount = گنتی;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->گنتی += thiscount;
    }

    واپس DISK_RET_SUCCESS;
}

صحیح
cdemu_process_op(ساخت disk_op_s *op)
{
    اگر (!CONFIG_CDROM_EMU)
        واپس 0;

    انتخاب (op->command) {
    حالت CMD_READ:
        واپس cdemu_read(op);
    حالت CMD_WRITE:
    حالت CMD_FORMAT:
        واپس DISK_RET_EWRITEPROTECT;
    طےشدہ:
        واپس default_process_op(op);
    }
}

خالی
cdrom_prepboot(خالی)
{
    اگر (!CONFIG_CDROM_EMU)
        واپس;
    اگر (!CDCount)
        واپس;
    اگر (create_bounce_buf() < 0)
        واپس;

    ساخت drive_s *drive = malloc_fseg(حجم(*drive));
    اگر (!drive) {
        warn_noalloc();
        واپس;
    }
    cdemu_drive_gf = drive;
    memset(drive, 0, حجم(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_سائز;
    drive->sectors = (u64)-1;
}


/****************************************************************
 * CD booting
 ****************************************************************/

صحیح
cdrom_boot(ساخت drive_s *drive)
{
    ASSERT32FLAT();
    ساخت disk_op_s dop;
    صحیح cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, حجم(dop));
    dop.drive_fl = drive;
    اگر (!dop.drive_fl || cdid < 0)
        واپس 1;

    صحیح ret = scsi_is_ready(&dop);
    اگر (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 بفر[CDROM_SECTOR_سائز];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.گنتی = 1;
    dop.buf_fl = بفر;
    ret = process_op(&dop);
    اگر (ret)
        واپس 3;

    // Validity checks
    اگر (بفر[0])
        واپس 4;
    اگر (strcmp((حرف*)&بفر[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        واپس 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&بفر[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.گنتی = 1;
    ret = process_op(&dop);
    اگر (ret)
        واپس 7;

    // Validation entry
    اگر (بفر[0x00] != 0x01)
        واپس 8;   //سرخی
    اگر (بفر[0x01] != 0x00)
        واپس 9;   // Platform
    اگر (بفر[0x1E] != 0x55)
        واپس 10;  // key 1
    اگر (بفر[0x1F] != 0xAA)
        واپس 10;  // key 2

    // Initial/Default Entry
    اگر (بفر[0x20] != 0x88)
        واپس 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_جوڑو_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), بفر), حجم(بفر));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = بفر[0x21];

    u16 boot_segment = *(u16*)&بفر[0x22];
    اگر (!boot_segment)
        boot_segment = 0x07C0;
    CDEmu.بھرو_segment = boot_segment;
    CDEmu.بفر_segment = 0x0000;

    u16 nbsectors = *(u16*)&بفر[0x26];
    CDEmu.sector_گنتی = nbsectors;

    lba = *(u32*)&بفر[0x28];
    CDEmu.ilba = lba;

    CDEmu.controller_اشاریہ = drive->cntl_id / 2;
    CDEmu.device_spec = drive->cntl_id % 2;

    // And we read the image in memory
    nbsectors = DIV_باری_اوپر(nbsectors, 4);
    dop.lba = lba;
    dop.buf_fl = MAKE_FLATPTR(boot_segment, 0);
    جب (nbsectors) {
        صحیح گنتی = nbsectors;
        اگر (گنتی > 64*1024/CDROM_SECTOR_سائز)
            گنتی = 64*1024/CDROM_SECTOR_سائز;
        dop.گنتی = گنتی;
        ret = process_op(&dop);
        اگر (ret)
            واپس 12;
        nbsectors -= گنتی;
        dop.lba += گنتی;
        dop.buf_fl += گنتی*CDROM_SECTOR_سائز;
    }

    اگر (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        واپس 0;
    }

    // Emulation of a floppy/harddisk requested
    اگر (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        واپس 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    اگر (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        سیٹ_equipment_flags(0x41, 0x41);

        انتخاب (media) {
        حالت 0x01:  // 1.2M floppy
            CDEmu.chs.sptcyl = 15;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            توڑو;
        حالت 0x02:  // 1.44M floppy
            CDEmu.chs.sptcyl = 18;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            توڑو;
        حالت 0x03:  // 2.88M floppy
            CDEmu.chs.sptcyl = 36;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            توڑو;
        }
    } ورنہ {
        // Harddrive emulation
        CDEmu.emulated_drive = 0x80;
        سیٹ_BDA(hdcount, GET_BDA(hdcount) + 1);

        // Peak at partition table to get chs.
        ساخت mbr_s *mbr = MAKE_FLATPTR(boot_segment, 0);
        CDEmu.chs = mbr->partitions[0].آخری;
    }

    // everything is ok, so from now on, the emulation is active
    CDEmu.media = media;
    dprintf(6, "cdemu media=%d\n", media);

    واپس 0;
}

// check if media is present and the drive is bootable.
// in case it is return the volume label.
حرف*
cdrom_media_info(ساخت drive_s *drive)
{
    ASSERT32FLAT();

    ساخت disk_op_s dop;
    memset(&dop, 0, حجم(dop));
    dop.drive_fl = drive;

    صحیح ret = scsi_is_ready(&dop);
    اگر (ret)
        واپس تہی;

    // Read the Boot Record Volume Descriptor
    u8 بفر[CDROM_SECTOR_سائز];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.گنتی = 1;
    dop.buf_fl = بفر;
    ret = process_op(&dop);
    اگر (ret)
        واپس تہی;

    // Is it bootable?
    اگر (بفر[0])
        واپس تہی;
    اگر (strcmp((حرف*)&بفر[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        واپس تہی;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.گنتی = 1;
    dop.buf_fl = بفر;
    ret = process_op(&dop);
    اگر (ret)
        واپس تہی;

    // Read volume id, trim trailing spaces
    حرف *volume = znprintf(30, "%s", بفر + 40);
    nullTrailingSpace(volume);
    واپس volume;
}

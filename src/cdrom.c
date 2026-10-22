// Support for booting from cdroms (the "El Torito" spec).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_GLOBAL
#ضمن "block.h" // struct drive_s
#ضمن "bregs.h" // struct bregs
#ضمن "hw/ata.h" // ATA_CMD_REQUEST_SENSE
#ضمن "hw/blockcmd.h" // CDB_CMD_REQUEST_SENSE
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "std/disk.h" // DISK_RET_SUCCESS
#ضمن "string.h" // memset
#ضمن "util.h" // cdrom_prepboot
#ضمن "tcgbios.h" // tpm_*


/****************************************************************
 * CD emulation
 ****************************************************************/

بنية eltorito_s CDEmu VARLOW = { .ناپ=حجم(CDEmu) };
بنية drive_s *emulated_drive_gf VARLOW;
بنية drive_s *cdemu_drive_gf VARFSEG;

ساكن صحيح
cdemu_read(بنية disk_op_s *op)
{
    بنية drive_s *drive_gf = GET_کم(emulated_drive_gf);
    بنية disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_کم(CDEmu.ilba) + op->lba / 4;

    صحيح گنتی = op->گنتی;
    op->گنتی = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    إذا (op->lba & 3) {
        // Partial read of first block.
        dop.گنتی = 1;
        dop.buf_fl = cdbuf_fl;
        صحيح ret = process_op(&dop);
        إذا (ret)
            أرجع ret;
        u8 thiscount = 4 - (op->lba & 3);
        إذا (thiscount > گنتی)
            thiscount = گنتی;
        گنتی -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->گنتی += thiscount;
        dop.lba++;
    }

    إذا (گنتی > 3) {
        // Read n number of regular blocks.
        dop.گنتی = گنتی / 4;
        dop.buf_fl = op->buf_fl;
        صحيح ret = process_op(&dop);
        op->گنتی += dop.گنتی * 4;
        إذا (ret)
            أرجع ret;
        u8 thiscount = گنتی & ~3;
        گنتی &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    إذا (گنتی) {
        // Partial read on last block.
        dop.گنتی = 1;
        dop.buf_fl = cdbuf_fl;
        صحيح ret = process_op(&dop);
        إذا (ret)
            أرجع ret;
        u8 thiscount = گنتی;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->گنتی += thiscount;
    }

    أرجع DISK_RET_SUCCESS;
}

صحيح
cdemu_process_op(بنية disk_op_s *op)
{
    إذا (!CONFIG_CDROM_EMU)
        أرجع 0;

    بدل (op->command) {
    حالة CMD_READ:
        أرجع cdemu_read(op);
    حالة CMD_WRITE:
    حالة CMD_FORMAT:
        أرجع DISK_RET_EWRITEPROTECT;
    افتراضي:
        أرجع default_process_op(op);
    }
}

فراغ
cdrom_prepboot(فراغ)
{
    إذا (!CONFIG_CDROM_EMU)
        أرجع;
    إذا (!CDCount)
        أرجع;
    إذا (create_bounce_buf() < 0)
        أرجع;

    بنية drive_s *drive = malloc_fseg(حجم(*drive));
    إذا (!drive) {
        warn_noalloc();
        أرجع;
    }
    cdemu_drive_gf = drive;
    memset(drive, 0, حجم(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_ناپ;
    drive->sectors = (u64)-1;
}


/****************************************************************
 * CD booting
 ****************************************************************/

صحيح
cdrom_boot(بنية drive_s *drive)
{
    ASSERT32FLAT();
    بنية disk_op_s dop;
    صحيح cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, حجم(dop));
    dop.drive_fl = drive;
    إذا (!dop.drive_fl || cdid < 0)
        أرجع 1;

    صحيح ret = scsi_is_ready(&dop);
    إذا (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 بفر[CDROM_SECTOR_ناپ];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.گنتی = 1;
    dop.buf_fl = بفر;
    ret = process_op(&dop);
    إذا (ret)
        أرجع 3;

    // Validity checks
    إذا (بفر[0])
        أرجع 4;
    إذا (strcmp((حرف*)&بفر[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        أرجع 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&بفر[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.گنتی = 1;
    ret = process_op(&dop);
    إذا (ret)
        أرجع 7;

    // Validation entry
    إذا (بفر[0x00] != 0x01)
        أرجع 8;   //سرخی
    إذا (بفر[0x01] != 0x00)
        أرجع 9;   // Platform
    إذا (بفر[0x1E] != 0x55)
        أرجع 10;  // key 1
    إذا (بفر[0x1F] != 0xAA)
        أرجع 10;  // key 2

    // Initial/Default Entry
    إذا (بفر[0x20] != 0x88)
        أرجع 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_جوڑو_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), بفر), حجم(بفر));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = بفر[0x21];

    u16 boot_segment = *(u16*)&بفر[0x22];
    إذا (!boot_segment)
        boot_segment = 0x07C0;
    CDEmu.لدو_segment = boot_segment;
    CDEmu.بفر_segment = 0x0000;

    u16 nbsectors = *(u16*)&بفر[0x26];
    CDEmu.sector_گنتی = nbsectors;

    lba = *(u32*)&بفر[0x28];
    CDEmu.ilba = lba;

    CDEmu.controller_اشاریہ = drive->cntl_id / 2;
    CDEmu.device_spec = drive->cntl_id % 2;

    // And we read the image in memory
    nbsectors = DIV_گیڑا_اتے(nbsectors, 4);
    dop.lba = lba;
    dop.buf_fl = MAKE_FLATPTR(boot_segment, 0);
    طالما (nbsectors) {
        صحيح گنتی = nbsectors;
        إذا (گنتی > 64*1024/CDROM_SECTOR_ناپ)
            گنتی = 64*1024/CDROM_SECTOR_ناپ;
        dop.گنتی = گنتی;
        ret = process_op(&dop);
        إذا (ret)
            أرجع 12;
        nbsectors -= گنتی;
        dop.lba += گنتی;
        dop.buf_fl += گنتی*CDROM_SECTOR_ناپ;
    }

    إذا (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        أرجع 0;
    }

    // Emulation of a floppy/harddisk requested
    إذا (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        أرجع 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    إذا (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        سیٹ_equipment_flags(0x41, 0x41);

        بدل (media) {
        حالة 0x01:  // 1.2M floppy
            CDEmu.chs.sptcyl = 15;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            اكسر;
        حالة 0x02:  // 1.44M floppy
            CDEmu.chs.sptcyl = 18;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            اكسر;
        حالة 0x03:  // 2.88M floppy
            CDEmu.chs.sptcyl = 36;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            اكسر;
        }
    } وإلا {
        // Harddrive emulation
        CDEmu.emulated_drive = 0x80;
        سیٹ_BDA(hdcount, GET_BDA(hdcount) + 1);

        // Peak at partition table to get chs.
        بنية mbr_s *mbr = MAKE_FLATPTR(boot_segment, 0);
        CDEmu.chs = mbr->partitions[0].آخری;
    }

    // everything is ok, so from now on, the emulation is active
    CDEmu.media = media;
    dprintf(6, "cdemu media=%d\n", media);

    أرجع 0;
}

// check if media is present and the drive is bootable.
// in case it is return the volume label.
حرف*
cdrom_media_info(بنية drive_s *drive)
{
    ASSERT32FLAT();

    بنية disk_op_s dop;
    memset(&dop, 0, حجم(dop));
    dop.drive_fl = drive;

    صحيح ret = scsi_is_ready(&dop);
    إذا (ret)
        أرجع عدم;

    // Read the Boot Record Volume Descriptor
    u8 بفر[CDROM_SECTOR_ناپ];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.گنتی = 1;
    dop.buf_fl = بفر;
    ret = process_op(&dop);
    إذا (ret)
        أرجع عدم;

    // Is it bootable?
    إذا (بفر[0])
        أرجع عدم;
    إذا (strcmp((حرف*)&بفر[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        أرجع عدم;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.گنتی = 1;
    dop.buf_fl = بفر;
    ret = process_op(&dop);
    إذا (ret)
        أرجع عدم;

    // Read volume id, trim trailing spaces
    حرف *volume = znprintf(30, "%s", بفر + 40);
    nullTrailingSpace(volume);
    أرجع volume;
}

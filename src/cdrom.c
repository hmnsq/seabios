// Support for booting from cdroms (the "El Torito" spec).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // GET_GLOBAL
#অন্তর্ভাব "block.h" // struct drive_s
#অন্তর্ভাব "bregs.h" // struct bregs
#অন্তর্ভাব "hw/ata.h" // ATA_CMD_REQUEST_SENSE
#অন্তর্ভাব "hw/blockcmd.h" // CDB_CMD_REQUEST_SENSE
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "std/disk.h" // DISK_RET_SUCCESS
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // cdrom_prepboot
#অন্তর্ভাব "tcgbios.h" // tpm_*


/****************************************************************
 * CD emulation
 ****************************************************************/

কাঠামো eltorito_s CDEmu VARLOW = { .আকার_2=আকার(CDEmu) };
কাঠামো drive_s *emulated_drive_gf VARLOW;
কাঠামো drive_s *cdemu_drive_gf VARFSEG;

স্থির পূর্ণসংখ্যা
cdemu_read(কাঠামো disk_op_s *op)
{
    কাঠামো drive_s *drive_gf = GET_নিচের(emulated_drive_gf);
    কাঠামো disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_নিচের(CDEmu.ilba) + op->lba / 4;

    পূর্ণসংখ্যা গুনতি = op->গুনতি;
    op->গুনতি = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    যদি (op->lba & 3) {
        // Partial read of first block.
        dop.গুনতি = 1;
        dop.buf_fl = cdbuf_fl;
        পূর্ণসংখ্যা ret = process_op(&dop);
        যদি (ret)
            ফেরত ret;
        u8 thiscount = 4 - (op->lba & 3);
        যদি (thiscount > গুনতি)
            thiscount = গুনতি;
        গুনতি -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->গুনতি += thiscount;
        dop.lba++;
    }

    যদি (গুনতি > 3) {
        // Read n number of regular blocks.
        dop.গুনতি = গুনতি / 4;
        dop.buf_fl = op->buf_fl;
        পূর্ণসংখ্যা ret = process_op(&dop);
        op->গুনতি += dop.গুনতি * 4;
        যদি (ret)
            ফেরত ret;
        u8 thiscount = গুনতি & ~3;
        গুনতি &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    যদি (গুনতি) {
        // Partial read on last block.
        dop.গুনতি = 1;
        dop.buf_fl = cdbuf_fl;
        পূর্ণসংখ্যা ret = process_op(&dop);
        যদি (ret)
            ফেরত ret;
        u8 thiscount = গুনতি;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->গুনতি += thiscount;
    }

    ফেরত DISK_RET_SUCCESS;
}

পূর্ণসংখ্যা
cdemu_process_op(কাঠামো disk_op_s *op)
{
    যদি (!CONFIG_CDROM_EMU)
        ফেরত 0;

    নির্বাচন (op->command) {
    ক্ষেত্র CMD_READ:
        ফেরত cdemu_read(op);
    ক্ষেত্র CMD_WRITE:
    ক্ষেত্র CMD_FORMAT:
        ফেরত DISK_RET_EWRITEPROTECT;
    পূর্বনির্ধারিত:
        ফেরত default_process_op(op);
    }
}

শূন্য
cdrom_prepboot(শূন্য)
{
    যদি (!CONFIG_CDROM_EMU)
        ফেরত;
    যদি (!CDCount)
        ফেরত;
    যদি (create_bounce_buf() < 0)
        ফেরত;

    কাঠামো drive_s *drive = malloc_fseg(আকার(*drive));
    যদি (!drive) {
        warn_noalloc();
        ফেরত;
    }
    cdemu_drive_gf = drive;
    memset(drive, 0, আকার(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_আকার;
    drive->sectors = (u64)-1;
}


/****************************************************************
 * CD booting
 ****************************************************************/

পূর্ণসংখ্যা
cdrom_boot(কাঠামো drive_s *drive)
{
    ASSERT32FLAT();
    কাঠামো disk_op_s dop;
    পূর্ণসংখ্যা cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, আকার(dop));
    dop.drive_fl = drive;
    যদি (!dop.drive_fl || cdid < 0)
        ফেরত 1;

    পূর্ণসংখ্যা ret = scsi_is_ready(&dop);
    যদি (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 বাফার[CDROM_SECTOR_আকার];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.গুনতি = 1;
    dop.buf_fl = বাফার;
    ret = process_op(&dop);
    যদি (ret)
        ফেরত 3;

    // Validity checks
    যদি (বাফার[0])
        ফেরত 4;
    যদি (strcmp((অক্ষর*)&বাফার[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ফেরত 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&বাফার[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.গুনতি = 1;
    ret = process_op(&dop);
    যদি (ret)
        ফেরত 7;

    // Validation entry
    যদি (বাফার[0x00] != 0x01)
        ফেরত 8;   // Header
    যদি (বাফার[0x01] != 0x00)
        ফেরত 9;   // Platform
    যদি (বাফার[0x1E] != 0x55)
        ফেরত 10;  // key 1
    যদি (বাফার[0x1F] != 0xAA)
        ফেরত 10;  // key 2

    // Initial/Default Entry
    যদি (বাফার[0x20] != 0x88)
        ফেরত 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_add_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), বাফার), আকার(বাফার));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = বাফার[0x21];

    u16 boot_segment = *(u16*)&বাফার[0x22];
    যদি (!boot_segment)
        boot_segment = 0x07C0;
    CDEmu.load_segment = boot_segment;
    CDEmu.বাফার_segment = 0x0000;

    u16 nbsectors = *(u16*)&বাফার[0x26];
    CDEmu.sector_গুনতি = nbsectors;

    lba = *(u32*)&বাফার[0x28];
    CDEmu.ilba = lba;

    CDEmu.controller_সূচক = drive->cntl_id / 2;
    CDEmu.device_spec = drive->cntl_id % 2;

    // And we read the image in memory
    nbsectors = DIV_ROUND_UP(nbsectors, 4);
    dop.lba = lba;
    dop.buf_fl = MAKE_FLATPTR(boot_segment, 0);
    যখন (nbsectors) {
        পূর্ণসংখ্যা গুনতি = nbsectors;
        যদি (গুনতি > 64*1024/CDROM_SECTOR_আকার)
            গুনতি = 64*1024/CDROM_SECTOR_আকার;
        dop.গুনতি = গুনতি;
        ret = process_op(&dop);
        যদি (ret)
            ফেরত 12;
        nbsectors -= গুনতি;
        dop.lba += গুনতি;
        dop.buf_fl += গুনতি*CDROM_SECTOR_আকার;
    }

    যদি (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        ফেরত 0;
    }

    // Emulation of a floppy/harddisk requested
    যদি (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        ফেরত 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    যদি (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        set_equipment_flags(0x41, 0x41);

        নির্বাচন (media) {
        ক্ষেত্র 0x01:  // 1.2M floppy
            CDEmu.chs.sptcyl = 15;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            বিরতি;
        ক্ষেত্র 0x02:  // 1.44M floppy
            CDEmu.chs.sptcyl = 18;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            বিরতি;
        ক্ষেত্র 0x03:  // 2.88M floppy
            CDEmu.chs.sptcyl = 36;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            বিরতি;
        }
    } অন্যথা {
        // Harddrive emulation
        CDEmu.emulated_drive = 0x80;
        SET_BDA(hdcount, GET_BDA(hdcount) + 1);

        // Peak at partition table to get chs.
        কাঠামো mbr_s *mbr = MAKE_FLATPTR(boot_segment, 0);
        CDEmu.chs = mbr->partitions[0].last;
    }

    // everything is ok, so from now on, the emulation is active
    CDEmu.media = media;
    dprintf(6, "cdemu media=%d\n", media);

    ফেরত 0;
}

// check if media is present and the drive is bootable.
// in case it is return the volume label.
অক্ষর*
cdrom_media_info(কাঠামো drive_s *drive)
{
    ASSERT32FLAT();

    কাঠামো disk_op_s dop;
    memset(&dop, 0, আকার(dop));
    dop.drive_fl = drive;

    পূর্ণসংখ্যা ret = scsi_is_ready(&dop);
    যদি (ret)
        ফেরত ফাঁকা;

    // Read the Boot Record Volume Descriptor
    u8 বাফার[CDROM_SECTOR_আকার];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.গুনতি = 1;
    dop.buf_fl = বাফার;
    ret = process_op(&dop);
    যদি (ret)
        ফেরত ফাঁকা;

    // Is it bootable?
    যদি (বাফার[0])
        ফেরত ফাঁকা;
    যদি (strcmp((অক্ষর*)&বাফার[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ফেরত ফাঁকা;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.গুনতি = 1;
    dop.buf_fl = বাফার;
    ret = process_op(&dop);
    যদি (ret)
        ফেরত ফাঁকা;

    // Read volume id, trim trailing spaces
    অক্ষর *volume = znprintf(30, "%s", বাফার + 40);
    nullTrailingSpace(volume);
    ফেরত volume;
}

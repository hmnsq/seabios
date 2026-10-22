// Support for booting from cdroms (the "El Torito" spec).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "biosvar.h" // GET_GLOBAL
#অন্তর্ভাৱ "block.h" // struct drive_s
#অন্তর্ভাৱ "bregs.h" // struct bregs
#অন্তর্ভাৱ "hw/ata.h" // ATA_CMD_REQUEST_SENSE
#অন্তর্ভাৱ "hw/blockcmd.h" // CDB_CMD_REQUEST_SENSE
#অন্তর্ভাৱ "malloc.h" // free
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "std/disk.h" // DISK_RET_SUCCESS
#অন্তর্ভাৱ "string.h" // memset
#অন্তর্ভাৱ "util.h" // cdrom_prepboot
#অন্তর্ভাৱ "tcgbios.h" // tpm_*


/****************************************************************
 * CD emulation
 ****************************************************************/

সংরচনা eltorito_s CDEmu VARLOW = { .size=পরিমাণ(CDEmu) };
সংরচনা drive_s *emulated_drive_gf VARLOW;
সংরচনা drive_s *cdemu_drive_gf VARFSEG;

স্থির পূর্ণাঙ্ক
cdemu_read(সংরচনা disk_op_s *op)
{
    সংরচনা drive_s *drive_gf = GET_নিচেৰ(emulated_drive_gf);
    সংরচনা disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_নিচেৰ(CDEmu.ilba) + op->lba / 4;

    পূর্ণাঙ্ক গুনতি = op->গুনতি;
    op->গুনতি = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    যদি (op->lba & 3) {
        // Partial read of first block.
        dop.গুনতি = 1;
        dop.buf_fl = cdbuf_fl;
        পূর্ণাঙ্ক ret = process_op(&dop);
        যদি (ret)
            ঘূৰাই ret;
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
        পূর্ণাঙ্ক ret = process_op(&dop);
        op->গুনতি += dop.গুনতি * 4;
        যদি (ret)
            ঘূৰাই ret;
        u8 thiscount = গুনতি & ~3;
        গুনতি &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    যদি (গুনতি) {
        // Partial read on last block.
        dop.গুনতি = 1;
        dop.buf_fl = cdbuf_fl;
        পূর্ণাঙ্ক ret = process_op(&dop);
        যদি (ret)
            ঘূৰাই ret;
        u8 thiscount = গুনতি;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->গুনতি += thiscount;
    }

    ঘূৰাই DISK_RET_SUCCESS;
}

পূর্ণাঙ্ক
cdemu_process_op(সংরচনা disk_op_s *op)
{
    যদি (!CONFIG_CDROM_EMU)
        ঘূৰাই 0;

    ৱিকল্প (op->command) {
    পক্ষ CMD_READ:
        ঘূৰাই cdemu_read(op);
    পক্ষ CMD_WRITE:
    পক্ষ CMD_FORMAT:
        ঘূৰাই DISK_RET_EWRITEPROTECT;
    স্ৱভাৱ:
        ঘূৰাই default_process_op(op);
    }
}

শূন্য
cdrom_prepboot(শূন্য)
{
    যদি (!CONFIG_CDROM_EMU)
        ঘূৰাই;
    যদি (!CDCount)
        ঘূৰাই;
    যদি (create_bounce_buf() < 0)
        ঘূৰাই;

    সংরচনা drive_s *drive = malloc_fseg(পরিমাণ(*drive));
    যদি (!drive) {
        warn_noalloc();
        ঘূৰাই;
    }
    cdemu_drive_gf = drive;
    memset(drive, 0, পরিমাণ(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_SIZE;
    drive->sectors = (u64)-1;
}


/****************************************************************
 * CD booting
 ****************************************************************/

পূর্ণাঙ্ক
cdrom_boot(সংরচনা drive_s *drive)
{
    ASSERT32FLAT();
    সংরচনা disk_op_s dop;
    পূর্ণাঙ্ক cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, পরিমাণ(dop));
    dop.drive_fl = drive;
    যদি (!dop.drive_fl || cdid < 0)
        ঘূৰাই 1;

    পূর্ণাঙ্ক ret = scsi_is_ready(&dop);
    যদি (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.গুনতি = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    যদি (ret)
        ঘূৰাই 3;

    // Validity checks
    যদি (buffer[0])
        ঘূৰাই 4;
    যদি (strcmp((ৱর্ণ*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ঘূৰাই 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&buffer[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.গুনতি = 1;
    ret = process_op(&dop);
    যদি (ret)
        ঘূৰাই 7;

    // Validation entry
    যদি (buffer[0x00] != 0x01)
        ঘূৰাই 8;   // Header
    যদি (buffer[0x01] != 0x00)
        ঘূৰাই 9;   // Platform
    যদি (buffer[0x1E] != 0x55)
        ঘূৰাই 10;  // key 1
    যদি (buffer[0x1F] != 0xAA)
        ঘূৰাই 10;  // key 2

    // Initial/Default Entry
    যদি (buffer[0x20] != 0x88)
        ঘূৰাই 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_add_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), buffer), পরিমাণ(buffer));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = buffer[0x21];

    u16 boot_segment = *(u16*)&buffer[0x22];
    যদি (!boot_segment)
        boot_segment = 0x07C0;
    CDEmu.load_segment = boot_segment;
    CDEmu.buffer_segment = 0x0000;

    u16 nbsectors = *(u16*)&buffer[0x26];
    CDEmu.sector_গুনতি = nbsectors;

    lba = *(u32*)&buffer[0x28];
    CDEmu.ilba = lba;

    CDEmu.controller_index = drive->cntl_id / 2;
    CDEmu.device_spec = drive->cntl_id % 2;

    // And we read the image in memory
    nbsectors = DIV_ROUND_UP(nbsectors, 4);
    dop.lba = lba;
    dop.buf_fl = MAKE_FLATPTR(boot_segment, 0);
    যেতিয়ালৈ (nbsectors) {
        পূর্ণাঙ্ক গুনতি = nbsectors;
        যদি (গুনতি > 64*1024/CDROM_SECTOR_SIZE)
            গুনতি = 64*1024/CDROM_SECTOR_SIZE;
        dop.গুনতি = গুনতি;
        ret = process_op(&dop);
        যদি (ret)
            ঘূৰাই 12;
        nbsectors -= গুনতি;
        dop.lba += গুনতি;
        dop.buf_fl += গুনতি*CDROM_SECTOR_SIZE;
    }

    যদি (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        ঘূৰাই 0;
    }

    // Emulation of a floppy/harddisk requested
    যদি (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        ঘূৰাই 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    যদি (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        set_equipment_flags(0x41, 0x41);

        ৱিকল্প (media) {
        পক্ষ 0x01:  // 1.2M floppy
            CDEmu.chs.sptcyl = 15;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            ভঙ্গ;
        পক্ষ 0x02:  // 1.44M floppy
            CDEmu.chs.sptcyl = 18;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            ভঙ্গ;
        পক্ষ 0x03:  // 2.88M floppy
            CDEmu.chs.sptcyl = 36;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            ভঙ্গ;
        }
    } অন্যথা {
        // Harddrive emulation
        CDEmu.emulated_drive = 0x80;
        SET_BDA(hdcount, GET_BDA(hdcount) + 1);

        // Peak at partition table to get chs.
        সংরচনা mbr_s *mbr = MAKE_FLATPTR(boot_segment, 0);
        CDEmu.chs = mbr->partitions[0].last;
    }

    // everything is ok, so from now on, the emulation is active
    CDEmu.media = media;
    dprintf(6, "cdemu media=%d\n", media);

    ঘূৰাই 0;
}

// check if media is present and the drive is bootable.
// in case it is return the volume label.
ৱর্ণ*
cdrom_media_info(সংরচনা drive_s *drive)
{
    ASSERT32FLAT();

    সংরচনা disk_op_s dop;
    memset(&dop, 0, পরিমাণ(dop));
    dop.drive_fl = drive;

    পূর্ণাঙ্ক ret = scsi_is_ready(&dop);
    যদি (ret)
        ঘূৰাই রিক্ত;

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.গুনতি = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    যদি (ret)
        ঘূৰাই রিক্ত;

    // Is it bootable?
    যদি (buffer[0])
        ঘূৰাই রিক্ত;
    যদি (strcmp((ৱর্ণ*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ঘূৰাই রিক্ত;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.গুনতি = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    যদি (ret)
        ঘূৰাই রিক্ত;

    // Read volume id, trim trailing spaces
    ৱর্ণ *volume = znprintf(30, "%s", buffer + 40);
    nullTrailingSpace(volume);
    ঘূৰাই volume;
}

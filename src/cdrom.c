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

بنية eltorito_s CDEmu VARLOW = { .size=حجم(CDEmu) };
بنية drive_s *emulated_drive_gf VARLOW;
بنية drive_s *cdemu_drive_gf VARFSEG;

ساكن صحيح
cdemu_read(بنية disk_op_s *op)
{
    بنية drive_s *drive_gf = GET_LOW(emulated_drive_gf);
    بنية disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_LOW(CDEmu.ilba) + op->lba / 4;

    صحيح count = op->count;
    op->count = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    ܐܢ (op->lba & 3) {
        // Partial read of first block.
        dop.count = 1;
        dop.buf_fl = cdbuf_fl;
        صحيح ret = process_op(&dop);
        ܐܢ (ret)
            ܦܢܐ ret;
        u8 thiscount = 4 - (op->lba & 3);
        ܐܢ (thiscount > count)
            thiscount = count;
        count -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->count += thiscount;
        dop.lba++;
    }

    ܐܢ (count > 3) {
        // Read n number of regular blocks.
        dop.count = count / 4;
        dop.buf_fl = op->buf_fl;
        صحيح ret = process_op(&dop);
        op->count += dop.count * 4;
        ܐܢ (ret)
            ܦܢܐ ret;
        u8 thiscount = count & ~3;
        count &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    ܐܢ (count) {
        // Partial read on last block.
        dop.count = 1;
        dop.buf_fl = cdbuf_fl;
        صحيح ret = process_op(&dop);
        ܐܢ (ret)
            ܦܢܐ ret;
        u8 thiscount = count;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->count += thiscount;
    }

    ܦܢܐ DISK_RET_SUCCESS;
}

صحيح
cdemu_process_op(بنية disk_op_s *op)
{
    ܐܢ (!CONFIG_CDROM_EMU)
        ܦܢܐ 0;

    بدل (op->command) {
    حالة CMD_READ:
        ܦܢܐ cdemu_read(op);
    حالة CMD_WRITE:
    حالة CMD_FORMAT:
        ܦܢܐ DISK_RET_EWRITEPROTECT;
    افتراضي:
        ܦܢܐ default_process_op(op);
    }
}

فراغ
cdrom_prepboot(فراغ)
{
    ܐܢ (!CONFIG_CDROM_EMU)
        ܦܢܐ;
    ܐܢ (!CDCount)
        ܦܢܐ;
    ܐܢ (create_bounce_buf() < 0)
        ܦܢܐ;

    بنية drive_s *drive = malloc_fseg(حجم(*drive));
    ܐܢ (!drive) {
        warn_noalloc();
        ܦܢܐ;
    }
    cdemu_drive_gf = drive;
    memset(drive, 0, حجم(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_SIZE;
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
    ܐܢ (!dop.drive_fl || cdid < 0)
        ܦܢܐ 1;

    صحيح ret = scsi_is_ready(&dop);
    ܐܢ (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.count = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    ܐܢ (ret)
        ܦܢܐ 3;

    // Validity checks
    ܐܢ (buffer[0])
        ܦܢܐ 4;
    ܐܢ (strcmp((حرف*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ܦܢܐ 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&buffer[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.count = 1;
    ret = process_op(&dop);
    ܐܢ (ret)
        ܦܢܐ 7;

    // Validation entry
    ܐܢ (buffer[0x00] != 0x01)
        ܦܢܐ 8;   // Header
    ܐܢ (buffer[0x01] != 0x00)
        ܦܢܐ 9;   // Platform
    ܐܢ (buffer[0x1E] != 0x55)
        ܦܢܐ 10;  // key 1
    ܐܢ (buffer[0x1F] != 0xAA)
        ܦܢܐ 10;  // key 2

    // Initial/Default Entry
    ܐܢ (buffer[0x20] != 0x88)
        ܦܢܐ 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_add_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), buffer), حجم(buffer));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = buffer[0x21];

    u16 boot_segment = *(u16*)&buffer[0x22];
    ܐܢ (!boot_segment)
        boot_segment = 0x07C0;
    CDEmu.load_segment = boot_segment;
    CDEmu.buffer_segment = 0x0000;

    u16 nbsectors = *(u16*)&buffer[0x26];
    CDEmu.sector_count = nbsectors;

    lba = *(u32*)&buffer[0x28];
    CDEmu.ilba = lba;

    CDEmu.controller_index = drive->cntl_id / 2;
    CDEmu.device_spec = drive->cntl_id % 2;

    // And we read the image in memory
    nbsectors = DIV_ROUND_UP(nbsectors, 4);
    dop.lba = lba;
    dop.buf_fl = MAKE_FLATPTR(boot_segment, 0);
    ܟܡܐ (nbsectors) {
        صحيح count = nbsectors;
        ܐܢ (count > 64*1024/CDROM_SECTOR_SIZE)
            count = 64*1024/CDROM_SECTOR_SIZE;
        dop.count = count;
        ret = process_op(&dop);
        ܐܢ (ret)
            ܦܢܐ 12;
        nbsectors -= count;
        dop.lba += count;
        dop.buf_fl += count*CDROM_SECTOR_SIZE;
    }

    ܐܢ (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        ܦܢܐ 0;
    }

    // Emulation of a floppy/harddisk requested
    ܐܢ (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        ܦܢܐ 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    ܐܢ (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        set_equipment_flags(0x41, 0x41);

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
    } ܐܚܪܢܐ {
        // Harddrive emulation
        CDEmu.emulated_drive = 0x80;
        SET_BDA(hdcount, GET_BDA(hdcount) + 1);

        // Peak at partition table to get chs.
        بنية mbr_s *mbr = MAKE_FLATPTR(boot_segment, 0);
        CDEmu.chs = mbr->partitions[0].last;
    }

    // everything is ok, so from now on, the emulation is active
    CDEmu.media = media;
    dprintf(6, "cdemu media=%d\n", media);

    ܦܢܐ 0;
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
    ܐܢ (ret)
        ܦܢܐ عدم;

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.count = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    ܐܢ (ret)
        ܦܢܐ عدم;

    // Is it bootable?
    ܐܢ (buffer[0])
        ܦܢܐ عدم;
    ܐܢ (strcmp((حرف*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ܦܢܐ عدم;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.count = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    ܐܢ (ret)
        ܦܢܐ عدم;

    // Read volume id, trim trailing spaces
    حرف *volume = znprintf(30, "%s", buffer + 40);
    nullTrailingSpace(volume);
    ܦܢܐ volume;
}

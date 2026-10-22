// Support for booting from cdroms (the "El Torito" spec).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBAL
#ಅನ್ತರ್ಭಾವ "block.h" // struct drive_s
#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "hw/ata.h" // ATA_CMD_REQUEST_SENSE
#ಅನ್ತರ್ಭಾವ "hw/blockcmd.h" // CDB_CMD_REQUEST_SENSE
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_SUCCESS
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // cdrom_prepboot
#ಅನ್ತರ್ಭಾವ "tcgbios.h" // tpm_*


/****************************************************************
 * CD emulation
 ****************************************************************/

ಸಂರಚನಾ eltorito_s CDEmu VARLOW = { .size=ಪರಿಮಾಣ(CDEmu) };
ಸಂರಚನಾ drive_s *emulated_drive_gf VARLOW;
ಸಂರಚನಾ drive_s *cdemu_drive_gf VARFSEG;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
cdemu_read(ಸಂರಚನಾ disk_op_s *op)
{
    ಸಂರಚನಾ drive_s *drive_gf = GET_ಕೆಳಗಿನ(emulated_drive_gf);
    ಸಂರಚನಾ disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_ಕೆಳಗಿನ(CDEmu.ilba) + op->lba / 4;

    ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = op->ಎಣಿಕೆ;
    op->ಎಣಿಕೆ = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    ಯದಿ (op->lba & 3) {
        // Partial read of first block.
        dop.ಎಣಿಕೆ = 1;
        dop.buf_fl = cdbuf_fl;
        ಪೂರ್ಣಾಙ್ಕ ret = process_op(&dop);
        ಯದಿ (ret)
            ಪ್ರತ್ಯಾಗಮ ret;
        u8 thiscount = 4 - (op->lba & 3);
        ಯದಿ (thiscount > ಎಣಿಕೆ)
            thiscount = ಎಣಿಕೆ;
        ಎಣಿಕೆ -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->ಎಣಿಕೆ += thiscount;
        dop.lba++;
    }

    ಯದಿ (ಎಣಿಕೆ > 3) {
        // Read n number of regular blocks.
        dop.ಎಣಿಕೆ = ಎಣಿಕೆ / 4;
        dop.buf_fl = op->buf_fl;
        ಪೂರ್ಣಾಙ್ಕ ret = process_op(&dop);
        op->ಎಣಿಕೆ += dop.ಎಣಿಕೆ * 4;
        ಯದಿ (ret)
            ಪ್ರತ್ಯಾಗಮ ret;
        u8 thiscount = ಎಣಿಕೆ & ~3;
        ಎಣಿಕೆ &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    ಯದಿ (ಎಣಿಕೆ) {
        // Partial read on last block.
        dop.ಎಣಿಕೆ = 1;
        dop.buf_fl = cdbuf_fl;
        ಪೂರ್ಣಾಙ್ಕ ret = process_op(&dop);
        ಯದಿ (ret)
            ಪ್ರತ್ಯಾಗಮ ret;
        u8 thiscount = ಎಣಿಕೆ;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->ಎಣಿಕೆ += thiscount;
    }

    ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;
}

ಪೂರ್ಣಾಙ್ಕ
cdemu_process_op(ಸಂರಚನಾ disk_op_s *op)
{
    ಯದಿ (!CONFIG_CDROM_EMU)
        ಪ್ರತ್ಯಾಗಮ 0;

    ವಿಕಲ್ಪ (op->command) {
    ಪಕ್ಷ CMD_READ:
        ಪ್ರತ್ಯಾಗಮ cdemu_read(op);
    ಪಕ್ಷ CMD_WRITE:
    ಪಕ್ಷ CMD_FORMAT:
        ಪ್ರತ್ಯಾಗಮ DISK_RET_EWRITEPROTECT;
    ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ default_process_op(op);
    }
}

ಶೂನ್ಯ
cdrom_prepboot(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_CDROM_EMU)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (!CDCount)
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (create_bounce_buf() < 0)
        ಪ್ರತ್ಯಾಗಮ;

    ಸಂರಚನಾ drive_s *drive = malloc_fseg(ಪರಿಮಾಣ(*drive));
    ಯದಿ (!drive) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    cdemu_drive_gf = drive;
    memset(drive, 0, ಪರಿಮಾಣ(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_SIZE;
    drive->sectors = (u64)-1;
}


/****************************************************************
 * CD booting
 ****************************************************************/

ಪೂರ್ಣಾಙ್ಕ
cdrom_boot(ಸಂರಚನಾ drive_s *drive)
{
    ASSERT32FLAT();
    ಸಂರಚನಾ disk_op_s dop;
    ಪೂರ್ಣಾಙ್ಕ cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, ಪರಿಮಾಣ(dop));
    dop.drive_fl = drive;
    ಯದಿ (!dop.drive_fl || cdid < 0)
        ಪ್ರತ್ಯಾಗಮ 1;

    ಪೂರ್ಣಾಙ್ಕ ret = scsi_is_ready(&dop);
    ಯದಿ (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.ಎಣಿಕೆ = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ 3;

    // Validity checks
    ಯದಿ (buffer[0])
        ಪ್ರತ್ಯಾಗಮ 4;
    ಯದಿ (strcmp((ವರ್ಣ*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ಪ್ರತ್ಯಾಗಮ 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&buffer[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.ಎಣಿಕೆ = 1;
    ret = process_op(&dop);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ 7;

    // Validation entry
    ಯದಿ (buffer[0x00] != 0x01)
        ಪ್ರತ್ಯಾಗಮ 8;   // Header
    ಯದಿ (buffer[0x01] != 0x00)
        ಪ್ರತ್ಯಾಗಮ 9;   // Platform
    ಯದಿ (buffer[0x1E] != 0x55)
        ಪ್ರತ್ಯಾಗಮ 10;  // key 1
    ಯದಿ (buffer[0x1F] != 0xAA)
        ಪ್ರತ್ಯಾಗಮ 10;  // key 2

    // Initial/Default Entry
    ಯದಿ (buffer[0x20] != 0x88)
        ಪ್ರತ್ಯಾಗಮ 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_add_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), buffer), ಪರಿಮಾಣ(buffer));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = buffer[0x21];

    u16 boot_segment = *(u16*)&buffer[0x22];
    ಯದಿ (!boot_segment)
        boot_segment = 0x07C0;
    CDEmu.load_segment = boot_segment;
    CDEmu.buffer_segment = 0x0000;

    u16 nbsectors = *(u16*)&buffer[0x26];
    CDEmu.sector_ಎಣಿಕೆ = nbsectors;

    lba = *(u32*)&buffer[0x28];
    CDEmu.ilba = lba;

    CDEmu.controller_index = drive->cntl_id / 2;
    CDEmu.device_spec = drive->cntl_id % 2;

    // And we read the image in memory
    nbsectors = DIV_ROUND_UP(nbsectors, 4);
    dop.lba = lba;
    dop.buf_fl = MAKE_FLATPTR(boot_segment, 0);
    ಯಾವತ್ (nbsectors) {
        ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = nbsectors;
        ಯದಿ (ಎಣಿಕೆ > 64*1024/CDROM_SECTOR_SIZE)
            ಎಣಿಕೆ = 64*1024/CDROM_SECTOR_SIZE;
        dop.ಎಣಿಕೆ = ಎಣಿಕೆ;
        ret = process_op(&dop);
        ಯದಿ (ret)
            ಪ್ರತ್ಯಾಗಮ 12;
        nbsectors -= ಎಣಿಕೆ;
        dop.lba += ಎಣಿಕೆ;
        dop.buf_fl += ಎಣಿಕೆ*CDROM_SECTOR_SIZE;
    }

    ಯದಿ (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        ಪ್ರತ್ಯಾಗಮ 0;
    }

    // Emulation of a floppy/harddisk requested
    ಯದಿ (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        ಪ್ರತ್ಯಾಗಮ 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    ಯದಿ (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        set_equipment_flags(0x41, 0x41);

        ವಿಕಲ್ಪ (media) {
        ಪಕ್ಷ 0x01:  // 1.2M floppy
            CDEmu.chs.sptcyl = 15;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            ಭಙ್ಗ;
        ಪಕ್ಷ 0x02:  // 1.44M floppy
            CDEmu.chs.sptcyl = 18;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            ಭಙ್ಗ;
        ಪಕ್ಷ 0x03:  // 2.88M floppy
            CDEmu.chs.sptcyl = 36;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            ಭಙ್ಗ;
        }
    } ಅನ್ಯಥಾ {
        // Harddrive emulation
        CDEmu.emulated_drive = 0x80;
        SET_BDA(hdcount, GET_BDA(hdcount) + 1);

        // Peak at partition table to get chs.
        ಸಂರಚನಾ mbr_s *mbr = MAKE_FLATPTR(boot_segment, 0);
        CDEmu.chs = mbr->partitions[0].last;
    }

    // everything is ok, so from now on, the emulation is active
    CDEmu.media = media;
    dprintf(6, "cdemu media=%d\n", media);

    ಪ್ರತ್ಯಾಗಮ 0;
}

// check if media is present and the drive is bootable.
// in case it is return the volume label.
ವರ್ಣ*
cdrom_media_info(ಸಂರಚನಾ drive_s *drive)
{
    ASSERT32FLAT();

    ಸಂರಚನಾ disk_op_s dop;
    memset(&dop, 0, ಪರಿಮಾಣ(dop));
    dop.drive_fl = drive;

    ಪೂರ್ಣಾಙ್ಕ ret = scsi_is_ready(&dop);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.ಎಣಿಕೆ = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

    // Is it bootable?
    ಯದಿ (buffer[0])
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    ಯದಿ (strcmp((ವರ್ಣ*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.ಎಣಿಕೆ = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

    // Read volume id, trim trailing spaces
    ವರ್ಣ *volume = znprintf(30, "%s", buffer + 40);
    nullTrailingSpace(volume);
    ಪ್ರತ್ಯಾಗಮ volume;
}

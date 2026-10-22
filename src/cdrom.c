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

ರಚನೆ eltorito_s CDEmu VARLOW = { .ಗಾತ್ರ_2=ಗಾತ್ರ(CDEmu) };
ರಚನೆ drive_s *emulated_drive_gf VARLOW;
ರಚನೆ drive_s *cdemu_drive_gf VARFSEG;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
cdemu_read(ರಚನೆ disk_op_s *op)
{
    ರಚನೆ drive_s *drive_gf = GET_ಕೆಳಗಿನ(emulated_drive_gf);
    ರಚನೆ disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_ಕೆಳಗಿನ(CDEmu.ilba) + op->lba / 4;

    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = op->ಎಣಿಕೆ_2;
    op->ಎಣಿಕೆ_2 = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    ಆದರೆ (op->lba & 3) {
        // Partial read of first block.
        dop.ಎಣಿಕೆ_2 = 1;
        dop.buf_fl = cdbuf_fl;
        ಪೂರ್ಣಾಂಕ ret = process_op(&dop);
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ ret;
        u8 thiscount = 4 - (op->lba & 3);
        ಆದರೆ (thiscount > ಎಣಿಕೆ_2)
            thiscount = ಎಣಿಕೆ_2;
        ಎಣಿಕೆ_2 -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->ಎಣಿಕೆ_2 += thiscount;
        dop.lba++;
    }

    ಆದರೆ (ಎಣಿಕೆ_2 > 3) {
        // Read n number of regular blocks.
        dop.ಎಣಿಕೆ_2 = ಎಣಿಕೆ_2 / 4;
        dop.buf_fl = op->buf_fl;
        ಪೂರ್ಣಾಂಕ ret = process_op(&dop);
        op->ಎಣಿಕೆ_2 += dop.ಎಣಿಕೆ_2 * 4;
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ ret;
        u8 thiscount = ಎಣಿಕೆ_2 & ~3;
        ಎಣಿಕೆ_2 &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    ಆದರೆ (ಎಣಿಕೆ_2) {
        // Partial read on last block.
        dop.ಎಣಿಕೆ_2 = 1;
        dop.buf_fl = cdbuf_fl;
        ಪೂರ್ಣಾಂಕ ret = process_op(&dop);
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ ret;
        u8 thiscount = ಎಣಿಕೆ_2;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->ಎಣಿಕೆ_2 += thiscount;
    }

    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}

ಪೂರ್ಣಾಂಕ
cdemu_process_op(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (!CONFIG_CDROM_EMU)
        ಹಿಂತಿರುಗಿ 0;

    ಆಯ್ಕೆ (op->command) {
    ಸ್ಥಿತಿ CMD_READ:
        ಹಿಂತಿರುಗಿ cdemu_read(op);
    ಸ್ಥಿತಿ CMD_WRITE:
    ಸ್ಥಿತಿ CMD_FORMAT:
        ಹಿಂತಿರುಗಿ DISK_RET_EWRITEPROTECT;
    ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ default_process_op(op);
    }
}

ಶೂನ್ಯ
cdrom_prepboot(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_CDROM_EMU)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (!CDCount)
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (create_bounce_buf() < 0)
        ಹಿಂತಿರುಗಿ;

    ರಚನೆ drive_s *drive = malloc_fseg(ಗಾತ್ರ(*drive));
    ಆದರೆ (!drive) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    cdemu_drive_gf = drive;
    memset(drive, 0, ಗಾತ್ರ(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_ಗಾತ್ರ;
    drive->sectors = (u64)-1;
}


/****************************************************************
 * CD booting
 ****************************************************************/

ಪೂರ್ಣಾಂಕ
cdrom_boot(ರಚನೆ drive_s *drive)
{
    ASSERT32FLAT();
    ರಚನೆ disk_op_s dop;
    ಪೂರ್ಣಾಂಕ cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, ಗಾತ್ರ(dop));
    dop.drive_fl = drive;
    ಆದರೆ (!dop.drive_fl || cdid < 0)
        ಹಿಂತಿರುಗಿ 1;

    ಪೂರ್ಣಾಂಕ ret = scsi_is_ready(&dop);
    ಆದರೆ (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 ಬಫರ್[CDROM_SECTOR_ಗಾತ್ರ];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.ಎಣಿಕೆ_2 = 1;
    dop.buf_fl = ಬಫರ್;
    ret = process_op(&dop);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ 3;

    // Validity checks
    ಆದರೆ (ಬಫರ್[0])
        ಹಿಂತಿರುಗಿ 4;
    ಆದರೆ (strcmp((ಅಕ್ಷರ*)&ಬಫರ್[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ಹಿಂತಿರುಗಿ 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&ಬಫರ್[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.ಎಣಿಕೆ_2 = 1;
    ret = process_op(&dop);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ 7;

    // Validation entry
    ಆದರೆ (ಬಫರ್[0x00] != 0x01)
        ಹಿಂತಿರುಗಿ 8;   //ಶೀರ್ಷಿಕೆ
    ಆದರೆ (ಬಫರ್[0x01] != 0x00)
        ಹಿಂತಿರುಗಿ 9;   // Platform
    ಆದರೆ (ಬಫರ್[0x1E] != 0x55)
        ಹಿಂತಿರುಗಿ 10;  // key 1
    ಆದರೆ (ಬಫರ್[0x1F] != 0xAA)
        ಹಿಂತಿರುಗಿ 10;  // key 2

    // Initial/Default Entry
    ಆದರೆ (ಬಫರ್[0x20] != 0x88)
        ಹಿಂತಿರುಗಿ 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_ಕೂಡಿಸು_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), ಬಫರ್), ಗಾತ್ರ(ಬಫರ್));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = ಬಫರ್[0x21];

    u16 boot_segment = *(u16*)&ಬಫರ್[0x22];
    ಆದರೆ (!boot_segment)
        boot_segment = 0x07C0;
    CDEmu.ತುಂಬು_segment = boot_segment;
    CDEmu.ಬಫರ್_segment = 0x0000;

    u16 nbsectors = *(u16*)&ಬಫರ್[0x26];
    CDEmu.sector_ಎಣಿಕೆ = nbsectors;

    lba = *(u32*)&ಬಫರ್[0x28];
    CDEmu.ilba = lba;

    CDEmu.controller_ಸೂಚ್ಯಂಕ = drive->cntl_id / 2;
    CDEmu.device_spec = drive->cntl_id % 2;

    // And we read the image in memory
    nbsectors = DIV_ಸುತ್ತು_ಮೇಲೆ(nbsectors, 4);
    dop.lba = lba;
    dop.buf_fl = MAKE_FLATPTR(boot_segment, 0);
    ಆಗ (nbsectors) {
        ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = nbsectors;
        ಆದರೆ (ಎಣಿಕೆ_2 > 64*1024/CDROM_SECTOR_ಗಾತ್ರ)
            ಎಣಿಕೆ_2 = 64*1024/CDROM_SECTOR_ಗಾತ್ರ;
        dop.ಎಣಿಕೆ_2 = ಎಣಿಕೆ_2;
        ret = process_op(&dop);
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ 12;
        nbsectors -= ಎಣಿಕೆ_2;
        dop.lba += ಎಣಿಕೆ_2;
        dop.buf_fl += ಎಣಿಕೆ_2*CDROM_SECTOR_ಗಾತ್ರ;
    }

    ಆದರೆ (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        ಹಿಂತಿರುಗಿ 0;
    }

    // Emulation of a floppy/harddisk requested
    ಆದರೆ (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        ಹಿಂತಿರುಗಿ 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    ಆದರೆ (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        ಗಣ_equipment_flags(0x41, 0x41);

        ಆಯ್ಕೆ (media) {
        ಸ್ಥಿತಿ 0x01:  // 1.2M floppy
            CDEmu.chs.sptcyl = 15;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 0x02:  // 1.44M floppy
            CDEmu.chs.sptcyl = 18;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 0x03:  // 2.88M floppy
            CDEmu.chs.sptcyl = 36;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            ನಿಲ್ಲಿಸು;
        }
    } ಇಲ್ಲದಿದ್ದರೆ {
        // Harddrive emulation
        CDEmu.emulated_drive = 0x80;
        ಗಣ_BDA(hdcount, GET_BDA(hdcount) + 1);

        // Peak at partition table to get chs.
        ರಚನೆ mbr_s *mbr = MAKE_FLATPTR(boot_segment, 0);
        CDEmu.chs = mbr->partitions[0].ಕೊನೆಯ;
    }

    // everything is ok, so from now on, the emulation is active
    CDEmu.media = media;
    dprintf(6, "cdemu media=%d\n", media);

    ಹಿಂತಿರುಗಿ 0;
}

// check if media is present and the drive is bootable.
// in case it is return the volume label.
ಅಕ್ಷರ*
cdrom_media_info(ರಚನೆ drive_s *drive)
{
    ASSERT32FLAT();

    ರಚನೆ disk_op_s dop;
    memset(&dop, 0, ಗಾತ್ರ(dop));
    dop.drive_fl = drive;

    ಪೂರ್ಣಾಂಕ ret = scsi_is_ready(&dop);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    // Read the Boot Record Volume Descriptor
    u8 ಬಫರ್[CDROM_SECTOR_ಗಾತ್ರ];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.ಎಣಿಕೆ_2 = 1;
    dop.buf_fl = ಬಫರ್;
    ret = process_op(&dop);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    // Is it bootable?
    ಆದರೆ (ಬಫರ್[0])
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಆದರೆ (strcmp((ಅಕ್ಷರ*)&ಬಫರ್[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.ಎಣಿಕೆ_2 = 1;
    dop.buf_fl = ಬಫರ್;
    ret = process_op(&dop);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    // Read volume id, trim trailing spaces
    ಅಕ್ಷರ *volume = znprintf(30, "%s", ಬಫರ್ + 40);
    nullTrailingSpace(volume);
    ಹಿಂತಿರುಗಿ volume;
}

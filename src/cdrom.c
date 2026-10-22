// Support for booting from cdroms (the "El Torito" spec).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBAL
#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // struct drive_s
#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/ata.h" // ATA_CMD_REQUEST_SENSE
#ਅਨ੍ਤਰ੍ਭਾਵ "hw/blockcmd.h" // CDB_CMD_REQUEST_SENSE
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // DISK_RET_SUCCESS
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // cdrom_prepboot
#ਅਨ੍ਤਰ੍ਭਾਵ "tcgbios.h" // tpm_*


/****************************************************************
 * CD emulation
 ****************************************************************/

ਬਣਤਰ eltorito_s CDEmu VARLOW = { .ਮਾਪ=ਆਕਾਰ(CDEmu) };
ਬਣਤਰ drive_s *emulated_drive_gf VARLOW;
ਬਣਤਰ drive_s *cdemu_drive_gf VARFSEG;

ਸਥਿਰ ਪੂਰਨ
cdemu_read(ਬਣਤਰ disk_op_s *op)
{
    ਬਣਤਰ drive_s *drive_gf = GET_ਨੀਵਾਂ(emulated_drive_gf);
    ਬਣਤਰ disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_ਨੀਵਾਂ(CDEmu.ilba) + op->lba / 4;

    ਪੂਰਨ ਗਿਣਤੀ_2 = op->ਗਿਣਤੀ_2;
    op->ਗਿਣਤੀ_2 = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    ਜੇ (op->lba & 3) {
        // Partial read of first block.
        dop.ਗਿਣਤੀ_2 = 1;
        dop.buf_fl = cdbuf_fl;
        ਪੂਰਨ ret = process_op(&dop);
        ਜੇ (ret)
            ਵਾਪਸ ret;
        u8 thiscount = 4 - (op->lba & 3);
        ਜੇ (thiscount > ਗਿਣਤੀ_2)
            thiscount = ਗਿਣਤੀ_2;
        ਗਿਣਤੀ_2 -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->ਗਿਣਤੀ_2 += thiscount;
        dop.lba++;
    }

    ਜੇ (ਗਿਣਤੀ_2 > 3) {
        // Read n number of regular blocks.
        dop.ਗਿਣਤੀ_2 = ਗਿਣਤੀ_2 / 4;
        dop.buf_fl = op->buf_fl;
        ਪੂਰਨ ret = process_op(&dop);
        op->ਗਿਣਤੀ_2 += dop.ਗਿਣਤੀ_2 * 4;
        ਜੇ (ret)
            ਵਾਪਸ ret;
        u8 thiscount = ਗਿਣਤੀ_2 & ~3;
        ਗਿਣਤੀ_2 &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    ਜੇ (ਗਿਣਤੀ_2) {
        // Partial read on last block.
        dop.ਗਿਣਤੀ_2 = 1;
        dop.buf_fl = cdbuf_fl;
        ਪੂਰਨ ret = process_op(&dop);
        ਜੇ (ret)
            ਵਾਪਸ ret;
        u8 thiscount = ਗਿਣਤੀ_2;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->ਗਿਣਤੀ_2 += thiscount;
    }

    ਵਾਪਸ DISK_RET_SUCCESS;
}

ਪੂਰਨ
cdemu_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (!CONFIG_CDROM_EMU)
        ਵਾਪਸ 0;

    ਚੋਣ (op->command) {
    ਹਾਲਤ CMD_READ:
        ਵਾਪਸ cdemu_read(op);
    ਹਾਲਤ CMD_WRITE:
    ਹਾਲਤ CMD_FORMAT:
        ਵਾਪਸ DISK_RET_EWRITEPROTECT;
    ਮੂਲ:
        ਵਾਪਸ default_process_op(op);
    }
}

ਖਾਲੀ
cdrom_prepboot(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_CDROM_EMU)
        ਵਾਪਸ;
    ਜੇ (!CDCount)
        ਵਾਪਸ;
    ਜੇ (create_bounce_buf() < 0)
        ਵਾਪਸ;

    ਬਣਤਰ drive_s *drive = malloc_fseg(ਆਕਾਰ(*drive));
    ਜੇ (!drive) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    cdemu_drive_gf = drive;
    memset(drive, 0, ਆਕਾਰ(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_ਮਾਪ;
    drive->sectors = (u64)-1;
}


/****************************************************************
 * CD booting
 ****************************************************************/

ਪੂਰਨ
cdrom_boot(ਬਣਤਰ drive_s *drive)
{
    ASSERT32FLAT();
    ਬਣਤਰ disk_op_s dop;
    ਪੂਰਨ cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, ਆਕਾਰ(dop));
    dop.drive_fl = drive;
    ਜੇ (!dop.drive_fl || cdid < 0)
        ਵਾਪਸ 1;

    ਪੂਰਨ ret = scsi_is_ready(&dop);
    ਜੇ (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 ਬਫਰ[CDROM_SECTOR_ਮਾਪ];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.ਗਿਣਤੀ_2 = 1;
    dop.buf_fl = ਬਫਰ;
    ret = process_op(&dop);
    ਜੇ (ret)
        ਵਾਪਸ 3;

    // Validity checks
    ਜੇ (ਬਫਰ[0])
        ਵਾਪਸ 4;
    ਜੇ (strcmp((ਅੱਖਰ*)&ਬਫਰ[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ਵਾਪਸ 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&ਬਫਰ[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.ਗਿਣਤੀ_2 = 1;
    ret = process_op(&dop);
    ਜੇ (ret)
        ਵਾਪਸ 7;

    // Validation entry
    ਜੇ (ਬਫਰ[0x00] != 0x01)
        ਵਾਪਸ 8;   //ਸਿਰਲੇਖ
    ਜੇ (ਬਫਰ[0x01] != 0x00)
        ਵਾਪਸ 9;   // Platform
    ਜੇ (ਬਫਰ[0x1E] != 0x55)
        ਵਾਪਸ 10;  // key 1
    ਜੇ (ਬਫਰ[0x1F] != 0xAA)
        ਵਾਪਸ 10;  // key 2

    // Initial/Default Entry
    ਜੇ (ਬਫਰ[0x20] != 0x88)
        ਵਾਪਸ 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_ਜੋੜੋ_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), ਬਫਰ), ਆਕਾਰ(ਬਫਰ));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = ਬਫਰ[0x21];

    u16 boot_segment = *(u16*)&ਬਫਰ[0x22];
    ਜੇ (!boot_segment)
        boot_segment = 0x07C0;
    CDEmu.ਲੱਦੋ_segment = boot_segment;
    CDEmu.ਬਫਰ_segment = 0x0000;

    u16 nbsectors = *(u16*)&ਬਫਰ[0x26];
    CDEmu.sector_ਗਿਣਤੀ = nbsectors;

    lba = *(u32*)&ਬਫਰ[0x28];
    CDEmu.ilba = lba;

    CDEmu.controller_ਸੂਚਕਾਂਕ = drive->cntl_id / 2;
    CDEmu.device_spec = drive->cntl_id % 2;

    // And we read the image in memory
    nbsectors = DIV_ਗੇੜਾ_ਉੱਪਰ(nbsectors, 4);
    dop.lba = lba;
    dop.buf_fl = MAKE_FLATPTR(boot_segment, 0);
    ਜਦੋਂ (nbsectors) {
        ਪੂਰਨ ਗਿਣਤੀ_2 = nbsectors;
        ਜੇ (ਗਿਣਤੀ_2 > 64*1024/CDROM_SECTOR_ਮਾਪ)
            ਗਿਣਤੀ_2 = 64*1024/CDROM_SECTOR_ਮਾਪ;
        dop.ਗਿਣਤੀ_2 = ਗਿਣਤੀ_2;
        ret = process_op(&dop);
        ਜੇ (ret)
            ਵਾਪਸ 12;
        nbsectors -= ਗਿਣਤੀ_2;
        dop.lba += ਗਿਣਤੀ_2;
        dop.buf_fl += ਗਿਣਤੀ_2*CDROM_SECTOR_ਮਾਪ;
    }

    ਜੇ (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        ਵਾਪਸ 0;
    }

    // Emulation of a floppy/harddisk requested
    ਜੇ (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        ਵਾਪਸ 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    ਜੇ (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        ਸੈੱਟ_equipment_flags(0x41, 0x41);

        ਚੋਣ (media) {
        ਹਾਲਤ 0x01:  // 1.2M floppy
            CDEmu.chs.sptcyl = 15;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            ਰੋਕੋ;
        ਹਾਲਤ 0x02:  // 1.44M floppy
            CDEmu.chs.sptcyl = 18;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            ਰੋਕੋ;
        ਹਾਲਤ 0x03:  // 2.88M floppy
            CDEmu.chs.sptcyl = 36;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            ਰੋਕੋ;
        }
    } ਨਹੀਂਤਾਂ {
        // Harddrive emulation
        CDEmu.emulated_drive = 0x80;
        ਸੈੱਟ_BDA(hdcount, GET_BDA(hdcount) + 1);

        // Peak at partition table to get chs.
        ਬਣਤਰ mbr_s *mbr = MAKE_FLATPTR(boot_segment, 0);
        CDEmu.chs = mbr->partitions[0].ਆਖਰੀ;
    }

    // everything is ok, so from now on, the emulation is active
    CDEmu.media = media;
    dprintf(6, "cdemu media=%d\n", media);

    ਵਾਪਸ 0;
}

// check if media is present and the drive is bootable.
// in case it is return the volume label.
ਅੱਖਰ*
cdrom_media_info(ਬਣਤਰ drive_s *drive)
{
    ASSERT32FLAT();

    ਬਣਤਰ disk_op_s dop;
    memset(&dop, 0, ਆਕਾਰ(dop));
    dop.drive_fl = drive;

    ਪੂਰਨ ret = scsi_is_ready(&dop);
    ਜੇ (ret)
        ਵਾਪਸ ਨਿਰਾ;

    // Read the Boot Record Volume Descriptor
    u8 ਬਫਰ[CDROM_SECTOR_ਮਾਪ];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.ਗਿਣਤੀ_2 = 1;
    dop.buf_fl = ਬਫਰ;
    ret = process_op(&dop);
    ਜੇ (ret)
        ਵਾਪਸ ਨਿਰਾ;

    // Is it bootable?
    ਜੇ (ਬਫਰ[0])
        ਵਾਪਸ ਨਿਰਾ;
    ਜੇ (strcmp((ਅੱਖਰ*)&ਬਫਰ[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ਵਾਪਸ ਨਿਰਾ;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.ਗਿਣਤੀ_2 = 1;
    dop.buf_fl = ਬਫਰ;
    ret = process_op(&dop);
    ਜੇ (ret)
        ਵਾਪਸ ਨਿਰਾ;

    // Read volume id, trim trailing spaces
    ਅੱਖਰ *volume = znprintf(30, "%s", ਬਫਰ + 40);
    nullTrailingSpace(volume);
    ਵਾਪਸ volume;
}

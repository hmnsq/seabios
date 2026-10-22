// Support for booting from cdroms (the "El Torito" spec).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_GLOBAL
#അന്തര്ഭാവ "block.h" // struct drive_s
#അന്തര്ഭാവ "bregs.h" // struct bregs
#അന്തര്ഭാവ "hw/ata.h" // ATA_CMD_REQUEST_SENSE
#അന്തര്ഭാവ "hw/blockcmd.h" // CDB_CMD_REQUEST_SENSE
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "std/disk.h" // DISK_RET_SUCCESS
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // cdrom_prepboot
#അന്തര്ഭാവ "tcgbios.h" // tpm_*


/****************************************************************
 * CD emulation
 ****************************************************************/

ഘടന eltorito_s CDEmu VARLOW = { .size=വലുപ്പം(CDEmu) };
ഘടന drive_s *emulated_drive_gf VARLOW;
ഘടന drive_s *cdemu_drive_gf VARFSEG;

സ്ഥിതം പൂർണ്ണസംഖ്യ
cdemu_read(ഘടന disk_op_s *op)
{
    ഘടന drive_s *drive_gf = GET_താഴെയുള്ള(emulated_drive_gf);
    ഘടന disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_താഴെയുള്ള(CDEmu.ilba) + op->lba / 4;

    പൂർണ്ണസംഖ്യ എണ്ണം = op->എണ്ണം;
    op->എണ്ണം = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    എങ്കിൽ (op->lba & 3) {
        // Partial read of first block.
        dop.എണ്ണം = 1;
        dop.buf_fl = cdbuf_fl;
        പൂർണ്ണസംഖ്യ ret = process_op(&dop);
        എങ്കിൽ (ret)
            തിരികെ ret;
        u8 thiscount = 4 - (op->lba & 3);
        എങ്കിൽ (thiscount > എണ്ണം)
            thiscount = എണ്ണം;
        എണ്ണം -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->എണ്ണം += thiscount;
        dop.lba++;
    }

    എങ്കിൽ (എണ്ണം > 3) {
        // Read n number of regular blocks.
        dop.എണ്ണം = എണ്ണം / 4;
        dop.buf_fl = op->buf_fl;
        പൂർണ്ണസംഖ്യ ret = process_op(&dop);
        op->എണ്ണം += dop.എണ്ണം * 4;
        എങ്കിൽ (ret)
            തിരികെ ret;
        u8 thiscount = എണ്ണം & ~3;
        എണ്ണം &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    എങ്കിൽ (എണ്ണം) {
        // Partial read on last block.
        dop.എണ്ണം = 1;
        dop.buf_fl = cdbuf_fl;
        പൂർണ്ണസംഖ്യ ret = process_op(&dop);
        എങ്കിൽ (ret)
            തിരികെ ret;
        u8 thiscount = എണ്ണം;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->എണ്ണം += thiscount;
    }

    തിരികെ DISK_RET_SUCCESS;
}

പൂർണ്ണസംഖ്യ
cdemu_process_op(ഘടന disk_op_s *op)
{
    എങ്കിൽ (!CONFIG_CDROM_EMU)
        തിരികെ 0;

    തിരഞ്ഞെടുപ്പ് (op->command) {
    അവസ്ഥ CMD_READ:
        തിരികെ cdemu_read(op);
    അവസ്ഥ CMD_WRITE:
    അവസ്ഥ CMD_FORMAT:
        തിരികെ DISK_RET_EWRITEPROTECT;
    സ്ഥിരസ്ഥിതി:
        തിരികെ default_process_op(op);
    }
}

ശൂന്യം
cdrom_prepboot(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_CDROM_EMU)
        തിരികെ;
    എങ്കിൽ (!CDCount)
        തിരികെ;
    എങ്കിൽ (create_bounce_buf() < 0)
        തിരികെ;

    ഘടന drive_s *drive = malloc_fseg(വലുപ്പം(*drive));
    എങ്കിൽ (!drive) {
        warn_noalloc();
        തിരികെ;
    }
    cdemu_drive_gf = drive;
    memset(drive, 0, വലുപ്പം(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_SIZE;
    drive->sectors = (u64)-1;
}


/****************************************************************
 * CD booting
 ****************************************************************/

പൂർണ്ണസംഖ്യ
cdrom_boot(ഘടന drive_s *drive)
{
    ASSERT32FLAT();
    ഘടന disk_op_s dop;
    പൂർണ്ണസംഖ്യ cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, വലുപ്പം(dop));
    dop.drive_fl = drive;
    എങ്കിൽ (!dop.drive_fl || cdid < 0)
        തിരികെ 1;

    പൂർണ്ണസംഖ്യ ret = scsi_is_ready(&dop);
    എങ്കിൽ (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.എണ്ണം = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    എങ്കിൽ (ret)
        തിരികെ 3;

    // Validity checks
    എങ്കിൽ (buffer[0])
        തിരികെ 4;
    എങ്കിൽ (strcmp((അക്ഷരം*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        തിരികെ 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&buffer[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.എണ്ണം = 1;
    ret = process_op(&dop);
    എങ്കിൽ (ret)
        തിരികെ 7;

    // Validation entry
    എങ്കിൽ (buffer[0x00] != 0x01)
        തിരികെ 8;   // Header
    എങ്കിൽ (buffer[0x01] != 0x00)
        തിരികെ 9;   // Platform
    എങ്കിൽ (buffer[0x1E] != 0x55)
        തിരികെ 10;  // key 1
    എങ്കിൽ (buffer[0x1F] != 0xAA)
        തിരികെ 10;  // key 2

    // Initial/Default Entry
    എങ്കിൽ (buffer[0x20] != 0x88)
        തിരികെ 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_add_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), buffer), വലുപ്പം(buffer));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = buffer[0x21];

    u16 boot_segment = *(u16*)&buffer[0x22];
    എങ്കിൽ (!boot_segment)
        boot_segment = 0x07C0;
    CDEmu.load_segment = boot_segment;
    CDEmu.buffer_segment = 0x0000;

    u16 nbsectors = *(u16*)&buffer[0x26];
    CDEmu.sector_എണ്ണം = nbsectors;

    lba = *(u32*)&buffer[0x28];
    CDEmu.ilba = lba;

    CDEmu.controller_index = drive->cntl_id / 2;
    CDEmu.device_spec = drive->cntl_id % 2;

    // And we read the image in memory
    nbsectors = DIV_ROUND_UP(nbsectors, 4);
    dop.lba = lba;
    dop.buf_fl = MAKE_FLATPTR(boot_segment, 0);
    ആകുമ്പോൾ (nbsectors) {
        പൂർണ്ണസംഖ്യ എണ്ണം = nbsectors;
        എങ്കിൽ (എണ്ണം > 64*1024/CDROM_SECTOR_SIZE)
            എണ്ണം = 64*1024/CDROM_SECTOR_SIZE;
        dop.എണ്ണം = എണ്ണം;
        ret = process_op(&dop);
        എങ്കിൽ (ret)
            തിരികെ 12;
        nbsectors -= എണ്ണം;
        dop.lba += എണ്ണം;
        dop.buf_fl += എണ്ണം*CDROM_SECTOR_SIZE;
    }

    എങ്കിൽ (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        തിരികെ 0;
    }

    // Emulation of a floppy/harddisk requested
    എങ്കിൽ (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        തിരികെ 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    എങ്കിൽ (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        set_equipment_flags(0x41, 0x41);

        തിരഞ്ഞെടുപ്പ് (media) {
        അവസ്ഥ 0x01:  // 1.2M floppy
            CDEmu.chs.sptcyl = 15;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            നിർത്തുക;
        അവസ്ഥ 0x02:  // 1.44M floppy
            CDEmu.chs.sptcyl = 18;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            നിർത്തുക;
        അവസ്ഥ 0x03:  // 2.88M floppy
            CDEmu.chs.sptcyl = 36;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            നിർത്തുക;
        }
    } അല്ലെങ്കിൽ {
        // Harddrive emulation
        CDEmu.emulated_drive = 0x80;
        SET_BDA(hdcount, GET_BDA(hdcount) + 1);

        // Peak at partition table to get chs.
        ഘടന mbr_s *mbr = MAKE_FLATPTR(boot_segment, 0);
        CDEmu.chs = mbr->partitions[0].last;
    }

    // everything is ok, so from now on, the emulation is active
    CDEmu.media = media;
    dprintf(6, "cdemu media=%d\n", media);

    തിരികെ 0;
}

// check if media is present and the drive is bootable.
// in case it is return the volume label.
അക്ഷരം*
cdrom_media_info(ഘടന drive_s *drive)
{
    ASSERT32FLAT();

    ഘടന disk_op_s dop;
    memset(&dop, 0, വലുപ്പം(dop));
    dop.drive_fl = drive;

    പൂർണ്ണസംഖ്യ ret = scsi_is_ready(&dop);
    എങ്കിൽ (ret)
        തിരികെ ശൂന്യ;

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.എണ്ണം = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    എങ്കിൽ (ret)
        തിരികെ ശൂന്യ;

    // Is it bootable?
    എങ്കിൽ (buffer[0])
        തിരികെ ശൂന്യ;
    എങ്കിൽ (strcmp((അക്ഷരം*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        തിരികെ ശൂന്യ;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.എണ്ണം = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    എങ്കിൽ (ret)
        തിരികെ ശൂന്യ;

    // Read volume id, trim trailing spaces
    അക്ഷരം *volume = znprintf(30, "%s", buffer + 40);
    nullTrailingSpace(volume);
    തിരികെ volume;
}

// Support for booting from cdroms (the "El Torito" spec).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_GLOBAL
#समावेश "block.h" // struct drive_s
#समावेश "bregs.h" // struct bregs
#समावेश "hw/ata.h" // ATA_CMD_REQUEST_SENSE
#समावेश "hw/blockcmd.h" // CDB_CMD_REQUEST_SENSE
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "std/disk.h" // DISK_RET_SUCCESS
#समावेश "string.h" // memset
#समावेश "util.h" // cdrom_prepboot
#समावेश "tcgbios.h" // tpm_*


/****************************************************************
 * CD emulation
 ****************************************************************/

काष्ठा eltorito_s CDEmu VARLOW = { .आकार=माप(CDEmu) };
काष्ठा drive_s *emulated_drive_gf VARLOW;
काष्ठा drive_s *cdemu_drive_gf VARFSEG;

अटल पूर्णांक
cdemu_read(काष्ठा disk_op_s *op)
{
    काष्ठा drive_s *drive_gf = GET_निचला(emulated_drive_gf);
    काष्ठा disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_निचला(CDEmu.ilba) + op->lba / 4;

    पूर्णांक गिनती = op->गिनती;
    op->गिनती = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    अगर (op->lba & 3) {
        // Partial read of first block.
        dop.गिनती = 1;
        dop.buf_fl = cdbuf_fl;
        पूर्णांक ret = process_op(&dop);
        अगर (ret)
            वापस ret;
        u8 thiscount = 4 - (op->lba & 3);
        अगर (thiscount > गिनती)
            thiscount = गिनती;
        गिनती -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->गिनती += thiscount;
        dop.lba++;
    }

    अगर (गिनती > 3) {
        // Read n number of regular blocks.
        dop.गिनती = गिनती / 4;
        dop.buf_fl = op->buf_fl;
        पूर्णांक ret = process_op(&dop);
        op->गिनती += dop.गिनती * 4;
        अगर (ret)
            वापस ret;
        u8 thiscount = गिनती & ~3;
        गिनती &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    अगर (गिनती) {
        // Partial read on last block.
        dop.गिनती = 1;
        dop.buf_fl = cdbuf_fl;
        पूर्णांक ret = process_op(&dop);
        अगर (ret)
            वापस ret;
        u8 thiscount = गिनती;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->गिनती += thiscount;
    }

    वापस DISK_RET_SUCCESS;
}

पूर्णांक
cdemu_process_op(काष्ठा disk_op_s *op)
{
    अगर (!CONFIG_CDROM_EMU)
        वापस 0;

    चयन (op->command) {
    हाल CMD_READ:
        वापस cdemu_read(op);
    हाल CMD_WRITE:
    हाल CMD_FORMAT:
        वापस DISK_RET_EWRITEPROTECT;
    शेष:
        वापस default_process_op(op);
    }
}

व्योम
cdrom_prepboot(व्योम)
{
    अगर (!CONFIG_CDROM_EMU)
        वापस;
    अगर (!CDCount)
        वापस;
    अगर (create_bounce_buf() < 0)
        वापस;

    काष्ठा drive_s *drive = malloc_fseg(माप(*drive));
    अगर (!drive) {
        warn_noalloc();
        वापस;
    }
    cdemu_drive_gf = drive;
    स_रखो(drive, 0, माप(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_आकार;
    drive->sectors = (u64)-1;
}


/****************************************************************
 * CD booting
 ****************************************************************/

पूर्णांक
cdrom_boot(काष्ठा drive_s *drive)
{
    ASSERT32FLAT();
    काष्ठा disk_op_s dop;
    पूर्णांक cdid = getDriveId(EXTTYPE_CD, drive);
    स_रखो(&dop, 0, माप(dop));
    dop.drive_fl = drive;
    अगर (!dop.drive_fl || cdid < 0)
        वापस 1;

    पूर्णांक ret = scsi_is_ready(&dop);
    अगर (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 बफर[CDROM_SECTOR_आकार];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.गिनती = 1;
    dop.buf_fl = बफर;
    ret = process_op(&dop);
    अगर (ret)
        वापस 3;

    // Validity checks
    अगर (बफर[0])
        वापस 4;
    अगर (म_भेद((अक्षर*)&बफर[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        वापस 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&बफर[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.गिनती = 1;
    ret = process_op(&dop);
    अगर (ret)
        वापस 7;

    // Validation entry
    अगर (बफर[0x00] != 0x01)
        वापस 8;   // Header
    अगर (बफर[0x01] != 0x00)
        वापस 9;   // Platform
    अगर (बफर[0x1E] != 0x55)
        वापस 10;  // key 1
    अगर (बफर[0x1F] != 0xAA)
        वापस 10;  // key 2

    // Initial/Default Entry
    अगर (बफर[0x20] != 0x88)
        वापस 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_add_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), बफर), माप(बफर));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = बफर[0x21];

    u16 boot_segment = *(u16*)&बफर[0x22];
    अगर (!boot_segment)
        boot_segment = 0x07C0;
    CDEmu.load_segment = boot_segment;
    CDEmu.बफर_segment = 0x0000;

    u16 nbsectors = *(u16*)&बफर[0x26];
    CDEmu.sector_गिनती = nbsectors;

    lba = *(u32*)&बफर[0x28];
    CDEmu.ilba = lba;

    CDEmu.controller_सूचकांक = drive->cntl_id / 2;
    CDEmu.device_spec = drive->cntl_id % 2;

    // And we read the image in memory
    nbsectors = DIV_ROUND_UP(nbsectors, 4);
    dop.lba = lba;
    dop.buf_fl = MAKE_FLATPTR(boot_segment, 0);
    जबतक (nbsectors) {
        पूर्णांक गिनती = nbsectors;
        अगर (गिनती > 64*1024/CDROM_SECTOR_आकार)
            गिनती = 64*1024/CDROM_SECTOR_आकार;
        dop.गिनती = गिनती;
        ret = process_op(&dop);
        अगर (ret)
            वापस 12;
        nbsectors -= गिनती;
        dop.lba += गिनती;
        dop.buf_fl += गिनती*CDROM_SECTOR_आकार;
    }

    अगर (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        वापस 0;
    }

    // Emulation of a floppy/harddisk requested
    अगर (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        वापस 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    अगर (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        set_equipment_flags(0x41, 0x41);

        चयन (media) {
        हाल 0x01:  // 1.2M floppy
            CDEmu.chs.sptcyl = 15;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            अवरोध;
        हाल 0x02:  // 1.44M floppy
            CDEmu.chs.sptcyl = 18;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            अवरोध;
        हाल 0x03:  // 2.88M floppy
            CDEmu.chs.sptcyl = 36;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            अवरोध;
        }
    } अन्यथा {
        // Harddrive emulation
        CDEmu.emulated_drive = 0x80;
        SET_BDA(hdcount, GET_BDA(hdcount) + 1);

        // Peak at partition table to get chs.
        काष्ठा mbr_s *mbr = MAKE_FLATPTR(boot_segment, 0);
        CDEmu.chs = mbr->partitions[0].last;
    }

    // everything is ok, so from now on, the emulation is active
    CDEmu.media = media;
    dprintf(6, "cdemu media=%d\n", media);

    वापस 0;
}

// check if media is present and the drive is bootable.
// in case it is return the volume label.
अक्षर*
cdrom_media_info(काष्ठा drive_s *drive)
{
    ASSERT32FLAT();

    काष्ठा disk_op_s dop;
    स_रखो(&dop, 0, माप(dop));
    dop.drive_fl = drive;

    पूर्णांक ret = scsi_is_ready(&dop);
    अगर (ret)
        वापस शून्य;

    // Read the Boot Record Volume Descriptor
    u8 बफर[CDROM_SECTOR_आकार];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.गिनती = 1;
    dop.buf_fl = बफर;
    ret = process_op(&dop);
    अगर (ret)
        वापस शून्य;

    // Is it bootable?
    अगर (बफर[0])
        वापस शून्य;
    अगर (म_भेद((अक्षर*)&बफर[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        वापस शून्य;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.गिनती = 1;
    dop.buf_fl = बफर;
    ret = process_op(&dop);
    अगर (ret)
        वापस शून्य;

    // Read volume id, trim trailing spaces
    अक्षर *volume = znprintf(30, "%s", बफर + 40);
    nullTrailingSpace(volume);
    वापस volume;
}

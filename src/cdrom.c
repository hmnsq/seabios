// Support for booting from cdroms (the "El Torito" spec).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // GET_GLOBAL
#அந்தர்பாவ "block.h" // struct drive_s
#அந்தர்பாவ "bregs.h" // struct bregs
#அந்தர்பாவ "hw/ata.h" // ATA_CMD_REQUEST_SENSE
#அந்தர்பாவ "hw/blockcmd.h" // CDB_CMD_REQUEST_SENSE
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "std/disk.h" // DISK_RET_SUCCESS
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // cdrom_prepboot
#அந்தர்பாவ "tcgbios.h" // tpm_*


/****************************************************************
 * CD emulation
 ****************************************************************/

கட்டமைப்பு eltorito_s CDEmu VARLOW = { .size=அளவு(CDEmu) };
கட்டமைப்பு drive_s *emulated_drive_gf VARLOW;
கட்டமைப்பு drive_s *cdemu_drive_gf VARFSEG;

நிலையான முழு
cdemu_read(கட்டமைப்பு disk_op_s *op)
{
    கட்டமைப்பு drive_s *drive_gf = GET_கீழ்(emulated_drive_gf);
    கட்டமைப்பு disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_கீழ்(CDEmu.ilba) + op->lba / 4;

    முழு மொத்தம் = op->மொத்தம்;
    op->மொத்தம் = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    எனில் (op->lba & 3) {
        // Partial read of first block.
        dop.மொத்தம் = 1;
        dop.buf_fl = cdbuf_fl;
        முழு ret = process_op(&dop);
        எனில் (ret)
            திரும்பு ret;
        u8 thiscount = 4 - (op->lba & 3);
        எனில் (thiscount > மொத்தம்)
            thiscount = மொத்தம்;
        மொத்தம் -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->மொத்தம் += thiscount;
        dop.lba++;
    }

    எனில் (மொத்தம் > 3) {
        // Read n number of regular blocks.
        dop.மொத்தம் = மொத்தம் / 4;
        dop.buf_fl = op->buf_fl;
        முழு ret = process_op(&dop);
        op->மொத்தம் += dop.மொத்தம் * 4;
        எனில் (ret)
            திரும்பு ret;
        u8 thiscount = மொத்தம் & ~3;
        மொத்தம் &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    எனில் (மொத்தம்) {
        // Partial read on last block.
        dop.மொத்தம் = 1;
        dop.buf_fl = cdbuf_fl;
        முழு ret = process_op(&dop);
        எனில் (ret)
            திரும்பு ret;
        u8 thiscount = மொத்தம்;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->மொத்தம் += thiscount;
    }

    திரும்பு DISK_RET_SUCCESS;
}

முழு
cdemu_process_op(கட்டமைப்பு disk_op_s *op)
{
    எனில் (!CONFIG_CDROM_EMU)
        திரும்பு 0;

    தேர்வு (op->command) {
    நிலை CMD_READ:
        திரும்பு cdemu_read(op);
    நிலை CMD_WRITE:
    நிலை CMD_FORMAT:
        திரும்பு DISK_RET_EWRITEPROTECT;
    இயல்பு:
        திரும்பு default_process_op(op);
    }
}

வெறுமை
cdrom_prepboot(வெறுமை)
{
    எனில் (!CONFIG_CDROM_EMU)
        திரும்பு;
    எனில் (!CDCount)
        திரும்பு;
    எனில் (create_bounce_buf() < 0)
        திரும்பு;

    கட்டமைப்பு drive_s *drive = malloc_fseg(அளவு(*drive));
    எனில் (!drive) {
        warn_noalloc();
        திரும்பு;
    }
    cdemu_drive_gf = drive;
    memset(drive, 0, அளவு(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_SIZE;
    drive->sectors = (u64)-1;
}


/****************************************************************
 * CD booting
 ****************************************************************/

முழு
cdrom_boot(கட்டமைப்பு drive_s *drive)
{
    ASSERT32FLAT();
    கட்டமைப்பு disk_op_s dop;
    முழு cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, அளவு(dop));
    dop.drive_fl = drive;
    எனில் (!dop.drive_fl || cdid < 0)
        திரும்பு 1;

    முழு ret = scsi_is_ready(&dop);
    எனில் (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.மொத்தம் = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    எனில் (ret)
        திரும்பு 3;

    // Validity checks
    எனில் (buffer[0])
        திரும்பு 4;
    எனில் (strcmp((எழுத்து*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        திரும்பு 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&buffer[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.மொத்தம் = 1;
    ret = process_op(&dop);
    எனில் (ret)
        திரும்பு 7;

    // Validation entry
    எனில் (buffer[0x00] != 0x01)
        திரும்பு 8;   // Header
    எனில் (buffer[0x01] != 0x00)
        திரும்பு 9;   // Platform
    எனில் (buffer[0x1E] != 0x55)
        திரும்பு 10;  // key 1
    எனில் (buffer[0x1F] != 0xAA)
        திரும்பு 10;  // key 2

    // Initial/Default Entry
    எனில் (buffer[0x20] != 0x88)
        திரும்பு 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_add_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), buffer), அளவு(buffer));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = buffer[0x21];

    u16 boot_segment = *(u16*)&buffer[0x22];
    எனில் (!boot_segment)
        boot_segment = 0x07C0;
    CDEmu.load_segment = boot_segment;
    CDEmu.buffer_segment = 0x0000;

    u16 nbsectors = *(u16*)&buffer[0x26];
    CDEmu.sector_மொத்தம் = nbsectors;

    lba = *(u32*)&buffer[0x28];
    CDEmu.ilba = lba;

    CDEmu.controller_index = drive->cntl_id / 2;
    CDEmu.device_spec = drive->cntl_id % 2;

    // And we read the image in memory
    nbsectors = DIV_ROUND_UP(nbsectors, 4);
    dop.lba = lba;
    dop.buf_fl = MAKE_FLATPTR(boot_segment, 0);
    போது (nbsectors) {
        முழு மொத்தம் = nbsectors;
        எனில் (மொத்தம் > 64*1024/CDROM_SECTOR_SIZE)
            மொத்தம் = 64*1024/CDROM_SECTOR_SIZE;
        dop.மொத்தம் = மொத்தம்;
        ret = process_op(&dop);
        எனில் (ret)
            திரும்பு 12;
        nbsectors -= மொத்தம்;
        dop.lba += மொத்தம்;
        dop.buf_fl += மொத்தம்*CDROM_SECTOR_SIZE;
    }

    எனில் (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        திரும்பு 0;
    }

    // Emulation of a floppy/harddisk requested
    எனில் (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        திரும்பு 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    எனில் (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        set_equipment_flags(0x41, 0x41);

        தேர்வு (media) {
        நிலை 0x01:  // 1.2M floppy
            CDEmu.chs.sptcyl = 15;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            நிறுத்து;
        நிலை 0x02:  // 1.44M floppy
            CDEmu.chs.sptcyl = 18;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            நிறுத்து;
        நிலை 0x03:  // 2.88M floppy
            CDEmu.chs.sptcyl = 36;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            நிறுத்து;
        }
    } இல்லையெனில் {
        // Harddrive emulation
        CDEmu.emulated_drive = 0x80;
        SET_BDA(hdcount, GET_BDA(hdcount) + 1);

        // Peak at partition table to get chs.
        கட்டமைப்பு mbr_s *mbr = MAKE_FLATPTR(boot_segment, 0);
        CDEmu.chs = mbr->partitions[0].last;
    }

    // everything is ok, so from now on, the emulation is active
    CDEmu.media = media;
    dprintf(6, "cdemu media=%d\n", media);

    திரும்பு 0;
}

// check if media is present and the drive is bootable.
// in case it is return the volume label.
எழுத்து*
cdrom_media_info(கட்டமைப்பு drive_s *drive)
{
    ASSERT32FLAT();

    கட்டமைப்பு disk_op_s dop;
    memset(&dop, 0, அளவு(dop));
    dop.drive_fl = drive;

    முழு ret = scsi_is_ready(&dop);
    எனில் (ret)
        திரும்பு வெற்று;

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.மொத்தம் = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    எனில் (ret)
        திரும்பு வெற்று;

    // Is it bootable?
    எனில் (buffer[0])
        திரும்பு வெற்று;
    எனில் (strcmp((எழுத்து*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        திரும்பு வெற்று;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.மொத்தம் = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    எனில் (ret)
        திரும்பு வெற்று;

    // Read volume id, trim trailing spaces
    எழுத்து *volume = znprintf(30, "%s", buffer + 40);
    nullTrailingSpace(volume);
    திரும்பு volume;
}

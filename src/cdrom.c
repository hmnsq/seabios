// Support for booting from cdroms (the "El Torito" spec).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // GET_GLOBAL
#અન્તર્ભાવ "block.h" // struct drive_s
#અન્તર્ભાવ "bregs.h" // struct bregs
#અન્તર્ભાવ "hw/ata.h" // ATA_CMD_REQUEST_SENSE
#અન્તર્ભાવ "hw/blockcmd.h" // CDB_CMD_REQUEST_SENSE
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "std/disk.h" // DISK_RET_SUCCESS
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // cdrom_prepboot
#અન્તર્ભાવ "tcgbios.h" // tpm_*


/****************************************************************
 * CD emulation
 ****************************************************************/

સંરચના eltorito_s CDEmu VARLOW = { .size=કદ(CDEmu) };
સંરચના drive_s *emulated_drive_gf VARLOW;
સંરચના drive_s *cdemu_drive_gf VARFSEG;

સ્થિર પૂર્ણાંક
cdemu_read(સંરચના disk_op_s *op)
{
    સંરચના drive_s *drive_gf = GET_નીચલું(emulated_drive_gf);
    સંરચના disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_નીચલું(CDEmu.ilba) + op->lba / 4;

    પૂર્ણાંક ગણતરી = op->ગણતરી;
    op->ગણતરી = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    જો (op->lba & 3) {
        // Partial read of first block.
        dop.ગણતરી = 1;
        dop.buf_fl = cdbuf_fl;
        પૂર્ણાંક ret = process_op(&dop);
        જો (ret)
            પાછા ret;
        u8 thiscount = 4 - (op->lba & 3);
        જો (thiscount > ગણતરી)
            thiscount = ગણતરી;
        ગણતરી -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->ગણતરી += thiscount;
        dop.lba++;
    }

    જો (ગણતરી > 3) {
        // Read n number of regular blocks.
        dop.ગણતરી = ગણતરી / 4;
        dop.buf_fl = op->buf_fl;
        પૂર્ણાંક ret = process_op(&dop);
        op->ગણતરી += dop.ગણતરી * 4;
        જો (ret)
            પાછા ret;
        u8 thiscount = ગણતરી & ~3;
        ગણતરી &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    જો (ગણતરી) {
        // Partial read on last block.
        dop.ગણતરી = 1;
        dop.buf_fl = cdbuf_fl;
        પૂર્ણાંક ret = process_op(&dop);
        જો (ret)
            પાછા ret;
        u8 thiscount = ગણતરી;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->ગણતરી += thiscount;
    }

    પાછા DISK_RET_SUCCESS;
}

પૂર્ણાંક
cdemu_process_op(સંરચના disk_op_s *op)
{
    જો (!CONFIG_CDROM_EMU)
        પાછા 0;

    પસંદગી (op->command) {
    સ્થિતિ CMD_READ:
        પાછા cdemu_read(op);
    સ્થિતિ CMD_WRITE:
    સ્થિતિ CMD_FORMAT:
        પાછા DISK_RET_EWRITEPROTECT;
    મૂળભૂત:
        પાછા default_process_op(op);
    }
}

શૂન્ય
cdrom_prepboot(શૂન્ય)
{
    જો (!CONFIG_CDROM_EMU)
        પાછા;
    જો (!CDCount)
        પાછા;
    જો (create_bounce_buf() < 0)
        પાછા;

    સંરચના drive_s *drive = malloc_fseg(કદ(*drive));
    જો (!drive) {
        warn_noalloc();
        પાછા;
    }
    cdemu_drive_gf = drive;
    memset(drive, 0, કદ(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_SIZE;
    drive->sectors = (u64)-1;
}


/****************************************************************
 * CD booting
 ****************************************************************/

પૂર્ણાંક
cdrom_boot(સંરચના drive_s *drive)
{
    ASSERT32FLAT();
    સંરચના disk_op_s dop;
    પૂર્ણાંક cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, કદ(dop));
    dop.drive_fl = drive;
    જો (!dop.drive_fl || cdid < 0)
        પાછા 1;

    પૂર્ણાંક ret = scsi_is_ready(&dop);
    જો (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.ગણતરી = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    જો (ret)
        પાછા 3;

    // Validity checks
    જો (buffer[0])
        પાછા 4;
    જો (strcmp((અક્ષર*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        પાછા 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&buffer[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.ગણતરી = 1;
    ret = process_op(&dop);
    જો (ret)
        પાછા 7;

    // Validation entry
    જો (buffer[0x00] != 0x01)
        પાછા 8;   // Header
    જો (buffer[0x01] != 0x00)
        પાછા 9;   // Platform
    જો (buffer[0x1E] != 0x55)
        પાછા 10;  // key 1
    જો (buffer[0x1F] != 0xAA)
        પાછા 10;  // key 2

    // Initial/Default Entry
    જો (buffer[0x20] != 0x88)
        પાછા 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_add_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), buffer), કદ(buffer));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = buffer[0x21];

    u16 boot_segment = *(u16*)&buffer[0x22];
    જો (!boot_segment)
        boot_segment = 0x07C0;
    CDEmu.load_segment = boot_segment;
    CDEmu.buffer_segment = 0x0000;

    u16 nbsectors = *(u16*)&buffer[0x26];
    CDEmu.sector_ગણતરી = nbsectors;

    lba = *(u32*)&buffer[0x28];
    CDEmu.ilba = lba;

    CDEmu.controller_index = drive->cntl_id / 2;
    CDEmu.device_spec = drive->cntl_id % 2;

    // And we read the image in memory
    nbsectors = DIV_ROUND_UP(nbsectors, 4);
    dop.lba = lba;
    dop.buf_fl = MAKE_FLATPTR(boot_segment, 0);
    જ્યારે (nbsectors) {
        પૂર્ણાંક ગણતરી = nbsectors;
        જો (ગણતરી > 64*1024/CDROM_SECTOR_SIZE)
            ગણતરી = 64*1024/CDROM_SECTOR_SIZE;
        dop.ગણતરી = ગણતરી;
        ret = process_op(&dop);
        જો (ret)
            પાછા 12;
        nbsectors -= ગણતરી;
        dop.lba += ગણતરી;
        dop.buf_fl += ગણતરી*CDROM_SECTOR_SIZE;
    }

    જો (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        પાછા 0;
    }

    // Emulation of a floppy/harddisk requested
    જો (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        પાછા 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    જો (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        set_equipment_flags(0x41, 0x41);

        પસંદગી (media) {
        સ્થિતિ 0x01:  // 1.2M floppy
            CDEmu.chs.sptcyl = 15;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            તોડો;
        સ્થિતિ 0x02:  // 1.44M floppy
            CDEmu.chs.sptcyl = 18;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            તોડો;
        સ્થિતિ 0x03:  // 2.88M floppy
            CDEmu.chs.sptcyl = 36;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            તોડો;
        }
    } નહીંતર {
        // Harddrive emulation
        CDEmu.emulated_drive = 0x80;
        SET_BDA(hdcount, GET_BDA(hdcount) + 1);

        // Peak at partition table to get chs.
        સંરચના mbr_s *mbr = MAKE_FLATPTR(boot_segment, 0);
        CDEmu.chs = mbr->partitions[0].last;
    }

    // everything is ok, so from now on, the emulation is active
    CDEmu.media = media;
    dprintf(6, "cdemu media=%d\n", media);

    પાછા 0;
}

// check if media is present and the drive is bootable.
// in case it is return the volume label.
અક્ષર*
cdrom_media_info(સંરચના drive_s *drive)
{
    ASSERT32FLAT();

    સંરચના disk_op_s dop;
    memset(&dop, 0, કદ(dop));
    dop.drive_fl = drive;

    પૂર્ણાંક ret = scsi_is_ready(&dop);
    જો (ret)
        પાછા ખાલી;

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.ગણતરી = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    જો (ret)
        પાછા ખાલી;

    // Is it bootable?
    જો (buffer[0])
        પાછા ખાલી;
    જો (strcmp((અક્ષર*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        પાછા ખાલી;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.ગણતરી = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    જો (ret)
        પાછા ખાલી;

    // Read volume id, trim trailing spaces
    અક્ષર *volume = znprintf(30, "%s", buffer + 40);
    nullTrailingSpace(volume);
    પાછા volume;
}

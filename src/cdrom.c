// Support for booting from cdroms (the "El Torito" spec).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_GLOBAL
#ଅନ୍ତର୍ଭାଵ "block.h" // struct drive_s
#ଅନ୍ତର୍ଭାଵ "bregs.h" // struct bregs
#ଅନ୍ତର୍ଭାଵ "hw/ata.h" // ATA_CMD_REQUEST_SENSE
#ଅନ୍ତର୍ଭାଵ "hw/blockcmd.h" // CDB_CMD_REQUEST_SENSE
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "std/disk.h" // DISK_RET_SUCCESS
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // cdrom_prepboot
#ଅନ୍ତର୍ଭାଵ "tcgbios.h" // tpm_*


/****************************************************************
 * CD emulation
 ****************************************************************/

ସଂରଚନା eltorito_s CDEmu VARLOW = { .size=ଆକାର(CDEmu) };
ସଂରଚନା drive_s *emulated_drive_gf VARLOW;
ସଂରଚନା drive_s *cdemu_drive_gf VARFSEG;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
cdemu_read(ସଂରଚନା disk_op_s *op)
{
    ସଂରଚନା drive_s *drive_gf = GET_ତଳ(emulated_drive_gf);
    ସଂରଚନା disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_ତଳ(CDEmu.ilba) + op->lba / 4;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = op->ଗଣନା_2;
    op->ଗଣନା_2 = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    ଯଦି (op->lba & 3) {
        // Partial read of first block.
        dop.ଗଣନା_2 = 1;
        dop.buf_fl = cdbuf_fl;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = process_op(&dop);
        ଯଦି (ret)
            ଫେରନ୍ତୁ ret;
        u8 thiscount = 4 - (op->lba & 3);
        ଯଦି (thiscount > ଗଣନା_2)
            thiscount = ଗଣନା_2;
        ଗଣନା_2 -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->ଗଣନା_2 += thiscount;
        dop.lba++;
    }

    ଯଦି (ଗଣନା_2 > 3) {
        // Read n number of regular blocks.
        dop.ଗଣନା_2 = ଗଣନା_2 / 4;
        dop.buf_fl = op->buf_fl;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = process_op(&dop);
        op->ଗଣନା_2 += dop.ଗଣନା_2 * 4;
        ଯଦି (ret)
            ଫେରନ୍ତୁ ret;
        u8 thiscount = ଗଣନା_2 & ~3;
        ଗଣନା_2 &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    ଯଦି (ଗଣନା_2) {
        // Partial read on last block.
        dop.ଗଣନା_2 = 1;
        dop.buf_fl = cdbuf_fl;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = process_op(&dop);
        ଯଦି (ret)
            ଫେରନ୍ତୁ ret;
        u8 thiscount = ଗଣନା_2;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->ଗଣନା_2 += thiscount;
    }

    ଫେରନ୍ତୁ DISK_RET_SUCCESS;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
cdemu_process_op(ସଂରଚନା disk_op_s *op)
{
    ଯଦି (!CONFIG_CDROM_EMU)
        ଫେରନ୍ତୁ 0;

    ବାଛ (op->command) {
    ସ୍ଥିତି CMD_READ:
        ଫେରନ୍ତୁ cdemu_read(op);
    ସ୍ଥିତି CMD_WRITE:
    ସ୍ଥିତି CMD_FORMAT:
        ଫେରନ୍ତୁ DISK_RET_EWRITEPROTECT;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ଫେରନ୍ତୁ default_process_op(op);
    }
}

ଶୂନ୍ୟ
cdrom_prepboot(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_CDROM_EMU)
        ଫେରନ୍ତୁ;
    ଯଦି (!CDCount)
        ଫେରନ୍ତୁ;
    ଯଦି (create_bounce_buf() < 0)
        ଫେରନ୍ତୁ;

    ସଂରଚନା drive_s *drive = malloc_fseg(ଆକାର(*drive));
    ଯଦି (!drive) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    cdemu_drive_gf = drive;
    memset(drive, 0, ଆକାର(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_SIZE;
    drive->sectors = (u64)-1;
}


/****************************************************************
 * CD booting
 ****************************************************************/

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
cdrom_boot(ସଂରଚନା drive_s *drive)
{
    ASSERT32FLAT();
    ସଂରଚନା disk_op_s dop;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, ଆକାର(dop));
    dop.drive_fl = drive;
    ଯଦି (!dop.drive_fl || cdid < 0)
        ଫେରନ୍ତୁ 1;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = scsi_is_ready(&dop);
    ଯଦି (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.ଗଣନା_2 = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    ଯଦି (ret)
        ଫେରନ୍ତୁ 3;

    // Validity checks
    ଯଦି (buffer[0])
        ଫେରନ୍ତୁ 4;
    ଯଦି (strcmp((ଅକ୍ଷର*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ଫେରନ୍ତୁ 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&buffer[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.ଗଣନା_2 = 1;
    ret = process_op(&dop);
    ଯଦି (ret)
        ଫେରନ୍ତୁ 7;

    // Validation entry
    ଯଦି (buffer[0x00] != 0x01)
        ଫେରନ୍ତୁ 8;   // Header
    ଯଦି (buffer[0x01] != 0x00)
        ଫେରନ୍ତୁ 9;   // Platform
    ଯଦି (buffer[0x1E] != 0x55)
        ଫେରନ୍ତୁ 10;  // key 1
    ଯଦି (buffer[0x1F] != 0xAA)
        ଫେରନ୍ତୁ 10;  // key 2

    // Initial/Default Entry
    ଯଦି (buffer[0x20] != 0x88)
        ଫେରନ୍ତୁ 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_add_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), buffer), ଆକାର(buffer));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = buffer[0x21];

    u16 boot_segment = *(u16*)&buffer[0x22];
    ଯଦି (!boot_segment)
        boot_segment = 0x07C0;
    CDEmu.load_segment = boot_segment;
    CDEmu.buffer_segment = 0x0000;

    u16 nbsectors = *(u16*)&buffer[0x26];
    CDEmu.sector_ଗଣନା = nbsectors;

    lba = *(u32*)&buffer[0x28];
    CDEmu.ilba = lba;

    CDEmu.controller_index = drive->cntl_id / 2;
    CDEmu.device_spec = drive->cntl_id % 2;

    // And we read the image in memory
    nbsectors = DIV_ROUND_UP(nbsectors, 4);
    dop.lba = lba;
    dop.buf_fl = MAKE_FLATPTR(boot_segment, 0);
    ଯେତେବେଳେ (nbsectors) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = nbsectors;
        ଯଦି (ଗଣନା_2 > 64*1024/CDROM_SECTOR_SIZE)
            ଗଣନା_2 = 64*1024/CDROM_SECTOR_SIZE;
        dop.ଗଣନା_2 = ଗଣନା_2;
        ret = process_op(&dop);
        ଯଦି (ret)
            ଫେରନ୍ତୁ 12;
        nbsectors -= ଗଣନା_2;
        dop.lba += ଗଣନା_2;
        dop.buf_fl += ଗଣନା_2*CDROM_SECTOR_SIZE;
    }

    ଯଦି (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        ଫେରନ୍ତୁ 0;
    }

    // Emulation of a floppy/harddisk requested
    ଯଦି (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        ଫେରନ୍ତୁ 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    ଯଦି (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        set_equipment_flags(0x41, 0x41);

        ବାଛ (media) {
        ସ୍ଥିତି 0x01:  // 1.2M floppy
            CDEmu.chs.sptcyl = 15;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 0x02:  // 1.44M floppy
            CDEmu.chs.sptcyl = 18;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 0x03:  // 2.88M floppy
            CDEmu.chs.sptcyl = 36;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            ଭାଙ୍ଗ;
        }
    } ଅନ୍ୟଥା {
        // Harddrive emulation
        CDEmu.emulated_drive = 0x80;
        SET_BDA(hdcount, GET_BDA(hdcount) + 1);

        // Peak at partition table to get chs.
        ସଂରଚନା mbr_s *mbr = MAKE_FLATPTR(boot_segment, 0);
        CDEmu.chs = mbr->partitions[0].last;
    }

    // everything is ok, so from now on, the emulation is active
    CDEmu.media = media;
    dprintf(6, "cdemu media=%d\n", media);

    ଫେରନ୍ତୁ 0;
}

// check if media is present and the drive is bootable.
// in case it is return the volume label.
ଅକ୍ଷର*
cdrom_media_info(ସଂରଚନା drive_s *drive)
{
    ASSERT32FLAT();

    ସଂରଚନା disk_op_s dop;
    memset(&dop, 0, ଆକାର(dop));
    dop.drive_fl = drive;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = scsi_is_ready(&dop);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ଖାଲି;

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.ଗଣନା_2 = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ଖାଲି;

    // Is it bootable?
    ଯଦି (buffer[0])
        ଫେରନ୍ତୁ ଖାଲି;
    ଯଦି (strcmp((ଅକ୍ଷର*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ଫେରନ୍ତୁ ଖାଲି;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.ଗଣନା_2 = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ଖାଲି;

    // Read volume id, trim trailing spaces
    ଅକ୍ଷର *volume = znprintf(30, "%s", buffer + 40);
    nullTrailingSpace(volume);
    ଫେରନ୍ତୁ volume;
}

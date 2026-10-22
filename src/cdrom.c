// Support for booting from cdroms (the "El Torito" spec).
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // GET_GLOBAL
#include "block.h" // struct drive_s
#include "bregs.h" // struct bregs
#include "hw/ata.h" // ATA_CMD_REQUEST_SENSE
#include "hw/blockcmd.h" // CDB_CMD_REQUEST_SENSE
#include "malloc.h" // free
#include "output.h" // dprintf
#include "std/disk.h" // DISK_RET_SUCCESS
#include "string.h" // memset
#include "util.h" // cdrom_prepboot
#include "tcgbios.h" // tpm_*


/****************************************************************
 * CD emulation
 ****************************************************************/

sa_nracanaa eltorito_s CDEmu VARLOW = { .size=parimaa_na(CDEmu) };
sa_nracanaa drive_s *emulated_drive_gf VARLOW;
sa_nracanaa drive_s *cdemu_drive_gf VARFSEG;

static පූර්ණ_සංඛ්‍යා
cdemu_read(sa_nracanaa disk_op_s *op)
{
    sa_nracanaa drive_s *drive_gf = GET_LOW(emulated_drive_gf);
    sa_nracanaa disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_LOW(CDEmu.ilba) + op->lba / 4;

    පූර්ණ_සංඛ්‍යා count = op->count;
    op->count = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    නම් (op->lba & 3) {
        // Partial read of first block.
        dop.count = 1;
        dop.buf_fl = cdbuf_fl;
        පූර්ණ_සංඛ්‍යා ret = process_op(&dop);
        නම් (ret)
            ආපසු ret;
        u8 thiscount = 4 - (op->lba & 3);
        නම් (thiscount > count)
            thiscount = count;
        count -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->count += thiscount;
        dop.lba++;
    }

    නම් (count > 3) {
        // Read n number of regular blocks.
        dop.count = count / 4;
        dop.buf_fl = op->buf_fl;
        පූර්ණ_සංඛ්‍යා ret = process_op(&dop);
        op->count += dop.count * 4;
        නම් (ret)
            ආපසු ret;
        u8 thiscount = count & ~3;
        count &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    නම් (count) {
        // Partial read on last block.
        dop.count = 1;
        dop.buf_fl = cdbuf_fl;
        පූර්ණ_සංඛ්‍යා ret = process_op(&dop);
        නම් (ret)
            ආපසු ret;
        u8 thiscount = count;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->count += thiscount;
    }

    ආපසු DISK_RET_SUCCESS;
}

පූර්ණ_සංඛ්‍යා
cdemu_process_op(sa_nracanaa disk_op_s *op)
{
    නම් (!CONFIG_CDROM_EMU)
        ආපසු 0;

    switch (op->command) {
    pak_sa CMD_READ:
        ආපසු cdemu_read(op);
    pak_sa CMD_WRITE:
    pak_sa CMD_FORMAT:
        ආපසු DISK_RET_EWRITEPROTECT;
    default:
        ආපසු default_process_op(op);
    }
}

void
cdrom_prepboot(void)
{
    නම් (!CONFIG_CDROM_EMU)
        ආපසු;
    නම් (!CDCount)
        ආපසු;
    නම් (create_bounce_buf() < 0)
        ආපසු;

    sa_nracanaa drive_s *drive = malloc_fseg(parimaa_na(*drive));
    නම් (!drive) {
        warn_noalloc();
        ආපසු;
    }
    cdemu_drive_gf = drive;
    memset(drive, 0, parimaa_na(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_SIZE;
    drive->sectors = (u64)-1;
}


/****************************************************************
 * CD booting
 ****************************************************************/

පූර්ණ_සංඛ්‍යා
cdrom_boot(sa_nracanaa drive_s *drive)
{
    ASSERT32FLAT();
    sa_nracanaa disk_op_s dop;
    පූර්ණ_සංඛ්‍යා cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, parimaa_na(dop));
    dop.drive_fl = drive;
    නම් (!dop.drive_fl || cdid < 0)
        ආපසු 1;

    පූර්ණ_සංඛ්‍යා ret = scsi_is_ready(&dop);
    නම් (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.count = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    නම් (ret)
        ආපසු 3;

    // Validity checks
    නම් (buffer[0])
        ආපසු 4;
    නම් (strcmp((var_na*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ආපසු 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&buffer[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.count = 1;
    ret = process_op(&dop);
    නම් (ret)
        ආපසු 7;

    // Validation entry
    නම් (buffer[0x00] != 0x01)
        ආපසු 8;   // Header
    නම් (buffer[0x01] != 0x00)
        ආපසු 9;   // Platform
    නම් (buffer[0x1E] != 0x55)
        ආපසු 10;  // key 1
    නම් (buffer[0x1F] != 0xAA)
        ආපසු 10;  // key 2

    // Initial/Default Entry
    නම් (buffer[0x20] != 0x88)
        ආපසු 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_add_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), buffer), parimaa_na(buffer));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = buffer[0x21];

    u16 boot_segment = *(u16*)&buffer[0x22];
    නම් (!boot_segment)
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
    අතරතුර (nbsectors) {
        පූර්ණ_සංඛ්‍යා count = nbsectors;
        නම් (count > 64*1024/CDROM_SECTOR_SIZE)
            count = 64*1024/CDROM_SECTOR_SIZE;
        dop.count = count;
        ret = process_op(&dop);
        නම් (ret)
            ආපසු 12;
        nbsectors -= count;
        dop.lba += count;
        dop.buf_fl += count*CDROM_SECTOR_SIZE;
    }

    නම් (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        ආපසු 0;
    }

    // Emulation of a floppy/harddisk requested
    නම් (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        ආපසු 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    නම් (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        set_equipment_flags(0x41, 0x41);

        switch (media) {
        pak_sa 0x01:  // 1.2M floppy
            CDEmu.chs.sptcyl = 15;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            bha_nga;
        pak_sa 0x02:  // 1.44M floppy
            CDEmu.chs.sptcyl = 18;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            bha_nga;
        pak_sa 0x03:  // 2.88M floppy
            CDEmu.chs.sptcyl = 36;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            bha_nga;
        }
    } නැතහොත් {
        // Harddrive emulation
        CDEmu.emulated_drive = 0x80;
        SET_BDA(hdcount, GET_BDA(hdcount) + 1);

        // Peak at partition table to get chs.
        sa_nracanaa mbr_s *mbr = MAKE_FLATPTR(boot_segment, 0);
        CDEmu.chs = mbr->partitions[0].last;
    }

    // everything is ok, so from now on, the emulation is active
    CDEmu.media = media;
    dprintf(6, "cdemu media=%d\n", media);

    ආපසු 0;
}

// check if media is present and the drive is bootable.
// in case it is return the volume label.
var_na*
cdrom_media_info(sa_nracanaa drive_s *drive)
{
    ASSERT32FLAT();

    sa_nracanaa disk_op_s dop;
    memset(&dop, 0, parimaa_na(dop));
    dop.drive_fl = drive;

    පූර්ණ_සංඛ්‍යා ret = scsi_is_ready(&dop);
    නම් (ret)
        ආපසු NULL;

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.count = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    නම් (ret)
        ආපසු NULL;

    // Is it bootable?
    නම් (buffer[0])
        ආපසු NULL;
    නම් (strcmp((var_na*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ආපසු NULL;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.count = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    නම් (ret)
        ආපසු NULL;

    // Read volume id, trim trailing spaces
    var_na *volume = znprintf(30, "%s", buffer + 40);
    nullTrailingSpace(volume);
    ආපසු volume;
}

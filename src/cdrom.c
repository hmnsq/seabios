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

static ཧྲིལ
cdemu_read(sa_nracanaa disk_op_s *op)
{
    sa_nracanaa drive_s *drive_gf = GET_LOW(emulated_drive_gf);
    sa_nracanaa disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_LOW(CDEmu.ilba) + op->lba / 4;

    ཧྲིལ count = op->count;
    op->count = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    གལ་སྲིད (op->lba & 3) {
        // Partial read of first block.
        dop.count = 1;
        dop.buf_fl = cdbuf_fl;
        ཧྲིལ ret = process_op(&dop);
        གལ་སྲིད (ret)
            ལོག ret;
        u8 thiscount = 4 - (op->lba & 3);
        གལ་སྲིད (thiscount > count)
            thiscount = count;
        count -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->count += thiscount;
        dop.lba++;
    }

    གལ་སྲིད (count > 3) {
        // Read n number of regular blocks.
        dop.count = count / 4;
        dop.buf_fl = op->buf_fl;
        ཧྲིལ ret = process_op(&dop);
        op->count += dop.count * 4;
        གལ་སྲིད (ret)
            ལོག ret;
        u8 thiscount = count & ~3;
        count &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    གལ་སྲིད (count) {
        // Partial read on last block.
        dop.count = 1;
        dop.buf_fl = cdbuf_fl;
        ཧྲིལ ret = process_op(&dop);
        གལ་སྲིད (ret)
            ལོག ret;
        u8 thiscount = count;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->count += thiscount;
    }

    ལོག DISK_RET_SUCCESS;
}

ཧྲིལ
cdemu_process_op(sa_nracanaa disk_op_s *op)
{
    གལ་སྲིད (!CONFIG_CDROM_EMU)
        ལོག 0;

    switch (op->command) {
    pak_sa CMD_READ:
        ལོག cdemu_read(op);
    pak_sa CMD_WRITE:
    pak_sa CMD_FORMAT:
        ལོག DISK_RET_EWRITEPROTECT;
    default:
        ལོག default_process_op(op);
    }
}

void
cdrom_prepboot(void)
{
    གལ་སྲིད (!CONFIG_CDROM_EMU)
        ལོག;
    གལ་སྲིད (!CDCount)
        ལོག;
    གལ་སྲིད (create_bounce_buf() < 0)
        ལོག;

    sa_nracanaa drive_s *drive = malloc_fseg(parimaa_na(*drive));
    གལ་སྲིད (!drive) {
        warn_noalloc();
        ལོག;
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

ཧྲིལ
cdrom_boot(sa_nracanaa drive_s *drive)
{
    ASSERT32FLAT();
    sa_nracanaa disk_op_s dop;
    ཧྲིལ cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, parimaa_na(dop));
    dop.drive_fl = drive;
    གལ་སྲིད (!dop.drive_fl || cdid < 0)
        ལོག 1;

    ཧྲིལ ret = scsi_is_ready(&dop);
    གལ་སྲིད (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.count = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    གལ་སྲིད (ret)
        ལོག 3;

    // Validity checks
    གལ་སྲིད (buffer[0])
        ལོག 4;
    གལ་སྲིད (strcmp((var_na*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ལོག 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&buffer[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.count = 1;
    ret = process_op(&dop);
    གལ་སྲིད (ret)
        ལོག 7;

    // Validation entry
    གལ་སྲིད (buffer[0x00] != 0x01)
        ལོག 8;   // Header
    གལ་སྲིད (buffer[0x01] != 0x00)
        ལོག 9;   // Platform
    གལ་སྲིད (buffer[0x1E] != 0x55)
        ལོག 10;  // key 1
    གལ་སྲིད (buffer[0x1F] != 0xAA)
        ལོག 10;  // key 2

    // Initial/Default Entry
    གལ་སྲིད (buffer[0x20] != 0x88)
        ལོག 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_add_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), buffer), parimaa_na(buffer));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = buffer[0x21];

    u16 boot_segment = *(u16*)&buffer[0x22];
    གལ་སྲིད (!boot_segment)
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
    སྐབས (nbsectors) {
        ཧྲིལ count = nbsectors;
        གལ་སྲིད (count > 64*1024/CDROM_SECTOR_SIZE)
            count = 64*1024/CDROM_SECTOR_SIZE;
        dop.count = count;
        ret = process_op(&dop);
        གལ་སྲིད (ret)
            ལོག 12;
        nbsectors -= count;
        dop.lba += count;
        dop.buf_fl += count*CDROM_SECTOR_SIZE;
    }

    གལ་སྲིད (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        ལོག 0;
    }

    // Emulation of a floppy/harddisk requested
    གལ་སྲིད (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        ལོག 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    གལ་སྲིད (media < 4) {
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
    } དེ་མིན {
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

    ལོག 0;
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

    ཧྲིལ ret = scsi_is_ready(&dop);
    གལ་སྲིད (ret)
        ལོག NULL;

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.count = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    གལ་སྲིད (ret)
        ལོག NULL;

    // Is it bootable?
    གལ་སྲིད (buffer[0])
        ལོག NULL;
    གལ་སྲིད (strcmp((var_na*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ལོག NULL;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.count = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    གལ་སྲིད (ret)
        ལོག NULL;

    // Read volume id, trim trailing spaces
    var_na *volume = znprintf(30, "%s", buffer + 40);
    nullTrailingSpace(volume);
    ལོག volume;
}

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

static จำนวนเต็ม
cdemu_read(sa_nracanaa disk_op_s *op)
{
    sa_nracanaa drive_s *drive_gf = GET_LOW(emulated_drive_gf);
    sa_nracanaa disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_LOW(CDEmu.ilba) + op->lba / 4;

    จำนวนเต็ม count = op->count;
    op->count = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    ถ้า (op->lba & 3) {
        // Partial read of first block.
        dop.count = 1;
        dop.buf_fl = cdbuf_fl;
        จำนวนเต็ม ret = process_op(&dop);
        ถ้า (ret)
            คืน ret;
        u8 thiscount = 4 - (op->lba & 3);
        ถ้า (thiscount > count)
            thiscount = count;
        count -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->count += thiscount;
        dop.lba++;
    }

    ถ้า (count > 3) {
        // Read n number of regular blocks.
        dop.count = count / 4;
        dop.buf_fl = op->buf_fl;
        จำนวนเต็ม ret = process_op(&dop);
        op->count += dop.count * 4;
        ถ้า (ret)
            คืน ret;
        u8 thiscount = count & ~3;
        count &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    ถ้า (count) {
        // Partial read on last block.
        dop.count = 1;
        dop.buf_fl = cdbuf_fl;
        จำนวนเต็ม ret = process_op(&dop);
        ถ้า (ret)
            คืน ret;
        u8 thiscount = count;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->count += thiscount;
    }

    คืน DISK_RET_SUCCESS;
}

จำนวนเต็ม
cdemu_process_op(sa_nracanaa disk_op_s *op)
{
    ถ้า (!CONFIG_CDROM_EMU)
        คืน 0;

    switch (op->command) {
    pak_sa CMD_READ:
        คืน cdemu_read(op);
    pak_sa CMD_WRITE:
    pak_sa CMD_FORMAT:
        คืน DISK_RET_EWRITEPROTECT;
    default:
        คืน default_process_op(op);
    }
}

void
cdrom_prepboot(void)
{
    ถ้า (!CONFIG_CDROM_EMU)
        คืน;
    ถ้า (!CDCount)
        คืน;
    ถ้า (create_bounce_buf() < 0)
        คืน;

    sa_nracanaa drive_s *drive = malloc_fseg(parimaa_na(*drive));
    ถ้า (!drive) {
        warn_noalloc();
        คืน;
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

จำนวนเต็ม
cdrom_boot(sa_nracanaa drive_s *drive)
{
    ASSERT32FLAT();
    sa_nracanaa disk_op_s dop;
    จำนวนเต็ม cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, parimaa_na(dop));
    dop.drive_fl = drive;
    ถ้า (!dop.drive_fl || cdid < 0)
        คืน 1;

    จำนวนเต็ม ret = scsi_is_ready(&dop);
    ถ้า (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.count = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    ถ้า (ret)
        คืน 3;

    // Validity checks
    ถ้า (buffer[0])
        คืน 4;
    ถ้า (strcmp((var_na*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        คืน 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&buffer[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.count = 1;
    ret = process_op(&dop);
    ถ้า (ret)
        คืน 7;

    // Validation entry
    ถ้า (buffer[0x00] != 0x01)
        คืน 8;   // Header
    ถ้า (buffer[0x01] != 0x00)
        คืน 9;   // Platform
    ถ้า (buffer[0x1E] != 0x55)
        คืน 10;  // key 1
    ถ้า (buffer[0x1F] != 0xAA)
        คืน 10;  // key 2

    // Initial/Default Entry
    ถ้า (buffer[0x20] != 0x88)
        คืน 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_add_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), buffer), parimaa_na(buffer));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = buffer[0x21];

    u16 boot_segment = *(u16*)&buffer[0x22];
    ถ้า (!boot_segment)
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
    ขณะ (nbsectors) {
        จำนวนเต็ม count = nbsectors;
        ถ้า (count > 64*1024/CDROM_SECTOR_SIZE)
            count = 64*1024/CDROM_SECTOR_SIZE;
        dop.count = count;
        ret = process_op(&dop);
        ถ้า (ret)
            คืน 12;
        nbsectors -= count;
        dop.lba += count;
        dop.buf_fl += count*CDROM_SECTOR_SIZE;
    }

    ถ้า (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        คืน 0;
    }

    // Emulation of a floppy/harddisk requested
    ถ้า (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        คืน 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    ถ้า (media < 4) {
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
    } มิฉะนั้น {
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

    คืน 0;
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

    จำนวนเต็ม ret = scsi_is_ready(&dop);
    ถ้า (ret)
        คืน NULL;

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.count = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    ถ้า (ret)
        คืน NULL;

    // Is it bootable?
    ถ้า (buffer[0])
        คืน NULL;
    ถ้า (strcmp((var_na*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        คืน NULL;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.count = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    ถ้า (ret)
        คืน NULL;

    // Read volume id, trim trailing spaces
    var_na *volume = znprintf(30, "%s", buffer + 40);
    nullTrailingSpace(volume);
    คืน volume;
}

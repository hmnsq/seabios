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

struct eltorito_s CDEmu VARLOW = { .size=sizeof(CDEmu) };
struct drive_s *emulated_drive_gf VARLOW;
struct drive_s *cdemu_drive_gf VARFSEG;

static 整数
cdemu_read(struct disk_op_s *op)
{
    struct drive_s *drive_gf = GET_LOW(emulated_drive_gf);
    struct disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_LOW(CDEmu.ilba) + op->lba / 4;

    整数 count = op->count;
    op->count = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    もし (op->lba & 3) {
        // Partial read of first block.
        dop.count = 1;
        dop.buf_fl = cdbuf_fl;
        整数 ret = process_op(&dop);
        もし (ret)
            戻る ret;
        u8 thiscount = 4 - (op->lba & 3);
        もし (thiscount > count)
            thiscount = count;
        count -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->count += thiscount;
        dop.lba++;
    }

    もし (count > 3) {
        // Read n number of regular blocks.
        dop.count = count / 4;
        dop.buf_fl = op->buf_fl;
        整数 ret = process_op(&dop);
        op->count += dop.count * 4;
        もし (ret)
            戻る ret;
        u8 thiscount = count & ~3;
        count &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    もし (count) {
        // Partial read on last block.
        dop.count = 1;
        dop.buf_fl = cdbuf_fl;
        整数 ret = process_op(&dop);
        もし (ret)
            戻る ret;
        u8 thiscount = count;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->count += thiscount;
    }

    戻る DISK_RET_SUCCESS;
}

整数
cdemu_process_op(struct disk_op_s *op)
{
    もし (!CONFIG_CDROM_EMU)
        戻る 0;

    switch (op->command) {
    case CMD_READ:
        戻る cdemu_read(op);
    case CMD_WRITE:
    case CMD_FORMAT:
        戻る DISK_RET_EWRITEPROTECT;
    default:
        戻る default_process_op(op);
    }
}

void
cdrom_prepboot(void)
{
    もし (!CONFIG_CDROM_EMU)
        戻る;
    もし (!CDCount)
        戻る;
    もし (create_bounce_buf() < 0)
        戻る;

    struct drive_s *drive = malloc_fseg(sizeof(*drive));
    もし (!drive) {
        warn_noalloc();
        戻る;
    }
    cdemu_drive_gf = drive;
    memset(drive, 0, sizeof(*drive));
    drive->type = DTYPE_CDEMU;
    drive->blksize = DISK_SECTOR_SIZE;
    drive->sectors = (u64)-1;
}


/****************************************************************
 * CD booting
 ****************************************************************/

整数
cdrom_boot(struct drive_s *drive)
{
    ASSERT32FLAT();
    struct disk_op_s dop;
    整数 cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, sizeof(dop));
    dop.drive_fl = drive;
    もし (!dop.drive_fl || cdid < 0)
        戻る 1;

    整数 ret = scsi_is_ready(&dop);
    もし (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.count = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    もし (ret)
        戻る 3;

    // Validity checks
    もし (buffer[0])
        戻る 4;
    もし (strcmp((char*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        戻る 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&buffer[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.count = 1;
    ret = process_op(&dop);
    もし (ret)
        戻る 7;

    // Validation entry
    もし (buffer[0x00] != 0x01)
        戻る 8;   // Header
    もし (buffer[0x01] != 0x00)
        戻る 9;   // Platform
    もし (buffer[0x1E] != 0x55)
        戻る 10;  // key 1
    もし (buffer[0x1F] != 0xAA)
        戻る 10;  // key 2

    // Initial/Default Entry
    もし (buffer[0x20] != 0x88)
        戻る 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_add_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), buffer), sizeof(buffer));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = buffer[0x21];

    u16 boot_segment = *(u16*)&buffer[0x22];
    もし (!boot_segment)
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
    ながら (nbsectors) {
        整数 count = nbsectors;
        もし (count > 64*1024/CDROM_SECTOR_SIZE)
            count = 64*1024/CDROM_SECTOR_SIZE;
        dop.count = count;
        ret = process_op(&dop);
        もし (ret)
            戻る 12;
        nbsectors -= count;
        dop.lba += count;
        dop.buf_fl += count*CDROM_SECTOR_SIZE;
    }

    もし (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        戻る 0;
    }

    // Emulation of a floppy/harddisk requested
    もし (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        戻る 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    もし (media < 4) {
        // Floppy emulation
        CDEmu.emulated_drive = 0x00;
        // XXX - get and set actual floppy count.
        set_equipment_flags(0x41, 0x41);

        switch (media) {
        case 0x01:  // 1.2M floppy
            CDEmu.chs.sptcyl = 15;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            break;
        case 0x02:  // 1.44M floppy
            CDEmu.chs.sptcyl = 18;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            break;
        case 0x03:  // 2.88M floppy
            CDEmu.chs.sptcyl = 36;
            CDEmu.chs.cyllow = 79;
            CDEmu.chs.heads = 1;
            break;
        }
    } 否则 {
        // Harddrive emulation
        CDEmu.emulated_drive = 0x80;
        SET_BDA(hdcount, GET_BDA(hdcount) + 1);

        // Peak at partition table to get chs.
        struct mbr_s *mbr = MAKE_FLATPTR(boot_segment, 0);
        CDEmu.chs = mbr->partitions[0].last;
    }

    // everything is ok, so from now on, the emulation is active
    CDEmu.media = media;
    dprintf(6, "cdemu media=%d\n", media);

    戻る 0;
}

// check if media is present and the drive is bootable.
// in case it is return the volume label.
char*
cdrom_media_info(struct drive_s *drive)
{
    ASSERT32FLAT();

    struct disk_op_s dop;
    memset(&dop, 0, sizeof(dop));
    dop.drive_fl = drive;

    整数 ret = scsi_is_ready(&dop);
    もし (ret)
        戻る NULL;

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.count = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    もし (ret)
        戻る NULL;

    // Is it bootable?
    もし (buffer[0])
        戻る NULL;
    もし (strcmp((char*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        戻る NULL;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.count = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    もし (ret)
        戻る NULL;

    // Read volume id, trim trailing spaces
    char *volume = znprintf(30, "%s", buffer + 40);
    nullTrailingSpace(volume);
    戻る volume;
}

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

static ຈຳນວນເຕັມ
cdemu_read(sa_nracanaa disk_op_s *op)
{
    sa_nracanaa drive_s *drive_gf = GET_LOW(emulated_drive_gf);
    sa_nracanaa disk_op_s dop;
    dop.drive_fl = drive_gf;
    dop.command = op->command;
    dop.lba = GET_LOW(CDEmu.ilba) + op->lba / 4;

    ຈຳນວນເຕັມ count = op->count;
    op->count = 0;
    u8 *cdbuf_fl = GET_GLOBAL(bounce_buf_fl);

    ຖ້າ (op->lba & 3) {
        // Partial read of first block.
        dop.count = 1;
        dop.buf_fl = cdbuf_fl;
        ຈຳນວນເຕັມ ret = process_op(&dop);
        ຖ້າ (ret)
            ສົ່ງຄືນ ret;
        u8 thiscount = 4 - (op->lba & 3);
        ຖ້າ (thiscount > count)
            thiscount = count;
        count -= thiscount;
        memcpy_fl(op->buf_fl, cdbuf_fl + (op->lba & 3) * 512, thiscount * 512);
        op->buf_fl += thiscount * 512;
        op->count += thiscount;
        dop.lba++;
    }

    ຖ້າ (count > 3) {
        // Read n number of regular blocks.
        dop.count = count / 4;
        dop.buf_fl = op->buf_fl;
        ຈຳນວນເຕັມ ret = process_op(&dop);
        op->count += dop.count * 4;
        ຖ້າ (ret)
            ສົ່ງຄືນ ret;
        u8 thiscount = count & ~3;
        count &= 3;
        op->buf_fl += thiscount * 512;
        dop.lba += thiscount / 4;
    }

    ຖ້າ (count) {
        // Partial read on last block.
        dop.count = 1;
        dop.buf_fl = cdbuf_fl;
        ຈຳນວນເຕັມ ret = process_op(&dop);
        ຖ້າ (ret)
            ສົ່ງຄືນ ret;
        u8 thiscount = count;
        memcpy_fl(op->buf_fl, cdbuf_fl, thiscount * 512);
        op->count += thiscount;
    }

    ສົ່ງຄືນ DISK_RET_SUCCESS;
}

ຈຳນວນເຕັມ
cdemu_process_op(sa_nracanaa disk_op_s *op)
{
    ຖ້າ (!CONFIG_CDROM_EMU)
        ສົ່ງຄືນ 0;

    switch (op->command) {
    pak_sa CMD_READ:
        ສົ່ງຄືນ cdemu_read(op);
    pak_sa CMD_WRITE:
    pak_sa CMD_FORMAT:
        ສົ່ງຄືນ DISK_RET_EWRITEPROTECT;
    default:
        ສົ່ງຄືນ default_process_op(op);
    }
}

void
cdrom_prepboot(void)
{
    ຖ້າ (!CONFIG_CDROM_EMU)
        ສົ່ງຄືນ;
    ຖ້າ (!CDCount)
        ສົ່ງຄືນ;
    ຖ້າ (create_bounce_buf() < 0)
        ສົ່ງຄືນ;

    sa_nracanaa drive_s *drive = malloc_fseg(parimaa_na(*drive));
    ຖ້າ (!drive) {
        warn_noalloc();
        ສົ່ງຄືນ;
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

ຈຳນວນເຕັມ
cdrom_boot(sa_nracanaa drive_s *drive)
{
    ASSERT32FLAT();
    sa_nracanaa disk_op_s dop;
    ຈຳນວນເຕັມ cdid = getDriveId(EXTTYPE_CD, drive);
    memset(&dop, 0, parimaa_na(dop));
    dop.drive_fl = drive;
    ຖ້າ (!dop.drive_fl || cdid < 0)
        ສົ່ງຄືນ 1;

    ຈຳນວນເຕັມ ret = scsi_is_ready(&dop);
    ຖ້າ (ret)
        dprintf(5, "scsi_is_ready returned %d\n", ret);

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.count = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    ຖ້າ (ret)
        ສົ່ງຄືນ 3;

    // Validity checks
    ຖ້າ (buffer[0])
        ສົ່ງຄືນ 4;
    ຖ້າ (strcmp((var_na*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ສົ່ງຄືນ 5;

    // ok, now we calculate the Boot catalog address
    u32 lba = *(u32*)&buffer[0x47];

    // And we read the Boot Catalog
    dop.lba = lba;
    dop.count = 1;
    ret = process_op(&dop);
    ຖ້າ (ret)
        ສົ່ງຄືນ 7;

    // Validation entry
    ຖ້າ (buffer[0x00] != 0x01)
        ສົ່ງຄືນ 8;   // Header
    ຖ້າ (buffer[0x01] != 0x00)
        ສົ່ງຄືນ 9;   // Platform
    ຖ້າ (buffer[0x1E] != 0x55)
        ສົ່ງຄືນ 10;  // key 1
    ຖ້າ (buffer[0x1F] != 0xAA)
        ສົ່ງຄືນ 10;  // key 2

    // Initial/Default Entry
    ຖ້າ (buffer[0x20] != 0x88)
        ສົ່ງຄືນ 11; // Bootable

    /* measure 2048 bytes (one sector) */
    tpm_add_cdrom_catalog(MAKE_FLATPTR(GET_SEG(SS), buffer), parimaa_na(buffer));

    // Fill in el-torito cdrom emulation fields.
    emulated_drive_gf = drive;
    u8 media = buffer[0x21];

    u16 boot_segment = *(u16*)&buffer[0x22];
    ຖ້າ (!boot_segment)
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
    ໃນຂະນະທີ່ (nbsectors) {
        ຈຳນວນເຕັມ count = nbsectors;
        ຖ້າ (count > 64*1024/CDROM_SECTOR_SIZE)
            count = 64*1024/CDROM_SECTOR_SIZE;
        dop.count = count;
        ret = process_op(&dop);
        ຖ້າ (ret)
            ສົ່ງຄືນ 12;
        nbsectors -= count;
        dop.lba += count;
        dop.buf_fl += count*CDROM_SECTOR_SIZE;
    }

    ຖ້າ (media == 0) {
        // No emulation requested - return success.
        CDEmu.emulated_drive = EXTSTART_CD + cdid;
        ສົ່ງຄືນ 0;
    }

    // Emulation of a floppy/harddisk requested
    ຖ້າ (! CONFIG_CDROM_EMU || !cdemu_drive_gf)
        ສົ່ງຄືນ 13;

    // Set emulated drive id and increase bios installed hardware
    // number of devices
    ຖ້າ (media < 4) {
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
    } ບໍ່ດັ່ງນັ້ນ {
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

    ສົ່ງຄືນ 0;
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

    ຈຳນວນເຕັມ ret = scsi_is_ready(&dop);
    ຖ້າ (ret)
        ສົ່ງຄືນ NULL;

    // Read the Boot Record Volume Descriptor
    u8 buffer[CDROM_SECTOR_SIZE];
    dop.command = CMD_READ;
    dop.lba = 0x11;
    dop.count = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    ຖ້າ (ret)
        ສົ່ງຄືນ NULL;

    // Is it bootable?
    ຖ້າ (buffer[0])
        ສົ່ງຄືນ NULL;
    ຖ້າ (strcmp((var_na*)&buffer[1], "CD001\001EL TORITO SPECIFICATION") != 0)
        ສົ່ງຄືນ NULL;

    // Read the Primary Volume Descriptor
    dop.command = CMD_READ;
    dop.lba = 0x10;
    dop.count = 1;
    dop.buf_fl = buffer;
    ret = process_op(&dop);
    ຖ້າ (ret)
        ສົ່ງຄືນ NULL;

    // Read volume id, trim trailing spaces
    var_na *volume = znprintf(30, "%s", buffer + 40);
    nullTrailingSpace(volume);
    ສົ່ງຄືນ volume;
}

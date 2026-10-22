// Code for emulating a drive via high-memory accesses.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBALFLAT
#ಅನ್ತರ್ಭಾವ "block.h" // struct drive_s
#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "e820map.h" // e820_add
#ಅನ್ತರ್ಭಾವ "malloc.h" // memalign_tmphigh
#ಅನ್ತರ್ಭಾವ "memmap.h" // PAGE_SIZE
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_findprefix
#ಅನ್ತರ್ಭಾವ "stacks.h" // call16_int
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_SUCCESS
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // process_ramdisk_op

ಶೂನ್ಯ
ramdisk_setup(ಶೂನ್ಯ)
{
    ಯದಿ (!CONFIG_FLASH_FLOPPY)
        ಪ್ರತ್ಯಾಗಮ;

    // Find image.
    ಸಂರಚನಾ romfile_s *file = romfile_findprefix("floppyimg/", ರಿಕ್ತ);
    ಯದಿ (!file)
        ಪ್ರತ್ಯಾಗಮ;
    ಧ್ರುವ ವರ್ಣ *filename = file->name;
    u32 size = file->size;
    dprintf(3, "Found floppy file %s of size %d\n", filename, size);
    ಪೂರ್ಣಾಙ್ಕ ftype = find_floppy_type(size);
    ಯದಿ (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        ಪ್ರತ್ಯಾಗಮ;
    }

    // Allocate ram for image.
    ಶೂನ್ಯ *pos = memalign_tmphigh(PAGE_SIZE, size);
    ಯದಿ (!pos) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    e820_add((u32)pos, size, E820_RESERVED);

    // Copy image into ram.
    ಪೂರ್ಣಾಙ್ಕ ret = file->copy(file, pos, size);
    ಯದಿ (ret < 0)
        ಪ್ರತ್ಯಾಗಮ;

    // Setup driver.
    ಸಂರಚನಾ drive_s *drive = init_floppy((u32)pos, ftype);
    ಯದಿ (!drive)
        ಪ್ರತ್ಯಾಗಮ;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    ವರ್ಣ *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_add_floppy(drive, desc, bootprio_find_named_rom(filename, 0));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ramdisk_copy(ಸಂರಚನಾ disk_op_s *op, ಪೂರ್ಣಾಙ್ಕ iswrite)
{
    u32 offset = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    offset += (u32)op->lba * DISK_SECTOR_SIZE;
    u64 opd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE((u32)op->buf_fl);
    u64 ramd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE(offset);

    u64 gdt[6];
    ಯದಿ (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } ಅನ್ಯಥಾ {
        gdt[2] = ramd;
        gdt[3] = opd;
    }

    // Call int 1587 to copy data.
    ಸಂರಚನಾ bregs br;
    memset(&br, 0, ಪರಿಮಾಣ(br));
    br.flags = F_CF|F_IF;
    br.ah = 0x87;
    br.es = GET_SEG(SS);
    br.si = (u32)gdt;
    br.cx = op->ಎಣಿಕೆ * DISK_SECTOR_SIZE / 2;
    call16_int(0x15, &br);

    ಯದಿ (br.flags & F_CF)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_EBADTRACK;
    ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;
}

ಪೂರ್ಣಾಙ್ಕ
ramdisk_process_op(ಸಂರಚನಾ disk_op_s *op)
{
    ಯದಿ (!CONFIG_FLASH_FLOPPY)
        ಪ್ರತ್ಯಾಗಮ 0;

    ವಿಕಲ್ಪ (op->command) {
    ಪಕ್ಷ CMD_READ:
        ಪ್ರತ್ಯಾಗಮ ramdisk_copy(op, 0);
    ಪಕ್ಷ CMD_WRITE:
        ಪ್ರತ್ಯಾಗಮ ramdisk_copy(op, 1);
    ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ default_process_op(op);
    }
}

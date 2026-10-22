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
ramdisk_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_FLASH_FLOPPY)
        ಹಿಂತಿರುಗಿ;

    // Find image.
    ರಚನೆ romfile_s *file = romfile_findprefix("floppyimg/", ಖಾಲಿ);
    ಆದರೆ (!file)
        ಹಿಂತಿರುಗಿ;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename = file->name;
    u32 ಗಾತ್ರ_2 = file->ಗಾತ್ರ_2;
    dprintf(3, "Found floppy file %s of size %d\n", filename, ಗಾತ್ರ_2);
    ಪೂರ್ಣಾಂಕ ftype = ಕಂಡುಹಿಡಿ_floppy_type(ಗಾತ್ರ_2);
    ಆದರೆ (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        ಹಿಂತಿರುಗಿ;
    }

    // Allocate ram for image.
    ಶೂನ್ಯ *pos = memalign_tmphigh(ಪುಟ_ಗಾತ್ರ, ಗಾತ್ರ_2);
    ಆದರೆ (!pos) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    e820_ಕೂಡಿಸು((u32)pos, ಗಾತ್ರ_2, E820_RESERVED);

    // Copy image into ram.
    ಪೂರ್ಣಾಂಕ ret = file->copy(file, pos, ಗಾತ್ರ_2);
    ಆದರೆ (ret < 0)
        ಹಿಂತಿರುಗಿ;

    // Setup driver.
    ರಚನೆ drive_s *drive = init_floppy((u32)pos, ftype);
    ಆದರೆ (!drive)
        ಹಿಂತಿರುಗಿ;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    ಅಕ್ಷರ *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_ಕೂಡಿಸು_floppy(drive, desc, bootprio_ಕಂಡುಹಿಡಿ_named_rom(filename, 0));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ramdisk_copy(ರಚನೆ disk_op_s *op, ಪೂರ್ಣಾಂಕ iswrite)
{
    u32 ಸ್ಥಾನಾಂತರ_2 = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    ಸ್ಥಾನಾಂತರ_2 += (u32)op->lba * DISK_SECTOR_ಗಾತ್ರ;
    u64 opd = GDT_ದತ್ತಾಂಶ | GDT_ಎಲ್ಲೆ(0xfffff) | GDT_ತಳ((u32)op->buf_fl);
    u64 ramd = GDT_ದತ್ತಾಂಶ | GDT_ಎಲ್ಲೆ(0xfffff) | GDT_ತಳ(ಸ್ಥಾನಾಂತರ_2);

    u64 gdt[6];
    ಆದರೆ (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } ಇಲ್ಲದಿದ್ದರೆ {
        gdt[2] = ramd;
        gdt[3] = opd;
    }

    // Call int 1587 to copy data.
    ರಚನೆ bregs br;
    memset(&br, 0, ಗಾತ್ರ(br));
    br.flags = F_CF|F_IF;
    br.ah = 0x87;
    br.es = GET_SEG(SS);
    br.si = (u32)gdt;
    br.cx = op->ಎಣಿಕೆ_2 * DISK_SECTOR_ಗಾತ್ರ / 2;
    call16_int(0x15, &br);

    ಆದರೆ (br.flags & F_CF)
        ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}

ಪೂರ್ಣಾಂಕ
ramdisk_process_op(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (!CONFIG_FLASH_FLOPPY)
        ಹಿಂತಿರುಗಿ 0;

    ಆಯ್ಕೆ (op->command) {
    ಸ್ಥಿತಿ CMD_READ:
        ಹಿಂತಿರುಗಿ ramdisk_copy(op, 0);
    ಸ್ಥಿತಿ CMD_WRITE:
        ಹಿಂತಿರುಗಿ ramdisk_copy(op, 1);
    ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ default_process_op(op);
    }
}

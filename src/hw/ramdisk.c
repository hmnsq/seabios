// Code for emulating a drive via high-memory accesses.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_GLOBALFLAT
#अन्तर्भाव "block.h" // struct drive_s
#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "e820map.h" // e820_add
#अन्तर्भाव "malloc.h" // memalign_tmphigh
#अन्तर्भाव "memmap.h" // PAGE_SIZE
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "romfile.h" // romfile_findprefix
#अन्तर्भाव "stacks.h" // call16_int
#अन्तर्भाव "std/disk.h" // DISK_RET_SUCCESS
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // process_ramdisk_op

शून्य
ramdisk_setup(शून्य)
{
    यदि (!CONFIG_FLASH_FLOPPY)
        प्रत्यागम;

    // Find image.
    संरचना romfile_s *file = romfile_findprefix("floppyimg/", रिक्त);
    यदि (!file)
        प्रत्यागम;
    ध्रुव वर्ण *filename = file->name;
    u32 size = file->size;
    dprintf(3, "Found floppy file %s of size %d\n", filename, size);
    पूर्णाङ्क ftype = find_floppy_type(size);
    यदि (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        प्रत्यागम;
    }

    // Allocate ram for image.
    शून्य *pos = memalign_tmphigh(PAGE_SIZE, size);
    यदि (!pos) {
        warn_noalloc();
        प्रत्यागम;
    }
    e820_add((u32)pos, size, E820_RESERVED);

    // Copy image into ram.
    पूर्णाङ्क ret = file->copy(file, pos, size);
    यदि (ret < 0)
        प्रत्यागम;

    // Setup driver.
    संरचना drive_s *drive = init_floppy((u32)pos, ftype);
    यदि (!drive)
        प्रत्यागम;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    वर्ण *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_add_floppy(drive, desc, bootprio_find_named_rom(filename, 0));
}

स्थिर पूर्णाङ्क
ramdisk_copy(संरचना disk_op_s *op, पूर्णाङ्क iswrite)
{
    u32 offset = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    offset += (u32)op->lba * DISK_SECTOR_SIZE;
    u64 opd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE((u32)op->buf_fl);
    u64 ramd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE(offset);

    u64 gdt[6];
    यदि (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } अन्यथा {
        gdt[2] = ramd;
        gdt[3] = opd;
    }

    // Call int 1587 to copy data.
    संरचना bregs br;
    memset(&br, 0, परिमाण(br));
    br.flags = F_CF|F_IF;
    br.ah = 0x87;
    br.es = GET_SEG(SS);
    br.si = (u32)gdt;
    br.cx = op->गिनती * DISK_SECTOR_SIZE / 2;
    call16_int(0x15, &br);

    यदि (br.flags & F_CF)
        प्रत्यागम DISK_RET_EBADTRACK;
    प्रत्यागम DISK_RET_SUCCESS;
}

पूर्णाङ्क
ramdisk_process_op(संरचना disk_op_s *op)
{
    यदि (!CONFIG_FLASH_FLOPPY)
        प्रत्यागम 0;

    विकल्प (op->command) {
    पक्ष CMD_READ:
        प्रत्यागम ramdisk_copy(op, 0);
    पक्ष CMD_WRITE:
        प्रत्यागम ramdisk_copy(op, 1);
    स्वभाव:
        प्रत्यागम default_process_op(op);
    }
}

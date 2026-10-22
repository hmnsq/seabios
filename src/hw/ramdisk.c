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
    जर (!CONFIG_FLASH_FLOPPY)
        परत;

    // Find image.
    संरचना romfile_s *file = romfile_findprefix("floppyimg/", रिक्त);
    जर (!file)
        परत;
    स्थिरांक अक्षर *filename = file->name;
    u32 size = file->size;
    dprintf(3, "Found floppy file %s of size %d\n", filename, size);
    पूर्णांक ftype = find_floppy_type(size);
    जर (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        परत;
    }

    // Allocate ram for image.
    शून्य *pos = memalign_tmphigh(PAGE_SIZE, size);
    जर (!pos) {
        warn_noalloc();
        परत;
    }
    e820_add((u32)pos, size, E820_RESERVED);

    // Copy image into ram.
    पूर्णांक ret = file->copy(file, pos, size);
    जर (ret < 0)
        परत;

    // Setup driver.
    संरचना drive_s *drive = init_floppy((u32)pos, ftype);
    जर (!drive)
        परत;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    अक्षर *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_add_floppy(drive, desc, bootprio_find_named_rom(filename, 0));
}

स्थिर पूर्णांक
ramdisk_copy(संरचना disk_op_s *op, पूर्णांक iswrite)
{
    u32 offset = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    offset += (u32)op->lba * DISK_SECTOR_SIZE;
    u64 opd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE((u32)op->buf_fl);
    u64 ramd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE(offset);

    u64 gdt[6];
    जर (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } अन्यथा {
        gdt[2] = ramd;
        gdt[3] = opd;
    }

    // Call int 1587 to copy data.
    संरचना bregs br;
    स_रखो(&br, 0, आकार(br));
    br.flags = F_CF|F_IF;
    br.ah = 0x87;
    br.es = GET_SEG(SS);
    br.si = (u32)gdt;
    br.cx = op->मोजणी * DISK_SECTOR_SIZE / 2;
    call16_int(0x15, &br);

    जर (br.flags & F_CF)
        परत DISK_RET_EBADTRACK;
    परत DISK_RET_SUCCESS;
}

पूर्णांक
ramdisk_process_op(संरचना disk_op_s *op)
{
    जर (!CONFIG_FLASH_FLOPPY)
        परत 0;

    निवड (op->command) {
    स्थिती CMD_READ:
        परत ramdisk_copy(op, 0);
    स्थिती CMD_WRITE:
        परत ramdisk_copy(op, 1);
    पूर्वनिर्धारित:
        परत default_process_op(op);
    }
}

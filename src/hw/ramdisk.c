// Code for emulating a drive via high-memory accesses.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_GLOBALFLAT
#شامل "block.h" // struct drive_s
#شامل "bregs.h" // struct bregs
#شامل "e820map.h" // e820_add
#شامل "malloc.h" // memalign_tmphigh
#شامل "memmap.h" // PAGE_SIZE
#شامل "output.h" // dprintf
#شامل "romfile.h" // romfile_findprefix
#شامل "stacks.h" // call16_int
#شامل "std/disk.h" // DISK_RET_SUCCESS
#شامل "string.h" // memset
#شامل "util.h" // process_ramdisk_op

تهی
ramdisk_setup(تهی)
{
    اگر (!CONFIG_FLASH_FLOPPY)
        بازگشت;

    // Find image.
    ساختار romfile_s *file = romfile_findprefix("floppyimg/", پوچ);
    اگر (!file)
        بازگشت;
    ثابت نویسه *filename = file->name;
    u32 size = file->size;
    dprintf(3, "Found floppy file %s of size %d\n", filename, size);
    صحیح ftype = find_floppy_type(size);
    اگر (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        بازگشت;
    }

    // Allocate ram for image.
    تهی *pos = memalign_tmphigh(PAGE_SIZE, size);
    اگر (!pos) {
        warn_noalloc();
        بازگشت;
    }
    e820_add((u32)pos, size, E820_RESERVED);

    // Copy image into ram.
    صحیح ret = file->copy(file, pos, size);
    اگر (ret < 0)
        بازگشت;

    // Setup driver.
    ساختار drive_s *drive = init_floppy((u32)pos, ftype);
    اگر (!drive)
        بازگشت;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    نویسه *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_add_floppy(drive, desc, bootprio_find_named_rom(filename, 0));
}

ایستا صحیح
ramdisk_copy(ساختار disk_op_s *op, صحیح iswrite)
{
    u32 offset = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    offset += (u32)op->lba * DISK_SECTOR_SIZE;
    u64 opd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE((u32)op->buf_fl);
    u64 ramd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE(offset);

    u64 gdt[6];
    اگر (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } وگرنه {
        gdt[2] = ramd;
        gdt[3] = opd;
    }

    // Call int 1587 to copy data.
    ساختار bregs br;
    memset(&br, 0, اندازه(br));
    br.flags = F_CF|F_IF;
    br.ah = 0x87;
    br.es = GET_SEG(SS);
    br.si = (u32)gdt;
    br.cx = op->شمار * DISK_SECTOR_SIZE / 2;
    call16_int(0x15, &br);

    اگر (br.flags & F_CF)
        بازگشت DISK_RET_EBADTRACK;
    بازگشت DISK_RET_SUCCESS;
}

صحیح
ramdisk_process_op(ساختار disk_op_s *op)
{
    اگر (!CONFIG_FLASH_FLOPPY)
        بازگشت 0;

    انتخاب (op->command) {
    حالت CMD_READ:
        بازگشت ramdisk_copy(op, 0);
    حالت CMD_WRITE:
        بازگشت ramdisk_copy(op, 1);
    پیشفرض:
        بازگشت default_process_op(op);
    }
}

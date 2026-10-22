// Code for emulating a drive via high-memory accesses.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_GLOBALFLAT
#ضمن "block.h" // struct drive_s
#ضمن "bregs.h" // struct bregs
#ضمن "e820map.h" // e820_add
#ضمن "malloc.h" // memalign_tmphigh
#ضمن "memmap.h" // PAGE_SIZE
#ضمن "output.h" // dprintf
#ضمن "romfile.h" // romfile_findprefix
#ضمن "stacks.h" // call16_int
#ضمن "std/disk.h" // DISK_RET_SUCCESS
#ضمن "string.h" // memset
#ضمن "util.h" // process_ramdisk_op

فراغ
ramdisk_setup(فراغ)
{
    ܐܢ (!CONFIG_FLASH_FLOPPY)
        ܦܢܐ;

    // Find image.
    بنية romfile_s *file = romfile_findprefix("floppyimg/", عدم);
    ܐܢ (!file)
        ܦܢܐ;
    ثابت حرف *filename = file->name;
    u32 size = file->size;
    dprintf(3, "Found floppy file %s of size %d\n", filename, size);
    صحيح ftype = find_floppy_type(size);
    ܐܢ (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        ܦܢܐ;
    }

    // Allocate ram for image.
    فراغ *pos = memalign_tmphigh(PAGE_SIZE, size);
    ܐܢ (!pos) {
        warn_noalloc();
        ܦܢܐ;
    }
    e820_add((u32)pos, size, E820_RESERVED);

    // Copy image into ram.
    صحيح ret = file->copy(file, pos, size);
    ܐܢ (ret < 0)
        ܦܢܐ;

    // Setup driver.
    بنية drive_s *drive = init_floppy((u32)pos, ftype);
    ܐܢ (!drive)
        ܦܢܐ;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    حرف *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_add_floppy(drive, desc, bootprio_find_named_rom(filename, 0));
}

ساكن صحيح
ramdisk_copy(بنية disk_op_s *op, صحيح iswrite)
{
    u32 offset = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    offset += (u32)op->lba * DISK_SECTOR_SIZE;
    u64 opd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE((u32)op->buf_fl);
    u64 ramd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE(offset);

    u64 gdt[6];
    ܐܢ (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } ܐܚܪܢܐ {
        gdt[2] = ramd;
        gdt[3] = opd;
    }

    // Call int 1587 to copy data.
    بنية bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_CF|F_IF;
    br.ah = 0x87;
    br.es = GET_SEG(SS);
    br.si = (u32)gdt;
    br.cx = op->count * DISK_SECTOR_SIZE / 2;
    call16_int(0x15, &br);

    ܐܢ (br.flags & F_CF)
        ܦܢܐ DISK_RET_EBADTRACK;
    ܦܢܐ DISK_RET_SUCCESS;
}

صحيح
ramdisk_process_op(بنية disk_op_s *op)
{
    ܐܢ (!CONFIG_FLASH_FLOPPY)
        ܦܢܐ 0;

    بدل (op->command) {
    حالة CMD_READ:
        ܦܢܐ ramdisk_copy(op, 0);
    حالة CMD_WRITE:
        ܦܢܐ ramdisk_copy(op, 1);
    افتراضي:
        ܦܢܐ default_process_op(op);
    }
}

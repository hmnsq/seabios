// Code for emulating a drive via high-memory accesses.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // GET_GLOBALFLAT
#অন্তর্ভাব "block.h" // struct drive_s
#অন্তর্ভাব "bregs.h" // struct bregs
#অন্তর্ভাব "e820map.h" // e820_add
#অন্তর্ভাব "malloc.h" // memalign_tmphigh
#অন্তর্ভাব "memmap.h" // PAGE_SIZE
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "romfile.h" // romfile_findprefix
#অন্তর্ভাব "stacks.h" // call16_int
#অন্তর্ভাব "std/disk.h" // DISK_RET_SUCCESS
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // process_ramdisk_op

শূন্য
ramdisk_setup(শূন্য)
{
    যদি (!CONFIG_FLASH_FLOPPY)
        ফেরত;

    // Find image.
    কাঠামো romfile_s *file = romfile_findprefix("floppyimg/", ফাঁকা);
    যদি (!file)
        ফেরত;
    ধ্রুবক অক্ষর *filename = file->name;
    u32 আকার_2 = file->আকার_2;
    dprintf(3, "Found floppy file %s of size %d\n", filename, আকার_2);
    পূর্ণসংখ্যা ftype = find_floppy_type(আকার_2);
    যদি (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        ফেরত;
    }

    // Allocate ram for image.
    শূন্য *pos = memalign_tmphigh(PAGE_আকার, আকার_2);
    যদি (!pos) {
        warn_noalloc();
        ফেরত;
    }
    e820_add((u32)pos, আকার_2, E820_RESERVED);

    // Copy image into ram.
    পূর্ণসংখ্যা ret = file->copy(file, pos, আকার_2);
    যদি (ret < 0)
        ফেরত;

    // Setup driver.
    কাঠামো drive_s *drive = init_floppy((u32)pos, ftype);
    যদি (!drive)
        ফেরত;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    অক্ষর *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_add_floppy(drive, desc, bootprio_find_named_rom(filename, 0));
}

স্থির পূর্ণসংখ্যা
ramdisk_copy(কাঠামো disk_op_s *op, পূর্ণসংখ্যা iswrite)
{
    u32 offset = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    offset += (u32)op->lba * DISK_SECTOR_আকার;
    u64 opd = GDT_তথ্য | GDT_LIMIT(0xfffff) | GDT_BASE((u32)op->buf_fl);
    u64 ramd = GDT_তথ্য | GDT_LIMIT(0xfffff) | GDT_BASE(offset);

    u64 gdt[6];
    যদি (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } অন্যথা {
        gdt[2] = ramd;
        gdt[3] = opd;
    }

    // Call int 1587 to copy data.
    কাঠামো bregs br;
    memset(&br, 0, আকার(br));
    br.flags = F_CF|F_IF;
    br.ah = 0x87;
    br.es = GET_SEG(SS);
    br.si = (u32)gdt;
    br.cx = op->গুনতি * DISK_SECTOR_আকার / 2;
    call16_int(0x15, &br);

    যদি (br.flags & F_CF)
        ফেরত DISK_RET_EBADTRACK;
    ফেরত DISK_RET_SUCCESS;
}

পূর্ণসংখ্যা
ramdisk_process_op(কাঠামো disk_op_s *op)
{
    যদি (!CONFIG_FLASH_FLOPPY)
        ফেরত 0;

    নির্বাচন (op->command) {
    ক্ষেত্র CMD_READ:
        ফেরত ramdisk_copy(op, 0);
    ক্ষেত্র CMD_WRITE:
        ফেরত ramdisk_copy(op, 1);
    পূর্বনির্ধারিত:
        ফেরত default_process_op(op);
    }
}

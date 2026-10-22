// Code for emulating a drive via high-memory accesses.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "biosvar.h" // GET_GLOBALFLAT
#অন্তর্ভাৱ "block.h" // struct drive_s
#অন্তর্ভাৱ "bregs.h" // struct bregs
#অন্তর্ভাৱ "e820map.h" // e820_add
#অন্তর্ভাৱ "malloc.h" // memalign_tmphigh
#অন্তর্ভাৱ "memmap.h" // PAGE_SIZE
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "romfile.h" // romfile_findprefix
#অন্তর্ভাৱ "stacks.h" // call16_int
#অন্তর্ভাৱ "std/disk.h" // DISK_RET_SUCCESS
#অন্তর্ভাৱ "string.h" // memset
#অন্তর্ভাৱ "util.h" // process_ramdisk_op

শূন্য
ramdisk_setup(শূন্য)
{
    যদি (!CONFIG_FLASH_FLOPPY)
        ঘূৰাই;

    // Find image.
    সংরচনা romfile_s *file = romfile_findprefix("floppyimg/", রিক্ত);
    যদি (!file)
        ঘূৰাই;
    ধ্রুৱ ৱর্ণ *filename = file->name;
    u32 size = file->size;
    dprintf(3, "Found floppy file %s of size %d\n", filename, size);
    পূর্ণাঙ্ক ftype = find_floppy_type(size);
    যদি (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        ঘূৰাই;
    }

    // Allocate ram for image.
    শূন্য *pos = memalign_tmphigh(PAGE_SIZE, size);
    যদি (!pos) {
        warn_noalloc();
        ঘূৰাই;
    }
    e820_add((u32)pos, size, E820_RESERVED);

    // Copy image into ram.
    পূর্ণাঙ্ক ret = file->copy(file, pos, size);
    যদি (ret < 0)
        ঘূৰাই;

    // Setup driver.
    সংরচনা drive_s *drive = init_floppy((u32)pos, ftype);
    যদি (!drive)
        ঘূৰাই;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    ৱর্ণ *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_add_floppy(drive, desc, bootprio_find_named_rom(filename, 0));
}

স্থির পূর্ণাঙ্ক
ramdisk_copy(সংরচনা disk_op_s *op, পূর্ণাঙ্ক iswrite)
{
    u32 offset = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    offset += (u32)op->lba * DISK_SECTOR_SIZE;
    u64 opd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE((u32)op->buf_fl);
    u64 ramd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE(offset);

    u64 gdt[6];
    যদি (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } অন্যথা {
        gdt[2] = ramd;
        gdt[3] = opd;
    }

    // Call int 1587 to copy data.
    সংরচনা bregs br;
    memset(&br, 0, পরিমাণ(br));
    br.flags = F_CF|F_IF;
    br.ah = 0x87;
    br.es = GET_SEG(SS);
    br.si = (u32)gdt;
    br.cx = op->গুনতি * DISK_SECTOR_SIZE / 2;
    call16_int(0x15, &br);

    যদি (br.flags & F_CF)
        ঘূৰাই DISK_RET_EBADTRACK;
    ঘূৰাই DISK_RET_SUCCESS;
}

পূর্ণাঙ্ক
ramdisk_process_op(সংরচনা disk_op_s *op)
{
    যদি (!CONFIG_FLASH_FLOPPY)
        ঘূৰাই 0;

    ৱিকল্প (op->command) {
    পক্ষ CMD_READ:
        ঘূৰাই ramdisk_copy(op, 0);
    পক্ষ CMD_WRITE:
        ঘূৰাই ramdisk_copy(op, 1);
    স্ৱভাৱ:
        ঘূৰাই default_process_op(op);
    }
}

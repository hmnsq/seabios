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
ramdisk_تياري(فراغ)
{
    إذا (!CONFIG_FLASH_FLOPPY)
        أرجع;

    // Find image.
    بنية romfile_s *file = romfile_findprefix("floppyimg/", عدم);
    إذا (!file)
        أرجع;
    ثابت حرف *filename = file->name;
    u32 سائيز = file->سائيز;
    dprintf(3, "Found floppy file %s of size %d\n", filename, سائيز);
    صحيح ftype = ڳوليو_floppy_type(سائيز);
    إذا (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        أرجع;
    }

    // Allocate ram for image.
    فراغ *pos = memalign_tmphigh(صفحو_سائيز, سائيز);
    إذا (!pos) {
        warn_noalloc();
        أرجع;
    }
    e820_جوڙيو((u32)pos, سائيز, E820_RESERVED);

    // Copy image into ram.
    صحيح ret = file->copy(file, pos, سائيز);
    إذا (ret < 0)
        أرجع;

    // Setup driver.
    بنية drive_s *drive = init_floppy((u32)pos, ftype);
    إذا (!drive)
        أرجع;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    حرف *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_جوڙيو_floppy(drive, desc, bootprio_ڳوليو_named_rom(filename, 0));
}

ساكن صحيح
ramdisk_copy(بنية disk_op_s *op, صحيح iswrite)
{
    u32 وٿي_2 = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    وٿي_2 += (u32)op->lba * DISK_SECTOR_سائيز;
    u64 opd = GDT_معلومات | GDT_حد_بندي(0xfffff) | GDT_بنياد((u32)op->buf_fl);
    u64 ramd = GDT_معلومات | GDT_حد_بندي(0xfffff) | GDT_بنياد(وٿي_2);

    u64 gdt[6];
    إذا (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } وإلا {
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
    br.cx = op->ڳڻپ * DISK_SECTOR_سائيز / 2;
    call16_int(0x15, &br);

    إذا (br.flags & F_CF)
        أرجع DISK_RET_EBADTRACK;
    أرجع DISK_RET_SUCCESS;
}

صحيح
ramdisk_process_op(بنية disk_op_s *op)
{
    إذا (!CONFIG_FLASH_FLOPPY)
        أرجع 0;

    بدل (op->command) {
    حالة CMD_READ:
        أرجع ramdisk_copy(op, 0);
    حالة CMD_WRITE:
        أرجع ramdisk_copy(op, 1);
    افتراضي:
        أرجع default_process_op(op);
    }
}

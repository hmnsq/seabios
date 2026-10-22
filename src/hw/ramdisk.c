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

خالی
ramdisk_تیاری(خالی)
{
    اگر (!CONFIG_FLASH_FLOPPY)
        واپس;

    // Find image.
    ساخت romfile_s *file = romfile_findprefix("floppyimg/", تہی);
    اگر (!file)
        واپس;
    ثابت حرف *filename = file->name;
    u32 سائز = file->سائز;
    dprintf(3, "Found floppy file %s of size %d\n", filename, سائز);
    صحیح ftype = ڈھونڈو_floppy_type(سائز);
    اگر (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        واپس;
    }

    // Allocate ram for image.
    خالی *pos = memalign_tmphigh(صفحہ_سائز, سائز);
    اگر (!pos) {
        warn_noalloc();
        واپس;
    }
    e820_جوڑو((u32)pos, سائز, E820_RESERVED);

    // Copy image into ram.
    صحیح ret = file->copy(file, pos, سائز);
    اگر (ret < 0)
        واپس;

    // Setup driver.
    ساخت drive_s *drive = init_floppy((u32)pos, ftype);
    اگر (!drive)
        واپس;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    حرف *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_جوڑو_floppy(drive, desc, bootprio_ڈھونڈو_named_rom(filename, 0));
}

ساکن صحیح
ramdisk_copy(ساخت disk_op_s *op, صحیح iswrite)
{
    u32 انحراف_2 = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    انحراف_2 += (u32)op->lba * DISK_SECTOR_سائز;
    u64 opd = GDT_ڈیٹا | GDT_حد_بندی(0xfffff) | GDT_بنیاد((u32)op->buf_fl);
    u64 ramd = GDT_ڈیٹا | GDT_حد_بندی(0xfffff) | GDT_بنیاد(انحراف_2);

    u64 gdt[6];
    اگر (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } ورنہ {
        gdt[2] = ramd;
        gdt[3] = opd;
    }

    // Call int 1587 to copy data.
    ساخت bregs br;
    memset(&br, 0, حجم(br));
    br.flags = F_CF|F_IF;
    br.ah = 0x87;
    br.es = GET_SEG(SS);
    br.si = (u32)gdt;
    br.cx = op->گنتی * DISK_SECTOR_سائز / 2;
    call16_int(0x15, &br);

    اگر (br.flags & F_CF)
        واپس DISK_RET_EBADTRACK;
    واپس DISK_RET_SUCCESS;
}

صحیح
ramdisk_process_op(ساخت disk_op_s *op)
{
    اگر (!CONFIG_FLASH_FLOPPY)
        واپس 0;

    انتخاب (op->command) {
    حالت CMD_READ:
        واپس ramdisk_copy(op, 0);
    حالت CMD_WRITE:
        واپس ramdisk_copy(op, 1);
    طےشدہ:
        واپس default_process_op(op);
    }
}

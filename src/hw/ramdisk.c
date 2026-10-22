// Code for emulating a drive via high-memory accesses.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBALFLAT
#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // struct drive_s
#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "e820map.h" // e820_add
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // memalign_tmphigh
#ਅਨ੍ਤਰ੍ਭਾਵ "memmap.h" // PAGE_SIZE
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_findprefix
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // call16_int
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // DISK_RET_SUCCESS
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // process_ramdisk_op

ਖਾਲੀ
ramdisk_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_FLASH_FLOPPY)
        ਵਾਪਸ;

    // Find image.
    ਬਣਤਰ romfile_s *file = romfile_findprefix("floppyimg/", ਨਿਰਾ);
    ਜੇ (!file)
        ਵਾਪਸ;
    ਅਚਲ ਅੱਖਰ *filename = file->name;
    u32 ਮਾਪ = file->ਮਾਪ;
    dprintf(3, "Found floppy file %s of size %d\n", filename, ਮਾਪ);
    ਪੂਰਨ ftype = ਲੱਭੋ_floppy_type(ਮਾਪ);
    ਜੇ (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        ਵਾਪਸ;
    }

    // Allocate ram for image.
    ਖਾਲੀ *pos = memalign_tmphigh(ਪੰਨਾ_ਮਾਪ, ਮਾਪ);
    ਜੇ (!pos) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    e820_ਜੋੜੋ((u32)pos, ਮਾਪ, E820_RESERVED);

    // Copy image into ram.
    ਪੂਰਨ ret = file->copy(file, pos, ਮਾਪ);
    ਜੇ (ret < 0)
        ਵਾਪਸ;

    // Setup driver.
    ਬਣਤਰ drive_s *drive = init_floppy((u32)pos, ftype);
    ਜੇ (!drive)
        ਵਾਪਸ;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    ਅੱਖਰ *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_ਜੋੜੋ_floppy(drive, desc, bootprio_ਲੱਭੋ_named_rom(filename, 0));
}

ਸਥਿਰ ਪੂਰਨ
ramdisk_copy(ਬਣਤਰ disk_op_s *op, ਪੂਰਨ iswrite)
{
    u32 ਵਿੱਥ_2 = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    ਵਿੱਥ_2 += (u32)op->lba * DISK_SECTOR_ਮਾਪ;
    u64 opd = GDT_ਅੰਕੜੇ | GDT_ਹੱਦਬੰਦੀ(0xfffff) | GDT_ਅਧਾਰ((u32)op->buf_fl);
    u64 ramd = GDT_ਅੰਕੜੇ | GDT_ਹੱਦਬੰਦੀ(0xfffff) | GDT_ਅਧਾਰ(ਵਿੱਥ_2);

    u64 gdt[6];
    ਜੇ (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } ਨਹੀਂਤਾਂ {
        gdt[2] = ramd;
        gdt[3] = opd;
    }

    // Call int 1587 to copy data.
    ਬਣਤਰ bregs br;
    memset(&br, 0, ਆਕਾਰ(br));
    br.flags = F_CF|F_IF;
    br.ah = 0x87;
    br.es = GET_SEG(SS);
    br.si = (u32)gdt;
    br.cx = op->ਗਿਣਤੀ_2 * DISK_SECTOR_ਮਾਪ / 2;
    call16_int(0x15, &br);

    ਜੇ (br.flags & F_CF)
        ਵਾਪਸ DISK_RET_EBADTRACK;
    ਵਾਪਸ DISK_RET_SUCCESS;
}

ਪੂਰਨ
ramdisk_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (!CONFIG_FLASH_FLOPPY)
        ਵਾਪਸ 0;

    ਚੋਣ (op->command) {
    ਹਾਲਤ CMD_READ:
        ਵਾਪਸ ramdisk_copy(op, 0);
    ਹਾਲਤ CMD_WRITE:
        ਵਾਪਸ ramdisk_copy(op, 1);
    ਮੂਲ:
        ਵਾਪਸ default_process_op(op);
    }
}

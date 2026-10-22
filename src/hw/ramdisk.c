// Code for emulating a drive via high-memory accesses.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_GLOBALFLAT
#അന്തര്ഭാവ "block.h" // struct drive_s
#അന്തര്ഭാവ "bregs.h" // struct bregs
#അന്തര്ഭാവ "e820map.h" // e820_add
#അന്തര്ഭാവ "malloc.h" // memalign_tmphigh
#അന്തര്ഭാവ "memmap.h" // PAGE_SIZE
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "romfile.h" // romfile_findprefix
#അന്തര്ഭാവ "stacks.h" // call16_int
#അന്തര്ഭാവ "std/disk.h" // DISK_RET_SUCCESS
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // process_ramdisk_op

ശൂന്യം
ramdisk_setup(ശൂന്യം)
{
    എങ്കിൽ (!CONFIG_FLASH_FLOPPY)
        തിരികെ;

    // Find image.
    ഘടന romfile_s *file = romfile_findprefix("floppyimg/", ശൂന്യ);
    എങ്കിൽ (!file)
        തിരികെ;
    സ്ഥിരം അക്ഷരം *filename = file->name;
    u32 size = file->size;
    dprintf(3, "Found floppy file %s of size %d\n", filename, size);
    പൂർണ്ണസംഖ്യ ftype = find_floppy_type(size);
    എങ്കിൽ (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        തിരികെ;
    }

    // Allocate ram for image.
    ശൂന്യം *pos = memalign_tmphigh(PAGE_SIZE, size);
    എങ്കിൽ (!pos) {
        warn_noalloc();
        തിരികെ;
    }
    e820_add((u32)pos, size, E820_RESERVED);

    // Copy image into ram.
    പൂർണ്ണസംഖ്യ ret = file->copy(file, pos, size);
    എങ്കിൽ (ret < 0)
        തിരികെ;

    // Setup driver.
    ഘടന drive_s *drive = init_floppy((u32)pos, ftype);
    എങ്കിൽ (!drive)
        തിരികെ;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    അക്ഷരം *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_add_floppy(drive, desc, bootprio_find_named_rom(filename, 0));
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
ramdisk_copy(ഘടന disk_op_s *op, പൂർണ്ണസംഖ്യ iswrite)
{
    u32 offset = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    offset += (u32)op->lba * DISK_SECTOR_SIZE;
    u64 opd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE((u32)op->buf_fl);
    u64 ramd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE(offset);

    u64 gdt[6];
    എങ്കിൽ (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } അല്ലെങ്കിൽ {
        gdt[2] = ramd;
        gdt[3] = opd;
    }

    // Call int 1587 to copy data.
    ഘടന bregs br;
    memset(&br, 0, വലുപ്പം(br));
    br.flags = F_CF|F_IF;
    br.ah = 0x87;
    br.es = GET_SEG(SS);
    br.si = (u32)gdt;
    br.cx = op->എണ്ണം * DISK_SECTOR_SIZE / 2;
    call16_int(0x15, &br);

    എങ്കിൽ (br.flags & F_CF)
        തിരികെ DISK_RET_EBADTRACK;
    തിരികെ DISK_RET_SUCCESS;
}

പൂർണ്ണസംഖ്യ
ramdisk_process_op(ഘടന disk_op_s *op)
{
    എങ്കിൽ (!CONFIG_FLASH_FLOPPY)
        തിരികെ 0;

    തിരഞ്ഞെടുപ്പ് (op->command) {
    അവസ്ഥ CMD_READ:
        തിരികെ ramdisk_copy(op, 0);
    അവസ്ഥ CMD_WRITE:
        തിരികെ ramdisk_copy(op, 1);
    സ്ഥിരസ്ഥിതി:
        തിരികെ default_process_op(op);
    }
}

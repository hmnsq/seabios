// Code for emulating a drive via high-memory accesses.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // GET_GLOBALFLAT
#அந்தர்பாவ "block.h" // struct drive_s
#அந்தர்பாவ "bregs.h" // struct bregs
#அந்தர்பாவ "e820map.h" // e820_add
#அந்தர்பாவ "malloc.h" // memalign_tmphigh
#அந்தர்பாவ "memmap.h" // PAGE_SIZE
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "romfile.h" // romfile_findprefix
#அந்தர்பாவ "stacks.h" // call16_int
#அந்தர்பாவ "std/disk.h" // DISK_RET_SUCCESS
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // process_ramdisk_op

வெறுமை
ramdisk_setup(வெறுமை)
{
    எனில் (!CONFIG_FLASH_FLOPPY)
        திரும்பு;

    // Find image.
    கட்டமைப்பு romfile_s *file = romfile_findprefix("floppyimg/", வெற்று);
    எனில் (!file)
        திரும்பு;
    மாறிலி எழுத்து *filename = file->name;
    u32 size = file->size;
    dprintf(3, "Found floppy file %s of size %d\n", filename, size);
    முழு ftype = find_floppy_type(size);
    எனில் (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        திரும்பு;
    }

    // Allocate ram for image.
    வெறுமை *pos = memalign_tmphigh(PAGE_SIZE, size);
    எனில் (!pos) {
        warn_noalloc();
        திரும்பு;
    }
    e820_add((u32)pos, size, E820_RESERVED);

    // Copy image into ram.
    முழு ret = file->copy(file, pos, size);
    எனில் (ret < 0)
        திரும்பு;

    // Setup driver.
    கட்டமைப்பு drive_s *drive = init_floppy((u32)pos, ftype);
    எனில் (!drive)
        திரும்பு;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    எழுத்து *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_add_floppy(drive, desc, bootprio_find_named_rom(filename, 0));
}

நிலையான முழு
ramdisk_copy(கட்டமைப்பு disk_op_s *op, முழு iswrite)
{
    u32 offset = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    offset += (u32)op->lba * DISK_SECTOR_SIZE;
    u64 opd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE((u32)op->buf_fl);
    u64 ramd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE(offset);

    u64 gdt[6];
    எனில் (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } இல்லையெனில் {
        gdt[2] = ramd;
        gdt[3] = opd;
    }

    // Call int 1587 to copy data.
    கட்டமைப்பு bregs br;
    memset(&br, 0, அளவு(br));
    br.flags = F_CF|F_IF;
    br.ah = 0x87;
    br.es = GET_SEG(SS);
    br.si = (u32)gdt;
    br.cx = op->மொத்தம் * DISK_SECTOR_SIZE / 2;
    call16_int(0x15, &br);

    எனில் (br.flags & F_CF)
        திரும்பு DISK_RET_EBADTRACK;
    திரும்பு DISK_RET_SUCCESS;
}

முழு
ramdisk_process_op(கட்டமைப்பு disk_op_s *op)
{
    எனில் (!CONFIG_FLASH_FLOPPY)
        திரும்பு 0;

    தேர்வு (op->command) {
    நிலை CMD_READ:
        திரும்பு ramdisk_copy(op, 0);
    நிலை CMD_WRITE:
        திரும்பு ramdisk_copy(op, 1);
    இயல்பு:
        திரும்பு default_process_op(op);
    }
}

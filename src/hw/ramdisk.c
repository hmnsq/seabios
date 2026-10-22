// Code for emulating a drive via high-memory accesses.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // GET_GLOBALFLAT
#અન્તર્ભાવ "block.h" // struct drive_s
#અન્તર્ભાવ "bregs.h" // struct bregs
#અન્તર્ભાવ "e820map.h" // e820_add
#અન્તર્ભાવ "malloc.h" // memalign_tmphigh
#અન્તર્ભાવ "memmap.h" // PAGE_SIZE
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "romfile.h" // romfile_findprefix
#અન્તર્ભાવ "stacks.h" // call16_int
#અન્તર્ભાવ "std/disk.h" // DISK_RET_SUCCESS
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // process_ramdisk_op

શૂન્ય
ramdisk_setup(શૂન્ય)
{
    જો (!CONFIG_FLASH_FLOPPY)
        પાછા;

    // Find image.
    સંરચના romfile_s *file = romfile_findprefix("floppyimg/", ખાલી);
    જો (!file)
        પાછા;
    અચલ અક્ષર *filename = file->name;
    u32 size = file->size;
    dprintf(3, "Found floppy file %s of size %d\n", filename, size);
    પૂર્ણાંક ftype = find_floppy_type(size);
    જો (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        પાછા;
    }

    // Allocate ram for image.
    શૂન્ય *pos = memalign_tmphigh(PAGE_SIZE, size);
    જો (!pos) {
        warn_noalloc();
        પાછા;
    }
    e820_add((u32)pos, size, E820_RESERVED);

    // Copy image into ram.
    પૂર્ણાંક ret = file->copy(file, pos, size);
    જો (ret < 0)
        પાછા;

    // Setup driver.
    સંરચના drive_s *drive = init_floppy((u32)pos, ftype);
    જો (!drive)
        પાછા;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    અક્ષર *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_add_floppy(drive, desc, bootprio_find_named_rom(filename, 0));
}

સ્થિર પૂર્ણાંક
ramdisk_copy(સંરચના disk_op_s *op, પૂર્ણાંક iswrite)
{
    u32 offset = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    offset += (u32)op->lba * DISK_SECTOR_SIZE;
    u64 opd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE((u32)op->buf_fl);
    u64 ramd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE(offset);

    u64 gdt[6];
    જો (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } નહીંતર {
        gdt[2] = ramd;
        gdt[3] = opd;
    }

    // Call int 1587 to copy data.
    સંરચના bregs br;
    memset(&br, 0, કદ(br));
    br.flags = F_CF|F_IF;
    br.ah = 0x87;
    br.es = GET_SEG(SS);
    br.si = (u32)gdt;
    br.cx = op->ગણતરી * DISK_SECTOR_SIZE / 2;
    call16_int(0x15, &br);

    જો (br.flags & F_CF)
        પાછા DISK_RET_EBADTRACK;
    પાછા DISK_RET_SUCCESS;
}

પૂર્ણાંક
ramdisk_process_op(સંરચના disk_op_s *op)
{
    જો (!CONFIG_FLASH_FLOPPY)
        પાછા 0;

    પસંદગી (op->command) {
    સ્થિતિ CMD_READ:
        પાછા ramdisk_copy(op, 0);
    સ્થિતિ CMD_WRITE:
        પાછા ramdisk_copy(op, 1);
    મૂળભૂત:
        પાછા default_process_op(op);
    }
}

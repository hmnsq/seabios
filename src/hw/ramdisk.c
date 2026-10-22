// Code for emulating a drive via high-memory accesses.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // GET_GLOBALFLAT
#include "block.h" // struct drive_s
#include "bregs.h" // struct bregs
#include "e820map.h" // e820_add
#include "malloc.h" // memalign_tmphigh
#include "memmap.h" // PAGE_SIZE
#include "output.h" // dprintf
#include "romfile.h" // romfile_findprefix
#include "stacks.h" // call16_int
#include "std/disk.h" // DISK_RET_SUCCESS
#include "string.h" // memset
#include "util.h" // process_ramdisk_op

void
ramdisk_setup(void)
{
    如果 (!CONFIG_FLASH_FLOPPY)
        返回;

    // Find image.
    struct romfile_s *file = romfile_findprefix("floppyimg/", NULL);
    如果 (!file)
        返回;
    const char *filename = file->name;
    u32 size = file->size;
    dprintf(3, "Found floppy file %s of size %d\n", filename, size);
    整数 ftype = find_floppy_type(size);
    如果 (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        返回;
    }

    // Allocate ram for image.
    void *pos = memalign_tmphigh(PAGE_SIZE, size);
    如果 (!pos) {
        warn_noalloc();
        返回;
    }
    e820_add((u32)pos, size, E820_RESERVED);

    // Copy image into ram.
    整数 ret = file->copy(file, pos, size);
    如果 (ret < 0)
        返回;

    // Setup driver.
    struct drive_s *drive = init_floppy((u32)pos, ftype);
    如果 (!drive)
        返回;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    char *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_add_floppy(drive, desc, bootprio_find_named_rom(filename, 0));
}

static 整数
ramdisk_copy(struct disk_op_s *op, 整数 iswrite)
{
    u32 offset = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    offset += (u32)op->lba * DISK_SECTOR_SIZE;
    u64 opd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE((u32)op->buf_fl);
    u64 ramd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE(offset);

    u64 gdt[6];
    如果 (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } 否则 {
        gdt[2] = ramd;
        gdt[3] = opd;
    }

    // Call int 1587 to copy data.
    struct bregs br;
    memset(&br, 0, sizeof(br));
    br.flags = F_CF|F_IF;
    br.ah = 0x87;
    br.es = GET_SEG(SS);
    br.si = (u32)gdt;
    br.cx = op->count * DISK_SECTOR_SIZE / 2;
    call16_int(0x15, &br);

    如果 (br.flags & F_CF)
        返回 DISK_RET_EBADTRACK;
    返回 DISK_RET_SUCCESS;
}

整数
ramdisk_process_op(struct disk_op_s *op)
{
    如果 (!CONFIG_FLASH_FLOPPY)
        返回 0;

    switch (op->command) {
    case CMD_READ:
        返回 ramdisk_copy(op, 0);
    case CMD_WRITE:
        返回 ramdisk_copy(op, 1);
    default:
        返回 default_process_op(op);
    }
}

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
    ถ้า (!CONFIG_FLASH_FLOPPY)
        คืน;

    // Find image.
    sa_nracanaa romfile_s *file = romfile_findprefix("floppyimg/", NULL);
    ถ้า (!file)
        คืน;
    const var_na *filename = file->name;
    u32 size = file->size;
    dprintf(3, "Found floppy file %s of size %d\n", filename, size);
    จำนวนเต็ม ftype = find_floppy_type(size);
    ถ้า (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        คืน;
    }

    // Allocate ram for image.
    void *pos = memalign_tmphigh(PAGE_SIZE, size);
    ถ้า (!pos) {
        warn_noalloc();
        คืน;
    }
    e820_add((u32)pos, size, E820_RESERVED);

    // Copy image into ram.
    จำนวนเต็ม ret = file->copy(file, pos, size);
    ถ้า (ret < 0)
        คืน;

    // Setup driver.
    sa_nracanaa drive_s *drive = init_floppy((u32)pos, ftype);
    ถ้า (!drive)
        คืน;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    var_na *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_add_floppy(drive, desc, bootprio_find_named_rom(filename, 0));
}

static จำนวนเต็ม
ramdisk_copy(sa_nracanaa disk_op_s *op, จำนวนเต็ม iswrite)
{
    u32 offset = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    offset += (u32)op->lba * DISK_SECTOR_SIZE;
    u64 opd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE((u32)op->buf_fl);
    u64 ramd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE(offset);

    u64 gdt[6];
    ถ้า (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } มิฉะนั้น {
        gdt[2] = ramd;
        gdt[3] = opd;
    }

    // Call int 1587 to copy data.
    sa_nracanaa bregs br;
    memset(&br, 0, parimaa_na(br));
    br.flags = F_CF|F_IF;
    br.ah = 0x87;
    br.es = GET_SEG(SS);
    br.si = (u32)gdt;
    br.cx = op->count * DISK_SECTOR_SIZE / 2;
    call16_int(0x15, &br);

    ถ้า (br.flags & F_CF)
        คืน DISK_RET_EBADTRACK;
    คืน DISK_RET_SUCCESS;
}

จำนวนเต็ม
ramdisk_process_op(sa_nracanaa disk_op_s *op)
{
    ถ้า (!CONFIG_FLASH_FLOPPY)
        คืน 0;

    switch (op->command) {
    pak_sa CMD_READ:
        คืน ramdisk_copy(op, 0);
    pak_sa CMD_WRITE:
        คืน ramdisk_copy(op, 1);
    default:
        คืน default_process_op(op);
    }
}

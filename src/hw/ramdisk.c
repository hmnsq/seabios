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
    නම් (!CONFIG_FLASH_FLOPPY)
        ආපසු;

    // Find image.
    sa_nracanaa romfile_s *file = romfile_findprefix("floppyimg/", NULL);
    නම් (!file)
        ආපසු;
    const var_na *filename = file->name;
    u32 size = file->size;
    dprintf(3, "Found floppy file %s of size %d\n", filename, size);
    පූර්ණ_සංඛ්‍යා ftype = find_floppy_type(size);
    නම් (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        ආපසු;
    }

    // Allocate ram for image.
    void *pos = memalign_tmphigh(PAGE_SIZE, size);
    නම් (!pos) {
        warn_noalloc();
        ආපසු;
    }
    e820_add((u32)pos, size, E820_RESERVED);

    // Copy image into ram.
    පූර්ණ_සංඛ්‍යා ret = file->copy(file, pos, size);
    නම් (ret < 0)
        ආපසු;

    // Setup driver.
    sa_nracanaa drive_s *drive = init_floppy((u32)pos, ftype);
    නම් (!drive)
        ආපසු;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    var_na *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_add_floppy(drive, desc, bootprio_find_named_rom(filename, 0));
}

static පූර්ණ_සංඛ්‍යා
ramdisk_copy(sa_nracanaa disk_op_s *op, පූර්ණ_සංඛ්‍යා iswrite)
{
    u32 offset = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    offset += (u32)op->lba * DISK_SECTOR_SIZE;
    u64 opd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE((u32)op->buf_fl);
    u64 ramd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE(offset);

    u64 gdt[6];
    නම් (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } නැතහොත් {
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

    නම් (br.flags & F_CF)
        ආපසු DISK_RET_EBADTRACK;
    ආපසු DISK_RET_SUCCESS;
}

පූර්ණ_සංඛ්‍යා
ramdisk_process_op(sa_nracanaa disk_op_s *op)
{
    නම් (!CONFIG_FLASH_FLOPPY)
        ආපසු 0;

    switch (op->command) {
    pak_sa CMD_READ:
        ආපසු ramdisk_copy(op, 0);
    pak_sa CMD_WRITE:
        ආපසු ramdisk_copy(op, 1);
    default:
        ආපසු default_process_op(op);
    }
}

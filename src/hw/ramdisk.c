// Code for emulating a drive via high-memory accesses.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_GLOBALFLAT
#समावेश "block.h" // struct drive_s
#समावेश "bregs.h" // struct bregs
#समावेश "e820map.h" // e820_add
#समावेश "malloc.h" // memalign_tmphigh
#समावेश "memmap.h" // PAGE_SIZE
#समावेश "output.h" // dprintf
#समावेश "romfile.h" // romfile_findprefix
#समावेश "stacks.h" // call16_int
#समावेश "std/disk.h" // DISK_RET_SUCCESS
#समावेश "string.h" // memset
#समावेश "util.h" // process_ramdisk_op

व्योम
ramdisk_setup(व्योम)
{
    अगर (!CONFIG_FLASH_FLOPPY)
        वापस;

    // Find image.
    काष्ठा romfile_s *file = romfile_findprefix("floppyimg/", शून्य);
    अगर (!file)
        वापस;
    स्थिर अक्षर *filename = file->name;
    u32 आकार = file->आकार;
    dprintf(3, "Found floppy file %s of size %d\n", filename, आकार);
    पूर्णांक ftype = find_floppy_type(आकार);
    अगर (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        वापस;
    }

    // Allocate ram for image.
    व्योम *pos = memalign_tmphigh(PAGE_आकार, आकार);
    अगर (!pos) {
        warn_noalloc();
        वापस;
    }
    e820_add((u32)pos, आकार, E820_RESERVED);

    // Copy image into ram.
    पूर्णांक ret = file->copy(file, pos, आकार);
    अगर (ret < 0)
        वापस;

    // Setup driver.
    काष्ठा drive_s *drive = init_floppy((u32)pos, ftype);
    अगर (!drive)
        वापस;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    अक्षर *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_add_floppy(drive, desc, bootprio_find_named_rom(filename, 0));
}

अटल पूर्णांक
ramdisk_copy(काष्ठा disk_op_s *op, पूर्णांक iswrite)
{
    u32 offset = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    offset += (u32)op->lba * DISK_SECTOR_आकार;
    u64 opd = GDT_तथ्य | GDT_LIMIT(0xfffff) | GDT_BASE((u32)op->buf_fl);
    u64 ramd = GDT_तथ्य | GDT_LIMIT(0xfffff) | GDT_BASE(offset);

    u64 gdt[6];
    अगर (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } अन्यथा {
        gdt[2] = ramd;
        gdt[3] = opd;
    }

    // Call int 1587 to copy data.
    काष्ठा bregs br;
    स_रखो(&br, 0, माप(br));
    br.flags = F_CF|F_IF;
    br.ah = 0x87;
    br.es = GET_SEG(SS);
    br.si = (u32)gdt;
    br.cx = op->गिनती * DISK_SECTOR_आकार / 2;
    call16_int(0x15, &br);

    अगर (br.flags & F_CF)
        वापस DISK_RET_EBADTRACK;
    वापस DISK_RET_SUCCESS;
}

पूर्णांक
ramdisk_process_op(काष्ठा disk_op_s *op)
{
    अगर (!CONFIG_FLASH_FLOPPY)
        वापस 0;

    चयन (op->command) {
    हाल CMD_READ:
        वापस ramdisk_copy(op, 0);
    हाल CMD_WRITE:
        वापस ramdisk_copy(op, 1);
    शेष:
        वापस default_process_op(op);
    }
}

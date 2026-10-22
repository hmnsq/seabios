// Code for emulating a drive via high-memory accesses.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_GLOBALFLAT
#ଅନ୍ତର୍ଭାଵ "block.h" // struct drive_s
#ଅନ୍ତର୍ଭାଵ "bregs.h" // struct bregs
#ଅନ୍ତର୍ଭାଵ "e820map.h" // e820_add
#ଅନ୍ତର୍ଭାଵ "malloc.h" // memalign_tmphigh
#ଅନ୍ତର୍ଭାଵ "memmap.h" // PAGE_SIZE
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "romfile.h" // romfile_findprefix
#ଅନ୍ତର୍ଭାଵ "stacks.h" // call16_int
#ଅନ୍ତର୍ଭାଵ "std/disk.h" // DISK_RET_SUCCESS
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // process_ramdisk_op

ଶୂନ୍ୟ
ramdisk_setup(ଶୂନ୍ୟ)
{
    ଯଦି (!CONFIG_FLASH_FLOPPY)
        ଫେରନ୍ତୁ;

    // Find image.
    ସଂରଚନା romfile_s *file = romfile_findprefix("floppyimg/", ଖାଲି);
    ଯଦି (!file)
        ଫେରନ୍ତୁ;
    ଅଚଳ ଅକ୍ଷର *filename = file->name;
    u32 size = file->size;
    dprintf(3, "Found floppy file %s of size %d\n", filename, size);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ftype = find_floppy_type(size);
    ଯଦି (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        ଫେରନ୍ତୁ;
    }

    // Allocate ram for image.
    ଶୂନ୍ୟ *pos = memalign_tmphigh(PAGE_SIZE, size);
    ଯଦି (!pos) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    e820_add((u32)pos, size, E820_RESERVED);

    // Copy image into ram.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = file->copy(file, pos, size);
    ଯଦି (ret < 0)
        ଫେରନ୍ତୁ;

    // Setup driver.
    ସଂରଚନା drive_s *drive = init_floppy((u32)pos, ftype);
    ଯଦି (!drive)
        ଫେରନ୍ତୁ;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    ଅକ୍ଷର *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_add_floppy(drive, desc, bootprio_find_named_rom(filename, 0));
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ramdisk_copy(ସଂରଚନା disk_op_s *op, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iswrite)
{
    u32 offset = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    offset += (u32)op->lba * DISK_SECTOR_SIZE;
    u64 opd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE((u32)op->buf_fl);
    u64 ramd = GDT_DATA | GDT_LIMIT(0xfffff) | GDT_BASE(offset);

    u64 gdt[6];
    ଯଦି (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } ଅନ୍ୟଥା {
        gdt[2] = ramd;
        gdt[3] = opd;
    }

    // Call int 1587 to copy data.
    ସଂରଚନା bregs br;
    memset(&br, 0, ଆକାର(br));
    br.flags = F_CF|F_IF;
    br.ah = 0x87;
    br.es = GET_SEG(SS);
    br.si = (u32)gdt;
    br.cx = op->ଗଣନା_2 * DISK_SECTOR_SIZE / 2;
    call16_int(0x15, &br);

    ଯଦି (br.flags & F_CF)
        ଫେରନ୍ତୁ DISK_RET_EBADTRACK;
    ଫେରନ୍ତୁ DISK_RET_SUCCESS;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ramdisk_process_op(ସଂରଚନା disk_op_s *op)
{
    ଯଦି (!CONFIG_FLASH_FLOPPY)
        ଫେରନ୍ତୁ 0;

    ବାଛ (op->command) {
    ସ୍ଥିତି CMD_READ:
        ଫେରନ୍ତୁ ramdisk_copy(op, 0);
    ସ୍ଥିତି CMD_WRITE:
        ଫେରନ୍ତୁ ramdisk_copy(op, 1);
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ଫେରନ୍ତୁ default_process_op(op);
    }
}

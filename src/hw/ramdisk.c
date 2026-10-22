// Code for emulating a drive via high-memory accesses.
//
// Copyright (C) 2009  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_GLOBALFLAT
#అన్తర్భావ "block.h" // struct drive_s
#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "e820map.h" // e820_add
#అన్తర్భావ "malloc.h" // memalign_tmphigh
#అన్తర్భావ "memmap.h" // PAGE_SIZE
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "romfile.h" // romfile_findprefix
#అన్తర్భావ "stacks.h" // call16_int
#అన్తర్భావ "std/disk.h" // DISK_RET_SUCCESS
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // process_ramdisk_op

శూన్య
ramdisk_అమరిక(శూన్య)
{
    ఒకవేళ (!CONFIG_FLASH_FLOPPY)
        తిరిగి;

    // Find image.
    నిర్మాణం romfile_s *file = romfile_findprefix("floppyimg/", ఖాళీ);
    ఒకవేళ (!file)
        తిరిగి;
    స్థిరరాశి అక్షరం *filename = file->name;
    u32 కొలత = file->కొలత;
    dprintf(3, "Found floppy file %s of size %d\n", filename, కొలత);
    పూర్ణాంకం ftype = కనుగొను_floppy_type(కొలత);
    ఒకవేళ (ftype < 0) {
        dprintf(3, "No floppy type found for ramdisk size\n");
        తిరిగి;
    }

    // Allocate ram for image.
    శూన్య *pos = memalign_tmphigh(పుట_కొలత, కొలత);
    ఒకవేళ (!pos) {
        warn_noalloc();
        తిరిగి;
    }
    e820_కూడు((u32)pos, కొలత, E820_RESERVED);

    // Copy image into ram.
    పూర్ణాంకం ret = file->copy(file, pos, కొలత);
    ఒకవేళ (ret < 0)
        తిరిగి;

    // Setup driver.
    నిర్మాణం drive_s *drive = init_floppy((u32)pos, ftype);
    ఒకవేళ (!drive)
        తిరిగి;
    drive->type = DTYPE_RAMDISK;
    dprintf(1, "Mapping floppy %s to addr %p\n", filename, pos);
    అక్షరం *desc = znprintf(MAXDESCSIZE, "Ramdisk [%s]", &filename[10]);
    boot_కూడు_floppy(drive, desc, bootprio_కనుగొను_named_rom(filename, 0));
}

స్థిర పూర్ణాంకం
ramdisk_copy(నిర్మాణం disk_op_s *op, పూర్ణాంకం iswrite)
{
    u32 స్థానభ్రంశం_2 = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    స్థానభ్రంశం_2 += (u32)op->lba * DISK_SECTOR_కొలత;
    u64 opd = GDT_దత్తాంశం | GDT_హద్దు(0xfffff) | GDT_పునాది((u32)op->buf_fl);
    u64 ramd = GDT_దత్తాంశం | GDT_హద్దు(0xfffff) | GDT_పునాది(స్థానభ్రంశం_2);

    u64 gdt[6];
    ఒకవేళ (iswrite) {
        gdt[2] = opd;
        gdt[3] = ramd;
    } లేకపోతే {
        gdt[2] = ramd;
        gdt[3] = opd;
    }

    // Call int 1587 to copy data.
    నిర్మాణం bregs br;
    memset(&br, 0, పరిమాణం(br));
    br.flags = F_CF|F_IF;
    br.ah = 0x87;
    br.es = GET_SEG(SS);
    br.si = (u32)gdt;
    br.cx = op->లెక్క * DISK_SECTOR_కొలత / 2;
    call16_int(0x15, &br);

    ఒకవేళ (br.flags & F_CF)
        తిరిగి DISK_RET_EBADTRACK;
    తిరిగి DISK_RET_SUCCESS;
}

పూర్ణాంకం
ramdisk_process_op(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (!CONFIG_FLASH_FLOPPY)
        తిరిగి 0;

    ఎంపిక (op->command) {
    స్థితి CMD_READ:
        తిరిగి ramdisk_copy(op, 0);
    స్థితి CMD_WRITE:
        తిరిగి ramdisk_copy(op, 1);
    అప్రమేయ:
        తిరిగి default_process_op(op);
    }
}

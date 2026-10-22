// 16bit code to access floppy drives.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // SET_BDA
#অন্তর্ভাব "block.h" // struct drive_s
#অন্তর্ভাব "bregs.h" // struct bregs
#অন্তর্ভাব "config.h" // CONFIG_FLOPPY
#অন্তর্ভাব "malloc.h" // malloc_fseg
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "pcidevice.h" // pci_find_class
#অন্তর্ভাব "pci_ids.h" // PCI_CLASS_BRIDGE_ISA
#অন্তর্ভাব "pic.h" // pic_eoi1
#অন্তর্ভাব "romfile.h" // romfile_loadint
#অন্তর্ভাব "rtc.h" // rtc_read
#অন্তর্ভাব "stacks.h" // yield
#অন্তর্ভাব "std/disk.h" // DISK_RET_SUCCESS
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // timer_calc

#define PORT_FD_BASE           0x03f0
#define PORT_FD_DOR            0x03f2
#define PORT_FD_STATUS         0x03f4
#define PORT_FD_তথ্য           0x03f5
#define PORT_FD_DIR            0x03f7

// Digital Input Register (read)
#define FLOPPY_DIR_DSKCHG      0x80

#define FLOPPY_আকার_CODE 0x02 // 512 byte sectors
#define FLOPPY_DATALEN 0xff   // Not used - because size code is 0x02
#define FLOPPY_MOTOR_TICKS 37 // ~2 seconds
#define FLOPPY_FILLBYTE 0xf6
#define FLOPPY_GAPLEN 0x1B
#define FLOPPY_FORMAT_GAPLEN 0x6c
#define FLOPPY_PIO_TIMEOUT 1000
#define FLOPPY_IRQ_TIMEOUT 5000
#define FLOPPY_SPECIFY1 0xAF  // step rate 12ms, head unload 240ms
#define FLOPPY_SPECIFY2 0x02  // head load time 4ms, DMA used
#define FLOPPY_STARTUP_TIME 8 // 1 second

#define FLOPPY_DOR_MOTOR_D     0x80 // Set to turn drive 3's motor ON
#define FLOPPY_DOR_MOTOR_C     0x40 // Set to turn drive 2's motor ON
#define FLOPPY_DOR_MOTOR_B     0x20 // Set to turn drive 1's motor ON
#define FLOPPY_DOR_MOTOR_A     0x10 // Set to turn drive 0's motor ON
#define FLOPPY_DOR_MOTOR_MASK  0xf0
#define FLOPPY_DOR_IRQ         0x08 // Set to enable IRQs and DMA
#define FLOPPY_DOR_RESET       0x04 // Clear = enter reset mode, Set = normal operation
#define FLOPPY_DOR_DSEL_MASK   0x03 // "Select" drive number for next access

// New diskette parameter table adding 3 parameters from IBM
// Since no provisions are made for multiple drive types, most
// values in this table are ignored.  I set parameters for 1.44M
// floppy here
কাঠামো floppy_ext_dbt_s diskette_param_table2 VARFSEG = {
    .dbt = {
        .specify1       = FLOPPY_SPECIFY1,
        .specify2       = FLOPPY_SPECIFY2,
        .shutoff_ticks  = FLOPPY_MOTOR_TICKS, // ~2 seconds
        .bps_code       = FLOPPY_আকার_CODE,
        .sectors        = 18,
        .interblock_len = FLOPPY_GAPLEN,
        .তথ্য_len       = FLOPPY_DATALEN,
        .gap_len        = FLOPPY_FORMAT_GAPLEN,
        .fill_byte      = FLOPPY_FILLBYTE,
        .settle_time    = 0x0F, // 15ms
        .startup_time   = FLOPPY_STARTUP_TIME,
    },
    .max_track      = 79,   // maximum track
    .তথ্য_rate      = 0,    // data transfer rate
    .drive_type     = 4,    // drive type in cmos
};

কাঠামো floppyinfo_s {
    কাঠামো chs_s chs;
    u8 floppy_আকার;
    u8 তথ্য_rate;
};

#define FLOPPY_আকার_525 0x01
#define FLOPPY_আকার_350 0x02

#define FLOPPY_RATE_500K 0x00
#define FLOPPY_RATE_300K 0x01
#define FLOPPY_RATE_250K 0x02
#define FLOPPY_RATE_1M   0x03

কাঠামো floppyinfo_s FloppyInfo[] VARFSEG = {
    // Unknown
    { {0, 0, 0}, 0x00, 0x00},
    // 1 - 360KB, 5.25" - 2 heads, 40 tracks, 9 sectors
    { {2, 40, 9}, FLOPPY_আকার_525, FLOPPY_RATE_300K},
    // 2 - 1.2MB, 5.25" - 2 heads, 80 tracks, 15 sectors
    { {2, 80, 15}, FLOPPY_আকার_525, FLOPPY_RATE_500K},
    // 3 - 720KB, 3.5"  - 2 heads, 80 tracks, 9 sectors
    { {2, 80, 9}, FLOPPY_আকার_350, FLOPPY_RATE_250K},
    // 4 - 1.44MB, 3.5" - 2 heads, 80 tracks, 18 sectors
    { {2, 80, 18}, FLOPPY_আকার_350, FLOPPY_RATE_500K},
    // 5 - 2.88MB, 3.5" - 2 heads, 80 tracks, 36 sectors
    { {2, 80, 36}, FLOPPY_আকার_350, FLOPPY_RATE_1M},
    // 6 - 160k, 5.25"  - 1 heads, 40 tracks, 8 sectors
    { {1, 40, 8}, FLOPPY_আকার_525, FLOPPY_RATE_250K},
    // 7 - 180k, 5.25"  - 1 heads, 40 tracks, 9 sectors
    { {1, 40, 9}, FLOPPY_আকার_525, FLOPPY_RATE_300K},
    // 8 - 320k, 5.25"  - 2 heads, 40 tracks, 8 sectors
    { {2, 40, 8}, FLOPPY_আকার_525, FLOPPY_RATE_250K},
};

কাঠামো drive_s *
init_floppy(পূর্ণসংখ্যা floppyid, পূর্ণসংখ্যা ftype)
{
    যদি (ftype <= 0 || ftype >= ARRAY_আকার(FloppyInfo)) {
        dprintf(1, "Bad floppy type %d\n", ftype);
        ফেরত ফাঁকা;
    }

    কাঠামো drive_s *drive = malloc_fseg(আকার(*drive));
    যদি (!drive) {
        warn_noalloc();
        ফেরত ফাঁকা;
    }
    memset(drive, 0, আকার(*drive));
    drive->cntl_id = floppyid;
    drive->type = DTYPE_FLOPPY;
    drive->blksize = DISK_SECTOR_আকার;
    drive->floppy_type = ftype;
    drive->sectors = (u64)-1;

    memcpy(&drive->lchs, &FloppyInfo[ftype].chs
           , আকার(FloppyInfo[ftype].chs));
    ফেরত drive;
}

স্থির শূন্য
addFloppy(পূর্ণসংখ্যা floppyid, পূর্ণসংখ্যা ftype)
{
    কাঠামো drive_s *drive = init_floppy(floppyid, ftype);
    যদি (!drive)
        ফেরত;
    অক্ষর *desc = znprintf(MAXDESCSIZE, "Floppy [drive %c]", 'A' + floppyid);
    কাঠামো pci_device *pci = pci_find_class(PCI_CLASS_BRIDGE_ISA); /* isa-to-pci bridge */
    পূর্ণসংখ্যা prio = bootprio_find_fdc_device(pci, PORT_FD_BASE, floppyid);
    boot_add_floppy(drive, desc, prio);
}

শূন্য
floppy_setup(শূন্য)
{
    memcpy(&diskette_param_table, &diskette_param_table2
           , আকার(diskette_param_table));
    SET_IVT(0x1E, SEGOFF(SEG_BIOS
                         , (u32)&diskette_param_table2 - BUILD_BIOS_ADDR));

    যদি (! CONFIG_FLOPPY)
        ফেরত;
    dprintf(3, "init floppy drives\n");

    যদি (CONFIG_QEMU) {
        u8 type = rtc_read(CMOS_FLOPPY_DRIVE_TYPE);
        যদি (type & 0xf0)
            addFloppy(0, type >> 4);
        যদি (type & 0x0f)
            addFloppy(1, type & 0x0f);
    } অন্যথা {
        u8 type = romfile_loadint("etc/floppy0", 0);
        যদি (type)
            addFloppy(0, type);
        type = romfile_loadint("etc/floppy1", 0);
        যদি (type)
            addFloppy(1, type);
    }

    enable_hwirq(6, FUNC16(entry_0e));
}

// Find a floppy type that matches a given image size.
পূর্ণসংখ্যা
find_floppy_type(u32 আকার_2)
{
    পূর্ণসংখ্যা i;
    জন্য (i=1; i<ARRAY_আকার(FloppyInfo); i++) {
        কাঠামো chs_s *c = &FloppyInfo[i].chs;
        যদি (c->cylinder * c->শীর্ষ * c->sector * DISK_SECTOR_আকার == আকার_2)
            ফেরত i;
    }
    ফেরত -1;
}


/****************************************************************
 * Low-level floppy IO
 ****************************************************************/

u8 FloppyDOR VARLOW;

স্থির অন্তঃরেখ u8
floppy_dor_read(শূন্য)
{
    ফেরত GET_নিচের(FloppyDOR);
}

স্থির অন্তঃরেখ শূন্য
floppy_dor_write(u8 val)
{
    outb(val, PORT_FD_DOR);
    SET_নিচের(FloppyDOR, val);
}

স্থির অন্তঃরেখ শূন্য
floppy_dor_mask(u8 off, u8 on)
{
    floppy_dor_write((floppy_dor_read() & ~off) | on);
}

স্থির শূন্য
floppy_disable_controller(শূন্য)
{
    dprintf(2, "Floppy_disable_controller\n");
    // Clear the reset bit (enter reset state) and clear 'enable IRQ and DMA'
    floppy_dor_mask(FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET, 0);
}

স্থির পূর্ণসংখ্যা
floppy_wait_irq(শূন্য)
{
    u8 frs = GET_BDA(floppy_recalibration_status);
    SET_BDA(floppy_recalibration_status, frs & ~FRS_IRQ);
    u32 end = timer_calc(FLOPPY_IRQ_TIMEOUT);
    জন্য (;;) {
        যদি (timer_check(end)) {
            warn_timeout();
            floppy_disable_controller();
            ফেরত DISK_RET_ETIMEOUT;
        }
        frs = GET_BDA(floppy_recalibration_status);
        যদি (frs & FRS_IRQ)
            বিরতি;
        // Could use yield_toirq() here, but that causes issues on
        // bochs, so use yield() instead.
        yield();
    }

    SET_BDA(floppy_recalibration_status, frs & ~FRS_IRQ);
    ফেরত DISK_RET_SUCCESS;
}

// Floppy commands
#define FCF_WAITIRQ 0x10000
#define FC_CHECKIRQ    (0x08 | (0<<8) | (2<<12))
#define FC_SEEK        (0x0f | (2<<8) | (0<<12) | FCF_WAITIRQ)
#define FC_RECALIBRATE (0x07 | (1<<8) | (0<<12) | FCF_WAITIRQ)
#define FC_READID      (0x4a | (1<<8) | (7<<12) | FCF_WAITIRQ)
#define FC_READ        (0xe6 | (8<<8) | (7<<12) | FCF_WAITIRQ)
#define FC_WRITE       (0xc5 | (8<<8) | (7<<12) | FCF_WAITIRQ)
#define FC_FORMAT      (0x4d | (5<<8) | (7<<12) | FCF_WAITIRQ)
#define FC_SPECIFY     (0x03 | (2<<8) | (0<<12))

// Send the specified command and it's parameters to the floppy controller.
স্থির পূর্ণসংখ্যা
floppy_pio(পূর্ণসংখ্যা command, u8 *param)
{
    dprintf(9, "Floppy pio command %x\n", command);
    // Send command and parameters to controller.
    u32 end = timer_calc(FLOPPY_PIO_TIMEOUT);
    পূর্ণসংখ্যা send = (command >> 8) & 0xf;
    পূর্ণসংখ্যা i = 0;
    জন্য (;;) {
        u8 sts = inb(PORT_FD_STATUS);
        যদি (!(sts & 0x80)) {
            যদি (timer_check(end)) {
                warn_timeout();
                floppy_disable_controller();
                ফেরত DISK_RET_ETIMEOUT;
            }
            yield();
            চালু;
        }
        যদি (sts & 0x40) {
            floppy_disable_controller();
            ফেরত DISK_RET_ECONTROLLER;
        }
        যদি (i == 0)
            outb(command & 0xff, PORT_FD_তথ্য);
        অন্যথা
            outb(param[i-1], PORT_FD_তথ্য);
        যদি (i++ >= send)
            বিরতি;
    }

    // Wait for command to complete.
    যদি (command & FCF_WAITIRQ) {
        পূর্ণসংখ্যা ret = floppy_wait_irq();
        যদি (ret)
            ফেরত ret;
    }

    // Read response from controller.
    end = timer_calc(FLOPPY_PIO_TIMEOUT);
    পূর্ণসংখ্যা receive = (command >> 12) & 0xf;
    i = 0;
    জন্য (;;) {
        u8 sts = inb(PORT_FD_STATUS);
        যদি (!(sts & 0x80)) {
            যদি (timer_check(end)) {
                warn_timeout();
                floppy_disable_controller();
                ফেরত DISK_RET_ETIMEOUT;
            }
            yield();
            চালু;
        }
        যদি (i >= receive) {
            যদি (sts & 0x40) {
                floppy_disable_controller();
                ফেরত DISK_RET_ECONTROLLER;
            }
            বিরতি;
        }
        যদি (!(sts & 0x40)) {
            floppy_disable_controller();
            ফেরত DISK_RET_ECONTROLLER;
        }
        param[i++] = inb(PORT_FD_তথ্য);
    }

    ফেরত DISK_RET_SUCCESS;
}

স্থির পূর্ণসংখ্যা
floppy_enable_controller(শূন্য)
{
    dprintf(2, "Floppy_enable_controller\n");
    // Clear the reset bit (enter reset state), but set 'enable IRQ and DMA'
    floppy_dor_mask(FLOPPY_DOR_RESET, FLOPPY_DOR_IRQ);
    // Real hardware needs a 4 microsecond delay
    usleep(4);
    // Set the reset bit (normal operation) and keep 'enable IRQ and DMA' on
    floppy_dor_mask(0, FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET);
    পূর্ণসংখ্যা ret = floppy_wait_irq();
    যদি (ret)
        ফেরত ret;

    // After the interrupt is received, send 4 SENSE INTERRUPT commands to
    // clear the interrupt status for each of the four logical drives,
    // supported by the controller.
    // See section 7.4 - "Drive Polling" of the Intel 82077AA datasheet for
    // a more detailed description of why this voodoo needs to be done.
    // Without this, initialization fails on real controllers (but still works
    // in QEMU)
    u8 param[2];
    পূর্ণসংখ্যা i;
    জন্য (i=0; i<4; i++) {
        ret = floppy_pio(FC_CHECKIRQ, param);
        যদি (ret)
            ফেরত ret;
    }
    ফেরত DISK_RET_SUCCESS;
}

// Activate a drive and send a command to it.
স্থির পূর্ণসংখ্যা
floppy_drive_pio(u8 floppyid, পূর্ণসংখ্যা command, u8 *param)
{
    // Enable controller if it isn't running.
    যদি (!(floppy_dor_read() & FLOPPY_DOR_RESET)) {
        পূর্ণসংখ্যা ret = floppy_enable_controller();
        যদি (ret)
            ফেরত ret;
    }

    // set the disk motor timeout value of INT 08 to the highest value
    SET_BDA(floppy_motor_গণক, 255);

    // Check if the motor is already running
    u8 motor_mask = FLOPPY_DOR_MOTOR_A << floppyid;
    পূর্ণসংখ্যা motor_already_running = floppy_dor_read() & motor_mask;

    // Turn on motor of selected drive, DMA & int enabled, normal operation
    floppy_dor_write(motor_mask | FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET | floppyid);

    // If the motor was just started, wait for it to get up to speed
    যদি (!motor_already_running && !CONFIG_QEMU)
        msleep(FLOPPY_STARTUP_TIME * 125);

    // Send command.
    পূর্ণসংখ্যা ret = floppy_pio(command, param);
    SET_BDA(floppy_motor_গণক, FLOPPY_MOTOR_TICKS); // reset motor timeout
    যদি (ret)
        ফেরত ret;

    // Check IRQ command is needed after irq commands with no results
    যদি ((command & FCF_WAITIRQ) && ((command >> 12) & 0xf) == 0)
        ফেরত floppy_pio(FC_CHECKIRQ, param);
    ফেরত DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy media sense and seeking
 ****************************************************************/

স্থির পূর্ণসংখ্যা
floppy_drive_recal(u8 floppyid)
{
    dprintf(2, "Floppy_drive_recal %d\n", floppyid);
    // send Recalibrate command to controller
    u8 param[2];
    param[0] = floppyid;
    পূর্ণসংখ্যা ret = floppy_drive_pio(floppyid, FC_RECALIBRATE, param);
    যদি (ret)
        ফেরত ret;

    u8 frs = GET_BDA(floppy_recalibration_status);
    SET_BDA(floppy_recalibration_status, frs | (1<<floppyid));
    SET_BDA(floppy_track[floppyid], 0);
    ফেরত DISK_RET_SUCCESS;
}

স্থির পূর্ণসংখ্যা
floppy_drive_seek(u8 floppyid, u8 cylinder)
{
    dprintf(2, "Floppy_drive_seek %d to %d\n", floppyid, cylinder);
    u8 param[2];
    param[0] = floppyid;
    param[1] = cylinder;
    পূর্ণসংখ্যা ret = floppy_drive_pio(floppyid, FC_SEEK, param);
    যদি (ret)
        ফেরত ret;

    SET_BDA(floppy_track[floppyid], cylinder);
    ফেরত DISK_RET_SUCCESS;
}

স্থির পূর্ণসংখ্যা
floppy_drive_specify(শূন্য)
{
    u8 param[2];
    param[0] = FLOPPY_SPECIFY1;
    param[1] = FLOPPY_SPECIFY2;
    ফেরত floppy_pio(FC_SPECIFY, param);
}

স্থির পূর্ণসংখ্যা
floppy_drive_readid(u8 floppyid, u8 তথ্য_rate, u8 শীর্ষ)
{
    // Set data rate.
    outb(তথ্য_rate, PORT_FD_DIR);

    // send Read Sector Id command
    u8 param[7];
    param[0] = (শীর্ষ << 2) | floppyid; // HD DR1 DR2
    পূর্ণসংখ্যা ret = floppy_drive_pio(floppyid, FC_READID, param);
    যদি (ret)
        ফেরত ret;
    যদি (param[0] & 0xc0)
        ফেরত -1;
    ফেরত 0;
}

// Only the 1.2MB 5.25" drive and the 3.5" drives have a disk change line.  On
// the older 5.25" types the pin is not driven and reads back as a permanent
// "changed".
স্থির পূর্ণসংখ্যা
floppy_has_changeline(u8 ftype)
{
    ফেরত ftype >= 2 && ftype <= 5;
}

// Determine whether a diskette is present, without waiting for a command to
// time out.  The disk change line latches when the medium is removed and is
// cleared only by a step pulse with a diskette in the drive.  The caller has
// just recalibrated, which leaves the head on cylinder 0 and need not step at
// all, so step to cylinder 1 and look again.
//
// Report a missing medium only when the drive is known to be empty -- an older
// drive without a change line is reported as present, so this can only ever
// fall back to the previous behaviour.
স্থির পূর্ণসংখ্যা
floppy_check_media(কাঠামো drive_s *drive_gf, u8 floppyid)
{
    যদি (!floppy_has_changeline(GET_GLOBALFLAT(drive_gf->floppy_type)))
        ফেরত DISK_RET_SUCCESS;

    // The caller has just recalibrated this drive, so it is selected in the
    // DOR and the DIR reflects its change line.
    যদি (!(inb(PORT_FD_DIR) & FLOPPY_DIR_DSKCHG))
        ফেরত DISK_RET_SUCCESS;

    পূর্ণসংখ্যা ret = floppy_drive_seek(floppyid, 1);
    যদি (ret)
        ফেরত ret;

    যদি (inb(PORT_FD_DIR) & FLOPPY_DIR_DSKCHG) {
        dprintf(2, "Floppy_check_media %d: no medium\n", floppyid);
        ফেরত DISK_RET_ETIMEOUT;
    }

    // A medium is present.  Put the head back on cylinder 0, where the caller
    // left it, so that media sensing reads the track it always did.
    ফেরত floppy_drive_seek(floppyid, 0);
}

স্থির পূর্ণসংখ্যা
floppy_media_sense(কাঠামো drive_s *drive_gf)
{
    u8 ftype = GET_GLOBALFLAT(drive_gf->floppy_type), stype = ftype;
    u8 floppyid = GET_GLOBALFLAT(drive_gf->cntl_id);

    u8 তথ্য_rate = GET_GLOBAL(FloppyInfo[stype].তথ্য_rate);
    পূর্ণসংখ্যা ret = floppy_drive_readid(floppyid, তথ্য_rate, 0);
    যদি (ret) {
        // Attempt media sense.
        জন্য (stype=1; ; stype++) {
            যদি (stype >= ARRAY_আকার(FloppyInfo))
                ফেরত DISK_RET_EBADTRACK;
            যদি (stype==ftype
                || (GET_GLOBAL(FloppyInfo[stype].floppy_আকার)
                    != GET_GLOBAL(FloppyInfo[ftype].floppy_আকার))
                || (GET_GLOBAL(FloppyInfo[stype].chs.শীর্ষ)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.শীর্ষ))
                || (GET_GLOBAL(FloppyInfo[stype].chs.cylinder)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.cylinder))
                || (GET_GLOBAL(FloppyInfo[stype].chs.sector)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.sector)))
                চালু;
            তথ্য_rate = GET_GLOBAL(FloppyInfo[stype].তথ্য_rate);
            ret = floppy_drive_readid(floppyid, তথ্য_rate, 0);
            যদি (!ret)
                বিরতি;
        }
    }
    dprintf(2, "Floppy_media_sense on drive %d found rate %d\n"
            , floppyid, তথ্য_rate);

    u8 old_তথ্য_rate = GET_BDA(floppy_media_state[floppyid]) >> 6;
    SET_BDA(floppy_last_তথ্য_rate, (old_তথ্য_rate<<2) | (তথ্য_rate<<6));
    u8 media = (stype == 1 ? 0x04 : (stype == 2 ? 0x05 : 0x07));
    u8 fms = (তথ্য_rate<<6) | FMS_MEDIA_DRIVE_ESTABLISHED | media;
    যদি (GET_GLOBAL(FloppyInfo[stype].chs.cylinder)
        < GET_GLOBAL(FloppyInfo[ftype].chs.cylinder))
        fms |= FMS_DOUBLE_STEPPING;
    SET_BDA(floppy_media_state[floppyid], fms);

    ফেরত DISK_RET_SUCCESS;
}

// Prepare a floppy for a data transfer.
স্থির পূর্ণসংখ্যা
floppy_prep(কাঠামো drive_s *drive_gf, u8 cylinder)
{
    u8 floppyid = GET_GLOBALFLAT(drive_gf->cntl_id);
    যদি (!(GET_BDA(floppy_recalibration_status) & (1<<floppyid)) ||
        !(GET_BDA(floppy_media_state[floppyid]) & FMS_MEDIA_DRIVE_ESTABLISHED)) {
        // Recalibrate drive.
        পূর্ণসংখ্যা ret = floppy_drive_recal(floppyid);
        যদি (ret)
            ফেরত ret;

        // An empty drive gives no index pulses, so the READ ID that
        // floppy_media_sense() issues would never complete and would wait out
        // its timeout once per data rate.  Ask the disk change line instead.
        ret = floppy_check_media(drive_gf, floppyid);
        যদি (ret)
            ফেরত ret;

        // Sense media.
        ret = floppy_media_sense(drive_gf);
        যদি (ret)
            ফেরত ret;

        // Execute a SPECIFY command (sets the Step Rate Time,
        // Head Load Time, Head Unload Time and the DMA enable/disable bit).
        ret = floppy_drive_specify();
        যদি (ret)
            ফেরত ret;
    }

    // Seek to cylinder if needed.
    যদি (cylinder != GET_BDA(floppy_track[floppyid])) {
        পূর্ণসংখ্যা ret = floppy_drive_seek(floppyid, cylinder);
        যদি (ret)
            ফেরত ret;
    }

    ফেরত DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy DMA transfer
 ****************************************************************/

// Perform a floppy transfer command (setup DMA and issue PIO).
স্থির পূর্ণসংখ্যা
floppy_dma_cmd(কাঠামো disk_op_s *op, পূর্ণসংখ্যা গুনতি, পূর্ণসংখ্যা command, u8 *param)
{
    // Setup DMA controller
    পূর্ণসংখ্যা isWrite = command != FC_READ;
    পূর্ণসংখ্যা ret = dma_floppy((u32)op->buf_fl, গুনতি, isWrite);
    যদি (ret)
        ফেরত DISK_RET_EBOUNDARY;

    // Invoke floppy controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    ret = floppy_drive_pio(floppyid, command, param);
    যদি (ret)
        ফেরত ret;

    // Populate floppy_return_status in BDA
    পূর্ণসংখ্যা i;
    জন্য (i=0; i<7; i++)
        SET_BDA(floppy_return_status[i], param[i]);

    যদি (param[0] & 0xc0) {
        যদি (param[1] & 0x02)
            ফেরত DISK_RET_EWRITEPROTECT;
        dprintf(1, "floppy error: %02x %02x %02x %02x %02x %02x %02x\n"
                , param[0], param[1], param[2], param[3]
                , param[4], param[5], param[6]);
        ফেরত DISK_RET_ECONTROLLER;
    }

    ফেরত DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy handlers
 ****************************************************************/

স্থির কাঠামো chs_s
lba2chs(কাঠামো disk_op_s *op)
{
    কাঠামো chs_s res = { };

    u32 tmp = op->lba;
    u16 nls = GET_GLOBALFLAT(op->drive_fl->lchs.sector);
    res.sector = (tmp % nls) + 1;

    tmp /= nls;
    u16 nlh = GET_GLOBALFLAT(op->drive_fl->lchs.শীর্ষ);
    res.শীর্ষ = tmp % nlh;

    tmp /= nlh;
    res.cylinder = tmp;

    ফেরত res;
}

// diskette controller reset
স্থির পূর্ণসংখ্যা
floppy_reset(কাঠামো disk_op_s *op)
{
    SET_BDA(floppy_recalibration_status, 0);
    SET_BDA(floppy_media_state[0], 0);
    SET_BDA(floppy_media_state[1], 0);
    SET_BDA(floppy_track[0], 0);
    SET_BDA(floppy_track[1], 0);
    SET_BDA(floppy_last_তথ্য_rate, 0);
    floppy_disable_controller();
    ফেরত floppy_enable_controller();
}

// Read Diskette Sectors
স্থির পূর্ণসংখ্যা
floppy_read(কাঠামো disk_op_s *op)
{
    কাঠামো chs_s chs = lba2chs(op);
    পূর্ণসংখ্যা ret = floppy_prep(op->drive_fl, chs.cylinder);
    যদি (ret)
        ফেরত ret;

    // send read-normal-data command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[8];
    param[0] = (chs.শীর্ষ << 2) | floppyid; // HD DR1 DR2
    param[1] = chs.cylinder;
    param[2] = chs.শীর্ষ;
    param[3] = chs.sector;
    param[4] = FLOPPY_আকার_CODE;
    param[5] = chs.sector + op->গুনতি - 1; // last sector to read on track
    param[6] = FLOPPY_GAPLEN;
    param[7] = FLOPPY_DATALEN;
    ফেরত floppy_dma_cmd(op, op->গুনতি * DISK_SECTOR_আকার, FC_READ, param);
}

// Write Diskette Sectors
স্থির পূর্ণসংখ্যা
floppy_write(কাঠামো disk_op_s *op)
{
    কাঠামো chs_s chs = lba2chs(op);
    পূর্ণসংখ্যা ret = floppy_prep(op->drive_fl, chs.cylinder);
    যদি (ret)
        ফেরত ret;

    // send write-normal-data command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[8];
    param[0] = (chs.শীর্ষ << 2) | floppyid; // HD DR1 DR2
    param[1] = chs.cylinder;
    param[2] = chs.শীর্ষ;
    param[3] = chs.sector;
    param[4] = FLOPPY_আকার_CODE;
    param[5] = chs.sector + op->গুনতি - 1; // last sector to write on track
    param[6] = FLOPPY_GAPLEN;
    param[7] = FLOPPY_DATALEN;
    ফেরত floppy_dma_cmd(op, op->গুনতি * DISK_SECTOR_আকার, FC_WRITE, param);
}

// Verify Diskette Sectors
স্থির পূর্ণসংখ্যা
floppy_verify(কাঠামো disk_op_s *op)
{
    কাঠামো chs_s chs = lba2chs(op);
    পূর্ণসংখ্যা ret = floppy_prep(op->drive_fl, chs.cylinder);
    যদি (ret)
        ফেরত ret;

    // This command isn't implemented - just return success.
    ফেরত DISK_RET_SUCCESS;
}

// format diskette track
স্থির পূর্ণসংখ্যা
floppy_format(কাঠামো disk_op_s *op)
{
    কাঠামো chs_s chs = lba2chs(op);
    পূর্ণসংখ্যা ret = floppy_prep(op->drive_fl, chs.cylinder);
    যদি (ret)
        ফেরত ret;

    // send format-track command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[7];
    param[0] = (chs.শীর্ষ << 2) | floppyid; // HD DR1 DR2
    param[1] = FLOPPY_আকার_CODE;
    param[2] = op->গুনতি; // number of sectors per track
    param[3] = FLOPPY_FORMAT_GAPLEN;
    param[4] = FLOPPY_FILLBYTE;
    ফেরত floppy_dma_cmd(op, op->গুনতি * 4, FC_FORMAT, param);
}

পূর্ণসংখ্যা
floppy_process_op(কাঠামো disk_op_s *op)
{
    যদি (!CONFIG_FLOPPY)
        ফেরত 0;

    নির্বাচন (op->command) {
    ক্ষেত্র CMD_RESET:
        ফেরত floppy_reset(op);
    ক্ষেত্র CMD_READ:
        ফেরত floppy_read(op);
    ক্ষেত্র CMD_WRITE:
        ফেরত floppy_write(op);
    ক্ষেত্র CMD_VERIFY:
        ফেরত floppy_verify(op);
    ক্ষেত্র CMD_FORMAT:
        ফেরত floppy_format(op);
    পূর্বনির্ধারিত:
        ফেরত DISK_RET_EPARAM;
    }
}


/****************************************************************
 * HW irqs
 ****************************************************************/

// INT 0Eh Diskette Hardware ISR Entry Point
শূন্য VISIBLE16
handle_0e(শূন্য)
{
    যদি (! CONFIG_FLOPPY)
        ফেরত;
    debug_isr(DEBUG_ISR_0e);

    // diskette interrupt has occurred
    u8 frs = GET_BDA(floppy_recalibration_status);
    SET_BDA(floppy_recalibration_status, frs | FRS_IRQ);

    pic_eoi1();
}

// Called from int08 handler.
শূন্য
floppy_tick(শূন্য)
{
    যদি (! CONFIG_FLOPPY)
        ফেরত;

    // time to turn off drive(s)?
    u8 fcount = GET_BDA(floppy_motor_গণক);
    যদি (fcount) {
        fcount--;
        SET_BDA(floppy_motor_গণক, fcount);
        যদি (fcount == 0)
            // turn motor(s) off
            floppy_dor_mask(FLOPPY_DOR_MOTOR_MASK, 0);
    }
}

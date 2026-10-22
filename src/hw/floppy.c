// 16bit code to access floppy drives.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // SET_BDA
#అన్తర్భావ "block.h" // struct drive_s
#అన్తర్భావ "bregs.h" // struct bregs
#అన్తర్భావ "config.h" // CONFIG_FLOPPY
#అన్తర్భావ "malloc.h" // malloc_fseg
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pcidevice.h" // pci_find_class
#అన్తర్భావ "pci_ids.h" // PCI_CLASS_BRIDGE_ISA
#అన్తర్భావ "pic.h" // pic_eoi1
#అన్తర్భావ "romfile.h" // romfile_loadint
#అన్తర్భావ "rtc.h" // rtc_read
#అన్తర్భావ "stacks.h" // yield
#అన్తర్భావ "std/disk.h" // DISK_RET_SUCCESS
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // timer_calc

#నిర్వచించు పోర్టు_FD_పునాది           0x03f0
#నిర్వచించు పోర్టు_FD_DOR            0x03f2
#నిర్వచించు పోర్టు_FD_స్థితి         0x03f4
#నిర్వచించు పోర్టు_FD_దత్తాంశం           0x03f5
#నిర్వచించు పోర్టు_FD_DIR            0x03f7

// Digital Input Register (read)
#నిర్వచించు FLOPPY_DIR_DSKCHG      0x80

#నిర్వచించు FLOPPY_కొలత_సంకేతం 0x02 // 512 byte sectors
#నిర్వచించు FLOPPY_DATALEN 0xff   // Not used - because size code is 0x02
#నిర్వచించు FLOPPY_MOTOR_TICKS 37 // ~2 seconds
#నిర్వచించు FLOPPY_FILLBYTE 0xf6
#నిర్వచించు FLOPPY_GAPLEN 0x1B
#నిర్వచించు FLOPPY_FORMAT_GAPLEN 0x6c
#నిర్వచించు FLOPPY_PIO_TIMEOUT 1000
#నిర్వచించు FLOPPY_IRQ_TIMEOUT 5000
#నిర్వచించు FLOPPY_SPECIFY1 0xAF  // step rate 12ms, head unload 240ms
#నిర్వచించు FLOPPY_SPECIFY2 0x02  // head load time 4ms, DMA used
#నిర్వచించు FLOPPY_STARTUP_సమయం 8 // 1 second

#నిర్వచించు FLOPPY_DOR_MOTOR_D     0x80 // Set to turn drive 3's motor ON
#నిర్వచించు FLOPPY_DOR_MOTOR_C     0x40 // Set to turn drive 2's motor ON
#నిర్వచించు FLOPPY_DOR_MOTOR_B     0x20 // Set to turn drive 1's motor ON
#నిర్వచించు FLOPPY_DOR_MOTOR_A     0x10 // Set to turn drive 0's motor ON
#నిర్వచించు FLOPPY_DOR_MOTOR_ముసుగు  0xf0
#నిర్వచించు FLOPPY_DOR_IRQ         0x08 // Set to enable IRQs and DMA
#నిర్వచించు FLOPPY_DOR_RESET       0x04 // Clear = enter reset mode, Set = normal operation
#నిర్వచించు FLOPPY_DOR_DSEL_ముసుగు   0x03 // "Select" drive number for next access

// New diskette parameter table adding 3 parameters from IBM
// Since no provisions are made for multiple drive types, most
// values in this table are ignored.  I set parameters for 1.44M
// floppy here
నిర్మాణం floppy_ext_dbt_s diskette_param_table2 VARFSEG = {
    .dbt = {
        .specify1       = FLOPPY_SPECIFY1,
        .specify2       = FLOPPY_SPECIFY2,
        .shutoff_ticks  = FLOPPY_MOTOR_TICKS, // ~2 seconds
        .bps_సంకేతం       = FLOPPY_కొలత_సంకేతం,
        .sectors        = 18,
        .interblock_len = FLOPPY_GAPLEN,
        .దత్తాంశం_len       = FLOPPY_DATALEN,
        .gap_len        = FLOPPY_FORMAT_GAPLEN,
        .fill_byte      = FLOPPY_FILLBYTE,
        .settle_సమయం    = 0x0F, // 15ms
        .startup_సమయం   = FLOPPY_STARTUP_సమయం,
    },
    .max_track      = 79,   // maximum track
    .దత్తాంశం_రేటు      = 0,    // data transfer rate
    .drive_type     = 4,    // drive type in cmos
};

నిర్మాణం floppyinfo_s {
    నిర్మాణం chs_s chs;
    u8 floppy_కొలత;
    u8 దత్తాంశం_రేటు;
};

#నిర్వచించు FLOPPY_కొలత_525 0x01
#నిర్వచించు FLOPPY_కొలత_350 0x02

#నిర్వచించు FLOPPY_రేటు_500_K 0x00
#నిర్వచించు FLOPPY_రేటు_300_K 0x01
#నిర్వచించు FLOPPY_రేటు_250_K 0x02
#నిర్వచించు FLOPPY_రేటు_1_M   0x03

నిర్మాణం floppyinfo_s FloppyInfo[] VARFSEG = {
    // Unknown
    { {0, 0, 0}, 0x00, 0x00},
    // 1 - 360KB, 5.25" - 2 heads, 40 tracks, 9 sectors
    { {2, 40, 9}, FLOPPY_కొలత_525, FLOPPY_రేటు_300_K},
    // 2 - 1.2MB, 5.25" - 2 heads, 80 tracks, 15 sectors
    { {2, 80, 15}, FLOPPY_కొలత_525, FLOPPY_రేటు_500_K},
    // 3 - 720KB, 3.5"  - 2 heads, 80 tracks, 9 sectors
    { {2, 80, 9}, FLOPPY_కొలత_350, FLOPPY_రేటు_250_K},
    // 4 - 1.44MB, 3.5" - 2 heads, 80 tracks, 18 sectors
    { {2, 80, 18}, FLOPPY_కొలత_350, FLOPPY_రేటు_500_K},
    // 5 - 2.88MB, 3.5" - 2 heads, 80 tracks, 36 sectors
    { {2, 80, 36}, FLOPPY_కొలత_350, FLOPPY_రేటు_1_M},
    // 6 - 160k, 5.25"  - 1 heads, 40 tracks, 8 sectors
    { {1, 40, 8}, FLOPPY_కొలత_525, FLOPPY_రేటు_250_K},
    // 7 - 180k, 5.25"  - 1 heads, 40 tracks, 9 sectors
    { {1, 40, 9}, FLOPPY_కొలత_525, FLOPPY_రేటు_300_K},
    // 8 - 320k, 5.25"  - 2 heads, 40 tracks, 8 sectors
    { {2, 40, 8}, FLOPPY_కొలత_525, FLOPPY_రేటు_250_K},
};

నిర్మాణం drive_s *
init_floppy(పూర్ణాంకం floppyid, పూర్ణాంకం ftype)
{
    ఒకవేళ (ftype <= 0 || ftype >= ARRAY_కొలత(FloppyInfo)) {
        dprintf(1, "Bad floppy type %d\n", ftype);
        తిరిగి ఖాళీ;
    }

    నిర్మాణం drive_s *drive = malloc_fseg(పరిమాణం(*drive));
    ఒకవేళ (!drive) {
        warn_noalloc();
        తిరిగి ఖాళీ;
    }
    memset(drive, 0, పరిమాణం(*drive));
    drive->cntl_id = floppyid;
    drive->type = DTYPE_FLOPPY;
    drive->blksize = DISK_SECTOR_కొలత;
    drive->floppy_type = ftype;
    drive->sectors = (u64)-1;

    జ్ఞాపకం_నకలు(&drive->lchs, &FloppyInfo[ftype].chs
           , పరిమాణం(FloppyInfo[ftype].chs));
    తిరిగి drive;
}

స్థిర శూన్య
కూడు_Floppy(పూర్ణాంకం floppyid, పూర్ణాంకం ftype)
{
    నిర్మాణం drive_s *drive = init_floppy(floppyid, ftype);
    ఒకవేళ (!drive)
        తిరిగి;
    అక్షరం *desc = znprintf(MAXDESCSIZE, "Floppy [drive %c]", 'A' + floppyid);
    నిర్మాణం pci_device *pci = pci_కనుగొను_class(PCI_CLASS_BRIDGE_ISA); /* isa-to-pci bridge */
    పూర్ణాంకం prio = bootprio_కనుగొను_fdc_device(pci, పోర్టు_FD_పునాది, floppyid);
    boot_కూడు_floppy(drive, desc, prio);
}

శూన్య
floppy_అమరిక(శూన్య)
{
    జ్ఞాపకం_నకలు(&diskette_param_పట్టిక, &diskette_param_table2
           , పరిమాణం(diskette_param_పట్టిక));
    సమితి_IVT(0x1E, SEGOFF(SEG_BIOS
                         , (u32)&diskette_param_table2 - BUILD_BIOS_ADDR));

    ఒకవేళ (! CONFIG_FLOPPY)
        తిరిగి;
    dprintf(3, "init floppy drives\n");

    ఒకవేళ (CONFIG_QEMU) {
        u8 type = rtc_read(CMOS_FLOPPY_DRIVE_TYPE);
        ఒకవేళ (type & 0xf0)
            కూడు_Floppy(0, type >> 4);
        ఒకవేళ (type & 0x0f)
            కూడు_Floppy(1, type & 0x0f);
    } లేకపోతే {
        u8 type = romfile_loadint("etc/floppy0", 0);
        ఒకవేళ (type)
            కూడు_Floppy(0, type);
        type = romfile_loadint("etc/floppy1", 0);
        ఒకవేళ (type)
            కూడు_Floppy(1, type);
    }

    enable_hwirq(6, FUNC16(నమోదు_0e));
}

// Find a floppy type that matches a given image size.
పూర్ణాంకం
కనుగొను_floppy_type(u32 కొలత)
{
    పూర్ణాంకం i;
    కొరకు (i=1; i<ARRAY_కొలత(FloppyInfo); i++) {
        నిర్మాణం chs_s *c = &FloppyInfo[i].chs;
        ఒకవేళ (c->cylinder * c->తల * c->sector * DISK_SECTOR_కొలత == కొలత)
            తిరిగి i;
    }
    తిరిగి -1;
}


/****************************************************************
 * Low-level floppy IO
 ****************************************************************/

u8 FloppyDOR VARLOW;

స్థిర అంతఃవరుస u8
floppy_dor_read(శూన్య)
{
    తిరిగి GET_దిగువ(FloppyDOR);
}

స్థిర అంతఃవరుస శూన్య
floppy_dor_write(u8 val)
{
    outb(val, పోర్టు_FD_DOR);
    సమితి_దిగువ(FloppyDOR, val);
}

స్థిర అంతఃవరుస శూన్య
floppy_dor_ముసుగు(u8 off, u8 on)
{
    floppy_dor_write((floppy_dor_read() & ~off) | on);
}

స్థిర శూన్య
floppy_disable_controller(శూన్య)
{
    dprintf(2, "Floppy_disable_controller\n");
    // Clear the reset bit (enter reset state) and clear 'enable IRQ and DMA'
    floppy_dor_ముసుగు(FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET, 0);
}

స్థిర పూర్ణాంకం
floppy_wait_irq(శూన్య)
{
    u8 frs = GET_BDA(floppy_recalibration_స్థితి);
    సమితి_BDA(floppy_recalibration_స్థితి, frs & ~FRS_IRQ);
    u32 చివర = timer_calc(FLOPPY_IRQ_TIMEOUT);
    కొరకు (;;) {
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            floppy_disable_controller();
            తిరిగి DISK_RET_ETIMEOUT;
        }
        frs = GET_BDA(floppy_recalibration_స్థితి);
        ఒకవేళ (frs & FRS_IRQ)
            ఆపు;
        // Could use yield_toirq() here, but that causes issues on
        // bochs, so use yield() instead.
        yield();
    }

    సమితి_BDA(floppy_recalibration_స్థితి, frs & ~FRS_IRQ);
    తిరిగి DISK_RET_SUCCESS;
}

// Floppy commands
#నిర్వచించు FCF_WAITIRQ 0x10000
#నిర్వచించు FC_CHECKIRQ    (0x08 | (0<<8) | (2<<12))
#నిర్వచించు FC_SEEK        (0x0f | (2<<8) | (0<<12) | FCF_WAITIRQ)
#నిర్వచించు FC_RECALIBRATE (0x07 | (1<<8) | (0<<12) | FCF_WAITIRQ)
#నిర్వచించు FC_READID      (0x4a | (1<<8) | (7<<12) | FCF_WAITIRQ)
#నిర్వచించు FC_READ        (0xe6 | (8<<8) | (7<<12) | FCF_WAITIRQ)
#నిర్వచించు FC_WRITE       (0xc5 | (8<<8) | (7<<12) | FCF_WAITIRQ)
#నిర్వచించు FC_FORMAT      (0x4d | (5<<8) | (7<<12) | FCF_WAITIRQ)
#నిర్వచించు FC_SPECIFY     (0x03 | (2<<8) | (0<<12))

// Send the specified command and it's parameters to the floppy controller.
స్థిర పూర్ణాంకం
floppy_pio(పూర్ణాంకం command, u8 *param)
{
    dprintf(9, "Floppy pio command %x\n", command);
    // Send command and parameters to controller.
    u32 చివర = timer_calc(FLOPPY_PIO_TIMEOUT);
    పూర్ణాంకం send = (command >> 8) & 0xf;
    పూర్ణాంకం i = 0;
    కొరకు (;;) {
        u8 sts = inb(పోర్టు_FD_స్థితి);
        ఒకవేళ (!(sts & 0x80)) {
            ఒకవేళ (timer_check(చివర)) {
                warn_timeout();
                floppy_disable_controller();
                తిరిగి DISK_RET_ETIMEOUT;
            }
            yield();
            కొనసాగు;
        }
        ఒకవేళ (sts & 0x40) {
            floppy_disable_controller();
            తిరిగి DISK_RET_ECONTROLLER;
        }
        ఒకవేళ (i == 0)
            outb(command & 0xff, పోర్టు_FD_దత్తాంశం);
        లేకపోతే
            outb(param[i-1], పోర్టు_FD_దత్తాంశం);
        ఒకవేళ (i++ >= send)
            ఆపు;
    }

    // Wait for command to complete.
    ఒకవేళ (command & FCF_WAITIRQ) {
        పూర్ణాంకం ret = floppy_wait_irq();
        ఒకవేళ (ret)
            తిరిగి ret;
    }

    // Read response from controller.
    చివర = timer_calc(FLOPPY_PIO_TIMEOUT);
    పూర్ణాంకం receive = (command >> 12) & 0xf;
    i = 0;
    కొరకు (;;) {
        u8 sts = inb(పోర్టు_FD_స్థితి);
        ఒకవేళ (!(sts & 0x80)) {
            ఒకవేళ (timer_check(చివర)) {
                warn_timeout();
                floppy_disable_controller();
                తిరిగి DISK_RET_ETIMEOUT;
            }
            yield();
            కొనసాగు;
        }
        ఒకవేళ (i >= receive) {
            ఒకవేళ (sts & 0x40) {
                floppy_disable_controller();
                తిరిగి DISK_RET_ECONTROLLER;
            }
            ఆపు;
        }
        ఒకవేళ (!(sts & 0x40)) {
            floppy_disable_controller();
            తిరిగి DISK_RET_ECONTROLLER;
        }
        param[i++] = inb(పోర్టు_FD_దత్తాంశం);
    }

    తిరిగి DISK_RET_SUCCESS;
}

స్థిర పూర్ణాంకం
floppy_enable_controller(శూన్య)
{
    dprintf(2, "Floppy_enable_controller\n");
    // Clear the reset bit (enter reset state), but set 'enable IRQ and DMA'
    floppy_dor_ముసుగు(FLOPPY_DOR_RESET, FLOPPY_DOR_IRQ);
    // Real hardware needs a 4 microsecond delay
    usleep(4);
    // Set the reset bit (normal operation) and keep 'enable IRQ and DMA' on
    floppy_dor_ముసుగు(0, FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET);
    పూర్ణాంకం ret = floppy_wait_irq();
    ఒకవేళ (ret)
        తిరిగి ret;

    // After the interrupt is received, send 4 SENSE INTERRUPT commands to
    // clear the interrupt status for each of the four logical drives,
    // supported by the controller.
    // See section 7.4 - "Drive Polling" of the Intel 82077AA datasheet for
    // a more detailed description of why this voodoo needs to be done.
    // Without this, initialization fails on real controllers (but still works
    // in QEMU)
    u8 param[2];
    పూర్ణాంకం i;
    కొరకు (i=0; i<4; i++) {
        ret = floppy_pio(FC_CHECKIRQ, param);
        ఒకవేళ (ret)
            తిరిగి ret;
    }
    తిరిగి DISK_RET_SUCCESS;
}

// Activate a drive and send a command to it.
స్థిర పూర్ణాంకం
floppy_drive_pio(u8 floppyid, పూర్ణాంకం command, u8 *param)
{
    // Enable controller if it isn't running.
    ఒకవేళ (!(floppy_dor_read() & FLOPPY_DOR_RESET)) {
        పూర్ణాంకం ret = floppy_enable_controller();
        ఒకవేళ (ret)
            తిరిగి ret;
    }

    // set the disk motor timeout value of INT 08 to the highest value
    సమితి_BDA(floppy_motor_counter, 255);

    // Check if the motor is already running
    u8 motor_ముసుగు = FLOPPY_DOR_MOTOR_A << floppyid;
    పూర్ణాంకం motor_already_running = floppy_dor_read() & motor_ముసుగు;

    // Turn on motor of selected drive, DMA & int enabled, normal operation
    floppy_dor_write(motor_ముసుగు | FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET | floppyid);

    // If the motor was just started, wait for it to get up to speed
    ఒకవేళ (!motor_already_running && !CONFIG_QEMU)
        msleep(FLOPPY_STARTUP_సమయం * 125);

    // Send command.
    పూర్ణాంకం ret = floppy_pio(command, param);
    సమితి_BDA(floppy_motor_counter, FLOPPY_MOTOR_TICKS); // reset motor timeout
    ఒకవేళ (ret)
        తిరిగి ret;

    // Check IRQ command is needed after irq commands with no results
    ఒకవేళ ((command & FCF_WAITIRQ) && ((command >> 12) & 0xf) == 0)
        తిరిగి floppy_pio(FC_CHECKIRQ, param);
    తిరిగి DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy media sense and seeking
 ****************************************************************/

స్థిర పూర్ణాంకం
floppy_drive_recal(u8 floppyid)
{
    dprintf(2, "Floppy_drive_recal %d\n", floppyid);
    // send Recalibrate command to controller
    u8 param[2];
    param[0] = floppyid;
    పూర్ణాంకం ret = floppy_drive_pio(floppyid, FC_RECALIBRATE, param);
    ఒకవేళ (ret)
        తిరిగి ret;

    u8 frs = GET_BDA(floppy_recalibration_స్థితి);
    సమితి_BDA(floppy_recalibration_స్థితి, frs | (1<<floppyid));
    సమితి_BDA(floppy_track[floppyid], 0);
    తిరిగి DISK_RET_SUCCESS;
}

స్థిర పూర్ణాంకం
floppy_drive_seek(u8 floppyid, u8 cylinder)
{
    dprintf(2, "Floppy_drive_seek %d to %d\n", floppyid, cylinder);
    u8 param[2];
    param[0] = floppyid;
    param[1] = cylinder;
    పూర్ణాంకం ret = floppy_drive_pio(floppyid, FC_SEEK, param);
    ఒకవేళ (ret)
        తిరిగి ret;

    సమితి_BDA(floppy_track[floppyid], cylinder);
    తిరిగి DISK_RET_SUCCESS;
}

స్థిర పూర్ణాంకం
floppy_drive_specify(శూన్య)
{
    u8 param[2];
    param[0] = FLOPPY_SPECIFY1;
    param[1] = FLOPPY_SPECIFY2;
    తిరిగి floppy_pio(FC_SPECIFY, param);
}

స్థిర పూర్ణాంకం
floppy_drive_readid(u8 floppyid, u8 దత్తాంశం_రేటు, u8 తల)
{
    // Set data rate.
    outb(దత్తాంశం_రేటు, పోర్టు_FD_DIR);

    // send Read Sector Id command
    u8 param[7];
    param[0] = (తల << 2) | floppyid; // HD DR1 DR2
    పూర్ణాంకం ret = floppy_drive_pio(floppyid, FC_READID, param);
    ఒకవేళ (ret)
        తిరిగి ret;
    ఒకవేళ (param[0] & 0xc0)
        తిరిగి -1;
    తిరిగి 0;
}

// Only the 1.2MB 5.25" drive and the 3.5" drives have a disk change line.  On
// the older 5.25" types the pin is not driven and reads back as a permanent
// "changed".
స్థిర పూర్ణాంకం
floppy_has_changeline(u8 ftype)
{
    తిరిగి ftype >= 2 && ftype <= 5;
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
స్థిర పూర్ణాంకం
floppy_check_media(నిర్మాణం drive_s *drive_gf, u8 floppyid)
{
    ఒకవేళ (!floppy_has_changeline(GET_GLOBALFLAT(drive_gf->floppy_type)))
        తిరిగి DISK_RET_SUCCESS;

    // The caller has just recalibrated this drive, so it is selected in the
    // DOR and the DIR reflects its change line.
    ఒకవేళ (!(inb(పోర్టు_FD_DIR) & FLOPPY_DIR_DSKCHG))
        తిరిగి DISK_RET_SUCCESS;

    పూర్ణాంకం ret = floppy_drive_seek(floppyid, 1);
    ఒకవేళ (ret)
        తిరిగి ret;

    ఒకవేళ (inb(పోర్టు_FD_DIR) & FLOPPY_DIR_DSKCHG) {
        dprintf(2, "Floppy_check_media %d: no medium\n", floppyid);
        తిరిగి DISK_RET_ETIMEOUT;
    }

    // A medium is present.  Put the head back on cylinder 0, where the caller
    // left it, so that media sensing reads the track it always did.
    తిరిగి floppy_drive_seek(floppyid, 0);
}

స్థిర పూర్ణాంకం
floppy_media_sense(నిర్మాణం drive_s *drive_gf)
{
    u8 ftype = GET_GLOBALFLAT(drive_gf->floppy_type), stype = ftype;
    u8 floppyid = GET_GLOBALFLAT(drive_gf->cntl_id);

    u8 దత్తాంశం_రేటు = GET_GLOBAL(FloppyInfo[stype].దత్తాంశం_రేటు);
    పూర్ణాంకం ret = floppy_drive_readid(floppyid, దత్తాంశం_రేటు, 0);
    ఒకవేళ (ret) {
        // Attempt media sense.
        కొరకు (stype=1; ; stype++) {
            ఒకవేళ (stype >= ARRAY_కొలత(FloppyInfo))
                తిరిగి DISK_RET_EBADTRACK;
            ఒకవేళ (stype==ftype
                || (GET_GLOBAL(FloppyInfo[stype].floppy_కొలత)
                    != GET_GLOBAL(FloppyInfo[ftype].floppy_కొలత))
                || (GET_GLOBAL(FloppyInfo[stype].chs.తల)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.తల))
                || (GET_GLOBAL(FloppyInfo[stype].chs.cylinder)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.cylinder))
                || (GET_GLOBAL(FloppyInfo[stype].chs.sector)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.sector)))
                కొనసాగు;
            దత్తాంశం_రేటు = GET_GLOBAL(FloppyInfo[stype].దత్తాంశం_రేటు);
            ret = floppy_drive_readid(floppyid, దత్తాంశం_రేటు, 0);
            ఒకవేళ (!ret)
                ఆపు;
        }
    }
    dprintf(2, "Floppy_media_sense on drive %d found rate %d\n"
            , floppyid, దత్తాంశం_రేటు);

    u8 old_దత్తాంశం_రేటు = GET_BDA(floppy_media_state[floppyid]) >> 6;
    సమితి_BDA(floppy_చివరి_దత్తాంశం_రేటు, (old_దత్తాంశం_రేటు<<2) | (దత్తాంశం_రేటు<<6));
    u8 media = (stype == 1 ? 0x04 : (stype == 2 ? 0x05 : 0x07));
    u8 fms = (దత్తాంశం_రేటు<<6) | FMS_MEDIA_DRIVE_ESTABLISHED | media;
    ఒకవేళ (GET_GLOBAL(FloppyInfo[stype].chs.cylinder)
        < GET_GLOBAL(FloppyInfo[ftype].chs.cylinder))
        fms |= FMS_DOUBLE_STEPPING;
    సమితి_BDA(floppy_media_state[floppyid], fms);

    తిరిగి DISK_RET_SUCCESS;
}

// Prepare a floppy for a data transfer.
స్థిర పూర్ణాంకం
floppy_prep(నిర్మాణం drive_s *drive_gf, u8 cylinder)
{
    u8 floppyid = GET_GLOBALFLAT(drive_gf->cntl_id);
    ఒకవేళ (!(GET_BDA(floppy_recalibration_స్థితి) & (1<<floppyid)) ||
        !(GET_BDA(floppy_media_state[floppyid]) & FMS_MEDIA_DRIVE_ESTABLISHED)) {
        // Recalibrate drive.
        పూర్ణాంకం ret = floppy_drive_recal(floppyid);
        ఒకవేళ (ret)
            తిరిగి ret;

        // An empty drive gives no index pulses, so the READ ID that
        // floppy_media_sense() issues would never complete and would wait out
        // its timeout once per data rate.  Ask the disk change line instead.
        ret = floppy_check_media(drive_gf, floppyid);
        ఒకవేళ (ret)
            తిరిగి ret;

        // Sense media.
        ret = floppy_media_sense(drive_gf);
        ఒకవేళ (ret)
            తిరిగి ret;

        // Execute a SPECIFY command (sets the Step Rate Time,
        // Head Load Time, Head Unload Time and the DMA enable/disable bit).
        ret = floppy_drive_specify();
        ఒకవేళ (ret)
            తిరిగి ret;
    }

    // Seek to cylinder if needed.
    ఒకవేళ (cylinder != GET_BDA(floppy_track[floppyid])) {
        పూర్ణాంకం ret = floppy_drive_seek(floppyid, cylinder);
        ఒకవేళ (ret)
            తిరిగి ret;
    }

    తిరిగి DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy DMA transfer
 ****************************************************************/

// Perform a floppy transfer command (setup DMA and issue PIO).
స్థిర పూర్ణాంకం
floppy_dma_cmd(నిర్మాణం disk_op_s *op, పూర్ణాంకం లెక్క, పూర్ణాంకం command, u8 *param)
{
    // Setup DMA controller
    పూర్ణాంకం isWrite = command != FC_READ;
    పూర్ణాంకం ret = dma_floppy((u32)op->buf_fl, లెక్క, isWrite);
    ఒకవేళ (ret)
        తిరిగి DISK_RET_EBOUNDARY;

    // Invoke floppy controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    ret = floppy_drive_pio(floppyid, command, param);
    ఒకవేళ (ret)
        తిరిగి ret;

    // Populate floppy_return_status in BDA
    పూర్ణాంకం i;
    కొరకు (i=0; i<7; i++)
        సమితి_BDA(floppy_return_స్థితి[i], param[i]);

    ఒకవేళ (param[0] & 0xc0) {
        ఒకవేళ (param[1] & 0x02)
            తిరిగి DISK_RET_EWRITEPROTECT;
        dprintf(1, "floppy error: %02x %02x %02x %02x %02x %02x %02x\n"
                , param[0], param[1], param[2], param[3]
                , param[4], param[5], param[6]);
        తిరిగి DISK_RET_ECONTROLLER;
    }

    తిరిగి DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy handlers
 ****************************************************************/

స్థిర నిర్మాణం chs_s
lba2chs(నిర్మాణం disk_op_s *op)
{
    నిర్మాణం chs_s res = { };

    u32 tmp = op->lba;
    u16 nls = GET_GLOBALFLAT(op->drive_fl->lchs.sector);
    res.sector = (tmp % nls) + 1;

    tmp /= nls;
    u16 nlh = GET_GLOBALFLAT(op->drive_fl->lchs.తల);
    res.తల = tmp % nlh;

    tmp /= nlh;
    res.cylinder = tmp;

    తిరిగి res;
}

// diskette controller reset
స్థిర పూర్ణాంకం
floppy_reset(నిర్మాణం disk_op_s *op)
{
    సమితి_BDA(floppy_recalibration_స్థితి, 0);
    సమితి_BDA(floppy_media_state[0], 0);
    సమితి_BDA(floppy_media_state[1], 0);
    సమితి_BDA(floppy_track[0], 0);
    సమితి_BDA(floppy_track[1], 0);
    సమితి_BDA(floppy_చివరి_దత్తాంశం_రేటు, 0);
    floppy_disable_controller();
    తిరిగి floppy_enable_controller();
}

// Read Diskette Sectors
స్థిర పూర్ణాంకం
floppy_read(నిర్మాణం disk_op_s *op)
{
    నిర్మాణం chs_s chs = lba2chs(op);
    పూర్ణాంకం ret = floppy_prep(op->drive_fl, chs.cylinder);
    ఒకవేళ (ret)
        తిరిగి ret;

    // send read-normal-data command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[8];
    param[0] = (chs.తల << 2) | floppyid; // HD DR1 DR2
    param[1] = chs.cylinder;
    param[2] = chs.తల;
    param[3] = chs.sector;
    param[4] = FLOPPY_కొలత_సంకేతం;
    param[5] = chs.sector + op->లెక్క - 1; // last sector to read on track
    param[6] = FLOPPY_GAPLEN;
    param[7] = FLOPPY_DATALEN;
    తిరిగి floppy_dma_cmd(op, op->లెక్క * DISK_SECTOR_కొలత, FC_READ, param);
}

// Write Diskette Sectors
స్థిర పూర్ణాంకం
floppy_write(నిర్మాణం disk_op_s *op)
{
    నిర్మాణం chs_s chs = lba2chs(op);
    పూర్ణాంకం ret = floppy_prep(op->drive_fl, chs.cylinder);
    ఒకవేళ (ret)
        తిరిగి ret;

    // send write-normal-data command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[8];
    param[0] = (chs.తల << 2) | floppyid; // HD DR1 DR2
    param[1] = chs.cylinder;
    param[2] = chs.తల;
    param[3] = chs.sector;
    param[4] = FLOPPY_కొలత_సంకేతం;
    param[5] = chs.sector + op->లెక్క - 1; // last sector to write on track
    param[6] = FLOPPY_GAPLEN;
    param[7] = FLOPPY_DATALEN;
    తిరిగి floppy_dma_cmd(op, op->లెక్క * DISK_SECTOR_కొలత, FC_WRITE, param);
}

// Verify Diskette Sectors
స్థిర పూర్ణాంకం
floppy_verify(నిర్మాణం disk_op_s *op)
{
    నిర్మాణం chs_s chs = lba2chs(op);
    పూర్ణాంకం ret = floppy_prep(op->drive_fl, chs.cylinder);
    ఒకవేళ (ret)
        తిరిగి ret;

    // This command isn't implemented - just return success.
    తిరిగి DISK_RET_SUCCESS;
}

// format diskette track
స్థిర పూర్ణాంకం
floppy_format(నిర్మాణం disk_op_s *op)
{
    నిర్మాణం chs_s chs = lba2chs(op);
    పూర్ణాంకం ret = floppy_prep(op->drive_fl, chs.cylinder);
    ఒకవేళ (ret)
        తిరిగి ret;

    // send format-track command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[7];
    param[0] = (chs.తల << 2) | floppyid; // HD DR1 DR2
    param[1] = FLOPPY_కొలత_సంకేతం;
    param[2] = op->లెక్క; // number of sectors per track
    param[3] = FLOPPY_FORMAT_GAPLEN;
    param[4] = FLOPPY_FILLBYTE;
    తిరిగి floppy_dma_cmd(op, op->లెక్క * 4, FC_FORMAT, param);
}

పూర్ణాంకం
floppy_process_op(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (!CONFIG_FLOPPY)
        తిరిగి 0;

    ఎంపిక (op->command) {
    స్థితి CMD_RESET:
        తిరిగి floppy_reset(op);
    స్థితి CMD_READ:
        తిరిగి floppy_read(op);
    స్థితి CMD_WRITE:
        తిరిగి floppy_write(op);
    స్థితి CMD_VERIFY:
        తిరిగి floppy_verify(op);
    స్థితి CMD_FORMAT:
        తిరిగి floppy_format(op);
    అప్రమేయ:
        తిరిగి DISK_RET_EPARAM;
    }
}


/****************************************************************
 * HW irqs
 ****************************************************************/

// INT 0Eh Diskette Hardware ISR Entry Point
శూన్య VISIBLE16
handle_0e(శూన్య)
{
    ఒకవేళ (! CONFIG_FLOPPY)
        తిరిగి;
    debug_isr(DEBUG_ISR_0e);

    // diskette interrupt has occurred
    u8 frs = GET_BDA(floppy_recalibration_స్థితి);
    సమితి_BDA(floppy_recalibration_స్థితి, frs | FRS_IRQ);

    pic_eoi1();
}

// Called from int08 handler.
శూన్య
floppy_tick(శూన్య)
{
    ఒకవేళ (! CONFIG_FLOPPY)
        తిరిగి;

    // time to turn off drive(s)?
    u8 fcount = GET_BDA(floppy_motor_counter);
    ఒకవేళ (fcount) {
        fcount--;
        సమితి_BDA(floppy_motor_counter, fcount);
        ఒకవేళ (fcount == 0)
            // turn motor(s) off
            floppy_dor_ముసుగు(FLOPPY_DOR_MOTOR_ముసుగు, 0);
    }
}

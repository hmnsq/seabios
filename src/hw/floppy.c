// 16bit code to access floppy drives.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // SET_BDA
#अन्तर्भाव "block.h" // struct drive_s
#अन्तर्भाव "bregs.h" // struct bregs
#अन्तर्भाव "config.h" // CONFIG_FLOPPY
#अन्तर्भाव "malloc.h" // malloc_fseg
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "pcidevice.h" // pci_find_class
#अन्तर्भाव "pci_ids.h" // PCI_CLASS_BRIDGE_ISA
#अन्तर्भाव "pic.h" // pic_eoi1
#अन्तर्भाव "romfile.h" // romfile_loadint
#अन्तर्भाव "rtc.h" // rtc_read
#अन्तर्भाव "stacks.h" // yield
#अन्तर्भाव "std/disk.h" // DISK_RET_SUCCESS
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // timer_calc

#परिभाषय पत्तनम्_FD_आधारः           0x03f0
#परिभाषय पत्तनम्_FD_DOR            0x03f2
#परिभाषय पत्तनम्_FD_स्थितिः         0x03f4
#परिभाषय पत्तनम्_FD_दत्त           0x03f5
#परिभाषय पत्तनम्_FD_DIR            0x03f7

// Digital Input Register (read)
#परिभाषय FLOPPY_DIR_DSKCHG      0x80

#परिभाषय FLOPPY_आकार_संहिता 0x02 // 512 byte sectors
#परिभाषय FLOPPY_DATALEN 0xff   // Not used - because size code is 0x02
#परिभाषय FLOPPY_MOTOR_TICKS 37 // ~2 seconds
#परिभाषय FLOPPY_FILLBYTE 0xf6
#परिभाषय FLOPPY_GAPLEN 0x1B
#परिभाषय FLOPPY_FORMAT_GAPLEN 0x6c
#परिभाषय FLOPPY_PIO_TIMEOUT 1000
#परिभाषय FLOPPY_IRQ_TIMEOUT 5000
#परिभाषय FLOPPY_SPECIFY1 0xAF  // step rate 12ms, head unload 240ms
#परिभाषय FLOPPY_SPECIFY2 0x02  // head load time 4ms, DMA used
#परिभाषय FLOPPY_STARTUP_TIME 8 // 1 second

#परिभाषय FLOPPY_DOR_MOTOR_D     0x80 // Set to turn drive 3's motor ON
#परिभाषय FLOPPY_DOR_MOTOR_C     0x40 // Set to turn drive 2's motor ON
#परिभाषय FLOPPY_DOR_MOTOR_B     0x20 // Set to turn drive 1's motor ON
#परिभाषय FLOPPY_DOR_MOTOR_A     0x10 // Set to turn drive 0's motor ON
#परिभाषय FLOPPY_DOR_MOTOR_आवरणम्  0xf0
#परिभाषय FLOPPY_DOR_IRQ         0x08 // Set to enable IRQs and DMA
#परिभाषय FLOPPY_DOR_RESET       0x04 // Clear = enter reset mode, Set = normal operation
#परिभाषय FLOPPY_DOR_DSEL_आवरणम्   0x03 // "Select" drive number for next access

// New diskette parameter table adding 3 parameters from IBM
// Since no provisions are made for multiple drive types, most
// values in this table are ignored.  I set parameters for 1.44M
// floppy here
संरचना floppy_ext_dbt_s diskette_param_table2 VARFSEG = {
    .dbt = {
        .specify1       = FLOPPY_SPECIFY1,
        .specify2       = FLOPPY_SPECIFY2,
        .shutoff_ticks  = FLOPPY_MOTOR_TICKS, // ~2 seconds
        .bps_संहिता       = FLOPPY_आकार_संहिता,
        .sectors        = 18,
        .interblock_len = FLOPPY_GAPLEN,
        .दत्त_len       = FLOPPY_DATALEN,
        .gap_len        = FLOPPY_FORMAT_GAPLEN,
        .fill_byte      = FLOPPY_FILLBYTE,
        .settle_time    = 0x0F, // 15ms
        .startup_time   = FLOPPY_STARTUP_TIME,
    },
    .max_track      = 79,   // maximum track
    .दत्त_दरः      = 0,    // data transfer rate
    .drive_type     = 4,    // drive type in cmos
};

संरचना floppyinfo_s {
    संरचना chs_s chs;
    u8 floppy_आकार;
    u8 दत्त_दरः;
};

#परिभाषय FLOPPY_आकार_525 0x01
#परिभाषय FLOPPY_आकार_350 0x02

#परिभाषय FLOPPY_दरः_500_K 0x00
#परिभाषय FLOPPY_दरः_300_K 0x01
#परिभाषय FLOPPY_दरः_250_K 0x02
#परिभाषय FLOPPY_दरः_1_M   0x03

संरचना floppyinfo_s FloppyInfo[] VARFSEG = {
    // Unknown
    { {0, 0, 0}, 0x00, 0x00},
    // 1 - 360KB, 5.25" - 2 heads, 40 tracks, 9 sectors
    { {2, 40, 9}, FLOPPY_आकार_525, FLOPPY_दरः_300_K},
    // 2 - 1.2MB, 5.25" - 2 heads, 80 tracks, 15 sectors
    { {2, 80, 15}, FLOPPY_आकार_525, FLOPPY_दरः_500_K},
    // 3 - 720KB, 3.5"  - 2 heads, 80 tracks, 9 sectors
    { {2, 80, 9}, FLOPPY_आकार_350, FLOPPY_दरः_250_K},
    // 4 - 1.44MB, 3.5" - 2 heads, 80 tracks, 18 sectors
    { {2, 80, 18}, FLOPPY_आकार_350, FLOPPY_दरः_500_K},
    // 5 - 2.88MB, 3.5" - 2 heads, 80 tracks, 36 sectors
    { {2, 80, 36}, FLOPPY_आकार_350, FLOPPY_दरः_1_M},
    // 6 - 160k, 5.25"  - 1 heads, 40 tracks, 8 sectors
    { {1, 40, 8}, FLOPPY_आकार_525, FLOPPY_दरः_250_K},
    // 7 - 180k, 5.25"  - 1 heads, 40 tracks, 9 sectors
    { {1, 40, 9}, FLOPPY_आकार_525, FLOPPY_दरः_300_K},
    // 8 - 320k, 5.25"  - 2 heads, 40 tracks, 8 sectors
    { {2, 40, 8}, FLOPPY_आकार_525, FLOPPY_दरः_250_K},
};

संरचना drive_s *
init_floppy(पूर्णाङ्क floppyid, पूर्णाङ्क ftype)
{
    यदि (ftype <= 0 || ftype >= ARRAY_आकार(FloppyInfo)) {
        dprintf(1, "Bad floppy type %d\n", ftype);
        प्रत्यागम रिक्त;
    }

    संरचना drive_s *drive = malloc_fseg(परिमाण(*drive));
    यदि (!drive) {
        warn_noalloc();
        प्रत्यागम रिक्त;
    }
    स्मृतिं_पूरय(drive, 0, परिमाण(*drive));
    drive->cntl_id = floppyid;
    drive->type = DTYPE_FLOPPY;
    drive->blksize = DISK_SECTOR_आकार;
    drive->floppy_type = ftype;
    drive->sectors = (u64)-1;

    स्मृतिं_प्रतिलिख(&drive->lchs, &FloppyInfo[ftype].chs
           , परिमाण(FloppyInfo[ftype].chs));
    प्रत्यागम drive;
}

स्थिर शून्य
सङ्कलय_Floppy(पूर्णाङ्क floppyid, पूर्णाङ्क ftype)
{
    संरचना drive_s *drive = init_floppy(floppyid, ftype);
    यदि (!drive)
        प्रत्यागम;
    वर्ण *desc = znprintf(MAXDESCSIZE, "Floppy [drive %c]", 'A' + floppyid);
    संरचना pci_device *pci = pci_find_class(PCI_CLASS_BRIDGE_ISA); /* isa-to-pci bridge */
    पूर्णाङ्क prio = bootprio_find_fdc_device(pci, पत्तनम्_FD_आधारः, floppyid);
    boot_सङ्कलय_floppy(drive, desc, prio);
}

शून्य
floppy_सज्जता(शून्य)
{
    स्मृतिं_प्रतिलिख(&diskette_param_सारणी, &diskette_param_table2
           , परिमाण(diskette_param_सारणी));
    समुच्चयः_IVT(0x1E, SEGOFF(SEG_BIOS
                         , (u32)&diskette_param_table2 - BUILD_BIOS_ADDR));

    यदि (! CONFIG_FLOPPY)
        प्रत्यागम;
    dprintf(3, "init floppy drives\n");

    यदि (CONFIG_QEMU) {
        u8 type = rtc_read(CMOS_FLOPPY_DRIVE_TYPE);
        यदि (type & 0xf0)
            सङ्कलय_Floppy(0, type >> 4);
        यदि (type & 0x0f)
            सङ्कलय_Floppy(1, type & 0x0f);
    } अन्यथा {
        u8 type = romfile_loadint("etc/floppy0", 0);
        यदि (type)
            सङ्कलय_Floppy(0, type);
        type = romfile_loadint("etc/floppy1", 0);
        यदि (type)
            सङ्कलय_Floppy(1, type);
    }

    enable_hwirq(6, FUNC16(प्रविष्टिः_0e));
}

// Find a floppy type that matches a given image size.
पूर्णाङ्क
find_floppy_type(u32 आकार)
{
    पूर्णाङ्क i;
    प्रति (i=1; i<ARRAY_आकार(FloppyInfo); i++) {
        संरचना chs_s *c = &FloppyInfo[i].chs;
        यदि (c->cylinder * c->शीर्ष * c->sector * DISK_SECTOR_आकार == आकार)
            प्रत्यागम i;
    }
    प्रत्यागम -1;
}


/****************************************************************
 * Low-level floppy IO
 ****************************************************************/

u8 FloppyDOR VARLOW;

स्थिर अन्तःस्थ u8
floppy_dor_read(शून्य)
{
    प्रत्यागम GET_नीचः(FloppyDOR);
}

स्थिर अन्तःस्थ शून्य
floppy_dor_write(u8 val)
{
    outb(val, पत्तनम्_FD_DOR);
    समुच्चयः_नीचः(FloppyDOR, val);
}

स्थिर अन्तःस्थ शून्य
floppy_dor_आवरणम्(u8 off, u8 on)
{
    floppy_dor_write((floppy_dor_read() & ~off) | on);
}

स्थिर शून्य
floppy_disable_controller(शून्य)
{
    dprintf(2, "Floppy_disable_controller\n");
    // Clear the reset bit (enter reset state) and clear 'enable IRQ and DMA'
    floppy_dor_आवरणम्(FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET, 0);
}

स्थिर पूर्णाङ्क
floppy_wait_irq(शून्य)
{
    u8 frs = GET_BDA(floppy_recalibration_स्थितिः);
    समुच्चयः_BDA(floppy_recalibration_स्थितिः, frs & ~FRS_IRQ);
    u32 अन्तः = timer_calc(FLOPPY_IRQ_TIMEOUT);
    प्रति (;;) {
        यदि (timer_check(अन्तः)) {
            warn_timeout();
            floppy_disable_controller();
            प्रत्यागम DISK_RET_ETIMEOUT;
        }
        frs = GET_BDA(floppy_recalibration_स्थितिः);
        यदि (frs & FRS_IRQ)
            भङ्ग;
        // Could use yield_toirq() here, but that causes issues on
        // bochs, so use yield() instead.
        yield();
    }

    समुच्चयः_BDA(floppy_recalibration_स्थितिः, frs & ~FRS_IRQ);
    प्रत्यागम DISK_RET_SUCCESS;
}

// Floppy commands
#परिभाषय FCF_WAITIRQ 0x10000
#परिभाषय FC_CHECKIRQ    (0x08 | (0<<8) | (2<<12))
#परिभाषय FC_SEEK        (0x0f | (2<<8) | (0<<12) | FCF_WAITIRQ)
#परिभाषय FC_RECALIBRATE (0x07 | (1<<8) | (0<<12) | FCF_WAITIRQ)
#परिभाषय FC_READID      (0x4a | (1<<8) | (7<<12) | FCF_WAITIRQ)
#परिभाषय FC_READ        (0xe6 | (8<<8) | (7<<12) | FCF_WAITIRQ)
#परिभाषय FC_WRITE       (0xc5 | (8<<8) | (7<<12) | FCF_WAITIRQ)
#परिभाषय FC_FORMAT      (0x4d | (5<<8) | (7<<12) | FCF_WAITIRQ)
#परिभाषय FC_SPECIFY     (0x03 | (2<<8) | (0<<12))

// Send the specified command and it's parameters to the floppy controller.
स्थिर पूर्णाङ्क
floppy_pio(पूर्णाङ्क command, u8 *param)
{
    dprintf(9, "Floppy pio command %x\n", command);
    // Send command and parameters to controller.
    u32 अन्तः = timer_calc(FLOPPY_PIO_TIMEOUT);
    पूर्णाङ्क send = (command >> 8) & 0xf;
    पूर्णाङ्क i = 0;
    प्रति (;;) {
        u8 sts = inb(पत्तनम्_FD_स्थितिः);
        यदि (!(sts & 0x80)) {
            यदि (timer_check(अन्तः)) {
                warn_timeout();
                floppy_disable_controller();
                प्रत्यागम DISK_RET_ETIMEOUT;
            }
            yield();
            अनुवर्त;
        }
        यदि (sts & 0x40) {
            floppy_disable_controller();
            प्रत्यागम DISK_RET_ECONTROLLER;
        }
        यदि (i == 0)
            outb(command & 0xff, पत्तनम्_FD_दत्त);
        अन्यथा
            outb(param[i-1], पत्तनम्_FD_दत्त);
        यदि (i++ >= send)
            भङ्ग;
    }

    // Wait for command to complete.
    यदि (command & FCF_WAITIRQ) {
        पूर्णाङ्क ret = floppy_wait_irq();
        यदि (ret)
            प्रत्यागम ret;
    }

    // Read response from controller.
    अन्तः = timer_calc(FLOPPY_PIO_TIMEOUT);
    पूर्णाङ्क receive = (command >> 12) & 0xf;
    i = 0;
    प्रति (;;) {
        u8 sts = inb(पत्तनम्_FD_स्थितिः);
        यदि (!(sts & 0x80)) {
            यदि (timer_check(अन्तः)) {
                warn_timeout();
                floppy_disable_controller();
                प्रत्यागम DISK_RET_ETIMEOUT;
            }
            yield();
            अनुवर्त;
        }
        यदि (i >= receive) {
            यदि (sts & 0x40) {
                floppy_disable_controller();
                प्रत्यागम DISK_RET_ECONTROLLER;
            }
            भङ्ग;
        }
        यदि (!(sts & 0x40)) {
            floppy_disable_controller();
            प्रत्यागम DISK_RET_ECONTROLLER;
        }
        param[i++] = inb(पत्तनम्_FD_दत्त);
    }

    प्रत्यागम DISK_RET_SUCCESS;
}

स्थिर पूर्णाङ्क
floppy_enable_controller(शून्य)
{
    dprintf(2, "Floppy_enable_controller\n");
    // Clear the reset bit (enter reset state), but set 'enable IRQ and DMA'
    floppy_dor_आवरणम्(FLOPPY_DOR_RESET, FLOPPY_DOR_IRQ);
    // Real hardware needs a 4 microsecond delay
    usleep(4);
    // Set the reset bit (normal operation) and keep 'enable IRQ and DMA' on
    floppy_dor_आवरणम्(0, FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET);
    पूर्णाङ्क ret = floppy_wait_irq();
    यदि (ret)
        प्रत्यागम ret;

    // After the interrupt is received, send 4 SENSE INTERRUPT commands to
    // clear the interrupt status for each of the four logical drives,
    // supported by the controller.
    // See section 7.4 - "Drive Polling" of the Intel 82077AA datasheet for
    // a more detailed description of why this voodoo needs to be done.
    // Without this, initialization fails on real controllers (but still works
    // in QEMU)
    u8 param[2];
    पूर्णाङ्क i;
    प्रति (i=0; i<4; i++) {
        ret = floppy_pio(FC_CHECKIRQ, param);
        यदि (ret)
            प्रत्यागम ret;
    }
    प्रत्यागम DISK_RET_SUCCESS;
}

// Activate a drive and send a command to it.
स्थिर पूर्णाङ्क
floppy_drive_pio(u8 floppyid, पूर्णाङ्क command, u8 *param)
{
    // Enable controller if it isn't running.
    यदि (!(floppy_dor_read() & FLOPPY_DOR_RESET)) {
        पूर्णाङ्क ret = floppy_enable_controller();
        यदि (ret)
            प्रत्यागम ret;
    }

    // set the disk motor timeout value of INT 08 to the highest value
    समुच्चयः_BDA(floppy_motor_गणक, 255);

    // Check if the motor is already running
    u8 motor_आवरणम् = FLOPPY_DOR_MOTOR_A << floppyid;
    पूर्णाङ्क motor_already_running = floppy_dor_read() & motor_आवरणम्;

    // Turn on motor of selected drive, DMA & int enabled, normal operation
    floppy_dor_write(motor_आवरणम् | FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET | floppyid);

    // If the motor was just started, wait for it to get up to speed
    यदि (!motor_already_running && !CONFIG_QEMU)
        msleep(FLOPPY_STARTUP_TIME * 125);

    // Send command.
    पूर्णाङ्क ret = floppy_pio(command, param);
    समुच्चयः_BDA(floppy_motor_गणक, FLOPPY_MOTOR_TICKS); // reset motor timeout
    यदि (ret)
        प्रत्यागम ret;

    // Check IRQ command is needed after irq commands with no results
    यदि ((command & FCF_WAITIRQ) && ((command >> 12) & 0xf) == 0)
        प्रत्यागम floppy_pio(FC_CHECKIRQ, param);
    प्रत्यागम DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy media sense and seeking
 ****************************************************************/

स्थिर पूर्णाङ्क
floppy_drive_recal(u8 floppyid)
{
    dprintf(2, "Floppy_drive_recal %d\n", floppyid);
    // send Recalibrate command to controller
    u8 param[2];
    param[0] = floppyid;
    पूर्णाङ्क ret = floppy_drive_pio(floppyid, FC_RECALIBRATE, param);
    यदि (ret)
        प्रत्यागम ret;

    u8 frs = GET_BDA(floppy_recalibration_स्थितिः);
    समुच्चयः_BDA(floppy_recalibration_स्थितिः, frs | (1<<floppyid));
    समुच्चयः_BDA(floppy_track[floppyid], 0);
    प्रत्यागम DISK_RET_SUCCESS;
}

स्थिर पूर्णाङ्क
floppy_drive_seek(u8 floppyid, u8 cylinder)
{
    dprintf(2, "Floppy_drive_seek %d to %d\n", floppyid, cylinder);
    u8 param[2];
    param[0] = floppyid;
    param[1] = cylinder;
    पूर्णाङ्क ret = floppy_drive_pio(floppyid, FC_SEEK, param);
    यदि (ret)
        प्रत्यागम ret;

    समुच्चयः_BDA(floppy_track[floppyid], cylinder);
    प्रत्यागम DISK_RET_SUCCESS;
}

स्थिर पूर्णाङ्क
floppy_drive_specify(शून्य)
{
    u8 param[2];
    param[0] = FLOPPY_SPECIFY1;
    param[1] = FLOPPY_SPECIFY2;
    प्रत्यागम floppy_pio(FC_SPECIFY, param);
}

स्थिर पूर्णाङ्क
floppy_drive_readid(u8 floppyid, u8 दत्त_दरः, u8 शीर्ष)
{
    // Set data rate.
    outb(दत्त_दरः, पत्तनम्_FD_DIR);

    // send Read Sector Id command
    u8 param[7];
    param[0] = (शीर्ष << 2) | floppyid; // HD DR1 DR2
    पूर्णाङ्क ret = floppy_drive_pio(floppyid, FC_READID, param);
    यदि (ret)
        प्रत्यागम ret;
    यदि (param[0] & 0xc0)
        प्रत्यागम -1;
    प्रत्यागम 0;
}

// Only the 1.2MB 5.25" drive and the 3.5" drives have a disk change line.  On
// the older 5.25" types the pin is not driven and reads back as a permanent
// "changed".
स्थिर पूर्णाङ्क
floppy_has_changeline(u8 ftype)
{
    प्रत्यागम ftype >= 2 && ftype <= 5;
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
स्थिर पूर्णाङ्क
floppy_check_media(संरचना drive_s *drive_gf, u8 floppyid)
{
    यदि (!floppy_has_changeline(GET_GLOBALFLAT(drive_gf->floppy_type)))
        प्रत्यागम DISK_RET_SUCCESS;

    // The caller has just recalibrated this drive, so it is selected in the
    // DOR and the DIR reflects its change line.
    यदि (!(inb(पत्तनम्_FD_DIR) & FLOPPY_DIR_DSKCHG))
        प्रत्यागम DISK_RET_SUCCESS;

    पूर्णाङ्क ret = floppy_drive_seek(floppyid, 1);
    यदि (ret)
        प्रत्यागम ret;

    यदि (inb(पत्तनम्_FD_DIR) & FLOPPY_DIR_DSKCHG) {
        dprintf(2, "Floppy_check_media %d: no medium\n", floppyid);
        प्रत्यागम DISK_RET_ETIMEOUT;
    }

    // A medium is present.  Put the head back on cylinder 0, where the caller
    // left it, so that media sensing reads the track it always did.
    प्रत्यागम floppy_drive_seek(floppyid, 0);
}

स्थिर पूर्णाङ्क
floppy_media_sense(संरचना drive_s *drive_gf)
{
    u8 ftype = GET_GLOBALFLAT(drive_gf->floppy_type), stype = ftype;
    u8 floppyid = GET_GLOBALFLAT(drive_gf->cntl_id);

    u8 दत्त_दरः = GET_GLOBAL(FloppyInfo[stype].दत्त_दरः);
    पूर्णाङ्क ret = floppy_drive_readid(floppyid, दत्त_दरः, 0);
    यदि (ret) {
        // Attempt media sense.
        प्रति (stype=1; ; stype++) {
            यदि (stype >= ARRAY_आकार(FloppyInfo))
                प्रत्यागम DISK_RET_EBADTRACK;
            यदि (stype==ftype
                || (GET_GLOBAL(FloppyInfo[stype].floppy_आकार)
                    != GET_GLOBAL(FloppyInfo[ftype].floppy_आकार))
                || (GET_GLOBAL(FloppyInfo[stype].chs.शीर्ष)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.शीर्ष))
                || (GET_GLOBAL(FloppyInfo[stype].chs.cylinder)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.cylinder))
                || (GET_GLOBAL(FloppyInfo[stype].chs.sector)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.sector)))
                अनुवर्त;
            दत्त_दरः = GET_GLOBAL(FloppyInfo[stype].दत्त_दरः);
            ret = floppy_drive_readid(floppyid, दत्त_दरः, 0);
            यदि (!ret)
                भङ्ग;
        }
    }
    dprintf(2, "Floppy_media_sense on drive %d found rate %d\n"
            , floppyid, दत्त_दरः);

    u8 old_दत्त_दरः = GET_BDA(floppy_media_state[floppyid]) >> 6;
    समुच्चयः_BDA(floppy_चरमः_दत्त_दरः, (old_दत्त_दरः<<2) | (दत्त_दरः<<6));
    u8 media = (stype == 1 ? 0x04 : (stype == 2 ? 0x05 : 0x07));
    u8 fms = (दत्त_दरः<<6) | FMS_MEDIA_DRIVE_ESTABLISHED | media;
    यदि (GET_GLOBAL(FloppyInfo[stype].chs.cylinder)
        < GET_GLOBAL(FloppyInfo[ftype].chs.cylinder))
        fms |= FMS_DOUBLE_STEPPING;
    समुच्चयः_BDA(floppy_media_state[floppyid], fms);

    प्रत्यागम DISK_RET_SUCCESS;
}

// Prepare a floppy for a data transfer.
स्थिर पूर्णाङ्क
floppy_prep(संरचना drive_s *drive_gf, u8 cylinder)
{
    u8 floppyid = GET_GLOBALFLAT(drive_gf->cntl_id);
    यदि (!(GET_BDA(floppy_recalibration_स्थितिः) & (1<<floppyid)) ||
        !(GET_BDA(floppy_media_state[floppyid]) & FMS_MEDIA_DRIVE_ESTABLISHED)) {
        // Recalibrate drive.
        पूर्णाङ्क ret = floppy_drive_recal(floppyid);
        यदि (ret)
            प्रत्यागम ret;

        // An empty drive gives no index pulses, so the READ ID that
        // floppy_media_sense() issues would never complete and would wait out
        // its timeout once per data rate.  Ask the disk change line instead.
        ret = floppy_check_media(drive_gf, floppyid);
        यदि (ret)
            प्रत्यागम ret;

        // Sense media.
        ret = floppy_media_sense(drive_gf);
        यदि (ret)
            प्रत्यागम ret;

        // Execute a SPECIFY command (sets the Step Rate Time,
        // Head Load Time, Head Unload Time and the DMA enable/disable bit).
        ret = floppy_drive_specify();
        यदि (ret)
            प्रत्यागम ret;
    }

    // Seek to cylinder if needed.
    यदि (cylinder != GET_BDA(floppy_track[floppyid])) {
        पूर्णाङ्क ret = floppy_drive_seek(floppyid, cylinder);
        यदि (ret)
            प्रत्यागम ret;
    }

    प्रत्यागम DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy DMA transfer
 ****************************************************************/

// Perform a floppy transfer command (setup DMA and issue PIO).
स्थिर पूर्णाङ्क
floppy_dma_cmd(संरचना disk_op_s *op, पूर्णाङ्क गणनम्, पूर्णाङ्क command, u8 *param)
{
    // Setup DMA controller
    पूर्णाङ्क isWrite = command != FC_READ;
    पूर्णाङ्क ret = dma_floppy((u32)op->buf_fl, गणनम्, isWrite);
    यदि (ret)
        प्रत्यागम DISK_RET_EBOUNDARY;

    // Invoke floppy controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    ret = floppy_drive_pio(floppyid, command, param);
    यदि (ret)
        प्रत्यागम ret;

    // Populate floppy_return_status in BDA
    पूर्णाङ्क i;
    प्रति (i=0; i<7; i++)
        समुच्चयः_BDA(floppy_return_स्थितिः[i], param[i]);

    यदि (param[0] & 0xc0) {
        यदि (param[1] & 0x02)
            प्रत्यागम DISK_RET_EWRITEPROTECT;
        dprintf(1, "floppy error: %02x %02x %02x %02x %02x %02x %02x\n"
                , param[0], param[1], param[2], param[3]
                , param[4], param[5], param[6]);
        प्रत्यागम DISK_RET_ECONTROLLER;
    }

    प्रत्यागम DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy handlers
 ****************************************************************/

स्थिर संरचना chs_s
lba2chs(संरचना disk_op_s *op)
{
    संरचना chs_s res = { };

    u32 tmp = op->lba;
    u16 nls = GET_GLOBALFLAT(op->drive_fl->lchs.sector);
    res.sector = (tmp % nls) + 1;

    tmp /= nls;
    u16 nlh = GET_GLOBALFLAT(op->drive_fl->lchs.शीर्ष);
    res.शीर्ष = tmp % nlh;

    tmp /= nlh;
    res.cylinder = tmp;

    प्रत्यागम res;
}

// diskette controller reset
स्थिर पूर्णाङ्क
floppy_reset(संरचना disk_op_s *op)
{
    समुच्चयः_BDA(floppy_recalibration_स्थितिः, 0);
    समुच्चयः_BDA(floppy_media_state[0], 0);
    समुच्चयः_BDA(floppy_media_state[1], 0);
    समुच्चयः_BDA(floppy_track[0], 0);
    समुच्चयः_BDA(floppy_track[1], 0);
    समुच्चयः_BDA(floppy_चरमः_दत्त_दरः, 0);
    floppy_disable_controller();
    प्रत्यागम floppy_enable_controller();
}

// Read Diskette Sectors
स्थिर पूर्णाङ्क
floppy_read(संरचना disk_op_s *op)
{
    संरचना chs_s chs = lba2chs(op);
    पूर्णाङ्क ret = floppy_prep(op->drive_fl, chs.cylinder);
    यदि (ret)
        प्रत्यागम ret;

    // send read-normal-data command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[8];
    param[0] = (chs.शीर्ष << 2) | floppyid; // HD DR1 DR2
    param[1] = chs.cylinder;
    param[2] = chs.शीर्ष;
    param[3] = chs.sector;
    param[4] = FLOPPY_आकार_संहिता;
    param[5] = chs.sector + op->गणनम् - 1; // last sector to read on track
    param[6] = FLOPPY_GAPLEN;
    param[7] = FLOPPY_DATALEN;
    प्रत्यागम floppy_dma_cmd(op, op->गणनम् * DISK_SECTOR_आकार, FC_READ, param);
}

// Write Diskette Sectors
स्थिर पूर्णाङ्क
floppy_write(संरचना disk_op_s *op)
{
    संरचना chs_s chs = lba2chs(op);
    पूर्णाङ्क ret = floppy_prep(op->drive_fl, chs.cylinder);
    यदि (ret)
        प्रत्यागम ret;

    // send write-normal-data command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[8];
    param[0] = (chs.शीर्ष << 2) | floppyid; // HD DR1 DR2
    param[1] = chs.cylinder;
    param[2] = chs.शीर्ष;
    param[3] = chs.sector;
    param[4] = FLOPPY_आकार_संहिता;
    param[5] = chs.sector + op->गणनम् - 1; // last sector to write on track
    param[6] = FLOPPY_GAPLEN;
    param[7] = FLOPPY_DATALEN;
    प्रत्यागम floppy_dma_cmd(op, op->गणनम् * DISK_SECTOR_आकार, FC_WRITE, param);
}

// Verify Diskette Sectors
स्थिर पूर्णाङ्क
floppy_verify(संरचना disk_op_s *op)
{
    संरचना chs_s chs = lba2chs(op);
    पूर्णाङ्क ret = floppy_prep(op->drive_fl, chs.cylinder);
    यदि (ret)
        प्रत्यागम ret;

    // This command isn't implemented - just return success.
    प्रत्यागम DISK_RET_SUCCESS;
}

// format diskette track
स्थिर पूर्णाङ्क
floppy_format(संरचना disk_op_s *op)
{
    संरचना chs_s chs = lba2chs(op);
    पूर्णाङ्क ret = floppy_prep(op->drive_fl, chs.cylinder);
    यदि (ret)
        प्रत्यागम ret;

    // send format-track command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[7];
    param[0] = (chs.शीर्ष << 2) | floppyid; // HD DR1 DR2
    param[1] = FLOPPY_आकार_संहिता;
    param[2] = op->गणनम्; // number of sectors per track
    param[3] = FLOPPY_FORMAT_GAPLEN;
    param[4] = FLOPPY_FILLBYTE;
    प्रत्यागम floppy_dma_cmd(op, op->गणनम् * 4, FC_FORMAT, param);
}

पूर्णाङ्क
floppy_process_op(संरचना disk_op_s *op)
{
    यदि (!CONFIG_FLOPPY)
        प्रत्यागम 0;

    विकल्प (op->command) {
    पक्ष CMD_RESET:
        प्रत्यागम floppy_reset(op);
    पक्ष CMD_READ:
        प्रत्यागम floppy_read(op);
    पक्ष CMD_WRITE:
        प्रत्यागम floppy_write(op);
    पक्ष CMD_VERIFY:
        प्रत्यागम floppy_verify(op);
    पक्ष CMD_FORMAT:
        प्रत्यागम floppy_format(op);
    स्वभाव:
        प्रत्यागम DISK_RET_EPARAM;
    }
}


/****************************************************************
 * HW irqs
 ****************************************************************/

// INT 0Eh Diskette Hardware ISR Entry Point
शून्य VISIBLE16
handle_0e(शून्य)
{
    यदि (! CONFIG_FLOPPY)
        प्रत्यागम;
    debug_isr(DEBUG_ISR_0e);

    // diskette interrupt has occurred
    u8 frs = GET_BDA(floppy_recalibration_स्थितिः);
    समुच्चयः_BDA(floppy_recalibration_स्थितिः, frs | FRS_IRQ);

    pic_eoi1();
}

// Called from int08 handler.
शून्य
floppy_tick(शून्य)
{
    यदि (! CONFIG_FLOPPY)
        प्रत्यागम;

    // time to turn off drive(s)?
    u8 fcount = GET_BDA(floppy_motor_गणक);
    यदि (fcount) {
        fcount--;
        समुच्चयः_BDA(floppy_motor_गणक, fcount);
        यदि (fcount == 0)
            // turn motor(s) off
            floppy_dor_आवरणम्(FLOPPY_DOR_MOTOR_आवरणम्, 0);
    }
}

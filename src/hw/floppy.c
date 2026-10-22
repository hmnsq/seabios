// 16bit code to access floppy drives.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // SET_BDA
#شامل "block.h" // struct drive_s
#شامل "bregs.h" // struct bregs
#شامل "config.h" // CONFIG_FLOPPY
#شامل "malloc.h" // malloc_fseg
#شامل "output.h" // dprintf
#شامل "pcidevice.h" // pci_find_class
#شامل "pci_ids.h" // PCI_CLASS_BRIDGE_ISA
#شامل "pic.h" // pic_eoi1
#شامل "romfile.h" // romfile_loadint
#شامل "rtc.h" // rtc_read
#شامل "stacks.h" // yield
#شامل "std/disk.h" // DISK_RET_SUCCESS
#شامل "string.h" // memset
#شامل "util.h" // timer_calc

#تعریف_کرو پورٹ_FD_بنیاد           0x03f0
#تعریف_کرو پورٹ_FD_DOR            0x03f2
#تعریف_کرو پورٹ_FD_حالت         0x03f4
#تعریف_کرو پورٹ_FD_ڈیٹا           0x03f5
#تعریف_کرو پورٹ_FD_DIR            0x03f7

// Digital Input Register (read)
#تعریف_کرو FLOPPY_DIR_DSKCHG      0x80

#تعریف_کرو FLOPPY_سائز_رمز 0x02 // 512 byte sectors
#تعریف_کرو FLOPPY_DATALEN 0xff   // Not used - because size code is 0x02
#تعریف_کرو FLOPPY_MOTOR_TICKS 37 // ~2 seconds
#تعریف_کرو FLOPPY_FILLBYTE 0xf6
#تعریف_کرو FLOPPY_GAPLEN 0x1B
#تعریف_کرو FLOPPY_FORMAT_GAPLEN 0x6c
#تعریف_کرو FLOPPY_PIO_TIMEOUT 1000
#تعریف_کرو FLOPPY_IRQ_TIMEOUT 5000
#تعریف_کرو FLOPPY_SPECIFY1 0xAF  // step rate 12ms, head unload 240ms
#تعریف_کرو FLOPPY_SPECIFY2 0x02  // head load time 4ms, DMA used
#تعریف_کرو FLOPPY_STARTUP_وقت 8 // 1 second

#تعریف_کرو FLOPPY_DOR_MOTOR_D     0x80 // Set to turn drive 3's motor ON
#تعریف_کرو FLOPPY_DOR_MOTOR_C     0x40 // Set to turn drive 2's motor ON
#تعریف_کرو FLOPPY_DOR_MOTOR_B     0x20 // Set to turn drive 1's motor ON
#تعریف_کرو FLOPPY_DOR_MOTOR_A     0x10 // Set to turn drive 0's motor ON
#تعریف_کرو FLOPPY_DOR_MOTOR_نقاب  0xf0
#تعریف_کرو FLOPPY_DOR_IRQ         0x08 // Set to enable IRQs and DMA
#تعریف_کرو FLOPPY_DOR_RESET       0x04 // Clear = enter reset mode, Set = normal operation
#تعریف_کرو FLOPPY_DOR_DSEL_نقاب   0x03 // "Select" drive number for next access

// New diskette parameter table adding 3 parameters from IBM
// Since no provisions are made for multiple drive types, most
// values in this table are ignored.  I set parameters for 1.44M
// floppy here
ساخت floppy_ext_dbt_s diskette_param_table2 VARFSEG = {
    .dbt = {
        .specify1       = FLOPPY_SPECIFY1,
        .specify2       = FLOPPY_SPECIFY2,
        .shutoff_ticks  = FLOPPY_MOTOR_TICKS, // ~2 seconds
        .bps_رمز       = FLOPPY_سائز_رمز,
        .sectors        = 18,
        .interblock_len = FLOPPY_GAPLEN,
        .ڈیٹا_len       = FLOPPY_DATALEN,
        .gap_len        = FLOPPY_FORMAT_GAPLEN,
        .fill_byte      = FLOPPY_FILLBYTE,
        .settle_وقت    = 0x0F, // 15ms
        .startup_وقت   = FLOPPY_STARTUP_وقت,
    },
    .max_track      = 79,   // maximum track
    .ڈیٹا_شرح      = 0,    // data transfer rate
    .drive_type     = 4,    // drive type in cmos
};

ساخت floppyinfo_s {
    ساخت chs_s chs;
    u8 floppy_سائز;
    u8 ڈیٹا_شرح;
};

#تعریف_کرو FLOPPY_سائز_525 0x01
#تعریف_کرو FLOPPY_سائز_350 0x02

#تعریف_کرو FLOPPY_شرح_500_K 0x00
#تعریف_کرو FLOPPY_شرح_300_K 0x01
#تعریف_کرو FLOPPY_شرح_250_K 0x02
#تعریف_کرو FLOPPY_شرح_1_M   0x03

ساخت floppyinfo_s FloppyInfo[] VARFSEG = {
    // Unknown
    { {0, 0, 0}, 0x00, 0x00},
    // 1 - 360KB, 5.25" - 2 heads, 40 tracks, 9 sectors
    { {2, 40, 9}, FLOPPY_سائز_525, FLOPPY_شرح_300_K},
    // 2 - 1.2MB, 5.25" - 2 heads, 80 tracks, 15 sectors
    { {2, 80, 15}, FLOPPY_سائز_525, FLOPPY_شرح_500_K},
    // 3 - 720KB, 3.5"  - 2 heads, 80 tracks, 9 sectors
    { {2, 80, 9}, FLOPPY_سائز_350, FLOPPY_شرح_250_K},
    // 4 - 1.44MB, 3.5" - 2 heads, 80 tracks, 18 sectors
    { {2, 80, 18}, FLOPPY_سائز_350, FLOPPY_شرح_500_K},
    // 5 - 2.88MB, 3.5" - 2 heads, 80 tracks, 36 sectors
    { {2, 80, 36}, FLOPPY_سائز_350, FLOPPY_شرح_1_M},
    // 6 - 160k, 5.25"  - 1 heads, 40 tracks, 8 sectors
    { {1, 40, 8}, FLOPPY_سائز_525, FLOPPY_شرح_250_K},
    // 7 - 180k, 5.25"  - 1 heads, 40 tracks, 9 sectors
    { {1, 40, 9}, FLOPPY_سائز_525, FLOPPY_شرح_300_K},
    // 8 - 320k, 5.25"  - 2 heads, 40 tracks, 8 sectors
    { {2, 40, 8}, FLOPPY_سائز_525, FLOPPY_شرح_250_K},
};

ساخت drive_s *
init_floppy(صحیح floppyid, صحیح ftype)
{
    اگر (ftype <= 0 || ftype >= ARRAY_سائز(FloppyInfo)) {
        dprintf(1, "Bad floppy type %d\n", ftype);
        واپس تہی;
    }

    ساخت drive_s *drive = malloc_fseg(حجم(*drive));
    اگر (!drive) {
        warn_noalloc();
        واپس تہی;
    }
    memset(drive, 0, حجم(*drive));
    drive->cntl_id = floppyid;
    drive->type = DTYPE_FLOPPY;
    drive->blksize = DISK_SECTOR_سائز;
    drive->floppy_type = ftype;
    drive->sectors = (u64)-1;

    حافظہ_نقل(&drive->lchs, &FloppyInfo[ftype].chs
           , حجم(FloppyInfo[ftype].chs));
    واپس drive;
}

ساکن خالی
جوڑو_Floppy(صحیح floppyid, صحیح ftype)
{
    ساخت drive_s *drive = init_floppy(floppyid, ftype);
    اگر (!drive)
        واپس;
    حرف *desc = znprintf(MAXDESCSIZE, "Floppy [drive %c]", 'A' + floppyid);
    ساخت pci_device *pci = pci_ڈھونڈو_class(PCI_CLASS_BRIDGE_ISA); /* isa-to-pci bridge */
    صحیح prio = bootprio_ڈھونڈو_fdc_device(pci, پورٹ_FD_بنیاد, floppyid);
    boot_جوڑو_floppy(drive, desc, prio);
}

خالی
floppy_تیاری(خالی)
{
    حافظہ_نقل(&diskette_param_جدول, &diskette_param_table2
           , حجم(diskette_param_جدول));
    سیٹ_IVT(0x1E, SEGOFF(SEG_BIOS
                         , (u32)&diskette_param_table2 - BUILD_BIOS_ADDR));

    اگر (! CONFIG_FLOPPY)
        واپس;
    dprintf(3, "init floppy drives\n");

    اگر (CONFIG_QEMU) {
        u8 type = rtc_read(CMOS_FLOPPY_DRIVE_TYPE);
        اگر (type & 0xf0)
            جوڑو_Floppy(0, type >> 4);
        اگر (type & 0x0f)
            جوڑو_Floppy(1, type & 0x0f);
    } ورنہ {
        u8 type = romfile_loadint("etc/floppy0", 0);
        اگر (type)
            جوڑو_Floppy(0, type);
        type = romfile_loadint("etc/floppy1", 0);
        اگر (type)
            جوڑو_Floppy(1, type);
    }

    enable_hwirq(6, FUNC16(اندراج_0e));
}

// Find a floppy type that matches a given image size.
صحیح
ڈھونڈو_floppy_type(u32 سائز)
{
    صحیح i;
    برائے (i=1; i<ARRAY_سائز(FloppyInfo); i++) {
        ساخت chs_s *c = &FloppyInfo[i].chs;
        اگر (c->cylinder * c->سر * c->sector * DISK_SECTOR_سائز == سائز)
            واپس i;
    }
    واپس -1;
}


/****************************************************************
 * Low-level floppy IO
 ****************************************************************/

u8 FloppyDOR VARLOW;

ساکن اندرونی u8
floppy_dor_read(خالی)
{
    واپس GET_کم(FloppyDOR);
}

ساکن اندرونی خالی
floppy_dor_write(u8 val)
{
    outb(val, پورٹ_FD_DOR);
    سیٹ_کم(FloppyDOR, val);
}

ساکن اندرونی خالی
floppy_dor_نقاب(u8 off, u8 on)
{
    floppy_dor_write((floppy_dor_read() & ~off) | on);
}

ساکن خالی
floppy_disable_controller(خالی)
{
    dprintf(2, "Floppy_disable_controller\n");
    // Clear the reset bit (enter reset state) and clear 'enable IRQ and DMA'
    floppy_dor_نقاب(FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET, 0);
}

ساکن صحیح
floppy_wait_irq(خالی)
{
    u8 frs = GET_BDA(floppy_recalibration_حالت);
    سیٹ_BDA(floppy_recalibration_حالت, frs & ~FRS_IRQ);
    u32 انتہا = timer_calc(FLOPPY_IRQ_TIMEOUT);
    برائے (;;) {
        اگر (timer_check(انتہا)) {
            warn_timeout();
            floppy_disable_controller();
            واپس DISK_RET_ETIMEOUT;
        }
        frs = GET_BDA(floppy_recalibration_حالت);
        اگر (frs & FRS_IRQ)
            توڑو;
        // Could use yield_toirq() here, but that causes issues on
        // bochs, so use yield() instead.
        yield();
    }

    سیٹ_BDA(floppy_recalibration_حالت, frs & ~FRS_IRQ);
    واپس DISK_RET_SUCCESS;
}

// Floppy commands
#تعریف_کرو FCF_WAITIRQ 0x10000
#تعریف_کرو FC_CHECKIRQ    (0x08 | (0<<8) | (2<<12))
#تعریف_کرو FC_SEEK        (0x0f | (2<<8) | (0<<12) | FCF_WAITIRQ)
#تعریف_کرو FC_RECALIBRATE (0x07 | (1<<8) | (0<<12) | FCF_WAITIRQ)
#تعریف_کرو FC_READID      (0x4a | (1<<8) | (7<<12) | FCF_WAITIRQ)
#تعریف_کرو FC_READ        (0xe6 | (8<<8) | (7<<12) | FCF_WAITIRQ)
#تعریف_کرو FC_WRITE       (0xc5 | (8<<8) | (7<<12) | FCF_WAITIRQ)
#تعریف_کرو FC_FORMAT      (0x4d | (5<<8) | (7<<12) | FCF_WAITIRQ)
#تعریف_کرو FC_SPECIFY     (0x03 | (2<<8) | (0<<12))

// Send the specified command and it's parameters to the floppy controller.
ساکن صحیح
floppy_pio(صحیح command, u8 *param)
{
    dprintf(9, "Floppy pio command %x\n", command);
    // Send command and parameters to controller.
    u32 انتہا = timer_calc(FLOPPY_PIO_TIMEOUT);
    صحیح send = (command >> 8) & 0xf;
    صحیح i = 0;
    برائے (;;) {
        u8 sts = inb(پورٹ_FD_حالت);
        اگر (!(sts & 0x80)) {
            اگر (timer_check(انتہا)) {
                warn_timeout();
                floppy_disable_controller();
                واپس DISK_RET_ETIMEOUT;
            }
            yield();
            جاری;
        }
        اگر (sts & 0x40) {
            floppy_disable_controller();
            واپس DISK_RET_ECONTROLLER;
        }
        اگر (i == 0)
            outb(command & 0xff, پورٹ_FD_ڈیٹا);
        ورنہ
            outb(param[i-1], پورٹ_FD_ڈیٹا);
        اگر (i++ >= send)
            توڑو;
    }

    // Wait for command to complete.
    اگر (command & FCF_WAITIRQ) {
        صحیح ret = floppy_wait_irq();
        اگر (ret)
            واپس ret;
    }

    // Read response from controller.
    انتہا = timer_calc(FLOPPY_PIO_TIMEOUT);
    صحیح receive = (command >> 12) & 0xf;
    i = 0;
    برائے (;;) {
        u8 sts = inb(پورٹ_FD_حالت);
        اگر (!(sts & 0x80)) {
            اگر (timer_check(انتہا)) {
                warn_timeout();
                floppy_disable_controller();
                واپس DISK_RET_ETIMEOUT;
            }
            yield();
            جاری;
        }
        اگر (i >= receive) {
            اگر (sts & 0x40) {
                floppy_disable_controller();
                واپس DISK_RET_ECONTROLLER;
            }
            توڑو;
        }
        اگر (!(sts & 0x40)) {
            floppy_disable_controller();
            واپس DISK_RET_ECONTROLLER;
        }
        param[i++] = inb(پورٹ_FD_ڈیٹا);
    }

    واپس DISK_RET_SUCCESS;
}

ساکن صحیح
floppy_enable_controller(خالی)
{
    dprintf(2, "Floppy_enable_controller\n");
    // Clear the reset bit (enter reset state), but set 'enable IRQ and DMA'
    floppy_dor_نقاب(FLOPPY_DOR_RESET, FLOPPY_DOR_IRQ);
    // Real hardware needs a 4 microsecond delay
    usleep(4);
    // Set the reset bit (normal operation) and keep 'enable IRQ and DMA' on
    floppy_dor_نقاب(0, FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET);
    صحیح ret = floppy_wait_irq();
    اگر (ret)
        واپس ret;

    // After the interrupt is received, send 4 SENSE INTERRUPT commands to
    // clear the interrupt status for each of the four logical drives,
    // supported by the controller.
    // See section 7.4 - "Drive Polling" of the Intel 82077AA datasheet for
    // a more detailed description of why this voodoo needs to be done.
    // Without this, initialization fails on real controllers (but still works
    // in QEMU)
    u8 param[2];
    صحیح i;
    برائے (i=0; i<4; i++) {
        ret = floppy_pio(FC_CHECKIRQ, param);
        اگر (ret)
            واپس ret;
    }
    واپس DISK_RET_SUCCESS;
}

// Activate a drive and send a command to it.
ساکن صحیح
floppy_drive_pio(u8 floppyid, صحیح command, u8 *param)
{
    // Enable controller if it isn't running.
    اگر (!(floppy_dor_read() & FLOPPY_DOR_RESET)) {
        صحیح ret = floppy_enable_controller();
        اگر (ret)
            واپس ret;
    }

    // set the disk motor timeout value of INT 08 to the highest value
    سیٹ_BDA(floppy_motor_شمار, 255);

    // Check if the motor is already running
    u8 motor_نقاب = FLOPPY_DOR_MOTOR_A << floppyid;
    صحیح motor_already_running = floppy_dor_read() & motor_نقاب;

    // Turn on motor of selected drive, DMA & int enabled, normal operation
    floppy_dor_write(motor_نقاب | FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET | floppyid);

    // If the motor was just started, wait for it to get up to speed
    اگر (!motor_already_running && !CONFIG_QEMU)
        msleep(FLOPPY_STARTUP_وقت * 125);

    // Send command.
    صحیح ret = floppy_pio(command, param);
    سیٹ_BDA(floppy_motor_شمار, FLOPPY_MOTOR_TICKS); // reset motor timeout
    اگر (ret)
        واپس ret;

    // Check IRQ command is needed after irq commands with no results
    اگر ((command & FCF_WAITIRQ) && ((command >> 12) & 0xf) == 0)
        واپس floppy_pio(FC_CHECKIRQ, param);
    واپس DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy media sense and seeking
 ****************************************************************/

ساکن صحیح
floppy_drive_recal(u8 floppyid)
{
    dprintf(2, "Floppy_drive_recal %d\n", floppyid);
    // send Recalibrate command to controller
    u8 param[2];
    param[0] = floppyid;
    صحیح ret = floppy_drive_pio(floppyid, FC_RECALIBRATE, param);
    اگر (ret)
        واپس ret;

    u8 frs = GET_BDA(floppy_recalibration_حالت);
    سیٹ_BDA(floppy_recalibration_حالت, frs | (1<<floppyid));
    سیٹ_BDA(floppy_track[floppyid], 0);
    واپس DISK_RET_SUCCESS;
}

ساکن صحیح
floppy_drive_seek(u8 floppyid, u8 cylinder)
{
    dprintf(2, "Floppy_drive_seek %d to %d\n", floppyid, cylinder);
    u8 param[2];
    param[0] = floppyid;
    param[1] = cylinder;
    صحیح ret = floppy_drive_pio(floppyid, FC_SEEK, param);
    اگر (ret)
        واپس ret;

    سیٹ_BDA(floppy_track[floppyid], cylinder);
    واپس DISK_RET_SUCCESS;
}

ساکن صحیح
floppy_drive_specify(خالی)
{
    u8 param[2];
    param[0] = FLOPPY_SPECIFY1;
    param[1] = FLOPPY_SPECIFY2;
    واپس floppy_pio(FC_SPECIFY, param);
}

ساکن صحیح
floppy_drive_readid(u8 floppyid, u8 ڈیٹا_شرح, u8 سر)
{
    // Set data rate.
    outb(ڈیٹا_شرح, پورٹ_FD_DIR);

    // send Read Sector Id command
    u8 param[7];
    param[0] = (سر << 2) | floppyid; // HD DR1 DR2
    صحیح ret = floppy_drive_pio(floppyid, FC_READID, param);
    اگر (ret)
        واپس ret;
    اگر (param[0] & 0xc0)
        واپس -1;
    واپس 0;
}

// Only the 1.2MB 5.25" drive and the 3.5" drives have a disk change line.  On
// the older 5.25" types the pin is not driven and reads back as a permanent
// "changed".
ساکن صحیح
floppy_has_changeline(u8 ftype)
{
    واپس ftype >= 2 && ftype <= 5;
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
ساکن صحیح
floppy_check_media(ساخت drive_s *drive_gf, u8 floppyid)
{
    اگر (!floppy_has_changeline(GET_GLOBALFLAT(drive_gf->floppy_type)))
        واپس DISK_RET_SUCCESS;

    // The caller has just recalibrated this drive, so it is selected in the
    // DOR and the DIR reflects its change line.
    اگر (!(inb(پورٹ_FD_DIR) & FLOPPY_DIR_DSKCHG))
        واپس DISK_RET_SUCCESS;

    صحیح ret = floppy_drive_seek(floppyid, 1);
    اگر (ret)
        واپس ret;

    اگر (inb(پورٹ_FD_DIR) & FLOPPY_DIR_DSKCHG) {
        dprintf(2, "Floppy_check_media %d: no medium\n", floppyid);
        واپس DISK_RET_ETIMEOUT;
    }

    // A medium is present.  Put the head back on cylinder 0, where the caller
    // left it, so that media sensing reads the track it always did.
    واپس floppy_drive_seek(floppyid, 0);
}

ساکن صحیح
floppy_media_sense(ساخت drive_s *drive_gf)
{
    u8 ftype = GET_GLOBALFLAT(drive_gf->floppy_type), stype = ftype;
    u8 floppyid = GET_GLOBALFLAT(drive_gf->cntl_id);

    u8 ڈیٹا_شرح = GET_GLOBAL(FloppyInfo[stype].ڈیٹا_شرح);
    صحیح ret = floppy_drive_readid(floppyid, ڈیٹا_شرح, 0);
    اگر (ret) {
        // Attempt media sense.
        برائے (stype=1; ; stype++) {
            اگر (stype >= ARRAY_سائز(FloppyInfo))
                واپس DISK_RET_EBADTRACK;
            اگر (stype==ftype
                || (GET_GLOBAL(FloppyInfo[stype].floppy_سائز)
                    != GET_GLOBAL(FloppyInfo[ftype].floppy_سائز))
                || (GET_GLOBAL(FloppyInfo[stype].chs.سر)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.سر))
                || (GET_GLOBAL(FloppyInfo[stype].chs.cylinder)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.cylinder))
                || (GET_GLOBAL(FloppyInfo[stype].chs.sector)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.sector)))
                جاری;
            ڈیٹا_شرح = GET_GLOBAL(FloppyInfo[stype].ڈیٹا_شرح);
            ret = floppy_drive_readid(floppyid, ڈیٹا_شرح, 0);
            اگر (!ret)
                توڑو;
        }
    }
    dprintf(2, "Floppy_media_sense on drive %d found rate %d\n"
            , floppyid, ڈیٹا_شرح);

    u8 old_ڈیٹا_شرح = GET_BDA(floppy_media_state[floppyid]) >> 6;
    سیٹ_BDA(floppy_آخری_ڈیٹا_شرح, (old_ڈیٹا_شرح<<2) | (ڈیٹا_شرح<<6));
    u8 media = (stype == 1 ? 0x04 : (stype == 2 ? 0x05 : 0x07));
    u8 fms = (ڈیٹا_شرح<<6) | FMS_MEDIA_DRIVE_ESTABLISHED | media;
    اگر (GET_GLOBAL(FloppyInfo[stype].chs.cylinder)
        < GET_GLOBAL(FloppyInfo[ftype].chs.cylinder))
        fms |= FMS_DOUBLE_STEPPING;
    سیٹ_BDA(floppy_media_state[floppyid], fms);

    واپس DISK_RET_SUCCESS;
}

// Prepare a floppy for a data transfer.
ساکن صحیح
floppy_prep(ساخت drive_s *drive_gf, u8 cylinder)
{
    u8 floppyid = GET_GLOBALFLAT(drive_gf->cntl_id);
    اگر (!(GET_BDA(floppy_recalibration_حالت) & (1<<floppyid)) ||
        !(GET_BDA(floppy_media_state[floppyid]) & FMS_MEDIA_DRIVE_ESTABLISHED)) {
        // Recalibrate drive.
        صحیح ret = floppy_drive_recal(floppyid);
        اگر (ret)
            واپس ret;

        // An empty drive gives no index pulses, so the READ ID that
        // floppy_media_sense() issues would never complete and would wait out
        // its timeout once per data rate.  Ask the disk change line instead.
        ret = floppy_check_media(drive_gf, floppyid);
        اگر (ret)
            واپس ret;

        // Sense media.
        ret = floppy_media_sense(drive_gf);
        اگر (ret)
            واپس ret;

        // Execute a SPECIFY command (sets the Step Rate Time,
        // Head Load Time, Head Unload Time and the DMA enable/disable bit).
        ret = floppy_drive_specify();
        اگر (ret)
            واپس ret;
    }

    // Seek to cylinder if needed.
    اگر (cylinder != GET_BDA(floppy_track[floppyid])) {
        صحیح ret = floppy_drive_seek(floppyid, cylinder);
        اگر (ret)
            واپس ret;
    }

    واپس DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy DMA transfer
 ****************************************************************/

// Perform a floppy transfer command (setup DMA and issue PIO).
ساکن صحیح
floppy_dma_cmd(ساخت disk_op_s *op, صحیح گنتی, صحیح command, u8 *param)
{
    // Setup DMA controller
    صحیح isWrite = command != FC_READ;
    صحیح ret = dma_floppy((u32)op->buf_fl, گنتی, isWrite);
    اگر (ret)
        واپس DISK_RET_EBOUNDARY;

    // Invoke floppy controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    ret = floppy_drive_pio(floppyid, command, param);
    اگر (ret)
        واپس ret;

    // Populate floppy_return_status in BDA
    صحیح i;
    برائے (i=0; i<7; i++)
        سیٹ_BDA(floppy_return_حالت[i], param[i]);

    اگر (param[0] & 0xc0) {
        اگر (param[1] & 0x02)
            واپس DISK_RET_EWRITEPROTECT;
        dprintf(1, "floppy error: %02x %02x %02x %02x %02x %02x %02x\n"
                , param[0], param[1], param[2], param[3]
                , param[4], param[5], param[6]);
        واپس DISK_RET_ECONTROLLER;
    }

    واپس DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy handlers
 ****************************************************************/

ساکن ساخت chs_s
lba2chs(ساخت disk_op_s *op)
{
    ساخت chs_s res = { };

    u32 tmp = op->lba;
    u16 nls = GET_GLOBALFLAT(op->drive_fl->lchs.sector);
    res.sector = (tmp % nls) + 1;

    tmp /= nls;
    u16 nlh = GET_GLOBALFLAT(op->drive_fl->lchs.سر);
    res.سر = tmp % nlh;

    tmp /= nlh;
    res.cylinder = tmp;

    واپس res;
}

// diskette controller reset
ساکن صحیح
floppy_reset(ساخت disk_op_s *op)
{
    سیٹ_BDA(floppy_recalibration_حالت, 0);
    سیٹ_BDA(floppy_media_state[0], 0);
    سیٹ_BDA(floppy_media_state[1], 0);
    سیٹ_BDA(floppy_track[0], 0);
    سیٹ_BDA(floppy_track[1], 0);
    سیٹ_BDA(floppy_آخری_ڈیٹا_شرح, 0);
    floppy_disable_controller();
    واپس floppy_enable_controller();
}

// Read Diskette Sectors
ساکن صحیح
floppy_read(ساخت disk_op_s *op)
{
    ساخت chs_s chs = lba2chs(op);
    صحیح ret = floppy_prep(op->drive_fl, chs.cylinder);
    اگر (ret)
        واپس ret;

    // send read-normal-data command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[8];
    param[0] = (chs.سر << 2) | floppyid; // HD DR1 DR2
    param[1] = chs.cylinder;
    param[2] = chs.سر;
    param[3] = chs.sector;
    param[4] = FLOPPY_سائز_رمز;
    param[5] = chs.sector + op->گنتی - 1; // last sector to read on track
    param[6] = FLOPPY_GAPLEN;
    param[7] = FLOPPY_DATALEN;
    واپس floppy_dma_cmd(op, op->گنتی * DISK_SECTOR_سائز, FC_READ, param);
}

// Write Diskette Sectors
ساکن صحیح
floppy_write(ساخت disk_op_s *op)
{
    ساخت chs_s chs = lba2chs(op);
    صحیح ret = floppy_prep(op->drive_fl, chs.cylinder);
    اگر (ret)
        واپس ret;

    // send write-normal-data command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[8];
    param[0] = (chs.سر << 2) | floppyid; // HD DR1 DR2
    param[1] = chs.cylinder;
    param[2] = chs.سر;
    param[3] = chs.sector;
    param[4] = FLOPPY_سائز_رمز;
    param[5] = chs.sector + op->گنتی - 1; // last sector to write on track
    param[6] = FLOPPY_GAPLEN;
    param[7] = FLOPPY_DATALEN;
    واپس floppy_dma_cmd(op, op->گنتی * DISK_SECTOR_سائز, FC_WRITE, param);
}

// Verify Diskette Sectors
ساکن صحیح
floppy_verify(ساخت disk_op_s *op)
{
    ساخت chs_s chs = lba2chs(op);
    صحیح ret = floppy_prep(op->drive_fl, chs.cylinder);
    اگر (ret)
        واپس ret;

    // This command isn't implemented - just return success.
    واپس DISK_RET_SUCCESS;
}

// format diskette track
ساکن صحیح
floppy_format(ساخت disk_op_s *op)
{
    ساخت chs_s chs = lba2chs(op);
    صحیح ret = floppy_prep(op->drive_fl, chs.cylinder);
    اگر (ret)
        واپس ret;

    // send format-track command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[7];
    param[0] = (chs.سر << 2) | floppyid; // HD DR1 DR2
    param[1] = FLOPPY_سائز_رمز;
    param[2] = op->گنتی; // number of sectors per track
    param[3] = FLOPPY_FORMAT_GAPLEN;
    param[4] = FLOPPY_FILLBYTE;
    واپس floppy_dma_cmd(op, op->گنتی * 4, FC_FORMAT, param);
}

صحیح
floppy_process_op(ساخت disk_op_s *op)
{
    اگر (!CONFIG_FLOPPY)
        واپس 0;

    انتخاب (op->command) {
    حالت CMD_RESET:
        واپس floppy_reset(op);
    حالت CMD_READ:
        واپس floppy_read(op);
    حالت CMD_WRITE:
        واپس floppy_write(op);
    حالت CMD_VERIFY:
        واپس floppy_verify(op);
    حالت CMD_FORMAT:
        واپس floppy_format(op);
    طےشدہ:
        واپس DISK_RET_EPARAM;
    }
}


/****************************************************************
 * HW irqs
 ****************************************************************/

// INT 0Eh Diskette Hardware ISR Entry Point
خالی VISIBLE16
handle_0e(خالی)
{
    اگر (! CONFIG_FLOPPY)
        واپس;
    debug_isr(DEBUG_ISR_0e);

    // diskette interrupt has occurred
    u8 frs = GET_BDA(floppy_recalibration_حالت);
    سیٹ_BDA(floppy_recalibration_حالت, frs | FRS_IRQ);

    pic_eoi1();
}

// Called from int08 handler.
خالی
floppy_tick(خالی)
{
    اگر (! CONFIG_FLOPPY)
        واپس;

    // time to turn off drive(s)?
    u8 fcount = GET_BDA(floppy_motor_شمار);
    اگر (fcount) {
        fcount--;
        سیٹ_BDA(floppy_motor_شمار, fcount);
        اگر (fcount == 0)
            // turn motor(s) off
            floppy_dor_نقاب(FLOPPY_DOR_MOTOR_نقاب, 0);
    }
}

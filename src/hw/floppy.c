// 16bit code to access floppy drives.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // SET_BDA
#ضمن "block.h" // struct drive_s
#ضمن "bregs.h" // struct bregs
#ضمن "config.h" // CONFIG_FLOPPY
#ضمن "malloc.h" // malloc_fseg
#ضمن "output.h" // dprintf
#ضمن "pcidevice.h" // pci_find_class
#ضمن "pci_ids.h" // PCI_CLASS_BRIDGE_ISA
#ضمن "pic.h" // pic_eoi1
#ضمن "romfile.h" // romfile_loadint
#ضمن "rtc.h" // rtc_read
#ضمن "stacks.h" // yield
#ضمن "std/disk.h" // DISK_RET_SUCCESS
#ضمن "string.h" // memset
#ضمن "util.h" // timer_calc

#وصف_ڪريو پورٽ_FD_بنياد           0x03f0
#وصف_ڪريو پورٽ_FD_DOR            0x03f2
#وصف_ڪريو پورٽ_FD_حالت         0x03f4
#وصف_ڪريو پورٽ_FD_معلومات           0x03f5
#وصف_ڪريو پورٽ_FD_DIR            0x03f7

// Digital Input Register (read)
#وصف_ڪريو FLOPPY_DIR_DSKCHG      0x80

#وصف_ڪريو FLOPPY_سائيز_ڪوڊ 0x02 // 512 byte sectors
#وصف_ڪريو FLOPPY_DATALEN 0xff   // Not used - because size code is 0x02
#وصف_ڪريو FLOPPY_MOTOR_TICKS 37 // ~2 seconds
#وصف_ڪريو FLOPPY_FILLBYTE 0xf6
#وصف_ڪريو FLOPPY_GAPLEN 0x1B
#وصف_ڪريو FLOPPY_FORMAT_GAPLEN 0x6c
#وصف_ڪريو FLOPPY_PIO_TIMEOUT 1000
#وصف_ڪريو FLOPPY_IRQ_TIMEOUT 5000
#وصف_ڪريو FLOPPY_SPECIFY1 0xAF  // step rate 12ms, head unload 240ms
#وصف_ڪريو FLOPPY_SPECIFY2 0x02  // head load time 4ms, DMA used
#وصف_ڪريو FLOPPY_STARTUP_وقت 8 // 1 second

#وصف_ڪريو FLOPPY_DOR_MOTOR_D     0x80 // Set to turn drive 3's motor ON
#وصف_ڪريو FLOPPY_DOR_MOTOR_C     0x40 // Set to turn drive 2's motor ON
#وصف_ڪريو FLOPPY_DOR_MOTOR_B     0x20 // Set to turn drive 1's motor ON
#وصف_ڪريو FLOPPY_DOR_MOTOR_A     0x10 // Set to turn drive 0's motor ON
#وصف_ڪريو FLOPPY_DOR_MOTOR_نقاب  0xf0
#وصف_ڪريو FLOPPY_DOR_IRQ         0x08 // Set to enable IRQs and DMA
#وصف_ڪريو FLOPPY_DOR_RESET       0x04 // Clear = enter reset mode, Set = normal operation
#وصف_ڪريو FLOPPY_DOR_DSEL_نقاب   0x03 // "Select" drive number for next access

// New diskette parameter table adding 3 parameters from IBM
// Since no provisions are made for multiple drive types, most
// values in this table are ignored.  I set parameters for 1.44M
// floppy here
بنية floppy_ext_dbt_s diskette_param_table2 VARFSEG = {
    .dbt = {
        .specify1       = FLOPPY_SPECIFY1,
        .specify2       = FLOPPY_SPECIFY2,
        .shutoff_ticks  = FLOPPY_MOTOR_TICKS, // ~2 seconds
        .bps_ڪوڊ       = FLOPPY_سائيز_ڪوڊ,
        .sectors        = 18,
        .interblock_len = FLOPPY_GAPLEN,
        .معلومات_len       = FLOPPY_DATALEN,
        .gap_len        = FLOPPY_FORMAT_GAPLEN,
        .fill_byte      = FLOPPY_FILLBYTE,
        .settle_وقت    = 0x0F, // 15ms
        .startup_وقت   = FLOPPY_STARTUP_وقت,
    },
    .max_track      = 79,   // maximum track
    .معلومات_شرح      = 0,    // data transfer rate
    .drive_type     = 4,    // drive type in cmos
};

بنية floppyinfo_s {
    بنية chs_s chs;
    u8 floppy_سائيز;
    u8 معلومات_شرح;
};

#وصف_ڪريو FLOPPY_سائيز_525 0x01
#وصف_ڪريو FLOPPY_سائيز_350 0x02

#وصف_ڪريو FLOPPY_شرح_500_K 0x00
#وصف_ڪريو FLOPPY_شرح_300_K 0x01
#وصف_ڪريو FLOPPY_شرح_250_K 0x02
#وصف_ڪريو FLOPPY_شرح_1_M   0x03

بنية floppyinfo_s FloppyInfo[] VARFSEG = {
    // Unknown
    { {0, 0, 0}, 0x00, 0x00},
    // 1 - 360KB, 5.25" - 2 heads, 40 tracks, 9 sectors
    { {2, 40, 9}, FLOPPY_سائيز_525, FLOPPY_شرح_300_K},
    // 2 - 1.2MB, 5.25" - 2 heads, 80 tracks, 15 sectors
    { {2, 80, 15}, FLOPPY_سائيز_525, FLOPPY_شرح_500_K},
    // 3 - 720KB, 3.5"  - 2 heads, 80 tracks, 9 sectors
    { {2, 80, 9}, FLOPPY_سائيز_350, FLOPPY_شرح_250_K},
    // 4 - 1.44MB, 3.5" - 2 heads, 80 tracks, 18 sectors
    { {2, 80, 18}, FLOPPY_سائيز_350, FLOPPY_شرح_500_K},
    // 5 - 2.88MB, 3.5" - 2 heads, 80 tracks, 36 sectors
    { {2, 80, 36}, FLOPPY_سائيز_350, FLOPPY_شرح_1_M},
    // 6 - 160k, 5.25"  - 1 heads, 40 tracks, 8 sectors
    { {1, 40, 8}, FLOPPY_سائيز_525, FLOPPY_شرح_250_K},
    // 7 - 180k, 5.25"  - 1 heads, 40 tracks, 9 sectors
    { {1, 40, 9}, FLOPPY_سائيز_525, FLOPPY_شرح_300_K},
    // 8 - 320k, 5.25"  - 2 heads, 40 tracks, 8 sectors
    { {2, 40, 8}, FLOPPY_سائيز_525, FLOPPY_شرح_250_K},
};

بنية drive_s *
init_floppy(صحيح floppyid, صحيح ftype)
{
    إذا (ftype <= 0 || ftype >= ARRAY_سائيز(FloppyInfo)) {
        dprintf(1, "Bad floppy type %d\n", ftype);
        أرجع عدم;
    }

    بنية drive_s *drive = malloc_fseg(حجم(*drive));
    إذا (!drive) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(drive, 0, حجم(*drive));
    drive->cntl_id = floppyid;
    drive->type = DTYPE_FLOPPY;
    drive->blksize = DISK_SECTOR_سائيز;
    drive->floppy_type = ftype;
    drive->sectors = (u64)-1;

    يادگيري_نقل(&drive->lchs, &FloppyInfo[ftype].chs
           , حجم(FloppyInfo[ftype].chs));
    أرجع drive;
}

ساكن فراغ
جوڙيو_Floppy(صحيح floppyid, صحيح ftype)
{
    بنية drive_s *drive = init_floppy(floppyid, ftype);
    إذا (!drive)
        أرجع;
    حرف *desc = znprintf(MAXDESCSIZE, "Floppy [drive %c]", 'A' + floppyid);
    بنية pci_device *pci = pci_ڳوليو_class(PCI_CLASS_BRIDGE_ISA); /* isa-to-pci bridge */
    صحيح prio = bootprio_ڳوليو_fdc_device(pci, پورٽ_FD_بنياد, floppyid);
    boot_جوڙيو_floppy(drive, desc, prio);
}

فراغ
floppy_تياري(فراغ)
{
    يادگيري_نقل(&diskette_param_جدول, &diskette_param_table2
           , حجم(diskette_param_جدول));
    سيٽ_IVT(0x1E, SEGOFF(SEG_BIOS
                         , (u32)&diskette_param_table2 - BUILD_BIOS_ADDR));

    إذا (! CONFIG_FLOPPY)
        أرجع;
    dprintf(3, "init floppy drives\n");

    إذا (CONFIG_QEMU) {
        u8 type = rtc_read(CMOS_FLOPPY_DRIVE_TYPE);
        إذا (type & 0xf0)
            جوڙيو_Floppy(0, type >> 4);
        إذا (type & 0x0f)
            جوڙيو_Floppy(1, type & 0x0f);
    } وإلا {
        u8 type = romfile_loadint("etc/floppy0", 0);
        إذا (type)
            جوڙيو_Floppy(0, type);
        type = romfile_loadint("etc/floppy1", 0);
        إذا (type)
            جوڙيو_Floppy(1, type);
    }

    enable_hwirq(6, FUNC16(داخلا_0e));
}

// Find a floppy type that matches a given image size.
صحيح
ڳوليو_floppy_type(u32 سائيز)
{
    صحيح i;
    لكل (i=1; i<ARRAY_سائيز(FloppyInfo); i++) {
        بنية chs_s *c = &FloppyInfo[i].chs;
        إذا (c->cylinder * c->سر * c->sector * DISK_SECTOR_سائيز == سائيز)
            أرجع i;
    }
    أرجع -1;
}


/****************************************************************
 * Low-level floppy IO
 ****************************************************************/

u8 FloppyDOR VARLOW;

ساكن مضمن u8
floppy_dor_read(فراغ)
{
    أرجع GET_هيٺيون(FloppyDOR);
}

ساكن مضمن فراغ
floppy_dor_write(u8 val)
{
    outb(val, پورٽ_FD_DOR);
    سيٽ_هيٺيون(FloppyDOR, val);
}

ساكن مضمن فراغ
floppy_dor_نقاب(u8 off, u8 on)
{
    floppy_dor_write((floppy_dor_read() & ~off) | on);
}

ساكن فراغ
floppy_disable_controller(فراغ)
{
    dprintf(2, "Floppy_disable_controller\n");
    // Clear the reset bit (enter reset state) and clear 'enable IRQ and DMA'
    floppy_dor_نقاب(FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET, 0);
}

ساكن صحيح
floppy_wait_irq(فراغ)
{
    u8 frs = GET_BDA(floppy_recalibration_حالت);
    سيٽ_BDA(floppy_recalibration_حالت, frs & ~FRS_IRQ);
    u32 پڇاڙي = timer_calc(FLOPPY_IRQ_TIMEOUT);
    لكل (;;) {
        إذا (timer_check(پڇاڙي)) {
            warn_timeout();
            floppy_disable_controller();
            أرجع DISK_RET_ETIMEOUT;
        }
        frs = GET_BDA(floppy_recalibration_حالت);
        إذا (frs & FRS_IRQ)
            اكسر;
        // Could use yield_toirq() here, but that causes issues on
        // bochs, so use yield() instead.
        yield();
    }

    سيٽ_BDA(floppy_recalibration_حالت, frs & ~FRS_IRQ);
    أرجع DISK_RET_SUCCESS;
}

// Floppy commands
#وصف_ڪريو FCF_WAITIRQ 0x10000
#وصف_ڪريو FC_CHECKIRQ    (0x08 | (0<<8) | (2<<12))
#وصف_ڪريو FC_SEEK        (0x0f | (2<<8) | (0<<12) | FCF_WAITIRQ)
#وصف_ڪريو FC_RECALIBRATE (0x07 | (1<<8) | (0<<12) | FCF_WAITIRQ)
#وصف_ڪريو FC_READID      (0x4a | (1<<8) | (7<<12) | FCF_WAITIRQ)
#وصف_ڪريو FC_READ        (0xe6 | (8<<8) | (7<<12) | FCF_WAITIRQ)
#وصف_ڪريو FC_WRITE       (0xc5 | (8<<8) | (7<<12) | FCF_WAITIRQ)
#وصف_ڪريو FC_FORMAT      (0x4d | (5<<8) | (7<<12) | FCF_WAITIRQ)
#وصف_ڪريو FC_SPECIFY     (0x03 | (2<<8) | (0<<12))

// Send the specified command and it's parameters to the floppy controller.
ساكن صحيح
floppy_pio(صحيح command, u8 *param)
{
    dprintf(9, "Floppy pio command %x\n", command);
    // Send command and parameters to controller.
    u32 پڇاڙي = timer_calc(FLOPPY_PIO_TIMEOUT);
    صحيح send = (command >> 8) & 0xf;
    صحيح i = 0;
    لكل (;;) {
        u8 sts = inb(پورٽ_FD_حالت);
        إذا (!(sts & 0x80)) {
            إذا (timer_check(پڇاڙي)) {
                warn_timeout();
                floppy_disable_controller();
                أرجع DISK_RET_ETIMEOUT;
            }
            yield();
            استمر;
        }
        إذا (sts & 0x40) {
            floppy_disable_controller();
            أرجع DISK_RET_ECONTROLLER;
        }
        إذا (i == 0)
            outb(command & 0xff, پورٽ_FD_معلومات);
        وإلا
            outb(param[i-1], پورٽ_FD_معلومات);
        إذا (i++ >= send)
            اكسر;
    }

    // Wait for command to complete.
    إذا (command & FCF_WAITIRQ) {
        صحيح ret = floppy_wait_irq();
        إذا (ret)
            أرجع ret;
    }

    // Read response from controller.
    پڇاڙي = timer_calc(FLOPPY_PIO_TIMEOUT);
    صحيح receive = (command >> 12) & 0xf;
    i = 0;
    لكل (;;) {
        u8 sts = inb(پورٽ_FD_حالت);
        إذا (!(sts & 0x80)) {
            إذا (timer_check(پڇاڙي)) {
                warn_timeout();
                floppy_disable_controller();
                أرجع DISK_RET_ETIMEOUT;
            }
            yield();
            استمر;
        }
        إذا (i >= receive) {
            إذا (sts & 0x40) {
                floppy_disable_controller();
                أرجع DISK_RET_ECONTROLLER;
            }
            اكسر;
        }
        إذا (!(sts & 0x40)) {
            floppy_disable_controller();
            أرجع DISK_RET_ECONTROLLER;
        }
        param[i++] = inb(پورٽ_FD_معلومات);
    }

    أرجع DISK_RET_SUCCESS;
}

ساكن صحيح
floppy_enable_controller(فراغ)
{
    dprintf(2, "Floppy_enable_controller\n");
    // Clear the reset bit (enter reset state), but set 'enable IRQ and DMA'
    floppy_dor_نقاب(FLOPPY_DOR_RESET, FLOPPY_DOR_IRQ);
    // Real hardware needs a 4 microsecond delay
    usleep(4);
    // Set the reset bit (normal operation) and keep 'enable IRQ and DMA' on
    floppy_dor_نقاب(0, FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET);
    صحيح ret = floppy_wait_irq();
    إذا (ret)
        أرجع ret;

    // After the interrupt is received, send 4 SENSE INTERRUPT commands to
    // clear the interrupt status for each of the four logical drives,
    // supported by the controller.
    // See section 7.4 - "Drive Polling" of the Intel 82077AA datasheet for
    // a more detailed description of why this voodoo needs to be done.
    // Without this, initialization fails on real controllers (but still works
    // in QEMU)
    u8 param[2];
    صحيح i;
    لكل (i=0; i<4; i++) {
        ret = floppy_pio(FC_CHECKIRQ, param);
        إذا (ret)
            أرجع ret;
    }
    أرجع DISK_RET_SUCCESS;
}

// Activate a drive and send a command to it.
ساكن صحيح
floppy_drive_pio(u8 floppyid, صحيح command, u8 *param)
{
    // Enable controller if it isn't running.
    إذا (!(floppy_dor_read() & FLOPPY_DOR_RESET)) {
        صحيح ret = floppy_enable_controller();
        إذا (ret)
            أرجع ret;
    }

    // set the disk motor timeout value of INT 08 to the highest value
    سيٽ_BDA(floppy_motor_counter, 255);

    // Check if the motor is already running
    u8 motor_نقاب = FLOPPY_DOR_MOTOR_A << floppyid;
    صحيح motor_already_running = floppy_dor_read() & motor_نقاب;

    // Turn on motor of selected drive, DMA & int enabled, normal operation
    floppy_dor_write(motor_نقاب | FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET | floppyid);

    // If the motor was just started, wait for it to get up to speed
    إذا (!motor_already_running && !CONFIG_QEMU)
        msleep(FLOPPY_STARTUP_وقت * 125);

    // Send command.
    صحيح ret = floppy_pio(command, param);
    سيٽ_BDA(floppy_motor_counter, FLOPPY_MOTOR_TICKS); // reset motor timeout
    إذا (ret)
        أرجع ret;

    // Check IRQ command is needed after irq commands with no results
    إذا ((command & FCF_WAITIRQ) && ((command >> 12) & 0xf) == 0)
        أرجع floppy_pio(FC_CHECKIRQ, param);
    أرجع DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy media sense and seeking
 ****************************************************************/

ساكن صحيح
floppy_drive_recal(u8 floppyid)
{
    dprintf(2, "Floppy_drive_recal %d\n", floppyid);
    // send Recalibrate command to controller
    u8 param[2];
    param[0] = floppyid;
    صحيح ret = floppy_drive_pio(floppyid, FC_RECALIBRATE, param);
    إذا (ret)
        أرجع ret;

    u8 frs = GET_BDA(floppy_recalibration_حالت);
    سيٽ_BDA(floppy_recalibration_حالت, frs | (1<<floppyid));
    سيٽ_BDA(floppy_track[floppyid], 0);
    أرجع DISK_RET_SUCCESS;
}

ساكن صحيح
floppy_drive_seek(u8 floppyid, u8 cylinder)
{
    dprintf(2, "Floppy_drive_seek %d to %d\n", floppyid, cylinder);
    u8 param[2];
    param[0] = floppyid;
    param[1] = cylinder;
    صحيح ret = floppy_drive_pio(floppyid, FC_SEEK, param);
    إذا (ret)
        أرجع ret;

    سيٽ_BDA(floppy_track[floppyid], cylinder);
    أرجع DISK_RET_SUCCESS;
}

ساكن صحيح
floppy_drive_specify(فراغ)
{
    u8 param[2];
    param[0] = FLOPPY_SPECIFY1;
    param[1] = FLOPPY_SPECIFY2;
    أرجع floppy_pio(FC_SPECIFY, param);
}

ساكن صحيح
floppy_drive_readid(u8 floppyid, u8 معلومات_شرح, u8 سر)
{
    // Set data rate.
    outb(معلومات_شرح, پورٽ_FD_DIR);

    // send Read Sector Id command
    u8 param[7];
    param[0] = (سر << 2) | floppyid; // HD DR1 DR2
    صحيح ret = floppy_drive_pio(floppyid, FC_READID, param);
    إذا (ret)
        أرجع ret;
    إذا (param[0] & 0xc0)
        أرجع -1;
    أرجع 0;
}

// Only the 1.2MB 5.25" drive and the 3.5" drives have a disk change line.  On
// the older 5.25" types the pin is not driven and reads back as a permanent
// "changed".
ساكن صحيح
floppy_has_changeline(u8 ftype)
{
    أرجع ftype >= 2 && ftype <= 5;
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
ساكن صحيح
floppy_check_media(بنية drive_s *drive_gf, u8 floppyid)
{
    إذا (!floppy_has_changeline(GET_GLOBALFLAT(drive_gf->floppy_type)))
        أرجع DISK_RET_SUCCESS;

    // The caller has just recalibrated this drive, so it is selected in the
    // DOR and the DIR reflects its change line.
    إذا (!(inb(پورٽ_FD_DIR) & FLOPPY_DIR_DSKCHG))
        أرجع DISK_RET_SUCCESS;

    صحيح ret = floppy_drive_seek(floppyid, 1);
    إذا (ret)
        أرجع ret;

    إذا (inb(پورٽ_FD_DIR) & FLOPPY_DIR_DSKCHG) {
        dprintf(2, "Floppy_check_media %d: no medium\n", floppyid);
        أرجع DISK_RET_ETIMEOUT;
    }

    // A medium is present.  Put the head back on cylinder 0, where the caller
    // left it, so that media sensing reads the track it always did.
    أرجع floppy_drive_seek(floppyid, 0);
}

ساكن صحيح
floppy_media_sense(بنية drive_s *drive_gf)
{
    u8 ftype = GET_GLOBALFLAT(drive_gf->floppy_type), stype = ftype;
    u8 floppyid = GET_GLOBALFLAT(drive_gf->cntl_id);

    u8 معلومات_شرح = GET_GLOBAL(FloppyInfo[stype].معلومات_شرح);
    صحيح ret = floppy_drive_readid(floppyid, معلومات_شرح, 0);
    إذا (ret) {
        // Attempt media sense.
        لكل (stype=1; ; stype++) {
            إذا (stype >= ARRAY_سائيز(FloppyInfo))
                أرجع DISK_RET_EBADTRACK;
            إذا (stype==ftype
                || (GET_GLOBAL(FloppyInfo[stype].floppy_سائيز)
                    != GET_GLOBAL(FloppyInfo[ftype].floppy_سائيز))
                || (GET_GLOBAL(FloppyInfo[stype].chs.سر)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.سر))
                || (GET_GLOBAL(FloppyInfo[stype].chs.cylinder)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.cylinder))
                || (GET_GLOBAL(FloppyInfo[stype].chs.sector)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.sector)))
                استمر;
            معلومات_شرح = GET_GLOBAL(FloppyInfo[stype].معلومات_شرح);
            ret = floppy_drive_readid(floppyid, معلومات_شرح, 0);
            إذا (!ret)
                اكسر;
        }
    }
    dprintf(2, "Floppy_media_sense on drive %d found rate %d\n"
            , floppyid, معلومات_شرح);

    u8 old_معلومات_شرح = GET_BDA(floppy_media_state[floppyid]) >> 6;
    سيٽ_BDA(floppy_آخري_معلومات_شرح, (old_معلومات_شرح<<2) | (معلومات_شرح<<6));
    u8 media = (stype == 1 ? 0x04 : (stype == 2 ? 0x05 : 0x07));
    u8 fms = (معلومات_شرح<<6) | FMS_MEDIA_DRIVE_ESTABLISHED | media;
    إذا (GET_GLOBAL(FloppyInfo[stype].chs.cylinder)
        < GET_GLOBAL(FloppyInfo[ftype].chs.cylinder))
        fms |= FMS_DOUBLE_STEPPING;
    سيٽ_BDA(floppy_media_state[floppyid], fms);

    أرجع DISK_RET_SUCCESS;
}

// Prepare a floppy for a data transfer.
ساكن صحيح
floppy_prep(بنية drive_s *drive_gf, u8 cylinder)
{
    u8 floppyid = GET_GLOBALFLAT(drive_gf->cntl_id);
    إذا (!(GET_BDA(floppy_recalibration_حالت) & (1<<floppyid)) ||
        !(GET_BDA(floppy_media_state[floppyid]) & FMS_MEDIA_DRIVE_ESTABLISHED)) {
        // Recalibrate drive.
        صحيح ret = floppy_drive_recal(floppyid);
        إذا (ret)
            أرجع ret;

        // An empty drive gives no index pulses, so the READ ID that
        // floppy_media_sense() issues would never complete and would wait out
        // its timeout once per data rate.  Ask the disk change line instead.
        ret = floppy_check_media(drive_gf, floppyid);
        إذا (ret)
            أرجع ret;

        // Sense media.
        ret = floppy_media_sense(drive_gf);
        إذا (ret)
            أرجع ret;

        // Execute a SPECIFY command (sets the Step Rate Time,
        // Head Load Time, Head Unload Time and the DMA enable/disable bit).
        ret = floppy_drive_specify();
        إذا (ret)
            أرجع ret;
    }

    // Seek to cylinder if needed.
    إذا (cylinder != GET_BDA(floppy_track[floppyid])) {
        صحيح ret = floppy_drive_seek(floppyid, cylinder);
        إذا (ret)
            أرجع ret;
    }

    أرجع DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy DMA transfer
 ****************************************************************/

// Perform a floppy transfer command (setup DMA and issue PIO).
ساكن صحيح
floppy_dma_cmd(بنية disk_op_s *op, صحيح ڳڻپ, صحيح command, u8 *param)
{
    // Setup DMA controller
    صحيح isWrite = command != FC_READ;
    صحيح ret = dma_floppy((u32)op->buf_fl, ڳڻپ, isWrite);
    إذا (ret)
        أرجع DISK_RET_EBOUNDARY;

    // Invoke floppy controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    ret = floppy_drive_pio(floppyid, command, param);
    إذا (ret)
        أرجع ret;

    // Populate floppy_return_status in BDA
    صحيح i;
    لكل (i=0; i<7; i++)
        سيٽ_BDA(floppy_return_حالت[i], param[i]);

    إذا (param[0] & 0xc0) {
        إذا (param[1] & 0x02)
            أرجع DISK_RET_EWRITEPROTECT;
        dprintf(1, "floppy error: %02x %02x %02x %02x %02x %02x %02x\n"
                , param[0], param[1], param[2], param[3]
                , param[4], param[5], param[6]);
        أرجع DISK_RET_ECONTROLLER;
    }

    أرجع DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy handlers
 ****************************************************************/

ساكن بنية chs_s
lba2chs(بنية disk_op_s *op)
{
    بنية chs_s res = { };

    u32 tmp = op->lba;
    u16 nls = GET_GLOBALFLAT(op->drive_fl->lchs.sector);
    res.sector = (tmp % nls) + 1;

    tmp /= nls;
    u16 nlh = GET_GLOBALFLAT(op->drive_fl->lchs.سر);
    res.سر = tmp % nlh;

    tmp /= nlh;
    res.cylinder = tmp;

    أرجع res;
}

// diskette controller reset
ساكن صحيح
floppy_reset(بنية disk_op_s *op)
{
    سيٽ_BDA(floppy_recalibration_حالت, 0);
    سيٽ_BDA(floppy_media_state[0], 0);
    سيٽ_BDA(floppy_media_state[1], 0);
    سيٽ_BDA(floppy_track[0], 0);
    سيٽ_BDA(floppy_track[1], 0);
    سيٽ_BDA(floppy_آخري_معلومات_شرح, 0);
    floppy_disable_controller();
    أرجع floppy_enable_controller();
}

// Read Diskette Sectors
ساكن صحيح
floppy_read(بنية disk_op_s *op)
{
    بنية chs_s chs = lba2chs(op);
    صحيح ret = floppy_prep(op->drive_fl, chs.cylinder);
    إذا (ret)
        أرجع ret;

    // send read-normal-data command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[8];
    param[0] = (chs.سر << 2) | floppyid; // HD DR1 DR2
    param[1] = chs.cylinder;
    param[2] = chs.سر;
    param[3] = chs.sector;
    param[4] = FLOPPY_سائيز_ڪوڊ;
    param[5] = chs.sector + op->ڳڻپ - 1; // last sector to read on track
    param[6] = FLOPPY_GAPLEN;
    param[7] = FLOPPY_DATALEN;
    أرجع floppy_dma_cmd(op, op->ڳڻپ * DISK_SECTOR_سائيز, FC_READ, param);
}

// Write Diskette Sectors
ساكن صحيح
floppy_write(بنية disk_op_s *op)
{
    بنية chs_s chs = lba2chs(op);
    صحيح ret = floppy_prep(op->drive_fl, chs.cylinder);
    إذا (ret)
        أرجع ret;

    // send write-normal-data command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[8];
    param[0] = (chs.سر << 2) | floppyid; // HD DR1 DR2
    param[1] = chs.cylinder;
    param[2] = chs.سر;
    param[3] = chs.sector;
    param[4] = FLOPPY_سائيز_ڪوڊ;
    param[5] = chs.sector + op->ڳڻپ - 1; // last sector to write on track
    param[6] = FLOPPY_GAPLEN;
    param[7] = FLOPPY_DATALEN;
    أرجع floppy_dma_cmd(op, op->ڳڻپ * DISK_SECTOR_سائيز, FC_WRITE, param);
}

// Verify Diskette Sectors
ساكن صحيح
floppy_verify(بنية disk_op_s *op)
{
    بنية chs_s chs = lba2chs(op);
    صحيح ret = floppy_prep(op->drive_fl, chs.cylinder);
    إذا (ret)
        أرجع ret;

    // This command isn't implemented - just return success.
    أرجع DISK_RET_SUCCESS;
}

// format diskette track
ساكن صحيح
floppy_format(بنية disk_op_s *op)
{
    بنية chs_s chs = lba2chs(op);
    صحيح ret = floppy_prep(op->drive_fl, chs.cylinder);
    إذا (ret)
        أرجع ret;

    // send format-track command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[7];
    param[0] = (chs.سر << 2) | floppyid; // HD DR1 DR2
    param[1] = FLOPPY_سائيز_ڪوڊ;
    param[2] = op->ڳڻپ; // number of sectors per track
    param[3] = FLOPPY_FORMAT_GAPLEN;
    param[4] = FLOPPY_FILLBYTE;
    أرجع floppy_dma_cmd(op, op->ڳڻپ * 4, FC_FORMAT, param);
}

صحيح
floppy_process_op(بنية disk_op_s *op)
{
    إذا (!CONFIG_FLOPPY)
        أرجع 0;

    بدل (op->command) {
    حالة CMD_RESET:
        أرجع floppy_reset(op);
    حالة CMD_READ:
        أرجع floppy_read(op);
    حالة CMD_WRITE:
        أرجع floppy_write(op);
    حالة CMD_VERIFY:
        أرجع floppy_verify(op);
    حالة CMD_FORMAT:
        أرجع floppy_format(op);
    افتراضي:
        أرجع DISK_RET_EPARAM;
    }
}


/****************************************************************
 * HW irqs
 ****************************************************************/

// INT 0Eh Diskette Hardware ISR Entry Point
فراغ VISIBLE16
handle_0e(فراغ)
{
    إذا (! CONFIG_FLOPPY)
        أرجع;
    debug_isr(DEBUG_ISR_0e);

    // diskette interrupt has occurred
    u8 frs = GET_BDA(floppy_recalibration_حالت);
    سيٽ_BDA(floppy_recalibration_حالت, frs | FRS_IRQ);

    pic_eoi1();
}

// Called from int08 handler.
فراغ
floppy_tick(فراغ)
{
    إذا (! CONFIG_FLOPPY)
        أرجع;

    // time to turn off drive(s)?
    u8 fcount = GET_BDA(floppy_motor_counter);
    إذا (fcount) {
        fcount--;
        سيٽ_BDA(floppy_motor_counter, fcount);
        إذا (fcount == 0)
            // turn motor(s) off
            floppy_dor_نقاب(FLOPPY_DOR_MOTOR_نقاب, 0);
    }
}

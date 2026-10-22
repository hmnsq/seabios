// 16bit code to access floppy drives.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // SET_BDA
#ଅନ୍ତର୍ଭାଵ "block.h" // struct drive_s
#ଅନ୍ତର୍ଭାଵ "bregs.h" // struct bregs
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_FLOPPY
#ଅନ୍ତର୍ଭାଵ "malloc.h" // malloc_fseg
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "pcidevice.h" // pci_find_class
#ଅନ୍ତର୍ଭାଵ "pci_ids.h" // PCI_CLASS_BRIDGE_ISA
#ଅନ୍ତର୍ଭାଵ "pic.h" // pic_eoi1
#ଅନ୍ତର୍ଭାଵ "romfile.h" // romfile_loadint
#ଅନ୍ତର୍ଭାଵ "rtc.h" // rtc_read
#ଅନ୍ତର୍ଭାଵ "stacks.h" // yield
#ଅନ୍ତର୍ଭାଵ "std/disk.h" // DISK_RET_SUCCESS
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // timer_calc

#define PORT_FD_BASE           0x03f0
#define PORT_FD_DOR            0x03f2
#define PORT_FD_STATUS         0x03f4
#define PORT_FD_DATA           0x03f5
#define PORT_FD_DIR            0x03f7

// Digital Input Register (read)
#define FLOPPY_DIR_DSKCHG      0x80

#define FLOPPY_SIZE_CODE 0x02 // 512 byte sectors
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
ସଂରଚନା floppy_ext_dbt_s diskette_param_table2 VARFSEG = {
    .dbt = {
        .specify1       = FLOPPY_SPECIFY1,
        .specify2       = FLOPPY_SPECIFY2,
        .shutoff_ticks  = FLOPPY_MOTOR_TICKS, // ~2 seconds
        .bps_code       = FLOPPY_SIZE_CODE,
        .sectors        = 18,
        .interblock_len = FLOPPY_GAPLEN,
        .data_len       = FLOPPY_DATALEN,
        .gap_len        = FLOPPY_FORMAT_GAPLEN,
        .fill_byte      = FLOPPY_FILLBYTE,
        .settle_time    = 0x0F, // 15ms
        .startup_time   = FLOPPY_STARTUP_TIME,
    },
    .max_track      = 79,   // maximum track
    .data_rate      = 0,    // data transfer rate
    .drive_type     = 4,    // drive type in cmos
};

ସଂରଚନା floppyinfo_s {
    ସଂରଚନା chs_s chs;
    u8 floppy_size;
    u8 data_rate;
};

#define FLOPPY_SIZE_525 0x01
#define FLOPPY_SIZE_350 0x02

#define FLOPPY_RATE_500K 0x00
#define FLOPPY_RATE_300K 0x01
#define FLOPPY_RATE_250K 0x02
#define FLOPPY_RATE_1M   0x03

ସଂରଚନା floppyinfo_s FloppyInfo[] VARFSEG = {
    // Unknown
    { {0, 0, 0}, 0x00, 0x00},
    // 1 - 360KB, 5.25" - 2 heads, 40 tracks, 9 sectors
    { {2, 40, 9}, FLOPPY_SIZE_525, FLOPPY_RATE_300K},
    // 2 - 1.2MB, 5.25" - 2 heads, 80 tracks, 15 sectors
    { {2, 80, 15}, FLOPPY_SIZE_525, FLOPPY_RATE_500K},
    // 3 - 720KB, 3.5"  - 2 heads, 80 tracks, 9 sectors
    { {2, 80, 9}, FLOPPY_SIZE_350, FLOPPY_RATE_250K},
    // 4 - 1.44MB, 3.5" - 2 heads, 80 tracks, 18 sectors
    { {2, 80, 18}, FLOPPY_SIZE_350, FLOPPY_RATE_500K},
    // 5 - 2.88MB, 3.5" - 2 heads, 80 tracks, 36 sectors
    { {2, 80, 36}, FLOPPY_SIZE_350, FLOPPY_RATE_1M},
    // 6 - 160k, 5.25"  - 1 heads, 40 tracks, 8 sectors
    { {1, 40, 8}, FLOPPY_SIZE_525, FLOPPY_RATE_250K},
    // 7 - 180k, 5.25"  - 1 heads, 40 tracks, 9 sectors
    { {1, 40, 9}, FLOPPY_SIZE_525, FLOPPY_RATE_300K},
    // 8 - 320k, 5.25"  - 2 heads, 40 tracks, 8 sectors
    { {2, 40, 8}, FLOPPY_SIZE_525, FLOPPY_RATE_250K},
};

ସଂରଚନା drive_s *
init_floppy(ପୂର୍ଣ୍ଣସଂଖ୍ୟା floppyid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ftype)
{
    ଯଦି (ftype <= 0 || ftype >= ARRAY_SIZE(FloppyInfo)) {
        dprintf(1, "Bad floppy type %d\n", ftype);
        ଫେରନ୍ତୁ ଖାଲି;
    }

    ସଂରଚନା drive_s *drive = malloc_fseg(ଆକାର(*drive));
    ଯଦି (!drive) {
        warn_noalloc();
        ଫେରନ୍ତୁ ଖାଲି;
    }
    memset(drive, 0, ଆକାର(*drive));
    drive->cntl_id = floppyid;
    drive->type = DTYPE_FLOPPY;
    drive->blksize = DISK_SECTOR_SIZE;
    drive->floppy_type = ftype;
    drive->sectors = (u64)-1;

    memcpy(&drive->lchs, &FloppyInfo[ftype].chs
           , ଆକାର(FloppyInfo[ftype].chs));
    ଫେରନ୍ତୁ drive;
}

ସ୍ଥିର ଶୂନ୍ୟ
addFloppy(ପୂର୍ଣ୍ଣସଂଖ୍ୟା floppyid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ftype)
{
    ସଂରଚନା drive_s *drive = init_floppy(floppyid, ftype);
    ଯଦି (!drive)
        ଫେରନ୍ତୁ;
    ଅକ୍ଷର *desc = znprintf(MAXDESCSIZE, "Floppy [drive %c]", 'A' + floppyid);
    ସଂରଚନା pci_device *pci = pci_find_class(PCI_CLASS_BRIDGE_ISA); /* isa-to-pci bridge */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio = bootprio_find_fdc_device(pci, PORT_FD_BASE, floppyid);
    boot_add_floppy(drive, desc, prio);
}

ଶୂନ୍ୟ
floppy_setup(ଶୂନ୍ୟ)
{
    memcpy(&diskette_param_table, &diskette_param_table2
           , ଆକାର(diskette_param_table));
    SET_IVT(0x1E, SEGOFF(SEG_BIOS
                         , (u32)&diskette_param_table2 - BUILD_BIOS_ADDR));

    ଯଦି (! CONFIG_FLOPPY)
        ଫେରନ୍ତୁ;
    dprintf(3, "init floppy drives\n");

    ଯଦି (CONFIG_QEMU) {
        u8 type = rtc_read(CMOS_FLOPPY_DRIVE_TYPE);
        ଯଦି (type & 0xf0)
            addFloppy(0, type >> 4);
        ଯଦି (type & 0x0f)
            addFloppy(1, type & 0x0f);
    } ଅନ୍ୟଥା {
        u8 type = romfile_loadint("etc/floppy0", 0);
        ଯଦି (type)
            addFloppy(0, type);
        type = romfile_loadint("etc/floppy1", 0);
        ଯଦି (type)
            addFloppy(1, type);
    }

    enable_hwirq(6, FUNC16(entry_0e));
}

// Find a floppy type that matches a given image size.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
find_floppy_type(u32 size)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=1; i<ARRAY_SIZE(FloppyInfo); i++) {
        ସଂରଚନା chs_s *c = &FloppyInfo[i].chs;
        ଯଦି (c->cylinder * c->head * c->sector * DISK_SECTOR_SIZE == size)
            ଫେରନ୍ତୁ i;
    }
    ଫେରନ୍ତୁ -1;
}


/****************************************************************
 * Low-level floppy IO
 ****************************************************************/

u8 FloppyDOR VARLOW;

ସ୍ଥିର ଧାଡ଼ିଭିତରେ u8
floppy_dor_read(ଶୂନ୍ୟ)
{
    ଫେରନ୍ତୁ GET_ତଳ(FloppyDOR);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
floppy_dor_write(u8 val)
{
    outb(val, PORT_FD_DOR);
    SET_ତଳ(FloppyDOR, val);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
floppy_dor_mask(u8 off, u8 on)
{
    floppy_dor_write((floppy_dor_read() & ~off) | on);
}

ସ୍ଥିର ଶୂନ୍ୟ
floppy_disable_controller(ଶୂନ୍ୟ)
{
    dprintf(2, "Floppy_disable_controller\n");
    // Clear the reset bit (enter reset state) and clear 'enable IRQ and DMA'
    floppy_dor_mask(FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET, 0);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_wait_irq(ଶୂନ୍ୟ)
{
    u8 frs = GET_BDA(floppy_recalibration_status);
    SET_BDA(floppy_recalibration_status, frs & ~FRS_IRQ);
    u32 end = timer_calc(FLOPPY_IRQ_TIMEOUT);
    ପାଇଁ (;;) {
        ଯଦି (timer_check(end)) {
            warn_timeout();
            floppy_disable_controller();
            ଫେରନ୍ତୁ DISK_RET_ETIMEOUT;
        }
        frs = GET_BDA(floppy_recalibration_status);
        ଯଦି (frs & FRS_IRQ)
            ଭାଙ୍ଗ;
        // Could use yield_toirq() here, but that causes issues on
        // bochs, so use yield() instead.
        yield();
    }

    SET_BDA(floppy_recalibration_status, frs & ~FRS_IRQ);
    ଫେରନ୍ତୁ DISK_RET_SUCCESS;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_pio(ପୂର୍ଣ୍ଣସଂଖ୍ୟା command, u8 *param)
{
    dprintf(9, "Floppy pio command %x\n", command);
    // Send command and parameters to controller.
    u32 end = timer_calc(FLOPPY_PIO_TIMEOUT);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା send = (command >> 8) & 0xf;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
    ପାଇଁ (;;) {
        u8 sts = inb(PORT_FD_STATUS);
        ଯଦି (!(sts & 0x80)) {
            ଯଦି (timer_check(end)) {
                warn_timeout();
                floppy_disable_controller();
                ଫେରନ୍ତୁ DISK_RET_ETIMEOUT;
            }
            yield();
            ଜାରି;
        }
        ଯଦି (sts & 0x40) {
            floppy_disable_controller();
            ଫେରନ୍ତୁ DISK_RET_ECONTROLLER;
        }
        ଯଦି (i == 0)
            outb(command & 0xff, PORT_FD_DATA);
        ଅନ୍ୟଥା
            outb(param[i-1], PORT_FD_DATA);
        ଯଦି (i++ >= send)
            ଭାଙ୍ଗ;
    }

    // Wait for command to complete.
    ଯଦି (command & FCF_WAITIRQ) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = floppy_wait_irq();
        ଯଦି (ret)
            ଫେରନ୍ତୁ ret;
    }

    // Read response from controller.
    end = timer_calc(FLOPPY_PIO_TIMEOUT);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା receive = (command >> 12) & 0xf;
    i = 0;
    ପାଇଁ (;;) {
        u8 sts = inb(PORT_FD_STATUS);
        ଯଦି (!(sts & 0x80)) {
            ଯଦି (timer_check(end)) {
                warn_timeout();
                floppy_disable_controller();
                ଫେରନ୍ତୁ DISK_RET_ETIMEOUT;
            }
            yield();
            ଜାରି;
        }
        ଯଦି (i >= receive) {
            ଯଦି (sts & 0x40) {
                floppy_disable_controller();
                ଫେରନ୍ତୁ DISK_RET_ECONTROLLER;
            }
            ଭାଙ୍ଗ;
        }
        ଯଦି (!(sts & 0x40)) {
            floppy_disable_controller();
            ଫେରନ୍ତୁ DISK_RET_ECONTROLLER;
        }
        param[i++] = inb(PORT_FD_DATA);
    }

    ଫେରନ୍ତୁ DISK_RET_SUCCESS;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_enable_controller(ଶୂନ୍ୟ)
{
    dprintf(2, "Floppy_enable_controller\n");
    // Clear the reset bit (enter reset state), but set 'enable IRQ and DMA'
    floppy_dor_mask(FLOPPY_DOR_RESET, FLOPPY_DOR_IRQ);
    // Real hardware needs a 4 microsecond delay
    usleep(4);
    // Set the reset bit (normal operation) and keep 'enable IRQ and DMA' on
    floppy_dor_mask(0, FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = floppy_wait_irq();
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    // After the interrupt is received, send 4 SENSE INTERRUPT commands to
    // clear the interrupt status for each of the four logical drives,
    // supported by the controller.
    // See section 7.4 - "Drive Polling" of the Intel 82077AA datasheet for
    // a more detailed description of why this voodoo needs to be done.
    // Without this, initialization fails on real controllers (but still works
    // in QEMU)
    u8 param[2];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<4; i++) {
        ret = floppy_pio(FC_CHECKIRQ, param);
        ଯଦି (ret)
            ଫେରନ୍ତୁ ret;
    }
    ଫେରନ୍ତୁ DISK_RET_SUCCESS;
}

// Activate a drive and send a command to it.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_drive_pio(u8 floppyid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା command, u8 *param)
{
    // Enable controller if it isn't running.
    ଯଦି (!(floppy_dor_read() & FLOPPY_DOR_RESET)) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = floppy_enable_controller();
        ଯଦି (ret)
            ଫେରନ୍ତୁ ret;
    }

    // set the disk motor timeout value of INT 08 to the highest value
    SET_BDA(floppy_motor_counter, 255);

    // Check if the motor is already running
    u8 motor_mask = FLOPPY_DOR_MOTOR_A << floppyid;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା motor_already_running = floppy_dor_read() & motor_mask;

    // Turn on motor of selected drive, DMA & int enabled, normal operation
    floppy_dor_write(motor_mask | FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET | floppyid);

    // If the motor was just started, wait for it to get up to speed
    ଯଦି (!motor_already_running && !CONFIG_QEMU)
        msleep(FLOPPY_STARTUP_TIME * 125);

    // Send command.
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = floppy_pio(command, param);
    SET_BDA(floppy_motor_counter, FLOPPY_MOTOR_TICKS); // reset motor timeout
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    // Check IRQ command is needed after irq commands with no results
    ଯଦି ((command & FCF_WAITIRQ) && ((command >> 12) & 0xf) == 0)
        ଫେରନ୍ତୁ floppy_pio(FC_CHECKIRQ, param);
    ଫେରନ୍ତୁ DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy media sense and seeking
 ****************************************************************/

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_drive_recal(u8 floppyid)
{
    dprintf(2, "Floppy_drive_recal %d\n", floppyid);
    // send Recalibrate command to controller
    u8 param[2];
    param[0] = floppyid;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = floppy_drive_pio(floppyid, FC_RECALIBRATE, param);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    u8 frs = GET_BDA(floppy_recalibration_status);
    SET_BDA(floppy_recalibration_status, frs | (1<<floppyid));
    SET_BDA(floppy_track[floppyid], 0);
    ଫେରନ୍ତୁ DISK_RET_SUCCESS;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_drive_seek(u8 floppyid, u8 cylinder)
{
    dprintf(2, "Floppy_drive_seek %d to %d\n", floppyid, cylinder);
    u8 param[2];
    param[0] = floppyid;
    param[1] = cylinder;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = floppy_drive_pio(floppyid, FC_SEEK, param);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    SET_BDA(floppy_track[floppyid], cylinder);
    ଫେରନ୍ତୁ DISK_RET_SUCCESS;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_drive_specify(ଶୂନ୍ୟ)
{
    u8 param[2];
    param[0] = FLOPPY_SPECIFY1;
    param[1] = FLOPPY_SPECIFY2;
    ଫେରନ୍ତୁ floppy_pio(FC_SPECIFY, param);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_drive_readid(u8 floppyid, u8 data_rate, u8 head)
{
    // Set data rate.
    outb(data_rate, PORT_FD_DIR);

    // send Read Sector Id command
    u8 param[7];
    param[0] = (head << 2) | floppyid; // HD DR1 DR2
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = floppy_drive_pio(floppyid, FC_READID, param);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;
    ଯଦି (param[0] & 0xc0)
        ଫେରନ୍ତୁ -1;
    ଫେରନ୍ତୁ 0;
}

// Only the 1.2MB 5.25" drive and the 3.5" drives have a disk change line.  On
// the older 5.25" types the pin is not driven and reads back as a permanent
// "changed".
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_has_changeline(u8 ftype)
{
    ଫେରନ୍ତୁ ftype >= 2 && ftype <= 5;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_check_media(ସଂରଚନା drive_s *drive_gf, u8 floppyid)
{
    ଯଦି (!floppy_has_changeline(GET_GLOBALFLAT(drive_gf->floppy_type)))
        ଫେରନ୍ତୁ DISK_RET_SUCCESS;

    // The caller has just recalibrated this drive, so it is selected in the
    // DOR and the DIR reflects its change line.
    ଯଦି (!(inb(PORT_FD_DIR) & FLOPPY_DIR_DSKCHG))
        ଫେରନ୍ତୁ DISK_RET_SUCCESS;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = floppy_drive_seek(floppyid, 1);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    ଯଦି (inb(PORT_FD_DIR) & FLOPPY_DIR_DSKCHG) {
        dprintf(2, "Floppy_check_media %d: no medium\n", floppyid);
        ଫେରନ୍ତୁ DISK_RET_ETIMEOUT;
    }

    // A medium is present.  Put the head back on cylinder 0, where the caller
    // left it, so that media sensing reads the track it always did.
    ଫେରନ୍ତୁ floppy_drive_seek(floppyid, 0);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_media_sense(ସଂରଚନା drive_s *drive_gf)
{
    u8 ftype = GET_GLOBALFLAT(drive_gf->floppy_type), stype = ftype;
    u8 floppyid = GET_GLOBALFLAT(drive_gf->cntl_id);

    u8 data_rate = GET_GLOBAL(FloppyInfo[stype].data_rate);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = floppy_drive_readid(floppyid, data_rate, 0);
    ଯଦି (ret) {
        // Attempt media sense.
        ପାଇଁ (stype=1; ; stype++) {
            ଯଦି (stype >= ARRAY_SIZE(FloppyInfo))
                ଫେରନ୍ତୁ DISK_RET_EBADTRACK;
            ଯଦି (stype==ftype
                || (GET_GLOBAL(FloppyInfo[stype].floppy_size)
                    != GET_GLOBAL(FloppyInfo[ftype].floppy_size))
                || (GET_GLOBAL(FloppyInfo[stype].chs.head)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.head))
                || (GET_GLOBAL(FloppyInfo[stype].chs.cylinder)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.cylinder))
                || (GET_GLOBAL(FloppyInfo[stype].chs.sector)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.sector)))
                ଜାରି;
            data_rate = GET_GLOBAL(FloppyInfo[stype].data_rate);
            ret = floppy_drive_readid(floppyid, data_rate, 0);
            ଯଦି (!ret)
                ଭାଙ୍ଗ;
        }
    }
    dprintf(2, "Floppy_media_sense on drive %d found rate %d\n"
            , floppyid, data_rate);

    u8 old_data_rate = GET_BDA(floppy_media_state[floppyid]) >> 6;
    SET_BDA(floppy_last_data_rate, (old_data_rate<<2) | (data_rate<<6));
    u8 media = (stype == 1 ? 0x04 : (stype == 2 ? 0x05 : 0x07));
    u8 fms = (data_rate<<6) | FMS_MEDIA_DRIVE_ESTABLISHED | media;
    ଯଦି (GET_GLOBAL(FloppyInfo[stype].chs.cylinder)
        < GET_GLOBAL(FloppyInfo[ftype].chs.cylinder))
        fms |= FMS_DOUBLE_STEPPING;
    SET_BDA(floppy_media_state[floppyid], fms);

    ଫେରନ୍ତୁ DISK_RET_SUCCESS;
}

// Prepare a floppy for a data transfer.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_prep(ସଂରଚନା drive_s *drive_gf, u8 cylinder)
{
    u8 floppyid = GET_GLOBALFLAT(drive_gf->cntl_id);
    ଯଦି (!(GET_BDA(floppy_recalibration_status) & (1<<floppyid)) ||
        !(GET_BDA(floppy_media_state[floppyid]) & FMS_MEDIA_DRIVE_ESTABLISHED)) {
        // Recalibrate drive.
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = floppy_drive_recal(floppyid);
        ଯଦି (ret)
            ଫେରନ୍ତୁ ret;

        // An empty drive gives no index pulses, so the READ ID that
        // floppy_media_sense() issues would never complete and would wait out
        // its timeout once per data rate.  Ask the disk change line instead.
        ret = floppy_check_media(drive_gf, floppyid);
        ଯଦି (ret)
            ଫେରନ୍ତୁ ret;

        // Sense media.
        ret = floppy_media_sense(drive_gf);
        ଯଦି (ret)
            ଫେରନ୍ତୁ ret;

        // Execute a SPECIFY command (sets the Step Rate Time,
        // Head Load Time, Head Unload Time and the DMA enable/disable bit).
        ret = floppy_drive_specify();
        ଯଦି (ret)
            ଫେରନ୍ତୁ ret;
    }

    // Seek to cylinder if needed.
    ଯଦି (cylinder != GET_BDA(floppy_track[floppyid])) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = floppy_drive_seek(floppyid, cylinder);
        ଯଦି (ret)
            ଫେରନ୍ତୁ ret;
    }

    ଫେରନ୍ତୁ DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy DMA transfer
 ****************************************************************/

// Perform a floppy transfer command (setup DMA and issue PIO).
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_dma_cmd(ସଂରଚନା disk_op_s *op, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2, ପୂର୍ଣ୍ଣସଂଖ୍ୟା command, u8 *param)
{
    // Setup DMA controller
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା isWrite = command != FC_READ;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = dma_floppy((u32)op->buf_fl, ଗଣନା_2, isWrite);
    ଯଦି (ret)
        ଫେରନ୍ତୁ DISK_RET_EBOUNDARY;

    // Invoke floppy controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    ret = floppy_drive_pio(floppyid, command, param);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    // Populate floppy_return_status in BDA
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<7; i++)
        SET_BDA(floppy_return_status[i], param[i]);

    ଯଦି (param[0] & 0xc0) {
        ଯଦି (param[1] & 0x02)
            ଫେରନ୍ତୁ DISK_RET_EWRITEPROTECT;
        dprintf(1, "floppy error: %02x %02x %02x %02x %02x %02x %02x\n"
                , param[0], param[1], param[2], param[3]
                , param[4], param[5], param[6]);
        ଫେରନ୍ତୁ DISK_RET_ECONTROLLER;
    }

    ଫେରନ୍ତୁ DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy handlers
 ****************************************************************/

ସ୍ଥିର ସଂରଚନା chs_s
lba2chs(ସଂରଚନା disk_op_s *op)
{
    ସଂରଚନା chs_s res = { };

    u32 tmp = op->lba;
    u16 nls = GET_GLOBALFLAT(op->drive_fl->lchs.sector);
    res.sector = (tmp % nls) + 1;

    tmp /= nls;
    u16 nlh = GET_GLOBALFLAT(op->drive_fl->lchs.head);
    res.head = tmp % nlh;

    tmp /= nlh;
    res.cylinder = tmp;

    ଫେରନ୍ତୁ res;
}

// diskette controller reset
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_reset(ସଂରଚନା disk_op_s *op)
{
    SET_BDA(floppy_recalibration_status, 0);
    SET_BDA(floppy_media_state[0], 0);
    SET_BDA(floppy_media_state[1], 0);
    SET_BDA(floppy_track[0], 0);
    SET_BDA(floppy_track[1], 0);
    SET_BDA(floppy_last_data_rate, 0);
    floppy_disable_controller();
    ଫେରନ୍ତୁ floppy_enable_controller();
}

// Read Diskette Sectors
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_read(ସଂରଚନା disk_op_s *op)
{
    ସଂରଚନା chs_s chs = lba2chs(op);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = floppy_prep(op->drive_fl, chs.cylinder);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    // send read-normal-data command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[8];
    param[0] = (chs.head << 2) | floppyid; // HD DR1 DR2
    param[1] = chs.cylinder;
    param[2] = chs.head;
    param[3] = chs.sector;
    param[4] = FLOPPY_SIZE_CODE;
    param[5] = chs.sector + op->ଗଣନା_2 - 1; // last sector to read on track
    param[6] = FLOPPY_GAPLEN;
    param[7] = FLOPPY_DATALEN;
    ଫେରନ୍ତୁ floppy_dma_cmd(op, op->ଗଣନା_2 * DISK_SECTOR_SIZE, FC_READ, param);
}

// Write Diskette Sectors
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_write(ସଂରଚନା disk_op_s *op)
{
    ସଂରଚନା chs_s chs = lba2chs(op);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = floppy_prep(op->drive_fl, chs.cylinder);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    // send write-normal-data command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[8];
    param[0] = (chs.head << 2) | floppyid; // HD DR1 DR2
    param[1] = chs.cylinder;
    param[2] = chs.head;
    param[3] = chs.sector;
    param[4] = FLOPPY_SIZE_CODE;
    param[5] = chs.sector + op->ଗଣନା_2 - 1; // last sector to write on track
    param[6] = FLOPPY_GAPLEN;
    param[7] = FLOPPY_DATALEN;
    ଫେରନ୍ତୁ floppy_dma_cmd(op, op->ଗଣନା_2 * DISK_SECTOR_SIZE, FC_WRITE, param);
}

// Verify Diskette Sectors
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_verify(ସଂରଚନା disk_op_s *op)
{
    ସଂରଚନା chs_s chs = lba2chs(op);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = floppy_prep(op->drive_fl, chs.cylinder);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    // This command isn't implemented - just return success.
    ଫେରନ୍ତୁ DISK_RET_SUCCESS;
}

// format diskette track
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_format(ସଂରଚନା disk_op_s *op)
{
    ସଂରଚନା chs_s chs = lba2chs(op);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = floppy_prep(op->drive_fl, chs.cylinder);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;

    // send format-track command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[7];
    param[0] = (chs.head << 2) | floppyid; // HD DR1 DR2
    param[1] = FLOPPY_SIZE_CODE;
    param[2] = op->ଗଣନା_2; // number of sectors per track
    param[3] = FLOPPY_FORMAT_GAPLEN;
    param[4] = FLOPPY_FILLBYTE;
    ଫେରନ୍ତୁ floppy_dma_cmd(op, op->ଗଣନା_2 * 4, FC_FORMAT, param);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
floppy_process_op(ସଂରଚନା disk_op_s *op)
{
    ଯଦି (!CONFIG_FLOPPY)
        ଫେରନ୍ତୁ 0;

    ବାଛ (op->command) {
    ସ୍ଥିତି CMD_RESET:
        ଫେରନ୍ତୁ floppy_reset(op);
    ସ୍ଥିତି CMD_READ:
        ଫେରନ୍ତୁ floppy_read(op);
    ସ୍ଥିତି CMD_WRITE:
        ଫେରନ୍ତୁ floppy_write(op);
    ସ୍ଥିତି CMD_VERIFY:
        ଫେରନ୍ତୁ floppy_verify(op);
    ସ୍ଥିତି CMD_FORMAT:
        ଫେରନ୍ତୁ floppy_format(op);
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ଫେରନ୍ତୁ DISK_RET_EPARAM;
    }
}


/****************************************************************
 * HW irqs
 ****************************************************************/

// INT 0Eh Diskette Hardware ISR Entry Point
ଶୂନ୍ୟ VISIBLE16
handle_0e(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_FLOPPY)
        ଫେରନ୍ତୁ;
    debug_isr(DEBUG_ISR_0e);

    // diskette interrupt has occurred
    u8 frs = GET_BDA(floppy_recalibration_status);
    SET_BDA(floppy_recalibration_status, frs | FRS_IRQ);

    pic_eoi1();
}

// Called from int08 handler.
ଶୂନ୍ୟ
floppy_tick(ଶୂନ୍ୟ)
{
    ଯଦି (! CONFIG_FLOPPY)
        ଫେରନ୍ତୁ;

    // time to turn off drive(s)?
    u8 fcount = GET_BDA(floppy_motor_counter);
    ଯଦି (fcount) {
        fcount--;
        SET_BDA(floppy_motor_counter, fcount);
        ଯଦି (fcount == 0)
            // turn motor(s) off
            floppy_dor_mask(FLOPPY_DOR_MOTOR_MASK, 0);
    }
}

// 16bit code to access floppy drives.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // SET_BDA
#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // struct drive_s
#ਅਨ੍ਤਰ੍ਭਾਵ "bregs.h" // struct bregs
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_FLOPPY
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // malloc_fseg
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pcidevice.h" // pci_find_class
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_ids.h" // PCI_CLASS_BRIDGE_ISA
#ਅਨ੍ਤਰ੍ਭਾਵ "pic.h" // pic_eoi1
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_loadint
#ਅਨ੍ਤਰ੍ਭਾਵ "rtc.h" // rtc_read
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // yield
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // DISK_RET_SUCCESS
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // timer_calc

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_FD_ਅਧਾਰ           0x03f0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_FD_DOR            0x03f2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_FD_ਸਥਿਤੀ         0x03f4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_FD_ਅੰਕੜੇ           0x03f5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੋਰਟ_FD_DIR            0x03f7

// Digital Input Register (read)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_DIR_DSKCHG      0x80

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_ਮਾਪ_ਕੋਡ 0x02 // 512 byte sectors
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_DATALEN 0xff   // Not used - because size code is 0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_MOTOR_TICKS 37 // ~2 seconds
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_FILLBYTE 0xf6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_GAPLEN 0x1B
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_FORMAT_GAPLEN 0x6c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_PIO_TIMEOUT 1000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_IRQ_TIMEOUT 5000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_SPECIFY1 0xAF  // step rate 12ms, head unload 240ms
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_SPECIFY2 0x02  // head load time 4ms, DMA used
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_STARTUP_ਸਮਾਂ 8 // 1 second

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_DOR_MOTOR_D     0x80 // Set to turn drive 3's motor ON
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_DOR_MOTOR_C     0x40 // Set to turn drive 2's motor ON
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_DOR_MOTOR_B     0x20 // Set to turn drive 1's motor ON
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_DOR_MOTOR_A     0x10 // Set to turn drive 0's motor ON
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_DOR_MOTOR_ਨਕਾਬ  0xf0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_DOR_IRQ         0x08 // Set to enable IRQs and DMA
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_DOR_RESET       0x04 // Clear = enter reset mode, Set = normal operation
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_DOR_DSEL_ਨਕਾਬ   0x03 // "Select" drive number for next access

// New diskette parameter table adding 3 parameters from IBM
// Since no provisions are made for multiple drive types, most
// values in this table are ignored.  I set parameters for 1.44M
// floppy here
ਬਣਤਰ floppy_ext_dbt_s diskette_param_table2 VARFSEG = {
    .dbt = {
        .specify1       = FLOPPY_SPECIFY1,
        .specify2       = FLOPPY_SPECIFY2,
        .shutoff_ticks  = FLOPPY_MOTOR_TICKS, // ~2 seconds
        .bps_ਕੋਡ       = FLOPPY_ਮਾਪ_ਕੋਡ,
        .sectors        = 18,
        .interblock_len = FLOPPY_GAPLEN,
        .ਅੰਕੜੇ_len       = FLOPPY_DATALEN,
        .gap_len        = FLOPPY_FORMAT_GAPLEN,
        .fill_byte      = FLOPPY_FILLBYTE,
        .settle_ਸਮਾਂ    = 0x0F, // 15ms
        .startup_ਸਮਾਂ   = FLOPPY_STARTUP_ਸਮਾਂ,
    },
    .max_track      = 79,   // maximum track
    .ਅੰਕੜੇ_ਦਰ      = 0,    // data transfer rate
    .drive_type     = 4,    // drive type in cmos
};

ਬਣਤਰ floppyinfo_s {
    ਬਣਤਰ chs_s chs;
    u8 floppy_ਮਾਪ;
    u8 ਅੰਕੜੇ_ਦਰ;
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_ਮਾਪ_525 0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_ਮਾਪ_350 0x02

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_ਦਰ_500_K 0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_ਦਰ_300_K 0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_ਦਰ_250_K 0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLOPPY_ਦਰ_1_M   0x03

ਬਣਤਰ floppyinfo_s FloppyInfo[] VARFSEG = {
    // Unknown
    { {0, 0, 0}, 0x00, 0x00},
    // 1 - 360KB, 5.25" - 2 heads, 40 tracks, 9 sectors
    { {2, 40, 9}, FLOPPY_ਮਾਪ_525, FLOPPY_ਦਰ_300_K},
    // 2 - 1.2MB, 5.25" - 2 heads, 80 tracks, 15 sectors
    { {2, 80, 15}, FLOPPY_ਮਾਪ_525, FLOPPY_ਦਰ_500_K},
    // 3 - 720KB, 3.5"  - 2 heads, 80 tracks, 9 sectors
    { {2, 80, 9}, FLOPPY_ਮਾਪ_350, FLOPPY_ਦਰ_250_K},
    // 4 - 1.44MB, 3.5" - 2 heads, 80 tracks, 18 sectors
    { {2, 80, 18}, FLOPPY_ਮਾਪ_350, FLOPPY_ਦਰ_500_K},
    // 5 - 2.88MB, 3.5" - 2 heads, 80 tracks, 36 sectors
    { {2, 80, 36}, FLOPPY_ਮਾਪ_350, FLOPPY_ਦਰ_1_M},
    // 6 - 160k, 5.25"  - 1 heads, 40 tracks, 8 sectors
    { {1, 40, 8}, FLOPPY_ਮਾਪ_525, FLOPPY_ਦਰ_250_K},
    // 7 - 180k, 5.25"  - 1 heads, 40 tracks, 9 sectors
    { {1, 40, 9}, FLOPPY_ਮਾਪ_525, FLOPPY_ਦਰ_300_K},
    // 8 - 320k, 5.25"  - 2 heads, 40 tracks, 8 sectors
    { {2, 40, 8}, FLOPPY_ਮਾਪ_525, FLOPPY_ਦਰ_250_K},
};

ਬਣਤਰ drive_s *
init_floppy(ਪੂਰਨ floppyid, ਪੂਰਨ ftype)
{
    ਜੇ (ftype <= 0 || ftype >= ARRAY_ਮਾਪ(FloppyInfo)) {
        dprintf(1, "Bad floppy type %d\n", ftype);
        ਵਾਪਸ ਨਿਰਾ;
    }

    ਬਣਤਰ drive_s *drive = malloc_fseg(ਆਕਾਰ(*drive));
    ਜੇ (!drive) {
        warn_noalloc();
        ਵਾਪਸ ਨਿਰਾ;
    }
    memset(drive, 0, ਆਕਾਰ(*drive));
    drive->cntl_id = floppyid;
    drive->type = DTYPE_FLOPPY;
    drive->blksize = DISK_SECTOR_ਮਾਪ;
    drive->floppy_type = ftype;
    drive->sectors = (u64)-1;

    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&drive->lchs, &FloppyInfo[ftype].chs
           , ਆਕਾਰ(FloppyInfo[ftype].chs));
    ਵਾਪਸ drive;
}

ਸਥਿਰ ਖਾਲੀ
ਜੋੜੋ_Floppy(ਪੂਰਨ floppyid, ਪੂਰਨ ftype)
{
    ਬਣਤਰ drive_s *drive = init_floppy(floppyid, ftype);
    ਜੇ (!drive)
        ਵਾਪਸ;
    ਅੱਖਰ *desc = znprintf(MAXDESCSIZE, "Floppy [drive %c]", 'A' + floppyid);
    ਬਣਤਰ pci_device *pci = pci_ਲੱਭੋ_class(PCI_CLASS_BRIDGE_ISA); /* isa-to-pci bridge */
    ਪੂਰਨ prio = bootprio_ਲੱਭੋ_fdc_device(pci, ਪੋਰਟ_FD_ਅਧਾਰ, floppyid);
    boot_ਜੋੜੋ_floppy(drive, desc, prio);
}

ਖਾਲੀ
floppy_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&diskette_param_ਸਾਰਣੀ, &diskette_param_table2
           , ਆਕਾਰ(diskette_param_ਸਾਰਣੀ));
    ਸੈੱਟ_IVT(0x1E, SEGOFF(SEG_BIOS
                         , (u32)&diskette_param_table2 - BUILD_BIOS_ADDR));

    ਜੇ (! CONFIG_FLOPPY)
        ਵਾਪਸ;
    dprintf(3, "init floppy drives\n");

    ਜੇ (CONFIG_QEMU) {
        u8 type = rtc_read(CMOS_FLOPPY_DRIVE_TYPE);
        ਜੇ (type & 0xf0)
            ਜੋੜੋ_Floppy(0, type >> 4);
        ਜੇ (type & 0x0f)
            ਜੋੜੋ_Floppy(1, type & 0x0f);
    } ਨਹੀਂਤਾਂ {
        u8 type = romfile_loadint("etc/floppy0", 0);
        ਜੇ (type)
            ਜੋੜੋ_Floppy(0, type);
        type = romfile_loadint("etc/floppy1", 0);
        ਜੇ (type)
            ਜੋੜੋ_Floppy(1, type);
    }

    enable_hwirq(6, FUNC16(ਇੰਦਰਾਜ_0e));
}

// Find a floppy type that matches a given image size.
ਪੂਰਨ
ਲੱਭੋ_floppy_type(u32 ਮਾਪ)
{
    ਪੂਰਨ i;
    ਲਈ (i=1; i<ARRAY_ਮਾਪ(FloppyInfo); i++) {
        ਬਣਤਰ chs_s *c = &FloppyInfo[i].chs;
        ਜੇ (c->cylinder * c->ਸਿਰ * c->sector * DISK_SECTOR_ਮਾਪ == ਮਾਪ)
            ਵਾਪਸ i;
    }
    ਵਾਪਸ -1;
}


/****************************************************************
 * Low-level floppy IO
 ****************************************************************/

u8 FloppyDOR VARLOW;

ਸਥਿਰ ਸਤਰਅੰਦਰ u8
floppy_dor_read(ਖਾਲੀ)
{
    ਵਾਪਸ GET_ਨੀਵਾਂ(FloppyDOR);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
floppy_dor_write(u8 val)
{
    outb(val, ਪੋਰਟ_FD_DOR);
    ਸੈੱਟ_ਨੀਵਾਂ(FloppyDOR, val);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
floppy_dor_ਨਕਾਬ(u8 off, u8 on)
{
    floppy_dor_write((floppy_dor_read() & ~off) | on);
}

ਸਥਿਰ ਖਾਲੀ
floppy_disable_controller(ਖਾਲੀ)
{
    dprintf(2, "Floppy_disable_controller\n");
    // Clear the reset bit (enter reset state) and clear 'enable IRQ and DMA'
    floppy_dor_ਨਕਾਬ(FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET, 0);
}

ਸਥਿਰ ਪੂਰਨ
floppy_wait_irq(ਖਾਲੀ)
{
    u8 frs = GET_BDA(floppy_recalibration_ਸਥਿਤੀ);
    ਸੈੱਟ_BDA(floppy_recalibration_ਸਥਿਤੀ, frs & ~FRS_IRQ);
    u32 ਸਿਰਾ = timer_calc(FLOPPY_IRQ_TIMEOUT);
    ਲਈ (;;) {
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            floppy_disable_controller();
            ਵਾਪਸ DISK_RET_ETIMEOUT;
        }
        frs = GET_BDA(floppy_recalibration_ਸਥਿਤੀ);
        ਜੇ (frs & FRS_IRQ)
            ਰੋਕੋ;
        // Could use yield_toirq() here, but that causes issues on
        // bochs, so use yield() instead.
        yield();
    }

    ਸੈੱਟ_BDA(floppy_recalibration_ਸਥਿਤੀ, frs & ~FRS_IRQ);
    ਵਾਪਸ DISK_RET_SUCCESS;
}

// Floppy commands
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FCF_WAITIRQ 0x10000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FC_CHECKIRQ    (0x08 | (0<<8) | (2<<12))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FC_SEEK        (0x0f | (2<<8) | (0<<12) | FCF_WAITIRQ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FC_RECALIBRATE (0x07 | (1<<8) | (0<<12) | FCF_WAITIRQ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FC_READID      (0x4a | (1<<8) | (7<<12) | FCF_WAITIRQ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FC_READ        (0xe6 | (8<<8) | (7<<12) | FCF_WAITIRQ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FC_WRITE       (0xc5 | (8<<8) | (7<<12) | FCF_WAITIRQ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FC_FORMAT      (0x4d | (5<<8) | (7<<12) | FCF_WAITIRQ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FC_SPECIFY     (0x03 | (2<<8) | (0<<12))

// Send the specified command and it's parameters to the floppy controller.
ਸਥਿਰ ਪੂਰਨ
floppy_pio(ਪੂਰਨ command, u8 *param)
{
    dprintf(9, "Floppy pio command %x\n", command);
    // Send command and parameters to controller.
    u32 ਸਿਰਾ = timer_calc(FLOPPY_PIO_TIMEOUT);
    ਪੂਰਨ send = (command >> 8) & 0xf;
    ਪੂਰਨ i = 0;
    ਲਈ (;;) {
        u8 sts = inb(ਪੋਰਟ_FD_ਸਥਿਤੀ);
        ਜੇ (!(sts & 0x80)) {
            ਜੇ (timer_check(ਸਿਰਾ)) {
                warn_timeout();
                floppy_disable_controller();
                ਵਾਪਸ DISK_RET_ETIMEOUT;
            }
            yield();
            ਜਾਰੀ;
        }
        ਜੇ (sts & 0x40) {
            floppy_disable_controller();
            ਵਾਪਸ DISK_RET_ECONTROLLER;
        }
        ਜੇ (i == 0)
            outb(command & 0xff, ਪੋਰਟ_FD_ਅੰਕੜੇ);
        ਨਹੀਂਤਾਂ
            outb(param[i-1], ਪੋਰਟ_FD_ਅੰਕੜੇ);
        ਜੇ (i++ >= send)
            ਰੋਕੋ;
    }

    // Wait for command to complete.
    ਜੇ (command & FCF_WAITIRQ) {
        ਪੂਰਨ ret = floppy_wait_irq();
        ਜੇ (ret)
            ਵਾਪਸ ret;
    }

    // Read response from controller.
    ਸਿਰਾ = timer_calc(FLOPPY_PIO_TIMEOUT);
    ਪੂਰਨ receive = (command >> 12) & 0xf;
    i = 0;
    ਲਈ (;;) {
        u8 sts = inb(ਪੋਰਟ_FD_ਸਥਿਤੀ);
        ਜੇ (!(sts & 0x80)) {
            ਜੇ (timer_check(ਸਿਰਾ)) {
                warn_timeout();
                floppy_disable_controller();
                ਵਾਪਸ DISK_RET_ETIMEOUT;
            }
            yield();
            ਜਾਰੀ;
        }
        ਜੇ (i >= receive) {
            ਜੇ (sts & 0x40) {
                floppy_disable_controller();
                ਵਾਪਸ DISK_RET_ECONTROLLER;
            }
            ਰੋਕੋ;
        }
        ਜੇ (!(sts & 0x40)) {
            floppy_disable_controller();
            ਵਾਪਸ DISK_RET_ECONTROLLER;
        }
        param[i++] = inb(ਪੋਰਟ_FD_ਅੰਕੜੇ);
    }

    ਵਾਪਸ DISK_RET_SUCCESS;
}

ਸਥਿਰ ਪੂਰਨ
floppy_enable_controller(ਖਾਲੀ)
{
    dprintf(2, "Floppy_enable_controller\n");
    // Clear the reset bit (enter reset state), but set 'enable IRQ and DMA'
    floppy_dor_ਨਕਾਬ(FLOPPY_DOR_RESET, FLOPPY_DOR_IRQ);
    // Real hardware needs a 4 microsecond delay
    usleep(4);
    // Set the reset bit (normal operation) and keep 'enable IRQ and DMA' on
    floppy_dor_ਨਕਾਬ(0, FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET);
    ਪੂਰਨ ret = floppy_wait_irq();
    ਜੇ (ret)
        ਵਾਪਸ ret;

    // After the interrupt is received, send 4 SENSE INTERRUPT commands to
    // clear the interrupt status for each of the four logical drives,
    // supported by the controller.
    // See section 7.4 - "Drive Polling" of the Intel 82077AA datasheet for
    // a more detailed description of why this voodoo needs to be done.
    // Without this, initialization fails on real controllers (but still works
    // in QEMU)
    u8 param[2];
    ਪੂਰਨ i;
    ਲਈ (i=0; i<4; i++) {
        ret = floppy_pio(FC_CHECKIRQ, param);
        ਜੇ (ret)
            ਵਾਪਸ ret;
    }
    ਵਾਪਸ DISK_RET_SUCCESS;
}

// Activate a drive and send a command to it.
ਸਥਿਰ ਪੂਰਨ
floppy_drive_pio(u8 floppyid, ਪੂਰਨ command, u8 *param)
{
    // Enable controller if it isn't running.
    ਜੇ (!(floppy_dor_read() & FLOPPY_DOR_RESET)) {
        ਪੂਰਨ ret = floppy_enable_controller();
        ਜੇ (ret)
            ਵਾਪਸ ret;
    }

    // set the disk motor timeout value of INT 08 to the highest value
    ਸੈੱਟ_BDA(floppy_motor_counter, 255);

    // Check if the motor is already running
    u8 motor_ਨਕਾਬ = FLOPPY_DOR_MOTOR_A << floppyid;
    ਪੂਰਨ motor_already_running = floppy_dor_read() & motor_ਨਕਾਬ;

    // Turn on motor of selected drive, DMA & int enabled, normal operation
    floppy_dor_write(motor_ਨਕਾਬ | FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET | floppyid);

    // If the motor was just started, wait for it to get up to speed
    ਜੇ (!motor_already_running && !CONFIG_QEMU)
        msleep(FLOPPY_STARTUP_ਸਮਾਂ * 125);

    // Send command.
    ਪੂਰਨ ret = floppy_pio(command, param);
    ਸੈੱਟ_BDA(floppy_motor_counter, FLOPPY_MOTOR_TICKS); // reset motor timeout
    ਜੇ (ret)
        ਵਾਪਸ ret;

    // Check IRQ command is needed after irq commands with no results
    ਜੇ ((command & FCF_WAITIRQ) && ((command >> 12) & 0xf) == 0)
        ਵਾਪਸ floppy_pio(FC_CHECKIRQ, param);
    ਵਾਪਸ DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy media sense and seeking
 ****************************************************************/

ਸਥਿਰ ਪੂਰਨ
floppy_drive_recal(u8 floppyid)
{
    dprintf(2, "Floppy_drive_recal %d\n", floppyid);
    // send Recalibrate command to controller
    u8 param[2];
    param[0] = floppyid;
    ਪੂਰਨ ret = floppy_drive_pio(floppyid, FC_RECALIBRATE, param);
    ਜੇ (ret)
        ਵਾਪਸ ret;

    u8 frs = GET_BDA(floppy_recalibration_ਸਥਿਤੀ);
    ਸੈੱਟ_BDA(floppy_recalibration_ਸਥਿਤੀ, frs | (1<<floppyid));
    ਸੈੱਟ_BDA(floppy_track[floppyid], 0);
    ਵਾਪਸ DISK_RET_SUCCESS;
}

ਸਥਿਰ ਪੂਰਨ
floppy_drive_seek(u8 floppyid, u8 cylinder)
{
    dprintf(2, "Floppy_drive_seek %d to %d\n", floppyid, cylinder);
    u8 param[2];
    param[0] = floppyid;
    param[1] = cylinder;
    ਪੂਰਨ ret = floppy_drive_pio(floppyid, FC_SEEK, param);
    ਜੇ (ret)
        ਵਾਪਸ ret;

    ਸੈੱਟ_BDA(floppy_track[floppyid], cylinder);
    ਵਾਪਸ DISK_RET_SUCCESS;
}

ਸਥਿਰ ਪੂਰਨ
floppy_drive_specify(ਖਾਲੀ)
{
    u8 param[2];
    param[0] = FLOPPY_SPECIFY1;
    param[1] = FLOPPY_SPECIFY2;
    ਵਾਪਸ floppy_pio(FC_SPECIFY, param);
}

ਸਥਿਰ ਪੂਰਨ
floppy_drive_readid(u8 floppyid, u8 ਅੰਕੜੇ_ਦਰ, u8 ਸਿਰ)
{
    // Set data rate.
    outb(ਅੰਕੜੇ_ਦਰ, ਪੋਰਟ_FD_DIR);

    // send Read Sector Id command
    u8 param[7];
    param[0] = (ਸਿਰ << 2) | floppyid; // HD DR1 DR2
    ਪੂਰਨ ret = floppy_drive_pio(floppyid, FC_READID, param);
    ਜੇ (ret)
        ਵਾਪਸ ret;
    ਜੇ (param[0] & 0xc0)
        ਵਾਪਸ -1;
    ਵਾਪਸ 0;
}

// Only the 1.2MB 5.25" drive and the 3.5" drives have a disk change line.  On
// the older 5.25" types the pin is not driven and reads back as a permanent
// "changed".
ਸਥਿਰ ਪੂਰਨ
floppy_has_changeline(u8 ftype)
{
    ਵਾਪਸ ftype >= 2 && ftype <= 5;
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
ਸਥਿਰ ਪੂਰਨ
floppy_check_media(ਬਣਤਰ drive_s *drive_gf, u8 floppyid)
{
    ਜੇ (!floppy_has_changeline(GET_GLOBALFLAT(drive_gf->floppy_type)))
        ਵਾਪਸ DISK_RET_SUCCESS;

    // The caller has just recalibrated this drive, so it is selected in the
    // DOR and the DIR reflects its change line.
    ਜੇ (!(inb(ਪੋਰਟ_FD_DIR) & FLOPPY_DIR_DSKCHG))
        ਵਾਪਸ DISK_RET_SUCCESS;

    ਪੂਰਨ ret = floppy_drive_seek(floppyid, 1);
    ਜੇ (ret)
        ਵਾਪਸ ret;

    ਜੇ (inb(ਪੋਰਟ_FD_DIR) & FLOPPY_DIR_DSKCHG) {
        dprintf(2, "Floppy_check_media %d: no medium\n", floppyid);
        ਵਾਪਸ DISK_RET_ETIMEOUT;
    }

    // A medium is present.  Put the head back on cylinder 0, where the caller
    // left it, so that media sensing reads the track it always did.
    ਵਾਪਸ floppy_drive_seek(floppyid, 0);
}

ਸਥਿਰ ਪੂਰਨ
floppy_media_sense(ਬਣਤਰ drive_s *drive_gf)
{
    u8 ftype = GET_GLOBALFLAT(drive_gf->floppy_type), stype = ftype;
    u8 floppyid = GET_GLOBALFLAT(drive_gf->cntl_id);

    u8 ਅੰਕੜੇ_ਦਰ = GET_GLOBAL(FloppyInfo[stype].ਅੰਕੜੇ_ਦਰ);
    ਪੂਰਨ ret = floppy_drive_readid(floppyid, ਅੰਕੜੇ_ਦਰ, 0);
    ਜੇ (ret) {
        // Attempt media sense.
        ਲਈ (stype=1; ; stype++) {
            ਜੇ (stype >= ARRAY_ਮਾਪ(FloppyInfo))
                ਵਾਪਸ DISK_RET_EBADTRACK;
            ਜੇ (stype==ftype
                || (GET_GLOBAL(FloppyInfo[stype].floppy_ਮਾਪ)
                    != GET_GLOBAL(FloppyInfo[ftype].floppy_ਮਾਪ))
                || (GET_GLOBAL(FloppyInfo[stype].chs.ਸਿਰ)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.ਸਿਰ))
                || (GET_GLOBAL(FloppyInfo[stype].chs.cylinder)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.cylinder))
                || (GET_GLOBAL(FloppyInfo[stype].chs.sector)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.sector)))
                ਜਾਰੀ;
            ਅੰਕੜੇ_ਦਰ = GET_GLOBAL(FloppyInfo[stype].ਅੰਕੜੇ_ਦਰ);
            ret = floppy_drive_readid(floppyid, ਅੰਕੜੇ_ਦਰ, 0);
            ਜੇ (!ret)
                ਰੋਕੋ;
        }
    }
    dprintf(2, "Floppy_media_sense on drive %d found rate %d\n"
            , floppyid, ਅੰਕੜੇ_ਦਰ);

    u8 old_ਅੰਕੜੇ_ਦਰ = GET_BDA(floppy_media_state[floppyid]) >> 6;
    ਸੈੱਟ_BDA(floppy_ਆਖਰੀ_ਅੰਕੜੇ_ਦਰ, (old_ਅੰਕੜੇ_ਦਰ<<2) | (ਅੰਕੜੇ_ਦਰ<<6));
    u8 media = (stype == 1 ? 0x04 : (stype == 2 ? 0x05 : 0x07));
    u8 fms = (ਅੰਕੜੇ_ਦਰ<<6) | FMS_MEDIA_DRIVE_ESTABLISHED | media;
    ਜੇ (GET_GLOBAL(FloppyInfo[stype].chs.cylinder)
        < GET_GLOBAL(FloppyInfo[ftype].chs.cylinder))
        fms |= FMS_DOUBLE_STEPPING;
    ਸੈੱਟ_BDA(floppy_media_state[floppyid], fms);

    ਵਾਪਸ DISK_RET_SUCCESS;
}

// Prepare a floppy for a data transfer.
ਸਥਿਰ ਪੂਰਨ
floppy_prep(ਬਣਤਰ drive_s *drive_gf, u8 cylinder)
{
    u8 floppyid = GET_GLOBALFLAT(drive_gf->cntl_id);
    ਜੇ (!(GET_BDA(floppy_recalibration_ਸਥਿਤੀ) & (1<<floppyid)) ||
        !(GET_BDA(floppy_media_state[floppyid]) & FMS_MEDIA_DRIVE_ESTABLISHED)) {
        // Recalibrate drive.
        ਪੂਰਨ ret = floppy_drive_recal(floppyid);
        ਜੇ (ret)
            ਵਾਪਸ ret;

        // An empty drive gives no index pulses, so the READ ID that
        // floppy_media_sense() issues would never complete and would wait out
        // its timeout once per data rate.  Ask the disk change line instead.
        ret = floppy_check_media(drive_gf, floppyid);
        ਜੇ (ret)
            ਵਾਪਸ ret;

        // Sense media.
        ret = floppy_media_sense(drive_gf);
        ਜੇ (ret)
            ਵਾਪਸ ret;

        // Execute a SPECIFY command (sets the Step Rate Time,
        // Head Load Time, Head Unload Time and the DMA enable/disable bit).
        ret = floppy_drive_specify();
        ਜੇ (ret)
            ਵਾਪਸ ret;
    }

    // Seek to cylinder if needed.
    ਜੇ (cylinder != GET_BDA(floppy_track[floppyid])) {
        ਪੂਰਨ ret = floppy_drive_seek(floppyid, cylinder);
        ਜੇ (ret)
            ਵਾਪਸ ret;
    }

    ਵਾਪਸ DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy DMA transfer
 ****************************************************************/

// Perform a floppy transfer command (setup DMA and issue PIO).
ਸਥਿਰ ਪੂਰਨ
floppy_dma_cmd(ਬਣਤਰ disk_op_s *op, ਪੂਰਨ ਗਿਣਤੀ_2, ਪੂਰਨ command, u8 *param)
{
    // Setup DMA controller
    ਪੂਰਨ isWrite = command != FC_READ;
    ਪੂਰਨ ret = dma_floppy((u32)op->buf_fl, ਗਿਣਤੀ_2, isWrite);
    ਜੇ (ret)
        ਵਾਪਸ DISK_RET_EBOUNDARY;

    // Invoke floppy controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    ret = floppy_drive_pio(floppyid, command, param);
    ਜੇ (ret)
        ਵਾਪਸ ret;

    // Populate floppy_return_status in BDA
    ਪੂਰਨ i;
    ਲਈ (i=0; i<7; i++)
        ਸੈੱਟ_BDA(floppy_return_ਸਥਿਤੀ[i], param[i]);

    ਜੇ (param[0] & 0xc0) {
        ਜੇ (param[1] & 0x02)
            ਵਾਪਸ DISK_RET_EWRITEPROTECT;
        dprintf(1, "floppy error: %02x %02x %02x %02x %02x %02x %02x\n"
                , param[0], param[1], param[2], param[3]
                , param[4], param[5], param[6]);
        ਵਾਪਸ DISK_RET_ECONTROLLER;
    }

    ਵਾਪਸ DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy handlers
 ****************************************************************/

ਸਥਿਰ ਬਣਤਰ chs_s
lba2chs(ਬਣਤਰ disk_op_s *op)
{
    ਬਣਤਰ chs_s res = { };

    u32 tmp = op->lba;
    u16 nls = GET_GLOBALFLAT(op->drive_fl->lchs.sector);
    res.sector = (tmp % nls) + 1;

    tmp /= nls;
    u16 nlh = GET_GLOBALFLAT(op->drive_fl->lchs.ਸਿਰ);
    res.ਸਿਰ = tmp % nlh;

    tmp /= nlh;
    res.cylinder = tmp;

    ਵਾਪਸ res;
}

// diskette controller reset
ਸਥਿਰ ਪੂਰਨ
floppy_reset(ਬਣਤਰ disk_op_s *op)
{
    ਸੈੱਟ_BDA(floppy_recalibration_ਸਥਿਤੀ, 0);
    ਸੈੱਟ_BDA(floppy_media_state[0], 0);
    ਸੈੱਟ_BDA(floppy_media_state[1], 0);
    ਸੈੱਟ_BDA(floppy_track[0], 0);
    ਸੈੱਟ_BDA(floppy_track[1], 0);
    ਸੈੱਟ_BDA(floppy_ਆਖਰੀ_ਅੰਕੜੇ_ਦਰ, 0);
    floppy_disable_controller();
    ਵਾਪਸ floppy_enable_controller();
}

// Read Diskette Sectors
ਸਥਿਰ ਪੂਰਨ
floppy_read(ਬਣਤਰ disk_op_s *op)
{
    ਬਣਤਰ chs_s chs = lba2chs(op);
    ਪੂਰਨ ret = floppy_prep(op->drive_fl, chs.cylinder);
    ਜੇ (ret)
        ਵਾਪਸ ret;

    // send read-normal-data command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[8];
    param[0] = (chs.ਸਿਰ << 2) | floppyid; // HD DR1 DR2
    param[1] = chs.cylinder;
    param[2] = chs.ਸਿਰ;
    param[3] = chs.sector;
    param[4] = FLOPPY_ਮਾਪ_ਕੋਡ;
    param[5] = chs.sector + op->ਗਿਣਤੀ_2 - 1; // last sector to read on track
    param[6] = FLOPPY_GAPLEN;
    param[7] = FLOPPY_DATALEN;
    ਵਾਪਸ floppy_dma_cmd(op, op->ਗਿਣਤੀ_2 * DISK_SECTOR_ਮਾਪ, FC_READ, param);
}

// Write Diskette Sectors
ਸਥਿਰ ਪੂਰਨ
floppy_write(ਬਣਤਰ disk_op_s *op)
{
    ਬਣਤਰ chs_s chs = lba2chs(op);
    ਪੂਰਨ ret = floppy_prep(op->drive_fl, chs.cylinder);
    ਜੇ (ret)
        ਵਾਪਸ ret;

    // send write-normal-data command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[8];
    param[0] = (chs.ਸਿਰ << 2) | floppyid; // HD DR1 DR2
    param[1] = chs.cylinder;
    param[2] = chs.ਸਿਰ;
    param[3] = chs.sector;
    param[4] = FLOPPY_ਮਾਪ_ਕੋਡ;
    param[5] = chs.sector + op->ਗਿਣਤੀ_2 - 1; // last sector to write on track
    param[6] = FLOPPY_GAPLEN;
    param[7] = FLOPPY_DATALEN;
    ਵਾਪਸ floppy_dma_cmd(op, op->ਗਿਣਤੀ_2 * DISK_SECTOR_ਮਾਪ, FC_WRITE, param);
}

// Verify Diskette Sectors
ਸਥਿਰ ਪੂਰਨ
floppy_verify(ਬਣਤਰ disk_op_s *op)
{
    ਬਣਤਰ chs_s chs = lba2chs(op);
    ਪੂਰਨ ret = floppy_prep(op->drive_fl, chs.cylinder);
    ਜੇ (ret)
        ਵਾਪਸ ret;

    // This command isn't implemented - just return success.
    ਵਾਪਸ DISK_RET_SUCCESS;
}

// format diskette track
ਸਥਿਰ ਪੂਰਨ
floppy_format(ਬਣਤਰ disk_op_s *op)
{
    ਬਣਤਰ chs_s chs = lba2chs(op);
    ਪੂਰਨ ret = floppy_prep(op->drive_fl, chs.cylinder);
    ਜੇ (ret)
        ਵਾਪਸ ret;

    // send format-track command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[7];
    param[0] = (chs.ਸਿਰ << 2) | floppyid; // HD DR1 DR2
    param[1] = FLOPPY_ਮਾਪ_ਕੋਡ;
    param[2] = op->ਗਿਣਤੀ_2; // number of sectors per track
    param[3] = FLOPPY_FORMAT_GAPLEN;
    param[4] = FLOPPY_FILLBYTE;
    ਵਾਪਸ floppy_dma_cmd(op, op->ਗਿਣਤੀ_2 * 4, FC_FORMAT, param);
}

ਪੂਰਨ
floppy_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (!CONFIG_FLOPPY)
        ਵਾਪਸ 0;

    ਚੋਣ (op->command) {
    ਹਾਲਤ CMD_RESET:
        ਵਾਪਸ floppy_reset(op);
    ਹਾਲਤ CMD_READ:
        ਵਾਪਸ floppy_read(op);
    ਹਾਲਤ CMD_WRITE:
        ਵਾਪਸ floppy_write(op);
    ਹਾਲਤ CMD_VERIFY:
        ਵਾਪਸ floppy_verify(op);
    ਹਾਲਤ CMD_FORMAT:
        ਵਾਪਸ floppy_format(op);
    ਮੂਲ:
        ਵਾਪਸ DISK_RET_EPARAM;
    }
}


/****************************************************************
 * HW irqs
 ****************************************************************/

// INT 0Eh Diskette Hardware ISR Entry Point
ਖਾਲੀ VISIBLE16
handle_0e(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_FLOPPY)
        ਵਾਪਸ;
    debug_isr(DEBUG_ISR_0e);

    // diskette interrupt has occurred
    u8 frs = GET_BDA(floppy_recalibration_ਸਥਿਤੀ);
    ਸੈੱਟ_BDA(floppy_recalibration_ਸਥਿਤੀ, frs | FRS_IRQ);

    pic_eoi1();
}

// Called from int08 handler.
ਖਾਲੀ
floppy_tick(ਖਾਲੀ)
{
    ਜੇ (! CONFIG_FLOPPY)
        ਵਾਪਸ;

    // time to turn off drive(s)?
    u8 fcount = GET_BDA(floppy_motor_counter);
    ਜੇ (fcount) {
        fcount--;
        ਸੈੱਟ_BDA(floppy_motor_counter, fcount);
        ਜੇ (fcount == 0)
            // turn motor(s) off
            floppy_dor_ਨਕਾਬ(FLOPPY_DOR_MOTOR_ਨਕਾਬ, 0);
    }
}

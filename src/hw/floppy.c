// 16bit code to access floppy drives.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // SET_BDA
#ಅನ್ತರ್ಭಾವ "block.h" // struct drive_s
#ಅನ್ತರ್ಭಾವ "bregs.h" // struct bregs
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_FLOPPY
#ಅನ್ತರ್ಭಾವ "malloc.h" // malloc_fseg
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // pci_find_class
#ಅನ್ತರ್ಭಾವ "pci_ids.h" // PCI_CLASS_BRIDGE_ISA
#ಅನ್ತರ್ಭಾವ "pic.h" // pic_eoi1
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_loadint
#ಅನ್ತರ್ಭಾವ "rtc.h" // rtc_read
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_SUCCESS
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // timer_calc

#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_FD_ತಳ           0x03f0
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_FD_DOR            0x03f2
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_FD_ಸ್ಥಿತಿ         0x03f4
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_FD_ದತ್ತಾಂಶ           0x03f5
#ವ್ಯಾಖ್ಯಾನಿಸು ಪೋರ್ಟ್_FD_DIR            0x03f7

// Digital Input Register (read)
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_DIR_DSKCHG      0x80

#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_ಗಾತ್ರ_ಸಂಕೇತ 0x02 // 512 byte sectors
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_DATALEN 0xff   // Not used - because size code is 0x02
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_MOTOR_TICKS 37 // ~2 seconds
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_FILLBYTE 0xf6
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_GAPLEN 0x1B
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_FORMAT_GAPLEN 0x6c
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_PIO_TIMEOUT 1000
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_IRQ_TIMEOUT 5000
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_SPECIFY1 0xAF  // step rate 12ms, head unload 240ms
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_SPECIFY2 0x02  // head load time 4ms, DMA used
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_STARTUP_ಸಮಯ 8 // 1 second

#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_DOR_MOTOR_D     0x80 // Set to turn drive 3's motor ON
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_DOR_MOTOR_C     0x40 // Set to turn drive 2's motor ON
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_DOR_MOTOR_B     0x20 // Set to turn drive 1's motor ON
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_DOR_MOTOR_A     0x10 // Set to turn drive 0's motor ON
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_DOR_MOTOR_ಮುಖವಾಡ  0xf0
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_DOR_IRQ         0x08 // Set to enable IRQs and DMA
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_DOR_RESET       0x04 // Clear = enter reset mode, Set = normal operation
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_DOR_DSEL_ಮುಖವಾಡ   0x03 // "Select" drive number for next access

// New diskette parameter table adding 3 parameters from IBM
// Since no provisions are made for multiple drive types, most
// values in this table are ignored.  I set parameters for 1.44M
// floppy here
ರಚನೆ floppy_ext_dbt_s diskette_param_table2 VARFSEG = {
    .dbt = {
        .specify1       = FLOPPY_SPECIFY1,
        .specify2       = FLOPPY_SPECIFY2,
        .shutoff_ticks  = FLOPPY_MOTOR_TICKS, // ~2 seconds
        .bps_ಸಂಕೇತ       = FLOPPY_ಗಾತ್ರ_ಸಂಕೇತ,
        .sectors        = 18,
        .interblock_len = FLOPPY_GAPLEN,
        .ದತ್ತಾಂಶ_len       = FLOPPY_DATALEN,
        .gap_len        = FLOPPY_FORMAT_GAPLEN,
        .fill_byte      = FLOPPY_FILLBYTE,
        .settle_ಸಮಯ    = 0x0F, // 15ms
        .startup_ಸಮಯ   = FLOPPY_STARTUP_ಸಮಯ,
    },
    .max_track      = 79,   // maximum track
    .ದತ್ತಾಂಶ_ದರ      = 0,    // data transfer rate
    .drive_type     = 4,    // drive type in cmos
};

ರಚನೆ floppyinfo_s {
    ರಚನೆ chs_s chs;
    u8 floppy_ಗಾತ್ರ;
    u8 ದತ್ತಾಂಶ_ದರ;
};

#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_ಗಾತ್ರ_525 0x01
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_ಗಾತ್ರ_350 0x02

#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_ದರ_500_K 0x00
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_ದರ_300_K 0x01
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_ದರ_250_K 0x02
#ವ್ಯಾಖ್ಯಾನಿಸು FLOPPY_ದರ_1_M   0x03

ರಚನೆ floppyinfo_s FloppyInfo[] VARFSEG = {
    // Unknown
    { {0, 0, 0}, 0x00, 0x00},
    // 1 - 360KB, 5.25" - 2 heads, 40 tracks, 9 sectors
    { {2, 40, 9}, FLOPPY_ಗಾತ್ರ_525, FLOPPY_ದರ_300_K},
    // 2 - 1.2MB, 5.25" - 2 heads, 80 tracks, 15 sectors
    { {2, 80, 15}, FLOPPY_ಗಾತ್ರ_525, FLOPPY_ದರ_500_K},
    // 3 - 720KB, 3.5"  - 2 heads, 80 tracks, 9 sectors
    { {2, 80, 9}, FLOPPY_ಗಾತ್ರ_350, FLOPPY_ದರ_250_K},
    // 4 - 1.44MB, 3.5" - 2 heads, 80 tracks, 18 sectors
    { {2, 80, 18}, FLOPPY_ಗಾತ್ರ_350, FLOPPY_ದರ_500_K},
    // 5 - 2.88MB, 3.5" - 2 heads, 80 tracks, 36 sectors
    { {2, 80, 36}, FLOPPY_ಗಾತ್ರ_350, FLOPPY_ದರ_1_M},
    // 6 - 160k, 5.25"  - 1 heads, 40 tracks, 8 sectors
    { {1, 40, 8}, FLOPPY_ಗಾತ್ರ_525, FLOPPY_ದರ_250_K},
    // 7 - 180k, 5.25"  - 1 heads, 40 tracks, 9 sectors
    { {1, 40, 9}, FLOPPY_ಗಾತ್ರ_525, FLOPPY_ದರ_300_K},
    // 8 - 320k, 5.25"  - 2 heads, 40 tracks, 8 sectors
    { {2, 40, 8}, FLOPPY_ಗಾತ್ರ_525, FLOPPY_ದರ_250_K},
};

ರಚನೆ drive_s *
init_floppy(ಪೂರ್ಣಾಂಕ floppyid, ಪೂರ್ಣಾಂಕ ftype)
{
    ಆದರೆ (ftype <= 0 || ftype >= ARRAY_ಗಾತ್ರ(FloppyInfo)) {
        dprintf(1, "Bad floppy type %d\n", ftype);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }

    ರಚನೆ drive_s *drive = malloc_fseg(ಗಾತ್ರ(*drive));
    ಆದರೆ (!drive) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    memset(drive, 0, ಗಾತ್ರ(*drive));
    drive->cntl_id = floppyid;
    drive->type = DTYPE_FLOPPY;
    drive->blksize = DISK_SECTOR_ಗಾತ್ರ;
    drive->floppy_type = ftype;
    drive->sectors = (u64)-1;

    ಸ್ಮರಣೆ_ನಕಲು(&drive->lchs, &FloppyInfo[ftype].chs
           , ಗಾತ್ರ(FloppyInfo[ftype].chs));
    ಹಿಂತಿರುಗಿ drive;
}

ಸ್ಥಿರ ಶೂನ್ಯ
ಕೂಡಿಸು_Floppy(ಪೂರ್ಣಾಂಕ floppyid, ಪೂರ್ಣಾಂಕ ftype)
{
    ರಚನೆ drive_s *drive = init_floppy(floppyid, ftype);
    ಆದರೆ (!drive)
        ಹಿಂತಿರುಗಿ;
    ಅಕ್ಷರ *desc = znprintf(MAXDESCSIZE, "Floppy [drive %c]", 'A' + floppyid);
    ರಚನೆ pci_device *pci = pci_ಕಂಡುಹಿಡಿ_class(PCI_CLASS_BRIDGE_ISA); /* isa-to-pci bridge */
    ಪೂರ್ಣಾಂಕ prio = bootprio_ಕಂಡುಹಿಡಿ_fdc_device(pci, ಪೋರ್ಟ್_FD_ತಳ, floppyid);
    boot_ಕೂಡಿಸು_floppy(drive, desc, prio);
}

ಶೂನ್ಯ
floppy_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಸ್ಮರಣೆ_ನಕಲು(&diskette_param_ಕೋಷ್ಟಕ, &diskette_param_table2
           , ಗಾತ್ರ(diskette_param_ಕೋಷ್ಟಕ));
    ಗಣ_IVT(0x1E, SEGOFF(SEG_BIOS
                         , (u32)&diskette_param_table2 - BUILD_BIOS_ADDR));

    ಆದರೆ (! CONFIG_FLOPPY)
        ಹಿಂತಿರುಗಿ;
    dprintf(3, "init floppy drives\n");

    ಆದರೆ (CONFIG_QEMU) {
        u8 type = rtc_read(CMOS_FLOPPY_DRIVE_TYPE);
        ಆದರೆ (type & 0xf0)
            ಕೂಡಿಸು_Floppy(0, type >> 4);
        ಆದರೆ (type & 0x0f)
            ಕೂಡಿಸು_Floppy(1, type & 0x0f);
    } ಇಲ್ಲದಿದ್ದರೆ {
        u8 type = romfile_loadint("etc/floppy0", 0);
        ಆದರೆ (type)
            ಕೂಡಿಸು_Floppy(0, type);
        type = romfile_loadint("etc/floppy1", 0);
        ಆದರೆ (type)
            ಕೂಡಿಸು_Floppy(1, type);
    }

    enable_hwirq(6, FUNC16(ನಮೂದು_0e));
}

// Find a floppy type that matches a given image size.
ಪೂರ್ಣಾಂಕ
ಕಂಡುಹಿಡಿ_floppy_type(u32 ಗಾತ್ರ_2)
{
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=1; i<ARRAY_ಗಾತ್ರ(FloppyInfo); i++) {
        ರಚನೆ chs_s *c = &FloppyInfo[i].chs;
        ಆದರೆ (c->cylinder * c->ತಲೆ * c->sector * DISK_SECTOR_ಗಾತ್ರ == ಗಾತ್ರ_2)
            ಹಿಂತಿರುಗಿ i;
    }
    ಹಿಂತಿರುಗಿ -1;
}


/****************************************************************
 * Low-level floppy IO
 ****************************************************************/

u8 FloppyDOR VARLOW;

ಸ್ಥಿರ ಸಾಲೊಳಗೆ u8
floppy_dor_read(ಶೂನ್ಯ)
{
    ಹಿಂತಿರುಗಿ GET_ಕೆಳಗಿನ(FloppyDOR);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
floppy_dor_write(u8 val)
{
    outb(val, ಪೋರ್ಟ್_FD_DOR);
    ಗಣ_ಕೆಳಗಿನ(FloppyDOR, val);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
floppy_dor_ಮುಖವಾಡ(u8 off, u8 on)
{
    floppy_dor_write((floppy_dor_read() & ~off) | on);
}

ಸ್ಥಿರ ಶೂನ್ಯ
floppy_disable_controller(ಶೂನ್ಯ)
{
    dprintf(2, "Floppy_disable_controller\n");
    // Clear the reset bit (enter reset state) and clear 'enable IRQ and DMA'
    floppy_dor_ಮುಖವಾಡ(FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET, 0);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
floppy_wait_irq(ಶೂನ್ಯ)
{
    u8 frs = GET_BDA(floppy_recalibration_ಸ್ಥಿತಿ);
    ಗಣ_BDA(floppy_recalibration_ಸ್ಥಿತಿ, frs & ~FRS_IRQ);
    u32 ಕೊನೆ = timer_calc(FLOPPY_IRQ_TIMEOUT);
    ಗಾಗಿ (;;) {
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            floppy_disable_controller();
            ಹಿಂತಿರುಗಿ DISK_RET_ETIMEOUT;
        }
        frs = GET_BDA(floppy_recalibration_ಸ್ಥಿತಿ);
        ಆದರೆ (frs & FRS_IRQ)
            ನಿಲ್ಲಿಸು;
        // Could use yield_toirq() here, but that causes issues on
        // bochs, so use yield() instead.
        yield();
    }

    ಗಣ_BDA(floppy_recalibration_ಸ್ಥಿತಿ, frs & ~FRS_IRQ);
    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}

// Floppy commands
#ವ್ಯಾಖ್ಯಾನಿಸು FCF_WAITIRQ 0x10000
#ವ್ಯಾಖ್ಯಾನಿಸು FC_CHECKIRQ    (0x08 | (0<<8) | (2<<12))
#ವ್ಯಾಖ್ಯಾನಿಸು FC_SEEK        (0x0f | (2<<8) | (0<<12) | FCF_WAITIRQ)
#ವ್ಯಾಖ್ಯಾನಿಸು FC_RECALIBRATE (0x07 | (1<<8) | (0<<12) | FCF_WAITIRQ)
#ವ್ಯಾಖ್ಯಾನಿಸು FC_READID      (0x4a | (1<<8) | (7<<12) | FCF_WAITIRQ)
#ವ್ಯಾಖ್ಯಾನಿಸು FC_READ        (0xe6 | (8<<8) | (7<<12) | FCF_WAITIRQ)
#ವ್ಯಾಖ್ಯಾನಿಸು FC_WRITE       (0xc5 | (8<<8) | (7<<12) | FCF_WAITIRQ)
#ವ್ಯಾಖ್ಯಾನಿಸು FC_FORMAT      (0x4d | (5<<8) | (7<<12) | FCF_WAITIRQ)
#ವ್ಯಾಖ್ಯಾನಿಸು FC_SPECIFY     (0x03 | (2<<8) | (0<<12))

// Send the specified command and it's parameters to the floppy controller.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
floppy_pio(ಪೂರ್ಣಾಂಕ command, u8 *param)
{
    dprintf(9, "Floppy pio command %x\n", command);
    // Send command and parameters to controller.
    u32 ಕೊನೆ = timer_calc(FLOPPY_PIO_TIMEOUT);
    ಪೂರ್ಣಾಂಕ send = (command >> 8) & 0xf;
    ಪೂರ್ಣಾಂಕ i = 0;
    ಗಾಗಿ (;;) {
        u8 sts = inb(ಪೋರ್ಟ್_FD_ಸ್ಥಿತಿ);
        ಆದರೆ (!(sts & 0x80)) {
            ಆದರೆ (timer_check(ಕೊನೆ)) {
                warn_timeout();
                floppy_disable_controller();
                ಹಿಂತಿರುಗಿ DISK_RET_ETIMEOUT;
            }
            yield();
            ಮುಂದುವರಿ;
        }
        ಆದರೆ (sts & 0x40) {
            floppy_disable_controller();
            ಹಿಂತಿರುಗಿ DISK_RET_ECONTROLLER;
        }
        ಆದರೆ (i == 0)
            outb(command & 0xff, ಪೋರ್ಟ್_FD_ದತ್ತಾಂಶ);
        ಇಲ್ಲದಿದ್ದರೆ
            outb(param[i-1], ಪೋರ್ಟ್_FD_ದತ್ತಾಂಶ);
        ಆದರೆ (i++ >= send)
            ನಿಲ್ಲಿಸು;
    }

    // Wait for command to complete.
    ಆದರೆ (command & FCF_WAITIRQ) {
        ಪೂರ್ಣಾಂಕ ret = floppy_wait_irq();
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ ret;
    }

    // Read response from controller.
    ಕೊನೆ = timer_calc(FLOPPY_PIO_TIMEOUT);
    ಪೂರ್ಣಾಂಕ receive = (command >> 12) & 0xf;
    i = 0;
    ಗಾಗಿ (;;) {
        u8 sts = inb(ಪೋರ್ಟ್_FD_ಸ್ಥಿತಿ);
        ಆದರೆ (!(sts & 0x80)) {
            ಆದರೆ (timer_check(ಕೊನೆ)) {
                warn_timeout();
                floppy_disable_controller();
                ಹಿಂತಿರುಗಿ DISK_RET_ETIMEOUT;
            }
            yield();
            ಮುಂದುವರಿ;
        }
        ಆದರೆ (i >= receive) {
            ಆದರೆ (sts & 0x40) {
                floppy_disable_controller();
                ಹಿಂತಿರುಗಿ DISK_RET_ECONTROLLER;
            }
            ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ (!(sts & 0x40)) {
            floppy_disable_controller();
            ಹಿಂತಿರುಗಿ DISK_RET_ECONTROLLER;
        }
        param[i++] = inb(ಪೋರ್ಟ್_FD_ದತ್ತಾಂಶ);
    }

    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
floppy_enable_controller(ಶೂನ್ಯ)
{
    dprintf(2, "Floppy_enable_controller\n");
    // Clear the reset bit (enter reset state), but set 'enable IRQ and DMA'
    floppy_dor_ಮುಖವಾಡ(FLOPPY_DOR_RESET, FLOPPY_DOR_IRQ);
    // Real hardware needs a 4 microsecond delay
    usleep(4);
    // Set the reset bit (normal operation) and keep 'enable IRQ and DMA' on
    floppy_dor_ಮುಖವಾಡ(0, FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET);
    ಪೂರ್ಣಾಂಕ ret = floppy_wait_irq();
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    // After the interrupt is received, send 4 SENSE INTERRUPT commands to
    // clear the interrupt status for each of the four logical drives,
    // supported by the controller.
    // See section 7.4 - "Drive Polling" of the Intel 82077AA datasheet for
    // a more detailed description of why this voodoo needs to be done.
    // Without this, initialization fails on real controllers (but still works
    // in QEMU)
    u8 param[2];
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<4; i++) {
        ret = floppy_pio(FC_CHECKIRQ, param);
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ ret;
    }
    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}

// Activate a drive and send a command to it.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
floppy_drive_pio(u8 floppyid, ಪೂರ್ಣಾಂಕ command, u8 *param)
{
    // Enable controller if it isn't running.
    ಆದರೆ (!(floppy_dor_read() & FLOPPY_DOR_RESET)) {
        ಪೂರ್ಣಾಂಕ ret = floppy_enable_controller();
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ ret;
    }

    // set the disk motor timeout value of INT 08 to the highest value
    ಗಣ_BDA(floppy_motor_counter, 255);

    // Check if the motor is already running
    u8 motor_ಮುಖವಾಡ = FLOPPY_DOR_MOTOR_A << floppyid;
    ಪೂರ್ಣಾಂಕ motor_already_running = floppy_dor_read() & motor_ಮುಖವಾಡ;

    // Turn on motor of selected drive, DMA & int enabled, normal operation
    floppy_dor_write(motor_ಮುಖವಾಡ | FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET | floppyid);

    // If the motor was just started, wait for it to get up to speed
    ಆದರೆ (!motor_already_running && !CONFIG_QEMU)
        msleep(FLOPPY_STARTUP_ಸಮಯ * 125);

    // Send command.
    ಪೂರ್ಣಾಂಕ ret = floppy_pio(command, param);
    ಗಣ_BDA(floppy_motor_counter, FLOPPY_MOTOR_TICKS); // reset motor timeout
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    // Check IRQ command is needed after irq commands with no results
    ಆದರೆ ((command & FCF_WAITIRQ) && ((command >> 12) & 0xf) == 0)
        ಹಿಂತಿರುಗಿ floppy_pio(FC_CHECKIRQ, param);
    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy media sense and seeking
 ****************************************************************/

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
floppy_drive_recal(u8 floppyid)
{
    dprintf(2, "Floppy_drive_recal %d\n", floppyid);
    // send Recalibrate command to controller
    u8 param[2];
    param[0] = floppyid;
    ಪೂರ್ಣಾಂಕ ret = floppy_drive_pio(floppyid, FC_RECALIBRATE, param);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    u8 frs = GET_BDA(floppy_recalibration_ಸ್ಥಿತಿ);
    ಗಣ_BDA(floppy_recalibration_ಸ್ಥಿತಿ, frs | (1<<floppyid));
    ಗಣ_BDA(floppy_track[floppyid], 0);
    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
floppy_drive_seek(u8 floppyid, u8 cylinder)
{
    dprintf(2, "Floppy_drive_seek %d to %d\n", floppyid, cylinder);
    u8 param[2];
    param[0] = floppyid;
    param[1] = cylinder;
    ಪೂರ್ಣಾಂಕ ret = floppy_drive_pio(floppyid, FC_SEEK, param);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    ಗಣ_BDA(floppy_track[floppyid], cylinder);
    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
floppy_drive_specify(ಶೂನ್ಯ)
{
    u8 param[2];
    param[0] = FLOPPY_SPECIFY1;
    param[1] = FLOPPY_SPECIFY2;
    ಹಿಂತಿರುಗಿ floppy_pio(FC_SPECIFY, param);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
floppy_drive_readid(u8 floppyid, u8 ದತ್ತಾಂಶ_ದರ, u8 ತಲೆ)
{
    // Set data rate.
    outb(ದತ್ತಾಂಶ_ದರ, ಪೋರ್ಟ್_FD_DIR);

    // send Read Sector Id command
    u8 param[7];
    param[0] = (ತಲೆ << 2) | floppyid; // HD DR1 DR2
    ಪೂರ್ಣಾಂಕ ret = floppy_drive_pio(floppyid, FC_READID, param);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;
    ಆದರೆ (param[0] & 0xc0)
        ಹಿಂತಿರುಗಿ -1;
    ಹಿಂತಿರುಗಿ 0;
}

// Only the 1.2MB 5.25" drive and the 3.5" drives have a disk change line.  On
// the older 5.25" types the pin is not driven and reads back as a permanent
// "changed".
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
floppy_has_changeline(u8 ftype)
{
    ಹಿಂತಿರುಗಿ ftype >= 2 && ftype <= 5;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
floppy_check_media(ರಚನೆ drive_s *drive_gf, u8 floppyid)
{
    ಆದರೆ (!floppy_has_changeline(GET_GLOBALFLAT(drive_gf->floppy_type)))
        ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;

    // The caller has just recalibrated this drive, so it is selected in the
    // DOR and the DIR reflects its change line.
    ಆದರೆ (!(inb(ಪೋರ್ಟ್_FD_DIR) & FLOPPY_DIR_DSKCHG))
        ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;

    ಪೂರ್ಣಾಂಕ ret = floppy_drive_seek(floppyid, 1);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    ಆದರೆ (inb(ಪೋರ್ಟ್_FD_DIR) & FLOPPY_DIR_DSKCHG) {
        dprintf(2, "Floppy_check_media %d: no medium\n", floppyid);
        ಹಿಂತಿರುಗಿ DISK_RET_ETIMEOUT;
    }

    // A medium is present.  Put the head back on cylinder 0, where the caller
    // left it, so that media sensing reads the track it always did.
    ಹಿಂತಿರುಗಿ floppy_drive_seek(floppyid, 0);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
floppy_media_sense(ರಚನೆ drive_s *drive_gf)
{
    u8 ftype = GET_GLOBALFLAT(drive_gf->floppy_type), stype = ftype;
    u8 floppyid = GET_GLOBALFLAT(drive_gf->cntl_id);

    u8 ದತ್ತಾಂಶ_ದರ = GET_GLOBAL(FloppyInfo[stype].ದತ್ತಾಂಶ_ದರ);
    ಪೂರ್ಣಾಂಕ ret = floppy_drive_readid(floppyid, ದತ್ತಾಂಶ_ದರ, 0);
    ಆದರೆ (ret) {
        // Attempt media sense.
        ಗಾಗಿ (stype=1; ; stype++) {
            ಆದರೆ (stype >= ARRAY_ಗಾತ್ರ(FloppyInfo))
                ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
            ಆದರೆ (stype==ftype
                || (GET_GLOBAL(FloppyInfo[stype].floppy_ಗಾತ್ರ)
                    != GET_GLOBAL(FloppyInfo[ftype].floppy_ಗಾತ್ರ))
                || (GET_GLOBAL(FloppyInfo[stype].chs.ತಲೆ)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.ತಲೆ))
                || (GET_GLOBAL(FloppyInfo[stype].chs.cylinder)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.cylinder))
                || (GET_GLOBAL(FloppyInfo[stype].chs.sector)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.sector)))
                ಮುಂದುವರಿ;
            ದತ್ತಾಂಶ_ದರ = GET_GLOBAL(FloppyInfo[stype].ದತ್ತಾಂಶ_ದರ);
            ret = floppy_drive_readid(floppyid, ದತ್ತಾಂಶ_ದರ, 0);
            ಆದರೆ (!ret)
                ನಿಲ್ಲಿಸು;
        }
    }
    dprintf(2, "Floppy_media_sense on drive %d found rate %d\n"
            , floppyid, ದತ್ತಾಂಶ_ದರ);

    u8 old_ದತ್ತಾಂಶ_ದರ = GET_BDA(floppy_media_state[floppyid]) >> 6;
    ಗಣ_BDA(floppy_ಕೊನೆಯ_ದತ್ತಾಂಶ_ದರ, (old_ದತ್ತಾಂಶ_ದರ<<2) | (ದತ್ತಾಂಶ_ದರ<<6));
    u8 media = (stype == 1 ? 0x04 : (stype == 2 ? 0x05 : 0x07));
    u8 fms = (ದತ್ತಾಂಶ_ದರ<<6) | FMS_MEDIA_DRIVE_ESTABLISHED | media;
    ಆದರೆ (GET_GLOBAL(FloppyInfo[stype].chs.cylinder)
        < GET_GLOBAL(FloppyInfo[ftype].chs.cylinder))
        fms |= FMS_DOUBLE_STEPPING;
    ಗಣ_BDA(floppy_media_state[floppyid], fms);

    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}

// Prepare a floppy for a data transfer.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
floppy_prep(ರಚನೆ drive_s *drive_gf, u8 cylinder)
{
    u8 floppyid = GET_GLOBALFLAT(drive_gf->cntl_id);
    ಆದರೆ (!(GET_BDA(floppy_recalibration_ಸ್ಥಿತಿ) & (1<<floppyid)) ||
        !(GET_BDA(floppy_media_state[floppyid]) & FMS_MEDIA_DRIVE_ESTABLISHED)) {
        // Recalibrate drive.
        ಪೂರ್ಣಾಂಕ ret = floppy_drive_recal(floppyid);
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ ret;

        // An empty drive gives no index pulses, so the READ ID that
        // floppy_media_sense() issues would never complete and would wait out
        // its timeout once per data rate.  Ask the disk change line instead.
        ret = floppy_check_media(drive_gf, floppyid);
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ ret;

        // Sense media.
        ret = floppy_media_sense(drive_gf);
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ ret;

        // Execute a SPECIFY command (sets the Step Rate Time,
        // Head Load Time, Head Unload Time and the DMA enable/disable bit).
        ret = floppy_drive_specify();
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ ret;
    }

    // Seek to cylinder if needed.
    ಆದರೆ (cylinder != GET_BDA(floppy_track[floppyid])) {
        ಪೂರ್ಣಾಂಕ ret = floppy_drive_seek(floppyid, cylinder);
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ ret;
    }

    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy DMA transfer
 ****************************************************************/

// Perform a floppy transfer command (setup DMA and issue PIO).
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
floppy_dma_cmd(ರಚನೆ disk_op_s *op, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2, ಪೂರ್ಣಾಂಕ command, u8 *param)
{
    // Setup DMA controller
    ಪೂರ್ಣಾಂಕ isWrite = command != FC_READ;
    ಪೂರ್ಣಾಂಕ ret = dma_floppy((u32)op->buf_fl, ಎಣಿಕೆ_2, isWrite);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ DISK_RET_EBOUNDARY;

    // Invoke floppy controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    ret = floppy_drive_pio(floppyid, command, param);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    // Populate floppy_return_status in BDA
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<7; i++)
        ಗಣ_BDA(floppy_return_ಸ್ಥಿತಿ[i], param[i]);

    ಆದರೆ (param[0] & 0xc0) {
        ಆದರೆ (param[1] & 0x02)
            ಹಿಂತಿರುಗಿ DISK_RET_EWRITEPROTECT;
        dprintf(1, "floppy error: %02x %02x %02x %02x %02x %02x %02x\n"
                , param[0], param[1], param[2], param[3]
                , param[4], param[5], param[6]);
        ಹಿಂತಿರುಗಿ DISK_RET_ECONTROLLER;
    }

    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy handlers
 ****************************************************************/

ಸ್ಥಿರ ರಚನೆ chs_s
lba2chs(ರಚನೆ disk_op_s *op)
{
    ರಚನೆ chs_s res = { };

    u32 tmp = op->lba;
    u16 nls = GET_GLOBALFLAT(op->drive_fl->lchs.sector);
    res.sector = (tmp % nls) + 1;

    tmp /= nls;
    u16 nlh = GET_GLOBALFLAT(op->drive_fl->lchs.ತಲೆ);
    res.ತಲೆ = tmp % nlh;

    tmp /= nlh;
    res.cylinder = tmp;

    ಹಿಂತಿರುಗಿ res;
}

// diskette controller reset
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
floppy_reset(ರಚನೆ disk_op_s *op)
{
    ಗಣ_BDA(floppy_recalibration_ಸ್ಥಿತಿ, 0);
    ಗಣ_BDA(floppy_media_state[0], 0);
    ಗಣ_BDA(floppy_media_state[1], 0);
    ಗಣ_BDA(floppy_track[0], 0);
    ಗಣ_BDA(floppy_track[1], 0);
    ಗಣ_BDA(floppy_ಕೊನೆಯ_ದತ್ತಾಂಶ_ದರ, 0);
    floppy_disable_controller();
    ಹಿಂತಿರುಗಿ floppy_enable_controller();
}

// Read Diskette Sectors
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
floppy_read(ರಚನೆ disk_op_s *op)
{
    ರಚನೆ chs_s chs = lba2chs(op);
    ಪೂರ್ಣಾಂಕ ret = floppy_prep(op->drive_fl, chs.cylinder);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    // send read-normal-data command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[8];
    param[0] = (chs.ತಲೆ << 2) | floppyid; // HD DR1 DR2
    param[1] = chs.cylinder;
    param[2] = chs.ತಲೆ;
    param[3] = chs.sector;
    param[4] = FLOPPY_ಗಾತ್ರ_ಸಂಕೇತ;
    param[5] = chs.sector + op->ಎಣಿಕೆ_2 - 1; // last sector to read on track
    param[6] = FLOPPY_GAPLEN;
    param[7] = FLOPPY_DATALEN;
    ಹಿಂತಿರುಗಿ floppy_dma_cmd(op, op->ಎಣಿಕೆ_2 * DISK_SECTOR_ಗಾತ್ರ, FC_READ, param);
}

// Write Diskette Sectors
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
floppy_write(ರಚನೆ disk_op_s *op)
{
    ರಚನೆ chs_s chs = lba2chs(op);
    ಪೂರ್ಣಾಂಕ ret = floppy_prep(op->drive_fl, chs.cylinder);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    // send write-normal-data command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[8];
    param[0] = (chs.ತಲೆ << 2) | floppyid; // HD DR1 DR2
    param[1] = chs.cylinder;
    param[2] = chs.ತಲೆ;
    param[3] = chs.sector;
    param[4] = FLOPPY_ಗಾತ್ರ_ಸಂಕೇತ;
    param[5] = chs.sector + op->ಎಣಿಕೆ_2 - 1; // last sector to write on track
    param[6] = FLOPPY_GAPLEN;
    param[7] = FLOPPY_DATALEN;
    ಹಿಂತಿರುಗಿ floppy_dma_cmd(op, op->ಎಣಿಕೆ_2 * DISK_SECTOR_ಗಾತ್ರ, FC_WRITE, param);
}

// Verify Diskette Sectors
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
floppy_verify(ರಚನೆ disk_op_s *op)
{
    ರಚನೆ chs_s chs = lba2chs(op);
    ಪೂರ್ಣಾಂಕ ret = floppy_prep(op->drive_fl, chs.cylinder);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    // This command isn't implemented - just return success.
    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}

// format diskette track
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
floppy_format(ರಚನೆ disk_op_s *op)
{
    ರಚನೆ chs_s chs = lba2chs(op);
    ಪೂರ್ಣಾಂಕ ret = floppy_prep(op->drive_fl, chs.cylinder);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;

    // send format-track command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[7];
    param[0] = (chs.ತಲೆ << 2) | floppyid; // HD DR1 DR2
    param[1] = FLOPPY_ಗಾತ್ರ_ಸಂಕೇತ;
    param[2] = op->ಎಣಿಕೆ_2; // number of sectors per track
    param[3] = FLOPPY_FORMAT_GAPLEN;
    param[4] = FLOPPY_FILLBYTE;
    ಹಿಂತಿರುಗಿ floppy_dma_cmd(op, op->ಎಣಿಕೆ_2 * 4, FC_FORMAT, param);
}

ಪೂರ್ಣಾಂಕ
floppy_process_op(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (!CONFIG_FLOPPY)
        ಹಿಂತಿರುಗಿ 0;

    ಆಯ್ಕೆ (op->command) {
    ಸ್ಥಿತಿ CMD_RESET:
        ಹಿಂತಿರುಗಿ floppy_reset(op);
    ಸ್ಥಿತಿ CMD_READ:
        ಹಿಂತಿರುಗಿ floppy_read(op);
    ಸ್ಥಿತಿ CMD_WRITE:
        ಹಿಂತಿರುಗಿ floppy_write(op);
    ಸ್ಥಿತಿ CMD_VERIFY:
        ಹಿಂತಿರುಗಿ floppy_verify(op);
    ಸ್ಥಿತಿ CMD_FORMAT:
        ಹಿಂತಿರುಗಿ floppy_format(op);
    ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ DISK_RET_EPARAM;
    }
}


/****************************************************************
 * HW irqs
 ****************************************************************/

// INT 0Eh Diskette Hardware ISR Entry Point
ಶೂನ್ಯ VISIBLE16
handle_0e(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_FLOPPY)
        ಹಿಂತಿರುಗಿ;
    debug_isr(DEBUG_ISR_0e);

    // diskette interrupt has occurred
    u8 frs = GET_BDA(floppy_recalibration_ಸ್ಥಿತಿ);
    ಗಣ_BDA(floppy_recalibration_ಸ್ಥಿತಿ, frs | FRS_IRQ);

    pic_eoi1();
}

// Called from int08 handler.
ಶೂನ್ಯ
floppy_tick(ಶೂನ್ಯ)
{
    ಆದರೆ (! CONFIG_FLOPPY)
        ಹಿಂತಿರುಗಿ;

    // time to turn off drive(s)?
    u8 fcount = GET_BDA(floppy_motor_counter);
    ಆದರೆ (fcount) {
        fcount--;
        ಗಣ_BDA(floppy_motor_counter, fcount);
        ಆದರೆ (fcount == 0)
            // turn motor(s) off
            floppy_dor_ಮುಖವಾಡ(FLOPPY_DOR_MOTOR_ಮುಖವಾಡ, 0);
    }
}

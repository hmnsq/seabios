// 16bit code to access floppy drives.
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "biosvar.h" // SET_BDA
#include "block.h" // struct drive_s
#include "bregs.h" // struct bregs
#include "config.h" // CONFIG_FLOPPY
#include "malloc.h" // malloc_fseg
#include "output.h" // dprintf
#include "pcidevice.h" // pci_find_class
#include "pci_ids.h" // PCI_CLASS_BRIDGE_ISA
#include "pic.h" // pic_eoi1
#include "romfile.h" // romfile_loadint
#include "rtc.h" // rtc_read
#include "stacks.h" // yield
#include "std/disk.h" // DISK_RET_SUCCESS
#include "string.h" // memset
#include "util.h" // timer_calc

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
sa_nracanaa floppy_ext_dbt_s diskette_param_table2 VARFSEG = {
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

sa_nracanaa floppyinfo_s {
    sa_nracanaa chs_s chs;
    u8 floppy_size;
    u8 data_rate;
};

#define FLOPPY_SIZE_525 0x01
#define FLOPPY_SIZE_350 0x02

#define FLOPPY_RATE_500K 0x00
#define FLOPPY_RATE_300K 0x01
#define FLOPPY_RATE_250K 0x02
#define FLOPPY_RATE_1M   0x03

sa_nracanaa floppyinfo_s FloppyInfo[] VARFSEG = {
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

sa_nracanaa drive_s *
init_floppy(puur_naa_nka floppyid, puur_naa_nka ftype)
{
    ᨀᨘ (ftype <= 0 || ftype >= ARRAY_SIZE(FloppyInfo)) {
        dprintf(1, "Bad floppy type %d\n", ftype);
        ᨀᨘᨒ NULL;
    }

    sa_nracanaa drive_s *drive = malloc_fseg(parimaa_na(*drive));
    ᨀᨘ (!drive) {
        warn_noalloc();
        ᨀᨘᨒ NULL;
    }
    memset(drive, 0, parimaa_na(*drive));
    drive->cntl_id = floppyid;
    drive->type = DTYPE_FLOPPY;
    drive->blksize = DISK_SECTOR_SIZE;
    drive->floppy_type = ftype;
    drive->sectors = (u64)-1;

    memcpy(&drive->lchs, &FloppyInfo[ftype].chs
           , parimaa_na(FloppyInfo[ftype].chs));
    ᨀᨘᨒ drive;
}

static void
addFloppy(puur_naa_nka floppyid, puur_naa_nka ftype)
{
    sa_nracanaa drive_s *drive = init_floppy(floppyid, ftype);
    ᨀᨘ (!drive)
        ᨀᨘᨒ;
    var_na *desc = znprintf(MAXDESCSIZE, "Floppy [drive %c]", 'A' + floppyid);
    sa_nracanaa pci_device *pci = pci_find_class(PCI_CLASS_BRIDGE_ISA); /* isa-to-pci bridge */
    puur_naa_nka prio = bootprio_find_fdc_device(pci, PORT_FD_BASE, floppyid);
    boot_add_floppy(drive, desc, prio);
}

void
floppy_setup(void)
{
    memcpy(&diskette_param_table, &diskette_param_table2
           , parimaa_na(diskette_param_table));
    SET_IVT(0x1E, SEGOFF(SEG_BIOS
                         , (u32)&diskette_param_table2 - BUILD_BIOS_ADDR));

    ᨀᨘ (! CONFIG_FLOPPY)
        ᨀᨘᨒ;
    dprintf(3, "init floppy drives\n");

    ᨀᨘ (CONFIG_QEMU) {
        u8 type = rtc_read(CMOS_FLOPPY_DRIVE_TYPE);
        ᨀᨘ (type & 0xf0)
            addFloppy(0, type >> 4);
        ᨀᨘ (type & 0x0f)
            addFloppy(1, type & 0x0f);
    } ᨀᨘᨁ {
        u8 type = romfile_loadint("etc/floppy0", 0);
        ᨀᨘ (type)
            addFloppy(0, type);
        type = romfile_loadint("etc/floppy1", 0);
        ᨀᨘ (type)
            addFloppy(1, type);
    }

    enable_hwirq(6, FUNC16(entry_0e));
}

// Find a floppy type that matches a given image size.
puur_naa_nka
find_floppy_type(u32 size)
{
    puur_naa_nka i;
    for (i=1; i<ARRAY_SIZE(FloppyInfo); i++) {
        sa_nracanaa chs_s *c = &FloppyInfo[i].chs;
        ᨀᨘ (c->cylinder * c->head * c->sector * DISK_SECTOR_SIZE == size)
            ᨀᨘᨒ i;
    }
    ᨀᨘᨒ -1;
}


/****************************************************************
 * Low-level floppy IO
 ****************************************************************/

u8 FloppyDOR VARLOW;

static anta_hstha u8
floppy_dor_read(void)
{
    ᨀᨘᨒ GET_LOW(FloppyDOR);
}

static anta_hstha void
floppy_dor_write(u8 val)
{
    outb(val, PORT_FD_DOR);
    SET_LOW(FloppyDOR, val);
}

static anta_hstha void
floppy_dor_mask(u8 off, u8 on)
{
    floppy_dor_write((floppy_dor_read() & ~off) | on);
}

static void
floppy_disable_controller(void)
{
    dprintf(2, "Floppy_disable_controller\n");
    // Clear the reset bit (enter reset state) and clear 'enable IRQ and DMA'
    floppy_dor_mask(FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET, 0);
}

static puur_naa_nka
floppy_wait_irq(void)
{
    u8 frs = GET_BDA(floppy_recalibration_status);
    SET_BDA(floppy_recalibration_status, frs & ~FRS_IRQ);
    u32 end = timer_calc(FLOPPY_IRQ_TIMEOUT);
    for (;;) {
        ᨀᨘ (timer_check(end)) {
            warn_timeout();
            floppy_disable_controller();
            ᨀᨘᨒ DISK_RET_ETIMEOUT;
        }
        frs = GET_BDA(floppy_recalibration_status);
        ᨀᨘ (frs & FRS_IRQ)
            bha_nga;
        // Could use yield_toirq() here, but that causes issues on
        // bochs, so use yield() instead.
        yield();
    }

    SET_BDA(floppy_recalibration_status, frs & ~FRS_IRQ);
    ᨀᨘᨒ DISK_RET_SUCCESS;
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
static puur_naa_nka
floppy_pio(puur_naa_nka command, u8 *param)
{
    dprintf(9, "Floppy pio command %x\n", command);
    // Send command and parameters to controller.
    u32 end = timer_calc(FLOPPY_PIO_TIMEOUT);
    puur_naa_nka send = (command >> 8) & 0xf;
    puur_naa_nka i = 0;
    for (;;) {
        u8 sts = inb(PORT_FD_STATUS);
        ᨀᨘ (!(sts & 0x80)) {
            ᨀᨘ (timer_check(end)) {
                warn_timeout();
                floppy_disable_controller();
                ᨀᨘᨒ DISK_RET_ETIMEOUT;
            }
            yield();
            continue;
        }
        ᨀᨘ (sts & 0x40) {
            floppy_disable_controller();
            ᨀᨘᨒ DISK_RET_ECONTROLLER;
        }
        ᨀᨘ (i == 0)
            outb(command & 0xff, PORT_FD_DATA);
        ᨀᨘᨁ
            outb(param[i-1], PORT_FD_DATA);
        ᨀᨘ (i++ >= send)
            bha_nga;
    }

    // Wait for command to complete.
    ᨀᨘ (command & FCF_WAITIRQ) {
        puur_naa_nka ret = floppy_wait_irq();
        ᨀᨘ (ret)
            ᨀᨘᨒ ret;
    }

    // Read response from controller.
    end = timer_calc(FLOPPY_PIO_TIMEOUT);
    puur_naa_nka receive = (command >> 12) & 0xf;
    i = 0;
    for (;;) {
        u8 sts = inb(PORT_FD_STATUS);
        ᨀᨘ (!(sts & 0x80)) {
            ᨀᨘ (timer_check(end)) {
                warn_timeout();
                floppy_disable_controller();
                ᨀᨘᨒ DISK_RET_ETIMEOUT;
            }
            yield();
            continue;
        }
        ᨀᨘ (i >= receive) {
            ᨀᨘ (sts & 0x40) {
                floppy_disable_controller();
                ᨀᨘᨒ DISK_RET_ECONTROLLER;
            }
            bha_nga;
        }
        ᨀᨘ (!(sts & 0x40)) {
            floppy_disable_controller();
            ᨀᨘᨒ DISK_RET_ECONTROLLER;
        }
        param[i++] = inb(PORT_FD_DATA);
    }

    ᨀᨘᨒ DISK_RET_SUCCESS;
}

static puur_naa_nka
floppy_enable_controller(void)
{
    dprintf(2, "Floppy_enable_controller\n");
    // Clear the reset bit (enter reset state), but set 'enable IRQ and DMA'
    floppy_dor_mask(FLOPPY_DOR_RESET, FLOPPY_DOR_IRQ);
    // Real hardware needs a 4 microsecond delay
    usleep(4);
    // Set the reset bit (normal operation) and keep 'enable IRQ and DMA' on
    floppy_dor_mask(0, FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET);
    puur_naa_nka ret = floppy_wait_irq();
    ᨀᨘ (ret)
        ᨀᨘᨒ ret;

    // After the interrupt is received, send 4 SENSE INTERRUPT commands to
    // clear the interrupt status for each of the four logical drives,
    // supported by the controller.
    // See section 7.4 - "Drive Polling" of the Intel 82077AA datasheet for
    // a more detailed description of why this voodoo needs to be done.
    // Without this, initialization fails on real controllers (but still works
    // in QEMU)
    u8 param[2];
    puur_naa_nka i;
    for (i=0; i<4; i++) {
        ret = floppy_pio(FC_CHECKIRQ, param);
        ᨀᨘ (ret)
            ᨀᨘᨒ ret;
    }
    ᨀᨘᨒ DISK_RET_SUCCESS;
}

// Activate a drive and send a command to it.
static puur_naa_nka
floppy_drive_pio(u8 floppyid, puur_naa_nka command, u8 *param)
{
    // Enable controller if it isn't running.
    ᨀᨘ (!(floppy_dor_read() & FLOPPY_DOR_RESET)) {
        puur_naa_nka ret = floppy_enable_controller();
        ᨀᨘ (ret)
            ᨀᨘᨒ ret;
    }

    // set the disk motor timeout value of INT 08 to the highest value
    SET_BDA(floppy_motor_counter, 255);

    // Check if the motor is already running
    u8 motor_mask = FLOPPY_DOR_MOTOR_A << floppyid;
    puur_naa_nka motor_already_running = floppy_dor_read() & motor_mask;

    // Turn on motor of selected drive, DMA & int enabled, normal operation
    floppy_dor_write(motor_mask | FLOPPY_DOR_IRQ | FLOPPY_DOR_RESET | floppyid);

    // If the motor was just started, wait for it to get up to speed
    ᨀᨘ (!motor_already_running && !CONFIG_QEMU)
        msleep(FLOPPY_STARTUP_TIME * 125);

    // Send command.
    puur_naa_nka ret = floppy_pio(command, param);
    SET_BDA(floppy_motor_counter, FLOPPY_MOTOR_TICKS); // reset motor timeout
    ᨀᨘ (ret)
        ᨀᨘᨒ ret;

    // Check IRQ command is needed after irq commands with no results
    ᨀᨘ ((command & FCF_WAITIRQ) && ((command >> 12) & 0xf) == 0)
        ᨀᨘᨒ floppy_pio(FC_CHECKIRQ, param);
    ᨀᨘᨒ DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy media sense and seeking
 ****************************************************************/

static puur_naa_nka
floppy_drive_recal(u8 floppyid)
{
    dprintf(2, "Floppy_drive_recal %d\n", floppyid);
    // send Recalibrate command to controller
    u8 param[2];
    param[0] = floppyid;
    puur_naa_nka ret = floppy_drive_pio(floppyid, FC_RECALIBRATE, param);
    ᨀᨘ (ret)
        ᨀᨘᨒ ret;

    u8 frs = GET_BDA(floppy_recalibration_status);
    SET_BDA(floppy_recalibration_status, frs | (1<<floppyid));
    SET_BDA(floppy_track[floppyid], 0);
    ᨀᨘᨒ DISK_RET_SUCCESS;
}

static puur_naa_nka
floppy_drive_seek(u8 floppyid, u8 cylinder)
{
    dprintf(2, "Floppy_drive_seek %d to %d\n", floppyid, cylinder);
    u8 param[2];
    param[0] = floppyid;
    param[1] = cylinder;
    puur_naa_nka ret = floppy_drive_pio(floppyid, FC_SEEK, param);
    ᨀᨘ (ret)
        ᨀᨘᨒ ret;

    SET_BDA(floppy_track[floppyid], cylinder);
    ᨀᨘᨒ DISK_RET_SUCCESS;
}

static puur_naa_nka
floppy_drive_specify(void)
{
    u8 param[2];
    param[0] = FLOPPY_SPECIFY1;
    param[1] = FLOPPY_SPECIFY2;
    ᨀᨘᨒ floppy_pio(FC_SPECIFY, param);
}

static puur_naa_nka
floppy_drive_readid(u8 floppyid, u8 data_rate, u8 head)
{
    // Set data rate.
    outb(data_rate, PORT_FD_DIR);

    // send Read Sector Id command
    u8 param[7];
    param[0] = (head << 2) | floppyid; // HD DR1 DR2
    puur_naa_nka ret = floppy_drive_pio(floppyid, FC_READID, param);
    ᨀᨘ (ret)
        ᨀᨘᨒ ret;
    ᨀᨘ (param[0] & 0xc0)
        ᨀᨘᨒ -1;
    ᨀᨘᨒ 0;
}

// Only the 1.2MB 5.25" drive and the 3.5" drives have a disk change line.  On
// the older 5.25" types the pin is not driven and reads back as a permanent
// "changed".
static puur_naa_nka
floppy_has_changeline(u8 ftype)
{
    ᨀᨘᨒ ftype >= 2 && ftype <= 5;
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
static puur_naa_nka
floppy_check_media(sa_nracanaa drive_s *drive_gf, u8 floppyid)
{
    ᨀᨘ (!floppy_has_changeline(GET_GLOBALFLAT(drive_gf->floppy_type)))
        ᨀᨘᨒ DISK_RET_SUCCESS;

    // The caller has just recalibrated this drive, so it is selected in the
    // DOR and the DIR reflects its change line.
    ᨀᨘ (!(inb(PORT_FD_DIR) & FLOPPY_DIR_DSKCHG))
        ᨀᨘᨒ DISK_RET_SUCCESS;

    puur_naa_nka ret = floppy_drive_seek(floppyid, 1);
    ᨀᨘ (ret)
        ᨀᨘᨒ ret;

    ᨀᨘ (inb(PORT_FD_DIR) & FLOPPY_DIR_DSKCHG) {
        dprintf(2, "Floppy_check_media %d: no medium\n", floppyid);
        ᨀᨘᨒ DISK_RET_ETIMEOUT;
    }

    // A medium is present.  Put the head back on cylinder 0, where the caller
    // left it, so that media sensing reads the track it always did.
    ᨀᨘᨒ floppy_drive_seek(floppyid, 0);
}

static puur_naa_nka
floppy_media_sense(sa_nracanaa drive_s *drive_gf)
{
    u8 ftype = GET_GLOBALFLAT(drive_gf->floppy_type), stype = ftype;
    u8 floppyid = GET_GLOBALFLAT(drive_gf->cntl_id);

    u8 data_rate = GET_GLOBAL(FloppyInfo[stype].data_rate);
    puur_naa_nka ret = floppy_drive_readid(floppyid, data_rate, 0);
    ᨀᨘ (ret) {
        // Attempt media sense.
        for (stype=1; ; stype++) {
            ᨀᨘ (stype >= ARRAY_SIZE(FloppyInfo))
                ᨀᨘᨒ DISK_RET_EBADTRACK;
            ᨀᨘ (stype==ftype
                || (GET_GLOBAL(FloppyInfo[stype].floppy_size)
                    != GET_GLOBAL(FloppyInfo[ftype].floppy_size))
                || (GET_GLOBAL(FloppyInfo[stype].chs.head)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.head))
                || (GET_GLOBAL(FloppyInfo[stype].chs.cylinder)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.cylinder))
                || (GET_GLOBAL(FloppyInfo[stype].chs.sector)
                    > GET_GLOBAL(FloppyInfo[ftype].chs.sector)))
                continue;
            data_rate = GET_GLOBAL(FloppyInfo[stype].data_rate);
            ret = floppy_drive_readid(floppyid, data_rate, 0);
            ᨀᨘ (!ret)
                bha_nga;
        }
    }
    dprintf(2, "Floppy_media_sense on drive %d found rate %d\n"
            , floppyid, data_rate);

    u8 old_data_rate = GET_BDA(floppy_media_state[floppyid]) >> 6;
    SET_BDA(floppy_last_data_rate, (old_data_rate<<2) | (data_rate<<6));
    u8 media = (stype == 1 ? 0x04 : (stype == 2 ? 0x05 : 0x07));
    u8 fms = (data_rate<<6) | FMS_MEDIA_DRIVE_ESTABLISHED | media;
    ᨀᨘ (GET_GLOBAL(FloppyInfo[stype].chs.cylinder)
        < GET_GLOBAL(FloppyInfo[ftype].chs.cylinder))
        fms |= FMS_DOUBLE_STEPPING;
    SET_BDA(floppy_media_state[floppyid], fms);

    ᨀᨘᨒ DISK_RET_SUCCESS;
}

// Prepare a floppy for a data transfer.
static puur_naa_nka
floppy_prep(sa_nracanaa drive_s *drive_gf, u8 cylinder)
{
    u8 floppyid = GET_GLOBALFLAT(drive_gf->cntl_id);
    ᨀᨘ (!(GET_BDA(floppy_recalibration_status) & (1<<floppyid)) ||
        !(GET_BDA(floppy_media_state[floppyid]) & FMS_MEDIA_DRIVE_ESTABLISHED)) {
        // Recalibrate drive.
        puur_naa_nka ret = floppy_drive_recal(floppyid);
        ᨀᨘ (ret)
            ᨀᨘᨒ ret;

        // An empty drive gives no index pulses, so the READ ID that
        // floppy_media_sense() issues would never complete and would wait out
        // its timeout once per data rate.  Ask the disk change line instead.
        ret = floppy_check_media(drive_gf, floppyid);
        ᨀᨘ (ret)
            ᨀᨘᨒ ret;

        // Sense media.
        ret = floppy_media_sense(drive_gf);
        ᨀᨘ (ret)
            ᨀᨘᨒ ret;

        // Execute a SPECIFY command (sets the Step Rate Time,
        // Head Load Time, Head Unload Time and the DMA enable/disable bit).
        ret = floppy_drive_specify();
        ᨀᨘ (ret)
            ᨀᨘᨒ ret;
    }

    // Seek to cylinder if needed.
    ᨀᨘ (cylinder != GET_BDA(floppy_track[floppyid])) {
        puur_naa_nka ret = floppy_drive_seek(floppyid, cylinder);
        ᨀᨘ (ret)
            ᨀᨘᨒ ret;
    }

    ᨀᨘᨒ DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy DMA transfer
 ****************************************************************/

// Perform a floppy transfer command (setup DMA and issue PIO).
static puur_naa_nka
floppy_dma_cmd(sa_nracanaa disk_op_s *op, puur_naa_nka count, puur_naa_nka command, u8 *param)
{
    // Setup DMA controller
    puur_naa_nka isWrite = command != FC_READ;
    puur_naa_nka ret = dma_floppy((u32)op->buf_fl, count, isWrite);
    ᨀᨘ (ret)
        ᨀᨘᨒ DISK_RET_EBOUNDARY;

    // Invoke floppy controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    ret = floppy_drive_pio(floppyid, command, param);
    ᨀᨘ (ret)
        ᨀᨘᨒ ret;

    // Populate floppy_return_status in BDA
    puur_naa_nka i;
    for (i=0; i<7; i++)
        SET_BDA(floppy_return_status[i], param[i]);

    ᨀᨘ (param[0] & 0xc0) {
        ᨀᨘ (param[1] & 0x02)
            ᨀᨘᨒ DISK_RET_EWRITEPROTECT;
        dprintf(1, "floppy error: %02x %02x %02x %02x %02x %02x %02x\n"
                , param[0], param[1], param[2], param[3]
                , param[4], param[5], param[6]);
        ᨀᨘᨒ DISK_RET_ECONTROLLER;
    }

    ᨀᨘᨒ DISK_RET_SUCCESS;
}


/****************************************************************
 * Floppy handlers
 ****************************************************************/

static sa_nracanaa chs_s
lba2chs(sa_nracanaa disk_op_s *op)
{
    sa_nracanaa chs_s res = { };

    u32 tmp = op->lba;
    u16 nls = GET_GLOBALFLAT(op->drive_fl->lchs.sector);
    res.sector = (tmp % nls) + 1;

    tmp /= nls;
    u16 nlh = GET_GLOBALFLAT(op->drive_fl->lchs.head);
    res.head = tmp % nlh;

    tmp /= nlh;
    res.cylinder = tmp;

    ᨀᨘᨒ res;
}

// diskette controller reset
static puur_naa_nka
floppy_reset(sa_nracanaa disk_op_s *op)
{
    SET_BDA(floppy_recalibration_status, 0);
    SET_BDA(floppy_media_state[0], 0);
    SET_BDA(floppy_media_state[1], 0);
    SET_BDA(floppy_track[0], 0);
    SET_BDA(floppy_track[1], 0);
    SET_BDA(floppy_last_data_rate, 0);
    floppy_disable_controller();
    ᨀᨘᨒ floppy_enable_controller();
}

// Read Diskette Sectors
static puur_naa_nka
floppy_read(sa_nracanaa disk_op_s *op)
{
    sa_nracanaa chs_s chs = lba2chs(op);
    puur_naa_nka ret = floppy_prep(op->drive_fl, chs.cylinder);
    ᨀᨘ (ret)
        ᨀᨘᨒ ret;

    // send read-normal-data command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[8];
    param[0] = (chs.head << 2) | floppyid; // HD DR1 DR2
    param[1] = chs.cylinder;
    param[2] = chs.head;
    param[3] = chs.sector;
    param[4] = FLOPPY_SIZE_CODE;
    param[5] = chs.sector + op->count - 1; // last sector to read on track
    param[6] = FLOPPY_GAPLEN;
    param[7] = FLOPPY_DATALEN;
    ᨀᨘᨒ floppy_dma_cmd(op, op->count * DISK_SECTOR_SIZE, FC_READ, param);
}

// Write Diskette Sectors
static puur_naa_nka
floppy_write(sa_nracanaa disk_op_s *op)
{
    sa_nracanaa chs_s chs = lba2chs(op);
    puur_naa_nka ret = floppy_prep(op->drive_fl, chs.cylinder);
    ᨀᨘ (ret)
        ᨀᨘᨒ ret;

    // send write-normal-data command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[8];
    param[0] = (chs.head << 2) | floppyid; // HD DR1 DR2
    param[1] = chs.cylinder;
    param[2] = chs.head;
    param[3] = chs.sector;
    param[4] = FLOPPY_SIZE_CODE;
    param[5] = chs.sector + op->count - 1; // last sector to write on track
    param[6] = FLOPPY_GAPLEN;
    param[7] = FLOPPY_DATALEN;
    ᨀᨘᨒ floppy_dma_cmd(op, op->count * DISK_SECTOR_SIZE, FC_WRITE, param);
}

// Verify Diskette Sectors
static puur_naa_nka
floppy_verify(sa_nracanaa disk_op_s *op)
{
    sa_nracanaa chs_s chs = lba2chs(op);
    puur_naa_nka ret = floppy_prep(op->drive_fl, chs.cylinder);
    ᨀᨘ (ret)
        ᨀᨘᨒ ret;

    // This command isn't implemented - just return success.
    ᨀᨘᨒ DISK_RET_SUCCESS;
}

// format diskette track
static puur_naa_nka
floppy_format(sa_nracanaa disk_op_s *op)
{
    sa_nracanaa chs_s chs = lba2chs(op);
    puur_naa_nka ret = floppy_prep(op->drive_fl, chs.cylinder);
    ᨀᨘ (ret)
        ᨀᨘᨒ ret;

    // send format-track command to controller
    u8 floppyid = GET_GLOBALFLAT(op->drive_fl->cntl_id);
    u8 param[7];
    param[0] = (chs.head << 2) | floppyid; // HD DR1 DR2
    param[1] = FLOPPY_SIZE_CODE;
    param[2] = op->count; // number of sectors per track
    param[3] = FLOPPY_FORMAT_GAPLEN;
    param[4] = FLOPPY_FILLBYTE;
    ᨀᨘᨒ floppy_dma_cmd(op, op->count * 4, FC_FORMAT, param);
}

puur_naa_nka
floppy_process_op(sa_nracanaa disk_op_s *op)
{
    ᨀᨘ (!CONFIG_FLOPPY)
        ᨀᨘᨒ 0;

    switch (op->command) {
    pak_sa CMD_RESET:
        ᨀᨘᨒ floppy_reset(op);
    pak_sa CMD_READ:
        ᨀᨘᨒ floppy_read(op);
    pak_sa CMD_WRITE:
        ᨀᨘᨒ floppy_write(op);
    pak_sa CMD_VERIFY:
        ᨀᨘᨒ floppy_verify(op);
    pak_sa CMD_FORMAT:
        ᨀᨘᨒ floppy_format(op);
    default:
        ᨀᨘᨒ DISK_RET_EPARAM;
    }
}


/****************************************************************
 * HW irqs
 ****************************************************************/

// INT 0Eh Diskette Hardware ISR Entry Point
void VISIBLE16
handle_0e(void)
{
    ᨀᨘ (! CONFIG_FLOPPY)
        ᨀᨘᨒ;
    debug_isr(DEBUG_ISR_0e);

    // diskette interrupt has occurred
    u8 frs = GET_BDA(floppy_recalibration_status);
    SET_BDA(floppy_recalibration_status, frs | FRS_IRQ);

    pic_eoi1();
}

// Called from int08 handler.
void
floppy_tick(void)
{
    ᨀᨘ (! CONFIG_FLOPPY)
        ᨀᨘᨒ;

    // time to turn off drive(s)?
    u8 fcount = GET_BDA(floppy_motor_counter);
    ᨀᨘ (fcount) {
        fcount--;
        SET_BDA(floppy_motor_counter, fcount);
        ᨀᨘ (fcount == 0)
            // turn motor(s) off
            floppy_dor_mask(FLOPPY_DOR_MOTOR_MASK, 0);
    }
}

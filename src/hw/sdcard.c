// PCI SD Host Controller Interface
//
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "block.h" // struct drive_s
#अन्तर्भाव "malloc.h" // malloc_fseg
#अन्तर्भाव "output.h" // znprintf
#अन्तर्भाव "pcidevice.h" // foreachpci
#अन्तर्भाव "pci_ids.h" // PCI_CLASS_SYSTEM_SDHCI
#अन्तर्भाव "pci_regs.h" // PCI_BASE_ADDRESS_0
#अन्तर्भाव "romfile.h" // romfile_findprefix
#अन्तर्भाव "stacks.h" // yield
#अन्तर्भाव "std/disk.h" // DISK_RET_SUCCESS
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // boot_add_hd
#अन्तर्भाव "x86.h" // writel

// SDHCI MMIO registers
संरचना sdhci_s {
    u32 sdma_addr;
    u16 block_size;
    u16 block_गणनम्;
    u32 arg;
    u16 transfer_mode;
    u16 cmd;
    u32 response[4];
    u32 data;
    u32 present_state;
    u8 host_control;
    u8 power_control;
    u8 block_gap_control;
    u8 wakeup_control;
    u16 clock_control;
    u8 timeout_control;
    u8 software_reset;
    u16 irq_status;
    u16 error_irq_status;
    u16 irq_enable;
    u16 error_irq_enable;
    u16 irq_signal;
    u16 error_signal;
    u16 auto_cmd12;
    u16 host_control2;
    u32 cap_lo, cap_hi;
    u64 max_current;
    u16 force_auto_cmd12;
    u16 force_error;
    u8 adma_error;
    u8 pad_55[3];
    u64 adma_addr;
    u8 pad_60[156];
    u16 slot_irq;
    u16 controller_version;
} PACKED;

// SDHCI commands
#define SCB_R0   0x00 // No response
#define SCB_R48  0x1a // Response R1 (no data), R5, R6, R7
#define SCB_R48d 0x3a // Response R1 (with data)
#define SCB_R48b 0x1b // Response R1b, R5b
#define SCB_R48o 0x02 // Response R3, R4
#define SCB_R136 0x09 // Response R2
#define SC_GO_IDLE_STATE        ((0<<8) | SCB_R0)
#define SC_SEND_OP_COND         ((1<<8) | SCB_R48o)
#define SC_ALL_SEND_CID         ((2<<8) | SCB_R136)
#define SC_SEND_RELATIVE_ADDR   ((3<<8) | SCB_R48)
#define SC_SELECT_DESELECT_CARD ((7<<8) | SCB_R48b)
#define SC_SEND_IF_COND         ((8<<8) | SCB_R48)
#define SC_SEND_EXT_CSD         ((8<<8) | SCB_R48d)
#define SC_SEND_CSD             ((9<<8) | SCB_R136)
#define SC_READ_SINGLE          ((17<<8) | SCB_R48d)
#define SC_READ_MULTIPLE        ((18<<8) | SCB_R48d)
#define SC_WRITE_SINGLE         ((24<<8) | SCB_R48d)
#define SC_WRITE_MULTIPLE       ((25<<8) | SCB_R48d)
#define SC_APP_CMD              ((55<<8) | SCB_R48)
#define SC_APP_SEND_OP_COND ((41<<8) | SCB_R48o)

// SDHCI irqs
#define SI_CMD_COMPLETE (1<<0)
#define SI_TRANS_DONE   (1<<1)
#define SI_WRITE_READY  (1<<4)
#define SI_READ_READY   (1<<5)
#define SI_ERROR        (1<<15)

// SDHCI present_state flags
#define SP_CMD_INHIBIT   (1<<0)
#define SP_DAT_INHIBIT   (1<<1)
#define SP_CARD_INSERTED (1<<16)

// SDHCI transfer_mode flags
#define ST_BLOCKCOUNT (1<<1)
#define ST_AUTO_CMD12 (1<<2)
#define ST_READ       (1<<4)
#define ST_MULTIPLE   (1<<5)

// SDHCI capabilities flags
#define SD_CAPLO_V33             (1<<24)
#define SD_CAPLO_V30             (1<<25)
#define SD_CAPLO_V18             (1<<26)
#define SD_CAPLO_BASECLOCK_SHIFT 8
#define SD_CAPLO_BASECLOCK_MASK  0xff

// SDHCI clock control flags
#define SCC_INTERNAL_ENABLE (1<<0)
#define SCC_STABLE          (1<<1)
#define SCC_CLOCK_ENABLE    (1<<2)
#define SCC_SDCLK_MASK      0xff
#define SCC_SDCLK_SHIFT     8
#define SCC_SDCLK_HI_MASK   0x300
#define SCC_SDCLK_HI_RSHIFT 2

// SDHCI power control flags
#define SPC_POWER_ON (1<<0)
#define SPC_V18      0x0a
#define SPC_V30      0x0c
#define SPC_V33      0x0e

// SDHCI software reset flags
#define SRF_ALL  0x01
#define SRF_CMD  0x02
#define SRF_DATA 0x04

// SDHCI result flags
#define SR_OCR_CCS     (1<<30)
#define SR_OCR_NOTBUSY (1<<31)

// SDHCI timeouts
#define SDHCI_POWER_OFF_TIME   1
#define SDHCI_POWER_ON_TIME    5
#define SDHCI_CLOCK_ON_TIME    1 // 74 clock cycles
#define SDHCI_POWERUP_TIMEOUT  1000
#define SDHCI_PIO_TIMEOUT      1000  // XXX - this is just made up

// Internal 'struct drive_s' storage for a detected card
संरचना sddrive_s {
    संरचना drive_s drive;
    संरचना sdhci_s *regs;
    पूर्णाङ्क card_type;
};

// SD card types
#define SF_MMC          (1<<0)
#define SF_HIGHCAPACITY (1<<1)

// Repeatedly read a u16 register until any bit in a given mask is set
स्थिर पूर्णाङ्क
sdcard_waitw(u16 *reg, u16 mask)
{
    u32 end = timer_calc(SDHCI_PIO_TIMEOUT);
    प्रति (;;) {
        u16 v = readw(reg);
        जइ (v & mask)
            प्रत्यागम v;
        जइ (timer_check(end)) {
            dprintf(1, "scard_waitw: %p %x %x\n", reg, mask, v);
            warn_timeout();
            प्रत्यागम -1;
        }
        yield();
    }
}

// Send an sdhci reset
स्थिर पूर्णाङ्क
sdcard_reset(संरचना sdhci_s *regs, पूर्णाङ्क flags)
{
    writeb(&regs->software_reset, flags);
    u32 end = timer_calc(SDHCI_PIO_TIMEOUT);
    जाव (readb(&regs->software_reset))
        जइ (timer_check(end)) {
            warn_timeout();
            प्रत्यागम -1;
        }
    प्रत्यागम 0;
}

// Send a command to the card.
स्थिर पूर्णाङ्क
sdcard_pio(संरचना sdhci_s *regs, पूर्णाङ्क cmd, u32 *param)
{
    u32 state = readl(&regs->present_state);
    dprintf(9, "sdcard_pio cmd %x %x %x\n", cmd, *param, state);
    जइ ((state & SP_CMD_INHIBIT)
        || ((cmd & 0x03) == 0x03 && state & SP_DAT_INHIBIT)) {
        dprintf(1, "sdcard_pio not ready %x\n", state);
        प्रत्यागम -1;
    }
    // Send command
    writel(&regs->arg, *param);
    writew(&regs->cmd, cmd);
    पूर्णाङ्क ret = sdcard_waitw(&regs->irq_status, SI_ERROR|SI_CMD_COMPLETE);
    जइ (ret < 0)
        प्रत्यागम ret;
    जइ (ret & SI_ERROR) {
        u16 err = readw(&regs->error_irq_status);
        dprintf(3, "sdcard_pio command stop (code=%x)\n", err);
        sdcard_reset(regs, SRF_CMD|SRF_DATA);
        writew(&regs->error_irq_status, err);
        प्रत्यागम -1;
    }
    writew(&regs->irq_status, SI_CMD_COMPLETE);
    // Read response
    memcpy(param, regs->response, परिमाण(regs->response));
    dprintf(9, "sdcard cmd %x response %x %x %x %x\n"
            , cmd, param[0], param[1], param[2], param[3]);
    प्रत्यागम 0;
}

// Send an "app specific" command to the card.
स्थिर पूर्णाङ्क
sdcard_pio_app(संरचना sdhci_s *regs, पूर्णाङ्क cmd, u32 *param)
{
    u32 aparam[4] = {};
    पूर्णाङ्क ret = sdcard_pio(regs, SC_APP_CMD, aparam);
    जइ (ret)
        प्रत्यागम ret;
    प्रत्यागम sdcard_pio(regs, cmd, param);
}

// Send a command to the card which transfers data.
स्थिर पूर्णाङ्क
sdcard_pio_transfer(संरचना sddrive_s *drive, पूर्णाङ्क cmd, u32 addr
                    , शून्य *data, पूर्णाङ्क गणनम्)
{
    // Send command
    writew(&drive->regs->block_size, DISK_SECTOR_SIZE);
    writew(&drive->regs->block_गणनम्, गणनम्);
    पूर्णाङ्क isread = cmd != SC_WRITE_SINGLE && cmd != SC_WRITE_MULTIPLE;
    u16 tmode = ((गणनम् > 1 ? ST_MULTIPLE|ST_AUTO_CMD12|ST_BLOCKCOUNT : 0)
                 | (isread ? ST_READ : 0));
    writew(&drive->regs->transfer_mode, tmode);
    जइ (!(drive->card_type & SF_HIGHCAPACITY))
        addr *= DISK_SECTOR_SIZE;
    u32 param[4] = { addr };
    पूर्णाङ्क ret = sdcard_pio(drive->regs, cmd, param);
    जइ (ret)
        प्रत्यागम ret;
    // Read/write data
    u16 cbit = isread ? SI_READ_READY : SI_WRITE_READY;
    जाव (गणनम्--) {
        ret = sdcard_waitw(&drive->regs->irq_status, cbit);
        जइ (ret < 0)
            प्रत्यागम ret;
        writew(&drive->regs->irq_status, cbit);
        पूर्णाङ्क i;
        प्रति (i=0; i<DISK_SECTOR_SIZE/4; i++) {
            जइ (isread)
                *(u32*)data = readl(&drive->regs->data);
            अण्णहा
                writel(&drive->regs->data, *(u32*)data);
            data += 4;
        }
    }
    // Complete command
    ret = sdcard_waitw(&drive->regs->irq_status, SI_TRANS_DONE);
    जइ (ret < 0)
        प्रत्यागम ret;
    writew(&drive->regs->irq_status, SI_TRANS_DONE);
    प्रत्यागम 0;
}

// Read/write a block of data to/from the card.
स्थिर पूर्णाङ्क
sdcard_readwrite(संरचना disk_op_s *op, पूर्णाङ्क iswrite)
{
    संरचना sddrive_s *drive = container_of(
        op->drive_fl, संरचना sddrive_s, drive);
    पूर्णाङ्क cmd = iswrite ? SC_WRITE_SINGLE : SC_READ_SINGLE;
    जइ (op->गणनम् > 1)
        cmd = iswrite ? SC_WRITE_MULTIPLE : SC_READ_MULTIPLE;
    पूर्णाङ्क ret = sdcard_pio_transfer(drive, cmd, op->lba, op->buf_fl, op->गणनम्);
    जइ (ret)
        प्रत्यागम DISK_RET_EBADTRACK;
    प्रत्यागम DISK_RET_SUCCESS;
}

पूर्णाङ्क
sdcard_process_op(संरचना disk_op_s *op)
{
    जइ (!CONFIG_SDCARD)
        प्रत्यागम 0;
    विकल्प (op->command) {
    पक्ष CMD_READ:
        प्रत्यागम sdcard_readwrite(op, 0);
    पक्ष CMD_WRITE:
        प्रत्यागम sdcard_readwrite(op, 1);
    स्वभाव:
        प्रत्यागम default_process_op(op);
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

स्थिर पूर्णाङ्क
sdcard_set_power(संरचना sdhci_s *regs)
{
    u32 cap = readl(&regs->cap_lo);
    u32 volt, vbits;
    जइ (cap & SD_CAPLO_V33) {
        volt = 1<<20;
        vbits = SPC_V33;
    } अण्णहा जइ (cap & SD_CAPLO_V30) {
        volt = 1<<18;
        vbits = SPC_V30;
    } अण्णहा जइ (cap & SD_CAPLO_V18) {
        volt = 1<<7;
        vbits = SPC_V18;
    } अण्णहा {
        dprintf(1, "SD controller unsupported volt range (%x)\n", cap);
        प्रत्यागम -1;
    }
    writeb(&regs->power_control, 0);
    msleep(SDHCI_POWER_OFF_TIME);
    writeb(&regs->power_control, vbits | SPC_POWER_ON);
    msleep(SDHCI_POWER_ON_TIME);
    प्रत्यागम volt;
}

स्थिर पूर्णाङ्क
sdcard_set_frequency(संरचना sdhci_s *regs, u32 khz)
{
    u16 ver = readw(&regs->controller_version);
    u32 cap = readl(&regs->cap_lo);
    u32 base_freq = (cap >> SD_CAPLO_BASECLOCK_SHIFT) & SD_CAPLO_BASECLOCK_MASK;
    जइ (!base_freq) {
        dprintf(1, "Unknown base frequency for SD controller\n");
        प्रत्यागम -1;
    }
    // Set new frequency
    u32 भाजकः = DIV_ROUND_UP(base_freq * 1000, khz);
    u16 creg;
    जइ ((ver & 0xff) <= 0x01) {
        भाजकः = भाजकः > 1 ? 1 << __fls(भाजकः-1) : 0;
        creg = (भाजकः & SCC_SDCLK_MASK) << SCC_SDCLK_SHIFT;
    } अण्णहा {
        भाजकः = DIV_ROUND_UP(भाजकः, 2);
        creg = (भाजकः & SCC_SDCLK_MASK) << SCC_SDCLK_SHIFT;
        creg |= (भाजकः & SCC_SDCLK_HI_MASK) >> SCC_SDCLK_HI_RSHIFT;
    }
    dprintf(3, "sdcard_set_frequency %d %d %x\n", base_freq, khz, creg);
    writew(&regs->clock_control, 0);
    writew(&regs->clock_control, creg | SCC_INTERNAL_ENABLE);
    // Wait for frequency to become active
    पूर्णाङ्क ret = sdcard_waitw(&regs->clock_control, SCC_STABLE);
    जइ (ret < 0)
        प्रत्यागम ret;
    // Enable SD clock
    writew(&regs->clock_control, creg | SCC_INTERNAL_ENABLE | SCC_CLOCK_ENABLE);
    प्रत्यागम 0;
}

// Obtain the disk size of an SD card
स्थिर पूर्णाङ्क
sdcard_get_capacity(संरचना sddrive_s *drive, u8 *csd)
{
    // Original MMC/SD card capacity formula
    u16 C_SIZE = (csd[6] >> 6) | (csd[7] << 2) | ((csd[8] & 0x03) << 10);
    u8 C_SIZE_MULT = (csd[4] >> 7) | ((csd[5] & 0x03) << 1);
    u8 READ_BL_LEN = csd[9] & 0x0f;
    u32 गणनम् = (C_SIZE+1) << (C_SIZE_MULT + 2 + READ_BL_LEN - 9);
    // Check for newer encoding formats.
    u8 CSD_STRUCTURE = csd[14] >> 6;
    जइ ((drive->card_type & SF_MMC) && CSD_STRUCTURE >= 2) {
        // Get capacity from EXT_CSD register
        u8 ext_csd[512];
        पूर्णाङ्क ret = sdcard_pio_transfer(drive, SC_SEND_EXT_CSD, 0, ext_csd, 1);
        जइ (ret)
            प्रत्यागम ret;
        गणनम् = *(u32*)&ext_csd[212];
    } अण्णहा जइ (!(drive->card_type & SF_MMC) && CSD_STRUCTURE >= 1) {
        // High capacity SD card
        u32 C_SIZE2 = csd[5] | (csd[6] << 8) | ((csd[7] & 0x3f) << 16);
        गणनम् = (C_SIZE2+1) << (19-9);
    }
    // Fill drive struct and return
    drive->drive.blksize = DISK_SECTOR_SIZE;
    drive->drive.sectors = गणनम्;
    प्रत्यागम 0;
}

// Initialize an SD card
स्थिर पूर्णाङ्क
sdcard_card_setup(संरचना sddrive_s *drive, पूर्णाङ्क volt, पूर्णाङ्क prio)
{
    संरचना sdhci_s *regs = drive->regs;
    // Set controller to initialization clock rate
    पूर्णाङ्क ret = sdcard_set_frequency(regs, 400);
    जइ (ret)
        प्रत्यागम ret;
    msleep(SDHCI_CLOCK_ON_TIME);
    // Reset card
    u32 param[4] = { };
    ret = sdcard_pio(regs, SC_GO_IDLE_STATE, param);
    जइ (ret)
        प्रत्यागम ret;
    // Let card know SDHC/SDXC is supported and confirm voltage
    u32 hcs = 0, vrange = (volt >= (1<<15) ? 0x100 : 0x200) | 0xaa;
    param[0] = vrange;
    ret = sdcard_pio(regs, SC_SEND_IF_COND, param);
    जइ (!ret && param[0] == vrange)
        hcs = (1<<30);
    // Verify SD card (instead of MMC or SDIO)
    param[0] = 0x00;
    ret = sdcard_pio_app(regs, SC_APP_SEND_OP_COND, param);
    जइ (ret) {
        // Check for MMC card
        param[0] = 0x00;
        ret = sdcard_pio(regs, SC_SEND_OP_COND, param);
        जइ (ret)
            प्रत्यागम ret;
        drive->card_type |= SF_MMC;
        hcs = (1<<30);
    }
    // Init card
    u32 end = timer_calc(SDHCI_POWERUP_TIMEOUT);
    प्रति (;;) {
        param[0] = hcs | volt; // high-capacity support and voltage level
        जइ (drive->card_type & SF_MMC)
            ret = sdcard_pio(regs, SC_SEND_OP_COND, param);
        अण्णहा
            ret = sdcard_pio_app(regs, SC_APP_SEND_OP_COND, param);
        जइ (ret)
            प्रत्यागम ret;
        जइ (param[0] & SR_OCR_NOTBUSY)
            भङ्ग;
        जइ (timer_check(end)) {
            warn_timeout();
            प्रत्यागम -1;
        }
        msleep(5); // Avoid flooding log when debugging
    }
    drive->card_type |= (param[0] & SR_OCR_CCS) ? SF_HIGHCAPACITY : 0;
    // Select card (get cid, set rca, get csd, select card)
    param[0] = 0x00;
    ret = sdcard_pio(regs, SC_ALL_SEND_CID, param);
    जइ (ret)
        प्रत्यागम ret;
    u8 cid[16];
    memcpy(cid, param, परिमाण(cid));
    param[0] = drive->card_type & SF_MMC ? 0x0001 << 16 : 0x00;
    ret = sdcard_pio(regs, SC_SEND_RELATIVE_ADDR, param);
    जइ (ret)
        प्रत्यागम ret;
    u16 rca = drive->card_type & SF_MMC ? 0x0001 : param[0] >> 16;
    param[0] = rca << 16;
    ret = sdcard_pio(regs, SC_SEND_CSD, param);
    जइ (ret)
        प्रत्यागम ret;
    u8 csd[16];
    memcpy(csd, param, परिमाण(csd));
    param[0] = rca << 16;
    ret = sdcard_pio(regs, SC_SELECT_DESELECT_CARD, param);
    जइ (ret)
        प्रत्यागम ret;
    // Set controller to data transfer clock rate
    ret = sdcard_set_frequency(regs, 25000);
    जइ (ret)
        प्रत्यागम ret;
    // Register drive
    ret = sdcard_get_capacity(drive, csd);
    जइ (ret)
        प्रत्यागम ret;
    वर्ण pnm[7] = {};
    पूर्णाङ्क i;
    प्रति (i=0; i < (drive->card_type & SF_MMC ? 6 : 5); i++)
        pnm[i] = cid[11-i];
    वर्ण *desc = znprintf(MAXDESCSIZE, "%s %s %dMiB"
                          , drive->card_type & SF_MMC ? "MMC drive" : "SD card"
                          , pnm, (u32)(drive->drive.sectors >> 11));
    dprintf(1, "Found sdcard at %p: %s\n", regs, desc);
    boot_add_hd(&drive->drive, desc, prio);
    प्रत्यागम 0;
}

// Setup and configure an SD card controller
स्थिर शून्य
sdcard_controller_setup(संरचना sdhci_s *regs, पूर्णाङ्क prio)
{
    // Initialize controller
    u32 present_state = readl(&regs->present_state);
    जइ (!(present_state & SP_CARD_INSERTED))
        // No card present
        प्रत्यागम;
    dprintf(3, "sdhci@%p ver=%x cap=%x %x\n", regs
            , readw(&regs->controller_version)
            , readl(&regs->cap_lo), readl(&regs->cap_hi));
    sdcard_reset(regs, SRF_ALL);
    writew(&regs->irq_signal, 0);
    writew(&regs->irq_enable, 0x01ff);
    writew(&regs->irq_status, readw(&regs->irq_status));
    writew(&regs->error_signal, 0);
    writew(&regs->error_irq_enable, 0x01ff);
    writew(&regs->error_irq_status, readw(&regs->error_irq_status));
    writeb(&regs->timeout_control, 0x0e); // Set to max timeout
    पूर्णाङ्क volt = sdcard_set_power(regs);
    जइ (volt < 0)
        प्रत्यागम;

    // Initialize card
    संरचना sddrive_s *drive = malloc_fseg(परिमाण(*drive));
    जइ (!drive) {
        warn_noalloc();
        गच्छ fail;
    }
    memset(drive, 0, परिमाण(*drive));
    drive->drive.type = DTYPE_SDCARD;
    drive->regs = regs;
    पूर्णाङ्क ret = sdcard_card_setup(drive, volt, prio);
    जइ (ret) {
        मुक्त(drive);
        गच्छ fail;
    }
    प्रत्यागम;
fail:
    writeb(&regs->power_control, 0);
    writew(&regs->clock_control, 0);
}

स्थिर शून्य
sdcard_pci_setup(शून्य *data)
{
    संरचना pci_device *pci = data;
    // XXX - bars dependent on slot index register in pci config space
    संरचना sdhci_s *regs = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    जइ (!regs)
        प्रत्यागम;
    पूर्णाङ्क prio = bootprio_find_pci_device(pci);
    sdcard_controller_setup(regs, prio);
}

स्थिर शून्य
sdcard_romfile_setup(शून्य *data)
{
    संरचना romfile_s *file = data;
    पूर्णाङ्क prio = bootprio_find_named_rom(file->name, 0);
    u32 addr = romfile_loadint(file->name, 0);
    dprintf(1, "Starting sdcard controller check at addr %x\n", addr);
    sdcard_controller_setup((शून्य*)addr, prio);
}

शून्य
sdcard_setup(शून्य)
{
    जइ (!CONFIG_SDCARD)
        प्रत्यागम;

    संरचना romfile_s *file = रिक्त;
    पूर्णाङ्क अङ्कः_romfiles = 0;
    प्रति (;;) {
        file = romfile_findprefix("etc/sdcard", file);
        जइ (!file)
            भङ्ग;
        run_thread(sdcard_romfile_setup, file);
        अङ्कः_romfiles++;
    }
    जइ (अङ्कः_romfiles)
        // only scan for PCI controllers if etc/sdcard not used
        प्रत्यागम;

    संरचना pci_device *pci;
    foreachpci(pci) {
        जइ (pci->class != PCI_CLASS_SYSTEM_SDHCI || pci->prog_if >= 2)
            // Not an SDHCI controller following SDHCI spec
            अनुवर्त;
        run_thread(sdcard_pci_setup, pci);
    }
}

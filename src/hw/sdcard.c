// PCI SD Host Controller Interface
//
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "block.h" // struct drive_s
#అన్తర్భావ "malloc.h" // malloc_fseg
#అన్తర్భావ "output.h" // znprintf
#అన్తర్భావ "pcidevice.h" // foreachpci
#అన్తర్భావ "pci_ids.h" // PCI_CLASS_SYSTEM_SDHCI
#అన్తర్భావ "pci_regs.h" // PCI_BASE_ADDRESS_0
#అన్తర్భావ "romfile.h" // romfile_findprefix
#అన్తర్భావ "stacks.h" // yield
#అన్తర్భావ "std/disk.h" // DISK_RET_SUCCESS
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // boot_add_hd
#అన్తర్భావ "x86.h" // writel

// SDHCI MMIO registers
నిర్మాణం sdhci_s {
    u32 sdma_addr;
    u16 ఖండం_కొలత;
    u16 ఖండం_లెక్క;
    u32 arg;
    u16 transfer_బహుళకం;
    u16 cmd;
    u32 response[4];
    u32 దత్తాంశం;
    u32 present_state;
    u8 host_control;
    u8 power_control;
    u8 ఖండం_gap_control;
    u8 wakeup_control;
    u16 clock_control;
    u8 timeout_control;
    u8 software_reset;
    u16 irq_స్థితి;
    u16 error_irq_స్థితి;
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
    u16 స్థానం_irq;
    u16 controller_సంస్కరణ;
} PACKED;

// SDHCI commands
#నిర్వచించు SCB_R0   0x00 // No response
#నిర్వచించు SCB_R48  0x1a // Response R1 (no data), R5, R6, R7
#నిర్వచించు SCB_R48d 0x3a // Response R1 (with data)
#నిర్వచించు SCB_R48b 0x1b // Response R1b, R5b
#నిర్వచించు SCB_R48o 0x02 // Response R3, R4
#నిర్వచించు SCB_R136 0x09 // Response R2
#నిర్వచించు SC_GO_IDLE_STATE        ((0<<8) | SCB_R0)
#నిర్వచించు SC_SEND_OP_COND         ((1<<8) | SCB_R48o)
#నిర్వచించు SC_ALL_SEND_CID         ((2<<8) | SCB_R136)
#నిర్వచించు SC_SEND_RELATIVE_ADDR   ((3<<8) | SCB_R48)
#నిర్వచించు SC_SELECT_DESELECT_CARD ((7<<8) | SCB_R48b)
#నిర్వచించు SC_SEND_IF_COND         ((8<<8) | SCB_R48)
#నిర్వచించు SC_SEND_EXT_CSD         ((8<<8) | SCB_R48d)
#నిర్వచించు SC_SEND_CSD             ((9<<8) | SCB_R136)
#నిర్వచించు SC_READ_SINGLE          ((17<<8) | SCB_R48d)
#నిర్వచించు SC_READ_గుణిజం        ((18<<8) | SCB_R48d)
#నిర్వచించు SC_WRITE_SINGLE         ((24<<8) | SCB_R48d)
#నిర్వచించు SC_WRITE_గుణిజం       ((25<<8) | SCB_R48d)
#నిర్వచించు SC_APP_CMD              ((55<<8) | SCB_R48)
#నిర్వచించు SC_APP_SEND_OP_COND ((41<<8) | SCB_R48o)

// SDHCI irqs
#నిర్వచించు SI_CMD_COMPLETE (1<<0)
#నిర్వచించు SI_TRANS_పూర్తి   (1<<1)
#నిర్వచించు SI_WRITE_READY  (1<<4)
#నిర్వచించు SI_READ_READY   (1<<5)
#నిర్వచించు SI_ERROR        (1<<15)

// SDHCI present_state flags
#నిర్వచించు SP_CMD_INHIBIT   (1<<0)
#నిర్వచించు SP_DAT_INHIBIT   (1<<1)
#నిర్వచించు SP_CARD_INSERTED (1<<16)

// SDHCI transfer_mode flags
#నిర్వచించు ST_BLOCKCOUNT (1<<1)
#నిర్వచించు ST_AUTO_CMD12 (1<<2)
#నిర్వచించు ST_READ       (1<<4)
#నిర్వచించు ST_గుణిజం   (1<<5)

// SDHCI capabilities flags
#నిర్వచించు SD_CAPLO_V33             (1<<24)
#నిర్వచించు SD_CAPLO_V30             (1<<25)
#నిర్వచించు SD_CAPLO_V18             (1<<26)
#నిర్వచించు SD_CAPLO_BASECLOCK_జరుపు 8
#నిర్వచించు SD_CAPLO_BASECLOCK_ముసుగు  0xff

// SDHCI clock control flags
#నిర్వచించు SCC_అంతర్గత_ENABLE (1<<0)
#నిర్వచించు SCC_STABLE          (1<<1)
#నిర్వచించు SCC_CLOCK_ENABLE    (1<<2)
#నిర్వచించు SCC_SDCLK_ముసుగు      0xff
#నిర్వచించు SCC_SDCLK_జరుపు     8
#నిర్వచించు SCC_SDCLK_HI_ముసుగు   0x300
#నిర్వచించు SCC_SDCLK_HI_RSHIFT 2

// SDHCI power control flags
#నిర్వచించు SPC_POWER_ON (1<<0)
#నిర్వచించు SPC_V18      0x0a
#నిర్వచించు SPC_V30      0x0c
#నిర్వచించు SPC_V33      0x0e

// SDHCI software reset flags
#నిర్వచించు SRF_ALL  0x01
#నిర్వచించు SRF_CMD  0x02
#నిర్వచించు SRF_దత్తాంశం 0x04

// SDHCI result flags
#నిర్వచించు SR_OCR_CCS     (1<<30)
#నిర్వచించు SR_OCR_NOTBUSY (1<<31)

// SDHCI timeouts
#నిర్వచించు SDHCI_POWER_OFF_సమయం   1
#నిర్వచించు SDHCI_POWER_ON_సమయం    5
#నిర్వచించు SDHCI_CLOCK_ON_సమయం    1 // 74 clock cycles
#నిర్వచించు SDHCI_POWERUP_TIMEOUT  1000
#నిర్వచించు SDHCI_PIO_TIMEOUT      1000  // XXX - this is just made up

// Internal 'struct drive_s' storage for a detected card
నిర్మాణం sddrive_s {
    నిర్మాణం drive_s drive;
    నిర్మాణం sdhci_s *regs;
    పూర్ణాంకం card_type;
};

// SD card types
#నిర్వచించు SF_MMC          (1<<0)
#నిర్వచించు SF_HIGHCAPACITY (1<<1)

// Repeatedly read a u16 register until any bit in a given mask is set
స్థిర పూర్ణాంకం
sdcard_waitw(u16 *reg, u16 ముసుగు)
{
    u32 చివర = timer_calc(SDHCI_PIO_TIMEOUT);
    కొరకు (;;) {
        u16 v = readw(reg);
        ఒకవేళ (v & ముసుగు)
            తిరిగి v;
        ఒకవేళ (timer_check(చివర)) {
            dprintf(1, "scard_waitw: %p %x %x\n", reg, ముసుగు, v);
            warn_timeout();
            తిరిగి -1;
        }
        yield();
    }
}

// Send an sdhci reset
స్థిర పూర్ణాంకం
sdcard_reset(నిర్మాణం sdhci_s *regs, పూర్ణాంకం flags)
{
    writeb(&regs->software_reset, flags);
    u32 చివర = timer_calc(SDHCI_PIO_TIMEOUT);
    అయితే (readb(&regs->software_reset))
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            తిరిగి -1;
        }
    తిరిగి 0;
}

// Send a command to the card.
స్థిర పూర్ణాంకం
sdcard_pio(నిర్మాణం sdhci_s *regs, పూర్ణాంకం cmd, u32 *param)
{
    u32 state = readl(&regs->present_state);
    dprintf(9, "sdcard_pio cmd %x %x %x\n", cmd, *param, state);
    ఒకవేళ ((state & SP_CMD_INHIBIT)
        || ((cmd & 0x03) == 0x03 && state & SP_DAT_INHIBIT)) {
        dprintf(1, "sdcard_pio not ready %x\n", state);
        తిరిగి -1;
    }
    // Send command
    writel(&regs->arg, *param);
    writew(&regs->cmd, cmd);
    పూర్ణాంకం ret = sdcard_waitw(&regs->irq_స్థితి, SI_ERROR|SI_CMD_COMPLETE);
    ఒకవేళ (ret < 0)
        తిరిగి ret;
    ఒకవేళ (ret & SI_ERROR) {
        u16 err = readw(&regs->error_irq_స్థితి);
        dprintf(3, "sdcard_pio command stop (code=%x)\n", err);
        sdcard_reset(regs, SRF_CMD|SRF_దత్తాంశం);
        writew(&regs->error_irq_స్థితి, err);
        తిరిగి -1;
    }
    writew(&regs->irq_స్థితి, SI_CMD_COMPLETE);
    // Read response
    జ్ఞాపకం_నకలు(param, regs->response, పరిమాణం(regs->response));
    dprintf(9, "sdcard cmd %x response %x %x %x %x\n"
            , cmd, param[0], param[1], param[2], param[3]);
    తిరిగి 0;
}

// Send an "app specific" command to the card.
స్థిర పూర్ణాంకం
sdcard_pio_app(నిర్మాణం sdhci_s *regs, పూర్ణాంకం cmd, u32 *param)
{
    u32 aparam[4] = {};
    పూర్ణాంకం ret = sdcard_pio(regs, SC_APP_CMD, aparam);
    ఒకవేళ (ret)
        తిరిగి ret;
    తిరిగి sdcard_pio(regs, cmd, param);
}

// Send a command to the card which transfers data.
స్థిర పూర్ణాంకం
sdcard_pio_transfer(నిర్మాణం sddrive_s *drive, పూర్ణాంకం cmd, u32 addr
                    , శూన్య *దత్తాంశం, పూర్ణాంకం లెక్క)
{
    // Send command
    writew(&drive->regs->ఖండం_కొలత, DISK_SECTOR_కొలత);
    writew(&drive->regs->ఖండం_లెక్క, లెక్క);
    పూర్ణాంకం isread = cmd != SC_WRITE_SINGLE && cmd != SC_WRITE_గుణిజం;
    u16 tmode = ((లెక్క > 1 ? ST_గుణిజం|ST_AUTO_CMD12|ST_BLOCKCOUNT : 0)
                 | (isread ? ST_READ : 0));
    writew(&drive->regs->transfer_బహుళకం, tmode);
    ఒకవేళ (!(drive->card_type & SF_HIGHCAPACITY))
        addr *= DISK_SECTOR_కొలత;
    u32 param[4] = { addr };
    పూర్ణాంకం ret = sdcard_pio(drive->regs, cmd, param);
    ఒకవేళ (ret)
        తిరిగి ret;
    // Read/write data
    u16 cbit = isread ? SI_READ_READY : SI_WRITE_READY;
    అయితే (లెక్క--) {
        ret = sdcard_waitw(&drive->regs->irq_స్థితి, cbit);
        ఒకవేళ (ret < 0)
            తిరిగి ret;
        writew(&drive->regs->irq_స్థితి, cbit);
        పూర్ణాంకం i;
        కొరకు (i=0; i<DISK_SECTOR_కొలత/4; i++) {
            ఒకవేళ (isread)
                *(u32*)దత్తాంశం = readl(&drive->regs->దత్తాంశం);
            లేకపోతే
                writel(&drive->regs->దత్తాంశం, *(u32*)దత్తాంశం);
            దత్తాంశం += 4;
        }
    }
    // Complete command
    ret = sdcard_waitw(&drive->regs->irq_స్థితి, SI_TRANS_పూర్తి);
    ఒకవేళ (ret < 0)
        తిరిగి ret;
    writew(&drive->regs->irq_స్థితి, SI_TRANS_పూర్తి);
    తిరిగి 0;
}

// Read/write a block of data to/from the card.
స్థిర పూర్ణాంకం
sdcard_readwrite(నిర్మాణం disk_op_s *op, పూర్ణాంకం iswrite)
{
    నిర్మాణం sddrive_s *drive = container_యొక్క(
        op->drive_fl, నిర్మాణం sddrive_s, drive);
    పూర్ణాంకం cmd = iswrite ? SC_WRITE_SINGLE : SC_READ_SINGLE;
    ఒకవేళ (op->లెక్క > 1)
        cmd = iswrite ? SC_WRITE_గుణిజం : SC_READ_గుణిజం;
    పూర్ణాంకం ret = sdcard_pio_transfer(drive, cmd, op->lba, op->buf_fl, op->లెక్క);
    ఒకవేళ (ret)
        తిరిగి DISK_RET_EBADTRACK;
    తిరిగి DISK_RET_SUCCESS;
}

పూర్ణాంకం
sdcard_process_op(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (!CONFIG_SDCARD)
        తిరిగి 0;
    ఎంపిక (op->command) {
    స్థితి CMD_READ:
        తిరిగి sdcard_readwrite(op, 0);
    స్థితి CMD_WRITE:
        తిరిగి sdcard_readwrite(op, 1);
    అప్రమేయ:
        తిరిగి default_process_op(op);
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

స్థిర పూర్ణాంకం
sdcard_సమితి_power(నిర్మాణం sdhci_s *regs)
{
    u32 cap = readl(&regs->cap_lo);
    u32 volt, vbits;
    ఒకవేళ (cap & SD_CAPLO_V33) {
        volt = 1<<20;
        vbits = SPC_V33;
    } లేకపోతే ఒకవేళ (cap & SD_CAPLO_V30) {
        volt = 1<<18;
        vbits = SPC_V30;
    } లేకపోతే ఒకవేళ (cap & SD_CAPLO_V18) {
        volt = 1<<7;
        vbits = SPC_V18;
    } లేకపోతే {
        dprintf(1, "SD controller unsupported volt range (%x)\n", cap);
        తిరిగి -1;
    }
    writeb(&regs->power_control, 0);
    msleep(SDHCI_POWER_OFF_సమయం);
    writeb(&regs->power_control, vbits | SPC_POWER_ON);
    msleep(SDHCI_POWER_ON_సమయం);
    తిరిగి volt;
}

స్థిర పూర్ణాంకం
sdcard_సమితి_frequency(నిర్మాణం sdhci_s *regs, u32 khz)
{
    u16 ver = readw(&regs->controller_సంస్కరణ);
    u32 cap = readl(&regs->cap_lo);
    u32 పునాది_freq = (cap >> SD_CAPLO_BASECLOCK_జరుపు) & SD_CAPLO_BASECLOCK_ముసుగు;
    ఒకవేళ (!పునాది_freq) {
        dprintf(1, "Unknown base frequency for SD controller\n");
        తిరిగి -1;
    }
    // Set new frequency
    u32 భాజకం = DIV_విడత_పైకి(పునాది_freq * 1000, khz);
    u16 creg;
    ఒకవేళ ((ver & 0xff) <= 0x01) {
        భాజకం = భాజకం > 1 ? 1 << __fls(భాజకం-1) : 0;
        creg = (భాజకం & SCC_SDCLK_ముసుగు) << SCC_SDCLK_జరుపు;
    } లేకపోతే {
        భాజకం = DIV_విడత_పైకి(భాజకం, 2);
        creg = (భాజకం & SCC_SDCLK_ముసుగు) << SCC_SDCLK_జరుపు;
        creg |= (భాజకం & SCC_SDCLK_HI_ముసుగు) >> SCC_SDCLK_HI_RSHIFT;
    }
    dprintf(3, "sdcard_set_frequency %d %d %x\n", పునాది_freq, khz, creg);
    writew(&regs->clock_control, 0);
    writew(&regs->clock_control, creg | SCC_అంతర్గత_ENABLE);
    // Wait for frequency to become active
    పూర్ణాంకం ret = sdcard_waitw(&regs->clock_control, SCC_STABLE);
    ఒకవేళ (ret < 0)
        తిరిగి ret;
    // Enable SD clock
    writew(&regs->clock_control, creg | SCC_అంతర్గత_ENABLE | SCC_CLOCK_ENABLE);
    తిరిగి 0;
}

// Obtain the disk size of an SD card
స్థిర పూర్ణాంకం
sdcard_get_సామర్థ్యం(నిర్మాణం sddrive_s *drive, u8 *csd)
{
    // Original MMC/SD card capacity formula
    u16 C_కొలత = (csd[6] >> 6) | (csd[7] << 2) | ((csd[8] & 0x03) << 10);
    u8 C_కొలత_MULT = (csd[4] >> 7) | ((csd[5] & 0x03) << 1);
    u8 READ_BL_LEN = csd[9] & 0x0f;
    u32 లెక్క = (C_కొలత+1) << (C_కొలత_MULT + 2 + READ_BL_LEN - 9);
    // Check for newer encoding formats.
    u8 CSD_STRUCTURE = csd[14] >> 6;
    ఒకవేళ ((drive->card_type & SF_MMC) && CSD_STRUCTURE >= 2) {
        // Get capacity from EXT_CSD register
        u8 ext_csd[512];
        పూర్ణాంకం ret = sdcard_pio_transfer(drive, SC_SEND_EXT_CSD, 0, ext_csd, 1);
        ఒకవేళ (ret)
            తిరిగి ret;
        లెక్క = *(u32*)&ext_csd[212];
    } లేకపోతే ఒకవేళ (!(drive->card_type & SF_MMC) && CSD_STRUCTURE >= 1) {
        // High capacity SD card
        u32 C_SIZE2 = csd[5] | (csd[6] << 8) | ((csd[7] & 0x3f) << 16);
        లెక్క = (C_SIZE2+1) << (19-9);
    }
    // Fill drive struct and return
    drive->drive.blksize = DISK_SECTOR_కొలత;
    drive->drive.sectors = లెక్క;
    తిరిగి 0;
}

// Initialize an SD card
స్థిర పూర్ణాంకం
sdcard_card_అమరిక(నిర్మాణం sddrive_s *drive, పూర్ణాంకం volt, పూర్ణాంకం prio)
{
    నిర్మాణం sdhci_s *regs = drive->regs;
    // Set controller to initialization clock rate
    పూర్ణాంకం ret = sdcard_సమితి_frequency(regs, 400);
    ఒకవేళ (ret)
        తిరిగి ret;
    msleep(SDHCI_CLOCK_ON_సమయం);
    // Reset card
    u32 param[4] = { };
    ret = sdcard_pio(regs, SC_GO_IDLE_STATE, param);
    ఒకవేళ (ret)
        తిరిగి ret;
    // Let card know SDHC/SDXC is supported and confirm voltage
    u32 hcs = 0, vrange = (volt >= (1<<15) ? 0x100 : 0x200) | 0xaa;
    param[0] = vrange;
    ret = sdcard_pio(regs, SC_SEND_IF_COND, param);
    ఒకవేళ (!ret && param[0] == vrange)
        hcs = (1<<30);
    // Verify SD card (instead of MMC or SDIO)
    param[0] = 0x00;
    ret = sdcard_pio_app(regs, SC_APP_SEND_OP_COND, param);
    ఒకవేళ (ret) {
        // Check for MMC card
        param[0] = 0x00;
        ret = sdcard_pio(regs, SC_SEND_OP_COND, param);
        ఒకవేళ (ret)
            తిరిగి ret;
        drive->card_type |= SF_MMC;
        hcs = (1<<30);
    }
    // Init card
    u32 చివర = timer_calc(SDHCI_POWERUP_TIMEOUT);
    కొరకు (;;) {
        param[0] = hcs | volt; // high-capacity support and voltage level
        ఒకవేళ (drive->card_type & SF_MMC)
            ret = sdcard_pio(regs, SC_SEND_OP_COND, param);
        లేకపోతే
            ret = sdcard_pio_app(regs, SC_APP_SEND_OP_COND, param);
        ఒకవేళ (ret)
            తిరిగి ret;
        ఒకవేళ (param[0] & SR_OCR_NOTBUSY)
            ఆపు;
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            తిరిగి -1;
        }
        msleep(5); // Avoid flooding log when debugging
    }
    drive->card_type |= (param[0] & SR_OCR_CCS) ? SF_HIGHCAPACITY : 0;
    // Select card (get cid, set rca, get csd, select card)
    param[0] = 0x00;
    ret = sdcard_pio(regs, SC_ALL_SEND_CID, param);
    ఒకవేళ (ret)
        తిరిగి ret;
    u8 cid[16];
    జ్ఞాపకం_నకలు(cid, param, పరిమాణం(cid));
    param[0] = drive->card_type & SF_MMC ? 0x0001 << 16 : 0x00;
    ret = sdcard_pio(regs, SC_SEND_RELATIVE_ADDR, param);
    ఒకవేళ (ret)
        తిరిగి ret;
    u16 rca = drive->card_type & SF_MMC ? 0x0001 : param[0] >> 16;
    param[0] = rca << 16;
    ret = sdcard_pio(regs, SC_SEND_CSD, param);
    ఒకవేళ (ret)
        తిరిగి ret;
    u8 csd[16];
    జ్ఞాపకం_నకలు(csd, param, పరిమాణం(csd));
    param[0] = rca << 16;
    ret = sdcard_pio(regs, SC_SELECT_DESELECT_CARD, param);
    ఒకవేళ (ret)
        తిరిగి ret;
    // Set controller to data transfer clock rate
    ret = sdcard_సమితి_frequency(regs, 25000);
    ఒకవేళ (ret)
        తిరిగి ret;
    // Register drive
    ret = sdcard_get_సామర్థ్యం(drive, csd);
    ఒకవేళ (ret)
        తిరిగి ret;
    అక్షరం pnm[7] = {};
    పూర్ణాంకం i;
    కొరకు (i=0; i < (drive->card_type & SF_MMC ? 6 : 5); i++)
        pnm[i] = cid[11-i];
    అక్షరం *desc = znprintf(MAXDESCSIZE, "%s %s %dMiB"
                          , drive->card_type & SF_MMC ? "MMC drive" : "SD card"
                          , pnm, (u32)(drive->drive.sectors >> 11));
    dprintf(1, "Found sdcard at %p: %s\n", regs, desc);
    boot_కూడు_hd(&drive->drive, desc, prio);
    తిరిగి 0;
}

// Setup and configure an SD card controller
స్థిర శూన్య
sdcard_controller_అమరిక(నిర్మాణం sdhci_s *regs, పూర్ణాంకం prio)
{
    // Initialize controller
    u32 present_state = readl(&regs->present_state);
    ఒకవేళ (!(present_state & SP_CARD_INSERTED))
        // No card present
        తిరిగి;
    dprintf(3, "sdhci@%p ver=%x cap=%x %x\n", regs
            , readw(&regs->controller_సంస్కరణ)
            , readl(&regs->cap_lo), readl(&regs->cap_hi));
    sdcard_reset(regs, SRF_ALL);
    writew(&regs->irq_signal, 0);
    writew(&regs->irq_enable, 0x01ff);
    writew(&regs->irq_స్థితి, readw(&regs->irq_స్థితి));
    writew(&regs->error_signal, 0);
    writew(&regs->error_irq_enable, 0x01ff);
    writew(&regs->error_irq_స్థితి, readw(&regs->error_irq_స్థితి));
    writeb(&regs->timeout_control, 0x0e); // Set to max timeout
    పూర్ణాంకం volt = sdcard_సమితి_power(regs);
    ఒకవేళ (volt < 0)
        తిరిగి;

    // Initialize card
    నిర్మాణం sddrive_s *drive = malloc_fseg(పరిమాణం(*drive));
    ఒకవేళ (!drive) {
        warn_noalloc();
        వెళ్ళు fail;
    }
    memset(drive, 0, పరిమాణం(*drive));
    drive->drive.type = DTYPE_SDCARD;
    drive->regs = regs;
    పూర్ణాంకం ret = sdcard_card_అమరిక(drive, volt, prio);
    ఒకవేళ (ret) {
        విడుదల(drive);
        వెళ్ళు fail;
    }
    తిరిగి;
fail:
    writeb(&regs->power_control, 0);
    writew(&regs->clock_control, 0);
}

స్థిర శూన్య
sdcard_pci_అమరిక(శూన్య *దత్తాంశం)
{
    నిర్మాణం pci_device *pci = దత్తాంశం;
    // XXX - bars dependent on slot index register in pci config space
    నిర్మాణం sdhci_s *regs = pci_enable_membar(pci, PCI_పునాది_చిరునామా_0);
    ఒకవేళ (!regs)
        తిరిగి;
    పూర్ణాంకం prio = bootprio_కనుగొను_pci_device(pci);
    sdcard_controller_అమరిక(regs, prio);
}

స్థిర శూన్య
sdcard_romfile_అమరిక(శూన్య *దత్తాంశం)
{
    నిర్మాణం romfile_s *file = దత్తాంశం;
    పూర్ణాంకం prio = bootprio_కనుగొను_named_rom(file->name, 0);
    u32 addr = romfile_loadint(file->name, 0);
    dprintf(1, "Starting sdcard controller check at addr %x\n", addr);
    sdcard_controller_అమరిక((శూన్య*)addr, prio);
}

శూన్య
sdcard_అమరిక(శూన్య)
{
    ఒకవేళ (!CONFIG_SDCARD)
        తిరిగి;

    నిర్మాణం romfile_s *file = ఖాళీ;
    పూర్ణాంకం అంకె_romfiles = 0;
    కొరకు (;;) {
        file = romfile_findprefix("etc/sdcard", file);
        ఒకవేళ (!file)
            ఆపు;
        నిరంతరం_thread(sdcard_romfile_అమరిక, file);
        అంకె_romfiles++;
    }
    ఒకవేళ (అంకె_romfiles)
        // only scan for PCI controllers if etc/sdcard not used
        తిరిగి;

    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (pci->class != PCI_CLASS_SYSTEM_SDHCI || pci->prog_if >= 2)
            // Not an SDHCI controller following SDHCI spec
            కొనసాగు;
        నిరంతరం_thread(sdcard_pci_అమరిక, pci);
    }
}

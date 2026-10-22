// PCI SD Host Controller Interface
//
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "block.h" // struct drive_s
#ضمن "malloc.h" // malloc_fseg
#ضمن "output.h" // znprintf
#ضمن "pcidevice.h" // foreachpci
#ضمن "pci_ids.h" // PCI_CLASS_SYSTEM_SDHCI
#ضمن "pci_regs.h" // PCI_BASE_ADDRESS_0
#ضمن "romfile.h" // romfile_findprefix
#ضمن "stacks.h" // yield
#ضمن "std/disk.h" // DISK_RET_SUCCESS
#ضمن "string.h" // memset
#ضمن "util.h" // boot_add_hd
#ضمن "x86.h" // writel

// SDHCI MMIO registers
بنية sdhci_s {
    u32 sdma_addr;
    u16 بلاک_ناپ;
    u16 بلاک_گنتی;
    u32 arg;
    u16 transfer_منوال;
    u16 cmd;
    u32 response[4];
    u32 انکڑے;
    u32 present_state;
    u8 host_control;
    u8 power_control;
    u8 بلاک_gap_control;
    u8 wakeup_control;
    u16 clock_control;
    u8 timeout_control;
    u8 software_reset;
    u16 irq_حالت;
    u16 error_irq_حالت;
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
    u16 تھاں_irq;
    u16 controller_نسخہ;
} PACKED;

// SDHCI commands
#تعریف_کرو SCB_R0   0x00 // No response
#تعریف_کرو SCB_R48  0x1a // Response R1 (no data), R5, R6, R7
#تعریف_کرو SCB_R48d 0x3a // Response R1 (with data)
#تعریف_کرو SCB_R48b 0x1b // Response R1b, R5b
#تعریف_کرو SCB_R48o 0x02 // Response R3, R4
#تعریف_کرو SCB_R136 0x09 // Response R2
#تعریف_کرو SC_GO_IDLE_STATE        ((0<<8) | SCB_R0)
#تعریف_کرو SC_SEND_OP_COND         ((1<<8) | SCB_R48o)
#تعریف_کرو SC_ALL_SEND_CID         ((2<<8) | SCB_R136)
#تعریف_کرو SC_SEND_RELATIVE_ADDR   ((3<<8) | SCB_R48)
#تعریف_کرو SC_SELECT_DESELECT_CARD ((7<<8) | SCB_R48b)
#تعریف_کرو SC_SEND_IF_COND         ((8<<8) | SCB_R48)
#تعریف_کرو SC_SEND_EXT_CSD         ((8<<8) | SCB_R48d)
#تعریف_کرو SC_SEND_CSD             ((9<<8) | SCB_R136)
#تعریف_کرو SC_READ_SINGLE          ((17<<8) | SCB_R48d)
#تعریف_کرو SC_READ_مضرب        ((18<<8) | SCB_R48d)
#تعریف_کرو SC_WRITE_SINGLE         ((24<<8) | SCB_R48d)
#تعریف_کرو SC_WRITE_مضرب       ((25<<8) | SCB_R48d)
#تعریف_کرو SC_APP_CMD              ((55<<8) | SCB_R48)
#تعریف_کرو SC_APP_SEND_OP_COND ((41<<8) | SCB_R48o)

// SDHCI irqs
#تعریف_کرو SI_CMD_COMPLETE (1<<0)
#تعریف_کرو SI_TRANS_پورا   (1<<1)
#تعریف_کرو SI_WRITE_READY  (1<<4)
#تعریف_کرو SI_READ_READY   (1<<5)
#تعریف_کرو SI_ERROR        (1<<15)

// SDHCI present_state flags
#تعریف_کرو SP_CMD_INHIBIT   (1<<0)
#تعریف_کرو SP_DAT_INHIBIT   (1<<1)
#تعریف_کرو SP_CARD_INSERTED (1<<16)

// SDHCI transfer_mode flags
#تعریف_کرو ST_BLOCKCOUNT (1<<1)
#تعریف_کرو ST_AUTO_CMD12 (1<<2)
#تعریف_کرو ST_READ       (1<<4)
#تعریف_کرو ST_مضرب   (1<<5)

// SDHCI capabilities flags
#تعریف_کرو SD_CAPLO_V33             (1<<24)
#تعریف_کرو SD_CAPLO_V30             (1<<25)
#تعریف_کرو SD_CAPLO_V18             (1<<26)
#تعریف_کرو SD_CAPLO_BASECLOCK_کھسکاؤ 8
#تعریف_کرو SD_CAPLO_BASECLOCK_نقاب  0xff

// SDHCI clock control flags
#تعریف_کرو SCC_اندرونی_ENABLE (1<<0)
#تعریف_کرو SCC_STABLE          (1<<1)
#تعریف_کرو SCC_CLOCK_ENABLE    (1<<2)
#تعریف_کرو SCC_SDCLK_نقاب      0xff
#تعریف_کرو SCC_SDCLK_کھسکاؤ     8
#تعریف_کرو SCC_SDCLK_HI_نقاب   0x300
#تعریف_کرو SCC_SDCLK_HI_RSHIFT 2

// SDHCI power control flags
#تعریف_کرو SPC_POWER_ON (1<<0)
#تعریف_کرو SPC_V18      0x0a
#تعریف_کرو SPC_V30      0x0c
#تعریف_کرو SPC_V33      0x0e

// SDHCI software reset flags
#تعریف_کرو SRF_ALL  0x01
#تعریف_کرو SRF_CMD  0x02
#تعریف_کرو SRF_انکڑے 0x04

// SDHCI result flags
#تعریف_کرو SR_OCR_CCS     (1<<30)
#تعریف_کرو SR_OCR_NOTBUSY (1<<31)

// SDHCI timeouts
#تعریف_کرو SDHCI_POWER_OFF_ویلا   1
#تعریف_کرو SDHCI_POWER_ON_ویلا    5
#تعریف_کرو SDHCI_CLOCK_ON_ویلا    1 // 74 clock cycles
#تعریف_کرو SDHCI_POWERUP_TIMEOUT  1000
#تعریف_کرو SDHCI_PIO_TIMEOUT      1000  // XXX - this is just made up

// Internal 'struct drive_s' storage for a detected card
بنية sddrive_s {
    بنية drive_s drive;
    بنية sdhci_s *regs;
    صحيح card_type;
};

// SD card types
#تعریف_کرو SF_MMC          (1<<0)
#تعریف_کرو SF_HIGHCAPACITY (1<<1)

// Repeatedly read a u16 register until any bit in a given mask is set
ساكن صحيح
sdcard_waitw(u16 *reg, u16 نقاب)
{
    u32 سرا = timer_calc(SDHCI_PIO_TIMEOUT);
    لكل (;;) {
        u16 v = readw(reg);
        إذا (v & نقاب)
            أرجع v;
        إذا (timer_check(سرا)) {
            dprintf(1, "scard_waitw: %p %x %x\n", reg, نقاب, v);
            warn_timeout();
            أرجع -1;
        }
        yield();
    }
}

// Send an sdhci reset
ساكن صحيح
sdcard_reset(بنية sdhci_s *regs, صحيح flags)
{
    writeb(&regs->software_reset, flags);
    u32 سرا = timer_calc(SDHCI_PIO_TIMEOUT);
    طالما (readb(&regs->software_reset))
        إذا (timer_check(سرا)) {
            warn_timeout();
            أرجع -1;
        }
    أرجع 0;
}

// Send a command to the card.
ساكن صحيح
sdcard_pio(بنية sdhci_s *regs, صحيح cmd, u32 *param)
{
    u32 state = readl(&regs->present_state);
    dprintf(9, "sdcard_pio cmd %x %x %x\n", cmd, *param, state);
    إذا ((state & SP_CMD_INHIBIT)
        || ((cmd & 0x03) == 0x03 && state & SP_DAT_INHIBIT)) {
        dprintf(1, "sdcard_pio not ready %x\n", state);
        أرجع -1;
    }
    // Send command
    writel(&regs->arg, *param);
    writew(&regs->cmd, cmd);
    صحيح ret = sdcard_waitw(&regs->irq_حالت, SI_ERROR|SI_CMD_COMPLETE);
    إذا (ret < 0)
        أرجع ret;
    إذا (ret & SI_ERROR) {
        u16 err = readw(&regs->error_irq_حالت);
        dprintf(3, "sdcard_pio command stop (code=%x)\n", err);
        sdcard_reset(regs, SRF_CMD|SRF_انکڑے);
        writew(&regs->error_irq_حالت, err);
        أرجع -1;
    }
    writew(&regs->irq_حالت, SI_CMD_COMPLETE);
    // Read response
    حافظہ_نقل(param, regs->response, حجم(regs->response));
    dprintf(9, "sdcard cmd %x response %x %x %x %x\n"
            , cmd, param[0], param[1], param[2], param[3]);
    أرجع 0;
}

// Send an "app specific" command to the card.
ساكن صحيح
sdcard_pio_app(بنية sdhci_s *regs, صحيح cmd, u32 *param)
{
    u32 aparam[4] = {};
    صحيح ret = sdcard_pio(regs, SC_APP_CMD, aparam);
    إذا (ret)
        أرجع ret;
    أرجع sdcard_pio(regs, cmd, param);
}

// Send a command to the card which transfers data.
ساكن صحيح
sdcard_pio_transfer(بنية sddrive_s *drive, صحيح cmd, u32 addr
                    , فراغ *انکڑے, صحيح گنتی)
{
    // Send command
    writew(&drive->regs->بلاک_ناپ, DISK_SECTOR_ناپ);
    writew(&drive->regs->بلاک_گنتی, گنتی);
    صحيح isread = cmd != SC_WRITE_SINGLE && cmd != SC_WRITE_مضرب;
    u16 tmode = ((گنتی > 1 ? ST_مضرب|ST_AUTO_CMD12|ST_BLOCKCOUNT : 0)
                 | (isread ? ST_READ : 0));
    writew(&drive->regs->transfer_منوال, tmode);
    إذا (!(drive->card_type & SF_HIGHCAPACITY))
        addr *= DISK_SECTOR_ناپ;
    u32 param[4] = { addr };
    صحيح ret = sdcard_pio(drive->regs, cmd, param);
    إذا (ret)
        أرجع ret;
    // Read/write data
    u16 cbit = isread ? SI_READ_READY : SI_WRITE_READY;
    طالما (گنتی--) {
        ret = sdcard_waitw(&drive->regs->irq_حالت, cbit);
        إذا (ret < 0)
            أرجع ret;
        writew(&drive->regs->irq_حالت, cbit);
        صحيح i;
        لكل (i=0; i<DISK_SECTOR_ناپ/4; i++) {
            إذا (isread)
                *(u32*)انکڑے = readl(&drive->regs->انکڑے);
            وإلا
                writel(&drive->regs->انکڑے, *(u32*)انکڑے);
            انکڑے += 4;
        }
    }
    // Complete command
    ret = sdcard_waitw(&drive->regs->irq_حالت, SI_TRANS_پورا);
    إذا (ret < 0)
        أرجع ret;
    writew(&drive->regs->irq_حالت, SI_TRANS_پورا);
    أرجع 0;
}

// Read/write a block of data to/from the card.
ساكن صحيح
sdcard_readwrite(بنية disk_op_s *op, صحيح iswrite)
{
    بنية sddrive_s *drive = container_دا(
        op->drive_fl, بنية sddrive_s, drive);
    صحيح cmd = iswrite ? SC_WRITE_SINGLE : SC_READ_SINGLE;
    إذا (op->گنتی > 1)
        cmd = iswrite ? SC_WRITE_مضرب : SC_READ_مضرب;
    صحيح ret = sdcard_pio_transfer(drive, cmd, op->lba, op->buf_fl, op->گنتی);
    إذا (ret)
        أرجع DISK_RET_EBADTRACK;
    أرجع DISK_RET_SUCCESS;
}

صحيح
sdcard_process_op(بنية disk_op_s *op)
{
    إذا (!CONFIG_SDCARD)
        أرجع 0;
    بدل (op->command) {
    حالة CMD_READ:
        أرجع sdcard_readwrite(op, 0);
    حالة CMD_WRITE:
        أرجع sdcard_readwrite(op, 1);
    افتراضي:
        أرجع default_process_op(op);
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

ساكن صحيح
sdcard_سیٹ_power(بنية sdhci_s *regs)
{
    u32 cap = readl(&regs->cap_lo);
    u32 volt, vbits;
    إذا (cap & SD_CAPLO_V33) {
        volt = 1<<20;
        vbits = SPC_V33;
    } وإلا إذا (cap & SD_CAPLO_V30) {
        volt = 1<<18;
        vbits = SPC_V30;
    } وإلا إذا (cap & SD_CAPLO_V18) {
        volt = 1<<7;
        vbits = SPC_V18;
    } وإلا {
        dprintf(1, "SD controller unsupported volt range (%x)\n", cap);
        أرجع -1;
    }
    writeb(&regs->power_control, 0);
    msleep(SDHCI_POWER_OFF_ویلا);
    writeb(&regs->power_control, vbits | SPC_POWER_ON);
    msleep(SDHCI_POWER_ON_ویلا);
    أرجع volt;
}

ساكن صحيح
sdcard_سیٹ_frequency(بنية sdhci_s *regs, u32 khz)
{
    u16 ver = readw(&regs->controller_نسخہ);
    u32 cap = readl(&regs->cap_lo);
    u32 ادھار_freq = (cap >> SD_CAPLO_BASECLOCK_کھسکاؤ) & SD_CAPLO_BASECLOCK_نقاب;
    إذا (!ادھار_freq) {
        dprintf(1, "Unknown base frequency for SD controller\n");
        أرجع -1;
    }
    // Set new frequency
    u32 مقسوم_علیہ = DIV_گیڑا_اتے(ادھار_freq * 1000, khz);
    u16 creg;
    إذا ((ver & 0xff) <= 0x01) {
        مقسوم_علیہ = مقسوم_علیہ > 1 ? 1 << __fls(مقسوم_علیہ-1) : 0;
        creg = (مقسوم_علیہ & SCC_SDCLK_نقاب) << SCC_SDCLK_کھسکاؤ;
    } وإلا {
        مقسوم_علیہ = DIV_گیڑا_اتے(مقسوم_علیہ, 2);
        creg = (مقسوم_علیہ & SCC_SDCLK_نقاب) << SCC_SDCLK_کھسکاؤ;
        creg |= (مقسوم_علیہ & SCC_SDCLK_HI_نقاب) >> SCC_SDCLK_HI_RSHIFT;
    }
    dprintf(3, "sdcard_set_frequency %d %d %x\n", ادھار_freq, khz, creg);
    writew(&regs->clock_control, 0);
    writew(&regs->clock_control, creg | SCC_اندرونی_ENABLE);
    // Wait for frequency to become active
    صحيح ret = sdcard_waitw(&regs->clock_control, SCC_STABLE);
    إذا (ret < 0)
        أرجع ret;
    // Enable SD clock
    writew(&regs->clock_control, creg | SCC_اندرونی_ENABLE | SCC_CLOCK_ENABLE);
    أرجع 0;
}

// Obtain the disk size of an SD card
ساكن صحيح
sdcard_get_سمرتھا(بنية sddrive_s *drive, u8 *csd)
{
    // Original MMC/SD card capacity formula
    u16 C_ناپ = (csd[6] >> 6) | (csd[7] << 2) | ((csd[8] & 0x03) << 10);
    u8 C_ناپ_MULT = (csd[4] >> 7) | ((csd[5] & 0x03) << 1);
    u8 READ_BL_LEN = csd[9] & 0x0f;
    u32 گنتی = (C_ناپ+1) << (C_ناپ_MULT + 2 + READ_BL_LEN - 9);
    // Check for newer encoding formats.
    u8 CSD_STRUCTURE = csd[14] >> 6;
    إذا ((drive->card_type & SF_MMC) && CSD_STRUCTURE >= 2) {
        // Get capacity from EXT_CSD register
        u8 ext_csd[512];
        صحيح ret = sdcard_pio_transfer(drive, SC_SEND_EXT_CSD, 0, ext_csd, 1);
        إذا (ret)
            أرجع ret;
        گنتی = *(u32*)&ext_csd[212];
    } وإلا إذا (!(drive->card_type & SF_MMC) && CSD_STRUCTURE >= 1) {
        // High capacity SD card
        u32 C_SIZE2 = csd[5] | (csd[6] << 8) | ((csd[7] & 0x3f) << 16);
        گنتی = (C_SIZE2+1) << (19-9);
    }
    // Fill drive struct and return
    drive->drive.blksize = DISK_SECTOR_ناپ;
    drive->drive.sectors = گنتی;
    أرجع 0;
}

// Initialize an SD card
ساكن صحيح
sdcard_card_تیاری(بنية sddrive_s *drive, صحيح volt, صحيح prio)
{
    بنية sdhci_s *regs = drive->regs;
    // Set controller to initialization clock rate
    صحيح ret = sdcard_سیٹ_frequency(regs, 400);
    إذا (ret)
        أرجع ret;
    msleep(SDHCI_CLOCK_ON_ویلا);
    // Reset card
    u32 param[4] = { };
    ret = sdcard_pio(regs, SC_GO_IDLE_STATE, param);
    إذا (ret)
        أرجع ret;
    // Let card know SDHC/SDXC is supported and confirm voltage
    u32 hcs = 0, vrange = (volt >= (1<<15) ? 0x100 : 0x200) | 0xaa;
    param[0] = vrange;
    ret = sdcard_pio(regs, SC_SEND_IF_COND, param);
    إذا (!ret && param[0] == vrange)
        hcs = (1<<30);
    // Verify SD card (instead of MMC or SDIO)
    param[0] = 0x00;
    ret = sdcard_pio_app(regs, SC_APP_SEND_OP_COND, param);
    إذا (ret) {
        // Check for MMC card
        param[0] = 0x00;
        ret = sdcard_pio(regs, SC_SEND_OP_COND, param);
        إذا (ret)
            أرجع ret;
        drive->card_type |= SF_MMC;
        hcs = (1<<30);
    }
    // Init card
    u32 سرا = timer_calc(SDHCI_POWERUP_TIMEOUT);
    لكل (;;) {
        param[0] = hcs | volt; // high-capacity support and voltage level
        إذا (drive->card_type & SF_MMC)
            ret = sdcard_pio(regs, SC_SEND_OP_COND, param);
        وإلا
            ret = sdcard_pio_app(regs, SC_APP_SEND_OP_COND, param);
        إذا (ret)
            أرجع ret;
        إذا (param[0] & SR_OCR_NOTBUSY)
            اكسر;
        إذا (timer_check(سرا)) {
            warn_timeout();
            أرجع -1;
        }
        msleep(5); // Avoid flooding log when debugging
    }
    drive->card_type |= (param[0] & SR_OCR_CCS) ? SF_HIGHCAPACITY : 0;
    // Select card (get cid, set rca, get csd, select card)
    param[0] = 0x00;
    ret = sdcard_pio(regs, SC_ALL_SEND_CID, param);
    إذا (ret)
        أرجع ret;
    u8 cid[16];
    حافظہ_نقل(cid, param, حجم(cid));
    param[0] = drive->card_type & SF_MMC ? 0x0001 << 16 : 0x00;
    ret = sdcard_pio(regs, SC_SEND_RELATIVE_ADDR, param);
    إذا (ret)
        أرجع ret;
    u16 rca = drive->card_type & SF_MMC ? 0x0001 : param[0] >> 16;
    param[0] = rca << 16;
    ret = sdcard_pio(regs, SC_SEND_CSD, param);
    إذا (ret)
        أرجع ret;
    u8 csd[16];
    حافظہ_نقل(csd, param, حجم(csd));
    param[0] = rca << 16;
    ret = sdcard_pio(regs, SC_SELECT_DESELECT_CARD, param);
    إذا (ret)
        أرجع ret;
    // Set controller to data transfer clock rate
    ret = sdcard_سیٹ_frequency(regs, 25000);
    إذا (ret)
        أرجع ret;
    // Register drive
    ret = sdcard_get_سمرتھا(drive, csd);
    إذا (ret)
        أرجع ret;
    حرف pnm[7] = {};
    صحيح i;
    لكل (i=0; i < (drive->card_type & SF_MMC ? 6 : 5); i++)
        pnm[i] = cid[11-i];
    حرف *desc = znprintf(MAXDESCSIZE, "%s %s %dMiB"
                          , drive->card_type & SF_MMC ? "MMC drive" : "SD card"
                          , pnm, (u32)(drive->drive.sectors >> 11));
    dprintf(1, "Found sdcard at %p: %s\n", regs, desc);
    boot_جوڑو_hd(&drive->drive, desc, prio);
    أرجع 0;
}

// Setup and configure an SD card controller
ساكن فراغ
sdcard_controller_تیاری(بنية sdhci_s *regs, صحيح prio)
{
    // Initialize controller
    u32 present_state = readl(&regs->present_state);
    إذا (!(present_state & SP_CARD_INSERTED))
        // No card present
        أرجع;
    dprintf(3, "sdhci@%p ver=%x cap=%x %x\n", regs
            , readw(&regs->controller_نسخہ)
            , readl(&regs->cap_lo), readl(&regs->cap_hi));
    sdcard_reset(regs, SRF_ALL);
    writew(&regs->irq_signal, 0);
    writew(&regs->irq_enable, 0x01ff);
    writew(&regs->irq_حالت, readw(&regs->irq_حالت));
    writew(&regs->error_signal, 0);
    writew(&regs->error_irq_enable, 0x01ff);
    writew(&regs->error_irq_حالت, readw(&regs->error_irq_حالت));
    writeb(&regs->timeout_control, 0x0e); // Set to max timeout
    صحيح volt = sdcard_سیٹ_power(regs);
    إذا (volt < 0)
        أرجع;

    // Initialize card
    بنية sddrive_s *drive = malloc_fseg(حجم(*drive));
    إذا (!drive) {
        warn_noalloc();
        اذهب fail;
    }
    memset(drive, 0, حجم(*drive));
    drive->drive.type = DTYPE_SDCARD;
    drive->regs = regs;
    صحيح ret = sdcard_card_تیاری(drive, volt, prio);
    إذا (ret) {
        حرر(drive);
        اذهب fail;
    }
    أرجع;
fail:
    writeb(&regs->power_control, 0);
    writew(&regs->clock_control, 0);
}

ساكن فراغ
sdcard_pci_تیاری(فراغ *انکڑے)
{
    بنية pci_device *pci = انکڑے;
    // XXX - bars dependent on slot index register in pci config space
    بنية sdhci_s *regs = pci_enable_membar(pci, PCI_ادھار_پتہ_0);
    إذا (!regs)
        أرجع;
    صحيح prio = bootprio_لبھو_pci_device(pci);
    sdcard_controller_تیاری(regs, prio);
}

ساكن فراغ
sdcard_romfile_تیاری(فراغ *انکڑے)
{
    بنية romfile_s *file = انکڑے;
    صحيح prio = bootprio_لبھو_named_rom(file->name, 0);
    u32 addr = romfile_loadint(file->name, 0);
    dprintf(1, "Starting sdcard controller check at addr %x\n", addr);
    sdcard_controller_تیاری((فراغ*)addr, prio);
}

فراغ
sdcard_تیاری(فراغ)
{
    إذا (!CONFIG_SDCARD)
        أرجع;

    بنية romfile_s *file = عدم;
    صحيح عدد_romfiles = 0;
    لكل (;;) {
        file = romfile_findprefix("etc/sdcard", file);
        إذا (!file)
            اكسر;
        لگاتار_thread(sdcard_romfile_تیاری, file);
        عدد_romfiles++;
    }
    إذا (عدد_romfiles)
        // only scan for PCI controllers if etc/sdcard not used
        أرجع;

    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci->class != PCI_CLASS_SYSTEM_SDHCI || pci->prog_if >= 2)
            // Not an SDHCI controller following SDHCI spec
            استمر;
        لگاتار_thread(sdcard_pci_تیاری, pci);
    }
}

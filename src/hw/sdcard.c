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
    u16 بلوک_اندازه;
    u16 بلوک_شمار;
    u32 arg;
    u16 transfer_نما;
    u16 cmd;
    u32 response[4];
    u32 داده;
    u32 present_state;
    u8 host_control;
    u8 power_control;
    u8 بلوک_gap_control;
    u8 wakeup_control;
    u16 clock_control;
    u8 timeout_control;
    u8 software_reset;
    u16 irq_وضعیت;
    u16 error_irq_وضعیت;
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
    u16 جایگاه_irq;
    u16 controller_نسخه;
} PACKED;

// SDHCI commands
#تعریف_کن SCB_R0   0x00 // No response
#تعریف_کن SCB_R48  0x1a // Response R1 (no data), R5, R6, R7
#تعریف_کن SCB_R48d 0x3a // Response R1 (with data)
#تعریف_کن SCB_R48b 0x1b // Response R1b, R5b
#تعریف_کن SCB_R48o 0x02 // Response R3, R4
#تعریف_کن SCB_R136 0x09 // Response R2
#تعریف_کن SC_GO_IDLE_STATE        ((0<<8) | SCB_R0)
#تعریف_کن SC_SEND_OP_COND         ((1<<8) | SCB_R48o)
#تعریف_کن SC_ALL_SEND_CID         ((2<<8) | SCB_R136)
#تعریف_کن SC_SEND_RELATIVE_ADDR   ((3<<8) | SCB_R48)
#تعریف_کن SC_SELECT_DESELECT_CARD ((7<<8) | SCB_R48b)
#تعریف_کن SC_SEND_IF_COND         ((8<<8) | SCB_R48)
#تعریف_کن SC_SEND_EXT_CSD         ((8<<8) | SCB_R48d)
#تعریف_کن SC_SEND_CSD             ((9<<8) | SCB_R136)
#تعریف_کن SC_READ_SINGLE          ((17<<8) | SCB_R48d)
#تعریف_کن SC_READ_مضرب        ((18<<8) | SCB_R48d)
#تعریف_کن SC_WRITE_SINGLE         ((24<<8) | SCB_R48d)
#تعریف_کن SC_WRITE_مضرب       ((25<<8) | SCB_R48d)
#تعریف_کن SC_APP_CMD              ((55<<8) | SCB_R48)
#تعریف_کن SC_APP_SEND_OP_COND ((41<<8) | SCB_R48o)

// SDHCI irqs
#تعریف_کن SI_CMD_COMPLETE (1<<0)
#تعریف_کن SI_TRANS_انجامشده   (1<<1)
#تعریف_کن SI_WRITE_READY  (1<<4)
#تعریف_کن SI_READ_READY   (1<<5)
#تعریف_کن SI_ERROR        (1<<15)

// SDHCI present_state flags
#تعریف_کن SP_CMD_INHIBIT   (1<<0)
#تعریف_کن SP_DAT_INHIBIT   (1<<1)
#تعریف_کن SP_CARD_INSERTED (1<<16)

// SDHCI transfer_mode flags
#تعریف_کن ST_BLOCKCOUNT (1<<1)
#تعریف_کن ST_AUTO_CMD12 (1<<2)
#تعریف_کن ST_READ       (1<<4)
#تعریف_کن ST_مضرب   (1<<5)

// SDHCI capabilities flags
#تعریف_کن SD_CAPLO_V33             (1<<24)
#تعریف_کن SD_CAPLO_V30             (1<<25)
#تعریف_کن SD_CAPLO_V18             (1<<26)
#تعریف_کن SD_CAPLO_BASECLOCK_لغزش 8
#تعریف_کن SD_CAPLO_BASECLOCK_ماسک  0xff

// SDHCI clock control flags
#تعریف_کن SCC_داخلی_ENABLE (1<<0)
#تعریف_کن SCC_STABLE          (1<<1)
#تعریف_کن SCC_CLOCK_ENABLE    (1<<2)
#تعریف_کن SCC_SDCLK_ماسک      0xff
#تعریف_کن SCC_SDCLK_لغزش     8
#تعریف_کن SCC_SDCLK_HI_ماسک   0x300
#تعریف_کن SCC_SDCLK_HI_RSHIFT 2

// SDHCI power control flags
#تعریف_کن SPC_POWER_ON (1<<0)
#تعریف_کن SPC_V18      0x0a
#تعریف_کن SPC_V30      0x0c
#تعریف_کن SPC_V33      0x0e

// SDHCI software reset flags
#تعریف_کن SRF_ALL  0x01
#تعریف_کن SRF_CMD  0x02
#تعریف_کن SRF_داده 0x04

// SDHCI result flags
#تعریف_کن SR_OCR_CCS     (1<<30)
#تعریف_کن SR_OCR_NOTBUSY (1<<31)

// SDHCI timeouts
#تعریف_کن SDHCI_POWER_OFF_زمان   1
#تعریف_کن SDHCI_POWER_ON_زمان    5
#تعریف_کن SDHCI_CLOCK_ON_زمان    1 // 74 clock cycles
#تعریف_کن SDHCI_POWERUP_TIMEOUT  1000
#تعریف_کن SDHCI_PIO_TIMEOUT      1000  // XXX - this is just made up

// Internal 'struct drive_s' storage for a detected card
بنية sddrive_s {
    بنية drive_s drive;
    بنية sdhci_s *regs;
    صحيح card_type;
};

// SD card types
#تعریف_کن SF_MMC          (1<<0)
#تعریف_کن SF_HIGHCAPACITY (1<<1)

// Repeatedly read a u16 register until any bit in a given mask is set
ساكن صحيح
sdcard_waitw(u16 *reg, u16 ماسک)
{
    u32 انتها = timer_calc(SDHCI_PIO_TIMEOUT);
    لكل (;;) {
        u16 v = readw(reg);
        إذا (v & ماسک)
            أرجع v;
        إذا (timer_check(انتها)) {
            dprintf(1, "scard_waitw: %p %x %x\n", reg, ماسک, v);
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
    u32 انتها = timer_calc(SDHCI_PIO_TIMEOUT);
    طالما (readb(&regs->software_reset))
        إذا (timer_check(انتها)) {
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
    صحيح ret = sdcard_waitw(&regs->irq_وضعیت, SI_ERROR|SI_CMD_COMPLETE);
    إذا (ret < 0)
        أرجع ret;
    إذا (ret & SI_ERROR) {
        u16 err = readw(&regs->error_irq_وضعیت);
        dprintf(3, "sdcard_pio command stop (code=%x)\n", err);
        sdcard_reset(regs, SRF_CMD|SRF_داده);
        writew(&regs->error_irq_وضعیت, err);
        أرجع -1;
    }
    writew(&regs->irq_وضعیت, SI_CMD_COMPLETE);
    // Read response
    کپی_حافظه(param, regs->response, حجم(regs->response));
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
                    , فراغ *داده, صحيح شمار)
{
    // Send command
    writew(&drive->regs->بلوک_اندازه, DISK_SECTOR_اندازه);
    writew(&drive->regs->بلوک_شمار, شمار);
    صحيح isread = cmd != SC_WRITE_SINGLE && cmd != SC_WRITE_مضرب;
    u16 tmode = ((شمار > 1 ? ST_مضرب|ST_AUTO_CMD12|ST_BLOCKCOUNT : 0)
                 | (isread ? ST_READ : 0));
    writew(&drive->regs->transfer_نما, tmode);
    إذا (!(drive->card_type & SF_HIGHCAPACITY))
        addr *= DISK_SECTOR_اندازه;
    u32 param[4] = { addr };
    صحيح ret = sdcard_pio(drive->regs, cmd, param);
    إذا (ret)
        أرجع ret;
    // Read/write data
    u16 cbit = isread ? SI_READ_READY : SI_WRITE_READY;
    طالما (شمار--) {
        ret = sdcard_waitw(&drive->regs->irq_وضعیت, cbit);
        إذا (ret < 0)
            أرجع ret;
        writew(&drive->regs->irq_وضعیت, cbit);
        صحيح i;
        لكل (i=0; i<DISK_SECTOR_اندازه/4; i++) {
            إذا (isread)
                *(u32*)داده = readl(&drive->regs->داده);
            وإلا
                writel(&drive->regs->داده, *(u32*)داده);
            داده += 4;
        }
    }
    // Complete command
    ret = sdcard_waitw(&drive->regs->irq_وضعیت, SI_TRANS_انجامشده);
    إذا (ret < 0)
        أرجع ret;
    writew(&drive->regs->irq_وضعیت, SI_TRANS_انجامشده);
    أرجع 0;
}

// Read/write a block of data to/from the card.
ساكن صحيح
sdcard_readwrite(بنية disk_op_s *op, صحيح iswrite)
{
    بنية sddrive_s *drive = container_از(
        op->drive_fl, بنية sddrive_s, drive);
    صحيح cmd = iswrite ? SC_WRITE_SINGLE : SC_READ_SINGLE;
    إذا (op->شمار > 1)
        cmd = iswrite ? SC_WRITE_مضرب : SC_READ_مضرب;
    صحيح ret = sdcard_pio_transfer(drive, cmd, op->lba, op->buf_fl, op->شمار);
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
sdcard_مجموعه_power(بنية sdhci_s *regs)
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
    msleep(SDHCI_POWER_OFF_زمان);
    writeb(&regs->power_control, vbits | SPC_POWER_ON);
    msleep(SDHCI_POWER_ON_زمان);
    أرجع volt;
}

ساكن صحيح
sdcard_مجموعه_frequency(بنية sdhci_s *regs, u32 khz)
{
    u16 ver = readw(&regs->controller_نسخه);
    u32 cap = readl(&regs->cap_lo);
    u32 پایه_freq = (cap >> SD_CAPLO_BASECLOCK_لغزش) & SD_CAPLO_BASECLOCK_ماسک;
    إذا (!پایه_freq) {
        dprintf(1, "Unknown base frequency for SD controller\n");
        أرجع -1;
    }
    // Set new frequency
    u32 مقسوم_علیه = DIV_دور_به_بالا(پایه_freq * 1000, khz);
    u16 creg;
    إذا ((ver & 0xff) <= 0x01) {
        مقسوم_علیه = مقسوم_علیه > 1 ? 1 << __fls(مقسوم_علیه-1) : 0;
        creg = (مقسوم_علیه & SCC_SDCLK_ماسک) << SCC_SDCLK_لغزش;
    } وإلا {
        مقسوم_علیه = DIV_دور_به_بالا(مقسوم_علیه, 2);
        creg = (مقسوم_علیه & SCC_SDCLK_ماسک) << SCC_SDCLK_لغزش;
        creg |= (مقسوم_علیه & SCC_SDCLK_HI_ماسک) >> SCC_SDCLK_HI_RSHIFT;
    }
    dprintf(3, "sdcard_set_frequency %d %d %x\n", پایه_freq, khz, creg);
    writew(&regs->clock_control, 0);
    writew(&regs->clock_control, creg | SCC_داخلی_ENABLE);
    // Wait for frequency to become active
    صحيح ret = sdcard_waitw(&regs->clock_control, SCC_STABLE);
    إذا (ret < 0)
        أرجع ret;
    // Enable SD clock
    writew(&regs->clock_control, creg | SCC_داخلی_ENABLE | SCC_CLOCK_ENABLE);
    أرجع 0;
}

// Obtain the disk size of an SD card
ساكن صحيح
sdcard_get_ظرفیت(بنية sddrive_s *drive, u8 *csd)
{
    // Original MMC/SD card capacity formula
    u16 C_اندازه = (csd[6] >> 6) | (csd[7] << 2) | ((csd[8] & 0x03) << 10);
    u8 C_اندازه_MULT = (csd[4] >> 7) | ((csd[5] & 0x03) << 1);
    u8 READ_BL_LEN = csd[9] & 0x0f;
    u32 شمار = (C_اندازه+1) << (C_اندازه_MULT + 2 + READ_BL_LEN - 9);
    // Check for newer encoding formats.
    u8 CSD_STRUCTURE = csd[14] >> 6;
    إذا ((drive->card_type & SF_MMC) && CSD_STRUCTURE >= 2) {
        // Get capacity from EXT_CSD register
        u8 ext_csd[512];
        صحيح ret = sdcard_pio_transfer(drive, SC_SEND_EXT_CSD, 0, ext_csd, 1);
        إذا (ret)
            أرجع ret;
        شمار = *(u32*)&ext_csd[212];
    } وإلا إذا (!(drive->card_type & SF_MMC) && CSD_STRUCTURE >= 1) {
        // High capacity SD card
        u32 C_SIZE2 = csd[5] | (csd[6] << 8) | ((csd[7] & 0x3f) << 16);
        شمار = (C_SIZE2+1) << (19-9);
    }
    // Fill drive struct and return
    drive->drive.blksize = DISK_SECTOR_اندازه;
    drive->drive.sectors = شمار;
    أرجع 0;
}

// Initialize an SD card
ساكن صحيح
sdcard_card_راهاندازی(بنية sddrive_s *drive, صحيح volt, صحيح prio)
{
    بنية sdhci_s *regs = drive->regs;
    // Set controller to initialization clock rate
    صحيح ret = sdcard_مجموعه_frequency(regs, 400);
    إذا (ret)
        أرجع ret;
    msleep(SDHCI_CLOCK_ON_زمان);
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
    u32 انتها = timer_calc(SDHCI_POWERUP_TIMEOUT);
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
        إذا (timer_check(انتها)) {
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
    کپی_حافظه(cid, param, حجم(cid));
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
    کپی_حافظه(csd, param, حجم(csd));
    param[0] = rca << 16;
    ret = sdcard_pio(regs, SC_SELECT_DESELECT_CARD, param);
    إذا (ret)
        أرجع ret;
    // Set controller to data transfer clock rate
    ret = sdcard_مجموعه_frequency(regs, 25000);
    إذا (ret)
        أرجع ret;
    // Register drive
    ret = sdcard_get_ظرفیت(drive, csd);
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
    boot_جمع_کن_hd(&drive->drive, desc, prio);
    أرجع 0;
}

// Setup and configure an SD card controller
ساكن فراغ
sdcard_controller_راهاندازی(بنية sdhci_s *regs, صحيح prio)
{
    // Initialize controller
    u32 present_state = readl(&regs->present_state);
    إذا (!(present_state & SP_CARD_INSERTED))
        // No card present
        أرجع;
    dprintf(3, "sdhci@%p ver=%x cap=%x %x\n", regs
            , readw(&regs->controller_نسخه)
            , readl(&regs->cap_lo), readl(&regs->cap_hi));
    sdcard_reset(regs, SRF_ALL);
    writew(&regs->irq_signal, 0);
    writew(&regs->irq_enable, 0x01ff);
    writew(&regs->irq_وضعیت, readw(&regs->irq_وضعیت));
    writew(&regs->error_signal, 0);
    writew(&regs->error_irq_enable, 0x01ff);
    writew(&regs->error_irq_وضعیت, readw(&regs->error_irq_وضعیت));
    writeb(&regs->timeout_control, 0x0e); // Set to max timeout
    صحيح volt = sdcard_مجموعه_power(regs);
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
    صحيح ret = sdcard_card_راهاندازی(drive, volt, prio);
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
sdcard_pci_راهاندازی(فراغ *داده)
{
    بنية pci_device *pci = داده;
    // XXX - bars dependent on slot index register in pci config space
    بنية sdhci_s *regs = pci_enable_membar(pci, PCI_پایه_آدرس_0);
    إذا (!regs)
        أرجع;
    صحيح prio = bootprio_پیدا_کن_pci_device(pci);
    sdcard_controller_راهاندازی(regs, prio);
}

ساكن فراغ
sdcard_romfile_راهاندازی(فراغ *داده)
{
    بنية romfile_s *file = داده;
    صحيح prio = bootprio_پیدا_کن_named_rom(file->name, 0);
    u32 addr = romfile_loadint(file->name, 0);
    dprintf(1, "Starting sdcard controller check at addr %x\n", addr);
    sdcard_controller_راهاندازی((فراغ*)addr, prio);
}

فراغ
sdcard_راهاندازی(فراغ)
{
    إذا (!CONFIG_SDCARD)
        أرجع;

    بنية romfile_s *file = عدم;
    صحيح رقم_romfiles = 0;
    لكل (;;) {
        file = romfile_findprefix("etc/sdcard", file);
        إذا (!file)
            اكسر;
        پیاپی_thread(sdcard_romfile_راهاندازی, file);
        رقم_romfiles++;
    }
    إذا (رقم_romfiles)
        // only scan for PCI controllers if etc/sdcard not used
        أرجع;

    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci->class != PCI_CLASS_SYSTEM_SDHCI || pci->prog_if >= 2)
            // Not an SDHCI controller following SDHCI spec
            استمر;
        پیاپی_thread(sdcard_pci_راهاندازی, pci);
    }
}

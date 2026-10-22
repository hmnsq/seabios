// PCI SD Host Controller Interface
//
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "block.h" // struct drive_s
#அந்தர்பாவ "malloc.h" // malloc_fseg
#அந்தர்பாவ "output.h" // znprintf
#அந்தர்பாவ "pcidevice.h" // foreachpci
#அந்தர்பாவ "pci_ids.h" // PCI_CLASS_SYSTEM_SDHCI
#அந்தர்பாவ "pci_regs.h" // PCI_BASE_ADDRESS_0
#அந்தர்பாவ "romfile.h" // romfile_findprefix
#அந்தர்பாவ "stacks.h" // yield
#அந்தர்பாவ "std/disk.h" // DISK_RET_SUCCESS
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // boot_add_hd
#அந்தர்பாவ "x86.h" // writel

// SDHCI MMIO registers
கட்டமைப்பு sdhci_s {
    u32 sdma_addr;
    u16 block_size;
    u16 block_மொத்தம்;
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
கட்டமைப்பு sddrive_s {
    கட்டமைப்பு drive_s drive;
    கட்டமைப்பு sdhci_s *regs;
    முழு card_type;
};

// SD card types
#define SF_MMC          (1<<0)
#define SF_HIGHCAPACITY (1<<1)

// Repeatedly read a u16 register until any bit in a given mask is set
நிலையான முழு
sdcard_waitw(u16 *reg, u16 mask)
{
    u32 end = timer_calc(SDHCI_PIO_TIMEOUT);
    க்கு (;;) {
        u16 v = readw(reg);
        எனில் (v & mask)
            திரும்பு v;
        எனில் (timer_check(end)) {
            dprintf(1, "scard_waitw: %p %x %x\n", reg, mask, v);
            warn_timeout();
            திரும்பு -1;
        }
        yield();
    }
}

// Send an sdhci reset
நிலையான முழு
sdcard_reset(கட்டமைப்பு sdhci_s *regs, முழு flags)
{
    writeb(&regs->software_reset, flags);
    u32 end = timer_calc(SDHCI_PIO_TIMEOUT);
    போது (readb(&regs->software_reset))
        எனில் (timer_check(end)) {
            warn_timeout();
            திரும்பு -1;
        }
    திரும்பு 0;
}

// Send a command to the card.
நிலையான முழு
sdcard_pio(கட்டமைப்பு sdhci_s *regs, முழு cmd, u32 *param)
{
    u32 state = readl(&regs->present_state);
    dprintf(9, "sdcard_pio cmd %x %x %x\n", cmd, *param, state);
    எனில் ((state & SP_CMD_INHIBIT)
        || ((cmd & 0x03) == 0x03 && state & SP_DAT_INHIBIT)) {
        dprintf(1, "sdcard_pio not ready %x\n", state);
        திரும்பு -1;
    }
    // Send command
    writel(&regs->arg, *param);
    writew(&regs->cmd, cmd);
    முழு ret = sdcard_waitw(&regs->irq_status, SI_ERROR|SI_CMD_COMPLETE);
    எனில் (ret < 0)
        திரும்பு ret;
    எனில் (ret & SI_ERROR) {
        u16 err = readw(&regs->error_irq_status);
        dprintf(3, "sdcard_pio command stop (code=%x)\n", err);
        sdcard_reset(regs, SRF_CMD|SRF_DATA);
        writew(&regs->error_irq_status, err);
        திரும்பு -1;
    }
    writew(&regs->irq_status, SI_CMD_COMPLETE);
    // Read response
    memcpy(param, regs->response, அளவு(regs->response));
    dprintf(9, "sdcard cmd %x response %x %x %x %x\n"
            , cmd, param[0], param[1], param[2], param[3]);
    திரும்பு 0;
}

// Send an "app specific" command to the card.
நிலையான முழு
sdcard_pio_app(கட்டமைப்பு sdhci_s *regs, முழு cmd, u32 *param)
{
    u32 aparam[4] = {};
    முழு ret = sdcard_pio(regs, SC_APP_CMD, aparam);
    எனில் (ret)
        திரும்பு ret;
    திரும்பு sdcard_pio(regs, cmd, param);
}

// Send a command to the card which transfers data.
நிலையான முழு
sdcard_pio_transfer(கட்டமைப்பு sddrive_s *drive, முழு cmd, u32 addr
                    , வெறுமை *data, முழு மொத்தம்)
{
    // Send command
    writew(&drive->regs->block_size, DISK_SECTOR_SIZE);
    writew(&drive->regs->block_மொத்தம், மொத்தம்);
    முழு isread = cmd != SC_WRITE_SINGLE && cmd != SC_WRITE_MULTIPLE;
    u16 tmode = ((மொத்தம் > 1 ? ST_MULTIPLE|ST_AUTO_CMD12|ST_BLOCKCOUNT : 0)
                 | (isread ? ST_READ : 0));
    writew(&drive->regs->transfer_mode, tmode);
    எனில் (!(drive->card_type & SF_HIGHCAPACITY))
        addr *= DISK_SECTOR_SIZE;
    u32 param[4] = { addr };
    முழு ret = sdcard_pio(drive->regs, cmd, param);
    எனில் (ret)
        திரும்பு ret;
    // Read/write data
    u16 cbit = isread ? SI_READ_READY : SI_WRITE_READY;
    போது (மொத்தம்--) {
        ret = sdcard_waitw(&drive->regs->irq_status, cbit);
        எனில் (ret < 0)
            திரும்பு ret;
        writew(&drive->regs->irq_status, cbit);
        முழு i;
        க்கு (i=0; i<DISK_SECTOR_SIZE/4; i++) {
            எனில் (isread)
                *(u32*)data = readl(&drive->regs->data);
            இல்லையெனில்
                writel(&drive->regs->data, *(u32*)data);
            data += 4;
        }
    }
    // Complete command
    ret = sdcard_waitw(&drive->regs->irq_status, SI_TRANS_DONE);
    எனில் (ret < 0)
        திரும்பு ret;
    writew(&drive->regs->irq_status, SI_TRANS_DONE);
    திரும்பு 0;
}

// Read/write a block of data to/from the card.
நிலையான முழு
sdcard_readwrite(கட்டமைப்பு disk_op_s *op, முழு iswrite)
{
    கட்டமைப்பு sddrive_s *drive = container_of(
        op->drive_fl, கட்டமைப்பு sddrive_s, drive);
    முழு cmd = iswrite ? SC_WRITE_SINGLE : SC_READ_SINGLE;
    எனில் (op->மொத்தம் > 1)
        cmd = iswrite ? SC_WRITE_MULTIPLE : SC_READ_MULTIPLE;
    முழு ret = sdcard_pio_transfer(drive, cmd, op->lba, op->buf_fl, op->மொத்தம்);
    எனில் (ret)
        திரும்பு DISK_RET_EBADTRACK;
    திரும்பு DISK_RET_SUCCESS;
}

முழு
sdcard_process_op(கட்டமைப்பு disk_op_s *op)
{
    எனில் (!CONFIG_SDCARD)
        திரும்பு 0;
    தேர்வு (op->command) {
    நிலை CMD_READ:
        திரும்பு sdcard_readwrite(op, 0);
    நிலை CMD_WRITE:
        திரும்பு sdcard_readwrite(op, 1);
    இயல்பு:
        திரும்பு default_process_op(op);
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

நிலையான முழு
sdcard_set_power(கட்டமைப்பு sdhci_s *regs)
{
    u32 cap = readl(&regs->cap_lo);
    u32 volt, vbits;
    எனில் (cap & SD_CAPLO_V33) {
        volt = 1<<20;
        vbits = SPC_V33;
    } இல்லையெனில் எனில் (cap & SD_CAPLO_V30) {
        volt = 1<<18;
        vbits = SPC_V30;
    } இல்லையெனில் எனில் (cap & SD_CAPLO_V18) {
        volt = 1<<7;
        vbits = SPC_V18;
    } இல்லையெனில் {
        dprintf(1, "SD controller unsupported volt range (%x)\n", cap);
        திரும்பு -1;
    }
    writeb(&regs->power_control, 0);
    msleep(SDHCI_POWER_OFF_TIME);
    writeb(&regs->power_control, vbits | SPC_POWER_ON);
    msleep(SDHCI_POWER_ON_TIME);
    திரும்பு volt;
}

நிலையான முழு
sdcard_set_frequency(கட்டமைப்பு sdhci_s *regs, u32 khz)
{
    u16 ver = readw(&regs->controller_version);
    u32 cap = readl(&regs->cap_lo);
    u32 base_freq = (cap >> SD_CAPLO_BASECLOCK_SHIFT) & SD_CAPLO_BASECLOCK_MASK;
    எனில் (!base_freq) {
        dprintf(1, "Unknown base frequency for SD controller\n");
        திரும்பு -1;
    }
    // Set new frequency
    u32 வகுஎண் = DIV_ROUND_UP(base_freq * 1000, khz);
    u16 creg;
    எனில் ((ver & 0xff) <= 0x01) {
        வகுஎண் = வகுஎண் > 1 ? 1 << __fls(வகுஎண்-1) : 0;
        creg = (வகுஎண் & SCC_SDCLK_MASK) << SCC_SDCLK_SHIFT;
    } இல்லையெனில் {
        வகுஎண் = DIV_ROUND_UP(வகுஎண், 2);
        creg = (வகுஎண் & SCC_SDCLK_MASK) << SCC_SDCLK_SHIFT;
        creg |= (வகுஎண் & SCC_SDCLK_HI_MASK) >> SCC_SDCLK_HI_RSHIFT;
    }
    dprintf(3, "sdcard_set_frequency %d %d %x\n", base_freq, khz, creg);
    writew(&regs->clock_control, 0);
    writew(&regs->clock_control, creg | SCC_INTERNAL_ENABLE);
    // Wait for frequency to become active
    முழு ret = sdcard_waitw(&regs->clock_control, SCC_STABLE);
    எனில் (ret < 0)
        திரும்பு ret;
    // Enable SD clock
    writew(&regs->clock_control, creg | SCC_INTERNAL_ENABLE | SCC_CLOCK_ENABLE);
    திரும்பு 0;
}

// Obtain the disk size of an SD card
நிலையான முழு
sdcard_get_capacity(கட்டமைப்பு sddrive_s *drive, u8 *csd)
{
    // Original MMC/SD card capacity formula
    u16 C_SIZE = (csd[6] >> 6) | (csd[7] << 2) | ((csd[8] & 0x03) << 10);
    u8 C_SIZE_MULT = (csd[4] >> 7) | ((csd[5] & 0x03) << 1);
    u8 READ_BL_LEN = csd[9] & 0x0f;
    u32 மொத்தம் = (C_SIZE+1) << (C_SIZE_MULT + 2 + READ_BL_LEN - 9);
    // Check for newer encoding formats.
    u8 CSD_STRUCTURE = csd[14] >> 6;
    எனில் ((drive->card_type & SF_MMC) && CSD_STRUCTURE >= 2) {
        // Get capacity from EXT_CSD register
        u8 ext_csd[512];
        முழு ret = sdcard_pio_transfer(drive, SC_SEND_EXT_CSD, 0, ext_csd, 1);
        எனில் (ret)
            திரும்பு ret;
        மொத்தம் = *(u32*)&ext_csd[212];
    } இல்லையெனில் எனில் (!(drive->card_type & SF_MMC) && CSD_STRUCTURE >= 1) {
        // High capacity SD card
        u32 C_SIZE2 = csd[5] | (csd[6] << 8) | ((csd[7] & 0x3f) << 16);
        மொத்தம் = (C_SIZE2+1) << (19-9);
    }
    // Fill drive struct and return
    drive->drive.blksize = DISK_SECTOR_SIZE;
    drive->drive.sectors = மொத்தம்;
    திரும்பு 0;
}

// Initialize an SD card
நிலையான முழு
sdcard_card_setup(கட்டமைப்பு sddrive_s *drive, முழு volt, முழு prio)
{
    கட்டமைப்பு sdhci_s *regs = drive->regs;
    // Set controller to initialization clock rate
    முழு ret = sdcard_set_frequency(regs, 400);
    எனில் (ret)
        திரும்பு ret;
    msleep(SDHCI_CLOCK_ON_TIME);
    // Reset card
    u32 param[4] = { };
    ret = sdcard_pio(regs, SC_GO_IDLE_STATE, param);
    எனில் (ret)
        திரும்பு ret;
    // Let card know SDHC/SDXC is supported and confirm voltage
    u32 hcs = 0, vrange = (volt >= (1<<15) ? 0x100 : 0x200) | 0xaa;
    param[0] = vrange;
    ret = sdcard_pio(regs, SC_SEND_IF_COND, param);
    எனில் (!ret && param[0] == vrange)
        hcs = (1<<30);
    // Verify SD card (instead of MMC or SDIO)
    param[0] = 0x00;
    ret = sdcard_pio_app(regs, SC_APP_SEND_OP_COND, param);
    எனில் (ret) {
        // Check for MMC card
        param[0] = 0x00;
        ret = sdcard_pio(regs, SC_SEND_OP_COND, param);
        எனில் (ret)
            திரும்பு ret;
        drive->card_type |= SF_MMC;
        hcs = (1<<30);
    }
    // Init card
    u32 end = timer_calc(SDHCI_POWERUP_TIMEOUT);
    க்கு (;;) {
        param[0] = hcs | volt; // high-capacity support and voltage level
        எனில் (drive->card_type & SF_MMC)
            ret = sdcard_pio(regs, SC_SEND_OP_COND, param);
        இல்லையெனில்
            ret = sdcard_pio_app(regs, SC_APP_SEND_OP_COND, param);
        எனில் (ret)
            திரும்பு ret;
        எனில் (param[0] & SR_OCR_NOTBUSY)
            நிறுத்து;
        எனில் (timer_check(end)) {
            warn_timeout();
            திரும்பு -1;
        }
        msleep(5); // Avoid flooding log when debugging
    }
    drive->card_type |= (param[0] & SR_OCR_CCS) ? SF_HIGHCAPACITY : 0;
    // Select card (get cid, set rca, get csd, select card)
    param[0] = 0x00;
    ret = sdcard_pio(regs, SC_ALL_SEND_CID, param);
    எனில் (ret)
        திரும்பு ret;
    u8 cid[16];
    memcpy(cid, param, அளவு(cid));
    param[0] = drive->card_type & SF_MMC ? 0x0001 << 16 : 0x00;
    ret = sdcard_pio(regs, SC_SEND_RELATIVE_ADDR, param);
    எனில் (ret)
        திரும்பு ret;
    u16 rca = drive->card_type & SF_MMC ? 0x0001 : param[0] >> 16;
    param[0] = rca << 16;
    ret = sdcard_pio(regs, SC_SEND_CSD, param);
    எனில் (ret)
        திரும்பு ret;
    u8 csd[16];
    memcpy(csd, param, அளவு(csd));
    param[0] = rca << 16;
    ret = sdcard_pio(regs, SC_SELECT_DESELECT_CARD, param);
    எனில் (ret)
        திரும்பு ret;
    // Set controller to data transfer clock rate
    ret = sdcard_set_frequency(regs, 25000);
    எனில் (ret)
        திரும்பு ret;
    // Register drive
    ret = sdcard_get_capacity(drive, csd);
    எனில் (ret)
        திரும்பு ret;
    எழுத்து pnm[7] = {};
    முழு i;
    க்கு (i=0; i < (drive->card_type & SF_MMC ? 6 : 5); i++)
        pnm[i] = cid[11-i];
    எழுத்து *desc = znprintf(MAXDESCSIZE, "%s %s %dMiB"
                          , drive->card_type & SF_MMC ? "MMC drive" : "SD card"
                          , pnm, (u32)(drive->drive.sectors >> 11));
    dprintf(1, "Found sdcard at %p: %s\n", regs, desc);
    boot_add_hd(&drive->drive, desc, prio);
    திரும்பு 0;
}

// Setup and configure an SD card controller
நிலையான வெறுமை
sdcard_controller_setup(கட்டமைப்பு sdhci_s *regs, முழு prio)
{
    // Initialize controller
    u32 present_state = readl(&regs->present_state);
    எனில் (!(present_state & SP_CARD_INSERTED))
        // No card present
        திரும்பு;
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
    முழு volt = sdcard_set_power(regs);
    எனில் (volt < 0)
        திரும்பு;

    // Initialize card
    கட்டமைப்பு sddrive_s *drive = malloc_fseg(அளவு(*drive));
    எனில் (!drive) {
        warn_noalloc();
        செல் fail;
    }
    memset(drive, 0, அளவு(*drive));
    drive->drive.type = DTYPE_SDCARD;
    drive->regs = regs;
    முழு ret = sdcard_card_setup(drive, volt, prio);
    எனில் (ret) {
        விடுவி(drive);
        செல் fail;
    }
    திரும்பு;
fail:
    writeb(&regs->power_control, 0);
    writew(&regs->clock_control, 0);
}

நிலையான வெறுமை
sdcard_pci_setup(வெறுமை *data)
{
    கட்டமைப்பு pci_device *pci = data;
    // XXX - bars dependent on slot index register in pci config space
    கட்டமைப்பு sdhci_s *regs = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    எனில் (!regs)
        திரும்பு;
    முழு prio = bootprio_find_pci_device(pci);
    sdcard_controller_setup(regs, prio);
}

நிலையான வெறுமை
sdcard_romfile_setup(வெறுமை *data)
{
    கட்டமைப்பு romfile_s *file = data;
    முழு prio = bootprio_find_named_rom(file->name, 0);
    u32 addr = romfile_loadint(file->name, 0);
    dprintf(1, "Starting sdcard controller check at addr %x\n", addr);
    sdcard_controller_setup((வெறுமை*)addr, prio);
}

வெறுமை
sdcard_setup(வெறுமை)
{
    எனில் (!CONFIG_SDCARD)
        திரும்பு;

    கட்டமைப்பு romfile_s *file = வெற்று;
    முழு இலக்கம்_romfiles = 0;
    க்கு (;;) {
        file = romfile_findprefix("etc/sdcard", file);
        எனில் (!file)
            நிறுத்து;
        run_thread(sdcard_romfile_setup, file);
        இலக்கம்_romfiles++;
    }
    எனில் (இலக்கம்_romfiles)
        // only scan for PCI controllers if etc/sdcard not used
        திரும்பு;

    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        எனில் (pci->class != PCI_CLASS_SYSTEM_SDHCI || pci->prog_if >= 2)
            // Not an SDHCI controller following SDHCI spec
            தொடர்;
        run_thread(sdcard_pci_setup, pci);
    }
}

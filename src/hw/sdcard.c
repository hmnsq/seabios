// PCI SD Host Controller Interface
//
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "block.h" // struct drive_s
#समावेश "malloc.h" // malloc_fseg
#समावेश "output.h" // znprintf
#समावेश "pcidevice.h" // foreachpci
#समावेश "pci_ids.h" // PCI_CLASS_SYSTEM_SDHCI
#समावेश "pci_regs.h" // PCI_BASE_ADDRESS_0
#समावेश "romfile.h" // romfile_findprefix
#समावेश "stacks.h" // yield
#समावेश "std/disk.h" // DISK_RET_SUCCESS
#समावेश "string.h" // memset
#समावेश "util.h" // boot_add_hd
#समावेश "x86.h" // writel

// SDHCI MMIO registers
काष्ठा sdhci_s {
    u32 sdma_addr;
    u16 block_आकार;
    u16 block_गिनती;
    u32 arg;
    u16 transfer_mode;
    u16 cmd;
    u32 response[4];
    u32 तथ्य;
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
#घोषणा SCB_R0   0x00 // No response
#घोषणा SCB_R48  0x1a // Response R1 (no data), R5, R6, R7
#घोषणा SCB_R48d 0x3a // Response R1 (with data)
#घोषणा SCB_R48b 0x1b // Response R1b, R5b
#घोषणा SCB_R48o 0x02 // Response R3, R4
#घोषणा SCB_R136 0x09 // Response R2
#घोषणा SC_GO_IDLE_STATE        ((0<<8) | SCB_R0)
#घोषणा SC_SEND_OP_COND         ((1<<8) | SCB_R48o)
#घोषणा SC_ALL_SEND_CID         ((2<<8) | SCB_R136)
#घोषणा SC_SEND_RELATIVE_ADDR   ((3<<8) | SCB_R48)
#घोषणा SC_SELECT_DESELECT_CARD ((7<<8) | SCB_R48b)
#घोषणा SC_SEND_IF_COND         ((8<<8) | SCB_R48)
#घोषणा SC_SEND_EXT_CSD         ((8<<8) | SCB_R48d)
#घोषणा SC_SEND_CSD             ((9<<8) | SCB_R136)
#घोषणा SC_READ_SINGLE          ((17<<8) | SCB_R48d)
#घोषणा SC_READ_MULTIPLE        ((18<<8) | SCB_R48d)
#घोषणा SC_WRITE_SINGLE         ((24<<8) | SCB_R48d)
#घोषणा SC_WRITE_MULTIPLE       ((25<<8) | SCB_R48d)
#घोषणा SC_APP_CMD              ((55<<8) | SCB_R48)
#घोषणा SC_APP_SEND_OP_COND ((41<<8) | SCB_R48o)

// SDHCI irqs
#घोषणा SI_CMD_COMPLETE (1<<0)
#घोषणा SI_TRANS_DONE   (1<<1)
#घोषणा SI_WRITE_READY  (1<<4)
#घोषणा SI_READ_READY   (1<<5)
#घोषणा SI_ERROR        (1<<15)

// SDHCI present_state flags
#घोषणा SP_CMD_INHIBIT   (1<<0)
#घोषणा SP_DAT_INHIBIT   (1<<1)
#घोषणा SP_CARD_INSERTED (1<<16)

// SDHCI transfer_mode flags
#घोषणा ST_BLOCKCOUNT (1<<1)
#घोषणा ST_AUTO_CMD12 (1<<2)
#घोषणा ST_READ       (1<<4)
#घोषणा ST_MULTIPLE   (1<<5)

// SDHCI capabilities flags
#घोषणा SD_CAPLO_V33             (1<<24)
#घोषणा SD_CAPLO_V30             (1<<25)
#घोषणा SD_CAPLO_V18             (1<<26)
#घोषणा SD_CAPLO_BASECLOCK_SHIFT 8
#घोषणा SD_CAPLO_BASECLOCK_MASK  0xff

// SDHCI clock control flags
#घोषणा SCC_INTERNAL_ENABLE (1<<0)
#घोषणा SCC_STABLE          (1<<1)
#घोषणा SCC_CLOCK_ENABLE    (1<<2)
#घोषणा SCC_SDCLK_MASK      0xff
#घोषणा SCC_SDCLK_SHIFT     8
#घोषणा SCC_SDCLK_HI_MASK   0x300
#घोषणा SCC_SDCLK_HI_RSHIFT 2

// SDHCI power control flags
#घोषणा SPC_POWER_ON (1<<0)
#घोषणा SPC_V18      0x0a
#घोषणा SPC_V30      0x0c
#घोषणा SPC_V33      0x0e

// SDHCI software reset flags
#घोषणा SRF_ALL  0x01
#घोषणा SRF_CMD  0x02
#घोषणा SRF_तथ्य 0x04

// SDHCI result flags
#घोषणा SR_OCR_CCS     (1<<30)
#घोषणा SR_OCR_NOTBUSY (1<<31)

// SDHCI timeouts
#घोषणा SDHCI_POWER_OFF_TIME   1
#घोषणा SDHCI_POWER_ON_TIME    5
#घोषणा SDHCI_CLOCK_ON_TIME    1 // 74 clock cycles
#घोषणा SDHCI_POWERUP_TIMEOUT  1000
#घोषणा SDHCI_PIO_TIMEOUT      1000  // XXX - this is just made up

// Internal 'struct drive_s' storage for a detected card
काष्ठा sddrive_s {
    काष्ठा drive_s drive;
    काष्ठा sdhci_s *regs;
    पूर्णांक card_type;
};

// SD card types
#घोषणा SF_MMC          (1<<0)
#घोषणा SF_HIGHCAPACITY (1<<1)

// Repeatedly read a u16 register until any bit in a given mask is set
अटल पूर्णांक
sdcard_waitw(u16 *reg, u16 mask)
{
    u32 end = timer_calc(SDHCI_PIO_TIMEOUT);
    क्रम (;;) {
        u16 v = readw(reg);
        अगर (v & mask)
            वापस v;
        अगर (timer_check(end)) {
            dprintf(1, "scard_waitw: %p %x %x\n", reg, mask, v);
            warn_timeout();
            वापस -1;
        }
        yield();
    }
}

// Send an sdhci reset
अटल पूर्णांक
sdcard_reset(काष्ठा sdhci_s *regs, पूर्णांक flags)
{
    writeb(&regs->software_reset, flags);
    u32 end = timer_calc(SDHCI_PIO_TIMEOUT);
    जबतक (readb(&regs->software_reset))
        अगर (timer_check(end)) {
            warn_timeout();
            वापस -1;
        }
    वापस 0;
}

// Send a command to the card.
अटल पूर्णांक
sdcard_pio(काष्ठा sdhci_s *regs, पूर्णांक cmd, u32 *param)
{
    u32 state = readl(&regs->present_state);
    dprintf(9, "sdcard_pio cmd %x %x %x\n", cmd, *param, state);
    अगर ((state & SP_CMD_INHIBIT)
        || ((cmd & 0x03) == 0x03 && state & SP_DAT_INHIBIT)) {
        dprintf(1, "sdcard_pio not ready %x\n", state);
        वापस -1;
    }
    // Send command
    writel(&regs->arg, *param);
    writew(&regs->cmd, cmd);
    पूर्णांक ret = sdcard_waitw(&regs->irq_status, SI_ERROR|SI_CMD_COMPLETE);
    अगर (ret < 0)
        वापस ret;
    अगर (ret & SI_ERROR) {
        u16 err = readw(&regs->error_irq_status);
        dprintf(3, "sdcard_pio command stop (code=%x)\n", err);
        sdcard_reset(regs, SRF_CMD|SRF_तथ्य);
        writew(&regs->error_irq_status, err);
        वापस -1;
    }
    writew(&regs->irq_status, SI_CMD_COMPLETE);
    // Read response
    स_नकल(param, regs->response, माप(regs->response));
    dprintf(9, "sdcard cmd %x response %x %x %x %x\n"
            , cmd, param[0], param[1], param[2], param[3]);
    वापस 0;
}

// Send an "app specific" command to the card.
अटल पूर्णांक
sdcard_pio_app(काष्ठा sdhci_s *regs, पूर्णांक cmd, u32 *param)
{
    u32 aparam[4] = {};
    पूर्णांक ret = sdcard_pio(regs, SC_APP_CMD, aparam);
    अगर (ret)
        वापस ret;
    वापस sdcard_pio(regs, cmd, param);
}

// Send a command to the card which transfers data.
अटल पूर्णांक
sdcard_pio_transfer(काष्ठा sddrive_s *drive, पूर्णांक cmd, u32 addr
                    , व्योम *तथ्य, पूर्णांक गिनती)
{
    // Send command
    writew(&drive->regs->block_आकार, DISK_SECTOR_आकार);
    writew(&drive->regs->block_गिनती, गिनती);
    पूर्णांक isread = cmd != SC_WRITE_SINGLE && cmd != SC_WRITE_MULTIPLE;
    u16 tmode = ((गिनती > 1 ? ST_MULTIPLE|ST_AUTO_CMD12|ST_BLOCKCOUNT : 0)
                 | (isread ? ST_READ : 0));
    writew(&drive->regs->transfer_mode, tmode);
    अगर (!(drive->card_type & SF_HIGHCAPACITY))
        addr *= DISK_SECTOR_आकार;
    u32 param[4] = { addr };
    पूर्णांक ret = sdcard_pio(drive->regs, cmd, param);
    अगर (ret)
        वापस ret;
    // Read/write data
    u16 cbit = isread ? SI_READ_READY : SI_WRITE_READY;
    जबतक (गिनती--) {
        ret = sdcard_waitw(&drive->regs->irq_status, cbit);
        अगर (ret < 0)
            वापस ret;
        writew(&drive->regs->irq_status, cbit);
        पूर्णांक i;
        क्रम (i=0; i<DISK_SECTOR_आकार/4; i++) {
            अगर (isread)
                *(u32*)तथ्य = readl(&drive->regs->तथ्य);
            अन्यथा
                writel(&drive->regs->तथ्य, *(u32*)तथ्य);
            तथ्य += 4;
        }
    }
    // Complete command
    ret = sdcard_waitw(&drive->regs->irq_status, SI_TRANS_DONE);
    अगर (ret < 0)
        वापस ret;
    writew(&drive->regs->irq_status, SI_TRANS_DONE);
    वापस 0;
}

// Read/write a block of data to/from the card.
अटल पूर्णांक
sdcard_readwrite(काष्ठा disk_op_s *op, पूर्णांक iswrite)
{
    काष्ठा sddrive_s *drive = container_of(
        op->drive_fl, काष्ठा sddrive_s, drive);
    पूर्णांक cmd = iswrite ? SC_WRITE_SINGLE : SC_READ_SINGLE;
    अगर (op->गिनती > 1)
        cmd = iswrite ? SC_WRITE_MULTIPLE : SC_READ_MULTIPLE;
    पूर्णांक ret = sdcard_pio_transfer(drive, cmd, op->lba, op->buf_fl, op->गिनती);
    अगर (ret)
        वापस DISK_RET_EBADTRACK;
    वापस DISK_RET_SUCCESS;
}

पूर्णांक
sdcard_process_op(काष्ठा disk_op_s *op)
{
    अगर (!CONFIG_SDCARD)
        वापस 0;
    चयन (op->command) {
    हाल CMD_READ:
        वापस sdcard_readwrite(op, 0);
    हाल CMD_WRITE:
        वापस sdcard_readwrite(op, 1);
    शेष:
        वापस default_process_op(op);
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

अटल पूर्णांक
sdcard_set_power(काष्ठा sdhci_s *regs)
{
    u32 cap = readl(&regs->cap_lo);
    u32 volt, vbits;
    अगर (cap & SD_CAPLO_V33) {
        volt = 1<<20;
        vbits = SPC_V33;
    } अन्यथा अगर (cap & SD_CAPLO_V30) {
        volt = 1<<18;
        vbits = SPC_V30;
    } अन्यथा अगर (cap & SD_CAPLO_V18) {
        volt = 1<<7;
        vbits = SPC_V18;
    } अन्यथा {
        dprintf(1, "SD controller unsupported volt range (%x)\n", cap);
        वापस -1;
    }
    writeb(&regs->power_control, 0);
    msleep(SDHCI_POWER_OFF_TIME);
    writeb(&regs->power_control, vbits | SPC_POWER_ON);
    msleep(SDHCI_POWER_ON_TIME);
    वापस volt;
}

अटल पूर्णांक
sdcard_set_frequency(काष्ठा sdhci_s *regs, u32 khz)
{
    u16 ver = readw(&regs->controller_version);
    u32 cap = readl(&regs->cap_lo);
    u32 base_freq = (cap >> SD_CAPLO_BASECLOCK_SHIFT) & SD_CAPLO_BASECLOCK_MASK;
    अगर (!base_freq) {
        dprintf(1, "Unknown base frequency for SD controller\n");
        वापस -1;
    }
    // Set new frequency
    u32 भाजक = DIV_ROUND_UP(base_freq * 1000, khz);
    u16 creg;
    अगर ((ver & 0xff) <= 0x01) {
        भाजक = भाजक > 1 ? 1 << __fls(भाजक-1) : 0;
        creg = (भाजक & SCC_SDCLK_MASK) << SCC_SDCLK_SHIFT;
    } अन्यथा {
        भाजक = DIV_ROUND_UP(भाजक, 2);
        creg = (भाजक & SCC_SDCLK_MASK) << SCC_SDCLK_SHIFT;
        creg |= (भाजक & SCC_SDCLK_HI_MASK) >> SCC_SDCLK_HI_RSHIFT;
    }
    dprintf(3, "sdcard_set_frequency %d %d %x\n", base_freq, khz, creg);
    writew(&regs->clock_control, 0);
    writew(&regs->clock_control, creg | SCC_INTERNAL_ENABLE);
    // Wait for frequency to become active
    पूर्णांक ret = sdcard_waitw(&regs->clock_control, SCC_STABLE);
    अगर (ret < 0)
        वापस ret;
    // Enable SD clock
    writew(&regs->clock_control, creg | SCC_INTERNAL_ENABLE | SCC_CLOCK_ENABLE);
    वापस 0;
}

// Obtain the disk size of an SD card
अटल पूर्णांक
sdcard_get_capacity(काष्ठा sddrive_s *drive, u8 *csd)
{
    // Original MMC/SD card capacity formula
    u16 C_आकार = (csd[6] >> 6) | (csd[7] << 2) | ((csd[8] & 0x03) << 10);
    u8 C_आकार_MULT = (csd[4] >> 7) | ((csd[5] & 0x03) << 1);
    u8 READ_BL_LEN = csd[9] & 0x0f;
    u32 गिनती = (C_आकार+1) << (C_आकार_MULT + 2 + READ_BL_LEN - 9);
    // Check for newer encoding formats.
    u8 CSD_STRUCTURE = csd[14] >> 6;
    अगर ((drive->card_type & SF_MMC) && CSD_STRUCTURE >= 2) {
        // Get capacity from EXT_CSD register
        u8 ext_csd[512];
        पूर्णांक ret = sdcard_pio_transfer(drive, SC_SEND_EXT_CSD, 0, ext_csd, 1);
        अगर (ret)
            वापस ret;
        गिनती = *(u32*)&ext_csd[212];
    } अन्यथा अगर (!(drive->card_type & SF_MMC) && CSD_STRUCTURE >= 1) {
        // High capacity SD card
        u32 C_SIZE2 = csd[5] | (csd[6] << 8) | ((csd[7] & 0x3f) << 16);
        गिनती = (C_SIZE2+1) << (19-9);
    }
    // Fill drive struct and return
    drive->drive.blksize = DISK_SECTOR_आकार;
    drive->drive.sectors = गिनती;
    वापस 0;
}

// Initialize an SD card
अटल पूर्णांक
sdcard_card_setup(काष्ठा sddrive_s *drive, पूर्णांक volt, पूर्णांक prio)
{
    काष्ठा sdhci_s *regs = drive->regs;
    // Set controller to initialization clock rate
    पूर्णांक ret = sdcard_set_frequency(regs, 400);
    अगर (ret)
        वापस ret;
    msleep(SDHCI_CLOCK_ON_TIME);
    // Reset card
    u32 param[4] = { };
    ret = sdcard_pio(regs, SC_GO_IDLE_STATE, param);
    अगर (ret)
        वापस ret;
    // Let card know SDHC/SDXC is supported and confirm voltage
    u32 hcs = 0, vrange = (volt >= (1<<15) ? 0x100 : 0x200) | 0xaa;
    param[0] = vrange;
    ret = sdcard_pio(regs, SC_SEND_IF_COND, param);
    अगर (!ret && param[0] == vrange)
        hcs = (1<<30);
    // Verify SD card (instead of MMC or SDIO)
    param[0] = 0x00;
    ret = sdcard_pio_app(regs, SC_APP_SEND_OP_COND, param);
    अगर (ret) {
        // Check for MMC card
        param[0] = 0x00;
        ret = sdcard_pio(regs, SC_SEND_OP_COND, param);
        अगर (ret)
            वापस ret;
        drive->card_type |= SF_MMC;
        hcs = (1<<30);
    }
    // Init card
    u32 end = timer_calc(SDHCI_POWERUP_TIMEOUT);
    क्रम (;;) {
        param[0] = hcs | volt; // high-capacity support and voltage level
        अगर (drive->card_type & SF_MMC)
            ret = sdcard_pio(regs, SC_SEND_OP_COND, param);
        अन्यथा
            ret = sdcard_pio_app(regs, SC_APP_SEND_OP_COND, param);
        अगर (ret)
            वापस ret;
        अगर (param[0] & SR_OCR_NOTBUSY)
            अवरोध;
        अगर (timer_check(end)) {
            warn_timeout();
            वापस -1;
        }
        msleep(5); // Avoid flooding log when debugging
    }
    drive->card_type |= (param[0] & SR_OCR_CCS) ? SF_HIGHCAPACITY : 0;
    // Select card (get cid, set rca, get csd, select card)
    param[0] = 0x00;
    ret = sdcard_pio(regs, SC_ALL_SEND_CID, param);
    अगर (ret)
        वापस ret;
    u8 cid[16];
    स_नकल(cid, param, माप(cid));
    param[0] = drive->card_type & SF_MMC ? 0x0001 << 16 : 0x00;
    ret = sdcard_pio(regs, SC_SEND_RELATIVE_ADDR, param);
    अगर (ret)
        वापस ret;
    u16 rca = drive->card_type & SF_MMC ? 0x0001 : param[0] >> 16;
    param[0] = rca << 16;
    ret = sdcard_pio(regs, SC_SEND_CSD, param);
    अगर (ret)
        वापस ret;
    u8 csd[16];
    स_नकल(csd, param, माप(csd));
    param[0] = rca << 16;
    ret = sdcard_pio(regs, SC_SELECT_DESELECT_CARD, param);
    अगर (ret)
        वापस ret;
    // Set controller to data transfer clock rate
    ret = sdcard_set_frequency(regs, 25000);
    अगर (ret)
        वापस ret;
    // Register drive
    ret = sdcard_get_capacity(drive, csd);
    अगर (ret)
        वापस ret;
    अक्षर pnm[7] = {};
    पूर्णांक i;
    क्रम (i=0; i < (drive->card_type & SF_MMC ? 6 : 5); i++)
        pnm[i] = cid[11-i];
    अक्षर *desc = znprintf(MAXDESCSIZE, "%s %s %dMiB"
                          , drive->card_type & SF_MMC ? "MMC drive" : "SD card"
                          , pnm, (u32)(drive->drive.sectors >> 11));
    dprintf(1, "Found sdcard at %p: %s\n", regs, desc);
    boot_add_hd(&drive->drive, desc, prio);
    वापस 0;
}

// Setup and configure an SD card controller
अटल व्योम
sdcard_controller_setup(काष्ठा sdhci_s *regs, पूर्णांक prio)
{
    // Initialize controller
    u32 present_state = readl(&regs->present_state);
    अगर (!(present_state & SP_CARD_INSERTED))
        // No card present
        वापस;
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
    पूर्णांक volt = sdcard_set_power(regs);
    अगर (volt < 0)
        वापस;

    // Initialize card
    काष्ठा sddrive_s *drive = malloc_fseg(माप(*drive));
    अगर (!drive) {
        warn_noalloc();
        जाओ fail;
    }
    स_रखो(drive, 0, माप(*drive));
    drive->drive.type = DTYPE_SDCARD;
    drive->regs = regs;
    पूर्णांक ret = sdcard_card_setup(drive, volt, prio);
    अगर (ret) {
        मुक्त(drive);
        जाओ fail;
    }
    वापस;
fail:
    writeb(&regs->power_control, 0);
    writew(&regs->clock_control, 0);
}

अटल व्योम
sdcard_pci_setup(व्योम *तथ्य)
{
    काष्ठा pci_device *pci = तथ्य;
    // XXX - bars dependent on slot index register in pci config space
    काष्ठा sdhci_s *regs = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    अगर (!regs)
        वापस;
    पूर्णांक prio = bootprio_find_pci_device(pci);
    sdcard_controller_setup(regs, prio);
}

अटल व्योम
sdcard_romfile_setup(व्योम *तथ्य)
{
    काष्ठा romfile_s *file = तथ्य;
    पूर्णांक prio = bootprio_find_named_rom(file->name, 0);
    u32 addr = romfile_loadint(file->name, 0);
    dprintf(1, "Starting sdcard controller check at addr %x\n", addr);
    sdcard_controller_setup((व्योम*)addr, prio);
}

व्योम
sdcard_setup(व्योम)
{
    अगर (!CONFIG_SDCARD)
        वापस;

    काष्ठा romfile_s *file = शून्य;
    पूर्णांक अंक_romfiles = 0;
    क्रम (;;) {
        file = romfile_findprefix("etc/sdcard", file);
        अगर (!file)
            अवरोध;
        run_thread(sdcard_romfile_setup, file);
        अंक_romfiles++;
    }
    अगर (अंक_romfiles)
        // only scan for PCI controllers if etc/sdcard not used
        वापस;

    काष्ठा pci_device *pci;
    foreachpci(pci) {
        अगर (pci->class != PCI_CLASS_SYSTEM_SDHCI || pci->prog_if >= 2)
            // Not an SDHCI controller following SDHCI spec
            जारी;
        run_thread(sdcard_pci_setup, pci);
    }
}

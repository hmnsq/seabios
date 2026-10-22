// PCI SD Host Controller Interface
//
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // struct drive_s
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // malloc_fseg
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // znprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pcidevice.h" // foreachpci
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_ids.h" // PCI_CLASS_SYSTEM_SDHCI
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ਅਨ੍ਤਰ੍ਭਾਵ "romfile.h" // romfile_findprefix
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // yield
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // DISK_RET_SUCCESS
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // boot_add_hd
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // writel

// SDHCI MMIO registers
ਬਣਤਰ sdhci_s {
    u32 sdma_addr;
    u16 ਖੰਡ_ਮਾਪ;
    u16 ਖੰਡ_ਗਿਣਤੀ;
    u32 arg;
    u16 transfer_ਬਹੁਲਕ;
    u16 cmd;
    u32 response[4];
    u32 ਅੰਕੜੇ;
    u32 present_state;
    u8 host_control;
    u8 power_control;
    u8 ਖੰਡ_gap_control;
    u8 wakeup_control;
    u16 clock_control;
    u8 timeout_control;
    u8 software_reset;
    u16 irq_ਸਥਿਤੀ;
    u16 error_irq_ਸਥਿਤੀ;
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
    u16 ਥਾਂ_irq;
    u16 controller_ਸੰਸਕਰਣ;
} PACKED;

// SDHCI commands
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCB_R0   0x00 // No response
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCB_R48  0x1a // Response R1 (no data), R5, R6, R7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCB_R48d 0x3a // Response R1 (with data)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCB_R48b 0x1b // Response R1b, R5b
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCB_R48o 0x02 // Response R3, R4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCB_R136 0x09 // Response R2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SC_GO_IDLE_STATE        ((0<<8) | SCB_R0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SC_SEND_OP_COND         ((1<<8) | SCB_R48o)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SC_ALL_SEND_CID         ((2<<8) | SCB_R136)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SC_SEND_RELATIVE_ADDR   ((3<<8) | SCB_R48)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SC_SELECT_DESELECT_CARD ((7<<8) | SCB_R48b)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SC_SEND_IF_COND         ((8<<8) | SCB_R48)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SC_SEND_EXT_CSD         ((8<<8) | SCB_R48d)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SC_SEND_CSD             ((9<<8) | SCB_R136)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SC_READ_SINGLE          ((17<<8) | SCB_R48d)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SC_READ_ਗੁਣਜ        ((18<<8) | SCB_R48d)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SC_WRITE_SINGLE         ((24<<8) | SCB_R48d)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SC_WRITE_ਗੁਣਜ       ((25<<8) | SCB_R48d)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SC_APP_CMD              ((55<<8) | SCB_R48)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SC_APP_SEND_OP_COND ((41<<8) | SCB_R48o)

// SDHCI irqs
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SI_CMD_COMPLETE (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SI_TRANS_ਪੂਰਾ   (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SI_WRITE_READY  (1<<4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SI_READ_READY   (1<<5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SI_ERROR        (1<<15)

// SDHCI present_state flags
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SP_CMD_INHIBIT   (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SP_DAT_INHIBIT   (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SP_CARD_INSERTED (1<<16)

// SDHCI transfer_mode flags
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ST_BLOCKCOUNT (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ST_AUTO_CMD12 (1<<2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ST_READ       (1<<4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ST_ਗੁਣਜ   (1<<5)

// SDHCI capabilities flags
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SD_CAPLO_V33             (1<<24)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SD_CAPLO_V30             (1<<25)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SD_CAPLO_V18             (1<<26)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SD_CAPLO_BASECLOCK_ਖਿਸਕਾਓ 8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SD_CAPLO_BASECLOCK_ਨਕਾਬ  0xff

// SDHCI clock control flags
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCC_ਅੰਦਰੂਨੀ_ENABLE (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCC_STABLE          (1<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCC_CLOCK_ENABLE    (1<<2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCC_SDCLK_ਨਕਾਬ      0xff
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCC_SDCLK_ਖਿਸਕਾਓ     8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCC_SDCLK_HI_ਨਕਾਬ   0x300
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCC_SDCLK_HI_RSHIFT 2

// SDHCI power control flags
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPC_POWER_ON (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPC_V18      0x0a
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPC_V30      0x0c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPC_V33      0x0e

// SDHCI software reset flags
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SRF_ALL  0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SRF_CMD  0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SRF_ਅੰਕੜੇ 0x04

// SDHCI result flags
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SR_OCR_CCS     (1<<30)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SR_OCR_NOTBUSY (1<<31)

// SDHCI timeouts
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SDHCI_POWER_OFF_ਸਮਾਂ   1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SDHCI_POWER_ON_ਸਮਾਂ    5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SDHCI_CLOCK_ON_ਸਮਾਂ    1 // 74 clock cycles
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SDHCI_POWERUP_TIMEOUT  1000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SDHCI_PIO_TIMEOUT      1000  // XXX - this is just made up

// Internal 'struct drive_s' storage for a detected card
ਬਣਤਰ sddrive_s {
    ਬਣਤਰ drive_s drive;
    ਬਣਤਰ sdhci_s *regs;
    ਪੂਰਨ card_type;
};

// SD card types
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_MMC          (1<<0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_HIGHCAPACITY (1<<1)

// Repeatedly read a u16 register until any bit in a given mask is set
ਸਥਿਰ ਪੂਰਨ
sdcard_waitw(u16 *reg, u16 ਨਕਾਬ)
{
    u32 ਸਿਰਾ = timer_calc(SDHCI_PIO_TIMEOUT);
    ਲਈ (;;) {
        u16 v = readw(reg);
        ਜੇ (v & ਨਕਾਬ)
            ਵਾਪਸ v;
        ਜੇ (timer_check(ਸਿਰਾ)) {
            dprintf(1, "scard_waitw: %p %x %x\n", reg, ਨਕਾਬ, v);
            warn_timeout();
            ਵਾਪਸ -1;
        }
        yield();
    }
}

// Send an sdhci reset
ਸਥਿਰ ਪੂਰਨ
sdcard_reset(ਬਣਤਰ sdhci_s *regs, ਪੂਰਨ flags)
{
    writeb(&regs->software_reset, flags);
    u32 ਸਿਰਾ = timer_calc(SDHCI_PIO_TIMEOUT);
    ਜਦੋਂ (readb(&regs->software_reset))
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            ਵਾਪਸ -1;
        }
    ਵਾਪਸ 0;
}

// Send a command to the card.
ਸਥਿਰ ਪੂਰਨ
sdcard_pio(ਬਣਤਰ sdhci_s *regs, ਪੂਰਨ cmd, u32 *param)
{
    u32 state = readl(&regs->present_state);
    dprintf(9, "sdcard_pio cmd %x %x %x\n", cmd, *param, state);
    ਜੇ ((state & SP_CMD_INHIBIT)
        || ((cmd & 0x03) == 0x03 && state & SP_DAT_INHIBIT)) {
        dprintf(1, "sdcard_pio not ready %x\n", state);
        ਵਾਪਸ -1;
    }
    // Send command
    writel(&regs->arg, *param);
    writew(&regs->cmd, cmd);
    ਪੂਰਨ ret = sdcard_waitw(&regs->irq_ਸਥਿਤੀ, SI_ERROR|SI_CMD_COMPLETE);
    ਜੇ (ret < 0)
        ਵਾਪਸ ret;
    ਜੇ (ret & SI_ERROR) {
        u16 err = readw(&regs->error_irq_ਸਥਿਤੀ);
        dprintf(3, "sdcard_pio command stop (code=%x)\n", err);
        sdcard_reset(regs, SRF_CMD|SRF_ਅੰਕੜੇ);
        writew(&regs->error_irq_ਸਥਿਤੀ, err);
        ਵਾਪਸ -1;
    }
    writew(&regs->irq_ਸਥਿਤੀ, SI_CMD_COMPLETE);
    // Read response
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(param, regs->response, ਆਕਾਰ(regs->response));
    dprintf(9, "sdcard cmd %x response %x %x %x %x\n"
            , cmd, param[0], param[1], param[2], param[3]);
    ਵਾਪਸ 0;
}

// Send an "app specific" command to the card.
ਸਥਿਰ ਪੂਰਨ
sdcard_pio_app(ਬਣਤਰ sdhci_s *regs, ਪੂਰਨ cmd, u32 *param)
{
    u32 aparam[4] = {};
    ਪੂਰਨ ret = sdcard_pio(regs, SC_APP_CMD, aparam);
    ਜੇ (ret)
        ਵਾਪਸ ret;
    ਵਾਪਸ sdcard_pio(regs, cmd, param);
}

// Send a command to the card which transfers data.
ਸਥਿਰ ਪੂਰਨ
sdcard_pio_transfer(ਬਣਤਰ sddrive_s *drive, ਪੂਰਨ cmd, u32 addr
                    , ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ ਗਿਣਤੀ_2)
{
    // Send command
    writew(&drive->regs->ਖੰਡ_ਮਾਪ, DISK_SECTOR_ਮਾਪ);
    writew(&drive->regs->ਖੰਡ_ਗਿਣਤੀ, ਗਿਣਤੀ_2);
    ਪੂਰਨ isread = cmd != SC_WRITE_SINGLE && cmd != SC_WRITE_ਗੁਣਜ;
    u16 tmode = ((ਗਿਣਤੀ_2 > 1 ? ST_ਗੁਣਜ|ST_AUTO_CMD12|ST_BLOCKCOUNT : 0)
                 | (isread ? ST_READ : 0));
    writew(&drive->regs->transfer_ਬਹੁਲਕ, tmode);
    ਜੇ (!(drive->card_type & SF_HIGHCAPACITY))
        addr *= DISK_SECTOR_ਮਾਪ;
    u32 param[4] = { addr };
    ਪੂਰਨ ret = sdcard_pio(drive->regs, cmd, param);
    ਜੇ (ret)
        ਵਾਪਸ ret;
    // Read/write data
    u16 cbit = isread ? SI_READ_READY : SI_WRITE_READY;
    ਜਦੋਂ (ਗਿਣਤੀ_2--) {
        ret = sdcard_waitw(&drive->regs->irq_ਸਥਿਤੀ, cbit);
        ਜੇ (ret < 0)
            ਵਾਪਸ ret;
        writew(&drive->regs->irq_ਸਥਿਤੀ, cbit);
        ਪੂਰਨ i;
        ਲਈ (i=0; i<DISK_SECTOR_ਮਾਪ/4; i++) {
            ਜੇ (isread)
                *(u32*)ਅੰਕੜੇ = readl(&drive->regs->ਅੰਕੜੇ);
            ਨਹੀਂਤਾਂ
                writel(&drive->regs->ਅੰਕੜੇ, *(u32*)ਅੰਕੜੇ);
            ਅੰਕੜੇ += 4;
        }
    }
    // Complete command
    ret = sdcard_waitw(&drive->regs->irq_ਸਥਿਤੀ, SI_TRANS_ਪੂਰਾ);
    ਜੇ (ret < 0)
        ਵਾਪਸ ret;
    writew(&drive->regs->irq_ਸਥਿਤੀ, SI_TRANS_ਪੂਰਾ);
    ਵਾਪਸ 0;
}

// Read/write a block of data to/from the card.
ਸਥਿਰ ਪੂਰਨ
sdcard_readwrite(ਬਣਤਰ disk_op_s *op, ਪੂਰਨ iswrite)
{
    ਬਣਤਰ sddrive_s *drive = container_ਦਾ(
        op->drive_fl, ਬਣਤਰ sddrive_s, drive);
    ਪੂਰਨ cmd = iswrite ? SC_WRITE_SINGLE : SC_READ_SINGLE;
    ਜੇ (op->ਗਿਣਤੀ_2 > 1)
        cmd = iswrite ? SC_WRITE_ਗੁਣਜ : SC_READ_ਗੁਣਜ;
    ਪੂਰਨ ret = sdcard_pio_transfer(drive, cmd, op->lba, op->buf_fl, op->ਗਿਣਤੀ_2);
    ਜੇ (ret)
        ਵਾਪਸ DISK_RET_EBADTRACK;
    ਵਾਪਸ DISK_RET_SUCCESS;
}

ਪੂਰਨ
sdcard_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (!CONFIG_SDCARD)
        ਵਾਪਸ 0;
    ਚੋਣ (op->command) {
    ਹਾਲਤ CMD_READ:
        ਵਾਪਸ sdcard_readwrite(op, 0);
    ਹਾਲਤ CMD_WRITE:
        ਵਾਪਸ sdcard_readwrite(op, 1);
    ਮੂਲ:
        ਵਾਪਸ default_process_op(op);
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

ਸਥਿਰ ਪੂਰਨ
sdcard_ਸੈੱਟ_power(ਬਣਤਰ sdhci_s *regs)
{
    u32 cap = readl(&regs->cap_lo);
    u32 volt, vbits;
    ਜੇ (cap & SD_CAPLO_V33) {
        volt = 1<<20;
        vbits = SPC_V33;
    } ਨਹੀਂਤਾਂ ਜੇ (cap & SD_CAPLO_V30) {
        volt = 1<<18;
        vbits = SPC_V30;
    } ਨਹੀਂਤਾਂ ਜੇ (cap & SD_CAPLO_V18) {
        volt = 1<<7;
        vbits = SPC_V18;
    } ਨਹੀਂਤਾਂ {
        dprintf(1, "SD controller unsupported volt range (%x)\n", cap);
        ਵਾਪਸ -1;
    }
    writeb(&regs->power_control, 0);
    msleep(SDHCI_POWER_OFF_ਸਮਾਂ);
    writeb(&regs->power_control, vbits | SPC_POWER_ON);
    msleep(SDHCI_POWER_ON_ਸਮਾਂ);
    ਵਾਪਸ volt;
}

ਸਥਿਰ ਪੂਰਨ
sdcard_ਸੈੱਟ_frequency(ਬਣਤਰ sdhci_s *regs, u32 khz)
{
    u16 ver = readw(&regs->controller_ਸੰਸਕਰਣ);
    u32 cap = readl(&regs->cap_lo);
    u32 ਅਧਾਰ_freq = (cap >> SD_CAPLO_BASECLOCK_ਖਿਸਕਾਓ) & SD_CAPLO_BASECLOCK_ਨਕਾਬ;
    ਜੇ (!ਅਧਾਰ_freq) {
        dprintf(1, "Unknown base frequency for SD controller\n");
        ਵਾਪਸ -1;
    }
    // Set new frequency
    u32 ਭਾਜਕ = DIV_ਗੇੜਾ_ਉੱਪਰ(ਅਧਾਰ_freq * 1000, khz);
    u16 creg;
    ਜੇ ((ver & 0xff) <= 0x01) {
        ਭਾਜਕ = ਭਾਜਕ > 1 ? 1 << __fls(ਭਾਜਕ-1) : 0;
        creg = (ਭਾਜਕ & SCC_SDCLK_ਨਕਾਬ) << SCC_SDCLK_ਖਿਸਕਾਓ;
    } ਨਹੀਂਤਾਂ {
        ਭਾਜਕ = DIV_ਗੇੜਾ_ਉੱਪਰ(ਭਾਜਕ, 2);
        creg = (ਭਾਜਕ & SCC_SDCLK_ਨਕਾਬ) << SCC_SDCLK_ਖਿਸਕਾਓ;
        creg |= (ਭਾਜਕ & SCC_SDCLK_HI_ਨਕਾਬ) >> SCC_SDCLK_HI_RSHIFT;
    }
    dprintf(3, "sdcard_set_frequency %d %d %x\n", ਅਧਾਰ_freq, khz, creg);
    writew(&regs->clock_control, 0);
    writew(&regs->clock_control, creg | SCC_ਅੰਦਰੂਨੀ_ENABLE);
    // Wait for frequency to become active
    ਪੂਰਨ ret = sdcard_waitw(&regs->clock_control, SCC_STABLE);
    ਜੇ (ret < 0)
        ਵਾਪਸ ret;
    // Enable SD clock
    writew(&regs->clock_control, creg | SCC_ਅੰਦਰੂਨੀ_ENABLE | SCC_CLOCK_ENABLE);
    ਵਾਪਸ 0;
}

// Obtain the disk size of an SD card
ਸਥਿਰ ਪੂਰਨ
sdcard_get_ਸਮਰੱਥਾ(ਬਣਤਰ sddrive_s *drive, u8 *csd)
{
    // Original MMC/SD card capacity formula
    u16 C_ਮਾਪ = (csd[6] >> 6) | (csd[7] << 2) | ((csd[8] & 0x03) << 10);
    u8 C_ਮਾਪ_MULT = (csd[4] >> 7) | ((csd[5] & 0x03) << 1);
    u8 READ_BL_LEN = csd[9] & 0x0f;
    u32 ਗਿਣਤੀ_2 = (C_ਮਾਪ+1) << (C_ਮਾਪ_MULT + 2 + READ_BL_LEN - 9);
    // Check for newer encoding formats.
    u8 CSD_STRUCTURE = csd[14] >> 6;
    ਜੇ ((drive->card_type & SF_MMC) && CSD_STRUCTURE >= 2) {
        // Get capacity from EXT_CSD register
        u8 ext_csd[512];
        ਪੂਰਨ ret = sdcard_pio_transfer(drive, SC_SEND_EXT_CSD, 0, ext_csd, 1);
        ਜੇ (ret)
            ਵਾਪਸ ret;
        ਗਿਣਤੀ_2 = *(u32*)&ext_csd[212];
    } ਨਹੀਂਤਾਂ ਜੇ (!(drive->card_type & SF_MMC) && CSD_STRUCTURE >= 1) {
        // High capacity SD card
        u32 C_SIZE2 = csd[5] | (csd[6] << 8) | ((csd[7] & 0x3f) << 16);
        ਗਿਣਤੀ_2 = (C_SIZE2+1) << (19-9);
    }
    // Fill drive struct and return
    drive->drive.blksize = DISK_SECTOR_ਮਾਪ;
    drive->drive.sectors = ਗਿਣਤੀ_2;
    ਵਾਪਸ 0;
}

// Initialize an SD card
ਸਥਿਰ ਪੂਰਨ
sdcard_card_ਤਿਆਰੀ(ਬਣਤਰ sddrive_s *drive, ਪੂਰਨ volt, ਪੂਰਨ prio)
{
    ਬਣਤਰ sdhci_s *regs = drive->regs;
    // Set controller to initialization clock rate
    ਪੂਰਨ ret = sdcard_ਸੈੱਟ_frequency(regs, 400);
    ਜੇ (ret)
        ਵਾਪਸ ret;
    msleep(SDHCI_CLOCK_ON_ਸਮਾਂ);
    // Reset card
    u32 param[4] = { };
    ret = sdcard_pio(regs, SC_GO_IDLE_STATE, param);
    ਜੇ (ret)
        ਵਾਪਸ ret;
    // Let card know SDHC/SDXC is supported and confirm voltage
    u32 hcs = 0, vrange = (volt >= (1<<15) ? 0x100 : 0x200) | 0xaa;
    param[0] = vrange;
    ret = sdcard_pio(regs, SC_SEND_IF_COND, param);
    ਜੇ (!ret && param[0] == vrange)
        hcs = (1<<30);
    // Verify SD card (instead of MMC or SDIO)
    param[0] = 0x00;
    ret = sdcard_pio_app(regs, SC_APP_SEND_OP_COND, param);
    ਜੇ (ret) {
        // Check for MMC card
        param[0] = 0x00;
        ret = sdcard_pio(regs, SC_SEND_OP_COND, param);
        ਜੇ (ret)
            ਵਾਪਸ ret;
        drive->card_type |= SF_MMC;
        hcs = (1<<30);
    }
    // Init card
    u32 ਸਿਰਾ = timer_calc(SDHCI_POWERUP_TIMEOUT);
    ਲਈ (;;) {
        param[0] = hcs | volt; // high-capacity support and voltage level
        ਜੇ (drive->card_type & SF_MMC)
            ret = sdcard_pio(regs, SC_SEND_OP_COND, param);
        ਨਹੀਂਤਾਂ
            ret = sdcard_pio_app(regs, SC_APP_SEND_OP_COND, param);
        ਜੇ (ret)
            ਵਾਪਸ ret;
        ਜੇ (param[0] & SR_OCR_NOTBUSY)
            ਰੋਕੋ;
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            ਵਾਪਸ -1;
        }
        msleep(5); // Avoid flooding log when debugging
    }
    drive->card_type |= (param[0] & SR_OCR_CCS) ? SF_HIGHCAPACITY : 0;
    // Select card (get cid, set rca, get csd, select card)
    param[0] = 0x00;
    ret = sdcard_pio(regs, SC_ALL_SEND_CID, param);
    ਜੇ (ret)
        ਵਾਪਸ ret;
    u8 cid[16];
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(cid, param, ਆਕਾਰ(cid));
    param[0] = drive->card_type & SF_MMC ? 0x0001 << 16 : 0x00;
    ret = sdcard_pio(regs, SC_SEND_RELATIVE_ADDR, param);
    ਜੇ (ret)
        ਵਾਪਸ ret;
    u16 rca = drive->card_type & SF_MMC ? 0x0001 : param[0] >> 16;
    param[0] = rca << 16;
    ret = sdcard_pio(regs, SC_SEND_CSD, param);
    ਜੇ (ret)
        ਵਾਪਸ ret;
    u8 csd[16];
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(csd, param, ਆਕਾਰ(csd));
    param[0] = rca << 16;
    ret = sdcard_pio(regs, SC_SELECT_DESELECT_CARD, param);
    ਜੇ (ret)
        ਵਾਪਸ ret;
    // Set controller to data transfer clock rate
    ret = sdcard_ਸੈੱਟ_frequency(regs, 25000);
    ਜੇ (ret)
        ਵਾਪਸ ret;
    // Register drive
    ret = sdcard_get_ਸਮਰੱਥਾ(drive, csd);
    ਜੇ (ret)
        ਵਾਪਸ ret;
    ਅੱਖਰ pnm[7] = {};
    ਪੂਰਨ i;
    ਲਈ (i=0; i < (drive->card_type & SF_MMC ? 6 : 5); i++)
        pnm[i] = cid[11-i];
    ਅੱਖਰ *desc = znprintf(MAXDESCSIZE, "%s %s %dMiB"
                          , drive->card_type & SF_MMC ? "MMC drive" : "SD card"
                          , pnm, (u32)(drive->drive.sectors >> 11));
    dprintf(1, "Found sdcard at %p: %s\n", regs, desc);
    boot_ਜੋੜੋ_hd(&drive->drive, desc, prio);
    ਵਾਪਸ 0;
}

// Setup and configure an SD card controller
ਸਥਿਰ ਖਾਲੀ
sdcard_controller_ਤਿਆਰੀ(ਬਣਤਰ sdhci_s *regs, ਪੂਰਨ prio)
{
    // Initialize controller
    u32 present_state = readl(&regs->present_state);
    ਜੇ (!(present_state & SP_CARD_INSERTED))
        // No card present
        ਵਾਪਸ;
    dprintf(3, "sdhci@%p ver=%x cap=%x %x\n", regs
            , readw(&regs->controller_ਸੰਸਕਰਣ)
            , readl(&regs->cap_lo), readl(&regs->cap_hi));
    sdcard_reset(regs, SRF_ALL);
    writew(&regs->irq_signal, 0);
    writew(&regs->irq_enable, 0x01ff);
    writew(&regs->irq_ਸਥਿਤੀ, readw(&regs->irq_ਸਥਿਤੀ));
    writew(&regs->error_signal, 0);
    writew(&regs->error_irq_enable, 0x01ff);
    writew(&regs->error_irq_ਸਥਿਤੀ, readw(&regs->error_irq_ਸਥਿਤੀ));
    writeb(&regs->timeout_control, 0x0e); // Set to max timeout
    ਪੂਰਨ volt = sdcard_ਸੈੱਟ_power(regs);
    ਜੇ (volt < 0)
        ਵਾਪਸ;

    // Initialize card
    ਬਣਤਰ sddrive_s *drive = malloc_fseg(ਆਕਾਰ(*drive));
    ਜੇ (!drive) {
        warn_noalloc();
        ਜਾਓ fail;
    }
    memset(drive, 0, ਆਕਾਰ(*drive));
    drive->drive.type = DTYPE_SDCARD;
    drive->regs = regs;
    ਪੂਰਨ ret = sdcard_card_ਤਿਆਰੀ(drive, volt, prio);
    ਜੇ (ret) {
        ਮੁਕਤ(drive);
        ਜਾਓ fail;
    }
    ਵਾਪਸ;
fail:
    writeb(&regs->power_control, 0);
    writew(&regs->clock_control, 0);
}

ਸਥਿਰ ਖਾਲੀ
sdcard_pci_ਤਿਆਰੀ(ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਬਣਤਰ pci_device *pci = ਅੰਕੜੇ;
    // XXX - bars dependent on slot index register in pci config space
    ਬਣਤਰ sdhci_s *regs = pci_enable_membar(pci, PCI_ਅਧਾਰ_ਪਤਾ_0);
    ਜੇ (!regs)
        ਵਾਪਸ;
    ਪੂਰਨ prio = bootprio_ਲੱਭੋ_pci_device(pci);
    sdcard_controller_ਤਿਆਰੀ(regs, prio);
}

ਸਥਿਰ ਖਾਲੀ
sdcard_romfile_ਤਿਆਰੀ(ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਬਣਤਰ romfile_s *file = ਅੰਕੜੇ;
    ਪੂਰਨ prio = bootprio_ਲੱਭੋ_named_rom(file->name, 0);
    u32 addr = romfile_loadint(file->name, 0);
    dprintf(1, "Starting sdcard controller check at addr %x\n", addr);
    sdcard_controller_ਤਿਆਰੀ((ਖਾਲੀ*)addr, prio);
}

ਖਾਲੀ
sdcard_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ਜੇ (!CONFIG_SDCARD)
        ਵਾਪਸ;

    ਬਣਤਰ romfile_s *file = ਨਿਰਾ;
    ਪੂਰਨ ਅੰਕ_romfiles = 0;
    ਲਈ (;;) {
        file = romfile_findprefix("etc/sdcard", file);
        ਜੇ (!file)
            ਰੋਕੋ;
        ਲੜੀਵਾਰ_thread(sdcard_romfile_ਤਿਆਰੀ, file);
        ਅੰਕ_romfiles++;
    }
    ਜੇ (ਅੰਕ_romfiles)
        // only scan for PCI controllers if etc/sdcard not used
        ਵਾਪਸ;

    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (pci->class != PCI_CLASS_SYSTEM_SDHCI || pci->prog_if >= 2)
            // Not an SDHCI controller following SDHCI spec
            ਜਾਰੀ;
        ਲੜੀਵਾਰ_thread(sdcard_pci_ਤਿਆਰੀ, pci);
    }
}

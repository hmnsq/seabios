// PCI SD Host Controller Interface
//
// Copyright (C) 2014  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "block.h" // struct drive_s
#ಅನ್ತರ್ಭಾವ "malloc.h" // malloc_fseg
#ಅನ್ತರ್ಭಾವ "output.h" // znprintf
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "pci_ids.h" // PCI_CLASS_SYSTEM_SDHCI
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ಅನ್ತರ್ಭಾವ "romfile.h" // romfile_findprefix
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_SUCCESS
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // boot_add_hd
#ಅನ್ತರ್ಭಾವ "x86.h" // writel

// SDHCI MMIO registers
ರಚನೆ sdhci_s {
    u32 sdma_addr;
    u16 ಖಂಡ_ಗಾತ್ರ;
    u16 ಖಂಡ_ಎಣಿಕೆ;
    u32 arg;
    u16 transfer_ಬಹುಲಕ;
    u16 cmd;
    u32 response[4];
    u32 ದತ್ತಾಂಶ;
    u32 present_state;
    u8 host_control;
    u8 power_control;
    u8 ಖಂಡ_gap_control;
    u8 wakeup_control;
    u16 clock_control;
    u8 timeout_control;
    u8 software_reset;
    u16 irq_ಸ್ಥಿತಿ;
    u16 error_irq_ಸ್ಥಿತಿ;
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
    u16 ಸ್ಥಾನ_irq;
    u16 controller_ಆವೃತ್ತಿ;
} PACKED;

// SDHCI commands
#ವ್ಯಾಖ್ಯಾನಿಸು SCB_R0   0x00 // No response
#ವ್ಯಾಖ್ಯಾನಿಸು SCB_R48  0x1a // Response R1 (no data), R5, R6, R7
#ವ್ಯಾಖ್ಯಾನಿಸು SCB_R48d 0x3a // Response R1 (with data)
#ವ್ಯಾಖ್ಯಾನಿಸು SCB_R48b 0x1b // Response R1b, R5b
#ವ್ಯಾಖ್ಯಾನಿಸು SCB_R48o 0x02 // Response R3, R4
#ವ್ಯಾಖ್ಯಾನಿಸು SCB_R136 0x09 // Response R2
#ವ್ಯಾಖ್ಯಾನಿಸು SC_GO_IDLE_STATE        ((0<<8) | SCB_R0)
#ವ್ಯಾಖ್ಯಾನಿಸು SC_SEND_OP_COND         ((1<<8) | SCB_R48o)
#ವ್ಯಾಖ್ಯಾನಿಸು SC_ALL_SEND_CID         ((2<<8) | SCB_R136)
#ವ್ಯಾಖ್ಯಾನಿಸು SC_SEND_RELATIVE_ADDR   ((3<<8) | SCB_R48)
#ವ್ಯಾಖ್ಯಾನಿಸು SC_SELECT_DESELECT_CARD ((7<<8) | SCB_R48b)
#ವ್ಯಾಖ್ಯಾನಿಸು SC_SEND_IF_COND         ((8<<8) | SCB_R48)
#ವ್ಯಾಖ್ಯಾನಿಸು SC_SEND_EXT_CSD         ((8<<8) | SCB_R48d)
#ವ್ಯಾಖ್ಯಾನಿಸು SC_SEND_CSD             ((9<<8) | SCB_R136)
#ವ್ಯಾಖ್ಯಾನಿಸು SC_READ_SINGLE          ((17<<8) | SCB_R48d)
#ವ್ಯಾಖ್ಯಾನಿಸು SC_READ_ಅಪವರ್ತ್ಯ        ((18<<8) | SCB_R48d)
#ವ್ಯಾಖ್ಯಾನಿಸು SC_WRITE_SINGLE         ((24<<8) | SCB_R48d)
#ವ್ಯಾಖ್ಯಾನಿಸು SC_WRITE_ಅಪವರ್ತ್ಯ       ((25<<8) | SCB_R48d)
#ವ್ಯಾಖ್ಯಾನಿಸು SC_APP_CMD              ((55<<8) | SCB_R48)
#ವ್ಯಾಖ್ಯಾನಿಸು SC_APP_SEND_OP_COND ((41<<8) | SCB_R48o)

// SDHCI irqs
#ವ್ಯಾಖ್ಯಾನಿಸು SI_CMD_COMPLETE (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು SI_TRANS_ಮುಗಿದ   (1<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು SI_WRITE_READY  (1<<4)
#ವ್ಯಾಖ್ಯಾನಿಸು SI_READ_READY   (1<<5)
#ವ್ಯಾಖ್ಯಾನಿಸು SI_ERROR        (1<<15)

// SDHCI present_state flags
#ವ್ಯಾಖ್ಯಾನಿಸು SP_CMD_INHIBIT   (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು SP_DAT_INHIBIT   (1<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು SP_CARD_INSERTED (1<<16)

// SDHCI transfer_mode flags
#ವ್ಯಾಖ್ಯಾನಿಸು ST_BLOCKCOUNT (1<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು ST_AUTO_CMD12 (1<<2)
#ವ್ಯಾಖ್ಯಾನಿಸು ST_READ       (1<<4)
#ವ್ಯಾಖ್ಯಾನಿಸು ST_ಅಪವರ್ತ್ಯ   (1<<5)

// SDHCI capabilities flags
#ವ್ಯಾಖ್ಯಾನಿಸು SD_CAPLO_V33             (1<<24)
#ವ್ಯಾಖ್ಯಾನಿಸು SD_CAPLO_V30             (1<<25)
#ವ್ಯಾಖ್ಯಾನಿಸು SD_CAPLO_V18             (1<<26)
#ವ್ಯಾಖ್ಯಾನಿಸು SD_CAPLO_BASECLOCK_ಸರಿಸು 8
#ವ್ಯಾಖ್ಯಾನಿಸು SD_CAPLO_BASECLOCK_ಮುಖವಾಡ  0xff

// SDHCI clock control flags
#ವ್ಯಾಖ್ಯಾನಿಸು SCC_ಆಂತರಿಕ_ENABLE (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು SCC_STABLE          (1<<1)
#ವ್ಯಾಖ್ಯಾನಿಸು SCC_CLOCK_ENABLE    (1<<2)
#ವ್ಯಾಖ್ಯಾನಿಸು SCC_SDCLK_ಮುಖವಾಡ      0xff
#ವ್ಯಾಖ್ಯಾನಿಸು SCC_SDCLK_ಸರಿಸು     8
#ವ್ಯಾಖ್ಯಾನಿಸು SCC_SDCLK_HI_ಮುಖವಾಡ   0x300
#ವ್ಯಾಖ್ಯಾನಿಸು SCC_SDCLK_HI_RSHIFT 2

// SDHCI power control flags
#ವ್ಯಾಖ್ಯಾನಿಸು SPC_POWER_ON (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು SPC_V18      0x0a
#ವ್ಯಾಖ್ಯಾನಿಸು SPC_V30      0x0c
#ವ್ಯಾಖ್ಯಾನಿಸು SPC_V33      0x0e

// SDHCI software reset flags
#ವ್ಯಾಖ್ಯಾನಿಸು SRF_ALL  0x01
#ವ್ಯಾಖ್ಯಾನಿಸು SRF_CMD  0x02
#ವ್ಯಾಖ್ಯಾನಿಸು SRF_ದತ್ತಾಂಶ 0x04

// SDHCI result flags
#ವ್ಯಾಖ್ಯಾನಿಸು SR_OCR_CCS     (1<<30)
#ವ್ಯಾಖ್ಯಾನಿಸು SR_OCR_NOTBUSY (1<<31)

// SDHCI timeouts
#ವ್ಯಾಖ್ಯಾನಿಸು SDHCI_POWER_OFF_ಸಮಯ   1
#ವ್ಯಾಖ್ಯಾನಿಸು SDHCI_POWER_ON_ಸಮಯ    5
#ವ್ಯಾಖ್ಯಾನಿಸು SDHCI_CLOCK_ON_ಸಮಯ    1 // 74 clock cycles
#ವ್ಯಾಖ್ಯಾನಿಸು SDHCI_POWERUP_TIMEOUT  1000
#ವ್ಯಾಖ್ಯಾನಿಸು SDHCI_PIO_TIMEOUT      1000  // XXX - this is just made up

// Internal 'struct drive_s' storage for a detected card
ರಚನೆ sddrive_s {
    ರಚನೆ drive_s drive;
    ರಚನೆ sdhci_s *regs;
    ಪೂರ್ಣಾಂಕ card_type;
};

// SD card types
#ವ್ಯಾಖ್ಯಾನಿಸು SF_MMC          (1<<0)
#ವ್ಯಾಖ್ಯಾನಿಸು SF_HIGHCAPACITY (1<<1)

// Repeatedly read a u16 register until any bit in a given mask is set
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
sdcard_waitw(u16 *reg, u16 ಮುಖವಾಡ)
{
    u32 ಕೊನೆ = timer_calc(SDHCI_PIO_TIMEOUT);
    ಗಾಗಿ (;;) {
        u16 v = readw(reg);
        ಆದರೆ (v & ಮುಖವಾಡ)
            ಹಿಂತಿರುಗಿ v;
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            dprintf(1, "scard_waitw: %p %x %x\n", reg, ಮುಖವಾಡ, v);
            warn_timeout();
            ಹಿಂತಿರುಗಿ -1;
        }
        yield();
    }
}

// Send an sdhci reset
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
sdcard_reset(ರಚನೆ sdhci_s *regs, ಪೂರ್ಣಾಂಕ flags)
{
    writeb(&regs->software_reset, flags);
    u32 ಕೊನೆ = timer_calc(SDHCI_PIO_TIMEOUT);
    ಆಗ (readb(&regs->software_reset))
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            ಹಿಂತಿರುಗಿ -1;
        }
    ಹಿಂತಿರುಗಿ 0;
}

// Send a command to the card.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
sdcard_pio(ರಚನೆ sdhci_s *regs, ಪೂರ್ಣಾಂಕ cmd, u32 *param)
{
    u32 state = readl(&regs->present_state);
    dprintf(9, "sdcard_pio cmd %x %x %x\n", cmd, *param, state);
    ಆದರೆ ((state & SP_CMD_INHIBIT)
        || ((cmd & 0x03) == 0x03 && state & SP_DAT_INHIBIT)) {
        dprintf(1, "sdcard_pio not ready %x\n", state);
        ಹಿಂತಿರುಗಿ -1;
    }
    // Send command
    writel(&regs->arg, *param);
    writew(&regs->cmd, cmd);
    ಪೂರ್ಣಾಂಕ ret = sdcard_waitw(&regs->irq_ಸ್ಥಿತಿ, SI_ERROR|SI_CMD_COMPLETE);
    ಆದರೆ (ret < 0)
        ಹಿಂತಿರುಗಿ ret;
    ಆದರೆ (ret & SI_ERROR) {
        u16 err = readw(&regs->error_irq_ಸ್ಥಿತಿ);
        dprintf(3, "sdcard_pio command stop (code=%x)\n", err);
        sdcard_reset(regs, SRF_CMD|SRF_ದತ್ತಾಂಶ);
        writew(&regs->error_irq_ಸ್ಥಿತಿ, err);
        ಹಿಂತಿರುಗಿ -1;
    }
    writew(&regs->irq_ಸ್ಥಿತಿ, SI_CMD_COMPLETE);
    // Read response
    ಸ್ಮರಣೆ_ನಕಲು(param, regs->response, ಗಾತ್ರ(regs->response));
    dprintf(9, "sdcard cmd %x response %x %x %x %x\n"
            , cmd, param[0], param[1], param[2], param[3]);
    ಹಿಂತಿರುಗಿ 0;
}

// Send an "app specific" command to the card.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
sdcard_pio_app(ರಚನೆ sdhci_s *regs, ಪೂರ್ಣಾಂಕ cmd, u32 *param)
{
    u32 aparam[4] = {};
    ಪೂರ್ಣಾಂಕ ret = sdcard_pio(regs, SC_APP_CMD, aparam);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;
    ಹಿಂತಿರುಗಿ sdcard_pio(regs, cmd, param);
}

// Send a command to the card which transfers data.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
sdcard_pio_transfer(ರಚನೆ sddrive_s *drive, ಪೂರ್ಣಾಂಕ cmd, u32 addr
                    , ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2)
{
    // Send command
    writew(&drive->regs->ಖಂಡ_ಗಾತ್ರ, DISK_SECTOR_ಗಾತ್ರ);
    writew(&drive->regs->ಖಂಡ_ಎಣಿಕೆ, ಎಣಿಕೆ_2);
    ಪೂರ್ಣಾಂಕ isread = cmd != SC_WRITE_SINGLE && cmd != SC_WRITE_ಅಪವರ್ತ್ಯ;
    u16 tmode = ((ಎಣಿಕೆ_2 > 1 ? ST_ಅಪವರ್ತ್ಯ|ST_AUTO_CMD12|ST_BLOCKCOUNT : 0)
                 | (isread ? ST_READ : 0));
    writew(&drive->regs->transfer_ಬಹುಲಕ, tmode);
    ಆದರೆ (!(drive->card_type & SF_HIGHCAPACITY))
        addr *= DISK_SECTOR_ಗಾತ್ರ;
    u32 param[4] = { addr };
    ಪೂರ್ಣಾಂಕ ret = sdcard_pio(drive->regs, cmd, param);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;
    // Read/write data
    u16 cbit = isread ? SI_READ_READY : SI_WRITE_READY;
    ಆಗ (ಎಣಿಕೆ_2--) {
        ret = sdcard_waitw(&drive->regs->irq_ಸ್ಥಿತಿ, cbit);
        ಆದರೆ (ret < 0)
            ಹಿಂತಿರುಗಿ ret;
        writew(&drive->regs->irq_ಸ್ಥಿತಿ, cbit);
        ಪೂರ್ಣಾಂಕ i;
        ಗಾಗಿ (i=0; i<DISK_SECTOR_ಗಾತ್ರ/4; i++) {
            ಆದರೆ (isread)
                *(u32*)ದತ್ತಾಂಶ = readl(&drive->regs->ದತ್ತಾಂಶ);
            ಇಲ್ಲದಿದ್ದರೆ
                writel(&drive->regs->ದತ್ತಾಂಶ, *(u32*)ದತ್ತಾಂಶ);
            ದತ್ತಾಂಶ += 4;
        }
    }
    // Complete command
    ret = sdcard_waitw(&drive->regs->irq_ಸ್ಥಿತಿ, SI_TRANS_ಮುಗಿದ);
    ಆದರೆ (ret < 0)
        ಹಿಂತಿರುಗಿ ret;
    writew(&drive->regs->irq_ಸ್ಥಿತಿ, SI_TRANS_ಮುಗಿದ);
    ಹಿಂತಿರುಗಿ 0;
}

// Read/write a block of data to/from the card.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
sdcard_readwrite(ರಚನೆ disk_op_s *op, ಪೂರ್ಣಾಂಕ iswrite)
{
    ರಚನೆ sddrive_s *drive = container_ದ(
        op->drive_fl, ರಚನೆ sddrive_s, drive);
    ಪೂರ್ಣಾಂಕ cmd = iswrite ? SC_WRITE_SINGLE : SC_READ_SINGLE;
    ಆದರೆ (op->ಎಣಿಕೆ_2 > 1)
        cmd = iswrite ? SC_WRITE_ಅಪವರ್ತ್ಯ : SC_READ_ಅಪವರ್ತ್ಯ;
    ಪೂರ್ಣಾಂಕ ret = sdcard_pio_transfer(drive, cmd, op->lba, op->buf_fl, op->ಎಣಿಕೆ_2);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}

ಪೂರ್ಣಾಂಕ
sdcard_process_op(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (!CONFIG_SDCARD)
        ಹಿಂತಿರುಗಿ 0;
    ಆಯ್ಕೆ (op->command) {
    ಸ್ಥಿತಿ CMD_READ:
        ಹಿಂತಿರುಗಿ sdcard_readwrite(op, 0);
    ಸ್ಥಿತಿ CMD_WRITE:
        ಹಿಂತಿರುಗಿ sdcard_readwrite(op, 1);
    ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ default_process_op(op);
    }
}


/****************************************************************
 * Setup
 ****************************************************************/

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
sdcard_ಗಣ_power(ರಚನೆ sdhci_s *regs)
{
    u32 cap = readl(&regs->cap_lo);
    u32 volt, vbits;
    ಆದರೆ (cap & SD_CAPLO_V33) {
        volt = 1<<20;
        vbits = SPC_V33;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cap & SD_CAPLO_V30) {
        volt = 1<<18;
        vbits = SPC_V30;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cap & SD_CAPLO_V18) {
        volt = 1<<7;
        vbits = SPC_V18;
    } ಇಲ್ಲದಿದ್ದರೆ {
        dprintf(1, "SD controller unsupported volt range (%x)\n", cap);
        ಹಿಂತಿರುಗಿ -1;
    }
    writeb(&regs->power_control, 0);
    msleep(SDHCI_POWER_OFF_ಸಮಯ);
    writeb(&regs->power_control, vbits | SPC_POWER_ON);
    msleep(SDHCI_POWER_ON_ಸಮಯ);
    ಹಿಂತಿರುಗಿ volt;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
sdcard_ಗಣ_frequency(ರಚನೆ sdhci_s *regs, u32 khz)
{
    u16 ver = readw(&regs->controller_ಆವೃತ್ತಿ);
    u32 cap = readl(&regs->cap_lo);
    u32 ತಳ_freq = (cap >> SD_CAPLO_BASECLOCK_ಸರಿಸು) & SD_CAPLO_BASECLOCK_ಮುಖವಾಡ;
    ಆದರೆ (!ತಳ_freq) {
        dprintf(1, "Unknown base frequency for SD controller\n");
        ಹಿಂತಿರುಗಿ -1;
    }
    // Set new frequency
    u32 ಭಾಜಕ = DIV_ಸುತ್ತು_ಮೇಲೆ(ತಳ_freq * 1000, khz);
    u16 creg;
    ಆದರೆ ((ver & 0xff) <= 0x01) {
        ಭಾಜಕ = ಭಾಜಕ > 1 ? 1 << __fls(ಭಾಜಕ-1) : 0;
        creg = (ಭಾಜಕ & SCC_SDCLK_ಮುಖವಾಡ) << SCC_SDCLK_ಸರಿಸು;
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಭಾಜಕ = DIV_ಸುತ್ತು_ಮೇಲೆ(ಭಾಜಕ, 2);
        creg = (ಭಾಜಕ & SCC_SDCLK_ಮುಖವಾಡ) << SCC_SDCLK_ಸರಿಸು;
        creg |= (ಭಾಜಕ & SCC_SDCLK_HI_ಮುಖವಾಡ) >> SCC_SDCLK_HI_RSHIFT;
    }
    dprintf(3, "sdcard_set_frequency %d %d %x\n", ತಳ_freq, khz, creg);
    writew(&regs->clock_control, 0);
    writew(&regs->clock_control, creg | SCC_ಆಂತರಿಕ_ENABLE);
    // Wait for frequency to become active
    ಪೂರ್ಣಾಂಕ ret = sdcard_waitw(&regs->clock_control, SCC_STABLE);
    ಆದರೆ (ret < 0)
        ಹಿಂತಿರುಗಿ ret;
    // Enable SD clock
    writew(&regs->clock_control, creg | SCC_ಆಂತರಿಕ_ENABLE | SCC_CLOCK_ENABLE);
    ಹಿಂತಿರುಗಿ 0;
}

// Obtain the disk size of an SD card
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
sdcard_get_ಸಾಮರ್ಥ್ಯ(ರಚನೆ sddrive_s *drive, u8 *csd)
{
    // Original MMC/SD card capacity formula
    u16 C_ಗಾತ್ರ = (csd[6] >> 6) | (csd[7] << 2) | ((csd[8] & 0x03) << 10);
    u8 C_ಗಾತ್ರ_MULT = (csd[4] >> 7) | ((csd[5] & 0x03) << 1);
    u8 READ_BL_LEN = csd[9] & 0x0f;
    u32 ಎಣಿಕೆ_2 = (C_ಗಾತ್ರ+1) << (C_ಗಾತ್ರ_MULT + 2 + READ_BL_LEN - 9);
    // Check for newer encoding formats.
    u8 CSD_STRUCTURE = csd[14] >> 6;
    ಆದರೆ ((drive->card_type & SF_MMC) && CSD_STRUCTURE >= 2) {
        // Get capacity from EXT_CSD register
        u8 ext_csd[512];
        ಪೂರ್ಣಾಂಕ ret = sdcard_pio_transfer(drive, SC_SEND_EXT_CSD, 0, ext_csd, 1);
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ ret;
        ಎಣಿಕೆ_2 = *(u32*)&ext_csd[212];
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!(drive->card_type & SF_MMC) && CSD_STRUCTURE >= 1) {
        // High capacity SD card
        u32 C_SIZE2 = csd[5] | (csd[6] << 8) | ((csd[7] & 0x3f) << 16);
        ಎಣಿಕೆ_2 = (C_SIZE2+1) << (19-9);
    }
    // Fill drive struct and return
    drive->drive.blksize = DISK_SECTOR_ಗಾತ್ರ;
    drive->drive.sectors = ಎಣಿಕೆ_2;
    ಹಿಂತಿರುಗಿ 0;
}

// Initialize an SD card
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
sdcard_card_ಸಿದ್ಧತೆ(ರಚನೆ sddrive_s *drive, ಪೂರ್ಣಾಂಕ volt, ಪೂರ್ಣಾಂಕ prio)
{
    ರಚನೆ sdhci_s *regs = drive->regs;
    // Set controller to initialization clock rate
    ಪೂರ್ಣಾಂಕ ret = sdcard_ಗಣ_frequency(regs, 400);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;
    msleep(SDHCI_CLOCK_ON_ಸಮಯ);
    // Reset card
    u32 param[4] = { };
    ret = sdcard_pio(regs, SC_GO_IDLE_STATE, param);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;
    // Let card know SDHC/SDXC is supported and confirm voltage
    u32 hcs = 0, vrange = (volt >= (1<<15) ? 0x100 : 0x200) | 0xaa;
    param[0] = vrange;
    ret = sdcard_pio(regs, SC_SEND_IF_COND, param);
    ಆದರೆ (!ret && param[0] == vrange)
        hcs = (1<<30);
    // Verify SD card (instead of MMC or SDIO)
    param[0] = 0x00;
    ret = sdcard_pio_app(regs, SC_APP_SEND_OP_COND, param);
    ಆದರೆ (ret) {
        // Check for MMC card
        param[0] = 0x00;
        ret = sdcard_pio(regs, SC_SEND_OP_COND, param);
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ ret;
        drive->card_type |= SF_MMC;
        hcs = (1<<30);
    }
    // Init card
    u32 ಕೊನೆ = timer_calc(SDHCI_POWERUP_TIMEOUT);
    ಗಾಗಿ (;;) {
        param[0] = hcs | volt; // high-capacity support and voltage level
        ಆದರೆ (drive->card_type & SF_MMC)
            ret = sdcard_pio(regs, SC_SEND_OP_COND, param);
        ಇಲ್ಲದಿದ್ದರೆ
            ret = sdcard_pio_app(regs, SC_APP_SEND_OP_COND, param);
        ಆದರೆ (ret)
            ಹಿಂತಿರುಗಿ ret;
        ಆದರೆ (param[0] & SR_OCR_NOTBUSY)
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            ಹಿಂತಿರುಗಿ -1;
        }
        msleep(5); // Avoid flooding log when debugging
    }
    drive->card_type |= (param[0] & SR_OCR_CCS) ? SF_HIGHCAPACITY : 0;
    // Select card (get cid, set rca, get csd, select card)
    param[0] = 0x00;
    ret = sdcard_pio(regs, SC_ALL_SEND_CID, param);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;
    u8 cid[16];
    ಸ್ಮರಣೆ_ನಕಲು(cid, param, ಗಾತ್ರ(cid));
    param[0] = drive->card_type & SF_MMC ? 0x0001 << 16 : 0x00;
    ret = sdcard_pio(regs, SC_SEND_RELATIVE_ADDR, param);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;
    u16 rca = drive->card_type & SF_MMC ? 0x0001 : param[0] >> 16;
    param[0] = rca << 16;
    ret = sdcard_pio(regs, SC_SEND_CSD, param);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;
    u8 csd[16];
    ಸ್ಮರಣೆ_ನಕಲು(csd, param, ಗಾತ್ರ(csd));
    param[0] = rca << 16;
    ret = sdcard_pio(regs, SC_SELECT_DESELECT_CARD, param);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;
    // Set controller to data transfer clock rate
    ret = sdcard_ಗಣ_frequency(regs, 25000);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;
    // Register drive
    ret = sdcard_get_ಸಾಮರ್ಥ್ಯ(drive, csd);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;
    ಅಕ್ಷರ pnm[7] = {};
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i < (drive->card_type & SF_MMC ? 6 : 5); i++)
        pnm[i] = cid[11-i];
    ಅಕ್ಷರ *desc = znprintf(MAXDESCSIZE, "%s %s %dMiB"
                          , drive->card_type & SF_MMC ? "MMC drive" : "SD card"
                          , pnm, (u32)(drive->drive.sectors >> 11));
    dprintf(1, "Found sdcard at %p: %s\n", regs, desc);
    boot_ಕೂಡಿಸು_hd(&drive->drive, desc, prio);
    ಹಿಂತಿರುಗಿ 0;
}

// Setup and configure an SD card controller
ಸ್ಥಿರ ಶೂನ್ಯ
sdcard_controller_ಸಿದ್ಧತೆ(ರಚನೆ sdhci_s *regs, ಪೂರ್ಣಾಂಕ prio)
{
    // Initialize controller
    u32 present_state = readl(&regs->present_state);
    ಆದರೆ (!(present_state & SP_CARD_INSERTED))
        // No card present
        ಹಿಂತಿರುಗಿ;
    dprintf(3, "sdhci@%p ver=%x cap=%x %x\n", regs
            , readw(&regs->controller_ಆವೃತ್ತಿ)
            , readl(&regs->cap_lo), readl(&regs->cap_hi));
    sdcard_reset(regs, SRF_ALL);
    writew(&regs->irq_signal, 0);
    writew(&regs->irq_enable, 0x01ff);
    writew(&regs->irq_ಸ್ಥಿತಿ, readw(&regs->irq_ಸ್ಥಿತಿ));
    writew(&regs->error_signal, 0);
    writew(&regs->error_irq_enable, 0x01ff);
    writew(&regs->error_irq_ಸ್ಥಿತಿ, readw(&regs->error_irq_ಸ್ಥಿತಿ));
    writeb(&regs->timeout_control, 0x0e); // Set to max timeout
    ಪೂರ್ಣಾಂಕ volt = sdcard_ಗಣ_power(regs);
    ಆದರೆ (volt < 0)
        ಹಿಂತಿರುಗಿ;

    // Initialize card
    ರಚನೆ sddrive_s *drive = malloc_fseg(ಗಾತ್ರ(*drive));
    ಆದರೆ (!drive) {
        warn_noalloc();
        ಹೋಗು fail;
    }
    memset(drive, 0, ಗಾತ್ರ(*drive));
    drive->drive.type = DTYPE_SDCARD;
    drive->regs = regs;
    ಪೂರ್ಣಾಂಕ ret = sdcard_card_ಸಿದ್ಧತೆ(drive, volt, prio);
    ಆದರೆ (ret) {
        ಬಿಡುಗಡೆ(drive);
        ಹೋಗು fail;
    }
    ಹಿಂತಿರುಗಿ;
fail:
    writeb(&regs->power_control, 0);
    writew(&regs->clock_control, 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ
sdcard_pci_ಸಿದ್ಧತೆ(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ರಚನೆ pci_device *pci = ದತ್ತಾಂಶ;
    // XXX - bars dependent on slot index register in pci config space
    ರಚನೆ sdhci_s *regs = pci_enable_membar(pci, PCI_ತಳ_ವಿಳಾಸ_0);
    ಆದರೆ (!regs)
        ಹಿಂತಿರುಗಿ;
    ಪೂರ್ಣಾಂಕ prio = bootprio_ಕಂಡುಹಿಡಿ_pci_device(pci);
    sdcard_controller_ಸಿದ್ಧತೆ(regs, prio);
}

ಸ್ಥಿರ ಶೂನ್ಯ
sdcard_romfile_ಸಿದ್ಧತೆ(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ರಚನೆ romfile_s *file = ದತ್ತಾಂಶ;
    ಪೂರ್ಣಾಂಕ prio = bootprio_ಕಂಡುಹಿಡಿ_named_rom(file->name, 0);
    u32 addr = romfile_loadint(file->name, 0);
    dprintf(1, "Starting sdcard controller check at addr %x\n", addr);
    sdcard_controller_ಸಿದ್ಧತೆ((ಶೂನ್ಯ*)addr, prio);
}

ಶೂನ್ಯ
sdcard_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (!CONFIG_SDCARD)
        ಹಿಂತಿರುಗಿ;

    ರಚನೆ romfile_s *file = ಖಾಲಿ;
    ಪೂರ್ಣಾಂಕ ಅಂಕೆ_romfiles = 0;
    ಗಾಗಿ (;;) {
        file = romfile_findprefix("etc/sdcard", file);
        ಆದರೆ (!file)
            ನಿಲ್ಲಿಸು;
        ಸತತ_thread(sdcard_romfile_ಸಿದ್ಧತೆ, file);
        ಅಂಕೆ_romfiles++;
    }
    ಆದರೆ (ಅಂಕೆ_romfiles)
        // only scan for PCI controllers if etc/sdcard not used
        ಹಿಂತಿರುಗಿ;

    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (pci->class != PCI_CLASS_SYSTEM_SDHCI || pci->prog_if >= 2)
            // Not an SDHCI controller following SDHCI spec
            ಮುಂದುವರಿ;
        ಸತತ_thread(sdcard_pci_ಸಿದ್ಧತೆ, pci);
    }
}

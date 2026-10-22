// Low level ATA disk access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "ata.h" // ATA_CB_STAT
#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBALFLAT
#ಅನ್ತರ್ಭಾವ "block.h" // struct drive_s
#ಅನ್ತರ್ಭಾವ "blockcmd.h" // CDB_CMD_READ_10
#ಅನ್ತರ್ಭಾವ "byteorder.h" // be16_to_cpu
#ಅನ್ತರ್ಭಾವ "malloc.h" // malloc_fseg
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pci.h" // pci_config_readb
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_INTERRUPT_LINE
#ಅನ್ತರ್ಭಾವ "pic.h" // enable_hwirq
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_SUCCESS
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // timer_calc
#ಅನ್ತರ್ಭಾವ "x86.h" // inb

#define IDE_TIMEOUT 32000 //32 seconds max for IDE ops


/****************************************************************
 * Helper functions
 ****************************************************************/

// Wait for the specified ide state
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ
await_ide(u8 mask, u8 flags, u16 base, u16 timeout)
{
    u32 end = timer_calc(timeout);
    ಪ್ರತಿ (;;) {
        u8 status = inb(base+ATA_CB_STAT);
        ಯದಿ ((status & mask) == flags)
            ಪ್ರತ್ಯಾಗಮ status;
        ಯದಿ (timer_check(end)) {
            warn_timeout();
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        yield();
    }
}

// Wait for the device to be not-busy.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
await_not_bsy(u16 base)
{
    ಪ್ರತ್ಯಾಗಮ await_ide(ATA_CB_STAT_BSY, 0, base, IDE_TIMEOUT);
}

// Wait for the device to be ready.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
await_rdy(u16 base)
{
    ಪ್ರತ್ಯಾಗಮ await_ide(ATA_CB_STAT_RDY, ATA_CB_STAT_RDY, base, IDE_TIMEOUT);
}

// Wait for ide state - pauses for one ata cycle first.
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ
pause_await_not_bsy(u16 iobase1, u16 iobase2)
{
    // Wait one PIO transfer cycle.
    inb(iobase2 + ATA_CB_ASTAT);

    ಪ್ರತ್ಯಾಗಮ await_not_bsy(iobase1);
}

// Wait for ide state - pause for 400ns first.
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ
ndelay_await_not_bsy(u16 iobase1)
{
    ndelay(400);
    ಪ್ರತ್ಯಾಗಮ await_not_bsy(iobase1);
}

// Reset a drive
ಸ್ಥಿರ ಶೂನ್ಯ
ata_reset(ಸಂರಚನಾ atadrive_s *adrive_gf)
{
    ಸಂರಚನಾ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    dprintf(6, "ata_reset drive=%p\n", &adrive_gf->drive);
    // Pulse SRST
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN | ATA_CB_DC_SRST, iobase2+ATA_CB_DC);
    udelay(5);
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2+ATA_CB_DC);
    msleep(2);

    // wait for device to become not busy.
    ಪೂರ್ಣಾಙ್ಕ status = await_not_bsy(iobase1);
    ಯದಿ (status < 0)
        ಗಚ್ಛ done;
    ಯದಿ (slave) {
        // Change device.
        u32 end = timer_calc(IDE_TIMEOUT);
        ಪ್ರತಿ (;;) {
            outb(ATA_CB_DH_DEV1, iobase1 + ATA_CB_DH);
            status = ndelay_await_not_bsy(iobase1);
            ಯದಿ (status < 0)
                ಗಚ್ಛ done;
            ಯದಿ (inb(iobase1 + ATA_CB_DH) == ATA_CB_DH_DEV1)
                ಭಙ್ಗ;
            // Change drive request failed to take effect - retry.
            ಯದಿ (timer_check(end)) {
                warn_timeout();
                ಗಚ್ಛ done;
            }
        }
    } ಅನ್ಯಥಾ {
        // QEMU doesn't reset dh on reset, so set it explicitly.
        outb(ATA_CB_DH_DEV0, iobase1 + ATA_CB_DH);
    }

    // On a user-reset request, wait for RDY if it is an ATA device.
    u8 type=GET_GLOBALFLAT(adrive_gf->drive.type);
    ಯದಿ (type == DTYPE_ATA)
        status = await_rdy(iobase1);

done:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    dprintf(6, "ata_reset exit status=%x\n", status);
}

// Check for drive RDY for 16bit interface command.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
isready(ಸಂರಚನಾ atadrive_s *adrive_gf)
{
    // Read the status from controller
    ಸಂರಚನಾ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u8 status = inb(iobase1 + ATA_CB_STAT);
    ಯದಿ ((status & (ATA_CB_STAT_BSY|ATA_CB_STAT_RDY)) == ATA_CB_STAT_RDY)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;
    ಪ್ರತ್ಯಾಗಮ DISK_RET_ENOTREADY;
}


/****************************************************************
 * ATA send command
 ****************************************************************/

ಸಂರಚನಾ ata_pio_command {
    u8 feature;
    u8 sector_ಎಣಿಕೆ;
    u8 lba_ಕೆಳಗಿನ;
    u8 lba_mid;
    u8 lba_ಮೇಲಿನ;
    u8 device;
    u8 command;

    u8 feature2;
    u8 sector_count2;
    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
};

// Send an ata command to the drive.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
send_cmd(ಸಂರಚನಾ atadrive_s *adrive_gf, ಸಂರಚನಾ ata_pio_command *cmd)
{
    ಸಂರಚನಾ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);

    // Select device
    ಪೂರ್ಣಾಙ್ಕ status = await_not_bsy(iobase1);
    ಯದಿ (status < 0)
        ಪ್ರತ್ಯಾಗಮ status;
    u8 newdh = ((cmd->device & ~ATA_CB_DH_DEV1)
                | (slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0));
    u8 olddh = inb(iobase1 + ATA_CB_DH);
    outb(newdh, iobase1 + ATA_CB_DH);
    ಯದಿ ((olddh ^ newdh) & (1<<4)) {
        // Was a device change - wait for device to become not busy.
        status = ndelay_await_not_bsy(iobase1);
        ಯದಿ (status < 0)
            ಪ್ರತ್ಯಾಗಮ status;
    }

    // Check for ATA_CMD_(READ|WRITE)_(SECTORS|DMA)_EXT commands.
    ಯದಿ ((cmd->command & ~0x11) == ATA_CMD_READ_SECTORS_EXT) {
        outb(cmd->feature2, iobase1 + ATA_CB_FR);
        outb(cmd->sector_count2, iobase1 + ATA_CB_SC);
        outb(cmd->lba_low2, iobase1 + ATA_CB_SN);
        outb(cmd->lba_mid2, iobase1 + ATA_CB_CL);
        outb(cmd->lba_high2, iobase1 + ATA_CB_CH);
    }
    outb(cmd->feature, iobase1 + ATA_CB_FR);
    outb(cmd->sector_ಎಣಿಕೆ, iobase1 + ATA_CB_SC);
    outb(cmd->lba_ಕೆಳಗಿನ, iobase1 + ATA_CB_SN);
    outb(cmd->lba_mid, iobase1 + ATA_CB_CL);
    outb(cmd->lba_ಮೇಲಿನ, iobase1 + ATA_CB_CH);
    outb(cmd->command, iobase1 + ATA_CB_CMD);

    ಪ್ರತ್ಯಾಗಮ 0;
}

// Wait for data after calling 'send_cmd'.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ata_wait_data(u16 iobase1)
{
    ಪೂರ್ಣಾಙ್ಕ status = ndelay_await_not_bsy(iobase1);
    ಯದಿ (status < 0)
        ಪ್ರತ್ಯಾಗಮ status;

    ಯದಿ (status & ATA_CB_STAT_ERR) {
        dprintf(6, "send_cmd : read error (status=%02x err=%02x)\n"
                , status, inb(iobase1 + ATA_CB_ERR));
        ಪ್ರತ್ಯಾಗಮ -4;
    }
    ಯದಿ (!(status & ATA_CB_STAT_DRQ)) {
        dprintf(6, "send_cmd : DRQ not set (status %02x)\n", status);
        ಪ್ರತ್ಯಾಗಮ -5;
    }

    ಪ್ರತ್ಯಾಗಮ 0;
}

// Send an ata command that does not transfer any further data.
ಪೂರ್ಣಾಙ್ಕ
ata_cmd_nondata(ಸಂರಚನಾ atadrive_s *adrive_gf, ಸಂರಚನಾ ata_pio_command *cmd)
{
    ಸಂರಚನಾ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    ಪೂರ್ಣಾಙ್ಕ ret = send_cmd(adrive_gf, cmd);
    ಯದಿ (ret)
        ಗಚ್ಛ fail;
    ret = ndelay_await_not_bsy(iobase1);
    ಯದಿ (ret < 0)
        ಗಚ್ಛ fail;

    ಯದಿ (ret & ATA_CB_STAT_ERR) {
        dprintf(6, "nondata cmd : read error (status=%02x err=%02x)\n"
                , ret, inb(iobase1 + ATA_CB_ERR));
        ret = -4;
        ಗಚ್ಛ fail;
    }
    ಯದಿ (ret & ATA_CB_STAT_DRQ) {
        dprintf(6, "nondata cmd : DRQ set (status %02x)\n", ret);
        ret = -5;
        ಗಚ್ಛ fail;
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    ಪ್ರತ್ಯಾಗಮ ret;
}


/****************************************************************
 * ATA PIO transfers
 ****************************************************************/

// Transfer 'op->count' blocks (of 'blocksize' bytes) to/from drive
// 'op->drive_fl'.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ata_pio_transfer(ಸಂರಚನಾ disk_op_s *op, ಪೂರ್ಣಾಙ್ಕ iswrite, ಪೂರ್ಣಾಙ್ಕ blocksize)
{
    dprintf(16, "ata_pio_transfer id=%p write=%d count=%d bs=%d buf=%p\n"
            , op->drive_fl, iswrite, op->ಎಣಿಕೆ, blocksize, op->buf_fl);

    ಸಂರಚನಾ atadrive_s *adrive_gf = container_of(
        op->drive_fl, ಸಂರಚನಾ atadrive_s, drive);
    ಸಂರಚನಾ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = op->ಎಣಿಕೆ;
    ಶೂನ್ಯ *buf_fl = op->buf_fl;
    ಪೂರ್ಣಾಙ್ಕ status;
    ಪ್ರತಿ (;;) {
        ಯದಿ (iswrite) {
            // Write data to controller
            dprintf(16, "Write sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            ಯದಿ (CONFIG_ATA_PIO32)
                outsl_fl(iobase1, buf_fl, blocksize / 4);
            ಅನ್ಯಥಾ
                outsw_fl(iobase1, buf_fl, blocksize / 2);
        } ಅನ್ಯಥಾ {
            // Read data from controller
            dprintf(16, "Read sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            ಯದಿ (CONFIG_ATA_PIO32)
                insl_fl(iobase1, buf_fl, blocksize / 4);
            ಅನ್ಯಥಾ
                insw_fl(iobase1, buf_fl, blocksize / 2);
        }
        buf_fl += blocksize;

        status = pause_await_not_bsy(iobase1, iobase2);
        ಯದಿ (status < 0) {
            // Error
            op->ಎಣಿಕೆ -= ಎಣಿಕೆ;
            ಪ್ರತ್ಯಾಗಮ status;
        }

        ಎಣಿಕೆ--;
        ಯದಿ (!ಎಣಿಕೆ)
            ಭಙ್ಗ;
        status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ | ATA_CB_STAT_ERR);
        ಯದಿ (status != ATA_CB_STAT_DRQ) {
            dprintf(6, "ata_pio_transfer : more sectors left (status %02x)\n"
                    , status);
            op->ಎಣಿಕೆ -= ಎಣಿಕೆ;
            ಪ್ರತ್ಯಾಗಮ -6;
        }
    }

    status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
               | ATA_CB_STAT_ERR);
    ಯದಿ (!iswrite)
        status &= ~ATA_CB_STAT_DF;
    ಯದಿ (status != 0) {
        dprintf(6, "ata_pio_transfer : no sectors left (status %02x)\n", status);
        ಪ್ರತ್ಯಾಗಮ -7;
    }

    ಪ್ರತ್ಯಾಗಮ 0;
}


/****************************************************************
 * ATA DMA transfers
 ****************************************************************/

#define BM_CMD    0
#define  BM_CMD_MEMWRITE  0x08
#define  BM_CMD_START     0x01
#define BM_STATUS 2
#define  BM_STATUS_IRQ    0x04
#define  BM_STATUS_ERROR  0x02
#define  BM_STATUS_ACTIVE 0x01
#define BM_TABLE  4

ಸಂರಚನಾ sff_dma_prd {
    u32 buf_fl;
    u32 ಎಣಿಕೆ;
};

// Check if DMA available and setup transfer if so.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ata_try_dma(ಸಂರಚನಾ disk_op_s *op, ಪೂರ್ಣಾಙ್ಕ iswrite, ಪೂರ್ಣಾಙ್ಕ blocksize)
{
    ASSERT16();
    ಯದಿ (! CONFIG_ATA_DMA)
        ಪ್ರತ್ಯಾಗಮ -1;
    u32 dest = (u32)op->buf_fl;
    ಯದಿ (dest & 1)
        // Need minimum alignment of 1.
        ಪ್ರತ್ಯಾಗಮ -1;
    ಸಂರಚನಾ atadrive_s *adrive_gf = container_of(
        op->drive_fl, ಸಂರಚನಾ atadrive_s, drive);
    ಸಂರಚನಾ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);
    ಯದಿ (! iomaster)
        ಪ್ರತ್ಯಾಗಮ -1;
    u32 bytes = op->ಎಣಿಕೆ * blocksize;
    ಯದಿ (! bytes)
        ಪ್ರತ್ಯಾಗಮ -1;

    // Build PRD dma structure.
    ಸಂರಚನಾ sff_dma_prd *dma = MAKE_FLATPTR(SEG_ಕೆಳಗಿನ, Extra_ರಾಶಿ);
    ಸಂರಚನಾ sff_dma_prd *origdma = dma;
    ಯಾವತ್ (bytes) {
        ಯದಿ (dma >= &origdma[16])
            // Too many descriptors..
            ಪ್ರತ್ಯಾಗಮ -1;
        u32 ಎಣಿಕೆ = bytes;
        u32 max = 0x10000 - (dest & 0xffff);
        ಯದಿ (ಎಣಿಕೆ > max)
            ಎಣಿಕೆ = max;

        SET_LOWFLAT(dma->buf_fl, dest);
        bytes -= ಎಣಿಕೆ;
        ಯದಿ (!bytes)
            // Last descriptor.
            ಎಣಿಕೆ |= 1<<31;
        dprintf(16, "dma@%p: %08x %08x\n", dma, dest, ಎಣಿಕೆ);
        dest += ಎಣಿಕೆ;
        SET_LOWFLAT(dma->ಎಣಿಕೆ, ಎಣಿಕೆ);
        dma++;
    }

    // Program bus-master controller.
    outl((u32)origdma, iomaster + BM_TABLE);
    u8 oldcmd = inb(iomaster + BM_CMD) & ~(BM_CMD_MEMWRITE|BM_CMD_START);
    outb(oldcmd | (iswrite ? 0x00 : BM_CMD_MEMWRITE), iomaster + BM_CMD);
    outb(BM_STATUS_ERROR|BM_STATUS_IRQ, iomaster + BM_STATUS);

    ಪ್ರತ್ಯಾಗಮ 0;
}

// Transfer data using DMA.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ata_dma_transfer(ಸಂರಚನಾ disk_op_s *op)
{
    ಯದಿ (! CONFIG_ATA_DMA)
        ಪ್ರತ್ಯಾಗಮ -1;
    dprintf(16, "ata_dma_transfer id=%p buf=%p\n", op->drive_fl, op->buf_fl);

    ಸಂರಚನಾ atadrive_s *adrive_gf = container_of(
        op->drive_fl, ಸಂರಚನಾ atadrive_s, drive);
    ಸಂರಚನಾ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);

    // Start bus-master controller.
    u8 oldcmd = inb(iomaster + BM_CMD);
    outb(oldcmd | BM_CMD_START, iomaster + BM_CMD);

    u32 end = timer_calc(IDE_TIMEOUT);
    u8 status;
    ಪ್ರತಿ (;;) {
        status = inb(iomaster + BM_STATUS);
        ಯದಿ (status & BM_STATUS_IRQ)
            ಭಙ್ಗ;
        // Transfer in progress
        ಯದಿ (timer_check(end)) {
            // Timeout.
            warn_timeout();
            ಭಙ್ಗ;
        }
        yield();
    }
    outb(oldcmd & ~BM_CMD_START, iomaster + BM_CMD);

    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    ಪೂರ್ಣಾಙ್ಕ idestatus = pause_await_not_bsy(iobase1, iobase2);

    ಯದಿ ((status & (BM_STATUS_IRQ|BM_STATUS_ACTIVE)) == BM_STATUS_IRQ
        && idestatus >= 0x00
        && (idestatus & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
                         | ATA_CB_STAT_ERR)) == 0x00)
        // Success.
        ಪ್ರತ್ಯಾಗಮ 0;

    dprintf(6, "IDE DMA error (dma=%x ide=%x/%x/%x)\n", status, idestatus
            , inb(iobase2 + ATA_CB_ASTAT), inb(iobase1 + ATA_CB_ERR));
    ಪ್ರತ್ಯಾಗಮ -1;
}


/****************************************************************
 * ATA hard drive functions
 ****************************************************************/

// Transfer data to harddrive using PIO protocol.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ata_pio_cmd_data(ಸಂರಚನಾ disk_op_s *op, ಪೂರ್ಣಾಙ್ಕ iswrite, ಸಂರಚನಾ ata_pio_command *cmd)
{
    ಸಂರಚನಾ atadrive_s *adrive_gf = container_of(
        op->drive_fl, ಸಂರಚನಾ atadrive_s, drive);
    ಸಂರಚನಾ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    ಪೂರ್ಣಾಙ್ಕ ret = send_cmd(adrive_gf, cmd);
    ಯದಿ (ret)
        ಗಚ್ಛ fail;
    ret = ata_wait_data(iobase1);
    ಯದಿ (ret)
        ಗಚ್ಛ fail;
    ret = ata_pio_transfer(op, iswrite, DISK_SECTOR_SIZE);

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    ಪ್ರತ್ಯಾಗಮ ret;
}

// Transfer data to harddrive using DMA protocol.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ata_dma_cmd_data(ಸಂರಚನಾ disk_op_s *op, ಸಂರಚನಾ ata_pio_command *cmd)
{
    ಯದಿ (! CONFIG_ATA_DMA)
        ಪ್ರತ್ಯಾಗಮ -1;
    ಸಂರಚನಾ atadrive_s *adrive_gf = container_of(
        op->drive_fl, ಸಂರಚನಾ atadrive_s, drive);
    ಪೂರ್ಣಾಙ್ಕ ret = send_cmd(adrive_gf, cmd);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ret;
    ಪ್ರತ್ಯಾಗಮ ata_dma_transfer(op);
}

// Read/write count blocks from a harddrive.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ata_readwrite(ಸಂರಚನಾ disk_op_s *op, ಪೂರ್ಣಾಙ್ಕ iswrite)
{
    u64 lba = op->lba;

    ಪೂರ್ಣಾಙ್ಕ usepio = ata_try_dma(op, iswrite, DISK_SECTOR_SIZE);

    ಸಂರಚನಾ ata_pio_command cmd;
    memset(&cmd, 0, ಪರಿಮಾಣ(cmd));

    ಯದಿ (op->ಎಣಿಕೆ >= (1<<8) || lba + op->ಎಣಿಕೆ >= (1<<28)) {
        cmd.sector_count2 = op->ಎಣಿಕೆ >> 8;
        cmd.lba_low2 = lba >> 24;
        cmd.lba_mid2 = lba >> 32;
        cmd.lba_high2 = lba >> 40;
        lba &= 0xffffff;

        ಯದಿ (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS_EXT
                           : ATA_CMD_READ_SECTORS_EXT);
        ಅನ್ಯಥಾ
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                           : ATA_CMD_READ_DMA_EXT);
    } ಅನ್ಯಥಾ {
        ಯದಿ (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS
                           : ATA_CMD_READ_SECTORS);
        ಅನ್ಯಥಾ
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA
                           : ATA_CMD_READ_DMA);
    }

    cmd.sector_ಎಣಿಕೆ = op->ಎಣಿಕೆ;
    cmd.lba_ಕೆಳಗಿನ = lba;
    cmd.lba_mid = lba >> 8;
    cmd.lba_ಮೇಲಿನ = lba >> 16;
    cmd.device = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;

    ಪೂರ್ಣಾಙ್ಕ ret;
    ಯದಿ (usepio)
        ret = ata_pio_cmd_data(op, iswrite, &cmd);
    ಅನ್ಯಥಾ
        ret = ata_dma_cmd_data(op, &cmd);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_EBADTRACK;
    ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;
}

// 16bit command demuxer for ATA harddrives.
ಪೂರ್ಣಾಙ್ಕ
ata_process_op(ಸಂರಚನಾ disk_op_s *op)
{
    ಯದಿ (!CONFIG_ATA)
        ಪ್ರತ್ಯಾಗಮ 0;

    ಸಂರಚನಾ atadrive_s *adrive_gf = container_of(
        op->drive_fl, ಸಂರಚನಾ atadrive_s, drive);
    ವಿಕಲ್ಪ (op->command) {
    ಪಕ್ಷ CMD_READ:
        ಪ್ರತ್ಯಾಗಮ ata_readwrite(op, 0);
    ಪಕ್ಷ CMD_WRITE:
        ಪ್ರತ್ಯಾಗಮ ata_readwrite(op, 1);
    ಪಕ್ಷ CMD_RESET:
        ata_reset(adrive_gf);
        ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;
    ಪಕ್ಷ CMD_ISREADY:
        ಪ್ರತ್ಯಾಗಮ isready(adrive_gf);
    ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ default_process_op(op);
    }
}


/****************************************************************
 * ATAPI functions
 ****************************************************************/

#define CDROM_CDB_SIZE 12

// Low-level atapi command transmit function.
ಪೂರ್ಣಾಙ್ಕ
ata_atapi_process_op(ಸಂರಚನಾ disk_op_s *op)
{
    ಯದಿ (! CONFIG_ATA)
        ಪ್ರತ್ಯಾಗಮ 0;

    ಯದಿ (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_EWRITEPROTECT;
    u8 cdbcmd[CDROM_CDB_SIZE];
    ಪೂರ್ಣಾಙ್ಕ blocksize = scsi_fill_cmd(op, cdbcmd, ಪರಿಮಾಣ(cdbcmd));
    ಯದಿ (blocksize < 0)
        ಪ್ರತ್ಯಾಗಮ default_process_op(op);

    ಸಂರಚನಾ atadrive_s *adrive_gf = container_of(
        op->drive_fl, ಸಂರಚನಾ atadrive_s, drive);
    ಸಂರಚನಾ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    ಸಂರಚನಾ ata_pio_command cmd;
    memset(&cmd, 0, ಪರಿಮಾಣ(cmd));
    cmd.lba_mid = blocksize;
    cmd.lba_ಮೇಲಿನ = blocksize >> 8;
    cmd.command = ATA_CMD_PACKET;

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    ಪೂರ್ಣಾಙ್ಕ ret = send_cmd(adrive_gf, &cmd);
    ಯದಿ (ret)
        ಗಚ್ಛ fail;
    ret = ata_wait_data(iobase1);
    ಯದಿ (ret)
        ಗಚ್ಛ fail;

    // Send command to device
    outsw_fl(iobase1, MAKE_FLATPTR(GET_SEG(SS), cdbcmd), CDROM_CDB_SIZE / 2);

    ಪೂರ್ಣಾಙ್ಕ status = pause_await_not_bsy(iobase1, iobase2);
    ಯದಿ (status < 0) {
        ret = status;
        ಗಚ್ಛ fail;
    }

    ಯದಿ (status & ATA_CB_STAT_ERR) {
        u8 err = inb(iobase1 + ATA_CB_ERR);
        // skip "Not Ready"
        ಯದಿ (err != 0x20)
            dprintf(6, "send_atapi_cmd : read error (status=%02x err=%02x)\n"
                    , status, err);
        ret = -2;
        ಗಚ್ಛ fail;
    }
    ಯದಿ (blocksize) {
        ಯದಿ (!(status & ATA_CB_STAT_DRQ)) {
            dprintf(6, "send_atapi_cmd : DRQ not set (status %02x)\n", status);
            ret = -3;
            ಗಚ್ಛ fail;
        }

        ret = ata_pio_transfer(op, 0, blocksize);
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_EBADTRACK;
    ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;
}


/****************************************************************
 * ATA detect and init
 ****************************************************************/

// Send an identify device or identify device packet command.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
send_ata_identity(ಸಂರಚನಾ atadrive_s *adrive, u16 *buffer, ಪೂರ್ಣಾಙ್ಕ command)
{
    memset(buffer, 0, DISK_SECTOR_SIZE);

    ಸಂರಚನಾ disk_op_s dop;
    memset(&dop, 0, ಪರಿಮಾಣ(dop));
    dop.drive_fl = &adrive->drive;
    dop.ಎಣಿಕೆ = 1;
    dop.lba = 1;
    dop.buf_fl = MAKE_FLATPTR(GET_SEG(SS), buffer);

    ಸಂರಚನಾ ata_pio_command cmd;
    memset(&cmd, 0, ಪರಿಮಾಣ(cmd));
    cmd.command = command;

    ಪ್ರತ್ಯಾಗಮ ata_pio_cmd_data(&dop, 0, &cmd);
}

// Extract the ATA/ATAPI version info.
ಪೂರ್ಣಾಙ್ಕ
ata_extract_version(u16 *buffer)
{
    // Extract ATA/ATAPI version.
    u16 ataversion = buffer[80];
    u8 version;
    ಪ್ರತಿ (version=15; version>0; version--)
        ಯದಿ (ataversion & (1<<version))
            ಭಙ್ಗ;
    ಪ್ರತ್ಯಾಗಮ version;
}

#define MAXMODEL 40

// Extract the ATA/ATAPI model info.
ವರ್ಣ *
ata_extract_model(ವರ್ಣ *model, u32 size, u16 *buffer)
{
    // Read model name
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i=0; i<size/2; i++)
        *(u16*)&model[i*2] = be16_to_cpu(buffer[27+i]);
    model[size] = 0x00;
    nullTrailingSpace(model);
    ಪ್ರತ್ಯಾಗಮ model;
}

// Common init code between ata and atapi
ಸ್ಥಿರ ಸಂರಚನಾ atadrive_s *
init_atadrive(ಸಂರಚನಾ atadrive_s *dummy, u16 *buffer)
{
    ಸಂರಚನಾ atadrive_s *adrive = malloc_fseg(ಪರಿಮಾಣ(*adrive));
    ಯದಿ (!adrive) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    memset(adrive, 0, ಪರಿಮಾಣ(*adrive));
    adrive->chan_gf = dummy->chan_gf;
    adrive->slave = dummy->slave;
    adrive->drive.cntl_id = adrive->chan_gf->ataid * 2 + dummy->slave;
    adrive->drive.removable = (buffer[0] & 0x80) ? 1 : 0;
    ಪ್ರತ್ಯಾಗಮ adrive;
}

// Detect if the given drive is an atapi - initialize it if so.
ಸ್ಥಿರ ಸಂರಚನಾ atadrive_s *
init_drive_atapi(ಸಂರಚನಾ atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE_PACKET command to device
    ಪೂರ್ಣಾಙ್ಕ ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

    // Success - setup as ATAPI.
    ಸಂರಚನಾ atadrive_s *adrive = init_atadrive(dummy, buffer);
    ಯದಿ (!adrive)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    adrive->drive.type = DTYPE_ATA_ATAPI;
    adrive->drive.blksize = CDROM_SECTOR_SIZE;
    adrive->drive.sectors = (u64)-1;
    u8 iscd = ((buffer[0] >> 8) & 0x1f) == 0x05;
    ವರ್ಣ model[MAXMODEL+1];
    ವರ್ಣ *desc = znprintf(MAXDESCSIZE
                          , "DVD/CD [ata%d-%d: %s ATAPI-%d %s]"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (iscd ? "DVD/CD" : "Device"));
    dprintf(1, "%s\n", desc);

    // fill cdidmap
    ಯದಿ (iscd) {
        ಪೂರ್ಣಾಙ್ಕ prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                            adrive->chan_gf->chanid,
                                            adrive->slave);
        boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                                  adrive->chan_gf->chanid,
                                  adrive->slave,
                                  &(adrive->drive.lchs));
        boot_add_cd(&adrive->drive, desc, prio);
    }

    ಪ್ರತ್ಯಾಗಮ adrive;
}

// Detect if the given drive is a regular ata drive - initialize it if so.
ಸ್ಥಿರ ಸಂರಚನಾ atadrive_s *
init_drive_ata(ಸಂರಚನಾ atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE command to device
    ಪೂರ್ಣಾಙ್ಕ ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_DEVICE);
    ಯದಿ (ret)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

    // Success - setup as ATA.
    ಸಂರಚನಾ atadrive_s *adrive = init_atadrive(dummy, buffer);
    ಯದಿ (!adrive)
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    adrive->drive.type = DTYPE_ATA;
    adrive->drive.blksize = DISK_SECTOR_SIZE;

    adrive->drive.pchs.cylinder = buffer[1];
    adrive->drive.pchs.head = buffer[3];
    adrive->drive.pchs.sector = buffer[6];

    u64 sectors;
    ಯದಿ (buffer[83] & (1 << 10)) // word 83 - lba48 support
        sectors = *(u64*)&buffer[100]; // word 100-103
    ಅನ್ಯಥಾ
        sectors = *(u32*)&buffer[60]; // word 60 and word 61
    adrive->drive.sectors = sectors;
    u64 adjsize = sectors >> 11;
    ವರ್ಣ adjprefix = 'M';
    ಯದಿ (adjsize >= (1 << 16)) {
        adjsize >>= 10;
        adjprefix = 'G';
    }
    ವರ್ಣ model[MAXMODEL+1];
    ವರ್ಣ *desc = znprintf(MAXDESCSIZE
                          , "ata%d-%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (u32)adjsize, adjprefix);
    dprintf(1, "%s\n", desc);

    ಪೂರ್ಣಾಙ್ಕ prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                        adrive->chan_gf->chanid,
                                        adrive->slave);
    boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                              adrive->chan_gf->chanid,
                              adrive->slave,
                              &(adrive->drive.lchs));
    // Register with bcv system.
    boot_add_hd(&adrive->drive, desc, prio);

    ಪ್ರತ್ಯಾಗಮ adrive;
}

ಸ್ಥಿರ u32 SpinupEnd;

// Wait for non-busy status and check for "floating bus" condition.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
powerup_await_non_bsy(u16 base)
{
    u8 orstatus = 0;
    u8 status;
    ಪ್ರತಿ (;;) {
        status = inb(base+ATA_CB_STAT);
        ಯದಿ (!(status & ATA_CB_STAT_BSY))
            ಭಙ್ಗ;
        orstatus |= status;
        ಯದಿ (orstatus == 0xff) {
            dprintf(4, "powerup IDE floating\n");
            ಪ್ರತ್ಯಾಗಮ orstatus;
        }
        ಯದಿ (timer_check(SpinupEnd)) {
            warn_timeout();
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        yield();
    }
    dprintf(6, "powerup iobase=%x st=%x\n", base, status);
    ಪ್ರತ್ಯಾಗಮ status;
}

// Detect any drives attached to a given controller.
ಸ್ಥಿರ ಶೂನ್ಯ
ata_detect(ಶೂನ್ಯ *data)
{
    ಸಂರಚನಾ ata_channel_s *chan_gf = data;
    ಸಂರಚನಾ atadrive_s dummy;
    memset(&dummy, 0, ಪರಿಮಾಣ(dummy));
    dummy.chan_gf = chan_gf;
    // Device detection
    ಪೂರ್ಣಾಙ್ಕ didreset = 0;
    u8 slave;
    ಪ್ರತಿ (slave=0; slave<=1; slave++) {
        // Wait for not-bsy.
        u16 iobase1 = chan_gf->iobase1;
        ಪೂರ್ಣಾಙ್ಕ status = powerup_await_non_bsy(iobase1);
        ಯದಿ (status < 0)
            ಅನುವರ್ತ;
        u8 newdh = slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0;
        outb(newdh, iobase1+ATA_CB_DH);
        ndelay(400);
        status = powerup_await_non_bsy(iobase1);
        ಯದಿ (status < 0)
            ಅನುವರ್ತ;

        // Check if ioport registers look valid.
        outb(newdh, iobase1+ATA_CB_DH);
        u8 dh = inb(iobase1+ATA_CB_DH);
        outb(0x55, iobase1+ATA_CB_SC);
        outb(0xaa, iobase1+ATA_CB_SN);
        u8 sc = inb(iobase1+ATA_CB_SC);
        u8 sn = inb(iobase1+ATA_CB_SN);
        dprintf(6, "ata_detect ata%d-%d: sc=%x sn=%x dh=%x\n"
                , chan_gf->ataid, slave, sc, sn, dh);
        ಯದಿ (sc != 0x55 || sn != 0xaa || dh != newdh)
            ಅನುವರ್ತ;

        // Prepare new drive.
        dummy.slave = slave;

        // reset the channel
        ಯದಿ (!didreset) {
            ata_reset(&dummy);
            didreset = 1;
        }

        // check for ATAPI
        u16 buffer[256];
        ಸಂರಚನಾ atadrive_s *adrive = init_drive_atapi(&dummy, buffer);
        ಯದಿ (!adrive) {
            // Didn't find an ATAPI drive - look for ATA drive.
            u8 st = inb(iobase1+ATA_CB_STAT);
            ಯದಿ (!st)
                // Status not set - can't be a valid drive.
                ಅನುವರ್ತ;

            // Wait for RDY.
            ಪೂರ್ಣಾಙ್ಕ ret = await_rdy(iobase1);
            ಯದಿ (ret < 0)
                ಅನುವರ್ತ;

            // check for ATA.
            adrive = init_drive_ata(&dummy, buffer);
            ಯದಿ (!adrive)
                // No ATA drive found
                ಅನುವರ್ತ;
        }

        u16 resetresult = buffer[93];
        dprintf(6, "ata_detect resetresult=%04x\n", resetresult);
        ಯದಿ (!slave && (resetresult & 0xdf61) == 0x4041)
            // resetresult looks valid and device 0 is responding to
            // device 1 requests - device 1 must not be present - skip
            // detection.
            ಭಙ್ಗ;
    }
}

// Initialize an ata controller and detect its drives.
ಸ್ಥಿರ ಶೂನ್ಯ
init_controller(ಸಂರಚನಾ pci_device *pci, ಪೂರ್ಣಾಙ್ಕ chanid, ಪೂರ್ಣಾಙ್ಕ irq
                , u32 port1, u32 port2, u32 master)
{
    ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ataid = 0;
    ಸಂರಚನಾ ata_channel_s *chan_gf = malloc_fseg(ಪರಿಮಾಣ(*chan_gf));
    ಯದಿ (!chan_gf) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    chan_gf->ataid = ataid++;
    chan_gf->chanid = chanid;
    chan_gf->irq = irq;
    chan_gf->pci_bdf = pci ? pci->bdf : -1;
    chan_gf->pci_tmp = pci;
    chan_gf->iobase1 = port1;
    chan_gf->iobase2 = port2;
    chan_gf->iomaster = master;
    dprintf(1, "ATA controller %d at %x/%x/%x (irq %d dev %x)\n"
            , ataid, port1, port2, master, irq, chan_gf->pci_bdf);
    run_thread(ata_detect, chan_gf);
}

#define IRQ_ATA1 14
#define IRQ_ATA2 15

// Handle controllers on an ATA PCI device.
ಸ್ಥಿರ ಶೂನ್ಯ
init_pciata(ಸಂರಚನಾ pci_device *pci, u8 prog_if)
{
    u8 pciirq = pci_config_readb(pci->bdf, PCI_INTERRUPT_ಸಾಲು);
    ಪೂರ್ಣಾಙ್ಕ master = 0;
    ಯದಿ (CONFIG_ATA_DMA && prog_if & 0x80) {
        // Check for bus-mastering.
        u32 bar = pci_config_readl(pci->bdf, PCI_BASE_ADDRESS_4);
        ಯದಿ (bar & PCI_BASE_ADDRESS_SPACE_IO) {
            master = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
            pci_enable_busmaster(pci);
        }
    }

    u32 port1, port2, irq;
    ಯದಿ (prog_if & 1) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_1);
        ಯದಿ (!port1 || !port2)
            ಪ್ರತ್ಯಾಗಮ;
        irq = pciirq;
    } ಅನ್ಯಥಾ {
        port1 = PORT_ATA1_CMD_BASE;
        port2 = PORT_ATA1_CTRL_BASE;
        irq = IRQ_ATA1;
    }
    init_controller(pci, 0, irq, port1, port2, master);

    ಯದಿ (prog_if & 4) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_2);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_3);
        ಯದಿ (!port1 || !port2)
            ಪ್ರತ್ಯಾಗಮ;
        irq = pciirq;
    } ಅನ್ಯಥಾ {
        port1 = PORT_ATA2_CMD_BASE;
        port2 = PORT_ATA2_CTRL_BASE;
        irq = IRQ_ATA2;
    }
    init_controller(pci, 1, irq, port1, port2, master ? master + 8 : 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ
found_genericata(ಸಂರಚನಾ pci_device *pci, ಶೂನ್ಯ *arg)
{
    init_pciata(pci, pci->prog_if);
}

ಸ್ಥಿರ ಶೂನ್ಯ
found_compatibleahci(ಸಂರಚನಾ pci_device *pci, ಶೂನ್ಯ *arg)
{
    ಯದಿ (CONFIG_AHCI)
        // Already handled directly via native ahci interface.
        ಪ್ರತ್ಯಾಗಮ;
    init_pciata(pci, 0x8f);
}

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ pci_device_id pci_ata_tbl[] = {
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE
                     , found_genericata),
    PCI_DEVICE(PCI_VENDOR_ID_ATI, 0x4391, found_compatibleahci),
    PCI_DEVICE_END,
};

// Locate and init ata controllers.
ಸ್ಥಿರ ಶೂನ್ಯ
ata_scan(ಶೂನ್ಯ)
{
    ಯದಿ (CONFIG_QEMU && hlist_empty(&PCIDevices)) {
        // No PCI devices found - probably a QEMU "-M isapc" machine.
        // Try using ISA ports for ATA controllers.
        init_controller(ರಿಕ್ತ, 0, IRQ_ATA1
                        , PORT_ATA1_CMD_BASE, PORT_ATA1_CTRL_BASE, 0);
        init_controller(ರಿಕ್ತ, 1, IRQ_ATA2
                        , PORT_ATA2_CMD_BASE, PORT_ATA2_CTRL_BASE, 0);
        ಪ್ರತ್ಯಾಗಮ;
    }

    // Scan PCI bus for ATA adapters
    ಸಂರಚನಾ pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_ata_tbl, pci, ರಿಕ್ತ);
    }
}

ಶೂನ್ಯ
ata_setup(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಯದಿ (!CONFIG_ATA)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(3, "init hard drives\n");

    SpinupEnd = timer_calc(IDE_TIMEOUT);
    ata_scan();

    SET_BDA(disk_control_byte, 0xc0);

    enable_hwirq(14, FUNC16(entry_76));
}

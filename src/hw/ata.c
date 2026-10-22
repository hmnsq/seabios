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

#ವ್ಯಾಖ್ಯಾನಿಸು IDE_TIMEOUT 32000 //32 seconds max for IDE ops


/****************************************************************
 * Helper functions
 ****************************************************************/

// Wait for the specified ide state
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ
await_ide(u8 ಮುಖವಾಡ, u8 flags, u16 ತಳ, u16 timeout)
{
    u32 ಕೊನೆ = timer_calc(timeout);
    ಗಾಗಿ (;;) {
        u8 ಸ್ಥಿತಿ_2 = inb(ತಳ+ATA_CB_STAT);
        ಆದರೆ ((ಸ್ಥಿತಿ_2 & ಮುಖವಾಡ) == flags)
            ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            ಹಿಂತಿರುಗಿ -1;
        }
        yield();
    }
}

// Wait for the device to be not-busy.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
await_not_bsy(u16 ತಳ)
{
    ಹಿಂತಿರುಗಿ await_ide(ATA_CB_STAT_BSY, 0, ತಳ, IDE_TIMEOUT);
}

// Wait for the device to be ready.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
await_rdy(u16 ತಳ)
{
    ಹಿಂತಿರುಗಿ await_ide(ATA_CB_STAT_RDY, ATA_CB_STAT_RDY, ತಳ, IDE_TIMEOUT);
}

// Wait for ide state - pauses for one ata cycle first.
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ
pause_await_not_bsy(u16 iobase1, u16 iobase2)
{
    // Wait one PIO transfer cycle.
    inb(iobase2 + ATA_CB_ASTAT);

    ಹಿಂತಿರುಗಿ await_not_bsy(iobase1);
}

// Wait for ide state - pause for 400ns first.
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ
ndelay_await_not_bsy(u16 iobase1)
{
    ndelay(400);
    ಹಿಂತಿರುಗಿ await_not_bsy(iobase1);
}

// Reset a drive
ಸ್ಥಿರ ಶೂನ್ಯ
ata_reset(ರಚನೆ atadrive_s *adrive_gf)
{
    ರಚನೆ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
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
    ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = await_not_bsy(iobase1);
    ಆದರೆ (ಸ್ಥಿತಿ_2 < 0)
        ಹೋಗು ಮುಗಿದ;
    ಆದರೆ (slave) {
        // Change device.
        u32 ಕೊನೆ = timer_calc(IDE_TIMEOUT);
        ಗಾಗಿ (;;) {
            outb(ATA_CB_DH_DEV1, iobase1 + ATA_CB_DH);
            ಸ್ಥಿತಿ_2 = ndelay_await_not_bsy(iobase1);
            ಆದರೆ (ಸ್ಥಿತಿ_2 < 0)
                ಹೋಗು ಮುಗಿದ;
            ಆದರೆ (inb(iobase1 + ATA_CB_DH) == ATA_CB_DH_DEV1)
                ನಿಲ್ಲಿಸು;
            // Change drive request failed to take effect - retry.
            ಆದರೆ (timer_check(ಕೊನೆ)) {
                warn_timeout();
                ಹೋಗು ಮುಗಿದ;
            }
        }
    } ಇಲ್ಲದಿದ್ದರೆ {
        // QEMU doesn't reset dh on reset, so set it explicitly.
        outb(ATA_CB_DH_DEV0, iobase1 + ATA_CB_DH);
    }

    // On a user-reset request, wait for RDY if it is an ATA device.
    u8 type=GET_GLOBALFLAT(adrive_gf->drive.type);
    ಆದರೆ (type == DTYPE_ATA)
        ಸ್ಥಿತಿ_2 = await_rdy(iobase1);

ಮುಗಿದ:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    dprintf(6, "ata_reset exit status=%x\n", ಸ್ಥಿತಿ_2);
}

// Check for drive RDY for 16bit interface command.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
isready(ರಚನೆ atadrive_s *adrive_gf)
{
    // Read the status from controller
    ರಚನೆ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u8 ಸ್ಥಿತಿ_2 = inb(iobase1 + ATA_CB_STAT);
    ಆದರೆ ((ಸ್ಥಿತಿ_2 & (ATA_CB_STAT_BSY|ATA_CB_STAT_RDY)) == ATA_CB_STAT_RDY)
        ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
    ಹಿಂತಿರುಗಿ DISK_RET_ENOTREADY;
}


/****************************************************************
 * ATA send command
 ****************************************************************/

ರಚನೆ ata_pio_command {
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
send_cmd(ರಚನೆ atadrive_s *adrive_gf, ರಚನೆ ata_pio_command *cmd)
{
    ರಚನೆ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);

    // Select device
    ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = await_not_bsy(iobase1);
    ಆದರೆ (ಸ್ಥಿತಿ_2 < 0)
        ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
    u8 newdh = ((cmd->device & ~ATA_CB_DH_DEV1)
                | (slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0));
    u8 olddh = inb(iobase1 + ATA_CB_DH);
    outb(newdh, iobase1 + ATA_CB_DH);
    ಆದರೆ ((olddh ^ newdh) & (1<<4)) {
        // Was a device change - wait for device to become not busy.
        ಸ್ಥಿತಿ_2 = ndelay_await_not_bsy(iobase1);
        ಆದರೆ (ಸ್ಥಿತಿ_2 < 0)
            ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
    }

    // Check for ATA_CMD_(READ|WRITE)_(SECTORS|DMA)_EXT commands.
    ಆದರೆ ((cmd->command & ~0x11) == ATA_CMD_READ_SECTORS_EXT) {
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

    ಹಿಂತಿರುಗಿ 0;
}

// Wait for data after calling 'send_cmd'.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ata_wait_ದತ್ತಾಂಶ(u16 iobase1)
{
    ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = ndelay_await_not_bsy(iobase1);
    ಆದರೆ (ಸ್ಥಿತಿ_2 < 0)
        ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;

    ಆದರೆ (ಸ್ಥಿತಿ_2 & ATA_CB_STAT_ERR) {
        dprintf(6, "send_cmd : read error (status=%02x err=%02x)\n"
                , ಸ್ಥಿತಿ_2, inb(iobase1 + ATA_CB_ERR));
        ಹಿಂತಿರುಗಿ -4;
    }
    ಆದರೆ (!(ಸ್ಥಿತಿ_2 & ATA_CB_STAT_DRQ)) {
        dprintf(6, "send_cmd : DRQ not set (status %02x)\n", ಸ್ಥಿತಿ_2);
        ಹಿಂತಿರುಗಿ -5;
    }

    ಹಿಂತಿರುಗಿ 0;
}

// Send an ata command that does not transfer any further data.
ಪೂರ್ಣಾಂಕ
ata_cmd_nondata(ರಚನೆ atadrive_s *adrive_gf, ರಚನೆ ata_pio_command *cmd)
{
    ರಚನೆ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    ಪೂರ್ಣಾಂಕ ret = send_cmd(adrive_gf, cmd);
    ಆದರೆ (ret)
        ಹೋಗು fail;
    ret = ndelay_await_not_bsy(iobase1);
    ಆದರೆ (ret < 0)
        ಹೋಗು fail;

    ಆದರೆ (ret & ATA_CB_STAT_ERR) {
        dprintf(6, "nondata cmd : read error (status=%02x err=%02x)\n"
                , ret, inb(iobase1 + ATA_CB_ERR));
        ret = -4;
        ಹೋಗು fail;
    }
    ಆದರೆ (ret & ATA_CB_STAT_DRQ) {
        dprintf(6, "nondata cmd : DRQ set (status %02x)\n", ret);
        ret = -5;
        ಹೋಗು fail;
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    ಹಿಂತಿರುಗಿ ret;
}


/****************************************************************
 * ATA PIO transfers
 ****************************************************************/

// Transfer 'op->count' blocks (of 'blocksize' bytes) to/from drive
// 'op->drive_fl'.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ata_pio_transfer(ರಚನೆ disk_op_s *op, ಪೂರ್ಣಾಂಕ iswrite, ಪೂರ್ಣಾಂಕ blocksize)
{
    dprintf(16, "ata_pio_transfer id=%p write=%d count=%d bs=%d buf=%p\n"
            , op->drive_fl, iswrite, op->ಎಣಿಕೆ_2, blocksize, op->buf_fl);

    ರಚನೆ atadrive_s *adrive_gf = container_ದ(
        op->drive_fl, ರಚನೆ atadrive_s, drive);
    ರಚನೆ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = op->ಎಣಿಕೆ_2;
    ಶೂನ್ಯ *buf_fl = op->buf_fl;
    ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
    ಗಾಗಿ (;;) {
        ಆದರೆ (iswrite) {
            // Write data to controller
            dprintf(16, "Write sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            ಆದರೆ (CONFIG_ATA_PIO32)
                outsl_fl(iobase1, buf_fl, blocksize / 4);
            ಇಲ್ಲದಿದ್ದರೆ
                outsw_fl(iobase1, buf_fl, blocksize / 2);
        } ಇಲ್ಲದಿದ್ದರೆ {
            // Read data from controller
            dprintf(16, "Read sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            ಆದರೆ (CONFIG_ATA_PIO32)
                insl_fl(iobase1, buf_fl, blocksize / 4);
            ಇಲ್ಲದಿದ್ದರೆ
                insw_fl(iobase1, buf_fl, blocksize / 2);
        }
        buf_fl += blocksize;

        ಸ್ಥಿತಿ_2 = pause_await_not_bsy(iobase1, iobase2);
        ಆದರೆ (ಸ್ಥಿತಿ_2 < 0) {
            // Error
            op->ಎಣಿಕೆ_2 -= ಎಣಿಕೆ_2;
            ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
        }

        ಎಣಿಕೆ_2--;
        ಆದರೆ (!ಎಣಿಕೆ_2)
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ_2 &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ | ATA_CB_STAT_ERR);
        ಆದರೆ (ಸ್ಥಿತಿ_2 != ATA_CB_STAT_DRQ) {
            dprintf(6, "ata_pio_transfer : more sectors left (status %02x)\n"
                    , ಸ್ಥಿತಿ_2);
            op->ಎಣಿಕೆ_2 -= ಎಣಿಕೆ_2;
            ಹಿಂತಿರುಗಿ -6;
        }
    }

    ಸ್ಥಿತಿ_2 &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
               | ATA_CB_STAT_ERR);
    ಆದರೆ (!iswrite)
        ಸ್ಥಿತಿ_2 &= ~ATA_CB_STAT_DF;
    ಆದರೆ (ಸ್ಥಿತಿ_2 != 0) {
        dprintf(6, "ata_pio_transfer : no sectors left (status %02x)\n", ಸ್ಥಿತಿ_2);
        ಹಿಂತಿರುಗಿ -7;
    }

    ಹಿಂತಿರುಗಿ 0;
}


/****************************************************************
 * ATA DMA transfers
 ****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು BM_CMD    0
#ವ್ಯಾಖ್ಯಾನಿಸು  BM_CMD_MEMWRITE  0x08
#ವ್ಯಾಖ್ಯಾನಿಸು  BM_CMD_ಆರಂಭ     0x01
#ವ್ಯಾಖ್ಯಾನಿಸು BM_ಸ್ಥಿತಿ 2
#ವ್ಯಾಖ್ಯಾನಿಸು  BM_ಸ್ಥಿತಿ_IRQ    0x04
#ವ್ಯಾಖ್ಯಾನಿಸು  BM_ಸ್ಥಿತಿ_ERROR  0x02
#ವ್ಯಾಖ್ಯಾನಿಸು  BM_ಸ್ಥಿತಿ_ಸಕ್ರಿಯ 0x01
#ವ್ಯಾಖ್ಯಾನಿಸು BM_ಕೋಷ್ಟಕ  4

ರಚನೆ sff_dma_prd {
    u32 buf_fl;
    u32 ಎಣಿಕೆ_2;
};

// Check if DMA available and setup transfer if so.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ata_try_dma(ರಚನೆ disk_op_s *op, ಪೂರ್ಣಾಂಕ iswrite, ಪೂರ್ಣಾಂಕ blocksize)
{
    ASSERT16();
    ಆದರೆ (! CONFIG_ATA_DMA)
        ಹಿಂತಿರುಗಿ -1;
    u32 dest = (u32)op->buf_fl;
    ಆದರೆ (dest & 1)
        // Need minimum alignment of 1.
        ಹಿಂತಿರುಗಿ -1;
    ರಚನೆ atadrive_s *adrive_gf = container_ದ(
        op->drive_fl, ರಚನೆ atadrive_s, drive);
    ರಚನೆ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);
    ಆದರೆ (! iomaster)
        ಹಿಂತಿರುಗಿ -1;
    u32 ಬೈಟುಗಳು = op->ಎಣಿಕೆ_2 * blocksize;
    ಆದರೆ (! ಬೈಟುಗಳು)
        ಹಿಂತಿರುಗಿ -1;

    // Build PRD dma structure.
    ರಚನೆ sff_dma_prd *dma = MAKE_FLATPTR(SEG_ಕೆಳಗಿನ, Extra_ರಾಶಿ);
    ರಚನೆ sff_dma_prd *origdma = dma;
    ಆಗ (ಬೈಟುಗಳು) {
        ಆದರೆ (dma >= &origdma[16])
            // Too many descriptors..
            ಹಿಂತಿರುಗಿ -1;
        u32 ಎಣಿಕೆ_2 = ಬೈಟುಗಳು;
        u32 max = 0x10000 - (dest & 0xffff);
        ಆದರೆ (ಎಣಿಕೆ_2 > max)
            ಎಣಿಕೆ_2 = max;

        ಗಣ_LOWFLAT(dma->buf_fl, dest);
        ಬೈಟುಗಳು -= ಎಣಿಕೆ_2;
        ಆದರೆ (!ಬೈಟುಗಳು)
            // Last descriptor.
            ಎಣಿಕೆ_2 |= 1<<31;
        dprintf(16, "dma@%p: %08x %08x\n", dma, dest, ಎಣಿಕೆ_2);
        dest += ಎಣಿಕೆ_2;
        ಗಣ_LOWFLAT(dma->ಎಣಿಕೆ_2, ಎಣಿಕೆ_2);
        dma++;
    }

    // Program bus-master controller.
    outl((u32)origdma, iomaster + BM_ಕೋಷ್ಟಕ);
    u8 oldcmd = inb(iomaster + BM_CMD) & ~(BM_CMD_MEMWRITE|BM_CMD_ಆರಂಭ);
    outb(oldcmd | (iswrite ? 0x00 : BM_CMD_MEMWRITE), iomaster + BM_CMD);
    outb(BM_ಸ್ಥಿತಿ_ERROR|BM_ಸ್ಥಿತಿ_IRQ, iomaster + BM_ಸ್ಥಿತಿ);

    ಹಿಂತಿರುಗಿ 0;
}

// Transfer data using DMA.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ata_dma_transfer(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (! CONFIG_ATA_DMA)
        ಹಿಂತಿರುಗಿ -1;
    dprintf(16, "ata_dma_transfer id=%p buf=%p\n", op->drive_fl, op->buf_fl);

    ರಚನೆ atadrive_s *adrive_gf = container_ದ(
        op->drive_fl, ರಚನೆ atadrive_s, drive);
    ರಚನೆ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);

    // Start bus-master controller.
    u8 oldcmd = inb(iomaster + BM_CMD);
    outb(oldcmd | BM_CMD_ಆರಂಭ, iomaster + BM_CMD);

    u32 ಕೊನೆ = timer_calc(IDE_TIMEOUT);
    u8 ಸ್ಥಿತಿ_2;
    ಗಾಗಿ (;;) {
        ಸ್ಥಿತಿ_2 = inb(iomaster + BM_ಸ್ಥಿತಿ);
        ಆದರೆ (ಸ್ಥಿತಿ_2 & BM_ಸ್ಥಿತಿ_IRQ)
            ನಿಲ್ಲಿಸು;
        // Transfer in progress
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            // Timeout.
            warn_timeout();
            ನಿಲ್ಲಿಸು;
        }
        yield();
    }
    outb(oldcmd & ~BM_CMD_ಆರಂಭ, iomaster + BM_CMD);

    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    ಪೂರ್ಣಾಂಕ idestatus = pause_await_not_bsy(iobase1, iobase2);

    ಆದರೆ ((ಸ್ಥಿತಿ_2 & (BM_ಸ್ಥಿತಿ_IRQ|BM_ಸ್ಥಿತಿ_ಸಕ್ರಿಯ)) == BM_ಸ್ಥಿತಿ_IRQ
        && idestatus >= 0x00
        && (idestatus & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
                         | ATA_CB_STAT_ERR)) == 0x00)
        // Success.
        ಹಿಂತಿರುಗಿ 0;

    dprintf(6, "IDE DMA error (dma=%x ide=%x/%x/%x)\n", ಸ್ಥಿತಿ_2, idestatus
            , inb(iobase2 + ATA_CB_ASTAT), inb(iobase1 + ATA_CB_ERR));
    ಹಿಂತಿರುಗಿ -1;
}


/****************************************************************
 * ATA hard drive functions
 ****************************************************************/

// Transfer data to harddrive using PIO protocol.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ata_pio_cmd_ದತ್ತಾಂಶ(ರಚನೆ disk_op_s *op, ಪೂರ್ಣಾಂಕ iswrite, ರಚನೆ ata_pio_command *cmd)
{
    ರಚನೆ atadrive_s *adrive_gf = container_ದ(
        op->drive_fl, ರಚನೆ atadrive_s, drive);
    ರಚನೆ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    ಪೂರ್ಣಾಂಕ ret = send_cmd(adrive_gf, cmd);
    ಆದರೆ (ret)
        ಹೋಗು fail;
    ret = ata_wait_ದತ್ತಾಂಶ(iobase1);
    ಆದರೆ (ret)
        ಹೋಗು fail;
    ret = ata_pio_transfer(op, iswrite, DISK_SECTOR_ಗಾತ್ರ);

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    ಹಿಂತಿರುಗಿ ret;
}

// Transfer data to harddrive using DMA protocol.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ata_dma_cmd_ದತ್ತಾಂಶ(ರಚನೆ disk_op_s *op, ರಚನೆ ata_pio_command *cmd)
{
    ಆದರೆ (! CONFIG_ATA_DMA)
        ಹಿಂತಿರುಗಿ -1;
    ರಚನೆ atadrive_s *adrive_gf = container_ದ(
        op->drive_fl, ರಚನೆ atadrive_s, drive);
    ಪೂರ್ಣಾಂಕ ret = send_cmd(adrive_gf, cmd);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ret;
    ಹಿಂತಿರುಗಿ ata_dma_transfer(op);
}

// Read/write count blocks from a harddrive.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ata_readwrite(ರಚನೆ disk_op_s *op, ಪೂರ್ಣಾಂಕ iswrite)
{
    u64 lba = op->lba;

    ಪೂರ್ಣಾಂಕ usepio = ata_try_dma(op, iswrite, DISK_SECTOR_ಗಾತ್ರ);

    ರಚನೆ ata_pio_command cmd;
    memset(&cmd, 0, ಗಾತ್ರ(cmd));

    ಆದರೆ (op->ಎಣಿಕೆ_2 >= (1<<8) || lba + op->ಎಣಿಕೆ_2 >= (1<<28)) {
        cmd.sector_count2 = op->ಎಣಿಕೆ_2 >> 8;
        cmd.lba_low2 = lba >> 24;
        cmd.lba_mid2 = lba >> 32;
        cmd.lba_high2 = lba >> 40;
        lba &= 0xffffff;

        ಆದರೆ (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS_EXT
                           : ATA_CMD_READ_SECTORS_EXT);
        ಇಲ್ಲದಿದ್ದರೆ
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                           : ATA_CMD_READ_DMA_EXT);
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಆದರೆ (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS
                           : ATA_CMD_READ_SECTORS);
        ಇಲ್ಲದಿದ್ದರೆ
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA
                           : ATA_CMD_READ_DMA);
    }

    cmd.sector_ಎಣಿಕೆ = op->ಎಣಿಕೆ_2;
    cmd.lba_ಕೆಳಗಿನ = lba;
    cmd.lba_mid = lba >> 8;
    cmd.lba_ಮೇಲಿನ = lba >> 16;
    cmd.device = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;

    ಪೂರ್ಣಾಂಕ ret;
    ಆದರೆ (usepio)
        ret = ata_pio_cmd_ದತ್ತಾಂಶ(op, iswrite, &cmd);
    ಇಲ್ಲದಿದ್ದರೆ
        ret = ata_dma_cmd_ದತ್ತಾಂಶ(op, &cmd);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}

// 16bit command demuxer for ATA harddrives.
ಪೂರ್ಣಾಂಕ
ata_process_op(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (!CONFIG_ATA)
        ಹಿಂತಿರುಗಿ 0;

    ರಚನೆ atadrive_s *adrive_gf = container_ದ(
        op->drive_fl, ರಚನೆ atadrive_s, drive);
    ಆಯ್ಕೆ (op->command) {
    ಸ್ಥಿತಿ CMD_READ:
        ಹಿಂತಿರುಗಿ ata_readwrite(op, 0);
    ಸ್ಥಿತಿ CMD_WRITE:
        ಹಿಂತಿರುಗಿ ata_readwrite(op, 1);
    ಸ್ಥಿತಿ CMD_RESET:
        ata_reset(adrive_gf);
        ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
    ಸ್ಥಿತಿ CMD_ISREADY:
        ಹಿಂತಿರುಗಿ isready(adrive_gf);
    ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ default_process_op(op);
    }
}


/****************************************************************
 * ATAPI functions
 ****************************************************************/

#ವ್ಯಾಖ್ಯಾನಿಸು CDROM_CDB_ಗಾತ್ರ 12

// Low-level atapi command transmit function.
ಪೂರ್ಣಾಂಕ
ata_atapi_process_op(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (! CONFIG_ATA)
        ಹಿಂತಿರುಗಿ 0;

    ಆದರೆ (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        ಹಿಂತಿರುಗಿ DISK_RET_EWRITEPROTECT;
    u8 cdbcmd[CDROM_CDB_ಗಾತ್ರ];
    ಪೂರ್ಣಾಂಕ blocksize = scsi_fill_cmd(op, cdbcmd, ಗಾತ್ರ(cdbcmd));
    ಆದರೆ (blocksize < 0)
        ಹಿಂತಿರುಗಿ default_process_op(op);

    ರಚನೆ atadrive_s *adrive_gf = container_ದ(
        op->drive_fl, ರಚನೆ atadrive_s, drive);
    ರಚನೆ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    ರಚನೆ ata_pio_command cmd;
    memset(&cmd, 0, ಗಾತ್ರ(cmd));
    cmd.lba_mid = blocksize;
    cmd.lba_ಮೇಲಿನ = blocksize >> 8;
    cmd.command = ATA_CMD_PACKET;

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    ಪೂರ್ಣಾಂಕ ret = send_cmd(adrive_gf, &cmd);
    ಆದರೆ (ret)
        ಹೋಗು fail;
    ret = ata_wait_ದತ್ತಾಂಶ(iobase1);
    ಆದರೆ (ret)
        ಹೋಗು fail;

    // Send command to device
    outsw_fl(iobase1, MAKE_FLATPTR(GET_SEG(SS), cdbcmd), CDROM_CDB_ಗಾತ್ರ / 2);

    ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = pause_await_not_bsy(iobase1, iobase2);
    ಆದರೆ (ಸ್ಥಿತಿ_2 < 0) {
        ret = ಸ್ಥಿತಿ_2;
        ಹೋಗು fail;
    }

    ಆದರೆ (ಸ್ಥಿತಿ_2 & ATA_CB_STAT_ERR) {
        u8 err = inb(iobase1 + ATA_CB_ERR);
        // skip "Not Ready"
        ಆದರೆ (err != 0x20)
            dprintf(6, "send_atapi_cmd : read error (status=%02x err=%02x)\n"
                    , ಸ್ಥಿತಿ_2, err);
        ret = -2;
        ಹೋಗು fail;
    }
    ಆದರೆ (blocksize) {
        ಆದರೆ (!(ಸ್ಥಿತಿ_2 & ATA_CB_STAT_DRQ)) {
            dprintf(6, "send_atapi_cmd : DRQ not set (status %02x)\n", ಸ್ಥಿತಿ_2);
            ret = -3;
            ಹೋಗು fail;
        }

        ret = ata_pio_transfer(op, 0, blocksize);
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}


/****************************************************************
 * ATA detect and init
 ****************************************************************/

// Send an identify device or identify device packet command.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
send_ata_ತಟಸ್ಥ(ರಚನೆ atadrive_s *adrive, u16 *ಬಫರ್, ಪೂರ್ಣಾಂಕ command)
{
    memset(ಬಫರ್, 0, DISK_SECTOR_ಗಾತ್ರ);

    ರಚನೆ disk_op_s dop;
    memset(&dop, 0, ಗಾತ್ರ(dop));
    dop.drive_fl = &adrive->drive;
    dop.ಎಣಿಕೆ_2 = 1;
    dop.lba = 1;
    dop.buf_fl = MAKE_FLATPTR(GET_SEG(SS), ಬಫರ್);

    ರಚನೆ ata_pio_command cmd;
    memset(&cmd, 0, ಗಾತ್ರ(cmd));
    cmd.command = command;

    ಹಿಂತಿರುಗಿ ata_pio_cmd_ದತ್ತಾಂಶ(&dop, 0, &cmd);
}

// Extract the ATA/ATAPI version info.
ಪೂರ್ಣಾಂಕ
ata_ಹೊರತೆಗೆ_ಆವೃತ್ತಿ(u16 *ಬಫರ್)
{
    // Extract ATA/ATAPI version.
    u16 ataversion = ಬಫರ್[80];
    u8 ಆವೃತ್ತಿ_2;
    ಗಾಗಿ (ಆವೃತ್ತಿ_2=15; ಆವೃತ್ತಿ_2>0; ಆವೃತ್ತಿ_2--)
        ಆದರೆ (ataversion & (1<<ಆವೃತ್ತಿ_2))
            ನಿಲ್ಲಿಸು;
    ಹಿಂತಿರುಗಿ ಆವೃತ್ತಿ_2;
}

#ವ್ಯಾಖ್ಯಾನಿಸು MAXMODEL 40

// Extract the ATA/ATAPI model info.
ಅಕ್ಷರ *
ata_ಹೊರತೆಗೆ_model(ಅಕ್ಷರ *model, u32 ಗಾತ್ರ_2, u16 *ಬಫರ್)
{
    // Read model name
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i=0; i<ಗಾತ್ರ_2/2; i++)
        *(u16*)&model[i*2] = be16_ಗಮ್ಯ_cpu(ಬಫರ್[27+i]);
    model[ಗಾತ್ರ_2] = 0x00;
    nullTrailingSpace(model);
    ಹಿಂತಿರುಗಿ model;
}

// Common init code between ata and atapi
ಸ್ಥಿರ ರಚನೆ atadrive_s *
init_atadrive(ರಚನೆ atadrive_s *dummy, u16 *ಬಫರ್)
{
    ರಚನೆ atadrive_s *adrive = malloc_fseg(ಗಾತ್ರ(*adrive));
    ಆದರೆ (!adrive) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    memset(adrive, 0, ಗಾತ್ರ(*adrive));
    adrive->chan_gf = dummy->chan_gf;
    adrive->slave = dummy->slave;
    adrive->drive.cntl_id = adrive->chan_gf->ataid * 2 + dummy->slave;
    adrive->drive.removable = (ಬಫರ್[0] & 0x80) ? 1 : 0;
    ಹಿಂತಿರುಗಿ adrive;
}

// Detect if the given drive is an atapi - initialize it if so.
ಸ್ಥಿರ ರಚನೆ atadrive_s *
init_drive_atapi(ರಚನೆ atadrive_s *dummy, u16 *ಬಫರ್)
{
    // Send an IDENTIFY_DEVICE_PACKET command to device
    ಪೂರ್ಣಾಂಕ ret = send_ata_ತಟಸ್ಥ(dummy, ಬಫರ್, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    // Success - setup as ATAPI.
    ರಚನೆ atadrive_s *adrive = init_atadrive(dummy, ಬಫರ್);
    ಆದರೆ (!adrive)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    adrive->drive.type = DTYPE_ATA_ATAPI;
    adrive->drive.blksize = CDROM_SECTOR_ಗಾತ್ರ;
    adrive->drive.sectors = (u64)-1;
    u8 iscd = ((ಬಫರ್[0] >> 8) & 0x1f) == 0x05;
    ಅಕ್ಷರ model[MAXMODEL+1];
    ಅಕ್ಷರ *desc = znprintf(MAXDESCSIZE
                          , "DVD/CD [ata%d-%d: %s ATAPI-%d %s]"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_ಹೊರತೆಗೆ_model(model, MAXMODEL, ಬಫರ್)
                          , ata_ಹೊರತೆಗೆ_ಆವೃತ್ತಿ(ಬಫರ್)
                          , (iscd ? "DVD/CD" : "Device"));
    dprintf(1, "%s\n", desc);

    // fill cdidmap
    ಆದರೆ (iscd) {
        ಪೂರ್ಣಾಂಕ prio = bootprio_ಕಂಡುಹಿಡಿ_ata_device(adrive->chan_gf->pci_tmp,
                                            adrive->chan_gf->chanid,
                                            adrive->slave);
        boot_lchs_ಕಂಡುಹಿಡಿ_ata_device(adrive->chan_gf->pci_tmp,
                                  adrive->chan_gf->chanid,
                                  adrive->slave,
                                  &(adrive->drive.lchs));
        boot_ಕೂಡಿಸು_cd(&adrive->drive, desc, prio);
    }

    ಹಿಂತಿರುಗಿ adrive;
}

// Detect if the given drive is a regular ata drive - initialize it if so.
ಸ್ಥಿರ ರಚನೆ atadrive_s *
init_drive_ata(ರಚನೆ atadrive_s *dummy, u16 *ಬಫರ್)
{
    // Send an IDENTIFY_DEVICE command to device
    ಪೂರ್ಣಾಂಕ ret = send_ata_ತಟಸ್ಥ(dummy, ಬಫರ್, ATA_CMD_IDENTIFY_DEVICE);
    ಆದರೆ (ret)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    // Success - setup as ATA.
    ರಚನೆ atadrive_s *adrive = init_atadrive(dummy, ಬಫರ್);
    ಆದರೆ (!adrive)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    adrive->drive.type = DTYPE_ATA;
    adrive->drive.blksize = DISK_SECTOR_ಗಾತ್ರ;

    adrive->drive.pchs.cylinder = ಬಫರ್[1];
    adrive->drive.pchs.ತಲೆ = ಬಫರ್[3];
    adrive->drive.pchs.sector = ಬಫರ್[6];

    u64 sectors;
    ಆದರೆ (ಬಫರ್[83] & (1 << 10)) // word 83 - lba48 support
        sectors = *(u64*)&ಬಫರ್[100]; // word 100-103
    ಇಲ್ಲದಿದ್ದರೆ
        sectors = *(u32*)&ಬಫರ್[60]; // word 60 and word 61
    adrive->drive.sectors = sectors;
    u64 adjsize = sectors >> 11;
    ಅಕ್ಷರ adjprefix = 'M';
    ಆದರೆ (adjsize >= (1 << 16)) {
        adjsize >>= 10;
        adjprefix = 'G';
    }
    ಅಕ್ಷರ model[MAXMODEL+1];
    ಅಕ್ಷರ *desc = znprintf(MAXDESCSIZE
                          , "ata%d-%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_ಹೊರತೆಗೆ_model(model, MAXMODEL, ಬಫರ್)
                          , ata_ಹೊರತೆಗೆ_ಆವೃತ್ತಿ(ಬಫರ್)
                          , (u32)adjsize, adjprefix);
    dprintf(1, "%s\n", desc);

    ಪೂರ್ಣಾಂಕ prio = bootprio_ಕಂಡುಹಿಡಿ_ata_device(adrive->chan_gf->pci_tmp,
                                        adrive->chan_gf->chanid,
                                        adrive->slave);
    boot_lchs_ಕಂಡುಹಿಡಿ_ata_device(adrive->chan_gf->pci_tmp,
                              adrive->chan_gf->chanid,
                              adrive->slave,
                              &(adrive->drive.lchs));
    // Register with bcv system.
    boot_ಕೂಡಿಸು_hd(&adrive->drive, desc, prio);

    ಹಿಂತಿರುಗಿ adrive;
}

ಸ್ಥಿರ u32 Spinup_ಕೊನೆ;

// Wait for non-busy status and check for "floating bus" condition.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
powerup_await_non_bsy(u16 ತಳ)
{
    u8 orstatus = 0;
    u8 ಸ್ಥಿತಿ_2;
    ಗಾಗಿ (;;) {
        ಸ್ಥಿತಿ_2 = inb(ತಳ+ATA_CB_STAT);
        ಆದರೆ (!(ಸ್ಥಿತಿ_2 & ATA_CB_STAT_BSY))
            ನಿಲ್ಲಿಸು;
        orstatus |= ಸ್ಥಿತಿ_2;
        ಆದರೆ (orstatus == 0xff) {
            dprintf(4, "powerup IDE floating\n");
            ಹಿಂತಿರುಗಿ orstatus;
        }
        ಆದರೆ (timer_check(Spinup_ಕೊನೆ)) {
            warn_timeout();
            ಹಿಂತಿರುಗಿ -1;
        }
        yield();
    }
    dprintf(6, "powerup iobase=%x st=%x\n", ತಳ, ಸ್ಥಿತಿ_2);
    ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}

// Detect any drives attached to a given controller.
ಸ್ಥಿರ ಶೂನ್ಯ
ata_detect(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ರಚನೆ ata_channel_s *chan_gf = ದತ್ತಾಂಶ;
    ರಚನೆ atadrive_s dummy;
    memset(&dummy, 0, ಗಾತ್ರ(dummy));
    dummy.chan_gf = chan_gf;
    // Device detection
    ಪೂರ್ಣಾಂಕ didreset = 0;
    u8 slave;
    ಗಾಗಿ (slave=0; slave<=1; slave++) {
        // Wait for not-bsy.
        u16 iobase1 = chan_gf->iobase1;
        ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = powerup_await_non_bsy(iobase1);
        ಆದರೆ (ಸ್ಥಿತಿ_2 < 0)
            ಮುಂದುವರಿ;
        u8 newdh = slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0;
        outb(newdh, iobase1+ATA_CB_DH);
        ndelay(400);
        ಸ್ಥಿತಿ_2 = powerup_await_non_bsy(iobase1);
        ಆದರೆ (ಸ್ಥಿತಿ_2 < 0)
            ಮುಂದುವರಿ;

        // Check if ioport registers look valid.
        outb(newdh, iobase1+ATA_CB_DH);
        u8 dh = inb(iobase1+ATA_CB_DH);
        outb(0x55, iobase1+ATA_CB_SC);
        outb(0xaa, iobase1+ATA_CB_SN);
        u8 sc = inb(iobase1+ATA_CB_SC);
        u8 sn = inb(iobase1+ATA_CB_SN);
        dprintf(6, "ata_detect ata%d-%d: sc=%x sn=%x dh=%x\n"
                , chan_gf->ataid, slave, sc, sn, dh);
        ಆದರೆ (sc != 0x55 || sn != 0xaa || dh != newdh)
            ಮುಂದುವರಿ;

        // Prepare new drive.
        dummy.slave = slave;

        // reset the channel
        ಆದರೆ (!didreset) {
            ata_reset(&dummy);
            didreset = 1;
        }

        // check for ATAPI
        u16 ಬಫರ್[256];
        ರಚನೆ atadrive_s *adrive = init_drive_atapi(&dummy, ಬಫರ್);
        ಆದರೆ (!adrive) {
            // Didn't find an ATAPI drive - look for ATA drive.
            u8 st = inb(iobase1+ATA_CB_STAT);
            ಆದರೆ (!st)
                // Status not set - can't be a valid drive.
                ಮುಂದುವರಿ;

            // Wait for RDY.
            ಪೂರ್ಣಾಂಕ ret = await_rdy(iobase1);
            ಆದರೆ (ret < 0)
                ಮುಂದುವರಿ;

            // check for ATA.
            adrive = init_drive_ata(&dummy, ಬಫರ್);
            ಆದರೆ (!adrive)
                // No ATA drive found
                ಮುಂದುವರಿ;
        }

        u16 resetresult = ಬಫರ್[93];
        dprintf(6, "ata_detect resetresult=%04x\n", resetresult);
        ಆದರೆ (!slave && (resetresult & 0xdf61) == 0x4041)
            // resetresult looks valid and device 0 is responding to
            // device 1 requests - device 1 must not be present - skip
            // detection.
            ನಿಲ್ಲಿಸು;
    }
}

// Initialize an ata controller and detect its drives.
ಸ್ಥಿರ ಶೂನ್ಯ
init_controller(ರಚನೆ pci_device *pci, ಪೂರ್ಣಾಂಕ chanid, ಪೂರ್ಣಾಂಕ irq
                , u32 port1, u32 port2, u32 master)
{
    ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ataid = 0;
    ರಚನೆ ata_channel_s *chan_gf = malloc_fseg(ಗಾತ್ರ(*chan_gf));
    ಆದರೆ (!chan_gf) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
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
    ಸತತ_thread(ata_detect, chan_gf);
}

#ವ್ಯಾಖ್ಯಾನಿಸು IRQ_ATA1 14
#ವ್ಯಾಖ್ಯಾನಿಸು IRQ_ATA2 15

// Handle controllers on an ATA PCI device.
ಸ್ಥಿರ ಶೂನ್ಯ
init_pciata(ರಚನೆ pci_device *pci, u8 prog_if)
{
    u8 pciirq = pci_config_readb(pci->bdf, PCI_ಅಡಚಣೆ_ಸಾಲು);
    ಪೂರ್ಣಾಂಕ master = 0;
    ಆದರೆ (CONFIG_ATA_DMA && prog_if & 0x80) {
        // Check for bus-mastering.
        u32 bar = pci_config_readl(pci->bdf, PCI_ತಳ_ವಿಳಾಸ_4);
        ಆದರೆ (bar & PCI_ತಳ_ವಿಳಾಸ_SPACE_IO) {
            master = pci_enable_iobar(pci, PCI_ತಳ_ವಿಳಾಸ_4);
            pci_enable_busmaster(pci);
        }
    }

    u32 port1, port2, irq;
    ಆದರೆ (prog_if & 1) {
        port1 = pci_enable_iobar(pci, PCI_ತಳ_ವಿಳಾಸ_0);
        port2 = pci_enable_iobar(pci, PCI_ತಳ_ವಿಳಾಸ_1);
        ಆದರೆ (!port1 || !port2)
            ಹಿಂತಿರುಗಿ;
        irq = pciirq;
    } ಇಲ್ಲದಿದ್ದರೆ {
        port1 = ಪೋರ್ಟ್_ATA1_CMD_ತಳ;
        port2 = ಪೋರ್ಟ್_ATA1_CTRL_ತಳ;
        irq = IRQ_ATA1;
    }
    init_controller(pci, 0, irq, port1, port2, master);

    ಆದರೆ (prog_if & 4) {
        port1 = pci_enable_iobar(pci, PCI_ತಳ_ವಿಳಾಸ_2);
        port2 = pci_enable_iobar(pci, PCI_ತಳ_ವಿಳಾಸ_3);
        ಆದರೆ (!port1 || !port2)
            ಹಿಂತಿರುಗಿ;
        irq = pciirq;
    } ಇಲ್ಲದಿದ್ದರೆ {
        port1 = ಪೋರ್ಟ್_ATA2_CMD_ತಳ;
        port2 = ಪೋರ್ಟ್_ATA2_CTRL_ತಳ;
        irq = IRQ_ATA2;
    }
    init_controller(pci, 1, irq, port1, port2, master ? master + 8 : 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ
ಸಿಕ್ಕಿತು_genericata(ರಚನೆ pci_device *pci, ಶೂನ್ಯ *arg)
{
    init_pciata(pci, pci->prog_if);
}

ಸ್ಥಿರ ಶೂನ್ಯ
ಸಿಕ್ಕಿತು_compatibleahci(ರಚನೆ pci_device *pci, ಶೂನ್ಯ *arg)
{
    ಆದರೆ (CONFIG_AHCI)
        // Already handled directly via native ahci interface.
        ಹಿಂತಿರುಗಿ;
    init_pciata(pci, 0x8f);
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ pci_device_id pci_ata_tbl[] = {
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE
                     , ಸಿಕ್ಕಿತು_genericata),
    PCI_DEVICE(PCI_VENDOR_ID_ATI, 0x4391, ಸಿಕ್ಕಿತು_compatibleahci),
    PCI_DEVICE_ಕೊನೆ,
};

// Locate and init ata controllers.
ಸ್ಥಿರ ಶೂನ್ಯ
ata_ಪರಿಶೀಲನೆ(ಶೂನ್ಯ)
{
    ಆದರೆ (CONFIG_QEMU && hlist_ಖಾಲಿಗಳು(&PCIDevices)) {
        // No PCI devices found - probably a QEMU "-M isapc" machine.
        // Try using ISA ports for ATA controllers.
        init_controller(ಖಾಲಿ, 0, IRQ_ATA1
                        , ಪೋರ್ಟ್_ATA1_CMD_ತಳ, ಪೋರ್ಟ್_ATA1_CTRL_ತಳ, 0);
        init_controller(ಖಾಲಿ, 1, IRQ_ATA2
                        , ಪೋರ್ಟ್_ATA2_CMD_ತಳ, ಪೋರ್ಟ್_ATA2_CTRL_ತಳ, 0);
        ಹಿಂತಿರುಗಿ;
    }

    // Scan PCI bus for ATA adapters
    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_ata_tbl, pci, ಖಾಲಿ);
    }
}

ಶೂನ್ಯ
ata_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಆದರೆ (!CONFIG_ATA)
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "init hard drives\n");

    Spinup_ಕೊನೆ = timer_calc(IDE_TIMEOUT);
    ata_ಪರಿಶೀಲನೆ();

    ಗಣ_BDA(disk_control_byte, 0xc0);

    enable_hwirq(14, FUNC16(ನಮೂದು_76));
}

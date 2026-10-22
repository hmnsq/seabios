// Low level ATA disk access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "ata.h" // ATA_CB_STAT
#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBALFLAT
#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // struct drive_s
#ਅਨ੍ਤਰ੍ਭਾਵ "blockcmd.h" // CDB_CMD_READ_10
#ਅਨ੍ਤਰ੍ਭਾਵ "byteorder.h" // be16_to_cpu
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // malloc_fseg
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pci.h" // pci_config_readb
#ਅਨ੍ਤਰ੍ਭਾਵ "pcidevice.h" // foreachpci
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_regs.h" // PCI_INTERRUPT_LINE
#ਅਨ੍ਤਰ੍ਭਾਵ "pic.h" // enable_hwirq
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // yield
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // DISK_RET_SUCCESS
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // timer_calc
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // inb

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IDE_TIMEOUT 32000 //32 seconds max for IDE ops


/****************************************************************
 * Helper functions
 ****************************************************************/

// Wait for the specified ide state
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ
await_ide(u8 ਨਕਾਬ, u8 flags, u16 ਅਧਾਰ, u16 timeout)
{
    u32 ਸਿਰਾ = timer_calc(timeout);
    ਲਈ (;;) {
        u8 ਸਥਿਤੀ = inb(ਅਧਾਰ+ATA_CB_STAT);
        ਜੇ ((ਸਥਿਤੀ & ਨਕਾਬ) == flags)
            ਵਾਪਸ ਸਥਿਤੀ;
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            ਵਾਪਸ -1;
        }
        yield();
    }
}

// Wait for the device to be not-busy.
ਸਥਿਰ ਪੂਰਨ
await_not_bsy(u16 ਅਧਾਰ)
{
    ਵਾਪਸ await_ide(ATA_CB_STAT_BSY, 0, ਅਧਾਰ, IDE_TIMEOUT);
}

// Wait for the device to be ready.
ਸਥਿਰ ਪੂਰਨ
await_rdy(u16 ਅਧਾਰ)
{
    ਵਾਪਸ await_ide(ATA_CB_STAT_RDY, ATA_CB_STAT_RDY, ਅਧਾਰ, IDE_TIMEOUT);
}

// Wait for ide state - pauses for one ata cycle first.
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ
pause_await_not_bsy(u16 iobase1, u16 iobase2)
{
    // Wait one PIO transfer cycle.
    inb(iobase2 + ATA_CB_ASTAT);

    ਵਾਪਸ await_not_bsy(iobase1);
}

// Wait for ide state - pause for 400ns first.
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ
ndelay_await_not_bsy(u16 iobase1)
{
    ndelay(400);
    ਵਾਪਸ await_not_bsy(iobase1);
}

// Reset a drive
ਸਥਿਰ ਖਾਲੀ
ata_reset(ਬਣਤਰ atadrive_s *adrive_gf)
{
    ਬਣਤਰ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
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
    ਪੂਰਨ ਸਥਿਤੀ = await_not_bsy(iobase1);
    ਜੇ (ਸਥਿਤੀ < 0)
        ਜਾਓ ਪੂਰਾ;
    ਜੇ (slave) {
        // Change device.
        u32 ਸਿਰਾ = timer_calc(IDE_TIMEOUT);
        ਲਈ (;;) {
            outb(ATA_CB_DH_DEV1, iobase1 + ATA_CB_DH);
            ਸਥਿਤੀ = ndelay_await_not_bsy(iobase1);
            ਜੇ (ਸਥਿਤੀ < 0)
                ਜਾਓ ਪੂਰਾ;
            ਜੇ (inb(iobase1 + ATA_CB_DH) == ATA_CB_DH_DEV1)
                ਰੋਕੋ;
            // Change drive request failed to take effect - retry.
            ਜੇ (timer_check(ਸਿਰਾ)) {
                warn_timeout();
                ਜਾਓ ਪੂਰਾ;
            }
        }
    } ਨਹੀਂਤਾਂ {
        // QEMU doesn't reset dh on reset, so set it explicitly.
        outb(ATA_CB_DH_DEV0, iobase1 + ATA_CB_DH);
    }

    // On a user-reset request, wait for RDY if it is an ATA device.
    u8 type=GET_GLOBALFLAT(adrive_gf->drive.type);
    ਜੇ (type == DTYPE_ATA)
        ਸਥਿਤੀ = await_rdy(iobase1);

ਪੂਰਾ:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    dprintf(6, "ata_reset exit status=%x\n", ਸਥਿਤੀ);
}

// Check for drive RDY for 16bit interface command.
ਸਥਿਰ ਪੂਰਨ
isready(ਬਣਤਰ atadrive_s *adrive_gf)
{
    // Read the status from controller
    ਬਣਤਰ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u8 ਸਥਿਤੀ = inb(iobase1 + ATA_CB_STAT);
    ਜੇ ((ਸਥਿਤੀ & (ATA_CB_STAT_BSY|ATA_CB_STAT_RDY)) == ATA_CB_STAT_RDY)
        ਵਾਪਸ DISK_RET_SUCCESS;
    ਵਾਪਸ DISK_RET_ENOTREADY;
}


/****************************************************************
 * ATA send command
 ****************************************************************/

ਬਣਤਰ ata_pio_command {
    u8 feature;
    u8 sector_ਗਿਣਤੀ;
    u8 lba_ਨੀਵਾਂ;
    u8 lba_mid;
    u8 lba_ਉੱਚਾ;
    u8 device;
    u8 command;

    u8 feature2;
    u8 sector_count2;
    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
};

// Send an ata command to the drive.
ਸਥਿਰ ਪੂਰਨ
send_cmd(ਬਣਤਰ atadrive_s *adrive_gf, ਬਣਤਰ ata_pio_command *cmd)
{
    ਬਣਤਰ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);

    // Select device
    ਪੂਰਨ ਸਥਿਤੀ = await_not_bsy(iobase1);
    ਜੇ (ਸਥਿਤੀ < 0)
        ਵਾਪਸ ਸਥਿਤੀ;
    u8 newdh = ((cmd->device & ~ATA_CB_DH_DEV1)
                | (slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0));
    u8 olddh = inb(iobase1 + ATA_CB_DH);
    outb(newdh, iobase1 + ATA_CB_DH);
    ਜੇ ((olddh ^ newdh) & (1<<4)) {
        // Was a device change - wait for device to become not busy.
        ਸਥਿਤੀ = ndelay_await_not_bsy(iobase1);
        ਜੇ (ਸਥਿਤੀ < 0)
            ਵਾਪਸ ਸਥਿਤੀ;
    }

    // Check for ATA_CMD_(READ|WRITE)_(SECTORS|DMA)_EXT commands.
    ਜੇ ((cmd->command & ~0x11) == ATA_CMD_READ_SECTORS_EXT) {
        outb(cmd->feature2, iobase1 + ATA_CB_FR);
        outb(cmd->sector_count2, iobase1 + ATA_CB_SC);
        outb(cmd->lba_low2, iobase1 + ATA_CB_SN);
        outb(cmd->lba_mid2, iobase1 + ATA_CB_CL);
        outb(cmd->lba_high2, iobase1 + ATA_CB_CH);
    }
    outb(cmd->feature, iobase1 + ATA_CB_FR);
    outb(cmd->sector_ਗਿਣਤੀ, iobase1 + ATA_CB_SC);
    outb(cmd->lba_ਨੀਵਾਂ, iobase1 + ATA_CB_SN);
    outb(cmd->lba_mid, iobase1 + ATA_CB_CL);
    outb(cmd->lba_ਉੱਚਾ, iobase1 + ATA_CB_CH);
    outb(cmd->command, iobase1 + ATA_CB_CMD);

    ਵਾਪਸ 0;
}

// Wait for data after calling 'send_cmd'.
ਸਥਿਰ ਪੂਰਨ
ata_wait_ਅੰਕੜੇ(u16 iobase1)
{
    ਪੂਰਨ ਸਥਿਤੀ = ndelay_await_not_bsy(iobase1);
    ਜੇ (ਸਥਿਤੀ < 0)
        ਵਾਪਸ ਸਥਿਤੀ;

    ਜੇ (ਸਥਿਤੀ & ATA_CB_STAT_ERR) {
        dprintf(6, "send_cmd : read error (status=%02x err=%02x)\n"
                , ਸਥਿਤੀ, inb(iobase1 + ATA_CB_ERR));
        ਵਾਪਸ -4;
    }
    ਜੇ (!(ਸਥਿਤੀ & ATA_CB_STAT_DRQ)) {
        dprintf(6, "send_cmd : DRQ not set (status %02x)\n", ਸਥਿਤੀ);
        ਵਾਪਸ -5;
    }

    ਵਾਪਸ 0;
}

// Send an ata command that does not transfer any further data.
ਪੂਰਨ
ata_cmd_nondata(ਬਣਤਰ atadrive_s *adrive_gf, ਬਣਤਰ ata_pio_command *cmd)
{
    ਬਣਤਰ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    ਪੂਰਨ ret = send_cmd(adrive_gf, cmd);
    ਜੇ (ret)
        ਜਾਓ fail;
    ret = ndelay_await_not_bsy(iobase1);
    ਜੇ (ret < 0)
        ਜਾਓ fail;

    ਜੇ (ret & ATA_CB_STAT_ERR) {
        dprintf(6, "nondata cmd : read error (status=%02x err=%02x)\n"
                , ret, inb(iobase1 + ATA_CB_ERR));
        ret = -4;
        ਜਾਓ fail;
    }
    ਜੇ (ret & ATA_CB_STAT_DRQ) {
        dprintf(6, "nondata cmd : DRQ set (status %02x)\n", ret);
        ret = -5;
        ਜਾਓ fail;
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    ਵਾਪਸ ret;
}


/****************************************************************
 * ATA PIO transfers
 ****************************************************************/

// Transfer 'op->count' blocks (of 'blocksize' bytes) to/from drive
// 'op->drive_fl'.
ਸਥਿਰ ਪੂਰਨ
ata_pio_transfer(ਬਣਤਰ disk_op_s *op, ਪੂਰਨ iswrite, ਪੂਰਨ blocksize)
{
    dprintf(16, "ata_pio_transfer id=%p write=%d count=%d bs=%d buf=%p\n"
            , op->drive_fl, iswrite, op->ਗਿਣਤੀ_2, blocksize, op->buf_fl);

    ਬਣਤਰ atadrive_s *adrive_gf = container_ਦਾ(
        op->drive_fl, ਬਣਤਰ atadrive_s, drive);
    ਬਣਤਰ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    ਪੂਰਨ ਗਿਣਤੀ_2 = op->ਗਿਣਤੀ_2;
    ਖਾਲੀ *buf_fl = op->buf_fl;
    ਪੂਰਨ ਸਥਿਤੀ;
    ਲਈ (;;) {
        ਜੇ (iswrite) {
            // Write data to controller
            dprintf(16, "Write sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            ਜੇ (CONFIG_ATA_PIO32)
                outsl_fl(iobase1, buf_fl, blocksize / 4);
            ਨਹੀਂਤਾਂ
                outsw_fl(iobase1, buf_fl, blocksize / 2);
        } ਨਹੀਂਤਾਂ {
            // Read data from controller
            dprintf(16, "Read sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            ਜੇ (CONFIG_ATA_PIO32)
                insl_fl(iobase1, buf_fl, blocksize / 4);
            ਨਹੀਂਤਾਂ
                insw_fl(iobase1, buf_fl, blocksize / 2);
        }
        buf_fl += blocksize;

        ਸਥਿਤੀ = pause_await_not_bsy(iobase1, iobase2);
        ਜੇ (ਸਥਿਤੀ < 0) {
            // Error
            op->ਗਿਣਤੀ_2 -= ਗਿਣਤੀ_2;
            ਵਾਪਸ ਸਥਿਤੀ;
        }

        ਗਿਣਤੀ_2--;
        ਜੇ (!ਗਿਣਤੀ_2)
            ਰੋਕੋ;
        ਸਥਿਤੀ &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ | ATA_CB_STAT_ERR);
        ਜੇ (ਸਥਿਤੀ != ATA_CB_STAT_DRQ) {
            dprintf(6, "ata_pio_transfer : more sectors left (status %02x)\n"
                    , ਸਥਿਤੀ);
            op->ਗਿਣਤੀ_2 -= ਗਿਣਤੀ_2;
            ਵਾਪਸ -6;
        }
    }

    ਸਥਿਤੀ &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
               | ATA_CB_STAT_ERR);
    ਜੇ (!iswrite)
        ਸਥਿਤੀ &= ~ATA_CB_STAT_DF;
    ਜੇ (ਸਥਿਤੀ != 0) {
        dprintf(6, "ata_pio_transfer : no sectors left (status %02x)\n", ਸਥਿਤੀ);
        ਵਾਪਸ -7;
    }

    ਵਾਪਸ 0;
}


/****************************************************************
 * ATA DMA transfers
 ****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BM_CMD    0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  BM_CMD_MEMWRITE  0x08
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  BM_CMD_ਸ਼ੁਰੂ     0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BM_ਸਥਿਤੀ 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  BM_ਸਥਿਤੀ_IRQ    0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  BM_ਸਥਿਤੀ_ERROR  0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ  BM_ਸਥਿਤੀ_ਸਰਗਰਮ 0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BM_ਸਾਰਣੀ  4

ਬਣਤਰ sff_dma_prd {
    u32 buf_fl;
    u32 ਗਿਣਤੀ_2;
};

// Check if DMA available and setup transfer if so.
ਸਥਿਰ ਪੂਰਨ
ata_try_dma(ਬਣਤਰ disk_op_s *op, ਪੂਰਨ iswrite, ਪੂਰਨ blocksize)
{
    ASSERT16();
    ਜੇ (! CONFIG_ATA_DMA)
        ਵਾਪਸ -1;
    u32 dest = (u32)op->buf_fl;
    ਜੇ (dest & 1)
        // Need minimum alignment of 1.
        ਵਾਪਸ -1;
    ਬਣਤਰ atadrive_s *adrive_gf = container_ਦਾ(
        op->drive_fl, ਬਣਤਰ atadrive_s, drive);
    ਬਣਤਰ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);
    ਜੇ (! iomaster)
        ਵਾਪਸ -1;
    u32 ਬਾਈਟਾਂ = op->ਗਿਣਤੀ_2 * blocksize;
    ਜੇ (! ਬਾਈਟਾਂ)
        ਵਾਪਸ -1;

    // Build PRD dma structure.
    ਬਣਤਰ sff_dma_prd *dma = MAKE_FLATPTR(SEG_ਨੀਵਾਂ, Extra_ਢੇਰ);
    ਬਣਤਰ sff_dma_prd *origdma = dma;
    ਜਦੋਂ (ਬਾਈਟਾਂ) {
        ਜੇ (dma >= &origdma[16])
            // Too many descriptors..
            ਵਾਪਸ -1;
        u32 ਗਿਣਤੀ_2 = ਬਾਈਟਾਂ;
        u32 max = 0x10000 - (dest & 0xffff);
        ਜੇ (ਗਿਣਤੀ_2 > max)
            ਗਿਣਤੀ_2 = max;

        ਸੈੱਟ_LOWFLAT(dma->buf_fl, dest);
        ਬਾਈਟਾਂ -= ਗਿਣਤੀ_2;
        ਜੇ (!ਬਾਈਟਾਂ)
            // Last descriptor.
            ਗਿਣਤੀ_2 |= 1<<31;
        dprintf(16, "dma@%p: %08x %08x\n", dma, dest, ਗਿਣਤੀ_2);
        dest += ਗਿਣਤੀ_2;
        ਸੈੱਟ_LOWFLAT(dma->ਗਿਣਤੀ_2, ਗਿਣਤੀ_2);
        dma++;
    }

    // Program bus-master controller.
    outl((u32)origdma, iomaster + BM_ਸਾਰਣੀ);
    u8 oldcmd = inb(iomaster + BM_CMD) & ~(BM_CMD_MEMWRITE|BM_CMD_ਸ਼ੁਰੂ);
    outb(oldcmd | (iswrite ? 0x00 : BM_CMD_MEMWRITE), iomaster + BM_CMD);
    outb(BM_ਸਥਿਤੀ_ERROR|BM_ਸਥਿਤੀ_IRQ, iomaster + BM_ਸਥਿਤੀ);

    ਵਾਪਸ 0;
}

// Transfer data using DMA.
ਸਥਿਰ ਪੂਰਨ
ata_dma_transfer(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (! CONFIG_ATA_DMA)
        ਵਾਪਸ -1;
    dprintf(16, "ata_dma_transfer id=%p buf=%p\n", op->drive_fl, op->buf_fl);

    ਬਣਤਰ atadrive_s *adrive_gf = container_ਦਾ(
        op->drive_fl, ਬਣਤਰ atadrive_s, drive);
    ਬਣਤਰ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);

    // Start bus-master controller.
    u8 oldcmd = inb(iomaster + BM_CMD);
    outb(oldcmd | BM_CMD_ਸ਼ੁਰੂ, iomaster + BM_CMD);

    u32 ਸਿਰਾ = timer_calc(IDE_TIMEOUT);
    u8 ਸਥਿਤੀ;
    ਲਈ (;;) {
        ਸਥਿਤੀ = inb(iomaster + BM_ਸਥਿਤੀ);
        ਜੇ (ਸਥਿਤੀ & BM_ਸਥਿਤੀ_IRQ)
            ਰੋਕੋ;
        // Transfer in progress
        ਜੇ (timer_check(ਸਿਰਾ)) {
            // Timeout.
            warn_timeout();
            ਰੋਕੋ;
        }
        yield();
    }
    outb(oldcmd & ~BM_CMD_ਸ਼ੁਰੂ, iomaster + BM_CMD);

    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    ਪੂਰਨ idestatus = pause_await_not_bsy(iobase1, iobase2);

    ਜੇ ((ਸਥਿਤੀ & (BM_ਸਥਿਤੀ_IRQ|BM_ਸਥਿਤੀ_ਸਰਗਰਮ)) == BM_ਸਥਿਤੀ_IRQ
        && idestatus >= 0x00
        && (idestatus & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
                         | ATA_CB_STAT_ERR)) == 0x00)
        // Success.
        ਵਾਪਸ 0;

    dprintf(6, "IDE DMA error (dma=%x ide=%x/%x/%x)\n", ਸਥਿਤੀ, idestatus
            , inb(iobase2 + ATA_CB_ASTAT), inb(iobase1 + ATA_CB_ERR));
    ਵਾਪਸ -1;
}


/****************************************************************
 * ATA hard drive functions
 ****************************************************************/

// Transfer data to harddrive using PIO protocol.
ਸਥਿਰ ਪੂਰਨ
ata_pio_cmd_ਅੰਕੜੇ(ਬਣਤਰ disk_op_s *op, ਪੂਰਨ iswrite, ਬਣਤਰ ata_pio_command *cmd)
{
    ਬਣਤਰ atadrive_s *adrive_gf = container_ਦਾ(
        op->drive_fl, ਬਣਤਰ atadrive_s, drive);
    ਬਣਤਰ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    ਪੂਰਨ ret = send_cmd(adrive_gf, cmd);
    ਜੇ (ret)
        ਜਾਓ fail;
    ret = ata_wait_ਅੰਕੜੇ(iobase1);
    ਜੇ (ret)
        ਜਾਓ fail;
    ret = ata_pio_transfer(op, iswrite, DISK_SECTOR_ਮਾਪ);

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    ਵਾਪਸ ret;
}

// Transfer data to harddrive using DMA protocol.
ਸਥਿਰ ਪੂਰਨ
ata_dma_cmd_ਅੰਕੜੇ(ਬਣਤਰ disk_op_s *op, ਬਣਤਰ ata_pio_command *cmd)
{
    ਜੇ (! CONFIG_ATA_DMA)
        ਵਾਪਸ -1;
    ਬਣਤਰ atadrive_s *adrive_gf = container_ਦਾ(
        op->drive_fl, ਬਣਤਰ atadrive_s, drive);
    ਪੂਰਨ ret = send_cmd(adrive_gf, cmd);
    ਜੇ (ret)
        ਵਾਪਸ ret;
    ਵਾਪਸ ata_dma_transfer(op);
}

// Read/write count blocks from a harddrive.
ਸਥਿਰ ਪੂਰਨ
ata_readwrite(ਬਣਤਰ disk_op_s *op, ਪੂਰਨ iswrite)
{
    u64 lba = op->lba;

    ਪੂਰਨ usepio = ata_try_dma(op, iswrite, DISK_SECTOR_ਮਾਪ);

    ਬਣਤਰ ata_pio_command cmd;
    memset(&cmd, 0, ਆਕਾਰ(cmd));

    ਜੇ (op->ਗਿਣਤੀ_2 >= (1<<8) || lba + op->ਗਿਣਤੀ_2 >= (1<<28)) {
        cmd.sector_count2 = op->ਗਿਣਤੀ_2 >> 8;
        cmd.lba_low2 = lba >> 24;
        cmd.lba_mid2 = lba >> 32;
        cmd.lba_high2 = lba >> 40;
        lba &= 0xffffff;

        ਜੇ (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS_EXT
                           : ATA_CMD_READ_SECTORS_EXT);
        ਨਹੀਂਤਾਂ
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                           : ATA_CMD_READ_DMA_EXT);
    } ਨਹੀਂਤਾਂ {
        ਜੇ (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS
                           : ATA_CMD_READ_SECTORS);
        ਨਹੀਂਤਾਂ
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA
                           : ATA_CMD_READ_DMA);
    }

    cmd.sector_ਗਿਣਤੀ = op->ਗਿਣਤੀ_2;
    cmd.lba_ਨੀਵਾਂ = lba;
    cmd.lba_mid = lba >> 8;
    cmd.lba_ਉੱਚਾ = lba >> 16;
    cmd.device = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;

    ਪੂਰਨ ret;
    ਜੇ (usepio)
        ret = ata_pio_cmd_ਅੰਕੜੇ(op, iswrite, &cmd);
    ਨਹੀਂਤਾਂ
        ret = ata_dma_cmd_ਅੰਕੜੇ(op, &cmd);
    ਜੇ (ret)
        ਵਾਪਸ DISK_RET_EBADTRACK;
    ਵਾਪਸ DISK_RET_SUCCESS;
}

// 16bit command demuxer for ATA harddrives.
ਪੂਰਨ
ata_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (!CONFIG_ATA)
        ਵਾਪਸ 0;

    ਬਣਤਰ atadrive_s *adrive_gf = container_ਦਾ(
        op->drive_fl, ਬਣਤਰ atadrive_s, drive);
    ਚੋਣ (op->command) {
    ਹਾਲਤ CMD_READ:
        ਵਾਪਸ ata_readwrite(op, 0);
    ਹਾਲਤ CMD_WRITE:
        ਵਾਪਸ ata_readwrite(op, 1);
    ਹਾਲਤ CMD_RESET:
        ata_reset(adrive_gf);
        ਵਾਪਸ DISK_RET_SUCCESS;
    ਹਾਲਤ CMD_ISREADY:
        ਵਾਪਸ isready(adrive_gf);
    ਮੂਲ:
        ਵਾਪਸ default_process_op(op);
    }
}


/****************************************************************
 * ATAPI functions
 ****************************************************************/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CDROM_CDB_ਮਾਪ 12

// Low-level atapi command transmit function.
ਪੂਰਨ
ata_atapi_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (! CONFIG_ATA)
        ਵਾਪਸ 0;

    ਜੇ (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        ਵਾਪਸ DISK_RET_EWRITEPROTECT;
    u8 cdbcmd[CDROM_CDB_ਮਾਪ];
    ਪੂਰਨ blocksize = scsi_fill_cmd(op, cdbcmd, ਆਕਾਰ(cdbcmd));
    ਜੇ (blocksize < 0)
        ਵਾਪਸ default_process_op(op);

    ਬਣਤਰ atadrive_s *adrive_gf = container_ਦਾ(
        op->drive_fl, ਬਣਤਰ atadrive_s, drive);
    ਬਣਤਰ ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    ਬਣਤਰ ata_pio_command cmd;
    memset(&cmd, 0, ਆਕਾਰ(cmd));
    cmd.lba_mid = blocksize;
    cmd.lba_ਉੱਚਾ = blocksize >> 8;
    cmd.command = ATA_CMD_PACKET;

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    ਪੂਰਨ ret = send_cmd(adrive_gf, &cmd);
    ਜੇ (ret)
        ਜਾਓ fail;
    ret = ata_wait_ਅੰਕੜੇ(iobase1);
    ਜੇ (ret)
        ਜਾਓ fail;

    // Send command to device
    outsw_fl(iobase1, MAKE_FLATPTR(GET_SEG(SS), cdbcmd), CDROM_CDB_ਮਾਪ / 2);

    ਪੂਰਨ ਸਥਿਤੀ = pause_await_not_bsy(iobase1, iobase2);
    ਜੇ (ਸਥਿਤੀ < 0) {
        ret = ਸਥਿਤੀ;
        ਜਾਓ fail;
    }

    ਜੇ (ਸਥਿਤੀ & ATA_CB_STAT_ERR) {
        u8 err = inb(iobase1 + ATA_CB_ERR);
        // skip "Not Ready"
        ਜੇ (err != 0x20)
            dprintf(6, "send_atapi_cmd : read error (status=%02x err=%02x)\n"
                    , ਸਥਿਤੀ, err);
        ret = -2;
        ਜਾਓ fail;
    }
    ਜੇ (blocksize) {
        ਜੇ (!(ਸਥਿਤੀ & ATA_CB_STAT_DRQ)) {
            dprintf(6, "send_atapi_cmd : DRQ not set (status %02x)\n", ਸਥਿਤੀ);
            ret = -3;
            ਜਾਓ fail;
        }

        ret = ata_pio_transfer(op, 0, blocksize);
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    ਜੇ (ret)
        ਵਾਪਸ DISK_RET_EBADTRACK;
    ਵਾਪਸ DISK_RET_SUCCESS;
}


/****************************************************************
 * ATA detect and init
 ****************************************************************/

// Send an identify device or identify device packet command.
ਸਥਿਰ ਪੂਰਨ
send_ata_ਪਛਾਣ_ਤੱਤ(ਬਣਤਰ atadrive_s *adrive, u16 *ਬਫਰ, ਪੂਰਨ command)
{
    memset(ਬਫਰ, 0, DISK_SECTOR_ਮਾਪ);

    ਬਣਤਰ disk_op_s dop;
    memset(&dop, 0, ਆਕਾਰ(dop));
    dop.drive_fl = &adrive->drive;
    dop.ਗਿਣਤੀ_2 = 1;
    dop.lba = 1;
    dop.buf_fl = MAKE_FLATPTR(GET_SEG(SS), ਬਫਰ);

    ਬਣਤਰ ata_pio_command cmd;
    memset(&cmd, 0, ਆਕਾਰ(cmd));
    cmd.command = command;

    ਵਾਪਸ ata_pio_cmd_ਅੰਕੜੇ(&dop, 0, &cmd);
}

// Extract the ATA/ATAPI version info.
ਪੂਰਨ
ata_ਕੱਢੋ_ਸੰਸਕਰਣ(u16 *ਬਫਰ)
{
    // Extract ATA/ATAPI version.
    u16 ataversion = ਬਫਰ[80];
    u8 ਸੰਸਕਰਣ_2;
    ਲਈ (ਸੰਸਕਰਣ_2=15; ਸੰਸਕਰਣ_2>0; ਸੰਸਕਰਣ_2--)
        ਜੇ (ataversion & (1<<ਸੰਸਕਰਣ_2))
            ਰੋਕੋ;
    ਵਾਪਸ ਸੰਸਕਰਣ_2;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXMODEL 40

// Extract the ATA/ATAPI model info.
ਅੱਖਰ *
ata_ਕੱਢੋ_model(ਅੱਖਰ *model, u32 ਮਾਪ, u16 *ਬਫਰ)
{
    // Read model name
    ਪੂਰਨ i;
    ਲਈ (i=0; i<ਮਾਪ/2; i++)
        *(u16*)&model[i*2] = be16_ਮੰਜ਼ਿਲ_cpu(ਬਫਰ[27+i]);
    model[ਮਾਪ] = 0x00;
    nullTrailingSpace(model);
    ਵਾਪਸ model;
}

// Common init code between ata and atapi
ਸਥਿਰ ਬਣਤਰ atadrive_s *
init_atadrive(ਬਣਤਰ atadrive_s *dummy, u16 *ਬਫਰ)
{
    ਬਣਤਰ atadrive_s *adrive = malloc_fseg(ਆਕਾਰ(*adrive));
    ਜੇ (!adrive) {
        warn_noalloc();
        ਵਾਪਸ ਨਿਰਾ;
    }
    memset(adrive, 0, ਆਕਾਰ(*adrive));
    adrive->chan_gf = dummy->chan_gf;
    adrive->slave = dummy->slave;
    adrive->drive.cntl_id = adrive->chan_gf->ataid * 2 + dummy->slave;
    adrive->drive.removable = (ਬਫਰ[0] & 0x80) ? 1 : 0;
    ਵਾਪਸ adrive;
}

// Detect if the given drive is an atapi - initialize it if so.
ਸਥਿਰ ਬਣਤਰ atadrive_s *
init_drive_atapi(ਬਣਤਰ atadrive_s *dummy, u16 *ਬਫਰ)
{
    // Send an IDENTIFY_DEVICE_PACKET command to device
    ਪੂਰਨ ret = send_ata_ਪਛਾਣ_ਤੱਤ(dummy, ਬਫਰ, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    ਜੇ (ret)
        ਵਾਪਸ ਨਿਰਾ;

    // Success - setup as ATAPI.
    ਬਣਤਰ atadrive_s *adrive = init_atadrive(dummy, ਬਫਰ);
    ਜੇ (!adrive)
        ਵਾਪਸ ਨਿਰਾ;
    adrive->drive.type = DTYPE_ATA_ATAPI;
    adrive->drive.blksize = CDROM_SECTOR_ਮਾਪ;
    adrive->drive.sectors = (u64)-1;
    u8 iscd = ((ਬਫਰ[0] >> 8) & 0x1f) == 0x05;
    ਅੱਖਰ model[MAXMODEL+1];
    ਅੱਖਰ *desc = znprintf(MAXDESCSIZE
                          , "DVD/CD [ata%d-%d: %s ATAPI-%d %s]"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_ਕੱਢੋ_model(model, MAXMODEL, ਬਫਰ)
                          , ata_ਕੱਢੋ_ਸੰਸਕਰਣ(ਬਫਰ)
                          , (iscd ? "DVD/CD" : "Device"));
    dprintf(1, "%s\n", desc);

    // fill cdidmap
    ਜੇ (iscd) {
        ਪੂਰਨ prio = bootprio_ਲੱਭੋ_ata_device(adrive->chan_gf->pci_tmp,
                                            adrive->chan_gf->chanid,
                                            adrive->slave);
        boot_lchs_ਲੱਭੋ_ata_device(adrive->chan_gf->pci_tmp,
                                  adrive->chan_gf->chanid,
                                  adrive->slave,
                                  &(adrive->drive.lchs));
        boot_ਜੋੜੋ_cd(&adrive->drive, desc, prio);
    }

    ਵਾਪਸ adrive;
}

// Detect if the given drive is a regular ata drive - initialize it if so.
ਸਥਿਰ ਬਣਤਰ atadrive_s *
init_drive_ata(ਬਣਤਰ atadrive_s *dummy, u16 *ਬਫਰ)
{
    // Send an IDENTIFY_DEVICE command to device
    ਪੂਰਨ ret = send_ata_ਪਛਾਣ_ਤੱਤ(dummy, ਬਫਰ, ATA_CMD_IDENTIFY_DEVICE);
    ਜੇ (ret)
        ਵਾਪਸ ਨਿਰਾ;

    // Success - setup as ATA.
    ਬਣਤਰ atadrive_s *adrive = init_atadrive(dummy, ਬਫਰ);
    ਜੇ (!adrive)
        ਵਾਪਸ ਨਿਰਾ;
    adrive->drive.type = DTYPE_ATA;
    adrive->drive.blksize = DISK_SECTOR_ਮਾਪ;

    adrive->drive.pchs.cylinder = ਬਫਰ[1];
    adrive->drive.pchs.ਸਿਰ = ਬਫਰ[3];
    adrive->drive.pchs.sector = ਬਫਰ[6];

    u64 sectors;
    ਜੇ (ਬਫਰ[83] & (1 << 10)) // word 83 - lba48 support
        sectors = *(u64*)&ਬਫਰ[100]; // word 100-103
    ਨਹੀਂਤਾਂ
        sectors = *(u32*)&ਬਫਰ[60]; // word 60 and word 61
    adrive->drive.sectors = sectors;
    u64 adjsize = sectors >> 11;
    ਅੱਖਰ adjprefix = 'M';
    ਜੇ (adjsize >= (1 << 16)) {
        adjsize >>= 10;
        adjprefix = 'G';
    }
    ਅੱਖਰ model[MAXMODEL+1];
    ਅੱਖਰ *desc = znprintf(MAXDESCSIZE
                          , "ata%d-%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_ਕੱਢੋ_model(model, MAXMODEL, ਬਫਰ)
                          , ata_ਕੱਢੋ_ਸੰਸਕਰਣ(ਬਫਰ)
                          , (u32)adjsize, adjprefix);
    dprintf(1, "%s\n", desc);

    ਪੂਰਨ prio = bootprio_ਲੱਭੋ_ata_device(adrive->chan_gf->pci_tmp,
                                        adrive->chan_gf->chanid,
                                        adrive->slave);
    boot_lchs_ਲੱਭੋ_ata_device(adrive->chan_gf->pci_tmp,
                              adrive->chan_gf->chanid,
                              adrive->slave,
                              &(adrive->drive.lchs));
    // Register with bcv system.
    boot_ਜੋੜੋ_hd(&adrive->drive, desc, prio);

    ਵਾਪਸ adrive;
}

ਸਥਿਰ u32 Spinup_ਸਿਰਾ;

// Wait for non-busy status and check for "floating bus" condition.
ਸਥਿਰ ਪੂਰਨ
powerup_await_non_bsy(u16 ਅਧਾਰ)
{
    u8 orstatus = 0;
    u8 ਸਥਿਤੀ;
    ਲਈ (;;) {
        ਸਥਿਤੀ = inb(ਅਧਾਰ+ATA_CB_STAT);
        ਜੇ (!(ਸਥਿਤੀ & ATA_CB_STAT_BSY))
            ਰੋਕੋ;
        orstatus |= ਸਥਿਤੀ;
        ਜੇ (orstatus == 0xff) {
            dprintf(4, "powerup IDE floating\n");
            ਵਾਪਸ orstatus;
        }
        ਜੇ (timer_check(Spinup_ਸਿਰਾ)) {
            warn_timeout();
            ਵਾਪਸ -1;
        }
        yield();
    }
    dprintf(6, "powerup iobase=%x st=%x\n", ਅਧਾਰ, ਸਥਿਤੀ);
    ਵਾਪਸ ਸਥਿਤੀ;
}

// Detect any drives attached to a given controller.
ਸਥਿਰ ਖਾਲੀ
ata_detect(ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਬਣਤਰ ata_channel_s *chan_gf = ਅੰਕੜੇ;
    ਬਣਤਰ atadrive_s dummy;
    memset(&dummy, 0, ਆਕਾਰ(dummy));
    dummy.chan_gf = chan_gf;
    // Device detection
    ਪੂਰਨ didreset = 0;
    u8 slave;
    ਲਈ (slave=0; slave<=1; slave++) {
        // Wait for not-bsy.
        u16 iobase1 = chan_gf->iobase1;
        ਪੂਰਨ ਸਥਿਤੀ = powerup_await_non_bsy(iobase1);
        ਜੇ (ਸਥਿਤੀ < 0)
            ਜਾਰੀ;
        u8 newdh = slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0;
        outb(newdh, iobase1+ATA_CB_DH);
        ndelay(400);
        ਸਥਿਤੀ = powerup_await_non_bsy(iobase1);
        ਜੇ (ਸਥਿਤੀ < 0)
            ਜਾਰੀ;

        // Check if ioport registers look valid.
        outb(newdh, iobase1+ATA_CB_DH);
        u8 dh = inb(iobase1+ATA_CB_DH);
        outb(0x55, iobase1+ATA_CB_SC);
        outb(0xaa, iobase1+ATA_CB_SN);
        u8 sc = inb(iobase1+ATA_CB_SC);
        u8 sn = inb(iobase1+ATA_CB_SN);
        dprintf(6, "ata_detect ata%d-%d: sc=%x sn=%x dh=%x\n"
                , chan_gf->ataid, slave, sc, sn, dh);
        ਜੇ (sc != 0x55 || sn != 0xaa || dh != newdh)
            ਜਾਰੀ;

        // Prepare new drive.
        dummy.slave = slave;

        // reset the channel
        ਜੇ (!didreset) {
            ata_reset(&dummy);
            didreset = 1;
        }

        // check for ATAPI
        u16 ਬਫਰ[256];
        ਬਣਤਰ atadrive_s *adrive = init_drive_atapi(&dummy, ਬਫਰ);
        ਜੇ (!adrive) {
            // Didn't find an ATAPI drive - look for ATA drive.
            u8 st = inb(iobase1+ATA_CB_STAT);
            ਜੇ (!st)
                // Status not set - can't be a valid drive.
                ਜਾਰੀ;

            // Wait for RDY.
            ਪੂਰਨ ret = await_rdy(iobase1);
            ਜੇ (ret < 0)
                ਜਾਰੀ;

            // check for ATA.
            adrive = init_drive_ata(&dummy, ਬਫਰ);
            ਜੇ (!adrive)
                // No ATA drive found
                ਜਾਰੀ;
        }

        u16 resetresult = ਬਫਰ[93];
        dprintf(6, "ata_detect resetresult=%04x\n", resetresult);
        ਜੇ (!slave && (resetresult & 0xdf61) == 0x4041)
            // resetresult looks valid and device 0 is responding to
            // device 1 requests - device 1 must not be present - skip
            // detection.
            ਰੋਕੋ;
    }
}

// Initialize an ata controller and detect its drives.
ਸਥਿਰ ਖਾਲੀ
init_controller(ਬਣਤਰ pci_device *pci, ਪੂਰਨ chanid, ਪੂਰਨ irq
                , u32 port1, u32 port2, u32 master)
{
    ਸਥਿਰ ਪੂਰਨ ataid = 0;
    ਬਣਤਰ ata_channel_s *chan_gf = malloc_fseg(ਆਕਾਰ(*chan_gf));
    ਜੇ (!chan_gf) {
        warn_noalloc();
        ਵਾਪਸ;
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
    ਲੜੀਵਾਰ_thread(ata_detect, chan_gf);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IRQ_ATA1 14
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IRQ_ATA2 15

// Handle controllers on an ATA PCI device.
ਸਥਿਰ ਖਾਲੀ
init_pciata(ਬਣਤਰ pci_device *pci, u8 prog_if)
{
    u8 pciirq = pci_config_readb(pci->bdf, PCI_ਰੁਕਾਵਟ_ਸਤਰ);
    ਪੂਰਨ master = 0;
    ਜੇ (CONFIG_ATA_DMA && prog_if & 0x80) {
        // Check for bus-mastering.
        u32 bar = pci_config_readl(pci->bdf, PCI_ਅਧਾਰ_ਪਤਾ_4);
        ਜੇ (bar & PCI_ਅਧਾਰ_ਪਤਾ_SPACE_IO) {
            master = pci_enable_iobar(pci, PCI_ਅਧਾਰ_ਪਤਾ_4);
            pci_enable_busmaster(pci);
        }
    }

    u32 port1, port2, irq;
    ਜੇ (prog_if & 1) {
        port1 = pci_enable_iobar(pci, PCI_ਅਧਾਰ_ਪਤਾ_0);
        port2 = pci_enable_iobar(pci, PCI_ਅਧਾਰ_ਪਤਾ_1);
        ਜੇ (!port1 || !port2)
            ਵਾਪਸ;
        irq = pciirq;
    } ਨਹੀਂਤਾਂ {
        port1 = ਪੋਰਟ_ATA1_CMD_ਅਧਾਰ;
        port2 = ਪੋਰਟ_ATA1_CTRL_ਅਧਾਰ;
        irq = IRQ_ATA1;
    }
    init_controller(pci, 0, irq, port1, port2, master);

    ਜੇ (prog_if & 4) {
        port1 = pci_enable_iobar(pci, PCI_ਅਧਾਰ_ਪਤਾ_2);
        port2 = pci_enable_iobar(pci, PCI_ਅਧਾਰ_ਪਤਾ_3);
        ਜੇ (!port1 || !port2)
            ਵਾਪਸ;
        irq = pciirq;
    } ਨਹੀਂਤਾਂ {
        port1 = ਪੋਰਟ_ATA2_CMD_ਅਧਾਰ;
        port2 = ਪੋਰਟ_ATA2_CTRL_ਅਧਾਰ;
        irq = IRQ_ATA2;
    }
    init_controller(pci, 1, irq, port1, port2, master ? master + 8 : 0);
}

ਸਥਿਰ ਖਾਲੀ
ਮਿਲਿਆ_genericata(ਬਣਤਰ pci_device *pci, ਖਾਲੀ *arg)
{
    init_pciata(pci, pci->prog_if);
}

ਸਥਿਰ ਖਾਲੀ
ਮਿਲਿਆ_compatibleahci(ਬਣਤਰ pci_device *pci, ਖਾਲੀ *arg)
{
    ਜੇ (CONFIG_AHCI)
        // Already handled directly via native ahci interface.
        ਵਾਪਸ;
    init_pciata(pci, 0x8f);
}

ਸਥਿਰ ਅਚਲ ਬਣਤਰ pci_device_id pci_ata_tbl[] = {
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE
                     , ਮਿਲਿਆ_genericata),
    PCI_DEVICE(PCI_VENDOR_ID_ATI, 0x4391, ਮਿਲਿਆ_compatibleahci),
    PCI_DEVICE_ਸਿਰਾ,
};

// Locate and init ata controllers.
ਸਥਿਰ ਖਾਲੀ
ata_ਜਾਂਚ(ਖਾਲੀ)
{
    ਜੇ (CONFIG_QEMU && hlist_ਖਾਲੀ_ਥਾਵਾਂ(&PCIDevices)) {
        // No PCI devices found - probably a QEMU "-M isapc" machine.
        // Try using ISA ports for ATA controllers.
        init_controller(ਨਿਰਾ, 0, IRQ_ATA1
                        , ਪੋਰਟ_ATA1_CMD_ਅਧਾਰ, ਪੋਰਟ_ATA1_CTRL_ਅਧਾਰ, 0);
        init_controller(ਨਿਰਾ, 1, IRQ_ATA2
                        , ਪੋਰਟ_ATA2_CMD_ਅਧਾਰ, ਪੋਰਟ_ATA2_CTRL_ਅਧਾਰ, 0);
        ਵਾਪਸ;
    }

    // Scan PCI bus for ATA adapters
    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_ata_tbl, pci, ਨਿਰਾ);
    }
}

ਖਾਲੀ
ata_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ASSERT32FLAT();
    ਜੇ (!CONFIG_ATA)
        ਵਾਪਸ;

    dprintf(3, "init hard drives\n");

    Spinup_ਸਿਰਾ = timer_calc(IDE_TIMEOUT);
    ata_ਜਾਂਚ();

    ਸੈੱਟ_BDA(disk_control_byte, 0xc0);

    enable_hwirq(14, FUNC16(ਇੰਦਰਾਜ_76));
}

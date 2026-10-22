// Low level ATA disk access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "ata.h" // ATA_CB_STAT
#అన్తర్భావ "biosvar.h" // GET_GLOBALFLAT
#అన్తర్భావ "block.h" // struct drive_s
#అన్తర్భావ "blockcmd.h" // CDB_CMD_READ_10
#అన్తర్భావ "byteorder.h" // be16_to_cpu
#అన్తర్భావ "malloc.h" // malloc_fseg
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pci.h" // pci_config_readb
#అన్తర్భావ "pcidevice.h" // foreachpci
#అన్తర్భావ "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#అన్తర్భావ "pci_regs.h" // PCI_INTERRUPT_LINE
#అన్తర్భావ "pic.h" // enable_hwirq
#అన్తర్భావ "stacks.h" // yield
#అన్తర్భావ "std/disk.h" // DISK_RET_SUCCESS
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // timer_calc
#అన్తర్భావ "x86.h" // inb

#నిర్వచించు IDE_TIMEOUT 32000 //32 seconds max for IDE ops


/****************************************************************
 * Helper functions
 ****************************************************************/

// Wait for the specified ide state
స్థిర అంతఃవరుస పూర్ణాంకం
await_ide(u8 ముసుగు, u8 flags, u16 పునాది, u16 timeout)
{
    u32 చివర = timer_calc(timeout);
    కొరకు (;;) {
        u8 స్థితి_2 = inb(పునాది+ATA_CB_STAT);
        ఒకవేళ ((స్థితి_2 & ముసుగు) == flags)
            తిరిగి స్థితి_2;
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            తిరిగి -1;
        }
        yield();
    }
}

// Wait for the device to be not-busy.
స్థిర పూర్ణాంకం
await_not_bsy(u16 పునాది)
{
    తిరిగి await_ide(ATA_CB_STAT_BSY, 0, పునాది, IDE_TIMEOUT);
}

// Wait for the device to be ready.
స్థిర పూర్ణాంకం
await_rdy(u16 పునాది)
{
    తిరిగి await_ide(ATA_CB_STAT_RDY, ATA_CB_STAT_RDY, పునాది, IDE_TIMEOUT);
}

// Wait for ide state - pauses for one ata cycle first.
స్థిర అంతఃవరుస పూర్ణాంకం
pause_await_not_bsy(u16 iobase1, u16 iobase2)
{
    // Wait one PIO transfer cycle.
    inb(iobase2 + ATA_CB_ASTAT);

    తిరిగి await_not_bsy(iobase1);
}

// Wait for ide state - pause for 400ns first.
స్థిర అంతఃవరుస పూర్ణాంకం
ndelay_await_not_bsy(u16 iobase1)
{
    ndelay(400);
    తిరిగి await_not_bsy(iobase1);
}

// Reset a drive
స్థిర శూన్య
ata_reset(నిర్మాణం atadrive_s *adrive_gf)
{
    నిర్మాణం ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
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
    పూర్ణాంకం స్థితి_2 = await_not_bsy(iobase1);
    ఒకవేళ (స్థితి_2 < 0)
        వెళ్ళు పూర్తి;
    ఒకవేళ (slave) {
        // Change device.
        u32 చివర = timer_calc(IDE_TIMEOUT);
        కొరకు (;;) {
            outb(ATA_CB_DH_DEV1, iobase1 + ATA_CB_DH);
            స్థితి_2 = ndelay_await_not_bsy(iobase1);
            ఒకవేళ (స్థితి_2 < 0)
                వెళ్ళు పూర్తి;
            ఒకవేళ (inb(iobase1 + ATA_CB_DH) == ATA_CB_DH_DEV1)
                ఆపు;
            // Change drive request failed to take effect - retry.
            ఒకవేళ (timer_check(చివర)) {
                warn_timeout();
                వెళ్ళు పూర్తి;
            }
        }
    } లేకపోతే {
        // QEMU doesn't reset dh on reset, so set it explicitly.
        outb(ATA_CB_DH_DEV0, iobase1 + ATA_CB_DH);
    }

    // On a user-reset request, wait for RDY if it is an ATA device.
    u8 type=GET_GLOBALFLAT(adrive_gf->drive.type);
    ఒకవేళ (type == DTYPE_ATA)
        స్థితి_2 = await_rdy(iobase1);

పూర్తి:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    dprintf(6, "ata_reset exit status=%x\n", స్థితి_2);
}

// Check for drive RDY for 16bit interface command.
స్థిర పూర్ణాంకం
isready(నిర్మాణం atadrive_s *adrive_gf)
{
    // Read the status from controller
    నిర్మాణం ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u8 స్థితి_2 = inb(iobase1 + ATA_CB_STAT);
    ఒకవేళ ((స్థితి_2 & (ATA_CB_STAT_BSY|ATA_CB_STAT_RDY)) == ATA_CB_STAT_RDY)
        తిరిగి DISK_RET_SUCCESS;
    తిరిగి DISK_RET_ENOTREADY;
}


/****************************************************************
 * ATA send command
 ****************************************************************/

నిర్మాణం ata_pio_command {
    u8 feature;
    u8 sector_లెక్క;
    u8 lba_దిగువ;
    u8 lba_mid;
    u8 lba_ఎగువ;
    u8 device;
    u8 command;

    u8 feature2;
    u8 sector_count2;
    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
};

// Send an ata command to the drive.
స్థిర పూర్ణాంకం
send_cmd(నిర్మాణం atadrive_s *adrive_gf, నిర్మాణం ata_pio_command *cmd)
{
    నిర్మాణం ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);

    // Select device
    పూర్ణాంకం స్థితి_2 = await_not_bsy(iobase1);
    ఒకవేళ (స్థితి_2 < 0)
        తిరిగి స్థితి_2;
    u8 newdh = ((cmd->device & ~ATA_CB_DH_DEV1)
                | (slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0));
    u8 olddh = inb(iobase1 + ATA_CB_DH);
    outb(newdh, iobase1 + ATA_CB_DH);
    ఒకవేళ ((olddh ^ newdh) & (1<<4)) {
        // Was a device change - wait for device to become not busy.
        స్థితి_2 = ndelay_await_not_bsy(iobase1);
        ఒకవేళ (స్థితి_2 < 0)
            తిరిగి స్థితి_2;
    }

    // Check for ATA_CMD_(READ|WRITE)_(SECTORS|DMA)_EXT commands.
    ఒకవేళ ((cmd->command & ~0x11) == ATA_CMD_READ_SECTORS_EXT) {
        outb(cmd->feature2, iobase1 + ATA_CB_FR);
        outb(cmd->sector_count2, iobase1 + ATA_CB_SC);
        outb(cmd->lba_low2, iobase1 + ATA_CB_SN);
        outb(cmd->lba_mid2, iobase1 + ATA_CB_CL);
        outb(cmd->lba_high2, iobase1 + ATA_CB_CH);
    }
    outb(cmd->feature, iobase1 + ATA_CB_FR);
    outb(cmd->sector_లెక్క, iobase1 + ATA_CB_SC);
    outb(cmd->lba_దిగువ, iobase1 + ATA_CB_SN);
    outb(cmd->lba_mid, iobase1 + ATA_CB_CL);
    outb(cmd->lba_ఎగువ, iobase1 + ATA_CB_CH);
    outb(cmd->command, iobase1 + ATA_CB_CMD);

    తిరిగి 0;
}

// Wait for data after calling 'send_cmd'.
స్థిర పూర్ణాంకం
ata_wait_దత్తాంశం(u16 iobase1)
{
    పూర్ణాంకం స్థితి_2 = ndelay_await_not_bsy(iobase1);
    ఒకవేళ (స్థితి_2 < 0)
        తిరిగి స్థితి_2;

    ఒకవేళ (స్థితి_2 & ATA_CB_STAT_ERR) {
        dprintf(6, "send_cmd : read error (status=%02x err=%02x)\n"
                , స్థితి_2, inb(iobase1 + ATA_CB_ERR));
        తిరిగి -4;
    }
    ఒకవేళ (!(స్థితి_2 & ATA_CB_STAT_DRQ)) {
        dprintf(6, "send_cmd : DRQ not set (status %02x)\n", స్థితి_2);
        తిరిగి -5;
    }

    తిరిగి 0;
}

// Send an ata command that does not transfer any further data.
పూర్ణాంకం
ata_cmd_nondata(నిర్మాణం atadrive_s *adrive_gf, నిర్మాణం ata_pio_command *cmd)
{
    నిర్మాణం ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    పూర్ణాంకం ret = send_cmd(adrive_gf, cmd);
    ఒకవేళ (ret)
        వెళ్ళు fail;
    ret = ndelay_await_not_bsy(iobase1);
    ఒకవేళ (ret < 0)
        వెళ్ళు fail;

    ఒకవేళ (ret & ATA_CB_STAT_ERR) {
        dprintf(6, "nondata cmd : read error (status=%02x err=%02x)\n"
                , ret, inb(iobase1 + ATA_CB_ERR));
        ret = -4;
        వెళ్ళు fail;
    }
    ఒకవేళ (ret & ATA_CB_STAT_DRQ) {
        dprintf(6, "nondata cmd : DRQ set (status %02x)\n", ret);
        ret = -5;
        వెళ్ళు fail;
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    తిరిగి ret;
}


/****************************************************************
 * ATA PIO transfers
 ****************************************************************/

// Transfer 'op->count' blocks (of 'blocksize' bytes) to/from drive
// 'op->drive_fl'.
స్థిర పూర్ణాంకం
ata_pio_transfer(నిర్మాణం disk_op_s *op, పూర్ణాంకం iswrite, పూర్ణాంకం blocksize)
{
    dprintf(16, "ata_pio_transfer id=%p write=%d count=%d bs=%d buf=%p\n"
            , op->drive_fl, iswrite, op->లెక్క, blocksize, op->buf_fl);

    నిర్మాణం atadrive_s *adrive_gf = container_యొక్క(
        op->drive_fl, నిర్మాణం atadrive_s, drive);
    నిర్మాణం ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    పూర్ణాంకం లెక్క = op->లెక్క;
    శూన్య *buf_fl = op->buf_fl;
    పూర్ణాంకం స్థితి_2;
    కొరకు (;;) {
        ఒకవేళ (iswrite) {
            // Write data to controller
            dprintf(16, "Write sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            ఒకవేళ (CONFIG_ATA_PIO32)
                outsl_fl(iobase1, buf_fl, blocksize / 4);
            లేకపోతే
                outsw_fl(iobase1, buf_fl, blocksize / 2);
        } లేకపోతే {
            // Read data from controller
            dprintf(16, "Read sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            ఒకవేళ (CONFIG_ATA_PIO32)
                insl_fl(iobase1, buf_fl, blocksize / 4);
            లేకపోతే
                insw_fl(iobase1, buf_fl, blocksize / 2);
        }
        buf_fl += blocksize;

        స్థితి_2 = pause_await_not_bsy(iobase1, iobase2);
        ఒకవేళ (స్థితి_2 < 0) {
            // Error
            op->లెక్క -= లెక్క;
            తిరిగి స్థితి_2;
        }

        లెక్క--;
        ఒకవేళ (!లెక్క)
            ఆపు;
        స్థితి_2 &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ | ATA_CB_STAT_ERR);
        ఒకవేళ (స్థితి_2 != ATA_CB_STAT_DRQ) {
            dprintf(6, "ata_pio_transfer : more sectors left (status %02x)\n"
                    , స్థితి_2);
            op->లెక్క -= లెక్క;
            తిరిగి -6;
        }
    }

    స్థితి_2 &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
               | ATA_CB_STAT_ERR);
    ఒకవేళ (!iswrite)
        స్థితి_2 &= ~ATA_CB_STAT_DF;
    ఒకవేళ (స్థితి_2 != 0) {
        dprintf(6, "ata_pio_transfer : no sectors left (status %02x)\n", స్థితి_2);
        తిరిగి -7;
    }

    తిరిగి 0;
}


/****************************************************************
 * ATA DMA transfers
 ****************************************************************/

#నిర్వచించు BM_CMD    0
#నిర్వచించు  BM_CMD_MEMWRITE  0x08
#నిర్వచించు  BM_CMD_ప్రారంభం     0x01
#నిర్వచించు BM_స్థితి 2
#నిర్వచించు  BM_స్థితి_IRQ    0x04
#నిర్వచించు  BM_స్థితి_ERROR  0x02
#నిర్వచించు  BM_స్థితి_చురుకైన 0x01
#నిర్వచించు BM_పట్టిక  4

నిర్మాణం sff_dma_prd {
    u32 buf_fl;
    u32 లెక్క;
};

// Check if DMA available and setup transfer if so.
స్థిర పూర్ణాంకం
ata_try_dma(నిర్మాణం disk_op_s *op, పూర్ణాంకం iswrite, పూర్ణాంకం blocksize)
{
    ASSERT16();
    ఒకవేళ (! CONFIG_ATA_DMA)
        తిరిగి -1;
    u32 dest = (u32)op->buf_fl;
    ఒకవేళ (dest & 1)
        // Need minimum alignment of 1.
        తిరిగి -1;
    నిర్మాణం atadrive_s *adrive_gf = container_యొక్క(
        op->drive_fl, నిర్మాణం atadrive_s, drive);
    నిర్మాణం ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);
    ఒకవేళ (! iomaster)
        తిరిగి -1;
    u32 బైట్లు = op->లెక్క * blocksize;
    ఒకవేళ (! బైట్లు)
        తిరిగి -1;

    // Build PRD dma structure.
    నిర్మాణం sff_dma_prd *dma = MAKE_FLATPTR(SEG_దిగువ, Extra_దొంతర);
    నిర్మాణం sff_dma_prd *origdma = dma;
    అయితే (బైట్లు) {
        ఒకవేళ (dma >= &origdma[16])
            // Too many descriptors..
            తిరిగి -1;
        u32 లెక్క = బైట్లు;
        u32 max = 0x10000 - (dest & 0xffff);
        ఒకవేళ (లెక్క > max)
            లెక్క = max;

        సమితి_LOWFLAT(dma->buf_fl, dest);
        బైట్లు -= లెక్క;
        ఒకవేళ (!బైట్లు)
            // Last descriptor.
            లెక్క |= 1<<31;
        dprintf(16, "dma@%p: %08x %08x\n", dma, dest, లెక్క);
        dest += లెక్క;
        సమితి_LOWFLAT(dma->లెక్క, లెక్క);
        dma++;
    }

    // Program bus-master controller.
    outl((u32)origdma, iomaster + BM_పట్టిక);
    u8 oldcmd = inb(iomaster + BM_CMD) & ~(BM_CMD_MEMWRITE|BM_CMD_ప్రారంభం);
    outb(oldcmd | (iswrite ? 0x00 : BM_CMD_MEMWRITE), iomaster + BM_CMD);
    outb(BM_స్థితి_ERROR|BM_స్థితి_IRQ, iomaster + BM_స్థితి);

    తిరిగి 0;
}

// Transfer data using DMA.
స్థిర పూర్ణాంకం
ata_dma_transfer(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (! CONFIG_ATA_DMA)
        తిరిగి -1;
    dprintf(16, "ata_dma_transfer id=%p buf=%p\n", op->drive_fl, op->buf_fl);

    నిర్మాణం atadrive_s *adrive_gf = container_యొక్క(
        op->drive_fl, నిర్మాణం atadrive_s, drive);
    నిర్మాణం ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);

    // Start bus-master controller.
    u8 oldcmd = inb(iomaster + BM_CMD);
    outb(oldcmd | BM_CMD_ప్రారంభం, iomaster + BM_CMD);

    u32 చివర = timer_calc(IDE_TIMEOUT);
    u8 స్థితి_2;
    కొరకు (;;) {
        స్థితి_2 = inb(iomaster + BM_స్థితి);
        ఒకవేళ (స్థితి_2 & BM_స్థితి_IRQ)
            ఆపు;
        // Transfer in progress
        ఒకవేళ (timer_check(చివర)) {
            // Timeout.
            warn_timeout();
            ఆపు;
        }
        yield();
    }
    outb(oldcmd & ~BM_CMD_ప్రారంభం, iomaster + BM_CMD);

    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    పూర్ణాంకం idestatus = pause_await_not_bsy(iobase1, iobase2);

    ఒకవేళ ((స్థితి_2 & (BM_స్థితి_IRQ|BM_స్థితి_చురుకైన)) == BM_స్థితి_IRQ
        && idestatus >= 0x00
        && (idestatus & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
                         | ATA_CB_STAT_ERR)) == 0x00)
        // Success.
        తిరిగి 0;

    dprintf(6, "IDE DMA error (dma=%x ide=%x/%x/%x)\n", స్థితి_2, idestatus
            , inb(iobase2 + ATA_CB_ASTAT), inb(iobase1 + ATA_CB_ERR));
    తిరిగి -1;
}


/****************************************************************
 * ATA hard drive functions
 ****************************************************************/

// Transfer data to harddrive using PIO protocol.
స్థిర పూర్ణాంకం
ata_pio_cmd_దత్తాంశం(నిర్మాణం disk_op_s *op, పూర్ణాంకం iswrite, నిర్మాణం ata_pio_command *cmd)
{
    నిర్మాణం atadrive_s *adrive_gf = container_యొక్క(
        op->drive_fl, నిర్మాణం atadrive_s, drive);
    నిర్మాణం ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    పూర్ణాంకం ret = send_cmd(adrive_gf, cmd);
    ఒకవేళ (ret)
        వెళ్ళు fail;
    ret = ata_wait_దత్తాంశం(iobase1);
    ఒకవేళ (ret)
        వెళ్ళు fail;
    ret = ata_pio_transfer(op, iswrite, DISK_SECTOR_కొలత);

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    తిరిగి ret;
}

// Transfer data to harddrive using DMA protocol.
స్థిర పూర్ణాంకం
ata_dma_cmd_దత్తాంశం(నిర్మాణం disk_op_s *op, నిర్మాణం ata_pio_command *cmd)
{
    ఒకవేళ (! CONFIG_ATA_DMA)
        తిరిగి -1;
    నిర్మాణం atadrive_s *adrive_gf = container_యొక్క(
        op->drive_fl, నిర్మాణం atadrive_s, drive);
    పూర్ణాంకం ret = send_cmd(adrive_gf, cmd);
    ఒకవేళ (ret)
        తిరిగి ret;
    తిరిగి ata_dma_transfer(op);
}

// Read/write count blocks from a harddrive.
స్థిర పూర్ణాంకం
ata_readwrite(నిర్మాణం disk_op_s *op, పూర్ణాంకం iswrite)
{
    u64 lba = op->lba;

    పూర్ణాంకం usepio = ata_try_dma(op, iswrite, DISK_SECTOR_కొలత);

    నిర్మాణం ata_pio_command cmd;
    memset(&cmd, 0, పరిమాణం(cmd));

    ఒకవేళ (op->లెక్క >= (1<<8) || lba + op->లెక్క >= (1<<28)) {
        cmd.sector_count2 = op->లెక్క >> 8;
        cmd.lba_low2 = lba >> 24;
        cmd.lba_mid2 = lba >> 32;
        cmd.lba_high2 = lba >> 40;
        lba &= 0xffffff;

        ఒకవేళ (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS_EXT
                           : ATA_CMD_READ_SECTORS_EXT);
        లేకపోతే
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                           : ATA_CMD_READ_DMA_EXT);
    } లేకపోతే {
        ఒకవేళ (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS
                           : ATA_CMD_READ_SECTORS);
        లేకపోతే
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA
                           : ATA_CMD_READ_DMA);
    }

    cmd.sector_లెక్క = op->లెక్క;
    cmd.lba_దిగువ = lba;
    cmd.lba_mid = lba >> 8;
    cmd.lba_ఎగువ = lba >> 16;
    cmd.device = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;

    పూర్ణాంకం ret;
    ఒకవేళ (usepio)
        ret = ata_pio_cmd_దత్తాంశం(op, iswrite, &cmd);
    లేకపోతే
        ret = ata_dma_cmd_దత్తాంశం(op, &cmd);
    ఒకవేళ (ret)
        తిరిగి DISK_RET_EBADTRACK;
    తిరిగి DISK_RET_SUCCESS;
}

// 16bit command demuxer for ATA harddrives.
పూర్ణాంకం
ata_process_op(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (!CONFIG_ATA)
        తిరిగి 0;

    నిర్మాణం atadrive_s *adrive_gf = container_యొక్క(
        op->drive_fl, నిర్మాణం atadrive_s, drive);
    ఎంపిక (op->command) {
    స్థితి CMD_READ:
        తిరిగి ata_readwrite(op, 0);
    స్థితి CMD_WRITE:
        తిరిగి ata_readwrite(op, 1);
    స్థితి CMD_RESET:
        ata_reset(adrive_gf);
        తిరిగి DISK_RET_SUCCESS;
    స్థితి CMD_ISREADY:
        తిరిగి isready(adrive_gf);
    అప్రమేయ:
        తిరిగి default_process_op(op);
    }
}


/****************************************************************
 * ATAPI functions
 ****************************************************************/

#నిర్వచించు CDROM_CDB_కొలత 12

// Low-level atapi command transmit function.
పూర్ణాంకం
ata_atapi_process_op(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (! CONFIG_ATA)
        తిరిగి 0;

    ఒకవేళ (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        తిరిగి DISK_RET_EWRITEPROTECT;
    u8 cdbcmd[CDROM_CDB_కొలత];
    పూర్ణాంకం blocksize = scsi_fill_cmd(op, cdbcmd, పరిమాణం(cdbcmd));
    ఒకవేళ (blocksize < 0)
        తిరిగి default_process_op(op);

    నిర్మాణం atadrive_s *adrive_gf = container_యొక్క(
        op->drive_fl, నిర్మాణం atadrive_s, drive);
    నిర్మాణం ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    నిర్మాణం ata_pio_command cmd;
    memset(&cmd, 0, పరిమాణం(cmd));
    cmd.lba_mid = blocksize;
    cmd.lba_ఎగువ = blocksize >> 8;
    cmd.command = ATA_CMD_PACKET;

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    పూర్ణాంకం ret = send_cmd(adrive_gf, &cmd);
    ఒకవేళ (ret)
        వెళ్ళు fail;
    ret = ata_wait_దత్తాంశం(iobase1);
    ఒకవేళ (ret)
        వెళ్ళు fail;

    // Send command to device
    outsw_fl(iobase1, MAKE_FLATPTR(GET_SEG(SS), cdbcmd), CDROM_CDB_కొలత / 2);

    పూర్ణాంకం స్థితి_2 = pause_await_not_bsy(iobase1, iobase2);
    ఒకవేళ (స్థితి_2 < 0) {
        ret = స్థితి_2;
        వెళ్ళు fail;
    }

    ఒకవేళ (స్థితి_2 & ATA_CB_STAT_ERR) {
        u8 err = inb(iobase1 + ATA_CB_ERR);
        // skip "Not Ready"
        ఒకవేళ (err != 0x20)
            dprintf(6, "send_atapi_cmd : read error (status=%02x err=%02x)\n"
                    , స్థితి_2, err);
        ret = -2;
        వెళ్ళు fail;
    }
    ఒకవేళ (blocksize) {
        ఒకవేళ (!(స్థితి_2 & ATA_CB_STAT_DRQ)) {
            dprintf(6, "send_atapi_cmd : DRQ not set (status %02x)\n", స్థితి_2);
            ret = -3;
            వెళ్ళు fail;
        }

        ret = ata_pio_transfer(op, 0, blocksize);
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    ఒకవేళ (ret)
        తిరిగి DISK_RET_EBADTRACK;
    తిరిగి DISK_RET_SUCCESS;
}


/****************************************************************
 * ATA detect and init
 ****************************************************************/

// Send an identify device or identify device packet command.
స్థిర పూర్ణాంకం
send_ata_తటస్థం(నిర్మాణం atadrive_s *adrive, u16 *బఫరు, పూర్ణాంకం command)
{
    memset(బఫరు, 0, DISK_SECTOR_కొలత);

    నిర్మాణం disk_op_s dop;
    memset(&dop, 0, పరిమాణం(dop));
    dop.drive_fl = &adrive->drive;
    dop.లెక్క = 1;
    dop.lba = 1;
    dop.buf_fl = MAKE_FLATPTR(GET_SEG(SS), బఫరు);

    నిర్మాణం ata_pio_command cmd;
    memset(&cmd, 0, పరిమాణం(cmd));
    cmd.command = command;

    తిరిగి ata_pio_cmd_దత్తాంశం(&dop, 0, &cmd);
}

// Extract the ATA/ATAPI version info.
పూర్ణాంకం
ata_వెలికితీయి_సంస్కరణ(u16 *బఫరు)
{
    // Extract ATA/ATAPI version.
    u16 ataversion = బఫరు[80];
    u8 సంస్కరణ_2;
    కొరకు (సంస్కరణ_2=15; సంస్కరణ_2>0; సంస్కరణ_2--)
        ఒకవేళ (ataversion & (1<<సంస్కరణ_2))
            ఆపు;
    తిరిగి సంస్కరణ_2;
}

#నిర్వచించు MAXMODEL 40

// Extract the ATA/ATAPI model info.
అక్షరం *
ata_వెలికితీయి_model(అక్షరం *model, u32 కొలత, u16 *బఫరు)
{
    // Read model name
    పూర్ణాంకం i;
    కొరకు (i=0; i<కొలత/2; i++)
        *(u16*)&model[i*2] = be16_గమ్యం_cpu(బఫరు[27+i]);
    model[కొలత] = 0x00;
    nullTrailingSpace(model);
    తిరిగి model;
}

// Common init code between ata and atapi
స్థిర నిర్మాణం atadrive_s *
init_atadrive(నిర్మాణం atadrive_s *dummy, u16 *బఫరు)
{
    నిర్మాణం atadrive_s *adrive = malloc_fseg(పరిమాణం(*adrive));
    ఒకవేళ (!adrive) {
        warn_noalloc();
        తిరిగి ఖాళీ;
    }
    memset(adrive, 0, పరిమాణం(*adrive));
    adrive->chan_gf = dummy->chan_gf;
    adrive->slave = dummy->slave;
    adrive->drive.cntl_id = adrive->chan_gf->ataid * 2 + dummy->slave;
    adrive->drive.removable = (బఫరు[0] & 0x80) ? 1 : 0;
    తిరిగి adrive;
}

// Detect if the given drive is an atapi - initialize it if so.
స్థిర నిర్మాణం atadrive_s *
init_drive_atapi(నిర్మాణం atadrive_s *dummy, u16 *బఫరు)
{
    // Send an IDENTIFY_DEVICE_PACKET command to device
    పూర్ణాంకం ret = send_ata_తటస్థం(dummy, బఫరు, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    ఒకవేళ (ret)
        తిరిగి ఖాళీ;

    // Success - setup as ATAPI.
    నిర్మాణం atadrive_s *adrive = init_atadrive(dummy, బఫరు);
    ఒకవేళ (!adrive)
        తిరిగి ఖాళీ;
    adrive->drive.type = DTYPE_ATA_ATAPI;
    adrive->drive.blksize = CDROM_SECTOR_కొలత;
    adrive->drive.sectors = (u64)-1;
    u8 iscd = ((బఫరు[0] >> 8) & 0x1f) == 0x05;
    అక్షరం model[MAXMODEL+1];
    అక్షరం *desc = znprintf(MAXDESCSIZE
                          , "DVD/CD [ata%d-%d: %s ATAPI-%d %s]"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_వెలికితీయి_model(model, MAXMODEL, బఫరు)
                          , ata_వెలికితీయి_సంస్కరణ(బఫరు)
                          , (iscd ? "DVD/CD" : "Device"));
    dprintf(1, "%s\n", desc);

    // fill cdidmap
    ఒకవేళ (iscd) {
        పూర్ణాంకం prio = bootprio_కనుగొను_ata_device(adrive->chan_gf->pci_tmp,
                                            adrive->chan_gf->chanid,
                                            adrive->slave);
        boot_lchs_కనుగొను_ata_device(adrive->chan_gf->pci_tmp,
                                  adrive->chan_gf->chanid,
                                  adrive->slave,
                                  &(adrive->drive.lchs));
        boot_కూడు_cd(&adrive->drive, desc, prio);
    }

    తిరిగి adrive;
}

// Detect if the given drive is a regular ata drive - initialize it if so.
స్థిర నిర్మాణం atadrive_s *
init_drive_ata(నిర్మాణం atadrive_s *dummy, u16 *బఫరు)
{
    // Send an IDENTIFY_DEVICE command to device
    పూర్ణాంకం ret = send_ata_తటస్థం(dummy, బఫరు, ATA_CMD_IDENTIFY_DEVICE);
    ఒకవేళ (ret)
        తిరిగి ఖాళీ;

    // Success - setup as ATA.
    నిర్మాణం atadrive_s *adrive = init_atadrive(dummy, బఫరు);
    ఒకవేళ (!adrive)
        తిరిగి ఖాళీ;
    adrive->drive.type = DTYPE_ATA;
    adrive->drive.blksize = DISK_SECTOR_కొలత;

    adrive->drive.pchs.cylinder = బఫరు[1];
    adrive->drive.pchs.తల = బఫరు[3];
    adrive->drive.pchs.sector = బఫరు[6];

    u64 sectors;
    ఒకవేళ (బఫరు[83] & (1 << 10)) // word 83 - lba48 support
        sectors = *(u64*)&బఫరు[100]; // word 100-103
    లేకపోతే
        sectors = *(u32*)&బఫరు[60]; // word 60 and word 61
    adrive->drive.sectors = sectors;
    u64 adjsize = sectors >> 11;
    అక్షరం adjprefix = 'M';
    ఒకవేళ (adjsize >= (1 << 16)) {
        adjsize >>= 10;
        adjprefix = 'G';
    }
    అక్షరం model[MAXMODEL+1];
    అక్షరం *desc = znprintf(MAXDESCSIZE
                          , "ata%d-%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_వెలికితీయి_model(model, MAXMODEL, బఫరు)
                          , ata_వెలికితీయి_సంస్కరణ(బఫరు)
                          , (u32)adjsize, adjprefix);
    dprintf(1, "%s\n", desc);

    పూర్ణాంకం prio = bootprio_కనుగొను_ata_device(adrive->chan_gf->pci_tmp,
                                        adrive->chan_gf->chanid,
                                        adrive->slave);
    boot_lchs_కనుగొను_ata_device(adrive->chan_gf->pci_tmp,
                              adrive->chan_gf->chanid,
                              adrive->slave,
                              &(adrive->drive.lchs));
    // Register with bcv system.
    boot_కూడు_hd(&adrive->drive, desc, prio);

    తిరిగి adrive;
}

స్థిర u32 Spinup_చివర;

// Wait for non-busy status and check for "floating bus" condition.
స్థిర పూర్ణాంకం
powerup_await_non_bsy(u16 పునాది)
{
    u8 orstatus = 0;
    u8 స్థితి_2;
    కొరకు (;;) {
        స్థితి_2 = inb(పునాది+ATA_CB_STAT);
        ఒకవేళ (!(స్థితి_2 & ATA_CB_STAT_BSY))
            ఆపు;
        orstatus |= స్థితి_2;
        ఒకవేళ (orstatus == 0xff) {
            dprintf(4, "powerup IDE floating\n");
            తిరిగి orstatus;
        }
        ఒకవేళ (timer_check(Spinup_చివర)) {
            warn_timeout();
            తిరిగి -1;
        }
        yield();
    }
    dprintf(6, "powerup iobase=%x st=%x\n", పునాది, స్థితి_2);
    తిరిగి స్థితి_2;
}

// Detect any drives attached to a given controller.
స్థిర శూన్య
ata_detect(శూన్య *దత్తాంశం)
{
    నిర్మాణం ata_channel_s *chan_gf = దత్తాంశం;
    నిర్మాణం atadrive_s dummy;
    memset(&dummy, 0, పరిమాణం(dummy));
    dummy.chan_gf = chan_gf;
    // Device detection
    పూర్ణాంకం didreset = 0;
    u8 slave;
    కొరకు (slave=0; slave<=1; slave++) {
        // Wait for not-bsy.
        u16 iobase1 = chan_gf->iobase1;
        పూర్ణాంకం స్థితి_2 = powerup_await_non_bsy(iobase1);
        ఒకవేళ (స్థితి_2 < 0)
            కొనసాగు;
        u8 newdh = slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0;
        outb(newdh, iobase1+ATA_CB_DH);
        ndelay(400);
        స్థితి_2 = powerup_await_non_bsy(iobase1);
        ఒకవేళ (స్థితి_2 < 0)
            కొనసాగు;

        // Check if ioport registers look valid.
        outb(newdh, iobase1+ATA_CB_DH);
        u8 dh = inb(iobase1+ATA_CB_DH);
        outb(0x55, iobase1+ATA_CB_SC);
        outb(0xaa, iobase1+ATA_CB_SN);
        u8 sc = inb(iobase1+ATA_CB_SC);
        u8 sn = inb(iobase1+ATA_CB_SN);
        dprintf(6, "ata_detect ata%d-%d: sc=%x sn=%x dh=%x\n"
                , chan_gf->ataid, slave, sc, sn, dh);
        ఒకవేళ (sc != 0x55 || sn != 0xaa || dh != newdh)
            కొనసాగు;

        // Prepare new drive.
        dummy.slave = slave;

        // reset the channel
        ఒకవేళ (!didreset) {
            ata_reset(&dummy);
            didreset = 1;
        }

        // check for ATAPI
        u16 బఫరు[256];
        నిర్మాణం atadrive_s *adrive = init_drive_atapi(&dummy, బఫరు);
        ఒకవేళ (!adrive) {
            // Didn't find an ATAPI drive - look for ATA drive.
            u8 st = inb(iobase1+ATA_CB_STAT);
            ఒకవేళ (!st)
                // Status not set - can't be a valid drive.
                కొనసాగు;

            // Wait for RDY.
            పూర్ణాంకం ret = await_rdy(iobase1);
            ఒకవేళ (ret < 0)
                కొనసాగు;

            // check for ATA.
            adrive = init_drive_ata(&dummy, బఫరు);
            ఒకవేళ (!adrive)
                // No ATA drive found
                కొనసాగు;
        }

        u16 resetresult = బఫరు[93];
        dprintf(6, "ata_detect resetresult=%04x\n", resetresult);
        ఒకవేళ (!slave && (resetresult & 0xdf61) == 0x4041)
            // resetresult looks valid and device 0 is responding to
            // device 1 requests - device 1 must not be present - skip
            // detection.
            ఆపు;
    }
}

// Initialize an ata controller and detect its drives.
స్థిర శూన్య
init_controller(నిర్మాణం pci_device *pci, పూర్ణాంకం chanid, పూర్ణాంకం irq
                , u32 port1, u32 port2, u32 master)
{
    స్థిర పూర్ణాంకం ataid = 0;
    నిర్మాణం ata_channel_s *chan_gf = malloc_fseg(పరిమాణం(*chan_gf));
    ఒకవేళ (!chan_gf) {
        warn_noalloc();
        తిరిగి;
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
    నిరంతరం_thread(ata_detect, chan_gf);
}

#నిర్వచించు IRQ_ATA1 14
#నిర్వచించు IRQ_ATA2 15

// Handle controllers on an ATA PCI device.
స్థిర శూన్య
init_pciata(నిర్మాణం pci_device *pci, u8 prog_if)
{
    u8 pciirq = pci_config_readb(pci->bdf, PCI_అంతరాయం_పంక్తి);
    పూర్ణాంకం master = 0;
    ఒకవేళ (CONFIG_ATA_DMA && prog_if & 0x80) {
        // Check for bus-mastering.
        u32 bar = pci_config_readl(pci->bdf, PCI_పునాది_చిరునామా_4);
        ఒకవేళ (bar & PCI_పునాది_చిరునామా_SPACE_IO) {
            master = pci_enable_iobar(pci, PCI_పునాది_చిరునామా_4);
            pci_enable_busmaster(pci);
        }
    }

    u32 port1, port2, irq;
    ఒకవేళ (prog_if & 1) {
        port1 = pci_enable_iobar(pci, PCI_పునాది_చిరునామా_0);
        port2 = pci_enable_iobar(pci, PCI_పునాది_చిరునామా_1);
        ఒకవేళ (!port1 || !port2)
            తిరిగి;
        irq = pciirq;
    } లేకపోతే {
        port1 = పోర్టు_ATA1_CMD_పునాది;
        port2 = పోర్టు_ATA1_CTRL_పునాది;
        irq = IRQ_ATA1;
    }
    init_controller(pci, 0, irq, port1, port2, master);

    ఒకవేళ (prog_if & 4) {
        port1 = pci_enable_iobar(pci, PCI_పునాది_చిరునామా_2);
        port2 = pci_enable_iobar(pci, PCI_పునాది_చిరునామా_3);
        ఒకవేళ (!port1 || !port2)
            తిరిగి;
        irq = pciirq;
    } లేకపోతే {
        port1 = పోర్టు_ATA2_CMD_పునాది;
        port2 = పోర్టు_ATA2_CTRL_పునాది;
        irq = IRQ_ATA2;
    }
    init_controller(pci, 1, irq, port1, port2, master ? master + 8 : 0);
}

స్థిర శూన్య
దొరికింది_genericata(నిర్మాణం pci_device *pci, శూన్య *arg)
{
    init_pciata(pci, pci->prog_if);
}

స్థిర శూన్య
దొరికింది_compatibleahci(నిర్మాణం pci_device *pci, శూన్య *arg)
{
    ఒకవేళ (CONFIG_AHCI)
        // Already handled directly via native ahci interface.
        తిరిగి;
    init_pciata(pci, 0x8f);
}

స్థిర స్థిరరాశి నిర్మాణం pci_device_id pci_ata_tbl[] = {
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE
                     , దొరికింది_genericata),
    PCI_DEVICE(PCI_VENDOR_ID_ATI, 0x4391, దొరికింది_compatibleahci),
    PCI_DEVICE_చివర,
};

// Locate and init ata controllers.
స్థిర శూన్య
ata_పరిశీలన(శూన్య)
{
    ఒకవేళ (CONFIG_QEMU && hlist_ఖాళీలు(&PCIDevices)) {
        // No PCI devices found - probably a QEMU "-M isapc" machine.
        // Try using ISA ports for ATA controllers.
        init_controller(ఖాళీ, 0, IRQ_ATA1
                        , పోర్టు_ATA1_CMD_పునాది, పోర్టు_ATA1_CTRL_పునాది, 0);
        init_controller(ఖాళీ, 1, IRQ_ATA2
                        , పోర్టు_ATA2_CMD_పునాది, పోర్టు_ATA2_CTRL_పునాది, 0);
        తిరిగి;
    }

    // Scan PCI bus for ATA adapters
    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_ata_tbl, pci, ఖాళీ);
    }
}

శూన్య
ata_అమరిక(శూన్య)
{
    ASSERT32FLAT();
    ఒకవేళ (!CONFIG_ATA)
        తిరిగి;

    dprintf(3, "init hard drives\n");

    Spinup_చివర = timer_calc(IDE_TIMEOUT);
    ata_పరిశీలన();

    సమితి_BDA(disk_control_byte, 0xc0);

    enable_hwirq(14, FUNC16(నమోదు_76));
}

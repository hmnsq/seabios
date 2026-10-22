// Low level ATA disk access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "ata.h" // ATA_CB_STAT
#અન્તર્ભાવ "biosvar.h" // GET_GLOBALFLAT
#અન્તર્ભાવ "block.h" // struct drive_s
#અન્તર્ભાવ "blockcmd.h" // CDB_CMD_READ_10
#અન્તર્ભાવ "byteorder.h" // be16_to_cpu
#અન્તર્ભાવ "malloc.h" // malloc_fseg
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "pci.h" // pci_config_readb
#અન્તર્ભાવ "pcidevice.h" // foreachpci
#અન્તર્ભાવ "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#અન્તર્ભાવ "pci_regs.h" // PCI_INTERRUPT_LINE
#અન્તર્ભાવ "pic.h" // enable_hwirq
#અન્તર્ભાવ "stacks.h" // yield
#અન્તર્ભાવ "std/disk.h" // DISK_RET_SUCCESS
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // timer_calc
#અન્તર્ભાવ "x86.h" // inb

#define IDE_TIMEOUT 32000 //32 seconds max for IDE ops


/****************************************************************
 * Helper functions
 ****************************************************************/

// Wait for the specified ide state
સ્થિર અંતઃરેખ પૂર્ણાંક
await_ide(u8 mask, u8 flags, u16 base, u16 timeout)
{
    u32 end = timer_calc(timeout);
    માટે (;;) {
        u8 status = inb(base+ATA_CB_STAT);
        જો ((status & mask) == flags)
            પાછા status;
        જો (timer_check(end)) {
            warn_timeout();
            પાછા -1;
        }
        yield();
    }
}

// Wait for the device to be not-busy.
સ્થિર પૂર્ણાંક
await_not_bsy(u16 base)
{
    પાછા await_ide(ATA_CB_STAT_BSY, 0, base, IDE_TIMEOUT);
}

// Wait for the device to be ready.
સ્થિર પૂર્ણાંક
await_rdy(u16 base)
{
    પાછા await_ide(ATA_CB_STAT_RDY, ATA_CB_STAT_RDY, base, IDE_TIMEOUT);
}

// Wait for ide state - pauses for one ata cycle first.
સ્થિર અંતઃરેખ પૂર્ણાંક
pause_await_not_bsy(u16 iobase1, u16 iobase2)
{
    // Wait one PIO transfer cycle.
    inb(iobase2 + ATA_CB_ASTAT);

    પાછા await_not_bsy(iobase1);
}

// Wait for ide state - pause for 400ns first.
સ્થિર અંતઃરેખ પૂર્ણાંક
ndelay_await_not_bsy(u16 iobase1)
{
    ndelay(400);
    પાછા await_not_bsy(iobase1);
}

// Reset a drive
સ્થિર શૂન્ય
ata_reset(સંરચના atadrive_s *adrive_gf)
{
    સંરચના ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
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
    પૂર્ણાંક status = await_not_bsy(iobase1);
    જો (status < 0)
        જાઓ done;
    જો (slave) {
        // Change device.
        u32 end = timer_calc(IDE_TIMEOUT);
        માટે (;;) {
            outb(ATA_CB_DH_DEV1, iobase1 + ATA_CB_DH);
            status = ndelay_await_not_bsy(iobase1);
            જો (status < 0)
                જાઓ done;
            જો (inb(iobase1 + ATA_CB_DH) == ATA_CB_DH_DEV1)
                તોડો;
            // Change drive request failed to take effect - retry.
            જો (timer_check(end)) {
                warn_timeout();
                જાઓ done;
            }
        }
    } નહીંતર {
        // QEMU doesn't reset dh on reset, so set it explicitly.
        outb(ATA_CB_DH_DEV0, iobase1 + ATA_CB_DH);
    }

    // On a user-reset request, wait for RDY if it is an ATA device.
    u8 type=GET_GLOBALFLAT(adrive_gf->drive.type);
    જો (type == DTYPE_ATA)
        status = await_rdy(iobase1);

done:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    dprintf(6, "ata_reset exit status=%x\n", status);
}

// Check for drive RDY for 16bit interface command.
સ્થિર પૂર્ણાંક
isready(સંરચના atadrive_s *adrive_gf)
{
    // Read the status from controller
    સંરચના ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u8 status = inb(iobase1 + ATA_CB_STAT);
    જો ((status & (ATA_CB_STAT_BSY|ATA_CB_STAT_RDY)) == ATA_CB_STAT_RDY)
        પાછા DISK_RET_SUCCESS;
    પાછા DISK_RET_ENOTREADY;
}


/****************************************************************
 * ATA send command
 ****************************************************************/

સંરચના ata_pio_command {
    u8 feature;
    u8 sector_ગણતરી;
    u8 lba_નીચલું;
    u8 lba_mid;
    u8 lba_ઉપલું;
    u8 device;
    u8 command;

    u8 feature2;
    u8 sector_count2;
    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
};

// Send an ata command to the drive.
સ્થિર પૂર્ણાંક
send_cmd(સંરચના atadrive_s *adrive_gf, સંરચના ata_pio_command *cmd)
{
    સંરચના ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);

    // Select device
    પૂર્ણાંક status = await_not_bsy(iobase1);
    જો (status < 0)
        પાછા status;
    u8 newdh = ((cmd->device & ~ATA_CB_DH_DEV1)
                | (slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0));
    u8 olddh = inb(iobase1 + ATA_CB_DH);
    outb(newdh, iobase1 + ATA_CB_DH);
    જો ((olddh ^ newdh) & (1<<4)) {
        // Was a device change - wait for device to become not busy.
        status = ndelay_await_not_bsy(iobase1);
        જો (status < 0)
            પાછા status;
    }

    // Check for ATA_CMD_(READ|WRITE)_(SECTORS|DMA)_EXT commands.
    જો ((cmd->command & ~0x11) == ATA_CMD_READ_SECTORS_EXT) {
        outb(cmd->feature2, iobase1 + ATA_CB_FR);
        outb(cmd->sector_count2, iobase1 + ATA_CB_SC);
        outb(cmd->lba_low2, iobase1 + ATA_CB_SN);
        outb(cmd->lba_mid2, iobase1 + ATA_CB_CL);
        outb(cmd->lba_high2, iobase1 + ATA_CB_CH);
    }
    outb(cmd->feature, iobase1 + ATA_CB_FR);
    outb(cmd->sector_ગણતરી, iobase1 + ATA_CB_SC);
    outb(cmd->lba_નીચલું, iobase1 + ATA_CB_SN);
    outb(cmd->lba_mid, iobase1 + ATA_CB_CL);
    outb(cmd->lba_ઉપલું, iobase1 + ATA_CB_CH);
    outb(cmd->command, iobase1 + ATA_CB_CMD);

    પાછા 0;
}

// Wait for data after calling 'send_cmd'.
સ્થિર પૂર્ણાંક
ata_wait_data(u16 iobase1)
{
    પૂર્ણાંક status = ndelay_await_not_bsy(iobase1);
    જો (status < 0)
        પાછા status;

    જો (status & ATA_CB_STAT_ERR) {
        dprintf(6, "send_cmd : read error (status=%02x err=%02x)\n"
                , status, inb(iobase1 + ATA_CB_ERR));
        પાછા -4;
    }
    જો (!(status & ATA_CB_STAT_DRQ)) {
        dprintf(6, "send_cmd : DRQ not set (status %02x)\n", status);
        પાછા -5;
    }

    પાછા 0;
}

// Send an ata command that does not transfer any further data.
પૂર્ણાંક
ata_cmd_nondata(સંરચના atadrive_s *adrive_gf, સંરચના ata_pio_command *cmd)
{
    સંરચના ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    પૂર્ણાંક ret = send_cmd(adrive_gf, cmd);
    જો (ret)
        જાઓ fail;
    ret = ndelay_await_not_bsy(iobase1);
    જો (ret < 0)
        જાઓ fail;

    જો (ret & ATA_CB_STAT_ERR) {
        dprintf(6, "nondata cmd : read error (status=%02x err=%02x)\n"
                , ret, inb(iobase1 + ATA_CB_ERR));
        ret = -4;
        જાઓ fail;
    }
    જો (ret & ATA_CB_STAT_DRQ) {
        dprintf(6, "nondata cmd : DRQ set (status %02x)\n", ret);
        ret = -5;
        જાઓ fail;
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    પાછા ret;
}


/****************************************************************
 * ATA PIO transfers
 ****************************************************************/

// Transfer 'op->count' blocks (of 'blocksize' bytes) to/from drive
// 'op->drive_fl'.
સ્થિર પૂર્ણાંક
ata_pio_transfer(સંરચના disk_op_s *op, પૂર્ણાંક iswrite, પૂર્ણાંક blocksize)
{
    dprintf(16, "ata_pio_transfer id=%p write=%d count=%d bs=%d buf=%p\n"
            , op->drive_fl, iswrite, op->ગણતરી, blocksize, op->buf_fl);

    સંરચના atadrive_s *adrive_gf = container_of(
        op->drive_fl, સંરચના atadrive_s, drive);
    સંરચના ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    પૂર્ણાંક ગણતરી = op->ગણતરી;
    શૂન્ય *buf_fl = op->buf_fl;
    પૂર્ણાંક status;
    માટે (;;) {
        જો (iswrite) {
            // Write data to controller
            dprintf(16, "Write sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            જો (CONFIG_ATA_PIO32)
                outsl_fl(iobase1, buf_fl, blocksize / 4);
            નહીંતર
                outsw_fl(iobase1, buf_fl, blocksize / 2);
        } નહીંતર {
            // Read data from controller
            dprintf(16, "Read sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            જો (CONFIG_ATA_PIO32)
                insl_fl(iobase1, buf_fl, blocksize / 4);
            નહીંતર
                insw_fl(iobase1, buf_fl, blocksize / 2);
        }
        buf_fl += blocksize;

        status = pause_await_not_bsy(iobase1, iobase2);
        જો (status < 0) {
            // Error
            op->ગણતરી -= ગણતરી;
            પાછા status;
        }

        ગણતરી--;
        જો (!ગણતરી)
            તોડો;
        status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ | ATA_CB_STAT_ERR);
        જો (status != ATA_CB_STAT_DRQ) {
            dprintf(6, "ata_pio_transfer : more sectors left (status %02x)\n"
                    , status);
            op->ગણતરી -= ગણતરી;
            પાછા -6;
        }
    }

    status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
               | ATA_CB_STAT_ERR);
    જો (!iswrite)
        status &= ~ATA_CB_STAT_DF;
    જો (status != 0) {
        dprintf(6, "ata_pio_transfer : no sectors left (status %02x)\n", status);
        પાછા -7;
    }

    પાછા 0;
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

સંરચના sff_dma_prd {
    u32 buf_fl;
    u32 ગણતરી;
};

// Check if DMA available and setup transfer if so.
સ્થિર પૂર્ણાંક
ata_try_dma(સંરચના disk_op_s *op, પૂર્ણાંક iswrite, પૂર્ણાંક blocksize)
{
    ASSERT16();
    જો (! CONFIG_ATA_DMA)
        પાછા -1;
    u32 dest = (u32)op->buf_fl;
    જો (dest & 1)
        // Need minimum alignment of 1.
        પાછા -1;
    સંરચના atadrive_s *adrive_gf = container_of(
        op->drive_fl, સંરચના atadrive_s, drive);
    સંરચના ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);
    જો (! iomaster)
        પાછા -1;
    u32 bytes = op->ગણતરી * blocksize;
    જો (! bytes)
        પાછા -1;

    // Build PRD dma structure.
    સંરચના sff_dma_prd *dma = MAKE_FLATPTR(SEG_નીચલું, Extra_ઢગલો);
    સંરચના sff_dma_prd *origdma = dma;
    જ્યારે (bytes) {
        જો (dma >= &origdma[16])
            // Too many descriptors..
            પાછા -1;
        u32 ગણતરી = bytes;
        u32 max = 0x10000 - (dest & 0xffff);
        જો (ગણતરી > max)
            ગણતરી = max;

        SET_LOWFLAT(dma->buf_fl, dest);
        bytes -= ગણતરી;
        જો (!bytes)
            // Last descriptor.
            ગણતરી |= 1<<31;
        dprintf(16, "dma@%p: %08x %08x\n", dma, dest, ગણતરી);
        dest += ગણતરી;
        SET_LOWFLAT(dma->ગણતરી, ગણતરી);
        dma++;
    }

    // Program bus-master controller.
    outl((u32)origdma, iomaster + BM_TABLE);
    u8 oldcmd = inb(iomaster + BM_CMD) & ~(BM_CMD_MEMWRITE|BM_CMD_START);
    outb(oldcmd | (iswrite ? 0x00 : BM_CMD_MEMWRITE), iomaster + BM_CMD);
    outb(BM_STATUS_ERROR|BM_STATUS_IRQ, iomaster + BM_STATUS);

    પાછા 0;
}

// Transfer data using DMA.
સ્થિર પૂર્ણાંક
ata_dma_transfer(સંરચના disk_op_s *op)
{
    જો (! CONFIG_ATA_DMA)
        પાછા -1;
    dprintf(16, "ata_dma_transfer id=%p buf=%p\n", op->drive_fl, op->buf_fl);

    સંરચના atadrive_s *adrive_gf = container_of(
        op->drive_fl, સંરચના atadrive_s, drive);
    સંરચના ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);

    // Start bus-master controller.
    u8 oldcmd = inb(iomaster + BM_CMD);
    outb(oldcmd | BM_CMD_START, iomaster + BM_CMD);

    u32 end = timer_calc(IDE_TIMEOUT);
    u8 status;
    માટે (;;) {
        status = inb(iomaster + BM_STATUS);
        જો (status & BM_STATUS_IRQ)
            તોડો;
        // Transfer in progress
        જો (timer_check(end)) {
            // Timeout.
            warn_timeout();
            તોડો;
        }
        yield();
    }
    outb(oldcmd & ~BM_CMD_START, iomaster + BM_CMD);

    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    પૂર્ણાંક idestatus = pause_await_not_bsy(iobase1, iobase2);

    જો ((status & (BM_STATUS_IRQ|BM_STATUS_ACTIVE)) == BM_STATUS_IRQ
        && idestatus >= 0x00
        && (idestatus & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
                         | ATA_CB_STAT_ERR)) == 0x00)
        // Success.
        પાછા 0;

    dprintf(6, "IDE DMA error (dma=%x ide=%x/%x/%x)\n", status, idestatus
            , inb(iobase2 + ATA_CB_ASTAT), inb(iobase1 + ATA_CB_ERR));
    પાછા -1;
}


/****************************************************************
 * ATA hard drive functions
 ****************************************************************/

// Transfer data to harddrive using PIO protocol.
સ્થિર પૂર્ણાંક
ata_pio_cmd_data(સંરચના disk_op_s *op, પૂર્ણાંક iswrite, સંરચના ata_pio_command *cmd)
{
    સંરચના atadrive_s *adrive_gf = container_of(
        op->drive_fl, સંરચના atadrive_s, drive);
    સંરચના ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    પૂર્ણાંક ret = send_cmd(adrive_gf, cmd);
    જો (ret)
        જાઓ fail;
    ret = ata_wait_data(iobase1);
    જો (ret)
        જાઓ fail;
    ret = ata_pio_transfer(op, iswrite, DISK_SECTOR_SIZE);

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    પાછા ret;
}

// Transfer data to harddrive using DMA protocol.
સ્થિર પૂર્ણાંક
ata_dma_cmd_data(સંરચના disk_op_s *op, સંરચના ata_pio_command *cmd)
{
    જો (! CONFIG_ATA_DMA)
        પાછા -1;
    સંરચના atadrive_s *adrive_gf = container_of(
        op->drive_fl, સંરચના atadrive_s, drive);
    પૂર્ણાંક ret = send_cmd(adrive_gf, cmd);
    જો (ret)
        પાછા ret;
    પાછા ata_dma_transfer(op);
}

// Read/write count blocks from a harddrive.
સ્થિર પૂર્ણાંક
ata_readwrite(સંરચના disk_op_s *op, પૂર્ણાંક iswrite)
{
    u64 lba = op->lba;

    પૂર્ણાંક usepio = ata_try_dma(op, iswrite, DISK_SECTOR_SIZE);

    સંરચના ata_pio_command cmd;
    memset(&cmd, 0, કદ(cmd));

    જો (op->ગણતરી >= (1<<8) || lba + op->ગણતરી >= (1<<28)) {
        cmd.sector_count2 = op->ગણતરી >> 8;
        cmd.lba_low2 = lba >> 24;
        cmd.lba_mid2 = lba >> 32;
        cmd.lba_high2 = lba >> 40;
        lba &= 0xffffff;

        જો (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS_EXT
                           : ATA_CMD_READ_SECTORS_EXT);
        નહીંતર
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                           : ATA_CMD_READ_DMA_EXT);
    } નહીંતર {
        જો (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS
                           : ATA_CMD_READ_SECTORS);
        નહીંતર
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA
                           : ATA_CMD_READ_DMA);
    }

    cmd.sector_ગણતરી = op->ગણતરી;
    cmd.lba_નીચલું = lba;
    cmd.lba_mid = lba >> 8;
    cmd.lba_ઉપલું = lba >> 16;
    cmd.device = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;

    પૂર્ણાંક ret;
    જો (usepio)
        ret = ata_pio_cmd_data(op, iswrite, &cmd);
    નહીંતર
        ret = ata_dma_cmd_data(op, &cmd);
    જો (ret)
        પાછા DISK_RET_EBADTRACK;
    પાછા DISK_RET_SUCCESS;
}

// 16bit command demuxer for ATA harddrives.
પૂર્ણાંક
ata_process_op(સંરચના disk_op_s *op)
{
    જો (!CONFIG_ATA)
        પાછા 0;

    સંરચના atadrive_s *adrive_gf = container_of(
        op->drive_fl, સંરચના atadrive_s, drive);
    પસંદગી (op->command) {
    સ્થિતિ CMD_READ:
        પાછા ata_readwrite(op, 0);
    સ્થિતિ CMD_WRITE:
        પાછા ata_readwrite(op, 1);
    સ્થિતિ CMD_RESET:
        ata_reset(adrive_gf);
        પાછા DISK_RET_SUCCESS;
    સ્થિતિ CMD_ISREADY:
        પાછા isready(adrive_gf);
    મૂળભૂત:
        પાછા default_process_op(op);
    }
}


/****************************************************************
 * ATAPI functions
 ****************************************************************/

#define CDROM_CDB_SIZE 12

// Low-level atapi command transmit function.
પૂર્ણાંક
ata_atapi_process_op(સંરચના disk_op_s *op)
{
    જો (! CONFIG_ATA)
        પાછા 0;

    જો (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        પાછા DISK_RET_EWRITEPROTECT;
    u8 cdbcmd[CDROM_CDB_SIZE];
    પૂર્ણાંક blocksize = scsi_fill_cmd(op, cdbcmd, કદ(cdbcmd));
    જો (blocksize < 0)
        પાછા default_process_op(op);

    સંરચના atadrive_s *adrive_gf = container_of(
        op->drive_fl, સંરચના atadrive_s, drive);
    સંરચના ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    સંરચના ata_pio_command cmd;
    memset(&cmd, 0, કદ(cmd));
    cmd.lba_mid = blocksize;
    cmd.lba_ઉપલું = blocksize >> 8;
    cmd.command = ATA_CMD_PACKET;

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    પૂર્ણાંક ret = send_cmd(adrive_gf, &cmd);
    જો (ret)
        જાઓ fail;
    ret = ata_wait_data(iobase1);
    જો (ret)
        જાઓ fail;

    // Send command to device
    outsw_fl(iobase1, MAKE_FLATPTR(GET_SEG(SS), cdbcmd), CDROM_CDB_SIZE / 2);

    પૂર્ણાંક status = pause_await_not_bsy(iobase1, iobase2);
    જો (status < 0) {
        ret = status;
        જાઓ fail;
    }

    જો (status & ATA_CB_STAT_ERR) {
        u8 err = inb(iobase1 + ATA_CB_ERR);
        // skip "Not Ready"
        જો (err != 0x20)
            dprintf(6, "send_atapi_cmd : read error (status=%02x err=%02x)\n"
                    , status, err);
        ret = -2;
        જાઓ fail;
    }
    જો (blocksize) {
        જો (!(status & ATA_CB_STAT_DRQ)) {
            dprintf(6, "send_atapi_cmd : DRQ not set (status %02x)\n", status);
            ret = -3;
            જાઓ fail;
        }

        ret = ata_pio_transfer(op, 0, blocksize);
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    જો (ret)
        પાછા DISK_RET_EBADTRACK;
    પાછા DISK_RET_SUCCESS;
}


/****************************************************************
 * ATA detect and init
 ****************************************************************/

// Send an identify device or identify device packet command.
સ્થિર પૂર્ણાંક
send_ata_identity(સંરચના atadrive_s *adrive, u16 *buffer, પૂર્ણાંક command)
{
    memset(buffer, 0, DISK_SECTOR_SIZE);

    સંરચના disk_op_s dop;
    memset(&dop, 0, કદ(dop));
    dop.drive_fl = &adrive->drive;
    dop.ગણતરી = 1;
    dop.lba = 1;
    dop.buf_fl = MAKE_FLATPTR(GET_SEG(SS), buffer);

    સંરચના ata_pio_command cmd;
    memset(&cmd, 0, કદ(cmd));
    cmd.command = command;

    પાછા ata_pio_cmd_data(&dop, 0, &cmd);
}

// Extract the ATA/ATAPI version info.
પૂર્ણાંક
ata_extract_version(u16 *buffer)
{
    // Extract ATA/ATAPI version.
    u16 ataversion = buffer[80];
    u8 version;
    માટે (version=15; version>0; version--)
        જો (ataversion & (1<<version))
            તોડો;
    પાછા version;
}

#define MAXMODEL 40

// Extract the ATA/ATAPI model info.
અક્ષર *
ata_extract_model(અક્ષર *model, u32 size, u16 *buffer)
{
    // Read model name
    પૂર્ણાંક i;
    માટે (i=0; i<size/2; i++)
        *(u16*)&model[i*2] = be16_to_cpu(buffer[27+i]);
    model[size] = 0x00;
    nullTrailingSpace(model);
    પાછા model;
}

// Common init code between ata and atapi
સ્થિર સંરચના atadrive_s *
init_atadrive(સંરચના atadrive_s *dummy, u16 *buffer)
{
    સંરચના atadrive_s *adrive = malloc_fseg(કદ(*adrive));
    જો (!adrive) {
        warn_noalloc();
        પાછા ખાલી;
    }
    memset(adrive, 0, કદ(*adrive));
    adrive->chan_gf = dummy->chan_gf;
    adrive->slave = dummy->slave;
    adrive->drive.cntl_id = adrive->chan_gf->ataid * 2 + dummy->slave;
    adrive->drive.removable = (buffer[0] & 0x80) ? 1 : 0;
    પાછા adrive;
}

// Detect if the given drive is an atapi - initialize it if so.
સ્થિર સંરચના atadrive_s *
init_drive_atapi(સંરચના atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE_PACKET command to device
    પૂર્ણાંક ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    જો (ret)
        પાછા ખાલી;

    // Success - setup as ATAPI.
    સંરચના atadrive_s *adrive = init_atadrive(dummy, buffer);
    જો (!adrive)
        પાછા ખાલી;
    adrive->drive.type = DTYPE_ATA_ATAPI;
    adrive->drive.blksize = CDROM_SECTOR_SIZE;
    adrive->drive.sectors = (u64)-1;
    u8 iscd = ((buffer[0] >> 8) & 0x1f) == 0x05;
    અક્ષર model[MAXMODEL+1];
    અક્ષર *desc = znprintf(MAXDESCSIZE
                          , "DVD/CD [ata%d-%d: %s ATAPI-%d %s]"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (iscd ? "DVD/CD" : "Device"));
    dprintf(1, "%s\n", desc);

    // fill cdidmap
    જો (iscd) {
        પૂર્ણાંક prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                            adrive->chan_gf->chanid,
                                            adrive->slave);
        boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                                  adrive->chan_gf->chanid,
                                  adrive->slave,
                                  &(adrive->drive.lchs));
        boot_add_cd(&adrive->drive, desc, prio);
    }

    પાછા adrive;
}

// Detect if the given drive is a regular ata drive - initialize it if so.
સ્થિર સંરચના atadrive_s *
init_drive_ata(સંરચના atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE command to device
    પૂર્ણાંક ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_DEVICE);
    જો (ret)
        પાછા ખાલી;

    // Success - setup as ATA.
    સંરચના atadrive_s *adrive = init_atadrive(dummy, buffer);
    જો (!adrive)
        પાછા ખાલી;
    adrive->drive.type = DTYPE_ATA;
    adrive->drive.blksize = DISK_SECTOR_SIZE;

    adrive->drive.pchs.cylinder = buffer[1];
    adrive->drive.pchs.head = buffer[3];
    adrive->drive.pchs.sector = buffer[6];

    u64 sectors;
    જો (buffer[83] & (1 << 10)) // word 83 - lba48 support
        sectors = *(u64*)&buffer[100]; // word 100-103
    નહીંતર
        sectors = *(u32*)&buffer[60]; // word 60 and word 61
    adrive->drive.sectors = sectors;
    u64 adjsize = sectors >> 11;
    અક્ષર adjprefix = 'M';
    જો (adjsize >= (1 << 16)) {
        adjsize >>= 10;
        adjprefix = 'G';
    }
    અક્ષર model[MAXMODEL+1];
    અક્ષર *desc = znprintf(MAXDESCSIZE
                          , "ata%d-%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (u32)adjsize, adjprefix);
    dprintf(1, "%s\n", desc);

    પૂર્ણાંક prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                        adrive->chan_gf->chanid,
                                        adrive->slave);
    boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                              adrive->chan_gf->chanid,
                              adrive->slave,
                              &(adrive->drive.lchs));
    // Register with bcv system.
    boot_add_hd(&adrive->drive, desc, prio);

    પાછા adrive;
}

સ્થિર u32 SpinupEnd;

// Wait for non-busy status and check for "floating bus" condition.
સ્થિર પૂર્ણાંક
powerup_await_non_bsy(u16 base)
{
    u8 orstatus = 0;
    u8 status;
    માટે (;;) {
        status = inb(base+ATA_CB_STAT);
        જો (!(status & ATA_CB_STAT_BSY))
            તોડો;
        orstatus |= status;
        જો (orstatus == 0xff) {
            dprintf(4, "powerup IDE floating\n");
            પાછા orstatus;
        }
        જો (timer_check(SpinupEnd)) {
            warn_timeout();
            પાછા -1;
        }
        yield();
    }
    dprintf(6, "powerup iobase=%x st=%x\n", base, status);
    પાછા status;
}

// Detect any drives attached to a given controller.
સ્થિર શૂન્ય
ata_detect(શૂન્ય *data)
{
    સંરચના ata_channel_s *chan_gf = data;
    સંરચના atadrive_s dummy;
    memset(&dummy, 0, કદ(dummy));
    dummy.chan_gf = chan_gf;
    // Device detection
    પૂર્ણાંક didreset = 0;
    u8 slave;
    માટે (slave=0; slave<=1; slave++) {
        // Wait for not-bsy.
        u16 iobase1 = chan_gf->iobase1;
        પૂર્ણાંક status = powerup_await_non_bsy(iobase1);
        જો (status < 0)
            ચાલુ;
        u8 newdh = slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0;
        outb(newdh, iobase1+ATA_CB_DH);
        ndelay(400);
        status = powerup_await_non_bsy(iobase1);
        જો (status < 0)
            ચાલુ;

        // Check if ioport registers look valid.
        outb(newdh, iobase1+ATA_CB_DH);
        u8 dh = inb(iobase1+ATA_CB_DH);
        outb(0x55, iobase1+ATA_CB_SC);
        outb(0xaa, iobase1+ATA_CB_SN);
        u8 sc = inb(iobase1+ATA_CB_SC);
        u8 sn = inb(iobase1+ATA_CB_SN);
        dprintf(6, "ata_detect ata%d-%d: sc=%x sn=%x dh=%x\n"
                , chan_gf->ataid, slave, sc, sn, dh);
        જો (sc != 0x55 || sn != 0xaa || dh != newdh)
            ચાલુ;

        // Prepare new drive.
        dummy.slave = slave;

        // reset the channel
        જો (!didreset) {
            ata_reset(&dummy);
            didreset = 1;
        }

        // check for ATAPI
        u16 buffer[256];
        સંરચના atadrive_s *adrive = init_drive_atapi(&dummy, buffer);
        જો (!adrive) {
            // Didn't find an ATAPI drive - look for ATA drive.
            u8 st = inb(iobase1+ATA_CB_STAT);
            જો (!st)
                // Status not set - can't be a valid drive.
                ચાલુ;

            // Wait for RDY.
            પૂર્ણાંક ret = await_rdy(iobase1);
            જો (ret < 0)
                ચાલુ;

            // check for ATA.
            adrive = init_drive_ata(&dummy, buffer);
            જો (!adrive)
                // No ATA drive found
                ચાલુ;
        }

        u16 resetresult = buffer[93];
        dprintf(6, "ata_detect resetresult=%04x\n", resetresult);
        જો (!slave && (resetresult & 0xdf61) == 0x4041)
            // resetresult looks valid and device 0 is responding to
            // device 1 requests - device 1 must not be present - skip
            // detection.
            તોડો;
    }
}

// Initialize an ata controller and detect its drives.
સ્થિર શૂન્ય
init_controller(સંરચના pci_device *pci, પૂર્ણાંક chanid, પૂર્ણાંક irq
                , u32 port1, u32 port2, u32 master)
{
    સ્થિર પૂર્ણાંક ataid = 0;
    સંરચના ata_channel_s *chan_gf = malloc_fseg(કદ(*chan_gf));
    જો (!chan_gf) {
        warn_noalloc();
        પાછા;
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
સ્થિર શૂન્ય
init_pciata(સંરચના pci_device *pci, u8 prog_if)
{
    u8 pciirq = pci_config_readb(pci->bdf, PCI_INTERRUPT_લીટી);
    પૂર્ણાંક master = 0;
    જો (CONFIG_ATA_DMA && prog_if & 0x80) {
        // Check for bus-mastering.
        u32 bar = pci_config_readl(pci->bdf, PCI_BASE_ADDRESS_4);
        જો (bar & PCI_BASE_ADDRESS_SPACE_IO) {
            master = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
            pci_enable_busmaster(pci);
        }
    }

    u32 port1, port2, irq;
    જો (prog_if & 1) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_1);
        જો (!port1 || !port2)
            પાછા;
        irq = pciirq;
    } નહીંતર {
        port1 = PORT_ATA1_CMD_BASE;
        port2 = PORT_ATA1_CTRL_BASE;
        irq = IRQ_ATA1;
    }
    init_controller(pci, 0, irq, port1, port2, master);

    જો (prog_if & 4) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_2);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_3);
        જો (!port1 || !port2)
            પાછા;
        irq = pciirq;
    } નહીંતર {
        port1 = PORT_ATA2_CMD_BASE;
        port2 = PORT_ATA2_CTRL_BASE;
        irq = IRQ_ATA2;
    }
    init_controller(pci, 1, irq, port1, port2, master ? master + 8 : 0);
}

સ્થિર શૂન્ય
found_genericata(સંરચના pci_device *pci, શૂન્ય *arg)
{
    init_pciata(pci, pci->prog_if);
}

સ્થિર શૂન્ય
found_compatibleahci(સંરચના pci_device *pci, શૂન્ય *arg)
{
    જો (CONFIG_AHCI)
        // Already handled directly via native ahci interface.
        પાછા;
    init_pciata(pci, 0x8f);
}

સ્થિર અચલ સંરચના pci_device_id pci_ata_tbl[] = {
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE
                     , found_genericata),
    PCI_DEVICE(PCI_VENDOR_ID_ATI, 0x4391, found_compatibleahci),
    PCI_DEVICE_END,
};

// Locate and init ata controllers.
સ્થિર શૂન્ય
ata_scan(શૂન્ય)
{
    જો (CONFIG_QEMU && hlist_empty(&PCIDevices)) {
        // No PCI devices found - probably a QEMU "-M isapc" machine.
        // Try using ISA ports for ATA controllers.
        init_controller(ખાલી, 0, IRQ_ATA1
                        , PORT_ATA1_CMD_BASE, PORT_ATA1_CTRL_BASE, 0);
        init_controller(ખાલી, 1, IRQ_ATA2
                        , PORT_ATA2_CMD_BASE, PORT_ATA2_CTRL_BASE, 0);
        પાછા;
    }

    // Scan PCI bus for ATA adapters
    સંરચના pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_ata_tbl, pci, ખાલી);
    }
}

શૂન્ય
ata_setup(શૂન્ય)
{
    ASSERT32FLAT();
    જો (!CONFIG_ATA)
        પાછા;

    dprintf(3, "init hard drives\n");

    SpinupEnd = timer_calc(IDE_TIMEOUT);
    ata_scan();

    SET_BDA(disk_control_byte, 0xc0);

    enable_hwirq(14, FUNC16(entry_76));
}

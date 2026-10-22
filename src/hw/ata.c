// Low level ATA disk access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "ata.h" // ATA_CB_STAT
#অন্তর্ভাব "biosvar.h" // GET_GLOBALFLAT
#অন্তর্ভাব "block.h" // struct drive_s
#অন্তর্ভাব "blockcmd.h" // CDB_CMD_READ_10
#অন্তর্ভাব "byteorder.h" // be16_to_cpu
#অন্তর্ভাব "malloc.h" // malloc_fseg
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "pci.h" // pci_config_readb
#অন্তর্ভাব "pcidevice.h" // foreachpci
#অন্তর্ভাব "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#অন্তর্ভাব "pci_regs.h" // PCI_INTERRUPT_LINE
#অন্তর্ভাব "pic.h" // enable_hwirq
#অন্তর্ভাব "stacks.h" // yield
#অন্তর্ভাব "std/disk.h" // DISK_RET_SUCCESS
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // timer_calc
#অন্তর্ভাব "x86.h" // inb

#define IDE_TIMEOUT 32000 //32 seconds max for IDE ops


/****************************************************************
 * Helper functions
 ****************************************************************/

// Wait for the specified ide state
স্থির অন্তঃরেখ পূর্ণসংখ্যা
await_ide(u8 mask, u8 flags, u16 base, u16 timeout)
{
    u32 end = timer_calc(timeout);
    জন্য (;;) {
        u8 status = inb(base+ATA_CB_STAT);
        যদি ((status & mask) == flags)
            ফেরত status;
        যদি (timer_check(end)) {
            warn_timeout();
            ফেরত -1;
        }
        yield();
    }
}

// Wait for the device to be not-busy.
স্থির পূর্ণসংখ্যা
await_not_bsy(u16 base)
{
    ফেরত await_ide(ATA_CB_STAT_BSY, 0, base, IDE_TIMEOUT);
}

// Wait for the device to be ready.
স্থির পূর্ণসংখ্যা
await_rdy(u16 base)
{
    ফেরত await_ide(ATA_CB_STAT_RDY, ATA_CB_STAT_RDY, base, IDE_TIMEOUT);
}

// Wait for ide state - pauses for one ata cycle first.
স্থির অন্তঃরেখ পূর্ণসংখ্যা
pause_await_not_bsy(u16 iobase1, u16 iobase2)
{
    // Wait one PIO transfer cycle.
    inb(iobase2 + ATA_CB_ASTAT);

    ফেরত await_not_bsy(iobase1);
}

// Wait for ide state - pause for 400ns first.
স্থির অন্তঃরেখ পূর্ণসংখ্যা
ndelay_await_not_bsy(u16 iobase1)
{
    ndelay(400);
    ফেরত await_not_bsy(iobase1);
}

// Reset a drive
স্থির শূন্য
ata_reset(কাঠামো atadrive_s *adrive_gf)
{
    কাঠামো ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
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
    পূর্ণসংখ্যা status = await_not_bsy(iobase1);
    যদি (status < 0)
        যাও done;
    যদি (slave) {
        // Change device.
        u32 end = timer_calc(IDE_TIMEOUT);
        জন্য (;;) {
            outb(ATA_CB_DH_DEV1, iobase1 + ATA_CB_DH);
            status = ndelay_await_not_bsy(iobase1);
            যদি (status < 0)
                যাও done;
            যদি (inb(iobase1 + ATA_CB_DH) == ATA_CB_DH_DEV1)
                বিরতি;
            // Change drive request failed to take effect - retry.
            যদি (timer_check(end)) {
                warn_timeout();
                যাও done;
            }
        }
    } অন্যথা {
        // QEMU doesn't reset dh on reset, so set it explicitly.
        outb(ATA_CB_DH_DEV0, iobase1 + ATA_CB_DH);
    }

    // On a user-reset request, wait for RDY if it is an ATA device.
    u8 type=GET_GLOBALFLAT(adrive_gf->drive.type);
    যদি (type == DTYPE_ATA)
        status = await_rdy(iobase1);

done:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    dprintf(6, "ata_reset exit status=%x\n", status);
}

// Check for drive RDY for 16bit interface command.
স্থির পূর্ণসংখ্যা
isready(কাঠামো atadrive_s *adrive_gf)
{
    // Read the status from controller
    কাঠামো ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u8 status = inb(iobase1 + ATA_CB_STAT);
    যদি ((status & (ATA_CB_STAT_BSY|ATA_CB_STAT_RDY)) == ATA_CB_STAT_RDY)
        ফেরত DISK_RET_SUCCESS;
    ফেরত DISK_RET_ENOTREADY;
}


/****************************************************************
 * ATA send command
 ****************************************************************/

কাঠামো ata_pio_command {
    u8 feature;
    u8 sector_গুনতি;
    u8 lba_নিচের;
    u8 lba_mid;
    u8 lba_উপরের;
    u8 device;
    u8 command;

    u8 feature2;
    u8 sector_count2;
    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
};

// Send an ata command to the drive.
স্থির পূর্ণসংখ্যা
send_cmd(কাঠামো atadrive_s *adrive_gf, কাঠামো ata_pio_command *cmd)
{
    কাঠামো ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);

    // Select device
    পূর্ণসংখ্যা status = await_not_bsy(iobase1);
    যদি (status < 0)
        ফেরত status;
    u8 newdh = ((cmd->device & ~ATA_CB_DH_DEV1)
                | (slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0));
    u8 olddh = inb(iobase1 + ATA_CB_DH);
    outb(newdh, iobase1 + ATA_CB_DH);
    যদি ((olddh ^ newdh) & (1<<4)) {
        // Was a device change - wait for device to become not busy.
        status = ndelay_await_not_bsy(iobase1);
        যদি (status < 0)
            ফেরত status;
    }

    // Check for ATA_CMD_(READ|WRITE)_(SECTORS|DMA)_EXT commands.
    যদি ((cmd->command & ~0x11) == ATA_CMD_READ_SECTORS_EXT) {
        outb(cmd->feature2, iobase1 + ATA_CB_FR);
        outb(cmd->sector_count2, iobase1 + ATA_CB_SC);
        outb(cmd->lba_low2, iobase1 + ATA_CB_SN);
        outb(cmd->lba_mid2, iobase1 + ATA_CB_CL);
        outb(cmd->lba_high2, iobase1 + ATA_CB_CH);
    }
    outb(cmd->feature, iobase1 + ATA_CB_FR);
    outb(cmd->sector_গুনতি, iobase1 + ATA_CB_SC);
    outb(cmd->lba_নিচের, iobase1 + ATA_CB_SN);
    outb(cmd->lba_mid, iobase1 + ATA_CB_CL);
    outb(cmd->lba_উপরের, iobase1 + ATA_CB_CH);
    outb(cmd->command, iobase1 + ATA_CB_CMD);

    ফেরত 0;
}

// Wait for data after calling 'send_cmd'.
স্থির পূর্ণসংখ্যা
ata_wait_তথ্য(u16 iobase1)
{
    পূর্ণসংখ্যা status = ndelay_await_not_bsy(iobase1);
    যদি (status < 0)
        ফেরত status;

    যদি (status & ATA_CB_STAT_ERR) {
        dprintf(6, "send_cmd : read error (status=%02x err=%02x)\n"
                , status, inb(iobase1 + ATA_CB_ERR));
        ফেরত -4;
    }
    যদি (!(status & ATA_CB_STAT_DRQ)) {
        dprintf(6, "send_cmd : DRQ not set (status %02x)\n", status);
        ফেরত -5;
    }

    ফেরত 0;
}

// Send an ata command that does not transfer any further data.
পূর্ণসংখ্যা
ata_cmd_nondata(কাঠামো atadrive_s *adrive_gf, কাঠামো ata_pio_command *cmd)
{
    কাঠামো ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    পূর্ণসংখ্যা ret = send_cmd(adrive_gf, cmd);
    যদি (ret)
        যাও fail;
    ret = ndelay_await_not_bsy(iobase1);
    যদি (ret < 0)
        যাও fail;

    যদি (ret & ATA_CB_STAT_ERR) {
        dprintf(6, "nondata cmd : read error (status=%02x err=%02x)\n"
                , ret, inb(iobase1 + ATA_CB_ERR));
        ret = -4;
        যাও fail;
    }
    যদি (ret & ATA_CB_STAT_DRQ) {
        dprintf(6, "nondata cmd : DRQ set (status %02x)\n", ret);
        ret = -5;
        যাও fail;
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    ফেরত ret;
}


/****************************************************************
 * ATA PIO transfers
 ****************************************************************/

// Transfer 'op->count' blocks (of 'blocksize' bytes) to/from drive
// 'op->drive_fl'.
স্থির পূর্ণসংখ্যা
ata_pio_transfer(কাঠামো disk_op_s *op, পূর্ণসংখ্যা iswrite, পূর্ণসংখ্যা blocksize)
{
    dprintf(16, "ata_pio_transfer id=%p write=%d count=%d bs=%d buf=%p\n"
            , op->drive_fl, iswrite, op->গুনতি, blocksize, op->buf_fl);

    কাঠামো atadrive_s *adrive_gf = container_of(
        op->drive_fl, কাঠামো atadrive_s, drive);
    কাঠামো ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    পূর্ণসংখ্যা গুনতি = op->গুনতি;
    শূন্য *buf_fl = op->buf_fl;
    পূর্ণসংখ্যা status;
    জন্য (;;) {
        যদি (iswrite) {
            // Write data to controller
            dprintf(16, "Write sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            যদি (CONFIG_ATA_PIO32)
                outsl_fl(iobase1, buf_fl, blocksize / 4);
            অন্যথা
                outsw_fl(iobase1, buf_fl, blocksize / 2);
        } অন্যথা {
            // Read data from controller
            dprintf(16, "Read sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            যদি (CONFIG_ATA_PIO32)
                insl_fl(iobase1, buf_fl, blocksize / 4);
            অন্যথা
                insw_fl(iobase1, buf_fl, blocksize / 2);
        }
        buf_fl += blocksize;

        status = pause_await_not_bsy(iobase1, iobase2);
        যদি (status < 0) {
            // Error
            op->গুনতি -= গুনতি;
            ফেরত status;
        }

        গুনতি--;
        যদি (!গুনতি)
            বিরতি;
        status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ | ATA_CB_STAT_ERR);
        যদি (status != ATA_CB_STAT_DRQ) {
            dprintf(6, "ata_pio_transfer : more sectors left (status %02x)\n"
                    , status);
            op->গুনতি -= গুনতি;
            ফেরত -6;
        }
    }

    status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
               | ATA_CB_STAT_ERR);
    যদি (!iswrite)
        status &= ~ATA_CB_STAT_DF;
    যদি (status != 0) {
        dprintf(6, "ata_pio_transfer : no sectors left (status %02x)\n", status);
        ফেরত -7;
    }

    ফেরত 0;
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

কাঠামো sff_dma_prd {
    u32 buf_fl;
    u32 গুনতি;
};

// Check if DMA available and setup transfer if so.
স্থির পূর্ণসংখ্যা
ata_try_dma(কাঠামো disk_op_s *op, পূর্ণসংখ্যা iswrite, পূর্ণসংখ্যা blocksize)
{
    ASSERT16();
    যদি (! CONFIG_ATA_DMA)
        ফেরত -1;
    u32 dest = (u32)op->buf_fl;
    যদি (dest & 1)
        // Need minimum alignment of 1.
        ফেরত -1;
    কাঠামো atadrive_s *adrive_gf = container_of(
        op->drive_fl, কাঠামো atadrive_s, drive);
    কাঠামো ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);
    যদি (! iomaster)
        ফেরত -1;
    u32 bytes = op->গুনতি * blocksize;
    যদি (! bytes)
        ফেরত -1;

    // Build PRD dma structure.
    কাঠামো sff_dma_prd *dma = MAKE_FLATPTR(SEG_নিচের, Extra_স্তূপ);
    কাঠামো sff_dma_prd *origdma = dma;
    যখন (bytes) {
        যদি (dma >= &origdma[16])
            // Too many descriptors..
            ফেরত -1;
        u32 গুনতি = bytes;
        u32 max = 0x10000 - (dest & 0xffff);
        যদি (গুনতি > max)
            গুনতি = max;

        SET_LOWFLAT(dma->buf_fl, dest);
        bytes -= গুনতি;
        যদি (!bytes)
            // Last descriptor.
            গুনতি |= 1<<31;
        dprintf(16, "dma@%p: %08x %08x\n", dma, dest, গুনতি);
        dest += গুনতি;
        SET_LOWFLAT(dma->গুনতি, গুনতি);
        dma++;
    }

    // Program bus-master controller.
    outl((u32)origdma, iomaster + BM_TABLE);
    u8 oldcmd = inb(iomaster + BM_CMD) & ~(BM_CMD_MEMWRITE|BM_CMD_START);
    outb(oldcmd | (iswrite ? 0x00 : BM_CMD_MEMWRITE), iomaster + BM_CMD);
    outb(BM_STATUS_ERROR|BM_STATUS_IRQ, iomaster + BM_STATUS);

    ফেরত 0;
}

// Transfer data using DMA.
স্থির পূর্ণসংখ্যা
ata_dma_transfer(কাঠামো disk_op_s *op)
{
    যদি (! CONFIG_ATA_DMA)
        ফেরত -1;
    dprintf(16, "ata_dma_transfer id=%p buf=%p\n", op->drive_fl, op->buf_fl);

    কাঠামো atadrive_s *adrive_gf = container_of(
        op->drive_fl, কাঠামো atadrive_s, drive);
    কাঠামো ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);

    // Start bus-master controller.
    u8 oldcmd = inb(iomaster + BM_CMD);
    outb(oldcmd | BM_CMD_START, iomaster + BM_CMD);

    u32 end = timer_calc(IDE_TIMEOUT);
    u8 status;
    জন্য (;;) {
        status = inb(iomaster + BM_STATUS);
        যদি (status & BM_STATUS_IRQ)
            বিরতি;
        // Transfer in progress
        যদি (timer_check(end)) {
            // Timeout.
            warn_timeout();
            বিরতি;
        }
        yield();
    }
    outb(oldcmd & ~BM_CMD_START, iomaster + BM_CMD);

    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    পূর্ণসংখ্যা idestatus = pause_await_not_bsy(iobase1, iobase2);

    যদি ((status & (BM_STATUS_IRQ|BM_STATUS_ACTIVE)) == BM_STATUS_IRQ
        && idestatus >= 0x00
        && (idestatus & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
                         | ATA_CB_STAT_ERR)) == 0x00)
        // Success.
        ফেরত 0;

    dprintf(6, "IDE DMA error (dma=%x ide=%x/%x/%x)\n", status, idestatus
            , inb(iobase2 + ATA_CB_ASTAT), inb(iobase1 + ATA_CB_ERR));
    ফেরত -1;
}


/****************************************************************
 * ATA hard drive functions
 ****************************************************************/

// Transfer data to harddrive using PIO protocol.
স্থির পূর্ণসংখ্যা
ata_pio_cmd_তথ্য(কাঠামো disk_op_s *op, পূর্ণসংখ্যা iswrite, কাঠামো ata_pio_command *cmd)
{
    কাঠামো atadrive_s *adrive_gf = container_of(
        op->drive_fl, কাঠামো atadrive_s, drive);
    কাঠামো ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    পূর্ণসংখ্যা ret = send_cmd(adrive_gf, cmd);
    যদি (ret)
        যাও fail;
    ret = ata_wait_তথ্য(iobase1);
    যদি (ret)
        যাও fail;
    ret = ata_pio_transfer(op, iswrite, DISK_SECTOR_আকার);

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    ফেরত ret;
}

// Transfer data to harddrive using DMA protocol.
স্থির পূর্ণসংখ্যা
ata_dma_cmd_তথ্য(কাঠামো disk_op_s *op, কাঠামো ata_pio_command *cmd)
{
    যদি (! CONFIG_ATA_DMA)
        ফেরত -1;
    কাঠামো atadrive_s *adrive_gf = container_of(
        op->drive_fl, কাঠামো atadrive_s, drive);
    পূর্ণসংখ্যা ret = send_cmd(adrive_gf, cmd);
    যদি (ret)
        ফেরত ret;
    ফেরত ata_dma_transfer(op);
}

// Read/write count blocks from a harddrive.
স্থির পূর্ণসংখ্যা
ata_readwrite(কাঠামো disk_op_s *op, পূর্ণসংখ্যা iswrite)
{
    u64 lba = op->lba;

    পূর্ণসংখ্যা usepio = ata_try_dma(op, iswrite, DISK_SECTOR_আকার);

    কাঠামো ata_pio_command cmd;
    memset(&cmd, 0, আকার(cmd));

    যদি (op->গুনতি >= (1<<8) || lba + op->গুনতি >= (1<<28)) {
        cmd.sector_count2 = op->গুনতি >> 8;
        cmd.lba_low2 = lba >> 24;
        cmd.lba_mid2 = lba >> 32;
        cmd.lba_high2 = lba >> 40;
        lba &= 0xffffff;

        যদি (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS_EXT
                           : ATA_CMD_READ_SECTORS_EXT);
        অন্যথা
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                           : ATA_CMD_READ_DMA_EXT);
    } অন্যথা {
        যদি (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS
                           : ATA_CMD_READ_SECTORS);
        অন্যথা
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA
                           : ATA_CMD_READ_DMA);
    }

    cmd.sector_গুনতি = op->গুনতি;
    cmd.lba_নিচের = lba;
    cmd.lba_mid = lba >> 8;
    cmd.lba_উপরের = lba >> 16;
    cmd.device = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;

    পূর্ণসংখ্যা ret;
    যদি (usepio)
        ret = ata_pio_cmd_তথ্য(op, iswrite, &cmd);
    অন্যথা
        ret = ata_dma_cmd_তথ্য(op, &cmd);
    যদি (ret)
        ফেরত DISK_RET_EBADTRACK;
    ফেরত DISK_RET_SUCCESS;
}

// 16bit command demuxer for ATA harddrives.
পূর্ণসংখ্যা
ata_process_op(কাঠামো disk_op_s *op)
{
    যদি (!CONFIG_ATA)
        ফেরত 0;

    কাঠামো atadrive_s *adrive_gf = container_of(
        op->drive_fl, কাঠামো atadrive_s, drive);
    নির্বাচন (op->command) {
    ক্ষেত্র CMD_READ:
        ফেরত ata_readwrite(op, 0);
    ক্ষেত্র CMD_WRITE:
        ফেরত ata_readwrite(op, 1);
    ক্ষেত্র CMD_RESET:
        ata_reset(adrive_gf);
        ফেরত DISK_RET_SUCCESS;
    ক্ষেত্র CMD_ISREADY:
        ফেরত isready(adrive_gf);
    পূর্বনির্ধারিত:
        ফেরত default_process_op(op);
    }
}


/****************************************************************
 * ATAPI functions
 ****************************************************************/

#define CDROM_CDB_আকার 12

// Low-level atapi command transmit function.
পূর্ণসংখ্যা
ata_atapi_process_op(কাঠামো disk_op_s *op)
{
    যদি (! CONFIG_ATA)
        ফেরত 0;

    যদি (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        ফেরত DISK_RET_EWRITEPROTECT;
    u8 cdbcmd[CDROM_CDB_আকার];
    পূর্ণসংখ্যা blocksize = scsi_fill_cmd(op, cdbcmd, আকার(cdbcmd));
    যদি (blocksize < 0)
        ফেরত default_process_op(op);

    কাঠামো atadrive_s *adrive_gf = container_of(
        op->drive_fl, কাঠামো atadrive_s, drive);
    কাঠামো ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    কাঠামো ata_pio_command cmd;
    memset(&cmd, 0, আকার(cmd));
    cmd.lba_mid = blocksize;
    cmd.lba_উপরের = blocksize >> 8;
    cmd.command = ATA_CMD_PACKET;

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    পূর্ণসংখ্যা ret = send_cmd(adrive_gf, &cmd);
    যদি (ret)
        যাও fail;
    ret = ata_wait_তথ্য(iobase1);
    যদি (ret)
        যাও fail;

    // Send command to device
    outsw_fl(iobase1, MAKE_FLATPTR(GET_SEG(SS), cdbcmd), CDROM_CDB_আকার / 2);

    পূর্ণসংখ্যা status = pause_await_not_bsy(iobase1, iobase2);
    যদি (status < 0) {
        ret = status;
        যাও fail;
    }

    যদি (status & ATA_CB_STAT_ERR) {
        u8 err = inb(iobase1 + ATA_CB_ERR);
        // skip "Not Ready"
        যদি (err != 0x20)
            dprintf(6, "send_atapi_cmd : read error (status=%02x err=%02x)\n"
                    , status, err);
        ret = -2;
        যাও fail;
    }
    যদি (blocksize) {
        যদি (!(status & ATA_CB_STAT_DRQ)) {
            dprintf(6, "send_atapi_cmd : DRQ not set (status %02x)\n", status);
            ret = -3;
            যাও fail;
        }

        ret = ata_pio_transfer(op, 0, blocksize);
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    যদি (ret)
        ফেরত DISK_RET_EBADTRACK;
    ফেরত DISK_RET_SUCCESS;
}


/****************************************************************
 * ATA detect and init
 ****************************************************************/

// Send an identify device or identify device packet command.
স্থির পূর্ণসংখ্যা
send_ata_identity(কাঠামো atadrive_s *adrive, u16 *বাফার, পূর্ণসংখ্যা command)
{
    memset(বাফার, 0, DISK_SECTOR_আকার);

    কাঠামো disk_op_s dop;
    memset(&dop, 0, আকার(dop));
    dop.drive_fl = &adrive->drive;
    dop.গুনতি = 1;
    dop.lba = 1;
    dop.buf_fl = MAKE_FLATPTR(GET_SEG(SS), বাফার);

    কাঠামো ata_pio_command cmd;
    memset(&cmd, 0, আকার(cmd));
    cmd.command = command;

    ফেরত ata_pio_cmd_তথ্য(&dop, 0, &cmd);
}

// Extract the ATA/ATAPI version info.
পূর্ণসংখ্যা
ata_extract_version(u16 *বাফার)
{
    // Extract ATA/ATAPI version.
    u16 ataversion = বাফার[80];
    u8 version;
    জন্য (version=15; version>0; version--)
        যদি (ataversion & (1<<version))
            বিরতি;
    ফেরত version;
}

#define MAXMODEL 40

// Extract the ATA/ATAPI model info.
অক্ষর *
ata_extract_model(অক্ষর *model, u32 আকার_2, u16 *বাফার)
{
    // Read model name
    পূর্ণসংখ্যা i;
    জন্য (i=0; i<আকার_2/2; i++)
        *(u16*)&model[i*2] = be16_to_cpu(বাফার[27+i]);
    model[আকার_2] = 0x00;
    nullTrailingSpace(model);
    ফেরত model;
}

// Common init code between ata and atapi
স্থির কাঠামো atadrive_s *
init_atadrive(কাঠামো atadrive_s *dummy, u16 *বাফার)
{
    কাঠামো atadrive_s *adrive = malloc_fseg(আকার(*adrive));
    যদি (!adrive) {
        warn_noalloc();
        ফেরত ফাঁকা;
    }
    memset(adrive, 0, আকার(*adrive));
    adrive->chan_gf = dummy->chan_gf;
    adrive->slave = dummy->slave;
    adrive->drive.cntl_id = adrive->chan_gf->ataid * 2 + dummy->slave;
    adrive->drive.removable = (বাফার[0] & 0x80) ? 1 : 0;
    ফেরত adrive;
}

// Detect if the given drive is an atapi - initialize it if so.
স্থির কাঠামো atadrive_s *
init_drive_atapi(কাঠামো atadrive_s *dummy, u16 *বাফার)
{
    // Send an IDENTIFY_DEVICE_PACKET command to device
    পূর্ণসংখ্যা ret = send_ata_identity(dummy, বাফার, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    যদি (ret)
        ফেরত ফাঁকা;

    // Success - setup as ATAPI.
    কাঠামো atadrive_s *adrive = init_atadrive(dummy, বাফার);
    যদি (!adrive)
        ফেরত ফাঁকা;
    adrive->drive.type = DTYPE_ATA_ATAPI;
    adrive->drive.blksize = CDROM_SECTOR_আকার;
    adrive->drive.sectors = (u64)-1;
    u8 iscd = ((বাফার[0] >> 8) & 0x1f) == 0x05;
    অক্ষর model[MAXMODEL+1];
    অক্ষর *desc = znprintf(MAXDESCSIZE
                          , "DVD/CD [ata%d-%d: %s ATAPI-%d %s]"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, বাফার)
                          , ata_extract_version(বাফার)
                          , (iscd ? "DVD/CD" : "Device"));
    dprintf(1, "%s\n", desc);

    // fill cdidmap
    যদি (iscd) {
        পূর্ণসংখ্যা prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                            adrive->chan_gf->chanid,
                                            adrive->slave);
        boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                                  adrive->chan_gf->chanid,
                                  adrive->slave,
                                  &(adrive->drive.lchs));
        boot_add_cd(&adrive->drive, desc, prio);
    }

    ফেরত adrive;
}

// Detect if the given drive is a regular ata drive - initialize it if so.
স্থির কাঠামো atadrive_s *
init_drive_ata(কাঠামো atadrive_s *dummy, u16 *বাফার)
{
    // Send an IDENTIFY_DEVICE command to device
    পূর্ণসংখ্যা ret = send_ata_identity(dummy, বাফার, ATA_CMD_IDENTIFY_DEVICE);
    যদি (ret)
        ফেরত ফাঁকা;

    // Success - setup as ATA.
    কাঠামো atadrive_s *adrive = init_atadrive(dummy, বাফার);
    যদি (!adrive)
        ফেরত ফাঁকা;
    adrive->drive.type = DTYPE_ATA;
    adrive->drive.blksize = DISK_SECTOR_আকার;

    adrive->drive.pchs.cylinder = বাফার[1];
    adrive->drive.pchs.শীর্ষ = বাফার[3];
    adrive->drive.pchs.sector = বাফার[6];

    u64 sectors;
    যদি (বাফার[83] & (1 << 10)) // word 83 - lba48 support
        sectors = *(u64*)&বাফার[100]; // word 100-103
    অন্যথা
        sectors = *(u32*)&বাফার[60]; // word 60 and word 61
    adrive->drive.sectors = sectors;
    u64 adjsize = sectors >> 11;
    অক্ষর adjprefix = 'M';
    যদি (adjsize >= (1 << 16)) {
        adjsize >>= 10;
        adjprefix = 'G';
    }
    অক্ষর model[MAXMODEL+1];
    অক্ষর *desc = znprintf(MAXDESCSIZE
                          , "ata%d-%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, বাফার)
                          , ata_extract_version(বাফার)
                          , (u32)adjsize, adjprefix);
    dprintf(1, "%s\n", desc);

    পূর্ণসংখ্যা prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                        adrive->chan_gf->chanid,
                                        adrive->slave);
    boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                              adrive->chan_gf->chanid,
                              adrive->slave,
                              &(adrive->drive.lchs));
    // Register with bcv system.
    boot_add_hd(&adrive->drive, desc, prio);

    ফেরত adrive;
}

স্থির u32 SpinupEnd;

// Wait for non-busy status and check for "floating bus" condition.
স্থির পূর্ণসংখ্যা
powerup_await_non_bsy(u16 base)
{
    u8 orstatus = 0;
    u8 status;
    জন্য (;;) {
        status = inb(base+ATA_CB_STAT);
        যদি (!(status & ATA_CB_STAT_BSY))
            বিরতি;
        orstatus |= status;
        যদি (orstatus == 0xff) {
            dprintf(4, "powerup IDE floating\n");
            ফেরত orstatus;
        }
        যদি (timer_check(SpinupEnd)) {
            warn_timeout();
            ফেরত -1;
        }
        yield();
    }
    dprintf(6, "powerup iobase=%x st=%x\n", base, status);
    ফেরত status;
}

// Detect any drives attached to a given controller.
স্থির শূন্য
ata_detect(শূন্য *তথ্য)
{
    কাঠামো ata_channel_s *chan_gf = তথ্য;
    কাঠামো atadrive_s dummy;
    memset(&dummy, 0, আকার(dummy));
    dummy.chan_gf = chan_gf;
    // Device detection
    পূর্ণসংখ্যা didreset = 0;
    u8 slave;
    জন্য (slave=0; slave<=1; slave++) {
        // Wait for not-bsy.
        u16 iobase1 = chan_gf->iobase1;
        পূর্ণসংখ্যা status = powerup_await_non_bsy(iobase1);
        যদি (status < 0)
            চালু;
        u8 newdh = slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0;
        outb(newdh, iobase1+ATA_CB_DH);
        ndelay(400);
        status = powerup_await_non_bsy(iobase1);
        যদি (status < 0)
            চালু;

        // Check if ioport registers look valid.
        outb(newdh, iobase1+ATA_CB_DH);
        u8 dh = inb(iobase1+ATA_CB_DH);
        outb(0x55, iobase1+ATA_CB_SC);
        outb(0xaa, iobase1+ATA_CB_SN);
        u8 sc = inb(iobase1+ATA_CB_SC);
        u8 sn = inb(iobase1+ATA_CB_SN);
        dprintf(6, "ata_detect ata%d-%d: sc=%x sn=%x dh=%x\n"
                , chan_gf->ataid, slave, sc, sn, dh);
        যদি (sc != 0x55 || sn != 0xaa || dh != newdh)
            চালু;

        // Prepare new drive.
        dummy.slave = slave;

        // reset the channel
        যদি (!didreset) {
            ata_reset(&dummy);
            didreset = 1;
        }

        // check for ATAPI
        u16 বাফার[256];
        কাঠামো atadrive_s *adrive = init_drive_atapi(&dummy, বাফার);
        যদি (!adrive) {
            // Didn't find an ATAPI drive - look for ATA drive.
            u8 st = inb(iobase1+ATA_CB_STAT);
            যদি (!st)
                // Status not set - can't be a valid drive.
                চালু;

            // Wait for RDY.
            পূর্ণসংখ্যা ret = await_rdy(iobase1);
            যদি (ret < 0)
                চালু;

            // check for ATA.
            adrive = init_drive_ata(&dummy, বাফার);
            যদি (!adrive)
                // No ATA drive found
                চালু;
        }

        u16 resetresult = বাফার[93];
        dprintf(6, "ata_detect resetresult=%04x\n", resetresult);
        যদি (!slave && (resetresult & 0xdf61) == 0x4041)
            // resetresult looks valid and device 0 is responding to
            // device 1 requests - device 1 must not be present - skip
            // detection.
            বিরতি;
    }
}

// Initialize an ata controller and detect its drives.
স্থির শূন্য
init_controller(কাঠামো pci_device *pci, পূর্ণসংখ্যা chanid, পূর্ণসংখ্যা irq
                , u32 port1, u32 port2, u32 master)
{
    স্থির পূর্ণসংখ্যা ataid = 0;
    কাঠামো ata_channel_s *chan_gf = malloc_fseg(আকার(*chan_gf));
    যদি (!chan_gf) {
        warn_noalloc();
        ফেরত;
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
স্থির শূন্য
init_pciata(কাঠামো pci_device *pci, u8 prog_if)
{
    u8 pciirq = pci_config_readb(pci->bdf, PCI_INTERRUPT_সারি);
    পূর্ণসংখ্যা master = 0;
    যদি (CONFIG_ATA_DMA && prog_if & 0x80) {
        // Check for bus-mastering.
        u32 bar = pci_config_readl(pci->bdf, PCI_BASE_ADDRESS_4);
        যদি (bar & PCI_BASE_ADDRESS_SPACE_IO) {
            master = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
            pci_enable_busmaster(pci);
        }
    }

    u32 port1, port2, irq;
    যদি (prog_if & 1) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_1);
        যদি (!port1 || !port2)
            ফেরত;
        irq = pciirq;
    } অন্যথা {
        port1 = PORT_ATA1_CMD_BASE;
        port2 = PORT_ATA1_CTRL_BASE;
        irq = IRQ_ATA1;
    }
    init_controller(pci, 0, irq, port1, port2, master);

    যদি (prog_if & 4) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_2);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_3);
        যদি (!port1 || !port2)
            ফেরত;
        irq = pciirq;
    } অন্যথা {
        port1 = PORT_ATA2_CMD_BASE;
        port2 = PORT_ATA2_CTRL_BASE;
        irq = IRQ_ATA2;
    }
    init_controller(pci, 1, irq, port1, port2, master ? master + 8 : 0);
}

স্থির শূন্য
found_genericata(কাঠামো pci_device *pci, শূন্য *arg)
{
    init_pciata(pci, pci->prog_if);
}

স্থির শূন্য
found_compatibleahci(কাঠামো pci_device *pci, শূন্য *arg)
{
    যদি (CONFIG_AHCI)
        // Already handled directly via native ahci interface.
        ফেরত;
    init_pciata(pci, 0x8f);
}

স্থির ধ্রুবক কাঠামো pci_device_id pci_ata_tbl[] = {
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE
                     , found_genericata),
    PCI_DEVICE(PCI_VENDOR_ID_ATI, 0x4391, found_compatibleahci),
    PCI_DEVICE_END,
};

// Locate and init ata controllers.
স্থির শূন্য
ata_scan(শূন্য)
{
    যদি (CONFIG_QEMU && hlist_empty(&PCIDevices)) {
        // No PCI devices found - probably a QEMU "-M isapc" machine.
        // Try using ISA ports for ATA controllers.
        init_controller(ফাঁকা, 0, IRQ_ATA1
                        , PORT_ATA1_CMD_BASE, PORT_ATA1_CTRL_BASE, 0);
        init_controller(ফাঁকা, 1, IRQ_ATA2
                        , PORT_ATA2_CMD_BASE, PORT_ATA2_CTRL_BASE, 0);
        ফেরত;
    }

    // Scan PCI bus for ATA adapters
    কাঠামো pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_ata_tbl, pci, ফাঁকা);
    }
}

শূন্য
ata_setup(শূন্য)
{
    ASSERT32FLAT();
    যদি (!CONFIG_ATA)
        ফেরত;

    dprintf(3, "init hard drives\n");

    SpinupEnd = timer_calc(IDE_TIMEOUT);
    ata_scan();

    SET_BDA(disk_control_byte, 0xc0);

    enable_hwirq(14, FUNC16(entry_76));
}

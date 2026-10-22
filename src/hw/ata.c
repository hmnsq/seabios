// Low level ATA disk access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "ata.h" // ATA_CB_STAT
#شامل "biosvar.h" // GET_GLOBALFLAT
#شامل "block.h" // struct drive_s
#شامل "blockcmd.h" // CDB_CMD_READ_10
#شامل "byteorder.h" // be16_to_cpu
#شامل "malloc.h" // malloc_fseg
#شامل "output.h" // dprintf
#شامل "pci.h" // pci_config_readb
#شامل "pcidevice.h" // foreachpci
#شامل "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#شامل "pci_regs.h" // PCI_INTERRUPT_LINE
#شامل "pic.h" // enable_hwirq
#شامل "stacks.h" // yield
#شامل "std/disk.h" // DISK_RET_SUCCESS
#شامل "string.h" // memset
#شامل "util.h" // timer_calc
#شامل "x86.h" // inb

#define IDE_TIMEOUT 32000 //32 seconds max for IDE ops


/****************************************************************
 * Helper functions
 ****************************************************************/

// Wait for the specified ide state
ایستا درونخطی صحیح
await_ide(u8 mask, u8 flags, u16 base, u16 timeout)
{
    u32 end = timer_calc(timeout);
    برای (;;) {
        u8 status = inb(base+ATA_CB_STAT);
        اگر ((status & mask) == flags)
            بازگشت status;
        اگر (timer_check(end)) {
            warn_timeout();
            بازگشت -1;
        }
        yield();
    }
}

// Wait for the device to be not-busy.
ایستا صحیح
await_not_bsy(u16 base)
{
    بازگشت await_ide(ATA_CB_STAT_BSY, 0, base, IDE_TIMEOUT);
}

// Wait for the device to be ready.
ایستا صحیح
await_rdy(u16 base)
{
    بازگشت await_ide(ATA_CB_STAT_RDY, ATA_CB_STAT_RDY, base, IDE_TIMEOUT);
}

// Wait for ide state - pauses for one ata cycle first.
ایستا درونخطی صحیح
pause_await_not_bsy(u16 iobase1, u16 iobase2)
{
    // Wait one PIO transfer cycle.
    inb(iobase2 + ATA_CB_ASTAT);

    بازگشت await_not_bsy(iobase1);
}

// Wait for ide state - pause for 400ns first.
ایستا درونخطی صحیح
ndelay_await_not_bsy(u16 iobase1)
{
    ndelay(400);
    بازگشت await_not_bsy(iobase1);
}

// Reset a drive
ایستا تهی
ata_reset(ساختار atadrive_s *adrive_gf)
{
    ساختار ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
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
    صحیح status = await_not_bsy(iobase1);
    اگر (status < 0)
        برو done;
    اگر (slave) {
        // Change device.
        u32 end = timer_calc(IDE_TIMEOUT);
        برای (;;) {
            outb(ATA_CB_DH_DEV1, iobase1 + ATA_CB_DH);
            status = ndelay_await_not_bsy(iobase1);
            اگر (status < 0)
                برو done;
            اگر (inb(iobase1 + ATA_CB_DH) == ATA_CB_DH_DEV1)
                بشکن;
            // Change drive request failed to take effect - retry.
            اگر (timer_check(end)) {
                warn_timeout();
                برو done;
            }
        }
    } وگرنه {
        // QEMU doesn't reset dh on reset, so set it explicitly.
        outb(ATA_CB_DH_DEV0, iobase1 + ATA_CB_DH);
    }

    // On a user-reset request, wait for RDY if it is an ATA device.
    u8 type=GET_GLOBALFLAT(adrive_gf->drive.type);
    اگر (type == DTYPE_ATA)
        status = await_rdy(iobase1);

done:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    dprintf(6, "ata_reset exit status=%x\n", status);
}

// Check for drive RDY for 16bit interface command.
ایستا صحیح
isready(ساختار atadrive_s *adrive_gf)
{
    // Read the status from controller
    ساختار ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u8 status = inb(iobase1 + ATA_CB_STAT);
    اگر ((status & (ATA_CB_STAT_BSY|ATA_CB_STAT_RDY)) == ATA_CB_STAT_RDY)
        بازگشت DISK_RET_SUCCESS;
    بازگشت DISK_RET_ENOTREADY;
}


/****************************************************************
 * ATA send command
 ****************************************************************/

ساختار ata_pio_command {
    u8 feature;
    u8 sector_شمار;
    u8 lba_کم;
    u8 lba_mid;
    u8 lba_بالا;
    u8 device;
    u8 command;

    u8 feature2;
    u8 sector_count2;
    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
};

// Send an ata command to the drive.
ایستا صحیح
send_cmd(ساختار atadrive_s *adrive_gf, ساختار ata_pio_command *cmd)
{
    ساختار ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);

    // Select device
    صحیح status = await_not_bsy(iobase1);
    اگر (status < 0)
        بازگشت status;
    u8 newdh = ((cmd->device & ~ATA_CB_DH_DEV1)
                | (slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0));
    u8 olddh = inb(iobase1 + ATA_CB_DH);
    outb(newdh, iobase1 + ATA_CB_DH);
    اگر ((olddh ^ newdh) & (1<<4)) {
        // Was a device change - wait for device to become not busy.
        status = ndelay_await_not_bsy(iobase1);
        اگر (status < 0)
            بازگشت status;
    }

    // Check for ATA_CMD_(READ|WRITE)_(SECTORS|DMA)_EXT commands.
    اگر ((cmd->command & ~0x11) == ATA_CMD_READ_SECTORS_EXT) {
        outb(cmd->feature2, iobase1 + ATA_CB_FR);
        outb(cmd->sector_count2, iobase1 + ATA_CB_SC);
        outb(cmd->lba_low2, iobase1 + ATA_CB_SN);
        outb(cmd->lba_mid2, iobase1 + ATA_CB_CL);
        outb(cmd->lba_high2, iobase1 + ATA_CB_CH);
    }
    outb(cmd->feature, iobase1 + ATA_CB_FR);
    outb(cmd->sector_شمار, iobase1 + ATA_CB_SC);
    outb(cmd->lba_کم, iobase1 + ATA_CB_SN);
    outb(cmd->lba_mid, iobase1 + ATA_CB_CL);
    outb(cmd->lba_بالا, iobase1 + ATA_CB_CH);
    outb(cmd->command, iobase1 + ATA_CB_CMD);

    بازگشت 0;
}

// Wait for data after calling 'send_cmd'.
ایستا صحیح
ata_wait_data(u16 iobase1)
{
    صحیح status = ndelay_await_not_bsy(iobase1);
    اگر (status < 0)
        بازگشت status;

    اگر (status & ATA_CB_STAT_ERR) {
        dprintf(6, "send_cmd : read error (status=%02x err=%02x)\n"
                , status, inb(iobase1 + ATA_CB_ERR));
        بازگشت -4;
    }
    اگر (!(status & ATA_CB_STAT_DRQ)) {
        dprintf(6, "send_cmd : DRQ not set (status %02x)\n", status);
        بازگشت -5;
    }

    بازگشت 0;
}

// Send an ata command that does not transfer any further data.
صحیح
ata_cmd_nondata(ساختار atadrive_s *adrive_gf, ساختار ata_pio_command *cmd)
{
    ساختار ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    صحیح ret = send_cmd(adrive_gf, cmd);
    اگر (ret)
        برو fail;
    ret = ndelay_await_not_bsy(iobase1);
    اگر (ret < 0)
        برو fail;

    اگر (ret & ATA_CB_STAT_ERR) {
        dprintf(6, "nondata cmd : read error (status=%02x err=%02x)\n"
                , ret, inb(iobase1 + ATA_CB_ERR));
        ret = -4;
        برو fail;
    }
    اگر (ret & ATA_CB_STAT_DRQ) {
        dprintf(6, "nondata cmd : DRQ set (status %02x)\n", ret);
        ret = -5;
        برو fail;
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    بازگشت ret;
}


/****************************************************************
 * ATA PIO transfers
 ****************************************************************/

// Transfer 'op->count' blocks (of 'blocksize' bytes) to/from drive
// 'op->drive_fl'.
ایستا صحیح
ata_pio_transfer(ساختار disk_op_s *op, صحیح iswrite, صحیح blocksize)
{
    dprintf(16, "ata_pio_transfer id=%p write=%d count=%d bs=%d buf=%p\n"
            , op->drive_fl, iswrite, op->شمار, blocksize, op->buf_fl);

    ساختار atadrive_s *adrive_gf = container_of(
        op->drive_fl, ساختار atadrive_s, drive);
    ساختار ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    صحیح شمار = op->شمار;
    تهی *buf_fl = op->buf_fl;
    صحیح status;
    برای (;;) {
        اگر (iswrite) {
            // Write data to controller
            dprintf(16, "Write sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            اگر (CONFIG_ATA_PIO32)
                outsl_fl(iobase1, buf_fl, blocksize / 4);
            وگرنه
                outsw_fl(iobase1, buf_fl, blocksize / 2);
        } وگرنه {
            // Read data from controller
            dprintf(16, "Read sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            اگر (CONFIG_ATA_PIO32)
                insl_fl(iobase1, buf_fl, blocksize / 4);
            وگرنه
                insw_fl(iobase1, buf_fl, blocksize / 2);
        }
        buf_fl += blocksize;

        status = pause_await_not_bsy(iobase1, iobase2);
        اگر (status < 0) {
            // Error
            op->شمار -= شمار;
            بازگشت status;
        }

        شمار--;
        اگر (!شمار)
            بشکن;
        status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ | ATA_CB_STAT_ERR);
        اگر (status != ATA_CB_STAT_DRQ) {
            dprintf(6, "ata_pio_transfer : more sectors left (status %02x)\n"
                    , status);
            op->شمار -= شمار;
            بازگشت -6;
        }
    }

    status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
               | ATA_CB_STAT_ERR);
    اگر (!iswrite)
        status &= ~ATA_CB_STAT_DF;
    اگر (status != 0) {
        dprintf(6, "ata_pio_transfer : no sectors left (status %02x)\n", status);
        بازگشت -7;
    }

    بازگشت 0;
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

ساختار sff_dma_prd {
    u32 buf_fl;
    u32 شمار;
};

// Check if DMA available and setup transfer if so.
ایستا صحیح
ata_try_dma(ساختار disk_op_s *op, صحیح iswrite, صحیح blocksize)
{
    ASSERT16();
    اگر (! CONFIG_ATA_DMA)
        بازگشت -1;
    u32 dest = (u32)op->buf_fl;
    اگر (dest & 1)
        // Need minimum alignment of 1.
        بازگشت -1;
    ساختار atadrive_s *adrive_gf = container_of(
        op->drive_fl, ساختار atadrive_s, drive);
    ساختار ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);
    اگر (! iomaster)
        بازگشت -1;
    u32 bytes = op->شمار * blocksize;
    اگر (! bytes)
        بازگشت -1;

    // Build PRD dma structure.
    ساختار sff_dma_prd *dma = MAKE_FLATPTR(SEG_کم, Extra_پشته);
    ساختار sff_dma_prd *origdma = dma;
    وقتی (bytes) {
        اگر (dma >= &origdma[16])
            // Too many descriptors..
            بازگشت -1;
        u32 شمار = bytes;
        u32 max = 0x10000 - (dest & 0xffff);
        اگر (شمار > max)
            شمار = max;

        SET_LOWFLAT(dma->buf_fl, dest);
        bytes -= شمار;
        اگر (!bytes)
            // Last descriptor.
            شمار |= 1<<31;
        dprintf(16, "dma@%p: %08x %08x\n", dma, dest, شمار);
        dest += شمار;
        SET_LOWFLAT(dma->شمار, شمار);
        dma++;
    }

    // Program bus-master controller.
    outl((u32)origdma, iomaster + BM_TABLE);
    u8 oldcmd = inb(iomaster + BM_CMD) & ~(BM_CMD_MEMWRITE|BM_CMD_START);
    outb(oldcmd | (iswrite ? 0x00 : BM_CMD_MEMWRITE), iomaster + BM_CMD);
    outb(BM_STATUS_ERROR|BM_STATUS_IRQ, iomaster + BM_STATUS);

    بازگشت 0;
}

// Transfer data using DMA.
ایستا صحیح
ata_dma_transfer(ساختار disk_op_s *op)
{
    اگر (! CONFIG_ATA_DMA)
        بازگشت -1;
    dprintf(16, "ata_dma_transfer id=%p buf=%p\n", op->drive_fl, op->buf_fl);

    ساختار atadrive_s *adrive_gf = container_of(
        op->drive_fl, ساختار atadrive_s, drive);
    ساختار ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);

    // Start bus-master controller.
    u8 oldcmd = inb(iomaster + BM_CMD);
    outb(oldcmd | BM_CMD_START, iomaster + BM_CMD);

    u32 end = timer_calc(IDE_TIMEOUT);
    u8 status;
    برای (;;) {
        status = inb(iomaster + BM_STATUS);
        اگر (status & BM_STATUS_IRQ)
            بشکن;
        // Transfer in progress
        اگر (timer_check(end)) {
            // Timeout.
            warn_timeout();
            بشکن;
        }
        yield();
    }
    outb(oldcmd & ~BM_CMD_START, iomaster + BM_CMD);

    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    صحیح idestatus = pause_await_not_bsy(iobase1, iobase2);

    اگر ((status & (BM_STATUS_IRQ|BM_STATUS_ACTIVE)) == BM_STATUS_IRQ
        && idestatus >= 0x00
        && (idestatus & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
                         | ATA_CB_STAT_ERR)) == 0x00)
        // Success.
        بازگشت 0;

    dprintf(6, "IDE DMA error (dma=%x ide=%x/%x/%x)\n", status, idestatus
            , inb(iobase2 + ATA_CB_ASTAT), inb(iobase1 + ATA_CB_ERR));
    بازگشت -1;
}


/****************************************************************
 * ATA hard drive functions
 ****************************************************************/

// Transfer data to harddrive using PIO protocol.
ایستا صحیح
ata_pio_cmd_data(ساختار disk_op_s *op, صحیح iswrite, ساختار ata_pio_command *cmd)
{
    ساختار atadrive_s *adrive_gf = container_of(
        op->drive_fl, ساختار atadrive_s, drive);
    ساختار ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    صحیح ret = send_cmd(adrive_gf, cmd);
    اگر (ret)
        برو fail;
    ret = ata_wait_data(iobase1);
    اگر (ret)
        برو fail;
    ret = ata_pio_transfer(op, iswrite, DISK_SECTOR_SIZE);

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    بازگشت ret;
}

// Transfer data to harddrive using DMA protocol.
ایستا صحیح
ata_dma_cmd_data(ساختار disk_op_s *op, ساختار ata_pio_command *cmd)
{
    اگر (! CONFIG_ATA_DMA)
        بازگشت -1;
    ساختار atadrive_s *adrive_gf = container_of(
        op->drive_fl, ساختار atadrive_s, drive);
    صحیح ret = send_cmd(adrive_gf, cmd);
    اگر (ret)
        بازگشت ret;
    بازگشت ata_dma_transfer(op);
}

// Read/write count blocks from a harddrive.
ایستا صحیح
ata_readwrite(ساختار disk_op_s *op, صحیح iswrite)
{
    u64 lba = op->lba;

    صحیح usepio = ata_try_dma(op, iswrite, DISK_SECTOR_SIZE);

    ساختار ata_pio_command cmd;
    memset(&cmd, 0, اندازه(cmd));

    اگر (op->شمار >= (1<<8) || lba + op->شمار >= (1<<28)) {
        cmd.sector_count2 = op->شمار >> 8;
        cmd.lba_low2 = lba >> 24;
        cmd.lba_mid2 = lba >> 32;
        cmd.lba_high2 = lba >> 40;
        lba &= 0xffffff;

        اگر (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS_EXT
                           : ATA_CMD_READ_SECTORS_EXT);
        وگرنه
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                           : ATA_CMD_READ_DMA_EXT);
    } وگرنه {
        اگر (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS
                           : ATA_CMD_READ_SECTORS);
        وگرنه
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA
                           : ATA_CMD_READ_DMA);
    }

    cmd.sector_شمار = op->شمار;
    cmd.lba_کم = lba;
    cmd.lba_mid = lba >> 8;
    cmd.lba_بالا = lba >> 16;
    cmd.device = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;

    صحیح ret;
    اگر (usepio)
        ret = ata_pio_cmd_data(op, iswrite, &cmd);
    وگرنه
        ret = ata_dma_cmd_data(op, &cmd);
    اگر (ret)
        بازگشت DISK_RET_EBADTRACK;
    بازگشت DISK_RET_SUCCESS;
}

// 16bit command demuxer for ATA harddrives.
صحیح
ata_process_op(ساختار disk_op_s *op)
{
    اگر (!CONFIG_ATA)
        بازگشت 0;

    ساختار atadrive_s *adrive_gf = container_of(
        op->drive_fl, ساختار atadrive_s, drive);
    انتخاب (op->command) {
    حالت CMD_READ:
        بازگشت ata_readwrite(op, 0);
    حالت CMD_WRITE:
        بازگشت ata_readwrite(op, 1);
    حالت CMD_RESET:
        ata_reset(adrive_gf);
        بازگشت DISK_RET_SUCCESS;
    حالت CMD_ISREADY:
        بازگشت isready(adrive_gf);
    پیشفرض:
        بازگشت default_process_op(op);
    }
}


/****************************************************************
 * ATAPI functions
 ****************************************************************/

#define CDROM_CDB_SIZE 12

// Low-level atapi command transmit function.
صحیح
ata_atapi_process_op(ساختار disk_op_s *op)
{
    اگر (! CONFIG_ATA)
        بازگشت 0;

    اگر (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        بازگشت DISK_RET_EWRITEPROTECT;
    u8 cdbcmd[CDROM_CDB_SIZE];
    صحیح blocksize = scsi_fill_cmd(op, cdbcmd, اندازه(cdbcmd));
    اگر (blocksize < 0)
        بازگشت default_process_op(op);

    ساختار atadrive_s *adrive_gf = container_of(
        op->drive_fl, ساختار atadrive_s, drive);
    ساختار ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    ساختار ata_pio_command cmd;
    memset(&cmd, 0, اندازه(cmd));
    cmd.lba_mid = blocksize;
    cmd.lba_بالا = blocksize >> 8;
    cmd.command = ATA_CMD_PACKET;

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    صحیح ret = send_cmd(adrive_gf, &cmd);
    اگر (ret)
        برو fail;
    ret = ata_wait_data(iobase1);
    اگر (ret)
        برو fail;

    // Send command to device
    outsw_fl(iobase1, MAKE_FLATPTR(GET_SEG(SS), cdbcmd), CDROM_CDB_SIZE / 2);

    صحیح status = pause_await_not_bsy(iobase1, iobase2);
    اگر (status < 0) {
        ret = status;
        برو fail;
    }

    اگر (status & ATA_CB_STAT_ERR) {
        u8 err = inb(iobase1 + ATA_CB_ERR);
        // skip "Not Ready"
        اگر (err != 0x20)
            dprintf(6, "send_atapi_cmd : read error (status=%02x err=%02x)\n"
                    , status, err);
        ret = -2;
        برو fail;
    }
    اگر (blocksize) {
        اگر (!(status & ATA_CB_STAT_DRQ)) {
            dprintf(6, "send_atapi_cmd : DRQ not set (status %02x)\n", status);
            ret = -3;
            برو fail;
        }

        ret = ata_pio_transfer(op, 0, blocksize);
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    اگر (ret)
        بازگشت DISK_RET_EBADTRACK;
    بازگشت DISK_RET_SUCCESS;
}


/****************************************************************
 * ATA detect and init
 ****************************************************************/

// Send an identify device or identify device packet command.
ایستا صحیح
send_ata_identity(ساختار atadrive_s *adrive, u16 *buffer, صحیح command)
{
    memset(buffer, 0, DISK_SECTOR_SIZE);

    ساختار disk_op_s dop;
    memset(&dop, 0, اندازه(dop));
    dop.drive_fl = &adrive->drive;
    dop.شمار = 1;
    dop.lba = 1;
    dop.buf_fl = MAKE_FLATPTR(GET_SEG(SS), buffer);

    ساختار ata_pio_command cmd;
    memset(&cmd, 0, اندازه(cmd));
    cmd.command = command;

    بازگشت ata_pio_cmd_data(&dop, 0, &cmd);
}

// Extract the ATA/ATAPI version info.
صحیح
ata_extract_version(u16 *buffer)
{
    // Extract ATA/ATAPI version.
    u16 ataversion = buffer[80];
    u8 version;
    برای (version=15; version>0; version--)
        اگر (ataversion & (1<<version))
            بشکن;
    بازگشت version;
}

#define MAXMODEL 40

// Extract the ATA/ATAPI model info.
نویسه *
ata_extract_model(نویسه *model, u32 size, u16 *buffer)
{
    // Read model name
    صحیح i;
    برای (i=0; i<size/2; i++)
        *(u16*)&model[i*2] = be16_to_cpu(buffer[27+i]);
    model[size] = 0x00;
    nullTrailingSpace(model);
    بازگشت model;
}

// Common init code between ata and atapi
ایستا ساختار atadrive_s *
init_atadrive(ساختار atadrive_s *dummy, u16 *buffer)
{
    ساختار atadrive_s *adrive = malloc_fseg(اندازه(*adrive));
    اگر (!adrive) {
        warn_noalloc();
        بازگشت پوچ;
    }
    memset(adrive, 0, اندازه(*adrive));
    adrive->chan_gf = dummy->chan_gf;
    adrive->slave = dummy->slave;
    adrive->drive.cntl_id = adrive->chan_gf->ataid * 2 + dummy->slave;
    adrive->drive.removable = (buffer[0] & 0x80) ? 1 : 0;
    بازگشت adrive;
}

// Detect if the given drive is an atapi - initialize it if so.
ایستا ساختار atadrive_s *
init_drive_atapi(ساختار atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE_PACKET command to device
    صحیح ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    اگر (ret)
        بازگشت پوچ;

    // Success - setup as ATAPI.
    ساختار atadrive_s *adrive = init_atadrive(dummy, buffer);
    اگر (!adrive)
        بازگشت پوچ;
    adrive->drive.type = DTYPE_ATA_ATAPI;
    adrive->drive.blksize = CDROM_SECTOR_SIZE;
    adrive->drive.sectors = (u64)-1;
    u8 iscd = ((buffer[0] >> 8) & 0x1f) == 0x05;
    نویسه model[MAXMODEL+1];
    نویسه *desc = znprintf(MAXDESCSIZE
                          , "DVD/CD [ata%d-%d: %s ATAPI-%d %s]"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (iscd ? "DVD/CD" : "Device"));
    dprintf(1, "%s\n", desc);

    // fill cdidmap
    اگر (iscd) {
        صحیح prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                            adrive->chan_gf->chanid,
                                            adrive->slave);
        boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                                  adrive->chan_gf->chanid,
                                  adrive->slave,
                                  &(adrive->drive.lchs));
        boot_add_cd(&adrive->drive, desc, prio);
    }

    بازگشت adrive;
}

// Detect if the given drive is a regular ata drive - initialize it if so.
ایستا ساختار atadrive_s *
init_drive_ata(ساختار atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE command to device
    صحیح ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_DEVICE);
    اگر (ret)
        بازگشت پوچ;

    // Success - setup as ATA.
    ساختار atadrive_s *adrive = init_atadrive(dummy, buffer);
    اگر (!adrive)
        بازگشت پوچ;
    adrive->drive.type = DTYPE_ATA;
    adrive->drive.blksize = DISK_SECTOR_SIZE;

    adrive->drive.pchs.cylinder = buffer[1];
    adrive->drive.pchs.head = buffer[3];
    adrive->drive.pchs.sector = buffer[6];

    u64 sectors;
    اگر (buffer[83] & (1 << 10)) // word 83 - lba48 support
        sectors = *(u64*)&buffer[100]; // word 100-103
    وگرنه
        sectors = *(u32*)&buffer[60]; // word 60 and word 61
    adrive->drive.sectors = sectors;
    u64 adjsize = sectors >> 11;
    نویسه adjprefix = 'M';
    اگر (adjsize >= (1 << 16)) {
        adjsize >>= 10;
        adjprefix = 'G';
    }
    نویسه model[MAXMODEL+1];
    نویسه *desc = znprintf(MAXDESCSIZE
                          , "ata%d-%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (u32)adjsize, adjprefix);
    dprintf(1, "%s\n", desc);

    صحیح prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                        adrive->chan_gf->chanid,
                                        adrive->slave);
    boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                              adrive->chan_gf->chanid,
                              adrive->slave,
                              &(adrive->drive.lchs));
    // Register with bcv system.
    boot_add_hd(&adrive->drive, desc, prio);

    بازگشت adrive;
}

ایستا u32 SpinupEnd;

// Wait for non-busy status and check for "floating bus" condition.
ایستا صحیح
powerup_await_non_bsy(u16 base)
{
    u8 orstatus = 0;
    u8 status;
    برای (;;) {
        status = inb(base+ATA_CB_STAT);
        اگر (!(status & ATA_CB_STAT_BSY))
            بشکن;
        orstatus |= status;
        اگر (orstatus == 0xff) {
            dprintf(4, "powerup IDE floating\n");
            بازگشت orstatus;
        }
        اگر (timer_check(SpinupEnd)) {
            warn_timeout();
            بازگشت -1;
        }
        yield();
    }
    dprintf(6, "powerup iobase=%x st=%x\n", base, status);
    بازگشت status;
}

// Detect any drives attached to a given controller.
ایستا تهی
ata_detect(تهی *data)
{
    ساختار ata_channel_s *chan_gf = data;
    ساختار atadrive_s dummy;
    memset(&dummy, 0, اندازه(dummy));
    dummy.chan_gf = chan_gf;
    // Device detection
    صحیح didreset = 0;
    u8 slave;
    برای (slave=0; slave<=1; slave++) {
        // Wait for not-bsy.
        u16 iobase1 = chan_gf->iobase1;
        صحیح status = powerup_await_non_bsy(iobase1);
        اگر (status < 0)
            ادامه;
        u8 newdh = slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0;
        outb(newdh, iobase1+ATA_CB_DH);
        ndelay(400);
        status = powerup_await_non_bsy(iobase1);
        اگر (status < 0)
            ادامه;

        // Check if ioport registers look valid.
        outb(newdh, iobase1+ATA_CB_DH);
        u8 dh = inb(iobase1+ATA_CB_DH);
        outb(0x55, iobase1+ATA_CB_SC);
        outb(0xaa, iobase1+ATA_CB_SN);
        u8 sc = inb(iobase1+ATA_CB_SC);
        u8 sn = inb(iobase1+ATA_CB_SN);
        dprintf(6, "ata_detect ata%d-%d: sc=%x sn=%x dh=%x\n"
                , chan_gf->ataid, slave, sc, sn, dh);
        اگر (sc != 0x55 || sn != 0xaa || dh != newdh)
            ادامه;

        // Prepare new drive.
        dummy.slave = slave;

        // reset the channel
        اگر (!didreset) {
            ata_reset(&dummy);
            didreset = 1;
        }

        // check for ATAPI
        u16 buffer[256];
        ساختار atadrive_s *adrive = init_drive_atapi(&dummy, buffer);
        اگر (!adrive) {
            // Didn't find an ATAPI drive - look for ATA drive.
            u8 st = inb(iobase1+ATA_CB_STAT);
            اگر (!st)
                // Status not set - can't be a valid drive.
                ادامه;

            // Wait for RDY.
            صحیح ret = await_rdy(iobase1);
            اگر (ret < 0)
                ادامه;

            // check for ATA.
            adrive = init_drive_ata(&dummy, buffer);
            اگر (!adrive)
                // No ATA drive found
                ادامه;
        }

        u16 resetresult = buffer[93];
        dprintf(6, "ata_detect resetresult=%04x\n", resetresult);
        اگر (!slave && (resetresult & 0xdf61) == 0x4041)
            // resetresult looks valid and device 0 is responding to
            // device 1 requests - device 1 must not be present - skip
            // detection.
            بشکن;
    }
}

// Initialize an ata controller and detect its drives.
ایستا تهی
init_controller(ساختار pci_device *pci, صحیح chanid, صحیح irq
                , u32 port1, u32 port2, u32 master)
{
    ایستا صحیح ataid = 0;
    ساختار ata_channel_s *chan_gf = malloc_fseg(اندازه(*chan_gf));
    اگر (!chan_gf) {
        warn_noalloc();
        بازگشت;
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
ایستا تهی
init_pciata(ساختار pci_device *pci, u8 prog_if)
{
    u8 pciirq = pci_config_readb(pci->bdf, PCI_INTERRUPT_سطر);
    صحیح master = 0;
    اگر (CONFIG_ATA_DMA && prog_if & 0x80) {
        // Check for bus-mastering.
        u32 bar = pci_config_readl(pci->bdf, PCI_BASE_ADDRESS_4);
        اگر (bar & PCI_BASE_ADDRESS_SPACE_IO) {
            master = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
            pci_enable_busmaster(pci);
        }
    }

    u32 port1, port2, irq;
    اگر (prog_if & 1) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_1);
        اگر (!port1 || !port2)
            بازگشت;
        irq = pciirq;
    } وگرنه {
        port1 = PORT_ATA1_CMD_BASE;
        port2 = PORT_ATA1_CTRL_BASE;
        irq = IRQ_ATA1;
    }
    init_controller(pci, 0, irq, port1, port2, master);

    اگر (prog_if & 4) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_2);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_3);
        اگر (!port1 || !port2)
            بازگشت;
        irq = pciirq;
    } وگرنه {
        port1 = PORT_ATA2_CMD_BASE;
        port2 = PORT_ATA2_CTRL_BASE;
        irq = IRQ_ATA2;
    }
    init_controller(pci, 1, irq, port1, port2, master ? master + 8 : 0);
}

ایستا تهی
found_genericata(ساختار pci_device *pci, تهی *arg)
{
    init_pciata(pci, pci->prog_if);
}

ایستا تهی
found_compatibleahci(ساختار pci_device *pci, تهی *arg)
{
    اگر (CONFIG_AHCI)
        // Already handled directly via native ahci interface.
        بازگشت;
    init_pciata(pci, 0x8f);
}

ایستا ثابت ساختار pci_device_id pci_ata_tbl[] = {
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE
                     , found_genericata),
    PCI_DEVICE(PCI_VENDOR_ID_ATI, 0x4391, found_compatibleahci),
    PCI_DEVICE_END,
};

// Locate and init ata controllers.
ایستا تهی
ata_scan(تهی)
{
    اگر (CONFIG_QEMU && hlist_empty(&PCIDevices)) {
        // No PCI devices found - probably a QEMU "-M isapc" machine.
        // Try using ISA ports for ATA controllers.
        init_controller(پوچ, 0, IRQ_ATA1
                        , PORT_ATA1_CMD_BASE, PORT_ATA1_CTRL_BASE, 0);
        init_controller(پوچ, 1, IRQ_ATA2
                        , PORT_ATA2_CMD_BASE, PORT_ATA2_CTRL_BASE, 0);
        بازگشت;
    }

    // Scan PCI bus for ATA adapters
    ساختار pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_ata_tbl, pci, پوچ);
    }
}

تهی
ata_setup(تهی)
{
    ASSERT32FLAT();
    اگر (!CONFIG_ATA)
        بازگشت;

    dprintf(3, "init hard drives\n");

    SpinupEnd = timer_calc(IDE_TIMEOUT);
    ata_scan();

    SET_BDA(disk_control_byte, 0xc0);

    enable_hwirq(14, FUNC16(entry_76));
}

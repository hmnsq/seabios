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

#تعریف_کرو IDE_TIMEOUT 32000 //32 seconds max for IDE ops


/****************************************************************
 * Helper functions
 ****************************************************************/

// Wait for the specified ide state
ساکن اندرونی صحیح
await_ide(u8 نقاب, u8 flags, u16 بنیاد, u16 timeout)
{
    u32 انتہا = timer_calc(timeout);
    برائے (;;) {
        u8 حالت_2 = inb(بنیاد+ATA_CB_STAT);
        اگر ((حالت_2 & نقاب) == flags)
            واپس حالت_2;
        اگر (timer_check(انتہا)) {
            warn_timeout();
            واپس -1;
        }
        yield();
    }
}

// Wait for the device to be not-busy.
ساکن صحیح
await_not_bsy(u16 بنیاد)
{
    واپس await_ide(ATA_CB_STAT_BSY, 0, بنیاد, IDE_TIMEOUT);
}

// Wait for the device to be ready.
ساکن صحیح
await_rdy(u16 بنیاد)
{
    واپس await_ide(ATA_CB_STAT_RDY, ATA_CB_STAT_RDY, بنیاد, IDE_TIMEOUT);
}

// Wait for ide state - pauses for one ata cycle first.
ساکن اندرونی صحیح
pause_await_not_bsy(u16 iobase1, u16 iobase2)
{
    // Wait one PIO transfer cycle.
    inb(iobase2 + ATA_CB_ASTAT);

    واپس await_not_bsy(iobase1);
}

// Wait for ide state - pause for 400ns first.
ساکن اندرونی صحیح
ndelay_await_not_bsy(u16 iobase1)
{
    ndelay(400);
    واپس await_not_bsy(iobase1);
}

// Reset a drive
ساکن خالی
ata_reset(ساخت atadrive_s *adrive_gf)
{
    ساخت ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
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
    صحیح حالت_2 = await_not_bsy(iobase1);
    اگر (حالت_2 < 0)
        جاؤ مکمل;
    اگر (slave) {
        // Change device.
        u32 انتہا = timer_calc(IDE_TIMEOUT);
        برائے (;;) {
            outb(ATA_CB_DH_DEV1, iobase1 + ATA_CB_DH);
            حالت_2 = ndelay_await_not_bsy(iobase1);
            اگر (حالت_2 < 0)
                جاؤ مکمل;
            اگر (inb(iobase1 + ATA_CB_DH) == ATA_CB_DH_DEV1)
                توڑو;
            // Change drive request failed to take effect - retry.
            اگر (timer_check(انتہا)) {
                warn_timeout();
                جاؤ مکمل;
            }
        }
    } ورنہ {
        // QEMU doesn't reset dh on reset, so set it explicitly.
        outb(ATA_CB_DH_DEV0, iobase1 + ATA_CB_DH);
    }

    // On a user-reset request, wait for RDY if it is an ATA device.
    u8 type=GET_GLOBALFLAT(adrive_gf->drive.type);
    اگر (type == DTYPE_ATA)
        حالت_2 = await_rdy(iobase1);

مکمل:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    dprintf(6, "ata_reset exit status=%x\n", حالت_2);
}

// Check for drive RDY for 16bit interface command.
ساکن صحیح
isready(ساخت atadrive_s *adrive_gf)
{
    // Read the status from controller
    ساخت ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u8 حالت_2 = inb(iobase1 + ATA_CB_STAT);
    اگر ((حالت_2 & (ATA_CB_STAT_BSY|ATA_CB_STAT_RDY)) == ATA_CB_STAT_RDY)
        واپس DISK_RET_SUCCESS;
    واپس DISK_RET_ENOTREADY;
}


/****************************************************************
 * ATA send command
 ****************************************************************/

ساخت ata_pio_command {
    u8 feature;
    u8 sector_گنتی;
    u8 lba_کم;
    u8 lba_mid;
    u8 lba_بلند;
    u8 device;
    u8 command;

    u8 feature2;
    u8 sector_count2;
    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
};

// Send an ata command to the drive.
ساکن صحیح
send_cmd(ساخت atadrive_s *adrive_gf, ساخت ata_pio_command *cmd)
{
    ساخت ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);

    // Select device
    صحیح حالت_2 = await_not_bsy(iobase1);
    اگر (حالت_2 < 0)
        واپس حالت_2;
    u8 newdh = ((cmd->device & ~ATA_CB_DH_DEV1)
                | (slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0));
    u8 olddh = inb(iobase1 + ATA_CB_DH);
    outb(newdh, iobase1 + ATA_CB_DH);
    اگر ((olddh ^ newdh) & (1<<4)) {
        // Was a device change - wait for device to become not busy.
        حالت_2 = ndelay_await_not_bsy(iobase1);
        اگر (حالت_2 < 0)
            واپس حالت_2;
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
    outb(cmd->sector_گنتی, iobase1 + ATA_CB_SC);
    outb(cmd->lba_کم, iobase1 + ATA_CB_SN);
    outb(cmd->lba_mid, iobase1 + ATA_CB_CL);
    outb(cmd->lba_بلند, iobase1 + ATA_CB_CH);
    outb(cmd->command, iobase1 + ATA_CB_CMD);

    واپس 0;
}

// Wait for data after calling 'send_cmd'.
ساکن صحیح
ata_wait_ڈیٹا(u16 iobase1)
{
    صحیح حالت_2 = ndelay_await_not_bsy(iobase1);
    اگر (حالت_2 < 0)
        واپس حالت_2;

    اگر (حالت_2 & ATA_CB_STAT_ERR) {
        dprintf(6, "send_cmd : read error (status=%02x err=%02x)\n"
                , حالت_2, inb(iobase1 + ATA_CB_ERR));
        واپس -4;
    }
    اگر (!(حالت_2 & ATA_CB_STAT_DRQ)) {
        dprintf(6, "send_cmd : DRQ not set (status %02x)\n", حالت_2);
        واپس -5;
    }

    واپس 0;
}

// Send an ata command that does not transfer any further data.
صحیح
ata_cmd_nondata(ساخت atadrive_s *adrive_gf, ساخت ata_pio_command *cmd)
{
    ساخت ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    صحیح ret = send_cmd(adrive_gf, cmd);
    اگر (ret)
        جاؤ fail;
    ret = ndelay_await_not_bsy(iobase1);
    اگر (ret < 0)
        جاؤ fail;

    اگر (ret & ATA_CB_STAT_ERR) {
        dprintf(6, "nondata cmd : read error (status=%02x err=%02x)\n"
                , ret, inb(iobase1 + ATA_CB_ERR));
        ret = -4;
        جاؤ fail;
    }
    اگر (ret & ATA_CB_STAT_DRQ) {
        dprintf(6, "nondata cmd : DRQ set (status %02x)\n", ret);
        ret = -5;
        جاؤ fail;
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    واپس ret;
}


/****************************************************************
 * ATA PIO transfers
 ****************************************************************/

// Transfer 'op->count' blocks (of 'blocksize' bytes) to/from drive
// 'op->drive_fl'.
ساکن صحیح
ata_pio_transfer(ساخت disk_op_s *op, صحیح iswrite, صحیح blocksize)
{
    dprintf(16, "ata_pio_transfer id=%p write=%d count=%d bs=%d buf=%p\n"
            , op->drive_fl, iswrite, op->گنتی, blocksize, op->buf_fl);

    ساخت atadrive_s *adrive_gf = container_کا(
        op->drive_fl, ساخت atadrive_s, drive);
    ساخت ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    صحیح گنتی = op->گنتی;
    خالی *buf_fl = op->buf_fl;
    صحیح حالت_2;
    برائے (;;) {
        اگر (iswrite) {
            // Write data to controller
            dprintf(16, "Write sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            اگر (CONFIG_ATA_PIO32)
                outsl_fl(iobase1, buf_fl, blocksize / 4);
            ورنہ
                outsw_fl(iobase1, buf_fl, blocksize / 2);
        } ورنہ {
            // Read data from controller
            dprintf(16, "Read sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            اگر (CONFIG_ATA_PIO32)
                insl_fl(iobase1, buf_fl, blocksize / 4);
            ورنہ
                insw_fl(iobase1, buf_fl, blocksize / 2);
        }
        buf_fl += blocksize;

        حالت_2 = pause_await_not_bsy(iobase1, iobase2);
        اگر (حالت_2 < 0) {
            // Error
            op->گنتی -= گنتی;
            واپس حالت_2;
        }

        گنتی--;
        اگر (!گنتی)
            توڑو;
        حالت_2 &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ | ATA_CB_STAT_ERR);
        اگر (حالت_2 != ATA_CB_STAT_DRQ) {
            dprintf(6, "ata_pio_transfer : more sectors left (status %02x)\n"
                    , حالت_2);
            op->گنتی -= گنتی;
            واپس -6;
        }
    }

    حالت_2 &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
               | ATA_CB_STAT_ERR);
    اگر (!iswrite)
        حالت_2 &= ~ATA_CB_STAT_DF;
    اگر (حالت_2 != 0) {
        dprintf(6, "ata_pio_transfer : no sectors left (status %02x)\n", حالت_2);
        واپس -7;
    }

    واپس 0;
}


/****************************************************************
 * ATA DMA transfers
 ****************************************************************/

#تعریف_کرو BM_CMD    0
#تعریف_کرو  BM_CMD_MEMWRITE  0x08
#تعریف_کرو  BM_CMD_آغاز     0x01
#تعریف_کرو BM_حالت 2
#تعریف_کرو  BM_حالت_IRQ    0x04
#تعریف_کرو  BM_حالت_ERROR  0x02
#تعریف_کرو  BM_حالت_فعال 0x01
#تعریف_کرو BM_جدول  4

ساخت sff_dma_prd {
    u32 buf_fl;
    u32 گنتی;
};

// Check if DMA available and setup transfer if so.
ساکن صحیح
ata_try_dma(ساخت disk_op_s *op, صحیح iswrite, صحیح blocksize)
{
    ASSERT16();
    اگر (! CONFIG_ATA_DMA)
        واپس -1;
    u32 dest = (u32)op->buf_fl;
    اگر (dest & 1)
        // Need minimum alignment of 1.
        واپس -1;
    ساخت atadrive_s *adrive_gf = container_کا(
        op->drive_fl, ساخت atadrive_s, drive);
    ساخت ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);
    اگر (! iomaster)
        واپس -1;
    u32 بائٹیں = op->گنتی * blocksize;
    اگر (! بائٹیں)
        واپس -1;

    // Build PRD dma structure.
    ساخت sff_dma_prd *dma = MAKE_FLATPTR(SEG_کم, Extra_ڈھیر);
    ساخت sff_dma_prd *origdma = dma;
    جب (بائٹیں) {
        اگر (dma >= &origdma[16])
            // Too many descriptors..
            واپس -1;
        u32 گنتی = بائٹیں;
        u32 max = 0x10000 - (dest & 0xffff);
        اگر (گنتی > max)
            گنتی = max;

        سیٹ_LOWFLAT(dma->buf_fl, dest);
        بائٹیں -= گنتی;
        اگر (!بائٹیں)
            // Last descriptor.
            گنتی |= 1<<31;
        dprintf(16, "dma@%p: %08x %08x\n", dma, dest, گنتی);
        dest += گنتی;
        سیٹ_LOWFLAT(dma->گنتی, گنتی);
        dma++;
    }

    // Program bus-master controller.
    outl((u32)origdma, iomaster + BM_جدول);
    u8 oldcmd = inb(iomaster + BM_CMD) & ~(BM_CMD_MEMWRITE|BM_CMD_آغاز);
    outb(oldcmd | (iswrite ? 0x00 : BM_CMD_MEMWRITE), iomaster + BM_CMD);
    outb(BM_حالت_ERROR|BM_حالت_IRQ, iomaster + BM_حالت);

    واپس 0;
}

// Transfer data using DMA.
ساکن صحیح
ata_dma_transfer(ساخت disk_op_s *op)
{
    اگر (! CONFIG_ATA_DMA)
        واپس -1;
    dprintf(16, "ata_dma_transfer id=%p buf=%p\n", op->drive_fl, op->buf_fl);

    ساخت atadrive_s *adrive_gf = container_کا(
        op->drive_fl, ساخت atadrive_s, drive);
    ساخت ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);

    // Start bus-master controller.
    u8 oldcmd = inb(iomaster + BM_CMD);
    outb(oldcmd | BM_CMD_آغاز, iomaster + BM_CMD);

    u32 انتہا = timer_calc(IDE_TIMEOUT);
    u8 حالت_2;
    برائے (;;) {
        حالت_2 = inb(iomaster + BM_حالت);
        اگر (حالت_2 & BM_حالت_IRQ)
            توڑو;
        // Transfer in progress
        اگر (timer_check(انتہا)) {
            // Timeout.
            warn_timeout();
            توڑو;
        }
        yield();
    }
    outb(oldcmd & ~BM_CMD_آغاز, iomaster + BM_CMD);

    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    صحیح idestatus = pause_await_not_bsy(iobase1, iobase2);

    اگر ((حالت_2 & (BM_حالت_IRQ|BM_حالت_فعال)) == BM_حالت_IRQ
        && idestatus >= 0x00
        && (idestatus & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
                         | ATA_CB_STAT_ERR)) == 0x00)
        // Success.
        واپس 0;

    dprintf(6, "IDE DMA error (dma=%x ide=%x/%x/%x)\n", حالت_2, idestatus
            , inb(iobase2 + ATA_CB_ASTAT), inb(iobase1 + ATA_CB_ERR));
    واپس -1;
}


/****************************************************************
 * ATA hard drive functions
 ****************************************************************/

// Transfer data to harddrive using PIO protocol.
ساکن صحیح
ata_pio_cmd_ڈیٹا(ساخت disk_op_s *op, صحیح iswrite, ساخت ata_pio_command *cmd)
{
    ساخت atadrive_s *adrive_gf = container_کا(
        op->drive_fl, ساخت atadrive_s, drive);
    ساخت ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    صحیح ret = send_cmd(adrive_gf, cmd);
    اگر (ret)
        جاؤ fail;
    ret = ata_wait_ڈیٹا(iobase1);
    اگر (ret)
        جاؤ fail;
    ret = ata_pio_transfer(op, iswrite, DISK_SECTOR_سائز);

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    واپس ret;
}

// Transfer data to harddrive using DMA protocol.
ساکن صحیح
ata_dma_cmd_ڈیٹا(ساخت disk_op_s *op, ساخت ata_pio_command *cmd)
{
    اگر (! CONFIG_ATA_DMA)
        واپس -1;
    ساخت atadrive_s *adrive_gf = container_کا(
        op->drive_fl, ساخت atadrive_s, drive);
    صحیح ret = send_cmd(adrive_gf, cmd);
    اگر (ret)
        واپس ret;
    واپس ata_dma_transfer(op);
}

// Read/write count blocks from a harddrive.
ساکن صحیح
ata_readwrite(ساخت disk_op_s *op, صحیح iswrite)
{
    u64 lba = op->lba;

    صحیح usepio = ata_try_dma(op, iswrite, DISK_SECTOR_سائز);

    ساخت ata_pio_command cmd;
    memset(&cmd, 0, حجم(cmd));

    اگر (op->گنتی >= (1<<8) || lba + op->گنتی >= (1<<28)) {
        cmd.sector_count2 = op->گنتی >> 8;
        cmd.lba_low2 = lba >> 24;
        cmd.lba_mid2 = lba >> 32;
        cmd.lba_high2 = lba >> 40;
        lba &= 0xffffff;

        اگر (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS_EXT
                           : ATA_CMD_READ_SECTORS_EXT);
        ورنہ
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                           : ATA_CMD_READ_DMA_EXT);
    } ورنہ {
        اگر (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS
                           : ATA_CMD_READ_SECTORS);
        ورنہ
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA
                           : ATA_CMD_READ_DMA);
    }

    cmd.sector_گنتی = op->گنتی;
    cmd.lba_کم = lba;
    cmd.lba_mid = lba >> 8;
    cmd.lba_بلند = lba >> 16;
    cmd.device = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;

    صحیح ret;
    اگر (usepio)
        ret = ata_pio_cmd_ڈیٹا(op, iswrite, &cmd);
    ورنہ
        ret = ata_dma_cmd_ڈیٹا(op, &cmd);
    اگر (ret)
        واپس DISK_RET_EBADTRACK;
    واپس DISK_RET_SUCCESS;
}

// 16bit command demuxer for ATA harddrives.
صحیح
ata_process_op(ساخت disk_op_s *op)
{
    اگر (!CONFIG_ATA)
        واپس 0;

    ساخت atadrive_s *adrive_gf = container_کا(
        op->drive_fl, ساخت atadrive_s, drive);
    انتخاب (op->command) {
    حالت CMD_READ:
        واپس ata_readwrite(op, 0);
    حالت CMD_WRITE:
        واپس ata_readwrite(op, 1);
    حالت CMD_RESET:
        ata_reset(adrive_gf);
        واپس DISK_RET_SUCCESS;
    حالت CMD_ISREADY:
        واپس isready(adrive_gf);
    طےشدہ:
        واپس default_process_op(op);
    }
}


/****************************************************************
 * ATAPI functions
 ****************************************************************/

#تعریف_کرو CDROM_CDB_سائز 12

// Low-level atapi command transmit function.
صحیح
ata_atapi_process_op(ساخت disk_op_s *op)
{
    اگر (! CONFIG_ATA)
        واپس 0;

    اگر (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        واپس DISK_RET_EWRITEPROTECT;
    u8 cdbcmd[CDROM_CDB_سائز];
    صحیح blocksize = scsi_fill_cmd(op, cdbcmd, حجم(cdbcmd));
    اگر (blocksize < 0)
        واپس default_process_op(op);

    ساخت atadrive_s *adrive_gf = container_کا(
        op->drive_fl, ساخت atadrive_s, drive);
    ساخت ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    ساخت ata_pio_command cmd;
    memset(&cmd, 0, حجم(cmd));
    cmd.lba_mid = blocksize;
    cmd.lba_بلند = blocksize >> 8;
    cmd.command = ATA_CMD_PACKET;

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    صحیح ret = send_cmd(adrive_gf, &cmd);
    اگر (ret)
        جاؤ fail;
    ret = ata_wait_ڈیٹا(iobase1);
    اگر (ret)
        جاؤ fail;

    // Send command to device
    outsw_fl(iobase1, MAKE_FLATPTR(GET_SEG(SS), cdbcmd), CDROM_CDB_سائز / 2);

    صحیح حالت_2 = pause_await_not_bsy(iobase1, iobase2);
    اگر (حالت_2 < 0) {
        ret = حالت_2;
        جاؤ fail;
    }

    اگر (حالت_2 & ATA_CB_STAT_ERR) {
        u8 err = inb(iobase1 + ATA_CB_ERR);
        // skip "Not Ready"
        اگر (err != 0x20)
            dprintf(6, "send_atapi_cmd : read error (status=%02x err=%02x)\n"
                    , حالت_2, err);
        ret = -2;
        جاؤ fail;
    }
    اگر (blocksize) {
        اگر (!(حالت_2 & ATA_CB_STAT_DRQ)) {
            dprintf(6, "send_atapi_cmd : DRQ not set (status %02x)\n", حالت_2);
            ret = -3;
            جاؤ fail;
        }

        ret = ata_pio_transfer(op, 0, blocksize);
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    اگر (ret)
        واپس DISK_RET_EBADTRACK;
    واپس DISK_RET_SUCCESS;
}


/****************************************************************
 * ATA detect and init
 ****************************************************************/

// Send an identify device or identify device packet command.
ساکن صحیح
send_ata_شناختی_عنصر(ساخت atadrive_s *adrive, u16 *بفر, صحیح command)
{
    memset(بفر, 0, DISK_SECTOR_سائز);

    ساخت disk_op_s dop;
    memset(&dop, 0, حجم(dop));
    dop.drive_fl = &adrive->drive;
    dop.گنتی = 1;
    dop.lba = 1;
    dop.buf_fl = MAKE_FLATPTR(GET_SEG(SS), بفر);

    ساخت ata_pio_command cmd;
    memset(&cmd, 0, حجم(cmd));
    cmd.command = command;

    واپس ata_pio_cmd_ڈیٹا(&dop, 0, &cmd);
}

// Extract the ATA/ATAPI version info.
صحیح
ata_نکالو_نسخہ(u16 *بفر)
{
    // Extract ATA/ATAPI version.
    u16 ataversion = بفر[80];
    u8 نسخہ_2;
    برائے (نسخہ_2=15; نسخہ_2>0; نسخہ_2--)
        اگر (ataversion & (1<<نسخہ_2))
            توڑو;
    واپس نسخہ_2;
}

#تعریف_کرو MAXMODEL 40

// Extract the ATA/ATAPI model info.
حرف *
ata_نکالو_model(حرف *model, u32 سائز, u16 *بفر)
{
    // Read model name
    صحیح i;
    برائے (i=0; i<سائز/2; i++)
        *(u16*)&model[i*2] = be16_مقصد_cpu(بفر[27+i]);
    model[سائز] = 0x00;
    nullTrailingSpace(model);
    واپس model;
}

// Common init code between ata and atapi
ساکن ساخت atadrive_s *
init_atadrive(ساخت atadrive_s *dummy, u16 *بفر)
{
    ساخت atadrive_s *adrive = malloc_fseg(حجم(*adrive));
    اگر (!adrive) {
        warn_noalloc();
        واپس تہی;
    }
    memset(adrive, 0, حجم(*adrive));
    adrive->chan_gf = dummy->chan_gf;
    adrive->slave = dummy->slave;
    adrive->drive.cntl_id = adrive->chan_gf->ataid * 2 + dummy->slave;
    adrive->drive.removable = (بفر[0] & 0x80) ? 1 : 0;
    واپس adrive;
}

// Detect if the given drive is an atapi - initialize it if so.
ساکن ساخت atadrive_s *
init_drive_atapi(ساخت atadrive_s *dummy, u16 *بفر)
{
    // Send an IDENTIFY_DEVICE_PACKET command to device
    صحیح ret = send_ata_شناختی_عنصر(dummy, بفر, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    اگر (ret)
        واپس تہی;

    // Success - setup as ATAPI.
    ساخت atadrive_s *adrive = init_atadrive(dummy, بفر);
    اگر (!adrive)
        واپس تہی;
    adrive->drive.type = DTYPE_ATA_ATAPI;
    adrive->drive.blksize = CDROM_SECTOR_سائز;
    adrive->drive.sectors = (u64)-1;
    u8 iscd = ((بفر[0] >> 8) & 0x1f) == 0x05;
    حرف model[MAXMODEL+1];
    حرف *desc = znprintf(MAXDESCSIZE
                          , "DVD/CD [ata%d-%d: %s ATAPI-%d %s]"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_نکالو_model(model, MAXMODEL, بفر)
                          , ata_نکالو_نسخہ(بفر)
                          , (iscd ? "DVD/CD" : "Device"));
    dprintf(1, "%s\n", desc);

    // fill cdidmap
    اگر (iscd) {
        صحیح prio = bootprio_ڈھونڈو_ata_device(adrive->chan_gf->pci_tmp,
                                            adrive->chan_gf->chanid,
                                            adrive->slave);
        boot_lchs_ڈھونڈو_ata_device(adrive->chan_gf->pci_tmp,
                                  adrive->chan_gf->chanid,
                                  adrive->slave,
                                  &(adrive->drive.lchs));
        boot_جوڑو_cd(&adrive->drive, desc, prio);
    }

    واپس adrive;
}

// Detect if the given drive is a regular ata drive - initialize it if so.
ساکن ساخت atadrive_s *
init_drive_ata(ساخت atadrive_s *dummy, u16 *بفر)
{
    // Send an IDENTIFY_DEVICE command to device
    صحیح ret = send_ata_شناختی_عنصر(dummy, بفر, ATA_CMD_IDENTIFY_DEVICE);
    اگر (ret)
        واپس تہی;

    // Success - setup as ATA.
    ساخت atadrive_s *adrive = init_atadrive(dummy, بفر);
    اگر (!adrive)
        واپس تہی;
    adrive->drive.type = DTYPE_ATA;
    adrive->drive.blksize = DISK_SECTOR_سائز;

    adrive->drive.pchs.cylinder = بفر[1];
    adrive->drive.pchs.سر = بفر[3];
    adrive->drive.pchs.sector = بفر[6];

    u64 sectors;
    اگر (بفر[83] & (1 << 10)) // word 83 - lba48 support
        sectors = *(u64*)&بفر[100]; // word 100-103
    ورنہ
        sectors = *(u32*)&بفر[60]; // word 60 and word 61
    adrive->drive.sectors = sectors;
    u64 adjsize = sectors >> 11;
    حرف adjprefix = 'M';
    اگر (adjsize >= (1 << 16)) {
        adjsize >>= 10;
        adjprefix = 'G';
    }
    حرف model[MAXMODEL+1];
    حرف *desc = znprintf(MAXDESCSIZE
                          , "ata%d-%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_نکالو_model(model, MAXMODEL, بفر)
                          , ata_نکالو_نسخہ(بفر)
                          , (u32)adjsize, adjprefix);
    dprintf(1, "%s\n", desc);

    صحیح prio = bootprio_ڈھونڈو_ata_device(adrive->chan_gf->pci_tmp,
                                        adrive->chan_gf->chanid,
                                        adrive->slave);
    boot_lchs_ڈھونڈو_ata_device(adrive->chan_gf->pci_tmp,
                              adrive->chan_gf->chanid,
                              adrive->slave,
                              &(adrive->drive.lchs));
    // Register with bcv system.
    boot_جوڑو_hd(&adrive->drive, desc, prio);

    واپس adrive;
}

ساکن u32 Spinup_انتہا;

// Wait for non-busy status and check for "floating bus" condition.
ساکن صحیح
powerup_await_non_bsy(u16 بنیاد)
{
    u8 orstatus = 0;
    u8 حالت_2;
    برائے (;;) {
        حالت_2 = inb(بنیاد+ATA_CB_STAT);
        اگر (!(حالت_2 & ATA_CB_STAT_BSY))
            توڑو;
        orstatus |= حالت_2;
        اگر (orstatus == 0xff) {
            dprintf(4, "powerup IDE floating\n");
            واپس orstatus;
        }
        اگر (timer_check(Spinup_انتہا)) {
            warn_timeout();
            واپس -1;
        }
        yield();
    }
    dprintf(6, "powerup iobase=%x st=%x\n", بنیاد, حالت_2);
    واپس حالت_2;
}

// Detect any drives attached to a given controller.
ساکن خالی
ata_detect(خالی *ڈیٹا)
{
    ساخت ata_channel_s *chan_gf = ڈیٹا;
    ساخت atadrive_s dummy;
    memset(&dummy, 0, حجم(dummy));
    dummy.chan_gf = chan_gf;
    // Device detection
    صحیح didreset = 0;
    u8 slave;
    برائے (slave=0; slave<=1; slave++) {
        // Wait for not-bsy.
        u16 iobase1 = chan_gf->iobase1;
        صحیح حالت_2 = powerup_await_non_bsy(iobase1);
        اگر (حالت_2 < 0)
            جاری;
        u8 newdh = slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0;
        outb(newdh, iobase1+ATA_CB_DH);
        ndelay(400);
        حالت_2 = powerup_await_non_bsy(iobase1);
        اگر (حالت_2 < 0)
            جاری;

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
            جاری;

        // Prepare new drive.
        dummy.slave = slave;

        // reset the channel
        اگر (!didreset) {
            ata_reset(&dummy);
            didreset = 1;
        }

        // check for ATAPI
        u16 بفر[256];
        ساخت atadrive_s *adrive = init_drive_atapi(&dummy, بفر);
        اگر (!adrive) {
            // Didn't find an ATAPI drive - look for ATA drive.
            u8 st = inb(iobase1+ATA_CB_STAT);
            اگر (!st)
                // Status not set - can't be a valid drive.
                جاری;

            // Wait for RDY.
            صحیح ret = await_rdy(iobase1);
            اگر (ret < 0)
                جاری;

            // check for ATA.
            adrive = init_drive_ata(&dummy, بفر);
            اگر (!adrive)
                // No ATA drive found
                جاری;
        }

        u16 resetresult = بفر[93];
        dprintf(6, "ata_detect resetresult=%04x\n", resetresult);
        اگر (!slave && (resetresult & 0xdf61) == 0x4041)
            // resetresult looks valid and device 0 is responding to
            // device 1 requests - device 1 must not be present - skip
            // detection.
            توڑو;
    }
}

// Initialize an ata controller and detect its drives.
ساکن خالی
init_controller(ساخت pci_device *pci, صحیح chanid, صحیح irq
                , u32 port1, u32 port2, u32 master)
{
    ساکن صحیح ataid = 0;
    ساخت ata_channel_s *chan_gf = malloc_fseg(حجم(*chan_gf));
    اگر (!chan_gf) {
        warn_noalloc();
        واپس;
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
    تسلسل_thread(ata_detect, chan_gf);
}

#تعریف_کرو IRQ_ATA1 14
#تعریف_کرو IRQ_ATA2 15

// Handle controllers on an ATA PCI device.
ساکن خالی
init_pciata(ساخت pci_device *pci, u8 prog_if)
{
    u8 pciirq = pci_config_readb(pci->bdf, PCI_مداخلت_سطر);
    صحیح master = 0;
    اگر (CONFIG_ATA_DMA && prog_if & 0x80) {
        // Check for bus-mastering.
        u32 bar = pci_config_readl(pci->bdf, PCI_بنیاد_پتہ_4);
        اگر (bar & PCI_بنیاد_پتہ_SPACE_IO) {
            master = pci_enable_iobar(pci, PCI_بنیاد_پتہ_4);
            pci_enable_busmaster(pci);
        }
    }

    u32 port1, port2, irq;
    اگر (prog_if & 1) {
        port1 = pci_enable_iobar(pci, PCI_بنیاد_پتہ_0);
        port2 = pci_enable_iobar(pci, PCI_بنیاد_پتہ_1);
        اگر (!port1 || !port2)
            واپس;
        irq = pciirq;
    } ورنہ {
        port1 = پورٹ_ATA1_CMD_بنیاد;
        port2 = پورٹ_ATA1_CTRL_بنیاد;
        irq = IRQ_ATA1;
    }
    init_controller(pci, 0, irq, port1, port2, master);

    اگر (prog_if & 4) {
        port1 = pci_enable_iobar(pci, PCI_بنیاد_پتہ_2);
        port2 = pci_enable_iobar(pci, PCI_بنیاد_پتہ_3);
        اگر (!port1 || !port2)
            واپس;
        irq = pciirq;
    } ورنہ {
        port1 = پورٹ_ATA2_CMD_بنیاد;
        port2 = پورٹ_ATA2_CTRL_بنیاد;
        irq = IRQ_ATA2;
    }
    init_controller(pci, 1, irq, port1, port2, master ? master + 8 : 0);
}

ساکن خالی
ملا_genericata(ساخت pci_device *pci, خالی *arg)
{
    init_pciata(pci, pci->prog_if);
}

ساکن خالی
ملا_compatibleahci(ساخت pci_device *pci, خالی *arg)
{
    اگر (CONFIG_AHCI)
        // Already handled directly via native ahci interface.
        واپس;
    init_pciata(pci, 0x8f);
}

ساکن ثابت ساخت pci_device_id pci_ata_tbl[] = {
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE
                     , ملا_genericata),
    PCI_DEVICE(PCI_VENDOR_ID_ATI, 0x4391, ملا_compatibleahci),
    PCI_DEVICE_انتہا,
};

// Locate and init ata controllers.
ساکن خالی
ata_جائزہ(خالی)
{
    اگر (CONFIG_QEMU && hlist_خالی_خانے(&PCIDevices)) {
        // No PCI devices found - probably a QEMU "-M isapc" machine.
        // Try using ISA ports for ATA controllers.
        init_controller(تہی, 0, IRQ_ATA1
                        , پورٹ_ATA1_CMD_بنیاد, پورٹ_ATA1_CTRL_بنیاد, 0);
        init_controller(تہی, 1, IRQ_ATA2
                        , پورٹ_ATA2_CMD_بنیاد, پورٹ_ATA2_CTRL_بنیاد, 0);
        واپس;
    }

    // Scan PCI bus for ATA adapters
    ساخت pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_ata_tbl, pci, تہی);
    }
}

خالی
ata_تیاری(خالی)
{
    ASSERT32FLAT();
    اگر (!CONFIG_ATA)
        واپس;

    dprintf(3, "init hard drives\n");

    Spinup_انتہا = timer_calc(IDE_TIMEOUT);
    ata_جائزہ();

    سیٹ_BDA(disk_control_byte, 0xc0);

    enable_hwirq(14, FUNC16(اندراج_76));
}

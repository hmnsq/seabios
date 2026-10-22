// Low level ATA disk access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "ata.h" // ATA_CB_STAT
#ضمن "biosvar.h" // GET_GLOBALFLAT
#ضمن "block.h" // struct drive_s
#ضمن "blockcmd.h" // CDB_CMD_READ_10
#ضمن "byteorder.h" // be16_to_cpu
#ضمن "malloc.h" // malloc_fseg
#ضمن "output.h" // dprintf
#ضمن "pci.h" // pci_config_readb
#ضمن "pcidevice.h" // foreachpci
#ضمن "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#ضمن "pci_regs.h" // PCI_INTERRUPT_LINE
#ضمن "pic.h" // enable_hwirq
#ضمن "stacks.h" // yield
#ضمن "std/disk.h" // DISK_RET_SUCCESS
#ضمن "string.h" // memset
#ضمن "util.h" // timer_calc
#ضمن "x86.h" // inb

#وصف_ڪريو IDE_TIMEOUT 32000 //32 seconds max for IDE ops


/****************************************************************
 * Helper functions
 ****************************************************************/

// Wait for the specified ide state
ساكن مضمن صحيح
await_ide(u8 نقاب, u8 flags, u16 بنياد, u16 timeout)
{
    u32 پڇاڙي = timer_calc(timeout);
    لكل (;;) {
        u8 حالت = inb(بنياد+ATA_CB_STAT);
        إذا ((حالت & نقاب) == flags)
            أرجع حالت;
        إذا (timer_check(پڇاڙي)) {
            warn_timeout();
            أرجع -1;
        }
        yield();
    }
}

// Wait for the device to be not-busy.
ساكن صحيح
await_not_bsy(u16 بنياد)
{
    أرجع await_ide(ATA_CB_STAT_BSY, 0, بنياد, IDE_TIMEOUT);
}

// Wait for the device to be ready.
ساكن صحيح
await_rdy(u16 بنياد)
{
    أرجع await_ide(ATA_CB_STAT_RDY, ATA_CB_STAT_RDY, بنياد, IDE_TIMEOUT);
}

// Wait for ide state - pauses for one ata cycle first.
ساكن مضمن صحيح
pause_await_not_bsy(u16 iobase1, u16 iobase2)
{
    // Wait one PIO transfer cycle.
    inb(iobase2 + ATA_CB_ASTAT);

    أرجع await_not_bsy(iobase1);
}

// Wait for ide state - pause for 400ns first.
ساكن مضمن صحيح
ndelay_await_not_bsy(u16 iobase1)
{
    ndelay(400);
    أرجع await_not_bsy(iobase1);
}

// Reset a drive
ساكن فراغ
ata_reset(بنية atadrive_s *adrive_gf)
{
    بنية ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
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
    صحيح حالت = await_not_bsy(iobase1);
    إذا (حالت < 0)
        اذهب پورو;
    إذا (slave) {
        // Change device.
        u32 پڇاڙي = timer_calc(IDE_TIMEOUT);
        لكل (;;) {
            outb(ATA_CB_DH_DEV1, iobase1 + ATA_CB_DH);
            حالت = ndelay_await_not_bsy(iobase1);
            إذا (حالت < 0)
                اذهب پورو;
            إذا (inb(iobase1 + ATA_CB_DH) == ATA_CB_DH_DEV1)
                اكسر;
            // Change drive request failed to take effect - retry.
            إذا (timer_check(پڇاڙي)) {
                warn_timeout();
                اذهب پورو;
            }
        }
    } وإلا {
        // QEMU doesn't reset dh on reset, so set it explicitly.
        outb(ATA_CB_DH_DEV0, iobase1 + ATA_CB_DH);
    }

    // On a user-reset request, wait for RDY if it is an ATA device.
    u8 type=GET_GLOBALFLAT(adrive_gf->drive.type);
    إذا (type == DTYPE_ATA)
        حالت = await_rdy(iobase1);

پورو:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    dprintf(6, "ata_reset exit status=%x\n", حالت);
}

// Check for drive RDY for 16bit interface command.
ساكن صحيح
isready(بنية atadrive_s *adrive_gf)
{
    // Read the status from controller
    بنية ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u8 حالت = inb(iobase1 + ATA_CB_STAT);
    إذا ((حالت & (ATA_CB_STAT_BSY|ATA_CB_STAT_RDY)) == ATA_CB_STAT_RDY)
        أرجع DISK_RET_SUCCESS;
    أرجع DISK_RET_ENOTREADY;
}


/****************************************************************
 * ATA send command
 ****************************************************************/

بنية ata_pio_command {
    u8 feature;
    u8 sector_ڳڻپ;
    u8 lba_هيٺيون;
    u8 lba_mid;
    u8 lba_مٿاهون;
    u8 device;
    u8 command;

    u8 feature2;
    u8 sector_count2;
    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
};

// Send an ata command to the drive.
ساكن صحيح
send_cmd(بنية atadrive_s *adrive_gf, بنية ata_pio_command *cmd)
{
    بنية ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);

    // Select device
    صحيح حالت = await_not_bsy(iobase1);
    إذا (حالت < 0)
        أرجع حالت;
    u8 newdh = ((cmd->device & ~ATA_CB_DH_DEV1)
                | (slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0));
    u8 olddh = inb(iobase1 + ATA_CB_DH);
    outb(newdh, iobase1 + ATA_CB_DH);
    إذا ((olddh ^ newdh) & (1<<4)) {
        // Was a device change - wait for device to become not busy.
        حالت = ndelay_await_not_bsy(iobase1);
        إذا (حالت < 0)
            أرجع حالت;
    }

    // Check for ATA_CMD_(READ|WRITE)_(SECTORS|DMA)_EXT commands.
    إذا ((cmd->command & ~0x11) == ATA_CMD_READ_SECTORS_EXT) {
        outb(cmd->feature2, iobase1 + ATA_CB_FR);
        outb(cmd->sector_count2, iobase1 + ATA_CB_SC);
        outb(cmd->lba_low2, iobase1 + ATA_CB_SN);
        outb(cmd->lba_mid2, iobase1 + ATA_CB_CL);
        outb(cmd->lba_high2, iobase1 + ATA_CB_CH);
    }
    outb(cmd->feature, iobase1 + ATA_CB_FR);
    outb(cmd->sector_ڳڻپ, iobase1 + ATA_CB_SC);
    outb(cmd->lba_هيٺيون, iobase1 + ATA_CB_SN);
    outb(cmd->lba_mid, iobase1 + ATA_CB_CL);
    outb(cmd->lba_مٿاهون, iobase1 + ATA_CB_CH);
    outb(cmd->command, iobase1 + ATA_CB_CMD);

    أرجع 0;
}

// Wait for data after calling 'send_cmd'.
ساكن صحيح
ata_wait_معلومات(u16 iobase1)
{
    صحيح حالت = ndelay_await_not_bsy(iobase1);
    إذا (حالت < 0)
        أرجع حالت;

    إذا (حالت & ATA_CB_STAT_ERR) {
        dprintf(6, "send_cmd : read error (status=%02x err=%02x)\n"
                , حالت, inb(iobase1 + ATA_CB_ERR));
        أرجع -4;
    }
    إذا (!(حالت & ATA_CB_STAT_DRQ)) {
        dprintf(6, "send_cmd : DRQ not set (status %02x)\n", حالت);
        أرجع -5;
    }

    أرجع 0;
}

// Send an ata command that does not transfer any further data.
صحيح
ata_cmd_nondata(بنية atadrive_s *adrive_gf, بنية ata_pio_command *cmd)
{
    بنية ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    صحيح ret = send_cmd(adrive_gf, cmd);
    إذا (ret)
        اذهب fail;
    ret = ndelay_await_not_bsy(iobase1);
    إذا (ret < 0)
        اذهب fail;

    إذا (ret & ATA_CB_STAT_ERR) {
        dprintf(6, "nondata cmd : read error (status=%02x err=%02x)\n"
                , ret, inb(iobase1 + ATA_CB_ERR));
        ret = -4;
        اذهب fail;
    }
    إذا (ret & ATA_CB_STAT_DRQ) {
        dprintf(6, "nondata cmd : DRQ set (status %02x)\n", ret);
        ret = -5;
        اذهب fail;
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    أرجع ret;
}


/****************************************************************
 * ATA PIO transfers
 ****************************************************************/

// Transfer 'op->count' blocks (of 'blocksize' bytes) to/from drive
// 'op->drive_fl'.
ساكن صحيح
ata_pio_transfer(بنية disk_op_s *op, صحيح iswrite, صحيح blocksize)
{
    dprintf(16, "ata_pio_transfer id=%p write=%d count=%d bs=%d buf=%p\n"
            , op->drive_fl, iswrite, op->ڳڻپ, blocksize, op->buf_fl);

    بنية atadrive_s *adrive_gf = container_جو(
        op->drive_fl, بنية atadrive_s, drive);
    بنية ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    صحيح ڳڻپ = op->ڳڻپ;
    فراغ *buf_fl = op->buf_fl;
    صحيح حالت;
    لكل (;;) {
        إذا (iswrite) {
            // Write data to controller
            dprintf(16, "Write sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            إذا (CONFIG_ATA_PIO32)
                outsl_fl(iobase1, buf_fl, blocksize / 4);
            وإلا
                outsw_fl(iobase1, buf_fl, blocksize / 2);
        } وإلا {
            // Read data from controller
            dprintf(16, "Read sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            إذا (CONFIG_ATA_PIO32)
                insl_fl(iobase1, buf_fl, blocksize / 4);
            وإلا
                insw_fl(iobase1, buf_fl, blocksize / 2);
        }
        buf_fl += blocksize;

        حالت = pause_await_not_bsy(iobase1, iobase2);
        إذا (حالت < 0) {
            // Error
            op->ڳڻپ -= ڳڻپ;
            أرجع حالت;
        }

        ڳڻپ--;
        إذا (!ڳڻپ)
            اكسر;
        حالت &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ | ATA_CB_STAT_ERR);
        إذا (حالت != ATA_CB_STAT_DRQ) {
            dprintf(6, "ata_pio_transfer : more sectors left (status %02x)\n"
                    , حالت);
            op->ڳڻپ -= ڳڻپ;
            أرجع -6;
        }
    }

    حالت &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
               | ATA_CB_STAT_ERR);
    إذا (!iswrite)
        حالت &= ~ATA_CB_STAT_DF;
    إذا (حالت != 0) {
        dprintf(6, "ata_pio_transfer : no sectors left (status %02x)\n", حالت);
        أرجع -7;
    }

    أرجع 0;
}


/****************************************************************
 * ATA DMA transfers
 ****************************************************************/

#وصف_ڪريو BM_CMD    0
#وصف_ڪريو  BM_CMD_MEMWRITE  0x08
#وصف_ڪريو  BM_CMD_شروعات     0x01
#وصف_ڪريو BM_حالت 2
#وصف_ڪريو  BM_حالت_IRQ    0x04
#وصف_ڪريو  BM_حالت_ERROR  0x02
#وصف_ڪريو  BM_حالت_سرگرم 0x01
#وصف_ڪريو BM_جدول  4

بنية sff_dma_prd {
    u32 buf_fl;
    u32 ڳڻپ;
};

// Check if DMA available and setup transfer if so.
ساكن صحيح
ata_try_dma(بنية disk_op_s *op, صحيح iswrite, صحيح blocksize)
{
    ASSERT16();
    إذا (! CONFIG_ATA_DMA)
        أرجع -1;
    u32 dest = (u32)op->buf_fl;
    إذا (dest & 1)
        // Need minimum alignment of 1.
        أرجع -1;
    بنية atadrive_s *adrive_gf = container_جو(
        op->drive_fl, بنية atadrive_s, drive);
    بنية ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);
    إذا (! iomaster)
        أرجع -1;
    u32 بائيٽون = op->ڳڻپ * blocksize;
    إذا (! بائيٽون)
        أرجع -1;

    // Build PRD dma structure.
    بنية sff_dma_prd *dma = MAKE_FLATPTR(SEG_هيٺيون, Extra_ڍير);
    بنية sff_dma_prd *origdma = dma;
    طالما (بائيٽون) {
        إذا (dma >= &origdma[16])
            // Too many descriptors..
            أرجع -1;
        u32 ڳڻپ = بائيٽون;
        u32 max = 0x10000 - (dest & 0xffff);
        إذا (ڳڻپ > max)
            ڳڻپ = max;

        سيٽ_LOWFLAT(dma->buf_fl, dest);
        بائيٽون -= ڳڻپ;
        إذا (!بائيٽون)
            // Last descriptor.
            ڳڻپ |= 1<<31;
        dprintf(16, "dma@%p: %08x %08x\n", dma, dest, ڳڻپ);
        dest += ڳڻپ;
        سيٽ_LOWFLAT(dma->ڳڻپ, ڳڻپ);
        dma++;
    }

    // Program bus-master controller.
    outl((u32)origdma, iomaster + BM_جدول);
    u8 oldcmd = inb(iomaster + BM_CMD) & ~(BM_CMD_MEMWRITE|BM_CMD_شروعات);
    outb(oldcmd | (iswrite ? 0x00 : BM_CMD_MEMWRITE), iomaster + BM_CMD);
    outb(BM_حالت_ERROR|BM_حالت_IRQ, iomaster + BM_حالت);

    أرجع 0;
}

// Transfer data using DMA.
ساكن صحيح
ata_dma_transfer(بنية disk_op_s *op)
{
    إذا (! CONFIG_ATA_DMA)
        أرجع -1;
    dprintf(16, "ata_dma_transfer id=%p buf=%p\n", op->drive_fl, op->buf_fl);

    بنية atadrive_s *adrive_gf = container_جو(
        op->drive_fl, بنية atadrive_s, drive);
    بنية ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);

    // Start bus-master controller.
    u8 oldcmd = inb(iomaster + BM_CMD);
    outb(oldcmd | BM_CMD_شروعات, iomaster + BM_CMD);

    u32 پڇاڙي = timer_calc(IDE_TIMEOUT);
    u8 حالت;
    لكل (;;) {
        حالت = inb(iomaster + BM_حالت);
        إذا (حالت & BM_حالت_IRQ)
            اكسر;
        // Transfer in progress
        إذا (timer_check(پڇاڙي)) {
            // Timeout.
            warn_timeout();
            اكسر;
        }
        yield();
    }
    outb(oldcmd & ~BM_CMD_شروعات, iomaster + BM_CMD);

    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    صحيح idestatus = pause_await_not_bsy(iobase1, iobase2);

    إذا ((حالت & (BM_حالت_IRQ|BM_حالت_سرگرم)) == BM_حالت_IRQ
        && idestatus >= 0x00
        && (idestatus & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
                         | ATA_CB_STAT_ERR)) == 0x00)
        // Success.
        أرجع 0;

    dprintf(6, "IDE DMA error (dma=%x ide=%x/%x/%x)\n", حالت, idestatus
            , inb(iobase2 + ATA_CB_ASTAT), inb(iobase1 + ATA_CB_ERR));
    أرجع -1;
}


/****************************************************************
 * ATA hard drive functions
 ****************************************************************/

// Transfer data to harddrive using PIO protocol.
ساكن صحيح
ata_pio_cmd_معلومات(بنية disk_op_s *op, صحيح iswrite, بنية ata_pio_command *cmd)
{
    بنية atadrive_s *adrive_gf = container_جو(
        op->drive_fl, بنية atadrive_s, drive);
    بنية ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    صحيح ret = send_cmd(adrive_gf, cmd);
    إذا (ret)
        اذهب fail;
    ret = ata_wait_معلومات(iobase1);
    إذا (ret)
        اذهب fail;
    ret = ata_pio_transfer(op, iswrite, DISK_SECTOR_سائيز);

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    أرجع ret;
}

// Transfer data to harddrive using DMA protocol.
ساكن صحيح
ata_dma_cmd_معلومات(بنية disk_op_s *op, بنية ata_pio_command *cmd)
{
    إذا (! CONFIG_ATA_DMA)
        أرجع -1;
    بنية atadrive_s *adrive_gf = container_جو(
        op->drive_fl, بنية atadrive_s, drive);
    صحيح ret = send_cmd(adrive_gf, cmd);
    إذا (ret)
        أرجع ret;
    أرجع ata_dma_transfer(op);
}

// Read/write count blocks from a harddrive.
ساكن صحيح
ata_readwrite(بنية disk_op_s *op, صحيح iswrite)
{
    u64 lba = op->lba;

    صحيح usepio = ata_try_dma(op, iswrite, DISK_SECTOR_سائيز);

    بنية ata_pio_command cmd;
    memset(&cmd, 0, حجم(cmd));

    إذا (op->ڳڻپ >= (1<<8) || lba + op->ڳڻپ >= (1<<28)) {
        cmd.sector_count2 = op->ڳڻپ >> 8;
        cmd.lba_low2 = lba >> 24;
        cmd.lba_mid2 = lba >> 32;
        cmd.lba_high2 = lba >> 40;
        lba &= 0xffffff;

        إذا (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS_EXT
                           : ATA_CMD_READ_SECTORS_EXT);
        وإلا
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                           : ATA_CMD_READ_DMA_EXT);
    } وإلا {
        إذا (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS
                           : ATA_CMD_READ_SECTORS);
        وإلا
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA
                           : ATA_CMD_READ_DMA);
    }

    cmd.sector_ڳڻپ = op->ڳڻپ;
    cmd.lba_هيٺيون = lba;
    cmd.lba_mid = lba >> 8;
    cmd.lba_مٿاهون = lba >> 16;
    cmd.device = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;

    صحيح ret;
    إذا (usepio)
        ret = ata_pio_cmd_معلومات(op, iswrite, &cmd);
    وإلا
        ret = ata_dma_cmd_معلومات(op, &cmd);
    إذا (ret)
        أرجع DISK_RET_EBADTRACK;
    أرجع DISK_RET_SUCCESS;
}

// 16bit command demuxer for ATA harddrives.
صحيح
ata_process_op(بنية disk_op_s *op)
{
    إذا (!CONFIG_ATA)
        أرجع 0;

    بنية atadrive_s *adrive_gf = container_جو(
        op->drive_fl, بنية atadrive_s, drive);
    بدل (op->command) {
    حالة CMD_READ:
        أرجع ata_readwrite(op, 0);
    حالة CMD_WRITE:
        أرجع ata_readwrite(op, 1);
    حالة CMD_RESET:
        ata_reset(adrive_gf);
        أرجع DISK_RET_SUCCESS;
    حالة CMD_ISREADY:
        أرجع isready(adrive_gf);
    افتراضي:
        أرجع default_process_op(op);
    }
}


/****************************************************************
 * ATAPI functions
 ****************************************************************/

#وصف_ڪريو CDROM_CDB_سائيز 12

// Low-level atapi command transmit function.
صحيح
ata_atapi_process_op(بنية disk_op_s *op)
{
    إذا (! CONFIG_ATA)
        أرجع 0;

    إذا (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        أرجع DISK_RET_EWRITEPROTECT;
    u8 cdbcmd[CDROM_CDB_سائيز];
    صحيح blocksize = scsi_fill_cmd(op, cdbcmd, حجم(cdbcmd));
    إذا (blocksize < 0)
        أرجع default_process_op(op);

    بنية atadrive_s *adrive_gf = container_جو(
        op->drive_fl, بنية atadrive_s, drive);
    بنية ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    بنية ata_pio_command cmd;
    memset(&cmd, 0, حجم(cmd));
    cmd.lba_mid = blocksize;
    cmd.lba_مٿاهون = blocksize >> 8;
    cmd.command = ATA_CMD_PACKET;

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    صحيح ret = send_cmd(adrive_gf, &cmd);
    إذا (ret)
        اذهب fail;
    ret = ata_wait_معلومات(iobase1);
    إذا (ret)
        اذهب fail;

    // Send command to device
    outsw_fl(iobase1, MAKE_FLATPTR(GET_SEG(SS), cdbcmd), CDROM_CDB_سائيز / 2);

    صحيح حالت = pause_await_not_bsy(iobase1, iobase2);
    إذا (حالت < 0) {
        ret = حالت;
        اذهب fail;
    }

    إذا (حالت & ATA_CB_STAT_ERR) {
        u8 err = inb(iobase1 + ATA_CB_ERR);
        // skip "Not Ready"
        إذا (err != 0x20)
            dprintf(6, "send_atapi_cmd : read error (status=%02x err=%02x)\n"
                    , حالت, err);
        ret = -2;
        اذهب fail;
    }
    إذا (blocksize) {
        إذا (!(حالت & ATA_CB_STAT_DRQ)) {
            dprintf(6, "send_atapi_cmd : DRQ not set (status %02x)\n", حالت);
            ret = -3;
            اذهب fail;
        }

        ret = ata_pio_transfer(op, 0, blocksize);
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    إذا (ret)
        أرجع DISK_RET_EBADTRACK;
    أرجع DISK_RET_SUCCESS;
}


/****************************************************************
 * ATA detect and init
 ****************************************************************/

// Send an identify device or identify device packet command.
ساكن صحيح
send_ata_سڃاڻپ_عنصر(بنية atadrive_s *adrive, u16 *بفر, صحيح command)
{
    memset(بفر, 0, DISK_SECTOR_سائيز);

    بنية disk_op_s dop;
    memset(&dop, 0, حجم(dop));
    dop.drive_fl = &adrive->drive;
    dop.ڳڻپ = 1;
    dop.lba = 1;
    dop.buf_fl = MAKE_FLATPTR(GET_SEG(SS), بفر);

    بنية ata_pio_command cmd;
    memset(&cmd, 0, حجم(cmd));
    cmd.command = command;

    أرجع ata_pio_cmd_معلومات(&dop, 0, &cmd);
}

// Extract the ATA/ATAPI version info.
صحيح
ata_ڪڍو_نسخو(u16 *بفر)
{
    // Extract ATA/ATAPI version.
    u16 ataversion = بفر[80];
    u8 نسخو_2;
    لكل (نسخو_2=15; نسخو_2>0; نسخو_2--)
        إذا (ataversion & (1<<نسخو_2))
            اكسر;
    أرجع نسخو_2;
}

#وصف_ڪريو MAXMODEL 40

// Extract the ATA/ATAPI model info.
حرف *
ata_ڪڍو_model(حرف *model, u32 سائيز, u16 *بفر)
{
    // Read model name
    صحيح i;
    لكل (i=0; i<سائيز/2; i++)
        *(u16*)&model[i*2] = be16_منزل_cpu(بفر[27+i]);
    model[سائيز] = 0x00;
    nullTrailingSpace(model);
    أرجع model;
}

// Common init code between ata and atapi
ساكن بنية atadrive_s *
init_atadrive(بنية atadrive_s *dummy, u16 *بفر)
{
    بنية atadrive_s *adrive = malloc_fseg(حجم(*adrive));
    إذا (!adrive) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(adrive, 0, حجم(*adrive));
    adrive->chan_gf = dummy->chan_gf;
    adrive->slave = dummy->slave;
    adrive->drive.cntl_id = adrive->chan_gf->ataid * 2 + dummy->slave;
    adrive->drive.removable = (بفر[0] & 0x80) ? 1 : 0;
    أرجع adrive;
}

// Detect if the given drive is an atapi - initialize it if so.
ساكن بنية atadrive_s *
init_drive_atapi(بنية atadrive_s *dummy, u16 *بفر)
{
    // Send an IDENTIFY_DEVICE_PACKET command to device
    صحيح ret = send_ata_سڃاڻپ_عنصر(dummy, بفر, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    إذا (ret)
        أرجع عدم;

    // Success - setup as ATAPI.
    بنية atadrive_s *adrive = init_atadrive(dummy, بفر);
    إذا (!adrive)
        أرجع عدم;
    adrive->drive.type = DTYPE_ATA_ATAPI;
    adrive->drive.blksize = CDROM_SECTOR_سائيز;
    adrive->drive.sectors = (u64)-1;
    u8 iscd = ((بفر[0] >> 8) & 0x1f) == 0x05;
    حرف model[MAXMODEL+1];
    حرف *desc = znprintf(MAXDESCSIZE
                          , "DVD/CD [ata%d-%d: %s ATAPI-%d %s]"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_ڪڍو_model(model, MAXMODEL, بفر)
                          , ata_ڪڍو_نسخو(بفر)
                          , (iscd ? "DVD/CD" : "Device"));
    dprintf(1, "%s\n", desc);

    // fill cdidmap
    إذا (iscd) {
        صحيح prio = bootprio_ڳوليو_ata_device(adrive->chan_gf->pci_tmp,
                                            adrive->chan_gf->chanid,
                                            adrive->slave);
        boot_lchs_ڳوليو_ata_device(adrive->chan_gf->pci_tmp,
                                  adrive->chan_gf->chanid,
                                  adrive->slave,
                                  &(adrive->drive.lchs));
        boot_جوڙيو_cd(&adrive->drive, desc, prio);
    }

    أرجع adrive;
}

// Detect if the given drive is a regular ata drive - initialize it if so.
ساكن بنية atadrive_s *
init_drive_ata(بنية atadrive_s *dummy, u16 *بفر)
{
    // Send an IDENTIFY_DEVICE command to device
    صحيح ret = send_ata_سڃاڻپ_عنصر(dummy, بفر, ATA_CMD_IDENTIFY_DEVICE);
    إذا (ret)
        أرجع عدم;

    // Success - setup as ATA.
    بنية atadrive_s *adrive = init_atadrive(dummy, بفر);
    إذا (!adrive)
        أرجع عدم;
    adrive->drive.type = DTYPE_ATA;
    adrive->drive.blksize = DISK_SECTOR_سائيز;

    adrive->drive.pchs.cylinder = بفر[1];
    adrive->drive.pchs.سر = بفر[3];
    adrive->drive.pchs.sector = بفر[6];

    u64 sectors;
    إذا (بفر[83] & (1 << 10)) // word 83 - lba48 support
        sectors = *(u64*)&بفر[100]; // word 100-103
    وإلا
        sectors = *(u32*)&بفر[60]; // word 60 and word 61
    adrive->drive.sectors = sectors;
    u64 adjsize = sectors >> 11;
    حرف adjprefix = 'M';
    إذا (adjsize >= (1 << 16)) {
        adjsize >>= 10;
        adjprefix = 'G';
    }
    حرف model[MAXMODEL+1];
    حرف *desc = znprintf(MAXDESCSIZE
                          , "ata%d-%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_ڪڍو_model(model, MAXMODEL, بفر)
                          , ata_ڪڍو_نسخو(بفر)
                          , (u32)adjsize, adjprefix);
    dprintf(1, "%s\n", desc);

    صحيح prio = bootprio_ڳوليو_ata_device(adrive->chan_gf->pci_tmp,
                                        adrive->chan_gf->chanid,
                                        adrive->slave);
    boot_lchs_ڳوليو_ata_device(adrive->chan_gf->pci_tmp,
                              adrive->chan_gf->chanid,
                              adrive->slave,
                              &(adrive->drive.lchs));
    // Register with bcv system.
    boot_جوڙيو_hd(&adrive->drive, desc, prio);

    أرجع adrive;
}

ساكن u32 Spinup_پڇاڙي;

// Wait for non-busy status and check for "floating bus" condition.
ساكن صحيح
powerup_await_non_bsy(u16 بنياد)
{
    u8 orstatus = 0;
    u8 حالت;
    لكل (;;) {
        حالت = inb(بنياد+ATA_CB_STAT);
        إذا (!(حالت & ATA_CB_STAT_BSY))
            اكسر;
        orstatus |= حالت;
        إذا (orstatus == 0xff) {
            dprintf(4, "powerup IDE floating\n");
            أرجع orstatus;
        }
        إذا (timer_check(Spinup_پڇاڙي)) {
            warn_timeout();
            أرجع -1;
        }
        yield();
    }
    dprintf(6, "powerup iobase=%x st=%x\n", بنياد, حالت);
    أرجع حالت;
}

// Detect any drives attached to a given controller.
ساكن فراغ
ata_detect(فراغ *معلومات)
{
    بنية ata_channel_s *chan_gf = معلومات;
    بنية atadrive_s dummy;
    memset(&dummy, 0, حجم(dummy));
    dummy.chan_gf = chan_gf;
    // Device detection
    صحيح didreset = 0;
    u8 slave;
    لكل (slave=0; slave<=1; slave++) {
        // Wait for not-bsy.
        u16 iobase1 = chan_gf->iobase1;
        صحيح حالت = powerup_await_non_bsy(iobase1);
        إذا (حالت < 0)
            استمر;
        u8 newdh = slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0;
        outb(newdh, iobase1+ATA_CB_DH);
        ndelay(400);
        حالت = powerup_await_non_bsy(iobase1);
        إذا (حالت < 0)
            استمر;

        // Check if ioport registers look valid.
        outb(newdh, iobase1+ATA_CB_DH);
        u8 dh = inb(iobase1+ATA_CB_DH);
        outb(0x55, iobase1+ATA_CB_SC);
        outb(0xaa, iobase1+ATA_CB_SN);
        u8 sc = inb(iobase1+ATA_CB_SC);
        u8 sn = inb(iobase1+ATA_CB_SN);
        dprintf(6, "ata_detect ata%d-%d: sc=%x sn=%x dh=%x\n"
                , chan_gf->ataid, slave, sc, sn, dh);
        إذا (sc != 0x55 || sn != 0xaa || dh != newdh)
            استمر;

        // Prepare new drive.
        dummy.slave = slave;

        // reset the channel
        إذا (!didreset) {
            ata_reset(&dummy);
            didreset = 1;
        }

        // check for ATAPI
        u16 بفر[256];
        بنية atadrive_s *adrive = init_drive_atapi(&dummy, بفر);
        إذا (!adrive) {
            // Didn't find an ATAPI drive - look for ATA drive.
            u8 st = inb(iobase1+ATA_CB_STAT);
            إذا (!st)
                // Status not set - can't be a valid drive.
                استمر;

            // Wait for RDY.
            صحيح ret = await_rdy(iobase1);
            إذا (ret < 0)
                استمر;

            // check for ATA.
            adrive = init_drive_ata(&dummy, بفر);
            إذا (!adrive)
                // No ATA drive found
                استمر;
        }

        u16 resetresult = بفر[93];
        dprintf(6, "ata_detect resetresult=%04x\n", resetresult);
        إذا (!slave && (resetresult & 0xdf61) == 0x4041)
            // resetresult looks valid and device 0 is responding to
            // device 1 requests - device 1 must not be present - skip
            // detection.
            اكسر;
    }
}

// Initialize an ata controller and detect its drives.
ساكن فراغ
init_controller(بنية pci_device *pci, صحيح chanid, صحيح irq
                , u32 port1, u32 port2, u32 master)
{
    ساكن صحيح ataid = 0;
    بنية ata_channel_s *chan_gf = malloc_fseg(حجم(*chan_gf));
    إذا (!chan_gf) {
        warn_noalloc();
        أرجع;
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
    لاڳيتو_thread(ata_detect, chan_gf);
}

#وصف_ڪريو IRQ_ATA1 14
#وصف_ڪريو IRQ_ATA2 15

// Handle controllers on an ATA PCI device.
ساكن فراغ
init_pciata(بنية pci_device *pci, u8 prog_if)
{
    u8 pciirq = pci_config_readb(pci->bdf, PCI_مداخلت_سٽ);
    صحيح master = 0;
    إذا (CONFIG_ATA_DMA && prog_if & 0x80) {
        // Check for bus-mastering.
        u32 bar = pci_config_readl(pci->bdf, PCI_بنياد_پتو_4);
        إذا (bar & PCI_بنياد_پتو_SPACE_IO) {
            master = pci_enable_iobar(pci, PCI_بنياد_پتو_4);
            pci_enable_busmaster(pci);
        }
    }

    u32 port1, port2, irq;
    إذا (prog_if & 1) {
        port1 = pci_enable_iobar(pci, PCI_بنياد_پتو_0);
        port2 = pci_enable_iobar(pci, PCI_بنياد_پتو_1);
        إذا (!port1 || !port2)
            أرجع;
        irq = pciirq;
    } وإلا {
        port1 = پورٽ_ATA1_CMD_بنياد;
        port2 = پورٽ_ATA1_CTRL_بنياد;
        irq = IRQ_ATA1;
    }
    init_controller(pci, 0, irq, port1, port2, master);

    إذا (prog_if & 4) {
        port1 = pci_enable_iobar(pci, PCI_بنياد_پتو_2);
        port2 = pci_enable_iobar(pci, PCI_بنياد_پتو_3);
        إذا (!port1 || !port2)
            أرجع;
        irq = pciirq;
    } وإلا {
        port1 = پورٽ_ATA2_CMD_بنياد;
        port2 = پورٽ_ATA2_CTRL_بنياد;
        irq = IRQ_ATA2;
    }
    init_controller(pci, 1, irq, port1, port2, master ? master + 8 : 0);
}

ساكن فراغ
مليو_genericata(بنية pci_device *pci, فراغ *arg)
{
    init_pciata(pci, pci->prog_if);
}

ساكن فراغ
مليو_compatibleahci(بنية pci_device *pci, فراغ *arg)
{
    إذا (CONFIG_AHCI)
        // Already handled directly via native ahci interface.
        أرجع;
    init_pciata(pci, 0x8f);
}

ساكن ثابت بنية pci_device_id pci_ata_tbl[] = {
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE
                     , مليو_genericata),
    PCI_DEVICE(PCI_VENDOR_ID_ATI, 0x4391, مليو_compatibleahci),
    PCI_DEVICE_پڇاڙي,
};

// Locate and init ata controllers.
ساكن فراغ
ata_جاچ(فراغ)
{
    إذا (CONFIG_QEMU && hlist_خالي_جايون(&PCIDevices)) {
        // No PCI devices found - probably a QEMU "-M isapc" machine.
        // Try using ISA ports for ATA controllers.
        init_controller(عدم, 0, IRQ_ATA1
                        , پورٽ_ATA1_CMD_بنياد, پورٽ_ATA1_CTRL_بنياد, 0);
        init_controller(عدم, 1, IRQ_ATA2
                        , پورٽ_ATA2_CMD_بنياد, پورٽ_ATA2_CTRL_بنياد, 0);
        أرجع;
    }

    // Scan PCI bus for ATA adapters
    بنية pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_ata_tbl, pci, عدم);
    }
}

فراغ
ata_تياري(فراغ)
{
    ASSERT32FLAT();
    إذا (!CONFIG_ATA)
        أرجع;

    dprintf(3, "init hard drives\n");

    Spinup_پڇاڙي = timer_calc(IDE_TIMEOUT);
    ata_جاچ();

    سيٽ_BDA(disk_control_byte, 0xc0);

    enable_hwirq(14, FUNC16(داخلا_76));
}

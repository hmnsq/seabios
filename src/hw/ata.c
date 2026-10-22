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

#define IDE_TIMEOUT 32000 //32 seconds max for IDE ops


/****************************************************************
 * Helper functions
 ****************************************************************/

// Wait for the specified ide state
ساكن مضمن صحيح
await_ide(u8 mask, u8 flags, u16 base, u16 timeout)
{
    u32 end = timer_calc(timeout);
    لكل (;;) {
        u8 status = inb(base+ATA_CB_STAT);
        𐤀𐤌 ((status & mask) == flags)
            أرجع status;
        𐤀𐤌 (timer_check(end)) {
            warn_timeout();
            أرجع -1;
        }
        yield();
    }
}

// Wait for the device to be not-busy.
ساكن صحيح
await_not_bsy(u16 base)
{
    أرجع await_ide(ATA_CB_STAT_BSY, 0, base, IDE_TIMEOUT);
}

// Wait for the device to be ready.
ساكن صحيح
await_rdy(u16 base)
{
    أرجع await_ide(ATA_CB_STAT_RDY, ATA_CB_STAT_RDY, base, IDE_TIMEOUT);
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
    صحيح status = await_not_bsy(iobase1);
    𐤀𐤌 (status < 0)
        اذهب done;
    𐤀𐤌 (slave) {
        // Change device.
        u32 end = timer_calc(IDE_TIMEOUT);
        لكل (;;) {
            outb(ATA_CB_DH_DEV1, iobase1 + ATA_CB_DH);
            status = ndelay_await_not_bsy(iobase1);
            𐤀𐤌 (status < 0)
                اذهب done;
            𐤀𐤌 (inb(iobase1 + ATA_CB_DH) == ATA_CB_DH_DEV1)
                اكسر;
            // Change drive request failed to take effect - retry.
            𐤀𐤌 (timer_check(end)) {
                warn_timeout();
                اذهب done;
            }
        }
    } وإلا {
        // QEMU doesn't reset dh on reset, so set it explicitly.
        outb(ATA_CB_DH_DEV0, iobase1 + ATA_CB_DH);
    }

    // On a user-reset request, wait for RDY if it is an ATA device.
    u8 type=GET_GLOBALFLAT(adrive_gf->drive.type);
    𐤀𐤌 (type == DTYPE_ATA)
        status = await_rdy(iobase1);

done:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    dprintf(6, "ata_reset exit status=%x\n", status);
}

// Check for drive RDY for 16bit interface command.
ساكن صحيح
isready(بنية atadrive_s *adrive_gf)
{
    // Read the status from controller
    بنية ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u8 status = inb(iobase1 + ATA_CB_STAT);
    𐤀𐤌 ((status & (ATA_CB_STAT_BSY|ATA_CB_STAT_RDY)) == ATA_CB_STAT_RDY)
        أرجع DISK_RET_SUCCESS;
    أرجع DISK_RET_ENOTREADY;
}


/****************************************************************
 * ATA send command
 ****************************************************************/

بنية ata_pio_command {
    u8 feature;
    u8 sector_count;
    u8 lba_low;
    u8 lba_mid;
    u8 lba_high;
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
    صحيح status = await_not_bsy(iobase1);
    𐤀𐤌 (status < 0)
        أرجع status;
    u8 newdh = ((cmd->device & ~ATA_CB_DH_DEV1)
                | (slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0));
    u8 olddh = inb(iobase1 + ATA_CB_DH);
    outb(newdh, iobase1 + ATA_CB_DH);
    𐤀𐤌 ((olddh ^ newdh) & (1<<4)) {
        // Was a device change - wait for device to become not busy.
        status = ndelay_await_not_bsy(iobase1);
        𐤀𐤌 (status < 0)
            أرجع status;
    }

    // Check for ATA_CMD_(READ|WRITE)_(SECTORS|DMA)_EXT commands.
    𐤀𐤌 ((cmd->command & ~0x11) == ATA_CMD_READ_SECTORS_EXT) {
        outb(cmd->feature2, iobase1 + ATA_CB_FR);
        outb(cmd->sector_count2, iobase1 + ATA_CB_SC);
        outb(cmd->lba_low2, iobase1 + ATA_CB_SN);
        outb(cmd->lba_mid2, iobase1 + ATA_CB_CL);
        outb(cmd->lba_high2, iobase1 + ATA_CB_CH);
    }
    outb(cmd->feature, iobase1 + ATA_CB_FR);
    outb(cmd->sector_count, iobase1 + ATA_CB_SC);
    outb(cmd->lba_low, iobase1 + ATA_CB_SN);
    outb(cmd->lba_mid, iobase1 + ATA_CB_CL);
    outb(cmd->lba_high, iobase1 + ATA_CB_CH);
    outb(cmd->command, iobase1 + ATA_CB_CMD);

    أرجع 0;
}

// Wait for data after calling 'send_cmd'.
ساكن صحيح
ata_wait_data(u16 iobase1)
{
    صحيح status = ndelay_await_not_bsy(iobase1);
    𐤀𐤌 (status < 0)
        أرجع status;

    𐤀𐤌 (status & ATA_CB_STAT_ERR) {
        dprintf(6, "send_cmd : read error (status=%02x err=%02x)\n"
                , status, inb(iobase1 + ATA_CB_ERR));
        أرجع -4;
    }
    𐤀𐤌 (!(status & ATA_CB_STAT_DRQ)) {
        dprintf(6, "send_cmd : DRQ not set (status %02x)\n", status);
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
    𐤀𐤌 (ret)
        اذهب fail;
    ret = ndelay_await_not_bsy(iobase1);
    𐤀𐤌 (ret < 0)
        اذهب fail;

    𐤀𐤌 (ret & ATA_CB_STAT_ERR) {
        dprintf(6, "nondata cmd : read error (status=%02x err=%02x)\n"
                , ret, inb(iobase1 + ATA_CB_ERR));
        ret = -4;
        اذهب fail;
    }
    𐤀𐤌 (ret & ATA_CB_STAT_DRQ) {
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
            , op->drive_fl, iswrite, op->count, blocksize, op->buf_fl);

    بنية atadrive_s *adrive_gf = container_of(
        op->drive_fl, بنية atadrive_s, drive);
    بنية ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    صحيح count = op->count;
    فراغ *buf_fl = op->buf_fl;
    صحيح status;
    لكل (;;) {
        𐤀𐤌 (iswrite) {
            // Write data to controller
            dprintf(16, "Write sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            𐤀𐤌 (CONFIG_ATA_PIO32)
                outsl_fl(iobase1, buf_fl, blocksize / 4);
            وإلا
                outsw_fl(iobase1, buf_fl, blocksize / 2);
        } وإلا {
            // Read data from controller
            dprintf(16, "Read sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            𐤀𐤌 (CONFIG_ATA_PIO32)
                insl_fl(iobase1, buf_fl, blocksize / 4);
            وإلا
                insw_fl(iobase1, buf_fl, blocksize / 2);
        }
        buf_fl += blocksize;

        status = pause_await_not_bsy(iobase1, iobase2);
        𐤀𐤌 (status < 0) {
            // Error
            op->count -= count;
            أرجع status;
        }

        count--;
        𐤀𐤌 (!count)
            اكسر;
        status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ | ATA_CB_STAT_ERR);
        𐤀𐤌 (status != ATA_CB_STAT_DRQ) {
            dprintf(6, "ata_pio_transfer : more sectors left (status %02x)\n"
                    , status);
            op->count -= count;
            أرجع -6;
        }
    }

    status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
               | ATA_CB_STAT_ERR);
    𐤀𐤌 (!iswrite)
        status &= ~ATA_CB_STAT_DF;
    𐤀𐤌 (status != 0) {
        dprintf(6, "ata_pio_transfer : no sectors left (status %02x)\n", status);
        أرجع -7;
    }

    أرجع 0;
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

بنية sff_dma_prd {
    u32 buf_fl;
    u32 count;
};

// Check if DMA available and setup transfer if so.
ساكن صحيح
ata_try_dma(بنية disk_op_s *op, صحيح iswrite, صحيح blocksize)
{
    ASSERT16();
    𐤀𐤌 (! CONFIG_ATA_DMA)
        أرجع -1;
    u32 dest = (u32)op->buf_fl;
    𐤀𐤌 (dest & 1)
        // Need minimum alignment of 1.
        أرجع -1;
    بنية atadrive_s *adrive_gf = container_of(
        op->drive_fl, بنية atadrive_s, drive);
    بنية ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);
    𐤀𐤌 (! iomaster)
        أرجع -1;
    u32 bytes = op->count * blocksize;
    𐤀𐤌 (! bytes)
        أرجع -1;

    // Build PRD dma structure.
    بنية sff_dma_prd *dma = MAKE_FLATPTR(SEG_LOW, ExtraStack);
    بنية sff_dma_prd *origdma = dma;
    طالما (bytes) {
        𐤀𐤌 (dma >= &origdma[16])
            // Too many descriptors..
            أرجع -1;
        u32 count = bytes;
        u32 max = 0x10000 - (dest & 0xffff);
        𐤀𐤌 (count > max)
            count = max;

        SET_LOWFLAT(dma->buf_fl, dest);
        bytes -= count;
        𐤀𐤌 (!bytes)
            // Last descriptor.
            count |= 1<<31;
        dprintf(16, "dma@%p: %08x %08x\n", dma, dest, count);
        dest += count;
        SET_LOWFLAT(dma->count, count);
        dma++;
    }

    // Program bus-master controller.
    outl((u32)origdma, iomaster + BM_TABLE);
    u8 oldcmd = inb(iomaster + BM_CMD) & ~(BM_CMD_MEMWRITE|BM_CMD_START);
    outb(oldcmd | (iswrite ? 0x00 : BM_CMD_MEMWRITE), iomaster + BM_CMD);
    outb(BM_STATUS_ERROR|BM_STATUS_IRQ, iomaster + BM_STATUS);

    أرجع 0;
}

// Transfer data using DMA.
ساكن صحيح
ata_dma_transfer(بنية disk_op_s *op)
{
    𐤀𐤌 (! CONFIG_ATA_DMA)
        أرجع -1;
    dprintf(16, "ata_dma_transfer id=%p buf=%p\n", op->drive_fl, op->buf_fl);

    بنية atadrive_s *adrive_gf = container_of(
        op->drive_fl, بنية atadrive_s, drive);
    بنية ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);

    // Start bus-master controller.
    u8 oldcmd = inb(iomaster + BM_CMD);
    outb(oldcmd | BM_CMD_START, iomaster + BM_CMD);

    u32 end = timer_calc(IDE_TIMEOUT);
    u8 status;
    لكل (;;) {
        status = inb(iomaster + BM_STATUS);
        𐤀𐤌 (status & BM_STATUS_IRQ)
            اكسر;
        // Transfer in progress
        𐤀𐤌 (timer_check(end)) {
            // Timeout.
            warn_timeout();
            اكسر;
        }
        yield();
    }
    outb(oldcmd & ~BM_CMD_START, iomaster + BM_CMD);

    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    صحيح idestatus = pause_await_not_bsy(iobase1, iobase2);

    𐤀𐤌 ((status & (BM_STATUS_IRQ|BM_STATUS_ACTIVE)) == BM_STATUS_IRQ
        && idestatus >= 0x00
        && (idestatus & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
                         | ATA_CB_STAT_ERR)) == 0x00)
        // Success.
        أرجع 0;

    dprintf(6, "IDE DMA error (dma=%x ide=%x/%x/%x)\n", status, idestatus
            , inb(iobase2 + ATA_CB_ASTAT), inb(iobase1 + ATA_CB_ERR));
    أرجع -1;
}


/****************************************************************
 * ATA hard drive functions
 ****************************************************************/

// Transfer data to harddrive using PIO protocol.
ساكن صحيح
ata_pio_cmd_data(بنية disk_op_s *op, صحيح iswrite, بنية ata_pio_command *cmd)
{
    بنية atadrive_s *adrive_gf = container_of(
        op->drive_fl, بنية atadrive_s, drive);
    بنية ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    صحيح ret = send_cmd(adrive_gf, cmd);
    𐤀𐤌 (ret)
        اذهب fail;
    ret = ata_wait_data(iobase1);
    𐤀𐤌 (ret)
        اذهب fail;
    ret = ata_pio_transfer(op, iswrite, DISK_SECTOR_SIZE);

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    أرجع ret;
}

// Transfer data to harddrive using DMA protocol.
ساكن صحيح
ata_dma_cmd_data(بنية disk_op_s *op, بنية ata_pio_command *cmd)
{
    𐤀𐤌 (! CONFIG_ATA_DMA)
        أرجع -1;
    بنية atadrive_s *adrive_gf = container_of(
        op->drive_fl, بنية atadrive_s, drive);
    صحيح ret = send_cmd(adrive_gf, cmd);
    𐤀𐤌 (ret)
        أرجع ret;
    أرجع ata_dma_transfer(op);
}

// Read/write count blocks from a harddrive.
ساكن صحيح
ata_readwrite(بنية disk_op_s *op, صحيح iswrite)
{
    u64 lba = op->lba;

    صحيح usepio = ata_try_dma(op, iswrite, DISK_SECTOR_SIZE);

    بنية ata_pio_command cmd;
    memset(&cmd, 0, حجم(cmd));

    𐤀𐤌 (op->count >= (1<<8) || lba + op->count >= (1<<28)) {
        cmd.sector_count2 = op->count >> 8;
        cmd.lba_low2 = lba >> 24;
        cmd.lba_mid2 = lba >> 32;
        cmd.lba_high2 = lba >> 40;
        lba &= 0xffffff;

        𐤀𐤌 (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS_EXT
                           : ATA_CMD_READ_SECTORS_EXT);
        وإلا
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                           : ATA_CMD_READ_DMA_EXT);
    } وإلا {
        𐤀𐤌 (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS
                           : ATA_CMD_READ_SECTORS);
        وإلا
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA
                           : ATA_CMD_READ_DMA);
    }

    cmd.sector_count = op->count;
    cmd.lba_low = lba;
    cmd.lba_mid = lba >> 8;
    cmd.lba_high = lba >> 16;
    cmd.device = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;

    صحيح ret;
    𐤀𐤌 (usepio)
        ret = ata_pio_cmd_data(op, iswrite, &cmd);
    وإلا
        ret = ata_dma_cmd_data(op, &cmd);
    𐤀𐤌 (ret)
        أرجع DISK_RET_EBADTRACK;
    أرجع DISK_RET_SUCCESS;
}

// 16bit command demuxer for ATA harddrives.
صحيح
ata_process_op(بنية disk_op_s *op)
{
    𐤀𐤌 (!CONFIG_ATA)
        أرجع 0;

    بنية atadrive_s *adrive_gf = container_of(
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

#define CDROM_CDB_SIZE 12

// Low-level atapi command transmit function.
صحيح
ata_atapi_process_op(بنية disk_op_s *op)
{
    𐤀𐤌 (! CONFIG_ATA)
        أرجع 0;

    𐤀𐤌 (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        أرجع DISK_RET_EWRITEPROTECT;
    u8 cdbcmd[CDROM_CDB_SIZE];
    صحيح blocksize = scsi_fill_cmd(op, cdbcmd, حجم(cdbcmd));
    𐤀𐤌 (blocksize < 0)
        أرجع default_process_op(op);

    بنية atadrive_s *adrive_gf = container_of(
        op->drive_fl, بنية atadrive_s, drive);
    بنية ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    بنية ata_pio_command cmd;
    memset(&cmd, 0, حجم(cmd));
    cmd.lba_mid = blocksize;
    cmd.lba_high = blocksize >> 8;
    cmd.command = ATA_CMD_PACKET;

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    صحيح ret = send_cmd(adrive_gf, &cmd);
    𐤀𐤌 (ret)
        اذهب fail;
    ret = ata_wait_data(iobase1);
    𐤀𐤌 (ret)
        اذهب fail;

    // Send command to device
    outsw_fl(iobase1, MAKE_FLATPTR(GET_SEG(SS), cdbcmd), CDROM_CDB_SIZE / 2);

    صحيح status = pause_await_not_bsy(iobase1, iobase2);
    𐤀𐤌 (status < 0) {
        ret = status;
        اذهب fail;
    }

    𐤀𐤌 (status & ATA_CB_STAT_ERR) {
        u8 err = inb(iobase1 + ATA_CB_ERR);
        // skip "Not Ready"
        𐤀𐤌 (err != 0x20)
            dprintf(6, "send_atapi_cmd : read error (status=%02x err=%02x)\n"
                    , status, err);
        ret = -2;
        اذهب fail;
    }
    𐤀𐤌 (blocksize) {
        𐤀𐤌 (!(status & ATA_CB_STAT_DRQ)) {
            dprintf(6, "send_atapi_cmd : DRQ not set (status %02x)\n", status);
            ret = -3;
            اذهب fail;
        }

        ret = ata_pio_transfer(op, 0, blocksize);
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    𐤀𐤌 (ret)
        أرجع DISK_RET_EBADTRACK;
    أرجع DISK_RET_SUCCESS;
}


/****************************************************************
 * ATA detect and init
 ****************************************************************/

// Send an identify device or identify device packet command.
ساكن صحيح
send_ata_identity(بنية atadrive_s *adrive, u16 *buffer, صحيح command)
{
    memset(buffer, 0, DISK_SECTOR_SIZE);

    بنية disk_op_s dop;
    memset(&dop, 0, حجم(dop));
    dop.drive_fl = &adrive->drive;
    dop.count = 1;
    dop.lba = 1;
    dop.buf_fl = MAKE_FLATPTR(GET_SEG(SS), buffer);

    بنية ata_pio_command cmd;
    memset(&cmd, 0, حجم(cmd));
    cmd.command = command;

    أرجع ata_pio_cmd_data(&dop, 0, &cmd);
}

// Extract the ATA/ATAPI version info.
صحيح
ata_extract_version(u16 *buffer)
{
    // Extract ATA/ATAPI version.
    u16 ataversion = buffer[80];
    u8 version;
    لكل (version=15; version>0; version--)
        𐤀𐤌 (ataversion & (1<<version))
            اكسر;
    أرجع version;
}

#define MAXMODEL 40

// Extract the ATA/ATAPI model info.
حرف *
ata_extract_model(حرف *model, u32 size, u16 *buffer)
{
    // Read model name
    صحيح i;
    لكل (i=0; i<size/2; i++)
        *(u16*)&model[i*2] = be16_to_cpu(buffer[27+i]);
    model[size] = 0x00;
    nullTrailingSpace(model);
    أرجع model;
}

// Common init code between ata and atapi
ساكن بنية atadrive_s *
init_atadrive(بنية atadrive_s *dummy, u16 *buffer)
{
    بنية atadrive_s *adrive = malloc_fseg(حجم(*adrive));
    𐤀𐤌 (!adrive) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(adrive, 0, حجم(*adrive));
    adrive->chan_gf = dummy->chan_gf;
    adrive->slave = dummy->slave;
    adrive->drive.cntl_id = adrive->chan_gf->ataid * 2 + dummy->slave;
    adrive->drive.removable = (buffer[0] & 0x80) ? 1 : 0;
    أرجع adrive;
}

// Detect if the given drive is an atapi - initialize it if so.
ساكن بنية atadrive_s *
init_drive_atapi(بنية atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE_PACKET command to device
    صحيح ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    𐤀𐤌 (ret)
        أرجع عدم;

    // Success - setup as ATAPI.
    بنية atadrive_s *adrive = init_atadrive(dummy, buffer);
    𐤀𐤌 (!adrive)
        أرجع عدم;
    adrive->drive.type = DTYPE_ATA_ATAPI;
    adrive->drive.blksize = CDROM_SECTOR_SIZE;
    adrive->drive.sectors = (u64)-1;
    u8 iscd = ((buffer[0] >> 8) & 0x1f) == 0x05;
    حرف model[MAXMODEL+1];
    حرف *desc = znprintf(MAXDESCSIZE
                          , "DVD/CD [ata%d-%d: %s ATAPI-%d %s]"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (iscd ? "DVD/CD" : "Device"));
    dprintf(1, "%s\n", desc);

    // fill cdidmap
    𐤀𐤌 (iscd) {
        صحيح prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                            adrive->chan_gf->chanid,
                                            adrive->slave);
        boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                                  adrive->chan_gf->chanid,
                                  adrive->slave,
                                  &(adrive->drive.lchs));
        boot_add_cd(&adrive->drive, desc, prio);
    }

    أرجع adrive;
}

// Detect if the given drive is a regular ata drive - initialize it if so.
ساكن بنية atadrive_s *
init_drive_ata(بنية atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE command to device
    صحيح ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_DEVICE);
    𐤀𐤌 (ret)
        أرجع عدم;

    // Success - setup as ATA.
    بنية atadrive_s *adrive = init_atadrive(dummy, buffer);
    𐤀𐤌 (!adrive)
        أرجع عدم;
    adrive->drive.type = DTYPE_ATA;
    adrive->drive.blksize = DISK_SECTOR_SIZE;

    adrive->drive.pchs.cylinder = buffer[1];
    adrive->drive.pchs.head = buffer[3];
    adrive->drive.pchs.sector = buffer[6];

    u64 sectors;
    𐤀𐤌 (buffer[83] & (1 << 10)) // word 83 - lba48 support
        sectors = *(u64*)&buffer[100]; // word 100-103
    وإلا
        sectors = *(u32*)&buffer[60]; // word 60 and word 61
    adrive->drive.sectors = sectors;
    u64 adjsize = sectors >> 11;
    حرف adjprefix = 'M';
    𐤀𐤌 (adjsize >= (1 << 16)) {
        adjsize >>= 10;
        adjprefix = 'G';
    }
    حرف model[MAXMODEL+1];
    حرف *desc = znprintf(MAXDESCSIZE
                          , "ata%d-%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (u32)adjsize, adjprefix);
    dprintf(1, "%s\n", desc);

    صحيح prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                        adrive->chan_gf->chanid,
                                        adrive->slave);
    boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                              adrive->chan_gf->chanid,
                              adrive->slave,
                              &(adrive->drive.lchs));
    // Register with bcv system.
    boot_add_hd(&adrive->drive, desc, prio);

    أرجع adrive;
}

ساكن u32 SpinupEnd;

// Wait for non-busy status and check for "floating bus" condition.
ساكن صحيح
powerup_await_non_bsy(u16 base)
{
    u8 orstatus = 0;
    u8 status;
    لكل (;;) {
        status = inb(base+ATA_CB_STAT);
        𐤀𐤌 (!(status & ATA_CB_STAT_BSY))
            اكسر;
        orstatus |= status;
        𐤀𐤌 (orstatus == 0xff) {
            dprintf(4, "powerup IDE floating\n");
            أرجع orstatus;
        }
        𐤀𐤌 (timer_check(SpinupEnd)) {
            warn_timeout();
            أرجع -1;
        }
        yield();
    }
    dprintf(6, "powerup iobase=%x st=%x\n", base, status);
    أرجع status;
}

// Detect any drives attached to a given controller.
ساكن فراغ
ata_detect(فراغ *data)
{
    بنية ata_channel_s *chan_gf = data;
    بنية atadrive_s dummy;
    memset(&dummy, 0, حجم(dummy));
    dummy.chan_gf = chan_gf;
    // Device detection
    صحيح didreset = 0;
    u8 slave;
    لكل (slave=0; slave<=1; slave++) {
        // Wait for not-bsy.
        u16 iobase1 = chan_gf->iobase1;
        صحيح status = powerup_await_non_bsy(iobase1);
        𐤀𐤌 (status < 0)
            استمر;
        u8 newdh = slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0;
        outb(newdh, iobase1+ATA_CB_DH);
        ndelay(400);
        status = powerup_await_non_bsy(iobase1);
        𐤀𐤌 (status < 0)
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
        𐤀𐤌 (sc != 0x55 || sn != 0xaa || dh != newdh)
            استمر;

        // Prepare new drive.
        dummy.slave = slave;

        // reset the channel
        𐤀𐤌 (!didreset) {
            ata_reset(&dummy);
            didreset = 1;
        }

        // check for ATAPI
        u16 buffer[256];
        بنية atadrive_s *adrive = init_drive_atapi(&dummy, buffer);
        𐤀𐤌 (!adrive) {
            // Didn't find an ATAPI drive - look for ATA drive.
            u8 st = inb(iobase1+ATA_CB_STAT);
            𐤀𐤌 (!st)
                // Status not set - can't be a valid drive.
                استمر;

            // Wait for RDY.
            صحيح ret = await_rdy(iobase1);
            𐤀𐤌 (ret < 0)
                استمر;

            // check for ATA.
            adrive = init_drive_ata(&dummy, buffer);
            𐤀𐤌 (!adrive)
                // No ATA drive found
                استمر;
        }

        u16 resetresult = buffer[93];
        dprintf(6, "ata_detect resetresult=%04x\n", resetresult);
        𐤀𐤌 (!slave && (resetresult & 0xdf61) == 0x4041)
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
    𐤀𐤌 (!chan_gf) {
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
    run_thread(ata_detect, chan_gf);
}

#define IRQ_ATA1 14
#define IRQ_ATA2 15

// Handle controllers on an ATA PCI device.
ساكن فراغ
init_pciata(بنية pci_device *pci, u8 prog_if)
{
    u8 pciirq = pci_config_readb(pci->bdf, PCI_INTERRUPT_LINE);
    صحيح master = 0;
    𐤀𐤌 (CONFIG_ATA_DMA && prog_if & 0x80) {
        // Check for bus-mastering.
        u32 bar = pci_config_readl(pci->bdf, PCI_BASE_ADDRESS_4);
        𐤀𐤌 (bar & PCI_BASE_ADDRESS_SPACE_IO) {
            master = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
            pci_enable_busmaster(pci);
        }
    }

    u32 port1, port2, irq;
    𐤀𐤌 (prog_if & 1) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_1);
        𐤀𐤌 (!port1 || !port2)
            أرجع;
        irq = pciirq;
    } وإلا {
        port1 = PORT_ATA1_CMD_BASE;
        port2 = PORT_ATA1_CTRL_BASE;
        irq = IRQ_ATA1;
    }
    init_controller(pci, 0, irq, port1, port2, master);

    𐤀𐤌 (prog_if & 4) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_2);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_3);
        𐤀𐤌 (!port1 || !port2)
            أرجع;
        irq = pciirq;
    } وإلا {
        port1 = PORT_ATA2_CMD_BASE;
        port2 = PORT_ATA2_CTRL_BASE;
        irq = IRQ_ATA2;
    }
    init_controller(pci, 1, irq, port1, port2, master ? master + 8 : 0);
}

ساكن فراغ
found_genericata(بنية pci_device *pci, فراغ *arg)
{
    init_pciata(pci, pci->prog_if);
}

ساكن فراغ
found_compatibleahci(بنية pci_device *pci, فراغ *arg)
{
    𐤀𐤌 (CONFIG_AHCI)
        // Already handled directly via native ahci interface.
        أرجع;
    init_pciata(pci, 0x8f);
}

ساكن ثابت بنية pci_device_id pci_ata_tbl[] = {
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE
                     , found_genericata),
    PCI_DEVICE(PCI_VENDOR_ID_ATI, 0x4391, found_compatibleahci),
    PCI_DEVICE_END,
};

// Locate and init ata controllers.
ساكن فراغ
ata_scan(فراغ)
{
    𐤀𐤌 (CONFIG_QEMU && hlist_empty(&PCIDevices)) {
        // No PCI devices found - probably a QEMU "-M isapc" machine.
        // Try using ISA ports for ATA controllers.
        init_controller(عدم, 0, IRQ_ATA1
                        , PORT_ATA1_CMD_BASE, PORT_ATA1_CTRL_BASE, 0);
        init_controller(عدم, 1, IRQ_ATA2
                        , PORT_ATA2_CMD_BASE, PORT_ATA2_CTRL_BASE, 0);
        أرجع;
    }

    // Scan PCI bus for ATA adapters
    بنية pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_ata_tbl, pci, عدم);
    }
}

فراغ
ata_setup(فراغ)
{
    ASSERT32FLAT();
    𐤀𐤌 (!CONFIG_ATA)
        أرجع;

    dprintf(3, "init hard drives\n");

    SpinupEnd = timer_calc(IDE_TIMEOUT);
    ata_scan();

    SET_BDA(disk_control_byte, 0xc0);

    enable_hwirq(14, FUNC16(entry_76));
}

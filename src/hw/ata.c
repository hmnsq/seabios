// Low level ATA disk access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "ata.h" // ATA_CB_STAT
#அந்தர்பாவ "biosvar.h" // GET_GLOBALFLAT
#அந்தர்பாவ "block.h" // struct drive_s
#அந்தர்பாவ "blockcmd.h" // CDB_CMD_READ_10
#அந்தர்பாவ "byteorder.h" // be16_to_cpu
#அந்தர்பாவ "malloc.h" // malloc_fseg
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "pci.h" // pci_config_readb
#அந்தர்பாவ "pcidevice.h" // foreachpci
#அந்தர்பாவ "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#அந்தர்பாவ "pci_regs.h" // PCI_INTERRUPT_LINE
#அந்தர்பாவ "pic.h" // enable_hwirq
#அந்தர்பாவ "stacks.h" // yield
#அந்தர்பாவ "std/disk.h" // DISK_RET_SUCCESS
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // timer_calc
#அந்தர்பாவ "x86.h" // inb

#define IDE_TIMEOUT 32000 //32 seconds max for IDE ops


/****************************************************************
 * Helper functions
 ****************************************************************/

// Wait for the specified ide state
நிலையான உள்வரி முழு
await_ide(u8 mask, u8 flags, u16 base, u16 timeout)
{
    u32 end = timer_calc(timeout);
    க்கு (;;) {
        u8 status = inb(base+ATA_CB_STAT);
        எனில் ((status & mask) == flags)
            திரும்பு status;
        எனில் (timer_check(end)) {
            warn_timeout();
            திரும்பு -1;
        }
        yield();
    }
}

// Wait for the device to be not-busy.
நிலையான முழு
await_not_bsy(u16 base)
{
    திரும்பு await_ide(ATA_CB_STAT_BSY, 0, base, IDE_TIMEOUT);
}

// Wait for the device to be ready.
நிலையான முழு
await_rdy(u16 base)
{
    திரும்பு await_ide(ATA_CB_STAT_RDY, ATA_CB_STAT_RDY, base, IDE_TIMEOUT);
}

// Wait for ide state - pauses for one ata cycle first.
நிலையான உள்வரி முழு
pause_await_not_bsy(u16 iobase1, u16 iobase2)
{
    // Wait one PIO transfer cycle.
    inb(iobase2 + ATA_CB_ASTAT);

    திரும்பு await_not_bsy(iobase1);
}

// Wait for ide state - pause for 400ns first.
நிலையான உள்வரி முழு
ndelay_await_not_bsy(u16 iobase1)
{
    ndelay(400);
    திரும்பு await_not_bsy(iobase1);
}

// Reset a drive
நிலையான வெறுமை
ata_reset(கட்டமைப்பு atadrive_s *adrive_gf)
{
    கட்டமைப்பு ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
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
    முழு status = await_not_bsy(iobase1);
    எனில் (status < 0)
        செல் done;
    எனில் (slave) {
        // Change device.
        u32 end = timer_calc(IDE_TIMEOUT);
        க்கு (;;) {
            outb(ATA_CB_DH_DEV1, iobase1 + ATA_CB_DH);
            status = ndelay_await_not_bsy(iobase1);
            எனில் (status < 0)
                செல் done;
            எனில் (inb(iobase1 + ATA_CB_DH) == ATA_CB_DH_DEV1)
                நிறுத்து;
            // Change drive request failed to take effect - retry.
            எனில் (timer_check(end)) {
                warn_timeout();
                செல் done;
            }
        }
    } இல்லையெனில் {
        // QEMU doesn't reset dh on reset, so set it explicitly.
        outb(ATA_CB_DH_DEV0, iobase1 + ATA_CB_DH);
    }

    // On a user-reset request, wait for RDY if it is an ATA device.
    u8 type=GET_GLOBALFLAT(adrive_gf->drive.type);
    எனில் (type == DTYPE_ATA)
        status = await_rdy(iobase1);

done:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    dprintf(6, "ata_reset exit status=%x\n", status);
}

// Check for drive RDY for 16bit interface command.
நிலையான முழு
isready(கட்டமைப்பு atadrive_s *adrive_gf)
{
    // Read the status from controller
    கட்டமைப்பு ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u8 status = inb(iobase1 + ATA_CB_STAT);
    எனில் ((status & (ATA_CB_STAT_BSY|ATA_CB_STAT_RDY)) == ATA_CB_STAT_RDY)
        திரும்பு DISK_RET_SUCCESS;
    திரும்பு DISK_RET_ENOTREADY;
}


/****************************************************************
 * ATA send command
 ****************************************************************/

கட்டமைப்பு ata_pio_command {
    u8 feature;
    u8 sector_மொத்தம்;
    u8 lba_கீழ்;
    u8 lba_mid;
    u8 lba_உயர்வு;
    u8 device;
    u8 command;

    u8 feature2;
    u8 sector_count2;
    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
};

// Send an ata command to the drive.
நிலையான முழு
send_cmd(கட்டமைப்பு atadrive_s *adrive_gf, கட்டமைப்பு ata_pio_command *cmd)
{
    கட்டமைப்பு ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);

    // Select device
    முழு status = await_not_bsy(iobase1);
    எனில் (status < 0)
        திரும்பு status;
    u8 newdh = ((cmd->device & ~ATA_CB_DH_DEV1)
                | (slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0));
    u8 olddh = inb(iobase1 + ATA_CB_DH);
    outb(newdh, iobase1 + ATA_CB_DH);
    எனில் ((olddh ^ newdh) & (1<<4)) {
        // Was a device change - wait for device to become not busy.
        status = ndelay_await_not_bsy(iobase1);
        எனில் (status < 0)
            திரும்பு status;
    }

    // Check for ATA_CMD_(READ|WRITE)_(SECTORS|DMA)_EXT commands.
    எனில் ((cmd->command & ~0x11) == ATA_CMD_READ_SECTORS_EXT) {
        outb(cmd->feature2, iobase1 + ATA_CB_FR);
        outb(cmd->sector_count2, iobase1 + ATA_CB_SC);
        outb(cmd->lba_low2, iobase1 + ATA_CB_SN);
        outb(cmd->lba_mid2, iobase1 + ATA_CB_CL);
        outb(cmd->lba_high2, iobase1 + ATA_CB_CH);
    }
    outb(cmd->feature, iobase1 + ATA_CB_FR);
    outb(cmd->sector_மொத்தம், iobase1 + ATA_CB_SC);
    outb(cmd->lba_கீழ், iobase1 + ATA_CB_SN);
    outb(cmd->lba_mid, iobase1 + ATA_CB_CL);
    outb(cmd->lba_உயர்வு, iobase1 + ATA_CB_CH);
    outb(cmd->command, iobase1 + ATA_CB_CMD);

    திரும்பு 0;
}

// Wait for data after calling 'send_cmd'.
நிலையான முழு
ata_wait_data(u16 iobase1)
{
    முழு status = ndelay_await_not_bsy(iobase1);
    எனில் (status < 0)
        திரும்பு status;

    எனில் (status & ATA_CB_STAT_ERR) {
        dprintf(6, "send_cmd : read error (status=%02x err=%02x)\n"
                , status, inb(iobase1 + ATA_CB_ERR));
        திரும்பு -4;
    }
    எனில் (!(status & ATA_CB_STAT_DRQ)) {
        dprintf(6, "send_cmd : DRQ not set (status %02x)\n", status);
        திரும்பு -5;
    }

    திரும்பு 0;
}

// Send an ata command that does not transfer any further data.
முழு
ata_cmd_nondata(கட்டமைப்பு atadrive_s *adrive_gf, கட்டமைப்பு ata_pio_command *cmd)
{
    கட்டமைப்பு ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    முழு ret = send_cmd(adrive_gf, cmd);
    எனில் (ret)
        செல் fail;
    ret = ndelay_await_not_bsy(iobase1);
    எனில் (ret < 0)
        செல் fail;

    எனில் (ret & ATA_CB_STAT_ERR) {
        dprintf(6, "nondata cmd : read error (status=%02x err=%02x)\n"
                , ret, inb(iobase1 + ATA_CB_ERR));
        ret = -4;
        செல் fail;
    }
    எனில் (ret & ATA_CB_STAT_DRQ) {
        dprintf(6, "nondata cmd : DRQ set (status %02x)\n", ret);
        ret = -5;
        செல் fail;
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    திரும்பு ret;
}


/****************************************************************
 * ATA PIO transfers
 ****************************************************************/

// Transfer 'op->count' blocks (of 'blocksize' bytes) to/from drive
// 'op->drive_fl'.
நிலையான முழு
ata_pio_transfer(கட்டமைப்பு disk_op_s *op, முழு iswrite, முழு blocksize)
{
    dprintf(16, "ata_pio_transfer id=%p write=%d count=%d bs=%d buf=%p\n"
            , op->drive_fl, iswrite, op->மொத்தம், blocksize, op->buf_fl);

    கட்டமைப்பு atadrive_s *adrive_gf = container_of(
        op->drive_fl, கட்டமைப்பு atadrive_s, drive);
    கட்டமைப்பு ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    முழு மொத்தம் = op->மொத்தம்;
    வெறுமை *buf_fl = op->buf_fl;
    முழு status;
    க்கு (;;) {
        எனில் (iswrite) {
            // Write data to controller
            dprintf(16, "Write sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            எனில் (CONFIG_ATA_PIO32)
                outsl_fl(iobase1, buf_fl, blocksize / 4);
            இல்லையெனில்
                outsw_fl(iobase1, buf_fl, blocksize / 2);
        } இல்லையெனில் {
            // Read data from controller
            dprintf(16, "Read sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            எனில் (CONFIG_ATA_PIO32)
                insl_fl(iobase1, buf_fl, blocksize / 4);
            இல்லையெனில்
                insw_fl(iobase1, buf_fl, blocksize / 2);
        }
        buf_fl += blocksize;

        status = pause_await_not_bsy(iobase1, iobase2);
        எனில் (status < 0) {
            // Error
            op->மொத்தம் -= மொத்தம்;
            திரும்பு status;
        }

        மொத்தம்--;
        எனில் (!மொத்தம்)
            நிறுத்து;
        status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ | ATA_CB_STAT_ERR);
        எனில் (status != ATA_CB_STAT_DRQ) {
            dprintf(6, "ata_pio_transfer : more sectors left (status %02x)\n"
                    , status);
            op->மொத்தம் -= மொத்தம்;
            திரும்பு -6;
        }
    }

    status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
               | ATA_CB_STAT_ERR);
    எனில் (!iswrite)
        status &= ~ATA_CB_STAT_DF;
    எனில் (status != 0) {
        dprintf(6, "ata_pio_transfer : no sectors left (status %02x)\n", status);
        திரும்பு -7;
    }

    திரும்பு 0;
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

கட்டமைப்பு sff_dma_prd {
    u32 buf_fl;
    u32 மொத்தம்;
};

// Check if DMA available and setup transfer if so.
நிலையான முழு
ata_try_dma(கட்டமைப்பு disk_op_s *op, முழு iswrite, முழு blocksize)
{
    ASSERT16();
    எனில் (! CONFIG_ATA_DMA)
        திரும்பு -1;
    u32 dest = (u32)op->buf_fl;
    எனில் (dest & 1)
        // Need minimum alignment of 1.
        திரும்பு -1;
    கட்டமைப்பு atadrive_s *adrive_gf = container_of(
        op->drive_fl, கட்டமைப்பு atadrive_s, drive);
    கட்டமைப்பு ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);
    எனில் (! iomaster)
        திரும்பு -1;
    u32 bytes = op->மொத்தம் * blocksize;
    எனில் (! bytes)
        திரும்பு -1;

    // Build PRD dma structure.
    கட்டமைப்பு sff_dma_prd *dma = MAKE_FLATPTR(SEG_கீழ், Extra_அடுக்கு);
    கட்டமைப்பு sff_dma_prd *origdma = dma;
    போது (bytes) {
        எனில் (dma >= &origdma[16])
            // Too many descriptors..
            திரும்பு -1;
        u32 மொத்தம் = bytes;
        u32 max = 0x10000 - (dest & 0xffff);
        எனில் (மொத்தம் > max)
            மொத்தம் = max;

        SET_LOWFLAT(dma->buf_fl, dest);
        bytes -= மொத்தம்;
        எனில் (!bytes)
            // Last descriptor.
            மொத்தம் |= 1<<31;
        dprintf(16, "dma@%p: %08x %08x\n", dma, dest, மொத்தம்);
        dest += மொத்தம்;
        SET_LOWFLAT(dma->மொத்தம், மொத்தம்);
        dma++;
    }

    // Program bus-master controller.
    outl((u32)origdma, iomaster + BM_TABLE);
    u8 oldcmd = inb(iomaster + BM_CMD) & ~(BM_CMD_MEMWRITE|BM_CMD_START);
    outb(oldcmd | (iswrite ? 0x00 : BM_CMD_MEMWRITE), iomaster + BM_CMD);
    outb(BM_STATUS_ERROR|BM_STATUS_IRQ, iomaster + BM_STATUS);

    திரும்பு 0;
}

// Transfer data using DMA.
நிலையான முழு
ata_dma_transfer(கட்டமைப்பு disk_op_s *op)
{
    எனில் (! CONFIG_ATA_DMA)
        திரும்பு -1;
    dprintf(16, "ata_dma_transfer id=%p buf=%p\n", op->drive_fl, op->buf_fl);

    கட்டமைப்பு atadrive_s *adrive_gf = container_of(
        op->drive_fl, கட்டமைப்பு atadrive_s, drive);
    கட்டமைப்பு ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);

    // Start bus-master controller.
    u8 oldcmd = inb(iomaster + BM_CMD);
    outb(oldcmd | BM_CMD_START, iomaster + BM_CMD);

    u32 end = timer_calc(IDE_TIMEOUT);
    u8 status;
    க்கு (;;) {
        status = inb(iomaster + BM_STATUS);
        எனில் (status & BM_STATUS_IRQ)
            நிறுத்து;
        // Transfer in progress
        எனில் (timer_check(end)) {
            // Timeout.
            warn_timeout();
            நிறுத்து;
        }
        yield();
    }
    outb(oldcmd & ~BM_CMD_START, iomaster + BM_CMD);

    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    முழு idestatus = pause_await_not_bsy(iobase1, iobase2);

    எனில் ((status & (BM_STATUS_IRQ|BM_STATUS_ACTIVE)) == BM_STATUS_IRQ
        && idestatus >= 0x00
        && (idestatus & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
                         | ATA_CB_STAT_ERR)) == 0x00)
        // Success.
        திரும்பு 0;

    dprintf(6, "IDE DMA error (dma=%x ide=%x/%x/%x)\n", status, idestatus
            , inb(iobase2 + ATA_CB_ASTAT), inb(iobase1 + ATA_CB_ERR));
    திரும்பு -1;
}


/****************************************************************
 * ATA hard drive functions
 ****************************************************************/

// Transfer data to harddrive using PIO protocol.
நிலையான முழு
ata_pio_cmd_data(கட்டமைப்பு disk_op_s *op, முழு iswrite, கட்டமைப்பு ata_pio_command *cmd)
{
    கட்டமைப்பு atadrive_s *adrive_gf = container_of(
        op->drive_fl, கட்டமைப்பு atadrive_s, drive);
    கட்டமைப்பு ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    முழு ret = send_cmd(adrive_gf, cmd);
    எனில் (ret)
        செல் fail;
    ret = ata_wait_data(iobase1);
    எனில் (ret)
        செல் fail;
    ret = ata_pio_transfer(op, iswrite, DISK_SECTOR_SIZE);

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    திரும்பு ret;
}

// Transfer data to harddrive using DMA protocol.
நிலையான முழு
ata_dma_cmd_data(கட்டமைப்பு disk_op_s *op, கட்டமைப்பு ata_pio_command *cmd)
{
    எனில் (! CONFIG_ATA_DMA)
        திரும்பு -1;
    கட்டமைப்பு atadrive_s *adrive_gf = container_of(
        op->drive_fl, கட்டமைப்பு atadrive_s, drive);
    முழு ret = send_cmd(adrive_gf, cmd);
    எனில் (ret)
        திரும்பு ret;
    திரும்பு ata_dma_transfer(op);
}

// Read/write count blocks from a harddrive.
நிலையான முழு
ata_readwrite(கட்டமைப்பு disk_op_s *op, முழு iswrite)
{
    u64 lba = op->lba;

    முழு usepio = ata_try_dma(op, iswrite, DISK_SECTOR_SIZE);

    கட்டமைப்பு ata_pio_command cmd;
    memset(&cmd, 0, அளவு(cmd));

    எனில் (op->மொத்தம் >= (1<<8) || lba + op->மொத்தம் >= (1<<28)) {
        cmd.sector_count2 = op->மொத்தம் >> 8;
        cmd.lba_low2 = lba >> 24;
        cmd.lba_mid2 = lba >> 32;
        cmd.lba_high2 = lba >> 40;
        lba &= 0xffffff;

        எனில் (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS_EXT
                           : ATA_CMD_READ_SECTORS_EXT);
        இல்லையெனில்
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                           : ATA_CMD_READ_DMA_EXT);
    } இல்லையெனில் {
        எனில் (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS
                           : ATA_CMD_READ_SECTORS);
        இல்லையெனில்
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA
                           : ATA_CMD_READ_DMA);
    }

    cmd.sector_மொத்தம் = op->மொத்தம்;
    cmd.lba_கீழ் = lba;
    cmd.lba_mid = lba >> 8;
    cmd.lba_உயர்வு = lba >> 16;
    cmd.device = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;

    முழு ret;
    எனில் (usepio)
        ret = ata_pio_cmd_data(op, iswrite, &cmd);
    இல்லையெனில்
        ret = ata_dma_cmd_data(op, &cmd);
    எனில் (ret)
        திரும்பு DISK_RET_EBADTRACK;
    திரும்பு DISK_RET_SUCCESS;
}

// 16bit command demuxer for ATA harddrives.
முழு
ata_process_op(கட்டமைப்பு disk_op_s *op)
{
    எனில் (!CONFIG_ATA)
        திரும்பு 0;

    கட்டமைப்பு atadrive_s *adrive_gf = container_of(
        op->drive_fl, கட்டமைப்பு atadrive_s, drive);
    தேர்வு (op->command) {
    நிலை CMD_READ:
        திரும்பு ata_readwrite(op, 0);
    நிலை CMD_WRITE:
        திரும்பு ata_readwrite(op, 1);
    நிலை CMD_RESET:
        ata_reset(adrive_gf);
        திரும்பு DISK_RET_SUCCESS;
    நிலை CMD_ISREADY:
        திரும்பு isready(adrive_gf);
    இயல்பு:
        திரும்பு default_process_op(op);
    }
}


/****************************************************************
 * ATAPI functions
 ****************************************************************/

#define CDROM_CDB_SIZE 12

// Low-level atapi command transmit function.
முழு
ata_atapi_process_op(கட்டமைப்பு disk_op_s *op)
{
    எனில் (! CONFIG_ATA)
        திரும்பு 0;

    எனில் (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        திரும்பு DISK_RET_EWRITEPROTECT;
    u8 cdbcmd[CDROM_CDB_SIZE];
    முழு blocksize = scsi_fill_cmd(op, cdbcmd, அளவு(cdbcmd));
    எனில் (blocksize < 0)
        திரும்பு default_process_op(op);

    கட்டமைப்பு atadrive_s *adrive_gf = container_of(
        op->drive_fl, கட்டமைப்பு atadrive_s, drive);
    கட்டமைப்பு ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    கட்டமைப்பு ata_pio_command cmd;
    memset(&cmd, 0, அளவு(cmd));
    cmd.lba_mid = blocksize;
    cmd.lba_உயர்வு = blocksize >> 8;
    cmd.command = ATA_CMD_PACKET;

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    முழு ret = send_cmd(adrive_gf, &cmd);
    எனில் (ret)
        செல் fail;
    ret = ata_wait_data(iobase1);
    எனில் (ret)
        செல் fail;

    // Send command to device
    outsw_fl(iobase1, MAKE_FLATPTR(GET_SEG(SS), cdbcmd), CDROM_CDB_SIZE / 2);

    முழு status = pause_await_not_bsy(iobase1, iobase2);
    எனில் (status < 0) {
        ret = status;
        செல் fail;
    }

    எனில் (status & ATA_CB_STAT_ERR) {
        u8 err = inb(iobase1 + ATA_CB_ERR);
        // skip "Not Ready"
        எனில் (err != 0x20)
            dprintf(6, "send_atapi_cmd : read error (status=%02x err=%02x)\n"
                    , status, err);
        ret = -2;
        செல் fail;
    }
    எனில் (blocksize) {
        எனில் (!(status & ATA_CB_STAT_DRQ)) {
            dprintf(6, "send_atapi_cmd : DRQ not set (status %02x)\n", status);
            ret = -3;
            செல் fail;
        }

        ret = ata_pio_transfer(op, 0, blocksize);
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    எனில் (ret)
        திரும்பு DISK_RET_EBADTRACK;
    திரும்பு DISK_RET_SUCCESS;
}


/****************************************************************
 * ATA detect and init
 ****************************************************************/

// Send an identify device or identify device packet command.
நிலையான முழு
send_ata_identity(கட்டமைப்பு atadrive_s *adrive, u16 *buffer, முழு command)
{
    memset(buffer, 0, DISK_SECTOR_SIZE);

    கட்டமைப்பு disk_op_s dop;
    memset(&dop, 0, அளவு(dop));
    dop.drive_fl = &adrive->drive;
    dop.மொத்தம் = 1;
    dop.lba = 1;
    dop.buf_fl = MAKE_FLATPTR(GET_SEG(SS), buffer);

    கட்டமைப்பு ata_pio_command cmd;
    memset(&cmd, 0, அளவு(cmd));
    cmd.command = command;

    திரும்பு ata_pio_cmd_data(&dop, 0, &cmd);
}

// Extract the ATA/ATAPI version info.
முழு
ata_extract_version(u16 *buffer)
{
    // Extract ATA/ATAPI version.
    u16 ataversion = buffer[80];
    u8 version;
    க்கு (version=15; version>0; version--)
        எனில் (ataversion & (1<<version))
            நிறுத்து;
    திரும்பு version;
}

#define MAXMODEL 40

// Extract the ATA/ATAPI model info.
எழுத்து *
ata_extract_model(எழுத்து *model, u32 size, u16 *buffer)
{
    // Read model name
    முழு i;
    க்கு (i=0; i<size/2; i++)
        *(u16*)&model[i*2] = be16_to_cpu(buffer[27+i]);
    model[size] = 0x00;
    nullTrailingSpace(model);
    திரும்பு model;
}

// Common init code between ata and atapi
நிலையான கட்டமைப்பு atadrive_s *
init_atadrive(கட்டமைப்பு atadrive_s *dummy, u16 *buffer)
{
    கட்டமைப்பு atadrive_s *adrive = malloc_fseg(அளவு(*adrive));
    எனில் (!adrive) {
        warn_noalloc();
        திரும்பு வெற்று;
    }
    memset(adrive, 0, அளவு(*adrive));
    adrive->chan_gf = dummy->chan_gf;
    adrive->slave = dummy->slave;
    adrive->drive.cntl_id = adrive->chan_gf->ataid * 2 + dummy->slave;
    adrive->drive.removable = (buffer[0] & 0x80) ? 1 : 0;
    திரும்பு adrive;
}

// Detect if the given drive is an atapi - initialize it if so.
நிலையான கட்டமைப்பு atadrive_s *
init_drive_atapi(கட்டமைப்பு atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE_PACKET command to device
    முழு ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    எனில் (ret)
        திரும்பு வெற்று;

    // Success - setup as ATAPI.
    கட்டமைப்பு atadrive_s *adrive = init_atadrive(dummy, buffer);
    எனில் (!adrive)
        திரும்பு வெற்று;
    adrive->drive.type = DTYPE_ATA_ATAPI;
    adrive->drive.blksize = CDROM_SECTOR_SIZE;
    adrive->drive.sectors = (u64)-1;
    u8 iscd = ((buffer[0] >> 8) & 0x1f) == 0x05;
    எழுத்து model[MAXMODEL+1];
    எழுத்து *desc = znprintf(MAXDESCSIZE
                          , "DVD/CD [ata%d-%d: %s ATAPI-%d %s]"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (iscd ? "DVD/CD" : "Device"));
    dprintf(1, "%s\n", desc);

    // fill cdidmap
    எனில் (iscd) {
        முழு prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                            adrive->chan_gf->chanid,
                                            adrive->slave);
        boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                                  adrive->chan_gf->chanid,
                                  adrive->slave,
                                  &(adrive->drive.lchs));
        boot_add_cd(&adrive->drive, desc, prio);
    }

    திரும்பு adrive;
}

// Detect if the given drive is a regular ata drive - initialize it if so.
நிலையான கட்டமைப்பு atadrive_s *
init_drive_ata(கட்டமைப்பு atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE command to device
    முழு ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_DEVICE);
    எனில் (ret)
        திரும்பு வெற்று;

    // Success - setup as ATA.
    கட்டமைப்பு atadrive_s *adrive = init_atadrive(dummy, buffer);
    எனில் (!adrive)
        திரும்பு வெற்று;
    adrive->drive.type = DTYPE_ATA;
    adrive->drive.blksize = DISK_SECTOR_SIZE;

    adrive->drive.pchs.cylinder = buffer[1];
    adrive->drive.pchs.head = buffer[3];
    adrive->drive.pchs.sector = buffer[6];

    u64 sectors;
    எனில் (buffer[83] & (1 << 10)) // word 83 - lba48 support
        sectors = *(u64*)&buffer[100]; // word 100-103
    இல்லையெனில்
        sectors = *(u32*)&buffer[60]; // word 60 and word 61
    adrive->drive.sectors = sectors;
    u64 adjsize = sectors >> 11;
    எழுத்து adjprefix = 'M';
    எனில் (adjsize >= (1 << 16)) {
        adjsize >>= 10;
        adjprefix = 'G';
    }
    எழுத்து model[MAXMODEL+1];
    எழுத்து *desc = znprintf(MAXDESCSIZE
                          , "ata%d-%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (u32)adjsize, adjprefix);
    dprintf(1, "%s\n", desc);

    முழு prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                        adrive->chan_gf->chanid,
                                        adrive->slave);
    boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                              adrive->chan_gf->chanid,
                              adrive->slave,
                              &(adrive->drive.lchs));
    // Register with bcv system.
    boot_add_hd(&adrive->drive, desc, prio);

    திரும்பு adrive;
}

நிலையான u32 SpinupEnd;

// Wait for non-busy status and check for "floating bus" condition.
நிலையான முழு
powerup_await_non_bsy(u16 base)
{
    u8 orstatus = 0;
    u8 status;
    க்கு (;;) {
        status = inb(base+ATA_CB_STAT);
        எனில் (!(status & ATA_CB_STAT_BSY))
            நிறுத்து;
        orstatus |= status;
        எனில் (orstatus == 0xff) {
            dprintf(4, "powerup IDE floating\n");
            திரும்பு orstatus;
        }
        எனில் (timer_check(SpinupEnd)) {
            warn_timeout();
            திரும்பு -1;
        }
        yield();
    }
    dprintf(6, "powerup iobase=%x st=%x\n", base, status);
    திரும்பு status;
}

// Detect any drives attached to a given controller.
நிலையான வெறுமை
ata_detect(வெறுமை *data)
{
    கட்டமைப்பு ata_channel_s *chan_gf = data;
    கட்டமைப்பு atadrive_s dummy;
    memset(&dummy, 0, அளவு(dummy));
    dummy.chan_gf = chan_gf;
    // Device detection
    முழு didreset = 0;
    u8 slave;
    க்கு (slave=0; slave<=1; slave++) {
        // Wait for not-bsy.
        u16 iobase1 = chan_gf->iobase1;
        முழு status = powerup_await_non_bsy(iobase1);
        எனில் (status < 0)
            தொடர்;
        u8 newdh = slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0;
        outb(newdh, iobase1+ATA_CB_DH);
        ndelay(400);
        status = powerup_await_non_bsy(iobase1);
        எனில் (status < 0)
            தொடர்;

        // Check if ioport registers look valid.
        outb(newdh, iobase1+ATA_CB_DH);
        u8 dh = inb(iobase1+ATA_CB_DH);
        outb(0x55, iobase1+ATA_CB_SC);
        outb(0xaa, iobase1+ATA_CB_SN);
        u8 sc = inb(iobase1+ATA_CB_SC);
        u8 sn = inb(iobase1+ATA_CB_SN);
        dprintf(6, "ata_detect ata%d-%d: sc=%x sn=%x dh=%x\n"
                , chan_gf->ataid, slave, sc, sn, dh);
        எனில் (sc != 0x55 || sn != 0xaa || dh != newdh)
            தொடர்;

        // Prepare new drive.
        dummy.slave = slave;

        // reset the channel
        எனில் (!didreset) {
            ata_reset(&dummy);
            didreset = 1;
        }

        // check for ATAPI
        u16 buffer[256];
        கட்டமைப்பு atadrive_s *adrive = init_drive_atapi(&dummy, buffer);
        எனில் (!adrive) {
            // Didn't find an ATAPI drive - look for ATA drive.
            u8 st = inb(iobase1+ATA_CB_STAT);
            எனில் (!st)
                // Status not set - can't be a valid drive.
                தொடர்;

            // Wait for RDY.
            முழு ret = await_rdy(iobase1);
            எனில் (ret < 0)
                தொடர்;

            // check for ATA.
            adrive = init_drive_ata(&dummy, buffer);
            எனில் (!adrive)
                // No ATA drive found
                தொடர்;
        }

        u16 resetresult = buffer[93];
        dprintf(6, "ata_detect resetresult=%04x\n", resetresult);
        எனில் (!slave && (resetresult & 0xdf61) == 0x4041)
            // resetresult looks valid and device 0 is responding to
            // device 1 requests - device 1 must not be present - skip
            // detection.
            நிறுத்து;
    }
}

// Initialize an ata controller and detect its drives.
நிலையான வெறுமை
init_controller(கட்டமைப்பு pci_device *pci, முழு chanid, முழு irq
                , u32 port1, u32 port2, u32 master)
{
    நிலையான முழு ataid = 0;
    கட்டமைப்பு ata_channel_s *chan_gf = malloc_fseg(அளவு(*chan_gf));
    எனில் (!chan_gf) {
        warn_noalloc();
        திரும்பு;
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
நிலையான வெறுமை
init_pciata(கட்டமைப்பு pci_device *pci, u8 prog_if)
{
    u8 pciirq = pci_config_readb(pci->bdf, PCI_INTERRUPT_வரி);
    முழு master = 0;
    எனில் (CONFIG_ATA_DMA && prog_if & 0x80) {
        // Check for bus-mastering.
        u32 bar = pci_config_readl(pci->bdf, PCI_BASE_ADDRESS_4);
        எனில் (bar & PCI_BASE_ADDRESS_SPACE_IO) {
            master = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
            pci_enable_busmaster(pci);
        }
    }

    u32 port1, port2, irq;
    எனில் (prog_if & 1) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_1);
        எனில் (!port1 || !port2)
            திரும்பு;
        irq = pciirq;
    } இல்லையெனில் {
        port1 = PORT_ATA1_CMD_BASE;
        port2 = PORT_ATA1_CTRL_BASE;
        irq = IRQ_ATA1;
    }
    init_controller(pci, 0, irq, port1, port2, master);

    எனில் (prog_if & 4) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_2);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_3);
        எனில் (!port1 || !port2)
            திரும்பு;
        irq = pciirq;
    } இல்லையெனில் {
        port1 = PORT_ATA2_CMD_BASE;
        port2 = PORT_ATA2_CTRL_BASE;
        irq = IRQ_ATA2;
    }
    init_controller(pci, 1, irq, port1, port2, master ? master + 8 : 0);
}

நிலையான வெறுமை
found_genericata(கட்டமைப்பு pci_device *pci, வெறுமை *arg)
{
    init_pciata(pci, pci->prog_if);
}

நிலையான வெறுமை
found_compatibleahci(கட்டமைப்பு pci_device *pci, வெறுமை *arg)
{
    எனில் (CONFIG_AHCI)
        // Already handled directly via native ahci interface.
        திரும்பு;
    init_pciata(pci, 0x8f);
}

நிலையான மாறிலி கட்டமைப்பு pci_device_id pci_ata_tbl[] = {
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE
                     , found_genericata),
    PCI_DEVICE(PCI_VENDOR_ID_ATI, 0x4391, found_compatibleahci),
    PCI_DEVICE_END,
};

// Locate and init ata controllers.
நிலையான வெறுமை
ata_scan(வெறுமை)
{
    எனில் (CONFIG_QEMU && hlist_empty(&PCIDevices)) {
        // No PCI devices found - probably a QEMU "-M isapc" machine.
        // Try using ISA ports for ATA controllers.
        init_controller(வெற்று, 0, IRQ_ATA1
                        , PORT_ATA1_CMD_BASE, PORT_ATA1_CTRL_BASE, 0);
        init_controller(வெற்று, 1, IRQ_ATA2
                        , PORT_ATA2_CMD_BASE, PORT_ATA2_CTRL_BASE, 0);
        திரும்பு;
    }

    // Scan PCI bus for ATA adapters
    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_ata_tbl, pci, வெற்று);
    }
}

வெறுமை
ata_setup(வெறுமை)
{
    ASSERT32FLAT();
    எனில் (!CONFIG_ATA)
        திரும்பு;

    dprintf(3, "init hard drives\n");

    SpinupEnd = timer_calc(IDE_TIMEOUT);
    ata_scan();

    SET_BDA(disk_control_byte, 0xc0);

    enable_hwirq(14, FUNC16(entry_76));
}

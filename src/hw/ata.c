// Low level ATA disk access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "ata.h" // ATA_CB_STAT
#include "biosvar.h" // GET_GLOBALFLAT
#include "block.h" // struct drive_s
#include "blockcmd.h" // CDB_CMD_READ_10
#include "byteorder.h" // be16_to_cpu
#include "malloc.h" // malloc_fseg
#include "output.h" // dprintf
#include "pci.h" // pci_config_readb
#include "pcidevice.h" // foreachpci
#include "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#include "pci_regs.h" // PCI_INTERRUPT_LINE
#include "pic.h" // enable_hwirq
#include "stacks.h" // yield
#include "std/disk.h" // DISK_RET_SUCCESS
#include "string.h" // memset
#include "util.h" // timer_calc
#include "x86.h" // inb

#define IDE_TIMEOUT 32000 //32 seconds max for IDE ops


/****************************************************************
 * Helper functions
 ****************************************************************/

// Wait for the specified ide state
static anta_hstha ཧྲིལ
await_ide(u8 mask, u8 flags, u16 base, u16 timeout)
{
    u32 end = timer_calc(timeout);
    དོན་ལུ (;;) {
        u8 status = inb(base+ATA_CB_STAT);
        གལ་སྲིད ((status & mask) == flags)
            ལོག status;
        གལ་སྲིད (timer_check(end)) {
            warn_timeout();
            ལོག -1;
        }
        yield();
    }
}

// Wait for the device to be not-busy.
static ཧྲིལ
await_not_bsy(u16 base)
{
    ལོག await_ide(ATA_CB_STAT_BSY, 0, base, IDE_TIMEOUT);
}

// Wait for the device to be ready.
static ཧྲིལ
await_rdy(u16 base)
{
    ལོག await_ide(ATA_CB_STAT_RDY, ATA_CB_STAT_RDY, base, IDE_TIMEOUT);
}

// Wait for ide state - pauses for one ata cycle first.
static anta_hstha ཧྲིལ
pause_await_not_bsy(u16 iobase1, u16 iobase2)
{
    // Wait one PIO transfer cycle.
    inb(iobase2 + ATA_CB_ASTAT);

    ལོག await_not_bsy(iobase1);
}

// Wait for ide state - pause for 400ns first.
static anta_hstha ཧྲིལ
ndelay_await_not_bsy(u16 iobase1)
{
    ndelay(400);
    ལོག await_not_bsy(iobase1);
}

// Reset a drive
static void
ata_reset(sa_nracanaa atadrive_s *adrive_gf)
{
    sa_nracanaa ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
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
    ཧྲིལ status = await_not_bsy(iobase1);
    གལ་སྲིད (status < 0)
        goto done;
    གལ་སྲིད (slave) {
        // Change device.
        u32 end = timer_calc(IDE_TIMEOUT);
        དོན་ལུ (;;) {
            outb(ATA_CB_DH_DEV1, iobase1 + ATA_CB_DH);
            status = ndelay_await_not_bsy(iobase1);
            གལ་སྲིད (status < 0)
                goto done;
            གལ་སྲིད (inb(iobase1 + ATA_CB_DH) == ATA_CB_DH_DEV1)
                bha_nga;
            // Change drive request failed to take effect - retry.
            གལ་སྲིད (timer_check(end)) {
                warn_timeout();
                goto done;
            }
        }
    } དེ་མིན {
        // QEMU doesn't reset dh on reset, so set it explicitly.
        outb(ATA_CB_DH_DEV0, iobase1 + ATA_CB_DH);
    }

    // On a user-reset request, wait for RDY if it is an ATA device.
    u8 type=GET_GLOBALFLAT(adrive_gf->drive.type);
    གལ་སྲིད (type == DTYPE_ATA)
        status = await_rdy(iobase1);

done:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    dprintf(6, "ata_reset exit status=%x\n", status);
}

// Check for drive RDY for 16bit interface command.
static ཧྲིལ
isready(sa_nracanaa atadrive_s *adrive_gf)
{
    // Read the status from controller
    sa_nracanaa ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u8 status = inb(iobase1 + ATA_CB_STAT);
    གལ་སྲིད ((status & (ATA_CB_STAT_BSY|ATA_CB_STAT_RDY)) == ATA_CB_STAT_RDY)
        ལོག DISK_RET_SUCCESS;
    ལོག DISK_RET_ENOTREADY;
}


/****************************************************************
 * ATA send command
 ****************************************************************/

sa_nracanaa ata_pio_command {
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
static ཧྲིལ
send_cmd(sa_nracanaa atadrive_s *adrive_gf, sa_nracanaa ata_pio_command *cmd)
{
    sa_nracanaa ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);

    // Select device
    ཧྲིལ status = await_not_bsy(iobase1);
    གལ་སྲིད (status < 0)
        ལོག status;
    u8 newdh = ((cmd->device & ~ATA_CB_DH_DEV1)
                | (slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0));
    u8 olddh = inb(iobase1 + ATA_CB_DH);
    outb(newdh, iobase1 + ATA_CB_DH);
    གལ་སྲིད ((olddh ^ newdh) & (1<<4)) {
        // Was a device change - wait for device to become not busy.
        status = ndelay_await_not_bsy(iobase1);
        གལ་སྲིད (status < 0)
            ལོག status;
    }

    // Check for ATA_CMD_(READ|WRITE)_(SECTORS|DMA)_EXT commands.
    གལ་སྲིད ((cmd->command & ~0x11) == ATA_CMD_READ_SECTORS_EXT) {
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

    ལོག 0;
}

// Wait for data after calling 'send_cmd'.
static ཧྲིལ
ata_wait_data(u16 iobase1)
{
    ཧྲིལ status = ndelay_await_not_bsy(iobase1);
    གལ་སྲིད (status < 0)
        ལོག status;

    གལ་སྲིད (status & ATA_CB_STAT_ERR) {
        dprintf(6, "send_cmd : read error (status=%02x err=%02x)\n"
                , status, inb(iobase1 + ATA_CB_ERR));
        ལོག -4;
    }
    གལ་སྲིད (!(status & ATA_CB_STAT_DRQ)) {
        dprintf(6, "send_cmd : DRQ not set (status %02x)\n", status);
        ལོག -5;
    }

    ལོག 0;
}

// Send an ata command that does not transfer any further data.
ཧྲིལ
ata_cmd_nondata(sa_nracanaa atadrive_s *adrive_gf, sa_nracanaa ata_pio_command *cmd)
{
    sa_nracanaa ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    ཧྲིལ ret = send_cmd(adrive_gf, cmd);
    གལ་སྲིད (ret)
        goto fail;
    ret = ndelay_await_not_bsy(iobase1);
    གལ་སྲིད (ret < 0)
        goto fail;

    གལ་སྲིད (ret & ATA_CB_STAT_ERR) {
        dprintf(6, "nondata cmd : read error (status=%02x err=%02x)\n"
                , ret, inb(iobase1 + ATA_CB_ERR));
        ret = -4;
        goto fail;
    }
    གལ་སྲིད (ret & ATA_CB_STAT_DRQ) {
        dprintf(6, "nondata cmd : DRQ set (status %02x)\n", ret);
        ret = -5;
        goto fail;
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    ལོག ret;
}


/****************************************************************
 * ATA PIO transfers
 ****************************************************************/

// Transfer 'op->count' blocks (of 'blocksize' bytes) to/from drive
// 'op->drive_fl'.
static ཧྲིལ
ata_pio_transfer(sa_nracanaa disk_op_s *op, ཧྲིལ iswrite, ཧྲིལ blocksize)
{
    dprintf(16, "ata_pio_transfer id=%p write=%d count=%d bs=%d buf=%p\n"
            , op->drive_fl, iswrite, op->count, blocksize, op->buf_fl);

    sa_nracanaa atadrive_s *adrive_gf = container_of(
        op->drive_fl, sa_nracanaa atadrive_s, drive);
    sa_nracanaa ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    ཧྲིལ count = op->count;
    void *buf_fl = op->buf_fl;
    ཧྲིལ status;
    དོན་ལུ (;;) {
        གལ་སྲིད (iswrite) {
            // Write data to controller
            dprintf(16, "Write sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            གལ་སྲིད (CONFIG_ATA_PIO32)
                outsl_fl(iobase1, buf_fl, blocksize / 4);
            དེ་མིན
                outsw_fl(iobase1, buf_fl, blocksize / 2);
        } དེ་མིན {
            // Read data from controller
            dprintf(16, "Read sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            གལ་སྲིད (CONFIG_ATA_PIO32)
                insl_fl(iobase1, buf_fl, blocksize / 4);
            དེ་མིན
                insw_fl(iobase1, buf_fl, blocksize / 2);
        }
        buf_fl += blocksize;

        status = pause_await_not_bsy(iobase1, iobase2);
        གལ་སྲིད (status < 0) {
            // Error
            op->count -= count;
            ལོག status;
        }

        count--;
        གལ་སྲིད (!count)
            bha_nga;
        status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ | ATA_CB_STAT_ERR);
        གལ་སྲིད (status != ATA_CB_STAT_DRQ) {
            dprintf(6, "ata_pio_transfer : more sectors left (status %02x)\n"
                    , status);
            op->count -= count;
            ལོག -6;
        }
    }

    status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
               | ATA_CB_STAT_ERR);
    གལ་སྲིད (!iswrite)
        status &= ~ATA_CB_STAT_DF;
    གལ་སྲིད (status != 0) {
        dprintf(6, "ata_pio_transfer : no sectors left (status %02x)\n", status);
        ལོག -7;
    }

    ལོག 0;
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

sa_nracanaa sff_dma_prd {
    u32 buf_fl;
    u32 count;
};

// Check if DMA available and setup transfer if so.
static ཧྲིལ
ata_try_dma(sa_nracanaa disk_op_s *op, ཧྲིལ iswrite, ཧྲིལ blocksize)
{
    ASSERT16();
    གལ་སྲིད (! CONFIG_ATA_DMA)
        ལོག -1;
    u32 dest = (u32)op->buf_fl;
    གལ་སྲིད (dest & 1)
        // Need minimum alignment of 1.
        ལོག -1;
    sa_nracanaa atadrive_s *adrive_gf = container_of(
        op->drive_fl, sa_nracanaa atadrive_s, drive);
    sa_nracanaa ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);
    གལ་སྲིད (! iomaster)
        ལོག -1;
    u32 bytes = op->count * blocksize;
    གལ་སྲིད (! bytes)
        ལོག -1;

    // Build PRD dma structure.
    sa_nracanaa sff_dma_prd *dma = MAKE_FLATPTR(SEG_LOW, ExtraStack);
    sa_nracanaa sff_dma_prd *origdma = dma;
    སྐབས (bytes) {
        གལ་སྲིད (dma >= &origdma[16])
            // Too many descriptors..
            ལོག -1;
        u32 count = bytes;
        u32 max = 0x10000 - (dest & 0xffff);
        གལ་སྲིད (count > max)
            count = max;

        SET_LOWFLAT(dma->buf_fl, dest);
        bytes -= count;
        གལ་སྲིད (!bytes)
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

    ལོག 0;
}

// Transfer data using DMA.
static ཧྲིལ
ata_dma_transfer(sa_nracanaa disk_op_s *op)
{
    གལ་སྲིད (! CONFIG_ATA_DMA)
        ལོག -1;
    dprintf(16, "ata_dma_transfer id=%p buf=%p\n", op->drive_fl, op->buf_fl);

    sa_nracanaa atadrive_s *adrive_gf = container_of(
        op->drive_fl, sa_nracanaa atadrive_s, drive);
    sa_nracanaa ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);

    // Start bus-master controller.
    u8 oldcmd = inb(iomaster + BM_CMD);
    outb(oldcmd | BM_CMD_START, iomaster + BM_CMD);

    u32 end = timer_calc(IDE_TIMEOUT);
    u8 status;
    དོན་ལུ (;;) {
        status = inb(iomaster + BM_STATUS);
        གལ་སྲིད (status & BM_STATUS_IRQ)
            bha_nga;
        // Transfer in progress
        གལ་སྲིད (timer_check(end)) {
            // Timeout.
            warn_timeout();
            bha_nga;
        }
        yield();
    }
    outb(oldcmd & ~BM_CMD_START, iomaster + BM_CMD);

    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    ཧྲིལ idestatus = pause_await_not_bsy(iobase1, iobase2);

    གལ་སྲིད ((status & (BM_STATUS_IRQ|BM_STATUS_ACTIVE)) == BM_STATUS_IRQ
        && idestatus >= 0x00
        && (idestatus & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
                         | ATA_CB_STAT_ERR)) == 0x00)
        // Success.
        ལོག 0;

    dprintf(6, "IDE DMA error (dma=%x ide=%x/%x/%x)\n", status, idestatus
            , inb(iobase2 + ATA_CB_ASTAT), inb(iobase1 + ATA_CB_ERR));
    ལོག -1;
}


/****************************************************************
 * ATA hard drive functions
 ****************************************************************/

// Transfer data to harddrive using PIO protocol.
static ཧྲིལ
ata_pio_cmd_data(sa_nracanaa disk_op_s *op, ཧྲིལ iswrite, sa_nracanaa ata_pio_command *cmd)
{
    sa_nracanaa atadrive_s *adrive_gf = container_of(
        op->drive_fl, sa_nracanaa atadrive_s, drive);
    sa_nracanaa ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    ཧྲིལ ret = send_cmd(adrive_gf, cmd);
    གལ་སྲིད (ret)
        goto fail;
    ret = ata_wait_data(iobase1);
    གལ་སྲིད (ret)
        goto fail;
    ret = ata_pio_transfer(op, iswrite, DISK_SECTOR_SIZE);

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    ལོག ret;
}

// Transfer data to harddrive using DMA protocol.
static ཧྲིལ
ata_dma_cmd_data(sa_nracanaa disk_op_s *op, sa_nracanaa ata_pio_command *cmd)
{
    གལ་སྲིད (! CONFIG_ATA_DMA)
        ལོག -1;
    sa_nracanaa atadrive_s *adrive_gf = container_of(
        op->drive_fl, sa_nracanaa atadrive_s, drive);
    ཧྲིལ ret = send_cmd(adrive_gf, cmd);
    གལ་སྲིད (ret)
        ལོག ret;
    ལོག ata_dma_transfer(op);
}

// Read/write count blocks from a harddrive.
static ཧྲིལ
ata_readwrite(sa_nracanaa disk_op_s *op, ཧྲིལ iswrite)
{
    u64 lba = op->lba;

    ཧྲིལ usepio = ata_try_dma(op, iswrite, DISK_SECTOR_SIZE);

    sa_nracanaa ata_pio_command cmd;
    memset(&cmd, 0, parimaa_na(cmd));

    གལ་སྲིད (op->count >= (1<<8) || lba + op->count >= (1<<28)) {
        cmd.sector_count2 = op->count >> 8;
        cmd.lba_low2 = lba >> 24;
        cmd.lba_mid2 = lba >> 32;
        cmd.lba_high2 = lba >> 40;
        lba &= 0xffffff;

        གལ་སྲིད (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS_EXT
                           : ATA_CMD_READ_SECTORS_EXT);
        དེ་མིན
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                           : ATA_CMD_READ_DMA_EXT);
    } དེ་མིན {
        གལ་སྲིད (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS
                           : ATA_CMD_READ_SECTORS);
        དེ་མིན
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA
                           : ATA_CMD_READ_DMA);
    }

    cmd.sector_count = op->count;
    cmd.lba_low = lba;
    cmd.lba_mid = lba >> 8;
    cmd.lba_high = lba >> 16;
    cmd.device = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;

    ཧྲིལ ret;
    གལ་སྲིད (usepio)
        ret = ata_pio_cmd_data(op, iswrite, &cmd);
    དེ་མིན
        ret = ata_dma_cmd_data(op, &cmd);
    གལ་སྲིད (ret)
        ལོག DISK_RET_EBADTRACK;
    ལོག DISK_RET_SUCCESS;
}

// 16bit command demuxer for ATA harddrives.
ཧྲིལ
ata_process_op(sa_nracanaa disk_op_s *op)
{
    གལ་སྲིད (!CONFIG_ATA)
        ལོག 0;

    sa_nracanaa atadrive_s *adrive_gf = container_of(
        op->drive_fl, sa_nracanaa atadrive_s, drive);
    switch (op->command) {
    pak_sa CMD_READ:
        ལོག ata_readwrite(op, 0);
    pak_sa CMD_WRITE:
        ལོག ata_readwrite(op, 1);
    pak_sa CMD_RESET:
        ata_reset(adrive_gf);
        ལོག DISK_RET_SUCCESS;
    pak_sa CMD_ISREADY:
        ལོག isready(adrive_gf);
    default:
        ལོག default_process_op(op);
    }
}


/****************************************************************
 * ATAPI functions
 ****************************************************************/

#define CDROM_CDB_SIZE 12

// Low-level atapi command transmit function.
ཧྲིལ
ata_atapi_process_op(sa_nracanaa disk_op_s *op)
{
    གལ་སྲིད (! CONFIG_ATA)
        ལོག 0;

    གལ་སྲིད (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        ལོག DISK_RET_EWRITEPROTECT;
    u8 cdbcmd[CDROM_CDB_SIZE];
    ཧྲིལ blocksize = scsi_fill_cmd(op, cdbcmd, parimaa_na(cdbcmd));
    གལ་སྲིད (blocksize < 0)
        ལོག default_process_op(op);

    sa_nracanaa atadrive_s *adrive_gf = container_of(
        op->drive_fl, sa_nracanaa atadrive_s, drive);
    sa_nracanaa ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    sa_nracanaa ata_pio_command cmd;
    memset(&cmd, 0, parimaa_na(cmd));
    cmd.lba_mid = blocksize;
    cmd.lba_high = blocksize >> 8;
    cmd.command = ATA_CMD_PACKET;

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    ཧྲིལ ret = send_cmd(adrive_gf, &cmd);
    གལ་སྲིད (ret)
        goto fail;
    ret = ata_wait_data(iobase1);
    གལ་སྲིད (ret)
        goto fail;

    // Send command to device
    outsw_fl(iobase1, MAKE_FLATPTR(GET_SEG(SS), cdbcmd), CDROM_CDB_SIZE / 2);

    ཧྲིལ status = pause_await_not_bsy(iobase1, iobase2);
    གལ་སྲིད (status < 0) {
        ret = status;
        goto fail;
    }

    གལ་སྲིད (status & ATA_CB_STAT_ERR) {
        u8 err = inb(iobase1 + ATA_CB_ERR);
        // skip "Not Ready"
        གལ་སྲིད (err != 0x20)
            dprintf(6, "send_atapi_cmd : read error (status=%02x err=%02x)\n"
                    , status, err);
        ret = -2;
        goto fail;
    }
    གལ་སྲིད (blocksize) {
        གལ་སྲིད (!(status & ATA_CB_STAT_DRQ)) {
            dprintf(6, "send_atapi_cmd : DRQ not set (status %02x)\n", status);
            ret = -3;
            goto fail;
        }

        ret = ata_pio_transfer(op, 0, blocksize);
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    གལ་སྲིད (ret)
        ལོག DISK_RET_EBADTRACK;
    ལོག DISK_RET_SUCCESS;
}


/****************************************************************
 * ATA detect and init
 ****************************************************************/

// Send an identify device or identify device packet command.
static ཧྲིལ
send_ata_identity(sa_nracanaa atadrive_s *adrive, u16 *buffer, ཧྲིལ command)
{
    memset(buffer, 0, DISK_SECTOR_SIZE);

    sa_nracanaa disk_op_s dop;
    memset(&dop, 0, parimaa_na(dop));
    dop.drive_fl = &adrive->drive;
    dop.count = 1;
    dop.lba = 1;
    dop.buf_fl = MAKE_FLATPTR(GET_SEG(SS), buffer);

    sa_nracanaa ata_pio_command cmd;
    memset(&cmd, 0, parimaa_na(cmd));
    cmd.command = command;

    ལོག ata_pio_cmd_data(&dop, 0, &cmd);
}

// Extract the ATA/ATAPI version info.
ཧྲིལ
ata_extract_version(u16 *buffer)
{
    // Extract ATA/ATAPI version.
    u16 ataversion = buffer[80];
    u8 version;
    དོན་ལུ (version=15; version>0; version--)
        གལ་སྲིད (ataversion & (1<<version))
            bha_nga;
    ལོག version;
}

#define MAXMODEL 40

// Extract the ATA/ATAPI model info.
var_na *
ata_extract_model(var_na *model, u32 size, u16 *buffer)
{
    // Read model name
    ཧྲིལ i;
    དོན་ལུ (i=0; i<size/2; i++)
        *(u16*)&model[i*2] = be16_to_cpu(buffer[27+i]);
    model[size] = 0x00;
    nullTrailingSpace(model);
    ལོག model;
}

// Common init code between ata and atapi
static sa_nracanaa atadrive_s *
init_atadrive(sa_nracanaa atadrive_s *dummy, u16 *buffer)
{
    sa_nracanaa atadrive_s *adrive = malloc_fseg(parimaa_na(*adrive));
    གལ་སྲིད (!adrive) {
        warn_noalloc();
        ལོག NULL;
    }
    memset(adrive, 0, parimaa_na(*adrive));
    adrive->chan_gf = dummy->chan_gf;
    adrive->slave = dummy->slave;
    adrive->drive.cntl_id = adrive->chan_gf->ataid * 2 + dummy->slave;
    adrive->drive.removable = (buffer[0] & 0x80) ? 1 : 0;
    ལོག adrive;
}

// Detect if the given drive is an atapi - initialize it if so.
static sa_nracanaa atadrive_s *
init_drive_atapi(sa_nracanaa atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE_PACKET command to device
    ཧྲིལ ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    གལ་སྲིད (ret)
        ལོག NULL;

    // Success - setup as ATAPI.
    sa_nracanaa atadrive_s *adrive = init_atadrive(dummy, buffer);
    གལ་སྲིད (!adrive)
        ལོག NULL;
    adrive->drive.type = DTYPE_ATA_ATAPI;
    adrive->drive.blksize = CDROM_SECTOR_SIZE;
    adrive->drive.sectors = (u64)-1;
    u8 iscd = ((buffer[0] >> 8) & 0x1f) == 0x05;
    var_na model[MAXMODEL+1];
    var_na *desc = znprintf(MAXDESCSIZE
                          , "DVD/CD [ata%d-%d: %s ATAPI-%d %s]"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (iscd ? "DVD/CD" : "Device"));
    dprintf(1, "%s\n", desc);

    // fill cdidmap
    གལ་སྲིད (iscd) {
        ཧྲིལ prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                            adrive->chan_gf->chanid,
                                            adrive->slave);
        boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                                  adrive->chan_gf->chanid,
                                  adrive->slave,
                                  &(adrive->drive.lchs));
        boot_add_cd(&adrive->drive, desc, prio);
    }

    ལོག adrive;
}

// Detect if the given drive is a regular ata drive - initialize it if so.
static sa_nracanaa atadrive_s *
init_drive_ata(sa_nracanaa atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE command to device
    ཧྲིལ ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_DEVICE);
    གལ་སྲིད (ret)
        ལོག NULL;

    // Success - setup as ATA.
    sa_nracanaa atadrive_s *adrive = init_atadrive(dummy, buffer);
    གལ་སྲིད (!adrive)
        ལོག NULL;
    adrive->drive.type = DTYPE_ATA;
    adrive->drive.blksize = DISK_SECTOR_SIZE;

    adrive->drive.pchs.cylinder = buffer[1];
    adrive->drive.pchs.head = buffer[3];
    adrive->drive.pchs.sector = buffer[6];

    u64 sectors;
    གལ་སྲིད (buffer[83] & (1 << 10)) // word 83 - lba48 support
        sectors = *(u64*)&buffer[100]; // word 100-103
    དེ་མིན
        sectors = *(u32*)&buffer[60]; // word 60 and word 61
    adrive->drive.sectors = sectors;
    u64 adjsize = sectors >> 11;
    var_na adjprefix = 'M';
    གལ་སྲིད (adjsize >= (1 << 16)) {
        adjsize >>= 10;
        adjprefix = 'G';
    }
    var_na model[MAXMODEL+1];
    var_na *desc = znprintf(MAXDESCSIZE
                          , "ata%d-%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (u32)adjsize, adjprefix);
    dprintf(1, "%s\n", desc);

    ཧྲིལ prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                        adrive->chan_gf->chanid,
                                        adrive->slave);
    boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                              adrive->chan_gf->chanid,
                              adrive->slave,
                              &(adrive->drive.lchs));
    // Register with bcv system.
    boot_add_hd(&adrive->drive, desc, prio);

    ལོག adrive;
}

static u32 SpinupEnd;

// Wait for non-busy status and check for "floating bus" condition.
static ཧྲིལ
powerup_await_non_bsy(u16 base)
{
    u8 orstatus = 0;
    u8 status;
    དོན་ལུ (;;) {
        status = inb(base+ATA_CB_STAT);
        གལ་སྲིད (!(status & ATA_CB_STAT_BSY))
            bha_nga;
        orstatus |= status;
        གལ་སྲིད (orstatus == 0xff) {
            dprintf(4, "powerup IDE floating\n");
            ལོག orstatus;
        }
        གལ་སྲིད (timer_check(SpinupEnd)) {
            warn_timeout();
            ལོག -1;
        }
        yield();
    }
    dprintf(6, "powerup iobase=%x st=%x\n", base, status);
    ལོག status;
}

// Detect any drives attached to a given controller.
static void
ata_detect(void *data)
{
    sa_nracanaa ata_channel_s *chan_gf = data;
    sa_nracanaa atadrive_s dummy;
    memset(&dummy, 0, parimaa_na(dummy));
    dummy.chan_gf = chan_gf;
    // Device detection
    ཧྲིལ didreset = 0;
    u8 slave;
    དོན་ལུ (slave=0; slave<=1; slave++) {
        // Wait for not-bsy.
        u16 iobase1 = chan_gf->iobase1;
        ཧྲིལ status = powerup_await_non_bsy(iobase1);
        གལ་སྲིད (status < 0)
            continue;
        u8 newdh = slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0;
        outb(newdh, iobase1+ATA_CB_DH);
        ndelay(400);
        status = powerup_await_non_bsy(iobase1);
        གལ་སྲིད (status < 0)
            continue;

        // Check if ioport registers look valid.
        outb(newdh, iobase1+ATA_CB_DH);
        u8 dh = inb(iobase1+ATA_CB_DH);
        outb(0x55, iobase1+ATA_CB_SC);
        outb(0xaa, iobase1+ATA_CB_SN);
        u8 sc = inb(iobase1+ATA_CB_SC);
        u8 sn = inb(iobase1+ATA_CB_SN);
        dprintf(6, "ata_detect ata%d-%d: sc=%x sn=%x dh=%x\n"
                , chan_gf->ataid, slave, sc, sn, dh);
        གལ་སྲིད (sc != 0x55 || sn != 0xaa || dh != newdh)
            continue;

        // Prepare new drive.
        dummy.slave = slave;

        // reset the channel
        གལ་སྲིད (!didreset) {
            ata_reset(&dummy);
            didreset = 1;
        }

        // check for ATAPI
        u16 buffer[256];
        sa_nracanaa atadrive_s *adrive = init_drive_atapi(&dummy, buffer);
        གལ་སྲིད (!adrive) {
            // Didn't find an ATAPI drive - look for ATA drive.
            u8 st = inb(iobase1+ATA_CB_STAT);
            གལ་སྲིད (!st)
                // Status not set - can't be a valid drive.
                continue;

            // Wait for RDY.
            ཧྲིལ ret = await_rdy(iobase1);
            གལ་སྲིད (ret < 0)
                continue;

            // check for ATA.
            adrive = init_drive_ata(&dummy, buffer);
            གལ་སྲིད (!adrive)
                // No ATA drive found
                continue;
        }

        u16 resetresult = buffer[93];
        dprintf(6, "ata_detect resetresult=%04x\n", resetresult);
        གལ་སྲིད (!slave && (resetresult & 0xdf61) == 0x4041)
            // resetresult looks valid and device 0 is responding to
            // device 1 requests - device 1 must not be present - skip
            // detection.
            bha_nga;
    }
}

// Initialize an ata controller and detect its drives.
static void
init_controller(sa_nracanaa pci_device *pci, ཧྲིལ chanid, ཧྲིལ irq
                , u32 port1, u32 port2, u32 master)
{
    static ཧྲིལ ataid = 0;
    sa_nracanaa ata_channel_s *chan_gf = malloc_fseg(parimaa_na(*chan_gf));
    གལ་སྲིད (!chan_gf) {
        warn_noalloc();
        ལོག;
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
static void
init_pciata(sa_nracanaa pci_device *pci, u8 prog_if)
{
    u8 pciirq = pci_config_readb(pci->bdf, PCI_INTERRUPT_LINE);
    ཧྲིལ master = 0;
    གལ་སྲིད (CONFIG_ATA_DMA && prog_if & 0x80) {
        // Check for bus-mastering.
        u32 bar = pci_config_readl(pci->bdf, PCI_BASE_ADDRESS_4);
        གལ་སྲིད (bar & PCI_BASE_ADDRESS_SPACE_IO) {
            master = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
            pci_enable_busmaster(pci);
        }
    }

    u32 port1, port2, irq;
    གལ་སྲིད (prog_if & 1) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_1);
        གལ་སྲིད (!port1 || !port2)
            ལོག;
        irq = pciirq;
    } དེ་མིན {
        port1 = PORT_ATA1_CMD_BASE;
        port2 = PORT_ATA1_CTRL_BASE;
        irq = IRQ_ATA1;
    }
    init_controller(pci, 0, irq, port1, port2, master);

    གལ་སྲིད (prog_if & 4) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_2);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_3);
        གལ་སྲིད (!port1 || !port2)
            ལོག;
        irq = pciirq;
    } དེ་མིན {
        port1 = PORT_ATA2_CMD_BASE;
        port2 = PORT_ATA2_CTRL_BASE;
        irq = IRQ_ATA2;
    }
    init_controller(pci, 1, irq, port1, port2, master ? master + 8 : 0);
}

static void
found_genericata(sa_nracanaa pci_device *pci, void *arg)
{
    init_pciata(pci, pci->prog_if);
}

static void
found_compatibleahci(sa_nracanaa pci_device *pci, void *arg)
{
    གལ་སྲིད (CONFIG_AHCI)
        // Already handled directly via native ahci interface.
        ལོག;
    init_pciata(pci, 0x8f);
}

static const sa_nracanaa pci_device_id pci_ata_tbl[] = {
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE
                     , found_genericata),
    PCI_DEVICE(PCI_VENDOR_ID_ATI, 0x4391, found_compatibleahci),
    PCI_DEVICE_END,
};

// Locate and init ata controllers.
static void
ata_scan(void)
{
    གལ་སྲིད (CONFIG_QEMU && hlist_empty(&PCIDevices)) {
        // No PCI devices found - probably a QEMU "-M isapc" machine.
        // Try using ISA ports for ATA controllers.
        init_controller(NULL, 0, IRQ_ATA1
                        , PORT_ATA1_CMD_BASE, PORT_ATA1_CTRL_BASE, 0);
        init_controller(NULL, 1, IRQ_ATA2
                        , PORT_ATA2_CMD_BASE, PORT_ATA2_CTRL_BASE, 0);
        ལོག;
    }

    // Scan PCI bus for ATA adapters
    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_ata_tbl, pci, NULL);
    }
}

void
ata_setup(void)
{
    ASSERT32FLAT();
    གལ་སྲིད (!CONFIG_ATA)
        ལོག;

    dprintf(3, "init hard drives\n");

    SpinupEnd = timer_calc(IDE_TIMEOUT);
    ata_scan();

    SET_BDA(disk_control_byte, 0xc0);

    enable_hwirq(14, FUNC16(entry_76));
}

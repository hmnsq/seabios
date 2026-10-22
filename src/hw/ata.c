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
static inline 정수
await_ide(u8 mask, u8 flags, u16 base, u16 timeout)
{
    u32 end = timer_calc(timeout);
    위해 (;;) {
        u8 status = inb(base+ATA_CB_STAT);
        만약 ((status & mask) == flags)
            반환 status;
        만약 (timer_check(end)) {
            warn_timeout();
            반환 -1;
        }
        yield();
    }
}

// Wait for the device to be not-busy.
static 정수
await_not_bsy(u16 base)
{
    반환 await_ide(ATA_CB_STAT_BSY, 0, base, IDE_TIMEOUT);
}

// Wait for the device to be ready.
static 정수
await_rdy(u16 base)
{
    반환 await_ide(ATA_CB_STAT_RDY, ATA_CB_STAT_RDY, base, IDE_TIMEOUT);
}

// Wait for ide state - pauses for one ata cycle first.
static inline 정수
pause_await_not_bsy(u16 iobase1, u16 iobase2)
{
    // Wait one PIO transfer cycle.
    inb(iobase2 + ATA_CB_ASTAT);

    반환 await_not_bsy(iobase1);
}

// Wait for ide state - pause for 400ns first.
static inline 정수
ndelay_await_not_bsy(u16 iobase1)
{
    ndelay(400);
    반환 await_not_bsy(iobase1);
}

// Reset a drive
static void
ata_reset(struct atadrive_s *adrive_gf)
{
    struct ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
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
    정수 status = await_not_bsy(iobase1);
    만약 (status < 0)
        goto done;
    만약 (slave) {
        // Change device.
        u32 end = timer_calc(IDE_TIMEOUT);
        위해 (;;) {
            outb(ATA_CB_DH_DEV1, iobase1 + ATA_CB_DH);
            status = ndelay_await_not_bsy(iobase1);
            만약 (status < 0)
                goto done;
            만약 (inb(iobase1 + ATA_CB_DH) == ATA_CB_DH_DEV1)
                break;
            // Change drive request failed to take effect - retry.
            만약 (timer_check(end)) {
                warn_timeout();
                goto done;
            }
        }
    } 아니면 {
        // QEMU doesn't reset dh on reset, so set it explicitly.
        outb(ATA_CB_DH_DEV0, iobase1 + ATA_CB_DH);
    }

    // On a user-reset request, wait for RDY if it is an ATA device.
    u8 type=GET_GLOBALFLAT(adrive_gf->drive.type);
    만약 (type == DTYPE_ATA)
        status = await_rdy(iobase1);

done:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    dprintf(6, "ata_reset exit status=%x\n", status);
}

// Check for drive RDY for 16bit interface command.
static 정수
isready(struct atadrive_s *adrive_gf)
{
    // Read the status from controller
    struct ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u8 status = inb(iobase1 + ATA_CB_STAT);
    만약 ((status & (ATA_CB_STAT_BSY|ATA_CB_STAT_RDY)) == ATA_CB_STAT_RDY)
        반환 DISK_RET_SUCCESS;
    반환 DISK_RET_ENOTREADY;
}


/****************************************************************
 * ATA send command
 ****************************************************************/

struct ata_pio_command {
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
static 정수
send_cmd(struct atadrive_s *adrive_gf, struct ata_pio_command *cmd)
{
    struct ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);

    // Select device
    정수 status = await_not_bsy(iobase1);
    만약 (status < 0)
        반환 status;
    u8 newdh = ((cmd->device & ~ATA_CB_DH_DEV1)
                | (slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0));
    u8 olddh = inb(iobase1 + ATA_CB_DH);
    outb(newdh, iobase1 + ATA_CB_DH);
    만약 ((olddh ^ newdh) & (1<<4)) {
        // Was a device change - wait for device to become not busy.
        status = ndelay_await_not_bsy(iobase1);
        만약 (status < 0)
            반환 status;
    }

    // Check for ATA_CMD_(READ|WRITE)_(SECTORS|DMA)_EXT commands.
    만약 ((cmd->command & ~0x11) == ATA_CMD_READ_SECTORS_EXT) {
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

    반환 0;
}

// Wait for data after calling 'send_cmd'.
static 정수
ata_wait_data(u16 iobase1)
{
    정수 status = ndelay_await_not_bsy(iobase1);
    만약 (status < 0)
        반환 status;

    만약 (status & ATA_CB_STAT_ERR) {
        dprintf(6, "send_cmd : read error (status=%02x err=%02x)\n"
                , status, inb(iobase1 + ATA_CB_ERR));
        반환 -4;
    }
    만약 (!(status & ATA_CB_STAT_DRQ)) {
        dprintf(6, "send_cmd : DRQ not set (status %02x)\n", status);
        반환 -5;
    }

    반환 0;
}

// Send an ata command that does not transfer any further data.
정수
ata_cmd_nondata(struct atadrive_s *adrive_gf, struct ata_pio_command *cmd)
{
    struct ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    정수 ret = send_cmd(adrive_gf, cmd);
    만약 (ret)
        goto fail;
    ret = ndelay_await_not_bsy(iobase1);
    만약 (ret < 0)
        goto fail;

    만약 (ret & ATA_CB_STAT_ERR) {
        dprintf(6, "nondata cmd : read error (status=%02x err=%02x)\n"
                , ret, inb(iobase1 + ATA_CB_ERR));
        ret = -4;
        goto fail;
    }
    만약 (ret & ATA_CB_STAT_DRQ) {
        dprintf(6, "nondata cmd : DRQ set (status %02x)\n", ret);
        ret = -5;
        goto fail;
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    반환 ret;
}


/****************************************************************
 * ATA PIO transfers
 ****************************************************************/

// Transfer 'op->count' blocks (of 'blocksize' bytes) to/from drive
// 'op->drive_fl'.
static 정수
ata_pio_transfer(struct disk_op_s *op, 정수 iswrite, 정수 blocksize)
{
    dprintf(16, "ata_pio_transfer id=%p write=%d count=%d bs=%d buf=%p\n"
            , op->drive_fl, iswrite, op->count, blocksize, op->buf_fl);

    struct atadrive_s *adrive_gf = container_of(
        op->drive_fl, struct atadrive_s, drive);
    struct ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    정수 count = op->count;
    void *buf_fl = op->buf_fl;
    정수 status;
    위해 (;;) {
        만약 (iswrite) {
            // Write data to controller
            dprintf(16, "Write sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            만약 (CONFIG_ATA_PIO32)
                outsl_fl(iobase1, buf_fl, blocksize / 4);
            아니면
                outsw_fl(iobase1, buf_fl, blocksize / 2);
        } 아니면 {
            // Read data from controller
            dprintf(16, "Read sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            만약 (CONFIG_ATA_PIO32)
                insl_fl(iobase1, buf_fl, blocksize / 4);
            아니면
                insw_fl(iobase1, buf_fl, blocksize / 2);
        }
        buf_fl += blocksize;

        status = pause_await_not_bsy(iobase1, iobase2);
        만약 (status < 0) {
            // Error
            op->count -= count;
            반환 status;
        }

        count--;
        만약 (!count)
            break;
        status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ | ATA_CB_STAT_ERR);
        만약 (status != ATA_CB_STAT_DRQ) {
            dprintf(6, "ata_pio_transfer : more sectors left (status %02x)\n"
                    , status);
            op->count -= count;
            반환 -6;
        }
    }

    status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
               | ATA_CB_STAT_ERR);
    만약 (!iswrite)
        status &= ~ATA_CB_STAT_DF;
    만약 (status != 0) {
        dprintf(6, "ata_pio_transfer : no sectors left (status %02x)\n", status);
        반환 -7;
    }

    반환 0;
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

struct sff_dma_prd {
    u32 buf_fl;
    u32 count;
};

// Check if DMA available and setup transfer if so.
static 정수
ata_try_dma(struct disk_op_s *op, 정수 iswrite, 정수 blocksize)
{
    ASSERT16();
    만약 (! CONFIG_ATA_DMA)
        반환 -1;
    u32 dest = (u32)op->buf_fl;
    만약 (dest & 1)
        // Need minimum alignment of 1.
        반환 -1;
    struct atadrive_s *adrive_gf = container_of(
        op->drive_fl, struct atadrive_s, drive);
    struct ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);
    만약 (! iomaster)
        반환 -1;
    u32 bytes = op->count * blocksize;
    만약 (! bytes)
        반환 -1;

    // Build PRD dma structure.
    struct sff_dma_prd *dma = MAKE_FLATPTR(SEG_LOW, ExtraStack);
    struct sff_dma_prd *origdma = dma;
    동안 (bytes) {
        만약 (dma >= &origdma[16])
            // Too many descriptors..
            반환 -1;
        u32 count = bytes;
        u32 max = 0x10000 - (dest & 0xffff);
        만약 (count > max)
            count = max;

        SET_LOWFLAT(dma->buf_fl, dest);
        bytes -= count;
        만약 (!bytes)
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

    반환 0;
}

// Transfer data using DMA.
static 정수
ata_dma_transfer(struct disk_op_s *op)
{
    만약 (! CONFIG_ATA_DMA)
        반환 -1;
    dprintf(16, "ata_dma_transfer id=%p buf=%p\n", op->drive_fl, op->buf_fl);

    struct atadrive_s *adrive_gf = container_of(
        op->drive_fl, struct atadrive_s, drive);
    struct ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);

    // Start bus-master controller.
    u8 oldcmd = inb(iomaster + BM_CMD);
    outb(oldcmd | BM_CMD_START, iomaster + BM_CMD);

    u32 end = timer_calc(IDE_TIMEOUT);
    u8 status;
    위해 (;;) {
        status = inb(iomaster + BM_STATUS);
        만약 (status & BM_STATUS_IRQ)
            break;
        // Transfer in progress
        만약 (timer_check(end)) {
            // Timeout.
            warn_timeout();
            break;
        }
        yield();
    }
    outb(oldcmd & ~BM_CMD_START, iomaster + BM_CMD);

    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    정수 idestatus = pause_await_not_bsy(iobase1, iobase2);

    만약 ((status & (BM_STATUS_IRQ|BM_STATUS_ACTIVE)) == BM_STATUS_IRQ
        && idestatus >= 0x00
        && (idestatus & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
                         | ATA_CB_STAT_ERR)) == 0x00)
        // Success.
        반환 0;

    dprintf(6, "IDE DMA error (dma=%x ide=%x/%x/%x)\n", status, idestatus
            , inb(iobase2 + ATA_CB_ASTAT), inb(iobase1 + ATA_CB_ERR));
    반환 -1;
}


/****************************************************************
 * ATA hard drive functions
 ****************************************************************/

// Transfer data to harddrive using PIO protocol.
static 정수
ata_pio_cmd_data(struct disk_op_s *op, 정수 iswrite, struct ata_pio_command *cmd)
{
    struct atadrive_s *adrive_gf = container_of(
        op->drive_fl, struct atadrive_s, drive);
    struct ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    정수 ret = send_cmd(adrive_gf, cmd);
    만약 (ret)
        goto fail;
    ret = ata_wait_data(iobase1);
    만약 (ret)
        goto fail;
    ret = ata_pio_transfer(op, iswrite, DISK_SECTOR_SIZE);

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    반환 ret;
}

// Transfer data to harddrive using DMA protocol.
static 정수
ata_dma_cmd_data(struct disk_op_s *op, struct ata_pio_command *cmd)
{
    만약 (! CONFIG_ATA_DMA)
        반환 -1;
    struct atadrive_s *adrive_gf = container_of(
        op->drive_fl, struct atadrive_s, drive);
    정수 ret = send_cmd(adrive_gf, cmd);
    만약 (ret)
        반환 ret;
    반환 ata_dma_transfer(op);
}

// Read/write count blocks from a harddrive.
static 정수
ata_readwrite(struct disk_op_s *op, 정수 iswrite)
{
    u64 lba = op->lba;

    정수 usepio = ata_try_dma(op, iswrite, DISK_SECTOR_SIZE);

    struct ata_pio_command cmd;
    memset(&cmd, 0, sizeof(cmd));

    만약 (op->count >= (1<<8) || lba + op->count >= (1<<28)) {
        cmd.sector_count2 = op->count >> 8;
        cmd.lba_low2 = lba >> 24;
        cmd.lba_mid2 = lba >> 32;
        cmd.lba_high2 = lba >> 40;
        lba &= 0xffffff;

        만약 (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS_EXT
                           : ATA_CMD_READ_SECTORS_EXT);
        아니면
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                           : ATA_CMD_READ_DMA_EXT);
    } 아니면 {
        만약 (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS
                           : ATA_CMD_READ_SECTORS);
        아니면
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA
                           : ATA_CMD_READ_DMA);
    }

    cmd.sector_count = op->count;
    cmd.lba_low = lba;
    cmd.lba_mid = lba >> 8;
    cmd.lba_high = lba >> 16;
    cmd.device = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;

    정수 ret;
    만약 (usepio)
        ret = ata_pio_cmd_data(op, iswrite, &cmd);
    아니면
        ret = ata_dma_cmd_data(op, &cmd);
    만약 (ret)
        반환 DISK_RET_EBADTRACK;
    반환 DISK_RET_SUCCESS;
}

// 16bit command demuxer for ATA harddrives.
정수
ata_process_op(struct disk_op_s *op)
{
    만약 (!CONFIG_ATA)
        반환 0;

    struct atadrive_s *adrive_gf = container_of(
        op->drive_fl, struct atadrive_s, drive);
    switch (op->command) {
    case CMD_READ:
        반환 ata_readwrite(op, 0);
    case CMD_WRITE:
        반환 ata_readwrite(op, 1);
    case CMD_RESET:
        ata_reset(adrive_gf);
        반환 DISK_RET_SUCCESS;
    case CMD_ISREADY:
        반환 isready(adrive_gf);
    default:
        반환 default_process_op(op);
    }
}


/****************************************************************
 * ATAPI functions
 ****************************************************************/

#define CDROM_CDB_SIZE 12

// Low-level atapi command transmit function.
정수
ata_atapi_process_op(struct disk_op_s *op)
{
    만약 (! CONFIG_ATA)
        반환 0;

    만약 (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        반환 DISK_RET_EWRITEPROTECT;
    u8 cdbcmd[CDROM_CDB_SIZE];
    정수 blocksize = scsi_fill_cmd(op, cdbcmd, sizeof(cdbcmd));
    만약 (blocksize < 0)
        반환 default_process_op(op);

    struct atadrive_s *adrive_gf = container_of(
        op->drive_fl, struct atadrive_s, drive);
    struct ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    struct ata_pio_command cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.lba_mid = blocksize;
    cmd.lba_high = blocksize >> 8;
    cmd.command = ATA_CMD_PACKET;

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    정수 ret = send_cmd(adrive_gf, &cmd);
    만약 (ret)
        goto fail;
    ret = ata_wait_data(iobase1);
    만약 (ret)
        goto fail;

    // Send command to device
    outsw_fl(iobase1, MAKE_FLATPTR(GET_SEG(SS), cdbcmd), CDROM_CDB_SIZE / 2);

    정수 status = pause_await_not_bsy(iobase1, iobase2);
    만약 (status < 0) {
        ret = status;
        goto fail;
    }

    만약 (status & ATA_CB_STAT_ERR) {
        u8 err = inb(iobase1 + ATA_CB_ERR);
        // skip "Not Ready"
        만약 (err != 0x20)
            dprintf(6, "send_atapi_cmd : read error (status=%02x err=%02x)\n"
                    , status, err);
        ret = -2;
        goto fail;
    }
    만약 (blocksize) {
        만약 (!(status & ATA_CB_STAT_DRQ)) {
            dprintf(6, "send_atapi_cmd : DRQ not set (status %02x)\n", status);
            ret = -3;
            goto fail;
        }

        ret = ata_pio_transfer(op, 0, blocksize);
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    만약 (ret)
        반환 DISK_RET_EBADTRACK;
    반환 DISK_RET_SUCCESS;
}


/****************************************************************
 * ATA detect and init
 ****************************************************************/

// Send an identify device or identify device packet command.
static 정수
send_ata_identity(struct atadrive_s *adrive, u16 *buffer, 정수 command)
{
    memset(buffer, 0, DISK_SECTOR_SIZE);

    struct disk_op_s dop;
    memset(&dop, 0, sizeof(dop));
    dop.drive_fl = &adrive->drive;
    dop.count = 1;
    dop.lba = 1;
    dop.buf_fl = MAKE_FLATPTR(GET_SEG(SS), buffer);

    struct ata_pio_command cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.command = command;

    반환 ata_pio_cmd_data(&dop, 0, &cmd);
}

// Extract the ATA/ATAPI version info.
정수
ata_extract_version(u16 *buffer)
{
    // Extract ATA/ATAPI version.
    u16 ataversion = buffer[80];
    u8 version;
    위해 (version=15; version>0; version--)
        만약 (ataversion & (1<<version))
            break;
    반환 version;
}

#define MAXMODEL 40

// Extract the ATA/ATAPI model info.
char *
ata_extract_model(char *model, u32 size, u16 *buffer)
{
    // Read model name
    정수 i;
    위해 (i=0; i<size/2; i++)
        *(u16*)&model[i*2] = be16_to_cpu(buffer[27+i]);
    model[size] = 0x00;
    nullTrailingSpace(model);
    반환 model;
}

// Common init code between ata and atapi
static struct atadrive_s *
init_atadrive(struct atadrive_s *dummy, u16 *buffer)
{
    struct atadrive_s *adrive = malloc_fseg(sizeof(*adrive));
    만약 (!adrive) {
        warn_noalloc();
        반환 NULL;
    }
    memset(adrive, 0, sizeof(*adrive));
    adrive->chan_gf = dummy->chan_gf;
    adrive->slave = dummy->slave;
    adrive->drive.cntl_id = adrive->chan_gf->ataid * 2 + dummy->slave;
    adrive->drive.removable = (buffer[0] & 0x80) ? 1 : 0;
    반환 adrive;
}

// Detect if the given drive is an atapi - initialize it if so.
static struct atadrive_s *
init_drive_atapi(struct atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE_PACKET command to device
    정수 ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    만약 (ret)
        반환 NULL;

    // Success - setup as ATAPI.
    struct atadrive_s *adrive = init_atadrive(dummy, buffer);
    만약 (!adrive)
        반환 NULL;
    adrive->drive.type = DTYPE_ATA_ATAPI;
    adrive->drive.blksize = CDROM_SECTOR_SIZE;
    adrive->drive.sectors = (u64)-1;
    u8 iscd = ((buffer[0] >> 8) & 0x1f) == 0x05;
    char model[MAXMODEL+1];
    char *desc = znprintf(MAXDESCSIZE
                          , "DVD/CD [ata%d-%d: %s ATAPI-%d %s]"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (iscd ? "DVD/CD" : "Device"));
    dprintf(1, "%s\n", desc);

    // fill cdidmap
    만약 (iscd) {
        정수 prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                            adrive->chan_gf->chanid,
                                            adrive->slave);
        boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                                  adrive->chan_gf->chanid,
                                  adrive->slave,
                                  &(adrive->drive.lchs));
        boot_add_cd(&adrive->drive, desc, prio);
    }

    반환 adrive;
}

// Detect if the given drive is a regular ata drive - initialize it if so.
static struct atadrive_s *
init_drive_ata(struct atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE command to device
    정수 ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_DEVICE);
    만약 (ret)
        반환 NULL;

    // Success - setup as ATA.
    struct atadrive_s *adrive = init_atadrive(dummy, buffer);
    만약 (!adrive)
        반환 NULL;
    adrive->drive.type = DTYPE_ATA;
    adrive->drive.blksize = DISK_SECTOR_SIZE;

    adrive->drive.pchs.cylinder = buffer[1];
    adrive->drive.pchs.head = buffer[3];
    adrive->drive.pchs.sector = buffer[6];

    u64 sectors;
    만약 (buffer[83] & (1 << 10)) // word 83 - lba48 support
        sectors = *(u64*)&buffer[100]; // word 100-103
    아니면
        sectors = *(u32*)&buffer[60]; // word 60 and word 61
    adrive->drive.sectors = sectors;
    u64 adjsize = sectors >> 11;
    char adjprefix = 'M';
    만약 (adjsize >= (1 << 16)) {
        adjsize >>= 10;
        adjprefix = 'G';
    }
    char model[MAXMODEL+1];
    char *desc = znprintf(MAXDESCSIZE
                          , "ata%d-%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (u32)adjsize, adjprefix);
    dprintf(1, "%s\n", desc);

    정수 prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                        adrive->chan_gf->chanid,
                                        adrive->slave);
    boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                              adrive->chan_gf->chanid,
                              adrive->slave,
                              &(adrive->drive.lchs));
    // Register with bcv system.
    boot_add_hd(&adrive->drive, desc, prio);

    반환 adrive;
}

static u32 SpinupEnd;

// Wait for non-busy status and check for "floating bus" condition.
static 정수
powerup_await_non_bsy(u16 base)
{
    u8 orstatus = 0;
    u8 status;
    위해 (;;) {
        status = inb(base+ATA_CB_STAT);
        만약 (!(status & ATA_CB_STAT_BSY))
            break;
        orstatus |= status;
        만약 (orstatus == 0xff) {
            dprintf(4, "powerup IDE floating\n");
            반환 orstatus;
        }
        만약 (timer_check(SpinupEnd)) {
            warn_timeout();
            반환 -1;
        }
        yield();
    }
    dprintf(6, "powerup iobase=%x st=%x\n", base, status);
    반환 status;
}

// Detect any drives attached to a given controller.
static void
ata_detect(void *data)
{
    struct ata_channel_s *chan_gf = data;
    struct atadrive_s dummy;
    memset(&dummy, 0, sizeof(dummy));
    dummy.chan_gf = chan_gf;
    // Device detection
    정수 didreset = 0;
    u8 slave;
    위해 (slave=0; slave<=1; slave++) {
        // Wait for not-bsy.
        u16 iobase1 = chan_gf->iobase1;
        정수 status = powerup_await_non_bsy(iobase1);
        만약 (status < 0)
            continue;
        u8 newdh = slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0;
        outb(newdh, iobase1+ATA_CB_DH);
        ndelay(400);
        status = powerup_await_non_bsy(iobase1);
        만약 (status < 0)
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
        만약 (sc != 0x55 || sn != 0xaa || dh != newdh)
            continue;

        // Prepare new drive.
        dummy.slave = slave;

        // reset the channel
        만약 (!didreset) {
            ata_reset(&dummy);
            didreset = 1;
        }

        // check for ATAPI
        u16 buffer[256];
        struct atadrive_s *adrive = init_drive_atapi(&dummy, buffer);
        만약 (!adrive) {
            // Didn't find an ATAPI drive - look for ATA drive.
            u8 st = inb(iobase1+ATA_CB_STAT);
            만약 (!st)
                // Status not set - can't be a valid drive.
                continue;

            // Wait for RDY.
            정수 ret = await_rdy(iobase1);
            만약 (ret < 0)
                continue;

            // check for ATA.
            adrive = init_drive_ata(&dummy, buffer);
            만약 (!adrive)
                // No ATA drive found
                continue;
        }

        u16 resetresult = buffer[93];
        dprintf(6, "ata_detect resetresult=%04x\n", resetresult);
        만약 (!slave && (resetresult & 0xdf61) == 0x4041)
            // resetresult looks valid and device 0 is responding to
            // device 1 requests - device 1 must not be present - skip
            // detection.
            break;
    }
}

// Initialize an ata controller and detect its drives.
static void
init_controller(struct pci_device *pci, 정수 chanid, 정수 irq
                , u32 port1, u32 port2, u32 master)
{
    static 정수 ataid = 0;
    struct ata_channel_s *chan_gf = malloc_fseg(sizeof(*chan_gf));
    만약 (!chan_gf) {
        warn_noalloc();
        반환;
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
init_pciata(struct pci_device *pci, u8 prog_if)
{
    u8 pciirq = pci_config_readb(pci->bdf, PCI_INTERRUPT_LINE);
    정수 master = 0;
    만약 (CONFIG_ATA_DMA && prog_if & 0x80) {
        // Check for bus-mastering.
        u32 bar = pci_config_readl(pci->bdf, PCI_BASE_ADDRESS_4);
        만약 (bar & PCI_BASE_ADDRESS_SPACE_IO) {
            master = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
            pci_enable_busmaster(pci);
        }
    }

    u32 port1, port2, irq;
    만약 (prog_if & 1) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_1);
        만약 (!port1 || !port2)
            반환;
        irq = pciirq;
    } 아니면 {
        port1 = PORT_ATA1_CMD_BASE;
        port2 = PORT_ATA1_CTRL_BASE;
        irq = IRQ_ATA1;
    }
    init_controller(pci, 0, irq, port1, port2, master);

    만약 (prog_if & 4) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_2);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_3);
        만약 (!port1 || !port2)
            반환;
        irq = pciirq;
    } 아니면 {
        port1 = PORT_ATA2_CMD_BASE;
        port2 = PORT_ATA2_CTRL_BASE;
        irq = IRQ_ATA2;
    }
    init_controller(pci, 1, irq, port1, port2, master ? master + 8 : 0);
}

static void
found_genericata(struct pci_device *pci, void *arg)
{
    init_pciata(pci, pci->prog_if);
}

static void
found_compatibleahci(struct pci_device *pci, void *arg)
{
    만약 (CONFIG_AHCI)
        // Already handled directly via native ahci interface.
        반환;
    init_pciata(pci, 0x8f);
}

static const struct pci_device_id pci_ata_tbl[] = {
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE
                     , found_genericata),
    PCI_DEVICE(PCI_VENDOR_ID_ATI, 0x4391, found_compatibleahci),
    PCI_DEVICE_END,
};

// Locate and init ata controllers.
static void
ata_scan(void)
{
    만약 (CONFIG_QEMU && hlist_empty(&PCIDevices)) {
        // No PCI devices found - probably a QEMU "-M isapc" machine.
        // Try using ISA ports for ATA controllers.
        init_controller(NULL, 0, IRQ_ATA1
                        , PORT_ATA1_CMD_BASE, PORT_ATA1_CTRL_BASE, 0);
        init_controller(NULL, 1, IRQ_ATA2
                        , PORT_ATA2_CMD_BASE, PORT_ATA2_CTRL_BASE, 0);
        반환;
    }

    // Scan PCI bus for ATA adapters
    struct pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_ata_tbl, pci, NULL);
    }
}

void
ata_setup(void)
{
    ASSERT32FLAT();
    만약 (!CONFIG_ATA)
        반환;

    dprintf(3, "init hard drives\n");

    SpinupEnd = timer_calc(IDE_TIMEOUT);
    ata_scan();

    SET_BDA(disk_control_byte, 0xc0);

    enable_hwirq(14, FUNC16(entry_76));
}

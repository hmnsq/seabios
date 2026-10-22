// Low level ATA disk access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "ata.h" // ATA_CB_STAT
#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_GLOBALFLAT
#ଅନ୍ତର୍ଭାଵ "block.h" // struct drive_s
#ଅନ୍ତର୍ଭାଵ "blockcmd.h" // CDB_CMD_READ_10
#ଅନ୍ତର୍ଭାଵ "byteorder.h" // be16_to_cpu
#ଅନ୍ତର୍ଭାଵ "malloc.h" // malloc_fseg
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "pci.h" // pci_config_readb
#ଅନ୍ତର୍ଭାଵ "pcidevice.h" // foreachpci
#ଅନ୍ତର୍ଭାଵ "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#ଅନ୍ତର୍ଭାଵ "pci_regs.h" // PCI_INTERRUPT_LINE
#ଅନ୍ତର୍ଭାଵ "pic.h" // enable_hwirq
#ଅନ୍ତର୍ଭାଵ "stacks.h" // yield
#ଅନ୍ତର୍ଭାଵ "std/disk.h" // DISK_RET_SUCCESS
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // timer_calc
#ଅନ୍ତର୍ଭାଵ "x86.h" // inb

#define IDE_TIMEOUT 32000 //32 seconds max for IDE ops


/****************************************************************
 * Helper functions
 ****************************************************************/

// Wait for the specified ide state
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା
await_ide(u8 mask, u8 flags, u16 base, u16 timeout)
{
    u32 end = timer_calc(timeout);
    ପାଇଁ (;;) {
        u8 status = inb(base+ATA_CB_STAT);
        ଯଦି ((status & mask) == flags)
            ଫେରନ୍ତୁ status;
        ଯଦି (timer_check(end)) {
            warn_timeout();
            ଫେରନ୍ତୁ -1;
        }
        yield();
    }
}

// Wait for the device to be not-busy.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
await_not_bsy(u16 base)
{
    ଫେରନ୍ତୁ await_ide(ATA_CB_STAT_BSY, 0, base, IDE_TIMEOUT);
}

// Wait for the device to be ready.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
await_rdy(u16 base)
{
    ଫେରନ୍ତୁ await_ide(ATA_CB_STAT_RDY, ATA_CB_STAT_RDY, base, IDE_TIMEOUT);
}

// Wait for ide state - pauses for one ata cycle first.
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା
pause_await_not_bsy(u16 iobase1, u16 iobase2)
{
    // Wait one PIO transfer cycle.
    inb(iobase2 + ATA_CB_ASTAT);

    ଫେରନ୍ତୁ await_not_bsy(iobase1);
}

// Wait for ide state - pause for 400ns first.
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ndelay_await_not_bsy(u16 iobase1)
{
    ndelay(400);
    ଫେରନ୍ତୁ await_not_bsy(iobase1);
}

// Reset a drive
ସ୍ଥିର ଶୂନ୍ୟ
ata_reset(ସଂରଚନା atadrive_s *adrive_gf)
{
    ସଂରଚନା ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
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
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = await_not_bsy(iobase1);
    ଯଦି (status < 0)
        ଯାଅ done;
    ଯଦି (slave) {
        // Change device.
        u32 end = timer_calc(IDE_TIMEOUT);
        ପାଇଁ (;;) {
            outb(ATA_CB_DH_DEV1, iobase1 + ATA_CB_DH);
            status = ndelay_await_not_bsy(iobase1);
            ଯଦି (status < 0)
                ଯାଅ done;
            ଯଦି (inb(iobase1 + ATA_CB_DH) == ATA_CB_DH_DEV1)
                ଭାଙ୍ଗ;
            // Change drive request failed to take effect - retry.
            ଯଦି (timer_check(end)) {
                warn_timeout();
                ଯାଅ done;
            }
        }
    } ଅନ୍ୟଥା {
        // QEMU doesn't reset dh on reset, so set it explicitly.
        outb(ATA_CB_DH_DEV0, iobase1 + ATA_CB_DH);
    }

    // On a user-reset request, wait for RDY if it is an ATA device.
    u8 type=GET_GLOBALFLAT(adrive_gf->drive.type);
    ଯଦି (type == DTYPE_ATA)
        status = await_rdy(iobase1);

done:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    dprintf(6, "ata_reset exit status=%x\n", status);
}

// Check for drive RDY for 16bit interface command.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
isready(ସଂରଚନା atadrive_s *adrive_gf)
{
    // Read the status from controller
    ସଂରଚନା ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u8 status = inb(iobase1 + ATA_CB_STAT);
    ଯଦି ((status & (ATA_CB_STAT_BSY|ATA_CB_STAT_RDY)) == ATA_CB_STAT_RDY)
        ଫେରନ୍ତୁ DISK_RET_SUCCESS;
    ଫେରନ୍ତୁ DISK_RET_ENOTREADY;
}


/****************************************************************
 * ATA send command
 ****************************************************************/

ସଂରଚନା ata_pio_command {
    u8 feature;
    u8 sector_ଗଣନା;
    u8 lba_ତଳ;
    u8 lba_mid;
    u8 lba_ଉପର;
    u8 device;
    u8 command;

    u8 feature2;
    u8 sector_count2;
    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
};

// Send an ata command to the drive.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
send_cmd(ସଂରଚନା atadrive_s *adrive_gf, ସଂରଚନା ata_pio_command *cmd)
{
    ସଂରଚନା ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);

    // Select device
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = await_not_bsy(iobase1);
    ଯଦି (status < 0)
        ଫେରନ୍ତୁ status;
    u8 newdh = ((cmd->device & ~ATA_CB_DH_DEV1)
                | (slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0));
    u8 olddh = inb(iobase1 + ATA_CB_DH);
    outb(newdh, iobase1 + ATA_CB_DH);
    ଯଦି ((olddh ^ newdh) & (1<<4)) {
        // Was a device change - wait for device to become not busy.
        status = ndelay_await_not_bsy(iobase1);
        ଯଦି (status < 0)
            ଫେରନ୍ତୁ status;
    }

    // Check for ATA_CMD_(READ|WRITE)_(SECTORS|DMA)_EXT commands.
    ଯଦି ((cmd->command & ~0x11) == ATA_CMD_READ_SECTORS_EXT) {
        outb(cmd->feature2, iobase1 + ATA_CB_FR);
        outb(cmd->sector_count2, iobase1 + ATA_CB_SC);
        outb(cmd->lba_low2, iobase1 + ATA_CB_SN);
        outb(cmd->lba_mid2, iobase1 + ATA_CB_CL);
        outb(cmd->lba_high2, iobase1 + ATA_CB_CH);
    }
    outb(cmd->feature, iobase1 + ATA_CB_FR);
    outb(cmd->sector_ଗଣନା, iobase1 + ATA_CB_SC);
    outb(cmd->lba_ତଳ, iobase1 + ATA_CB_SN);
    outb(cmd->lba_mid, iobase1 + ATA_CB_CL);
    outb(cmd->lba_ଉପର, iobase1 + ATA_CB_CH);
    outb(cmd->command, iobase1 + ATA_CB_CMD);

    ଫେରନ୍ତୁ 0;
}

// Wait for data after calling 'send_cmd'.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ata_wait_data(u16 iobase1)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = ndelay_await_not_bsy(iobase1);
    ଯଦି (status < 0)
        ଫେରନ୍ତୁ status;

    ଯଦି (status & ATA_CB_STAT_ERR) {
        dprintf(6, "send_cmd : read error (status=%02x err=%02x)\n"
                , status, inb(iobase1 + ATA_CB_ERR));
        ଫେରନ୍ତୁ -4;
    }
    ଯଦି (!(status & ATA_CB_STAT_DRQ)) {
        dprintf(6, "send_cmd : DRQ not set (status %02x)\n", status);
        ଫେରନ୍ତୁ -5;
    }

    ଫେରନ୍ତୁ 0;
}

// Send an ata command that does not transfer any further data.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ata_cmd_nondata(ସଂରଚନା atadrive_s *adrive_gf, ସଂରଚନା ata_pio_command *cmd)
{
    ସଂରଚନା ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = send_cmd(adrive_gf, cmd);
    ଯଦି (ret)
        ଯାଅ fail;
    ret = ndelay_await_not_bsy(iobase1);
    ଯଦି (ret < 0)
        ଯାଅ fail;

    ଯଦି (ret & ATA_CB_STAT_ERR) {
        dprintf(6, "nondata cmd : read error (status=%02x err=%02x)\n"
                , ret, inb(iobase1 + ATA_CB_ERR));
        ret = -4;
        ଯାଅ fail;
    }
    ଯଦି (ret & ATA_CB_STAT_DRQ) {
        dprintf(6, "nondata cmd : DRQ set (status %02x)\n", ret);
        ret = -5;
        ଯାଅ fail;
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    ଫେରନ୍ତୁ ret;
}


/****************************************************************
 * ATA PIO transfers
 ****************************************************************/

// Transfer 'op->count' blocks (of 'blocksize' bytes) to/from drive
// 'op->drive_fl'.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ata_pio_transfer(ସଂରଚନା disk_op_s *op, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iswrite, ପୂର୍ଣ୍ଣସଂଖ୍ୟା blocksize)
{
    dprintf(16, "ata_pio_transfer id=%p write=%d count=%d bs=%d buf=%p\n"
            , op->drive_fl, iswrite, op->ଗଣନା_2, blocksize, op->buf_fl);

    ସଂରଚନା atadrive_s *adrive_gf = container_of(
        op->drive_fl, ସଂରଚନା atadrive_s, drive);
    ସଂରଚନା ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = op->ଗଣନା_2;
    ଶୂନ୍ୟ *buf_fl = op->buf_fl;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
    ପାଇଁ (;;) {
        ଯଦି (iswrite) {
            // Write data to controller
            dprintf(16, "Write sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            ଯଦି (CONFIG_ATA_PIO32)
                outsl_fl(iobase1, buf_fl, blocksize / 4);
            ଅନ୍ୟଥା
                outsw_fl(iobase1, buf_fl, blocksize / 2);
        } ଅନ୍ୟଥା {
            // Read data from controller
            dprintf(16, "Read sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            ଯଦି (CONFIG_ATA_PIO32)
                insl_fl(iobase1, buf_fl, blocksize / 4);
            ଅନ୍ୟଥା
                insw_fl(iobase1, buf_fl, blocksize / 2);
        }
        buf_fl += blocksize;

        status = pause_await_not_bsy(iobase1, iobase2);
        ଯଦି (status < 0) {
            // Error
            op->ଗଣନା_2 -= ଗଣନା_2;
            ଫେରନ୍ତୁ status;
        }

        ଗଣନା_2--;
        ଯଦି (!ଗଣନା_2)
            ଭାଙ୍ଗ;
        status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ | ATA_CB_STAT_ERR);
        ଯଦି (status != ATA_CB_STAT_DRQ) {
            dprintf(6, "ata_pio_transfer : more sectors left (status %02x)\n"
                    , status);
            op->ଗଣନା_2 -= ଗଣନା_2;
            ଫେରନ୍ତୁ -6;
        }
    }

    status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
               | ATA_CB_STAT_ERR);
    ଯଦି (!iswrite)
        status &= ~ATA_CB_STAT_DF;
    ଯଦି (status != 0) {
        dprintf(6, "ata_pio_transfer : no sectors left (status %02x)\n", status);
        ଫେରନ୍ତୁ -7;
    }

    ଫେରନ୍ତୁ 0;
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

ସଂରଚନା sff_dma_prd {
    u32 buf_fl;
    u32 ଗଣନା_2;
};

// Check if DMA available and setup transfer if so.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ata_try_dma(ସଂରଚନା disk_op_s *op, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iswrite, ପୂର୍ଣ୍ଣସଂଖ୍ୟା blocksize)
{
    ASSERT16();
    ଯଦି (! CONFIG_ATA_DMA)
        ଫେରନ୍ତୁ -1;
    u32 dest = (u32)op->buf_fl;
    ଯଦି (dest & 1)
        // Need minimum alignment of 1.
        ଫେରନ୍ତୁ -1;
    ସଂରଚନା atadrive_s *adrive_gf = container_of(
        op->drive_fl, ସଂରଚନା atadrive_s, drive);
    ସଂରଚନା ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);
    ଯଦି (! iomaster)
        ଫେରନ୍ତୁ -1;
    u32 bytes = op->ଗଣନା_2 * blocksize;
    ଯଦି (! bytes)
        ଫେରନ୍ତୁ -1;

    // Build PRD dma structure.
    ସଂରଚନା sff_dma_prd *dma = MAKE_FLATPTR(SEG_ତଳ, Extra_ଗଦା);
    ସଂରଚନା sff_dma_prd *origdma = dma;
    ଯେତେବେଳେ (bytes) {
        ଯଦି (dma >= &origdma[16])
            // Too many descriptors..
            ଫେରନ୍ତୁ -1;
        u32 ଗଣନା_2 = bytes;
        u32 max = 0x10000 - (dest & 0xffff);
        ଯଦି (ଗଣନା_2 > max)
            ଗଣନା_2 = max;

        SET_LOWFLAT(dma->buf_fl, dest);
        bytes -= ଗଣନା_2;
        ଯଦି (!bytes)
            // Last descriptor.
            ଗଣନା_2 |= 1<<31;
        dprintf(16, "dma@%p: %08x %08x\n", dma, dest, ଗଣନା_2);
        dest += ଗଣନା_2;
        SET_LOWFLAT(dma->ଗଣନା_2, ଗଣନା_2);
        dma++;
    }

    // Program bus-master controller.
    outl((u32)origdma, iomaster + BM_TABLE);
    u8 oldcmd = inb(iomaster + BM_CMD) & ~(BM_CMD_MEMWRITE|BM_CMD_START);
    outb(oldcmd | (iswrite ? 0x00 : BM_CMD_MEMWRITE), iomaster + BM_CMD);
    outb(BM_STATUS_ERROR|BM_STATUS_IRQ, iomaster + BM_STATUS);

    ଫେରନ୍ତୁ 0;
}

// Transfer data using DMA.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ata_dma_transfer(ସଂରଚନା disk_op_s *op)
{
    ଯଦି (! CONFIG_ATA_DMA)
        ଫେରନ୍ତୁ -1;
    dprintf(16, "ata_dma_transfer id=%p buf=%p\n", op->drive_fl, op->buf_fl);

    ସଂରଚନା atadrive_s *adrive_gf = container_of(
        op->drive_fl, ସଂରଚନା atadrive_s, drive);
    ସଂରଚନା ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);

    // Start bus-master controller.
    u8 oldcmd = inb(iomaster + BM_CMD);
    outb(oldcmd | BM_CMD_START, iomaster + BM_CMD);

    u32 end = timer_calc(IDE_TIMEOUT);
    u8 status;
    ପାଇଁ (;;) {
        status = inb(iomaster + BM_STATUS);
        ଯଦି (status & BM_STATUS_IRQ)
            ଭାଙ୍ଗ;
        // Transfer in progress
        ଯଦି (timer_check(end)) {
            // Timeout.
            warn_timeout();
            ଭାଙ୍ଗ;
        }
        yield();
    }
    outb(oldcmd & ~BM_CMD_START, iomaster + BM_CMD);

    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା idestatus = pause_await_not_bsy(iobase1, iobase2);

    ଯଦି ((status & (BM_STATUS_IRQ|BM_STATUS_ACTIVE)) == BM_STATUS_IRQ
        && idestatus >= 0x00
        && (idestatus & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
                         | ATA_CB_STAT_ERR)) == 0x00)
        // Success.
        ଫେରନ୍ତୁ 0;

    dprintf(6, "IDE DMA error (dma=%x ide=%x/%x/%x)\n", status, idestatus
            , inb(iobase2 + ATA_CB_ASTAT), inb(iobase1 + ATA_CB_ERR));
    ଫେରନ୍ତୁ -1;
}


/****************************************************************
 * ATA hard drive functions
 ****************************************************************/

// Transfer data to harddrive using PIO protocol.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ata_pio_cmd_data(ସଂରଚନା disk_op_s *op, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iswrite, ସଂରଚନା ata_pio_command *cmd)
{
    ସଂରଚନା atadrive_s *adrive_gf = container_of(
        op->drive_fl, ସଂରଚନା atadrive_s, drive);
    ସଂରଚନା ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = send_cmd(adrive_gf, cmd);
    ଯଦି (ret)
        ଯାଅ fail;
    ret = ata_wait_data(iobase1);
    ଯଦି (ret)
        ଯାଅ fail;
    ret = ata_pio_transfer(op, iswrite, DISK_SECTOR_SIZE);

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    ଫେରନ୍ତୁ ret;
}

// Transfer data to harddrive using DMA protocol.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ata_dma_cmd_data(ସଂରଚନା disk_op_s *op, ସଂରଚନା ata_pio_command *cmd)
{
    ଯଦି (! CONFIG_ATA_DMA)
        ଫେରନ୍ତୁ -1;
    ସଂରଚନା atadrive_s *adrive_gf = container_of(
        op->drive_fl, ସଂରଚନା atadrive_s, drive);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = send_cmd(adrive_gf, cmd);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ret;
    ଫେରନ୍ତୁ ata_dma_transfer(op);
}

// Read/write count blocks from a harddrive.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ata_readwrite(ସଂରଚନା disk_op_s *op, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iswrite)
{
    u64 lba = op->lba;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା usepio = ata_try_dma(op, iswrite, DISK_SECTOR_SIZE);

    ସଂରଚନା ata_pio_command cmd;
    memset(&cmd, 0, ଆକାର(cmd));

    ଯଦି (op->ଗଣନା_2 >= (1<<8) || lba + op->ଗଣନା_2 >= (1<<28)) {
        cmd.sector_count2 = op->ଗଣନା_2 >> 8;
        cmd.lba_low2 = lba >> 24;
        cmd.lba_mid2 = lba >> 32;
        cmd.lba_high2 = lba >> 40;
        lba &= 0xffffff;

        ଯଦି (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS_EXT
                           : ATA_CMD_READ_SECTORS_EXT);
        ଅନ୍ୟଥା
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                           : ATA_CMD_READ_DMA_EXT);
    } ଅନ୍ୟଥା {
        ଯଦି (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS
                           : ATA_CMD_READ_SECTORS);
        ଅନ୍ୟଥା
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA
                           : ATA_CMD_READ_DMA);
    }

    cmd.sector_ଗଣନା = op->ଗଣନା_2;
    cmd.lba_ତଳ = lba;
    cmd.lba_mid = lba >> 8;
    cmd.lba_ଉପର = lba >> 16;
    cmd.device = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
    ଯଦି (usepio)
        ret = ata_pio_cmd_data(op, iswrite, &cmd);
    ଅନ୍ୟଥା
        ret = ata_dma_cmd_data(op, &cmd);
    ଯଦି (ret)
        ଫେରନ୍ତୁ DISK_RET_EBADTRACK;
    ଫେରନ୍ତୁ DISK_RET_SUCCESS;
}

// 16bit command demuxer for ATA harddrives.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ata_process_op(ସଂରଚନା disk_op_s *op)
{
    ଯଦି (!CONFIG_ATA)
        ଫେରନ୍ତୁ 0;

    ସଂରଚନା atadrive_s *adrive_gf = container_of(
        op->drive_fl, ସଂରଚନା atadrive_s, drive);
    ବାଛ (op->command) {
    ସ୍ଥିତି CMD_READ:
        ଫେରନ୍ତୁ ata_readwrite(op, 0);
    ସ୍ଥିତି CMD_WRITE:
        ଫେରନ୍ତୁ ata_readwrite(op, 1);
    ସ୍ଥିତି CMD_RESET:
        ata_reset(adrive_gf);
        ଫେରନ୍ତୁ DISK_RET_SUCCESS;
    ସ୍ଥିତି CMD_ISREADY:
        ଫେରନ୍ତୁ isready(adrive_gf);
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ଫେରନ୍ତୁ default_process_op(op);
    }
}


/****************************************************************
 * ATAPI functions
 ****************************************************************/

#define CDROM_CDB_SIZE 12

// Low-level atapi command transmit function.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ata_atapi_process_op(ସଂରଚନା disk_op_s *op)
{
    ଯଦି (! CONFIG_ATA)
        ଫେରନ୍ତୁ 0;

    ଯଦି (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        ଫେରନ୍ତୁ DISK_RET_EWRITEPROTECT;
    u8 cdbcmd[CDROM_CDB_SIZE];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା blocksize = scsi_fill_cmd(op, cdbcmd, ଆକାର(cdbcmd));
    ଯଦି (blocksize < 0)
        ଫେରନ୍ତୁ default_process_op(op);

    ସଂରଚନା atadrive_s *adrive_gf = container_of(
        op->drive_fl, ସଂରଚନା atadrive_s, drive);
    ସଂରଚନା ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    ସଂରଚନା ata_pio_command cmd;
    memset(&cmd, 0, ଆକାର(cmd));
    cmd.lba_mid = blocksize;
    cmd.lba_ଉପର = blocksize >> 8;
    cmd.command = ATA_CMD_PACKET;

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = send_cmd(adrive_gf, &cmd);
    ଯଦି (ret)
        ଯାଅ fail;
    ret = ata_wait_data(iobase1);
    ଯଦି (ret)
        ଯାଅ fail;

    // Send command to device
    outsw_fl(iobase1, MAKE_FLATPTR(GET_SEG(SS), cdbcmd), CDROM_CDB_SIZE / 2);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = pause_await_not_bsy(iobase1, iobase2);
    ଯଦି (status < 0) {
        ret = status;
        ଯାଅ fail;
    }

    ଯଦି (status & ATA_CB_STAT_ERR) {
        u8 err = inb(iobase1 + ATA_CB_ERR);
        // skip "Not Ready"
        ଯଦି (err != 0x20)
            dprintf(6, "send_atapi_cmd : read error (status=%02x err=%02x)\n"
                    , status, err);
        ret = -2;
        ଯାଅ fail;
    }
    ଯଦି (blocksize) {
        ଯଦି (!(status & ATA_CB_STAT_DRQ)) {
            dprintf(6, "send_atapi_cmd : DRQ not set (status %02x)\n", status);
            ret = -3;
            ଯାଅ fail;
        }

        ret = ata_pio_transfer(op, 0, blocksize);
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    ଯଦି (ret)
        ଫେରନ୍ତୁ DISK_RET_EBADTRACK;
    ଫେରନ୍ତୁ DISK_RET_SUCCESS;
}


/****************************************************************
 * ATA detect and init
 ****************************************************************/

// Send an identify device or identify device packet command.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
send_ata_identity(ସଂରଚନା atadrive_s *adrive, u16 *buffer, ପୂର୍ଣ୍ଣସଂଖ୍ୟା command)
{
    memset(buffer, 0, DISK_SECTOR_SIZE);

    ସଂରଚନା disk_op_s dop;
    memset(&dop, 0, ଆକାର(dop));
    dop.drive_fl = &adrive->drive;
    dop.ଗଣନା_2 = 1;
    dop.lba = 1;
    dop.buf_fl = MAKE_FLATPTR(GET_SEG(SS), buffer);

    ସଂରଚନା ata_pio_command cmd;
    memset(&cmd, 0, ଆକାର(cmd));
    cmd.command = command;

    ଫେରନ୍ତୁ ata_pio_cmd_data(&dop, 0, &cmd);
}

// Extract the ATA/ATAPI version info.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ata_extract_version(u16 *buffer)
{
    // Extract ATA/ATAPI version.
    u16 ataversion = buffer[80];
    u8 version;
    ପାଇଁ (version=15; version>0; version--)
        ଯଦି (ataversion & (1<<version))
            ଭାଙ୍ଗ;
    ଫେରନ୍ତୁ version;
}

#define MAXMODEL 40

// Extract the ATA/ATAPI model info.
ଅକ୍ଷର *
ata_extract_model(ଅକ୍ଷର *model, u32 size, u16 *buffer)
{
    // Read model name
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i=0; i<size/2; i++)
        *(u16*)&model[i*2] = be16_to_cpu(buffer[27+i]);
    model[size] = 0x00;
    nullTrailingSpace(model);
    ଫେରନ୍ତୁ model;
}

// Common init code between ata and atapi
ସ୍ଥିର ସଂରଚନା atadrive_s *
init_atadrive(ସଂରଚନା atadrive_s *dummy, u16 *buffer)
{
    ସଂରଚନା atadrive_s *adrive = malloc_fseg(ଆକାର(*adrive));
    ଯଦି (!adrive) {
        warn_noalloc();
        ଫେରନ୍ତୁ ଖାଲି;
    }
    memset(adrive, 0, ଆକାର(*adrive));
    adrive->chan_gf = dummy->chan_gf;
    adrive->slave = dummy->slave;
    adrive->drive.cntl_id = adrive->chan_gf->ataid * 2 + dummy->slave;
    adrive->drive.removable = (buffer[0] & 0x80) ? 1 : 0;
    ଫେରନ୍ତୁ adrive;
}

// Detect if the given drive is an atapi - initialize it if so.
ସ୍ଥିର ସଂରଚନା atadrive_s *
init_drive_atapi(ସଂରଚନା atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE_PACKET command to device
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ଖାଲି;

    // Success - setup as ATAPI.
    ସଂରଚନା atadrive_s *adrive = init_atadrive(dummy, buffer);
    ଯଦି (!adrive)
        ଫେରନ୍ତୁ ଖାଲି;
    adrive->drive.type = DTYPE_ATA_ATAPI;
    adrive->drive.blksize = CDROM_SECTOR_SIZE;
    adrive->drive.sectors = (u64)-1;
    u8 iscd = ((buffer[0] >> 8) & 0x1f) == 0x05;
    ଅକ୍ଷର model[MAXMODEL+1];
    ଅକ୍ଷର *desc = znprintf(MAXDESCSIZE
                          , "DVD/CD [ata%d-%d: %s ATAPI-%d %s]"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (iscd ? "DVD/CD" : "Device"));
    dprintf(1, "%s\n", desc);

    // fill cdidmap
    ଯଦି (iscd) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                            adrive->chan_gf->chanid,
                                            adrive->slave);
        boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                                  adrive->chan_gf->chanid,
                                  adrive->slave,
                                  &(adrive->drive.lchs));
        boot_add_cd(&adrive->drive, desc, prio);
    }

    ଫେରନ୍ତୁ adrive;
}

// Detect if the given drive is a regular ata drive - initialize it if so.
ସ୍ଥିର ସଂରଚନା atadrive_s *
init_drive_ata(ସଂରଚନା atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE command to device
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_DEVICE);
    ଯଦି (ret)
        ଫେରନ୍ତୁ ଖାଲି;

    // Success - setup as ATA.
    ସଂରଚନା atadrive_s *adrive = init_atadrive(dummy, buffer);
    ଯଦି (!adrive)
        ଫେରନ୍ତୁ ଖାଲି;
    adrive->drive.type = DTYPE_ATA;
    adrive->drive.blksize = DISK_SECTOR_SIZE;

    adrive->drive.pchs.cylinder = buffer[1];
    adrive->drive.pchs.head = buffer[3];
    adrive->drive.pchs.sector = buffer[6];

    u64 sectors;
    ଯଦି (buffer[83] & (1 << 10)) // word 83 - lba48 support
        sectors = *(u64*)&buffer[100]; // word 100-103
    ଅନ୍ୟଥା
        sectors = *(u32*)&buffer[60]; // word 60 and word 61
    adrive->drive.sectors = sectors;
    u64 adjsize = sectors >> 11;
    ଅକ୍ଷର adjprefix = 'M';
    ଯଦି (adjsize >= (1 << 16)) {
        adjsize >>= 10;
        adjprefix = 'G';
    }
    ଅକ୍ଷର model[MAXMODEL+1];
    ଅକ୍ଷର *desc = znprintf(MAXDESCSIZE
                          , "ata%d-%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (u32)adjsize, adjprefix);
    dprintf(1, "%s\n", desc);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                        adrive->chan_gf->chanid,
                                        adrive->slave);
    boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                              adrive->chan_gf->chanid,
                              adrive->slave,
                              &(adrive->drive.lchs));
    // Register with bcv system.
    boot_add_hd(&adrive->drive, desc, prio);

    ଫେରନ୍ତୁ adrive;
}

ସ୍ଥିର u32 SpinupEnd;

// Wait for non-busy status and check for "floating bus" condition.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
powerup_await_non_bsy(u16 base)
{
    u8 orstatus = 0;
    u8 status;
    ପାଇଁ (;;) {
        status = inb(base+ATA_CB_STAT);
        ଯଦି (!(status & ATA_CB_STAT_BSY))
            ଭାଙ୍ଗ;
        orstatus |= status;
        ଯଦି (orstatus == 0xff) {
            dprintf(4, "powerup IDE floating\n");
            ଫେରନ୍ତୁ orstatus;
        }
        ଯଦି (timer_check(SpinupEnd)) {
            warn_timeout();
            ଫେରନ୍ତୁ -1;
        }
        yield();
    }
    dprintf(6, "powerup iobase=%x st=%x\n", base, status);
    ଫେରନ୍ତୁ status;
}

// Detect any drives attached to a given controller.
ସ୍ଥିର ଶୂନ୍ୟ
ata_detect(ଶୂନ୍ୟ *data)
{
    ସଂରଚନା ata_channel_s *chan_gf = data;
    ସଂରଚନା atadrive_s dummy;
    memset(&dummy, 0, ଆକାର(dummy));
    dummy.chan_gf = chan_gf;
    // Device detection
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା didreset = 0;
    u8 slave;
    ପାଇଁ (slave=0; slave<=1; slave++) {
        // Wait for not-bsy.
        u16 iobase1 = chan_gf->iobase1;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = powerup_await_non_bsy(iobase1);
        ଯଦି (status < 0)
            ଜାରି;
        u8 newdh = slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0;
        outb(newdh, iobase1+ATA_CB_DH);
        ndelay(400);
        status = powerup_await_non_bsy(iobase1);
        ଯଦି (status < 0)
            ଜାରି;

        // Check if ioport registers look valid.
        outb(newdh, iobase1+ATA_CB_DH);
        u8 dh = inb(iobase1+ATA_CB_DH);
        outb(0x55, iobase1+ATA_CB_SC);
        outb(0xaa, iobase1+ATA_CB_SN);
        u8 sc = inb(iobase1+ATA_CB_SC);
        u8 sn = inb(iobase1+ATA_CB_SN);
        dprintf(6, "ata_detect ata%d-%d: sc=%x sn=%x dh=%x\n"
                , chan_gf->ataid, slave, sc, sn, dh);
        ଯଦି (sc != 0x55 || sn != 0xaa || dh != newdh)
            ଜାରି;

        // Prepare new drive.
        dummy.slave = slave;

        // reset the channel
        ଯଦି (!didreset) {
            ata_reset(&dummy);
            didreset = 1;
        }

        // check for ATAPI
        u16 buffer[256];
        ସଂରଚନା atadrive_s *adrive = init_drive_atapi(&dummy, buffer);
        ଯଦି (!adrive) {
            // Didn't find an ATAPI drive - look for ATA drive.
            u8 st = inb(iobase1+ATA_CB_STAT);
            ଯଦି (!st)
                // Status not set - can't be a valid drive.
                ଜାରି;

            // Wait for RDY.
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = await_rdy(iobase1);
            ଯଦି (ret < 0)
                ଜାରି;

            // check for ATA.
            adrive = init_drive_ata(&dummy, buffer);
            ଯଦି (!adrive)
                // No ATA drive found
                ଜାରି;
        }

        u16 resetresult = buffer[93];
        dprintf(6, "ata_detect resetresult=%04x\n", resetresult);
        ଯଦି (!slave && (resetresult & 0xdf61) == 0x4041)
            // resetresult looks valid and device 0 is responding to
            // device 1 requests - device 1 must not be present - skip
            // detection.
            ଭାଙ୍ଗ;
    }
}

// Initialize an ata controller and detect its drives.
ସ୍ଥିର ଶୂନ୍ୟ
init_controller(ସଂରଚନା pci_device *pci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା chanid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା irq
                , u32 port1, u32 port2, u32 master)
{
    ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ataid = 0;
    ସଂରଚନା ata_channel_s *chan_gf = malloc_fseg(ଆକାର(*chan_gf));
    ଯଦି (!chan_gf) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
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
ସ୍ଥିର ଶୂନ୍ୟ
init_pciata(ସଂରଚନା pci_device *pci, u8 prog_if)
{
    u8 pciirq = pci_config_readb(pci->bdf, PCI_INTERRUPT_ଧାଡ଼ି);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା master = 0;
    ଯଦି (CONFIG_ATA_DMA && prog_if & 0x80) {
        // Check for bus-mastering.
        u32 bar = pci_config_readl(pci->bdf, PCI_BASE_ADDRESS_4);
        ଯଦି (bar & PCI_BASE_ADDRESS_SPACE_IO) {
            master = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
            pci_enable_busmaster(pci);
        }
    }

    u32 port1, port2, irq;
    ଯଦି (prog_if & 1) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_1);
        ଯଦି (!port1 || !port2)
            ଫେରନ୍ତୁ;
        irq = pciirq;
    } ଅନ୍ୟଥା {
        port1 = PORT_ATA1_CMD_BASE;
        port2 = PORT_ATA1_CTRL_BASE;
        irq = IRQ_ATA1;
    }
    init_controller(pci, 0, irq, port1, port2, master);

    ଯଦି (prog_if & 4) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_2);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_3);
        ଯଦି (!port1 || !port2)
            ଫେରନ୍ତୁ;
        irq = pciirq;
    } ଅନ୍ୟଥା {
        port1 = PORT_ATA2_CMD_BASE;
        port2 = PORT_ATA2_CTRL_BASE;
        irq = IRQ_ATA2;
    }
    init_controller(pci, 1, irq, port1, port2, master ? master + 8 : 0);
}

ସ୍ଥିର ଶୂନ୍ୟ
found_genericata(ସଂରଚନା pci_device *pci, ଶୂନ୍ୟ *arg)
{
    init_pciata(pci, pci->prog_if);
}

ସ୍ଥିର ଶୂନ୍ୟ
found_compatibleahci(ସଂରଚନା pci_device *pci, ଶୂନ୍ୟ *arg)
{
    ଯଦି (CONFIG_AHCI)
        // Already handled directly via native ahci interface.
        ଫେରନ୍ତୁ;
    init_pciata(pci, 0x8f);
}

ସ୍ଥିର ଅଚଳ ସଂରଚନା pci_device_id pci_ata_tbl[] = {
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE
                     , found_genericata),
    PCI_DEVICE(PCI_VENDOR_ID_ATI, 0x4391, found_compatibleahci),
    PCI_DEVICE_END,
};

// Locate and init ata controllers.
ସ୍ଥିର ଶୂନ୍ୟ
ata_scan(ଶୂନ୍ୟ)
{
    ଯଦି (CONFIG_QEMU && hlist_empty(&PCIDevices)) {
        // No PCI devices found - probably a QEMU "-M isapc" machine.
        // Try using ISA ports for ATA controllers.
        init_controller(ଖାଲି, 0, IRQ_ATA1
                        , PORT_ATA1_CMD_BASE, PORT_ATA1_CTRL_BASE, 0);
        init_controller(ଖାଲି, 1, IRQ_ATA2
                        , PORT_ATA2_CMD_BASE, PORT_ATA2_CTRL_BASE, 0);
        ଫେରନ୍ତୁ;
    }

    // Scan PCI bus for ATA adapters
    ସଂରଚନା pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_ata_tbl, pci, ଖାଲି);
    }
}

ଶୂନ୍ୟ
ata_setup(ଶୂନ୍ୟ)
{
    ASSERT32FLAT();
    ଯଦି (!CONFIG_ATA)
        ଫେରନ୍ତୁ;

    dprintf(3, "init hard drives\n");

    SpinupEnd = timer_calc(IDE_TIMEOUT);
    ata_scan();

    SET_BDA(disk_control_byte, 0xc0);

    enable_hwirq(14, FUNC16(entry_76));
}

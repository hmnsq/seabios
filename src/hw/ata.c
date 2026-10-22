// Low level ATA disk access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "ata.h" // ATA_CB_STAT
#അന്തര്ഭാവ "biosvar.h" // GET_GLOBALFLAT
#അന്തര്ഭാവ "block.h" // struct drive_s
#അന്തര്ഭാവ "blockcmd.h" // CDB_CMD_READ_10
#അന്തര്ഭാവ "byteorder.h" // be16_to_cpu
#അന്തര്ഭാവ "malloc.h" // malloc_fseg
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "pci.h" // pci_config_readb
#അന്തര്ഭാവ "pcidevice.h" // foreachpci
#അന്തര്ഭാവ "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#അന്തര്ഭാവ "pci_regs.h" // PCI_INTERRUPT_LINE
#അന്തര്ഭാവ "pic.h" // enable_hwirq
#അന്തര്ഭാവ "stacks.h" // yield
#അന്തര്ഭാവ "std/disk.h" // DISK_RET_SUCCESS
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // timer_calc
#അന്തര്ഭാവ "x86.h" // inb

#define IDE_TIMEOUT 32000 //32 seconds max for IDE ops


/****************************************************************
 * Helper functions
 ****************************************************************/

// Wait for the specified ide state
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ
await_ide(u8 mask, u8 flags, u16 base, u16 timeout)
{
    u32 end = timer_calc(timeout);
    വേണ്ടി (;;) {
        u8 status = inb(base+ATA_CB_STAT);
        എങ്കിൽ ((status & mask) == flags)
            തിരികെ status;
        എങ്കിൽ (timer_check(end)) {
            warn_timeout();
            തിരികെ -1;
        }
        yield();
    }
}

// Wait for the device to be not-busy.
സ്ഥിതം പൂർണ്ണസംഖ്യ
await_not_bsy(u16 base)
{
    തിരികെ await_ide(ATA_CB_STAT_BSY, 0, base, IDE_TIMEOUT);
}

// Wait for the device to be ready.
സ്ഥിതം പൂർണ്ണസംഖ്യ
await_rdy(u16 base)
{
    തിരികെ await_ide(ATA_CB_STAT_RDY, ATA_CB_STAT_RDY, base, IDE_TIMEOUT);
}

// Wait for ide state - pauses for one ata cycle first.
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ
pause_await_not_bsy(u16 iobase1, u16 iobase2)
{
    // Wait one PIO transfer cycle.
    inb(iobase2 + ATA_CB_ASTAT);

    തിരികെ await_not_bsy(iobase1);
}

// Wait for ide state - pause for 400ns first.
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ
ndelay_await_not_bsy(u16 iobase1)
{
    ndelay(400);
    തിരികെ await_not_bsy(iobase1);
}

// Reset a drive
സ്ഥിതം ശൂന്യം
ata_reset(ഘടന atadrive_s *adrive_gf)
{
    ഘടന ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
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
    പൂർണ്ണസംഖ്യ status = await_not_bsy(iobase1);
    എങ്കിൽ (status < 0)
        പോകുക done;
    എങ്കിൽ (slave) {
        // Change device.
        u32 end = timer_calc(IDE_TIMEOUT);
        വേണ്ടി (;;) {
            outb(ATA_CB_DH_DEV1, iobase1 + ATA_CB_DH);
            status = ndelay_await_not_bsy(iobase1);
            എങ്കിൽ (status < 0)
                പോകുക done;
            എങ്കിൽ (inb(iobase1 + ATA_CB_DH) == ATA_CB_DH_DEV1)
                നിർത്തുക;
            // Change drive request failed to take effect - retry.
            എങ്കിൽ (timer_check(end)) {
                warn_timeout();
                പോകുക done;
            }
        }
    } അല്ലെങ്കിൽ {
        // QEMU doesn't reset dh on reset, so set it explicitly.
        outb(ATA_CB_DH_DEV0, iobase1 + ATA_CB_DH);
    }

    // On a user-reset request, wait for RDY if it is an ATA device.
    u8 type=GET_GLOBALFLAT(adrive_gf->drive.type);
    എങ്കിൽ (type == DTYPE_ATA)
        status = await_rdy(iobase1);

done:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    dprintf(6, "ata_reset exit status=%x\n", status);
}

// Check for drive RDY for 16bit interface command.
സ്ഥിതം പൂർണ്ണസംഖ്യ
isready(ഘടന atadrive_s *adrive_gf)
{
    // Read the status from controller
    ഘടന ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u8 status = inb(iobase1 + ATA_CB_STAT);
    എങ്കിൽ ((status & (ATA_CB_STAT_BSY|ATA_CB_STAT_RDY)) == ATA_CB_STAT_RDY)
        തിരികെ DISK_RET_SUCCESS;
    തിരികെ DISK_RET_ENOTREADY;
}


/****************************************************************
 * ATA send command
 ****************************************************************/

ഘടന ata_pio_command {
    u8 feature;
    u8 sector_എണ്ണം;
    u8 lba_താഴെയുള്ള;
    u8 lba_mid;
    u8 lba_മുകളിലെ;
    u8 device;
    u8 command;

    u8 feature2;
    u8 sector_count2;
    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
};

// Send an ata command to the drive.
സ്ഥിതം പൂർണ്ണസംഖ്യ
send_cmd(ഘടന atadrive_s *adrive_gf, ഘടന ata_pio_command *cmd)
{
    ഘടന ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);

    // Select device
    പൂർണ്ണസംഖ്യ status = await_not_bsy(iobase1);
    എങ്കിൽ (status < 0)
        തിരികെ status;
    u8 newdh = ((cmd->device & ~ATA_CB_DH_DEV1)
                | (slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0));
    u8 olddh = inb(iobase1 + ATA_CB_DH);
    outb(newdh, iobase1 + ATA_CB_DH);
    എങ്കിൽ ((olddh ^ newdh) & (1<<4)) {
        // Was a device change - wait for device to become not busy.
        status = ndelay_await_not_bsy(iobase1);
        എങ്കിൽ (status < 0)
            തിരികെ status;
    }

    // Check for ATA_CMD_(READ|WRITE)_(SECTORS|DMA)_EXT commands.
    എങ്കിൽ ((cmd->command & ~0x11) == ATA_CMD_READ_SECTORS_EXT) {
        outb(cmd->feature2, iobase1 + ATA_CB_FR);
        outb(cmd->sector_count2, iobase1 + ATA_CB_SC);
        outb(cmd->lba_low2, iobase1 + ATA_CB_SN);
        outb(cmd->lba_mid2, iobase1 + ATA_CB_CL);
        outb(cmd->lba_high2, iobase1 + ATA_CB_CH);
    }
    outb(cmd->feature, iobase1 + ATA_CB_FR);
    outb(cmd->sector_എണ്ണം, iobase1 + ATA_CB_SC);
    outb(cmd->lba_താഴെയുള്ള, iobase1 + ATA_CB_SN);
    outb(cmd->lba_mid, iobase1 + ATA_CB_CL);
    outb(cmd->lba_മുകളിലെ, iobase1 + ATA_CB_CH);
    outb(cmd->command, iobase1 + ATA_CB_CMD);

    തിരികെ 0;
}

// Wait for data after calling 'send_cmd'.
സ്ഥിതം പൂർണ്ണസംഖ്യ
ata_wait_data(u16 iobase1)
{
    പൂർണ്ണസംഖ്യ status = ndelay_await_not_bsy(iobase1);
    എങ്കിൽ (status < 0)
        തിരികെ status;

    എങ്കിൽ (status & ATA_CB_STAT_ERR) {
        dprintf(6, "send_cmd : read error (status=%02x err=%02x)\n"
                , status, inb(iobase1 + ATA_CB_ERR));
        തിരികെ -4;
    }
    എങ്കിൽ (!(status & ATA_CB_STAT_DRQ)) {
        dprintf(6, "send_cmd : DRQ not set (status %02x)\n", status);
        തിരികെ -5;
    }

    തിരികെ 0;
}

// Send an ata command that does not transfer any further data.
പൂർണ്ണസംഖ്യ
ata_cmd_nondata(ഘടന atadrive_s *adrive_gf, ഘടന ata_pio_command *cmd)
{
    ഘടന ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    പൂർണ്ണസംഖ്യ ret = send_cmd(adrive_gf, cmd);
    എങ്കിൽ (ret)
        പോകുക fail;
    ret = ndelay_await_not_bsy(iobase1);
    എങ്കിൽ (ret < 0)
        പോകുക fail;

    എങ്കിൽ (ret & ATA_CB_STAT_ERR) {
        dprintf(6, "nondata cmd : read error (status=%02x err=%02x)\n"
                , ret, inb(iobase1 + ATA_CB_ERR));
        ret = -4;
        പോകുക fail;
    }
    എങ്കിൽ (ret & ATA_CB_STAT_DRQ) {
        dprintf(6, "nondata cmd : DRQ set (status %02x)\n", ret);
        ret = -5;
        പോകുക fail;
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    തിരികെ ret;
}


/****************************************************************
 * ATA PIO transfers
 ****************************************************************/

// Transfer 'op->count' blocks (of 'blocksize' bytes) to/from drive
// 'op->drive_fl'.
സ്ഥിതം പൂർണ്ണസംഖ്യ
ata_pio_transfer(ഘടന disk_op_s *op, പൂർണ്ണസംഖ്യ iswrite, പൂർണ്ണസംഖ്യ blocksize)
{
    dprintf(16, "ata_pio_transfer id=%p write=%d count=%d bs=%d buf=%p\n"
            , op->drive_fl, iswrite, op->എണ്ണം, blocksize, op->buf_fl);

    ഘടന atadrive_s *adrive_gf = container_of(
        op->drive_fl, ഘടന atadrive_s, drive);
    ഘടന ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    പൂർണ്ണസംഖ്യ എണ്ണം = op->എണ്ണം;
    ശൂന്യം *buf_fl = op->buf_fl;
    പൂർണ്ണസംഖ്യ status;
    വേണ്ടി (;;) {
        എങ്കിൽ (iswrite) {
            // Write data to controller
            dprintf(16, "Write sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            എങ്കിൽ (CONFIG_ATA_PIO32)
                outsl_fl(iobase1, buf_fl, blocksize / 4);
            അല്ലെങ്കിൽ
                outsw_fl(iobase1, buf_fl, blocksize / 2);
        } അല്ലെങ്കിൽ {
            // Read data from controller
            dprintf(16, "Read sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            എങ്കിൽ (CONFIG_ATA_PIO32)
                insl_fl(iobase1, buf_fl, blocksize / 4);
            അല്ലെങ്കിൽ
                insw_fl(iobase1, buf_fl, blocksize / 2);
        }
        buf_fl += blocksize;

        status = pause_await_not_bsy(iobase1, iobase2);
        എങ്കിൽ (status < 0) {
            // Error
            op->എണ്ണം -= എണ്ണം;
            തിരികെ status;
        }

        എണ്ണം--;
        എങ്കിൽ (!എണ്ണം)
            നിർത്തുക;
        status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ | ATA_CB_STAT_ERR);
        എങ്കിൽ (status != ATA_CB_STAT_DRQ) {
            dprintf(6, "ata_pio_transfer : more sectors left (status %02x)\n"
                    , status);
            op->എണ്ണം -= എണ്ണം;
            തിരികെ -6;
        }
    }

    status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
               | ATA_CB_STAT_ERR);
    എങ്കിൽ (!iswrite)
        status &= ~ATA_CB_STAT_DF;
    എങ്കിൽ (status != 0) {
        dprintf(6, "ata_pio_transfer : no sectors left (status %02x)\n", status);
        തിരികെ -7;
    }

    തിരികെ 0;
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

ഘടന sff_dma_prd {
    u32 buf_fl;
    u32 എണ്ണം;
};

// Check if DMA available and setup transfer if so.
സ്ഥിതം പൂർണ്ണസംഖ്യ
ata_try_dma(ഘടന disk_op_s *op, പൂർണ്ണസംഖ്യ iswrite, പൂർണ്ണസംഖ്യ blocksize)
{
    ASSERT16();
    എങ്കിൽ (! CONFIG_ATA_DMA)
        തിരികെ -1;
    u32 dest = (u32)op->buf_fl;
    എങ്കിൽ (dest & 1)
        // Need minimum alignment of 1.
        തിരികെ -1;
    ഘടന atadrive_s *adrive_gf = container_of(
        op->drive_fl, ഘടന atadrive_s, drive);
    ഘടന ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);
    എങ്കിൽ (! iomaster)
        തിരികെ -1;
    u32 bytes = op->എണ്ണം * blocksize;
    എങ്കിൽ (! bytes)
        തിരികെ -1;

    // Build PRD dma structure.
    ഘടന sff_dma_prd *dma = MAKE_FLATPTR(SEG_താഴെയുള്ള, Extra_അട്ടി);
    ഘടന sff_dma_prd *origdma = dma;
    ആകുമ്പോൾ (bytes) {
        എങ്കിൽ (dma >= &origdma[16])
            // Too many descriptors..
            തിരികെ -1;
        u32 എണ്ണം = bytes;
        u32 max = 0x10000 - (dest & 0xffff);
        എങ്കിൽ (എണ്ണം > max)
            എണ്ണം = max;

        SET_LOWFLAT(dma->buf_fl, dest);
        bytes -= എണ്ണം;
        എങ്കിൽ (!bytes)
            // Last descriptor.
            എണ്ണം |= 1<<31;
        dprintf(16, "dma@%p: %08x %08x\n", dma, dest, എണ്ണം);
        dest += എണ്ണം;
        SET_LOWFLAT(dma->എണ്ണം, എണ്ണം);
        dma++;
    }

    // Program bus-master controller.
    outl((u32)origdma, iomaster + BM_TABLE);
    u8 oldcmd = inb(iomaster + BM_CMD) & ~(BM_CMD_MEMWRITE|BM_CMD_START);
    outb(oldcmd | (iswrite ? 0x00 : BM_CMD_MEMWRITE), iomaster + BM_CMD);
    outb(BM_STATUS_ERROR|BM_STATUS_IRQ, iomaster + BM_STATUS);

    തിരികെ 0;
}

// Transfer data using DMA.
സ്ഥിതം പൂർണ്ണസംഖ്യ
ata_dma_transfer(ഘടന disk_op_s *op)
{
    എങ്കിൽ (! CONFIG_ATA_DMA)
        തിരികെ -1;
    dprintf(16, "ata_dma_transfer id=%p buf=%p\n", op->drive_fl, op->buf_fl);

    ഘടന atadrive_s *adrive_gf = container_of(
        op->drive_fl, ഘടന atadrive_s, drive);
    ഘടന ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);

    // Start bus-master controller.
    u8 oldcmd = inb(iomaster + BM_CMD);
    outb(oldcmd | BM_CMD_START, iomaster + BM_CMD);

    u32 end = timer_calc(IDE_TIMEOUT);
    u8 status;
    വേണ്ടി (;;) {
        status = inb(iomaster + BM_STATUS);
        എങ്കിൽ (status & BM_STATUS_IRQ)
            നിർത്തുക;
        // Transfer in progress
        എങ്കിൽ (timer_check(end)) {
            // Timeout.
            warn_timeout();
            നിർത്തുക;
        }
        yield();
    }
    outb(oldcmd & ~BM_CMD_START, iomaster + BM_CMD);

    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    പൂർണ്ണസംഖ്യ idestatus = pause_await_not_bsy(iobase1, iobase2);

    എങ്കിൽ ((status & (BM_STATUS_IRQ|BM_STATUS_ACTIVE)) == BM_STATUS_IRQ
        && idestatus >= 0x00
        && (idestatus & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
                         | ATA_CB_STAT_ERR)) == 0x00)
        // Success.
        തിരികെ 0;

    dprintf(6, "IDE DMA error (dma=%x ide=%x/%x/%x)\n", status, idestatus
            , inb(iobase2 + ATA_CB_ASTAT), inb(iobase1 + ATA_CB_ERR));
    തിരികെ -1;
}


/****************************************************************
 * ATA hard drive functions
 ****************************************************************/

// Transfer data to harddrive using PIO protocol.
സ്ഥിതം പൂർണ്ണസംഖ്യ
ata_pio_cmd_data(ഘടന disk_op_s *op, പൂർണ്ണസംഖ്യ iswrite, ഘടന ata_pio_command *cmd)
{
    ഘടന atadrive_s *adrive_gf = container_of(
        op->drive_fl, ഘടന atadrive_s, drive);
    ഘടന ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    പൂർണ്ണസംഖ്യ ret = send_cmd(adrive_gf, cmd);
    എങ്കിൽ (ret)
        പോകുക fail;
    ret = ata_wait_data(iobase1);
    എങ്കിൽ (ret)
        പോകുക fail;
    ret = ata_pio_transfer(op, iswrite, DISK_SECTOR_SIZE);

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    തിരികെ ret;
}

// Transfer data to harddrive using DMA protocol.
സ്ഥിതം പൂർണ്ണസംഖ്യ
ata_dma_cmd_data(ഘടന disk_op_s *op, ഘടന ata_pio_command *cmd)
{
    എങ്കിൽ (! CONFIG_ATA_DMA)
        തിരികെ -1;
    ഘടന atadrive_s *adrive_gf = container_of(
        op->drive_fl, ഘടന atadrive_s, drive);
    പൂർണ്ണസംഖ്യ ret = send_cmd(adrive_gf, cmd);
    എങ്കിൽ (ret)
        തിരികെ ret;
    തിരികെ ata_dma_transfer(op);
}

// Read/write count blocks from a harddrive.
സ്ഥിതം പൂർണ്ണസംഖ്യ
ata_readwrite(ഘടന disk_op_s *op, പൂർണ്ണസംഖ്യ iswrite)
{
    u64 lba = op->lba;

    പൂർണ്ണസംഖ്യ usepio = ata_try_dma(op, iswrite, DISK_SECTOR_SIZE);

    ഘടന ata_pio_command cmd;
    memset(&cmd, 0, വലുപ്പം(cmd));

    എങ്കിൽ (op->എണ്ണം >= (1<<8) || lba + op->എണ്ണം >= (1<<28)) {
        cmd.sector_count2 = op->എണ്ണം >> 8;
        cmd.lba_low2 = lba >> 24;
        cmd.lba_mid2 = lba >> 32;
        cmd.lba_high2 = lba >> 40;
        lba &= 0xffffff;

        എങ്കിൽ (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS_EXT
                           : ATA_CMD_READ_SECTORS_EXT);
        അല്ലെങ്കിൽ
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                           : ATA_CMD_READ_DMA_EXT);
    } അല്ലെങ്കിൽ {
        എങ്കിൽ (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS
                           : ATA_CMD_READ_SECTORS);
        അല്ലെങ്കിൽ
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA
                           : ATA_CMD_READ_DMA);
    }

    cmd.sector_എണ്ണം = op->എണ്ണം;
    cmd.lba_താഴെയുള്ള = lba;
    cmd.lba_mid = lba >> 8;
    cmd.lba_മുകളിലെ = lba >> 16;
    cmd.device = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;

    പൂർണ്ണസംഖ്യ ret;
    എങ്കിൽ (usepio)
        ret = ata_pio_cmd_data(op, iswrite, &cmd);
    അല്ലെങ്കിൽ
        ret = ata_dma_cmd_data(op, &cmd);
    എങ്കിൽ (ret)
        തിരികെ DISK_RET_EBADTRACK;
    തിരികെ DISK_RET_SUCCESS;
}

// 16bit command demuxer for ATA harddrives.
പൂർണ്ണസംഖ്യ
ata_process_op(ഘടന disk_op_s *op)
{
    എങ്കിൽ (!CONFIG_ATA)
        തിരികെ 0;

    ഘടന atadrive_s *adrive_gf = container_of(
        op->drive_fl, ഘടന atadrive_s, drive);
    തിരഞ്ഞെടുപ്പ് (op->command) {
    അവസ്ഥ CMD_READ:
        തിരികെ ata_readwrite(op, 0);
    അവസ്ഥ CMD_WRITE:
        തിരികെ ata_readwrite(op, 1);
    അവസ്ഥ CMD_RESET:
        ata_reset(adrive_gf);
        തിരികെ DISK_RET_SUCCESS;
    അവസ്ഥ CMD_ISREADY:
        തിരികെ isready(adrive_gf);
    സ്ഥിരസ്ഥിതി:
        തിരികെ default_process_op(op);
    }
}


/****************************************************************
 * ATAPI functions
 ****************************************************************/

#define CDROM_CDB_SIZE 12

// Low-level atapi command transmit function.
പൂർണ്ണസംഖ്യ
ata_atapi_process_op(ഘടന disk_op_s *op)
{
    എങ്കിൽ (! CONFIG_ATA)
        തിരികെ 0;

    എങ്കിൽ (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        തിരികെ DISK_RET_EWRITEPROTECT;
    u8 cdbcmd[CDROM_CDB_SIZE];
    പൂർണ്ണസംഖ്യ blocksize = scsi_fill_cmd(op, cdbcmd, വലുപ്പം(cdbcmd));
    എങ്കിൽ (blocksize < 0)
        തിരികെ default_process_op(op);

    ഘടന atadrive_s *adrive_gf = container_of(
        op->drive_fl, ഘടന atadrive_s, drive);
    ഘടന ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    ഘടന ata_pio_command cmd;
    memset(&cmd, 0, വലുപ്പം(cmd));
    cmd.lba_mid = blocksize;
    cmd.lba_മുകളിലെ = blocksize >> 8;
    cmd.command = ATA_CMD_PACKET;

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    പൂർണ്ണസംഖ്യ ret = send_cmd(adrive_gf, &cmd);
    എങ്കിൽ (ret)
        പോകുക fail;
    ret = ata_wait_data(iobase1);
    എങ്കിൽ (ret)
        പോകുക fail;

    // Send command to device
    outsw_fl(iobase1, MAKE_FLATPTR(GET_SEG(SS), cdbcmd), CDROM_CDB_SIZE / 2);

    പൂർണ്ണസംഖ്യ status = pause_await_not_bsy(iobase1, iobase2);
    എങ്കിൽ (status < 0) {
        ret = status;
        പോകുക fail;
    }

    എങ്കിൽ (status & ATA_CB_STAT_ERR) {
        u8 err = inb(iobase1 + ATA_CB_ERR);
        // skip "Not Ready"
        എങ്കിൽ (err != 0x20)
            dprintf(6, "send_atapi_cmd : read error (status=%02x err=%02x)\n"
                    , status, err);
        ret = -2;
        പോകുക fail;
    }
    എങ്കിൽ (blocksize) {
        എങ്കിൽ (!(status & ATA_CB_STAT_DRQ)) {
            dprintf(6, "send_atapi_cmd : DRQ not set (status %02x)\n", status);
            ret = -3;
            പോകുക fail;
        }

        ret = ata_pio_transfer(op, 0, blocksize);
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    എങ്കിൽ (ret)
        തിരികെ DISK_RET_EBADTRACK;
    തിരികെ DISK_RET_SUCCESS;
}


/****************************************************************
 * ATA detect and init
 ****************************************************************/

// Send an identify device or identify device packet command.
സ്ഥിതം പൂർണ്ണസംഖ്യ
send_ata_identity(ഘടന atadrive_s *adrive, u16 *buffer, പൂർണ്ണസംഖ്യ command)
{
    memset(buffer, 0, DISK_SECTOR_SIZE);

    ഘടന disk_op_s dop;
    memset(&dop, 0, വലുപ്പം(dop));
    dop.drive_fl = &adrive->drive;
    dop.എണ്ണം = 1;
    dop.lba = 1;
    dop.buf_fl = MAKE_FLATPTR(GET_SEG(SS), buffer);

    ഘടന ata_pio_command cmd;
    memset(&cmd, 0, വലുപ്പം(cmd));
    cmd.command = command;

    തിരികെ ata_pio_cmd_data(&dop, 0, &cmd);
}

// Extract the ATA/ATAPI version info.
പൂർണ്ണസംഖ്യ
ata_extract_version(u16 *buffer)
{
    // Extract ATA/ATAPI version.
    u16 ataversion = buffer[80];
    u8 version;
    വേണ്ടി (version=15; version>0; version--)
        എങ്കിൽ (ataversion & (1<<version))
            നിർത്തുക;
    തിരികെ version;
}

#define MAXMODEL 40

// Extract the ATA/ATAPI model info.
അക്ഷരം *
ata_extract_model(അക്ഷരം *model, u32 size, u16 *buffer)
{
    // Read model name
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i=0; i<size/2; i++)
        *(u16*)&model[i*2] = be16_to_cpu(buffer[27+i]);
    model[size] = 0x00;
    nullTrailingSpace(model);
    തിരികെ model;
}

// Common init code between ata and atapi
സ്ഥിതം ഘടന atadrive_s *
init_atadrive(ഘടന atadrive_s *dummy, u16 *buffer)
{
    ഘടന atadrive_s *adrive = malloc_fseg(വലുപ്പം(*adrive));
    എങ്കിൽ (!adrive) {
        warn_noalloc();
        തിരികെ ശൂന്യ;
    }
    memset(adrive, 0, വലുപ്പം(*adrive));
    adrive->chan_gf = dummy->chan_gf;
    adrive->slave = dummy->slave;
    adrive->drive.cntl_id = adrive->chan_gf->ataid * 2 + dummy->slave;
    adrive->drive.removable = (buffer[0] & 0x80) ? 1 : 0;
    തിരികെ adrive;
}

// Detect if the given drive is an atapi - initialize it if so.
സ്ഥിതം ഘടന atadrive_s *
init_drive_atapi(ഘടന atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE_PACKET command to device
    പൂർണ്ണസംഖ്യ ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    എങ്കിൽ (ret)
        തിരികെ ശൂന്യ;

    // Success - setup as ATAPI.
    ഘടന atadrive_s *adrive = init_atadrive(dummy, buffer);
    എങ്കിൽ (!adrive)
        തിരികെ ശൂന്യ;
    adrive->drive.type = DTYPE_ATA_ATAPI;
    adrive->drive.blksize = CDROM_SECTOR_SIZE;
    adrive->drive.sectors = (u64)-1;
    u8 iscd = ((buffer[0] >> 8) & 0x1f) == 0x05;
    അക്ഷരം model[MAXMODEL+1];
    അക്ഷരം *desc = znprintf(MAXDESCSIZE
                          , "DVD/CD [ata%d-%d: %s ATAPI-%d %s]"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (iscd ? "DVD/CD" : "Device"));
    dprintf(1, "%s\n", desc);

    // fill cdidmap
    എങ്കിൽ (iscd) {
        പൂർണ്ണസംഖ്യ prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                            adrive->chan_gf->chanid,
                                            adrive->slave);
        boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                                  adrive->chan_gf->chanid,
                                  adrive->slave,
                                  &(adrive->drive.lchs));
        boot_add_cd(&adrive->drive, desc, prio);
    }

    തിരികെ adrive;
}

// Detect if the given drive is a regular ata drive - initialize it if so.
സ്ഥിതം ഘടന atadrive_s *
init_drive_ata(ഘടന atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE command to device
    പൂർണ്ണസംഖ്യ ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_DEVICE);
    എങ്കിൽ (ret)
        തിരികെ ശൂന്യ;

    // Success - setup as ATA.
    ഘടന atadrive_s *adrive = init_atadrive(dummy, buffer);
    എങ്കിൽ (!adrive)
        തിരികെ ശൂന്യ;
    adrive->drive.type = DTYPE_ATA;
    adrive->drive.blksize = DISK_SECTOR_SIZE;

    adrive->drive.pchs.cylinder = buffer[1];
    adrive->drive.pchs.head = buffer[3];
    adrive->drive.pchs.sector = buffer[6];

    u64 sectors;
    എങ്കിൽ (buffer[83] & (1 << 10)) // word 83 - lba48 support
        sectors = *(u64*)&buffer[100]; // word 100-103
    അല്ലെങ്കിൽ
        sectors = *(u32*)&buffer[60]; // word 60 and word 61
    adrive->drive.sectors = sectors;
    u64 adjsize = sectors >> 11;
    അക്ഷരം adjprefix = 'M';
    എങ്കിൽ (adjsize >= (1 << 16)) {
        adjsize >>= 10;
        adjprefix = 'G';
    }
    അക്ഷരം model[MAXMODEL+1];
    അക്ഷരം *desc = znprintf(MAXDESCSIZE
                          , "ata%d-%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (u32)adjsize, adjprefix);
    dprintf(1, "%s\n", desc);

    പൂർണ്ണസംഖ്യ prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                        adrive->chan_gf->chanid,
                                        adrive->slave);
    boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                              adrive->chan_gf->chanid,
                              adrive->slave,
                              &(adrive->drive.lchs));
    // Register with bcv system.
    boot_add_hd(&adrive->drive, desc, prio);

    തിരികെ adrive;
}

സ്ഥിതം u32 SpinupEnd;

// Wait for non-busy status and check for "floating bus" condition.
സ്ഥിതം പൂർണ്ണസംഖ്യ
powerup_await_non_bsy(u16 base)
{
    u8 orstatus = 0;
    u8 status;
    വേണ്ടി (;;) {
        status = inb(base+ATA_CB_STAT);
        എങ്കിൽ (!(status & ATA_CB_STAT_BSY))
            നിർത്തുക;
        orstatus |= status;
        എങ്കിൽ (orstatus == 0xff) {
            dprintf(4, "powerup IDE floating\n");
            തിരികെ orstatus;
        }
        എങ്കിൽ (timer_check(SpinupEnd)) {
            warn_timeout();
            തിരികെ -1;
        }
        yield();
    }
    dprintf(6, "powerup iobase=%x st=%x\n", base, status);
    തിരികെ status;
}

// Detect any drives attached to a given controller.
സ്ഥിതം ശൂന്യം
ata_detect(ശൂന്യം *data)
{
    ഘടന ata_channel_s *chan_gf = data;
    ഘടന atadrive_s dummy;
    memset(&dummy, 0, വലുപ്പം(dummy));
    dummy.chan_gf = chan_gf;
    // Device detection
    പൂർണ്ണസംഖ്യ didreset = 0;
    u8 slave;
    വേണ്ടി (slave=0; slave<=1; slave++) {
        // Wait for not-bsy.
        u16 iobase1 = chan_gf->iobase1;
        പൂർണ്ണസംഖ്യ status = powerup_await_non_bsy(iobase1);
        എങ്കിൽ (status < 0)
            തുടരുക;
        u8 newdh = slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0;
        outb(newdh, iobase1+ATA_CB_DH);
        ndelay(400);
        status = powerup_await_non_bsy(iobase1);
        എങ്കിൽ (status < 0)
            തുടരുക;

        // Check if ioport registers look valid.
        outb(newdh, iobase1+ATA_CB_DH);
        u8 dh = inb(iobase1+ATA_CB_DH);
        outb(0x55, iobase1+ATA_CB_SC);
        outb(0xaa, iobase1+ATA_CB_SN);
        u8 sc = inb(iobase1+ATA_CB_SC);
        u8 sn = inb(iobase1+ATA_CB_SN);
        dprintf(6, "ata_detect ata%d-%d: sc=%x sn=%x dh=%x\n"
                , chan_gf->ataid, slave, sc, sn, dh);
        എങ്കിൽ (sc != 0x55 || sn != 0xaa || dh != newdh)
            തുടരുക;

        // Prepare new drive.
        dummy.slave = slave;

        // reset the channel
        എങ്കിൽ (!didreset) {
            ata_reset(&dummy);
            didreset = 1;
        }

        // check for ATAPI
        u16 buffer[256];
        ഘടന atadrive_s *adrive = init_drive_atapi(&dummy, buffer);
        എങ്കിൽ (!adrive) {
            // Didn't find an ATAPI drive - look for ATA drive.
            u8 st = inb(iobase1+ATA_CB_STAT);
            എങ്കിൽ (!st)
                // Status not set - can't be a valid drive.
                തുടരുക;

            // Wait for RDY.
            പൂർണ്ണസംഖ്യ ret = await_rdy(iobase1);
            എങ്കിൽ (ret < 0)
                തുടരുക;

            // check for ATA.
            adrive = init_drive_ata(&dummy, buffer);
            എങ്കിൽ (!adrive)
                // No ATA drive found
                തുടരുക;
        }

        u16 resetresult = buffer[93];
        dprintf(6, "ata_detect resetresult=%04x\n", resetresult);
        എങ്കിൽ (!slave && (resetresult & 0xdf61) == 0x4041)
            // resetresult looks valid and device 0 is responding to
            // device 1 requests - device 1 must not be present - skip
            // detection.
            നിർത്തുക;
    }
}

// Initialize an ata controller and detect its drives.
സ്ഥിതം ശൂന്യം
init_controller(ഘടന pci_device *pci, പൂർണ്ണസംഖ്യ chanid, പൂർണ്ണസംഖ്യ irq
                , u32 port1, u32 port2, u32 master)
{
    സ്ഥിതം പൂർണ്ണസംഖ്യ ataid = 0;
    ഘടന ata_channel_s *chan_gf = malloc_fseg(വലുപ്പം(*chan_gf));
    എങ്കിൽ (!chan_gf) {
        warn_noalloc();
        തിരികെ;
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
സ്ഥിതം ശൂന്യം
init_pciata(ഘടന pci_device *pci, u8 prog_if)
{
    u8 pciirq = pci_config_readb(pci->bdf, PCI_INTERRUPT_വരി);
    പൂർണ്ണസംഖ്യ master = 0;
    എങ്കിൽ (CONFIG_ATA_DMA && prog_if & 0x80) {
        // Check for bus-mastering.
        u32 bar = pci_config_readl(pci->bdf, PCI_BASE_ADDRESS_4);
        എങ്കിൽ (bar & PCI_BASE_ADDRESS_SPACE_IO) {
            master = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
            pci_enable_busmaster(pci);
        }
    }

    u32 port1, port2, irq;
    എങ്കിൽ (prog_if & 1) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_1);
        എങ്കിൽ (!port1 || !port2)
            തിരികെ;
        irq = pciirq;
    } അല്ലെങ്കിൽ {
        port1 = PORT_ATA1_CMD_BASE;
        port2 = PORT_ATA1_CTRL_BASE;
        irq = IRQ_ATA1;
    }
    init_controller(pci, 0, irq, port1, port2, master);

    എങ്കിൽ (prog_if & 4) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_2);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_3);
        എങ്കിൽ (!port1 || !port2)
            തിരികെ;
        irq = pciirq;
    } അല്ലെങ്കിൽ {
        port1 = PORT_ATA2_CMD_BASE;
        port2 = PORT_ATA2_CTRL_BASE;
        irq = IRQ_ATA2;
    }
    init_controller(pci, 1, irq, port1, port2, master ? master + 8 : 0);
}

സ്ഥിതം ശൂന്യം
found_genericata(ഘടന pci_device *pci, ശൂന്യം *arg)
{
    init_pciata(pci, pci->prog_if);
}

സ്ഥിതം ശൂന്യം
found_compatibleahci(ഘടന pci_device *pci, ശൂന്യം *arg)
{
    എങ്കിൽ (CONFIG_AHCI)
        // Already handled directly via native ahci interface.
        തിരികെ;
    init_pciata(pci, 0x8f);
}

സ്ഥിതം സ്ഥിരം ഘടന pci_device_id pci_ata_tbl[] = {
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE
                     , found_genericata),
    PCI_DEVICE(PCI_VENDOR_ID_ATI, 0x4391, found_compatibleahci),
    PCI_DEVICE_END,
};

// Locate and init ata controllers.
സ്ഥിതം ശൂന്യം
ata_scan(ശൂന്യം)
{
    എങ്കിൽ (CONFIG_QEMU && hlist_empty(&PCIDevices)) {
        // No PCI devices found - probably a QEMU "-M isapc" machine.
        // Try using ISA ports for ATA controllers.
        init_controller(ശൂന്യ, 0, IRQ_ATA1
                        , PORT_ATA1_CMD_BASE, PORT_ATA1_CTRL_BASE, 0);
        init_controller(ശൂന്യ, 1, IRQ_ATA2
                        , PORT_ATA2_CMD_BASE, PORT_ATA2_CTRL_BASE, 0);
        തിരികെ;
    }

    // Scan PCI bus for ATA adapters
    ഘടന pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_ata_tbl, pci, ശൂന്യ);
    }
}

ശൂന്യം
ata_setup(ശൂന്യം)
{
    ASSERT32FLAT();
    എങ്കിൽ (!CONFIG_ATA)
        തിരികെ;

    dprintf(3, "init hard drives\n");

    SpinupEnd = timer_calc(IDE_TIMEOUT);
    ata_scan();

    SET_BDA(disk_control_byte, 0xc0);

    enable_hwirq(14, FUNC16(entry_76));
}

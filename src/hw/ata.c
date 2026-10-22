// Low level ATA disk access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "ata.h" // ATA_CB_STAT
#अन्तर्भाव "biosvar.h" // GET_GLOBALFLAT
#अन्तर्भाव "block.h" // struct drive_s
#अन्तर्भाव "blockcmd.h" // CDB_CMD_READ_10
#अन्तर्भाव "byteorder.h" // be16_to_cpu
#अन्तर्भाव "malloc.h" // malloc_fseg
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "pci.h" // pci_config_readb
#अन्तर्भाव "pcidevice.h" // foreachpci
#अन्तर्भाव "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#अन्तर्भाव "pci_regs.h" // PCI_INTERRUPT_LINE
#अन्तर्भाव "pic.h" // enable_hwirq
#अन्तर्भाव "stacks.h" // yield
#अन्तर्भाव "std/disk.h" // DISK_RET_SUCCESS
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // timer_calc
#अन्तर्भाव "x86.h" // inb

#define IDE_TIMEOUT 32000 //32 seconds max for IDE ops


/****************************************************************
 * Helper functions
 ****************************************************************/

// Wait for the specified ide state
स्थिर अन्तःस्थ पूर्णाङ्क
await_ide(u8 mask, u8 flags, u16 base, u16 timeout)
{
    u32 end = timer_calc(timeout);
    प्रति (;;) {
        u8 status = inb(base+ATA_CB_STAT);
        यदि ((status & mask) == flags)
            प्रत्यागम status;
        यदि (timer_check(end)) {
            warn_timeout();
            प्रत्यागम -1;
        }
        yield();
    }
}

// Wait for the device to be not-busy.
स्थिर पूर्णाङ्क
await_not_bsy(u16 base)
{
    प्रत्यागम await_ide(ATA_CB_STAT_BSY, 0, base, IDE_TIMEOUT);
}

// Wait for the device to be ready.
स्थिर पूर्णाङ्क
await_rdy(u16 base)
{
    प्रत्यागम await_ide(ATA_CB_STAT_RDY, ATA_CB_STAT_RDY, base, IDE_TIMEOUT);
}

// Wait for ide state - pauses for one ata cycle first.
स्थिर अन्तःस्थ पूर्णाङ्क
pause_await_not_bsy(u16 iobase1, u16 iobase2)
{
    // Wait one PIO transfer cycle.
    inb(iobase2 + ATA_CB_ASTAT);

    प्रत्यागम await_not_bsy(iobase1);
}

// Wait for ide state - pause for 400ns first.
स्थिर अन्तःस्थ पूर्णाङ्क
ndelay_await_not_bsy(u16 iobase1)
{
    ndelay(400);
    प्रत्यागम await_not_bsy(iobase1);
}

// Reset a drive
स्थिर शून्य
ata_reset(संरचना atadrive_s *adrive_gf)
{
    संरचना ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
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
    पूर्णाङ्क status = await_not_bsy(iobase1);
    यदि (status < 0)
        गच्छ done;
    यदि (slave) {
        // Change device.
        u32 end = timer_calc(IDE_TIMEOUT);
        प्रति (;;) {
            outb(ATA_CB_DH_DEV1, iobase1 + ATA_CB_DH);
            status = ndelay_await_not_bsy(iobase1);
            यदि (status < 0)
                गच्छ done;
            यदि (inb(iobase1 + ATA_CB_DH) == ATA_CB_DH_DEV1)
                भङ्ग;
            // Change drive request failed to take effect - retry.
            यदि (timer_check(end)) {
                warn_timeout();
                गच्छ done;
            }
        }
    } अन्यथा {
        // QEMU doesn't reset dh on reset, so set it explicitly.
        outb(ATA_CB_DH_DEV0, iobase1 + ATA_CB_DH);
    }

    // On a user-reset request, wait for RDY if it is an ATA device.
    u8 type=GET_GLOBALFLAT(adrive_gf->drive.type);
    यदि (type == DTYPE_ATA)
        status = await_rdy(iobase1);

done:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    dprintf(6, "ata_reset exit status=%x\n", status);
}

// Check for drive RDY for 16bit interface command.
स्थिर पूर्णाङ्क
isready(संरचना atadrive_s *adrive_gf)
{
    // Read the status from controller
    संरचना ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u8 status = inb(iobase1 + ATA_CB_STAT);
    यदि ((status & (ATA_CB_STAT_BSY|ATA_CB_STAT_RDY)) == ATA_CB_STAT_RDY)
        प्रत्यागम DISK_RET_SUCCESS;
    प्रत्यागम DISK_RET_ENOTREADY;
}


/****************************************************************
 * ATA send command
 ****************************************************************/

संरचना ata_pio_command {
    u8 feature;
    u8 sector_गिनती;
    u8 lba_निचला;
    u8 lba_mid;
    u8 lba_ऊपरी;
    u8 device;
    u8 command;

    u8 feature2;
    u8 sector_count2;
    u8 lba_low2;
    u8 lba_mid2;
    u8 lba_high2;
};

// Send an ata command to the drive.
स्थिर पूर्णाङ्क
send_cmd(संरचना atadrive_s *adrive_gf, संरचना ata_pio_command *cmd)
{
    संरचना ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);

    // Select device
    पूर्णाङ्क status = await_not_bsy(iobase1);
    यदि (status < 0)
        प्रत्यागम status;
    u8 newdh = ((cmd->device & ~ATA_CB_DH_DEV1)
                | (slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0));
    u8 olddh = inb(iobase1 + ATA_CB_DH);
    outb(newdh, iobase1 + ATA_CB_DH);
    यदि ((olddh ^ newdh) & (1<<4)) {
        // Was a device change - wait for device to become not busy.
        status = ndelay_await_not_bsy(iobase1);
        यदि (status < 0)
            प्रत्यागम status;
    }

    // Check for ATA_CMD_(READ|WRITE)_(SECTORS|DMA)_EXT commands.
    यदि ((cmd->command & ~0x11) == ATA_CMD_READ_SECTORS_EXT) {
        outb(cmd->feature2, iobase1 + ATA_CB_FR);
        outb(cmd->sector_count2, iobase1 + ATA_CB_SC);
        outb(cmd->lba_low2, iobase1 + ATA_CB_SN);
        outb(cmd->lba_mid2, iobase1 + ATA_CB_CL);
        outb(cmd->lba_high2, iobase1 + ATA_CB_CH);
    }
    outb(cmd->feature, iobase1 + ATA_CB_FR);
    outb(cmd->sector_गिनती, iobase1 + ATA_CB_SC);
    outb(cmd->lba_निचला, iobase1 + ATA_CB_SN);
    outb(cmd->lba_mid, iobase1 + ATA_CB_CL);
    outb(cmd->lba_ऊपरी, iobase1 + ATA_CB_CH);
    outb(cmd->command, iobase1 + ATA_CB_CMD);

    प्रत्यागम 0;
}

// Wait for data after calling 'send_cmd'.
स्थिर पूर्णाङ्क
ata_wait_data(u16 iobase1)
{
    पूर्णाङ्क status = ndelay_await_not_bsy(iobase1);
    यदि (status < 0)
        प्रत्यागम status;

    यदि (status & ATA_CB_STAT_ERR) {
        dprintf(6, "send_cmd : read error (status=%02x err=%02x)\n"
                , status, inb(iobase1 + ATA_CB_ERR));
        प्रत्यागम -4;
    }
    यदि (!(status & ATA_CB_STAT_DRQ)) {
        dprintf(6, "send_cmd : DRQ not set (status %02x)\n", status);
        प्रत्यागम -5;
    }

    प्रत्यागम 0;
}

// Send an ata command that does not transfer any further data.
पूर्णाङ्क
ata_cmd_nondata(संरचना atadrive_s *adrive_gf, संरचना ata_pio_command *cmd)
{
    संरचना ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    पूर्णाङ्क ret = send_cmd(adrive_gf, cmd);
    यदि (ret)
        गच्छ fail;
    ret = ndelay_await_not_bsy(iobase1);
    यदि (ret < 0)
        गच्छ fail;

    यदि (ret & ATA_CB_STAT_ERR) {
        dprintf(6, "nondata cmd : read error (status=%02x err=%02x)\n"
                , ret, inb(iobase1 + ATA_CB_ERR));
        ret = -4;
        गच्छ fail;
    }
    यदि (ret & ATA_CB_STAT_DRQ) {
        dprintf(6, "nondata cmd : DRQ set (status %02x)\n", ret);
        ret = -5;
        गच्छ fail;
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    प्रत्यागम ret;
}


/****************************************************************
 * ATA PIO transfers
 ****************************************************************/

// Transfer 'op->count' blocks (of 'blocksize' bytes) to/from drive
// 'op->drive_fl'.
स्थिर पूर्णाङ्क
ata_pio_transfer(संरचना disk_op_s *op, पूर्णाङ्क iswrite, पूर्णाङ्क blocksize)
{
    dprintf(16, "ata_pio_transfer id=%p write=%d count=%d bs=%d buf=%p\n"
            , op->drive_fl, iswrite, op->गिनती, blocksize, op->buf_fl);

    संरचना atadrive_s *adrive_gf = container_of(
        op->drive_fl, संरचना atadrive_s, drive);
    संरचना ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    पूर्णाङ्क गिनती = op->गिनती;
    शून्य *buf_fl = op->buf_fl;
    पूर्णाङ्क status;
    प्रति (;;) {
        यदि (iswrite) {
            // Write data to controller
            dprintf(16, "Write sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            यदि (CONFIG_ATA_PIO32)
                outsl_fl(iobase1, buf_fl, blocksize / 4);
            अन्यथा
                outsw_fl(iobase1, buf_fl, blocksize / 2);
        } अन्यथा {
            // Read data from controller
            dprintf(16, "Read sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            यदि (CONFIG_ATA_PIO32)
                insl_fl(iobase1, buf_fl, blocksize / 4);
            अन्यथा
                insw_fl(iobase1, buf_fl, blocksize / 2);
        }
        buf_fl += blocksize;

        status = pause_await_not_bsy(iobase1, iobase2);
        यदि (status < 0) {
            // Error
            op->गिनती -= गिनती;
            प्रत्यागम status;
        }

        गिनती--;
        यदि (!गिनती)
            भङ्ग;
        status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ | ATA_CB_STAT_ERR);
        यदि (status != ATA_CB_STAT_DRQ) {
            dprintf(6, "ata_pio_transfer : more sectors left (status %02x)\n"
                    , status);
            op->गिनती -= गिनती;
            प्रत्यागम -6;
        }
    }

    status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
               | ATA_CB_STAT_ERR);
    यदि (!iswrite)
        status &= ~ATA_CB_STAT_DF;
    यदि (status != 0) {
        dprintf(6, "ata_pio_transfer : no sectors left (status %02x)\n", status);
        प्रत्यागम -7;
    }

    प्रत्यागम 0;
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

संरचना sff_dma_prd {
    u32 buf_fl;
    u32 गिनती;
};

// Check if DMA available and setup transfer if so.
स्थिर पूर्णाङ्क
ata_try_dma(संरचना disk_op_s *op, पूर्णाङ्क iswrite, पूर्णाङ्क blocksize)
{
    ASSERT16();
    यदि (! CONFIG_ATA_DMA)
        प्रत्यागम -1;
    u32 dest = (u32)op->buf_fl;
    यदि (dest & 1)
        // Need minimum alignment of 1.
        प्रत्यागम -1;
    संरचना atadrive_s *adrive_gf = container_of(
        op->drive_fl, संरचना atadrive_s, drive);
    संरचना ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);
    यदि (! iomaster)
        प्रत्यागम -1;
    u32 bytes = op->गिनती * blocksize;
    यदि (! bytes)
        प्रत्यागम -1;

    // Build PRD dma structure.
    संरचना sff_dma_prd *dma = MAKE_FLATPTR(SEG_निचला, Extra_ढेर);
    संरचना sff_dma_prd *origdma = dma;
    यावत् (bytes) {
        यदि (dma >= &origdma[16])
            // Too many descriptors..
            प्रत्यागम -1;
        u32 गिनती = bytes;
        u32 max = 0x10000 - (dest & 0xffff);
        यदि (गिनती > max)
            गिनती = max;

        SET_LOWFLAT(dma->buf_fl, dest);
        bytes -= गिनती;
        यदि (!bytes)
            // Last descriptor.
            गिनती |= 1<<31;
        dprintf(16, "dma@%p: %08x %08x\n", dma, dest, गिनती);
        dest += गिनती;
        SET_LOWFLAT(dma->गिनती, गिनती);
        dma++;
    }

    // Program bus-master controller.
    outl((u32)origdma, iomaster + BM_TABLE);
    u8 oldcmd = inb(iomaster + BM_CMD) & ~(BM_CMD_MEMWRITE|BM_CMD_START);
    outb(oldcmd | (iswrite ? 0x00 : BM_CMD_MEMWRITE), iomaster + BM_CMD);
    outb(BM_STATUS_ERROR|BM_STATUS_IRQ, iomaster + BM_STATUS);

    प्रत्यागम 0;
}

// Transfer data using DMA.
स्थिर पूर्णाङ्क
ata_dma_transfer(संरचना disk_op_s *op)
{
    यदि (! CONFIG_ATA_DMA)
        प्रत्यागम -1;
    dprintf(16, "ata_dma_transfer id=%p buf=%p\n", op->drive_fl, op->buf_fl);

    संरचना atadrive_s *adrive_gf = container_of(
        op->drive_fl, संरचना atadrive_s, drive);
    संरचना ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);

    // Start bus-master controller.
    u8 oldcmd = inb(iomaster + BM_CMD);
    outb(oldcmd | BM_CMD_START, iomaster + BM_CMD);

    u32 end = timer_calc(IDE_TIMEOUT);
    u8 status;
    प्रति (;;) {
        status = inb(iomaster + BM_STATUS);
        यदि (status & BM_STATUS_IRQ)
            भङ्ग;
        // Transfer in progress
        यदि (timer_check(end)) {
            // Timeout.
            warn_timeout();
            भङ्ग;
        }
        yield();
    }
    outb(oldcmd & ~BM_CMD_START, iomaster + BM_CMD);

    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    पूर्णाङ्क idestatus = pause_await_not_bsy(iobase1, iobase2);

    यदि ((status & (BM_STATUS_IRQ|BM_STATUS_ACTIVE)) == BM_STATUS_IRQ
        && idestatus >= 0x00
        && (idestatus & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
                         | ATA_CB_STAT_ERR)) == 0x00)
        // Success.
        प्रत्यागम 0;

    dprintf(6, "IDE DMA error (dma=%x ide=%x/%x/%x)\n", status, idestatus
            , inb(iobase2 + ATA_CB_ASTAT), inb(iobase1 + ATA_CB_ERR));
    प्रत्यागम -1;
}


/****************************************************************
 * ATA hard drive functions
 ****************************************************************/

// Transfer data to harddrive using PIO protocol.
स्थिर पूर्णाङ्क
ata_pio_cmd_data(संरचना disk_op_s *op, पूर्णाङ्क iswrite, संरचना ata_pio_command *cmd)
{
    संरचना atadrive_s *adrive_gf = container_of(
        op->drive_fl, संरचना atadrive_s, drive);
    संरचना ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    पूर्णाङ्क ret = send_cmd(adrive_gf, cmd);
    यदि (ret)
        गच्छ fail;
    ret = ata_wait_data(iobase1);
    यदि (ret)
        गच्छ fail;
    ret = ata_pio_transfer(op, iswrite, DISK_SECTOR_SIZE);

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    प्रत्यागम ret;
}

// Transfer data to harddrive using DMA protocol.
स्थिर पूर्णाङ्क
ata_dma_cmd_data(संरचना disk_op_s *op, संरचना ata_pio_command *cmd)
{
    यदि (! CONFIG_ATA_DMA)
        प्रत्यागम -1;
    संरचना atadrive_s *adrive_gf = container_of(
        op->drive_fl, संरचना atadrive_s, drive);
    पूर्णाङ्क ret = send_cmd(adrive_gf, cmd);
    यदि (ret)
        प्रत्यागम ret;
    प्रत्यागम ata_dma_transfer(op);
}

// Read/write count blocks from a harddrive.
स्थिर पूर्णाङ्क
ata_readwrite(संरचना disk_op_s *op, पूर्णाङ्क iswrite)
{
    u64 lba = op->lba;

    पूर्णाङ्क usepio = ata_try_dma(op, iswrite, DISK_SECTOR_SIZE);

    संरचना ata_pio_command cmd;
    memset(&cmd, 0, परिमाण(cmd));

    यदि (op->गिनती >= (1<<8) || lba + op->गिनती >= (1<<28)) {
        cmd.sector_count2 = op->गिनती >> 8;
        cmd.lba_low2 = lba >> 24;
        cmd.lba_mid2 = lba >> 32;
        cmd.lba_high2 = lba >> 40;
        lba &= 0xffffff;

        यदि (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS_EXT
                           : ATA_CMD_READ_SECTORS_EXT);
        अन्यथा
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                           : ATA_CMD_READ_DMA_EXT);
    } अन्यथा {
        यदि (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS
                           : ATA_CMD_READ_SECTORS);
        अन्यथा
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA
                           : ATA_CMD_READ_DMA);
    }

    cmd.sector_गिनती = op->गिनती;
    cmd.lba_निचला = lba;
    cmd.lba_mid = lba >> 8;
    cmd.lba_ऊपरी = lba >> 16;
    cmd.device = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;

    पूर्णाङ्क ret;
    यदि (usepio)
        ret = ata_pio_cmd_data(op, iswrite, &cmd);
    अन्यथा
        ret = ata_dma_cmd_data(op, &cmd);
    यदि (ret)
        प्रत्यागम DISK_RET_EBADTRACK;
    प्रत्यागम DISK_RET_SUCCESS;
}

// 16bit command demuxer for ATA harddrives.
पूर्णाङ्क
ata_process_op(संरचना disk_op_s *op)
{
    यदि (!CONFIG_ATA)
        प्रत्यागम 0;

    संरचना atadrive_s *adrive_gf = container_of(
        op->drive_fl, संरचना atadrive_s, drive);
    विकल्प (op->command) {
    पक्ष CMD_READ:
        प्रत्यागम ata_readwrite(op, 0);
    पक्ष CMD_WRITE:
        प्रत्यागम ata_readwrite(op, 1);
    पक्ष CMD_RESET:
        ata_reset(adrive_gf);
        प्रत्यागम DISK_RET_SUCCESS;
    पक्ष CMD_ISREADY:
        प्रत्यागम isready(adrive_gf);
    स्वभाव:
        प्रत्यागम default_process_op(op);
    }
}


/****************************************************************
 * ATAPI functions
 ****************************************************************/

#define CDROM_CDB_SIZE 12

// Low-level atapi command transmit function.
पूर्णाङ्क
ata_atapi_process_op(संरचना disk_op_s *op)
{
    यदि (! CONFIG_ATA)
        प्रत्यागम 0;

    यदि (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        प्रत्यागम DISK_RET_EWRITEPROTECT;
    u8 cdbcmd[CDROM_CDB_SIZE];
    पूर्णाङ्क blocksize = scsi_fill_cmd(op, cdbcmd, परिमाण(cdbcmd));
    यदि (blocksize < 0)
        प्रत्यागम default_process_op(op);

    संरचना atadrive_s *adrive_gf = container_of(
        op->drive_fl, संरचना atadrive_s, drive);
    संरचना ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    संरचना ata_pio_command cmd;
    memset(&cmd, 0, परिमाण(cmd));
    cmd.lba_mid = blocksize;
    cmd.lba_ऊपरी = blocksize >> 8;
    cmd.command = ATA_CMD_PACKET;

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    पूर्णाङ्क ret = send_cmd(adrive_gf, &cmd);
    यदि (ret)
        गच्छ fail;
    ret = ata_wait_data(iobase1);
    यदि (ret)
        गच्छ fail;

    // Send command to device
    outsw_fl(iobase1, MAKE_FLATPTR(GET_SEG(SS), cdbcmd), CDROM_CDB_SIZE / 2);

    पूर्णाङ्क status = pause_await_not_bsy(iobase1, iobase2);
    यदि (status < 0) {
        ret = status;
        गच्छ fail;
    }

    यदि (status & ATA_CB_STAT_ERR) {
        u8 err = inb(iobase1 + ATA_CB_ERR);
        // skip "Not Ready"
        यदि (err != 0x20)
            dprintf(6, "send_atapi_cmd : read error (status=%02x err=%02x)\n"
                    , status, err);
        ret = -2;
        गच्छ fail;
    }
    यदि (blocksize) {
        यदि (!(status & ATA_CB_STAT_DRQ)) {
            dprintf(6, "send_atapi_cmd : DRQ not set (status %02x)\n", status);
            ret = -3;
            गच्छ fail;
        }

        ret = ata_pio_transfer(op, 0, blocksize);
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    यदि (ret)
        प्रत्यागम DISK_RET_EBADTRACK;
    प्रत्यागम DISK_RET_SUCCESS;
}


/****************************************************************
 * ATA detect and init
 ****************************************************************/

// Send an identify device or identify device packet command.
स्थिर पूर्णाङ्क
send_ata_identity(संरचना atadrive_s *adrive, u16 *buffer, पूर्णाङ्क command)
{
    memset(buffer, 0, DISK_SECTOR_SIZE);

    संरचना disk_op_s dop;
    memset(&dop, 0, परिमाण(dop));
    dop.drive_fl = &adrive->drive;
    dop.गिनती = 1;
    dop.lba = 1;
    dop.buf_fl = MAKE_FLATPTR(GET_SEG(SS), buffer);

    संरचना ata_pio_command cmd;
    memset(&cmd, 0, परिमाण(cmd));
    cmd.command = command;

    प्रत्यागम ata_pio_cmd_data(&dop, 0, &cmd);
}

// Extract the ATA/ATAPI version info.
पूर्णाङ्क
ata_extract_version(u16 *buffer)
{
    // Extract ATA/ATAPI version.
    u16 ataversion = buffer[80];
    u8 version;
    प्रति (version=15; version>0; version--)
        यदि (ataversion & (1<<version))
            भङ्ग;
    प्रत्यागम version;
}

#define MAXMODEL 40

// Extract the ATA/ATAPI model info.
वर्ण *
ata_extract_model(वर्ण *model, u32 size, u16 *buffer)
{
    // Read model name
    पूर्णाङ्क i;
    प्रति (i=0; i<size/2; i++)
        *(u16*)&model[i*2] = be16_to_cpu(buffer[27+i]);
    model[size] = 0x00;
    nullTrailingSpace(model);
    प्रत्यागम model;
}

// Common init code between ata and atapi
स्थिर संरचना atadrive_s *
init_atadrive(संरचना atadrive_s *dummy, u16 *buffer)
{
    संरचना atadrive_s *adrive = malloc_fseg(परिमाण(*adrive));
    यदि (!adrive) {
        warn_noalloc();
        प्रत्यागम रिक्त;
    }
    memset(adrive, 0, परिमाण(*adrive));
    adrive->chan_gf = dummy->chan_gf;
    adrive->slave = dummy->slave;
    adrive->drive.cntl_id = adrive->chan_gf->ataid * 2 + dummy->slave;
    adrive->drive.removable = (buffer[0] & 0x80) ? 1 : 0;
    प्रत्यागम adrive;
}

// Detect if the given drive is an atapi - initialize it if so.
स्थिर संरचना atadrive_s *
init_drive_atapi(संरचना atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE_PACKET command to device
    पूर्णाङ्क ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    यदि (ret)
        प्रत्यागम रिक्त;

    // Success - setup as ATAPI.
    संरचना atadrive_s *adrive = init_atadrive(dummy, buffer);
    यदि (!adrive)
        प्रत्यागम रिक्त;
    adrive->drive.type = DTYPE_ATA_ATAPI;
    adrive->drive.blksize = CDROM_SECTOR_SIZE;
    adrive->drive.sectors = (u64)-1;
    u8 iscd = ((buffer[0] >> 8) & 0x1f) == 0x05;
    वर्ण model[MAXMODEL+1];
    वर्ण *desc = znprintf(MAXDESCSIZE
                          , "DVD/CD [ata%d-%d: %s ATAPI-%d %s]"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (iscd ? "DVD/CD" : "Device"));
    dprintf(1, "%s\n", desc);

    // fill cdidmap
    यदि (iscd) {
        पूर्णाङ्क prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                            adrive->chan_gf->chanid,
                                            adrive->slave);
        boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                                  adrive->chan_gf->chanid,
                                  adrive->slave,
                                  &(adrive->drive.lchs));
        boot_add_cd(&adrive->drive, desc, prio);
    }

    प्रत्यागम adrive;
}

// Detect if the given drive is a regular ata drive - initialize it if so.
स्थिर संरचना atadrive_s *
init_drive_ata(संरचना atadrive_s *dummy, u16 *buffer)
{
    // Send an IDENTIFY_DEVICE command to device
    पूर्णाङ्क ret = send_ata_identity(dummy, buffer, ATA_CMD_IDENTIFY_DEVICE);
    यदि (ret)
        प्रत्यागम रिक्त;

    // Success - setup as ATA.
    संरचना atadrive_s *adrive = init_atadrive(dummy, buffer);
    यदि (!adrive)
        प्रत्यागम रिक्त;
    adrive->drive.type = DTYPE_ATA;
    adrive->drive.blksize = DISK_SECTOR_SIZE;

    adrive->drive.pchs.cylinder = buffer[1];
    adrive->drive.pchs.head = buffer[3];
    adrive->drive.pchs.sector = buffer[6];

    u64 sectors;
    यदि (buffer[83] & (1 << 10)) // word 83 - lba48 support
        sectors = *(u64*)&buffer[100]; // word 100-103
    अन्यथा
        sectors = *(u32*)&buffer[60]; // word 60 and word 61
    adrive->drive.sectors = sectors;
    u64 adjsize = sectors >> 11;
    वर्ण adjprefix = 'M';
    यदि (adjsize >= (1 << 16)) {
        adjsize >>= 10;
        adjprefix = 'G';
    }
    वर्ण model[MAXMODEL+1];
    वर्ण *desc = znprintf(MAXDESCSIZE
                          , "ata%d-%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, buffer)
                          , ata_extract_version(buffer)
                          , (u32)adjsize, adjprefix);
    dprintf(1, "%s\n", desc);

    पूर्णाङ्क prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                        adrive->chan_gf->chanid,
                                        adrive->slave);
    boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                              adrive->chan_gf->chanid,
                              adrive->slave,
                              &(adrive->drive.lchs));
    // Register with bcv system.
    boot_add_hd(&adrive->drive, desc, prio);

    प्रत्यागम adrive;
}

स्थिर u32 SpinupEnd;

// Wait for non-busy status and check for "floating bus" condition.
स्थिर पूर्णाङ्क
powerup_await_non_bsy(u16 base)
{
    u8 orstatus = 0;
    u8 status;
    प्रति (;;) {
        status = inb(base+ATA_CB_STAT);
        यदि (!(status & ATA_CB_STAT_BSY))
            भङ्ग;
        orstatus |= status;
        यदि (orstatus == 0xff) {
            dprintf(4, "powerup IDE floating\n");
            प्रत्यागम orstatus;
        }
        यदि (timer_check(SpinupEnd)) {
            warn_timeout();
            प्रत्यागम -1;
        }
        yield();
    }
    dprintf(6, "powerup iobase=%x st=%x\n", base, status);
    प्रत्यागम status;
}

// Detect any drives attached to a given controller.
स्थिर शून्य
ata_detect(शून्य *data)
{
    संरचना ata_channel_s *chan_gf = data;
    संरचना atadrive_s dummy;
    memset(&dummy, 0, परिमाण(dummy));
    dummy.chan_gf = chan_gf;
    // Device detection
    पूर्णाङ्क didreset = 0;
    u8 slave;
    प्रति (slave=0; slave<=1; slave++) {
        // Wait for not-bsy.
        u16 iobase1 = chan_gf->iobase1;
        पूर्णाङ्क status = powerup_await_non_bsy(iobase1);
        यदि (status < 0)
            अनुवर्त;
        u8 newdh = slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0;
        outb(newdh, iobase1+ATA_CB_DH);
        ndelay(400);
        status = powerup_await_non_bsy(iobase1);
        यदि (status < 0)
            अनुवर्त;

        // Check if ioport registers look valid.
        outb(newdh, iobase1+ATA_CB_DH);
        u8 dh = inb(iobase1+ATA_CB_DH);
        outb(0x55, iobase1+ATA_CB_SC);
        outb(0xaa, iobase1+ATA_CB_SN);
        u8 sc = inb(iobase1+ATA_CB_SC);
        u8 sn = inb(iobase1+ATA_CB_SN);
        dprintf(6, "ata_detect ata%d-%d: sc=%x sn=%x dh=%x\n"
                , chan_gf->ataid, slave, sc, sn, dh);
        यदि (sc != 0x55 || sn != 0xaa || dh != newdh)
            अनुवर्त;

        // Prepare new drive.
        dummy.slave = slave;

        // reset the channel
        यदि (!didreset) {
            ata_reset(&dummy);
            didreset = 1;
        }

        // check for ATAPI
        u16 buffer[256];
        संरचना atadrive_s *adrive = init_drive_atapi(&dummy, buffer);
        यदि (!adrive) {
            // Didn't find an ATAPI drive - look for ATA drive.
            u8 st = inb(iobase1+ATA_CB_STAT);
            यदि (!st)
                // Status not set - can't be a valid drive.
                अनुवर्त;

            // Wait for RDY.
            पूर्णाङ्क ret = await_rdy(iobase1);
            यदि (ret < 0)
                अनुवर्त;

            // check for ATA.
            adrive = init_drive_ata(&dummy, buffer);
            यदि (!adrive)
                // No ATA drive found
                अनुवर्त;
        }

        u16 resetresult = buffer[93];
        dprintf(6, "ata_detect resetresult=%04x\n", resetresult);
        यदि (!slave && (resetresult & 0xdf61) == 0x4041)
            // resetresult looks valid and device 0 is responding to
            // device 1 requests - device 1 must not be present - skip
            // detection.
            भङ्ग;
    }
}

// Initialize an ata controller and detect its drives.
स्थिर शून्य
init_controller(संरचना pci_device *pci, पूर्णाङ्क chanid, पूर्णाङ्क irq
                , u32 port1, u32 port2, u32 master)
{
    स्थिर पूर्णाङ्क ataid = 0;
    संरचना ata_channel_s *chan_gf = malloc_fseg(परिमाण(*chan_gf));
    यदि (!chan_gf) {
        warn_noalloc();
        प्रत्यागम;
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
स्थिर शून्य
init_pciata(संरचना pci_device *pci, u8 prog_if)
{
    u8 pciirq = pci_config_readb(pci->bdf, PCI_INTERRUPT_पंक्ति);
    पूर्णाङ्क master = 0;
    यदि (CONFIG_ATA_DMA && prog_if & 0x80) {
        // Check for bus-mastering.
        u32 bar = pci_config_readl(pci->bdf, PCI_BASE_ADDRESS_4);
        यदि (bar & PCI_BASE_ADDRESS_SPACE_IO) {
            master = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
            pci_enable_busmaster(pci);
        }
    }

    u32 port1, port2, irq;
    यदि (prog_if & 1) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_1);
        यदि (!port1 || !port2)
            प्रत्यागम;
        irq = pciirq;
    } अन्यथा {
        port1 = PORT_ATA1_CMD_BASE;
        port2 = PORT_ATA1_CTRL_BASE;
        irq = IRQ_ATA1;
    }
    init_controller(pci, 0, irq, port1, port2, master);

    यदि (prog_if & 4) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_2);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_3);
        यदि (!port1 || !port2)
            प्रत्यागम;
        irq = pciirq;
    } अन्यथा {
        port1 = PORT_ATA2_CMD_BASE;
        port2 = PORT_ATA2_CTRL_BASE;
        irq = IRQ_ATA2;
    }
    init_controller(pci, 1, irq, port1, port2, master ? master + 8 : 0);
}

स्थिर शून्य
found_genericata(संरचना pci_device *pci, शून्य *arg)
{
    init_pciata(pci, pci->prog_if);
}

स्थिर शून्य
found_compatibleahci(संरचना pci_device *pci, शून्य *arg)
{
    यदि (CONFIG_AHCI)
        // Already handled directly via native ahci interface.
        प्रत्यागम;
    init_pciata(pci, 0x8f);
}

स्थिर ध्रुव संरचना pci_device_id pci_ata_tbl[] = {
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE
                     , found_genericata),
    PCI_DEVICE(PCI_VENDOR_ID_ATI, 0x4391, found_compatibleahci),
    PCI_DEVICE_END,
};

// Locate and init ata controllers.
स्थिर शून्य
ata_scan(शून्य)
{
    यदि (CONFIG_QEMU && hlist_empty(&PCIDevices)) {
        // No PCI devices found - probably a QEMU "-M isapc" machine.
        // Try using ISA ports for ATA controllers.
        init_controller(रिक्त, 0, IRQ_ATA1
                        , PORT_ATA1_CMD_BASE, PORT_ATA1_CTRL_BASE, 0);
        init_controller(रिक्त, 1, IRQ_ATA2
                        , PORT_ATA2_CMD_BASE, PORT_ATA2_CTRL_BASE, 0);
        प्रत्यागम;
    }

    // Scan PCI bus for ATA adapters
    संरचना pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_ata_tbl, pci, रिक्त);
    }
}

शून्य
ata_setup(शून्य)
{
    ASSERT32FLAT();
    यदि (!CONFIG_ATA)
        प्रत्यागम;

    dprintf(3, "init hard drives\n");

    SpinupEnd = timer_calc(IDE_TIMEOUT);
    ata_scan();

    SET_BDA(disk_control_byte, 0xc0);

    enable_hwirq(14, FUNC16(entry_76));
}

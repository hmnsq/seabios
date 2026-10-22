// Low level ATA disk access
//
// Copyright (C) 2008,2009  Kevin O'Connor <kevin@koconnor.net>
// Copyright (C) 2002  MandrakeSoft S.A.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "ata.h" // ATA_CB_STAT
#समावेश "biosvar.h" // GET_GLOBALFLAT
#समावेश "block.h" // struct drive_s
#समावेश "blockcmd.h" // CDB_CMD_READ_10
#समावेश "byteorder.h" // be16_to_cpu
#समावेश "malloc.h" // malloc_fseg
#समावेश "output.h" // dprintf
#समावेश "pci.h" // pci_config_readb
#समावेश "pcidevice.h" // foreachpci
#समावेश "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#समावेश "pci_regs.h" // PCI_INTERRUPT_LINE
#समावेश "pic.h" // enable_hwirq
#समावेश "stacks.h" // yield
#समावेश "std/disk.h" // DISK_RET_SUCCESS
#समावेश "string.h" // memset
#समावेश "util.h" // timer_calc
#समावेश "x86.h" // inb

#घोषणा IDE_TIMEOUT 32000 //32 seconds max for IDE ops


/****************************************************************
 * Helper functions
 ****************************************************************/

// Wait for the specified ide state
अटल अंतरभूत पूर्णांक
await_ide(u8 mask, u8 flags, u16 base, u16 timeout)
{
    u32 end = timer_calc(timeout);
    क्रम (;;) {
        u8 status = inb(base+ATA_CB_STAT);
        अगर ((status & mask) == flags)
            वापस status;
        अगर (timer_check(end)) {
            warn_timeout();
            वापस -1;
        }
        yield();
    }
}

// Wait for the device to be not-busy.
अटल पूर्णांक
await_not_bsy(u16 base)
{
    वापस await_ide(ATA_CB_STAT_BSY, 0, base, IDE_TIMEOUT);
}

// Wait for the device to be ready.
अटल पूर्णांक
await_rdy(u16 base)
{
    वापस await_ide(ATA_CB_STAT_RDY, ATA_CB_STAT_RDY, base, IDE_TIMEOUT);
}

// Wait for ide state - pauses for one ata cycle first.
अटल अंतरभूत पूर्णांक
pause_await_not_bsy(u16 iobase1, u16 iobase2)
{
    // Wait one PIO transfer cycle.
    inb(iobase2 + ATA_CB_ASTAT);

    वापस await_not_bsy(iobase1);
}

// Wait for ide state - pause for 400ns first.
अटल अंतरभूत पूर्णांक
ndelay_await_not_bsy(u16 iobase1)
{
    ndelay(400);
    वापस await_not_bsy(iobase1);
}

// Reset a drive
अटल व्योम
ata_reset(काष्ठा atadrive_s *adrive_gf)
{
    काष्ठा ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
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
    पूर्णांक status = await_not_bsy(iobase1);
    अगर (status < 0)
        जाओ done;
    अगर (slave) {
        // Change device.
        u32 end = timer_calc(IDE_TIMEOUT);
        क्रम (;;) {
            outb(ATA_CB_DH_DEV1, iobase1 + ATA_CB_DH);
            status = ndelay_await_not_bsy(iobase1);
            अगर (status < 0)
                जाओ done;
            अगर (inb(iobase1 + ATA_CB_DH) == ATA_CB_DH_DEV1)
                अवरोध;
            // Change drive request failed to take effect - retry.
            अगर (timer_check(end)) {
                warn_timeout();
                जाओ done;
            }
        }
    } अन्यथा {
        // QEMU doesn't reset dh on reset, so set it explicitly.
        outb(ATA_CB_DH_DEV0, iobase1 + ATA_CB_DH);
    }

    // On a user-reset request, wait for RDY if it is an ATA device.
    u8 type=GET_GLOBALFLAT(adrive_gf->drive.type);
    अगर (type == DTYPE_ATA)
        status = await_rdy(iobase1);

done:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    dprintf(6, "ata_reset exit status=%x\n", status);
}

// Check for drive RDY for 16bit interface command.
अटल पूर्णांक
isready(काष्ठा atadrive_s *adrive_gf)
{
    // Read the status from controller
    काष्ठा ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u8 status = inb(iobase1 + ATA_CB_STAT);
    अगर ((status & (ATA_CB_STAT_BSY|ATA_CB_STAT_RDY)) == ATA_CB_STAT_RDY)
        वापस DISK_RET_SUCCESS;
    वापस DISK_RET_ENOTREADY;
}


/****************************************************************
 * ATA send command
 ****************************************************************/

काष्ठा ata_pio_command {
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
अटल पूर्णांक
send_cmd(काष्ठा atadrive_s *adrive_gf, काष्ठा ata_pio_command *cmd)
{
    काष्ठा ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u8 slave = GET_GLOBALFLAT(adrive_gf->slave);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);

    // Select device
    पूर्णांक status = await_not_bsy(iobase1);
    अगर (status < 0)
        वापस status;
    u8 newdh = ((cmd->device & ~ATA_CB_DH_DEV1)
                | (slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0));
    u8 olddh = inb(iobase1 + ATA_CB_DH);
    outb(newdh, iobase1 + ATA_CB_DH);
    अगर ((olddh ^ newdh) & (1<<4)) {
        // Was a device change - wait for device to become not busy.
        status = ndelay_await_not_bsy(iobase1);
        अगर (status < 0)
            वापस status;
    }

    // Check for ATA_CMD_(READ|WRITE)_(SECTORS|DMA)_EXT commands.
    अगर ((cmd->command & ~0x11) == ATA_CMD_READ_SECTORS_EXT) {
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

    वापस 0;
}

// Wait for data after calling 'send_cmd'.
अटल पूर्णांक
ata_wait_तथ्य(u16 iobase1)
{
    पूर्णांक status = ndelay_await_not_bsy(iobase1);
    अगर (status < 0)
        वापस status;

    अगर (status & ATA_CB_STAT_ERR) {
        dprintf(6, "send_cmd : read error (status=%02x err=%02x)\n"
                , status, inb(iobase1 + ATA_CB_ERR));
        वापस -4;
    }
    अगर (!(status & ATA_CB_STAT_DRQ)) {
        dprintf(6, "send_cmd : DRQ not set (status %02x)\n", status);
        वापस -5;
    }

    वापस 0;
}

// Send an ata command that does not transfer any further data.
पूर्णांक
ata_cmd_nondata(काष्ठा atadrive_s *adrive_gf, काष्ठा ata_pio_command *cmd)
{
    काष्ठा ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    पूर्णांक ret = send_cmd(adrive_gf, cmd);
    अगर (ret)
        जाओ fail;
    ret = ndelay_await_not_bsy(iobase1);
    अगर (ret < 0)
        जाओ fail;

    अगर (ret & ATA_CB_STAT_ERR) {
        dprintf(6, "nondata cmd : read error (status=%02x err=%02x)\n"
                , ret, inb(iobase1 + ATA_CB_ERR));
        ret = -4;
        जाओ fail;
    }
    अगर (ret & ATA_CB_STAT_DRQ) {
        dprintf(6, "nondata cmd : DRQ set (status %02x)\n", ret);
        ret = -5;
        जाओ fail;
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);

    वापस ret;
}


/****************************************************************
 * ATA PIO transfers
 ****************************************************************/

// Transfer 'op->count' blocks (of 'blocksize' bytes) to/from drive
// 'op->drive_fl'.
अटल पूर्णांक
ata_pio_transfer(काष्ठा disk_op_s *op, पूर्णांक iswrite, पूर्णांक blocksize)
{
    dprintf(16, "ata_pio_transfer id=%p write=%d count=%d bs=%d buf=%p\n"
            , op->drive_fl, iswrite, op->गिनती, blocksize, op->buf_fl);

    काष्ठा atadrive_s *adrive_gf = container_of(
        op->drive_fl, काष्ठा atadrive_s, drive);
    काष्ठा ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    पूर्णांक गिनती = op->गिनती;
    व्योम *buf_fl = op->buf_fl;
    पूर्णांक status;
    क्रम (;;) {
        अगर (iswrite) {
            // Write data to controller
            dprintf(16, "Write sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            अगर (CONFIG_ATA_PIO32)
                outsl_fl(iobase1, buf_fl, blocksize / 4);
            अन्यथा
                outsw_fl(iobase1, buf_fl, blocksize / 2);
        } अन्यथा {
            // Read data from controller
            dprintf(16, "Read sector id=%p dest=%p\n", op->drive_fl, buf_fl);
            अगर (CONFIG_ATA_PIO32)
                insl_fl(iobase1, buf_fl, blocksize / 4);
            अन्यथा
                insw_fl(iobase1, buf_fl, blocksize / 2);
        }
        buf_fl += blocksize;

        status = pause_await_not_bsy(iobase1, iobase2);
        अगर (status < 0) {
            // Error
            op->गिनती -= गिनती;
            वापस status;
        }

        गिनती--;
        अगर (!गिनती)
            अवरोध;
        status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ | ATA_CB_STAT_ERR);
        अगर (status != ATA_CB_STAT_DRQ) {
            dprintf(6, "ata_pio_transfer : more sectors left (status %02x)\n"
                    , status);
            op->गिनती -= गिनती;
            वापस -6;
        }
    }

    status &= (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
               | ATA_CB_STAT_ERR);
    अगर (!iswrite)
        status &= ~ATA_CB_STAT_DF;
    अगर (status != 0) {
        dprintf(6, "ata_pio_transfer : no sectors left (status %02x)\n", status);
        वापस -7;
    }

    वापस 0;
}


/****************************************************************
 * ATA DMA transfers
 ****************************************************************/

#घोषणा BM_CMD    0
#घोषणा  BM_CMD_MEMWRITE  0x08
#घोषणा  BM_CMD_START     0x01
#घोषणा BM_STATUS 2
#घोषणा  BM_STATUS_IRQ    0x04
#घोषणा  BM_STATUS_ERROR  0x02
#घोषणा  BM_STATUS_ACTIVE 0x01
#घोषणा BM_TABLE  4

काष्ठा sff_dma_prd {
    u32 buf_fl;
    u32 गिनती;
};

// Check if DMA available and setup transfer if so.
अटल पूर्णांक
ata_try_dma(काष्ठा disk_op_s *op, पूर्णांक iswrite, पूर्णांक blocksize)
{
    ASSERT16();
    अगर (! CONFIG_ATA_DMA)
        वापस -1;
    u32 dest = (u32)op->buf_fl;
    अगर (dest & 1)
        // Need minimum alignment of 1.
        वापस -1;
    काष्ठा atadrive_s *adrive_gf = container_of(
        op->drive_fl, काष्ठा atadrive_s, drive);
    काष्ठा ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);
    अगर (! iomaster)
        वापस -1;
    u32 bytes = op->गिनती * blocksize;
    अगर (! bytes)
        वापस -1;

    // Build PRD dma structure.
    काष्ठा sff_dma_prd *dma = MAKE_FLATPTR(SEG_निचला, Extra_ढेर);
    काष्ठा sff_dma_prd *origdma = dma;
    जबतक (bytes) {
        अगर (dma >= &origdma[16])
            // Too many descriptors..
            वापस -1;
        u32 गिनती = bytes;
        u32 max = 0x10000 - (dest & 0xffff);
        अगर (गिनती > max)
            गिनती = max;

        SET_LOWFLAT(dma->buf_fl, dest);
        bytes -= गिनती;
        अगर (!bytes)
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

    वापस 0;
}

// Transfer data using DMA.
अटल पूर्णांक
ata_dma_transfer(काष्ठा disk_op_s *op)
{
    अगर (! CONFIG_ATA_DMA)
        वापस -1;
    dprintf(16, "ata_dma_transfer id=%p buf=%p\n", op->drive_fl, op->buf_fl);

    काष्ठा atadrive_s *adrive_gf = container_of(
        op->drive_fl, काष्ठा atadrive_s, drive);
    काष्ठा ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iomaster = GET_GLOBALFLAT(chan_gf->iomaster);

    // Start bus-master controller.
    u8 oldcmd = inb(iomaster + BM_CMD);
    outb(oldcmd | BM_CMD_START, iomaster + BM_CMD);

    u32 end = timer_calc(IDE_TIMEOUT);
    u8 status;
    क्रम (;;) {
        status = inb(iomaster + BM_STATUS);
        अगर (status & BM_STATUS_IRQ)
            अवरोध;
        // Transfer in progress
        अगर (timer_check(end)) {
            // Timeout.
            warn_timeout();
            अवरोध;
        }
        yield();
    }
    outb(oldcmd & ~BM_CMD_START, iomaster + BM_CMD);

    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);
    पूर्णांक idestatus = pause_await_not_bsy(iobase1, iobase2);

    अगर ((status & (BM_STATUS_IRQ|BM_STATUS_ACTIVE)) == BM_STATUS_IRQ
        && idestatus >= 0x00
        && (idestatus & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF | ATA_CB_STAT_DRQ
                         | ATA_CB_STAT_ERR)) == 0x00)
        // Success.
        वापस 0;

    dprintf(6, "IDE DMA error (dma=%x ide=%x/%x/%x)\n", status, idestatus
            , inb(iobase2 + ATA_CB_ASTAT), inb(iobase1 + ATA_CB_ERR));
    वापस -1;
}


/****************************************************************
 * ATA hard drive functions
 ****************************************************************/

// Transfer data to harddrive using PIO protocol.
अटल पूर्णांक
ata_pio_cmd_तथ्य(काष्ठा disk_op_s *op, पूर्णांक iswrite, काष्ठा ata_pio_command *cmd)
{
    काष्ठा atadrive_s *adrive_gf = container_of(
        op->drive_fl, काष्ठा atadrive_s, drive);
    काष्ठा ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    पूर्णांक ret = send_cmd(adrive_gf, cmd);
    अगर (ret)
        जाओ fail;
    ret = ata_wait_तथ्य(iobase1);
    अगर (ret)
        जाओ fail;
    ret = ata_pio_transfer(op, iswrite, DISK_SECTOR_आकार);

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    वापस ret;
}

// Transfer data to harddrive using DMA protocol.
अटल पूर्णांक
ata_dma_cmd_तथ्य(काष्ठा disk_op_s *op, काष्ठा ata_pio_command *cmd)
{
    अगर (! CONFIG_ATA_DMA)
        वापस -1;
    काष्ठा atadrive_s *adrive_gf = container_of(
        op->drive_fl, काष्ठा atadrive_s, drive);
    पूर्णांक ret = send_cmd(adrive_gf, cmd);
    अगर (ret)
        वापस ret;
    वापस ata_dma_transfer(op);
}

// Read/write count blocks from a harddrive.
अटल पूर्णांक
ata_readwrite(काष्ठा disk_op_s *op, पूर्णांक iswrite)
{
    u64 lba = op->lba;

    पूर्णांक usepio = ata_try_dma(op, iswrite, DISK_SECTOR_आकार);

    काष्ठा ata_pio_command cmd;
    स_रखो(&cmd, 0, माप(cmd));

    अगर (op->गिनती >= (1<<8) || lba + op->गिनती >= (1<<28)) {
        cmd.sector_count2 = op->गिनती >> 8;
        cmd.lba_low2 = lba >> 24;
        cmd.lba_mid2 = lba >> 32;
        cmd.lba_high2 = lba >> 40;
        lba &= 0xffffff;

        अगर (usepio)
            cmd.command = (iswrite ? ATA_CMD_WRITE_SECTORS_EXT
                           : ATA_CMD_READ_SECTORS_EXT);
        अन्यथा
            cmd.command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                           : ATA_CMD_READ_DMA_EXT);
    } अन्यथा {
        अगर (usepio)
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

    पूर्णांक ret;
    अगर (usepio)
        ret = ata_pio_cmd_तथ्य(op, iswrite, &cmd);
    अन्यथा
        ret = ata_dma_cmd_तथ्य(op, &cmd);
    अगर (ret)
        वापस DISK_RET_EBADTRACK;
    वापस DISK_RET_SUCCESS;
}

// 16bit command demuxer for ATA harddrives.
पूर्णांक
ata_process_op(काष्ठा disk_op_s *op)
{
    अगर (!CONFIG_ATA)
        वापस 0;

    काष्ठा atadrive_s *adrive_gf = container_of(
        op->drive_fl, काष्ठा atadrive_s, drive);
    चयन (op->command) {
    हाल CMD_READ:
        वापस ata_readwrite(op, 0);
    हाल CMD_WRITE:
        वापस ata_readwrite(op, 1);
    हाल CMD_RESET:
        ata_reset(adrive_gf);
        वापस DISK_RET_SUCCESS;
    हाल CMD_ISREADY:
        वापस isready(adrive_gf);
    शेष:
        वापस default_process_op(op);
    }
}


/****************************************************************
 * ATAPI functions
 ****************************************************************/

#घोषणा CDROM_CDB_आकार 12

// Low-level atapi command transmit function.
पूर्णांक
ata_atapi_process_op(काष्ठा disk_op_s *op)
{
    अगर (! CONFIG_ATA)
        वापस 0;

    अगर (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        वापस DISK_RET_EWRITEPROTECT;
    u8 cdbcmd[CDROM_CDB_आकार];
    पूर्णांक blocksize = scsi_fill_cmd(op, cdbcmd, माप(cdbcmd));
    अगर (blocksize < 0)
        वापस default_process_op(op);

    काष्ठा atadrive_s *adrive_gf = container_of(
        op->drive_fl, काष्ठा atadrive_s, drive);
    काष्ठा ata_channel_s *chan_gf = GET_GLOBALFLAT(adrive_gf->chan_gf);
    u16 iobase1 = GET_GLOBALFLAT(chan_gf->iobase1);
    u16 iobase2 = GET_GLOBALFLAT(chan_gf->iobase2);

    काष्ठा ata_pio_command cmd;
    स_रखो(&cmd, 0, माप(cmd));
    cmd.lba_mid = blocksize;
    cmd.lba_ऊपरी = blocksize >> 8;
    cmd.command = ATA_CMD_PACKET;

    // Disable interrupts
    outb(ATA_CB_DC_HD15 | ATA_CB_DC_NIEN, iobase2 + ATA_CB_DC);

    पूर्णांक ret = send_cmd(adrive_gf, &cmd);
    अगर (ret)
        जाओ fail;
    ret = ata_wait_तथ्य(iobase1);
    अगर (ret)
        जाओ fail;

    // Send command to device
    outsw_fl(iobase1, MAKE_FLATPTR(GET_SEG(SS), cdbcmd), CDROM_CDB_आकार / 2);

    पूर्णांक status = pause_await_not_bsy(iobase1, iobase2);
    अगर (status < 0) {
        ret = status;
        जाओ fail;
    }

    अगर (status & ATA_CB_STAT_ERR) {
        u8 err = inb(iobase1 + ATA_CB_ERR);
        // skip "Not Ready"
        अगर (err != 0x20)
            dprintf(6, "send_atapi_cmd : read error (status=%02x err=%02x)\n"
                    , status, err);
        ret = -2;
        जाओ fail;
    }
    अगर (blocksize) {
        अगर (!(status & ATA_CB_STAT_DRQ)) {
            dprintf(6, "send_atapi_cmd : DRQ not set (status %02x)\n", status);
            ret = -3;
            जाओ fail;
        }

        ret = ata_pio_transfer(op, 0, blocksize);
    }

fail:
    // Enable interrupts
    outb(ATA_CB_DC_HD15, iobase2+ATA_CB_DC);
    अगर (ret)
        वापस DISK_RET_EBADTRACK;
    वापस DISK_RET_SUCCESS;
}


/****************************************************************
 * ATA detect and init
 ****************************************************************/

// Send an identify device or identify device packet command.
अटल पूर्णांक
send_ata_identity(काष्ठा atadrive_s *adrive, u16 *बफर, पूर्णांक command)
{
    स_रखो(बफर, 0, DISK_SECTOR_आकार);

    काष्ठा disk_op_s dop;
    स_रखो(&dop, 0, माप(dop));
    dop.drive_fl = &adrive->drive;
    dop.गिनती = 1;
    dop.lba = 1;
    dop.buf_fl = MAKE_FLATPTR(GET_SEG(SS), बफर);

    काष्ठा ata_pio_command cmd;
    स_रखो(&cmd, 0, माप(cmd));
    cmd.command = command;

    वापस ata_pio_cmd_तथ्य(&dop, 0, &cmd);
}

// Extract the ATA/ATAPI version info.
पूर्णांक
ata_extract_version(u16 *बफर)
{
    // Extract ATA/ATAPI version.
    u16 ataversion = बफर[80];
    u8 version;
    क्रम (version=15; version>0; version--)
        अगर (ataversion & (1<<version))
            अवरोध;
    वापस version;
}

#घोषणा MAXMODEL 40

// Extract the ATA/ATAPI model info.
अक्षर *
ata_extract_model(अक्षर *model, u32 आकार, u16 *बफर)
{
    // Read model name
    पूर्णांक i;
    क्रम (i=0; i<आकार/2; i++)
        *(u16*)&model[i*2] = be16_to_cpu(बफर[27+i]);
    model[आकार] = 0x00;
    nullTrailingSpace(model);
    वापस model;
}

// Common init code between ata and atapi
अटल काष्ठा atadrive_s *
init_atadrive(काष्ठा atadrive_s *dummy, u16 *बफर)
{
    काष्ठा atadrive_s *adrive = malloc_fseg(माप(*adrive));
    अगर (!adrive) {
        warn_noalloc();
        वापस शून्य;
    }
    स_रखो(adrive, 0, माप(*adrive));
    adrive->chan_gf = dummy->chan_gf;
    adrive->slave = dummy->slave;
    adrive->drive.cntl_id = adrive->chan_gf->ataid * 2 + dummy->slave;
    adrive->drive.removable = (बफर[0] & 0x80) ? 1 : 0;
    वापस adrive;
}

// Detect if the given drive is an atapi - initialize it if so.
अटल काष्ठा atadrive_s *
init_drive_atapi(काष्ठा atadrive_s *dummy, u16 *बफर)
{
    // Send an IDENTIFY_DEVICE_PACKET command to device
    पूर्णांक ret = send_ata_identity(dummy, बफर, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    अगर (ret)
        वापस शून्य;

    // Success - setup as ATAPI.
    काष्ठा atadrive_s *adrive = init_atadrive(dummy, बफर);
    अगर (!adrive)
        वापस शून्य;
    adrive->drive.type = DTYPE_ATA_ATAPI;
    adrive->drive.blksize = CDROM_SECTOR_आकार;
    adrive->drive.sectors = (u64)-1;
    u8 iscd = ((बफर[0] >> 8) & 0x1f) == 0x05;
    अक्षर model[MAXMODEL+1];
    अक्षर *desc = znprintf(MAXDESCSIZE
                          , "DVD/CD [ata%d-%d: %s ATAPI-%d %s]"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, बफर)
                          , ata_extract_version(बफर)
                          , (iscd ? "DVD/CD" : "Device"));
    dprintf(1, "%s\n", desc);

    // fill cdidmap
    अगर (iscd) {
        पूर्णांक prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                            adrive->chan_gf->chanid,
                                            adrive->slave);
        boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                                  adrive->chan_gf->chanid,
                                  adrive->slave,
                                  &(adrive->drive.lchs));
        boot_add_cd(&adrive->drive, desc, prio);
    }

    वापस adrive;
}

// Detect if the given drive is a regular ata drive - initialize it if so.
अटल काष्ठा atadrive_s *
init_drive_ata(काष्ठा atadrive_s *dummy, u16 *बफर)
{
    // Send an IDENTIFY_DEVICE command to device
    पूर्णांक ret = send_ata_identity(dummy, बफर, ATA_CMD_IDENTIFY_DEVICE);
    अगर (ret)
        वापस शून्य;

    // Success - setup as ATA.
    काष्ठा atadrive_s *adrive = init_atadrive(dummy, बफर);
    अगर (!adrive)
        वापस शून्य;
    adrive->drive.type = DTYPE_ATA;
    adrive->drive.blksize = DISK_SECTOR_आकार;

    adrive->drive.pchs.cylinder = बफर[1];
    adrive->drive.pchs.शीर्ष = बफर[3];
    adrive->drive.pchs.sector = बफर[6];

    u64 sectors;
    अगर (बफर[83] & (1 << 10)) // word 83 - lba48 support
        sectors = *(u64*)&बफर[100]; // word 100-103
    अन्यथा
        sectors = *(u32*)&बफर[60]; // word 60 and word 61
    adrive->drive.sectors = sectors;
    u64 adjsize = sectors >> 11;
    अक्षर adjprefix = 'M';
    अगर (adjsize >= (1 << 16)) {
        adjsize >>= 10;
        adjprefix = 'G';
    }
    अक्षर model[MAXMODEL+1];
    अक्षर *desc = znprintf(MAXDESCSIZE
                          , "ata%d-%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                          , adrive->chan_gf->ataid, adrive->slave
                          , ata_extract_model(model, MAXMODEL, बफर)
                          , ata_extract_version(बफर)
                          , (u32)adjsize, adjprefix);
    dprintf(1, "%s\n", desc);

    पूर्णांक prio = bootprio_find_ata_device(adrive->chan_gf->pci_tmp,
                                        adrive->chan_gf->chanid,
                                        adrive->slave);
    boot_lchs_find_ata_device(adrive->chan_gf->pci_tmp,
                              adrive->chan_gf->chanid,
                              adrive->slave,
                              &(adrive->drive.lchs));
    // Register with bcv system.
    boot_add_hd(&adrive->drive, desc, prio);

    वापस adrive;
}

अटल u32 SpinupEnd;

// Wait for non-busy status and check for "floating bus" condition.
अटल पूर्णांक
powerup_await_non_bsy(u16 base)
{
    u8 orstatus = 0;
    u8 status;
    क्रम (;;) {
        status = inb(base+ATA_CB_STAT);
        अगर (!(status & ATA_CB_STAT_BSY))
            अवरोध;
        orstatus |= status;
        अगर (orstatus == 0xff) {
            dprintf(4, "powerup IDE floating\n");
            वापस orstatus;
        }
        अगर (timer_check(SpinupEnd)) {
            warn_timeout();
            वापस -1;
        }
        yield();
    }
    dprintf(6, "powerup iobase=%x st=%x\n", base, status);
    वापस status;
}

// Detect any drives attached to a given controller.
अटल व्योम
ata_detect(व्योम *तथ्य)
{
    काष्ठा ata_channel_s *chan_gf = तथ्य;
    काष्ठा atadrive_s dummy;
    स_रखो(&dummy, 0, माप(dummy));
    dummy.chan_gf = chan_gf;
    // Device detection
    पूर्णांक didreset = 0;
    u8 slave;
    क्रम (slave=0; slave<=1; slave++) {
        // Wait for not-bsy.
        u16 iobase1 = chan_gf->iobase1;
        पूर्णांक status = powerup_await_non_bsy(iobase1);
        अगर (status < 0)
            जारी;
        u8 newdh = slave ? ATA_CB_DH_DEV1 : ATA_CB_DH_DEV0;
        outb(newdh, iobase1+ATA_CB_DH);
        ndelay(400);
        status = powerup_await_non_bsy(iobase1);
        अगर (status < 0)
            जारी;

        // Check if ioport registers look valid.
        outb(newdh, iobase1+ATA_CB_DH);
        u8 dh = inb(iobase1+ATA_CB_DH);
        outb(0x55, iobase1+ATA_CB_SC);
        outb(0xaa, iobase1+ATA_CB_SN);
        u8 sc = inb(iobase1+ATA_CB_SC);
        u8 sn = inb(iobase1+ATA_CB_SN);
        dprintf(6, "ata_detect ata%d-%d: sc=%x sn=%x dh=%x\n"
                , chan_gf->ataid, slave, sc, sn, dh);
        अगर (sc != 0x55 || sn != 0xaa || dh != newdh)
            जारी;

        // Prepare new drive.
        dummy.slave = slave;

        // reset the channel
        अगर (!didreset) {
            ata_reset(&dummy);
            didreset = 1;
        }

        // check for ATAPI
        u16 बफर[256];
        काष्ठा atadrive_s *adrive = init_drive_atapi(&dummy, बफर);
        अगर (!adrive) {
            // Didn't find an ATAPI drive - look for ATA drive.
            u8 st = inb(iobase1+ATA_CB_STAT);
            अगर (!st)
                // Status not set - can't be a valid drive.
                जारी;

            // Wait for RDY.
            पूर्णांक ret = await_rdy(iobase1);
            अगर (ret < 0)
                जारी;

            // check for ATA.
            adrive = init_drive_ata(&dummy, बफर);
            अगर (!adrive)
                // No ATA drive found
                जारी;
        }

        u16 resetresult = बफर[93];
        dprintf(6, "ata_detect resetresult=%04x\n", resetresult);
        अगर (!slave && (resetresult & 0xdf61) == 0x4041)
            // resetresult looks valid and device 0 is responding to
            // device 1 requests - device 1 must not be present - skip
            // detection.
            अवरोध;
    }
}

// Initialize an ata controller and detect its drives.
अटल व्योम
init_controller(काष्ठा pci_device *pci, पूर्णांक chanid, पूर्णांक irq
                , u32 port1, u32 port2, u32 master)
{
    अटल पूर्णांक ataid = 0;
    काष्ठा ata_channel_s *chan_gf = malloc_fseg(माप(*chan_gf));
    अगर (!chan_gf) {
        warn_noalloc();
        वापस;
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

#घोषणा IRQ_ATA1 14
#घोषणा IRQ_ATA2 15

// Handle controllers on an ATA PCI device.
अटल व्योम
init_pciata(काष्ठा pci_device *pci, u8 prog_if)
{
    u8 pciirq = pci_config_readb(pci->bdf, PCI_INTERRUPT_पंक्ति);
    पूर्णांक master = 0;
    अगर (CONFIG_ATA_DMA && prog_if & 0x80) {
        // Check for bus-mastering.
        u32 bar = pci_config_readl(pci->bdf, PCI_BASE_ADDRESS_4);
        अगर (bar & PCI_BASE_ADDRESS_SPACE_IO) {
            master = pci_enable_iobar(pci, PCI_BASE_ADDRESS_4);
            pci_enable_busmaster(pci);
        }
    }

    u32 port1, port2, irq;
    अगर (prog_if & 1) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_1);
        अगर (!port1 || !port2)
            वापस;
        irq = pciirq;
    } अन्यथा {
        port1 = PORT_ATA1_CMD_BASE;
        port2 = PORT_ATA1_CTRL_BASE;
        irq = IRQ_ATA1;
    }
    init_controller(pci, 0, irq, port1, port2, master);

    अगर (prog_if & 4) {
        port1 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_2);
        port2 = pci_enable_iobar(pci, PCI_BASE_ADDRESS_3);
        अगर (!port1 || !port2)
            वापस;
        irq = pciirq;
    } अन्यथा {
        port1 = PORT_ATA2_CMD_BASE;
        port2 = PORT_ATA2_CTRL_BASE;
        irq = IRQ_ATA2;
    }
    init_controller(pci, 1, irq, port1, port2, master ? master + 8 : 0);
}

अटल व्योम
found_genericata(काष्ठा pci_device *pci, व्योम *arg)
{
    init_pciata(pci, pci->prog_if);
}

अटल व्योम
found_compatibleahci(काष्ठा pci_device *pci, व्योम *arg)
{
    अगर (CONFIG_AHCI)
        // Already handled directly via native ahci interface.
        वापस;
    init_pciata(pci, 0x8f);
}

अटल स्थिर काष्ठा pci_device_id pci_ata_tbl[] = {
    PCI_DEVICE_CLASS(PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE
                     , found_genericata),
    PCI_DEVICE(PCI_VENDOR_ID_ATI, 0x4391, found_compatibleahci),
    PCI_DEVICE_END,
};

// Locate and init ata controllers.
अटल व्योम
ata_scan(व्योम)
{
    अगर (CONFIG_QEMU && hlist_empty(&PCIDevices)) {
        // No PCI devices found - probably a QEMU "-M isapc" machine.
        // Try using ISA ports for ATA controllers.
        init_controller(शून्य, 0, IRQ_ATA1
                        , PORT_ATA1_CMD_BASE, PORT_ATA1_CTRL_BASE, 0);
        init_controller(शून्य, 1, IRQ_ATA2
                        , PORT_ATA2_CMD_BASE, PORT_ATA2_CTRL_BASE, 0);
        वापस;
    }

    // Scan PCI bus for ATA adapters
    काष्ठा pci_device *pci;
    foreachpci(pci) {
        pci_init_device(pci_ata_tbl, pci, शून्य);
    }
}

व्योम
ata_setup(व्योम)
{
    ASSERT32FLAT();
    अगर (!CONFIG_ATA)
        वापस;

    dprintf(3, "init hard drives\n");

    SpinupEnd = timer_calc(IDE_TIMEOUT);
    ata_scan();

    SET_BDA(disk_control_byte, 0xc0);

    enable_hwirq(14, FUNC16(entry_76));
}

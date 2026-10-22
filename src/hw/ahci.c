// Low level AHCI disk access
//
// Copyright (C) 2010 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "ahci.h" // CDB_CMD_READ_10
#ಅನ್ತರ್ಭಾವ "ata.h" // ATA_CB_STAT
#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBAL
#ಅನ್ತರ್ಭಾವ "blockcmd.h" // CDB_CMD_READ_10
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pci.h" // pci_config_readb
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_INTERRUPT_LINE
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_SUCCESS
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // timer_calc
#ಅನ್ತರ್ಭಾವ "x86.h" // inb

#define AHCI_REQUEST_TIMEOUT 32000 // 32 seconds max for IDE ops
#define AHCI_RESET_TIMEOUT     500 // 500 miliseconds
#define AHCI_LINK_TIMEOUT       10 // 10 miliseconds

// prepare sata command fis
ಸ್ಥಿರ ಶೂನ್ಯ sata_prep_simple(ಸಂರಚನಾ sata_cmd_fis *fis, u8 command)
{
    memset_fl(fis, 0, ಪರಿಮಾಣ(*fis));
    fis->command = command;
}

ಸ್ಥಿರ ಶೂನ್ಯ sata_prep_readwrite(ಸಂರಚನಾ sata_cmd_fis *fis,
                                ಸಂರಚನಾ disk_op_s *op, ಪೂರ್ಣಾಙ್ಕ iswrite)
{
    u64 lba = op->lba;
    u8 command;

    memset_fl(fis, 0, ಪರಿಮಾಣ(*fis));

    ಯದಿ (op->ಎಣಿಕೆ >= (1<<8) || lba + op->ಎಣಿಕೆ >= (1<<28)) {
        fis->sector_count2 = op->ಎಣಿಕೆ >> 8;
        fis->lba_low2      = lba >> 24;
        fis->lba_mid2      = lba >> 32;
        fis->lba_high2     = lba >> 40;
        lba &= 0xffffff;
        command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                   : ATA_CMD_READ_DMA_EXT);
    } ಅನ್ಯಥಾ {
        command = (iswrite ? ATA_CMD_WRITE_DMA
                   : ATA_CMD_READ_DMA);
    }
    fis->feature      = 1; /* dma */
    fis->command      = command;
    fis->sector_ಎಣಿಕೆ = op->ಎಣಿಕೆ;
    fis->lba_ಕೆಳಗಿನ      = lba;
    fis->lba_mid      = lba >> 8;
    fis->lba_ಮೇಲಿನ     = lba >> 16;
    fis->device       = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;
}

ಸ್ಥಿರ ಶೂನ್ಯ sata_prep_atapi(ಸಂರಚನಾ sata_cmd_fis *fis, u16 blocksize)
{
    memset_fl(fis, 0, ಪರಿಮಾಣ(*fis));
    fis->command  = ATA_CMD_PACKET;
    fis->feature  = 1; /* dma */
    fis->lba_mid  = blocksize;
    fis->lba_ಮೇಲಿನ = blocksize >> 8;
}

// ahci register access helpers
ಸ್ಥಿರ u32 ahci_ctrl_readl(ಸಂರಚನಾ ahci_ctrl_s *ctrl, u32 reg)
{
    ಪ್ರತ್ಯಾಗಮ readl(ctrl->iobase + reg);
}

ಸ್ಥಿರ ಶೂನ್ಯ ahci_ctrl_writel(ಸಂರಚನಾ ahci_ctrl_s *ctrl, u32 reg, u32 val)
{
    writel(ctrl->iobase + reg, val);
}

ಸ್ಥಿರ u32 ahci_port_to_ctrl(u32 pnr, u32 port_reg)
{
    u32 ctrl_reg = 0x100;
    ctrl_reg += pnr * 0x80;
    ctrl_reg += port_reg;
    ಪ್ರತ್ಯಾಗಮ ctrl_reg;
}

ಸ್ಥಿರ u32 ahci_port_readl(ಸಂರಚನಾ ahci_ctrl_s *ctrl, u32 pnr, u32 reg)
{
    u32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    ಪ್ರತ್ಯಾಗಮ ahci_ctrl_readl(ctrl, ctrl_reg);
}

ಸ್ಥಿರ ಶೂನ್ಯ ahci_port_writel(ಸಂರಚನಾ ahci_ctrl_s *ctrl, u32 pnr, u32 reg, u32 val)
{
    u32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    ahci_ctrl_writel(ctrl, ctrl_reg, val);
}

// submit ahci command + wait for result
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ahci_command(ಸಂರಚನಾ ahci_port_s *port_gf, ಪೂರ್ಣಾಙ್ಕ iswrite, ಪೂರ್ಣಾಙ್ಕ isatapi,
                        ಶೂನ್ಯ *buffer, u32 bsize)
{
    u32 val, status, success, flags, intbits, error;
    ಸಂರಚನಾ ahci_ctrl_s *ctrl = port_gf->ctrl;
    ಸಂರಚನಾ ahci_cmd_s  *cmd  = port_gf->cmd;
    ಸಂರಚನಾ ahci_fis_s  *fis  = port_gf->fis;
    ಸಂರಚನಾ ahci_list_s *list = port_gf->list;
    u32 pnr                  = port_gf->pnr;

    cmd->fis.reg       = 0x27;
    cmd->fis.pmp_type  = 1 << 7; /* cmd fis */
    cmd->prdt[0].base  = (u32)buffer;
    cmd->prdt[0].baseu = 0;
    cmd->prdt[0].flags = bsize-1;

    flags = ((1 << 16) | /* one prd entry */
             (iswrite ? (1 << 6) : 0) |
             (isatapi ? (1 << 5) : 0) |
             (5 << 0)); /* fis length (dwords) */
    list[0].flags  = flags;
    list[0].bytes  = 0;
    list[0].base   = (u32)(cmd);
    list[0].baseu  = 0;

    dprintf(8, "AHCI/%d: send cmd ...\n", pnr);
    intbits = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
    ಯದಿ (intbits)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
    ahci_port_writel(ctrl, pnr, PORT_CMD_ISSUE, 1);

    u32 end = timer_calc(AHCI_REQUEST_TIMEOUT);
    ಕುರು {
        ಪ್ರತಿ (;;) {
            intbits = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
            ಯದಿ (intbits) {
                ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
                ಯದಿ (intbits & 0x40000000) {
                    u32 tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
                    status = tf & 0xff;
                    error = (tf & 0xff00) >> 8;
                    ಭಙ್ಗ;
                }
                ಯದಿ (intbits & 0x02) {
                    status = GET_LOWFLAT(fis->psfis[2]);
                    error  = GET_LOWFLAT(fis->psfis[3]);
                    ಭಙ್ಗ;
                }
                ಯದಿ (intbits & 0x01) {
                    status = GET_LOWFLAT(fis->rfis[2]);
                    error  = GET_LOWFLAT(fis->rfis[3]);
                    ಭಙ್ಗ;
                }
            }
            ಯದಿ (timer_check(end)) {
                warn_timeout();
                ಪ್ರತ್ಯಾಗಮ -1;
            }
            yield();
        }
        dprintf(8, "AHCI/%d: ... intbits 0x%x, status 0x%x ...\n",
                pnr, intbits, status);
    } ಯಾವತ್ (status & ATA_CB_STAT_BSY);

    success = (0x00 == (status & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF |
                                  ATA_CB_STAT_ERR)) &&
               ATA_CB_STAT_RDY == (status & (ATA_CB_STAT_RDY)));
    ಯದಿ (success) {
        dprintf(8, "AHCI/%d: ... finished, status 0x%x, OK\n", pnr,
                status);
    } ಅನ್ಯಥಾ {
        dprintf(2, "AHCI/%d: ... finished, status 0x%x, ERROR 0x%x\n", pnr,
                status, error);

        // non-queued error recovery (AHCI 1.3 section 6.2.2.1)
        // Clears PxCMD.ST to 0 to reset the PxCI register
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val & ~PORT_CMD_START);

        // waits for PxCMD.CR to clear to 0
        ಯಾವತ್ (1) {
            val = ahci_port_readl(ctrl, pnr, PORT_CMD);
            ಯದಿ ((val & PORT_CMD_LIST_ON) == 0)
                ಭಙ್ಗ;
            yield();
        }

        // Clears any error bits in PxSERR to enable capturing new errors
        val = ahci_port_readl(ctrl, pnr, PORT_SCR_ERR);
        ahci_port_writel(ctrl, pnr, PORT_SCR_ERR, val);

        // Clears status bits in PxIS as appropriate
        val = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, val);

        // If PxTFD.STS.BSY or PxTFD.STS.DRQ is set to 1, issue
        // a COMRESET to the device to put it in an idle state
        val = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
        ಯದಿ (val & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ)) {
            dprintf(2, "AHCI/%d: issue comreset\n", pnr);
            val = ahci_port_readl(ctrl, pnr, PORT_SCR_CTL);
            // set Device Detection Initialization (DET) to 1 for 1 ms for comreset
            ahci_port_writel(ctrl, pnr, PORT_SCR_CTL, val | 1);
            mdelay (1);
            ahci_port_writel(ctrl, pnr, PORT_SCR_CTL, val);
        }

        // Sets PxCMD.ST to 1 to enable issuing new commands
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val | PORT_CMD_START);
    }
    ಪ್ರತ್ಯಾಗಮ success ? 0 : -1;
}

#define CDROM_CDB_SIZE 12

ಪೂರ್ಣಾಙ್ಕ ahci_atapi_process_op(ಸಂರಚನಾ disk_op_s *op)
{
    ಯದಿ (! CONFIG_AHCI)
        ಪ್ರತ್ಯಾಗಮ 0;

    ಸಂರಚನಾ ahci_port_s *port_gf = container_of(
        op->drive_fl, ಸಂರಚನಾ ahci_port_s, drive);
    ಸಂರಚನಾ ahci_cmd_s *cmd = port_gf->cmd;

    ಯದಿ (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_EWRITEPROTECT;
    ಪೂರ್ಣಾಙ್ಕ blocksize = scsi_fill_cmd(op, cmd->atapi, CDROM_CDB_SIZE);
    ಯದಿ (blocksize < 0)
        ಪ್ರತ್ಯಾಗಮ default_process_op(op);
    sata_prep_atapi(&cmd->fis, blocksize);
    ಪೂರ್ಣಾಙ್ಕ rc = ahci_command(port_gf, 0, 1, op->buf_fl, op->ಎಣಿಕೆ * blocksize);
    ಯದಿ (rc < 0)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_EBADTRACK;
    ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive, op->buf_fl must be word aligned
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ahci_disk_readwrite_aligned(ಸಂರಚನಾ disk_op_s *op, ಪೂರ್ಣಾಙ್ಕ iswrite)
{
    ಸಂರಚನಾ ahci_port_s *port_gf = container_of(
        op->drive_fl, ಸಂರಚನಾ ahci_port_s, drive);
    ಸಂರಚನಾ ahci_cmd_s *cmd = port_gf->cmd;
    ಪೂರ್ಣಾಙ್ಕ rc;

    sata_prep_readwrite(&cmd->fis, op, iswrite);
    rc = ahci_command(port_gf, iswrite, 0, op->buf_fl,
                      op->ಎಣಿಕೆ * DISK_SECTOR_SIZE);
    dprintf(8, "ahci disk %s, lba %6x, count %3x, buf %p, rc %d\n",
            iswrite ? "write" : "read", (u32)op->lba, op->ಎಣಿಕೆ, op->buf_fl, rc);
    ಯದಿ (rc < 0)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_EBADTRACK;
    ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive.
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ahci_disk_readwrite(ಸಂರಚನಾ disk_op_s *op, ಪೂರ್ಣಾಙ್ಕ iswrite)
{
    // if caller's buffer is word aligned, use it directly
    ಯದಿ (((u32) op->buf_fl & 1) == 0)
        ಪ್ರತ್ಯಾಗಮ ahci_disk_readwrite_aligned(op, iswrite);

    // Use a word aligned buffer for AHCI I/O
    ಪೂರ್ಣಾಙ್ಕ rc;
    ಸಂರಚನಾ disk_op_s localop = *op;
    u8 *alignedbuf_fl = bounce_buf_fl;
    u8 *position = op->buf_fl;

    localop.buf_fl = alignedbuf_fl;
    localop.ಎಣಿಕೆ = 1;

    ಯದಿ (iswrite) {
        u16 block;
        ಪ್ರತಿ (block = 0; block < op->ಎಣಿಕೆ; block++) {
            memcpy_fl (alignedbuf_fl, position, DISK_SECTOR_SIZE);
            rc = ahci_disk_readwrite_aligned (&localop, 1);
            ಯದಿ (rc)
                ಪ್ರತ್ಯಾಗಮ rc;
            position += DISK_SECTOR_SIZE;
            localop.lba++;
        }
    } ಅನ್ಯಥಾ { // read
        u16 block;
        ಪ್ರತಿ (block = 0; block < op->ಎಣಿಕೆ; block++) {
            rc = ahci_disk_readwrite_aligned (&localop, 0);
            ಯದಿ (rc)
                ಪ್ರತ್ಯಾಗಮ rc;
            memcpy_fl (position, alignedbuf_fl, DISK_SECTOR_SIZE);
            position += DISK_SECTOR_SIZE;
            localop.lba++;
        }
    }
    ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;
}

// command demuxer
ಪೂರ್ಣಾಙ್ಕ
ahci_process_op(ಸಂರಚನಾ disk_op_s *op)
{
    ಯದಿ (!CONFIG_AHCI)
        ಪ್ರತ್ಯಾಗಮ 0;
    ವಿಕಲ್ಪ (op->command) {
    ಪಕ್ಷ CMD_READ:
        ಪ್ರತ್ಯಾಗಮ ahci_disk_readwrite(op, 0);
    ಪಕ್ಷ CMD_WRITE:
        ಪ್ರತ್ಯಾಗಮ ahci_disk_readwrite(op, 1);
    ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ default_process_op(op);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
ahci_port_reset(ಸಂರಚನಾ ahci_ctrl_s *ctrl, u32 pnr)
{
    u32 val;

    /* disable FIS + CMD */
    u32 end = timer_calc(AHCI_RESET_TIMEOUT);
    ಪ್ರತಿ (;;) {
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        ಯದಿ (!(val & (PORT_CMD_FIS_RX | PORT_CMD_START |
                     PORT_CMD_FIS_ON | PORT_CMD_LIST_ON)))
            ಭಙ್ಗ;
        val &= ~(PORT_CMD_FIS_RX | PORT_CMD_START);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val);
        ಯದಿ (timer_check(end)) {
            warn_timeout();
            ಭಙ್ಗ;
        }
        yield();
    }

    /* disable + clear IRQs */
    ahci_port_writel(ctrl, pnr, PORT_IRQ_MASK, 0);
    val = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
    ಯದಿ (val)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, val);
}

ಸ್ಥಿರ ಸಂರಚನಾ ahci_port_s*
ahci_port_alloc(ಸಂರಚನಾ ahci_ctrl_s *ctrl, u32 pnr)
{
    ಸಂರಚನಾ ahci_port_s *port = malloc_tmp(ಪರಿಮಾಣ(*port));

    ಯದಿ (!port) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    memset(port, 0, ಪರಿಮಾಣ(*port));
    port->pnr = pnr;
    port->ctrl = ctrl;
    port->list = memalign_tmp(1024, 1024);
    port->fis = memalign_tmp(256, 256);
    port->cmd = memalign_tmp(256, 256);
    ಯದಿ (port->list == ರಿಕ್ತ || port->fis == ರಿಕ್ತ || port->cmd == ರಿಕ್ತ) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    memset(port->list, 0, 1024);
    memset(port->fis, 0, 256);
    memset(port->cmd, 0, 256);

    ahci_port_writel(ctrl, pnr, PORT_LST_ADDR, (u32)port->list);
    ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR, (u32)port->fis);
    ಯದಿ (ctrl->caps & HOST_CAP_64) {
        ahci_port_writel(ctrl, pnr, PORT_LST_ADDR_HI, 0);
        ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR_HI, 0);
    }

    ಪ್ರತ್ಯಾಗಮ port;
}

ಸ್ಥಿರ ಶೂನ್ಯ ahci_port_release(ಸಂರಚನಾ ahci_port_s *port)
{
    ahci_port_reset(port->ctrl, port->pnr);
    ಮುಕ್ತ(port->list);
    ಮುಕ್ತ(port->fis);
    ಮುಕ್ತ(port->cmd);
    ಮುಕ್ತ(port);
}

ಸ್ಥಿರ ಸಂರಚನಾ ahci_port_s* ahci_port_realloc(ಸಂರಚನಾ ahci_port_s *port)
{
    ಸಂರಚನಾ ahci_port_s *tmp;
    u32 cmd;

    tmp = malloc_fseg(ಪರಿಮಾಣ(*port));
    ಯದಿ (!tmp) {
        warn_noalloc();
        ahci_port_release(port);
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    *tmp = *port;
    ಮುಕ್ತ(port);
    port = tmp;

    ahci_port_reset(port->ctrl, port->pnr);

    ಮುಕ್ತ(port->list);
    ಮುಕ್ತ(port->fis);
    ಮುಕ್ತ(port->cmd);
    port->list = memalign_ಮೇಲಿನ(1024, 1024);
    port->fis = memalign_ಮೇಲಿನ(256, 256);
    port->cmd = memalign_ಮೇಲಿನ(256, 256);
    ಯದಿ (!port->list || !port->fis || !port->cmd) {
        warn_noalloc();
        ಮುಕ್ತ(port->list);
        ಮುಕ್ತ(port->fis);
        ಮುಕ್ತ(port->cmd);
        ಮುಕ್ತ(port);
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }

    ahci_port_writel(port->ctrl, port->pnr, PORT_LST_ADDR, (u32)port->list);
    ahci_port_writel(port->ctrl, port->pnr, PORT_FIS_ADDR, (u32)port->fis);

    cmd = ahci_port_readl(port->ctrl, port->pnr, PORT_CMD);
    cmd |= (PORT_CMD_FIS_RX|PORT_CMD_START);
    ahci_port_writel(port->ctrl, port->pnr, PORT_CMD, cmd);

    ಪ್ರತ್ಯಾಗಮ port;
}

#define MAXMODEL 40

/* See ahci spec chapter 10.1 "Software Initialization of HBA" */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ahci_port_setup(ಸಂರಚನಾ ahci_port_s *port)
{
    ಸಂರಚನಾ ahci_ctrl_s *ctrl = port->ctrl;
    u32 pnr = port->pnr;
    ವರ್ಣ model[MAXMODEL+1];
    u16 buffer[256];
    u32 cmd, stat, err, tf;
    ಪೂರ್ಣಾಙ್ಕ rc;

    /* enable FIS recv */
    cmd = ahci_port_readl(ctrl, pnr, PORT_CMD);
    cmd |= PORT_CMD_FIS_RX;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    /* spin up */
    cmd &= ~PORT_CMD_ICC_MASK;
    cmd |= PORT_CMD_SPIN_UP | PORT_CMD_POWER_ON | PORT_CMD_ICC_ACTIVE;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);
    u32 end = timer_calc(AHCI_LINK_TIMEOUT);
    ಪ್ರತಿ (;;) {
        stat = ahci_port_readl(ctrl, pnr, PORT_SCR_STAT);
        ಯದಿ ((stat & 0x07) == 0x03) {
            dprintf(2, "AHCI/%d: link up\n", port->pnr);
            ಭಙ್ಗ;
        }
        ಯದಿ (timer_check(end)) {
            dprintf(2, "AHCI/%d: link down\n", port->pnr);
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        yield();
    }

    /* clear error status */
    err = ahci_port_readl(ctrl, pnr, PORT_SCR_ERR);
    ಯದಿ (err)
        ahci_port_writel(ctrl, pnr, PORT_SCR_ERR, err);

    /* wait for device becoming ready */
    end = timer_calc(AHCI_REQUEST_TIMEOUT);
    ಪ್ರತಿ (;;) {
        tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
        ಯದಿ (!(tf & (ATA_CB_STAT_BSY |
                    ATA_CB_STAT_DRQ)))
            ಭಙ್ಗ;
        ಯದಿ (timer_check(end)) {
            warn_timeout();
            dprintf(1, "AHCI/%d: device not ready (tf 0x%x)\n", port->pnr, tf);
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        yield();
    }

    /* start device */
    cmd |= PORT_CMD_START;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    rc = ahci_command(port, 0, 0, buffer, ಪರಿಮಾಣ(buffer));
    ಯದಿ (rc == 0) {
        port->atapi = 1;
    } ಅನ್ಯಥಾ {
        port->atapi = 0;
        sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
        rc = ahci_command(port, 0, 0, buffer, ಪರಿಮಾಣ(buffer));
        ಯದಿ (rc < 0)
            ಪ್ರತ್ಯಾಗಮ -1;
    }

    port->drive.cntl_id = pnr;
    port->drive.removable = (buffer[0] & 0x80) ? 1 : 0;

    ಯದಿ (!port->atapi) {
        // found disk (ata)
        port->drive.type = DTYPE_AHCI;
        port->drive.blksize = DISK_SECTOR_SIZE;
        port->drive.pchs.cylinder = buffer[1];
        port->drive.pchs.head = buffer[3];
        port->drive.pchs.sector = buffer[6];

        u64 sectors;
        ಯದಿ (buffer[83] & (1 << 10)) // word 83 - lba48 support
            sectors = *(u64*)&buffer[100]; // word 100-103
        ಅನ್ಯಥಾ
            sectors = *(u32*)&buffer[60]; // word 60 and word 61
        port->drive.sectors = sectors;
        u64 adjsize = sectors >> 11;
        ವರ್ಣ adjprefix = 'M';
        ಯದಿ (adjsize >= (1 << 16)) {
            adjsize >>= 10;
            adjprefix = 'G';
        }
        port->desc = znprintf(MAXDESCSIZE
                              , "AHCI/%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                              , port->pnr
                              , ata_extract_model(model, MAXMODEL, buffer)
                              , ata_extract_version(buffer)
                              , (u32)adjsize, adjprefix);
        port->prio = bootprio_find_ata_device(ctrl->pci_tmp, pnr, 0);

        s8 multi_dma = -1;
        s8 pio_mode = -1;
        s8 udma_mode = -1;
        // If bit 2 in word 53 is set, udma information is valid in word 88.
        ಯದಿ (buffer[53] & 0x04) {
            udma_mode = 6;
            ಯಾವತ್ ((udma_mode >= 0) &&
                   !((buffer[88] & 0x7f) & ( 1 << udma_mode ))) {
                udma_mode--;
            }
        }
        // If bit 1 in word 53 is set, multiword-dma and advanced pio modes
        // are available in words 63 and 64.
        ಯದಿ (buffer[53] & 0x02) {
            pio_mode = 4;
            multi_dma = 3;
            ಯಾವತ್ ((multi_dma >= 0) &&
                   !((buffer[63] & 0x7) & ( 1 << multi_dma ))) {
                multi_dma--;
            }
            ಯಾವತ್ ((pio_mode >= 3) &&
                   !((buffer[64] & 0x3) & ( 1 << ( pio_mode - 3 ) ))) {
                pio_mode--;
            }
        }
        dprintf(2, "AHCI/%d: supported modes: udma %d, multi-dma %d, pio %d\n",
                port->pnr, udma_mode, multi_dma, pio_mode);

        sata_prep_simple(&port->cmd->fis, ATA_CMD_SET_FEATURES);
        port->cmd->fis.feature = ATA_SET_FEATRUE_TRANSFER_MODE;
        // Select used mode. UDMA first, then Multi-DMA followed by
        // advanced PIO modes 3 or 4. If non, set default PIO.
        ಯದಿ (udma_mode >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to UDMA-%d\n",
                    port->pnr, udma_mode);
            port->cmd->fis.sector_ಎಣಿಕೆ = ATA_TRANSFER_MODE_ULTRA_DMA
                                          | udma_mode;
        } ಅನ್ಯಥಾ ಯದಿ (multi_dma >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to Multi-DMA-%d\n",
                    port->pnr, multi_dma);
            port->cmd->fis.sector_ಎಣಿಕೆ = ATA_TRANSFER_MODE_MULTIWORD_DMA
                                          | multi_dma;
        } ಅನ್ಯಥಾ ಯದಿ (pio_mode >= 3) {
            dprintf(1, "AHCI/%d: Set transfer mode to PIO-%d\n",
                    port->pnr, pio_mode);
            port->cmd->fis.sector_ಎಣಿಕೆ = ATA_TRANSFER_MODE_PIO_FLOW_CTRL
                                          | pio_mode;
        } ಅನ್ಯಥಾ {
            dprintf(1, "AHCI/%d: Set transfer mode to default PIO\n",
                    port->pnr);
            port->cmd->fis.sector_ಎಣಿಕೆ = ATA_TRANSFER_MODE_DEFAULT_PIO;
        }
        rc = ahci_command(port, 1, 0, 0, 0);
        ಯದಿ (rc < 0) {
            dprintf(1, "AHCI/%d: Set transfer mode failed.\n", port->pnr);
        }
    } ಅನ್ಯಥಾ {
        // found cdrom (atapi)
        port->drive.type = DTYPE_AHCI_ATAPI;
        port->drive.blksize = CDROM_SECTOR_SIZE;
        port->drive.sectors = (u64)-1;
        u8 iscd = ((buffer[0] >> 8) & 0x1f) == 0x05;
        ಯದಿ (!iscd) {
            dprintf(1, "AHCI/%d: atapi device isn't a cdrom\n", port->pnr);
            ಪ್ರತ್ಯಾಗಮ -1;
        }
        port->desc = znprintf(MAXDESCSIZE
                              , "DVD/CD [AHCI/%d: %s ATAPI-%d DVD/CD]"
                              , port->pnr
                              , ata_extract_model(model, MAXMODEL, buffer)
                              , ata_extract_version(buffer));
        port->prio = bootprio_find_ata_device(ctrl->pci_tmp, pnr, 0);
    }
    boot_lchs_find_ata_device(ctrl->pci_tmp, pnr, 0, &(port->drive.lchs));
    ಪ್ರತ್ಯಾಗಮ 0;
}

// Detect any drives attached to a given controller.
ಸ್ಥಿರ ಶೂನ್ಯ
ahci_port_detect(ಶೂನ್ಯ *data)
{
    ಸಂರಚನಾ ahci_port_s *port = data;
    ಪೂರ್ಣಾಙ್ಕ rc;

    dprintf(2, "AHCI/%d: probing\n", port->pnr);
    ahci_port_reset(port->ctrl, port->pnr);
    rc = ahci_port_setup(port);
    ಯದಿ (rc < 0)
        ahci_port_release(port);
    ಅನ್ಯಥಾ {
        port = ahci_port_realloc(port);
        ಯದಿ (port == ರಿಕ್ತ)
            ಪ್ರತ್ಯಾಗಮ;
        dprintf(1, "AHCI/%d: registering: \"%s\"\n", port->pnr, port->desc);
        ಯದಿ (!port->atapi) {
            // Register with bcv system.
            boot_add_hd(&port->drive, port->desc, port->prio);
        } ಅನ್ಯಥಾ {
            // fill cdidmap
            boot_add_cd(&port->drive, port->desc, port->prio);
        }
    }
}

// Initialize an ata controller and detect its drives.
ಸ್ಥಿರ ಶೂನ್ಯ
ahci_controller_setup(ಸಂರಚನಾ pci_device *pci)
{
    ಸಂರಚನಾ ahci_port_s *port;
    u32 pnr, max;

    ಯದಿ (create_bounce_buf() < 0)
        ಪ್ರತ್ಯಾಗಮ;

    ಶೂನ್ಯ *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_5);
    ಯದಿ (!iobase)
        ಪ್ರತ್ಯಾಗಮ;

    ಸಂರಚನಾ ahci_ctrl_s *ctrl = malloc_fseg(ಪರಿಮಾಣ(*ctrl));
    ಯದಿ (!ctrl) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }

    ctrl->pci_tmp = pci;
    ctrl->iobase = iobase;
    ctrl->irq = pci_config_readb(pci->bdf, PCI_INTERRUPT_ಸಾಲು);
    dprintf(1, "AHCI controller at %pP, iobase %p, irq %d\n"
            , pci, ctrl->iobase, ctrl->irq);

    pci_enable_busmaster(pci);

    u32 val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_RESET);
    u32 end = timer_calc(AHCI_RESET_TIMEOUT);
    ಪ್ರತಿ (;;) {
        val = ahci_ctrl_readl(ctrl, HOST_CTL);
        ಯದಿ (!(val & HOST_CTL_RESET))
            ಭಙ್ಗ;
        ಯದಿ (timer_check(end)) {
            warn_timeout();
            dprintf(1, "AHCI: controller reset failed\n");
            ಮುಕ್ತ(ctrl);
            ಪ್ರತ್ಯಾಗಮ;
        }
        yield();
    }
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_AHCI_EN);
    (ಶೂನ್ಯ)ahci_ctrl_readl(ctrl, HOST_CTL); /* Flush */

    ctrl->caps = ahci_ctrl_readl(ctrl, HOST_CAP);
    ctrl->ports = ahci_ctrl_readl(ctrl, HOST_PORTS_IMPL);
    dprintf(2, "AHCI: cap 0x%x, ports_impl 0x%x\n",
            ctrl->caps, ctrl->ports);

    max = 0x1f;
    ಪ್ರತಿ (pnr = 0; pnr <= max; pnr++) {
        ಯದಿ (!(ctrl->ports & (1 << pnr)))
            ಅನುವರ್ತ;
        port = ahci_port_alloc(ctrl, pnr);
        ಯದಿ (port == ರಿಕ್ತ)
            ಅನುವರ್ತ;
        run_thread(ahci_port_detect, port);
    }
}

// Locate and init ahci controllers.
ಸ್ಥಿರ ಶೂನ್ಯ
ahci_scan(ಶೂನ್ಯ)
{
    // Scan PCI bus for ATA adapters
    ಸಂರಚನಾ pci_device *pci;
    foreachpci(pci) {
        ಯದಿ (pci->class != PCI_CLASS_STORAGE_SATA)
            ಅನುವರ್ತ;
        ಯದಿ (pci->prog_if != 1 /* AHCI rev 1 */)
            ಅನುವರ್ತ;
        ahci_controller_setup(pci);
    }
}

ಶೂನ್ಯ
ahci_setup(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಯದಿ (!CONFIG_AHCI)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(3, "init ahci\n");
    ahci_scan();
}

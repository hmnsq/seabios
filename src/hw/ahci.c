// Low level AHCI disk access
//
// Copyright (C) 2010 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "ahci.h" // CDB_CMD_READ_10
#અન્તર્ભાવ "ata.h" // ATA_CB_STAT
#અન્તર્ભાવ "biosvar.h" // GET_GLOBAL
#અન્તર્ભાવ "blockcmd.h" // CDB_CMD_READ_10
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "pci.h" // pci_config_readb
#અન્તર્ભાવ "pcidevice.h" // foreachpci
#અન્તર્ભાવ "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#અન્તર્ભાવ "pci_regs.h" // PCI_INTERRUPT_LINE
#અન્તર્ભાવ "stacks.h" // yield
#અન્તર્ભાવ "std/disk.h" // DISK_RET_SUCCESS
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // timer_calc
#અન્તર્ભાવ "x86.h" // inb

#define AHCI_REQUEST_TIMEOUT 32000 // 32 seconds max for IDE ops
#define AHCI_RESET_TIMEOUT     500 // 500 miliseconds
#define AHCI_LINK_TIMEOUT       10 // 10 miliseconds

// prepare sata command fis
સ્થિર શૂન્ય sata_prep_simple(સંરચના sata_cmd_fis *fis, u8 command)
{
    memset_fl(fis, 0, કદ(*fis));
    fis->command = command;
}

સ્થિર શૂન્ય sata_prep_readwrite(સંરચના sata_cmd_fis *fis,
                                સંરચના disk_op_s *op, પૂર્ણાંક iswrite)
{
    u64 lba = op->lba;
    u8 command;

    memset_fl(fis, 0, કદ(*fis));

    જો (op->ગણતરી >= (1<<8) || lba + op->ગણતરી >= (1<<28)) {
        fis->sector_count2 = op->ગણતરી >> 8;
        fis->lba_low2      = lba >> 24;
        fis->lba_mid2      = lba >> 32;
        fis->lba_high2     = lba >> 40;
        lba &= 0xffffff;
        command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                   : ATA_CMD_READ_DMA_EXT);
    } નહીંતર {
        command = (iswrite ? ATA_CMD_WRITE_DMA
                   : ATA_CMD_READ_DMA);
    }
    fis->feature      = 1; /* dma */
    fis->command      = command;
    fis->sector_ગણતરી = op->ગણતરી;
    fis->lba_નીચલું      = lba;
    fis->lba_mid      = lba >> 8;
    fis->lba_ઉપલું     = lba >> 16;
    fis->device       = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;
}

સ્થિર શૂન્ય sata_prep_atapi(સંરચના sata_cmd_fis *fis, u16 blocksize)
{
    memset_fl(fis, 0, કદ(*fis));
    fis->command  = ATA_CMD_PACKET;
    fis->feature  = 1; /* dma */
    fis->lba_mid  = blocksize;
    fis->lba_ઉપલું = blocksize >> 8;
}

// ahci register access helpers
સ્થિર u32 ahci_ctrl_readl(સંરચના ahci_ctrl_s *ctrl, u32 reg)
{
    પાછા readl(ctrl->iobase + reg);
}

સ્થિર શૂન્ય ahci_ctrl_writel(સંરચના ahci_ctrl_s *ctrl, u32 reg, u32 val)
{
    writel(ctrl->iobase + reg, val);
}

સ્થિર u32 ahci_port_to_ctrl(u32 pnr, u32 port_reg)
{
    u32 ctrl_reg = 0x100;
    ctrl_reg += pnr * 0x80;
    ctrl_reg += port_reg;
    પાછા ctrl_reg;
}

સ્થિર u32 ahci_port_readl(સંરચના ahci_ctrl_s *ctrl, u32 pnr, u32 reg)
{
    u32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    પાછા ahci_ctrl_readl(ctrl, ctrl_reg);
}

સ્થિર શૂન્ય ahci_port_writel(સંરચના ahci_ctrl_s *ctrl, u32 pnr, u32 reg, u32 val)
{
    u32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    ahci_ctrl_writel(ctrl, ctrl_reg, val);
}

// submit ahci command + wait for result
સ્થિર પૂર્ણાંક ahci_command(સંરચના ahci_port_s *port_gf, પૂર્ણાંક iswrite, પૂર્ણાંક isatapi,
                        શૂન્ય *buffer, u32 bsize)
{
    u32 val, status, success, flags, intbits, error;
    સંરચના ahci_ctrl_s *ctrl = port_gf->ctrl;
    સંરચના ahci_cmd_s  *cmd  = port_gf->cmd;
    સંરચના ahci_fis_s  *fis  = port_gf->fis;
    સંરચના ahci_list_s *list = port_gf->list;
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
    જો (intbits)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
    ahci_port_writel(ctrl, pnr, PORT_CMD_ISSUE, 1);

    u32 end = timer_calc(AHCI_REQUEST_TIMEOUT);
    કરો {
        માટે (;;) {
            intbits = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
            જો (intbits) {
                ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
                જો (intbits & 0x40000000) {
                    u32 tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
                    status = tf & 0xff;
                    error = (tf & 0xff00) >> 8;
                    તોડો;
                }
                જો (intbits & 0x02) {
                    status = GET_LOWFLAT(fis->psfis[2]);
                    error  = GET_LOWFLAT(fis->psfis[3]);
                    તોડો;
                }
                જો (intbits & 0x01) {
                    status = GET_LOWFLAT(fis->rfis[2]);
                    error  = GET_LOWFLAT(fis->rfis[3]);
                    તોડો;
                }
            }
            જો (timer_check(end)) {
                warn_timeout();
                પાછા -1;
            }
            yield();
        }
        dprintf(8, "AHCI/%d: ... intbits 0x%x, status 0x%x ...\n",
                pnr, intbits, status);
    } જ્યારે (status & ATA_CB_STAT_BSY);

    success = (0x00 == (status & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF |
                                  ATA_CB_STAT_ERR)) &&
               ATA_CB_STAT_RDY == (status & (ATA_CB_STAT_RDY)));
    જો (success) {
        dprintf(8, "AHCI/%d: ... finished, status 0x%x, OK\n", pnr,
                status);
    } નહીંતર {
        dprintf(2, "AHCI/%d: ... finished, status 0x%x, ERROR 0x%x\n", pnr,
                status, error);

        // non-queued error recovery (AHCI 1.3 section 6.2.2.1)
        // Clears PxCMD.ST to 0 to reset the PxCI register
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val & ~PORT_CMD_START);

        // waits for PxCMD.CR to clear to 0
        જ્યારે (1) {
            val = ahci_port_readl(ctrl, pnr, PORT_CMD);
            જો ((val & PORT_CMD_LIST_ON) == 0)
                તોડો;
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
        જો (val & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ)) {
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
    પાછા success ? 0 : -1;
}

#define CDROM_CDB_SIZE 12

પૂર્ણાંક ahci_atapi_process_op(સંરચના disk_op_s *op)
{
    જો (! CONFIG_AHCI)
        પાછા 0;

    સંરચના ahci_port_s *port_gf = container_of(
        op->drive_fl, સંરચના ahci_port_s, drive);
    સંરચના ahci_cmd_s *cmd = port_gf->cmd;

    જો (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        પાછા DISK_RET_EWRITEPROTECT;
    પૂર્ણાંક blocksize = scsi_fill_cmd(op, cmd->atapi, CDROM_CDB_SIZE);
    જો (blocksize < 0)
        પાછા default_process_op(op);
    sata_prep_atapi(&cmd->fis, blocksize);
    પૂર્ણાંક rc = ahci_command(port_gf, 0, 1, op->buf_fl, op->ગણતરી * blocksize);
    જો (rc < 0)
        પાછા DISK_RET_EBADTRACK;
    પાછા DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive, op->buf_fl must be word aligned
સ્થિર પૂર્ણાંક
ahci_disk_readwrite_aligned(સંરચના disk_op_s *op, પૂર્ણાંક iswrite)
{
    સંરચના ahci_port_s *port_gf = container_of(
        op->drive_fl, સંરચના ahci_port_s, drive);
    સંરચના ahci_cmd_s *cmd = port_gf->cmd;
    પૂર્ણાંક rc;

    sata_prep_readwrite(&cmd->fis, op, iswrite);
    rc = ahci_command(port_gf, iswrite, 0, op->buf_fl,
                      op->ગણતરી * DISK_SECTOR_SIZE);
    dprintf(8, "ahci disk %s, lba %6x, count %3x, buf %p, rc %d\n",
            iswrite ? "write" : "read", (u32)op->lba, op->ગણતરી, op->buf_fl, rc);
    જો (rc < 0)
        પાછા DISK_RET_EBADTRACK;
    પાછા DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive.
સ્થિર પૂર્ણાંક
ahci_disk_readwrite(સંરચના disk_op_s *op, પૂર્ણાંક iswrite)
{
    // if caller's buffer is word aligned, use it directly
    જો (((u32) op->buf_fl & 1) == 0)
        પાછા ahci_disk_readwrite_aligned(op, iswrite);

    // Use a word aligned buffer for AHCI I/O
    પૂર્ણાંક rc;
    સંરચના disk_op_s localop = *op;
    u8 *alignedbuf_fl = bounce_buf_fl;
    u8 *position = op->buf_fl;

    localop.buf_fl = alignedbuf_fl;
    localop.ગણતરી = 1;

    જો (iswrite) {
        u16 block;
        માટે (block = 0; block < op->ગણતરી; block++) {
            memcpy_fl (alignedbuf_fl, position, DISK_SECTOR_SIZE);
            rc = ahci_disk_readwrite_aligned (&localop, 1);
            જો (rc)
                પાછા rc;
            position += DISK_SECTOR_SIZE;
            localop.lba++;
        }
    } નહીંતર { // read
        u16 block;
        માટે (block = 0; block < op->ગણતરી; block++) {
            rc = ahci_disk_readwrite_aligned (&localop, 0);
            જો (rc)
                પાછા rc;
            memcpy_fl (position, alignedbuf_fl, DISK_SECTOR_SIZE);
            position += DISK_SECTOR_SIZE;
            localop.lba++;
        }
    }
    પાછા DISK_RET_SUCCESS;
}

// command demuxer
પૂર્ણાંક
ahci_process_op(સંરચના disk_op_s *op)
{
    જો (!CONFIG_AHCI)
        પાછા 0;
    પસંદગી (op->command) {
    સ્થિતિ CMD_READ:
        પાછા ahci_disk_readwrite(op, 0);
    સ્થિતિ CMD_WRITE:
        પાછા ahci_disk_readwrite(op, 1);
    મૂળભૂત:
        પાછા default_process_op(op);
    }
}

સ્થિર શૂન્ય
ahci_port_reset(સંરચના ahci_ctrl_s *ctrl, u32 pnr)
{
    u32 val;

    /* disable FIS + CMD */
    u32 end = timer_calc(AHCI_RESET_TIMEOUT);
    માટે (;;) {
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        જો (!(val & (PORT_CMD_FIS_RX | PORT_CMD_START |
                     PORT_CMD_FIS_ON | PORT_CMD_LIST_ON)))
            તોડો;
        val &= ~(PORT_CMD_FIS_RX | PORT_CMD_START);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val);
        જો (timer_check(end)) {
            warn_timeout();
            તોડો;
        }
        yield();
    }

    /* disable + clear IRQs */
    ahci_port_writel(ctrl, pnr, PORT_IRQ_MASK, 0);
    val = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
    જો (val)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, val);
}

સ્થિર સંરચના ahci_port_s*
ahci_port_alloc(સંરચના ahci_ctrl_s *ctrl, u32 pnr)
{
    સંરચના ahci_port_s *port = malloc_tmp(કદ(*port));

    જો (!port) {
        warn_noalloc();
        પાછા ખાલી;
    }
    memset(port, 0, કદ(*port));
    port->pnr = pnr;
    port->ctrl = ctrl;
    port->list = memalign_tmp(1024, 1024);
    port->fis = memalign_tmp(256, 256);
    port->cmd = memalign_tmp(256, 256);
    જો (port->list == ખાલી || port->fis == ખાલી || port->cmd == ખાલી) {
        warn_noalloc();
        પાછા ખાલી;
    }
    memset(port->list, 0, 1024);
    memset(port->fis, 0, 256);
    memset(port->cmd, 0, 256);

    ahci_port_writel(ctrl, pnr, PORT_LST_ADDR, (u32)port->list);
    ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR, (u32)port->fis);
    જો (ctrl->caps & HOST_CAP_64) {
        ahci_port_writel(ctrl, pnr, PORT_LST_ADDR_HI, 0);
        ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR_HI, 0);
    }

    પાછા port;
}

સ્થિર શૂન્ય ahci_port_release(સંરચના ahci_port_s *port)
{
    ahci_port_reset(port->ctrl, port->pnr);
    મુક્ત(port->list);
    મુક્ત(port->fis);
    મુક્ત(port->cmd);
    મુક્ત(port);
}

સ્થિર સંરચના ahci_port_s* ahci_port_realloc(સંરચના ahci_port_s *port)
{
    સંરચના ahci_port_s *tmp;
    u32 cmd;

    tmp = malloc_fseg(કદ(*port));
    જો (!tmp) {
        warn_noalloc();
        ahci_port_release(port);
        પાછા ખાલી;
    }
    *tmp = *port;
    મુક્ત(port);
    port = tmp;

    ahci_port_reset(port->ctrl, port->pnr);

    મુક્ત(port->list);
    મુક્ત(port->fis);
    મુક્ત(port->cmd);
    port->list = memalign_ઉપલું(1024, 1024);
    port->fis = memalign_ઉપલું(256, 256);
    port->cmd = memalign_ઉપલું(256, 256);
    જો (!port->list || !port->fis || !port->cmd) {
        warn_noalloc();
        મુક્ત(port->list);
        મુક્ત(port->fis);
        મુક્ત(port->cmd);
        મુક્ત(port);
        પાછા ખાલી;
    }

    ahci_port_writel(port->ctrl, port->pnr, PORT_LST_ADDR, (u32)port->list);
    ahci_port_writel(port->ctrl, port->pnr, PORT_FIS_ADDR, (u32)port->fis);

    cmd = ahci_port_readl(port->ctrl, port->pnr, PORT_CMD);
    cmd |= (PORT_CMD_FIS_RX|PORT_CMD_START);
    ahci_port_writel(port->ctrl, port->pnr, PORT_CMD, cmd);

    પાછા port;
}

#define MAXMODEL 40

/* See ahci spec chapter 10.1 "Software Initialization of HBA" */
સ્થિર પૂર્ણાંક ahci_port_setup(સંરચના ahci_port_s *port)
{
    સંરચના ahci_ctrl_s *ctrl = port->ctrl;
    u32 pnr = port->pnr;
    અક્ષર model[MAXMODEL+1];
    u16 buffer[256];
    u32 cmd, stat, err, tf;
    પૂર્ણાંક rc;

    /* enable FIS recv */
    cmd = ahci_port_readl(ctrl, pnr, PORT_CMD);
    cmd |= PORT_CMD_FIS_RX;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    /* spin up */
    cmd &= ~PORT_CMD_ICC_MASK;
    cmd |= PORT_CMD_SPIN_UP | PORT_CMD_POWER_ON | PORT_CMD_ICC_ACTIVE;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);
    u32 end = timer_calc(AHCI_LINK_TIMEOUT);
    માટે (;;) {
        stat = ahci_port_readl(ctrl, pnr, PORT_SCR_STAT);
        જો ((stat & 0x07) == 0x03) {
            dprintf(2, "AHCI/%d: link up\n", port->pnr);
            તોડો;
        }
        જો (timer_check(end)) {
            dprintf(2, "AHCI/%d: link down\n", port->pnr);
            પાછા -1;
        }
        yield();
    }

    /* clear error status */
    err = ahci_port_readl(ctrl, pnr, PORT_SCR_ERR);
    જો (err)
        ahci_port_writel(ctrl, pnr, PORT_SCR_ERR, err);

    /* wait for device becoming ready */
    end = timer_calc(AHCI_REQUEST_TIMEOUT);
    માટે (;;) {
        tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
        જો (!(tf & (ATA_CB_STAT_BSY |
                    ATA_CB_STAT_DRQ)))
            તોડો;
        જો (timer_check(end)) {
            warn_timeout();
            dprintf(1, "AHCI/%d: device not ready (tf 0x%x)\n", port->pnr, tf);
            પાછા -1;
        }
        yield();
    }

    /* start device */
    cmd |= PORT_CMD_START;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    rc = ahci_command(port, 0, 0, buffer, કદ(buffer));
    જો (rc == 0) {
        port->atapi = 1;
    } નહીંતર {
        port->atapi = 0;
        sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
        rc = ahci_command(port, 0, 0, buffer, કદ(buffer));
        જો (rc < 0)
            પાછા -1;
    }

    port->drive.cntl_id = pnr;
    port->drive.removable = (buffer[0] & 0x80) ? 1 : 0;

    જો (!port->atapi) {
        // found disk (ata)
        port->drive.type = DTYPE_AHCI;
        port->drive.blksize = DISK_SECTOR_SIZE;
        port->drive.pchs.cylinder = buffer[1];
        port->drive.pchs.head = buffer[3];
        port->drive.pchs.sector = buffer[6];

        u64 sectors;
        જો (buffer[83] & (1 << 10)) // word 83 - lba48 support
            sectors = *(u64*)&buffer[100]; // word 100-103
        નહીંતર
            sectors = *(u32*)&buffer[60]; // word 60 and word 61
        port->drive.sectors = sectors;
        u64 adjsize = sectors >> 11;
        અક્ષર adjprefix = 'M';
        જો (adjsize >= (1 << 16)) {
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
        જો (buffer[53] & 0x04) {
            udma_mode = 6;
            જ્યારે ((udma_mode >= 0) &&
                   !((buffer[88] & 0x7f) & ( 1 << udma_mode ))) {
                udma_mode--;
            }
        }
        // If bit 1 in word 53 is set, multiword-dma and advanced pio modes
        // are available in words 63 and 64.
        જો (buffer[53] & 0x02) {
            pio_mode = 4;
            multi_dma = 3;
            જ્યારે ((multi_dma >= 0) &&
                   !((buffer[63] & 0x7) & ( 1 << multi_dma ))) {
                multi_dma--;
            }
            જ્યારે ((pio_mode >= 3) &&
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
        જો (udma_mode >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to UDMA-%d\n",
                    port->pnr, udma_mode);
            port->cmd->fis.sector_ગણતરી = ATA_TRANSFER_MODE_ULTRA_DMA
                                          | udma_mode;
        } નહીંતર જો (multi_dma >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to Multi-DMA-%d\n",
                    port->pnr, multi_dma);
            port->cmd->fis.sector_ગણતરી = ATA_TRANSFER_MODE_MULTIWORD_DMA
                                          | multi_dma;
        } નહીંતર જો (pio_mode >= 3) {
            dprintf(1, "AHCI/%d: Set transfer mode to PIO-%d\n",
                    port->pnr, pio_mode);
            port->cmd->fis.sector_ગણતરી = ATA_TRANSFER_MODE_PIO_FLOW_CTRL
                                          | pio_mode;
        } નહીંતર {
            dprintf(1, "AHCI/%d: Set transfer mode to default PIO\n",
                    port->pnr);
            port->cmd->fis.sector_ગણતરી = ATA_TRANSFER_MODE_DEFAULT_PIO;
        }
        rc = ahci_command(port, 1, 0, 0, 0);
        જો (rc < 0) {
            dprintf(1, "AHCI/%d: Set transfer mode failed.\n", port->pnr);
        }
    } નહીંતર {
        // found cdrom (atapi)
        port->drive.type = DTYPE_AHCI_ATAPI;
        port->drive.blksize = CDROM_SECTOR_SIZE;
        port->drive.sectors = (u64)-1;
        u8 iscd = ((buffer[0] >> 8) & 0x1f) == 0x05;
        જો (!iscd) {
            dprintf(1, "AHCI/%d: atapi device isn't a cdrom\n", port->pnr);
            પાછા -1;
        }
        port->desc = znprintf(MAXDESCSIZE
                              , "DVD/CD [AHCI/%d: %s ATAPI-%d DVD/CD]"
                              , port->pnr
                              , ata_extract_model(model, MAXMODEL, buffer)
                              , ata_extract_version(buffer));
        port->prio = bootprio_find_ata_device(ctrl->pci_tmp, pnr, 0);
    }
    boot_lchs_find_ata_device(ctrl->pci_tmp, pnr, 0, &(port->drive.lchs));
    પાછા 0;
}

// Detect any drives attached to a given controller.
સ્થિર શૂન્ય
ahci_port_detect(શૂન્ય *data)
{
    સંરચના ahci_port_s *port = data;
    પૂર્ણાંક rc;

    dprintf(2, "AHCI/%d: probing\n", port->pnr);
    ahci_port_reset(port->ctrl, port->pnr);
    rc = ahci_port_setup(port);
    જો (rc < 0)
        ahci_port_release(port);
    નહીંતર {
        port = ahci_port_realloc(port);
        જો (port == ખાલી)
            પાછા;
        dprintf(1, "AHCI/%d: registering: \"%s\"\n", port->pnr, port->desc);
        જો (!port->atapi) {
            // Register with bcv system.
            boot_add_hd(&port->drive, port->desc, port->prio);
        } નહીંતર {
            // fill cdidmap
            boot_add_cd(&port->drive, port->desc, port->prio);
        }
    }
}

// Initialize an ata controller and detect its drives.
સ્થિર શૂન્ય
ahci_controller_setup(સંરચના pci_device *pci)
{
    સંરચના ahci_port_s *port;
    u32 pnr, max;

    જો (create_bounce_buf() < 0)
        પાછા;

    શૂન્ય *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_5);
    જો (!iobase)
        પાછા;

    સંરચના ahci_ctrl_s *ctrl = malloc_fseg(કદ(*ctrl));
    જો (!ctrl) {
        warn_noalloc();
        પાછા;
    }

    ctrl->pci_tmp = pci;
    ctrl->iobase = iobase;
    ctrl->irq = pci_config_readb(pci->bdf, PCI_INTERRUPT_લીટી);
    dprintf(1, "AHCI controller at %pP, iobase %p, irq %d\n"
            , pci, ctrl->iobase, ctrl->irq);

    pci_enable_busmaster(pci);

    u32 val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_RESET);
    u32 end = timer_calc(AHCI_RESET_TIMEOUT);
    માટે (;;) {
        val = ahci_ctrl_readl(ctrl, HOST_CTL);
        જો (!(val & HOST_CTL_RESET))
            તોડો;
        જો (timer_check(end)) {
            warn_timeout();
            dprintf(1, "AHCI: controller reset failed\n");
            મુક્ત(ctrl);
            પાછા;
        }
        yield();
    }
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_AHCI_EN);
    (શૂન્ય)ahci_ctrl_readl(ctrl, HOST_CTL); /* Flush */

    ctrl->caps = ahci_ctrl_readl(ctrl, HOST_CAP);
    ctrl->ports = ahci_ctrl_readl(ctrl, HOST_PORTS_IMPL);
    dprintf(2, "AHCI: cap 0x%x, ports_impl 0x%x\n",
            ctrl->caps, ctrl->ports);

    max = 0x1f;
    માટે (pnr = 0; pnr <= max; pnr++) {
        જો (!(ctrl->ports & (1 << pnr)))
            ચાલુ;
        port = ahci_port_alloc(ctrl, pnr);
        જો (port == ખાલી)
            ચાલુ;
        run_thread(ahci_port_detect, port);
    }
}

// Locate and init ahci controllers.
સ્થિર શૂન્ય
ahci_scan(શૂન્ય)
{
    // Scan PCI bus for ATA adapters
    સંરચના pci_device *pci;
    foreachpci(pci) {
        જો (pci->class != PCI_CLASS_STORAGE_SATA)
            ચાલુ;
        જો (pci->prog_if != 1 /* AHCI rev 1 */)
            ચાલુ;
        ahci_controller_setup(pci);
    }
}

શૂન્ય
ahci_setup(શૂન્ય)
{
    ASSERT32FLAT();
    જો (!CONFIG_AHCI)
        પાછા;

    dprintf(3, "init ahci\n");
    ahci_scan();
}

// Low level AHCI disk access
//
// Copyright (C) 2010 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "ahci.h" // CDB_CMD_READ_10
#समावेश "ata.h" // ATA_CB_STAT
#समावेश "biosvar.h" // GET_GLOBAL
#समावेश "blockcmd.h" // CDB_CMD_READ_10
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "pci.h" // pci_config_readb
#समावेश "pcidevice.h" // foreachpci
#समावेश "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#समावेश "pci_regs.h" // PCI_INTERRUPT_LINE
#समावेश "stacks.h" // yield
#समावेश "std/disk.h" // DISK_RET_SUCCESS
#समावेश "string.h" // memset
#समावेश "util.h" // timer_calc
#समावेश "x86.h" // inb

#घोषणा AHCI_REQUEST_TIMEOUT 32000 // 32 seconds max for IDE ops
#घोषणा AHCI_RESET_TIMEOUT     500 // 500 miliseconds
#घोषणा AHCI_LINK_TIMEOUT       10 // 10 miliseconds

// prepare sata command fis
अटल व्योम sata_prep_simple(काष्ठा sata_cmd_fis *fis, u8 command)
{
    memset_fl(fis, 0, माप(*fis));
    fis->command = command;
}

अटल व्योम sata_prep_readwrite(काष्ठा sata_cmd_fis *fis,
                                काष्ठा disk_op_s *op, पूर्णांक iswrite)
{
    u64 lba = op->lba;
    u8 command;

    memset_fl(fis, 0, माप(*fis));

    अगर (op->गिनती >= (1<<8) || lba + op->गिनती >= (1<<28)) {
        fis->sector_count2 = op->गिनती >> 8;
        fis->lba_low2      = lba >> 24;
        fis->lba_mid2      = lba >> 32;
        fis->lba_high2     = lba >> 40;
        lba &= 0xffffff;
        command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                   : ATA_CMD_READ_DMA_EXT);
    } अन्यथा {
        command = (iswrite ? ATA_CMD_WRITE_DMA
                   : ATA_CMD_READ_DMA);
    }
    fis->feature      = 1; /* dma */
    fis->command      = command;
    fis->sector_गिनती = op->गिनती;
    fis->lba_निचला      = lba;
    fis->lba_mid      = lba >> 8;
    fis->lba_ऊपरी     = lba >> 16;
    fis->device       = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;
}

अटल व्योम sata_prep_atapi(काष्ठा sata_cmd_fis *fis, u16 blocksize)
{
    memset_fl(fis, 0, माप(*fis));
    fis->command  = ATA_CMD_PACKET;
    fis->feature  = 1; /* dma */
    fis->lba_mid  = blocksize;
    fis->lba_ऊपरी = blocksize >> 8;
}

// ahci register access helpers
अटल u32 ahci_ctrl_readl(काष्ठा ahci_ctrl_s *ctrl, u32 reg)
{
    वापस readl(ctrl->iobase + reg);
}

अटल व्योम ahci_ctrl_writel(काष्ठा ahci_ctrl_s *ctrl, u32 reg, u32 val)
{
    writel(ctrl->iobase + reg, val);
}

अटल u32 ahci_port_to_ctrl(u32 pnr, u32 port_reg)
{
    u32 ctrl_reg = 0x100;
    ctrl_reg += pnr * 0x80;
    ctrl_reg += port_reg;
    वापस ctrl_reg;
}

अटल u32 ahci_port_readl(काष्ठा ahci_ctrl_s *ctrl, u32 pnr, u32 reg)
{
    u32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    वापस ahci_ctrl_readl(ctrl, ctrl_reg);
}

अटल व्योम ahci_port_writel(काष्ठा ahci_ctrl_s *ctrl, u32 pnr, u32 reg, u32 val)
{
    u32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    ahci_ctrl_writel(ctrl, ctrl_reg, val);
}

// submit ahci command + wait for result
अटल पूर्णांक ahci_command(काष्ठा ahci_port_s *port_gf, पूर्णांक iswrite, पूर्णांक isatapi,
                        व्योम *बफर, u32 bsize)
{
    u32 val, status, success, flags, intbits, error;
    काष्ठा ahci_ctrl_s *ctrl = port_gf->ctrl;
    काष्ठा ahci_cmd_s  *cmd  = port_gf->cmd;
    काष्ठा ahci_fis_s  *fis  = port_gf->fis;
    काष्ठा ahci_list_s *list = port_gf->list;
    u32 pnr                  = port_gf->pnr;

    cmd->fis.reg       = 0x27;
    cmd->fis.pmp_type  = 1 << 7; /* cmd fis */
    cmd->prdt[0].base  = (u32)बफर;
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
    अगर (intbits)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
    ahci_port_writel(ctrl, pnr, PORT_CMD_ISSUE, 1);

    u32 end = timer_calc(AHCI_REQUEST_TIMEOUT);
    करो {
        क्रम (;;) {
            intbits = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
            अगर (intbits) {
                ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
                अगर (intbits & 0x40000000) {
                    u32 tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
                    status = tf & 0xff;
                    error = (tf & 0xff00) >> 8;
                    अवरोध;
                }
                अगर (intbits & 0x02) {
                    status = GET_LOWFLAT(fis->psfis[2]);
                    error  = GET_LOWFLAT(fis->psfis[3]);
                    अवरोध;
                }
                अगर (intbits & 0x01) {
                    status = GET_LOWFLAT(fis->rfis[2]);
                    error  = GET_LOWFLAT(fis->rfis[3]);
                    अवरोध;
                }
            }
            अगर (timer_check(end)) {
                warn_timeout();
                वापस -1;
            }
            yield();
        }
        dprintf(8, "AHCI/%d: ... intbits 0x%x, status 0x%x ...\n",
                pnr, intbits, status);
    } जबतक (status & ATA_CB_STAT_BSY);

    success = (0x00 == (status & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF |
                                  ATA_CB_STAT_ERR)) &&
               ATA_CB_STAT_RDY == (status & (ATA_CB_STAT_RDY)));
    अगर (success) {
        dprintf(8, "AHCI/%d: ... finished, status 0x%x, OK\n", pnr,
                status);
    } अन्यथा {
        dprintf(2, "AHCI/%d: ... finished, status 0x%x, ERROR 0x%x\n", pnr,
                status, error);

        // non-queued error recovery (AHCI 1.3 section 6.2.2.1)
        // Clears PxCMD.ST to 0 to reset the PxCI register
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val & ~PORT_CMD_START);

        // waits for PxCMD.CR to clear to 0
        जबतक (1) {
            val = ahci_port_readl(ctrl, pnr, PORT_CMD);
            अगर ((val & PORT_CMD_LIST_ON) == 0)
                अवरोध;
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
        अगर (val & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ)) {
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
    वापस success ? 0 : -1;
}

#घोषणा CDROM_CDB_आकार 12

पूर्णांक ahci_atapi_process_op(काष्ठा disk_op_s *op)
{
    अगर (! CONFIG_AHCI)
        वापस 0;

    काष्ठा ahci_port_s *port_gf = container_of(
        op->drive_fl, काष्ठा ahci_port_s, drive);
    काष्ठा ahci_cmd_s *cmd = port_gf->cmd;

    अगर (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        वापस DISK_RET_EWRITEPROTECT;
    पूर्णांक blocksize = scsi_fill_cmd(op, cmd->atapi, CDROM_CDB_आकार);
    अगर (blocksize < 0)
        वापस default_process_op(op);
    sata_prep_atapi(&cmd->fis, blocksize);
    पूर्णांक rc = ahci_command(port_gf, 0, 1, op->buf_fl, op->गिनती * blocksize);
    अगर (rc < 0)
        वापस DISK_RET_EBADTRACK;
    वापस DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive, op->buf_fl must be word aligned
अटल पूर्णांक
ahci_disk_readwrite_aligned(काष्ठा disk_op_s *op, पूर्णांक iswrite)
{
    काष्ठा ahci_port_s *port_gf = container_of(
        op->drive_fl, काष्ठा ahci_port_s, drive);
    काष्ठा ahci_cmd_s *cmd = port_gf->cmd;
    पूर्णांक rc;

    sata_prep_readwrite(&cmd->fis, op, iswrite);
    rc = ahci_command(port_gf, iswrite, 0, op->buf_fl,
                      op->गिनती * DISK_SECTOR_आकार);
    dprintf(8, "ahci disk %s, lba %6x, count %3x, buf %p, rc %d\n",
            iswrite ? "write" : "read", (u32)op->lba, op->गिनती, op->buf_fl, rc);
    अगर (rc < 0)
        वापस DISK_RET_EBADTRACK;
    वापस DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive.
अटल पूर्णांक
ahci_disk_readwrite(काष्ठा disk_op_s *op, पूर्णांक iswrite)
{
    // if caller's buffer is word aligned, use it directly
    अगर (((u32) op->buf_fl & 1) == 0)
        वापस ahci_disk_readwrite_aligned(op, iswrite);

    // Use a word aligned buffer for AHCI I/O
    पूर्णांक rc;
    काष्ठा disk_op_s localop = *op;
    u8 *alignedbuf_fl = bounce_buf_fl;
    u8 *position = op->buf_fl;

    localop.buf_fl = alignedbuf_fl;
    localop.गिनती = 1;

    अगर (iswrite) {
        u16 block;
        क्रम (block = 0; block < op->गिनती; block++) {
            memcpy_fl (alignedbuf_fl, position, DISK_SECTOR_आकार);
            rc = ahci_disk_readwrite_aligned (&localop, 1);
            अगर (rc)
                वापस rc;
            position += DISK_SECTOR_आकार;
            localop.lba++;
        }
    } अन्यथा { // read
        u16 block;
        क्रम (block = 0; block < op->गिनती; block++) {
            rc = ahci_disk_readwrite_aligned (&localop, 0);
            अगर (rc)
                वापस rc;
            memcpy_fl (position, alignedbuf_fl, DISK_SECTOR_आकार);
            position += DISK_SECTOR_आकार;
            localop.lba++;
        }
    }
    वापस DISK_RET_SUCCESS;
}

// command demuxer
पूर्णांक
ahci_process_op(काष्ठा disk_op_s *op)
{
    अगर (!CONFIG_AHCI)
        वापस 0;
    चयन (op->command) {
    हाल CMD_READ:
        वापस ahci_disk_readwrite(op, 0);
    हाल CMD_WRITE:
        वापस ahci_disk_readwrite(op, 1);
    शेष:
        वापस default_process_op(op);
    }
}

अटल व्योम
ahci_port_reset(काष्ठा ahci_ctrl_s *ctrl, u32 pnr)
{
    u32 val;

    /* disable FIS + CMD */
    u32 end = timer_calc(AHCI_RESET_TIMEOUT);
    क्रम (;;) {
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        अगर (!(val & (PORT_CMD_FIS_RX | PORT_CMD_START |
                     PORT_CMD_FIS_ON | PORT_CMD_LIST_ON)))
            अवरोध;
        val &= ~(PORT_CMD_FIS_RX | PORT_CMD_START);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val);
        अगर (timer_check(end)) {
            warn_timeout();
            अवरोध;
        }
        yield();
    }

    /* disable + clear IRQs */
    ahci_port_writel(ctrl, pnr, PORT_IRQ_MASK, 0);
    val = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
    अगर (val)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, val);
}

अटल काष्ठा ahci_port_s*
ahci_port_alloc(काष्ठा ahci_ctrl_s *ctrl, u32 pnr)
{
    काष्ठा ahci_port_s *port = malloc_tmp(माप(*port));

    अगर (!port) {
        warn_noalloc();
        वापस शून्य;
    }
    स_रखो(port, 0, माप(*port));
    port->pnr = pnr;
    port->ctrl = ctrl;
    port->list = memalign_tmp(1024, 1024);
    port->fis = memalign_tmp(256, 256);
    port->cmd = memalign_tmp(256, 256);
    अगर (port->list == शून्य || port->fis == शून्य || port->cmd == शून्य) {
        warn_noalloc();
        वापस शून्य;
    }
    स_रखो(port->list, 0, 1024);
    स_रखो(port->fis, 0, 256);
    स_रखो(port->cmd, 0, 256);

    ahci_port_writel(ctrl, pnr, PORT_LST_ADDR, (u32)port->list);
    ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR, (u32)port->fis);
    अगर (ctrl->caps & HOST_CAP_64) {
        ahci_port_writel(ctrl, pnr, PORT_LST_ADDR_HI, 0);
        ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR_HI, 0);
    }

    वापस port;
}

अटल व्योम ahci_port_release(काष्ठा ahci_port_s *port)
{
    ahci_port_reset(port->ctrl, port->pnr);
    मुक्त(port->list);
    मुक्त(port->fis);
    मुक्त(port->cmd);
    मुक्त(port);
}

अटल काष्ठा ahci_port_s* ahci_port_realloc(काष्ठा ahci_port_s *port)
{
    काष्ठा ahci_port_s *tmp;
    u32 cmd;

    tmp = malloc_fseg(माप(*port));
    अगर (!tmp) {
        warn_noalloc();
        ahci_port_release(port);
        वापस शून्य;
    }
    *tmp = *port;
    मुक्त(port);
    port = tmp;

    ahci_port_reset(port->ctrl, port->pnr);

    मुक्त(port->list);
    मुक्त(port->fis);
    मुक्त(port->cmd);
    port->list = memalign_ऊपरी(1024, 1024);
    port->fis = memalign_ऊपरी(256, 256);
    port->cmd = memalign_ऊपरी(256, 256);
    अगर (!port->list || !port->fis || !port->cmd) {
        warn_noalloc();
        मुक्त(port->list);
        मुक्त(port->fis);
        मुक्त(port->cmd);
        मुक्त(port);
        वापस शून्य;
    }

    ahci_port_writel(port->ctrl, port->pnr, PORT_LST_ADDR, (u32)port->list);
    ahci_port_writel(port->ctrl, port->pnr, PORT_FIS_ADDR, (u32)port->fis);

    cmd = ahci_port_readl(port->ctrl, port->pnr, PORT_CMD);
    cmd |= (PORT_CMD_FIS_RX|PORT_CMD_START);
    ahci_port_writel(port->ctrl, port->pnr, PORT_CMD, cmd);

    वापस port;
}

#घोषणा MAXMODEL 40

/* See ahci spec chapter 10.1 "Software Initialization of HBA" */
अटल पूर्णांक ahci_port_setup(काष्ठा ahci_port_s *port)
{
    काष्ठा ahci_ctrl_s *ctrl = port->ctrl;
    u32 pnr = port->pnr;
    अक्षर model[MAXMODEL+1];
    u16 बफर[256];
    u32 cmd, stat, err, tf;
    पूर्णांक rc;

    /* enable FIS recv */
    cmd = ahci_port_readl(ctrl, pnr, PORT_CMD);
    cmd |= PORT_CMD_FIS_RX;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    /* spin up */
    cmd &= ~PORT_CMD_ICC_MASK;
    cmd |= PORT_CMD_SPIN_UP | PORT_CMD_POWER_ON | PORT_CMD_ICC_ACTIVE;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);
    u32 end = timer_calc(AHCI_LINK_TIMEOUT);
    क्रम (;;) {
        stat = ahci_port_readl(ctrl, pnr, PORT_SCR_STAT);
        अगर ((stat & 0x07) == 0x03) {
            dprintf(2, "AHCI/%d: link up\n", port->pnr);
            अवरोध;
        }
        अगर (timer_check(end)) {
            dprintf(2, "AHCI/%d: link down\n", port->pnr);
            वापस -1;
        }
        yield();
    }

    /* clear error status */
    err = ahci_port_readl(ctrl, pnr, PORT_SCR_ERR);
    अगर (err)
        ahci_port_writel(ctrl, pnr, PORT_SCR_ERR, err);

    /* wait for device becoming ready */
    end = timer_calc(AHCI_REQUEST_TIMEOUT);
    क्रम (;;) {
        tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
        अगर (!(tf & (ATA_CB_STAT_BSY |
                    ATA_CB_STAT_DRQ)))
            अवरोध;
        अगर (timer_check(end)) {
            warn_timeout();
            dprintf(1, "AHCI/%d: device not ready (tf 0x%x)\n", port->pnr, tf);
            वापस -1;
        }
        yield();
    }

    /* start device */
    cmd |= PORT_CMD_START;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    rc = ahci_command(port, 0, 0, बफर, माप(बफर));
    अगर (rc == 0) {
        port->atapi = 1;
    } अन्यथा {
        port->atapi = 0;
        sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
        rc = ahci_command(port, 0, 0, बफर, माप(बफर));
        अगर (rc < 0)
            वापस -1;
    }

    port->drive.cntl_id = pnr;
    port->drive.removable = (बफर[0] & 0x80) ? 1 : 0;

    अगर (!port->atapi) {
        // found disk (ata)
        port->drive.type = DTYPE_AHCI;
        port->drive.blksize = DISK_SECTOR_आकार;
        port->drive.pchs.cylinder = बफर[1];
        port->drive.pchs.शीर्ष = बफर[3];
        port->drive.pchs.sector = बफर[6];

        u64 sectors;
        अगर (बफर[83] & (1 << 10)) // word 83 - lba48 support
            sectors = *(u64*)&बफर[100]; // word 100-103
        अन्यथा
            sectors = *(u32*)&बफर[60]; // word 60 and word 61
        port->drive.sectors = sectors;
        u64 adjsize = sectors >> 11;
        अक्षर adjprefix = 'M';
        अगर (adjsize >= (1 << 16)) {
            adjsize >>= 10;
            adjprefix = 'G';
        }
        port->desc = znprintf(MAXDESCSIZE
                              , "AHCI/%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                              , port->pnr
                              , ata_extract_model(model, MAXMODEL, बफर)
                              , ata_extract_version(बफर)
                              , (u32)adjsize, adjprefix);
        port->prio = bootprio_find_ata_device(ctrl->pci_tmp, pnr, 0);

        s8 multi_dma = -1;
        s8 pio_mode = -1;
        s8 udma_mode = -1;
        // If bit 2 in word 53 is set, udma information is valid in word 88.
        अगर (बफर[53] & 0x04) {
            udma_mode = 6;
            जबतक ((udma_mode >= 0) &&
                   !((बफर[88] & 0x7f) & ( 1 << udma_mode ))) {
                udma_mode--;
            }
        }
        // If bit 1 in word 53 is set, multiword-dma and advanced pio modes
        // are available in words 63 and 64.
        अगर (बफर[53] & 0x02) {
            pio_mode = 4;
            multi_dma = 3;
            जबतक ((multi_dma >= 0) &&
                   !((बफर[63] & 0x7) & ( 1 << multi_dma ))) {
                multi_dma--;
            }
            जबतक ((pio_mode >= 3) &&
                   !((बफर[64] & 0x3) & ( 1 << ( pio_mode - 3 ) ))) {
                pio_mode--;
            }
        }
        dprintf(2, "AHCI/%d: supported modes: udma %d, multi-dma %d, pio %d\n",
                port->pnr, udma_mode, multi_dma, pio_mode);

        sata_prep_simple(&port->cmd->fis, ATA_CMD_SET_FEATURES);
        port->cmd->fis.feature = ATA_SET_FEATRUE_TRANSFER_MODE;
        // Select used mode. UDMA first, then Multi-DMA followed by
        // advanced PIO modes 3 or 4. If non, set default PIO.
        अगर (udma_mode >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to UDMA-%d\n",
                    port->pnr, udma_mode);
            port->cmd->fis.sector_गिनती = ATA_TRANSFER_MODE_ULTRA_DMA
                                          | udma_mode;
        } अन्यथा अगर (multi_dma >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to Multi-DMA-%d\n",
                    port->pnr, multi_dma);
            port->cmd->fis.sector_गिनती = ATA_TRANSFER_MODE_MULTIWORD_DMA
                                          | multi_dma;
        } अन्यथा अगर (pio_mode >= 3) {
            dprintf(1, "AHCI/%d: Set transfer mode to PIO-%d\n",
                    port->pnr, pio_mode);
            port->cmd->fis.sector_गिनती = ATA_TRANSFER_MODE_PIO_FLOW_CTRL
                                          | pio_mode;
        } अन्यथा {
            dprintf(1, "AHCI/%d: Set transfer mode to default PIO\n",
                    port->pnr);
            port->cmd->fis.sector_गिनती = ATA_TRANSFER_MODE_DEFAULT_PIO;
        }
        rc = ahci_command(port, 1, 0, 0, 0);
        अगर (rc < 0) {
            dprintf(1, "AHCI/%d: Set transfer mode failed.\n", port->pnr);
        }
    } अन्यथा {
        // found cdrom (atapi)
        port->drive.type = DTYPE_AHCI_ATAPI;
        port->drive.blksize = CDROM_SECTOR_आकार;
        port->drive.sectors = (u64)-1;
        u8 iscd = ((बफर[0] >> 8) & 0x1f) == 0x05;
        अगर (!iscd) {
            dprintf(1, "AHCI/%d: atapi device isn't a cdrom\n", port->pnr);
            वापस -1;
        }
        port->desc = znprintf(MAXDESCSIZE
                              , "DVD/CD [AHCI/%d: %s ATAPI-%d DVD/CD]"
                              , port->pnr
                              , ata_extract_model(model, MAXMODEL, बफर)
                              , ata_extract_version(बफर));
        port->prio = bootprio_find_ata_device(ctrl->pci_tmp, pnr, 0);
    }
    boot_lchs_find_ata_device(ctrl->pci_tmp, pnr, 0, &(port->drive.lchs));
    वापस 0;
}

// Detect any drives attached to a given controller.
अटल व्योम
ahci_port_detect(व्योम *तथ्य)
{
    काष्ठा ahci_port_s *port = तथ्य;
    पूर्णांक rc;

    dprintf(2, "AHCI/%d: probing\n", port->pnr);
    ahci_port_reset(port->ctrl, port->pnr);
    rc = ahci_port_setup(port);
    अगर (rc < 0)
        ahci_port_release(port);
    अन्यथा {
        port = ahci_port_realloc(port);
        अगर (port == शून्य)
            वापस;
        dprintf(1, "AHCI/%d: registering: \"%s\"\n", port->pnr, port->desc);
        अगर (!port->atapi) {
            // Register with bcv system.
            boot_add_hd(&port->drive, port->desc, port->prio);
        } अन्यथा {
            // fill cdidmap
            boot_add_cd(&port->drive, port->desc, port->prio);
        }
    }
}

// Initialize an ata controller and detect its drives.
अटल व्योम
ahci_controller_setup(काष्ठा pci_device *pci)
{
    काष्ठा ahci_port_s *port;
    u32 pnr, max;

    अगर (create_bounce_buf() < 0)
        वापस;

    व्योम *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_5);
    अगर (!iobase)
        वापस;

    काष्ठा ahci_ctrl_s *ctrl = malloc_fseg(माप(*ctrl));
    अगर (!ctrl) {
        warn_noalloc();
        वापस;
    }

    ctrl->pci_tmp = pci;
    ctrl->iobase = iobase;
    ctrl->irq = pci_config_readb(pci->bdf, PCI_INTERRUPT_पंक्ति);
    dprintf(1, "AHCI controller at %pP, iobase %p, irq %d\n"
            , pci, ctrl->iobase, ctrl->irq);

    pci_enable_busmaster(pci);

    u32 val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_RESET);
    u32 end = timer_calc(AHCI_RESET_TIMEOUT);
    क्रम (;;) {
        val = ahci_ctrl_readl(ctrl, HOST_CTL);
        अगर (!(val & HOST_CTL_RESET))
            अवरोध;
        अगर (timer_check(end)) {
            warn_timeout();
            dprintf(1, "AHCI: controller reset failed\n");
            मुक्त(ctrl);
            वापस;
        }
        yield();
    }
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_AHCI_EN);
    (व्योम)ahci_ctrl_readl(ctrl, HOST_CTL); /* Flush */

    ctrl->caps = ahci_ctrl_readl(ctrl, HOST_CAP);
    ctrl->ports = ahci_ctrl_readl(ctrl, HOST_PORTS_IMPL);
    dprintf(2, "AHCI: cap 0x%x, ports_impl 0x%x\n",
            ctrl->caps, ctrl->ports);

    max = 0x1f;
    क्रम (pnr = 0; pnr <= max; pnr++) {
        अगर (!(ctrl->ports & (1 << pnr)))
            जारी;
        port = ahci_port_alloc(ctrl, pnr);
        अगर (port == शून्य)
            जारी;
        run_thread(ahci_port_detect, port);
    }
}

// Locate and init ahci controllers.
अटल व्योम
ahci_scan(व्योम)
{
    // Scan PCI bus for ATA adapters
    काष्ठा pci_device *pci;
    foreachpci(pci) {
        अगर (pci->class != PCI_CLASS_STORAGE_SATA)
            जारी;
        अगर (pci->prog_if != 1 /* AHCI rev 1 */)
            जारी;
        ahci_controller_setup(pci);
    }
}

व्योम
ahci_setup(व्योम)
{
    ASSERT32FLAT();
    अगर (!CONFIG_AHCI)
        वापस;

    dprintf(3, "init ahci\n");
    ahci_scan();
}

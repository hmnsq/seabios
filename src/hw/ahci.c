// Low level AHCI disk access
//
// Copyright (C) 2010 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "ahci.h" // CDB_CMD_READ_10
#अन्तर्भाव "ata.h" // ATA_CB_STAT
#अन्तर्भाव "biosvar.h" // GET_GLOBAL
#अन्तर्भाव "blockcmd.h" // CDB_CMD_READ_10
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "pci.h" // pci_config_readb
#अन्तर्भाव "pcidevice.h" // foreachpci
#अन्तर्भाव "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#अन्तर्भाव "pci_regs.h" // PCI_INTERRUPT_LINE
#अन्तर्भाव "stacks.h" // yield
#अन्तर्भाव "std/disk.h" // DISK_RET_SUCCESS
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // timer_calc
#अन्तर्भाव "x86.h" // inb

#define AHCI_REQUEST_TIMEOUT 32000 // 32 seconds max for IDE ops
#define AHCI_RESET_TIMEOUT     500 // 500 miliseconds
#define AHCI_LINK_TIMEOUT       10 // 10 miliseconds

// prepare sata command fis
स्थिर शून्य sata_prep_simple(संरचना sata_cmd_fis *fis, u8 command)
{
    memset_fl(fis, 0, परिमाण(*fis));
    fis->command = command;
}

स्थिर शून्य sata_prep_readwrite(संरचना sata_cmd_fis *fis,
                                संरचना disk_op_s *op, पूर्णाङ्क iswrite)
{
    u64 lba = op->lba;
    u8 command;

    memset_fl(fis, 0, परिमाण(*fis));

    यदि (op->गन्ती >= (1<<8) || lba + op->गन्ती >= (1<<28)) {
        fis->sector_count2 = op->गन्ती >> 8;
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
    fis->sector_गन्ती = op->गन्ती;
    fis->lba_तल्लो      = lba;
    fis->lba_mid      = lba >> 8;
    fis->lba_माथिल्लो     = lba >> 16;
    fis->device       = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;
}

स्थिर शून्य sata_prep_atapi(संरचना sata_cmd_fis *fis, u16 blocksize)
{
    memset_fl(fis, 0, परिमाण(*fis));
    fis->command  = ATA_CMD_PACKET;
    fis->feature  = 1; /* dma */
    fis->lba_mid  = blocksize;
    fis->lba_माथिल्लो = blocksize >> 8;
}

// ahci register access helpers
स्थिर u32 ahci_ctrl_readl(संरचना ahci_ctrl_s *ctrl, u32 reg)
{
    फर्काऊ readl(ctrl->iobase + reg);
}

स्थिर शून्य ahci_ctrl_writel(संरचना ahci_ctrl_s *ctrl, u32 reg, u32 val)
{
    writel(ctrl->iobase + reg, val);
}

स्थिर u32 ahci_port_to_ctrl(u32 pnr, u32 port_reg)
{
    u32 ctrl_reg = 0x100;
    ctrl_reg += pnr * 0x80;
    ctrl_reg += port_reg;
    फर्काऊ ctrl_reg;
}

स्थिर u32 ahci_port_readl(संरचना ahci_ctrl_s *ctrl, u32 pnr, u32 reg)
{
    u32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    फर्काऊ ahci_ctrl_readl(ctrl, ctrl_reg);
}

स्थिर शून्य ahci_port_writel(संरचना ahci_ctrl_s *ctrl, u32 pnr, u32 reg, u32 val)
{
    u32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    ahci_ctrl_writel(ctrl, ctrl_reg, val);
}

// submit ahci command + wait for result
स्थिर पूर्णाङ्क ahci_command(संरचना ahci_port_s *port_gf, पूर्णाङ्क iswrite, पूर्णाङ्क isatapi,
                        शून्य *buffer, u32 bsize)
{
    u32 val, status, success, flags, intbits, error;
    संरचना ahci_ctrl_s *ctrl = port_gf->ctrl;
    संरचना ahci_cmd_s  *cmd  = port_gf->cmd;
    संरचना ahci_fis_s  *fis  = port_gf->fis;
    संरचना ahci_list_s *list = port_gf->list;
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
    यदि (intbits)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
    ahci_port_writel(ctrl, pnr, PORT_CMD_ISSUE, 1);

    u32 end = timer_calc(AHCI_REQUEST_TIMEOUT);
    गर {
        लागि (;;) {
            intbits = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
            यदि (intbits) {
                ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
                यदि (intbits & 0x40000000) {
                    u32 tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
                    status = tf & 0xff;
                    error = (tf & 0xff00) >> 8;
                    रोक;
                }
                यदि (intbits & 0x02) {
                    status = GET_LOWFLAT(fis->psfis[2]);
                    error  = GET_LOWFLAT(fis->psfis[3]);
                    रोक;
                }
                यदि (intbits & 0x01) {
                    status = GET_LOWFLAT(fis->rfis[2]);
                    error  = GET_LOWFLAT(fis->rfis[3]);
                    रोक;
                }
            }
            यदि (timer_check(end)) {
                warn_timeout();
                फर्काऊ -1;
            }
            yield();
        }
        dprintf(8, "AHCI/%d: ... intbits 0x%x, status 0x%x ...\n",
                pnr, intbits, status);
    } जबसम्म (status & ATA_CB_STAT_BSY);

    success = (0x00 == (status & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF |
                                  ATA_CB_STAT_ERR)) &&
               ATA_CB_STAT_RDY == (status & (ATA_CB_STAT_RDY)));
    यदि (success) {
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
        जबसम्म (1) {
            val = ahci_port_readl(ctrl, pnr, PORT_CMD);
            यदि ((val & PORT_CMD_LIST_ON) == 0)
                रोक;
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
        यदि (val & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ)) {
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
    फर्काऊ success ? 0 : -1;
}

#define CDROM_CDB_SIZE 12

पूर्णाङ्क ahci_atapi_process_op(संरचना disk_op_s *op)
{
    यदि (! CONFIG_AHCI)
        फर्काऊ 0;

    संरचना ahci_port_s *port_gf = container_of(
        op->drive_fl, संरचना ahci_port_s, drive);
    संरचना ahci_cmd_s *cmd = port_gf->cmd;

    यदि (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        फर्काऊ DISK_RET_EWRITEPROTECT;
    पूर्णाङ्क blocksize = scsi_fill_cmd(op, cmd->atapi, CDROM_CDB_SIZE);
    यदि (blocksize < 0)
        फर्काऊ default_process_op(op);
    sata_prep_atapi(&cmd->fis, blocksize);
    पूर्णाङ्क rc = ahci_command(port_gf, 0, 1, op->buf_fl, op->गन्ती * blocksize);
    यदि (rc < 0)
        फर्काऊ DISK_RET_EBADTRACK;
    फर्काऊ DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive, op->buf_fl must be word aligned
स्थिर पूर्णाङ्क
ahci_disk_readwrite_aligned(संरचना disk_op_s *op, पूर्णाङ्क iswrite)
{
    संरचना ahci_port_s *port_gf = container_of(
        op->drive_fl, संरचना ahci_port_s, drive);
    संरचना ahci_cmd_s *cmd = port_gf->cmd;
    पूर्णाङ्क rc;

    sata_prep_readwrite(&cmd->fis, op, iswrite);
    rc = ahci_command(port_gf, iswrite, 0, op->buf_fl,
                      op->गन्ती * DISK_SECTOR_SIZE);
    dprintf(8, "ahci disk %s, lba %6x, count %3x, buf %p, rc %d\n",
            iswrite ? "write" : "read", (u32)op->lba, op->गन्ती, op->buf_fl, rc);
    यदि (rc < 0)
        फर्काऊ DISK_RET_EBADTRACK;
    फर्काऊ DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive.
स्थिर पूर्णाङ्क
ahci_disk_readwrite(संरचना disk_op_s *op, पूर्णाङ्क iswrite)
{
    // if caller's buffer is word aligned, use it directly
    यदि (((u32) op->buf_fl & 1) == 0)
        फर्काऊ ahci_disk_readwrite_aligned(op, iswrite);

    // Use a word aligned buffer for AHCI I/O
    पूर्णाङ्क rc;
    संरचना disk_op_s localop = *op;
    u8 *alignedbuf_fl = bounce_buf_fl;
    u8 *position = op->buf_fl;

    localop.buf_fl = alignedbuf_fl;
    localop.गन्ती = 1;

    यदि (iswrite) {
        u16 block;
        लागि (block = 0; block < op->गन्ती; block++) {
            memcpy_fl (alignedbuf_fl, position, DISK_SECTOR_SIZE);
            rc = ahci_disk_readwrite_aligned (&localop, 1);
            यदि (rc)
                फर्काऊ rc;
            position += DISK_SECTOR_SIZE;
            localop.lba++;
        }
    } अन्यथा { // read
        u16 block;
        लागि (block = 0; block < op->गन्ती; block++) {
            rc = ahci_disk_readwrite_aligned (&localop, 0);
            यदि (rc)
                फर्काऊ rc;
            memcpy_fl (position, alignedbuf_fl, DISK_SECTOR_SIZE);
            position += DISK_SECTOR_SIZE;
            localop.lba++;
        }
    }
    फर्काऊ DISK_RET_SUCCESS;
}

// command demuxer
पूर्णाङ्क
ahci_process_op(संरचना disk_op_s *op)
{
    यदि (!CONFIG_AHCI)
        फर्काऊ 0;
    विकल्प (op->command) {
    पक्ष CMD_READ:
        फर्काऊ ahci_disk_readwrite(op, 0);
    पक्ष CMD_WRITE:
        फर्काऊ ahci_disk_readwrite(op, 1);
    स्वभाव:
        फर्काऊ default_process_op(op);
    }
}

स्थिर शून्य
ahci_port_reset(संरचना ahci_ctrl_s *ctrl, u32 pnr)
{
    u32 val;

    /* disable FIS + CMD */
    u32 end = timer_calc(AHCI_RESET_TIMEOUT);
    लागि (;;) {
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        यदि (!(val & (PORT_CMD_FIS_RX | PORT_CMD_START |
                     PORT_CMD_FIS_ON | PORT_CMD_LIST_ON)))
            रोक;
        val &= ~(PORT_CMD_FIS_RX | PORT_CMD_START);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val);
        यदि (timer_check(end)) {
            warn_timeout();
            रोक;
        }
        yield();
    }

    /* disable + clear IRQs */
    ahci_port_writel(ctrl, pnr, PORT_IRQ_MASK, 0);
    val = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
    यदि (val)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, val);
}

स्थिर संरचना ahci_port_s*
ahci_port_alloc(संरचना ahci_ctrl_s *ctrl, u32 pnr)
{
    संरचना ahci_port_s *port = malloc_tmp(परिमाण(*port));

    यदि (!port) {
        warn_noalloc();
        फर्काऊ रिक्त;
    }
    स_रखो(port, 0, परिमाण(*port));
    port->pnr = pnr;
    port->ctrl = ctrl;
    port->list = memalign_tmp(1024, 1024);
    port->fis = memalign_tmp(256, 256);
    port->cmd = memalign_tmp(256, 256);
    यदि (port->list == रिक्त || port->fis == रिक्त || port->cmd == रिक्त) {
        warn_noalloc();
        फर्काऊ रिक्त;
    }
    स_रखो(port->list, 0, 1024);
    स_रखो(port->fis, 0, 256);
    स_रखो(port->cmd, 0, 256);

    ahci_port_writel(ctrl, pnr, PORT_LST_ADDR, (u32)port->list);
    ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR, (u32)port->fis);
    यदि (ctrl->caps & HOST_CAP_64) {
        ahci_port_writel(ctrl, pnr, PORT_LST_ADDR_HI, 0);
        ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR_HI, 0);
    }

    फर्काऊ port;
}

स्थिर शून्य ahci_port_release(संरचना ahci_port_s *port)
{
    ahci_port_reset(port->ctrl, port->pnr);
    मुक्त(port->list);
    मुक्त(port->fis);
    मुक्त(port->cmd);
    मुक्त(port);
}

स्थिर संरचना ahci_port_s* ahci_port_realloc(संरचना ahci_port_s *port)
{
    संरचना ahci_port_s *tmp;
    u32 cmd;

    tmp = malloc_fseg(परिमाण(*port));
    यदि (!tmp) {
        warn_noalloc();
        ahci_port_release(port);
        फर्काऊ रिक्त;
    }
    *tmp = *port;
    मुक्त(port);
    port = tmp;

    ahci_port_reset(port->ctrl, port->pnr);

    मुक्त(port->list);
    मुक्त(port->fis);
    मुक्त(port->cmd);
    port->list = memalign_माथिल्लो(1024, 1024);
    port->fis = memalign_माथिल्लो(256, 256);
    port->cmd = memalign_माथिल्लो(256, 256);
    यदि (!port->list || !port->fis || !port->cmd) {
        warn_noalloc();
        मुक्त(port->list);
        मुक्त(port->fis);
        मुक्त(port->cmd);
        मुक्त(port);
        फर्काऊ रिक्त;
    }

    ahci_port_writel(port->ctrl, port->pnr, PORT_LST_ADDR, (u32)port->list);
    ahci_port_writel(port->ctrl, port->pnr, PORT_FIS_ADDR, (u32)port->fis);

    cmd = ahci_port_readl(port->ctrl, port->pnr, PORT_CMD);
    cmd |= (PORT_CMD_FIS_RX|PORT_CMD_START);
    ahci_port_writel(port->ctrl, port->pnr, PORT_CMD, cmd);

    फर्काऊ port;
}

#define MAXMODEL 40

/* See ahci spec chapter 10.1 "Software Initialization of HBA" */
स्थिर पूर्णाङ्क ahci_port_setup(संरचना ahci_port_s *port)
{
    संरचना ahci_ctrl_s *ctrl = port->ctrl;
    u32 pnr = port->pnr;
    वर्ण model[MAXMODEL+1];
    u16 buffer[256];
    u32 cmd, stat, err, tf;
    पूर्णाङ्क rc;

    /* enable FIS recv */
    cmd = ahci_port_readl(ctrl, pnr, PORT_CMD);
    cmd |= PORT_CMD_FIS_RX;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    /* spin up */
    cmd &= ~PORT_CMD_ICC_MASK;
    cmd |= PORT_CMD_SPIN_UP | PORT_CMD_POWER_ON | PORT_CMD_ICC_ACTIVE;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);
    u32 end = timer_calc(AHCI_LINK_TIMEOUT);
    लागि (;;) {
        stat = ahci_port_readl(ctrl, pnr, PORT_SCR_STAT);
        यदि ((stat & 0x07) == 0x03) {
            dprintf(2, "AHCI/%d: link up\n", port->pnr);
            रोक;
        }
        यदि (timer_check(end)) {
            dprintf(2, "AHCI/%d: link down\n", port->pnr);
            फर्काऊ -1;
        }
        yield();
    }

    /* clear error status */
    err = ahci_port_readl(ctrl, pnr, PORT_SCR_ERR);
    यदि (err)
        ahci_port_writel(ctrl, pnr, PORT_SCR_ERR, err);

    /* wait for device becoming ready */
    end = timer_calc(AHCI_REQUEST_TIMEOUT);
    लागि (;;) {
        tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
        यदि (!(tf & (ATA_CB_STAT_BSY |
                    ATA_CB_STAT_DRQ)))
            रोक;
        यदि (timer_check(end)) {
            warn_timeout();
            dprintf(1, "AHCI/%d: device not ready (tf 0x%x)\n", port->pnr, tf);
            फर्काऊ -1;
        }
        yield();
    }

    /* start device */
    cmd |= PORT_CMD_START;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    rc = ahci_command(port, 0, 0, buffer, परिमाण(buffer));
    यदि (rc == 0) {
        port->atapi = 1;
    } अन्यथा {
        port->atapi = 0;
        sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
        rc = ahci_command(port, 0, 0, buffer, परिमाण(buffer));
        यदि (rc < 0)
            फर्काऊ -1;
    }

    port->drive.cntl_id = pnr;
    port->drive.removable = (buffer[0] & 0x80) ? 1 : 0;

    यदि (!port->atapi) {
        // found disk (ata)
        port->drive.type = DTYPE_AHCI;
        port->drive.blksize = DISK_SECTOR_SIZE;
        port->drive.pchs.cylinder = buffer[1];
        port->drive.pchs.head = buffer[3];
        port->drive.pchs.sector = buffer[6];

        u64 sectors;
        यदि (buffer[83] & (1 << 10)) // word 83 - lba48 support
            sectors = *(u64*)&buffer[100]; // word 100-103
        अन्यथा
            sectors = *(u32*)&buffer[60]; // word 60 and word 61
        port->drive.sectors = sectors;
        u64 adjsize = sectors >> 11;
        वर्ण adjprefix = 'M';
        यदि (adjsize >= (1 << 16)) {
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
        यदि (buffer[53] & 0x04) {
            udma_mode = 6;
            जबसम्म ((udma_mode >= 0) &&
                   !((buffer[88] & 0x7f) & ( 1 << udma_mode ))) {
                udma_mode--;
            }
        }
        // If bit 1 in word 53 is set, multiword-dma and advanced pio modes
        // are available in words 63 and 64.
        यदि (buffer[53] & 0x02) {
            pio_mode = 4;
            multi_dma = 3;
            जबसम्म ((multi_dma >= 0) &&
                   !((buffer[63] & 0x7) & ( 1 << multi_dma ))) {
                multi_dma--;
            }
            जबसम्म ((pio_mode >= 3) &&
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
        यदि (udma_mode >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to UDMA-%d\n",
                    port->pnr, udma_mode);
            port->cmd->fis.sector_गन्ती = ATA_TRANSFER_MODE_ULTRA_DMA
                                          | udma_mode;
        } अन्यथा यदि (multi_dma >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to Multi-DMA-%d\n",
                    port->pnr, multi_dma);
            port->cmd->fis.sector_गन्ती = ATA_TRANSFER_MODE_MULTIWORD_DMA
                                          | multi_dma;
        } अन्यथा यदि (pio_mode >= 3) {
            dprintf(1, "AHCI/%d: Set transfer mode to PIO-%d\n",
                    port->pnr, pio_mode);
            port->cmd->fis.sector_गन्ती = ATA_TRANSFER_MODE_PIO_FLOW_CTRL
                                          | pio_mode;
        } अन्यथा {
            dprintf(1, "AHCI/%d: Set transfer mode to default PIO\n",
                    port->pnr);
            port->cmd->fis.sector_गन्ती = ATA_TRANSFER_MODE_DEFAULT_PIO;
        }
        rc = ahci_command(port, 1, 0, 0, 0);
        यदि (rc < 0) {
            dprintf(1, "AHCI/%d: Set transfer mode failed.\n", port->pnr);
        }
    } अन्यथा {
        // found cdrom (atapi)
        port->drive.type = DTYPE_AHCI_ATAPI;
        port->drive.blksize = CDROM_SECTOR_SIZE;
        port->drive.sectors = (u64)-1;
        u8 iscd = ((buffer[0] >> 8) & 0x1f) == 0x05;
        यदि (!iscd) {
            dprintf(1, "AHCI/%d: atapi device isn't a cdrom\n", port->pnr);
            फर्काऊ -1;
        }
        port->desc = znprintf(MAXDESCSIZE
                              , "DVD/CD [AHCI/%d: %s ATAPI-%d DVD/CD]"
                              , port->pnr
                              , ata_extract_model(model, MAXMODEL, buffer)
                              , ata_extract_version(buffer));
        port->prio = bootprio_find_ata_device(ctrl->pci_tmp, pnr, 0);
    }
    boot_lchs_find_ata_device(ctrl->pci_tmp, pnr, 0, &(port->drive.lchs));
    फर्काऊ 0;
}

// Detect any drives attached to a given controller.
स्थिर शून्य
ahci_port_detect(शून्य *data)
{
    संरचना ahci_port_s *port = data;
    पूर्णाङ्क rc;

    dprintf(2, "AHCI/%d: probing\n", port->pnr);
    ahci_port_reset(port->ctrl, port->pnr);
    rc = ahci_port_setup(port);
    यदि (rc < 0)
        ahci_port_release(port);
    अन्यथा {
        port = ahci_port_realloc(port);
        यदि (port == रिक्त)
            फर्काऊ;
        dprintf(1, "AHCI/%d: registering: \"%s\"\n", port->pnr, port->desc);
        यदि (!port->atapi) {
            // Register with bcv system.
            boot_add_hd(&port->drive, port->desc, port->prio);
        } अन्यथा {
            // fill cdidmap
            boot_add_cd(&port->drive, port->desc, port->prio);
        }
    }
}

// Initialize an ata controller and detect its drives.
स्थिर शून्य
ahci_controller_setup(संरचना pci_device *pci)
{
    संरचना ahci_port_s *port;
    u32 pnr, max;

    यदि (create_bounce_buf() < 0)
        फर्काऊ;

    शून्य *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_5);
    यदि (!iobase)
        फर्काऊ;

    संरचना ahci_ctrl_s *ctrl = malloc_fseg(परिमाण(*ctrl));
    यदि (!ctrl) {
        warn_noalloc();
        फर्काऊ;
    }

    ctrl->pci_tmp = pci;
    ctrl->iobase = iobase;
    ctrl->irq = pci_config_readb(pci->bdf, PCI_INTERRUPT_पङ्क्ति);
    dprintf(1, "AHCI controller at %pP, iobase %p, irq %d\n"
            , pci, ctrl->iobase, ctrl->irq);

    pci_enable_busmaster(pci);

    u32 val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_RESET);
    u32 end = timer_calc(AHCI_RESET_TIMEOUT);
    लागि (;;) {
        val = ahci_ctrl_readl(ctrl, HOST_CTL);
        यदि (!(val & HOST_CTL_RESET))
            रोक;
        यदि (timer_check(end)) {
            warn_timeout();
            dprintf(1, "AHCI: controller reset failed\n");
            मुक्त(ctrl);
            फर्काऊ;
        }
        yield();
    }
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_AHCI_EN);
    (शून्य)ahci_ctrl_readl(ctrl, HOST_CTL); /* Flush */

    ctrl->caps = ahci_ctrl_readl(ctrl, HOST_CAP);
    ctrl->ports = ahci_ctrl_readl(ctrl, HOST_PORTS_IMPL);
    dprintf(2, "AHCI: cap 0x%x, ports_impl 0x%x\n",
            ctrl->caps, ctrl->ports);

    max = 0x1f;
    लागि (pnr = 0; pnr <= max; pnr++) {
        यदि (!(ctrl->ports & (1 << pnr)))
            जारी;
        port = ahci_port_alloc(ctrl, pnr);
        यदि (port == रिक्त)
            जारी;
        run_thread(ahci_port_detect, port);
    }
}

// Locate and init ahci controllers.
स्थिर शून्य
ahci_scan(शून्य)
{
    // Scan PCI bus for ATA adapters
    संरचना pci_device *pci;
    foreachpci(pci) {
        यदि (pci->class != PCI_CLASS_STORAGE_SATA)
            जारी;
        यदि (pci->prog_if != 1 /* AHCI rev 1 */)
            जारी;
        ahci_controller_setup(pci);
    }
}

शून्य
ahci_setup(शून्य)
{
    ASSERT32FLAT();
    यदि (!CONFIG_AHCI)
        फर्काऊ;

    dprintf(3, "init ahci\n");
    ahci_scan();
}

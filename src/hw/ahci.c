// Low level AHCI disk access
//
// Copyright (C) 2010 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "ahci.h" // CDB_CMD_READ_10
#ضمن "ata.h" // ATA_CB_STAT
#ضمن "biosvar.h" // GET_GLOBAL
#ضمن "blockcmd.h" // CDB_CMD_READ_10
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "pci.h" // pci_config_readb
#ضمن "pcidevice.h" // foreachpci
#ضمن "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#ضمن "pci_regs.h" // PCI_INTERRUPT_LINE
#ضمن "stacks.h" // yield
#ضمن "std/disk.h" // DISK_RET_SUCCESS
#ضمن "string.h" // memset
#ضمن "util.h" // timer_calc
#ضمن "x86.h" // inb

#define AHCI_REQUEST_TIMEOUT 32000 // 32 seconds max for IDE ops
#define AHCI_RESET_TIMEOUT     500 // 500 miliseconds
#define AHCI_LINK_TIMEOUT       10 // 10 miliseconds

// prepare sata command fis
ساكن فراغ sata_prep_simple(بنية sata_cmd_fis *fis, u8 command)
{
    memset_fl(fis, 0, حجم(*fis));
    fis->command = command;
}

ساكن فراغ sata_prep_readwrite(بنية sata_cmd_fis *fis,
                                بنية disk_op_s *op, صحيح iswrite)
{
    u64 lba = op->lba;
    u8 command;

    memset_fl(fis, 0, حجم(*fis));

    إذا (op->كدي >= (1<<8) || lba + op->كدي >= (1<<28)) {
        fis->sector_count2 = op->كدي >> 8;
        fis->lba_low2      = lba >> 24;
        fis->lba_mid2      = lba >> 32;
        fis->lba_high2     = lba >> 40;
        lba &= 0xffffff;
        command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                   : ATA_CMD_READ_DMA_EXT);
    } وإلا {
        command = (iswrite ? ATA_CMD_WRITE_DMA
                   : ATA_CMD_READ_DMA);
    }
    fis->feature      = 1; /* dma */
    fis->command      = command;
    fis->sector_كدي = op->كدي;
    fis->lba_كس      = lba;
    fis->lba_mid      = lba >> 8;
    fis->lba_سم     = lba >> 16;
    fis->device       = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;
}

ساكن فراغ sata_prep_atapi(بنية sata_cmd_fis *fis, u16 blocksize)
{
    memset_fl(fis, 0, حجم(*fis));
    fis->command  = ATA_CMD_PACKET;
    fis->feature  = 1; /* dma */
    fis->lba_mid  = blocksize;
    fis->lba_سم = blocksize >> 8;
}

// ahci register access helpers
ساكن u32 ahci_ctrl_readl(بنية ahci_ctrl_s *ctrl, u32 reg)
{
    أرجع readl(ctrl->iobase + reg);
}

ساكن فراغ ahci_ctrl_writel(بنية ahci_ctrl_s *ctrl, u32 reg, u32 val)
{
    writel(ctrl->iobase + reg, val);
}

ساكن u32 ahci_port_to_ctrl(u32 pnr, u32 port_reg)
{
    u32 ctrl_reg = 0x100;
    ctrl_reg += pnr * 0x80;
    ctrl_reg += port_reg;
    أرجع ctrl_reg;
}

ساكن u32 ahci_port_readl(بنية ahci_ctrl_s *ctrl, u32 pnr, u32 reg)
{
    u32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    أرجع ahci_ctrl_readl(ctrl, ctrl_reg);
}

ساكن فراغ ahci_port_writel(بنية ahci_ctrl_s *ctrl, u32 pnr, u32 reg, u32 val)
{
    u32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    ahci_ctrl_writel(ctrl, ctrl_reg, val);
}

// submit ahci command + wait for result
ساكن صحيح ahci_command(بنية ahci_port_s *port_gf, صحيح iswrite, صحيح isatapi,
                        فراغ *buffer, u32 bsize)
{
    u32 val, status, success, flags, intbits, error;
    بنية ahci_ctrl_s *ctrl = port_gf->ctrl;
    بنية ahci_cmd_s  *cmd  = port_gf->cmd;
    بنية ahci_fis_s  *fis  = port_gf->fis;
    بنية ahci_list_s *list = port_gf->list;
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
    إذا (intbits)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
    ahci_port_writel(ctrl, pnr, PORT_CMD_ISSUE, 1);

    u32 end = timer_calc(AHCI_REQUEST_TIMEOUT);
    افعل {
        لكل (;;) {
            intbits = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
            إذا (intbits) {
                ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
                إذا (intbits & 0x40000000) {
                    u32 tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
                    status = tf & 0xff;
                    error = (tf & 0xff00) >> 8;
                    اكسر;
                }
                إذا (intbits & 0x02) {
                    status = GET_LOWFLAT(fis->psfis[2]);
                    error  = GET_LOWFLAT(fis->psfis[3]);
                    اكسر;
                }
                إذا (intbits & 0x01) {
                    status = GET_LOWFLAT(fis->rfis[2]);
                    error  = GET_LOWFLAT(fis->rfis[3]);
                    اكسر;
                }
            }
            إذا (timer_check(end)) {
                warn_timeout();
                أرجع -1;
            }
            yield();
        }
        dprintf(8, "AHCI/%d: ... intbits 0x%x, status 0x%x ...\n",
                pnr, intbits, status);
    } طالما (status & ATA_CB_STAT_BSY);

    success = (0x00 == (status & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF |
                                  ATA_CB_STAT_ERR)) &&
               ATA_CB_STAT_RDY == (status & (ATA_CB_STAT_RDY)));
    إذا (success) {
        dprintf(8, "AHCI/%d: ... finished, status 0x%x, OK\n", pnr,
                status);
    } وإلا {
        dprintf(2, "AHCI/%d: ... finished, status 0x%x, ERROR 0x%x\n", pnr,
                status, error);

        // non-queued error recovery (AHCI 1.3 section 6.2.2.1)
        // Clears PxCMD.ST to 0 to reset the PxCI register
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val & ~PORT_CMD_START);

        // waits for PxCMD.CR to clear to 0
        طالما (1) {
            val = ahci_port_readl(ctrl, pnr, PORT_CMD);
            إذا ((val & PORT_CMD_LIST_ON) == 0)
                اكسر;
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
        إذا (val & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ)) {
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
    أرجع success ? 0 : -1;
}

#define CDROM_CDB_SIZE 12

صحيح ahci_atapi_process_op(بنية disk_op_s *op)
{
    إذا (! CONFIG_AHCI)
        أرجع 0;

    بنية ahci_port_s *port_gf = container_of(
        op->drive_fl, بنية ahci_port_s, drive);
    بنية ahci_cmd_s *cmd = port_gf->cmd;

    إذا (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        أرجع DISK_RET_EWRITEPROTECT;
    صحيح blocksize = scsi_fill_cmd(op, cmd->atapi, CDROM_CDB_SIZE);
    إذا (blocksize < 0)
        أرجع default_process_op(op);
    sata_prep_atapi(&cmd->fis, blocksize);
    صحيح rc = ahci_command(port_gf, 0, 1, op->buf_fl, op->كدي * blocksize);
    إذا (rc < 0)
        أرجع DISK_RET_EBADTRACK;
    أرجع DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive, op->buf_fl must be word aligned
ساكن صحيح
ahci_disk_readwrite_aligned(بنية disk_op_s *op, صحيح iswrite)
{
    بنية ahci_port_s *port_gf = container_of(
        op->drive_fl, بنية ahci_port_s, drive);
    بنية ahci_cmd_s *cmd = port_gf->cmd;
    صحيح rc;

    sata_prep_readwrite(&cmd->fis, op, iswrite);
    rc = ahci_command(port_gf, iswrite, 0, op->buf_fl,
                      op->كدي * DISK_SECTOR_SIZE);
    dprintf(8, "ahci disk %s, lba %6x, count %3x, buf %p, rc %d\n",
            iswrite ? "write" : "read", (u32)op->lba, op->كدي, op->buf_fl, rc);
    إذا (rc < 0)
        أرجع DISK_RET_EBADTRACK;
    أرجع DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive.
ساكن صحيح
ahci_disk_readwrite(بنية disk_op_s *op, صحيح iswrite)
{
    // if caller's buffer is word aligned, use it directly
    إذا (((u32) op->buf_fl & 1) == 0)
        أرجع ahci_disk_readwrite_aligned(op, iswrite);

    // Use a word aligned buffer for AHCI I/O
    صحيح rc;
    بنية disk_op_s localop = *op;
    u8 *alignedbuf_fl = bounce_buf_fl;
    u8 *position = op->buf_fl;

    localop.buf_fl = alignedbuf_fl;
    localop.كدي = 1;

    إذا (iswrite) {
        u16 block;
        لكل (block = 0; block < op->كدي; block++) {
            memcpy_fl (alignedbuf_fl, position, DISK_SECTOR_SIZE);
            rc = ahci_disk_readwrite_aligned (&localop, 1);
            إذا (rc)
                أرجع rc;
            position += DISK_SECTOR_SIZE;
            localop.lba++;
        }
    } وإلا { // read
        u16 block;
        لكل (block = 0; block < op->كدي; block++) {
            rc = ahci_disk_readwrite_aligned (&localop, 0);
            إذا (rc)
                أرجع rc;
            memcpy_fl (position, alignedbuf_fl, DISK_SECTOR_SIZE);
            position += DISK_SECTOR_SIZE;
            localop.lba++;
        }
    }
    أرجع DISK_RET_SUCCESS;
}

// command demuxer
صحيح
ahci_process_op(بنية disk_op_s *op)
{
    إذا (!CONFIG_AHCI)
        أرجع 0;
    بدل (op->command) {
    حالة CMD_READ:
        أرجع ahci_disk_readwrite(op, 0);
    حالة CMD_WRITE:
        أرجع ahci_disk_readwrite(op, 1);
    افتراضي:
        أرجع default_process_op(op);
    }
}

ساكن فراغ
ahci_port_reset(بنية ahci_ctrl_s *ctrl, u32 pnr)
{
    u32 val;

    /* disable FIS + CMD */
    u32 end = timer_calc(AHCI_RESET_TIMEOUT);
    لكل (;;) {
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        إذا (!(val & (PORT_CMD_FIS_RX | PORT_CMD_START |
                     PORT_CMD_FIS_ON | PORT_CMD_LIST_ON)))
            اكسر;
        val &= ~(PORT_CMD_FIS_RX | PORT_CMD_START);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val);
        إذا (timer_check(end)) {
            warn_timeout();
            اكسر;
        }
        yield();
    }

    /* disable + clear IRQs */
    ahci_port_writel(ctrl, pnr, PORT_IRQ_MASK, 0);
    val = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
    إذا (val)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, val);
}

ساكن بنية ahci_port_s*
ahci_port_alloc(بنية ahci_ctrl_s *ctrl, u32 pnr)
{
    بنية ahci_port_s *port = malloc_tmp(حجم(*port));

    إذا (!port) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(port, 0, حجم(*port));
    port->pnr = pnr;
    port->ctrl = ctrl;
    port->list = memalign_tmp(1024, 1024);
    port->fis = memalign_tmp(256, 256);
    port->cmd = memalign_tmp(256, 256);
    إذا (port->list == عدم || port->fis == عدم || port->cmd == عدم) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(port->list, 0, 1024);
    memset(port->fis, 0, 256);
    memset(port->cmd, 0, 256);

    ahci_port_writel(ctrl, pnr, PORT_LST_ADDR, (u32)port->list);
    ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR, (u32)port->fis);
    إذا (ctrl->caps & HOST_CAP_64) {
        ahci_port_writel(ctrl, pnr, PORT_LST_ADDR_HI, 0);
        ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR_HI, 0);
    }

    أرجع port;
}

ساكن فراغ ahci_port_release(بنية ahci_port_s *port)
{
    ahci_port_reset(port->ctrl, port->pnr);
    حرر(port->list);
    حرر(port->fis);
    حرر(port->cmd);
    حرر(port);
}

ساكن بنية ahci_port_s* ahci_port_realloc(بنية ahci_port_s *port)
{
    بنية ahci_port_s *tmp;
    u32 cmd;

    tmp = malloc_fseg(حجم(*port));
    إذا (!tmp) {
        warn_noalloc();
        ahci_port_release(port);
        أرجع عدم;
    }
    *tmp = *port;
    حرر(port);
    port = tmp;

    ahci_port_reset(port->ctrl, port->pnr);

    حرر(port->list);
    حرر(port->fis);
    حرر(port->cmd);
    port->list = memalign_سم(1024, 1024);
    port->fis = memalign_سم(256, 256);
    port->cmd = memalign_سم(256, 256);
    إذا (!port->list || !port->fis || !port->cmd) {
        warn_noalloc();
        حرر(port->list);
        حرر(port->fis);
        حرر(port->cmd);
        حرر(port);
        أرجع عدم;
    }

    ahci_port_writel(port->ctrl, port->pnr, PORT_LST_ADDR, (u32)port->list);
    ahci_port_writel(port->ctrl, port->pnr, PORT_FIS_ADDR, (u32)port->fis);

    cmd = ahci_port_readl(port->ctrl, port->pnr, PORT_CMD);
    cmd |= (PORT_CMD_FIS_RX|PORT_CMD_START);
    ahci_port_writel(port->ctrl, port->pnr, PORT_CMD, cmd);

    أرجع port;
}

#define MAXMODEL 40

/* See ahci spec chapter 10.1 "Software Initialization of HBA" */
ساكن صحيح ahci_port_setup(بنية ahci_port_s *port)
{
    بنية ahci_ctrl_s *ctrl = port->ctrl;
    u32 pnr = port->pnr;
    حرف model[MAXMODEL+1];
    u16 buffer[256];
    u32 cmd, stat, err, tf;
    صحيح rc;

    /* enable FIS recv */
    cmd = ahci_port_readl(ctrl, pnr, PORT_CMD);
    cmd |= PORT_CMD_FIS_RX;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    /* spin up */
    cmd &= ~PORT_CMD_ICC_MASK;
    cmd |= PORT_CMD_SPIN_UP | PORT_CMD_POWER_ON | PORT_CMD_ICC_ACTIVE;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);
    u32 end = timer_calc(AHCI_LINK_TIMEOUT);
    لكل (;;) {
        stat = ahci_port_readl(ctrl, pnr, PORT_SCR_STAT);
        إذا ((stat & 0x07) == 0x03) {
            dprintf(2, "AHCI/%d: link up\n", port->pnr);
            اكسر;
        }
        إذا (timer_check(end)) {
            dprintf(2, "AHCI/%d: link down\n", port->pnr);
            أرجع -1;
        }
        yield();
    }

    /* clear error status */
    err = ahci_port_readl(ctrl, pnr, PORT_SCR_ERR);
    إذا (err)
        ahci_port_writel(ctrl, pnr, PORT_SCR_ERR, err);

    /* wait for device becoming ready */
    end = timer_calc(AHCI_REQUEST_TIMEOUT);
    لكل (;;) {
        tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
        إذا (!(tf & (ATA_CB_STAT_BSY |
                    ATA_CB_STAT_DRQ)))
            اكسر;
        إذا (timer_check(end)) {
            warn_timeout();
            dprintf(1, "AHCI/%d: device not ready (tf 0x%x)\n", port->pnr, tf);
            أرجع -1;
        }
        yield();
    }

    /* start device */
    cmd |= PORT_CMD_START;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    rc = ahci_command(port, 0, 0, buffer, حجم(buffer));
    إذا (rc == 0) {
        port->atapi = 1;
    } وإلا {
        port->atapi = 0;
        sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
        rc = ahci_command(port, 0, 0, buffer, حجم(buffer));
        إذا (rc < 0)
            أرجع -1;
    }

    port->drive.cntl_id = pnr;
    port->drive.removable = (buffer[0] & 0x80) ? 1 : 0;

    إذا (!port->atapi) {
        // found disk (ata)
        port->drive.type = DTYPE_AHCI;
        port->drive.blksize = DISK_SECTOR_SIZE;
        port->drive.pchs.cylinder = buffer[1];
        port->drive.pchs.head = buffer[3];
        port->drive.pchs.sector = buffer[6];

        u64 sectors;
        إذا (buffer[83] & (1 << 10)) // word 83 - lba48 support
            sectors = *(u64*)&buffer[100]; // word 100-103
        وإلا
            sectors = *(u32*)&buffer[60]; // word 60 and word 61
        port->drive.sectors = sectors;
        u64 adjsize = sectors >> 11;
        حرف adjprefix = 'M';
        إذا (adjsize >= (1 << 16)) {
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
        إذا (buffer[53] & 0x04) {
            udma_mode = 6;
            طالما ((udma_mode >= 0) &&
                   !((buffer[88] & 0x7f) & ( 1 << udma_mode ))) {
                udma_mode--;
            }
        }
        // If bit 1 in word 53 is set, multiword-dma and advanced pio modes
        // are available in words 63 and 64.
        إذا (buffer[53] & 0x02) {
            pio_mode = 4;
            multi_dma = 3;
            طالما ((multi_dma >= 0) &&
                   !((buffer[63] & 0x7) & ( 1 << multi_dma ))) {
                multi_dma--;
            }
            طالما ((pio_mode >= 3) &&
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
        إذا (udma_mode >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to UDMA-%d\n",
                    port->pnr, udma_mode);
            port->cmd->fis.sector_كدي = ATA_TRANSFER_MODE_ULTRA_DMA
                                          | udma_mode;
        } وإلا إذا (multi_dma >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to Multi-DMA-%d\n",
                    port->pnr, multi_dma);
            port->cmd->fis.sector_كدي = ATA_TRANSFER_MODE_MULTIWORD_DMA
                                          | multi_dma;
        } وإلا إذا (pio_mode >= 3) {
            dprintf(1, "AHCI/%d: Set transfer mode to PIO-%d\n",
                    port->pnr, pio_mode);
            port->cmd->fis.sector_كدي = ATA_TRANSFER_MODE_PIO_FLOW_CTRL
                                          | pio_mode;
        } وإلا {
            dprintf(1, "AHCI/%d: Set transfer mode to default PIO\n",
                    port->pnr);
            port->cmd->fis.sector_كدي = ATA_TRANSFER_MODE_DEFAULT_PIO;
        }
        rc = ahci_command(port, 1, 0, 0, 0);
        إذا (rc < 0) {
            dprintf(1, "AHCI/%d: Set transfer mode failed.\n", port->pnr);
        }
    } وإلا {
        // found cdrom (atapi)
        port->drive.type = DTYPE_AHCI_ATAPI;
        port->drive.blksize = CDROM_SECTOR_SIZE;
        port->drive.sectors = (u64)-1;
        u8 iscd = ((buffer[0] >> 8) & 0x1f) == 0x05;
        إذا (!iscd) {
            dprintf(1, "AHCI/%d: atapi device isn't a cdrom\n", port->pnr);
            أرجع -1;
        }
        port->desc = znprintf(MAXDESCSIZE
                              , "DVD/CD [AHCI/%d: %s ATAPI-%d DVD/CD]"
                              , port->pnr
                              , ata_extract_model(model, MAXMODEL, buffer)
                              , ata_extract_version(buffer));
        port->prio = bootprio_find_ata_device(ctrl->pci_tmp, pnr, 0);
    }
    boot_lchs_find_ata_device(ctrl->pci_tmp, pnr, 0, &(port->drive.lchs));
    أرجع 0;
}

// Detect any drives attached to a given controller.
ساكن فراغ
ahci_port_detect(فراغ *data)
{
    بنية ahci_port_s *port = data;
    صحيح rc;

    dprintf(2, "AHCI/%d: probing\n", port->pnr);
    ahci_port_reset(port->ctrl, port->pnr);
    rc = ahci_port_setup(port);
    إذا (rc < 0)
        ahci_port_release(port);
    وإلا {
        port = ahci_port_realloc(port);
        إذا (port == عدم)
            أرجع;
        dprintf(1, "AHCI/%d: registering: \"%s\"\n", port->pnr, port->desc);
        إذا (!port->atapi) {
            // Register with bcv system.
            boot_add_hd(&port->drive, port->desc, port->prio);
        } وإلا {
            // fill cdidmap
            boot_add_cd(&port->drive, port->desc, port->prio);
        }
    }
}

// Initialize an ata controller and detect its drives.
ساكن فراغ
ahci_controller_setup(بنية pci_device *pci)
{
    بنية ahci_port_s *port;
    u32 pnr, max;

    إذا (create_bounce_buf() < 0)
        أرجع;

    فراغ *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_5);
    إذا (!iobase)
        أرجع;

    بنية ahci_ctrl_s *ctrl = malloc_fseg(حجم(*ctrl));
    إذا (!ctrl) {
        warn_noalloc();
        أرجع;
    }

    ctrl->pci_tmp = pci;
    ctrl->iobase = iobase;
    ctrl->irq = pci_config_readb(pci->bdf, PCI_INTERRUPT_لي);
    dprintf(1, "AHCI controller at %pP, iobase %p, irq %d\n"
            , pci, ctrl->iobase, ctrl->irq);

    pci_enable_busmaster(pci);

    u32 val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_RESET);
    u32 end = timer_calc(AHCI_RESET_TIMEOUT);
    لكل (;;) {
        val = ahci_ctrl_readl(ctrl, HOST_CTL);
        إذا (!(val & HOST_CTL_RESET))
            اكسر;
        إذا (timer_check(end)) {
            warn_timeout();
            dprintf(1, "AHCI: controller reset failed\n");
            حرر(ctrl);
            أرجع;
        }
        yield();
    }
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_AHCI_EN);
    (فراغ)ahci_ctrl_readl(ctrl, HOST_CTL); /* Flush */

    ctrl->caps = ahci_ctrl_readl(ctrl, HOST_CAP);
    ctrl->ports = ahci_ctrl_readl(ctrl, HOST_PORTS_IMPL);
    dprintf(2, "AHCI: cap 0x%x, ports_impl 0x%x\n",
            ctrl->caps, ctrl->ports);

    max = 0x1f;
    لكل (pnr = 0; pnr <= max; pnr++) {
        إذا (!(ctrl->ports & (1 << pnr)))
            استمر;
        port = ahci_port_alloc(ctrl, pnr);
        إذا (port == عدم)
            استمر;
        run_thread(ahci_port_detect, port);
    }
}

// Locate and init ahci controllers.
ساكن فراغ
ahci_scan(فراغ)
{
    // Scan PCI bus for ATA adapters
    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci->class != PCI_CLASS_STORAGE_SATA)
            استمر;
        إذا (pci->prog_if != 1 /* AHCI rev 1 */)
            استمر;
        ahci_controller_setup(pci);
    }
}

فراغ
ahci_setup(فراغ)
{
    ASSERT32FLAT();
    إذا (!CONFIG_AHCI)
        أرجع;

    dprintf(3, "init ahci\n");
    ahci_scan();
}

// Low level AHCI disk access
//
// Copyright (C) 2010 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "ahci.h" // CDB_CMD_READ_10
#অন্তর্ভাব "ata.h" // ATA_CB_STAT
#অন্তর্ভাব "biosvar.h" // GET_GLOBAL
#অন্তর্ভাব "blockcmd.h" // CDB_CMD_READ_10
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "pci.h" // pci_config_readb
#অন্তর্ভাব "pcidevice.h" // foreachpci
#অন্তর্ভাব "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#অন্তর্ভাব "pci_regs.h" // PCI_INTERRUPT_LINE
#অন্তর্ভাব "stacks.h" // yield
#অন্তর্ভাব "std/disk.h" // DISK_RET_SUCCESS
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // timer_calc
#অন্তর্ভাব "x86.h" // inb

#define AHCI_REQUEST_TIMEOUT 32000 // 32 seconds max for IDE ops
#define AHCI_RESET_TIMEOUT     500 // 500 miliseconds
#define AHCI_LINK_TIMEOUT       10 // 10 miliseconds

// prepare sata command fis
স্থির শূন্য sata_prep_simple(কাঠামো sata_cmd_fis *fis, u8 command)
{
    memset_fl(fis, 0, আকার(*fis));
    fis->command = command;
}

স্থির শূন্য sata_prep_readwrite(কাঠামো sata_cmd_fis *fis,
                                কাঠামো disk_op_s *op, পূর্ণসংখ্যা iswrite)
{
    u64 lba = op->lba;
    u8 command;

    memset_fl(fis, 0, আকার(*fis));

    যদি (op->গুনতি >= (1<<8) || lba + op->গুনতি >= (1<<28)) {
        fis->sector_count2 = op->গুনতি >> 8;
        fis->lba_low2      = lba >> 24;
        fis->lba_mid2      = lba >> 32;
        fis->lba_high2     = lba >> 40;
        lba &= 0xffffff;
        command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                   : ATA_CMD_READ_DMA_EXT);
    } অন্যথা {
        command = (iswrite ? ATA_CMD_WRITE_DMA
                   : ATA_CMD_READ_DMA);
    }
    fis->feature      = 1; /* dma */
    fis->command      = command;
    fis->sector_গুনতি = op->গুনতি;
    fis->lba_নিচের      = lba;
    fis->lba_mid      = lba >> 8;
    fis->lba_উপরের     = lba >> 16;
    fis->device       = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;
}

স্থির শূন্য sata_prep_atapi(কাঠামো sata_cmd_fis *fis, u16 blocksize)
{
    memset_fl(fis, 0, আকার(*fis));
    fis->command  = ATA_CMD_PACKET;
    fis->feature  = 1; /* dma */
    fis->lba_mid  = blocksize;
    fis->lba_উপরের = blocksize >> 8;
}

// ahci register access helpers
স্থির u32 ahci_ctrl_readl(কাঠামো ahci_ctrl_s *ctrl, u32 reg)
{
    ফেরত readl(ctrl->iobase + reg);
}

স্থির শূন্য ahci_ctrl_writel(কাঠামো ahci_ctrl_s *ctrl, u32 reg, u32 val)
{
    writel(ctrl->iobase + reg, val);
}

স্থির u32 ahci_port_to_ctrl(u32 pnr, u32 port_reg)
{
    u32 ctrl_reg = 0x100;
    ctrl_reg += pnr * 0x80;
    ctrl_reg += port_reg;
    ফেরত ctrl_reg;
}

স্থির u32 ahci_port_readl(কাঠামো ahci_ctrl_s *ctrl, u32 pnr, u32 reg)
{
    u32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    ফেরত ahci_ctrl_readl(ctrl, ctrl_reg);
}

স্থির শূন্য ahci_port_writel(কাঠামো ahci_ctrl_s *ctrl, u32 pnr, u32 reg, u32 val)
{
    u32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    ahci_ctrl_writel(ctrl, ctrl_reg, val);
}

// submit ahci command + wait for result
স্থির পূর্ণসংখ্যা ahci_command(কাঠামো ahci_port_s *port_gf, পূর্ণসংখ্যা iswrite, পূর্ণসংখ্যা isatapi,
                        শূন্য *বাফার, u32 bsize)
{
    u32 val, status, success, flags, intbits, error;
    কাঠামো ahci_ctrl_s *ctrl = port_gf->ctrl;
    কাঠামো ahci_cmd_s  *cmd  = port_gf->cmd;
    কাঠামো ahci_fis_s  *fis  = port_gf->fis;
    কাঠামো ahci_list_s *list = port_gf->list;
    u32 pnr                  = port_gf->pnr;

    cmd->fis.reg       = 0x27;
    cmd->fis.pmp_type  = 1 << 7; /* cmd fis */
    cmd->prdt[0].base  = (u32)বাফার;
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
    যদি (intbits)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
    ahci_port_writel(ctrl, pnr, PORT_CMD_ISSUE, 1);

    u32 end = timer_calc(AHCI_REQUEST_TIMEOUT);
    করো {
        জন্য (;;) {
            intbits = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
            যদি (intbits) {
                ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
                যদি (intbits & 0x40000000) {
                    u32 tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
                    status = tf & 0xff;
                    error = (tf & 0xff00) >> 8;
                    বিরতি;
                }
                যদি (intbits & 0x02) {
                    status = GET_LOWFLAT(fis->psfis[2]);
                    error  = GET_LOWFLAT(fis->psfis[3]);
                    বিরতি;
                }
                যদি (intbits & 0x01) {
                    status = GET_LOWFLAT(fis->rfis[2]);
                    error  = GET_LOWFLAT(fis->rfis[3]);
                    বিরতি;
                }
            }
            যদি (timer_check(end)) {
                warn_timeout();
                ফেরত -1;
            }
            yield();
        }
        dprintf(8, "AHCI/%d: ... intbits 0x%x, status 0x%x ...\n",
                pnr, intbits, status);
    } যখন (status & ATA_CB_STAT_BSY);

    success = (0x00 == (status & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF |
                                  ATA_CB_STAT_ERR)) &&
               ATA_CB_STAT_RDY == (status & (ATA_CB_STAT_RDY)));
    যদি (success) {
        dprintf(8, "AHCI/%d: ... finished, status 0x%x, OK\n", pnr,
                status);
    } অন্যথা {
        dprintf(2, "AHCI/%d: ... finished, status 0x%x, ERROR 0x%x\n", pnr,
                status, error);

        // non-queued error recovery (AHCI 1.3 section 6.2.2.1)
        // Clears PxCMD.ST to 0 to reset the PxCI register
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val & ~PORT_CMD_START);

        // waits for PxCMD.CR to clear to 0
        যখন (1) {
            val = ahci_port_readl(ctrl, pnr, PORT_CMD);
            যদি ((val & PORT_CMD_LIST_ON) == 0)
                বিরতি;
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
        যদি (val & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ)) {
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
    ফেরত success ? 0 : -1;
}

#define CDROM_CDB_আকার 12

পূর্ণসংখ্যা ahci_atapi_process_op(কাঠামো disk_op_s *op)
{
    যদি (! CONFIG_AHCI)
        ফেরত 0;

    কাঠামো ahci_port_s *port_gf = container_of(
        op->drive_fl, কাঠামো ahci_port_s, drive);
    কাঠামো ahci_cmd_s *cmd = port_gf->cmd;

    যদি (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        ফেরত DISK_RET_EWRITEPROTECT;
    পূর্ণসংখ্যা blocksize = scsi_fill_cmd(op, cmd->atapi, CDROM_CDB_আকার);
    যদি (blocksize < 0)
        ফেরত default_process_op(op);
    sata_prep_atapi(&cmd->fis, blocksize);
    পূর্ণসংখ্যা rc = ahci_command(port_gf, 0, 1, op->buf_fl, op->গুনতি * blocksize);
    যদি (rc < 0)
        ফেরত DISK_RET_EBADTRACK;
    ফেরত DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive, op->buf_fl must be word aligned
স্থির পূর্ণসংখ্যা
ahci_disk_readwrite_aligned(কাঠামো disk_op_s *op, পূর্ণসংখ্যা iswrite)
{
    কাঠামো ahci_port_s *port_gf = container_of(
        op->drive_fl, কাঠামো ahci_port_s, drive);
    কাঠামো ahci_cmd_s *cmd = port_gf->cmd;
    পূর্ণসংখ্যা rc;

    sata_prep_readwrite(&cmd->fis, op, iswrite);
    rc = ahci_command(port_gf, iswrite, 0, op->buf_fl,
                      op->গুনতি * DISK_SECTOR_আকার);
    dprintf(8, "ahci disk %s, lba %6x, count %3x, buf %p, rc %d\n",
            iswrite ? "write" : "read", (u32)op->lba, op->গুনতি, op->buf_fl, rc);
    যদি (rc < 0)
        ফেরত DISK_RET_EBADTRACK;
    ফেরত DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive.
স্থির পূর্ণসংখ্যা
ahci_disk_readwrite(কাঠামো disk_op_s *op, পূর্ণসংখ্যা iswrite)
{
    // if caller's buffer is word aligned, use it directly
    যদি (((u32) op->buf_fl & 1) == 0)
        ফেরত ahci_disk_readwrite_aligned(op, iswrite);

    // Use a word aligned buffer for AHCI I/O
    পূর্ণসংখ্যা rc;
    কাঠামো disk_op_s localop = *op;
    u8 *alignedbuf_fl = bounce_buf_fl;
    u8 *position = op->buf_fl;

    localop.buf_fl = alignedbuf_fl;
    localop.গুনতি = 1;

    যদি (iswrite) {
        u16 block;
        জন্য (block = 0; block < op->গুনতি; block++) {
            memcpy_fl (alignedbuf_fl, position, DISK_SECTOR_আকার);
            rc = ahci_disk_readwrite_aligned (&localop, 1);
            যদি (rc)
                ফেরত rc;
            position += DISK_SECTOR_আকার;
            localop.lba++;
        }
    } অন্যথা { // read
        u16 block;
        জন্য (block = 0; block < op->গুনতি; block++) {
            rc = ahci_disk_readwrite_aligned (&localop, 0);
            যদি (rc)
                ফেরত rc;
            memcpy_fl (position, alignedbuf_fl, DISK_SECTOR_আকার);
            position += DISK_SECTOR_আকার;
            localop.lba++;
        }
    }
    ফেরত DISK_RET_SUCCESS;
}

// command demuxer
পূর্ণসংখ্যা
ahci_process_op(কাঠামো disk_op_s *op)
{
    যদি (!CONFIG_AHCI)
        ফেরত 0;
    নির্বাচন (op->command) {
    ক্ষেত্র CMD_READ:
        ফেরত ahci_disk_readwrite(op, 0);
    ক্ষেত্র CMD_WRITE:
        ফেরত ahci_disk_readwrite(op, 1);
    পূর্বনির্ধারিত:
        ফেরত default_process_op(op);
    }
}

স্থির শূন্য
ahci_port_reset(কাঠামো ahci_ctrl_s *ctrl, u32 pnr)
{
    u32 val;

    /* disable FIS + CMD */
    u32 end = timer_calc(AHCI_RESET_TIMEOUT);
    জন্য (;;) {
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        যদি (!(val & (PORT_CMD_FIS_RX | PORT_CMD_START |
                     PORT_CMD_FIS_ON | PORT_CMD_LIST_ON)))
            বিরতি;
        val &= ~(PORT_CMD_FIS_RX | PORT_CMD_START);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val);
        যদি (timer_check(end)) {
            warn_timeout();
            বিরতি;
        }
        yield();
    }

    /* disable + clear IRQs */
    ahci_port_writel(ctrl, pnr, PORT_IRQ_MASK, 0);
    val = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
    যদি (val)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, val);
}

স্থির কাঠামো ahci_port_s*
ahci_port_alloc(কাঠামো ahci_ctrl_s *ctrl, u32 pnr)
{
    কাঠামো ahci_port_s *port = malloc_tmp(আকার(*port));

    যদি (!port) {
        warn_noalloc();
        ফেরত ফাঁকা;
    }
    memset(port, 0, আকার(*port));
    port->pnr = pnr;
    port->ctrl = ctrl;
    port->list = memalign_tmp(1024, 1024);
    port->fis = memalign_tmp(256, 256);
    port->cmd = memalign_tmp(256, 256);
    যদি (port->list == ফাঁকা || port->fis == ফাঁকা || port->cmd == ফাঁকা) {
        warn_noalloc();
        ফেরত ফাঁকা;
    }
    memset(port->list, 0, 1024);
    memset(port->fis, 0, 256);
    memset(port->cmd, 0, 256);

    ahci_port_writel(ctrl, pnr, PORT_LST_ADDR, (u32)port->list);
    ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR, (u32)port->fis);
    যদি (ctrl->caps & HOST_CAP_64) {
        ahci_port_writel(ctrl, pnr, PORT_LST_ADDR_HI, 0);
        ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR_HI, 0);
    }

    ফেরত port;
}

স্থির শূন্য ahci_port_release(কাঠামো ahci_port_s *port)
{
    ahci_port_reset(port->ctrl, port->pnr);
    মুক্ত(port->list);
    মুক্ত(port->fis);
    মুক্ত(port->cmd);
    মুক্ত(port);
}

স্থির কাঠামো ahci_port_s* ahci_port_realloc(কাঠামো ahci_port_s *port)
{
    কাঠামো ahci_port_s *tmp;
    u32 cmd;

    tmp = malloc_fseg(আকার(*port));
    যদি (!tmp) {
        warn_noalloc();
        ahci_port_release(port);
        ফেরত ফাঁকা;
    }
    *tmp = *port;
    মুক্ত(port);
    port = tmp;

    ahci_port_reset(port->ctrl, port->pnr);

    মুক্ত(port->list);
    মুক্ত(port->fis);
    মুক্ত(port->cmd);
    port->list = memalign_উপরের(1024, 1024);
    port->fis = memalign_উপরের(256, 256);
    port->cmd = memalign_উপরের(256, 256);
    যদি (!port->list || !port->fis || !port->cmd) {
        warn_noalloc();
        মুক্ত(port->list);
        মুক্ত(port->fis);
        মুক্ত(port->cmd);
        মুক্ত(port);
        ফেরত ফাঁকা;
    }

    ahci_port_writel(port->ctrl, port->pnr, PORT_LST_ADDR, (u32)port->list);
    ahci_port_writel(port->ctrl, port->pnr, PORT_FIS_ADDR, (u32)port->fis);

    cmd = ahci_port_readl(port->ctrl, port->pnr, PORT_CMD);
    cmd |= (PORT_CMD_FIS_RX|PORT_CMD_START);
    ahci_port_writel(port->ctrl, port->pnr, PORT_CMD, cmd);

    ফেরত port;
}

#define MAXMODEL 40

/* See ahci spec chapter 10.1 "Software Initialization of HBA" */
স্থির পূর্ণসংখ্যা ahci_port_setup(কাঠামো ahci_port_s *port)
{
    কাঠামো ahci_ctrl_s *ctrl = port->ctrl;
    u32 pnr = port->pnr;
    অক্ষর model[MAXMODEL+1];
    u16 বাফার[256];
    u32 cmd, stat, err, tf;
    পূর্ণসংখ্যা rc;

    /* enable FIS recv */
    cmd = ahci_port_readl(ctrl, pnr, PORT_CMD);
    cmd |= PORT_CMD_FIS_RX;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    /* spin up */
    cmd &= ~PORT_CMD_ICC_MASK;
    cmd |= PORT_CMD_SPIN_UP | PORT_CMD_POWER_ON | PORT_CMD_ICC_ACTIVE;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);
    u32 end = timer_calc(AHCI_LINK_TIMEOUT);
    জন্য (;;) {
        stat = ahci_port_readl(ctrl, pnr, PORT_SCR_STAT);
        যদি ((stat & 0x07) == 0x03) {
            dprintf(2, "AHCI/%d: link up\n", port->pnr);
            বিরতি;
        }
        যদি (timer_check(end)) {
            dprintf(2, "AHCI/%d: link down\n", port->pnr);
            ফেরত -1;
        }
        yield();
    }

    /* clear error status */
    err = ahci_port_readl(ctrl, pnr, PORT_SCR_ERR);
    যদি (err)
        ahci_port_writel(ctrl, pnr, PORT_SCR_ERR, err);

    /* wait for device becoming ready */
    end = timer_calc(AHCI_REQUEST_TIMEOUT);
    জন্য (;;) {
        tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
        যদি (!(tf & (ATA_CB_STAT_BSY |
                    ATA_CB_STAT_DRQ)))
            বিরতি;
        যদি (timer_check(end)) {
            warn_timeout();
            dprintf(1, "AHCI/%d: device not ready (tf 0x%x)\n", port->pnr, tf);
            ফেরত -1;
        }
        yield();
    }

    /* start device */
    cmd |= PORT_CMD_START;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    rc = ahci_command(port, 0, 0, বাফার, আকার(বাফার));
    যদি (rc == 0) {
        port->atapi = 1;
    } অন্যথা {
        port->atapi = 0;
        sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
        rc = ahci_command(port, 0, 0, বাফার, আকার(বাফার));
        যদি (rc < 0)
            ফেরত -1;
    }

    port->drive.cntl_id = pnr;
    port->drive.removable = (বাফার[0] & 0x80) ? 1 : 0;

    যদি (!port->atapi) {
        // found disk (ata)
        port->drive.type = DTYPE_AHCI;
        port->drive.blksize = DISK_SECTOR_আকার;
        port->drive.pchs.cylinder = বাফার[1];
        port->drive.pchs.শীর্ষ = বাফার[3];
        port->drive.pchs.sector = বাফার[6];

        u64 sectors;
        যদি (বাফার[83] & (1 << 10)) // word 83 - lba48 support
            sectors = *(u64*)&বাফার[100]; // word 100-103
        অন্যথা
            sectors = *(u32*)&বাফার[60]; // word 60 and word 61
        port->drive.sectors = sectors;
        u64 adjsize = sectors >> 11;
        অক্ষর adjprefix = 'M';
        যদি (adjsize >= (1 << 16)) {
            adjsize >>= 10;
            adjprefix = 'G';
        }
        port->desc = znprintf(MAXDESCSIZE
                              , "AHCI/%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                              , port->pnr
                              , ata_extract_model(model, MAXMODEL, বাফার)
                              , ata_extract_version(বাফার)
                              , (u32)adjsize, adjprefix);
        port->prio = bootprio_find_ata_device(ctrl->pci_tmp, pnr, 0);

        s8 multi_dma = -1;
        s8 pio_mode = -1;
        s8 udma_mode = -1;
        // If bit 2 in word 53 is set, udma information is valid in word 88.
        যদি (বাফার[53] & 0x04) {
            udma_mode = 6;
            যখন ((udma_mode >= 0) &&
                   !((বাফার[88] & 0x7f) & ( 1 << udma_mode ))) {
                udma_mode--;
            }
        }
        // If bit 1 in word 53 is set, multiword-dma and advanced pio modes
        // are available in words 63 and 64.
        যদি (বাফার[53] & 0x02) {
            pio_mode = 4;
            multi_dma = 3;
            যখন ((multi_dma >= 0) &&
                   !((বাফার[63] & 0x7) & ( 1 << multi_dma ))) {
                multi_dma--;
            }
            যখন ((pio_mode >= 3) &&
                   !((বাফার[64] & 0x3) & ( 1 << ( pio_mode - 3 ) ))) {
                pio_mode--;
            }
        }
        dprintf(2, "AHCI/%d: supported modes: udma %d, multi-dma %d, pio %d\n",
                port->pnr, udma_mode, multi_dma, pio_mode);

        sata_prep_simple(&port->cmd->fis, ATA_CMD_SET_FEATURES);
        port->cmd->fis.feature = ATA_SET_FEATRUE_TRANSFER_MODE;
        // Select used mode. UDMA first, then Multi-DMA followed by
        // advanced PIO modes 3 or 4. If non, set default PIO.
        যদি (udma_mode >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to UDMA-%d\n",
                    port->pnr, udma_mode);
            port->cmd->fis.sector_গুনতি = ATA_TRANSFER_MODE_ULTRA_DMA
                                          | udma_mode;
        } অন্যথা যদি (multi_dma >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to Multi-DMA-%d\n",
                    port->pnr, multi_dma);
            port->cmd->fis.sector_গুনতি = ATA_TRANSFER_MODE_MULTIWORD_DMA
                                          | multi_dma;
        } অন্যথা যদি (pio_mode >= 3) {
            dprintf(1, "AHCI/%d: Set transfer mode to PIO-%d\n",
                    port->pnr, pio_mode);
            port->cmd->fis.sector_গুনতি = ATA_TRANSFER_MODE_PIO_FLOW_CTRL
                                          | pio_mode;
        } অন্যথা {
            dprintf(1, "AHCI/%d: Set transfer mode to default PIO\n",
                    port->pnr);
            port->cmd->fis.sector_গুনতি = ATA_TRANSFER_MODE_DEFAULT_PIO;
        }
        rc = ahci_command(port, 1, 0, 0, 0);
        যদি (rc < 0) {
            dprintf(1, "AHCI/%d: Set transfer mode failed.\n", port->pnr);
        }
    } অন্যথা {
        // found cdrom (atapi)
        port->drive.type = DTYPE_AHCI_ATAPI;
        port->drive.blksize = CDROM_SECTOR_আকার;
        port->drive.sectors = (u64)-1;
        u8 iscd = ((বাফার[0] >> 8) & 0x1f) == 0x05;
        যদি (!iscd) {
            dprintf(1, "AHCI/%d: atapi device isn't a cdrom\n", port->pnr);
            ফেরত -1;
        }
        port->desc = znprintf(MAXDESCSIZE
                              , "DVD/CD [AHCI/%d: %s ATAPI-%d DVD/CD]"
                              , port->pnr
                              , ata_extract_model(model, MAXMODEL, বাফার)
                              , ata_extract_version(বাফার));
        port->prio = bootprio_find_ata_device(ctrl->pci_tmp, pnr, 0);
    }
    boot_lchs_find_ata_device(ctrl->pci_tmp, pnr, 0, &(port->drive.lchs));
    ফেরত 0;
}

// Detect any drives attached to a given controller.
স্থির শূন্য
ahci_port_detect(শূন্য *তথ্য)
{
    কাঠামো ahci_port_s *port = তথ্য;
    পূর্ণসংখ্যা rc;

    dprintf(2, "AHCI/%d: probing\n", port->pnr);
    ahci_port_reset(port->ctrl, port->pnr);
    rc = ahci_port_setup(port);
    যদি (rc < 0)
        ahci_port_release(port);
    অন্যথা {
        port = ahci_port_realloc(port);
        যদি (port == ফাঁকা)
            ফেরত;
        dprintf(1, "AHCI/%d: registering: \"%s\"\n", port->pnr, port->desc);
        যদি (!port->atapi) {
            // Register with bcv system.
            boot_add_hd(&port->drive, port->desc, port->prio);
        } অন্যথা {
            // fill cdidmap
            boot_add_cd(&port->drive, port->desc, port->prio);
        }
    }
}

// Initialize an ata controller and detect its drives.
স্থির শূন্য
ahci_controller_setup(কাঠামো pci_device *pci)
{
    কাঠামো ahci_port_s *port;
    u32 pnr, max;

    যদি (create_bounce_buf() < 0)
        ফেরত;

    শূন্য *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_5);
    যদি (!iobase)
        ফেরত;

    কাঠামো ahci_ctrl_s *ctrl = malloc_fseg(আকার(*ctrl));
    যদি (!ctrl) {
        warn_noalloc();
        ফেরত;
    }

    ctrl->pci_tmp = pci;
    ctrl->iobase = iobase;
    ctrl->irq = pci_config_readb(pci->bdf, PCI_INTERRUPT_সারি);
    dprintf(1, "AHCI controller at %pP, iobase %p, irq %d\n"
            , pci, ctrl->iobase, ctrl->irq);

    pci_enable_busmaster(pci);

    u32 val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_RESET);
    u32 end = timer_calc(AHCI_RESET_TIMEOUT);
    জন্য (;;) {
        val = ahci_ctrl_readl(ctrl, HOST_CTL);
        যদি (!(val & HOST_CTL_RESET))
            বিরতি;
        যদি (timer_check(end)) {
            warn_timeout();
            dprintf(1, "AHCI: controller reset failed\n");
            মুক্ত(ctrl);
            ফেরত;
        }
        yield();
    }
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_AHCI_EN);
    (শূন্য)ahci_ctrl_readl(ctrl, HOST_CTL); /* Flush */

    ctrl->caps = ahci_ctrl_readl(ctrl, HOST_CAP);
    ctrl->ports = ahci_ctrl_readl(ctrl, HOST_PORTS_IMPL);
    dprintf(2, "AHCI: cap 0x%x, ports_impl 0x%x\n",
            ctrl->caps, ctrl->ports);

    max = 0x1f;
    জন্য (pnr = 0; pnr <= max; pnr++) {
        যদি (!(ctrl->ports & (1 << pnr)))
            চালু;
        port = ahci_port_alloc(ctrl, pnr);
        যদি (port == ফাঁকা)
            চালু;
        run_thread(ahci_port_detect, port);
    }
}

// Locate and init ahci controllers.
স্থির শূন্য
ahci_scan(শূন্য)
{
    // Scan PCI bus for ATA adapters
    কাঠামো pci_device *pci;
    foreachpci(pci) {
        যদি (pci->class != PCI_CLASS_STORAGE_SATA)
            চালু;
        যদি (pci->prog_if != 1 /* AHCI rev 1 */)
            চালু;
        ahci_controller_setup(pci);
    }
}

শূন্য
ahci_setup(শূন্য)
{
    ASSERT32FLAT();
    যদি (!CONFIG_AHCI)
        ফেরত;

    dprintf(3, "init ahci\n");
    ahci_scan();
}

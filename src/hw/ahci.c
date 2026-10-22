// Low level AHCI disk access
//
// Copyright (C) 2010 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "ahci.h" // CDB_CMD_READ_10
#అన్తర్భావ "ata.h" // ATA_CB_STAT
#అన్తర్భావ "biosvar.h" // GET_GLOBAL
#అన్తర్భావ "blockcmd.h" // CDB_CMD_READ_10
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pci.h" // pci_config_readb
#అన్తర్భావ "pcidevice.h" // foreachpci
#అన్తర్భావ "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#అన్తర్భావ "pci_regs.h" // PCI_INTERRUPT_LINE
#అన్తర్భావ "stacks.h" // yield
#అన్తర్భావ "std/disk.h" // DISK_RET_SUCCESS
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // timer_calc
#అన్తర్భావ "x86.h" // inb

#నిర్వచించు AHCI_REQUEST_TIMEOUT 32000 // 32 seconds max for IDE ops
#నిర్వచించు AHCI_RESET_TIMEOUT     500 // 500 miliseconds
#నిర్వచించు AHCI_LINK_TIMEOUT       10 // 10 miliseconds

// prepare sata command fis
స్థిర శూన్య sata_prep_simple(నిర్మాణం sata_cmd_fis *fis, u8 command)
{
    memset_fl(fis, 0, పరిమాణం(*fis));
    fis->command = command;
}

స్థిర శూన్య sata_prep_readwrite(నిర్మాణం sata_cmd_fis *fis,
                                నిర్మాణం disk_op_s *op, పూర్ణాంకం iswrite)
{
    u64 lba = op->lba;
    u8 command;

    memset_fl(fis, 0, పరిమాణం(*fis));

    ఒకవేళ (op->లెక్క >= (1<<8) || lba + op->లెక్క >= (1<<28)) {
        fis->sector_count2 = op->లెక్క >> 8;
        fis->lba_low2      = lba >> 24;
        fis->lba_mid2      = lba >> 32;
        fis->lba_high2     = lba >> 40;
        lba &= 0xffffff;
        command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                   : ATA_CMD_READ_DMA_EXT);
    } లేకపోతే {
        command = (iswrite ? ATA_CMD_WRITE_DMA
                   : ATA_CMD_READ_DMA);
    }
    fis->feature      = 1; /* dma */
    fis->command      = command;
    fis->sector_లెక్క = op->లెక్క;
    fis->lba_దిగువ      = lba;
    fis->lba_mid      = lba >> 8;
    fis->lba_ఎగువ     = lba >> 16;
    fis->device       = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;
}

స్థిర శూన్య sata_prep_atapi(నిర్మాణం sata_cmd_fis *fis, u16 blocksize)
{
    memset_fl(fis, 0, పరిమాణం(*fis));
    fis->command  = ATA_CMD_PACKET;
    fis->feature  = 1; /* dma */
    fis->lba_mid  = blocksize;
    fis->lba_ఎగువ = blocksize >> 8;
}

// ahci register access helpers
స్థిర u32 ahci_ctrl_readl(నిర్మాణం ahci_ctrl_s *ctrl, u32 reg)
{
    తిరిగి readl(ctrl->iobase + reg);
}

స్థిర శూన్య ahci_ctrl_writel(నిర్మాణం ahci_ctrl_s *ctrl, u32 reg, u32 val)
{
    writel(ctrl->iobase + reg, val);
}

స్థిర u32 ahci_పోర్టు_గమ్యం_ctrl(u32 pnr, u32 పోర్టు_reg)
{
    u32 ctrl_reg = 0x100;
    ctrl_reg += pnr * 0x80;
    ctrl_reg += పోర్టు_reg;
    తిరిగి ctrl_reg;
}

స్థిర u32 ahci_పోర్టు_readl(నిర్మాణం ahci_ctrl_s *ctrl, u32 pnr, u32 reg)
{
    u32 ctrl_reg = ahci_పోర్టు_గమ్యం_ctrl(pnr, reg);
    తిరిగి ahci_ctrl_readl(ctrl, ctrl_reg);
}

స్థిర శూన్య ahci_పోర్టు_writel(నిర్మాణం ahci_ctrl_s *ctrl, u32 pnr, u32 reg, u32 val)
{
    u32 ctrl_reg = ahci_పోర్టు_గమ్యం_ctrl(pnr, reg);
    ahci_ctrl_writel(ctrl, ctrl_reg, val);
}

// submit ahci command + wait for result
స్థిర పూర్ణాంకం ahci_command(నిర్మాణం ahci_పోర్టు_s *పోర్టు_gf, పూర్ణాంకం iswrite, పూర్ణాంకం isatapi,
                        శూన్య *బఫరు, u32 bsize)
{
    u32 val, స్థితి_2, success, flags, intbits, error;
    నిర్మాణం ahci_ctrl_s *ctrl = పోర్టు_gf->ctrl;
    నిర్మాణం ahci_cmd_s  *cmd  = పోర్టు_gf->cmd;
    నిర్మాణం ahci_fis_s  *fis  = పోర్టు_gf->fis;
    నిర్మాణం ahci_list_s *list = పోర్టు_gf->list;
    u32 pnr                  = పోర్టు_gf->pnr;

    cmd->fis.reg       = 0x27;
    cmd->fis.pmp_type  = 1 << 7; /* cmd fis */
    cmd->prdt[0].పునాది  = (u32)బఫరు;
    cmd->prdt[0].baseu = 0;
    cmd->prdt[0].flags = bsize-1;

    flags = ((1 << 16) | /* one prd entry */
             (iswrite ? (1 << 6) : 0) |
             (isatapi ? (1 << 5) : 0) |
             (5 << 0)); /* fis length (dwords) */
    list[0].flags  = flags;
    list[0].బైట్లు  = 0;
    list[0].పునాది   = (u32)(cmd);
    list[0].baseu  = 0;

    dprintf(8, "AHCI/%d: send cmd ...\n", pnr);
    intbits = ahci_పోర్టు_readl(ctrl, pnr, పోర్టు_IRQ_STAT);
    ఒకవేళ (intbits)
        ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_IRQ_STAT, intbits);
    ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_CMD_ISSUE, 1);

    u32 చివర = timer_calc(AHCI_REQUEST_TIMEOUT);
    చేయి {
        కొరకు (;;) {
            intbits = ahci_పోర్టు_readl(ctrl, pnr, పోర్టు_IRQ_STAT);
            ఒకవేళ (intbits) {
                ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_IRQ_STAT, intbits);
                ఒకవేళ (intbits & 0x40000000) {
                    u32 tf = ahci_పోర్టు_readl(ctrl, pnr, పోర్టు_TFDATA);
                    స్థితి_2 = tf & 0xff;
                    error = (tf & 0xff00) >> 8;
                    ఆపు;
                }
                ఒకవేళ (intbits & 0x02) {
                    స్థితి_2 = GET_LOWFLAT(fis->psfis[2]);
                    error  = GET_LOWFLAT(fis->psfis[3]);
                    ఆపు;
                }
                ఒకవేళ (intbits & 0x01) {
                    స్థితి_2 = GET_LOWFLAT(fis->rfis[2]);
                    error  = GET_LOWFLAT(fis->rfis[3]);
                    ఆపు;
                }
            }
            ఒకవేళ (timer_check(చివర)) {
                warn_timeout();
                తిరిగి -1;
            }
            yield();
        }
        dprintf(8, "AHCI/%d: ... intbits 0x%x, status 0x%x ...\n",
                pnr, intbits, స్థితి_2);
    } అయితే (స్థితి_2 & ATA_CB_STAT_BSY);

    success = (0x00 == (స్థితి_2 & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF |
                                  ATA_CB_STAT_ERR)) &&
               ATA_CB_STAT_RDY == (స్థితి_2 & (ATA_CB_STAT_RDY)));
    ఒకవేళ (success) {
        dprintf(8, "AHCI/%d: ... finished, status 0x%x, OK\n", pnr,
                స్థితి_2);
    } లేకపోతే {
        dprintf(2, "AHCI/%d: ... finished, status 0x%x, ERROR 0x%x\n", pnr,
                స్థితి_2, error);

        // non-queued error recovery (AHCI 1.3 section 6.2.2.1)
        // Clears PxCMD.ST to 0 to reset the PxCI register
        val = ahci_పోర్టు_readl(ctrl, pnr, పోర్టు_CMD);
        ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_CMD, val & ~పోర్టు_CMD_ప్రారంభం);

        // waits for PxCMD.CR to clear to 0
        అయితే (1) {
            val = ahci_పోర్టు_readl(ctrl, pnr, పోర్టు_CMD);
            ఒకవేళ ((val & పోర్టు_CMD_LIST_ON) == 0)
                ఆపు;
            yield();
        }

        // Clears any error bits in PxSERR to enable capturing new errors
        val = ahci_పోర్టు_readl(ctrl, pnr, పోర్టు_SCR_ERR);
        ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_SCR_ERR, val);

        // Clears status bits in PxIS as appropriate
        val = ahci_పోర్టు_readl(ctrl, pnr, పోర్టు_IRQ_STAT);
        ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_IRQ_STAT, val);

        // If PxTFD.STS.BSY or PxTFD.STS.DRQ is set to 1, issue
        // a COMRESET to the device to put it in an idle state
        val = ahci_పోర్టు_readl(ctrl, pnr, పోర్టు_TFDATA);
        ఒకవేళ (val & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ)) {
            dprintf(2, "AHCI/%d: issue comreset\n", pnr);
            val = ahci_పోర్టు_readl(ctrl, pnr, పోర్టు_SCR_CTL);
            // set Device Detection Initialization (DET) to 1 for 1 ms for comreset
            ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_SCR_CTL, val | 1);
            mdelay (1);
            ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_SCR_CTL, val);
        }

        // Sets PxCMD.ST to 1 to enable issuing new commands
        val = ahci_పోర్టు_readl(ctrl, pnr, పోర్టు_CMD);
        ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_CMD, val | పోర్టు_CMD_ప్రారంభం);
    }
    తిరిగి success ? 0 : -1;
}

#నిర్వచించు CDROM_CDB_కొలత 12

పూర్ణాంకం ahci_atapi_process_op(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (! CONFIG_AHCI)
        తిరిగి 0;

    నిర్మాణం ahci_పోర్టు_s *పోర్టు_gf = container_యొక్క(
        op->drive_fl, నిర్మాణం ahci_పోర్టు_s, drive);
    నిర్మాణం ahci_cmd_s *cmd = పోర్టు_gf->cmd;

    ఒకవేళ (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        తిరిగి DISK_RET_EWRITEPROTECT;
    పూర్ణాంకం blocksize = scsi_fill_cmd(op, cmd->atapi, CDROM_CDB_కొలత);
    ఒకవేళ (blocksize < 0)
        తిరిగి default_process_op(op);
    sata_prep_atapi(&cmd->fis, blocksize);
    పూర్ణాంకం rc = ahci_command(పోర్టు_gf, 0, 1, op->buf_fl, op->లెక్క * blocksize);
    ఒకవేళ (rc < 0)
        తిరిగి DISK_RET_EBADTRACK;
    తిరిగి DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive, op->buf_fl must be word aligned
స్థిర పూర్ణాంకం
ahci_disk_readwrite_aligned(నిర్మాణం disk_op_s *op, పూర్ణాంకం iswrite)
{
    నిర్మాణం ahci_పోర్టు_s *పోర్టు_gf = container_యొక్క(
        op->drive_fl, నిర్మాణం ahci_పోర్టు_s, drive);
    నిర్మాణం ahci_cmd_s *cmd = పోర్టు_gf->cmd;
    పూర్ణాంకం rc;

    sata_prep_readwrite(&cmd->fis, op, iswrite);
    rc = ahci_command(పోర్టు_gf, iswrite, 0, op->buf_fl,
                      op->లెక్క * DISK_SECTOR_కొలత);
    dprintf(8, "ahci disk %s, lba %6x, count %3x, buf %p, rc %d\n",
            iswrite ? "write" : "read", (u32)op->lba, op->లెక్క, op->buf_fl, rc);
    ఒకవేళ (rc < 0)
        తిరిగి DISK_RET_EBADTRACK;
    తిరిగి DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive.
స్థిర పూర్ణాంకం
ahci_disk_readwrite(నిర్మాణం disk_op_s *op, పూర్ణాంకం iswrite)
{
    // if caller's buffer is word aligned, use it directly
    ఒకవేళ (((u32) op->buf_fl & 1) == 0)
        తిరిగి ahci_disk_readwrite_aligned(op, iswrite);

    // Use a word aligned buffer for AHCI I/O
    పూర్ణాంకం rc;
    నిర్మాణం disk_op_s localop = *op;
    u8 *alignedbuf_fl = bounce_buf_fl;
    u8 *position = op->buf_fl;

    localop.buf_fl = alignedbuf_fl;
    localop.లెక్క = 1;

    ఒకవేళ (iswrite) {
        u16 ఖండం;
        కొరకు (ఖండం = 0; ఖండం < op->లెక్క; ఖండం++) {
            memcpy_fl (alignedbuf_fl, position, DISK_SECTOR_కొలత);
            rc = ahci_disk_readwrite_aligned (&localop, 1);
            ఒకవేళ (rc)
                తిరిగి rc;
            position += DISK_SECTOR_కొలత;
            localop.lba++;
        }
    } లేకపోతే { // read
        u16 ఖండం;
        కొరకు (ఖండం = 0; ఖండం < op->లెక్క; ఖండం++) {
            rc = ahci_disk_readwrite_aligned (&localop, 0);
            ఒకవేళ (rc)
                తిరిగి rc;
            memcpy_fl (position, alignedbuf_fl, DISK_SECTOR_కొలత);
            position += DISK_SECTOR_కొలత;
            localop.lba++;
        }
    }
    తిరిగి DISK_RET_SUCCESS;
}

// command demuxer
పూర్ణాంకం
ahci_process_op(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (!CONFIG_AHCI)
        తిరిగి 0;
    ఎంపిక (op->command) {
    స్థితి CMD_READ:
        తిరిగి ahci_disk_readwrite(op, 0);
    స్థితి CMD_WRITE:
        తిరిగి ahci_disk_readwrite(op, 1);
    అప్రమేయ:
        తిరిగి default_process_op(op);
    }
}

స్థిర శూన్య
ahci_పోర్టు_reset(నిర్మాణం ahci_ctrl_s *ctrl, u32 pnr)
{
    u32 val;

    /* disable FIS + CMD */
    u32 చివర = timer_calc(AHCI_RESET_TIMEOUT);
    కొరకు (;;) {
        val = ahci_పోర్టు_readl(ctrl, pnr, పోర్టు_CMD);
        ఒకవేళ (!(val & (పోర్టు_CMD_FIS_RX | పోర్టు_CMD_ప్రారంభం |
                     పోర్టు_CMD_FIS_ON | పోర్టు_CMD_LIST_ON)))
            ఆపు;
        val &= ~(పోర్టు_CMD_FIS_RX | పోర్టు_CMD_ప్రారంభం);
        ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_CMD, val);
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            ఆపు;
        }
        yield();
    }

    /* disable + clear IRQs */
    ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_IRQ_ముసుగు, 0);
    val = ahci_పోర్టు_readl(ctrl, pnr, పోర్టు_IRQ_STAT);
    ఒకవేళ (val)
        ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_IRQ_STAT, val);
}

స్థిర నిర్మాణం ahci_పోర్టు_s*
ahci_పోర్టు_alloc(నిర్మాణం ahci_ctrl_s *ctrl, u32 pnr)
{
    నిర్మాణం ahci_పోర్టు_s *పోర్టు = malloc_tmp(పరిమాణం(*పోర్టు));

    ఒకవేళ (!పోర్టు) {
        warn_noalloc();
        తిరిగి ఖాళీ;
    }
    memset(పోర్టు, 0, పరిమాణం(*పోర్టు));
    పోర్టు->pnr = pnr;
    పోర్టు->ctrl = ctrl;
    పోర్టు->list = memalign_tmp(1024, 1024);
    పోర్టు->fis = memalign_tmp(256, 256);
    పోర్టు->cmd = memalign_tmp(256, 256);
    ఒకవేళ (పోర్టు->list == ఖాళీ || పోర్టు->fis == ఖాళీ || పోర్టు->cmd == ఖాళీ) {
        warn_noalloc();
        తిరిగి ఖాళీ;
    }
    memset(పోర్టు->list, 0, 1024);
    memset(పోర్టు->fis, 0, 256);
    memset(పోర్టు->cmd, 0, 256);

    ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_LST_ADDR, (u32)పోర్టు->list);
    ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_FIS_ADDR, (u32)పోర్టు->fis);
    ఒకవేళ (ctrl->caps & HOST_CAP_64) {
        ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_LST_ADDR_HI, 0);
        ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_FIS_ADDR_HI, 0);
    }

    తిరిగి పోర్టు;
}

స్థిర శూన్య ahci_పోర్టు_release(నిర్మాణం ahci_పోర్టు_s *పోర్టు)
{
    ahci_పోర్టు_reset(పోర్టు->ctrl, పోర్టు->pnr);
    విడుదల(పోర్టు->list);
    విడుదల(పోర్టు->fis);
    విడుదల(పోర్టు->cmd);
    విడుదల(పోర్టు);
}

స్థిర నిర్మాణం ahci_పోర్టు_s* ahci_పోర్టు_realloc(నిర్మాణం ahci_పోర్టు_s *పోర్టు)
{
    నిర్మాణం ahci_పోర్టు_s *tmp;
    u32 cmd;

    tmp = malloc_fseg(పరిమాణం(*పోర్టు));
    ఒకవేళ (!tmp) {
        warn_noalloc();
        ahci_పోర్టు_release(పోర్టు);
        తిరిగి ఖాళీ;
    }
    *tmp = *పోర్టు;
    విడుదల(పోర్టు);
    పోర్టు = tmp;

    ahci_పోర్టు_reset(పోర్టు->ctrl, పోర్టు->pnr);

    విడుదల(పోర్టు->list);
    విడుదల(పోర్టు->fis);
    విడుదల(పోర్టు->cmd);
    పోర్టు->list = memalign_ఎగువ(1024, 1024);
    పోర్టు->fis = memalign_ఎగువ(256, 256);
    పోర్టు->cmd = memalign_ఎగువ(256, 256);
    ఒకవేళ (!పోర్టు->list || !పోర్టు->fis || !పోర్టు->cmd) {
        warn_noalloc();
        విడుదల(పోర్టు->list);
        విడుదల(పోర్టు->fis);
        విడుదల(పోర్టు->cmd);
        విడుదల(పోర్టు);
        తిరిగి ఖాళీ;
    }

    ahci_పోర్టు_writel(పోర్టు->ctrl, పోర్టు->pnr, పోర్టు_LST_ADDR, (u32)పోర్టు->list);
    ahci_పోర్టు_writel(పోర్టు->ctrl, పోర్టు->pnr, పోర్టు_FIS_ADDR, (u32)పోర్టు->fis);

    cmd = ahci_పోర్టు_readl(పోర్టు->ctrl, పోర్టు->pnr, పోర్టు_CMD);
    cmd |= (పోర్టు_CMD_FIS_RX|పోర్టు_CMD_ప్రారంభం);
    ahci_పోర్టు_writel(పోర్టు->ctrl, పోర్టు->pnr, పోర్టు_CMD, cmd);

    తిరిగి పోర్టు;
}

#నిర్వచించు MAXMODEL 40

/* See ahci spec chapter 10.1 "Software Initialization of HBA" */
స్థిర పూర్ణాంకం ahci_పోర్టు_అమరిక(నిర్మాణం ahci_పోర్టు_s *పోర్టు)
{
    నిర్మాణం ahci_ctrl_s *ctrl = పోర్టు->ctrl;
    u32 pnr = పోర్టు->pnr;
    అక్షరం model[MAXMODEL+1];
    u16 బఫరు[256];
    u32 cmd, stat, err, tf;
    పూర్ణాంకం rc;

    /* enable FIS recv */
    cmd = ahci_పోర్టు_readl(ctrl, pnr, పోర్టు_CMD);
    cmd |= పోర్టు_CMD_FIS_RX;
    ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_CMD, cmd);

    /* spin up */
    cmd &= ~పోర్టు_CMD_ICC_ముసుగు;
    cmd |= పోర్టు_CMD_SPIN_పైకి | పోర్టు_CMD_POWER_ON | పోర్టు_CMD_ICC_చురుకైన;
    ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_CMD, cmd);
    u32 చివర = timer_calc(AHCI_LINK_TIMEOUT);
    కొరకు (;;) {
        stat = ahci_పోర్టు_readl(ctrl, pnr, పోర్టు_SCR_STAT);
        ఒకవేళ ((stat & 0x07) == 0x03) {
            dprintf(2, "AHCI/%d: link up\n", పోర్టు->pnr);
            ఆపు;
        }
        ఒకవేళ (timer_check(చివర)) {
            dprintf(2, "AHCI/%d: link down\n", పోర్టు->pnr);
            తిరిగి -1;
        }
        yield();
    }

    /* clear error status */
    err = ahci_పోర్టు_readl(ctrl, pnr, పోర్టు_SCR_ERR);
    ఒకవేళ (err)
        ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_SCR_ERR, err);

    /* wait for device becoming ready */
    చివర = timer_calc(AHCI_REQUEST_TIMEOUT);
    కొరకు (;;) {
        tf = ahci_పోర్టు_readl(ctrl, pnr, పోర్టు_TFDATA);
        ఒకవేళ (!(tf & (ATA_CB_STAT_BSY |
                    ATA_CB_STAT_DRQ)))
            ఆపు;
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            dprintf(1, "AHCI/%d: device not ready (tf 0x%x)\n", పోర్టు->pnr, tf);
            తిరిగి -1;
        }
        yield();
    }

    /* start device */
    cmd |= పోర్టు_CMD_ప్రారంభం;
    ahci_పోర్టు_writel(ctrl, pnr, పోర్టు_CMD, cmd);

    sata_prep_simple(&పోర్టు->cmd->fis, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    rc = ahci_command(పోర్టు, 0, 0, బఫరు, పరిమాణం(బఫరు));
    ఒకవేళ (rc == 0) {
        పోర్టు->atapi = 1;
    } లేకపోతే {
        పోర్టు->atapi = 0;
        sata_prep_simple(&పోర్టు->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
        rc = ahci_command(పోర్టు, 0, 0, బఫరు, పరిమాణం(బఫరు));
        ఒకవేళ (rc < 0)
            తిరిగి -1;
    }

    పోర్టు->drive.cntl_id = pnr;
    పోర్టు->drive.removable = (బఫరు[0] & 0x80) ? 1 : 0;

    ఒకవేళ (!పోర్టు->atapi) {
        // found disk (ata)
        పోర్టు->drive.type = DTYPE_AHCI;
        పోర్టు->drive.blksize = DISK_SECTOR_కొలత;
        పోర్టు->drive.pchs.cylinder = బఫరు[1];
        పోర్టు->drive.pchs.తల = బఫరు[3];
        పోర్టు->drive.pchs.sector = బఫరు[6];

        u64 sectors;
        ఒకవేళ (బఫరు[83] & (1 << 10)) // word 83 - lba48 support
            sectors = *(u64*)&బఫరు[100]; // word 100-103
        లేకపోతే
            sectors = *(u32*)&బఫరు[60]; // word 60 and word 61
        పోర్టు->drive.sectors = sectors;
        u64 adjsize = sectors >> 11;
        అక్షరం adjprefix = 'M';
        ఒకవేళ (adjsize >= (1 << 16)) {
            adjsize >>= 10;
            adjprefix = 'G';
        }
        పోర్టు->desc = znprintf(MAXDESCSIZE
                              , "AHCI/%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                              , పోర్టు->pnr
                              , ata_వెలికితీయి_model(model, MAXMODEL, బఫరు)
                              , ata_వెలికితీయి_సంస్కరణ(బఫరు)
                              , (u32)adjsize, adjprefix);
        పోర్టు->prio = bootprio_కనుగొను_ata_device(ctrl->pci_tmp, pnr, 0);

        s8 multi_dma = -1;
        s8 pio_బహుళకం = -1;
        s8 udma_బహుళకం = -1;
        // If bit 2 in word 53 is set, udma information is valid in word 88.
        ఒకవేళ (బఫరు[53] & 0x04) {
            udma_బహుళకం = 6;
            అయితే ((udma_బహుళకం >= 0) &&
                   !((బఫరు[88] & 0x7f) & ( 1 << udma_బహుళకం ))) {
                udma_బహుళకం--;
            }
        }
        // If bit 1 in word 53 is set, multiword-dma and advanced pio modes
        // are available in words 63 and 64.
        ఒకవేళ (బఫరు[53] & 0x02) {
            pio_బహుళకం = 4;
            multi_dma = 3;
            అయితే ((multi_dma >= 0) &&
                   !((బఫరు[63] & 0x7) & ( 1 << multi_dma ))) {
                multi_dma--;
            }
            అయితే ((pio_బహుళకం >= 3) &&
                   !((బఫరు[64] & 0x3) & ( 1 << ( pio_బహుళకం - 3 ) ))) {
                pio_బహుళకం--;
            }
        }
        dprintf(2, "AHCI/%d: supported modes: udma %d, multi-dma %d, pio %d\n",
                పోర్టు->pnr, udma_బహుళకం, multi_dma, pio_బహుళకం);

        sata_prep_simple(&పోర్టు->cmd->fis, ATA_CMD_సమితి_FEATURES);
        పోర్టు->cmd->fis.feature = ATA_సమితి_FEATRUE_TRANSFER_బహుళకం;
        // Select used mode. UDMA first, then Multi-DMA followed by
        // advanced PIO modes 3 or 4. If non, set default PIO.
        ఒకవేళ (udma_బహుళకం >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to UDMA-%d\n",
                    పోర్టు->pnr, udma_బహుళకం);
            పోర్టు->cmd->fis.sector_లెక్క = ATA_TRANSFER_బహుళకం_ULTRA_DMA
                                          | udma_బహుళకం;
        } లేకపోతే ఒకవేళ (multi_dma >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to Multi-DMA-%d\n",
                    పోర్టు->pnr, multi_dma);
            పోర్టు->cmd->fis.sector_లెక్క = ATA_TRANSFER_బహుళకం_MULTIWORD_DMA
                                          | multi_dma;
        } లేకపోతే ఒకవేళ (pio_బహుళకం >= 3) {
            dprintf(1, "AHCI/%d: Set transfer mode to PIO-%d\n",
                    పోర్టు->pnr, pio_బహుళకం);
            పోర్టు->cmd->fis.sector_లెక్క = ATA_TRANSFER_బహుళకం_PIO_FLOW_CTRL
                                          | pio_బహుళకం;
        } లేకపోతే {
            dprintf(1, "AHCI/%d: Set transfer mode to default PIO\n",
                    పోర్టు->pnr);
            పోర్టు->cmd->fis.sector_లెక్క = ATA_TRANSFER_బహుళకం_DEFAULT_PIO;
        }
        rc = ahci_command(పోర్టు, 1, 0, 0, 0);
        ఒకవేళ (rc < 0) {
            dprintf(1, "AHCI/%d: Set transfer mode failed.\n", పోర్టు->pnr);
        }
    } లేకపోతే {
        // found cdrom (atapi)
        పోర్టు->drive.type = DTYPE_AHCI_ATAPI;
        పోర్టు->drive.blksize = CDROM_SECTOR_కొలత;
        పోర్టు->drive.sectors = (u64)-1;
        u8 iscd = ((బఫరు[0] >> 8) & 0x1f) == 0x05;
        ఒకవేళ (!iscd) {
            dprintf(1, "AHCI/%d: atapi device isn't a cdrom\n", పోర్టు->pnr);
            తిరిగి -1;
        }
        పోర్టు->desc = znprintf(MAXDESCSIZE
                              , "DVD/CD [AHCI/%d: %s ATAPI-%d DVD/CD]"
                              , పోర్టు->pnr
                              , ata_వెలికితీయి_model(model, MAXMODEL, బఫరు)
                              , ata_వెలికితీయి_సంస్కరణ(బఫరు));
        పోర్టు->prio = bootprio_కనుగొను_ata_device(ctrl->pci_tmp, pnr, 0);
    }
    boot_lchs_కనుగొను_ata_device(ctrl->pci_tmp, pnr, 0, &(పోర్టు->drive.lchs));
    తిరిగి 0;
}

// Detect any drives attached to a given controller.
స్థిర శూన్య
ahci_పోర్టు_detect(శూన్య *దత్తాంశం)
{
    నిర్మాణం ahci_పోర్టు_s *పోర్టు = దత్తాంశం;
    పూర్ణాంకం rc;

    dprintf(2, "AHCI/%d: probing\n", పోర్టు->pnr);
    ahci_పోర్టు_reset(పోర్టు->ctrl, పోర్టు->pnr);
    rc = ahci_పోర్టు_అమరిక(పోర్టు);
    ఒకవేళ (rc < 0)
        ahci_పోర్టు_release(పోర్టు);
    లేకపోతే {
        పోర్టు = ahci_పోర్టు_realloc(పోర్టు);
        ఒకవేళ (పోర్టు == ఖాళీ)
            తిరిగి;
        dprintf(1, "AHCI/%d: registering: \"%s\"\n", పోర్టు->pnr, పోర్టు->desc);
        ఒకవేళ (!పోర్టు->atapi) {
            // Register with bcv system.
            boot_కూడు_hd(&పోర్టు->drive, పోర్టు->desc, పోర్టు->prio);
        } లేకపోతే {
            // fill cdidmap
            boot_కూడు_cd(&పోర్టు->drive, పోర్టు->desc, పోర్టు->prio);
        }
    }
}

// Initialize an ata controller and detect its drives.
స్థిర శూన్య
ahci_controller_అమరిక(నిర్మాణం pci_device *pci)
{
    నిర్మాణం ahci_పోర్టు_s *పోర్టు;
    u32 pnr, max;

    ఒకవేళ (create_bounce_buf() < 0)
        తిరిగి;

    శూన్య *iobase = pci_enable_membar(pci, PCI_పునాది_చిరునామా_5);
    ఒకవేళ (!iobase)
        తిరిగి;

    నిర్మాణం ahci_ctrl_s *ctrl = malloc_fseg(పరిమాణం(*ctrl));
    ఒకవేళ (!ctrl) {
        warn_noalloc();
        తిరిగి;
    }

    ctrl->pci_tmp = pci;
    ctrl->iobase = iobase;
    ctrl->irq = pci_config_readb(pci->bdf, PCI_అంతరాయం_పంక్తి);
    dprintf(1, "AHCI controller at %pP, iobase %p, irq %d\n"
            , pci, ctrl->iobase, ctrl->irq);

    pci_enable_busmaster(pci);

    u32 val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_RESET);
    u32 చివర = timer_calc(AHCI_RESET_TIMEOUT);
    కొరకు (;;) {
        val = ahci_ctrl_readl(ctrl, HOST_CTL);
        ఒకవేళ (!(val & HOST_CTL_RESET))
            ఆపు;
        ఒకవేళ (timer_check(చివర)) {
            warn_timeout();
            dprintf(1, "AHCI: controller reset failed\n");
            విడుదల(ctrl);
            తిరిగి;
        }
        yield();
    }
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_AHCI_EN);
    (శూన్య)ahci_ctrl_readl(ctrl, HOST_CTL); /* Flush */

    ctrl->caps = ahci_ctrl_readl(ctrl, HOST_CAP);
    ctrl->ports = ahci_ctrl_readl(ctrl, HOST_PORTS_IMPL);
    dprintf(2, "AHCI: cap 0x%x, ports_impl 0x%x\n",
            ctrl->caps, ctrl->ports);

    max = 0x1f;
    కొరకు (pnr = 0; pnr <= max; pnr++) {
        ఒకవేళ (!(ctrl->ports & (1 << pnr)))
            కొనసాగు;
        పోర్టు = ahci_పోర్టు_alloc(ctrl, pnr);
        ఒకవేళ (పోర్టు == ఖాళీ)
            కొనసాగు;
        నిరంతరం_thread(ahci_పోర్టు_detect, పోర్టు);
    }
}

// Locate and init ahci controllers.
స్థిర శూన్య
ahci_పరిశీలన(శూన్య)
{
    // Scan PCI bus for ATA adapters
    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (pci->class != PCI_CLASS_STORAGE_SATA)
            కొనసాగు;
        ఒకవేళ (pci->prog_if != 1 /* AHCI rev 1 */)
            కొనసాగు;
        ahci_controller_అమరిక(pci);
    }
}

శూన్య
ahci_అమరిక(శూన్య)
{
    ASSERT32FLAT();
    ఒకవేళ (!CONFIG_AHCI)
        తిరిగి;

    dprintf(3, "init ahci\n");
    ahci_పరిశీలన();
}

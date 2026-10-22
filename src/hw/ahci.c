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

#परिभाषय AHCI_REQUEST_TIMEOUT 32000 // 32 seconds max for IDE ops
#परिभाषय AHCI_RESET_TIMEOUT     500 // 500 miliseconds
#परिभाषय AHCI_LINK_TIMEOUT       10 // 10 miliseconds

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

    यदि (op->गणनम् >= (1<<8) || lba + op->गणनम् >= (1<<28)) {
        fis->sector_count2 = op->गणनम् >> 8;
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
    fis->sector_गणनम् = op->गणनम्;
    fis->lba_नीचः      = lba;
    fis->lba_mid      = lba >> 8;
    fis->lba_उच्चः     = lba >> 16;
    fis->device       = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;
}

स्थिर शून्य sata_prep_atapi(संरचना sata_cmd_fis *fis, u16 blocksize)
{
    memset_fl(fis, 0, परिमाण(*fis));
    fis->command  = ATA_CMD_PACKET;
    fis->feature  = 1; /* dma */
    fis->lba_mid  = blocksize;
    fis->lba_उच्चः = blocksize >> 8;
}

// ahci register access helpers
स्थिर u32 ahci_ctrl_readl(संरचना ahci_ctrl_s *ctrl, u32 reg)
{
    प्रत्यागम readl(ctrl->iobase + reg);
}

स्थिर शून्य ahci_ctrl_writel(संरचना ahci_ctrl_s *ctrl, u32 reg, u32 val)
{
    writel(ctrl->iobase + reg, val);
}

स्थिर u32 ahci_पत्तनम्_गन्तव्यम्_ctrl(u32 pnr, u32 पत्तनम्_reg)
{
    u32 ctrl_reg = 0x100;
    ctrl_reg += pnr * 0x80;
    ctrl_reg += पत्तनम्_reg;
    प्रत्यागम ctrl_reg;
}

स्थिर u32 ahci_पत्तनम्_readl(संरचना ahci_ctrl_s *ctrl, u32 pnr, u32 reg)
{
    u32 ctrl_reg = ahci_पत्तनम्_गन्तव्यम्_ctrl(pnr, reg);
    प्रत्यागम ahci_ctrl_readl(ctrl, ctrl_reg);
}

स्थिर शून्य ahci_पत्तनम्_writel(संरचना ahci_ctrl_s *ctrl, u32 pnr, u32 reg, u32 val)
{
    u32 ctrl_reg = ahci_पत्तनम्_गन्तव्यम्_ctrl(pnr, reg);
    ahci_ctrl_writel(ctrl, ctrl_reg, val);
}

// submit ahci command + wait for result
स्थिर पूर्णाङ्क ahci_command(संरचना ahci_पत्तनम्_s *पत्तनम्_gf, पूर्णाङ्क iswrite, पूर्णाङ्क isatapi,
                        शून्य *बफर, u32 bsize)
{
    u32 val, स्थितिः, success, flags, intbits, error;
    संरचना ahci_ctrl_s *ctrl = पत्तनम्_gf->ctrl;
    संरचना ahci_cmd_s  *cmd  = पत्तनम्_gf->cmd;
    संरचना ahci_fis_s  *fis  = पत्तनम्_gf->fis;
    संरचना ahci_list_s *list = पत्तनम्_gf->list;
    u32 pnr                  = पत्तनम्_gf->pnr;

    cmd->fis.reg       = 0x27;
    cmd->fis.pmp_type  = 1 << 7; /* cmd fis */
    cmd->prdt[0].आधारः  = (u32)बफर;
    cmd->prdt[0].baseu = 0;
    cmd->prdt[0].flags = bsize-1;

    flags = ((1 << 16) | /* one prd entry */
             (iswrite ? (1 << 6) : 0) |
             (isatapi ? (1 << 5) : 0) |
             (5 << 0)); /* fis length (dwords) */
    list[0].flags  = flags;
    list[0].अष्टकानि  = 0;
    list[0].आधारः   = (u32)(cmd);
    list[0].baseu  = 0;

    dprintf(8, "AHCI/%d: send cmd ...\n", pnr);
    intbits = ahci_पत्तनम्_readl(ctrl, pnr, पत्तनम्_IRQ_STAT);
    यदि (intbits)
        ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_IRQ_STAT, intbits);
    ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_CMD_ISSUE, 1);

    u32 अन्तः = timer_calc(AHCI_REQUEST_TIMEOUT);
    कुरु {
        प्रति (;;) {
            intbits = ahci_पत्तनम्_readl(ctrl, pnr, पत्तनम्_IRQ_STAT);
            यदि (intbits) {
                ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_IRQ_STAT, intbits);
                यदि (intbits & 0x40000000) {
                    u32 tf = ahci_पत्तनम्_readl(ctrl, pnr, पत्तनम्_TFDATA);
                    स्थितिः = tf & 0xff;
                    error = (tf & 0xff00) >> 8;
                    भङ्ग;
                }
                यदि (intbits & 0x02) {
                    स्थितिः = GET_LOWFLAT(fis->psfis[2]);
                    error  = GET_LOWFLAT(fis->psfis[3]);
                    भङ्ग;
                }
                यदि (intbits & 0x01) {
                    स्थितिः = GET_LOWFLAT(fis->rfis[2]);
                    error  = GET_LOWFLAT(fis->rfis[3]);
                    भङ्ग;
                }
            }
            यदि (timer_check(अन्तः)) {
                warn_timeout();
                प्रत्यागम -1;
            }
            yield();
        }
        dprintf(8, "AHCI/%d: ... intbits 0x%x, status 0x%x ...\n",
                pnr, intbits, स्थितिः);
    } यावत् (स्थितिः & ATA_CB_STAT_BSY);

    success = (0x00 == (स्थितिः & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF |
                                  ATA_CB_STAT_ERR)) &&
               ATA_CB_STAT_RDY == (स्थितिः & (ATA_CB_STAT_RDY)));
    यदि (success) {
        dprintf(8, "AHCI/%d: ... finished, status 0x%x, OK\n", pnr,
                स्थितिः);
    } अन्यथा {
        dprintf(2, "AHCI/%d: ... finished, status 0x%x, ERROR 0x%x\n", pnr,
                स्थितिः, error);

        // non-queued error recovery (AHCI 1.3 section 6.2.2.1)
        // Clears PxCMD.ST to 0 to reset the PxCI register
        val = ahci_पत्तनम्_readl(ctrl, pnr, पत्तनम्_CMD);
        ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_CMD, val & ~पत्तनम्_CMD_START);

        // waits for PxCMD.CR to clear to 0
        यावत् (1) {
            val = ahci_पत्तनम्_readl(ctrl, pnr, पत्तनम्_CMD);
            यदि ((val & पत्तनम्_CMD_LIST_ON) == 0)
                भङ्ग;
            yield();
        }

        // Clears any error bits in PxSERR to enable capturing new errors
        val = ahci_पत्तनम्_readl(ctrl, pnr, पत्तनम्_SCR_ERR);
        ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_SCR_ERR, val);

        // Clears status bits in PxIS as appropriate
        val = ahci_पत्तनम्_readl(ctrl, pnr, पत्तनम्_IRQ_STAT);
        ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_IRQ_STAT, val);

        // If PxTFD.STS.BSY or PxTFD.STS.DRQ is set to 1, issue
        // a COMRESET to the device to put it in an idle state
        val = ahci_पत्तनम्_readl(ctrl, pnr, पत्तनम्_TFDATA);
        यदि (val & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ)) {
            dprintf(2, "AHCI/%d: issue comreset\n", pnr);
            val = ahci_पत्तनम्_readl(ctrl, pnr, पत्तनम्_SCR_CTL);
            // set Device Detection Initialization (DET) to 1 for 1 ms for comreset
            ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_SCR_CTL, val | 1);
            mdelay (1);
            ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_SCR_CTL, val);
        }

        // Sets PxCMD.ST to 1 to enable issuing new commands
        val = ahci_पत्तनम्_readl(ctrl, pnr, पत्तनम्_CMD);
        ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_CMD, val | पत्तनम्_CMD_START);
    }
    प्रत्यागम success ? 0 : -1;
}

#परिभाषय CDROM_CDB_आकार 12

पूर्णाङ्क ahci_atapi_process_op(संरचना disk_op_s *op)
{
    यदि (! CONFIG_AHCI)
        प्रत्यागम 0;

    संरचना ahci_पत्तनम्_s *पत्तनम्_gf = container_सम्बन्धि(
        op->drive_fl, संरचना ahci_पत्तनम्_s, drive);
    संरचना ahci_cmd_s *cmd = पत्तनम्_gf->cmd;

    यदि (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        प्रत्यागम DISK_RET_EWRITEPROTECT;
    पूर्णाङ्क blocksize = scsi_fill_cmd(op, cmd->atapi, CDROM_CDB_आकार);
    यदि (blocksize < 0)
        प्रत्यागम default_process_op(op);
    sata_prep_atapi(&cmd->fis, blocksize);
    पूर्णाङ्क rc = ahci_command(पत्तनम्_gf, 0, 1, op->buf_fl, op->गणनम् * blocksize);
    यदि (rc < 0)
        प्रत्यागम DISK_RET_EBADTRACK;
    प्रत्यागम DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive, op->buf_fl must be word aligned
स्थिर पूर्णाङ्क
ahci_disk_readwrite_aligned(संरचना disk_op_s *op, पूर्णाङ्क iswrite)
{
    संरचना ahci_पत्तनम्_s *पत्तनम्_gf = container_सम्बन्धि(
        op->drive_fl, संरचना ahci_पत्तनम्_s, drive);
    संरचना ahci_cmd_s *cmd = पत्तनम्_gf->cmd;
    पूर्णाङ्क rc;

    sata_prep_readwrite(&cmd->fis, op, iswrite);
    rc = ahci_command(पत्तनम्_gf, iswrite, 0, op->buf_fl,
                      op->गणनम् * DISK_SECTOR_आकार);
    dprintf(8, "ahci disk %s, lba %6x, count %3x, buf %p, rc %d\n",
            iswrite ? "write" : "read", (u32)op->lba, op->गणनम्, op->buf_fl, rc);
    यदि (rc < 0)
        प्रत्यागम DISK_RET_EBADTRACK;
    प्रत्यागम DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive.
स्थिर पूर्णाङ्क
ahci_disk_readwrite(संरचना disk_op_s *op, पूर्णाङ्क iswrite)
{
    // if caller's buffer is word aligned, use it directly
    यदि (((u32) op->buf_fl & 1) == 0)
        प्रत्यागम ahci_disk_readwrite_aligned(op, iswrite);

    // Use a word aligned buffer for AHCI I/O
    पूर्णाङ्क rc;
    संरचना disk_op_s localop = *op;
    u8 *alignedbuf_fl = bounce_buf_fl;
    u8 *position = op->buf_fl;

    localop.buf_fl = alignedbuf_fl;
    localop.गणनम् = 1;

    यदि (iswrite) {
        u16 खण्डम्;
        प्रति (खण्डम् = 0; खण्डम् < op->गणनम्; खण्डम्++) {
            memcpy_fl (alignedbuf_fl, position, DISK_SECTOR_आकार);
            rc = ahci_disk_readwrite_aligned (&localop, 1);
            यदि (rc)
                प्रत्यागम rc;
            position += DISK_SECTOR_आकार;
            localop.lba++;
        }
    } अन्यथा { // read
        u16 खण्डम्;
        प्रति (खण्डम् = 0; खण्डम् < op->गणनम्; खण्डम्++) {
            rc = ahci_disk_readwrite_aligned (&localop, 0);
            यदि (rc)
                प्रत्यागम rc;
            memcpy_fl (position, alignedbuf_fl, DISK_SECTOR_आकार);
            position += DISK_SECTOR_आकार;
            localop.lba++;
        }
    }
    प्रत्यागम DISK_RET_SUCCESS;
}

// command demuxer
पूर्णाङ्क
ahci_process_op(संरचना disk_op_s *op)
{
    यदि (!CONFIG_AHCI)
        प्रत्यागम 0;
    विकल्प (op->command) {
    पक्ष CMD_READ:
        प्रत्यागम ahci_disk_readwrite(op, 0);
    पक्ष CMD_WRITE:
        प्रत्यागम ahci_disk_readwrite(op, 1);
    स्वभाव:
        प्रत्यागम default_process_op(op);
    }
}

स्थिर शून्य
ahci_पत्तनम्_reset(संरचना ahci_ctrl_s *ctrl, u32 pnr)
{
    u32 val;

    /* disable FIS + CMD */
    u32 अन्तः = timer_calc(AHCI_RESET_TIMEOUT);
    प्रति (;;) {
        val = ahci_पत्तनम्_readl(ctrl, pnr, पत्तनम्_CMD);
        यदि (!(val & (पत्तनम्_CMD_FIS_RX | पत्तनम्_CMD_START |
                     पत्तनम्_CMD_FIS_ON | पत्तनम्_CMD_LIST_ON)))
            भङ्ग;
        val &= ~(पत्तनम्_CMD_FIS_RX | पत्तनम्_CMD_START);
        ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_CMD, val);
        यदि (timer_check(अन्तः)) {
            warn_timeout();
            भङ्ग;
        }
        yield();
    }

    /* disable + clear IRQs */
    ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_IRQ_आवरणम्, 0);
    val = ahci_पत्तनम्_readl(ctrl, pnr, पत्तनम्_IRQ_STAT);
    यदि (val)
        ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_IRQ_STAT, val);
}

स्थिर संरचना ahci_पत्तनम्_s*
ahci_पत्तनम्_alloc(संरचना ahci_ctrl_s *ctrl, u32 pnr)
{
    संरचना ahci_पत्तनम्_s *पत्तनम् = malloc_tmp(परिमाण(*पत्तनम्));

    यदि (!पत्तनम्) {
        warn_noalloc();
        प्रत्यागम रिक्त;
    }
    स्मृतिं_पूरय(पत्तनम्, 0, परिमाण(*पत्तनम्));
    पत्तनम्->pnr = pnr;
    पत्तनम्->ctrl = ctrl;
    पत्तनम्->list = memalign_tmp(1024, 1024);
    पत्तनम्->fis = memalign_tmp(256, 256);
    पत्तनम्->cmd = memalign_tmp(256, 256);
    यदि (पत्तनम्->list == रिक्त || पत्तनम्->fis == रिक्त || पत्तनम्->cmd == रिक्त) {
        warn_noalloc();
        प्रत्यागम रिक्त;
    }
    स्मृतिं_पूरय(पत्तनम्->list, 0, 1024);
    स्मृतिं_पूरय(पत्तनम्->fis, 0, 256);
    स्मृतिं_पूरय(पत्तनम्->cmd, 0, 256);

    ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_LST_ADDR, (u32)पत्तनम्->list);
    ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_FIS_ADDR, (u32)पत्तनम्->fis);
    यदि (ctrl->caps & HOST_CAP_64) {
        ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_LST_ADDR_HI, 0);
        ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_FIS_ADDR_HI, 0);
    }

    प्रत्यागम पत्तनम्;
}

स्थिर शून्य ahci_पत्तनम्_release(संरचना ahci_पत्तनम्_s *पत्तनम्)
{
    ahci_पत्तनम्_reset(पत्तनम्->ctrl, पत्तनम्->pnr);
    मुक्त(पत्तनम्->list);
    मुक्त(पत्तनम्->fis);
    मुक्त(पत्तनम्->cmd);
    मुक्त(पत्तनम्);
}

स्थिर संरचना ahci_पत्तनम्_s* ahci_पत्तनम्_realloc(संरचना ahci_पत्तनम्_s *पत्तनम्)
{
    संरचना ahci_पत्तनम्_s *tmp;
    u32 cmd;

    tmp = malloc_fseg(परिमाण(*पत्तनम्));
    यदि (!tmp) {
        warn_noalloc();
        ahci_पत्तनम्_release(पत्तनम्);
        प्रत्यागम रिक्त;
    }
    *tmp = *पत्तनम्;
    मुक्त(पत्तनम्);
    पत्तनम् = tmp;

    ahci_पत्तनम्_reset(पत्तनम्->ctrl, पत्तनम्->pnr);

    मुक्त(पत्तनम्->list);
    मुक्त(पत्तनम्->fis);
    मुक्त(पत्तनम्->cmd);
    पत्तनम्->list = memalign_उच्चः(1024, 1024);
    पत्तनम्->fis = memalign_उच्चः(256, 256);
    पत्तनम्->cmd = memalign_उच्चः(256, 256);
    यदि (!पत्तनम्->list || !पत्तनम्->fis || !पत्तनम्->cmd) {
        warn_noalloc();
        मुक्त(पत्तनम्->list);
        मुक्त(पत्तनम्->fis);
        मुक्त(पत्तनम्->cmd);
        मुक्त(पत्तनम्);
        प्रत्यागम रिक्त;
    }

    ahci_पत्तनम्_writel(पत्तनम्->ctrl, पत्तनम्->pnr, पत्तनम्_LST_ADDR, (u32)पत्तनम्->list);
    ahci_पत्तनम्_writel(पत्तनम्->ctrl, पत्तनम्->pnr, पत्तनम्_FIS_ADDR, (u32)पत्तनम्->fis);

    cmd = ahci_पत्तनम्_readl(पत्तनम्->ctrl, पत्तनम्->pnr, पत्तनम्_CMD);
    cmd |= (पत्तनम्_CMD_FIS_RX|पत्तनम्_CMD_START);
    ahci_पत्तनम्_writel(पत्तनम्->ctrl, पत्तनम्->pnr, पत्तनम्_CMD, cmd);

    प्रत्यागम पत्तनम्;
}

#परिभाषय MAXMODEL 40

/* See ahci spec chapter 10.1 "Software Initialization of HBA" */
स्थिर पूर्णाङ्क ahci_पत्तनम्_सज्जता(संरचना ahci_पत्तनम्_s *पत्तनम्)
{
    संरचना ahci_ctrl_s *ctrl = पत्तनम्->ctrl;
    u32 pnr = पत्तनम्->pnr;
    वर्ण model[MAXMODEL+1];
    u16 बफर[256];
    u32 cmd, stat, err, tf;
    पूर्णाङ्क rc;

    /* enable FIS recv */
    cmd = ahci_पत्तनम्_readl(ctrl, pnr, पत्तनम्_CMD);
    cmd |= पत्तनम्_CMD_FIS_RX;
    ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_CMD, cmd);

    /* spin up */
    cmd &= ~पत्तनम्_CMD_ICC_आवरणम्;
    cmd |= पत्तनम्_CMD_SPIN_ऊर्ध्वम् | पत्तनम्_CMD_POWER_ON | पत्तनम्_CMD_ICC_सक्रिय;
    ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_CMD, cmd);
    u32 अन्तः = timer_calc(AHCI_LINK_TIMEOUT);
    प्रति (;;) {
        stat = ahci_पत्तनम्_readl(ctrl, pnr, पत्तनम्_SCR_STAT);
        यदि ((stat & 0x07) == 0x03) {
            dprintf(2, "AHCI/%d: link up\n", पत्तनम्->pnr);
            भङ्ग;
        }
        यदि (timer_check(अन्तः)) {
            dprintf(2, "AHCI/%d: link down\n", पत्तनम्->pnr);
            प्रत्यागम -1;
        }
        yield();
    }

    /* clear error status */
    err = ahci_पत्तनम्_readl(ctrl, pnr, पत्तनम्_SCR_ERR);
    यदि (err)
        ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_SCR_ERR, err);

    /* wait for device becoming ready */
    अन्तः = timer_calc(AHCI_REQUEST_TIMEOUT);
    प्रति (;;) {
        tf = ahci_पत्तनम्_readl(ctrl, pnr, पत्तनम्_TFDATA);
        यदि (!(tf & (ATA_CB_STAT_BSY |
                    ATA_CB_STAT_DRQ)))
            भङ्ग;
        यदि (timer_check(अन्तः)) {
            warn_timeout();
            dprintf(1, "AHCI/%d: device not ready (tf 0x%x)\n", पत्तनम्->pnr, tf);
            प्रत्यागम -1;
        }
        yield();
    }

    /* start device */
    cmd |= पत्तनम्_CMD_START;
    ahci_पत्तनम्_writel(ctrl, pnr, पत्तनम्_CMD, cmd);

    sata_prep_simple(&पत्तनम्->cmd->fis, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    rc = ahci_command(पत्तनम्, 0, 0, बफर, परिमाण(बफर));
    यदि (rc == 0) {
        पत्तनम्->atapi = 1;
    } अन्यथा {
        पत्तनम्->atapi = 0;
        sata_prep_simple(&पत्तनम्->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
        rc = ahci_command(पत्तनम्, 0, 0, बफर, परिमाण(बफर));
        यदि (rc < 0)
            प्रत्यागम -1;
    }

    पत्तनम्->drive.cntl_id = pnr;
    पत्तनम्->drive.removable = (बफर[0] & 0x80) ? 1 : 0;

    यदि (!पत्तनम्->atapi) {
        // found disk (ata)
        पत्तनम्->drive.type = DTYPE_AHCI;
        पत्तनम्->drive.blksize = DISK_SECTOR_आकार;
        पत्तनम्->drive.pchs.cylinder = बफर[1];
        पत्तनम्->drive.pchs.शीर्ष = बफर[3];
        पत्तनम्->drive.pchs.sector = बफर[6];

        u64 sectors;
        यदि (बफर[83] & (1 << 10)) // word 83 - lba48 support
            sectors = *(u64*)&बफर[100]; // word 100-103
        अन्यथा
            sectors = *(u32*)&बफर[60]; // word 60 and word 61
        पत्तनम्->drive.sectors = sectors;
        u64 adjsize = sectors >> 11;
        वर्ण adjprefix = 'M';
        यदि (adjsize >= (1 << 16)) {
            adjsize >>= 10;
            adjprefix = 'G';
        }
        पत्तनम्->desc = znprintf(MAXDESCSIZE
                              , "AHCI/%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                              , पत्तनम्->pnr
                              , ata_extract_model(model, MAXMODEL, बफर)
                              , ata_extract_संस्करणम्(बफर)
                              , (u32)adjsize, adjprefix);
        पत्तनम्->prio = bootprio_find_ata_device(ctrl->pci_tmp, pnr, 0);

        s8 multi_dma = -1;
        s8 pio_बहुलकम् = -1;
        s8 udma_बहुलकम् = -1;
        // If bit 2 in word 53 is set, udma information is valid in word 88.
        यदि (बफर[53] & 0x04) {
            udma_बहुलकम् = 6;
            यावत् ((udma_बहुलकम् >= 0) &&
                   !((बफर[88] & 0x7f) & ( 1 << udma_बहुलकम् ))) {
                udma_बहुलकम्--;
            }
        }
        // If bit 1 in word 53 is set, multiword-dma and advanced pio modes
        // are available in words 63 and 64.
        यदि (बफर[53] & 0x02) {
            pio_बहुलकम् = 4;
            multi_dma = 3;
            यावत् ((multi_dma >= 0) &&
                   !((बफर[63] & 0x7) & ( 1 << multi_dma ))) {
                multi_dma--;
            }
            यावत् ((pio_बहुलकम् >= 3) &&
                   !((बफर[64] & 0x3) & ( 1 << ( pio_बहुलकम् - 3 ) ))) {
                pio_बहुलकम्--;
            }
        }
        dprintf(2, "AHCI/%d: supported modes: udma %d, multi-dma %d, pio %d\n",
                पत्तनम्->pnr, udma_बहुलकम्, multi_dma, pio_बहुलकम्);

        sata_prep_simple(&पत्तनम्->cmd->fis, ATA_CMD_समुच्चयः_FEATURES);
        पत्तनम्->cmd->fis.feature = ATA_समुच्चयः_FEATRUE_TRANSFER_बहुलकम्;
        // Select used mode. UDMA first, then Multi-DMA followed by
        // advanced PIO modes 3 or 4. If non, set default PIO.
        यदि (udma_बहुलकम् >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to UDMA-%d\n",
                    पत्तनम्->pnr, udma_बहुलकम्);
            पत्तनम्->cmd->fis.sector_गणनम् = ATA_TRANSFER_बहुलकम्_ULTRA_DMA
                                          | udma_बहुलकम्;
        } अन्यथा यदि (multi_dma >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to Multi-DMA-%d\n",
                    पत्तनम्->pnr, multi_dma);
            पत्तनम्->cmd->fis.sector_गणनम् = ATA_TRANSFER_बहुलकम्_MULTIWORD_DMA
                                          | multi_dma;
        } अन्यथा यदि (pio_बहुलकम् >= 3) {
            dprintf(1, "AHCI/%d: Set transfer mode to PIO-%d\n",
                    पत्तनम्->pnr, pio_बहुलकम्);
            पत्तनम्->cmd->fis.sector_गणनम् = ATA_TRANSFER_बहुलकम्_PIO_FLOW_CTRL
                                          | pio_बहुलकम्;
        } अन्यथा {
            dprintf(1, "AHCI/%d: Set transfer mode to default PIO\n",
                    पत्तनम्->pnr);
            पत्तनम्->cmd->fis.sector_गणनम् = ATA_TRANSFER_बहुलकम्_DEFAULT_PIO;
        }
        rc = ahci_command(पत्तनम्, 1, 0, 0, 0);
        यदि (rc < 0) {
            dprintf(1, "AHCI/%d: Set transfer mode failed.\n", पत्तनम्->pnr);
        }
    } अन्यथा {
        // found cdrom (atapi)
        पत्तनम्->drive.type = DTYPE_AHCI_ATAPI;
        पत्तनम्->drive.blksize = CDROM_SECTOR_आकार;
        पत्तनम्->drive.sectors = (u64)-1;
        u8 iscd = ((बफर[0] >> 8) & 0x1f) == 0x05;
        यदि (!iscd) {
            dprintf(1, "AHCI/%d: atapi device isn't a cdrom\n", पत्तनम्->pnr);
            प्रत्यागम -1;
        }
        पत्तनम्->desc = znprintf(MAXDESCSIZE
                              , "DVD/CD [AHCI/%d: %s ATAPI-%d DVD/CD]"
                              , पत्तनम्->pnr
                              , ata_extract_model(model, MAXMODEL, बफर)
                              , ata_extract_संस्करणम्(बफर));
        पत्तनम्->prio = bootprio_find_ata_device(ctrl->pci_tmp, pnr, 0);
    }
    boot_lchs_find_ata_device(ctrl->pci_tmp, pnr, 0, &(पत्तनम्->drive.lchs));
    प्रत्यागम 0;
}

// Detect any drives attached to a given controller.
स्थिर शून्य
ahci_पत्तनम्_detect(शून्य *दत्त)
{
    संरचना ahci_पत्तनम्_s *पत्तनम् = दत्त;
    पूर्णाङ्क rc;

    dprintf(2, "AHCI/%d: probing\n", पत्तनम्->pnr);
    ahci_पत्तनम्_reset(पत्तनम्->ctrl, पत्तनम्->pnr);
    rc = ahci_पत्तनम्_सज्जता(पत्तनम्);
    यदि (rc < 0)
        ahci_पत्तनम्_release(पत्तनम्);
    अन्यथा {
        पत्तनम् = ahci_पत्तनम्_realloc(पत्तनम्);
        यदि (पत्तनम् == रिक्त)
            प्रत्यागम;
        dprintf(1, "AHCI/%d: registering: \"%s\"\n", पत्तनम्->pnr, पत्तनम्->desc);
        यदि (!पत्तनम्->atapi) {
            // Register with bcv system.
            boot_सङ्कलय_hd(&पत्तनम्->drive, पत्तनम्->desc, पत्तनम्->prio);
        } अन्यथा {
            // fill cdidmap
            boot_सङ्कलय_cd(&पत्तनम्->drive, पत्तनम्->desc, पत्तनम्->prio);
        }
    }
}

// Initialize an ata controller and detect its drives.
स्थिर शून्य
ahci_controller_सज्जता(संरचना pci_device *pci)
{
    संरचना ahci_पत्तनम्_s *पत्तनम्;
    u32 pnr, max;

    यदि (create_bounce_buf() < 0)
        प्रत्यागम;

    शून्य *iobase = pci_enable_membar(pci, PCI_आधारः_स्थानाङ्कः_5);
    यदि (!iobase)
        प्रत्यागम;

    संरचना ahci_ctrl_s *ctrl = malloc_fseg(परिमाण(*ctrl));
    यदि (!ctrl) {
        warn_noalloc();
        प्रत्यागम;
    }

    ctrl->pci_tmp = pci;
    ctrl->iobase = iobase;
    ctrl->irq = pci_config_readb(pci->bdf, PCI_व्यवधानम्_पङ्क्तिः);
    dprintf(1, "AHCI controller at %pP, iobase %p, irq %d\n"
            , pci, ctrl->iobase, ctrl->irq);

    pci_enable_busmaster(pci);

    u32 val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_RESET);
    u32 अन्तः = timer_calc(AHCI_RESET_TIMEOUT);
    प्रति (;;) {
        val = ahci_ctrl_readl(ctrl, HOST_CTL);
        यदि (!(val & HOST_CTL_RESET))
            भङ्ग;
        यदि (timer_check(अन्तः)) {
            warn_timeout();
            dprintf(1, "AHCI: controller reset failed\n");
            मुक्त(ctrl);
            प्रत्यागम;
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
    प्रति (pnr = 0; pnr <= max; pnr++) {
        यदि (!(ctrl->ports & (1 << pnr)))
            अनुवर्त;
        पत्तनम् = ahci_पत्तनम्_alloc(ctrl, pnr);
        यदि (पत्तनम् == रिक्त)
            अनुवर्त;
        सततम्_thread(ahci_पत्तनम्_detect, पत्तनम्);
    }
}

// Locate and init ahci controllers.
स्थिर शून्य
ahci_अवलोकनम्(शून्य)
{
    // Scan PCI bus for ATA adapters
    संरचना pci_device *pci;
    foreachpci(pci) {
        यदि (pci->class != PCI_CLASS_STORAGE_SATA)
            अनुवर्त;
        यदि (pci->prog_if != 1 /* AHCI rev 1 */)
            अनुवर्त;
        ahci_controller_सज्जता(pci);
    }
}

शून्य
ahci_सज्जता(शून्य)
{
    ASSERT32FLAT();
    यदि (!CONFIG_AHCI)
        प्रत्यागम;

    dprintf(3, "init ahci\n");
    ahci_अवलोकनम्();
}

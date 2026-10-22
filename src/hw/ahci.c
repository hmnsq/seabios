// Low level AHCI disk access
//
// Copyright (C) 2010 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "ahci.h" // CDB_CMD_READ_10
#ਅਨ੍ਤਰ੍ਭਾਵ "ata.h" // ATA_CB_STAT
#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBAL
#ਅਨ੍ਤਰ੍ਭਾਵ "blockcmd.h" // CDB_CMD_READ_10
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pci.h" // pci_config_readb
#ਅਨ੍ਤਰ੍ਭਾਵ "pcidevice.h" // foreachpci
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_regs.h" // PCI_INTERRUPT_LINE
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // yield
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // DISK_RET_SUCCESS
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // timer_calc
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // inb

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AHCI_REQUEST_TIMEOUT 32000 // 32 seconds max for IDE ops
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AHCI_RESET_TIMEOUT     500 // 500 miliseconds
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AHCI_LINK_TIMEOUT       10 // 10 miliseconds

// prepare sata command fis
ਸਥਿਰ ਖਾਲੀ sata_prep_simple(ਬਣਤਰ sata_cmd_fis *fis, u8 command)
{
    memset_fl(fis, 0, ਆਕਾਰ(*fis));
    fis->command = command;
}

ਸਥਿਰ ਖਾਲੀ sata_prep_readwrite(ਬਣਤਰ sata_cmd_fis *fis,
                                ਬਣਤਰ disk_op_s *op, ਪੂਰਨ iswrite)
{
    u64 lba = op->lba;
    u8 command;

    memset_fl(fis, 0, ਆਕਾਰ(*fis));

    ਜੇ (op->ਗਿਣਤੀ_2 >= (1<<8) || lba + op->ਗਿਣਤੀ_2 >= (1<<28)) {
        fis->sector_count2 = op->ਗਿਣਤੀ_2 >> 8;
        fis->lba_low2      = lba >> 24;
        fis->lba_mid2      = lba >> 32;
        fis->lba_high2     = lba >> 40;
        lba &= 0xffffff;
        command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                   : ATA_CMD_READ_DMA_EXT);
    } ਨਹੀਂਤਾਂ {
        command = (iswrite ? ATA_CMD_WRITE_DMA
                   : ATA_CMD_READ_DMA);
    }
    fis->feature      = 1; /* dma */
    fis->command      = command;
    fis->sector_ਗਿਣਤੀ = op->ਗਿਣਤੀ_2;
    fis->lba_ਨੀਵਾਂ      = lba;
    fis->lba_mid      = lba >> 8;
    fis->lba_ਉੱਚਾ     = lba >> 16;
    fis->device       = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;
}

ਸਥਿਰ ਖਾਲੀ sata_prep_atapi(ਬਣਤਰ sata_cmd_fis *fis, u16 blocksize)
{
    memset_fl(fis, 0, ਆਕਾਰ(*fis));
    fis->command  = ATA_CMD_PACKET;
    fis->feature  = 1; /* dma */
    fis->lba_mid  = blocksize;
    fis->lba_ਉੱਚਾ = blocksize >> 8;
}

// ahci register access helpers
ਸਥਿਰ u32 ahci_ctrl_readl(ਬਣਤਰ ahci_ctrl_s *ctrl, u32 reg)
{
    ਵਾਪਸ readl(ctrl->iobase + reg);
}

ਸਥਿਰ ਖਾਲੀ ahci_ctrl_writel(ਬਣਤਰ ahci_ctrl_s *ctrl, u32 reg, u32 val)
{
    writel(ctrl->iobase + reg, val);
}

ਸਥਿਰ u32 ahci_ਪੋਰਟ_ਮੰਜ਼ਿਲ_ctrl(u32 pnr, u32 ਪੋਰਟ_reg)
{
    u32 ctrl_reg = 0x100;
    ctrl_reg += pnr * 0x80;
    ctrl_reg += ਪੋਰਟ_reg;
    ਵਾਪਸ ctrl_reg;
}

ਸਥਿਰ u32 ahci_ਪੋਰਟ_readl(ਬਣਤਰ ahci_ctrl_s *ctrl, u32 pnr, u32 reg)
{
    u32 ctrl_reg = ahci_ਪੋਰਟ_ਮੰਜ਼ਿਲ_ctrl(pnr, reg);
    ਵਾਪਸ ahci_ctrl_readl(ctrl, ctrl_reg);
}

ਸਥਿਰ ਖਾਲੀ ahci_ਪੋਰਟ_writel(ਬਣਤਰ ahci_ctrl_s *ctrl, u32 pnr, u32 reg, u32 val)
{
    u32 ctrl_reg = ahci_ਪੋਰਟ_ਮੰਜ਼ਿਲ_ctrl(pnr, reg);
    ahci_ctrl_writel(ctrl, ctrl_reg, val);
}

// submit ahci command + wait for result
ਸਥਿਰ ਪੂਰਨ ahci_command(ਬਣਤਰ ahci_ਪੋਰਟ_s *ਪੋਰਟ_gf, ਪੂਰਨ iswrite, ਪੂਰਨ isatapi,
                        ਖਾਲੀ *ਬਫਰ, u32 bsize)
{
    u32 val, ਸਥਿਤੀ, success, flags, intbits, error;
    ਬਣਤਰ ahci_ctrl_s *ctrl = ਪੋਰਟ_gf->ctrl;
    ਬਣਤਰ ahci_cmd_s  *cmd  = ਪੋਰਟ_gf->cmd;
    ਬਣਤਰ ahci_fis_s  *fis  = ਪੋਰਟ_gf->fis;
    ਬਣਤਰ ahci_list_s *list = ਪੋਰਟ_gf->list;
    u32 pnr                  = ਪੋਰਟ_gf->pnr;

    cmd->fis.reg       = 0x27;
    cmd->fis.pmp_type  = 1 << 7; /* cmd fis */
    cmd->prdt[0].ਅਧਾਰ  = (u32)ਬਫਰ;
    cmd->prdt[0].baseu = 0;
    cmd->prdt[0].flags = bsize-1;

    flags = ((1 << 16) | /* one prd entry */
             (iswrite ? (1 << 6) : 0) |
             (isatapi ? (1 << 5) : 0) |
             (5 << 0)); /* fis length (dwords) */
    list[0].flags  = flags;
    list[0].ਬਾਈਟਾਂ  = 0;
    list[0].ਅਧਾਰ   = (u32)(cmd);
    list[0].baseu  = 0;

    dprintf(8, "AHCI/%d: send cmd ...\n", pnr);
    intbits = ahci_ਪੋਰਟ_readl(ctrl, pnr, ਪੋਰਟ_IRQ_STAT);
    ਜੇ (intbits)
        ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_IRQ_STAT, intbits);
    ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_CMD_ISSUE, 1);

    u32 ਸਿਰਾ = timer_calc(AHCI_REQUEST_TIMEOUT);
    ਕਰੋ {
        ਲਈ (;;) {
            intbits = ahci_ਪੋਰਟ_readl(ctrl, pnr, ਪੋਰਟ_IRQ_STAT);
            ਜੇ (intbits) {
                ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_IRQ_STAT, intbits);
                ਜੇ (intbits & 0x40000000) {
                    u32 tf = ahci_ਪੋਰਟ_readl(ctrl, pnr, ਪੋਰਟ_TFDATA);
                    ਸਥਿਤੀ = tf & 0xff;
                    error = (tf & 0xff00) >> 8;
                    ਰੋਕੋ;
                }
                ਜੇ (intbits & 0x02) {
                    ਸਥਿਤੀ = GET_LOWFLAT(fis->psfis[2]);
                    error  = GET_LOWFLAT(fis->psfis[3]);
                    ਰੋਕੋ;
                }
                ਜੇ (intbits & 0x01) {
                    ਸਥਿਤੀ = GET_LOWFLAT(fis->rfis[2]);
                    error  = GET_LOWFLAT(fis->rfis[3]);
                    ਰੋਕੋ;
                }
            }
            ਜੇ (timer_check(ਸਿਰਾ)) {
                warn_timeout();
                ਵਾਪਸ -1;
            }
            yield();
        }
        dprintf(8, "AHCI/%d: ... intbits 0x%x, status 0x%x ...\n",
                pnr, intbits, ਸਥਿਤੀ);
    } ਜਦੋਂ (ਸਥਿਤੀ & ATA_CB_STAT_BSY);

    success = (0x00 == (ਸਥਿਤੀ & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF |
                                  ATA_CB_STAT_ERR)) &&
               ATA_CB_STAT_RDY == (ਸਥਿਤੀ & (ATA_CB_STAT_RDY)));
    ਜੇ (success) {
        dprintf(8, "AHCI/%d: ... finished, status 0x%x, OK\n", pnr,
                ਸਥਿਤੀ);
    } ਨਹੀਂਤਾਂ {
        dprintf(2, "AHCI/%d: ... finished, status 0x%x, ERROR 0x%x\n", pnr,
                ਸਥਿਤੀ, error);

        // non-queued error recovery (AHCI 1.3 section 6.2.2.1)
        // Clears PxCMD.ST to 0 to reset the PxCI register
        val = ahci_ਪੋਰਟ_readl(ctrl, pnr, ਪੋਰਟ_CMD);
        ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_CMD, val & ~ਪੋਰਟ_CMD_ਸ਼ੁਰੂ);

        // waits for PxCMD.CR to clear to 0
        ਜਦੋਂ (1) {
            val = ahci_ਪੋਰਟ_readl(ctrl, pnr, ਪੋਰਟ_CMD);
            ਜੇ ((val & ਪੋਰਟ_CMD_LIST_ON) == 0)
                ਰੋਕੋ;
            yield();
        }

        // Clears any error bits in PxSERR to enable capturing new errors
        val = ahci_ਪੋਰਟ_readl(ctrl, pnr, ਪੋਰਟ_SCR_ERR);
        ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_SCR_ERR, val);

        // Clears status bits in PxIS as appropriate
        val = ahci_ਪੋਰਟ_readl(ctrl, pnr, ਪੋਰਟ_IRQ_STAT);
        ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_IRQ_STAT, val);

        // If PxTFD.STS.BSY or PxTFD.STS.DRQ is set to 1, issue
        // a COMRESET to the device to put it in an idle state
        val = ahci_ਪੋਰਟ_readl(ctrl, pnr, ਪੋਰਟ_TFDATA);
        ਜੇ (val & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ)) {
            dprintf(2, "AHCI/%d: issue comreset\n", pnr);
            val = ahci_ਪੋਰਟ_readl(ctrl, pnr, ਪੋਰਟ_SCR_CTL);
            // set Device Detection Initialization (DET) to 1 for 1 ms for comreset
            ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_SCR_CTL, val | 1);
            mdelay (1);
            ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_SCR_CTL, val);
        }

        // Sets PxCMD.ST to 1 to enable issuing new commands
        val = ahci_ਪੋਰਟ_readl(ctrl, pnr, ਪੋਰਟ_CMD);
        ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_CMD, val | ਪੋਰਟ_CMD_ਸ਼ੁਰੂ);
    }
    ਵਾਪਸ success ? 0 : -1;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CDROM_CDB_ਮਾਪ 12

ਪੂਰਨ ahci_atapi_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (! CONFIG_AHCI)
        ਵਾਪਸ 0;

    ਬਣਤਰ ahci_ਪੋਰਟ_s *ਪੋਰਟ_gf = container_ਦਾ(
        op->drive_fl, ਬਣਤਰ ahci_ਪੋਰਟ_s, drive);
    ਬਣਤਰ ahci_cmd_s *cmd = ਪੋਰਟ_gf->cmd;

    ਜੇ (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        ਵਾਪਸ DISK_RET_EWRITEPROTECT;
    ਪੂਰਨ blocksize = scsi_fill_cmd(op, cmd->atapi, CDROM_CDB_ਮਾਪ);
    ਜੇ (blocksize < 0)
        ਵਾਪਸ default_process_op(op);
    sata_prep_atapi(&cmd->fis, blocksize);
    ਪੂਰਨ rc = ahci_command(ਪੋਰਟ_gf, 0, 1, op->buf_fl, op->ਗਿਣਤੀ_2 * blocksize);
    ਜੇ (rc < 0)
        ਵਾਪਸ DISK_RET_EBADTRACK;
    ਵਾਪਸ DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive, op->buf_fl must be word aligned
ਸਥਿਰ ਪੂਰਨ
ahci_disk_readwrite_aligned(ਬਣਤਰ disk_op_s *op, ਪੂਰਨ iswrite)
{
    ਬਣਤਰ ahci_ਪੋਰਟ_s *ਪੋਰਟ_gf = container_ਦਾ(
        op->drive_fl, ਬਣਤਰ ahci_ਪੋਰਟ_s, drive);
    ਬਣਤਰ ahci_cmd_s *cmd = ਪੋਰਟ_gf->cmd;
    ਪੂਰਨ rc;

    sata_prep_readwrite(&cmd->fis, op, iswrite);
    rc = ahci_command(ਪੋਰਟ_gf, iswrite, 0, op->buf_fl,
                      op->ਗਿਣਤੀ_2 * DISK_SECTOR_ਮਾਪ);
    dprintf(8, "ahci disk %s, lba %6x, count %3x, buf %p, rc %d\n",
            iswrite ? "write" : "read", (u32)op->lba, op->ਗਿਣਤੀ_2, op->buf_fl, rc);
    ਜੇ (rc < 0)
        ਵਾਪਸ DISK_RET_EBADTRACK;
    ਵਾਪਸ DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive.
ਸਥਿਰ ਪੂਰਨ
ahci_disk_readwrite(ਬਣਤਰ disk_op_s *op, ਪੂਰਨ iswrite)
{
    // if caller's buffer is word aligned, use it directly
    ਜੇ (((u32) op->buf_fl & 1) == 0)
        ਵਾਪਸ ahci_disk_readwrite_aligned(op, iswrite);

    // Use a word aligned buffer for AHCI I/O
    ਪੂਰਨ rc;
    ਬਣਤਰ disk_op_s localop = *op;
    u8 *alignedbuf_fl = bounce_buf_fl;
    u8 *position = op->buf_fl;

    localop.buf_fl = alignedbuf_fl;
    localop.ਗਿਣਤੀ_2 = 1;

    ਜੇ (iswrite) {
        u16 ਖੰਡ;
        ਲਈ (ਖੰਡ = 0; ਖੰਡ < op->ਗਿਣਤੀ_2; ਖੰਡ++) {
            memcpy_fl (alignedbuf_fl, position, DISK_SECTOR_ਮਾਪ);
            rc = ahci_disk_readwrite_aligned (&localop, 1);
            ਜੇ (rc)
                ਵਾਪਸ rc;
            position += DISK_SECTOR_ਮਾਪ;
            localop.lba++;
        }
    } ਨਹੀਂਤਾਂ { // read
        u16 ਖੰਡ;
        ਲਈ (ਖੰਡ = 0; ਖੰਡ < op->ਗਿਣਤੀ_2; ਖੰਡ++) {
            rc = ahci_disk_readwrite_aligned (&localop, 0);
            ਜੇ (rc)
                ਵਾਪਸ rc;
            memcpy_fl (position, alignedbuf_fl, DISK_SECTOR_ਮਾਪ);
            position += DISK_SECTOR_ਮਾਪ;
            localop.lba++;
        }
    }
    ਵਾਪਸ DISK_RET_SUCCESS;
}

// command demuxer
ਪੂਰਨ
ahci_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (!CONFIG_AHCI)
        ਵਾਪਸ 0;
    ਚੋਣ (op->command) {
    ਹਾਲਤ CMD_READ:
        ਵਾਪਸ ahci_disk_readwrite(op, 0);
    ਹਾਲਤ CMD_WRITE:
        ਵਾਪਸ ahci_disk_readwrite(op, 1);
    ਮੂਲ:
        ਵਾਪਸ default_process_op(op);
    }
}

ਸਥਿਰ ਖਾਲੀ
ahci_ਪੋਰਟ_reset(ਬਣਤਰ ahci_ctrl_s *ctrl, u32 pnr)
{
    u32 val;

    /* disable FIS + CMD */
    u32 ਸਿਰਾ = timer_calc(AHCI_RESET_TIMEOUT);
    ਲਈ (;;) {
        val = ahci_ਪੋਰਟ_readl(ctrl, pnr, ਪੋਰਟ_CMD);
        ਜੇ (!(val & (ਪੋਰਟ_CMD_FIS_RX | ਪੋਰਟ_CMD_ਸ਼ੁਰੂ |
                     ਪੋਰਟ_CMD_FIS_ON | ਪੋਰਟ_CMD_LIST_ON)))
            ਰੋਕੋ;
        val &= ~(ਪੋਰਟ_CMD_FIS_RX | ਪੋਰਟ_CMD_ਸ਼ੁਰੂ);
        ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_CMD, val);
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            ਰੋਕੋ;
        }
        yield();
    }

    /* disable + clear IRQs */
    ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_IRQ_ਨਕਾਬ, 0);
    val = ahci_ਪੋਰਟ_readl(ctrl, pnr, ਪੋਰਟ_IRQ_STAT);
    ਜੇ (val)
        ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_IRQ_STAT, val);
}

ਸਥਿਰ ਬਣਤਰ ahci_ਪੋਰਟ_s*
ahci_ਪੋਰਟ_alloc(ਬਣਤਰ ahci_ctrl_s *ctrl, u32 pnr)
{
    ਬਣਤਰ ahci_ਪੋਰਟ_s *ਪੋਰਟ = malloc_tmp(ਆਕਾਰ(*ਪੋਰਟ));

    ਜੇ (!ਪੋਰਟ) {
        warn_noalloc();
        ਵਾਪਸ ਨਿਰਾ;
    }
    memset(ਪੋਰਟ, 0, ਆਕਾਰ(*ਪੋਰਟ));
    ਪੋਰਟ->pnr = pnr;
    ਪੋਰਟ->ctrl = ctrl;
    ਪੋਰਟ->list = memalign_tmp(1024, 1024);
    ਪੋਰਟ->fis = memalign_tmp(256, 256);
    ਪੋਰਟ->cmd = memalign_tmp(256, 256);
    ਜੇ (ਪੋਰਟ->list == ਨਿਰਾ || ਪੋਰਟ->fis == ਨਿਰਾ || ਪੋਰਟ->cmd == ਨਿਰਾ) {
        warn_noalloc();
        ਵਾਪਸ ਨਿਰਾ;
    }
    memset(ਪੋਰਟ->list, 0, 1024);
    memset(ਪੋਰਟ->fis, 0, 256);
    memset(ਪੋਰਟ->cmd, 0, 256);

    ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_LST_ADDR, (u32)ਪੋਰਟ->list);
    ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_FIS_ADDR, (u32)ਪੋਰਟ->fis);
    ਜੇ (ctrl->caps & HOST_CAP_64) {
        ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_LST_ADDR_HI, 0);
        ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_FIS_ADDR_HI, 0);
    }

    ਵਾਪਸ ਪੋਰਟ;
}

ਸਥਿਰ ਖਾਲੀ ahci_ਪੋਰਟ_release(ਬਣਤਰ ahci_ਪੋਰਟ_s *ਪੋਰਟ)
{
    ahci_ਪੋਰਟ_reset(ਪੋਰਟ->ctrl, ਪੋਰਟ->pnr);
    ਮੁਕਤ(ਪੋਰਟ->list);
    ਮੁਕਤ(ਪੋਰਟ->fis);
    ਮੁਕਤ(ਪੋਰਟ->cmd);
    ਮੁਕਤ(ਪੋਰਟ);
}

ਸਥਿਰ ਬਣਤਰ ahci_ਪੋਰਟ_s* ahci_ਪੋਰਟ_realloc(ਬਣਤਰ ahci_ਪੋਰਟ_s *ਪੋਰਟ)
{
    ਬਣਤਰ ahci_ਪੋਰਟ_s *tmp;
    u32 cmd;

    tmp = malloc_fseg(ਆਕਾਰ(*ਪੋਰਟ));
    ਜੇ (!tmp) {
        warn_noalloc();
        ahci_ਪੋਰਟ_release(ਪੋਰਟ);
        ਵਾਪਸ ਨਿਰਾ;
    }
    *tmp = *ਪੋਰਟ;
    ਮੁਕਤ(ਪੋਰਟ);
    ਪੋਰਟ = tmp;

    ahci_ਪੋਰਟ_reset(ਪੋਰਟ->ctrl, ਪੋਰਟ->pnr);

    ਮੁਕਤ(ਪੋਰਟ->list);
    ਮੁਕਤ(ਪੋਰਟ->fis);
    ਮੁਕਤ(ਪੋਰਟ->cmd);
    ਪੋਰਟ->list = memalign_ਉੱਚਾ(1024, 1024);
    ਪੋਰਟ->fis = memalign_ਉੱਚਾ(256, 256);
    ਪੋਰਟ->cmd = memalign_ਉੱਚਾ(256, 256);
    ਜੇ (!ਪੋਰਟ->list || !ਪੋਰਟ->fis || !ਪੋਰਟ->cmd) {
        warn_noalloc();
        ਮੁਕਤ(ਪੋਰਟ->list);
        ਮੁਕਤ(ਪੋਰਟ->fis);
        ਮੁਕਤ(ਪੋਰਟ->cmd);
        ਮੁਕਤ(ਪੋਰਟ);
        ਵਾਪਸ ਨਿਰਾ;
    }

    ahci_ਪੋਰਟ_writel(ਪੋਰਟ->ctrl, ਪੋਰਟ->pnr, ਪੋਰਟ_LST_ADDR, (u32)ਪੋਰਟ->list);
    ahci_ਪੋਰਟ_writel(ਪੋਰਟ->ctrl, ਪੋਰਟ->pnr, ਪੋਰਟ_FIS_ADDR, (u32)ਪੋਰਟ->fis);

    cmd = ahci_ਪੋਰਟ_readl(ਪੋਰਟ->ctrl, ਪੋਰਟ->pnr, ਪੋਰਟ_CMD);
    cmd |= (ਪੋਰਟ_CMD_FIS_RX|ਪੋਰਟ_CMD_ਸ਼ੁਰੂ);
    ahci_ਪੋਰਟ_writel(ਪੋਰਟ->ctrl, ਪੋਰਟ->pnr, ਪੋਰਟ_CMD, cmd);

    ਵਾਪਸ ਪੋਰਟ;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXMODEL 40

/* See ahci spec chapter 10.1 "Software Initialization of HBA" */
ਸਥਿਰ ਪੂਰਨ ahci_ਪੋਰਟ_ਤਿਆਰੀ(ਬਣਤਰ ahci_ਪੋਰਟ_s *ਪੋਰਟ)
{
    ਬਣਤਰ ahci_ctrl_s *ctrl = ਪੋਰਟ->ctrl;
    u32 pnr = ਪੋਰਟ->pnr;
    ਅੱਖਰ model[MAXMODEL+1];
    u16 ਬਫਰ[256];
    u32 cmd, stat, err, tf;
    ਪੂਰਨ rc;

    /* enable FIS recv */
    cmd = ahci_ਪੋਰਟ_readl(ctrl, pnr, ਪੋਰਟ_CMD);
    cmd |= ਪੋਰਟ_CMD_FIS_RX;
    ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_CMD, cmd);

    /* spin up */
    cmd &= ~ਪੋਰਟ_CMD_ICC_ਨਕਾਬ;
    cmd |= ਪੋਰਟ_CMD_SPIN_ਉੱਪਰ | ਪੋਰਟ_CMD_POWER_ON | ਪੋਰਟ_CMD_ICC_ਸਰਗਰਮ;
    ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_CMD, cmd);
    u32 ਸਿਰਾ = timer_calc(AHCI_LINK_TIMEOUT);
    ਲਈ (;;) {
        stat = ahci_ਪੋਰਟ_readl(ctrl, pnr, ਪੋਰਟ_SCR_STAT);
        ਜੇ ((stat & 0x07) == 0x03) {
            dprintf(2, "AHCI/%d: link up\n", ਪੋਰਟ->pnr);
            ਰੋਕੋ;
        }
        ਜੇ (timer_check(ਸਿਰਾ)) {
            dprintf(2, "AHCI/%d: link down\n", ਪੋਰਟ->pnr);
            ਵਾਪਸ -1;
        }
        yield();
    }

    /* clear error status */
    err = ahci_ਪੋਰਟ_readl(ctrl, pnr, ਪੋਰਟ_SCR_ERR);
    ਜੇ (err)
        ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_SCR_ERR, err);

    /* wait for device becoming ready */
    ਸਿਰਾ = timer_calc(AHCI_REQUEST_TIMEOUT);
    ਲਈ (;;) {
        tf = ahci_ਪੋਰਟ_readl(ctrl, pnr, ਪੋਰਟ_TFDATA);
        ਜੇ (!(tf & (ATA_CB_STAT_BSY |
                    ATA_CB_STAT_DRQ)))
            ਰੋਕੋ;
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            dprintf(1, "AHCI/%d: device not ready (tf 0x%x)\n", ਪੋਰਟ->pnr, tf);
            ਵਾਪਸ -1;
        }
        yield();
    }

    /* start device */
    cmd |= ਪੋਰਟ_CMD_ਸ਼ੁਰੂ;
    ahci_ਪੋਰਟ_writel(ctrl, pnr, ਪੋਰਟ_CMD, cmd);

    sata_prep_simple(&ਪੋਰਟ->cmd->fis, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    rc = ahci_command(ਪੋਰਟ, 0, 0, ਬਫਰ, ਆਕਾਰ(ਬਫਰ));
    ਜੇ (rc == 0) {
        ਪੋਰਟ->atapi = 1;
    } ਨਹੀਂਤਾਂ {
        ਪੋਰਟ->atapi = 0;
        sata_prep_simple(&ਪੋਰਟ->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
        rc = ahci_command(ਪੋਰਟ, 0, 0, ਬਫਰ, ਆਕਾਰ(ਬਫਰ));
        ਜੇ (rc < 0)
            ਵਾਪਸ -1;
    }

    ਪੋਰਟ->drive.cntl_id = pnr;
    ਪੋਰਟ->drive.removable = (ਬਫਰ[0] & 0x80) ? 1 : 0;

    ਜੇ (!ਪੋਰਟ->atapi) {
        // found disk (ata)
        ਪੋਰਟ->drive.type = DTYPE_AHCI;
        ਪੋਰਟ->drive.blksize = DISK_SECTOR_ਮਾਪ;
        ਪੋਰਟ->drive.pchs.cylinder = ਬਫਰ[1];
        ਪੋਰਟ->drive.pchs.ਸਿਰ = ਬਫਰ[3];
        ਪੋਰਟ->drive.pchs.sector = ਬਫਰ[6];

        u64 sectors;
        ਜੇ (ਬਫਰ[83] & (1 << 10)) // word 83 - lba48 support
            sectors = *(u64*)&ਬਫਰ[100]; // word 100-103
        ਨਹੀਂਤਾਂ
            sectors = *(u32*)&ਬਫਰ[60]; // word 60 and word 61
        ਪੋਰਟ->drive.sectors = sectors;
        u64 adjsize = sectors >> 11;
        ਅੱਖਰ adjprefix = 'M';
        ਜੇ (adjsize >= (1 << 16)) {
            adjsize >>= 10;
            adjprefix = 'G';
        }
        ਪੋਰਟ->desc = znprintf(MAXDESCSIZE
                              , "AHCI/%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                              , ਪੋਰਟ->pnr
                              , ata_ਕੱਢੋ_model(model, MAXMODEL, ਬਫਰ)
                              , ata_ਕੱਢੋ_ਸੰਸਕਰਣ(ਬਫਰ)
                              , (u32)adjsize, adjprefix);
        ਪੋਰਟ->prio = bootprio_ਲੱਭੋ_ata_device(ctrl->pci_tmp, pnr, 0);

        s8 multi_dma = -1;
        s8 pio_ਬਹੁਲਕ = -1;
        s8 udma_ਬਹੁਲਕ = -1;
        // If bit 2 in word 53 is set, udma information is valid in word 88.
        ਜੇ (ਬਫਰ[53] & 0x04) {
            udma_ਬਹੁਲਕ = 6;
            ਜਦੋਂ ((udma_ਬਹੁਲਕ >= 0) &&
                   !((ਬਫਰ[88] & 0x7f) & ( 1 << udma_ਬਹੁਲਕ ))) {
                udma_ਬਹੁਲਕ--;
            }
        }
        // If bit 1 in word 53 is set, multiword-dma and advanced pio modes
        // are available in words 63 and 64.
        ਜੇ (ਬਫਰ[53] & 0x02) {
            pio_ਬਹੁਲਕ = 4;
            multi_dma = 3;
            ਜਦੋਂ ((multi_dma >= 0) &&
                   !((ਬਫਰ[63] & 0x7) & ( 1 << multi_dma ))) {
                multi_dma--;
            }
            ਜਦੋਂ ((pio_ਬਹੁਲਕ >= 3) &&
                   !((ਬਫਰ[64] & 0x3) & ( 1 << ( pio_ਬਹੁਲਕ - 3 ) ))) {
                pio_ਬਹੁਲਕ--;
            }
        }
        dprintf(2, "AHCI/%d: supported modes: udma %d, multi-dma %d, pio %d\n",
                ਪੋਰਟ->pnr, udma_ਬਹੁਲਕ, multi_dma, pio_ਬਹੁਲਕ);

        sata_prep_simple(&ਪੋਰਟ->cmd->fis, ATA_CMD_ਸੈੱਟ_FEATURES);
        ਪੋਰਟ->cmd->fis.feature = ATA_ਸੈੱਟ_FEATRUE_TRANSFER_ਬਹੁਲਕ;
        // Select used mode. UDMA first, then Multi-DMA followed by
        // advanced PIO modes 3 or 4. If non, set default PIO.
        ਜੇ (udma_ਬਹੁਲਕ >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to UDMA-%d\n",
                    ਪੋਰਟ->pnr, udma_ਬਹੁਲਕ);
            ਪੋਰਟ->cmd->fis.sector_ਗਿਣਤੀ = ATA_TRANSFER_ਬਹੁਲਕ_ULTRA_DMA
                                          | udma_ਬਹੁਲਕ;
        } ਨਹੀਂਤਾਂ ਜੇ (multi_dma >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to Multi-DMA-%d\n",
                    ਪੋਰਟ->pnr, multi_dma);
            ਪੋਰਟ->cmd->fis.sector_ਗਿਣਤੀ = ATA_TRANSFER_ਬਹੁਲਕ_MULTIWORD_DMA
                                          | multi_dma;
        } ਨਹੀਂਤਾਂ ਜੇ (pio_ਬਹੁਲਕ >= 3) {
            dprintf(1, "AHCI/%d: Set transfer mode to PIO-%d\n",
                    ਪੋਰਟ->pnr, pio_ਬਹੁਲਕ);
            ਪੋਰਟ->cmd->fis.sector_ਗਿਣਤੀ = ATA_TRANSFER_ਬਹੁਲਕ_PIO_FLOW_CTRL
                                          | pio_ਬਹੁਲਕ;
        } ਨਹੀਂਤਾਂ {
            dprintf(1, "AHCI/%d: Set transfer mode to default PIO\n",
                    ਪੋਰਟ->pnr);
            ਪੋਰਟ->cmd->fis.sector_ਗਿਣਤੀ = ATA_TRANSFER_ਬਹੁਲਕ_DEFAULT_PIO;
        }
        rc = ahci_command(ਪੋਰਟ, 1, 0, 0, 0);
        ਜੇ (rc < 0) {
            dprintf(1, "AHCI/%d: Set transfer mode failed.\n", ਪੋਰਟ->pnr);
        }
    } ਨਹੀਂਤਾਂ {
        // found cdrom (atapi)
        ਪੋਰਟ->drive.type = DTYPE_AHCI_ATAPI;
        ਪੋਰਟ->drive.blksize = CDROM_SECTOR_ਮਾਪ;
        ਪੋਰਟ->drive.sectors = (u64)-1;
        u8 iscd = ((ਬਫਰ[0] >> 8) & 0x1f) == 0x05;
        ਜੇ (!iscd) {
            dprintf(1, "AHCI/%d: atapi device isn't a cdrom\n", ਪੋਰਟ->pnr);
            ਵਾਪਸ -1;
        }
        ਪੋਰਟ->desc = znprintf(MAXDESCSIZE
                              , "DVD/CD [AHCI/%d: %s ATAPI-%d DVD/CD]"
                              , ਪੋਰਟ->pnr
                              , ata_ਕੱਢੋ_model(model, MAXMODEL, ਬਫਰ)
                              , ata_ਕੱਢੋ_ਸੰਸਕਰਣ(ਬਫਰ));
        ਪੋਰਟ->prio = bootprio_ਲੱਭੋ_ata_device(ctrl->pci_tmp, pnr, 0);
    }
    boot_lchs_ਲੱਭੋ_ata_device(ctrl->pci_tmp, pnr, 0, &(ਪੋਰਟ->drive.lchs));
    ਵਾਪਸ 0;
}

// Detect any drives attached to a given controller.
ਸਥਿਰ ਖਾਲੀ
ahci_ਪੋਰਟ_detect(ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਬਣਤਰ ahci_ਪੋਰਟ_s *ਪੋਰਟ = ਅੰਕੜੇ;
    ਪੂਰਨ rc;

    dprintf(2, "AHCI/%d: probing\n", ਪੋਰਟ->pnr);
    ahci_ਪੋਰਟ_reset(ਪੋਰਟ->ctrl, ਪੋਰਟ->pnr);
    rc = ahci_ਪੋਰਟ_ਤਿਆਰੀ(ਪੋਰਟ);
    ਜੇ (rc < 0)
        ahci_ਪੋਰਟ_release(ਪੋਰਟ);
    ਨਹੀਂਤਾਂ {
        ਪੋਰਟ = ahci_ਪੋਰਟ_realloc(ਪੋਰਟ);
        ਜੇ (ਪੋਰਟ == ਨਿਰਾ)
            ਵਾਪਸ;
        dprintf(1, "AHCI/%d: registering: \"%s\"\n", ਪੋਰਟ->pnr, ਪੋਰਟ->desc);
        ਜੇ (!ਪੋਰਟ->atapi) {
            // Register with bcv system.
            boot_ਜੋੜੋ_hd(&ਪੋਰਟ->drive, ਪੋਰਟ->desc, ਪੋਰਟ->prio);
        } ਨਹੀਂਤਾਂ {
            // fill cdidmap
            boot_ਜੋੜੋ_cd(&ਪੋਰਟ->drive, ਪੋਰਟ->desc, ਪੋਰਟ->prio);
        }
    }
}

// Initialize an ata controller and detect its drives.
ਸਥਿਰ ਖਾਲੀ
ahci_controller_ਤਿਆਰੀ(ਬਣਤਰ pci_device *pci)
{
    ਬਣਤਰ ahci_ਪੋਰਟ_s *ਪੋਰਟ;
    u32 pnr, max;

    ਜੇ (create_bounce_buf() < 0)
        ਵਾਪਸ;

    ਖਾਲੀ *iobase = pci_enable_membar(pci, PCI_ਅਧਾਰ_ਪਤਾ_5);
    ਜੇ (!iobase)
        ਵਾਪਸ;

    ਬਣਤਰ ahci_ctrl_s *ctrl = malloc_fseg(ਆਕਾਰ(*ctrl));
    ਜੇ (!ctrl) {
        warn_noalloc();
        ਵਾਪਸ;
    }

    ctrl->pci_tmp = pci;
    ctrl->iobase = iobase;
    ctrl->irq = pci_config_readb(pci->bdf, PCI_ਰੁਕਾਵਟ_ਸਤਰ);
    dprintf(1, "AHCI controller at %pP, iobase %p, irq %d\n"
            , pci, ctrl->iobase, ctrl->irq);

    pci_enable_busmaster(pci);

    u32 val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_RESET);
    u32 ਸਿਰਾ = timer_calc(AHCI_RESET_TIMEOUT);
    ਲਈ (;;) {
        val = ahci_ctrl_readl(ctrl, HOST_CTL);
        ਜੇ (!(val & HOST_CTL_RESET))
            ਰੋਕੋ;
        ਜੇ (timer_check(ਸਿਰਾ)) {
            warn_timeout();
            dprintf(1, "AHCI: controller reset failed\n");
            ਮੁਕਤ(ctrl);
            ਵਾਪਸ;
        }
        yield();
    }
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_AHCI_EN);
    (ਖਾਲੀ)ahci_ctrl_readl(ctrl, HOST_CTL); /* Flush */

    ctrl->caps = ahci_ctrl_readl(ctrl, HOST_CAP);
    ctrl->ports = ahci_ctrl_readl(ctrl, HOST_PORTS_IMPL);
    dprintf(2, "AHCI: cap 0x%x, ports_impl 0x%x\n",
            ctrl->caps, ctrl->ports);

    max = 0x1f;
    ਲਈ (pnr = 0; pnr <= max; pnr++) {
        ਜੇ (!(ctrl->ports & (1 << pnr)))
            ਜਾਰੀ;
        ਪੋਰਟ = ahci_ਪੋਰਟ_alloc(ctrl, pnr);
        ਜੇ (ਪੋਰਟ == ਨਿਰਾ)
            ਜਾਰੀ;
        ਲੜੀਵਾਰ_thread(ahci_ਪੋਰਟ_detect, ਪੋਰਟ);
    }
}

// Locate and init ahci controllers.
ਸਥਿਰ ਖਾਲੀ
ahci_ਜਾਂਚ(ਖਾਲੀ)
{
    // Scan PCI bus for ATA adapters
    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (pci->class != PCI_CLASS_STORAGE_SATA)
            ਜਾਰੀ;
        ਜੇ (pci->prog_if != 1 /* AHCI rev 1 */)
            ਜਾਰੀ;
        ahci_controller_ਤਿਆਰੀ(pci);
    }
}

ਖਾਲੀ
ahci_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ASSERT32FLAT();
    ਜੇ (!CONFIG_AHCI)
        ਵਾਪਸ;

    dprintf(3, "init ahci\n");
    ahci_ਜਾਂਚ();
}

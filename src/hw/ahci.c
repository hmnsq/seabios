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

#وصف_ڪريو AHCI_REQUEST_TIMEOUT 32000 // 32 seconds max for IDE ops
#وصف_ڪريو AHCI_RESET_TIMEOUT     500 // 500 miliseconds
#وصف_ڪريو AHCI_LINK_TIMEOUT       10 // 10 miliseconds

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

    إذا (op->ڳڻپ >= (1<<8) || lba + op->ڳڻپ >= (1<<28)) {
        fis->sector_count2 = op->ڳڻپ >> 8;
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
    fis->sector_ڳڻپ = op->ڳڻپ;
    fis->lba_هيٺيون      = lba;
    fis->lba_mid      = lba >> 8;
    fis->lba_مٿاهون     = lba >> 16;
    fis->device       = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;
}

ساكن فراغ sata_prep_atapi(بنية sata_cmd_fis *fis, u16 blocksize)
{
    memset_fl(fis, 0, حجم(*fis));
    fis->command  = ATA_CMD_PACKET;
    fis->feature  = 1; /* dma */
    fis->lba_mid  = blocksize;
    fis->lba_مٿاهون = blocksize >> 8;
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

ساكن u32 ahci_پورٽ_منزل_ctrl(u32 pnr, u32 پورٽ_reg)
{
    u32 ctrl_reg = 0x100;
    ctrl_reg += pnr * 0x80;
    ctrl_reg += پورٽ_reg;
    أرجع ctrl_reg;
}

ساكن u32 ahci_پورٽ_readl(بنية ahci_ctrl_s *ctrl, u32 pnr, u32 reg)
{
    u32 ctrl_reg = ahci_پورٽ_منزل_ctrl(pnr, reg);
    أرجع ahci_ctrl_readl(ctrl, ctrl_reg);
}

ساكن فراغ ahci_پورٽ_writel(بنية ahci_ctrl_s *ctrl, u32 pnr, u32 reg, u32 val)
{
    u32 ctrl_reg = ahci_پورٽ_منزل_ctrl(pnr, reg);
    ahci_ctrl_writel(ctrl, ctrl_reg, val);
}

// submit ahci command + wait for result
ساكن صحيح ahci_command(بنية ahci_پورٽ_s *پورٽ_gf, صحيح iswrite, صحيح isatapi,
                        فراغ *بفر, u32 bsize)
{
    u32 val, حالت, success, flags, intbits, error;
    بنية ahci_ctrl_s *ctrl = پورٽ_gf->ctrl;
    بنية ahci_cmd_s  *cmd  = پورٽ_gf->cmd;
    بنية ahci_fis_s  *fis  = پورٽ_gf->fis;
    بنية ahci_list_s *list = پورٽ_gf->list;
    u32 pnr                  = پورٽ_gf->pnr;

    cmd->fis.reg       = 0x27;
    cmd->fis.pmp_type  = 1 << 7; /* cmd fis */
    cmd->prdt[0].بنياد  = (u32)بفر;
    cmd->prdt[0].baseu = 0;
    cmd->prdt[0].flags = bsize-1;

    flags = ((1 << 16) | /* one prd entry */
             (iswrite ? (1 << 6) : 0) |
             (isatapi ? (1 << 5) : 0) |
             (5 << 0)); /* fis length (dwords) */
    list[0].flags  = flags;
    list[0].بائيٽون  = 0;
    list[0].بنياد   = (u32)(cmd);
    list[0].baseu  = 0;

    dprintf(8, "AHCI/%d: send cmd ...\n", pnr);
    intbits = ahci_پورٽ_readl(ctrl, pnr, پورٽ_IRQ_STAT);
    إذا (intbits)
        ahci_پورٽ_writel(ctrl, pnr, پورٽ_IRQ_STAT, intbits);
    ahci_پورٽ_writel(ctrl, pnr, پورٽ_CMD_ISSUE, 1);

    u32 پڇاڙي = timer_calc(AHCI_REQUEST_TIMEOUT);
    افعل {
        لكل (;;) {
            intbits = ahci_پورٽ_readl(ctrl, pnr, پورٽ_IRQ_STAT);
            إذا (intbits) {
                ahci_پورٽ_writel(ctrl, pnr, پورٽ_IRQ_STAT, intbits);
                إذا (intbits & 0x40000000) {
                    u32 tf = ahci_پورٽ_readl(ctrl, pnr, پورٽ_TFDATA);
                    حالت = tf & 0xff;
                    error = (tf & 0xff00) >> 8;
                    اكسر;
                }
                إذا (intbits & 0x02) {
                    حالت = GET_LOWFLAT(fis->psfis[2]);
                    error  = GET_LOWFLAT(fis->psfis[3]);
                    اكسر;
                }
                إذا (intbits & 0x01) {
                    حالت = GET_LOWFLAT(fis->rfis[2]);
                    error  = GET_LOWFLAT(fis->rfis[3]);
                    اكسر;
                }
            }
            إذا (timer_check(پڇاڙي)) {
                warn_timeout();
                أرجع -1;
            }
            yield();
        }
        dprintf(8, "AHCI/%d: ... intbits 0x%x, status 0x%x ...\n",
                pnr, intbits, حالت);
    } طالما (حالت & ATA_CB_STAT_BSY);

    success = (0x00 == (حالت & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF |
                                  ATA_CB_STAT_ERR)) &&
               ATA_CB_STAT_RDY == (حالت & (ATA_CB_STAT_RDY)));
    إذا (success) {
        dprintf(8, "AHCI/%d: ... finished, status 0x%x, OK\n", pnr,
                حالت);
    } وإلا {
        dprintf(2, "AHCI/%d: ... finished, status 0x%x, ERROR 0x%x\n", pnr,
                حالت, error);

        // non-queued error recovery (AHCI 1.3 section 6.2.2.1)
        // Clears PxCMD.ST to 0 to reset the PxCI register
        val = ahci_پورٽ_readl(ctrl, pnr, پورٽ_CMD);
        ahci_پورٽ_writel(ctrl, pnr, پورٽ_CMD, val & ~پورٽ_CMD_شروعات);

        // waits for PxCMD.CR to clear to 0
        طالما (1) {
            val = ahci_پورٽ_readl(ctrl, pnr, پورٽ_CMD);
            إذا ((val & پورٽ_CMD_LIST_ON) == 0)
                اكسر;
            yield();
        }

        // Clears any error bits in PxSERR to enable capturing new errors
        val = ahci_پورٽ_readl(ctrl, pnr, پورٽ_SCR_ERR);
        ahci_پورٽ_writel(ctrl, pnr, پورٽ_SCR_ERR, val);

        // Clears status bits in PxIS as appropriate
        val = ahci_پورٽ_readl(ctrl, pnr, پورٽ_IRQ_STAT);
        ahci_پورٽ_writel(ctrl, pnr, پورٽ_IRQ_STAT, val);

        // If PxTFD.STS.BSY or PxTFD.STS.DRQ is set to 1, issue
        // a COMRESET to the device to put it in an idle state
        val = ahci_پورٽ_readl(ctrl, pnr, پورٽ_TFDATA);
        إذا (val & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ)) {
            dprintf(2, "AHCI/%d: issue comreset\n", pnr);
            val = ahci_پورٽ_readl(ctrl, pnr, پورٽ_SCR_CTL);
            // set Device Detection Initialization (DET) to 1 for 1 ms for comreset
            ahci_پورٽ_writel(ctrl, pnr, پورٽ_SCR_CTL, val | 1);
            mdelay (1);
            ahci_پورٽ_writel(ctrl, pnr, پورٽ_SCR_CTL, val);
        }

        // Sets PxCMD.ST to 1 to enable issuing new commands
        val = ahci_پورٽ_readl(ctrl, pnr, پورٽ_CMD);
        ahci_پورٽ_writel(ctrl, pnr, پورٽ_CMD, val | پورٽ_CMD_شروعات);
    }
    أرجع success ? 0 : -1;
}

#وصف_ڪريو CDROM_CDB_سائيز 12

صحيح ahci_atapi_process_op(بنية disk_op_s *op)
{
    إذا (! CONFIG_AHCI)
        أرجع 0;

    بنية ahci_پورٽ_s *پورٽ_gf = container_جو(
        op->drive_fl, بنية ahci_پورٽ_s, drive);
    بنية ahci_cmd_s *cmd = پورٽ_gf->cmd;

    إذا (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        أرجع DISK_RET_EWRITEPROTECT;
    صحيح blocksize = scsi_fill_cmd(op, cmd->atapi, CDROM_CDB_سائيز);
    إذا (blocksize < 0)
        أرجع default_process_op(op);
    sata_prep_atapi(&cmd->fis, blocksize);
    صحيح rc = ahci_command(پورٽ_gf, 0, 1, op->buf_fl, op->ڳڻپ * blocksize);
    إذا (rc < 0)
        أرجع DISK_RET_EBADTRACK;
    أرجع DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive, op->buf_fl must be word aligned
ساكن صحيح
ahci_disk_readwrite_aligned(بنية disk_op_s *op, صحيح iswrite)
{
    بنية ahci_پورٽ_s *پورٽ_gf = container_جو(
        op->drive_fl, بنية ahci_پورٽ_s, drive);
    بنية ahci_cmd_s *cmd = پورٽ_gf->cmd;
    صحيح rc;

    sata_prep_readwrite(&cmd->fis, op, iswrite);
    rc = ahci_command(پورٽ_gf, iswrite, 0, op->buf_fl,
                      op->ڳڻپ * DISK_SECTOR_سائيز);
    dprintf(8, "ahci disk %s, lba %6x, count %3x, buf %p, rc %d\n",
            iswrite ? "write" : "read", (u32)op->lba, op->ڳڻپ, op->buf_fl, rc);
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
    localop.ڳڻپ = 1;

    إذا (iswrite) {
        u16 بلاڪ;
        لكل (بلاڪ = 0; بلاڪ < op->ڳڻپ; بلاڪ++) {
            memcpy_fl (alignedbuf_fl, position, DISK_SECTOR_سائيز);
            rc = ahci_disk_readwrite_aligned (&localop, 1);
            إذا (rc)
                أرجع rc;
            position += DISK_SECTOR_سائيز;
            localop.lba++;
        }
    } وإلا { // read
        u16 بلاڪ;
        لكل (بلاڪ = 0; بلاڪ < op->ڳڻپ; بلاڪ++) {
            rc = ahci_disk_readwrite_aligned (&localop, 0);
            إذا (rc)
                أرجع rc;
            memcpy_fl (position, alignedbuf_fl, DISK_SECTOR_سائيز);
            position += DISK_SECTOR_سائيز;
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
ahci_پورٽ_reset(بنية ahci_ctrl_s *ctrl, u32 pnr)
{
    u32 val;

    /* disable FIS + CMD */
    u32 پڇاڙي = timer_calc(AHCI_RESET_TIMEOUT);
    لكل (;;) {
        val = ahci_پورٽ_readl(ctrl, pnr, پورٽ_CMD);
        إذا (!(val & (پورٽ_CMD_FIS_RX | پورٽ_CMD_شروعات |
                     پورٽ_CMD_FIS_ON | پورٽ_CMD_LIST_ON)))
            اكسر;
        val &= ~(پورٽ_CMD_FIS_RX | پورٽ_CMD_شروعات);
        ahci_پورٽ_writel(ctrl, pnr, پورٽ_CMD, val);
        إذا (timer_check(پڇاڙي)) {
            warn_timeout();
            اكسر;
        }
        yield();
    }

    /* disable + clear IRQs */
    ahci_پورٽ_writel(ctrl, pnr, پورٽ_IRQ_نقاب, 0);
    val = ahci_پورٽ_readl(ctrl, pnr, پورٽ_IRQ_STAT);
    إذا (val)
        ahci_پورٽ_writel(ctrl, pnr, پورٽ_IRQ_STAT, val);
}

ساكن بنية ahci_پورٽ_s*
ahci_پورٽ_alloc(بنية ahci_ctrl_s *ctrl, u32 pnr)
{
    بنية ahci_پورٽ_s *پورٽ = malloc_tmp(حجم(*پورٽ));

    إذا (!پورٽ) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(پورٽ, 0, حجم(*پورٽ));
    پورٽ->pnr = pnr;
    پورٽ->ctrl = ctrl;
    پورٽ->list = memalign_tmp(1024, 1024);
    پورٽ->fis = memalign_tmp(256, 256);
    پورٽ->cmd = memalign_tmp(256, 256);
    إذا (پورٽ->list == عدم || پورٽ->fis == عدم || پورٽ->cmd == عدم) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(پورٽ->list, 0, 1024);
    memset(پورٽ->fis, 0, 256);
    memset(پورٽ->cmd, 0, 256);

    ahci_پورٽ_writel(ctrl, pnr, پورٽ_LST_ADDR, (u32)پورٽ->list);
    ahci_پورٽ_writel(ctrl, pnr, پورٽ_FIS_ADDR, (u32)پورٽ->fis);
    إذا (ctrl->caps & HOST_CAP_64) {
        ahci_پورٽ_writel(ctrl, pnr, پورٽ_LST_ADDR_HI, 0);
        ahci_پورٽ_writel(ctrl, pnr, پورٽ_FIS_ADDR_HI, 0);
    }

    أرجع پورٽ;
}

ساكن فراغ ahci_پورٽ_release(بنية ahci_پورٽ_s *پورٽ)
{
    ahci_پورٽ_reset(پورٽ->ctrl, پورٽ->pnr);
    حرر(پورٽ->list);
    حرر(پورٽ->fis);
    حرر(پورٽ->cmd);
    حرر(پورٽ);
}

ساكن بنية ahci_پورٽ_s* ahci_پورٽ_realloc(بنية ahci_پورٽ_s *پورٽ)
{
    بنية ahci_پورٽ_s *tmp;
    u32 cmd;

    tmp = malloc_fseg(حجم(*پورٽ));
    إذا (!tmp) {
        warn_noalloc();
        ahci_پورٽ_release(پورٽ);
        أرجع عدم;
    }
    *tmp = *پورٽ;
    حرر(پورٽ);
    پورٽ = tmp;

    ahci_پورٽ_reset(پورٽ->ctrl, پورٽ->pnr);

    حرر(پورٽ->list);
    حرر(پورٽ->fis);
    حرر(پورٽ->cmd);
    پورٽ->list = memalign_مٿاهون(1024, 1024);
    پورٽ->fis = memalign_مٿاهون(256, 256);
    پورٽ->cmd = memalign_مٿاهون(256, 256);
    إذا (!پورٽ->list || !پورٽ->fis || !پورٽ->cmd) {
        warn_noalloc();
        حرر(پورٽ->list);
        حرر(پورٽ->fis);
        حرر(پورٽ->cmd);
        حرر(پورٽ);
        أرجع عدم;
    }

    ahci_پورٽ_writel(پورٽ->ctrl, پورٽ->pnr, پورٽ_LST_ADDR, (u32)پورٽ->list);
    ahci_پورٽ_writel(پورٽ->ctrl, پورٽ->pnr, پورٽ_FIS_ADDR, (u32)پورٽ->fis);

    cmd = ahci_پورٽ_readl(پورٽ->ctrl, پورٽ->pnr, پورٽ_CMD);
    cmd |= (پورٽ_CMD_FIS_RX|پورٽ_CMD_شروعات);
    ahci_پورٽ_writel(پورٽ->ctrl, پورٽ->pnr, پورٽ_CMD, cmd);

    أرجع پورٽ;
}

#وصف_ڪريو MAXMODEL 40

/* See ahci spec chapter 10.1 "Software Initialization of HBA" */
ساكن صحيح ahci_پورٽ_تياري(بنية ahci_پورٽ_s *پورٽ)
{
    بنية ahci_ctrl_s *ctrl = پورٽ->ctrl;
    u32 pnr = پورٽ->pnr;
    حرف model[MAXMODEL+1];
    u16 بفر[256];
    u32 cmd, stat, err, tf;
    صحيح rc;

    /* enable FIS recv */
    cmd = ahci_پورٽ_readl(ctrl, pnr, پورٽ_CMD);
    cmd |= پورٽ_CMD_FIS_RX;
    ahci_پورٽ_writel(ctrl, pnr, پورٽ_CMD, cmd);

    /* spin up */
    cmd &= ~پورٽ_CMD_ICC_نقاب;
    cmd |= پورٽ_CMD_SPIN_مٿي | پورٽ_CMD_POWER_ON | پورٽ_CMD_ICC_سرگرم;
    ahci_پورٽ_writel(ctrl, pnr, پورٽ_CMD, cmd);
    u32 پڇاڙي = timer_calc(AHCI_LINK_TIMEOUT);
    لكل (;;) {
        stat = ahci_پورٽ_readl(ctrl, pnr, پورٽ_SCR_STAT);
        إذا ((stat & 0x07) == 0x03) {
            dprintf(2, "AHCI/%d: link up\n", پورٽ->pnr);
            اكسر;
        }
        إذا (timer_check(پڇاڙي)) {
            dprintf(2, "AHCI/%d: link down\n", پورٽ->pnr);
            أرجع -1;
        }
        yield();
    }

    /* clear error status */
    err = ahci_پورٽ_readl(ctrl, pnr, پورٽ_SCR_ERR);
    إذا (err)
        ahci_پورٽ_writel(ctrl, pnr, پورٽ_SCR_ERR, err);

    /* wait for device becoming ready */
    پڇاڙي = timer_calc(AHCI_REQUEST_TIMEOUT);
    لكل (;;) {
        tf = ahci_پورٽ_readl(ctrl, pnr, پورٽ_TFDATA);
        إذا (!(tf & (ATA_CB_STAT_BSY |
                    ATA_CB_STAT_DRQ)))
            اكسر;
        إذا (timer_check(پڇاڙي)) {
            warn_timeout();
            dprintf(1, "AHCI/%d: device not ready (tf 0x%x)\n", پورٽ->pnr, tf);
            أرجع -1;
        }
        yield();
    }

    /* start device */
    cmd |= پورٽ_CMD_شروعات;
    ahci_پورٽ_writel(ctrl, pnr, پورٽ_CMD, cmd);

    sata_prep_simple(&پورٽ->cmd->fis, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    rc = ahci_command(پورٽ, 0, 0, بفر, حجم(بفر));
    إذا (rc == 0) {
        پورٽ->atapi = 1;
    } وإلا {
        پورٽ->atapi = 0;
        sata_prep_simple(&پورٽ->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
        rc = ahci_command(پورٽ, 0, 0, بفر, حجم(بفر));
        إذا (rc < 0)
            أرجع -1;
    }

    پورٽ->drive.cntl_id = pnr;
    پورٽ->drive.removable = (بفر[0] & 0x80) ? 1 : 0;

    إذا (!پورٽ->atapi) {
        // found disk (ata)
        پورٽ->drive.type = DTYPE_AHCI;
        پورٽ->drive.blksize = DISK_SECTOR_سائيز;
        پورٽ->drive.pchs.cylinder = بفر[1];
        پورٽ->drive.pchs.سر = بفر[3];
        پورٽ->drive.pchs.sector = بفر[6];

        u64 sectors;
        إذا (بفر[83] & (1 << 10)) // word 83 - lba48 support
            sectors = *(u64*)&بفر[100]; // word 100-103
        وإلا
            sectors = *(u32*)&بفر[60]; // word 60 and word 61
        پورٽ->drive.sectors = sectors;
        u64 adjsize = sectors >> 11;
        حرف adjprefix = 'M';
        إذا (adjsize >= (1 << 16)) {
            adjsize >>= 10;
            adjprefix = 'G';
        }
        پورٽ->desc = znprintf(MAXDESCSIZE
                              , "AHCI/%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                              , پورٽ->pnr
                              , ata_ڪڍو_model(model, MAXMODEL, بفر)
                              , ata_ڪڍو_نسخو(بفر)
                              , (u32)adjsize, adjprefix);
        پورٽ->prio = bootprio_ڳوليو_ata_device(ctrl->pci_tmp, pnr, 0);

        s8 multi_dma = -1;
        s8 pio_گهڻائي = -1;
        s8 udma_گهڻائي = -1;
        // If bit 2 in word 53 is set, udma information is valid in word 88.
        إذا (بفر[53] & 0x04) {
            udma_گهڻائي = 6;
            طالما ((udma_گهڻائي >= 0) &&
                   !((بفر[88] & 0x7f) & ( 1 << udma_گهڻائي ))) {
                udma_گهڻائي--;
            }
        }
        // If bit 1 in word 53 is set, multiword-dma and advanced pio modes
        // are available in words 63 and 64.
        إذا (بفر[53] & 0x02) {
            pio_گهڻائي = 4;
            multi_dma = 3;
            طالما ((multi_dma >= 0) &&
                   !((بفر[63] & 0x7) & ( 1 << multi_dma ))) {
                multi_dma--;
            }
            طالما ((pio_گهڻائي >= 3) &&
                   !((بفر[64] & 0x3) & ( 1 << ( pio_گهڻائي - 3 ) ))) {
                pio_گهڻائي--;
            }
        }
        dprintf(2, "AHCI/%d: supported modes: udma %d, multi-dma %d, pio %d\n",
                پورٽ->pnr, udma_گهڻائي, multi_dma, pio_گهڻائي);

        sata_prep_simple(&پورٽ->cmd->fis, ATA_CMD_سيٽ_FEATURES);
        پورٽ->cmd->fis.feature = ATA_سيٽ_FEATRUE_TRANSFER_گهڻائي;
        // Select used mode. UDMA first, then Multi-DMA followed by
        // advanced PIO modes 3 or 4. If non, set default PIO.
        إذا (udma_گهڻائي >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to UDMA-%d\n",
                    پورٽ->pnr, udma_گهڻائي);
            پورٽ->cmd->fis.sector_ڳڻپ = ATA_TRANSFER_گهڻائي_ULTRA_DMA
                                          | udma_گهڻائي;
        } وإلا إذا (multi_dma >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to Multi-DMA-%d\n",
                    پورٽ->pnr, multi_dma);
            پورٽ->cmd->fis.sector_ڳڻپ = ATA_TRANSFER_گهڻائي_MULTIWORD_DMA
                                          | multi_dma;
        } وإلا إذا (pio_گهڻائي >= 3) {
            dprintf(1, "AHCI/%d: Set transfer mode to PIO-%d\n",
                    پورٽ->pnr, pio_گهڻائي);
            پورٽ->cmd->fis.sector_ڳڻپ = ATA_TRANSFER_گهڻائي_PIO_FLOW_CTRL
                                          | pio_گهڻائي;
        } وإلا {
            dprintf(1, "AHCI/%d: Set transfer mode to default PIO\n",
                    پورٽ->pnr);
            پورٽ->cmd->fis.sector_ڳڻپ = ATA_TRANSFER_گهڻائي_DEFAULT_PIO;
        }
        rc = ahci_command(پورٽ, 1, 0, 0, 0);
        إذا (rc < 0) {
            dprintf(1, "AHCI/%d: Set transfer mode failed.\n", پورٽ->pnr);
        }
    } وإلا {
        // found cdrom (atapi)
        پورٽ->drive.type = DTYPE_AHCI_ATAPI;
        پورٽ->drive.blksize = CDROM_SECTOR_سائيز;
        پورٽ->drive.sectors = (u64)-1;
        u8 iscd = ((بفر[0] >> 8) & 0x1f) == 0x05;
        إذا (!iscd) {
            dprintf(1, "AHCI/%d: atapi device isn't a cdrom\n", پورٽ->pnr);
            أرجع -1;
        }
        پورٽ->desc = znprintf(MAXDESCSIZE
                              , "DVD/CD [AHCI/%d: %s ATAPI-%d DVD/CD]"
                              , پورٽ->pnr
                              , ata_ڪڍو_model(model, MAXMODEL, بفر)
                              , ata_ڪڍو_نسخو(بفر));
        پورٽ->prio = bootprio_ڳوليو_ata_device(ctrl->pci_tmp, pnr, 0);
    }
    boot_lchs_ڳوليو_ata_device(ctrl->pci_tmp, pnr, 0, &(پورٽ->drive.lchs));
    أرجع 0;
}

// Detect any drives attached to a given controller.
ساكن فراغ
ahci_پورٽ_detect(فراغ *معلومات)
{
    بنية ahci_پورٽ_s *پورٽ = معلومات;
    صحيح rc;

    dprintf(2, "AHCI/%d: probing\n", پورٽ->pnr);
    ahci_پورٽ_reset(پورٽ->ctrl, پورٽ->pnr);
    rc = ahci_پورٽ_تياري(پورٽ);
    إذا (rc < 0)
        ahci_پورٽ_release(پورٽ);
    وإلا {
        پورٽ = ahci_پورٽ_realloc(پورٽ);
        إذا (پورٽ == عدم)
            أرجع;
        dprintf(1, "AHCI/%d: registering: \"%s\"\n", پورٽ->pnr, پورٽ->desc);
        إذا (!پورٽ->atapi) {
            // Register with bcv system.
            boot_جوڙيو_hd(&پورٽ->drive, پورٽ->desc, پورٽ->prio);
        } وإلا {
            // fill cdidmap
            boot_جوڙيو_cd(&پورٽ->drive, پورٽ->desc, پورٽ->prio);
        }
    }
}

// Initialize an ata controller and detect its drives.
ساكن فراغ
ahci_controller_تياري(بنية pci_device *pci)
{
    بنية ahci_پورٽ_s *پورٽ;
    u32 pnr, max;

    إذا (create_bounce_buf() < 0)
        أرجع;

    فراغ *iobase = pci_enable_membar(pci, PCI_بنياد_پتو_5);
    إذا (!iobase)
        أرجع;

    بنية ahci_ctrl_s *ctrl = malloc_fseg(حجم(*ctrl));
    إذا (!ctrl) {
        warn_noalloc();
        أرجع;
    }

    ctrl->pci_tmp = pci;
    ctrl->iobase = iobase;
    ctrl->irq = pci_config_readb(pci->bdf, PCI_مداخلت_سٽ);
    dprintf(1, "AHCI controller at %pP, iobase %p, irq %d\n"
            , pci, ctrl->iobase, ctrl->irq);

    pci_enable_busmaster(pci);

    u32 val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_RESET);
    u32 پڇاڙي = timer_calc(AHCI_RESET_TIMEOUT);
    لكل (;;) {
        val = ahci_ctrl_readl(ctrl, HOST_CTL);
        إذا (!(val & HOST_CTL_RESET))
            اكسر;
        إذا (timer_check(پڇاڙي)) {
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
        پورٽ = ahci_پورٽ_alloc(ctrl, pnr);
        إذا (پورٽ == عدم)
            استمر;
        لاڳيتو_thread(ahci_پورٽ_detect, پورٽ);
    }
}

// Locate and init ahci controllers.
ساكن فراغ
ahci_جاچ(فراغ)
{
    // Scan PCI bus for ATA adapters
    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci->class != PCI_CLASS_STORAGE_SATA)
            استمر;
        إذا (pci->prog_if != 1 /* AHCI rev 1 */)
            استمر;
        ahci_controller_تياري(pci);
    }
}

فراغ
ahci_تياري(فراغ)
{
    ASSERT32FLAT();
    إذا (!CONFIG_AHCI)
        أرجع;

    dprintf(3, "init ahci\n");
    ahci_جاچ();
}

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

#تعریف_کړه AHCI_REQUEST_TIMEOUT 32000 // 32 seconds max for IDE ops
#تعریف_کړه AHCI_RESET_TIMEOUT     500 // 500 miliseconds
#تعریف_کړه AHCI_LINK_TIMEOUT       10 // 10 miliseconds

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

    إذا (op->شمېر >= (1<<8) || lba + op->شمېر >= (1<<28)) {
        fis->sector_count2 = op->شمېر >> 8;
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
    fis->sector_شمېر = op->شمېر;
    fis->lba_ټیټ      = lba;
    fis->lba_mid      = lba >> 8;
    fis->lba_لوړ     = lba >> 16;
    fis->device       = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;
}

ساكن فراغ sata_prep_atapi(بنية sata_cmd_fis *fis, u16 blocksize)
{
    memset_fl(fis, 0, حجم(*fis));
    fis->command  = ATA_CMD_PACKET;
    fis->feature  = 1; /* dma */
    fis->lba_mid  = blocksize;
    fis->lba_لوړ = blocksize >> 8;
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

ساكن u32 ahci_پورټ_منزل_ctrl(u32 pnr, u32 پورټ_reg)
{
    u32 ctrl_reg = 0x100;
    ctrl_reg += pnr * 0x80;
    ctrl_reg += پورټ_reg;
    أرجع ctrl_reg;
}

ساكن u32 ahci_پورټ_readl(بنية ahci_ctrl_s *ctrl, u32 pnr, u32 reg)
{
    u32 ctrl_reg = ahci_پورټ_منزل_ctrl(pnr, reg);
    أرجع ahci_ctrl_readl(ctrl, ctrl_reg);
}

ساكن فراغ ahci_پورټ_writel(بنية ahci_ctrl_s *ctrl, u32 pnr, u32 reg, u32 val)
{
    u32 ctrl_reg = ahci_پورټ_منزل_ctrl(pnr, reg);
    ahci_ctrl_writel(ctrl, ctrl_reg, val);
}

// submit ahci command + wait for result
ساكن صحيح ahci_command(بنية ahci_پورټ_s *پورټ_gf, صحيح iswrite, صحيح isatapi,
                        فراغ *بفر, u32 bsize)
{
    u32 val, حالت, success, flags, intbits, error;
    بنية ahci_ctrl_s *ctrl = پورټ_gf->ctrl;
    بنية ahci_cmd_s  *cmd  = پورټ_gf->cmd;
    بنية ahci_fis_s  *fis  = پورټ_gf->fis;
    بنية ahci_list_s *list = پورټ_gf->list;
    u32 pnr                  = پورټ_gf->pnr;

    cmd->fis.reg       = 0x27;
    cmd->fis.pmp_type  = 1 << 7; /* cmd fis */
    cmd->prdt[0].بنسټ  = (u32)بفر;
    cmd->prdt[0].baseu = 0;
    cmd->prdt[0].flags = bsize-1;

    flags = ((1 << 16) | /* one prd entry */
             (iswrite ? (1 << 6) : 0) |
             (isatapi ? (1 << 5) : 0) |
             (5 << 0)); /* fis length (dwords) */
    list[0].flags  = flags;
    list[0].بایټونه  = 0;
    list[0].بنسټ   = (u32)(cmd);
    list[0].baseu  = 0;

    dprintf(8, "AHCI/%d: send cmd ...\n", pnr);
    intbits = ahci_پورټ_readl(ctrl, pnr, پورټ_IRQ_STAT);
    إذا (intbits)
        ahci_پورټ_writel(ctrl, pnr, پورټ_IRQ_STAT, intbits);
    ahci_پورټ_writel(ctrl, pnr, پورټ_CMD_ISSUE, 1);

    u32 پای_ټکی = timer_calc(AHCI_REQUEST_TIMEOUT);
    افعل {
        لكل (;;) {
            intbits = ahci_پورټ_readl(ctrl, pnr, پورټ_IRQ_STAT);
            إذا (intbits) {
                ahci_پورټ_writel(ctrl, pnr, پورټ_IRQ_STAT, intbits);
                إذا (intbits & 0x40000000) {
                    u32 tf = ahci_پورټ_readl(ctrl, pnr, پورټ_TFDATA);
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
            إذا (timer_check(پای_ټکی)) {
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
        val = ahci_پورټ_readl(ctrl, pnr, پورټ_CMD);
        ahci_پورټ_writel(ctrl, pnr, پورټ_CMD, val & ~پورټ_CMD_پیل);

        // waits for PxCMD.CR to clear to 0
        طالما (1) {
            val = ahci_پورټ_readl(ctrl, pnr, پورټ_CMD);
            إذا ((val & پورټ_CMD_LIST_ON) == 0)
                اكسر;
            yield();
        }

        // Clears any error bits in PxSERR to enable capturing new errors
        val = ahci_پورټ_readl(ctrl, pnr, پورټ_SCR_ERR);
        ahci_پورټ_writel(ctrl, pnr, پورټ_SCR_ERR, val);

        // Clears status bits in PxIS as appropriate
        val = ahci_پورټ_readl(ctrl, pnr, پورټ_IRQ_STAT);
        ahci_پورټ_writel(ctrl, pnr, پورټ_IRQ_STAT, val);

        // If PxTFD.STS.BSY or PxTFD.STS.DRQ is set to 1, issue
        // a COMRESET to the device to put it in an idle state
        val = ahci_پورټ_readl(ctrl, pnr, پورټ_TFDATA);
        إذا (val & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ)) {
            dprintf(2, "AHCI/%d: issue comreset\n", pnr);
            val = ahci_پورټ_readl(ctrl, pnr, پورټ_SCR_CTL);
            // set Device Detection Initialization (DET) to 1 for 1 ms for comreset
            ahci_پورټ_writel(ctrl, pnr, پورټ_SCR_CTL, val | 1);
            mdelay (1);
            ahci_پورټ_writel(ctrl, pnr, پورټ_SCR_CTL, val);
        }

        // Sets PxCMD.ST to 1 to enable issuing new commands
        val = ahci_پورټ_readl(ctrl, pnr, پورټ_CMD);
        ahci_پورټ_writel(ctrl, pnr, پورټ_CMD, val | پورټ_CMD_پیل);
    }
    أرجع success ? 0 : -1;
}

#تعریف_کړه CDROM_CDB_اندازه 12

صحيح ahci_atapi_process_op(بنية disk_op_s *op)
{
    إذا (! CONFIG_AHCI)
        أرجع 0;

    بنية ahci_پورټ_s *پورټ_gf = container_د(
        op->drive_fl, بنية ahci_پورټ_s, drive);
    بنية ahci_cmd_s *cmd = پورټ_gf->cmd;

    إذا (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        أرجع DISK_RET_EWRITEPROTECT;
    صحيح blocksize = scsi_fill_cmd(op, cmd->atapi, CDROM_CDB_اندازه);
    إذا (blocksize < 0)
        أرجع default_process_op(op);
    sata_prep_atapi(&cmd->fis, blocksize);
    صحيح rc = ahci_command(پورټ_gf, 0, 1, op->buf_fl, op->شمېر * blocksize);
    إذا (rc < 0)
        أرجع DISK_RET_EBADTRACK;
    أرجع DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive, op->buf_fl must be word aligned
ساكن صحيح
ahci_disk_readwrite_aligned(بنية disk_op_s *op, صحيح iswrite)
{
    بنية ahci_پورټ_s *پورټ_gf = container_د(
        op->drive_fl, بنية ahci_پورټ_s, drive);
    بنية ahci_cmd_s *cmd = پورټ_gf->cmd;
    صحيح rc;

    sata_prep_readwrite(&cmd->fis, op, iswrite);
    rc = ahci_command(پورټ_gf, iswrite, 0, op->buf_fl,
                      op->شمېر * DISK_SECTOR_اندازه);
    dprintf(8, "ahci disk %s, lba %6x, count %3x, buf %p, rc %d\n",
            iswrite ? "write" : "read", (u32)op->lba, op->شمېر, op->buf_fl, rc);
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
    localop.شمېر = 1;

    إذا (iswrite) {
        u16 بلاک;
        لكل (بلاک = 0; بلاک < op->شمېر; بلاک++) {
            memcpy_fl (alignedbuf_fl, position, DISK_SECTOR_اندازه);
            rc = ahci_disk_readwrite_aligned (&localop, 1);
            إذا (rc)
                أرجع rc;
            position += DISK_SECTOR_اندازه;
            localop.lba++;
        }
    } وإلا { // read
        u16 بلاک;
        لكل (بلاک = 0; بلاک < op->شمېر; بلاک++) {
            rc = ahci_disk_readwrite_aligned (&localop, 0);
            إذا (rc)
                أرجع rc;
            memcpy_fl (position, alignedbuf_fl, DISK_SECTOR_اندازه);
            position += DISK_SECTOR_اندازه;
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
ahci_پورټ_reset(بنية ahci_ctrl_s *ctrl, u32 pnr)
{
    u32 val;

    /* disable FIS + CMD */
    u32 پای_ټکی = timer_calc(AHCI_RESET_TIMEOUT);
    لكل (;;) {
        val = ahci_پورټ_readl(ctrl, pnr, پورټ_CMD);
        إذا (!(val & (پورټ_CMD_FIS_RX | پورټ_CMD_پیل |
                     پورټ_CMD_FIS_ON | پورټ_CMD_LIST_ON)))
            اكسر;
        val &= ~(پورټ_CMD_FIS_RX | پورټ_CMD_پیل);
        ahci_پورټ_writel(ctrl, pnr, پورټ_CMD, val);
        إذا (timer_check(پای_ټکی)) {
            warn_timeout();
            اكسر;
        }
        yield();
    }

    /* disable + clear IRQs */
    ahci_پورټ_writel(ctrl, pnr, پورټ_IRQ_ماسک, 0);
    val = ahci_پورټ_readl(ctrl, pnr, پورټ_IRQ_STAT);
    إذا (val)
        ahci_پورټ_writel(ctrl, pnr, پورټ_IRQ_STAT, val);
}

ساكن بنية ahci_پورټ_s*
ahci_پورټ_alloc(بنية ahci_ctrl_s *ctrl, u32 pnr)
{
    بنية ahci_پورټ_s *پورټ = malloc_tmp(حجم(*پورټ));

    إذا (!پورټ) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(پورټ, 0, حجم(*پورټ));
    پورټ->pnr = pnr;
    پورټ->ctrl = ctrl;
    پورټ->list = memalign_tmp(1024, 1024);
    پورټ->fis = memalign_tmp(256, 256);
    پورټ->cmd = memalign_tmp(256, 256);
    إذا (پورټ->list == عدم || پورټ->fis == عدم || پورټ->cmd == عدم) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(پورټ->list, 0, 1024);
    memset(پورټ->fis, 0, 256);
    memset(پورټ->cmd, 0, 256);

    ahci_پورټ_writel(ctrl, pnr, پورټ_LST_ADDR, (u32)پورټ->list);
    ahci_پورټ_writel(ctrl, pnr, پورټ_FIS_ADDR, (u32)پورټ->fis);
    إذا (ctrl->caps & HOST_CAP_64) {
        ahci_پورټ_writel(ctrl, pnr, پورټ_LST_ADDR_HI, 0);
        ahci_پورټ_writel(ctrl, pnr, پورټ_FIS_ADDR_HI, 0);
    }

    أرجع پورټ;
}

ساكن فراغ ahci_پورټ_release(بنية ahci_پورټ_s *پورټ)
{
    ahci_پورټ_reset(پورټ->ctrl, پورټ->pnr);
    حرر(پورټ->list);
    حرر(پورټ->fis);
    حرر(پورټ->cmd);
    حرر(پورټ);
}

ساكن بنية ahci_پورټ_s* ahci_پورټ_realloc(بنية ahci_پورټ_s *پورټ)
{
    بنية ahci_پورټ_s *tmp;
    u32 cmd;

    tmp = malloc_fseg(حجم(*پورټ));
    إذا (!tmp) {
        warn_noalloc();
        ahci_پورټ_release(پورټ);
        أرجع عدم;
    }
    *tmp = *پورټ;
    حرر(پورټ);
    پورټ = tmp;

    ahci_پورټ_reset(پورټ->ctrl, پورټ->pnr);

    حرر(پورټ->list);
    حرر(پورټ->fis);
    حرر(پورټ->cmd);
    پورټ->list = memalign_لوړ(1024, 1024);
    پورټ->fis = memalign_لوړ(256, 256);
    پورټ->cmd = memalign_لوړ(256, 256);
    إذا (!پورټ->list || !پورټ->fis || !پورټ->cmd) {
        warn_noalloc();
        حرر(پورټ->list);
        حرر(پورټ->fis);
        حرر(پورټ->cmd);
        حرر(پورټ);
        أرجع عدم;
    }

    ahci_پورټ_writel(پورټ->ctrl, پورټ->pnr, پورټ_LST_ADDR, (u32)پورټ->list);
    ahci_پورټ_writel(پورټ->ctrl, پورټ->pnr, پورټ_FIS_ADDR, (u32)پورټ->fis);

    cmd = ahci_پورټ_readl(پورټ->ctrl, پورټ->pnr, پورټ_CMD);
    cmd |= (پورټ_CMD_FIS_RX|پورټ_CMD_پیل);
    ahci_پورټ_writel(پورټ->ctrl, پورټ->pnr, پورټ_CMD, cmd);

    أرجع پورټ;
}

#تعریف_کړه MAXMODEL 40

/* See ahci spec chapter 10.1 "Software Initialization of HBA" */
ساكن صحيح ahci_پورټ_تیاری(بنية ahci_پورټ_s *پورټ)
{
    بنية ahci_ctrl_s *ctrl = پورټ->ctrl;
    u32 pnr = پورټ->pnr;
    حرف model[MAXMODEL+1];
    u16 بفر[256];
    u32 cmd, stat, err, tf;
    صحيح rc;

    /* enable FIS recv */
    cmd = ahci_پورټ_readl(ctrl, pnr, پورټ_CMD);
    cmd |= پورټ_CMD_FIS_RX;
    ahci_پورټ_writel(ctrl, pnr, پورټ_CMD, cmd);

    /* spin up */
    cmd &= ~پورټ_CMD_ICC_ماسک;
    cmd |= پورټ_CMD_SPIN_پورته | پورټ_CMD_POWER_ON | پورټ_CMD_ICC_فعال;
    ahci_پورټ_writel(ctrl, pnr, پورټ_CMD, cmd);
    u32 پای_ټکی = timer_calc(AHCI_LINK_TIMEOUT);
    لكل (;;) {
        stat = ahci_پورټ_readl(ctrl, pnr, پورټ_SCR_STAT);
        إذا ((stat & 0x07) == 0x03) {
            dprintf(2, "AHCI/%d: link up\n", پورټ->pnr);
            اكسر;
        }
        إذا (timer_check(پای_ټکی)) {
            dprintf(2, "AHCI/%d: link down\n", پورټ->pnr);
            أرجع -1;
        }
        yield();
    }

    /* clear error status */
    err = ahci_پورټ_readl(ctrl, pnr, پورټ_SCR_ERR);
    إذا (err)
        ahci_پورټ_writel(ctrl, pnr, پورټ_SCR_ERR, err);

    /* wait for device becoming ready */
    پای_ټکی = timer_calc(AHCI_REQUEST_TIMEOUT);
    لكل (;;) {
        tf = ahci_پورټ_readl(ctrl, pnr, پورټ_TFDATA);
        إذا (!(tf & (ATA_CB_STAT_BSY |
                    ATA_CB_STAT_DRQ)))
            اكسر;
        إذا (timer_check(پای_ټکی)) {
            warn_timeout();
            dprintf(1, "AHCI/%d: device not ready (tf 0x%x)\n", پورټ->pnr, tf);
            أرجع -1;
        }
        yield();
    }

    /* start device */
    cmd |= پورټ_CMD_پیل;
    ahci_پورټ_writel(ctrl, pnr, پورټ_CMD, cmd);

    sata_prep_simple(&پورټ->cmd->fis, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    rc = ahci_command(پورټ, 0, 0, بفر, حجم(بفر));
    إذا (rc == 0) {
        پورټ->atapi = 1;
    } وإلا {
        پورټ->atapi = 0;
        sata_prep_simple(&پورټ->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
        rc = ahci_command(پورټ, 0, 0, بفر, حجم(بفر));
        إذا (rc < 0)
            أرجع -1;
    }

    پورټ->drive.cntl_id = pnr;
    پورټ->drive.removable = (بفر[0] & 0x80) ? 1 : 0;

    إذا (!پورټ->atapi) {
        // found disk (ata)
        پورټ->drive.type = DTYPE_AHCI;
        پورټ->drive.blksize = DISK_SECTOR_اندازه;
        پورټ->drive.pchs.cylinder = بفر[1];
        پورټ->drive.pchs.سر = بفر[3];
        پورټ->drive.pchs.sector = بفر[6];

        u64 sectors;
        إذا (بفر[83] & (1 << 10)) // word 83 - lba48 support
            sectors = *(u64*)&بفر[100]; // word 100-103
        وإلا
            sectors = *(u32*)&بفر[60]; // word 60 and word 61
        پورټ->drive.sectors = sectors;
        u64 adjsize = sectors >> 11;
        حرف adjprefix = 'M';
        إذا (adjsize >= (1 << 16)) {
            adjsize >>= 10;
            adjprefix = 'G';
        }
        پورټ->desc = znprintf(MAXDESCSIZE
                              , "AHCI/%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                              , پورټ->pnr
                              , ata_ایستل_model(model, MAXMODEL, بفر)
                              , ata_ایستل_نسخه(بفر)
                              , (u32)adjsize, adjprefix);
        پورټ->prio = bootprio_پیدا_کړه_ata_device(ctrl->pci_tmp, pnr, 0);

        s8 multi_dma = -1;
        s8 pio_ډېرپېښ = -1;
        s8 udma_ډېرپېښ = -1;
        // If bit 2 in word 53 is set, udma information is valid in word 88.
        إذا (بفر[53] & 0x04) {
            udma_ډېرپېښ = 6;
            طالما ((udma_ډېرپېښ >= 0) &&
                   !((بفر[88] & 0x7f) & ( 1 << udma_ډېرپېښ ))) {
                udma_ډېرپېښ--;
            }
        }
        // If bit 1 in word 53 is set, multiword-dma and advanced pio modes
        // are available in words 63 and 64.
        إذا (بفر[53] & 0x02) {
            pio_ډېرپېښ = 4;
            multi_dma = 3;
            طالما ((multi_dma >= 0) &&
                   !((بفر[63] & 0x7) & ( 1 << multi_dma ))) {
                multi_dma--;
            }
            طالما ((pio_ډېرپېښ >= 3) &&
                   !((بفر[64] & 0x3) & ( 1 << ( pio_ډېرپېښ - 3 ) ))) {
                pio_ډېرپېښ--;
            }
        }
        dprintf(2, "AHCI/%d: supported modes: udma %d, multi-dma %d, pio %d\n",
                پورټ->pnr, udma_ډېرپېښ, multi_dma, pio_ډېرپېښ);

        sata_prep_simple(&پورټ->cmd->fis, ATA_CMD_سېټ_FEATURES);
        پورټ->cmd->fis.feature = ATA_سېټ_FEATRUE_TRANSFER_ډېرپېښ;
        // Select used mode. UDMA first, then Multi-DMA followed by
        // advanced PIO modes 3 or 4. If non, set default PIO.
        إذا (udma_ډېرپېښ >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to UDMA-%d\n",
                    پورټ->pnr, udma_ډېرپېښ);
            پورټ->cmd->fis.sector_شمېر = ATA_TRANSFER_ډېرپېښ_ULTRA_DMA
                                          | udma_ډېرپېښ;
        } وإلا إذا (multi_dma >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to Multi-DMA-%d\n",
                    پورټ->pnr, multi_dma);
            پورټ->cmd->fis.sector_شمېر = ATA_TRANSFER_ډېرپېښ_MULTIWORD_DMA
                                          | multi_dma;
        } وإلا إذا (pio_ډېرپېښ >= 3) {
            dprintf(1, "AHCI/%d: Set transfer mode to PIO-%d\n",
                    پورټ->pnr, pio_ډېرپېښ);
            پورټ->cmd->fis.sector_شمېر = ATA_TRANSFER_ډېرپېښ_PIO_FLOW_CTRL
                                          | pio_ډېرپېښ;
        } وإلا {
            dprintf(1, "AHCI/%d: Set transfer mode to default PIO\n",
                    پورټ->pnr);
            پورټ->cmd->fis.sector_شمېر = ATA_TRANSFER_ډېرپېښ_DEFAULT_PIO;
        }
        rc = ahci_command(پورټ, 1, 0, 0, 0);
        إذا (rc < 0) {
            dprintf(1, "AHCI/%d: Set transfer mode failed.\n", پورټ->pnr);
        }
    } وإلا {
        // found cdrom (atapi)
        پورټ->drive.type = DTYPE_AHCI_ATAPI;
        پورټ->drive.blksize = CDROM_SECTOR_اندازه;
        پورټ->drive.sectors = (u64)-1;
        u8 iscd = ((بفر[0] >> 8) & 0x1f) == 0x05;
        إذا (!iscd) {
            dprintf(1, "AHCI/%d: atapi device isn't a cdrom\n", پورټ->pnr);
            أرجع -1;
        }
        پورټ->desc = znprintf(MAXDESCSIZE
                              , "DVD/CD [AHCI/%d: %s ATAPI-%d DVD/CD]"
                              , پورټ->pnr
                              , ata_ایستل_model(model, MAXMODEL, بفر)
                              , ata_ایستل_نسخه(بفر));
        پورټ->prio = bootprio_پیدا_کړه_ata_device(ctrl->pci_tmp, pnr, 0);
    }
    boot_lchs_پیدا_کړه_ata_device(ctrl->pci_tmp, pnr, 0, &(پورټ->drive.lchs));
    أرجع 0;
}

// Detect any drives attached to a given controller.
ساكن فراغ
ahci_پورټ_detect(فراغ *معلومات)
{
    بنية ahci_پورټ_s *پورټ = معلومات;
    صحيح rc;

    dprintf(2, "AHCI/%d: probing\n", پورټ->pnr);
    ahci_پورټ_reset(پورټ->ctrl, پورټ->pnr);
    rc = ahci_پورټ_تیاری(پورټ);
    إذا (rc < 0)
        ahci_پورټ_release(پورټ);
    وإلا {
        پورټ = ahci_پورټ_realloc(پورټ);
        إذا (پورټ == عدم)
            أرجع;
        dprintf(1, "AHCI/%d: registering: \"%s\"\n", پورټ->pnr, پورټ->desc);
        إذا (!پورټ->atapi) {
            // Register with bcv system.
            boot_جمع_کړه_hd(&پورټ->drive, پورټ->desc, پورټ->prio);
        } وإلا {
            // fill cdidmap
            boot_جمع_کړه_cd(&پورټ->drive, پورټ->desc, پورټ->prio);
        }
    }
}

// Initialize an ata controller and detect its drives.
ساكن فراغ
ahci_controller_تیاری(بنية pci_device *pci)
{
    بنية ahci_پورټ_s *پورټ;
    u32 pnr, max;

    إذا (create_bounce_buf() < 0)
        أرجع;

    فراغ *iobase = pci_enable_membar(pci, PCI_بنسټ_پته_5);
    إذا (!iobase)
        أرجع;

    بنية ahci_ctrl_s *ctrl = malloc_fseg(حجم(*ctrl));
    إذا (!ctrl) {
        warn_noalloc();
        أرجع;
    }

    ctrl->pci_tmp = pci;
    ctrl->iobase = iobase;
    ctrl->irq = pci_config_readb(pci->bdf, PCI_مداخله_کرښه);
    dprintf(1, "AHCI controller at %pP, iobase %p, irq %d\n"
            , pci, ctrl->iobase, ctrl->irq);

    pci_enable_busmaster(pci);

    u32 val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_RESET);
    u32 پای_ټکی = timer_calc(AHCI_RESET_TIMEOUT);
    لكل (;;) {
        val = ahci_ctrl_readl(ctrl, HOST_CTL);
        إذا (!(val & HOST_CTL_RESET))
            اكسر;
        إذا (timer_check(پای_ټکی)) {
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
        پورټ = ahci_پورټ_alloc(ctrl, pnr);
        إذا (پورټ == عدم)
            استمر;
        پرلپسې_thread(ahci_پورټ_detect, پورټ);
    }
}

// Locate and init ahci controllers.
ساكن فراغ
ahci_لټون(فراغ)
{
    // Scan PCI bus for ATA adapters
    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci->class != PCI_CLASS_STORAGE_SATA)
            استمر;
        إذا (pci->prog_if != 1 /* AHCI rev 1 */)
            استمر;
        ahci_controller_تیاری(pci);
    }
}

فراغ
ahci_تیاری(فراغ)
{
    ASSERT32FLAT();
    إذا (!CONFIG_AHCI)
        أرجع;

    dprintf(3, "init ahci\n");
    ahci_لټون();
}

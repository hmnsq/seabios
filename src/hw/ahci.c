// Low level AHCI disk access
//
// Copyright (C) 2010 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "ahci.h" // CDB_CMD_READ_10
#شامل "ata.h" // ATA_CB_STAT
#شامل "biosvar.h" // GET_GLOBAL
#شامل "blockcmd.h" // CDB_CMD_READ_10
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "pci.h" // pci_config_readb
#شامل "pcidevice.h" // foreachpci
#شامل "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#شامل "pci_regs.h" // PCI_INTERRUPT_LINE
#شامل "stacks.h" // yield
#شامل "std/disk.h" // DISK_RET_SUCCESS
#شامل "string.h" // memset
#شامل "util.h" // timer_calc
#شامل "x86.h" // inb

#تعریف_کرو AHCI_REQUEST_TIMEOUT 32000 // 32 seconds max for IDE ops
#تعریف_کرو AHCI_RESET_TIMEOUT     500 // 500 miliseconds
#تعریف_کرو AHCI_LINK_TIMEOUT       10 // 10 miliseconds

// prepare sata command fis
ساکن خالی sata_prep_simple(ساخت sata_cmd_fis *fis, u8 command)
{
    memset_fl(fis, 0, حجم(*fis));
    fis->command = command;
}

ساکن خالی sata_prep_readwrite(ساخت sata_cmd_fis *fis,
                                ساخت disk_op_s *op, صحیح iswrite)
{
    u64 lba = op->lba;
    u8 command;

    memset_fl(fis, 0, حجم(*fis));

    اگر (op->گنتی >= (1<<8) || lba + op->گنتی >= (1<<28)) {
        fis->sector_count2 = op->گنتی >> 8;
        fis->lba_low2      = lba >> 24;
        fis->lba_mid2      = lba >> 32;
        fis->lba_high2     = lba >> 40;
        lba &= 0xffffff;
        command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                   : ATA_CMD_READ_DMA_EXT);
    } ورنہ {
        command = (iswrite ? ATA_CMD_WRITE_DMA
                   : ATA_CMD_READ_DMA);
    }
    fis->feature      = 1; /* dma */
    fis->command      = command;
    fis->sector_گنتی = op->گنتی;
    fis->lba_کم      = lba;
    fis->lba_mid      = lba >> 8;
    fis->lba_بلند     = lba >> 16;
    fis->device       = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;
}

ساکن خالی sata_prep_atapi(ساخت sata_cmd_fis *fis, u16 blocksize)
{
    memset_fl(fis, 0, حجم(*fis));
    fis->command  = ATA_CMD_PACKET;
    fis->feature  = 1; /* dma */
    fis->lba_mid  = blocksize;
    fis->lba_بلند = blocksize >> 8;
}

// ahci register access helpers
ساکن u32 ahci_ctrl_readl(ساخت ahci_ctrl_s *ctrl, u32 reg)
{
    واپس readl(ctrl->iobase + reg);
}

ساکن خالی ahci_ctrl_writel(ساخت ahci_ctrl_s *ctrl, u32 reg, u32 val)
{
    writel(ctrl->iobase + reg, val);
}

ساکن u32 ahci_پورٹ_مقصد_ctrl(u32 pnr, u32 پورٹ_reg)
{
    u32 ctrl_reg = 0x100;
    ctrl_reg += pnr * 0x80;
    ctrl_reg += پورٹ_reg;
    واپس ctrl_reg;
}

ساکن u32 ahci_پورٹ_readl(ساخت ahci_ctrl_s *ctrl, u32 pnr, u32 reg)
{
    u32 ctrl_reg = ahci_پورٹ_مقصد_ctrl(pnr, reg);
    واپس ahci_ctrl_readl(ctrl, ctrl_reg);
}

ساکن خالی ahci_پورٹ_writel(ساخت ahci_ctrl_s *ctrl, u32 pnr, u32 reg, u32 val)
{
    u32 ctrl_reg = ahci_پورٹ_مقصد_ctrl(pnr, reg);
    ahci_ctrl_writel(ctrl, ctrl_reg, val);
}

// submit ahci command + wait for result
ساکن صحیح ahci_command(ساخت ahci_پورٹ_s *پورٹ_gf, صحیح iswrite, صحیح isatapi,
                        خالی *بفر, u32 bsize)
{
    u32 val, حالت_2, success, flags, intbits, error;
    ساخت ahci_ctrl_s *ctrl = پورٹ_gf->ctrl;
    ساخت ahci_cmd_s  *cmd  = پورٹ_gf->cmd;
    ساخت ahci_fis_s  *fis  = پورٹ_gf->fis;
    ساخت ahci_list_s *list = پورٹ_gf->list;
    u32 pnr                  = پورٹ_gf->pnr;

    cmd->fis.reg       = 0x27;
    cmd->fis.pmp_type  = 1 << 7; /* cmd fis */
    cmd->prdt[0].بنیاد  = (u32)بفر;
    cmd->prdt[0].baseu = 0;
    cmd->prdt[0].flags = bsize-1;

    flags = ((1 << 16) | /* one prd entry */
             (iswrite ? (1 << 6) : 0) |
             (isatapi ? (1 << 5) : 0) |
             (5 << 0)); /* fis length (dwords) */
    list[0].flags  = flags;
    list[0].بائٹیں  = 0;
    list[0].بنیاد   = (u32)(cmd);
    list[0].baseu  = 0;

    dprintf(8, "AHCI/%d: send cmd ...\n", pnr);
    intbits = ahci_پورٹ_readl(ctrl, pnr, پورٹ_IRQ_STAT);
    اگر (intbits)
        ahci_پورٹ_writel(ctrl, pnr, پورٹ_IRQ_STAT, intbits);
    ahci_پورٹ_writel(ctrl, pnr, پورٹ_CMD_ISSUE, 1);

    u32 انتہا = timer_calc(AHCI_REQUEST_TIMEOUT);
    کرو {
        برائے (;;) {
            intbits = ahci_پورٹ_readl(ctrl, pnr, پورٹ_IRQ_STAT);
            اگر (intbits) {
                ahci_پورٹ_writel(ctrl, pnr, پورٹ_IRQ_STAT, intbits);
                اگر (intbits & 0x40000000) {
                    u32 tf = ahci_پورٹ_readl(ctrl, pnr, پورٹ_TFDATA);
                    حالت_2 = tf & 0xff;
                    error = (tf & 0xff00) >> 8;
                    توڑو;
                }
                اگر (intbits & 0x02) {
                    حالت_2 = GET_LOWFLAT(fis->psfis[2]);
                    error  = GET_LOWFLAT(fis->psfis[3]);
                    توڑو;
                }
                اگر (intbits & 0x01) {
                    حالت_2 = GET_LOWFLAT(fis->rfis[2]);
                    error  = GET_LOWFLAT(fis->rfis[3]);
                    توڑو;
                }
            }
            اگر (timer_check(انتہا)) {
                warn_timeout();
                واپس -1;
            }
            yield();
        }
        dprintf(8, "AHCI/%d: ... intbits 0x%x, status 0x%x ...\n",
                pnr, intbits, حالت_2);
    } جب (حالت_2 & ATA_CB_STAT_BSY);

    success = (0x00 == (حالت_2 & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF |
                                  ATA_CB_STAT_ERR)) &&
               ATA_CB_STAT_RDY == (حالت_2 & (ATA_CB_STAT_RDY)));
    اگر (success) {
        dprintf(8, "AHCI/%d: ... finished, status 0x%x, OK\n", pnr,
                حالت_2);
    } ورنہ {
        dprintf(2, "AHCI/%d: ... finished, status 0x%x, ERROR 0x%x\n", pnr,
                حالت_2, error);

        // non-queued error recovery (AHCI 1.3 section 6.2.2.1)
        // Clears PxCMD.ST to 0 to reset the PxCI register
        val = ahci_پورٹ_readl(ctrl, pnr, پورٹ_CMD);
        ahci_پورٹ_writel(ctrl, pnr, پورٹ_CMD, val & ~پورٹ_CMD_آغاز);

        // waits for PxCMD.CR to clear to 0
        جب (1) {
            val = ahci_پورٹ_readl(ctrl, pnr, پورٹ_CMD);
            اگر ((val & پورٹ_CMD_LIST_ON) == 0)
                توڑو;
            yield();
        }

        // Clears any error bits in PxSERR to enable capturing new errors
        val = ahci_پورٹ_readl(ctrl, pnr, پورٹ_SCR_ERR);
        ahci_پورٹ_writel(ctrl, pnr, پورٹ_SCR_ERR, val);

        // Clears status bits in PxIS as appropriate
        val = ahci_پورٹ_readl(ctrl, pnr, پورٹ_IRQ_STAT);
        ahci_پورٹ_writel(ctrl, pnr, پورٹ_IRQ_STAT, val);

        // If PxTFD.STS.BSY or PxTFD.STS.DRQ is set to 1, issue
        // a COMRESET to the device to put it in an idle state
        val = ahci_پورٹ_readl(ctrl, pnr, پورٹ_TFDATA);
        اگر (val & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ)) {
            dprintf(2, "AHCI/%d: issue comreset\n", pnr);
            val = ahci_پورٹ_readl(ctrl, pnr, پورٹ_SCR_CTL);
            // set Device Detection Initialization (DET) to 1 for 1 ms for comreset
            ahci_پورٹ_writel(ctrl, pnr, پورٹ_SCR_CTL, val | 1);
            mdelay (1);
            ahci_پورٹ_writel(ctrl, pnr, پورٹ_SCR_CTL, val);
        }

        // Sets PxCMD.ST to 1 to enable issuing new commands
        val = ahci_پورٹ_readl(ctrl, pnr, پورٹ_CMD);
        ahci_پورٹ_writel(ctrl, pnr, پورٹ_CMD, val | پورٹ_CMD_آغاز);
    }
    واپس success ? 0 : -1;
}

#تعریف_کرو CDROM_CDB_سائز 12

صحیح ahci_atapi_process_op(ساخت disk_op_s *op)
{
    اگر (! CONFIG_AHCI)
        واپس 0;

    ساخت ahci_پورٹ_s *پورٹ_gf = container_کا(
        op->drive_fl, ساخت ahci_پورٹ_s, drive);
    ساخت ahci_cmd_s *cmd = پورٹ_gf->cmd;

    اگر (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        واپس DISK_RET_EWRITEPROTECT;
    صحیح blocksize = scsi_fill_cmd(op, cmd->atapi, CDROM_CDB_سائز);
    اگر (blocksize < 0)
        واپس default_process_op(op);
    sata_prep_atapi(&cmd->fis, blocksize);
    صحیح rc = ahci_command(پورٹ_gf, 0, 1, op->buf_fl, op->گنتی * blocksize);
    اگر (rc < 0)
        واپس DISK_RET_EBADTRACK;
    واپس DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive, op->buf_fl must be word aligned
ساکن صحیح
ahci_disk_readwrite_aligned(ساخت disk_op_s *op, صحیح iswrite)
{
    ساخت ahci_پورٹ_s *پورٹ_gf = container_کا(
        op->drive_fl, ساخت ahci_پورٹ_s, drive);
    ساخت ahci_cmd_s *cmd = پورٹ_gf->cmd;
    صحیح rc;

    sata_prep_readwrite(&cmd->fis, op, iswrite);
    rc = ahci_command(پورٹ_gf, iswrite, 0, op->buf_fl,
                      op->گنتی * DISK_SECTOR_سائز);
    dprintf(8, "ahci disk %s, lba %6x, count %3x, buf %p, rc %d\n",
            iswrite ? "write" : "read", (u32)op->lba, op->گنتی, op->buf_fl, rc);
    اگر (rc < 0)
        واپس DISK_RET_EBADTRACK;
    واپس DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive.
ساکن صحیح
ahci_disk_readwrite(ساخت disk_op_s *op, صحیح iswrite)
{
    // if caller's buffer is word aligned, use it directly
    اگر (((u32) op->buf_fl & 1) == 0)
        واپس ahci_disk_readwrite_aligned(op, iswrite);

    // Use a word aligned buffer for AHCI I/O
    صحیح rc;
    ساخت disk_op_s localop = *op;
    u8 *alignedbuf_fl = bounce_buf_fl;
    u8 *position = op->buf_fl;

    localop.buf_fl = alignedbuf_fl;
    localop.گنتی = 1;

    اگر (iswrite) {
        u16 بلاک;
        برائے (بلاک = 0; بلاک < op->گنتی; بلاک++) {
            memcpy_fl (alignedbuf_fl, position, DISK_SECTOR_سائز);
            rc = ahci_disk_readwrite_aligned (&localop, 1);
            اگر (rc)
                واپس rc;
            position += DISK_SECTOR_سائز;
            localop.lba++;
        }
    } ورنہ { // read
        u16 بلاک;
        برائے (بلاک = 0; بلاک < op->گنتی; بلاک++) {
            rc = ahci_disk_readwrite_aligned (&localop, 0);
            اگر (rc)
                واپس rc;
            memcpy_fl (position, alignedbuf_fl, DISK_SECTOR_سائز);
            position += DISK_SECTOR_سائز;
            localop.lba++;
        }
    }
    واپس DISK_RET_SUCCESS;
}

// command demuxer
صحیح
ahci_process_op(ساخت disk_op_s *op)
{
    اگر (!CONFIG_AHCI)
        واپس 0;
    انتخاب (op->command) {
    حالت CMD_READ:
        واپس ahci_disk_readwrite(op, 0);
    حالت CMD_WRITE:
        واپس ahci_disk_readwrite(op, 1);
    طےشدہ:
        واپس default_process_op(op);
    }
}

ساکن خالی
ahci_پورٹ_reset(ساخت ahci_ctrl_s *ctrl, u32 pnr)
{
    u32 val;

    /* disable FIS + CMD */
    u32 انتہا = timer_calc(AHCI_RESET_TIMEOUT);
    برائے (;;) {
        val = ahci_پورٹ_readl(ctrl, pnr, پورٹ_CMD);
        اگر (!(val & (پورٹ_CMD_FIS_RX | پورٹ_CMD_آغاز |
                     پورٹ_CMD_FIS_ON | پورٹ_CMD_LIST_ON)))
            توڑو;
        val &= ~(پورٹ_CMD_FIS_RX | پورٹ_CMD_آغاز);
        ahci_پورٹ_writel(ctrl, pnr, پورٹ_CMD, val);
        اگر (timer_check(انتہا)) {
            warn_timeout();
            توڑو;
        }
        yield();
    }

    /* disable + clear IRQs */
    ahci_پورٹ_writel(ctrl, pnr, پورٹ_IRQ_نقاب, 0);
    val = ahci_پورٹ_readl(ctrl, pnr, پورٹ_IRQ_STAT);
    اگر (val)
        ahci_پورٹ_writel(ctrl, pnr, پورٹ_IRQ_STAT, val);
}

ساکن ساخت ahci_پورٹ_s*
ahci_پورٹ_alloc(ساخت ahci_ctrl_s *ctrl, u32 pnr)
{
    ساخت ahci_پورٹ_s *پورٹ = malloc_tmp(حجم(*پورٹ));

    اگر (!پورٹ) {
        warn_noalloc();
        واپس تہی;
    }
    memset(پورٹ, 0, حجم(*پورٹ));
    پورٹ->pnr = pnr;
    پورٹ->ctrl = ctrl;
    پورٹ->list = memalign_tmp(1024, 1024);
    پورٹ->fis = memalign_tmp(256, 256);
    پورٹ->cmd = memalign_tmp(256, 256);
    اگر (پورٹ->list == تہی || پورٹ->fis == تہی || پورٹ->cmd == تہی) {
        warn_noalloc();
        واپس تہی;
    }
    memset(پورٹ->list, 0, 1024);
    memset(پورٹ->fis, 0, 256);
    memset(پورٹ->cmd, 0, 256);

    ahci_پورٹ_writel(ctrl, pnr, پورٹ_LST_ADDR, (u32)پورٹ->list);
    ahci_پورٹ_writel(ctrl, pnr, پورٹ_FIS_ADDR, (u32)پورٹ->fis);
    اگر (ctrl->caps & HOST_CAP_64) {
        ahci_پورٹ_writel(ctrl, pnr, پورٹ_LST_ADDR_HI, 0);
        ahci_پورٹ_writel(ctrl, pnr, پورٹ_FIS_ADDR_HI, 0);
    }

    واپس پورٹ;
}

ساکن خالی ahci_پورٹ_release(ساخت ahci_پورٹ_s *پورٹ)
{
    ahci_پورٹ_reset(پورٹ->ctrl, پورٹ->pnr);
    آزاد(پورٹ->list);
    آزاد(پورٹ->fis);
    آزاد(پورٹ->cmd);
    آزاد(پورٹ);
}

ساکن ساخت ahci_پورٹ_s* ahci_پورٹ_realloc(ساخت ahci_پورٹ_s *پورٹ)
{
    ساخت ahci_پورٹ_s *tmp;
    u32 cmd;

    tmp = malloc_fseg(حجم(*پورٹ));
    اگر (!tmp) {
        warn_noalloc();
        ahci_پورٹ_release(پورٹ);
        واپس تہی;
    }
    *tmp = *پورٹ;
    آزاد(پورٹ);
    پورٹ = tmp;

    ahci_پورٹ_reset(پورٹ->ctrl, پورٹ->pnr);

    آزاد(پورٹ->list);
    آزاد(پورٹ->fis);
    آزاد(پورٹ->cmd);
    پورٹ->list = memalign_بلند(1024, 1024);
    پورٹ->fis = memalign_بلند(256, 256);
    پورٹ->cmd = memalign_بلند(256, 256);
    اگر (!پورٹ->list || !پورٹ->fis || !پورٹ->cmd) {
        warn_noalloc();
        آزاد(پورٹ->list);
        آزاد(پورٹ->fis);
        آزاد(پورٹ->cmd);
        آزاد(پورٹ);
        واپس تہی;
    }

    ahci_پورٹ_writel(پورٹ->ctrl, پورٹ->pnr, پورٹ_LST_ADDR, (u32)پورٹ->list);
    ahci_پورٹ_writel(پورٹ->ctrl, پورٹ->pnr, پورٹ_FIS_ADDR, (u32)پورٹ->fis);

    cmd = ahci_پورٹ_readl(پورٹ->ctrl, پورٹ->pnr, پورٹ_CMD);
    cmd |= (پورٹ_CMD_FIS_RX|پورٹ_CMD_آغاز);
    ahci_پورٹ_writel(پورٹ->ctrl, پورٹ->pnr, پورٹ_CMD, cmd);

    واپس پورٹ;
}

#تعریف_کرو MAXMODEL 40

/* See ahci spec chapter 10.1 "Software Initialization of HBA" */
ساکن صحیح ahci_پورٹ_تیاری(ساخت ahci_پورٹ_s *پورٹ)
{
    ساخت ahci_ctrl_s *ctrl = پورٹ->ctrl;
    u32 pnr = پورٹ->pnr;
    حرف model[MAXMODEL+1];
    u16 بفر[256];
    u32 cmd, stat, err, tf;
    صحیح rc;

    /* enable FIS recv */
    cmd = ahci_پورٹ_readl(ctrl, pnr, پورٹ_CMD);
    cmd |= پورٹ_CMD_FIS_RX;
    ahci_پورٹ_writel(ctrl, pnr, پورٹ_CMD, cmd);

    /* spin up */
    cmd &= ~پورٹ_CMD_ICC_نقاب;
    cmd |= پورٹ_CMD_SPIN_اوپر | پورٹ_CMD_POWER_ON | پورٹ_CMD_ICC_فعال;
    ahci_پورٹ_writel(ctrl, pnr, پورٹ_CMD, cmd);
    u32 انتہا = timer_calc(AHCI_LINK_TIMEOUT);
    برائے (;;) {
        stat = ahci_پورٹ_readl(ctrl, pnr, پورٹ_SCR_STAT);
        اگر ((stat & 0x07) == 0x03) {
            dprintf(2, "AHCI/%d: link up\n", پورٹ->pnr);
            توڑو;
        }
        اگر (timer_check(انتہا)) {
            dprintf(2, "AHCI/%d: link down\n", پورٹ->pnr);
            واپس -1;
        }
        yield();
    }

    /* clear error status */
    err = ahci_پورٹ_readl(ctrl, pnr, پورٹ_SCR_ERR);
    اگر (err)
        ahci_پورٹ_writel(ctrl, pnr, پورٹ_SCR_ERR, err);

    /* wait for device becoming ready */
    انتہا = timer_calc(AHCI_REQUEST_TIMEOUT);
    برائے (;;) {
        tf = ahci_پورٹ_readl(ctrl, pnr, پورٹ_TFDATA);
        اگر (!(tf & (ATA_CB_STAT_BSY |
                    ATA_CB_STAT_DRQ)))
            توڑو;
        اگر (timer_check(انتہا)) {
            warn_timeout();
            dprintf(1, "AHCI/%d: device not ready (tf 0x%x)\n", پورٹ->pnr, tf);
            واپس -1;
        }
        yield();
    }

    /* start device */
    cmd |= پورٹ_CMD_آغاز;
    ahci_پورٹ_writel(ctrl, pnr, پورٹ_CMD, cmd);

    sata_prep_simple(&پورٹ->cmd->fis, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    rc = ahci_command(پورٹ, 0, 0, بفر, حجم(بفر));
    اگر (rc == 0) {
        پورٹ->atapi = 1;
    } ورنہ {
        پورٹ->atapi = 0;
        sata_prep_simple(&پورٹ->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
        rc = ahci_command(پورٹ, 0, 0, بفر, حجم(بفر));
        اگر (rc < 0)
            واپس -1;
    }

    پورٹ->drive.cntl_id = pnr;
    پورٹ->drive.removable = (بفر[0] & 0x80) ? 1 : 0;

    اگر (!پورٹ->atapi) {
        // found disk (ata)
        پورٹ->drive.type = DTYPE_AHCI;
        پورٹ->drive.blksize = DISK_SECTOR_سائز;
        پورٹ->drive.pchs.cylinder = بفر[1];
        پورٹ->drive.pchs.سر = بفر[3];
        پورٹ->drive.pchs.sector = بفر[6];

        u64 sectors;
        اگر (بفر[83] & (1 << 10)) // word 83 - lba48 support
            sectors = *(u64*)&بفر[100]; // word 100-103
        ورنہ
            sectors = *(u32*)&بفر[60]; // word 60 and word 61
        پورٹ->drive.sectors = sectors;
        u64 adjsize = sectors >> 11;
        حرف adjprefix = 'M';
        اگر (adjsize >= (1 << 16)) {
            adjsize >>= 10;
            adjprefix = 'G';
        }
        پورٹ->desc = znprintf(MAXDESCSIZE
                              , "AHCI/%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                              , پورٹ->pnr
                              , ata_نکالو_model(model, MAXMODEL, بفر)
                              , ata_نکالو_نسخہ(بفر)
                              , (u32)adjsize, adjprefix);
        پورٹ->prio = bootprio_ڈھونڈو_ata_device(ctrl->pci_tmp, pnr, 0);

        s8 multi_dma = -1;
        s8 pio_منوال = -1;
        s8 udma_منوال = -1;
        // If bit 2 in word 53 is set, udma information is valid in word 88.
        اگر (بفر[53] & 0x04) {
            udma_منوال = 6;
            جب ((udma_منوال >= 0) &&
                   !((بفر[88] & 0x7f) & ( 1 << udma_منوال ))) {
                udma_منوال--;
            }
        }
        // If bit 1 in word 53 is set, multiword-dma and advanced pio modes
        // are available in words 63 and 64.
        اگر (بفر[53] & 0x02) {
            pio_منوال = 4;
            multi_dma = 3;
            جب ((multi_dma >= 0) &&
                   !((بفر[63] & 0x7) & ( 1 << multi_dma ))) {
                multi_dma--;
            }
            جب ((pio_منوال >= 3) &&
                   !((بفر[64] & 0x3) & ( 1 << ( pio_منوال - 3 ) ))) {
                pio_منوال--;
            }
        }
        dprintf(2, "AHCI/%d: supported modes: udma %d, multi-dma %d, pio %d\n",
                پورٹ->pnr, udma_منوال, multi_dma, pio_منوال);

        sata_prep_simple(&پورٹ->cmd->fis, ATA_CMD_سیٹ_FEATURES);
        پورٹ->cmd->fis.feature = ATA_سیٹ_FEATRUE_TRANSFER_منوال;
        // Select used mode. UDMA first, then Multi-DMA followed by
        // advanced PIO modes 3 or 4. If non, set default PIO.
        اگر (udma_منوال >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to UDMA-%d\n",
                    پورٹ->pnr, udma_منوال);
            پورٹ->cmd->fis.sector_گنتی = ATA_TRANSFER_منوال_ULTRA_DMA
                                          | udma_منوال;
        } ورنہ اگر (multi_dma >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to Multi-DMA-%d\n",
                    پورٹ->pnr, multi_dma);
            پورٹ->cmd->fis.sector_گنتی = ATA_TRANSFER_منوال_MULTIWORD_DMA
                                          | multi_dma;
        } ورنہ اگر (pio_منوال >= 3) {
            dprintf(1, "AHCI/%d: Set transfer mode to PIO-%d\n",
                    پورٹ->pnr, pio_منوال);
            پورٹ->cmd->fis.sector_گنتی = ATA_TRANSFER_منوال_PIO_FLOW_CTRL
                                          | pio_منوال;
        } ورنہ {
            dprintf(1, "AHCI/%d: Set transfer mode to default PIO\n",
                    پورٹ->pnr);
            پورٹ->cmd->fis.sector_گنتی = ATA_TRANSFER_منوال_DEFAULT_PIO;
        }
        rc = ahci_command(پورٹ, 1, 0, 0, 0);
        اگر (rc < 0) {
            dprintf(1, "AHCI/%d: Set transfer mode failed.\n", پورٹ->pnr);
        }
    } ورنہ {
        // found cdrom (atapi)
        پورٹ->drive.type = DTYPE_AHCI_ATAPI;
        پورٹ->drive.blksize = CDROM_SECTOR_سائز;
        پورٹ->drive.sectors = (u64)-1;
        u8 iscd = ((بفر[0] >> 8) & 0x1f) == 0x05;
        اگر (!iscd) {
            dprintf(1, "AHCI/%d: atapi device isn't a cdrom\n", پورٹ->pnr);
            واپس -1;
        }
        پورٹ->desc = znprintf(MAXDESCSIZE
                              , "DVD/CD [AHCI/%d: %s ATAPI-%d DVD/CD]"
                              , پورٹ->pnr
                              , ata_نکالو_model(model, MAXMODEL, بفر)
                              , ata_نکالو_نسخہ(بفر));
        پورٹ->prio = bootprio_ڈھونڈو_ata_device(ctrl->pci_tmp, pnr, 0);
    }
    boot_lchs_ڈھونڈو_ata_device(ctrl->pci_tmp, pnr, 0, &(پورٹ->drive.lchs));
    واپس 0;
}

// Detect any drives attached to a given controller.
ساکن خالی
ahci_پورٹ_detect(خالی *ڈیٹا)
{
    ساخت ahci_پورٹ_s *پورٹ = ڈیٹا;
    صحیح rc;

    dprintf(2, "AHCI/%d: probing\n", پورٹ->pnr);
    ahci_پورٹ_reset(پورٹ->ctrl, پورٹ->pnr);
    rc = ahci_پورٹ_تیاری(پورٹ);
    اگر (rc < 0)
        ahci_پورٹ_release(پورٹ);
    ورنہ {
        پورٹ = ahci_پورٹ_realloc(پورٹ);
        اگر (پورٹ == تہی)
            واپس;
        dprintf(1, "AHCI/%d: registering: \"%s\"\n", پورٹ->pnr, پورٹ->desc);
        اگر (!پورٹ->atapi) {
            // Register with bcv system.
            boot_جوڑو_hd(&پورٹ->drive, پورٹ->desc, پورٹ->prio);
        } ورنہ {
            // fill cdidmap
            boot_جوڑو_cd(&پورٹ->drive, پورٹ->desc, پورٹ->prio);
        }
    }
}

// Initialize an ata controller and detect its drives.
ساکن خالی
ahci_controller_تیاری(ساخت pci_device *pci)
{
    ساخت ahci_پورٹ_s *پورٹ;
    u32 pnr, max;

    اگر (create_bounce_buf() < 0)
        واپس;

    خالی *iobase = pci_enable_membar(pci, PCI_بنیاد_پتہ_5);
    اگر (!iobase)
        واپس;

    ساخت ahci_ctrl_s *ctrl = malloc_fseg(حجم(*ctrl));
    اگر (!ctrl) {
        warn_noalloc();
        واپس;
    }

    ctrl->pci_tmp = pci;
    ctrl->iobase = iobase;
    ctrl->irq = pci_config_readb(pci->bdf, PCI_مداخلت_سطر);
    dprintf(1, "AHCI controller at %pP, iobase %p, irq %d\n"
            , pci, ctrl->iobase, ctrl->irq);

    pci_enable_busmaster(pci);

    u32 val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_RESET);
    u32 انتہا = timer_calc(AHCI_RESET_TIMEOUT);
    برائے (;;) {
        val = ahci_ctrl_readl(ctrl, HOST_CTL);
        اگر (!(val & HOST_CTL_RESET))
            توڑو;
        اگر (timer_check(انتہا)) {
            warn_timeout();
            dprintf(1, "AHCI: controller reset failed\n");
            آزاد(ctrl);
            واپس;
        }
        yield();
    }
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_AHCI_EN);
    (خالی)ahci_ctrl_readl(ctrl, HOST_CTL); /* Flush */

    ctrl->caps = ahci_ctrl_readl(ctrl, HOST_CAP);
    ctrl->ports = ahci_ctrl_readl(ctrl, HOST_PORTS_IMPL);
    dprintf(2, "AHCI: cap 0x%x, ports_impl 0x%x\n",
            ctrl->caps, ctrl->ports);

    max = 0x1f;
    برائے (pnr = 0; pnr <= max; pnr++) {
        اگر (!(ctrl->ports & (1 << pnr)))
            جاری;
        پورٹ = ahci_پورٹ_alloc(ctrl, pnr);
        اگر (پورٹ == تہی)
            جاری;
        تسلسل_thread(ahci_پورٹ_detect, پورٹ);
    }
}

// Locate and init ahci controllers.
ساکن خالی
ahci_جائزہ(خالی)
{
    // Scan PCI bus for ATA adapters
    ساخت pci_device *pci;
    foreachpci(pci) {
        اگر (pci->class != PCI_CLASS_STORAGE_SATA)
            جاری;
        اگر (pci->prog_if != 1 /* AHCI rev 1 */)
            جاری;
        ahci_controller_تیاری(pci);
    }
}

خالی
ahci_تیاری(خالی)
{
    ASSERT32FLAT();
    اگر (!CONFIG_AHCI)
        واپس;

    dprintf(3, "init ahci\n");
    ahci_جائزہ();
}

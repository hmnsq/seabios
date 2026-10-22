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

#تعریف_کن AHCI_REQUEST_TIMEOUT 32000 // 32 seconds max for IDE ops
#تعریف_کن AHCI_RESET_TIMEOUT     500 // 500 miliseconds
#تعریف_کن AHCI_LINK_TIMEOUT       10 // 10 miliseconds

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

    إذا (op->شمار >= (1<<8) || lba + op->شمار >= (1<<28)) {
        fis->sector_count2 = op->شمار >> 8;
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
    fis->sector_شمار = op->شمار;
    fis->lba_کم      = lba;
    fis->lba_mid      = lba >> 8;
    fis->lba_بالا     = lba >> 16;
    fis->device       = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;
}

ساكن فراغ sata_prep_atapi(بنية sata_cmd_fis *fis, u16 blocksize)
{
    memset_fl(fis, 0, حجم(*fis));
    fis->command  = ATA_CMD_PACKET;
    fis->feature  = 1; /* dma */
    fis->lba_mid  = blocksize;
    fis->lba_بالا = blocksize >> 8;
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

ساكن u32 ahci_پورت_مقصد_ctrl(u32 pnr, u32 پورت_reg)
{
    u32 ctrl_reg = 0x100;
    ctrl_reg += pnr * 0x80;
    ctrl_reg += پورت_reg;
    أرجع ctrl_reg;
}

ساكن u32 ahci_پورت_readl(بنية ahci_ctrl_s *ctrl, u32 pnr, u32 reg)
{
    u32 ctrl_reg = ahci_پورت_مقصد_ctrl(pnr, reg);
    أرجع ahci_ctrl_readl(ctrl, ctrl_reg);
}

ساكن فراغ ahci_پورت_writel(بنية ahci_ctrl_s *ctrl, u32 pnr, u32 reg, u32 val)
{
    u32 ctrl_reg = ahci_پورت_مقصد_ctrl(pnr, reg);
    ahci_ctrl_writel(ctrl, ctrl_reg, val);
}

// submit ahci command + wait for result
ساكن صحيح ahci_command(بنية ahci_پورت_s *پورت_gf, صحيح iswrite, صحيح isatapi,
                        فراغ *میانگیر, u32 bsize)
{
    u32 val, وضعیت, success, flags, intbits, error;
    بنية ahci_ctrl_s *ctrl = پورت_gf->ctrl;
    بنية ahci_cmd_s  *cmd  = پورت_gf->cmd;
    بنية ahci_fis_s  *fis  = پورت_gf->fis;
    بنية ahci_list_s *list = پورت_gf->list;
    u32 pnr                  = پورت_gf->pnr;

    cmd->fis.reg       = 0x27;
    cmd->fis.pmp_type  = 1 << 7; /* cmd fis */
    cmd->prdt[0].پایه  = (u32)میانگیر;
    cmd->prdt[0].baseu = 0;
    cmd->prdt[0].flags = bsize-1;

    flags = ((1 << 16) | /* one prd entry */
             (iswrite ? (1 << 6) : 0) |
             (isatapi ? (1 << 5) : 0) |
             (5 << 0)); /* fis length (dwords) */
    list[0].flags  = flags;
    list[0].بایتها  = 0;
    list[0].پایه   = (u32)(cmd);
    list[0].baseu  = 0;

    dprintf(8, "AHCI/%d: send cmd ...\n", pnr);
    intbits = ahci_پورت_readl(ctrl, pnr, پورت_IRQ_STAT);
    إذا (intbits)
        ahci_پورت_writel(ctrl, pnr, پورت_IRQ_STAT, intbits);
    ahci_پورت_writel(ctrl, pnr, پورت_CMD_ISSUE, 1);

    u32 انتها = timer_calc(AHCI_REQUEST_TIMEOUT);
    افعل {
        لكل (;;) {
            intbits = ahci_پورت_readl(ctrl, pnr, پورت_IRQ_STAT);
            إذا (intbits) {
                ahci_پورت_writel(ctrl, pnr, پورت_IRQ_STAT, intbits);
                إذا (intbits & 0x40000000) {
                    u32 tf = ahci_پورت_readl(ctrl, pnr, پورت_TFDATA);
                    وضعیت = tf & 0xff;
                    error = (tf & 0xff00) >> 8;
                    اكسر;
                }
                إذا (intbits & 0x02) {
                    وضعیت = GET_LOWFLAT(fis->psfis[2]);
                    error  = GET_LOWFLAT(fis->psfis[3]);
                    اكسر;
                }
                إذا (intbits & 0x01) {
                    وضعیت = GET_LOWFLAT(fis->rfis[2]);
                    error  = GET_LOWFLAT(fis->rfis[3]);
                    اكسر;
                }
            }
            إذا (timer_check(انتها)) {
                warn_timeout();
                أرجع -1;
            }
            yield();
        }
        dprintf(8, "AHCI/%d: ... intbits 0x%x, status 0x%x ...\n",
                pnr, intbits, وضعیت);
    } طالما (وضعیت & ATA_CB_STAT_BSY);

    success = (0x00 == (وضعیت & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF |
                                  ATA_CB_STAT_ERR)) &&
               ATA_CB_STAT_RDY == (وضعیت & (ATA_CB_STAT_RDY)));
    إذا (success) {
        dprintf(8, "AHCI/%d: ... finished, status 0x%x, OK\n", pnr,
                وضعیت);
    } وإلا {
        dprintf(2, "AHCI/%d: ... finished, status 0x%x, ERROR 0x%x\n", pnr,
                وضعیت, error);

        // non-queued error recovery (AHCI 1.3 section 6.2.2.1)
        // Clears PxCMD.ST to 0 to reset the PxCI register
        val = ahci_پورت_readl(ctrl, pnr, پورت_CMD);
        ahci_پورت_writel(ctrl, pnr, پورت_CMD, val & ~پورت_CMD_شروع);

        // waits for PxCMD.CR to clear to 0
        طالما (1) {
            val = ahci_پورت_readl(ctrl, pnr, پورت_CMD);
            إذا ((val & پورت_CMD_LIST_ON) == 0)
                اكسر;
            yield();
        }

        // Clears any error bits in PxSERR to enable capturing new errors
        val = ahci_پورت_readl(ctrl, pnr, پورت_SCR_ERR);
        ahci_پورت_writel(ctrl, pnr, پورت_SCR_ERR, val);

        // Clears status bits in PxIS as appropriate
        val = ahci_پورت_readl(ctrl, pnr, پورت_IRQ_STAT);
        ahci_پورت_writel(ctrl, pnr, پورت_IRQ_STAT, val);

        // If PxTFD.STS.BSY or PxTFD.STS.DRQ is set to 1, issue
        // a COMRESET to the device to put it in an idle state
        val = ahci_پورت_readl(ctrl, pnr, پورت_TFDATA);
        إذا (val & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ)) {
            dprintf(2, "AHCI/%d: issue comreset\n", pnr);
            val = ahci_پورت_readl(ctrl, pnr, پورت_SCR_CTL);
            // set Device Detection Initialization (DET) to 1 for 1 ms for comreset
            ahci_پورت_writel(ctrl, pnr, پورت_SCR_CTL, val | 1);
            mdelay (1);
            ahci_پورت_writel(ctrl, pnr, پورت_SCR_CTL, val);
        }

        // Sets PxCMD.ST to 1 to enable issuing new commands
        val = ahci_پورت_readl(ctrl, pnr, پورت_CMD);
        ahci_پورت_writel(ctrl, pnr, پورت_CMD, val | پورت_CMD_شروع);
    }
    أرجع success ? 0 : -1;
}

#تعریف_کن CDROM_CDB_اندازه 12

صحيح ahci_atapi_process_op(بنية disk_op_s *op)
{
    إذا (! CONFIG_AHCI)
        أرجع 0;

    بنية ahci_پورت_s *پورت_gf = container_از(
        op->drive_fl, بنية ahci_پورت_s, drive);
    بنية ahci_cmd_s *cmd = پورت_gf->cmd;

    إذا (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        أرجع DISK_RET_EWRITEPROTECT;
    صحيح blocksize = scsi_fill_cmd(op, cmd->atapi, CDROM_CDB_اندازه);
    إذا (blocksize < 0)
        أرجع default_process_op(op);
    sata_prep_atapi(&cmd->fis, blocksize);
    صحيح rc = ahci_command(پورت_gf, 0, 1, op->buf_fl, op->شمار * blocksize);
    إذا (rc < 0)
        أرجع DISK_RET_EBADTRACK;
    أرجع DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive, op->buf_fl must be word aligned
ساكن صحيح
ahci_disk_readwrite_aligned(بنية disk_op_s *op, صحيح iswrite)
{
    بنية ahci_پورت_s *پورت_gf = container_از(
        op->drive_fl, بنية ahci_پورت_s, drive);
    بنية ahci_cmd_s *cmd = پورت_gf->cmd;
    صحيح rc;

    sata_prep_readwrite(&cmd->fis, op, iswrite);
    rc = ahci_command(پورت_gf, iswrite, 0, op->buf_fl,
                      op->شمار * DISK_SECTOR_اندازه);
    dprintf(8, "ahci disk %s, lba %6x, count %3x, buf %p, rc %d\n",
            iswrite ? "write" : "read", (u32)op->lba, op->شمار, op->buf_fl, rc);
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
    localop.شمار = 1;

    إذا (iswrite) {
        u16 بلوک;
        لكل (بلوک = 0; بلوک < op->شمار; بلوک++) {
            memcpy_fl (alignedbuf_fl, position, DISK_SECTOR_اندازه);
            rc = ahci_disk_readwrite_aligned (&localop, 1);
            إذا (rc)
                أرجع rc;
            position += DISK_SECTOR_اندازه;
            localop.lba++;
        }
    } وإلا { // read
        u16 بلوک;
        لكل (بلوک = 0; بلوک < op->شمار; بلوک++) {
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
ahci_پورت_reset(بنية ahci_ctrl_s *ctrl, u32 pnr)
{
    u32 val;

    /* disable FIS + CMD */
    u32 انتها = timer_calc(AHCI_RESET_TIMEOUT);
    لكل (;;) {
        val = ahci_پورت_readl(ctrl, pnr, پورت_CMD);
        إذا (!(val & (پورت_CMD_FIS_RX | پورت_CMD_شروع |
                     پورت_CMD_FIS_ON | پورت_CMD_LIST_ON)))
            اكسر;
        val &= ~(پورت_CMD_FIS_RX | پورت_CMD_شروع);
        ahci_پورت_writel(ctrl, pnr, پورت_CMD, val);
        إذا (timer_check(انتها)) {
            warn_timeout();
            اكسر;
        }
        yield();
    }

    /* disable + clear IRQs */
    ahci_پورت_writel(ctrl, pnr, پورت_IRQ_ماسک, 0);
    val = ahci_پورت_readl(ctrl, pnr, پورت_IRQ_STAT);
    إذا (val)
        ahci_پورت_writel(ctrl, pnr, پورت_IRQ_STAT, val);
}

ساكن بنية ahci_پورت_s*
ahci_پورت_alloc(بنية ahci_ctrl_s *ctrl, u32 pnr)
{
    بنية ahci_پورت_s *پورت = malloc_tmp(حجم(*پورت));

    إذا (!پورت) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(پورت, 0, حجم(*پورت));
    پورت->pnr = pnr;
    پورت->ctrl = ctrl;
    پورت->list = memalign_tmp(1024, 1024);
    پورت->fis = memalign_tmp(256, 256);
    پورت->cmd = memalign_tmp(256, 256);
    إذا (پورت->list == عدم || پورت->fis == عدم || پورت->cmd == عدم) {
        warn_noalloc();
        أرجع عدم;
    }
    memset(پورت->list, 0, 1024);
    memset(پورت->fis, 0, 256);
    memset(پورت->cmd, 0, 256);

    ahci_پورت_writel(ctrl, pnr, پورت_LST_ADDR, (u32)پورت->list);
    ahci_پورت_writel(ctrl, pnr, پورت_FIS_ADDR, (u32)پورت->fis);
    إذا (ctrl->caps & HOST_CAP_64) {
        ahci_پورت_writel(ctrl, pnr, پورت_LST_ADDR_HI, 0);
        ahci_پورت_writel(ctrl, pnr, پورت_FIS_ADDR_HI, 0);
    }

    أرجع پورت;
}

ساكن فراغ ahci_پورت_release(بنية ahci_پورت_s *پورت)
{
    ahci_پورت_reset(پورت->ctrl, پورت->pnr);
    حرر(پورت->list);
    حرر(پورت->fis);
    حرر(پورت->cmd);
    حرر(پورت);
}

ساكن بنية ahci_پورت_s* ahci_پورت_realloc(بنية ahci_پورت_s *پورت)
{
    بنية ahci_پورت_s *tmp;
    u32 cmd;

    tmp = malloc_fseg(حجم(*پورت));
    إذا (!tmp) {
        warn_noalloc();
        ahci_پورت_release(پورت);
        أرجع عدم;
    }
    *tmp = *پورت;
    حرر(پورت);
    پورت = tmp;

    ahci_پورت_reset(پورت->ctrl, پورت->pnr);

    حرر(پورت->list);
    حرر(پورت->fis);
    حرر(پورت->cmd);
    پورت->list = memalign_بالا(1024, 1024);
    پورت->fis = memalign_بالا(256, 256);
    پورت->cmd = memalign_بالا(256, 256);
    إذا (!پورت->list || !پورت->fis || !پورت->cmd) {
        warn_noalloc();
        حرر(پورت->list);
        حرر(پورت->fis);
        حرر(پورت->cmd);
        حرر(پورت);
        أرجع عدم;
    }

    ahci_پورت_writel(پورت->ctrl, پورت->pnr, پورت_LST_ADDR, (u32)پورت->list);
    ahci_پورت_writel(پورت->ctrl, پورت->pnr, پورت_FIS_ADDR, (u32)پورت->fis);

    cmd = ahci_پورت_readl(پورت->ctrl, پورت->pnr, پورت_CMD);
    cmd |= (پورت_CMD_FIS_RX|پورت_CMD_شروع);
    ahci_پورت_writel(پورت->ctrl, پورت->pnr, پورت_CMD, cmd);

    أرجع پورت;
}

#تعریف_کن MAXMODEL 40

/* See ahci spec chapter 10.1 "Software Initialization of HBA" */
ساكن صحيح ahci_پورت_راهاندازی(بنية ahci_پورت_s *پورت)
{
    بنية ahci_ctrl_s *ctrl = پورت->ctrl;
    u32 pnr = پورت->pnr;
    حرف model[MAXMODEL+1];
    u16 میانگیر[256];
    u32 cmd, stat, err, tf;
    صحيح rc;

    /* enable FIS recv */
    cmd = ahci_پورت_readl(ctrl, pnr, پورت_CMD);
    cmd |= پورت_CMD_FIS_RX;
    ahci_پورت_writel(ctrl, pnr, پورت_CMD, cmd);

    /* spin up */
    cmd &= ~پورت_CMD_ICC_ماسک;
    cmd |= پورت_CMD_SPIN_به_بالا | پورت_CMD_POWER_ON | پورت_CMD_ICC_فعال;
    ahci_پورت_writel(ctrl, pnr, پورت_CMD, cmd);
    u32 انتها = timer_calc(AHCI_LINK_TIMEOUT);
    لكل (;;) {
        stat = ahci_پورت_readl(ctrl, pnr, پورت_SCR_STAT);
        إذا ((stat & 0x07) == 0x03) {
            dprintf(2, "AHCI/%d: link up\n", پورت->pnr);
            اكسر;
        }
        إذا (timer_check(انتها)) {
            dprintf(2, "AHCI/%d: link down\n", پورت->pnr);
            أرجع -1;
        }
        yield();
    }

    /* clear error status */
    err = ahci_پورت_readl(ctrl, pnr, پورت_SCR_ERR);
    إذا (err)
        ahci_پورت_writel(ctrl, pnr, پورت_SCR_ERR, err);

    /* wait for device becoming ready */
    انتها = timer_calc(AHCI_REQUEST_TIMEOUT);
    لكل (;;) {
        tf = ahci_پورت_readl(ctrl, pnr, پورت_TFDATA);
        إذا (!(tf & (ATA_CB_STAT_BSY |
                    ATA_CB_STAT_DRQ)))
            اكسر;
        إذا (timer_check(انتها)) {
            warn_timeout();
            dprintf(1, "AHCI/%d: device not ready (tf 0x%x)\n", پورت->pnr, tf);
            أرجع -1;
        }
        yield();
    }

    /* start device */
    cmd |= پورت_CMD_شروع;
    ahci_پورت_writel(ctrl, pnr, پورت_CMD, cmd);

    sata_prep_simple(&پورت->cmd->fis, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    rc = ahci_command(پورت, 0, 0, میانگیر, حجم(میانگیر));
    إذا (rc == 0) {
        پورت->atapi = 1;
    } وإلا {
        پورت->atapi = 0;
        sata_prep_simple(&پورت->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
        rc = ahci_command(پورت, 0, 0, میانگیر, حجم(میانگیر));
        إذا (rc < 0)
            أرجع -1;
    }

    پورت->drive.cntl_id = pnr;
    پورت->drive.removable = (میانگیر[0] & 0x80) ? 1 : 0;

    إذا (!پورت->atapi) {
        // found disk (ata)
        پورت->drive.type = DTYPE_AHCI;
        پورت->drive.blksize = DISK_SECTOR_اندازه;
        پورت->drive.pchs.cylinder = میانگیر[1];
        پورت->drive.pchs.سر = میانگیر[3];
        پورت->drive.pchs.sector = میانگیر[6];

        u64 sectors;
        إذا (میانگیر[83] & (1 << 10)) // word 83 - lba48 support
            sectors = *(u64*)&میانگیر[100]; // word 100-103
        وإلا
            sectors = *(u32*)&میانگیر[60]; // word 60 and word 61
        پورت->drive.sectors = sectors;
        u64 adjsize = sectors >> 11;
        حرف adjprefix = 'M';
        إذا (adjsize >= (1 << 16)) {
            adjsize >>= 10;
            adjprefix = 'G';
        }
        پورت->desc = znprintf(MAXDESCSIZE
                              , "AHCI/%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                              , پورت->pnr
                              , ata_استخراج_model(model, MAXMODEL, میانگیر)
                              , ata_استخراج_نسخه(میانگیر)
                              , (u32)adjsize, adjprefix);
        پورت->prio = bootprio_پیدا_کن_ata_device(ctrl->pci_tmp, pnr, 0);

        s8 multi_dma = -1;
        s8 pio_نما = -1;
        s8 udma_نما = -1;
        // If bit 2 in word 53 is set, udma information is valid in word 88.
        إذا (میانگیر[53] & 0x04) {
            udma_نما = 6;
            طالما ((udma_نما >= 0) &&
                   !((میانگیر[88] & 0x7f) & ( 1 << udma_نما ))) {
                udma_نما--;
            }
        }
        // If bit 1 in word 53 is set, multiword-dma and advanced pio modes
        // are available in words 63 and 64.
        إذا (میانگیر[53] & 0x02) {
            pio_نما = 4;
            multi_dma = 3;
            طالما ((multi_dma >= 0) &&
                   !((میانگیر[63] & 0x7) & ( 1 << multi_dma ))) {
                multi_dma--;
            }
            طالما ((pio_نما >= 3) &&
                   !((میانگیر[64] & 0x3) & ( 1 << ( pio_نما - 3 ) ))) {
                pio_نما--;
            }
        }
        dprintf(2, "AHCI/%d: supported modes: udma %d, multi-dma %d, pio %d\n",
                پورت->pnr, udma_نما, multi_dma, pio_نما);

        sata_prep_simple(&پورت->cmd->fis, ATA_CMD_مجموعه_FEATURES);
        پورت->cmd->fis.feature = ATA_مجموعه_FEATRUE_TRANSFER_نما;
        // Select used mode. UDMA first, then Multi-DMA followed by
        // advanced PIO modes 3 or 4. If non, set default PIO.
        إذا (udma_نما >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to UDMA-%d\n",
                    پورت->pnr, udma_نما);
            پورت->cmd->fis.sector_شمار = ATA_TRANSFER_نما_ULTRA_DMA
                                          | udma_نما;
        } وإلا إذا (multi_dma >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to Multi-DMA-%d\n",
                    پورت->pnr, multi_dma);
            پورت->cmd->fis.sector_شمار = ATA_TRANSFER_نما_MULTIWORD_DMA
                                          | multi_dma;
        } وإلا إذا (pio_نما >= 3) {
            dprintf(1, "AHCI/%d: Set transfer mode to PIO-%d\n",
                    پورت->pnr, pio_نما);
            پورت->cmd->fis.sector_شمار = ATA_TRANSFER_نما_PIO_FLOW_CTRL
                                          | pio_نما;
        } وإلا {
            dprintf(1, "AHCI/%d: Set transfer mode to default PIO\n",
                    پورت->pnr);
            پورت->cmd->fis.sector_شمار = ATA_TRANSFER_نما_DEFAULT_PIO;
        }
        rc = ahci_command(پورت, 1, 0, 0, 0);
        إذا (rc < 0) {
            dprintf(1, "AHCI/%d: Set transfer mode failed.\n", پورت->pnr);
        }
    } وإلا {
        // found cdrom (atapi)
        پورت->drive.type = DTYPE_AHCI_ATAPI;
        پورت->drive.blksize = CDROM_SECTOR_اندازه;
        پورت->drive.sectors = (u64)-1;
        u8 iscd = ((میانگیر[0] >> 8) & 0x1f) == 0x05;
        إذا (!iscd) {
            dprintf(1, "AHCI/%d: atapi device isn't a cdrom\n", پورت->pnr);
            أرجع -1;
        }
        پورت->desc = znprintf(MAXDESCSIZE
                              , "DVD/CD [AHCI/%d: %s ATAPI-%d DVD/CD]"
                              , پورت->pnr
                              , ata_استخراج_model(model, MAXMODEL, میانگیر)
                              , ata_استخراج_نسخه(میانگیر));
        پورت->prio = bootprio_پیدا_کن_ata_device(ctrl->pci_tmp, pnr, 0);
    }
    boot_lchs_پیدا_کن_ata_device(ctrl->pci_tmp, pnr, 0, &(پورت->drive.lchs));
    أرجع 0;
}

// Detect any drives attached to a given controller.
ساكن فراغ
ahci_پورت_detect(فراغ *داده)
{
    بنية ahci_پورت_s *پورت = داده;
    صحيح rc;

    dprintf(2, "AHCI/%d: probing\n", پورت->pnr);
    ahci_پورت_reset(پورت->ctrl, پورت->pnr);
    rc = ahci_پورت_راهاندازی(پورت);
    إذا (rc < 0)
        ahci_پورت_release(پورت);
    وإلا {
        پورت = ahci_پورت_realloc(پورت);
        إذا (پورت == عدم)
            أرجع;
        dprintf(1, "AHCI/%d: registering: \"%s\"\n", پورت->pnr, پورت->desc);
        إذا (!پورت->atapi) {
            // Register with bcv system.
            boot_جمع_کن_hd(&پورت->drive, پورت->desc, پورت->prio);
        } وإلا {
            // fill cdidmap
            boot_جمع_کن_cd(&پورت->drive, پورت->desc, پورت->prio);
        }
    }
}

// Initialize an ata controller and detect its drives.
ساكن فراغ
ahci_controller_راهاندازی(بنية pci_device *pci)
{
    بنية ahci_پورت_s *پورت;
    u32 pnr, max;

    إذا (create_bounce_buf() < 0)
        أرجع;

    فراغ *iobase = pci_enable_membar(pci, PCI_پایه_آدرس_5);
    إذا (!iobase)
        أرجع;

    بنية ahci_ctrl_s *ctrl = malloc_fseg(حجم(*ctrl));
    إذا (!ctrl) {
        warn_noalloc();
        أرجع;
    }

    ctrl->pci_tmp = pci;
    ctrl->iobase = iobase;
    ctrl->irq = pci_config_readb(pci->bdf, PCI_وقفه_سطر);
    dprintf(1, "AHCI controller at %pP, iobase %p, irq %d\n"
            , pci, ctrl->iobase, ctrl->irq);

    pci_enable_busmaster(pci);

    u32 val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_RESET);
    u32 انتها = timer_calc(AHCI_RESET_TIMEOUT);
    لكل (;;) {
        val = ahci_ctrl_readl(ctrl, HOST_CTL);
        إذا (!(val & HOST_CTL_RESET))
            اكسر;
        إذا (timer_check(انتها)) {
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
        پورت = ahci_پورت_alloc(ctrl, pnr);
        إذا (پورت == عدم)
            استمر;
        پیاپی_thread(ahci_پورت_detect, پورت);
    }
}

// Locate and init ahci controllers.
ساكن فراغ
ahci_پیمایش(فراغ)
{
    // Scan PCI bus for ATA adapters
    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci->class != PCI_CLASS_STORAGE_SATA)
            استمر;
        إذا (pci->prog_if != 1 /* AHCI rev 1 */)
            استمر;
        ahci_controller_راهاندازی(pci);
    }
}

فراغ
ahci_راهاندازی(فراغ)
{
    ASSERT32FLAT();
    إذا (!CONFIG_AHCI)
        أرجع;

    dprintf(3, "init ahci\n");
    ahci_پیمایش();
}

// Low level AHCI disk access
//
// Copyright (C) 2010 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "ahci.h" // CDB_CMD_READ_10
#ಅನ್ತರ್ಭಾವ "ata.h" // ATA_CB_STAT
#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBAL
#ಅನ್ತರ್ಭಾವ "blockcmd.h" // CDB_CMD_READ_10
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pci.h" // pci_config_readb
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_INTERRUPT_LINE
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_SUCCESS
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // timer_calc
#ಅನ್ತರ್ಭಾವ "x86.h" // inb

#ವ್ಯಾಖ್ಯಾನಿಸು AHCI_REQUEST_TIMEOUT 32000 // 32 seconds max for IDE ops
#ವ್ಯಾಖ್ಯಾನಿಸು AHCI_RESET_TIMEOUT     500 // 500 miliseconds
#ವ್ಯಾಖ್ಯಾನಿಸು AHCI_LINK_TIMEOUT       10 // 10 miliseconds

// prepare sata command fis
ಸ್ಥಿರ ಶೂನ್ಯ sata_prep_simple(ರಚನೆ sata_cmd_fis *fis, u8 command)
{
    memset_fl(fis, 0, ಗಾತ್ರ(*fis));
    fis->command = command;
}

ಸ್ಥಿರ ಶೂನ್ಯ sata_prep_readwrite(ರಚನೆ sata_cmd_fis *fis,
                                ರಚನೆ disk_op_s *op, ಪೂರ್ಣಾಂಕ iswrite)
{
    u64 lba = op->lba;
    u8 command;

    memset_fl(fis, 0, ಗಾತ್ರ(*fis));

    ಆದರೆ (op->ಎಣಿಕೆ_2 >= (1<<8) || lba + op->ಎಣಿಕೆ_2 >= (1<<28)) {
        fis->sector_count2 = op->ಎಣಿಕೆ_2 >> 8;
        fis->lba_low2      = lba >> 24;
        fis->lba_mid2      = lba >> 32;
        fis->lba_high2     = lba >> 40;
        lba &= 0xffffff;
        command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                   : ATA_CMD_READ_DMA_EXT);
    } ಇಲ್ಲದಿದ್ದರೆ {
        command = (iswrite ? ATA_CMD_WRITE_DMA
                   : ATA_CMD_READ_DMA);
    }
    fis->feature      = 1; /* dma */
    fis->command      = command;
    fis->sector_ಎಣಿಕೆ = op->ಎಣಿಕೆ_2;
    fis->lba_ಕೆಳಗಿನ      = lba;
    fis->lba_mid      = lba >> 8;
    fis->lba_ಮೇಲಿನ     = lba >> 16;
    fis->device       = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;
}

ಸ್ಥಿರ ಶೂನ್ಯ sata_prep_atapi(ರಚನೆ sata_cmd_fis *fis, u16 blocksize)
{
    memset_fl(fis, 0, ಗಾತ್ರ(*fis));
    fis->command  = ATA_CMD_PACKET;
    fis->feature  = 1; /* dma */
    fis->lba_mid  = blocksize;
    fis->lba_ಮೇಲಿನ = blocksize >> 8;
}

// ahci register access helpers
ಸ್ಥಿರ u32 ahci_ctrl_readl(ರಚನೆ ahci_ctrl_s *ctrl, u32 reg)
{
    ಹಿಂತಿರುಗಿ readl(ctrl->iobase + reg);
}

ಸ್ಥಿರ ಶೂನ್ಯ ahci_ctrl_writel(ರಚನೆ ahci_ctrl_s *ctrl, u32 reg, u32 val)
{
    writel(ctrl->iobase + reg, val);
}

ಸ್ಥಿರ u32 ahci_ಪೋರ್ಟ್_ಗಮ್ಯ_ctrl(u32 pnr, u32 ಪೋರ್ಟ್_reg)
{
    u32 ctrl_reg = 0x100;
    ctrl_reg += pnr * 0x80;
    ctrl_reg += ಪೋರ್ಟ್_reg;
    ಹಿಂತಿರುಗಿ ctrl_reg;
}

ಸ್ಥಿರ u32 ahci_ಪೋರ್ಟ್_readl(ರಚನೆ ahci_ctrl_s *ctrl, u32 pnr, u32 reg)
{
    u32 ctrl_reg = ahci_ಪೋರ್ಟ್_ಗಮ್ಯ_ctrl(pnr, reg);
    ಹಿಂತಿರುಗಿ ahci_ctrl_readl(ctrl, ctrl_reg);
}

ಸ್ಥಿರ ಶೂನ್ಯ ahci_ಪೋರ್ಟ್_writel(ರಚನೆ ahci_ctrl_s *ctrl, u32 pnr, u32 reg, u32 val)
{
    u32 ctrl_reg = ahci_ಪೋರ್ಟ್_ಗಮ್ಯ_ctrl(pnr, reg);
    ahci_ctrl_writel(ctrl, ctrl_reg, val);
}

// submit ahci command + wait for result
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ahci_command(ರಚನೆ ahci_ಪೋರ್ಟ್_s *ಪೋರ್ಟ್_gf, ಪೂರ್ಣಾಂಕ iswrite, ಪೂರ್ಣಾಂಕ isatapi,
                        ಶೂನ್ಯ *ಬಫರ್, u32 bsize)
{
    u32 val, ಸ್ಥಿತಿ_2, success, flags, intbits, error;
    ರಚನೆ ahci_ctrl_s *ctrl = ಪೋರ್ಟ್_gf->ctrl;
    ರಚನೆ ahci_cmd_s  *cmd  = ಪೋರ್ಟ್_gf->cmd;
    ರಚನೆ ahci_fis_s  *fis  = ಪೋರ್ಟ್_gf->fis;
    ರಚನೆ ahci_list_s *list = ಪೋರ್ಟ್_gf->list;
    u32 pnr                  = ಪೋರ್ಟ್_gf->pnr;

    cmd->fis.reg       = 0x27;
    cmd->fis.pmp_type  = 1 << 7; /* cmd fis */
    cmd->prdt[0].ತಳ  = (u32)ಬಫರ್;
    cmd->prdt[0].baseu = 0;
    cmd->prdt[0].flags = bsize-1;

    flags = ((1 << 16) | /* one prd entry */
             (iswrite ? (1 << 6) : 0) |
             (isatapi ? (1 << 5) : 0) |
             (5 << 0)); /* fis length (dwords) */
    list[0].flags  = flags;
    list[0].ಬೈಟುಗಳು  = 0;
    list[0].ತಳ   = (u32)(cmd);
    list[0].baseu  = 0;

    dprintf(8, "AHCI/%d: send cmd ...\n", pnr);
    intbits = ahci_ಪೋರ್ಟ್_readl(ctrl, pnr, ಪೋರ್ಟ್_IRQ_STAT);
    ಆದರೆ (intbits)
        ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_IRQ_STAT, intbits);
    ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_CMD_ISSUE, 1);

    u32 ಕೊನೆ = timer_calc(AHCI_REQUEST_TIMEOUT);
    ಮಾಡು {
        ಗಾಗಿ (;;) {
            intbits = ahci_ಪೋರ್ಟ್_readl(ctrl, pnr, ಪೋರ್ಟ್_IRQ_STAT);
            ಆದರೆ (intbits) {
                ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_IRQ_STAT, intbits);
                ಆದರೆ (intbits & 0x40000000) {
                    u32 tf = ahci_ಪೋರ್ಟ್_readl(ctrl, pnr, ಪೋರ್ಟ್_TFDATA);
                    ಸ್ಥಿತಿ_2 = tf & 0xff;
                    error = (tf & 0xff00) >> 8;
                    ನಿಲ್ಲಿಸು;
                }
                ಆದರೆ (intbits & 0x02) {
                    ಸ್ಥಿತಿ_2 = GET_LOWFLAT(fis->psfis[2]);
                    error  = GET_LOWFLAT(fis->psfis[3]);
                    ನಿಲ್ಲಿಸು;
                }
                ಆದರೆ (intbits & 0x01) {
                    ಸ್ಥಿತಿ_2 = GET_LOWFLAT(fis->rfis[2]);
                    error  = GET_LOWFLAT(fis->rfis[3]);
                    ನಿಲ್ಲಿಸು;
                }
            }
            ಆದರೆ (timer_check(ಕೊನೆ)) {
                warn_timeout();
                ಹಿಂತಿರುಗಿ -1;
            }
            yield();
        }
        dprintf(8, "AHCI/%d: ... intbits 0x%x, status 0x%x ...\n",
                pnr, intbits, ಸ್ಥಿತಿ_2);
    } ಆಗ (ಸ್ಥಿತಿ_2 & ATA_CB_STAT_BSY);

    success = (0x00 == (ಸ್ಥಿತಿ_2 & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF |
                                  ATA_CB_STAT_ERR)) &&
               ATA_CB_STAT_RDY == (ಸ್ಥಿತಿ_2 & (ATA_CB_STAT_RDY)));
    ಆದರೆ (success) {
        dprintf(8, "AHCI/%d: ... finished, status 0x%x, OK\n", pnr,
                ಸ್ಥಿತಿ_2);
    } ಇಲ್ಲದಿದ್ದರೆ {
        dprintf(2, "AHCI/%d: ... finished, status 0x%x, ERROR 0x%x\n", pnr,
                ಸ್ಥಿತಿ_2, error);

        // non-queued error recovery (AHCI 1.3 section 6.2.2.1)
        // Clears PxCMD.ST to 0 to reset the PxCI register
        val = ahci_ಪೋರ್ಟ್_readl(ctrl, pnr, ಪೋರ್ಟ್_CMD);
        ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_CMD, val & ~ಪೋರ್ಟ್_CMD_ಆರಂಭ);

        // waits for PxCMD.CR to clear to 0
        ಆಗ (1) {
            val = ahci_ಪೋರ್ಟ್_readl(ctrl, pnr, ಪೋರ್ಟ್_CMD);
            ಆದರೆ ((val & ಪೋರ್ಟ್_CMD_LIST_ON) == 0)
                ನಿಲ್ಲಿಸು;
            yield();
        }

        // Clears any error bits in PxSERR to enable capturing new errors
        val = ahci_ಪೋರ್ಟ್_readl(ctrl, pnr, ಪೋರ್ಟ್_SCR_ERR);
        ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_SCR_ERR, val);

        // Clears status bits in PxIS as appropriate
        val = ahci_ಪೋರ್ಟ್_readl(ctrl, pnr, ಪೋರ್ಟ್_IRQ_STAT);
        ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_IRQ_STAT, val);

        // If PxTFD.STS.BSY or PxTFD.STS.DRQ is set to 1, issue
        // a COMRESET to the device to put it in an idle state
        val = ahci_ಪೋರ್ಟ್_readl(ctrl, pnr, ಪೋರ್ಟ್_TFDATA);
        ಆದರೆ (val & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ)) {
            dprintf(2, "AHCI/%d: issue comreset\n", pnr);
            val = ahci_ಪೋರ್ಟ್_readl(ctrl, pnr, ಪೋರ್ಟ್_SCR_CTL);
            // set Device Detection Initialization (DET) to 1 for 1 ms for comreset
            ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_SCR_CTL, val | 1);
            mdelay (1);
            ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_SCR_CTL, val);
        }

        // Sets PxCMD.ST to 1 to enable issuing new commands
        val = ahci_ಪೋರ್ಟ್_readl(ctrl, pnr, ಪೋರ್ಟ್_CMD);
        ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_CMD, val | ಪೋರ್ಟ್_CMD_ಆರಂಭ);
    }
    ಹಿಂತಿರುಗಿ success ? 0 : -1;
}

#ವ್ಯಾಖ್ಯಾನಿಸು CDROM_CDB_ಗಾತ್ರ 12

ಪೂರ್ಣಾಂಕ ahci_atapi_process_op(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (! CONFIG_AHCI)
        ಹಿಂತಿರುಗಿ 0;

    ರಚನೆ ahci_ಪೋರ್ಟ್_s *ಪೋರ್ಟ್_gf = container_ದ(
        op->drive_fl, ರಚನೆ ahci_ಪೋರ್ಟ್_s, drive);
    ರಚನೆ ahci_cmd_s *cmd = ಪೋರ್ಟ್_gf->cmd;

    ಆದರೆ (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        ಹಿಂತಿರುಗಿ DISK_RET_EWRITEPROTECT;
    ಪೂರ್ಣಾಂಕ blocksize = scsi_fill_cmd(op, cmd->atapi, CDROM_CDB_ಗಾತ್ರ);
    ಆದರೆ (blocksize < 0)
        ಹಿಂತಿರುಗಿ default_process_op(op);
    sata_prep_atapi(&cmd->fis, blocksize);
    ಪೂರ್ಣಾಂಕ rc = ahci_command(ಪೋರ್ಟ್_gf, 0, 1, op->buf_fl, op->ಎಣಿಕೆ_2 * blocksize);
    ಆದರೆ (rc < 0)
        ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive, op->buf_fl must be word aligned
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ahci_disk_readwrite_aligned(ರಚನೆ disk_op_s *op, ಪೂರ್ಣಾಂಕ iswrite)
{
    ರಚನೆ ahci_ಪೋರ್ಟ್_s *ಪೋರ್ಟ್_gf = container_ದ(
        op->drive_fl, ರಚನೆ ahci_ಪೋರ್ಟ್_s, drive);
    ರಚನೆ ahci_cmd_s *cmd = ಪೋರ್ಟ್_gf->cmd;
    ಪೂರ್ಣಾಂಕ rc;

    sata_prep_readwrite(&cmd->fis, op, iswrite);
    rc = ahci_command(ಪೋರ್ಟ್_gf, iswrite, 0, op->buf_fl,
                      op->ಎಣಿಕೆ_2 * DISK_SECTOR_ಗಾತ್ರ);
    dprintf(8, "ahci disk %s, lba %6x, count %3x, buf %p, rc %d\n",
            iswrite ? "write" : "read", (u32)op->lba, op->ಎಣಿಕೆ_2, op->buf_fl, rc);
    ಆದರೆ (rc < 0)
        ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ahci_disk_readwrite(ರಚನೆ disk_op_s *op, ಪೂರ್ಣಾಂಕ iswrite)
{
    // if caller's buffer is word aligned, use it directly
    ಆದರೆ (((u32) op->buf_fl & 1) == 0)
        ಹಿಂತಿರುಗಿ ahci_disk_readwrite_aligned(op, iswrite);

    // Use a word aligned buffer for AHCI I/O
    ಪೂರ್ಣಾಂಕ rc;
    ರಚನೆ disk_op_s localop = *op;
    u8 *alignedbuf_fl = bounce_buf_fl;
    u8 *position = op->buf_fl;

    localop.buf_fl = alignedbuf_fl;
    localop.ಎಣಿಕೆ_2 = 1;

    ಆದರೆ (iswrite) {
        u16 ಖಂಡ;
        ಗಾಗಿ (ಖಂಡ = 0; ಖಂಡ < op->ಎಣಿಕೆ_2; ಖಂಡ++) {
            memcpy_fl (alignedbuf_fl, position, DISK_SECTOR_ಗಾತ್ರ);
            rc = ahci_disk_readwrite_aligned (&localop, 1);
            ಆದರೆ (rc)
                ಹಿಂತಿರುಗಿ rc;
            position += DISK_SECTOR_ಗಾತ್ರ;
            localop.lba++;
        }
    } ಇಲ್ಲದಿದ್ದರೆ { // read
        u16 ಖಂಡ;
        ಗಾಗಿ (ಖಂಡ = 0; ಖಂಡ < op->ಎಣಿಕೆ_2; ಖಂಡ++) {
            rc = ahci_disk_readwrite_aligned (&localop, 0);
            ಆದರೆ (rc)
                ಹಿಂತಿರುಗಿ rc;
            memcpy_fl (position, alignedbuf_fl, DISK_SECTOR_ಗಾತ್ರ);
            position += DISK_SECTOR_ಗಾತ್ರ;
            localop.lba++;
        }
    }
    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}

// command demuxer
ಪೂರ್ಣಾಂಕ
ahci_process_op(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (!CONFIG_AHCI)
        ಹಿಂತಿರುಗಿ 0;
    ಆಯ್ಕೆ (op->command) {
    ಸ್ಥಿತಿ CMD_READ:
        ಹಿಂತಿರುಗಿ ahci_disk_readwrite(op, 0);
    ಸ್ಥಿತಿ CMD_WRITE:
        ಹಿಂತಿರುಗಿ ahci_disk_readwrite(op, 1);
    ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ default_process_op(op);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
ahci_ಪೋರ್ಟ್_reset(ರಚನೆ ahci_ctrl_s *ctrl, u32 pnr)
{
    u32 val;

    /* disable FIS + CMD */
    u32 ಕೊನೆ = timer_calc(AHCI_RESET_TIMEOUT);
    ಗಾಗಿ (;;) {
        val = ahci_ಪೋರ್ಟ್_readl(ctrl, pnr, ಪೋರ್ಟ್_CMD);
        ಆದರೆ (!(val & (ಪೋರ್ಟ್_CMD_FIS_RX | ಪೋರ್ಟ್_CMD_ಆರಂಭ |
                     ಪೋರ್ಟ್_CMD_FIS_ON | ಪೋರ್ಟ್_CMD_LIST_ON)))
            ನಿಲ್ಲಿಸು;
        val &= ~(ಪೋರ್ಟ್_CMD_FIS_RX | ಪೋರ್ಟ್_CMD_ಆರಂಭ);
        ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_CMD, val);
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            ನಿಲ್ಲಿಸು;
        }
        yield();
    }

    /* disable + clear IRQs */
    ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_IRQ_ಮುಖವಾಡ, 0);
    val = ahci_ಪೋರ್ಟ್_readl(ctrl, pnr, ಪೋರ್ಟ್_IRQ_STAT);
    ಆದರೆ (val)
        ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_IRQ_STAT, val);
}

ಸ್ಥಿರ ರಚನೆ ahci_ಪೋರ್ಟ್_s*
ahci_ಪೋರ್ಟ್_alloc(ರಚನೆ ahci_ctrl_s *ctrl, u32 pnr)
{
    ರಚನೆ ahci_ಪೋರ್ಟ್_s *ಪೋರ್ಟ್ = malloc_tmp(ಗಾತ್ರ(*ಪೋರ್ಟ್));

    ಆದರೆ (!ಪೋರ್ಟ್) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    memset(ಪೋರ್ಟ್, 0, ಗಾತ್ರ(*ಪೋರ್ಟ್));
    ಪೋರ್ಟ್->pnr = pnr;
    ಪೋರ್ಟ್->ctrl = ctrl;
    ಪೋರ್ಟ್->list = memalign_tmp(1024, 1024);
    ಪೋರ್ಟ್->fis = memalign_tmp(256, 256);
    ಪೋರ್ಟ್->cmd = memalign_tmp(256, 256);
    ಆದರೆ (ಪೋರ್ಟ್->list == ಖಾಲಿ || ಪೋರ್ಟ್->fis == ಖಾಲಿ || ಪೋರ್ಟ್->cmd == ಖಾಲಿ) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    memset(ಪೋರ್ಟ್->list, 0, 1024);
    memset(ಪೋರ್ಟ್->fis, 0, 256);
    memset(ಪೋರ್ಟ್->cmd, 0, 256);

    ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_LST_ADDR, (u32)ಪೋರ್ಟ್->list);
    ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_FIS_ADDR, (u32)ಪೋರ್ಟ್->fis);
    ಆದರೆ (ctrl->caps & HOST_CAP_64) {
        ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_LST_ADDR_HI, 0);
        ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_FIS_ADDR_HI, 0);
    }

    ಹಿಂತಿರುಗಿ ಪೋರ್ಟ್;
}

ಸ್ಥಿರ ಶೂನ್ಯ ahci_ಪೋರ್ಟ್_release(ರಚನೆ ahci_ಪೋರ್ಟ್_s *ಪೋರ್ಟ್)
{
    ahci_ಪೋರ್ಟ್_reset(ಪೋರ್ಟ್->ctrl, ಪೋರ್ಟ್->pnr);
    ಬಿಡುಗಡೆ(ಪೋರ್ಟ್->list);
    ಬಿಡುಗಡೆ(ಪೋರ್ಟ್->fis);
    ಬಿಡುಗಡೆ(ಪೋರ್ಟ್->cmd);
    ಬಿಡುಗಡೆ(ಪೋರ್ಟ್);
}

ಸ್ಥಿರ ರಚನೆ ahci_ಪೋರ್ಟ್_s* ahci_ಪೋರ್ಟ್_realloc(ರಚನೆ ahci_ಪೋರ್ಟ್_s *ಪೋರ್ಟ್)
{
    ರಚನೆ ahci_ಪೋರ್ಟ್_s *tmp;
    u32 cmd;

    tmp = malloc_fseg(ಗಾತ್ರ(*ಪೋರ್ಟ್));
    ಆದರೆ (!tmp) {
        warn_noalloc();
        ahci_ಪೋರ್ಟ್_release(ಪೋರ್ಟ್);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    *tmp = *ಪೋರ್ಟ್;
    ಬಿಡುಗಡೆ(ಪೋರ್ಟ್);
    ಪೋರ್ಟ್ = tmp;

    ahci_ಪೋರ್ಟ್_reset(ಪೋರ್ಟ್->ctrl, ಪೋರ್ಟ್->pnr);

    ಬಿಡುಗಡೆ(ಪೋರ್ಟ್->list);
    ಬಿಡುಗಡೆ(ಪೋರ್ಟ್->fis);
    ಬಿಡುಗಡೆ(ಪೋರ್ಟ್->cmd);
    ಪೋರ್ಟ್->list = memalign_ಮೇಲಿನ(1024, 1024);
    ಪೋರ್ಟ್->fis = memalign_ಮೇಲಿನ(256, 256);
    ಪೋರ್ಟ್->cmd = memalign_ಮೇಲಿನ(256, 256);
    ಆದರೆ (!ಪೋರ್ಟ್->list || !ಪೋರ್ಟ್->fis || !ಪೋರ್ಟ್->cmd) {
        warn_noalloc();
        ಬಿಡುಗಡೆ(ಪೋರ್ಟ್->list);
        ಬಿಡುಗಡೆ(ಪೋರ್ಟ್->fis);
        ಬಿಡುಗಡೆ(ಪೋರ್ಟ್->cmd);
        ಬಿಡುಗಡೆ(ಪೋರ್ಟ್);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }

    ahci_ಪೋರ್ಟ್_writel(ಪೋರ್ಟ್->ctrl, ಪೋರ್ಟ್->pnr, ಪೋರ್ಟ್_LST_ADDR, (u32)ಪೋರ್ಟ್->list);
    ahci_ಪೋರ್ಟ್_writel(ಪೋರ್ಟ್->ctrl, ಪೋರ್ಟ್->pnr, ಪೋರ್ಟ್_FIS_ADDR, (u32)ಪೋರ್ಟ್->fis);

    cmd = ahci_ಪೋರ್ಟ್_readl(ಪೋರ್ಟ್->ctrl, ಪೋರ್ಟ್->pnr, ಪೋರ್ಟ್_CMD);
    cmd |= (ಪೋರ್ಟ್_CMD_FIS_RX|ಪೋರ್ಟ್_CMD_ಆರಂಭ);
    ahci_ಪೋರ್ಟ್_writel(ಪೋರ್ಟ್->ctrl, ಪೋರ್ಟ್->pnr, ಪೋರ್ಟ್_CMD, cmd);

    ಹಿಂತಿರುಗಿ ಪೋರ್ಟ್;
}

#ವ್ಯಾಖ್ಯಾನಿಸು MAXMODEL 40

/* See ahci spec chapter 10.1 "Software Initialization of HBA" */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ahci_ಪೋರ್ಟ್_ಸಿದ್ಧತೆ(ರಚನೆ ahci_ಪೋರ್ಟ್_s *ಪೋರ್ಟ್)
{
    ರಚನೆ ahci_ctrl_s *ctrl = ಪೋರ್ಟ್->ctrl;
    u32 pnr = ಪೋರ್ಟ್->pnr;
    ಅಕ್ಷರ model[MAXMODEL+1];
    u16 ಬಫರ್[256];
    u32 cmd, stat, err, tf;
    ಪೂರ್ಣಾಂಕ rc;

    /* enable FIS recv */
    cmd = ahci_ಪೋರ್ಟ್_readl(ctrl, pnr, ಪೋರ್ಟ್_CMD);
    cmd |= ಪೋರ್ಟ್_CMD_FIS_RX;
    ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_CMD, cmd);

    /* spin up */
    cmd &= ~ಪೋರ್ಟ್_CMD_ICC_ಮುಖವಾಡ;
    cmd |= ಪೋರ್ಟ್_CMD_SPIN_ಮೇಲೆ | ಪೋರ್ಟ್_CMD_POWER_ON | ಪೋರ್ಟ್_CMD_ICC_ಸಕ್ರಿಯ;
    ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_CMD, cmd);
    u32 ಕೊನೆ = timer_calc(AHCI_LINK_TIMEOUT);
    ಗಾಗಿ (;;) {
        stat = ahci_ಪೋರ್ಟ್_readl(ctrl, pnr, ಪೋರ್ಟ್_SCR_STAT);
        ಆದರೆ ((stat & 0x07) == 0x03) {
            dprintf(2, "AHCI/%d: link up\n", ಪೋರ್ಟ್->pnr);
            ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            dprintf(2, "AHCI/%d: link down\n", ಪೋರ್ಟ್->pnr);
            ಹಿಂತಿರುಗಿ -1;
        }
        yield();
    }

    /* clear error status */
    err = ahci_ಪೋರ್ಟ್_readl(ctrl, pnr, ಪೋರ್ಟ್_SCR_ERR);
    ಆದರೆ (err)
        ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_SCR_ERR, err);

    /* wait for device becoming ready */
    ಕೊನೆ = timer_calc(AHCI_REQUEST_TIMEOUT);
    ಗಾಗಿ (;;) {
        tf = ahci_ಪೋರ್ಟ್_readl(ctrl, pnr, ಪೋರ್ಟ್_TFDATA);
        ಆದರೆ (!(tf & (ATA_CB_STAT_BSY |
                    ATA_CB_STAT_DRQ)))
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            dprintf(1, "AHCI/%d: device not ready (tf 0x%x)\n", ಪೋರ್ಟ್->pnr, tf);
            ಹಿಂತಿರುಗಿ -1;
        }
        yield();
    }

    /* start device */
    cmd |= ಪೋರ್ಟ್_CMD_ಆರಂಭ;
    ahci_ಪೋರ್ಟ್_writel(ctrl, pnr, ಪೋರ್ಟ್_CMD, cmd);

    sata_prep_simple(&ಪೋರ್ಟ್->cmd->fis, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    rc = ahci_command(ಪೋರ್ಟ್, 0, 0, ಬಫರ್, ಗಾತ್ರ(ಬಫರ್));
    ಆದರೆ (rc == 0) {
        ಪೋರ್ಟ್->atapi = 1;
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಪೋರ್ಟ್->atapi = 0;
        sata_prep_simple(&ಪೋರ್ಟ್->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
        rc = ahci_command(ಪೋರ್ಟ್, 0, 0, ಬಫರ್, ಗಾತ್ರ(ಬಫರ್));
        ಆದರೆ (rc < 0)
            ಹಿಂತಿರುಗಿ -1;
    }

    ಪೋರ್ಟ್->drive.cntl_id = pnr;
    ಪೋರ್ಟ್->drive.removable = (ಬಫರ್[0] & 0x80) ? 1 : 0;

    ಆದರೆ (!ಪೋರ್ಟ್->atapi) {
        // found disk (ata)
        ಪೋರ್ಟ್->drive.type = DTYPE_AHCI;
        ಪೋರ್ಟ್->drive.blksize = DISK_SECTOR_ಗಾತ್ರ;
        ಪೋರ್ಟ್->drive.pchs.cylinder = ಬಫರ್[1];
        ಪೋರ್ಟ್->drive.pchs.ತಲೆ = ಬಫರ್[3];
        ಪೋರ್ಟ್->drive.pchs.sector = ಬಫರ್[6];

        u64 sectors;
        ಆದರೆ (ಬಫರ್[83] & (1 << 10)) // word 83 - lba48 support
            sectors = *(u64*)&ಬಫರ್[100]; // word 100-103
        ಇಲ್ಲದಿದ್ದರೆ
            sectors = *(u32*)&ಬಫರ್[60]; // word 60 and word 61
        ಪೋರ್ಟ್->drive.sectors = sectors;
        u64 adjsize = sectors >> 11;
        ಅಕ್ಷರ adjprefix = 'M';
        ಆದರೆ (adjsize >= (1 << 16)) {
            adjsize >>= 10;
            adjprefix = 'G';
        }
        ಪೋರ್ಟ್->desc = znprintf(MAXDESCSIZE
                              , "AHCI/%d: %s ATA-%d Hard-Disk (%u %ciBytes)"
                              , ಪೋರ್ಟ್->pnr
                              , ata_ಹೊರತೆಗೆ_model(model, MAXMODEL, ಬಫರ್)
                              , ata_ಹೊರತೆಗೆ_ಆವೃತ್ತಿ(ಬಫರ್)
                              , (u32)adjsize, adjprefix);
        ಪೋರ್ಟ್->prio = bootprio_ಕಂಡುಹಿಡಿ_ata_device(ctrl->pci_tmp, pnr, 0);

        s8 multi_dma = -1;
        s8 pio_ಬಹುಲಕ = -1;
        s8 udma_ಬಹುಲಕ = -1;
        // If bit 2 in word 53 is set, udma information is valid in word 88.
        ಆದರೆ (ಬಫರ್[53] & 0x04) {
            udma_ಬಹುಲಕ = 6;
            ಆಗ ((udma_ಬಹುಲಕ >= 0) &&
                   !((ಬಫರ್[88] & 0x7f) & ( 1 << udma_ಬಹುಲಕ ))) {
                udma_ಬಹುಲಕ--;
            }
        }
        // If bit 1 in word 53 is set, multiword-dma and advanced pio modes
        // are available in words 63 and 64.
        ಆದರೆ (ಬಫರ್[53] & 0x02) {
            pio_ಬಹುಲಕ = 4;
            multi_dma = 3;
            ಆಗ ((multi_dma >= 0) &&
                   !((ಬಫರ್[63] & 0x7) & ( 1 << multi_dma ))) {
                multi_dma--;
            }
            ಆಗ ((pio_ಬಹುಲಕ >= 3) &&
                   !((ಬಫರ್[64] & 0x3) & ( 1 << ( pio_ಬಹುಲಕ - 3 ) ))) {
                pio_ಬಹುಲಕ--;
            }
        }
        dprintf(2, "AHCI/%d: supported modes: udma %d, multi-dma %d, pio %d\n",
                ಪೋರ್ಟ್->pnr, udma_ಬಹುಲಕ, multi_dma, pio_ಬಹುಲಕ);

        sata_prep_simple(&ಪೋರ್ಟ್->cmd->fis, ATA_CMD_ಗಣ_FEATURES);
        ಪೋರ್ಟ್->cmd->fis.feature = ATA_ಗಣ_FEATRUE_TRANSFER_ಬಹುಲಕ;
        // Select used mode. UDMA first, then Multi-DMA followed by
        // advanced PIO modes 3 or 4. If non, set default PIO.
        ಆದರೆ (udma_ಬಹುಲಕ >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to UDMA-%d\n",
                    ಪೋರ್ಟ್->pnr, udma_ಬಹುಲಕ);
            ಪೋರ್ಟ್->cmd->fis.sector_ಎಣಿಕೆ = ATA_TRANSFER_ಬಹುಲಕ_ULTRA_DMA
                                          | udma_ಬಹುಲಕ;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (multi_dma >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to Multi-DMA-%d\n",
                    ಪೋರ್ಟ್->pnr, multi_dma);
            ಪೋರ್ಟ್->cmd->fis.sector_ಎಣಿಕೆ = ATA_TRANSFER_ಬಹುಲಕ_MULTIWORD_DMA
                                          | multi_dma;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pio_ಬಹುಲಕ >= 3) {
            dprintf(1, "AHCI/%d: Set transfer mode to PIO-%d\n",
                    ಪೋರ್ಟ್->pnr, pio_ಬಹುಲಕ);
            ಪೋರ್ಟ್->cmd->fis.sector_ಎಣಿಕೆ = ATA_TRANSFER_ಬಹುಲಕ_PIO_FLOW_CTRL
                                          | pio_ಬಹುಲಕ;
        } ಇಲ್ಲದಿದ್ದರೆ {
            dprintf(1, "AHCI/%d: Set transfer mode to default PIO\n",
                    ಪೋರ್ಟ್->pnr);
            ಪೋರ್ಟ್->cmd->fis.sector_ಎಣಿಕೆ = ATA_TRANSFER_ಬಹುಲಕ_DEFAULT_PIO;
        }
        rc = ahci_command(ಪೋರ್ಟ್, 1, 0, 0, 0);
        ಆದರೆ (rc < 0) {
            dprintf(1, "AHCI/%d: Set transfer mode failed.\n", ಪೋರ್ಟ್->pnr);
        }
    } ಇಲ್ಲದಿದ್ದರೆ {
        // found cdrom (atapi)
        ಪೋರ್ಟ್->drive.type = DTYPE_AHCI_ATAPI;
        ಪೋರ್ಟ್->drive.blksize = CDROM_SECTOR_ಗಾತ್ರ;
        ಪೋರ್ಟ್->drive.sectors = (u64)-1;
        u8 iscd = ((ಬಫರ್[0] >> 8) & 0x1f) == 0x05;
        ಆದರೆ (!iscd) {
            dprintf(1, "AHCI/%d: atapi device isn't a cdrom\n", ಪೋರ್ಟ್->pnr);
            ಹಿಂತಿರುಗಿ -1;
        }
        ಪೋರ್ಟ್->desc = znprintf(MAXDESCSIZE
                              , "DVD/CD [AHCI/%d: %s ATAPI-%d DVD/CD]"
                              , ಪೋರ್ಟ್->pnr
                              , ata_ಹೊರತೆಗೆ_model(model, MAXMODEL, ಬಫರ್)
                              , ata_ಹೊರತೆಗೆ_ಆವೃತ್ತಿ(ಬಫರ್));
        ಪೋರ್ಟ್->prio = bootprio_ಕಂಡುಹಿಡಿ_ata_device(ctrl->pci_tmp, pnr, 0);
    }
    boot_lchs_ಕಂಡುಹಿಡಿ_ata_device(ctrl->pci_tmp, pnr, 0, &(ಪೋರ್ಟ್->drive.lchs));
    ಹಿಂತಿರುಗಿ 0;
}

// Detect any drives attached to a given controller.
ಸ್ಥಿರ ಶೂನ್ಯ
ahci_ಪೋರ್ಟ್_detect(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ರಚನೆ ahci_ಪೋರ್ಟ್_s *ಪೋರ್ಟ್ = ದತ್ತಾಂಶ;
    ಪೂರ್ಣಾಂಕ rc;

    dprintf(2, "AHCI/%d: probing\n", ಪೋರ್ಟ್->pnr);
    ahci_ಪೋರ್ಟ್_reset(ಪೋರ್ಟ್->ctrl, ಪೋರ್ಟ್->pnr);
    rc = ahci_ಪೋರ್ಟ್_ಸಿದ್ಧತೆ(ಪೋರ್ಟ್);
    ಆದರೆ (rc < 0)
        ahci_ಪೋರ್ಟ್_release(ಪೋರ್ಟ್);
    ಇಲ್ಲದಿದ್ದರೆ {
        ಪೋರ್ಟ್ = ahci_ಪೋರ್ಟ್_realloc(ಪೋರ್ಟ್);
        ಆದರೆ (ಪೋರ್ಟ್ == ಖಾಲಿ)
            ಹಿಂತಿರುಗಿ;
        dprintf(1, "AHCI/%d: registering: \"%s\"\n", ಪೋರ್ಟ್->pnr, ಪೋರ್ಟ್->desc);
        ಆದರೆ (!ಪೋರ್ಟ್->atapi) {
            // Register with bcv system.
            boot_ಕೂಡಿಸು_hd(&ಪೋರ್ಟ್->drive, ಪೋರ್ಟ್->desc, ಪೋರ್ಟ್->prio);
        } ಇಲ್ಲದಿದ್ದರೆ {
            // fill cdidmap
            boot_ಕೂಡಿಸು_cd(&ಪೋರ್ಟ್->drive, ಪೋರ್ಟ್->desc, ಪೋರ್ಟ್->prio);
        }
    }
}

// Initialize an ata controller and detect its drives.
ಸ್ಥಿರ ಶೂನ್ಯ
ahci_controller_ಸಿದ್ಧತೆ(ರಚನೆ pci_device *pci)
{
    ರಚನೆ ahci_ಪೋರ್ಟ್_s *ಪೋರ್ಟ್;
    u32 pnr, max;

    ಆದರೆ (create_bounce_buf() < 0)
        ಹಿಂತಿರುಗಿ;

    ಶೂನ್ಯ *iobase = pci_enable_membar(pci, PCI_ತಳ_ವಿಳಾಸ_5);
    ಆದರೆ (!iobase)
        ಹಿಂತಿರುಗಿ;

    ರಚನೆ ahci_ctrl_s *ctrl = malloc_fseg(ಗಾತ್ರ(*ctrl));
    ಆದರೆ (!ctrl) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }

    ctrl->pci_tmp = pci;
    ctrl->iobase = iobase;
    ctrl->irq = pci_config_readb(pci->bdf, PCI_ಅಡಚಣೆ_ಸಾಲು);
    dprintf(1, "AHCI controller at %pP, iobase %p, irq %d\n"
            , pci, ctrl->iobase, ctrl->irq);

    pci_enable_busmaster(pci);

    u32 val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_RESET);
    u32 ಕೊನೆ = timer_calc(AHCI_RESET_TIMEOUT);
    ಗಾಗಿ (;;) {
        val = ahci_ctrl_readl(ctrl, HOST_CTL);
        ಆದರೆ (!(val & HOST_CTL_RESET))
            ನಿಲ್ಲಿಸು;
        ಆದರೆ (timer_check(ಕೊನೆ)) {
            warn_timeout();
            dprintf(1, "AHCI: controller reset failed\n");
            ಬಿಡುಗಡೆ(ctrl);
            ಹಿಂತಿರುಗಿ;
        }
        yield();
    }
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_AHCI_EN);
    (ಶೂನ್ಯ)ahci_ctrl_readl(ctrl, HOST_CTL); /* Flush */

    ctrl->caps = ahci_ctrl_readl(ctrl, HOST_CAP);
    ctrl->ports = ahci_ctrl_readl(ctrl, HOST_PORTS_IMPL);
    dprintf(2, "AHCI: cap 0x%x, ports_impl 0x%x\n",
            ctrl->caps, ctrl->ports);

    max = 0x1f;
    ಗಾಗಿ (pnr = 0; pnr <= max; pnr++) {
        ಆದರೆ (!(ctrl->ports & (1 << pnr)))
            ಮುಂದುವರಿ;
        ಪೋರ್ಟ್ = ahci_ಪೋರ್ಟ್_alloc(ctrl, pnr);
        ಆದರೆ (ಪೋರ್ಟ್ == ಖಾಲಿ)
            ಮುಂದುವರಿ;
        ಸತತ_thread(ahci_ಪೋರ್ಟ್_detect, ಪೋರ್ಟ್);
    }
}

// Locate and init ahci controllers.
ಸ್ಥಿರ ಶೂನ್ಯ
ahci_ಪರಿಶೀಲನೆ(ಶೂನ್ಯ)
{
    // Scan PCI bus for ATA adapters
    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (pci->class != PCI_CLASS_STORAGE_SATA)
            ಮುಂದುವರಿ;
        ಆದರೆ (pci->prog_if != 1 /* AHCI rev 1 */)
            ಮುಂದುವರಿ;
        ahci_controller_ಸಿದ್ಧತೆ(pci);
    }
}

ಶೂನ್ಯ
ahci_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಆದರೆ (!CONFIG_AHCI)
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "init ahci\n");
    ahci_ಪರಿಶೀಲನೆ();
}

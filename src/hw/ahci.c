// Low level AHCI disk access
//
// Copyright (C) 2010 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "ahci.h" // CDB_CMD_READ_10
#ଅନ୍ତର୍ଭାଵ "ata.h" // ATA_CB_STAT
#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_GLOBAL
#ଅନ୍ତର୍ଭାଵ "blockcmd.h" // CDB_CMD_READ_10
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "pci.h" // pci_config_readb
#ଅନ୍ତର୍ଭାଵ "pcidevice.h" // foreachpci
#ଅନ୍ତର୍ଭାଵ "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#ଅନ୍ତର୍ଭାଵ "pci_regs.h" // PCI_INTERRUPT_LINE
#ଅନ୍ତର୍ଭାଵ "stacks.h" // yield
#ଅନ୍ତର୍ଭାଵ "std/disk.h" // DISK_RET_SUCCESS
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // timer_calc
#ଅନ୍ତର୍ଭାଵ "x86.h" // inb

#define AHCI_REQUEST_TIMEOUT 32000 // 32 seconds max for IDE ops
#define AHCI_RESET_TIMEOUT     500 // 500 miliseconds
#define AHCI_LINK_TIMEOUT       10 // 10 miliseconds

// prepare sata command fis
ସ୍ଥିର ଶୂନ୍ୟ sata_prep_simple(ସଂରଚନା sata_cmd_fis *fis, u8 command)
{
    memset_fl(fis, 0, ଆକାର(*fis));
    fis->command = command;
}

ସ୍ଥିର ଶୂନ୍ୟ sata_prep_readwrite(ସଂରଚନା sata_cmd_fis *fis,
                                ସଂରଚନା disk_op_s *op, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iswrite)
{
    u64 lba = op->lba;
    u8 command;

    memset_fl(fis, 0, ଆକାର(*fis));

    ଯଦି (op->ଗଣନା_2 >= (1<<8) || lba + op->ଗଣନା_2 >= (1<<28)) {
        fis->sector_count2 = op->ଗଣନା_2 >> 8;
        fis->lba_low2      = lba >> 24;
        fis->lba_mid2      = lba >> 32;
        fis->lba_high2     = lba >> 40;
        lba &= 0xffffff;
        command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                   : ATA_CMD_READ_DMA_EXT);
    } ଅନ୍ୟଥା {
        command = (iswrite ? ATA_CMD_WRITE_DMA
                   : ATA_CMD_READ_DMA);
    }
    fis->feature      = 1; /* dma */
    fis->command      = command;
    fis->sector_ଗଣନା = op->ଗଣନା_2;
    fis->lba_ତଳ      = lba;
    fis->lba_mid      = lba >> 8;
    fis->lba_ଉପର     = lba >> 16;
    fis->device       = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;
}

ସ୍ଥିର ଶୂନ୍ୟ sata_prep_atapi(ସଂରଚନା sata_cmd_fis *fis, u16 blocksize)
{
    memset_fl(fis, 0, ଆକାର(*fis));
    fis->command  = ATA_CMD_PACKET;
    fis->feature  = 1; /* dma */
    fis->lba_mid  = blocksize;
    fis->lba_ଉପର = blocksize >> 8;
}

// ahci register access helpers
ସ୍ଥିର u32 ahci_ctrl_readl(ସଂରଚନା ahci_ctrl_s *ctrl, u32 reg)
{
    ଫେରନ୍ତୁ readl(ctrl->iobase + reg);
}

ସ୍ଥିର ଶୂନ୍ୟ ahci_ctrl_writel(ସଂରଚନା ahci_ctrl_s *ctrl, u32 reg, u32 val)
{
    writel(ctrl->iobase + reg, val);
}

ସ୍ଥିର u32 ahci_port_to_ctrl(u32 pnr, u32 port_reg)
{
    u32 ctrl_reg = 0x100;
    ctrl_reg += pnr * 0x80;
    ctrl_reg += port_reg;
    ଫେରନ୍ତୁ ctrl_reg;
}

ସ୍ଥିର u32 ahci_port_readl(ସଂରଚନା ahci_ctrl_s *ctrl, u32 pnr, u32 reg)
{
    u32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    ଫେରନ୍ତୁ ahci_ctrl_readl(ctrl, ctrl_reg);
}

ସ୍ଥିର ଶୂନ୍ୟ ahci_port_writel(ସଂରଚନା ahci_ctrl_s *ctrl, u32 pnr, u32 reg, u32 val)
{
    u32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    ahci_ctrl_writel(ctrl, ctrl_reg, val);
}

// submit ahci command + wait for result
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ahci_command(ସଂରଚନା ahci_port_s *port_gf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iswrite, ପୂର୍ଣ୍ଣସଂଖ୍ୟା isatapi,
                        ଶୂନ୍ୟ *buffer, u32 bsize)
{
    u32 val, status, success, flags, intbits, error;
    ସଂରଚନା ahci_ctrl_s *ctrl = port_gf->ctrl;
    ସଂରଚନା ahci_cmd_s  *cmd  = port_gf->cmd;
    ସଂରଚନା ahci_fis_s  *fis  = port_gf->fis;
    ସଂରଚନା ahci_list_s *list = port_gf->list;
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
    ଯଦି (intbits)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
    ahci_port_writel(ctrl, pnr, PORT_CMD_ISSUE, 1);

    u32 end = timer_calc(AHCI_REQUEST_TIMEOUT);
    କର {
        ପାଇଁ (;;) {
            intbits = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
            ଯଦି (intbits) {
                ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
                ଯଦି (intbits & 0x40000000) {
                    u32 tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
                    status = tf & 0xff;
                    error = (tf & 0xff00) >> 8;
                    ଭାଙ୍ଗ;
                }
                ଯଦି (intbits & 0x02) {
                    status = GET_LOWFLAT(fis->psfis[2]);
                    error  = GET_LOWFLAT(fis->psfis[3]);
                    ଭାଙ୍ଗ;
                }
                ଯଦି (intbits & 0x01) {
                    status = GET_LOWFLAT(fis->rfis[2]);
                    error  = GET_LOWFLAT(fis->rfis[3]);
                    ଭାଙ୍ଗ;
                }
            }
            ଯଦି (timer_check(end)) {
                warn_timeout();
                ଫେରନ୍ତୁ -1;
            }
            yield();
        }
        dprintf(8, "AHCI/%d: ... intbits 0x%x, status 0x%x ...\n",
                pnr, intbits, status);
    } ଯେତେବେଳେ (status & ATA_CB_STAT_BSY);

    success = (0x00 == (status & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF |
                                  ATA_CB_STAT_ERR)) &&
               ATA_CB_STAT_RDY == (status & (ATA_CB_STAT_RDY)));
    ଯଦି (success) {
        dprintf(8, "AHCI/%d: ... finished, status 0x%x, OK\n", pnr,
                status);
    } ଅନ୍ୟଥା {
        dprintf(2, "AHCI/%d: ... finished, status 0x%x, ERROR 0x%x\n", pnr,
                status, error);

        // non-queued error recovery (AHCI 1.3 section 6.2.2.1)
        // Clears PxCMD.ST to 0 to reset the PxCI register
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val & ~PORT_CMD_START);

        // waits for PxCMD.CR to clear to 0
        ଯେତେବେଳେ (1) {
            val = ahci_port_readl(ctrl, pnr, PORT_CMD);
            ଯଦି ((val & PORT_CMD_LIST_ON) == 0)
                ଭାଙ୍ଗ;
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
        ଯଦି (val & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ)) {
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
    ଫେରନ୍ତୁ success ? 0 : -1;
}

#define CDROM_CDB_SIZE 12

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ahci_atapi_process_op(ସଂରଚନା disk_op_s *op)
{
    ଯଦି (! CONFIG_AHCI)
        ଫେରନ୍ତୁ 0;

    ସଂରଚନା ahci_port_s *port_gf = container_of(
        op->drive_fl, ସଂରଚନା ahci_port_s, drive);
    ସଂରଚନା ahci_cmd_s *cmd = port_gf->cmd;

    ଯଦି (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        ଫେରନ୍ତୁ DISK_RET_EWRITEPROTECT;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା blocksize = scsi_fill_cmd(op, cmd->atapi, CDROM_CDB_SIZE);
    ଯଦି (blocksize < 0)
        ଫେରନ୍ତୁ default_process_op(op);
    sata_prep_atapi(&cmd->fis, blocksize);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = ahci_command(port_gf, 0, 1, op->buf_fl, op->ଗଣନା_2 * blocksize);
    ଯଦି (rc < 0)
        ଫେରନ୍ତୁ DISK_RET_EBADTRACK;
    ଫେରନ୍ତୁ DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive, op->buf_fl must be word aligned
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ahci_disk_readwrite_aligned(ସଂରଚନା disk_op_s *op, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iswrite)
{
    ସଂରଚନା ahci_port_s *port_gf = container_of(
        op->drive_fl, ସଂରଚନା ahci_port_s, drive);
    ସଂରଚନା ahci_cmd_s *cmd = port_gf->cmd;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

    sata_prep_readwrite(&cmd->fis, op, iswrite);
    rc = ahci_command(port_gf, iswrite, 0, op->buf_fl,
                      op->ଗଣନା_2 * DISK_SECTOR_SIZE);
    dprintf(8, "ahci disk %s, lba %6x, count %3x, buf %p, rc %d\n",
            iswrite ? "write" : "read", (u32)op->lba, op->ଗଣନା_2, op->buf_fl, rc);
    ଯଦି (rc < 0)
        ଫେରନ୍ତୁ DISK_RET_EBADTRACK;
    ଫେରନ୍ତୁ DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive.
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ahci_disk_readwrite(ସଂରଚନା disk_op_s *op, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iswrite)
{
    // if caller's buffer is word aligned, use it directly
    ଯଦି (((u32) op->buf_fl & 1) == 0)
        ଫେରନ୍ତୁ ahci_disk_readwrite_aligned(op, iswrite);

    // Use a word aligned buffer for AHCI I/O
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
    ସଂରଚନା disk_op_s localop = *op;
    u8 *alignedbuf_fl = bounce_buf_fl;
    u8 *position = op->buf_fl;

    localop.buf_fl = alignedbuf_fl;
    localop.ଗଣନା_2 = 1;

    ଯଦି (iswrite) {
        u16 block;
        ପାଇଁ (block = 0; block < op->ଗଣନା_2; block++) {
            memcpy_fl (alignedbuf_fl, position, DISK_SECTOR_SIZE);
            rc = ahci_disk_readwrite_aligned (&localop, 1);
            ଯଦି (rc)
                ଫେରନ୍ତୁ rc;
            position += DISK_SECTOR_SIZE;
            localop.lba++;
        }
    } ଅନ୍ୟଥା { // read
        u16 block;
        ପାଇଁ (block = 0; block < op->ଗଣନା_2; block++) {
            rc = ahci_disk_readwrite_aligned (&localop, 0);
            ଯଦି (rc)
                ଫେରନ୍ତୁ rc;
            memcpy_fl (position, alignedbuf_fl, DISK_SECTOR_SIZE);
            position += DISK_SECTOR_SIZE;
            localop.lba++;
        }
    }
    ଫେରନ୍ତୁ DISK_RET_SUCCESS;
}

// command demuxer
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ahci_process_op(ସଂରଚନା disk_op_s *op)
{
    ଯଦି (!CONFIG_AHCI)
        ଫେରନ୍ତୁ 0;
    ବାଛ (op->command) {
    ସ୍ଥିତି CMD_READ:
        ଫେରନ୍ତୁ ahci_disk_readwrite(op, 0);
    ସ୍ଥିତି CMD_WRITE:
        ଫେରନ୍ତୁ ahci_disk_readwrite(op, 1);
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ଫେରନ୍ତୁ default_process_op(op);
    }
}

ସ୍ଥିର ଶୂନ୍ୟ
ahci_port_reset(ସଂରଚନା ahci_ctrl_s *ctrl, u32 pnr)
{
    u32 val;

    /* disable FIS + CMD */
    u32 end = timer_calc(AHCI_RESET_TIMEOUT);
    ପାଇଁ (;;) {
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        ଯଦି (!(val & (PORT_CMD_FIS_RX | PORT_CMD_START |
                     PORT_CMD_FIS_ON | PORT_CMD_LIST_ON)))
            ଭାଙ୍ଗ;
        val &= ~(PORT_CMD_FIS_RX | PORT_CMD_START);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val);
        ଯଦି (timer_check(end)) {
            warn_timeout();
            ଭାଙ୍ଗ;
        }
        yield();
    }

    /* disable + clear IRQs */
    ahci_port_writel(ctrl, pnr, PORT_IRQ_MASK, 0);
    val = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
    ଯଦି (val)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, val);
}

ସ୍ଥିର ସଂରଚନା ahci_port_s*
ahci_port_alloc(ସଂରଚନା ahci_ctrl_s *ctrl, u32 pnr)
{
    ସଂରଚନା ahci_port_s *port = malloc_tmp(ଆକାର(*port));

    ଯଦି (!port) {
        warn_noalloc();
        ଫେରନ୍ତୁ ଖାଲି;
    }
    memset(port, 0, ଆକାର(*port));
    port->pnr = pnr;
    port->ctrl = ctrl;
    port->list = memalign_tmp(1024, 1024);
    port->fis = memalign_tmp(256, 256);
    port->cmd = memalign_tmp(256, 256);
    ଯଦି (port->list == ଖାଲି || port->fis == ଖାଲି || port->cmd == ଖାଲି) {
        warn_noalloc();
        ଫେରନ୍ତୁ ଖାଲି;
    }
    memset(port->list, 0, 1024);
    memset(port->fis, 0, 256);
    memset(port->cmd, 0, 256);

    ahci_port_writel(ctrl, pnr, PORT_LST_ADDR, (u32)port->list);
    ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR, (u32)port->fis);
    ଯଦି (ctrl->caps & HOST_CAP_64) {
        ahci_port_writel(ctrl, pnr, PORT_LST_ADDR_HI, 0);
        ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR_HI, 0);
    }

    ଫେରନ୍ତୁ port;
}

ସ୍ଥିର ଶୂନ୍ୟ ahci_port_release(ସଂରଚନା ahci_port_s *port)
{
    ahci_port_reset(port->ctrl, port->pnr);
    ମୁକ୍ତ(port->list);
    ମୁକ୍ତ(port->fis);
    ମୁକ୍ତ(port->cmd);
    ମୁକ୍ତ(port);
}

ସ୍ଥିର ସଂରଚନା ahci_port_s* ahci_port_realloc(ସଂରଚନା ahci_port_s *port)
{
    ସଂରଚନା ahci_port_s *tmp;
    u32 cmd;

    tmp = malloc_fseg(ଆକାର(*port));
    ଯଦି (!tmp) {
        warn_noalloc();
        ahci_port_release(port);
        ଫେରନ୍ତୁ ଖାଲି;
    }
    *tmp = *port;
    ମୁକ୍ତ(port);
    port = tmp;

    ahci_port_reset(port->ctrl, port->pnr);

    ମୁକ୍ତ(port->list);
    ମୁକ୍ତ(port->fis);
    ମୁକ୍ତ(port->cmd);
    port->list = memalign_ଉପର(1024, 1024);
    port->fis = memalign_ଉପର(256, 256);
    port->cmd = memalign_ଉପର(256, 256);
    ଯଦି (!port->list || !port->fis || !port->cmd) {
        warn_noalloc();
        ମୁକ୍ତ(port->list);
        ମୁକ୍ତ(port->fis);
        ମୁକ୍ତ(port->cmd);
        ମୁକ୍ତ(port);
        ଫେରନ୍ତୁ ଖାଲି;
    }

    ahci_port_writel(port->ctrl, port->pnr, PORT_LST_ADDR, (u32)port->list);
    ahci_port_writel(port->ctrl, port->pnr, PORT_FIS_ADDR, (u32)port->fis);

    cmd = ahci_port_readl(port->ctrl, port->pnr, PORT_CMD);
    cmd |= (PORT_CMD_FIS_RX|PORT_CMD_START);
    ahci_port_writel(port->ctrl, port->pnr, PORT_CMD, cmd);

    ଫେରନ୍ତୁ port;
}

#define MAXMODEL 40

/* See ahci spec chapter 10.1 "Software Initialization of HBA" */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ahci_port_setup(ସଂରଚନା ahci_port_s *port)
{
    ସଂରଚନା ahci_ctrl_s *ctrl = port->ctrl;
    u32 pnr = port->pnr;
    ଅକ୍ଷର model[MAXMODEL+1];
    u16 buffer[256];
    u32 cmd, stat, err, tf;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

    /* enable FIS recv */
    cmd = ahci_port_readl(ctrl, pnr, PORT_CMD);
    cmd |= PORT_CMD_FIS_RX;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    /* spin up */
    cmd &= ~PORT_CMD_ICC_MASK;
    cmd |= PORT_CMD_SPIN_UP | PORT_CMD_POWER_ON | PORT_CMD_ICC_ACTIVE;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);
    u32 end = timer_calc(AHCI_LINK_TIMEOUT);
    ପାଇଁ (;;) {
        stat = ahci_port_readl(ctrl, pnr, PORT_SCR_STAT);
        ଯଦି ((stat & 0x07) == 0x03) {
            dprintf(2, "AHCI/%d: link up\n", port->pnr);
            ଭାଙ୍ଗ;
        }
        ଯଦି (timer_check(end)) {
            dprintf(2, "AHCI/%d: link down\n", port->pnr);
            ଫେରନ୍ତୁ -1;
        }
        yield();
    }

    /* clear error status */
    err = ahci_port_readl(ctrl, pnr, PORT_SCR_ERR);
    ଯଦି (err)
        ahci_port_writel(ctrl, pnr, PORT_SCR_ERR, err);

    /* wait for device becoming ready */
    end = timer_calc(AHCI_REQUEST_TIMEOUT);
    ପାଇଁ (;;) {
        tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
        ଯଦି (!(tf & (ATA_CB_STAT_BSY |
                    ATA_CB_STAT_DRQ)))
            ଭାଙ୍ଗ;
        ଯଦି (timer_check(end)) {
            warn_timeout();
            dprintf(1, "AHCI/%d: device not ready (tf 0x%x)\n", port->pnr, tf);
            ଫେରନ୍ତୁ -1;
        }
        yield();
    }

    /* start device */
    cmd |= PORT_CMD_START;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    rc = ahci_command(port, 0, 0, buffer, ଆକାର(buffer));
    ଯଦି (rc == 0) {
        port->atapi = 1;
    } ଅନ୍ୟଥା {
        port->atapi = 0;
        sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
        rc = ahci_command(port, 0, 0, buffer, ଆକାର(buffer));
        ଯଦି (rc < 0)
            ଫେରନ୍ତୁ -1;
    }

    port->drive.cntl_id = pnr;
    port->drive.removable = (buffer[0] & 0x80) ? 1 : 0;

    ଯଦି (!port->atapi) {
        // found disk (ata)
        port->drive.type = DTYPE_AHCI;
        port->drive.blksize = DISK_SECTOR_SIZE;
        port->drive.pchs.cylinder = buffer[1];
        port->drive.pchs.head = buffer[3];
        port->drive.pchs.sector = buffer[6];

        u64 sectors;
        ଯଦି (buffer[83] & (1 << 10)) // word 83 - lba48 support
            sectors = *(u64*)&buffer[100]; // word 100-103
        ଅନ୍ୟଥା
            sectors = *(u32*)&buffer[60]; // word 60 and word 61
        port->drive.sectors = sectors;
        u64 adjsize = sectors >> 11;
        ଅକ୍ଷର adjprefix = 'M';
        ଯଦି (adjsize >= (1 << 16)) {
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
        ଯଦି (buffer[53] & 0x04) {
            udma_mode = 6;
            ଯେତେବେଳେ ((udma_mode >= 0) &&
                   !((buffer[88] & 0x7f) & ( 1 << udma_mode ))) {
                udma_mode--;
            }
        }
        // If bit 1 in word 53 is set, multiword-dma and advanced pio modes
        // are available in words 63 and 64.
        ଯଦି (buffer[53] & 0x02) {
            pio_mode = 4;
            multi_dma = 3;
            ଯେତେବେଳେ ((multi_dma >= 0) &&
                   !((buffer[63] & 0x7) & ( 1 << multi_dma ))) {
                multi_dma--;
            }
            ଯେତେବେଳେ ((pio_mode >= 3) &&
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
        ଯଦି (udma_mode >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to UDMA-%d\n",
                    port->pnr, udma_mode);
            port->cmd->fis.sector_ଗଣନା = ATA_TRANSFER_MODE_ULTRA_DMA
                                          | udma_mode;
        } ଅନ୍ୟଥା ଯଦି (multi_dma >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to Multi-DMA-%d\n",
                    port->pnr, multi_dma);
            port->cmd->fis.sector_ଗଣନା = ATA_TRANSFER_MODE_MULTIWORD_DMA
                                          | multi_dma;
        } ଅନ୍ୟଥା ଯଦି (pio_mode >= 3) {
            dprintf(1, "AHCI/%d: Set transfer mode to PIO-%d\n",
                    port->pnr, pio_mode);
            port->cmd->fis.sector_ଗଣନା = ATA_TRANSFER_MODE_PIO_FLOW_CTRL
                                          | pio_mode;
        } ଅନ୍ୟଥା {
            dprintf(1, "AHCI/%d: Set transfer mode to default PIO\n",
                    port->pnr);
            port->cmd->fis.sector_ଗଣନା = ATA_TRANSFER_MODE_DEFAULT_PIO;
        }
        rc = ahci_command(port, 1, 0, 0, 0);
        ଯଦି (rc < 0) {
            dprintf(1, "AHCI/%d: Set transfer mode failed.\n", port->pnr);
        }
    } ଅନ୍ୟଥା {
        // found cdrom (atapi)
        port->drive.type = DTYPE_AHCI_ATAPI;
        port->drive.blksize = CDROM_SECTOR_SIZE;
        port->drive.sectors = (u64)-1;
        u8 iscd = ((buffer[0] >> 8) & 0x1f) == 0x05;
        ଯଦି (!iscd) {
            dprintf(1, "AHCI/%d: atapi device isn't a cdrom\n", port->pnr);
            ଫେରନ୍ତୁ -1;
        }
        port->desc = znprintf(MAXDESCSIZE
                              , "DVD/CD [AHCI/%d: %s ATAPI-%d DVD/CD]"
                              , port->pnr
                              , ata_extract_model(model, MAXMODEL, buffer)
                              , ata_extract_version(buffer));
        port->prio = bootprio_find_ata_device(ctrl->pci_tmp, pnr, 0);
    }
    boot_lchs_find_ata_device(ctrl->pci_tmp, pnr, 0, &(port->drive.lchs));
    ଫେରନ୍ତୁ 0;
}

// Detect any drives attached to a given controller.
ସ୍ଥିର ଶୂନ୍ୟ
ahci_port_detect(ଶୂନ୍ୟ *data)
{
    ସଂରଚନା ahci_port_s *port = data;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

    dprintf(2, "AHCI/%d: probing\n", port->pnr);
    ahci_port_reset(port->ctrl, port->pnr);
    rc = ahci_port_setup(port);
    ଯଦି (rc < 0)
        ahci_port_release(port);
    ଅନ୍ୟଥା {
        port = ahci_port_realloc(port);
        ଯଦି (port == ଖାଲି)
            ଫେରନ୍ତୁ;
        dprintf(1, "AHCI/%d: registering: \"%s\"\n", port->pnr, port->desc);
        ଯଦି (!port->atapi) {
            // Register with bcv system.
            boot_add_hd(&port->drive, port->desc, port->prio);
        } ଅନ୍ୟଥା {
            // fill cdidmap
            boot_add_cd(&port->drive, port->desc, port->prio);
        }
    }
}

// Initialize an ata controller and detect its drives.
ସ୍ଥିର ଶୂନ୍ୟ
ahci_controller_setup(ସଂରଚନା pci_device *pci)
{
    ସଂରଚନା ahci_port_s *port;
    u32 pnr, max;

    ଯଦି (create_bounce_buf() < 0)
        ଫେରନ୍ତୁ;

    ଶୂନ୍ୟ *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_5);
    ଯଦି (!iobase)
        ଫେରନ୍ତୁ;

    ସଂରଚନା ahci_ctrl_s *ctrl = malloc_fseg(ଆକାର(*ctrl));
    ଯଦି (!ctrl) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }

    ctrl->pci_tmp = pci;
    ctrl->iobase = iobase;
    ctrl->irq = pci_config_readb(pci->bdf, PCI_INTERRUPT_ଧାଡ଼ି);
    dprintf(1, "AHCI controller at %pP, iobase %p, irq %d\n"
            , pci, ctrl->iobase, ctrl->irq);

    pci_enable_busmaster(pci);

    u32 val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_RESET);
    u32 end = timer_calc(AHCI_RESET_TIMEOUT);
    ପାଇଁ (;;) {
        val = ahci_ctrl_readl(ctrl, HOST_CTL);
        ଯଦି (!(val & HOST_CTL_RESET))
            ଭାଙ୍ଗ;
        ଯଦି (timer_check(end)) {
            warn_timeout();
            dprintf(1, "AHCI: controller reset failed\n");
            ମୁକ୍ତ(ctrl);
            ଫେରନ୍ତୁ;
        }
        yield();
    }
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_AHCI_EN);
    (ଶୂନ୍ୟ)ahci_ctrl_readl(ctrl, HOST_CTL); /* Flush */

    ctrl->caps = ahci_ctrl_readl(ctrl, HOST_CAP);
    ctrl->ports = ahci_ctrl_readl(ctrl, HOST_PORTS_IMPL);
    dprintf(2, "AHCI: cap 0x%x, ports_impl 0x%x\n",
            ctrl->caps, ctrl->ports);

    max = 0x1f;
    ପାଇଁ (pnr = 0; pnr <= max; pnr++) {
        ଯଦି (!(ctrl->ports & (1 << pnr)))
            ଜାରି;
        port = ahci_port_alloc(ctrl, pnr);
        ଯଦି (port == ଖାଲି)
            ଜାରି;
        run_thread(ahci_port_detect, port);
    }
}

// Locate and init ahci controllers.
ସ୍ଥିର ଶୂନ୍ୟ
ahci_scan(ଶୂନ୍ୟ)
{
    // Scan PCI bus for ATA adapters
    ସଂରଚନା pci_device *pci;
    foreachpci(pci) {
        ଯଦି (pci->class != PCI_CLASS_STORAGE_SATA)
            ଜାରି;
        ଯଦି (pci->prog_if != 1 /* AHCI rev 1 */)
            ଜାରି;
        ahci_controller_setup(pci);
    }
}

ଶୂନ୍ୟ
ahci_setup(ଶୂନ୍ୟ)
{
    ASSERT32FLAT();
    ଯଦି (!CONFIG_AHCI)
        ଫେରନ୍ତୁ;

    dprintf(3, "init ahci\n");
    ahci_scan();
}

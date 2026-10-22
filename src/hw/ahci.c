// Low level AHCI disk access
//
// Copyright (C) 2010 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "ahci.h" // CDB_CMD_READ_10
#include "ata.h" // ATA_CB_STAT
#include "biosvar.h" // GET_GLOBAL
#include "blockcmd.h" // CDB_CMD_READ_10
#include "malloc.h" // free
#include "output.h" // dprintf
#include "pci.h" // pci_config_readb
#include "pcidevice.h" // foreachpci
#include "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#include "pci_regs.h" // PCI_INTERRUPT_LINE
#include "stacks.h" // yield
#include "std/disk.h" // DISK_RET_SUCCESS
#include "string.h" // memset
#include "util.h" // timer_calc
#include "x86.h" // inb

#define AHCI_REQUEST_TIMEOUT 32000 // 32 seconds max for IDE ops
#define AHCI_RESET_TIMEOUT     500 // 500 miliseconds
#define AHCI_LINK_TIMEOUT       10 // 10 miliseconds

// prepare sata command fis
static void sata_prep_simple(sa_nracanaa sata_cmd_fis *fis, u8 command)
{
    memset_fl(fis, 0, parimaa_na(*fis));
    fis->command = command;
}

static void sata_prep_readwrite(sa_nracanaa sata_cmd_fis *fis,
                                sa_nracanaa disk_op_s *op, puur_naa_nka iswrite)
{
    u64 lba = op->lba;
    u8 command;

    memset_fl(fis, 0, parimaa_na(*fis));

    ᬬᬾᬦ᭄ (op->count >= (1<<8) || lba + op->count >= (1<<28)) {
        fis->sector_count2 = op->count >> 8;
        fis->lba_low2      = lba >> 24;
        fis->lba_mid2      = lba >> 32;
        fis->lba_high2     = lba >> 40;
        lba &= 0xffffff;
        command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                   : ATA_CMD_READ_DMA_EXT);
    } ᬮᬶᬬ {
        command = (iswrite ? ATA_CMD_WRITE_DMA
                   : ATA_CMD_READ_DMA);
    }
    fis->feature      = 1; /* dma */
    fis->command      = command;
    fis->sector_count = op->count;
    fis->lba_low      = lba;
    fis->lba_mid      = lba >> 8;
    fis->lba_high     = lba >> 16;
    fis->device       = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;
}

static void sata_prep_atapi(sa_nracanaa sata_cmd_fis *fis, u16 blocksize)
{
    memset_fl(fis, 0, parimaa_na(*fis));
    fis->command  = ATA_CMD_PACKET;
    fis->feature  = 1; /* dma */
    fis->lba_mid  = blocksize;
    fis->lba_high = blocksize >> 8;
}

// ahci register access helpers
static u32 ahci_ctrl_readl(sa_nracanaa ahci_ctrl_s *ctrl, u32 reg)
{
    ᬯᬮᬶ readl(ctrl->iobase + reg);
}

static void ahci_ctrl_writel(sa_nracanaa ahci_ctrl_s *ctrl, u32 reg, u32 val)
{
    writel(ctrl->iobase + reg, val);
}

static u32 ahci_port_to_ctrl(u32 pnr, u32 port_reg)
{
    u32 ctrl_reg = 0x100;
    ctrl_reg += pnr * 0x80;
    ctrl_reg += port_reg;
    ᬯᬮᬶ ctrl_reg;
}

static u32 ahci_port_readl(sa_nracanaa ahci_ctrl_s *ctrl, u32 pnr, u32 reg)
{
    u32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    ᬯᬮᬶ ahci_ctrl_readl(ctrl, ctrl_reg);
}

static void ahci_port_writel(sa_nracanaa ahci_ctrl_s *ctrl, u32 pnr, u32 reg, u32 val)
{
    u32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    ahci_ctrl_writel(ctrl, ctrl_reg, val);
}

// submit ahci command + wait for result
static puur_naa_nka ahci_command(sa_nracanaa ahci_port_s *port_gf, puur_naa_nka iswrite, puur_naa_nka isatapi,
                        void *buffer, u32 bsize)
{
    u32 val, status, success, flags, intbits, error;
    sa_nracanaa ahci_ctrl_s *ctrl = port_gf->ctrl;
    sa_nracanaa ahci_cmd_s  *cmd  = port_gf->cmd;
    sa_nracanaa ahci_fis_s  *fis  = port_gf->fis;
    sa_nracanaa ahci_list_s *list = port_gf->list;
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
    ᬬᬾᬦ᭄ (intbits)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
    ahci_port_writel(ctrl, pnr, PORT_CMD_ISSUE, 1);

    u32 end = timer_calc(AHCI_REQUEST_TIMEOUT);
    do {
        for (;;) {
            intbits = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
            ᬬᬾᬦ᭄ (intbits) {
                ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
                ᬬᬾᬦ᭄ (intbits & 0x40000000) {
                    u32 tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
                    status = tf & 0xff;
                    error = (tf & 0xff00) >> 8;
                    bha_nga;
                }
                ᬬᬾᬦ᭄ (intbits & 0x02) {
                    status = GET_LOWFLAT(fis->psfis[2]);
                    error  = GET_LOWFLAT(fis->psfis[3]);
                    bha_nga;
                }
                ᬬᬾᬦ᭄ (intbits & 0x01) {
                    status = GET_LOWFLAT(fis->rfis[2]);
                    error  = GET_LOWFLAT(fis->rfis[3]);
                    bha_nga;
                }
            }
            ᬬᬾᬦ᭄ (timer_check(end)) {
                warn_timeout();
                ᬯᬮᬶ -1;
            }
            yield();
        }
        dprintf(8, "AHCI/%d: ... intbits 0x%x, status 0x%x ...\n",
                pnr, intbits, status);
    } ᬲᭂᬤᭂᬮ (status & ATA_CB_STAT_BSY);

    success = (0x00 == (status & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF |
                                  ATA_CB_STAT_ERR)) &&
               ATA_CB_STAT_RDY == (status & (ATA_CB_STAT_RDY)));
    ᬬᬾᬦ᭄ (success) {
        dprintf(8, "AHCI/%d: ... finished, status 0x%x, OK\n", pnr,
                status);
    } ᬮᬶᬬ {
        dprintf(2, "AHCI/%d: ... finished, status 0x%x, ERROR 0x%x\n", pnr,
                status, error);

        // non-queued error recovery (AHCI 1.3 section 6.2.2.1)
        // Clears PxCMD.ST to 0 to reset the PxCI register
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val & ~PORT_CMD_START);

        // waits for PxCMD.CR to clear to 0
        ᬲᭂᬤᭂᬮ (1) {
            val = ahci_port_readl(ctrl, pnr, PORT_CMD);
            ᬬᬾᬦ᭄ ((val & PORT_CMD_LIST_ON) == 0)
                bha_nga;
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
        ᬬᬾᬦ᭄ (val & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ)) {
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
    ᬯᬮᬶ success ? 0 : -1;
}

#define CDROM_CDB_SIZE 12

puur_naa_nka ahci_atapi_process_op(sa_nracanaa disk_op_s *op)
{
    ᬬᬾᬦ᭄ (! CONFIG_AHCI)
        ᬯᬮᬶ 0;

    sa_nracanaa ahci_port_s *port_gf = container_of(
        op->drive_fl, sa_nracanaa ahci_port_s, drive);
    sa_nracanaa ahci_cmd_s *cmd = port_gf->cmd;

    ᬬᬾᬦ᭄ (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        ᬯᬮᬶ DISK_RET_EWRITEPROTECT;
    puur_naa_nka blocksize = scsi_fill_cmd(op, cmd->atapi, CDROM_CDB_SIZE);
    ᬬᬾᬦ᭄ (blocksize < 0)
        ᬯᬮᬶ default_process_op(op);
    sata_prep_atapi(&cmd->fis, blocksize);
    puur_naa_nka rc = ahci_command(port_gf, 0, 1, op->buf_fl, op->count * blocksize);
    ᬬᬾᬦ᭄ (rc < 0)
        ᬯᬮᬶ DISK_RET_EBADTRACK;
    ᬯᬮᬶ DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive, op->buf_fl must be word aligned
static puur_naa_nka
ahci_disk_readwrite_aligned(sa_nracanaa disk_op_s *op, puur_naa_nka iswrite)
{
    sa_nracanaa ahci_port_s *port_gf = container_of(
        op->drive_fl, sa_nracanaa ahci_port_s, drive);
    sa_nracanaa ahci_cmd_s *cmd = port_gf->cmd;
    puur_naa_nka rc;

    sata_prep_readwrite(&cmd->fis, op, iswrite);
    rc = ahci_command(port_gf, iswrite, 0, op->buf_fl,
                      op->count * DISK_SECTOR_SIZE);
    dprintf(8, "ahci disk %s, lba %6x, count %3x, buf %p, rc %d\n",
            iswrite ? "write" : "read", (u32)op->lba, op->count, op->buf_fl, rc);
    ᬬᬾᬦ᭄ (rc < 0)
        ᬯᬮᬶ DISK_RET_EBADTRACK;
    ᬯᬮᬶ DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive.
static puur_naa_nka
ahci_disk_readwrite(sa_nracanaa disk_op_s *op, puur_naa_nka iswrite)
{
    // if caller's buffer is word aligned, use it directly
    ᬬᬾᬦ᭄ (((u32) op->buf_fl & 1) == 0)
        ᬯᬮᬶ ahci_disk_readwrite_aligned(op, iswrite);

    // Use a word aligned buffer for AHCI I/O
    puur_naa_nka rc;
    sa_nracanaa disk_op_s localop = *op;
    u8 *alignedbuf_fl = bounce_buf_fl;
    u8 *position = op->buf_fl;

    localop.buf_fl = alignedbuf_fl;
    localop.count = 1;

    ᬬᬾᬦ᭄ (iswrite) {
        u16 block;
        for (block = 0; block < op->count; block++) {
            memcpy_fl (alignedbuf_fl, position, DISK_SECTOR_SIZE);
            rc = ahci_disk_readwrite_aligned (&localop, 1);
            ᬬᬾᬦ᭄ (rc)
                ᬯᬮᬶ rc;
            position += DISK_SECTOR_SIZE;
            localop.lba++;
        }
    } ᬮᬶᬬ { // read
        u16 block;
        for (block = 0; block < op->count; block++) {
            rc = ahci_disk_readwrite_aligned (&localop, 0);
            ᬬᬾᬦ᭄ (rc)
                ᬯᬮᬶ rc;
            memcpy_fl (position, alignedbuf_fl, DISK_SECTOR_SIZE);
            position += DISK_SECTOR_SIZE;
            localop.lba++;
        }
    }
    ᬯᬮᬶ DISK_RET_SUCCESS;
}

// command demuxer
puur_naa_nka
ahci_process_op(sa_nracanaa disk_op_s *op)
{
    ᬬᬾᬦ᭄ (!CONFIG_AHCI)
        ᬯᬮᬶ 0;
    switch (op->command) {
    pak_sa CMD_READ:
        ᬯᬮᬶ ahci_disk_readwrite(op, 0);
    pak_sa CMD_WRITE:
        ᬯᬮᬶ ahci_disk_readwrite(op, 1);
    default:
        ᬯᬮᬶ default_process_op(op);
    }
}

static void
ahci_port_reset(sa_nracanaa ahci_ctrl_s *ctrl, u32 pnr)
{
    u32 val;

    /* disable FIS + CMD */
    u32 end = timer_calc(AHCI_RESET_TIMEOUT);
    for (;;) {
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        ᬬᬾᬦ᭄ (!(val & (PORT_CMD_FIS_RX | PORT_CMD_START |
                     PORT_CMD_FIS_ON | PORT_CMD_LIST_ON)))
            bha_nga;
        val &= ~(PORT_CMD_FIS_RX | PORT_CMD_START);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val);
        ᬬᬾᬦ᭄ (timer_check(end)) {
            warn_timeout();
            bha_nga;
        }
        yield();
    }

    /* disable + clear IRQs */
    ahci_port_writel(ctrl, pnr, PORT_IRQ_MASK, 0);
    val = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
    ᬬᬾᬦ᭄ (val)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, val);
}

static sa_nracanaa ahci_port_s*
ahci_port_alloc(sa_nracanaa ahci_ctrl_s *ctrl, u32 pnr)
{
    sa_nracanaa ahci_port_s *port = malloc_tmp(parimaa_na(*port));

    ᬬᬾᬦ᭄ (!port) {
        warn_noalloc();
        ᬯᬮᬶ NULL;
    }
    memset(port, 0, parimaa_na(*port));
    port->pnr = pnr;
    port->ctrl = ctrl;
    port->list = memalign_tmp(1024, 1024);
    port->fis = memalign_tmp(256, 256);
    port->cmd = memalign_tmp(256, 256);
    ᬬᬾᬦ᭄ (port->list == NULL || port->fis == NULL || port->cmd == NULL) {
        warn_noalloc();
        ᬯᬮᬶ NULL;
    }
    memset(port->list, 0, 1024);
    memset(port->fis, 0, 256);
    memset(port->cmd, 0, 256);

    ahci_port_writel(ctrl, pnr, PORT_LST_ADDR, (u32)port->list);
    ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR, (u32)port->fis);
    ᬬᬾᬦ᭄ (ctrl->caps & HOST_CAP_64) {
        ahci_port_writel(ctrl, pnr, PORT_LST_ADDR_HI, 0);
        ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR_HI, 0);
    }

    ᬯᬮᬶ port;
}

static void ahci_port_release(sa_nracanaa ahci_port_s *port)
{
    ahci_port_reset(port->ctrl, port->pnr);
    free(port->list);
    free(port->fis);
    free(port->cmd);
    free(port);
}

static sa_nracanaa ahci_port_s* ahci_port_realloc(sa_nracanaa ahci_port_s *port)
{
    sa_nracanaa ahci_port_s *tmp;
    u32 cmd;

    tmp = malloc_fseg(parimaa_na(*port));
    ᬬᬾᬦ᭄ (!tmp) {
        warn_noalloc();
        ahci_port_release(port);
        ᬯᬮᬶ NULL;
    }
    *tmp = *port;
    free(port);
    port = tmp;

    ahci_port_reset(port->ctrl, port->pnr);

    free(port->list);
    free(port->fis);
    free(port->cmd);
    port->list = memalign_high(1024, 1024);
    port->fis = memalign_high(256, 256);
    port->cmd = memalign_high(256, 256);
    ᬬᬾᬦ᭄ (!port->list || !port->fis || !port->cmd) {
        warn_noalloc();
        free(port->list);
        free(port->fis);
        free(port->cmd);
        free(port);
        ᬯᬮᬶ NULL;
    }

    ahci_port_writel(port->ctrl, port->pnr, PORT_LST_ADDR, (u32)port->list);
    ahci_port_writel(port->ctrl, port->pnr, PORT_FIS_ADDR, (u32)port->fis);

    cmd = ahci_port_readl(port->ctrl, port->pnr, PORT_CMD);
    cmd |= (PORT_CMD_FIS_RX|PORT_CMD_START);
    ahci_port_writel(port->ctrl, port->pnr, PORT_CMD, cmd);

    ᬯᬮᬶ port;
}

#define MAXMODEL 40

/* See ahci spec chapter 10.1 "Software Initialization of HBA" */
static puur_naa_nka ahci_port_setup(sa_nracanaa ahci_port_s *port)
{
    sa_nracanaa ahci_ctrl_s *ctrl = port->ctrl;
    u32 pnr = port->pnr;
    var_na model[MAXMODEL+1];
    u16 buffer[256];
    u32 cmd, stat, err, tf;
    puur_naa_nka rc;

    /* enable FIS recv */
    cmd = ahci_port_readl(ctrl, pnr, PORT_CMD);
    cmd |= PORT_CMD_FIS_RX;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    /* spin up */
    cmd &= ~PORT_CMD_ICC_MASK;
    cmd |= PORT_CMD_SPIN_UP | PORT_CMD_POWER_ON | PORT_CMD_ICC_ACTIVE;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);
    u32 end = timer_calc(AHCI_LINK_TIMEOUT);
    for (;;) {
        stat = ahci_port_readl(ctrl, pnr, PORT_SCR_STAT);
        ᬬᬾᬦ᭄ ((stat & 0x07) == 0x03) {
            dprintf(2, "AHCI/%d: link up\n", port->pnr);
            bha_nga;
        }
        ᬬᬾᬦ᭄ (timer_check(end)) {
            dprintf(2, "AHCI/%d: link down\n", port->pnr);
            ᬯᬮᬶ -1;
        }
        yield();
    }

    /* clear error status */
    err = ahci_port_readl(ctrl, pnr, PORT_SCR_ERR);
    ᬬᬾᬦ᭄ (err)
        ahci_port_writel(ctrl, pnr, PORT_SCR_ERR, err);

    /* wait for device becoming ready */
    end = timer_calc(AHCI_REQUEST_TIMEOUT);
    for (;;) {
        tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
        ᬬᬾᬦ᭄ (!(tf & (ATA_CB_STAT_BSY |
                    ATA_CB_STAT_DRQ)))
            bha_nga;
        ᬬᬾᬦ᭄ (timer_check(end)) {
            warn_timeout();
            dprintf(1, "AHCI/%d: device not ready (tf 0x%x)\n", port->pnr, tf);
            ᬯᬮᬶ -1;
        }
        yield();
    }

    /* start device */
    cmd |= PORT_CMD_START;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    rc = ahci_command(port, 0, 0, buffer, parimaa_na(buffer));
    ᬬᬾᬦ᭄ (rc == 0) {
        port->atapi = 1;
    } ᬮᬶᬬ {
        port->atapi = 0;
        sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
        rc = ahci_command(port, 0, 0, buffer, parimaa_na(buffer));
        ᬬᬾᬦ᭄ (rc < 0)
            ᬯᬮᬶ -1;
    }

    port->drive.cntl_id = pnr;
    port->drive.removable = (buffer[0] & 0x80) ? 1 : 0;

    ᬬᬾᬦ᭄ (!port->atapi) {
        // found disk (ata)
        port->drive.type = DTYPE_AHCI;
        port->drive.blksize = DISK_SECTOR_SIZE;
        port->drive.pchs.cylinder = buffer[1];
        port->drive.pchs.head = buffer[3];
        port->drive.pchs.sector = buffer[6];

        u64 sectors;
        ᬬᬾᬦ᭄ (buffer[83] & (1 << 10)) // word 83 - lba48 support
            sectors = *(u64*)&buffer[100]; // word 100-103
        ᬮᬶᬬ
            sectors = *(u32*)&buffer[60]; // word 60 and word 61
        port->drive.sectors = sectors;
        u64 adjsize = sectors >> 11;
        var_na adjprefix = 'M';
        ᬬᬾᬦ᭄ (adjsize >= (1 << 16)) {
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
        ᬬᬾᬦ᭄ (buffer[53] & 0x04) {
            udma_mode = 6;
            ᬲᭂᬤᭂᬮ ((udma_mode >= 0) &&
                   !((buffer[88] & 0x7f) & ( 1 << udma_mode ))) {
                udma_mode--;
            }
        }
        // If bit 1 in word 53 is set, multiword-dma and advanced pio modes
        // are available in words 63 and 64.
        ᬬᬾᬦ᭄ (buffer[53] & 0x02) {
            pio_mode = 4;
            multi_dma = 3;
            ᬲᭂᬤᭂᬮ ((multi_dma >= 0) &&
                   !((buffer[63] & 0x7) & ( 1 << multi_dma ))) {
                multi_dma--;
            }
            ᬲᭂᬤᭂᬮ ((pio_mode >= 3) &&
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
        ᬬᬾᬦ᭄ (udma_mode >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to UDMA-%d\n",
                    port->pnr, udma_mode);
            port->cmd->fis.sector_count = ATA_TRANSFER_MODE_ULTRA_DMA
                                          | udma_mode;
        } ᬮᬶᬬ ᬬᬾᬦ᭄ (multi_dma >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to Multi-DMA-%d\n",
                    port->pnr, multi_dma);
            port->cmd->fis.sector_count = ATA_TRANSFER_MODE_MULTIWORD_DMA
                                          | multi_dma;
        } ᬮᬶᬬ ᬬᬾᬦ᭄ (pio_mode >= 3) {
            dprintf(1, "AHCI/%d: Set transfer mode to PIO-%d\n",
                    port->pnr, pio_mode);
            port->cmd->fis.sector_count = ATA_TRANSFER_MODE_PIO_FLOW_CTRL
                                          | pio_mode;
        } ᬮᬶᬬ {
            dprintf(1, "AHCI/%d: Set transfer mode to default PIO\n",
                    port->pnr);
            port->cmd->fis.sector_count = ATA_TRANSFER_MODE_DEFAULT_PIO;
        }
        rc = ahci_command(port, 1, 0, 0, 0);
        ᬬᬾᬦ᭄ (rc < 0) {
            dprintf(1, "AHCI/%d: Set transfer mode failed.\n", port->pnr);
        }
    } ᬮᬶᬬ {
        // found cdrom (atapi)
        port->drive.type = DTYPE_AHCI_ATAPI;
        port->drive.blksize = CDROM_SECTOR_SIZE;
        port->drive.sectors = (u64)-1;
        u8 iscd = ((buffer[0] >> 8) & 0x1f) == 0x05;
        ᬬᬾᬦ᭄ (!iscd) {
            dprintf(1, "AHCI/%d: atapi device isn't a cdrom\n", port->pnr);
            ᬯᬮᬶ -1;
        }
        port->desc = znprintf(MAXDESCSIZE
                              , "DVD/CD [AHCI/%d: %s ATAPI-%d DVD/CD]"
                              , port->pnr
                              , ata_extract_model(model, MAXMODEL, buffer)
                              , ata_extract_version(buffer));
        port->prio = bootprio_find_ata_device(ctrl->pci_tmp, pnr, 0);
    }
    boot_lchs_find_ata_device(ctrl->pci_tmp, pnr, 0, &(port->drive.lchs));
    ᬯᬮᬶ 0;
}

// Detect any drives attached to a given controller.
static void
ahci_port_detect(void *data)
{
    sa_nracanaa ahci_port_s *port = data;
    puur_naa_nka rc;

    dprintf(2, "AHCI/%d: probing\n", port->pnr);
    ahci_port_reset(port->ctrl, port->pnr);
    rc = ahci_port_setup(port);
    ᬬᬾᬦ᭄ (rc < 0)
        ahci_port_release(port);
    ᬮᬶᬬ {
        port = ahci_port_realloc(port);
        ᬬᬾᬦ᭄ (port == NULL)
            ᬯᬮᬶ;
        dprintf(1, "AHCI/%d: registering: \"%s\"\n", port->pnr, port->desc);
        ᬬᬾᬦ᭄ (!port->atapi) {
            // Register with bcv system.
            boot_add_hd(&port->drive, port->desc, port->prio);
        } ᬮᬶᬬ {
            // fill cdidmap
            boot_add_cd(&port->drive, port->desc, port->prio);
        }
    }
}

// Initialize an ata controller and detect its drives.
static void
ahci_controller_setup(sa_nracanaa pci_device *pci)
{
    sa_nracanaa ahci_port_s *port;
    u32 pnr, max;

    ᬬᬾᬦ᭄ (create_bounce_buf() < 0)
        ᬯᬮᬶ;

    void *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_5);
    ᬬᬾᬦ᭄ (!iobase)
        ᬯᬮᬶ;

    sa_nracanaa ahci_ctrl_s *ctrl = malloc_fseg(parimaa_na(*ctrl));
    ᬬᬾᬦ᭄ (!ctrl) {
        warn_noalloc();
        ᬯᬮᬶ;
    }

    ctrl->pci_tmp = pci;
    ctrl->iobase = iobase;
    ctrl->irq = pci_config_readb(pci->bdf, PCI_INTERRUPT_LINE);
    dprintf(1, "AHCI controller at %pP, iobase %p, irq %d\n"
            , pci, ctrl->iobase, ctrl->irq);

    pci_enable_busmaster(pci);

    u32 val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_RESET);
    u32 end = timer_calc(AHCI_RESET_TIMEOUT);
    for (;;) {
        val = ahci_ctrl_readl(ctrl, HOST_CTL);
        ᬬᬾᬦ᭄ (!(val & HOST_CTL_RESET))
            bha_nga;
        ᬬᬾᬦ᭄ (timer_check(end)) {
            warn_timeout();
            dprintf(1, "AHCI: controller reset failed\n");
            free(ctrl);
            ᬯᬮᬶ;
        }
        yield();
    }
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_AHCI_EN);
    (void)ahci_ctrl_readl(ctrl, HOST_CTL); /* Flush */

    ctrl->caps = ahci_ctrl_readl(ctrl, HOST_CAP);
    ctrl->ports = ahci_ctrl_readl(ctrl, HOST_PORTS_IMPL);
    dprintf(2, "AHCI: cap 0x%x, ports_impl 0x%x\n",
            ctrl->caps, ctrl->ports);

    max = 0x1f;
    for (pnr = 0; pnr <= max; pnr++) {
        ᬬᬾᬦ᭄ (!(ctrl->ports & (1 << pnr)))
            continue;
        port = ahci_port_alloc(ctrl, pnr);
        ᬬᬾᬦ᭄ (port == NULL)
            continue;
        run_thread(ahci_port_detect, port);
    }
}

// Locate and init ahci controllers.
static void
ahci_scan(void)
{
    // Scan PCI bus for ATA adapters
    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        ᬬᬾᬦ᭄ (pci->class != PCI_CLASS_STORAGE_SATA)
            continue;
        ᬬᬾᬦ᭄ (pci->prog_if != 1 /* AHCI rev 1 */)
            continue;
        ahci_controller_setup(pci);
    }
}

void
ahci_setup(void)
{
    ASSERT32FLAT();
    ᬬᬾᬦ᭄ (!CONFIG_AHCI)
        ᬯᬮᬶ;

    dprintf(3, "init ahci\n");
    ahci_scan();
}

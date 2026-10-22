// Low level AHCI disk access
//
// Copyright (C) 2010 Gerd Hoffmann <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "ahci.h" // CDB_CMD_READ_10
#அந்தர்பாவ "ata.h" // ATA_CB_STAT
#அந்தர்பாவ "biosvar.h" // GET_GLOBAL
#அந்தர்பாவ "blockcmd.h" // CDB_CMD_READ_10
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "pci.h" // pci_config_readb
#அந்தர்பாவ "pcidevice.h" // foreachpci
#அந்தர்பாவ "pci_ids.h" // PCI_CLASS_STORAGE_OTHER
#அந்தர்பாவ "pci_regs.h" // PCI_INTERRUPT_LINE
#அந்தர்பாவ "stacks.h" // yield
#அந்தர்பாவ "std/disk.h" // DISK_RET_SUCCESS
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // timer_calc
#அந்தர்பாவ "x86.h" // inb

#define AHCI_REQUEST_TIMEOUT 32000 // 32 seconds max for IDE ops
#define AHCI_RESET_TIMEOUT     500 // 500 miliseconds
#define AHCI_LINK_TIMEOUT       10 // 10 miliseconds

// prepare sata command fis
நிலையான வெறுமை sata_prep_simple(கட்டமைப்பு sata_cmd_fis *fis, u8 command)
{
    memset_fl(fis, 0, அளவு(*fis));
    fis->command = command;
}

நிலையான வெறுமை sata_prep_readwrite(கட்டமைப்பு sata_cmd_fis *fis,
                                கட்டமைப்பு disk_op_s *op, முழு iswrite)
{
    u64 lba = op->lba;
    u8 command;

    memset_fl(fis, 0, அளவு(*fis));

    எனில் (op->மொத்தம் >= (1<<8) || lba + op->மொத்தம் >= (1<<28)) {
        fis->sector_count2 = op->மொத்தம் >> 8;
        fis->lba_low2      = lba >> 24;
        fis->lba_mid2      = lba >> 32;
        fis->lba_high2     = lba >> 40;
        lba &= 0xffffff;
        command = (iswrite ? ATA_CMD_WRITE_DMA_EXT
                   : ATA_CMD_READ_DMA_EXT);
    } இல்லையெனில் {
        command = (iswrite ? ATA_CMD_WRITE_DMA
                   : ATA_CMD_READ_DMA);
    }
    fis->feature      = 1; /* dma */
    fis->command      = command;
    fis->sector_மொத்தம் = op->மொத்தம்;
    fis->lba_கீழ்      = lba;
    fis->lba_mid      = lba >> 8;
    fis->lba_உயர்வு     = lba >> 16;
    fis->device       = ((lba >> 24) & 0xf) | ATA_CB_DH_LBA;
}

நிலையான வெறுமை sata_prep_atapi(கட்டமைப்பு sata_cmd_fis *fis, u16 blocksize)
{
    memset_fl(fis, 0, அளவு(*fis));
    fis->command  = ATA_CMD_PACKET;
    fis->feature  = 1; /* dma */
    fis->lba_mid  = blocksize;
    fis->lba_உயர்வு = blocksize >> 8;
}

// ahci register access helpers
நிலையான u32 ahci_ctrl_readl(கட்டமைப்பு ahci_ctrl_s *ctrl, u32 reg)
{
    திரும்பு readl(ctrl->iobase + reg);
}

நிலையான வெறுமை ahci_ctrl_writel(கட்டமைப்பு ahci_ctrl_s *ctrl, u32 reg, u32 val)
{
    writel(ctrl->iobase + reg, val);
}

நிலையான u32 ahci_port_to_ctrl(u32 pnr, u32 port_reg)
{
    u32 ctrl_reg = 0x100;
    ctrl_reg += pnr * 0x80;
    ctrl_reg += port_reg;
    திரும்பு ctrl_reg;
}

நிலையான u32 ahci_port_readl(கட்டமைப்பு ahci_ctrl_s *ctrl, u32 pnr, u32 reg)
{
    u32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    திரும்பு ahci_ctrl_readl(ctrl, ctrl_reg);
}

நிலையான வெறுமை ahci_port_writel(கட்டமைப்பு ahci_ctrl_s *ctrl, u32 pnr, u32 reg, u32 val)
{
    u32 ctrl_reg = ahci_port_to_ctrl(pnr, reg);
    ahci_ctrl_writel(ctrl, ctrl_reg, val);
}

// submit ahci command + wait for result
நிலையான முழு ahci_command(கட்டமைப்பு ahci_port_s *port_gf, முழு iswrite, முழு isatapi,
                        வெறுமை *buffer, u32 bsize)
{
    u32 val, status, success, flags, intbits, error;
    கட்டமைப்பு ahci_ctrl_s *ctrl = port_gf->ctrl;
    கட்டமைப்பு ahci_cmd_s  *cmd  = port_gf->cmd;
    கட்டமைப்பு ahci_fis_s  *fis  = port_gf->fis;
    கட்டமைப்பு ahci_list_s *list = port_gf->list;
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
    எனில் (intbits)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
    ahci_port_writel(ctrl, pnr, PORT_CMD_ISSUE, 1);

    u32 end = timer_calc(AHCI_REQUEST_TIMEOUT);
    செய் {
        க்கு (;;) {
            intbits = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
            எனில் (intbits) {
                ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, intbits);
                எனில் (intbits & 0x40000000) {
                    u32 tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
                    status = tf & 0xff;
                    error = (tf & 0xff00) >> 8;
                    நிறுத்து;
                }
                எனில் (intbits & 0x02) {
                    status = GET_LOWFLAT(fis->psfis[2]);
                    error  = GET_LOWFLAT(fis->psfis[3]);
                    நிறுத்து;
                }
                எனில் (intbits & 0x01) {
                    status = GET_LOWFLAT(fis->rfis[2]);
                    error  = GET_LOWFLAT(fis->rfis[3]);
                    நிறுத்து;
                }
            }
            எனில் (timer_check(end)) {
                warn_timeout();
                திரும்பு -1;
            }
            yield();
        }
        dprintf(8, "AHCI/%d: ... intbits 0x%x, status 0x%x ...\n",
                pnr, intbits, status);
    } போது (status & ATA_CB_STAT_BSY);

    success = (0x00 == (status & (ATA_CB_STAT_BSY | ATA_CB_STAT_DF |
                                  ATA_CB_STAT_ERR)) &&
               ATA_CB_STAT_RDY == (status & (ATA_CB_STAT_RDY)));
    எனில் (success) {
        dprintf(8, "AHCI/%d: ... finished, status 0x%x, OK\n", pnr,
                status);
    } இல்லையெனில் {
        dprintf(2, "AHCI/%d: ... finished, status 0x%x, ERROR 0x%x\n", pnr,
                status, error);

        // non-queued error recovery (AHCI 1.3 section 6.2.2.1)
        // Clears PxCMD.ST to 0 to reset the PxCI register
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val & ~PORT_CMD_START);

        // waits for PxCMD.CR to clear to 0
        போது (1) {
            val = ahci_port_readl(ctrl, pnr, PORT_CMD);
            எனில் ((val & PORT_CMD_LIST_ON) == 0)
                நிறுத்து;
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
        எனில் (val & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ)) {
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
    திரும்பு success ? 0 : -1;
}

#define CDROM_CDB_SIZE 12

முழு ahci_atapi_process_op(கட்டமைப்பு disk_op_s *op)
{
    எனில் (! CONFIG_AHCI)
        திரும்பு 0;

    கட்டமைப்பு ahci_port_s *port_gf = container_of(
        op->drive_fl, கட்டமைப்பு ahci_port_s, drive);
    கட்டமைப்பு ahci_cmd_s *cmd = port_gf->cmd;

    எனில் (op->command == CMD_WRITE || op->command == CMD_FORMAT)
        திரும்பு DISK_RET_EWRITEPROTECT;
    முழு blocksize = scsi_fill_cmd(op, cmd->atapi, CDROM_CDB_SIZE);
    எனில் (blocksize < 0)
        திரும்பு default_process_op(op);
    sata_prep_atapi(&cmd->fis, blocksize);
    முழு rc = ahci_command(port_gf, 0, 1, op->buf_fl, op->மொத்தம் * blocksize);
    எனில் (rc < 0)
        திரும்பு DISK_RET_EBADTRACK;
    திரும்பு DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive, op->buf_fl must be word aligned
நிலையான முழு
ahci_disk_readwrite_aligned(கட்டமைப்பு disk_op_s *op, முழு iswrite)
{
    கட்டமைப்பு ahci_port_s *port_gf = container_of(
        op->drive_fl, கட்டமைப்பு ahci_port_s, drive);
    கட்டமைப்பு ahci_cmd_s *cmd = port_gf->cmd;
    முழு rc;

    sata_prep_readwrite(&cmd->fis, op, iswrite);
    rc = ahci_command(port_gf, iswrite, 0, op->buf_fl,
                      op->மொத்தம் * DISK_SECTOR_SIZE);
    dprintf(8, "ahci disk %s, lba %6x, count %3x, buf %p, rc %d\n",
            iswrite ? "write" : "read", (u32)op->lba, op->மொத்தம், op->buf_fl, rc);
    எனில் (rc < 0)
        திரும்பு DISK_RET_EBADTRACK;
    திரும்பு DISK_RET_SUCCESS;
}

// read/write count blocks from a harddrive.
நிலையான முழு
ahci_disk_readwrite(கட்டமைப்பு disk_op_s *op, முழு iswrite)
{
    // if caller's buffer is word aligned, use it directly
    எனில் (((u32) op->buf_fl & 1) == 0)
        திரும்பு ahci_disk_readwrite_aligned(op, iswrite);

    // Use a word aligned buffer for AHCI I/O
    முழு rc;
    கட்டமைப்பு disk_op_s localop = *op;
    u8 *alignedbuf_fl = bounce_buf_fl;
    u8 *position = op->buf_fl;

    localop.buf_fl = alignedbuf_fl;
    localop.மொத்தம் = 1;

    எனில் (iswrite) {
        u16 block;
        க்கு (block = 0; block < op->மொத்தம்; block++) {
            memcpy_fl (alignedbuf_fl, position, DISK_SECTOR_SIZE);
            rc = ahci_disk_readwrite_aligned (&localop, 1);
            எனில் (rc)
                திரும்பு rc;
            position += DISK_SECTOR_SIZE;
            localop.lba++;
        }
    } இல்லையெனில் { // read
        u16 block;
        க்கு (block = 0; block < op->மொத்தம்; block++) {
            rc = ahci_disk_readwrite_aligned (&localop, 0);
            எனில் (rc)
                திரும்பு rc;
            memcpy_fl (position, alignedbuf_fl, DISK_SECTOR_SIZE);
            position += DISK_SECTOR_SIZE;
            localop.lba++;
        }
    }
    திரும்பு DISK_RET_SUCCESS;
}

// command demuxer
முழு
ahci_process_op(கட்டமைப்பு disk_op_s *op)
{
    எனில் (!CONFIG_AHCI)
        திரும்பு 0;
    தேர்வு (op->command) {
    நிலை CMD_READ:
        திரும்பு ahci_disk_readwrite(op, 0);
    நிலை CMD_WRITE:
        திரும்பு ahci_disk_readwrite(op, 1);
    இயல்பு:
        திரும்பு default_process_op(op);
    }
}

நிலையான வெறுமை
ahci_port_reset(கட்டமைப்பு ahci_ctrl_s *ctrl, u32 pnr)
{
    u32 val;

    /* disable FIS + CMD */
    u32 end = timer_calc(AHCI_RESET_TIMEOUT);
    க்கு (;;) {
        val = ahci_port_readl(ctrl, pnr, PORT_CMD);
        எனில் (!(val & (PORT_CMD_FIS_RX | PORT_CMD_START |
                     PORT_CMD_FIS_ON | PORT_CMD_LIST_ON)))
            நிறுத்து;
        val &= ~(PORT_CMD_FIS_RX | PORT_CMD_START);
        ahci_port_writel(ctrl, pnr, PORT_CMD, val);
        எனில் (timer_check(end)) {
            warn_timeout();
            நிறுத்து;
        }
        yield();
    }

    /* disable + clear IRQs */
    ahci_port_writel(ctrl, pnr, PORT_IRQ_MASK, 0);
    val = ahci_port_readl(ctrl, pnr, PORT_IRQ_STAT);
    எனில் (val)
        ahci_port_writel(ctrl, pnr, PORT_IRQ_STAT, val);
}

நிலையான கட்டமைப்பு ahci_port_s*
ahci_port_alloc(கட்டமைப்பு ahci_ctrl_s *ctrl, u32 pnr)
{
    கட்டமைப்பு ahci_port_s *port = malloc_tmp(அளவு(*port));

    எனில் (!port) {
        warn_noalloc();
        திரும்பு வெற்று;
    }
    memset(port, 0, அளவு(*port));
    port->pnr = pnr;
    port->ctrl = ctrl;
    port->list = memalign_tmp(1024, 1024);
    port->fis = memalign_tmp(256, 256);
    port->cmd = memalign_tmp(256, 256);
    எனில் (port->list == வெற்று || port->fis == வெற்று || port->cmd == வெற்று) {
        warn_noalloc();
        திரும்பு வெற்று;
    }
    memset(port->list, 0, 1024);
    memset(port->fis, 0, 256);
    memset(port->cmd, 0, 256);

    ahci_port_writel(ctrl, pnr, PORT_LST_ADDR, (u32)port->list);
    ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR, (u32)port->fis);
    எனில் (ctrl->caps & HOST_CAP_64) {
        ahci_port_writel(ctrl, pnr, PORT_LST_ADDR_HI, 0);
        ahci_port_writel(ctrl, pnr, PORT_FIS_ADDR_HI, 0);
    }

    திரும்பு port;
}

நிலையான வெறுமை ahci_port_release(கட்டமைப்பு ahci_port_s *port)
{
    ahci_port_reset(port->ctrl, port->pnr);
    விடுவி(port->list);
    விடுவி(port->fis);
    விடுவி(port->cmd);
    விடுவி(port);
}

நிலையான கட்டமைப்பு ahci_port_s* ahci_port_realloc(கட்டமைப்பு ahci_port_s *port)
{
    கட்டமைப்பு ahci_port_s *tmp;
    u32 cmd;

    tmp = malloc_fseg(அளவு(*port));
    எனில் (!tmp) {
        warn_noalloc();
        ahci_port_release(port);
        திரும்பு வெற்று;
    }
    *tmp = *port;
    விடுவி(port);
    port = tmp;

    ahci_port_reset(port->ctrl, port->pnr);

    விடுவி(port->list);
    விடுவி(port->fis);
    விடுவி(port->cmd);
    port->list = memalign_உயர்வு(1024, 1024);
    port->fis = memalign_உயர்வு(256, 256);
    port->cmd = memalign_உயர்வு(256, 256);
    எனில் (!port->list || !port->fis || !port->cmd) {
        warn_noalloc();
        விடுவி(port->list);
        விடுவி(port->fis);
        விடுவி(port->cmd);
        விடுவி(port);
        திரும்பு வெற்று;
    }

    ahci_port_writel(port->ctrl, port->pnr, PORT_LST_ADDR, (u32)port->list);
    ahci_port_writel(port->ctrl, port->pnr, PORT_FIS_ADDR, (u32)port->fis);

    cmd = ahci_port_readl(port->ctrl, port->pnr, PORT_CMD);
    cmd |= (PORT_CMD_FIS_RX|PORT_CMD_START);
    ahci_port_writel(port->ctrl, port->pnr, PORT_CMD, cmd);

    திரும்பு port;
}

#define MAXMODEL 40

/* See ahci spec chapter 10.1 "Software Initialization of HBA" */
நிலையான முழு ahci_port_setup(கட்டமைப்பு ahci_port_s *port)
{
    கட்டமைப்பு ahci_ctrl_s *ctrl = port->ctrl;
    u32 pnr = port->pnr;
    எழுத்து model[MAXMODEL+1];
    u16 buffer[256];
    u32 cmd, stat, err, tf;
    முழு rc;

    /* enable FIS recv */
    cmd = ahci_port_readl(ctrl, pnr, PORT_CMD);
    cmd |= PORT_CMD_FIS_RX;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    /* spin up */
    cmd &= ~PORT_CMD_ICC_MASK;
    cmd |= PORT_CMD_SPIN_UP | PORT_CMD_POWER_ON | PORT_CMD_ICC_ACTIVE;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);
    u32 end = timer_calc(AHCI_LINK_TIMEOUT);
    க்கு (;;) {
        stat = ahci_port_readl(ctrl, pnr, PORT_SCR_STAT);
        எனில் ((stat & 0x07) == 0x03) {
            dprintf(2, "AHCI/%d: link up\n", port->pnr);
            நிறுத்து;
        }
        எனில் (timer_check(end)) {
            dprintf(2, "AHCI/%d: link down\n", port->pnr);
            திரும்பு -1;
        }
        yield();
    }

    /* clear error status */
    err = ahci_port_readl(ctrl, pnr, PORT_SCR_ERR);
    எனில் (err)
        ahci_port_writel(ctrl, pnr, PORT_SCR_ERR, err);

    /* wait for device becoming ready */
    end = timer_calc(AHCI_REQUEST_TIMEOUT);
    க்கு (;;) {
        tf = ahci_port_readl(ctrl, pnr, PORT_TFDATA);
        எனில் (!(tf & (ATA_CB_STAT_BSY |
                    ATA_CB_STAT_DRQ)))
            நிறுத்து;
        எனில் (timer_check(end)) {
            warn_timeout();
            dprintf(1, "AHCI/%d: device not ready (tf 0x%x)\n", port->pnr, tf);
            திரும்பு -1;
        }
        yield();
    }

    /* start device */
    cmd |= PORT_CMD_START;
    ahci_port_writel(ctrl, pnr, PORT_CMD, cmd);

    sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_PACKET_DEVICE);
    rc = ahci_command(port, 0, 0, buffer, அளவு(buffer));
    எனில் (rc == 0) {
        port->atapi = 1;
    } இல்லையெனில் {
        port->atapi = 0;
        sata_prep_simple(&port->cmd->fis, ATA_CMD_IDENTIFY_DEVICE);
        rc = ahci_command(port, 0, 0, buffer, அளவு(buffer));
        எனில் (rc < 0)
            திரும்பு -1;
    }

    port->drive.cntl_id = pnr;
    port->drive.removable = (buffer[0] & 0x80) ? 1 : 0;

    எனில் (!port->atapi) {
        // found disk (ata)
        port->drive.type = DTYPE_AHCI;
        port->drive.blksize = DISK_SECTOR_SIZE;
        port->drive.pchs.cylinder = buffer[1];
        port->drive.pchs.head = buffer[3];
        port->drive.pchs.sector = buffer[6];

        u64 sectors;
        எனில் (buffer[83] & (1 << 10)) // word 83 - lba48 support
            sectors = *(u64*)&buffer[100]; // word 100-103
        இல்லையெனில்
            sectors = *(u32*)&buffer[60]; // word 60 and word 61
        port->drive.sectors = sectors;
        u64 adjsize = sectors >> 11;
        எழுத்து adjprefix = 'M';
        எனில் (adjsize >= (1 << 16)) {
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
        எனில் (buffer[53] & 0x04) {
            udma_mode = 6;
            போது ((udma_mode >= 0) &&
                   !((buffer[88] & 0x7f) & ( 1 << udma_mode ))) {
                udma_mode--;
            }
        }
        // If bit 1 in word 53 is set, multiword-dma and advanced pio modes
        // are available in words 63 and 64.
        எனில் (buffer[53] & 0x02) {
            pio_mode = 4;
            multi_dma = 3;
            போது ((multi_dma >= 0) &&
                   !((buffer[63] & 0x7) & ( 1 << multi_dma ))) {
                multi_dma--;
            }
            போது ((pio_mode >= 3) &&
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
        எனில் (udma_mode >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to UDMA-%d\n",
                    port->pnr, udma_mode);
            port->cmd->fis.sector_மொத்தம் = ATA_TRANSFER_MODE_ULTRA_DMA
                                          | udma_mode;
        } இல்லையெனில் எனில் (multi_dma >= 0) {
            dprintf(1, "AHCI/%d: Set transfer mode to Multi-DMA-%d\n",
                    port->pnr, multi_dma);
            port->cmd->fis.sector_மொத்தம் = ATA_TRANSFER_MODE_MULTIWORD_DMA
                                          | multi_dma;
        } இல்லையெனில் எனில் (pio_mode >= 3) {
            dprintf(1, "AHCI/%d: Set transfer mode to PIO-%d\n",
                    port->pnr, pio_mode);
            port->cmd->fis.sector_மொத்தம் = ATA_TRANSFER_MODE_PIO_FLOW_CTRL
                                          | pio_mode;
        } இல்லையெனில் {
            dprintf(1, "AHCI/%d: Set transfer mode to default PIO\n",
                    port->pnr);
            port->cmd->fis.sector_மொத்தம் = ATA_TRANSFER_MODE_DEFAULT_PIO;
        }
        rc = ahci_command(port, 1, 0, 0, 0);
        எனில் (rc < 0) {
            dprintf(1, "AHCI/%d: Set transfer mode failed.\n", port->pnr);
        }
    } இல்லையெனில் {
        // found cdrom (atapi)
        port->drive.type = DTYPE_AHCI_ATAPI;
        port->drive.blksize = CDROM_SECTOR_SIZE;
        port->drive.sectors = (u64)-1;
        u8 iscd = ((buffer[0] >> 8) & 0x1f) == 0x05;
        எனில் (!iscd) {
            dprintf(1, "AHCI/%d: atapi device isn't a cdrom\n", port->pnr);
            திரும்பு -1;
        }
        port->desc = znprintf(MAXDESCSIZE
                              , "DVD/CD [AHCI/%d: %s ATAPI-%d DVD/CD]"
                              , port->pnr
                              , ata_extract_model(model, MAXMODEL, buffer)
                              , ata_extract_version(buffer));
        port->prio = bootprio_find_ata_device(ctrl->pci_tmp, pnr, 0);
    }
    boot_lchs_find_ata_device(ctrl->pci_tmp, pnr, 0, &(port->drive.lchs));
    திரும்பு 0;
}

// Detect any drives attached to a given controller.
நிலையான வெறுமை
ahci_port_detect(வெறுமை *data)
{
    கட்டமைப்பு ahci_port_s *port = data;
    முழு rc;

    dprintf(2, "AHCI/%d: probing\n", port->pnr);
    ahci_port_reset(port->ctrl, port->pnr);
    rc = ahci_port_setup(port);
    எனில் (rc < 0)
        ahci_port_release(port);
    இல்லையெனில் {
        port = ahci_port_realloc(port);
        எனில் (port == வெற்று)
            திரும்பு;
        dprintf(1, "AHCI/%d: registering: \"%s\"\n", port->pnr, port->desc);
        எனில் (!port->atapi) {
            // Register with bcv system.
            boot_add_hd(&port->drive, port->desc, port->prio);
        } இல்லையெனில் {
            // fill cdidmap
            boot_add_cd(&port->drive, port->desc, port->prio);
        }
    }
}

// Initialize an ata controller and detect its drives.
நிலையான வெறுமை
ahci_controller_setup(கட்டமைப்பு pci_device *pci)
{
    கட்டமைப்பு ahci_port_s *port;
    u32 pnr, max;

    எனில் (create_bounce_buf() < 0)
        திரும்பு;

    வெறுமை *iobase = pci_enable_membar(pci, PCI_BASE_ADDRESS_5);
    எனில் (!iobase)
        திரும்பு;

    கட்டமைப்பு ahci_ctrl_s *ctrl = malloc_fseg(அளவு(*ctrl));
    எனில் (!ctrl) {
        warn_noalloc();
        திரும்பு;
    }

    ctrl->pci_tmp = pci;
    ctrl->iobase = iobase;
    ctrl->irq = pci_config_readb(pci->bdf, PCI_INTERRUPT_வரி);
    dprintf(1, "AHCI controller at %pP, iobase %p, irq %d\n"
            , pci, ctrl->iobase, ctrl->irq);

    pci_enable_busmaster(pci);

    u32 val = ahci_ctrl_readl(ctrl, HOST_CTL);
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_RESET);
    u32 end = timer_calc(AHCI_RESET_TIMEOUT);
    க்கு (;;) {
        val = ahci_ctrl_readl(ctrl, HOST_CTL);
        எனில் (!(val & HOST_CTL_RESET))
            நிறுத்து;
        எனில் (timer_check(end)) {
            warn_timeout();
            dprintf(1, "AHCI: controller reset failed\n");
            விடுவி(ctrl);
            திரும்பு;
        }
        yield();
    }
    ahci_ctrl_writel(ctrl, HOST_CTL, val | HOST_CTL_AHCI_EN);
    (வெறுமை)ahci_ctrl_readl(ctrl, HOST_CTL); /* Flush */

    ctrl->caps = ahci_ctrl_readl(ctrl, HOST_CAP);
    ctrl->ports = ahci_ctrl_readl(ctrl, HOST_PORTS_IMPL);
    dprintf(2, "AHCI: cap 0x%x, ports_impl 0x%x\n",
            ctrl->caps, ctrl->ports);

    max = 0x1f;
    க்கு (pnr = 0; pnr <= max; pnr++) {
        எனில் (!(ctrl->ports & (1 << pnr)))
            தொடர்;
        port = ahci_port_alloc(ctrl, pnr);
        எனில் (port == வெற்று)
            தொடர்;
        run_thread(ahci_port_detect, port);
    }
}

// Locate and init ahci controllers.
நிலையான வெறுமை
ahci_scan(வெறுமை)
{
    // Scan PCI bus for ATA adapters
    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        எனில் (pci->class != PCI_CLASS_STORAGE_SATA)
            தொடர்;
        எனில் (pci->prog_if != 1 /* AHCI rev 1 */)
            தொடர்;
        ahci_controller_setup(pci);
    }
}

வெறுமை
ahci_setup(வெறுமை)
{
    ASSERT32FLAT();
    எனில் (!CONFIG_AHCI)
        திரும்பு;

    dprintf(3, "init ahci\n");
    ahci_scan();
}

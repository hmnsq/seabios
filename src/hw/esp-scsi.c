// AMD PCscsi boot support.
//
// Copyright (C) 2012 Red Hat Inc.
//
// Authors:
//  Paolo Bonzini <pbonzini@redhat.com>
//
// based on lsi-scsi.c which is written by:
//  Gerd Hoffman <kraxel@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_GLOBALFLAT
#అన్తర్భావ "block.h" // struct drive_s
#అన్తర్భావ "blockcmd.h" // scsi_drive_setup
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "fw/paravirt.h" // runningOnQEMU
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pcidevice.h" // foreachpci
#అన్తర్భావ "pci_ids.h" // PCI_DEVICE_ID
#అన్తర్భావ "pci_regs.h" // PCI_VENDOR_ID
#అన్తర్భావ "stacks.h" // run_thread
#అన్తర్భావ "std/disk.h" // DISK_RET_SUCCESS
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // usleep

#నిర్వచించు ESP_TCLO      0x00
#నిర్వచించు ESP_TCMID     0x04
#నిర్వచించు ESP_FIFO      0x08
#నిర్వచించు ESP_CMD       0x0c
#నిర్వచించు ESP_WBUSID    0x10
#నిర్వచించు ESP_TCHI      0x38

#నిర్వచించు ESP_RSTAT     0x10
#నిర్వచించు ESP_RINTR     0x14
#నిర్వచించు ESP_RFLAGS    0x1c

#నిర్వచించు ESP_DMA_CMD   0x40
#నిర్వచించు ESP_DMA_STC   0x44
#నిర్వచించు ESP_DMA_SPA   0x48
#నిర్వచించు ESP_DMA_WBC   0x4c
#నిర్వచించు ESP_DMA_WAC   0x50
#నిర్వచించు ESP_DMA_STAT  0x54
#నిర్వచించు ESP_DMA_SMDLA 0x58
#నిర్వచించు ESP_DMA_WMAC  0x58c

#నిర్వచించు ESP_CMD_DMA      0x80
#నిర్వచించు ESP_CMD_FLUSH    0x01
#నిర్వచించు ESP_CMD_RESET    0x02
#నిర్వచించు ESP_CMD_TI       0x10
#నిర్వచించు ESP_CMD_ICCS     0x11
#నిర్వచించు ESP_CMD_MSGACC   0x12
#నిర్వచించు ESP_CMD_SELATN   0x42

#నిర్వచించు ESP_STAT_DI      0x01
#నిర్వచించు ESP_STAT_CD      0x02
#నిర్వచించు ESP_STAT_సందేశం     0x04
#నిర్వచించు ESP_STAT_TC      0x10

#నిర్వచించు ESP_INTR_FC      0x08
#నిర్వచించు ESP_INTR_BS      0x10
#నిర్వచించు ESP_INTR_DC      0x20

నిర్మాణం esp_lun_s {
    నిర్మాణం drive_s drive;
    నిర్మాణం pci_device *pci;
    u32 iobase;
    u8 లక్ష్యం;
    u8 lun;
};

స్థిర శూన్య
esp_scsi_dma(u32 iobase, u32 buf, u32 len, పూర్ణాంకం పఠ)
{
    outb(len         & 0xff, iobase + ESP_TCLO);
    outb((len >> 8)  & 0xff, iobase + ESP_TCMID);
    outb((len >> 16) & 0xff, iobase + ESP_TCHI);
    outl(buf,                iobase + ESP_DMA_SPA);
    outl(len,                iobase + ESP_DMA_STC);
    outb(పఠ ? 0x83 : 0x03, iobase + ESP_DMA_CMD);
}

పూర్ణాంకం
esp_scsi_process_op(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (!CONFIG_ESP_SCSI)
        తిరిగి DISK_RET_EBADTRACK;
    నిర్మాణం esp_lun_s *llun_gf =
        container_యొక్క(op->drive_fl, నిర్మాణం esp_lun_s, drive);
    u16 లక్ష్యం = GET_GLOBALFLAT(llun_gf->లక్ష్యం);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u8 cdbcmd[16];
    పూర్ణాంకం blocksize = scsi_fill_cmd(op, cdbcmd, పరిమాణం(cdbcmd));
    ఒకవేళ (blocksize < 0)
        తిరిగి default_process_op(op);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    పూర్ణాంకం i, state;
    u8 స్థితి_2;

    outb(లక్ష్యం, iobase + ESP_WBUSID);

    /* Clear FIFO and interrupts before sending command.  */
    outb(ESP_CMD_FLUSH, iobase + ESP_CMD);
    inb(iobase + ESP_RINTR);

    /*
     * We need to pass the LUN at the beginning of the command, and the FIFO
     * is only 16 bytes, so we cannot support 16-byte CDBs.  The alternative
     * would be to use DMA for the 17-byte command too, which is quite
     * overkill.
     */
    outb(lun, iobase + ESP_FIFO);
    cdbcmd[1] &= 0x1f;
    cdbcmd[1] |= lun << 5;
    కొరకు (i = 0; i < 12; i++)
        outb(cdbcmd[i], iobase + ESP_FIFO);
    outb(ESP_CMD_SELATN, iobase + ESP_CMD);

    కొరకు (state = 0;;) {
        u8 stat = inb(iobase + ESP_RSTAT);
        u8 intr;

        ఒకవేళ (state == 0) {
            intr = inb(iobase + ESP_RINTR);

            /* Detect disconnected device.  */
            ఒకవేళ (intr & ESP_INTR_DC) {
                తిరిగి DISK_RET_ENOTREADY;
            }

            /* HBA reads command, executes it, sets BS/FC -> do DMA if needed.  */
            ఒకవేళ (intr & (ESP_INTR_BS | ESP_INTR_FC)) {
                state++;
                ఒకవేళ (op->లెక్క && blocksize) {
                    /* Data phase.  */
                    u32 లెక్క = (u32)op->లెక్క * blocksize;
                    esp_scsi_dma(iobase, (u32)op->buf_fl, లెక్క, scsi_is_read(op));
                    outb(ESP_CMD_TI | ESP_CMD_DMA, iobase + ESP_CMD);
                    కొనసాగు;
                } లేకపోతే {
                    /* No data phase.  */
                    state++;
                }
            }
        }

        /* At end of DMA TC is set again -> complete command.  */
        ఒకవేళ (state == 1 && (stat & ESP_STAT_TC)) {
            state++;
            /* Terminate esp_scsi_dma() command */
            outb(0, iobase + ESP_DMA_CMD);
            కొనసాగు;
        }

        /* Request message in data.  */
        ఒకవేళ (state == 2) {
            state++;
            outb(ESP_CMD_ICCS, iobase + ESP_CMD);
            కొనసాగు;
        }

        /* Finally read data from the message in phase and accept.  */
        ఒకవేళ (state == 3 && (stat & ESP_STAT_సందేశం)) {
            state++;
            స్థితి_2 = inb(iobase + ESP_FIFO);
            inb(iobase + ESP_FIFO);
            outb(ESP_CMD_MSGACC, iobase + ESP_CMD);
            ఆపు;
        }
        usleep(5);
    }

    ఒకవేళ (స్థితి_2 == 0) {
        తిరిగి DISK_RET_SUCCESS;
    }

    తిరిగి DISK_RET_EBADTRACK;
}

స్థిర శూన్య
esp_scsi_init_lun(నిర్మాణం esp_lun_s *llun, నిర్మాణం pci_device *pci, u32 iobase,
                  u8 లక్ష్యం, u8 lun)
{
    memset(llun, 0, పరిమాణం(*llun));
    llun->drive.type = DTYPE_ESP_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->లక్ష్యం = లక్ష్యం;
    llun->lun = lun;
    llun->iobase = iobase;
}

స్థిర పూర్ణాంకం
esp_scsi_కూడు_lun(u32 lun, నిర్మాణం drive_s *tmpl_drv)
{
    నిర్మాణం esp_lun_s *tmpl_llun =
        container_యొక్క(tmpl_drv, నిర్మాణం esp_lun_s, drive);
    నిర్మాణం esp_lun_s *llun = malloc_fseg(పరిమాణం(*llun));
    ఒకవేళ (!llun) {
        warn_noalloc();
        తిరిగి -1;
    }
    esp_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->లక్ష్యం, lun);

    అక్షరం *name = znprintf(MAXDESCSIZE, "esp %pP %d:%d",
                          llun->pci, llun->లక్ష్యం, llun->lun);
    boot_lchs_కనుగొను_scsi_device(llun->pci, llun->లక్ష్యం, llun->lun,
                               &(llun->drive.lchs));
    పూర్ణాంకం prio = bootprio_కనుగొను_scsi_device(llun->pci, llun->లక్ష్యం, llun->lun);
    పూర్ణాంకం ret = scsi_drive_అమరిక(&llun->drive, name, prio);
    విడుదల(name);
    ఒకవేళ (ret)
        వెళ్ళు fail;
    తిరిగి 0;

fail:
    విడుదల(llun);
    తిరిగి -1;
}

స్థిర శూన్య
esp_scsi_పరిశీలన_లక్ష్యం(నిర్మాణం pci_device *pci, u32 iobase, u8 లక్ష్యం)
{
    నిర్మాణం esp_lun_s llun0;

    esp_scsi_init_lun(&llun0, pci, iobase, లక్ష్యం, 0);

    scsi_rep_luns_పరిశీలన(&llun0.drive, esp_scsi_కూడు_lun);
}

స్థిర శూన్య
init_esp_scsi(శూన్య *దత్తాంశం)
{
    నిర్మాణం pci_device *pci = దత్తాంశం;
    u32 iobase = pci_enable_iobar(pci, PCI_పునాది_చిరునామా_0);
    ఒకవేళ (!iobase)
        తిరిగి;
    pci_enable_busmaster(pci);

    dprintf(1, "found esp at %pP, io @ %x\n", pci, iobase);

    // reset
    outb(ESP_CMD_RESET, iobase + ESP_CMD);

    పూర్ణాంకం i;
    కొరకు (i = 0; i <= 7; i++)
        esp_scsi_పరిశీలన_లక్ష్యం(pci, iobase, i);
}

శూన్య
esp_scsi_అమరిక(శూన్య)
{
    ASSERT32FLAT();
    ఒకవేళ (!CONFIG_ESP_SCSI || !runningOnQEMU())
        తిరిగి;

    dprintf(3, "init esp\n");

    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (pci->vendor != PCI_VENDOR_ID_AMD
            || pci->device != PCI_DEVICE_ID_AMD_SCSI)
            కొనసాగు;
        నిరంతరం_thread(init_esp_scsi, pci);
    }
}

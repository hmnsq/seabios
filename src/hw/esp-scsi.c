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

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_GLOBALFLAT
#ଅନ୍ତର୍ଭାଵ "block.h" // struct drive_s
#ଅନ୍ତର୍ଭାଵ "blockcmd.h" // scsi_drive_setup
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "fw/paravirt.h" // runningOnQEMU
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "pcidevice.h" // foreachpci
#ଅନ୍ତର୍ଭାଵ "pci_ids.h" // PCI_DEVICE_ID
#ଅନ୍ତର୍ଭାଵ "pci_regs.h" // PCI_VENDOR_ID
#ଅନ୍ତର୍ଭାଵ "stacks.h" // run_thread
#ଅନ୍ତର୍ଭାଵ "std/disk.h" // DISK_RET_SUCCESS
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // usleep

#define ESP_TCLO      0x00
#define ESP_TCMID     0x04
#define ESP_FIFO      0x08
#define ESP_CMD       0x0c
#define ESP_WBUSID    0x10
#define ESP_TCHI      0x38

#define ESP_RSTAT     0x10
#define ESP_RINTR     0x14
#define ESP_RFLAGS    0x1c

#define ESP_DMA_CMD   0x40
#define ESP_DMA_STC   0x44
#define ESP_DMA_SPA   0x48
#define ESP_DMA_WBC   0x4c
#define ESP_DMA_WAC   0x50
#define ESP_DMA_STAT  0x54
#define ESP_DMA_SMDLA 0x58
#define ESP_DMA_WMAC  0x58c

#define ESP_CMD_DMA      0x80
#define ESP_CMD_FLUSH    0x01
#define ESP_CMD_RESET    0x02
#define ESP_CMD_TI       0x10
#define ESP_CMD_ICCS     0x11
#define ESP_CMD_MSGACC   0x12
#define ESP_CMD_SELATN   0x42

#define ESP_STAT_DI      0x01
#define ESP_STAT_CD      0x02
#define ESP_STAT_ସନ୍ଦେଶ     0x04
#define ESP_STAT_TC      0x10

#define ESP_INTR_FC      0x08
#define ESP_INTR_BS      0x10
#define ESP_INTR_DC      0x20

ସଂରଚନା esp_lun_s {
    ସଂରଚନା drive_s drive;
    ସଂରଚନା pci_device *pci;
    u32 iobase;
    u8 target;
    u8 lun;
};

ସ୍ଥିର ଶୂନ୍ୟ
esp_scsi_dma(u32 iobase, u32 buf, u32 len, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଠ)
{
    outb(len         & 0xff, iobase + ESP_TCLO);
    outb((len >> 8)  & 0xff, iobase + ESP_TCMID);
    outb((len >> 16) & 0xff, iobase + ESP_TCHI);
    outl(buf,                iobase + ESP_DMA_SPA);
    outl(len,                iobase + ESP_DMA_STC);
    outb(ପଠ ? 0x83 : 0x03, iobase + ESP_DMA_CMD);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
esp_scsi_process_op(ସଂରଚନା disk_op_s *op)
{
    ଯଦି (!CONFIG_ESP_SCSI)
        ଫେରନ୍ତୁ DISK_RET_EBADTRACK;
    ସଂରଚନା esp_lun_s *llun_gf =
        container_of(op->drive_fl, ସଂରଚନା esp_lun_s, drive);
    u16 target = GET_GLOBALFLAT(llun_gf->target);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u8 cdbcmd[16];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା blocksize = scsi_fill_cmd(op, cdbcmd, ଆକାର(cdbcmd));
    ଯଦି (blocksize < 0)
        ଫେରନ୍ତୁ default_process_op(op);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, state;
    u8 status;

    outb(target, iobase + ESP_WBUSID);

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
    ପାଇଁ (i = 0; i < 12; i++)
        outb(cdbcmd[i], iobase + ESP_FIFO);
    outb(ESP_CMD_SELATN, iobase + ESP_CMD);

    ପାଇଁ (state = 0;;) {
        u8 stat = inb(iobase + ESP_RSTAT);
        u8 intr;

        ଯଦି (state == 0) {
            intr = inb(iobase + ESP_RINTR);

            /* Detect disconnected device.  */
            ଯଦି (intr & ESP_INTR_DC) {
                ଫେରନ୍ତୁ DISK_RET_ENOTREADY;
            }

            /* HBA reads command, executes it, sets BS/FC -> do DMA if needed.  */
            ଯଦି (intr & (ESP_INTR_BS | ESP_INTR_FC)) {
                state++;
                ଯଦି (op->ଗଣନା_2 && blocksize) {
                    /* Data phase.  */
                    u32 ଗଣନା_2 = (u32)op->ଗଣନା_2 * blocksize;
                    esp_scsi_dma(iobase, (u32)op->buf_fl, ଗଣନା_2, scsi_is_read(op));
                    outb(ESP_CMD_TI | ESP_CMD_DMA, iobase + ESP_CMD);
                    ଜାରି;
                } ଅନ୍ୟଥା {
                    /* No data phase.  */
                    state++;
                }
            }
        }

        /* At end of DMA TC is set again -> complete command.  */
        ଯଦି (state == 1 && (stat & ESP_STAT_TC)) {
            state++;
            /* Terminate esp_scsi_dma() command */
            outb(0, iobase + ESP_DMA_CMD);
            ଜାରି;
        }

        /* Request message in data.  */
        ଯଦି (state == 2) {
            state++;
            outb(ESP_CMD_ICCS, iobase + ESP_CMD);
            ଜାରି;
        }

        /* Finally read data from the message in phase and accept.  */
        ଯଦି (state == 3 && (stat & ESP_STAT_ସନ୍ଦେଶ)) {
            state++;
            status = inb(iobase + ESP_FIFO);
            inb(iobase + ESP_FIFO);
            outb(ESP_CMD_MSGACC, iobase + ESP_CMD);
            ଭାଙ୍ଗ;
        }
        usleep(5);
    }

    ଯଦି (status == 0) {
        ଫେରନ୍ତୁ DISK_RET_SUCCESS;
    }

    ଫେରନ୍ତୁ DISK_RET_EBADTRACK;
}

ସ୍ଥିର ଶୂନ୍ୟ
esp_scsi_init_lun(ସଂରଚନା esp_lun_s *llun, ସଂରଚନା pci_device *pci, u32 iobase,
                  u8 target, u8 lun)
{
    memset(llun, 0, ଆକାର(*llun));
    llun->drive.type = DTYPE_ESP_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->target = target;
    llun->lun = lun;
    llun->iobase = iobase;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
esp_scsi_add_lun(u32 lun, ସଂରଚନା drive_s *tmpl_drv)
{
    ସଂରଚନା esp_lun_s *tmpl_llun =
        container_of(tmpl_drv, ସଂରଚନା esp_lun_s, drive);
    ସଂରଚନା esp_lun_s *llun = malloc_fseg(ଆକାର(*llun));
    ଯଦି (!llun) {
        warn_noalloc();
        ଫେରନ୍ତୁ -1;
    }
    esp_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->target, lun);

    ଅକ୍ଷର *name = znprintf(MAXDESCSIZE, "esp %pP %d:%d",
                          llun->pci, llun->target, llun->lun);
    boot_lchs_find_scsi_device(llun->pci, llun->target, llun->lun,
                               &(llun->drive.lchs));
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio = bootprio_find_scsi_device(llun->pci, llun->target, llun->lun);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = scsi_drive_setup(&llun->drive, name, prio);
    ମୁକ୍ତ(name);
    ଯଦି (ret)
        ଯାଅ fail;
    ଫେରନ୍ତୁ 0;

fail:
    ମୁକ୍ତ(llun);
    ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ଶୂନ୍ୟ
esp_scsi_scan_target(ସଂରଚନା pci_device *pci, u32 iobase, u8 target)
{
    ସଂରଚନା esp_lun_s llun0;

    esp_scsi_init_lun(&llun0, pci, iobase, target, 0);

    scsi_rep_luns_scan(&llun0.drive, esp_scsi_add_lun);
}

ସ୍ଥିର ଶୂନ୍ୟ
init_esp_scsi(ଶୂନ୍ୟ *data)
{
    ସଂରଚନା pci_device *pci = data;
    u32 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
    ଯଦି (!iobase)
        ଫେରନ୍ତୁ;
    pci_enable_busmaster(pci);

    dprintf(1, "found esp at %pP, io @ %x\n", pci, iobase);

    // reset
    outb(ESP_CMD_RESET, iobase + ESP_CMD);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i = 0; i <= 7; i++)
        esp_scsi_scan_target(pci, iobase, i);
}

ଶୂନ୍ୟ
esp_scsi_setup(ଶୂନ୍ୟ)
{
    ASSERT32FLAT();
    ଯଦି (!CONFIG_ESP_SCSI || !runningOnQEMU())
        ଫେରନ୍ତୁ;

    dprintf(3, "init esp\n");

    ସଂରଚନା pci_device *pci;
    foreachpci(pci) {
        ଯଦି (pci->vendor != PCI_VENDOR_ID_AMD
            || pci->device != PCI_DEVICE_ID_AMD_SCSI)
            ଜାରି;
        run_thread(init_esp_scsi, pci);
    }
}

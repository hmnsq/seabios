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

#অন্তর্ভাব "biosvar.h" // GET_GLOBALFLAT
#অন্তর্ভাব "block.h" // struct drive_s
#অন্তর্ভাব "blockcmd.h" // scsi_drive_setup
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "fw/paravirt.h" // runningOnQEMU
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "pcidevice.h" // foreachpci
#অন্তর্ভাব "pci_ids.h" // PCI_DEVICE_ID
#অন্তর্ভাব "pci_regs.h" // PCI_VENDOR_ID
#অন্তর্ভাব "stacks.h" // run_thread
#অন্তর্ভাব "std/disk.h" // DISK_RET_SUCCESS
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // usleep

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
#define ESP_STAT_বার্তা     0x04
#define ESP_STAT_TC      0x10

#define ESP_INTR_FC      0x08
#define ESP_INTR_BS      0x10
#define ESP_INTR_DC      0x20

কাঠামো esp_lun_s {
    কাঠামো drive_s drive;
    কাঠামো pci_device *pci;
    u32 iobase;
    u8 target;
    u8 lun;
};

স্থির শূন্য
esp_scsi_dma(u32 iobase, u32 buf, u32 len, পূর্ণসংখ্যা পঠ)
{
    outb(len         & 0xff, iobase + ESP_TCLO);
    outb((len >> 8)  & 0xff, iobase + ESP_TCMID);
    outb((len >> 16) & 0xff, iobase + ESP_TCHI);
    outl(buf,                iobase + ESP_DMA_SPA);
    outl(len,                iobase + ESP_DMA_STC);
    outb(পঠ ? 0x83 : 0x03, iobase + ESP_DMA_CMD);
}

পূর্ণসংখ্যা
esp_scsi_process_op(কাঠামো disk_op_s *op)
{
    যদি (!CONFIG_ESP_SCSI)
        ফেরত DISK_RET_EBADTRACK;
    কাঠামো esp_lun_s *llun_gf =
        container_of(op->drive_fl, কাঠামো esp_lun_s, drive);
    u16 target = GET_GLOBALFLAT(llun_gf->target);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u8 cdbcmd[16];
    পূর্ণসংখ্যা blocksize = scsi_fill_cmd(op, cdbcmd, আকার(cdbcmd));
    যদি (blocksize < 0)
        ফেরত default_process_op(op);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    পূর্ণসংখ্যা i, state;
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
    জন্য (i = 0; i < 12; i++)
        outb(cdbcmd[i], iobase + ESP_FIFO);
    outb(ESP_CMD_SELATN, iobase + ESP_CMD);

    জন্য (state = 0;;) {
        u8 stat = inb(iobase + ESP_RSTAT);
        u8 intr;

        যদি (state == 0) {
            intr = inb(iobase + ESP_RINTR);

            /* Detect disconnected device.  */
            যদি (intr & ESP_INTR_DC) {
                ফেরত DISK_RET_ENOTREADY;
            }

            /* HBA reads command, executes it, sets BS/FC -> do DMA if needed.  */
            যদি (intr & (ESP_INTR_BS | ESP_INTR_FC)) {
                state++;
                যদি (op->গুনতি && blocksize) {
                    /* Data phase.  */
                    u32 গুনতি = (u32)op->গুনতি * blocksize;
                    esp_scsi_dma(iobase, (u32)op->buf_fl, গুনতি, scsi_is_read(op));
                    outb(ESP_CMD_TI | ESP_CMD_DMA, iobase + ESP_CMD);
                    চালু;
                } অন্যথা {
                    /* No data phase.  */
                    state++;
                }
            }
        }

        /* At end of DMA TC is set again -> complete command.  */
        যদি (state == 1 && (stat & ESP_STAT_TC)) {
            state++;
            /* Terminate esp_scsi_dma() command */
            outb(0, iobase + ESP_DMA_CMD);
            চালু;
        }

        /* Request message in data.  */
        যদি (state == 2) {
            state++;
            outb(ESP_CMD_ICCS, iobase + ESP_CMD);
            চালু;
        }

        /* Finally read data from the message in phase and accept.  */
        যদি (state == 3 && (stat & ESP_STAT_বার্তা)) {
            state++;
            status = inb(iobase + ESP_FIFO);
            inb(iobase + ESP_FIFO);
            outb(ESP_CMD_MSGACC, iobase + ESP_CMD);
            বিরতি;
        }
        usleep(5);
    }

    যদি (status == 0) {
        ফেরত DISK_RET_SUCCESS;
    }

    ফেরত DISK_RET_EBADTRACK;
}

স্থির শূন্য
esp_scsi_init_lun(কাঠামো esp_lun_s *llun, কাঠামো pci_device *pci, u32 iobase,
                  u8 target, u8 lun)
{
    memset(llun, 0, আকার(*llun));
    llun->drive.type = DTYPE_ESP_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->target = target;
    llun->lun = lun;
    llun->iobase = iobase;
}

স্থির পূর্ণসংখ্যা
esp_scsi_add_lun(u32 lun, কাঠামো drive_s *tmpl_drv)
{
    কাঠামো esp_lun_s *tmpl_llun =
        container_of(tmpl_drv, কাঠামো esp_lun_s, drive);
    কাঠামো esp_lun_s *llun = malloc_fseg(আকার(*llun));
    যদি (!llun) {
        warn_noalloc();
        ফেরত -1;
    }
    esp_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->target, lun);

    অক্ষর *name = znprintf(MAXDESCSIZE, "esp %pP %d:%d",
                          llun->pci, llun->target, llun->lun);
    boot_lchs_find_scsi_device(llun->pci, llun->target, llun->lun,
                               &(llun->drive.lchs));
    পূর্ণসংখ্যা prio = bootprio_find_scsi_device(llun->pci, llun->target, llun->lun);
    পূর্ণসংখ্যা ret = scsi_drive_setup(&llun->drive, name, prio);
    মুক্ত(name);
    যদি (ret)
        যাও fail;
    ফেরত 0;

fail:
    মুক্ত(llun);
    ফেরত -1;
}

স্থির শূন্য
esp_scsi_scan_target(কাঠামো pci_device *pci, u32 iobase, u8 target)
{
    কাঠামো esp_lun_s llun0;

    esp_scsi_init_lun(&llun0, pci, iobase, target, 0);

    scsi_rep_luns_scan(&llun0.drive, esp_scsi_add_lun);
}

স্থির শূন্য
init_esp_scsi(শূন্য *তথ্য)
{
    কাঠামো pci_device *pci = তথ্য;
    u32 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
    যদি (!iobase)
        ফেরত;
    pci_enable_busmaster(pci);

    dprintf(1, "found esp at %pP, io @ %x\n", pci, iobase);

    // reset
    outb(ESP_CMD_RESET, iobase + ESP_CMD);

    পূর্ণসংখ্যা i;
    জন্য (i = 0; i <= 7; i++)
        esp_scsi_scan_target(pci, iobase, i);
}

শূন্য
esp_scsi_setup(শূন্য)
{
    ASSERT32FLAT();
    যদি (!CONFIG_ESP_SCSI || !runningOnQEMU())
        ফেরত;

    dprintf(3, "init esp\n");

    কাঠামো pci_device *pci;
    foreachpci(pci) {
        যদি (pci->vendor != PCI_VENDOR_ID_AMD
            || pci->device != PCI_DEVICE_ID_AMD_SCSI)
            চালু;
        run_thread(init_esp_scsi, pci);
    }
}

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

#ضمن "biosvar.h" // GET_GLOBALFLAT
#ضمن "block.h" // struct drive_s
#ضمن "blockcmd.h" // scsi_drive_setup
#ضمن "config.h" // CONFIG_*
#ضمن "fw/paravirt.h" // runningOnQEMU
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "pcidevice.h" // foreachpci
#ضمن "pci_ids.h" // PCI_DEVICE_ID
#ضمن "pci_regs.h" // PCI_VENDOR_ID
#ضمن "stacks.h" // run_thread
#ضمن "std/disk.h" // DISK_RET_SUCCESS
#ضمن "string.h" // memset
#ضمن "util.h" // usleep

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
#define ESP_STAT_ڤسنن     0x04
#define ESP_STAT_TC      0x10

#define ESP_INTR_FC      0x08
#define ESP_INTR_BS      0x10
#define ESP_INTR_DC      0x20

بنية esp_lun_s {
    بنية drive_s drive;
    بنية pci_device *pci;
    u32 iobase;
    u8 target;
    u8 lun;
};

ساكن فراغ
esp_scsi_dma(u32 iobase, u32 buf, u32 len, صحيح اقرأ)
{
    outb(len         & 0xff, iobase + ESP_TCLO);
    outb((len >> 8)  & 0xff, iobase + ESP_TCMID);
    outb((len >> 16) & 0xff, iobase + ESP_TCHI);
    outl(buf,                iobase + ESP_DMA_SPA);
    outl(len,                iobase + ESP_DMA_STC);
    outb(اقرأ ? 0x83 : 0x03, iobase + ESP_DMA_CMD);
}

صحيح
esp_scsi_process_op(بنية disk_op_s *op)
{
    إذا (!CONFIG_ESP_SCSI)
        أرجع DISK_RET_EBADTRACK;
    بنية esp_lun_s *llun_gf =
        container_of(op->drive_fl, بنية esp_lun_s, drive);
    u16 target = GET_GLOBALFLAT(llun_gf->target);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u8 cdbcmd[16];
    صحيح blocksize = scsi_fill_cmd(op, cdbcmd, حجم(cdbcmd));
    إذا (blocksize < 0)
        أرجع default_process_op(op);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    صحيح i, state;
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
    لكل (i = 0; i < 12; i++)
        outb(cdbcmd[i], iobase + ESP_FIFO);
    outb(ESP_CMD_SELATN, iobase + ESP_CMD);

    لكل (state = 0;;) {
        u8 stat = inb(iobase + ESP_RSTAT);
        u8 intr;

        إذا (state == 0) {
            intr = inb(iobase + ESP_RINTR);

            /* Detect disconnected device.  */
            إذا (intr & ESP_INTR_DC) {
                أرجع DISK_RET_ENOTREADY;
            }

            /* HBA reads command, executes it, sets BS/FC -> do DMA if needed.  */
            إذا (intr & (ESP_INTR_BS | ESP_INTR_FC)) {
                state++;
                إذا (op->کيراءن && blocksize) {
                    /* Data phase.  */
                    u32 کيراءن = (u32)op->کيراءن * blocksize;
                    esp_scsi_dma(iobase, (u32)op->buf_fl, کيراءن, scsi_is_read(op));
                    outb(ESP_CMD_TI | ESP_CMD_DMA, iobase + ESP_CMD);
                    استمر;
                } وإلا {
                    /* No data phase.  */
                    state++;
                }
            }
        }

        /* At end of DMA TC is set again -> complete command.  */
        إذا (state == 1 && (stat & ESP_STAT_TC)) {
            state++;
            /* Terminate esp_scsi_dma() command */
            outb(0, iobase + ESP_DMA_CMD);
            استمر;
        }

        /* Request message in data.  */
        إذا (state == 2) {
            state++;
            outb(ESP_CMD_ICCS, iobase + ESP_CMD);
            استمر;
        }

        /* Finally read data from the message in phase and accept.  */
        إذا (state == 3 && (stat & ESP_STAT_ڤسنن)) {
            state++;
            status = inb(iobase + ESP_FIFO);
            inb(iobase + ESP_FIFO);
            outb(ESP_CMD_MSGACC, iobase + ESP_CMD);
            اكسر;
        }
        usleep(5);
    }

    إذا (status == 0) {
        أرجع DISK_RET_SUCCESS;
    }

    أرجع DISK_RET_EBADTRACK;
}

ساكن فراغ
esp_scsi_init_lun(بنية esp_lun_s *llun, بنية pci_device *pci, u32 iobase,
                  u8 target, u8 lun)
{
    memset(llun, 0, حجم(*llun));
    llun->drive.type = DTYPE_ESP_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->target = target;
    llun->lun = lun;
    llun->iobase = iobase;
}

ساكن صحيح
esp_scsi_add_lun(u32 lun, بنية drive_s *tmpl_drv)
{
    بنية esp_lun_s *tmpl_llun =
        container_of(tmpl_drv, بنية esp_lun_s, drive);
    بنية esp_lun_s *llun = malloc_fseg(حجم(*llun));
    إذا (!llun) {
        warn_noalloc();
        أرجع -1;
    }
    esp_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->target, lun);

    حرف *name = znprintf(MAXDESCSIZE, "esp %pP %d:%d",
                          llun->pci, llun->target, llun->lun);
    boot_lchs_find_scsi_device(llun->pci, llun->target, llun->lun,
                               &(llun->drive.lchs));
    صحيح prio = bootprio_find_scsi_device(llun->pci, llun->target, llun->lun);
    صحيح ret = scsi_drive_setup(&llun->drive, name, prio);
    حرر(name);
    إذا (ret)
        اذهب fail;
    أرجع 0;

fail:
    حرر(llun);
    أرجع -1;
}

ساكن فراغ
esp_scsi_scan_target(بنية pci_device *pci, u32 iobase, u8 target)
{
    بنية esp_lun_s llun0;

    esp_scsi_init_lun(&llun0, pci, iobase, target, 0);

    scsi_rep_luns_scan(&llun0.drive, esp_scsi_add_lun);
}

ساكن فراغ
init_esp_scsi(فراغ *data)
{
    بنية pci_device *pci = data;
    u32 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
    إذا (!iobase)
        أرجع;
    pci_enable_busmaster(pci);

    dprintf(1, "found esp at %pP, io @ %x\n", pci, iobase);

    // reset
    outb(ESP_CMD_RESET, iobase + ESP_CMD);

    صحيح i;
    لكل (i = 0; i <= 7; i++)
        esp_scsi_scan_target(pci, iobase, i);
}

فراغ
esp_scsi_setup(فراغ)
{
    ASSERT32FLAT();
    إذا (!CONFIG_ESP_SCSI || !runningOnQEMU())
        أرجع;

    dprintf(3, "init esp\n");

    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci->vendor != PCI_VENDOR_ID_AMD
            || pci->device != PCI_DEVICE_ID_AMD_SCSI)
            استمر;
        run_thread(init_esp_scsi, pci);
    }
}

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

#شامل "biosvar.h" // GET_GLOBALFLAT
#شامل "block.h" // struct drive_s
#شامل "blockcmd.h" // scsi_drive_setup
#شامل "config.h" // CONFIG_*
#شامل "fw/paravirt.h" // runningOnQEMU
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "pcidevice.h" // foreachpci
#شامل "pci_ids.h" // PCI_DEVICE_ID
#شامل "pci_regs.h" // PCI_VENDOR_ID
#شامل "stacks.h" // run_thread
#شامل "std/disk.h" // DISK_RET_SUCCESS
#شامل "string.h" // memset
#شامل "util.h" // usleep

#تعریف_کرو ESP_TCLO      0x00
#تعریف_کرو ESP_TCMID     0x04
#تعریف_کرو ESP_FIFO      0x08
#تعریف_کرو ESP_CMD       0x0c
#تعریف_کرو ESP_WBUSID    0x10
#تعریف_کرو ESP_TCHI      0x38

#تعریف_کرو ESP_RSTAT     0x10
#تعریف_کرو ESP_RINTR     0x14
#تعریف_کرو ESP_RFLAGS    0x1c

#تعریف_کرو ESP_DMA_CMD   0x40
#تعریف_کرو ESP_DMA_STC   0x44
#تعریف_کرو ESP_DMA_SPA   0x48
#تعریف_کرو ESP_DMA_WBC   0x4c
#تعریف_کرو ESP_DMA_WAC   0x50
#تعریف_کرو ESP_DMA_STAT  0x54
#تعریف_کرو ESP_DMA_SMDLA 0x58
#تعریف_کرو ESP_DMA_WMAC  0x58c

#تعریف_کرو ESP_CMD_DMA      0x80
#تعریف_کرو ESP_CMD_FLUSH    0x01
#تعریف_کرو ESP_CMD_RESET    0x02
#تعریف_کرو ESP_CMD_TI       0x10
#تعریف_کرو ESP_CMD_ICCS     0x11
#تعریف_کرو ESP_CMD_MSGACC   0x12
#تعریف_کرو ESP_CMD_SELATN   0x42

#تعریف_کرو ESP_STAT_DI      0x01
#تعریف_کرو ESP_STAT_CD      0x02
#تعریف_کرو ESP_STAT_پیغام     0x04
#تعریف_کرو ESP_STAT_TC      0x10

#تعریف_کرو ESP_INTR_FC      0x08
#تعریف_کرو ESP_INTR_BS      0x10
#تعریف_کرو ESP_INTR_DC      0x20

ساخت esp_lun_s {
    ساخت drive_s drive;
    ساخت pci_device *pci;
    u32 iobase;
    u8 ہدف;
    u8 lun;
};

ساکن خالی
esp_scsi_dma(u32 iobase, u32 buf, u32 len, صحیح اقرأ)
{
    outb(len         & 0xff, iobase + ESP_TCLO);
    outb((len >> 8)  & 0xff, iobase + ESP_TCMID);
    outb((len >> 16) & 0xff, iobase + ESP_TCHI);
    outl(buf,                iobase + ESP_DMA_SPA);
    outl(len,                iobase + ESP_DMA_STC);
    outb(اقرأ ? 0x83 : 0x03, iobase + ESP_DMA_CMD);
}

صحیح
esp_scsi_process_op(ساخت disk_op_s *op)
{
    اگر (!CONFIG_ESP_SCSI)
        واپس DISK_RET_EBADTRACK;
    ساخت esp_lun_s *llun_gf =
        container_کا(op->drive_fl, ساخت esp_lun_s, drive);
    u16 ہدف = GET_GLOBALFLAT(llun_gf->ہدف);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u8 cdbcmd[16];
    صحیح blocksize = scsi_fill_cmd(op, cdbcmd, حجم(cdbcmd));
    اگر (blocksize < 0)
        واپس default_process_op(op);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    صحیح i, state;
    u8 حالت_2;

    outb(ہدف, iobase + ESP_WBUSID);

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
    برائے (i = 0; i < 12; i++)
        outb(cdbcmd[i], iobase + ESP_FIFO);
    outb(ESP_CMD_SELATN, iobase + ESP_CMD);

    برائے (state = 0;;) {
        u8 stat = inb(iobase + ESP_RSTAT);
        u8 intr;

        اگر (state == 0) {
            intr = inb(iobase + ESP_RINTR);

            /* Detect disconnected device.  */
            اگر (intr & ESP_INTR_DC) {
                واپس DISK_RET_ENOTREADY;
            }

            /* HBA reads command, executes it, sets BS/FC -> do DMA if needed.  */
            اگر (intr & (ESP_INTR_BS | ESP_INTR_FC)) {
                state++;
                اگر (op->گنتی && blocksize) {
                    /* Data phase.  */
                    u32 گنتی = (u32)op->گنتی * blocksize;
                    esp_scsi_dma(iobase, (u32)op->buf_fl, گنتی, scsi_is_read(op));
                    outb(ESP_CMD_TI | ESP_CMD_DMA, iobase + ESP_CMD);
                    جاری;
                } ورنہ {
                    /* No data phase.  */
                    state++;
                }
            }
        }

        /* At end of DMA TC is set again -> complete command.  */
        اگر (state == 1 && (stat & ESP_STAT_TC)) {
            state++;
            /* Terminate esp_scsi_dma() command */
            outb(0, iobase + ESP_DMA_CMD);
            جاری;
        }

        /* Request message in data.  */
        اگر (state == 2) {
            state++;
            outb(ESP_CMD_ICCS, iobase + ESP_CMD);
            جاری;
        }

        /* Finally read data from the message in phase and accept.  */
        اگر (state == 3 && (stat & ESP_STAT_پیغام)) {
            state++;
            حالت_2 = inb(iobase + ESP_FIFO);
            inb(iobase + ESP_FIFO);
            outb(ESP_CMD_MSGACC, iobase + ESP_CMD);
            توڑو;
        }
        usleep(5);
    }

    اگر (حالت_2 == 0) {
        واپس DISK_RET_SUCCESS;
    }

    واپس DISK_RET_EBADTRACK;
}

ساکن خالی
esp_scsi_init_lun(ساخت esp_lun_s *llun, ساخت pci_device *pci, u32 iobase,
                  u8 ہدف, u8 lun)
{
    memset(llun, 0, حجم(*llun));
    llun->drive.type = DTYPE_ESP_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->ہدف = ہدف;
    llun->lun = lun;
    llun->iobase = iobase;
}

ساکن صحیح
esp_scsi_جوڑو_lun(u32 lun, ساخت drive_s *tmpl_drv)
{
    ساخت esp_lun_s *tmpl_llun =
        container_کا(tmpl_drv, ساخت esp_lun_s, drive);
    ساخت esp_lun_s *llun = malloc_fseg(حجم(*llun));
    اگر (!llun) {
        warn_noalloc();
        واپس -1;
    }
    esp_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->ہدف, lun);

    حرف *name = znprintf(MAXDESCSIZE, "esp %pP %d:%d",
                          llun->pci, llun->ہدف, llun->lun);
    boot_lchs_ڈھونڈو_scsi_device(llun->pci, llun->ہدف, llun->lun,
                               &(llun->drive.lchs));
    صحیح prio = bootprio_ڈھونڈو_scsi_device(llun->pci, llun->ہدف, llun->lun);
    صحیح ret = scsi_drive_تیاری(&llun->drive, name, prio);
    آزاد(name);
    اگر (ret)
        جاؤ fail;
    واپس 0;

fail:
    آزاد(llun);
    واپس -1;
}

ساکن خالی
esp_scsi_جائزہ_ہدف(ساخت pci_device *pci, u32 iobase, u8 ہدف)
{
    ساخت esp_lun_s llun0;

    esp_scsi_init_lun(&llun0, pci, iobase, ہدف, 0);

    scsi_rep_luns_جائزہ(&llun0.drive, esp_scsi_جوڑو_lun);
}

ساکن خالی
init_esp_scsi(خالی *ڈیٹا)
{
    ساخت pci_device *pci = ڈیٹا;
    u32 iobase = pci_enable_iobar(pci, PCI_بنیاد_پتہ_0);
    اگر (!iobase)
        واپس;
    pci_enable_busmaster(pci);

    dprintf(1, "found esp at %pP, io @ %x\n", pci, iobase);

    // reset
    outb(ESP_CMD_RESET, iobase + ESP_CMD);

    صحیح i;
    برائے (i = 0; i <= 7; i++)
        esp_scsi_جائزہ_ہدف(pci, iobase, i);
}

خالی
esp_scsi_تیاری(خالی)
{
    ASSERT32FLAT();
    اگر (!CONFIG_ESP_SCSI || !runningOnQEMU())
        واپس;

    dprintf(3, "init esp\n");

    ساخت pci_device *pci;
    foreachpci(pci) {
        اگر (pci->vendor != PCI_VENDOR_ID_AMD
            || pci->device != PCI_DEVICE_ID_AMD_SCSI)
            جاری;
        تسلسل_thread(init_esp_scsi, pci);
    }
}

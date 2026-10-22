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

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBALFLAT
#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // struct drive_s
#ਅਨ੍ਤਰ੍ਭਾਵ "blockcmd.h" // scsi_drive_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "fw/paravirt.h" // runningOnQEMU
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pcidevice.h" // foreachpci
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_ids.h" // PCI_DEVICE_ID
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_regs.h" // PCI_VENDOR_ID
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // run_thread
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // DISK_RET_SUCCESS
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // usleep

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_TCLO      0x00
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_TCMID     0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_FIFO      0x08
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_CMD       0x0c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_WBUSID    0x10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_TCHI      0x38

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_RSTAT     0x10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_RINTR     0x14
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_RFLAGS    0x1c

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_DMA_CMD   0x40
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_DMA_STC   0x44
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_DMA_SPA   0x48
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_DMA_WBC   0x4c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_DMA_WAC   0x50
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_DMA_STAT  0x54
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_DMA_SMDLA 0x58
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_DMA_WMAC  0x58c

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_CMD_DMA      0x80
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_CMD_FLUSH    0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_CMD_RESET    0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_CMD_TI       0x10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_CMD_ICCS     0x11
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_CMD_MSGACC   0x12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_CMD_SELATN   0x42

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_STAT_DI      0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_STAT_CD      0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_STAT_ਸੁਨੇਹਾ     0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_STAT_TC      0x10

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_INTR_FC      0x08
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_INTR_BS      0x10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ESP_INTR_DC      0x20

ਬਣਤਰ esp_lun_s {
    ਬਣਤਰ drive_s drive;
    ਬਣਤਰ pci_device *pci;
    u32 iobase;
    u8 ਨਿਸ਼ਾਨਾ;
    u8 lun;
};

ਸਥਿਰ ਖਾਲੀ
esp_scsi_dma(u32 iobase, u32 buf, u32 len, ਪੂਰਨ ਪਠ)
{
    outb(len         & 0xff, iobase + ESP_TCLO);
    outb((len >> 8)  & 0xff, iobase + ESP_TCMID);
    outb((len >> 16) & 0xff, iobase + ESP_TCHI);
    outl(buf,                iobase + ESP_DMA_SPA);
    outl(len,                iobase + ESP_DMA_STC);
    outb(ਪਠ ? 0x83 : 0x03, iobase + ESP_DMA_CMD);
}

ਪੂਰਨ
esp_scsi_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (!CONFIG_ESP_SCSI)
        ਵਾਪਸ DISK_RET_EBADTRACK;
    ਬਣਤਰ esp_lun_s *llun_gf =
        container_ਦਾ(op->drive_fl, ਬਣਤਰ esp_lun_s, drive);
    u16 ਨਿਸ਼ਾਨਾ = GET_GLOBALFLAT(llun_gf->ਨਿਸ਼ਾਨਾ);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u8 cdbcmd[16];
    ਪੂਰਨ blocksize = scsi_fill_cmd(op, cdbcmd, ਆਕਾਰ(cdbcmd));
    ਜੇ (blocksize < 0)
        ਵਾਪਸ default_process_op(op);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    ਪੂਰਨ i, state;
    u8 ਸਥਿਤੀ;

    outb(ਨਿਸ਼ਾਨਾ, iobase + ESP_WBUSID);

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
    ਲਈ (i = 0; i < 12; i++)
        outb(cdbcmd[i], iobase + ESP_FIFO);
    outb(ESP_CMD_SELATN, iobase + ESP_CMD);

    ਲਈ (state = 0;;) {
        u8 stat = inb(iobase + ESP_RSTAT);
        u8 intr;

        ਜੇ (state == 0) {
            intr = inb(iobase + ESP_RINTR);

            /* Detect disconnected device.  */
            ਜੇ (intr & ESP_INTR_DC) {
                ਵਾਪਸ DISK_RET_ENOTREADY;
            }

            /* HBA reads command, executes it, sets BS/FC -> do DMA if needed.  */
            ਜੇ (intr & (ESP_INTR_BS | ESP_INTR_FC)) {
                state++;
                ਜੇ (op->ਗਿਣਤੀ_2 && blocksize) {
                    /* Data phase.  */
                    u32 ਗਿਣਤੀ_2 = (u32)op->ਗਿਣਤੀ_2 * blocksize;
                    esp_scsi_dma(iobase, (u32)op->buf_fl, ਗਿਣਤੀ_2, scsi_is_read(op));
                    outb(ESP_CMD_TI | ESP_CMD_DMA, iobase + ESP_CMD);
                    ਜਾਰੀ;
                } ਨਹੀਂਤਾਂ {
                    /* No data phase.  */
                    state++;
                }
            }
        }

        /* At end of DMA TC is set again -> complete command.  */
        ਜੇ (state == 1 && (stat & ESP_STAT_TC)) {
            state++;
            /* Terminate esp_scsi_dma() command */
            outb(0, iobase + ESP_DMA_CMD);
            ਜਾਰੀ;
        }

        /* Request message in data.  */
        ਜੇ (state == 2) {
            state++;
            outb(ESP_CMD_ICCS, iobase + ESP_CMD);
            ਜਾਰੀ;
        }

        /* Finally read data from the message in phase and accept.  */
        ਜੇ (state == 3 && (stat & ESP_STAT_ਸੁਨੇਹਾ)) {
            state++;
            ਸਥਿਤੀ = inb(iobase + ESP_FIFO);
            inb(iobase + ESP_FIFO);
            outb(ESP_CMD_MSGACC, iobase + ESP_CMD);
            ਰੋਕੋ;
        }
        usleep(5);
    }

    ਜੇ (ਸਥਿਤੀ == 0) {
        ਵਾਪਸ DISK_RET_SUCCESS;
    }

    ਵਾਪਸ DISK_RET_EBADTRACK;
}

ਸਥਿਰ ਖਾਲੀ
esp_scsi_init_lun(ਬਣਤਰ esp_lun_s *llun, ਬਣਤਰ pci_device *pci, u32 iobase,
                  u8 ਨਿਸ਼ਾਨਾ, u8 lun)
{
    memset(llun, 0, ਆਕਾਰ(*llun));
    llun->drive.type = DTYPE_ESP_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->ਨਿਸ਼ਾਨਾ = ਨਿਸ਼ਾਨਾ;
    llun->lun = lun;
    llun->iobase = iobase;
}

ਸਥਿਰ ਪੂਰਨ
esp_scsi_ਜੋੜੋ_lun(u32 lun, ਬਣਤਰ drive_s *tmpl_drv)
{
    ਬਣਤਰ esp_lun_s *tmpl_llun =
        container_ਦਾ(tmpl_drv, ਬਣਤਰ esp_lun_s, drive);
    ਬਣਤਰ esp_lun_s *llun = malloc_fseg(ਆਕਾਰ(*llun));
    ਜੇ (!llun) {
        warn_noalloc();
        ਵਾਪਸ -1;
    }
    esp_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->ਨਿਸ਼ਾਨਾ, lun);

    ਅੱਖਰ *name = znprintf(MAXDESCSIZE, "esp %pP %d:%d",
                          llun->pci, llun->ਨਿਸ਼ਾਨਾ, llun->lun);
    boot_lchs_ਲੱਭੋ_scsi_device(llun->pci, llun->ਨਿਸ਼ਾਨਾ, llun->lun,
                               &(llun->drive.lchs));
    ਪੂਰਨ prio = bootprio_ਲੱਭੋ_scsi_device(llun->pci, llun->ਨਿਸ਼ਾਨਾ, llun->lun);
    ਪੂਰਨ ret = scsi_drive_ਤਿਆਰੀ(&llun->drive, name, prio);
    ਮੁਕਤ(name);
    ਜੇ (ret)
        ਜਾਓ fail;
    ਵਾਪਸ 0;

fail:
    ਮੁਕਤ(llun);
    ਵਾਪਸ -1;
}

ਸਥਿਰ ਖਾਲੀ
esp_scsi_ਜਾਂਚ_ਨਿਸ਼ਾਨਾ(ਬਣਤਰ pci_device *pci, u32 iobase, u8 ਨਿਸ਼ਾਨਾ)
{
    ਬਣਤਰ esp_lun_s llun0;

    esp_scsi_init_lun(&llun0, pci, iobase, ਨਿਸ਼ਾਨਾ, 0);

    scsi_rep_luns_ਜਾਂਚ(&llun0.drive, esp_scsi_ਜੋੜੋ_lun);
}

ਸਥਿਰ ਖਾਲੀ
init_esp_scsi(ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਬਣਤਰ pci_device *pci = ਅੰਕੜੇ;
    u32 iobase = pci_enable_iobar(pci, PCI_ਅਧਾਰ_ਪਤਾ_0);
    ਜੇ (!iobase)
        ਵਾਪਸ;
    pci_enable_busmaster(pci);

    dprintf(1, "found esp at %pP, io @ %x\n", pci, iobase);

    // reset
    outb(ESP_CMD_RESET, iobase + ESP_CMD);

    ਪੂਰਨ i;
    ਲਈ (i = 0; i <= 7; i++)
        esp_scsi_ਜਾਂਚ_ਨਿਸ਼ਾਨਾ(pci, iobase, i);
}

ਖਾਲੀ
esp_scsi_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ASSERT32FLAT();
    ਜੇ (!CONFIG_ESP_SCSI || !runningOnQEMU())
        ਵਾਪਸ;

    dprintf(3, "init esp\n");

    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (pci->vendor != PCI_VENDOR_ID_AMD
            || pci->device != PCI_DEVICE_ID_AMD_SCSI)
            ਜਾਰੀ;
        ਲੜੀਵਾਰ_thread(init_esp_scsi, pci);
    }
}

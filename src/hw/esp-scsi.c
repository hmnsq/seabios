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

#समावेश "biosvar.h" // GET_GLOBALFLAT
#समावेश "block.h" // struct drive_s
#समावेश "blockcmd.h" // scsi_drive_setup
#समावेश "config.h" // CONFIG_*
#समावेश "fw/paravirt.h" // runningOnQEMU
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "pcidevice.h" // foreachpci
#समावेश "pci_ids.h" // PCI_DEVICE_ID
#समावेश "pci_regs.h" // PCI_VENDOR_ID
#समावेश "stacks.h" // run_thread
#समावेश "std/disk.h" // DISK_RET_SUCCESS
#समावेश "string.h" // memset
#समावेश "util.h" // usleep

#घोषणा ESP_TCLO      0x00
#घोषणा ESP_TCMID     0x04
#घोषणा ESP_FIFO      0x08
#घोषणा ESP_CMD       0x0c
#घोषणा ESP_WBUSID    0x10
#घोषणा ESP_TCHI      0x38

#घोषणा ESP_RSTAT     0x10
#घोषणा ESP_RINTR     0x14
#घोषणा ESP_RFLAGS    0x1c

#घोषणा ESP_DMA_CMD   0x40
#घोषणा ESP_DMA_STC   0x44
#घोषणा ESP_DMA_SPA   0x48
#घोषणा ESP_DMA_WBC   0x4c
#घोषणा ESP_DMA_WAC   0x50
#घोषणा ESP_DMA_STAT  0x54
#घोषणा ESP_DMA_SMDLA 0x58
#घोषणा ESP_DMA_WMAC  0x58c

#घोषणा ESP_CMD_DMA      0x80
#घोषणा ESP_CMD_FLUSH    0x01
#घोषणा ESP_CMD_RESET    0x02
#घोषणा ESP_CMD_TI       0x10
#घोषणा ESP_CMD_ICCS     0x11
#घोषणा ESP_CMD_MSGACC   0x12
#घोषणा ESP_CMD_SELATN   0x42

#घोषणा ESP_STAT_DI      0x01
#घोषणा ESP_STAT_CD      0x02
#घोषणा ESP_STAT_संदेश     0x04
#घोषणा ESP_STAT_TC      0x10

#घोषणा ESP_INTR_FC      0x08
#घोषणा ESP_INTR_BS      0x10
#घोषणा ESP_INTR_DC      0x20

काष्ठा esp_lun_s {
    काष्ठा drive_s drive;
    काष्ठा pci_device *pci;
    u32 iobase;
    u8 target;
    u8 lun;
};

अटल व्योम
esp_scsi_dma(u32 iobase, u32 buf, u32 len, पूर्णांक पढ़ो)
{
    outb(len         & 0xff, iobase + ESP_TCLO);
    outb((len >> 8)  & 0xff, iobase + ESP_TCMID);
    outb((len >> 16) & 0xff, iobase + ESP_TCHI);
    outl(buf,                iobase + ESP_DMA_SPA);
    outl(len,                iobase + ESP_DMA_STC);
    outb(पढ़ो ? 0x83 : 0x03, iobase + ESP_DMA_CMD);
}

पूर्णांक
esp_scsi_process_op(काष्ठा disk_op_s *op)
{
    अगर (!CONFIG_ESP_SCSI)
        वापस DISK_RET_EBADTRACK;
    काष्ठा esp_lun_s *llun_gf =
        container_of(op->drive_fl, काष्ठा esp_lun_s, drive);
    u16 target = GET_GLOBALFLAT(llun_gf->target);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u8 cdbcmd[16];
    पूर्णांक blocksize = scsi_fill_cmd(op, cdbcmd, माप(cdbcmd));
    अगर (blocksize < 0)
        वापस default_process_op(op);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    पूर्णांक i, state;
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
    क्रम (i = 0; i < 12; i++)
        outb(cdbcmd[i], iobase + ESP_FIFO);
    outb(ESP_CMD_SELATN, iobase + ESP_CMD);

    क्रम (state = 0;;) {
        u8 stat = inb(iobase + ESP_RSTAT);
        u8 intr;

        अगर (state == 0) {
            intr = inb(iobase + ESP_RINTR);

            /* Detect disconnected device.  */
            अगर (intr & ESP_INTR_DC) {
                वापस DISK_RET_ENOTREADY;
            }

            /* HBA reads command, executes it, sets BS/FC -> do DMA if needed.  */
            अगर (intr & (ESP_INTR_BS | ESP_INTR_FC)) {
                state++;
                अगर (op->गिनती && blocksize) {
                    /* Data phase.  */
                    u32 गिनती = (u32)op->गिनती * blocksize;
                    esp_scsi_dma(iobase, (u32)op->buf_fl, गिनती, scsi_is_read(op));
                    outb(ESP_CMD_TI | ESP_CMD_DMA, iobase + ESP_CMD);
                    जारी;
                } अन्यथा {
                    /* No data phase.  */
                    state++;
                }
            }
        }

        /* At end of DMA TC is set again -> complete command.  */
        अगर (state == 1 && (stat & ESP_STAT_TC)) {
            state++;
            /* Terminate esp_scsi_dma() command */
            outb(0, iobase + ESP_DMA_CMD);
            जारी;
        }

        /* Request message in data.  */
        अगर (state == 2) {
            state++;
            outb(ESP_CMD_ICCS, iobase + ESP_CMD);
            जारी;
        }

        /* Finally read data from the message in phase and accept.  */
        अगर (state == 3 && (stat & ESP_STAT_संदेश)) {
            state++;
            status = inb(iobase + ESP_FIFO);
            inb(iobase + ESP_FIFO);
            outb(ESP_CMD_MSGACC, iobase + ESP_CMD);
            अवरोध;
        }
        usleep(5);
    }

    अगर (status == 0) {
        वापस DISK_RET_SUCCESS;
    }

    वापस DISK_RET_EBADTRACK;
}

अटल व्योम
esp_scsi_init_lun(काष्ठा esp_lun_s *llun, काष्ठा pci_device *pci, u32 iobase,
                  u8 target, u8 lun)
{
    स_रखो(llun, 0, माप(*llun));
    llun->drive.type = DTYPE_ESP_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->target = target;
    llun->lun = lun;
    llun->iobase = iobase;
}

अटल पूर्णांक
esp_scsi_add_lun(u32 lun, काष्ठा drive_s *tmpl_drv)
{
    काष्ठा esp_lun_s *tmpl_llun =
        container_of(tmpl_drv, काष्ठा esp_lun_s, drive);
    काष्ठा esp_lun_s *llun = malloc_fseg(माप(*llun));
    अगर (!llun) {
        warn_noalloc();
        वापस -1;
    }
    esp_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->target, lun);

    अक्षर *name = znprintf(MAXDESCSIZE, "esp %pP %d:%d",
                          llun->pci, llun->target, llun->lun);
    boot_lchs_find_scsi_device(llun->pci, llun->target, llun->lun,
                               &(llun->drive.lchs));
    पूर्णांक prio = bootprio_find_scsi_device(llun->pci, llun->target, llun->lun);
    पूर्णांक ret = scsi_drive_setup(&llun->drive, name, prio);
    मुक्त(name);
    अगर (ret)
        जाओ fail;
    वापस 0;

fail:
    मुक्त(llun);
    वापस -1;
}

अटल व्योम
esp_scsi_scan_target(काष्ठा pci_device *pci, u32 iobase, u8 target)
{
    काष्ठा esp_lun_s llun0;

    esp_scsi_init_lun(&llun0, pci, iobase, target, 0);

    scsi_rep_luns_scan(&llun0.drive, esp_scsi_add_lun);
}

अटल व्योम
init_esp_scsi(व्योम *तथ्य)
{
    काष्ठा pci_device *pci = तथ्य;
    u32 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
    अगर (!iobase)
        वापस;
    pci_enable_busmaster(pci);

    dprintf(1, "found esp at %pP, io @ %x\n", pci, iobase);

    // reset
    outb(ESP_CMD_RESET, iobase + ESP_CMD);

    पूर्णांक i;
    क्रम (i = 0; i <= 7; i++)
        esp_scsi_scan_target(pci, iobase, i);
}

व्योम
esp_scsi_setup(व्योम)
{
    ASSERT32FLAT();
    अगर (!CONFIG_ESP_SCSI || !runningOnQEMU())
        वापस;

    dprintf(3, "init esp\n");

    काष्ठा pci_device *pci;
    foreachpci(pci) {
        अगर (pci->vendor != PCI_VENDOR_ID_AMD
            || pci->device != PCI_DEVICE_ID_AMD_SCSI)
            जारी;
        run_thread(init_esp_scsi, pci);
    }
}

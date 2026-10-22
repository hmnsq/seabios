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

#அந்தர்பாவ "biosvar.h" // GET_GLOBALFLAT
#அந்தர்பாவ "block.h" // struct drive_s
#அந்தர்பாவ "blockcmd.h" // scsi_drive_setup
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "fw/paravirt.h" // runningOnQEMU
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "pcidevice.h" // foreachpci
#அந்தர்பாவ "pci_ids.h" // PCI_DEVICE_ID
#அந்தர்பாவ "pci_regs.h" // PCI_VENDOR_ID
#அந்தர்பாவ "stacks.h" // run_thread
#அந்தர்பாவ "std/disk.h" // DISK_RET_SUCCESS
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // usleep

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
#define ESP_STAT_செய்தி     0x04
#define ESP_STAT_TC      0x10

#define ESP_INTR_FC      0x08
#define ESP_INTR_BS      0x10
#define ESP_INTR_DC      0x20

கட்டமைப்பு esp_lun_s {
    கட்டமைப்பு drive_s drive;
    கட்டமைப்பு pci_device *pci;
    u32 iobase;
    u8 target;
    u8 lun;
};

நிலையான வெறுமை
esp_scsi_dma(u32 iobase, u32 buf, u32 len, முழு பட)
{
    outb(len         & 0xff, iobase + ESP_TCLO);
    outb((len >> 8)  & 0xff, iobase + ESP_TCMID);
    outb((len >> 16) & 0xff, iobase + ESP_TCHI);
    outl(buf,                iobase + ESP_DMA_SPA);
    outl(len,                iobase + ESP_DMA_STC);
    outb(பட ? 0x83 : 0x03, iobase + ESP_DMA_CMD);
}

முழு
esp_scsi_process_op(கட்டமைப்பு disk_op_s *op)
{
    எனில் (!CONFIG_ESP_SCSI)
        திரும்பு DISK_RET_EBADTRACK;
    கட்டமைப்பு esp_lun_s *llun_gf =
        container_of(op->drive_fl, கட்டமைப்பு esp_lun_s, drive);
    u16 target = GET_GLOBALFLAT(llun_gf->target);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u8 cdbcmd[16];
    முழு blocksize = scsi_fill_cmd(op, cdbcmd, அளவு(cdbcmd));
    எனில் (blocksize < 0)
        திரும்பு default_process_op(op);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    முழு i, state;
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
    க்கு (i = 0; i < 12; i++)
        outb(cdbcmd[i], iobase + ESP_FIFO);
    outb(ESP_CMD_SELATN, iobase + ESP_CMD);

    க்கு (state = 0;;) {
        u8 stat = inb(iobase + ESP_RSTAT);
        u8 intr;

        எனில் (state == 0) {
            intr = inb(iobase + ESP_RINTR);

            /* Detect disconnected device.  */
            எனில் (intr & ESP_INTR_DC) {
                திரும்பு DISK_RET_ENOTREADY;
            }

            /* HBA reads command, executes it, sets BS/FC -> do DMA if needed.  */
            எனில் (intr & (ESP_INTR_BS | ESP_INTR_FC)) {
                state++;
                எனில் (op->மொத்தம் && blocksize) {
                    /* Data phase.  */
                    u32 மொத்தம் = (u32)op->மொத்தம் * blocksize;
                    esp_scsi_dma(iobase, (u32)op->buf_fl, மொத்தம், scsi_is_read(op));
                    outb(ESP_CMD_TI | ESP_CMD_DMA, iobase + ESP_CMD);
                    தொடர்;
                } இல்லையெனில் {
                    /* No data phase.  */
                    state++;
                }
            }
        }

        /* At end of DMA TC is set again -> complete command.  */
        எனில் (state == 1 && (stat & ESP_STAT_TC)) {
            state++;
            /* Terminate esp_scsi_dma() command */
            outb(0, iobase + ESP_DMA_CMD);
            தொடர்;
        }

        /* Request message in data.  */
        எனில் (state == 2) {
            state++;
            outb(ESP_CMD_ICCS, iobase + ESP_CMD);
            தொடர்;
        }

        /* Finally read data from the message in phase and accept.  */
        எனில் (state == 3 && (stat & ESP_STAT_செய்தி)) {
            state++;
            status = inb(iobase + ESP_FIFO);
            inb(iobase + ESP_FIFO);
            outb(ESP_CMD_MSGACC, iobase + ESP_CMD);
            நிறுத்து;
        }
        usleep(5);
    }

    எனில் (status == 0) {
        திரும்பு DISK_RET_SUCCESS;
    }

    திரும்பு DISK_RET_EBADTRACK;
}

நிலையான வெறுமை
esp_scsi_init_lun(கட்டமைப்பு esp_lun_s *llun, கட்டமைப்பு pci_device *pci, u32 iobase,
                  u8 target, u8 lun)
{
    memset(llun, 0, அளவு(*llun));
    llun->drive.type = DTYPE_ESP_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->target = target;
    llun->lun = lun;
    llun->iobase = iobase;
}

நிலையான முழு
esp_scsi_add_lun(u32 lun, கட்டமைப்பு drive_s *tmpl_drv)
{
    கட்டமைப்பு esp_lun_s *tmpl_llun =
        container_of(tmpl_drv, கட்டமைப்பு esp_lun_s, drive);
    கட்டமைப்பு esp_lun_s *llun = malloc_fseg(அளவு(*llun));
    எனில் (!llun) {
        warn_noalloc();
        திரும்பு -1;
    }
    esp_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->target, lun);

    எழுத்து *name = znprintf(MAXDESCSIZE, "esp %pP %d:%d",
                          llun->pci, llun->target, llun->lun);
    boot_lchs_find_scsi_device(llun->pci, llun->target, llun->lun,
                               &(llun->drive.lchs));
    முழு prio = bootprio_find_scsi_device(llun->pci, llun->target, llun->lun);
    முழு ret = scsi_drive_setup(&llun->drive, name, prio);
    விடுவி(name);
    எனில் (ret)
        செல் fail;
    திரும்பு 0;

fail:
    விடுவி(llun);
    திரும்பு -1;
}

நிலையான வெறுமை
esp_scsi_scan_target(கட்டமைப்பு pci_device *pci, u32 iobase, u8 target)
{
    கட்டமைப்பு esp_lun_s llun0;

    esp_scsi_init_lun(&llun0, pci, iobase, target, 0);

    scsi_rep_luns_scan(&llun0.drive, esp_scsi_add_lun);
}

நிலையான வெறுமை
init_esp_scsi(வெறுமை *data)
{
    கட்டமைப்பு pci_device *pci = data;
    u32 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
    எனில் (!iobase)
        திரும்பு;
    pci_enable_busmaster(pci);

    dprintf(1, "found esp at %pP, io @ %x\n", pci, iobase);

    // reset
    outb(ESP_CMD_RESET, iobase + ESP_CMD);

    முழு i;
    க்கு (i = 0; i <= 7; i++)
        esp_scsi_scan_target(pci, iobase, i);
}

வெறுமை
esp_scsi_setup(வெறுமை)
{
    ASSERT32FLAT();
    எனில் (!CONFIG_ESP_SCSI || !runningOnQEMU())
        திரும்பு;

    dprintf(3, "init esp\n");

    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        எனில் (pci->vendor != PCI_VENDOR_ID_AMD
            || pci->device != PCI_DEVICE_ID_AMD_SCSI)
            தொடர்;
        run_thread(init_esp_scsi, pci);
    }
}

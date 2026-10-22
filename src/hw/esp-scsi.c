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

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBALFLAT
#ಅನ್ತರ್ಭಾವ "block.h" // struct drive_s
#ಅನ್ತರ್ಭಾವ "blockcmd.h" // scsi_drive_setup
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "fw/paravirt.h" // runningOnQEMU
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "pci_ids.h" // PCI_DEVICE_ID
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_VENDOR_ID
#ಅನ್ತರ್ಭಾವ "stacks.h" // run_thread
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_SUCCESS
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // usleep

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
#define ESP_STAT_ಸಂದೇಶ     0x04
#define ESP_STAT_TC      0x10

#define ESP_INTR_FC      0x08
#define ESP_INTR_BS      0x10
#define ESP_INTR_DC      0x20

ಸಂರಚನಾ esp_lun_s {
    ಸಂರಚನಾ drive_s drive;
    ಸಂರಚನಾ pci_device *pci;
    u32 iobase;
    u8 target;
    u8 lun;
};

ಸ್ಥಿರ ಶೂನ್ಯ
esp_scsi_dma(u32 iobase, u32 buf, u32 len, ಪೂರ್ಣಾಙ್ಕ ಪಠ)
{
    outb(len         & 0xff, iobase + ESP_TCLO);
    outb((len >> 8)  & 0xff, iobase + ESP_TCMID);
    outb((len >> 16) & 0xff, iobase + ESP_TCHI);
    outl(buf,                iobase + ESP_DMA_SPA);
    outl(len,                iobase + ESP_DMA_STC);
    outb(ಪಠ ? 0x83 : 0x03, iobase + ESP_DMA_CMD);
}

ಪೂರ್ಣಾಙ್ಕ
esp_scsi_process_op(ಸಂರಚನಾ disk_op_s *op)
{
    ಯದಿ (!CONFIG_ESP_SCSI)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_EBADTRACK;
    ಸಂರಚನಾ esp_lun_s *llun_gf =
        container_of(op->drive_fl, ಸಂರಚನಾ esp_lun_s, drive);
    u16 target = GET_GLOBALFLAT(llun_gf->target);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u8 cdbcmd[16];
    ಪೂರ್ಣಾಙ್ಕ blocksize = scsi_fill_cmd(op, cdbcmd, ಪರಿಮಾಣ(cdbcmd));
    ಯದಿ (blocksize < 0)
        ಪ್ರತ್ಯಾಗಮ default_process_op(op);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    ಪೂರ್ಣಾಙ್ಕ i, state;
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
    ಪ್ರತಿ (i = 0; i < 12; i++)
        outb(cdbcmd[i], iobase + ESP_FIFO);
    outb(ESP_CMD_SELATN, iobase + ESP_CMD);

    ಪ್ರತಿ (state = 0;;) {
        u8 stat = inb(iobase + ESP_RSTAT);
        u8 intr;

        ಯದಿ (state == 0) {
            intr = inb(iobase + ESP_RINTR);

            /* Detect disconnected device.  */
            ಯದಿ (intr & ESP_INTR_DC) {
                ಪ್ರತ್ಯಾಗಮ DISK_RET_ENOTREADY;
            }

            /* HBA reads command, executes it, sets BS/FC -> do DMA if needed.  */
            ಯದಿ (intr & (ESP_INTR_BS | ESP_INTR_FC)) {
                state++;
                ಯದಿ (op->ಎಣಿಕೆ && blocksize) {
                    /* Data phase.  */
                    u32 ಎಣಿಕೆ = (u32)op->ಎಣಿಕೆ * blocksize;
                    esp_scsi_dma(iobase, (u32)op->buf_fl, ಎಣಿಕೆ, scsi_is_read(op));
                    outb(ESP_CMD_TI | ESP_CMD_DMA, iobase + ESP_CMD);
                    ಅನುವರ್ತ;
                } ಅನ್ಯಥಾ {
                    /* No data phase.  */
                    state++;
                }
            }
        }

        /* At end of DMA TC is set again -> complete command.  */
        ಯದಿ (state == 1 && (stat & ESP_STAT_TC)) {
            state++;
            /* Terminate esp_scsi_dma() command */
            outb(0, iobase + ESP_DMA_CMD);
            ಅನುವರ್ತ;
        }

        /* Request message in data.  */
        ಯದಿ (state == 2) {
            state++;
            outb(ESP_CMD_ICCS, iobase + ESP_CMD);
            ಅನುವರ್ತ;
        }

        /* Finally read data from the message in phase and accept.  */
        ಯದಿ (state == 3 && (stat & ESP_STAT_ಸಂದೇಶ)) {
            state++;
            status = inb(iobase + ESP_FIFO);
            inb(iobase + ESP_FIFO);
            outb(ESP_CMD_MSGACC, iobase + ESP_CMD);
            ಭಙ್ಗ;
        }
        usleep(5);
    }

    ಯದಿ (status == 0) {
        ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;
    }

    ಪ್ರತ್ಯಾಗಮ DISK_RET_EBADTRACK;
}

ಸ್ಥಿರ ಶೂನ್ಯ
esp_scsi_init_lun(ಸಂರಚನಾ esp_lun_s *llun, ಸಂರಚನಾ pci_device *pci, u32 iobase,
                  u8 target, u8 lun)
{
    memset(llun, 0, ಪರಿಮಾಣ(*llun));
    llun->drive.type = DTYPE_ESP_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->target = target;
    llun->lun = lun;
    llun->iobase = iobase;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
esp_scsi_add_lun(u32 lun, ಸಂರಚನಾ drive_s *tmpl_drv)
{
    ಸಂರಚನಾ esp_lun_s *tmpl_llun =
        container_of(tmpl_drv, ಸಂರಚನಾ esp_lun_s, drive);
    ಸಂರಚನಾ esp_lun_s *llun = malloc_fseg(ಪರಿಮಾಣ(*llun));
    ಯದಿ (!llun) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    esp_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->target, lun);

    ವರ್ಣ *name = znprintf(MAXDESCSIZE, "esp %pP %d:%d",
                          llun->pci, llun->target, llun->lun);
    boot_lchs_find_scsi_device(llun->pci, llun->target, llun->lun,
                               &(llun->drive.lchs));
    ಪೂರ್ಣಾಙ್ಕ prio = bootprio_find_scsi_device(llun->pci, llun->target, llun->lun);
    ಪೂರ್ಣಾಙ್ಕ ret = scsi_drive_setup(&llun->drive, name, prio);
    ಮುಕ್ತ(name);
    ಯದಿ (ret)
        ಗಚ್ಛ fail;
    ಪ್ರತ್ಯಾಗಮ 0;

fail:
    ಮುಕ್ತ(llun);
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಶೂನ್ಯ
esp_scsi_scan_target(ಸಂರಚನಾ pci_device *pci, u32 iobase, u8 target)
{
    ಸಂರಚನಾ esp_lun_s llun0;

    esp_scsi_init_lun(&llun0, pci, iobase, target, 0);

    scsi_rep_luns_scan(&llun0.drive, esp_scsi_add_lun);
}

ಸ್ಥಿರ ಶೂನ್ಯ
init_esp_scsi(ಶೂನ್ಯ *data)
{
    ಸಂರಚನಾ pci_device *pci = data;
    u32 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
    ಯದಿ (!iobase)
        ಪ್ರತ್ಯಾಗಮ;
    pci_enable_busmaster(pci);

    dprintf(1, "found esp at %pP, io @ %x\n", pci, iobase);

    // reset
    outb(ESP_CMD_RESET, iobase + ESP_CMD);

    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i = 0; i <= 7; i++)
        esp_scsi_scan_target(pci, iobase, i);
}

ಶೂನ್ಯ
esp_scsi_setup(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಯದಿ (!CONFIG_ESP_SCSI || !runningOnQEMU())
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(3, "init esp\n");

    ಸಂರಚನಾ pci_device *pci;
    foreachpci(pci) {
        ಯದಿ (pci->vendor != PCI_VENDOR_ID_AMD
            || pci->device != PCI_DEVICE_ID_AMD_SCSI)
            ಅನುವರ್ತ;
        run_thread(init_esp_scsi, pci);
    }
}

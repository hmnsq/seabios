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

#ವ್ಯಾಖ್ಯಾನಿಸು ESP_TCLO      0x00
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_TCMID     0x04
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_FIFO      0x08
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_CMD       0x0c
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_WBUSID    0x10
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_TCHI      0x38

#ವ್ಯಾಖ್ಯಾನಿಸು ESP_RSTAT     0x10
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_RINTR     0x14
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_RFLAGS    0x1c

#ವ್ಯಾಖ್ಯಾನಿಸು ESP_DMA_CMD   0x40
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_DMA_STC   0x44
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_DMA_SPA   0x48
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_DMA_WBC   0x4c
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_DMA_WAC   0x50
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_DMA_STAT  0x54
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_DMA_SMDLA 0x58
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_DMA_WMAC  0x58c

#ವ್ಯಾಖ್ಯಾನಿಸು ESP_CMD_DMA      0x80
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_CMD_FLUSH    0x01
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_CMD_RESET    0x02
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_CMD_TI       0x10
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_CMD_ICCS     0x11
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_CMD_MSGACC   0x12
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_CMD_SELATN   0x42

#ವ್ಯಾಖ್ಯಾನಿಸು ESP_STAT_DI      0x01
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_STAT_CD      0x02
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_STAT_ಸಂದೇಶ     0x04
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_STAT_TC      0x10

#ವ್ಯಾಖ್ಯಾನಿಸು ESP_INTR_FC      0x08
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_INTR_BS      0x10
#ವ್ಯಾಖ್ಯಾನಿಸು ESP_INTR_DC      0x20

ರಚನೆ esp_lun_s {
    ರಚನೆ drive_s drive;
    ರಚನೆ pci_device *pci;
    u32 iobase;
    u8 ಗುರಿ;
    u8 lun;
};

ಸ್ಥಿರ ಶೂನ್ಯ
esp_scsi_dma(u32 iobase, u32 buf, u32 len, ಪೂರ್ಣಾಂಕ ಪಠ)
{
    outb(len         & 0xff, iobase + ESP_TCLO);
    outb((len >> 8)  & 0xff, iobase + ESP_TCMID);
    outb((len >> 16) & 0xff, iobase + ESP_TCHI);
    outl(buf,                iobase + ESP_DMA_SPA);
    outl(len,                iobase + ESP_DMA_STC);
    outb(ಪಠ ? 0x83 : 0x03, iobase + ESP_DMA_CMD);
}

ಪೂರ್ಣಾಂಕ
esp_scsi_process_op(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (!CONFIG_ESP_SCSI)
        ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
    ರಚನೆ esp_lun_s *llun_gf =
        container_ದ(op->drive_fl, ರಚನೆ esp_lun_s, drive);
    u16 ಗುರಿ = GET_GLOBALFLAT(llun_gf->ಗುರಿ);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u8 cdbcmd[16];
    ಪೂರ್ಣಾಂಕ blocksize = scsi_fill_cmd(op, cdbcmd, ಗಾತ್ರ(cdbcmd));
    ಆದರೆ (blocksize < 0)
        ಹಿಂತಿರುಗಿ default_process_op(op);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    ಪೂರ್ಣಾಂಕ i, state;
    u8 ಸ್ಥಿತಿ_2;

    outb(ಗುರಿ, iobase + ESP_WBUSID);

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
    ಗಾಗಿ (i = 0; i < 12; i++)
        outb(cdbcmd[i], iobase + ESP_FIFO);
    outb(ESP_CMD_SELATN, iobase + ESP_CMD);

    ಗಾಗಿ (state = 0;;) {
        u8 stat = inb(iobase + ESP_RSTAT);
        u8 intr;

        ಆದರೆ (state == 0) {
            intr = inb(iobase + ESP_RINTR);

            /* Detect disconnected device.  */
            ಆದರೆ (intr & ESP_INTR_DC) {
                ಹಿಂತಿರುಗಿ DISK_RET_ENOTREADY;
            }

            /* HBA reads command, executes it, sets BS/FC -> do DMA if needed.  */
            ಆದರೆ (intr & (ESP_INTR_BS | ESP_INTR_FC)) {
                state++;
                ಆದರೆ (op->ಎಣಿಕೆ_2 && blocksize) {
                    /* Data phase.  */
                    u32 ಎಣಿಕೆ_2 = (u32)op->ಎಣಿಕೆ_2 * blocksize;
                    esp_scsi_dma(iobase, (u32)op->buf_fl, ಎಣಿಕೆ_2, scsi_is_read(op));
                    outb(ESP_CMD_TI | ESP_CMD_DMA, iobase + ESP_CMD);
                    ಮುಂದುವರಿ;
                } ಇಲ್ಲದಿದ್ದರೆ {
                    /* No data phase.  */
                    state++;
                }
            }
        }

        /* At end of DMA TC is set again -> complete command.  */
        ಆದರೆ (state == 1 && (stat & ESP_STAT_TC)) {
            state++;
            /* Terminate esp_scsi_dma() command */
            outb(0, iobase + ESP_DMA_CMD);
            ಮುಂದುವರಿ;
        }

        /* Request message in data.  */
        ಆದರೆ (state == 2) {
            state++;
            outb(ESP_CMD_ICCS, iobase + ESP_CMD);
            ಮುಂದುವರಿ;
        }

        /* Finally read data from the message in phase and accept.  */
        ಆದರೆ (state == 3 && (stat & ESP_STAT_ಸಂದೇಶ)) {
            state++;
            ಸ್ಥಿತಿ_2 = inb(iobase + ESP_FIFO);
            inb(iobase + ESP_FIFO);
            outb(ESP_CMD_MSGACC, iobase + ESP_CMD);
            ನಿಲ್ಲಿಸು;
        }
        usleep(5);
    }

    ಆದರೆ (ಸ್ಥಿತಿ_2 == 0) {
        ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
    }

    ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
}

ಸ್ಥಿರ ಶೂನ್ಯ
esp_scsi_init_lun(ರಚನೆ esp_lun_s *llun, ರಚನೆ pci_device *pci, u32 iobase,
                  u8 ಗುರಿ, u8 lun)
{
    memset(llun, 0, ಗಾತ್ರ(*llun));
    llun->drive.type = DTYPE_ESP_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->ಗುರಿ = ಗುರಿ;
    llun->lun = lun;
    llun->iobase = iobase;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
esp_scsi_ಕೂಡಿಸು_lun(u32 lun, ರಚನೆ drive_s *tmpl_drv)
{
    ರಚನೆ esp_lun_s *tmpl_llun =
        container_ದ(tmpl_drv, ರಚನೆ esp_lun_s, drive);
    ರಚನೆ esp_lun_s *llun = malloc_fseg(ಗಾತ್ರ(*llun));
    ಆದರೆ (!llun) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ -1;
    }
    esp_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->ಗುರಿ, lun);

    ಅಕ್ಷರ *name = znprintf(MAXDESCSIZE, "esp %pP %d:%d",
                          llun->pci, llun->ಗುರಿ, llun->lun);
    boot_lchs_ಕಂಡುಹಿಡಿ_scsi_device(llun->pci, llun->ಗುರಿ, llun->lun,
                               &(llun->drive.lchs));
    ಪೂರ್ಣಾಂಕ prio = bootprio_ಕಂಡುಹಿಡಿ_scsi_device(llun->pci, llun->ಗುರಿ, llun->lun);
    ಪೂರ್ಣಾಂಕ ret = scsi_drive_ಸಿದ್ಧತೆ(&llun->drive, name, prio);
    ಬಿಡುಗಡೆ(name);
    ಆದರೆ (ret)
        ಹೋಗು fail;
    ಹಿಂತಿರುಗಿ 0;

fail:
    ಬಿಡುಗಡೆ(llun);
    ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಶೂನ್ಯ
esp_scsi_ಪರಿಶೀಲನೆ_ಗುರಿ(ರಚನೆ pci_device *pci, u32 iobase, u8 ಗುರಿ)
{
    ರಚನೆ esp_lun_s llun0;

    esp_scsi_init_lun(&llun0, pci, iobase, ಗುರಿ, 0);

    scsi_rep_luns_ಪರಿಶೀಲನೆ(&llun0.drive, esp_scsi_ಕೂಡಿಸು_lun);
}

ಸ್ಥಿರ ಶೂನ್ಯ
init_esp_scsi(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ರಚನೆ pci_device *pci = ದತ್ತಾಂಶ;
    u32 iobase = pci_enable_iobar(pci, PCI_ತಳ_ವಿಳಾಸ_0);
    ಆದರೆ (!iobase)
        ಹಿಂತಿರುಗಿ;
    pci_enable_busmaster(pci);

    dprintf(1, "found esp at %pP, io @ %x\n", pci, iobase);

    // reset
    outb(ESP_CMD_RESET, iobase + ESP_CMD);

    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i = 0; i <= 7; i++)
        esp_scsi_ಪರಿಶೀಲನೆ_ಗುರಿ(pci, iobase, i);
}

ಶೂನ್ಯ
esp_scsi_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಆದರೆ (!CONFIG_ESP_SCSI || !runningOnQEMU())
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "init esp\n");

    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (pci->vendor != PCI_VENDOR_ID_AMD
            || pci->device != PCI_DEVICE_ID_AMD_SCSI)
            ಮುಂದುವರಿ;
        ಸತತ_thread(init_esp_scsi, pci);
    }
}

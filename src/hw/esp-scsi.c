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

#അന്തര്ഭാവ "biosvar.h" // GET_GLOBALFLAT
#അന്തര്ഭാവ "block.h" // struct drive_s
#അന്തര്ഭാവ "blockcmd.h" // scsi_drive_setup
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "fw/paravirt.h" // runningOnQEMU
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "pcidevice.h" // foreachpci
#അന്തര്ഭാവ "pci_ids.h" // PCI_DEVICE_ID
#അന്തര്ഭാവ "pci_regs.h" // PCI_VENDOR_ID
#അന്തര്ഭാവ "stacks.h" // run_thread
#അന്തര്ഭാവ "std/disk.h" // DISK_RET_SUCCESS
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // usleep

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
#define ESP_STAT_സന്ദേശം     0x04
#define ESP_STAT_TC      0x10

#define ESP_INTR_FC      0x08
#define ESP_INTR_BS      0x10
#define ESP_INTR_DC      0x20

ഘടന esp_lun_s {
    ഘടന drive_s drive;
    ഘടന pci_device *pci;
    u32 iobase;
    u8 target;
    u8 lun;
};

സ്ഥിതം ശൂന്യം
esp_scsi_dma(u32 iobase, u32 buf, u32 len, പൂർണ്ണസംഖ്യ പഠ)
{
    outb(len         & 0xff, iobase + ESP_TCLO);
    outb((len >> 8)  & 0xff, iobase + ESP_TCMID);
    outb((len >> 16) & 0xff, iobase + ESP_TCHI);
    outl(buf,                iobase + ESP_DMA_SPA);
    outl(len,                iobase + ESP_DMA_STC);
    outb(പഠ ? 0x83 : 0x03, iobase + ESP_DMA_CMD);
}

പൂർണ്ണസംഖ്യ
esp_scsi_process_op(ഘടന disk_op_s *op)
{
    എങ്കിൽ (!CONFIG_ESP_SCSI)
        തിരികെ DISK_RET_EBADTRACK;
    ഘടന esp_lun_s *llun_gf =
        container_of(op->drive_fl, ഘടന esp_lun_s, drive);
    u16 target = GET_GLOBALFLAT(llun_gf->target);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u8 cdbcmd[16];
    പൂർണ്ണസംഖ്യ blocksize = scsi_fill_cmd(op, cdbcmd, വലുപ്പം(cdbcmd));
    എങ്കിൽ (blocksize < 0)
        തിരികെ default_process_op(op);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    പൂർണ്ണസംഖ്യ i, state;
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
    വേണ്ടി (i = 0; i < 12; i++)
        outb(cdbcmd[i], iobase + ESP_FIFO);
    outb(ESP_CMD_SELATN, iobase + ESP_CMD);

    വേണ്ടി (state = 0;;) {
        u8 stat = inb(iobase + ESP_RSTAT);
        u8 intr;

        എങ്കിൽ (state == 0) {
            intr = inb(iobase + ESP_RINTR);

            /* Detect disconnected device.  */
            എങ്കിൽ (intr & ESP_INTR_DC) {
                തിരികെ DISK_RET_ENOTREADY;
            }

            /* HBA reads command, executes it, sets BS/FC -> do DMA if needed.  */
            എങ്കിൽ (intr & (ESP_INTR_BS | ESP_INTR_FC)) {
                state++;
                എങ്കിൽ (op->എണ്ണം && blocksize) {
                    /* Data phase.  */
                    u32 എണ്ണം = (u32)op->എണ്ണം * blocksize;
                    esp_scsi_dma(iobase, (u32)op->buf_fl, എണ്ണം, scsi_is_read(op));
                    outb(ESP_CMD_TI | ESP_CMD_DMA, iobase + ESP_CMD);
                    തുടരുക;
                } അല്ലെങ്കിൽ {
                    /* No data phase.  */
                    state++;
                }
            }
        }

        /* At end of DMA TC is set again -> complete command.  */
        എങ്കിൽ (state == 1 && (stat & ESP_STAT_TC)) {
            state++;
            /* Terminate esp_scsi_dma() command */
            outb(0, iobase + ESP_DMA_CMD);
            തുടരുക;
        }

        /* Request message in data.  */
        എങ്കിൽ (state == 2) {
            state++;
            outb(ESP_CMD_ICCS, iobase + ESP_CMD);
            തുടരുക;
        }

        /* Finally read data from the message in phase and accept.  */
        എങ്കിൽ (state == 3 && (stat & ESP_STAT_സന്ദേശം)) {
            state++;
            status = inb(iobase + ESP_FIFO);
            inb(iobase + ESP_FIFO);
            outb(ESP_CMD_MSGACC, iobase + ESP_CMD);
            നിർത്തുക;
        }
        usleep(5);
    }

    എങ്കിൽ (status == 0) {
        തിരികെ DISK_RET_SUCCESS;
    }

    തിരികെ DISK_RET_EBADTRACK;
}

സ്ഥിതം ശൂന്യം
esp_scsi_init_lun(ഘടന esp_lun_s *llun, ഘടന pci_device *pci, u32 iobase,
                  u8 target, u8 lun)
{
    memset(llun, 0, വലുപ്പം(*llun));
    llun->drive.type = DTYPE_ESP_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->target = target;
    llun->lun = lun;
    llun->iobase = iobase;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
esp_scsi_add_lun(u32 lun, ഘടന drive_s *tmpl_drv)
{
    ഘടന esp_lun_s *tmpl_llun =
        container_of(tmpl_drv, ഘടന esp_lun_s, drive);
    ഘടന esp_lun_s *llun = malloc_fseg(വലുപ്പം(*llun));
    എങ്കിൽ (!llun) {
        warn_noalloc();
        തിരികെ -1;
    }
    esp_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->target, lun);

    അക്ഷരം *name = znprintf(MAXDESCSIZE, "esp %pP %d:%d",
                          llun->pci, llun->target, llun->lun);
    boot_lchs_find_scsi_device(llun->pci, llun->target, llun->lun,
                               &(llun->drive.lchs));
    പൂർണ്ണസംഖ്യ prio = bootprio_find_scsi_device(llun->pci, llun->target, llun->lun);
    പൂർണ്ണസംഖ്യ ret = scsi_drive_setup(&llun->drive, name, prio);
    സ്വതന്ത്രം(name);
    എങ്കിൽ (ret)
        പോകുക fail;
    തിരികെ 0;

fail:
    സ്വതന്ത്രം(llun);
    തിരികെ -1;
}

സ്ഥിതം ശൂന്യം
esp_scsi_scan_target(ഘടന pci_device *pci, u32 iobase, u8 target)
{
    ഘടന esp_lun_s llun0;

    esp_scsi_init_lun(&llun0, pci, iobase, target, 0);

    scsi_rep_luns_scan(&llun0.drive, esp_scsi_add_lun);
}

സ്ഥിതം ശൂന്യം
init_esp_scsi(ശൂന്യം *data)
{
    ഘടന pci_device *pci = data;
    u32 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
    എങ്കിൽ (!iobase)
        തിരികെ;
    pci_enable_busmaster(pci);

    dprintf(1, "found esp at %pP, io @ %x\n", pci, iobase);

    // reset
    outb(ESP_CMD_RESET, iobase + ESP_CMD);

    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i = 0; i <= 7; i++)
        esp_scsi_scan_target(pci, iobase, i);
}

ശൂന്യം
esp_scsi_setup(ശൂന്യം)
{
    ASSERT32FLAT();
    എങ്കിൽ (!CONFIG_ESP_SCSI || !runningOnQEMU())
        തിരികെ;

    dprintf(3, "init esp\n");

    ഘടന pci_device *pci;
    foreachpci(pci) {
        എങ്കിൽ (pci->vendor != PCI_VENDOR_ID_AMD
            || pci->device != PCI_DEVICE_ID_AMD_SCSI)
            തുടരുക;
        run_thread(init_esp_scsi, pci);
    }
}

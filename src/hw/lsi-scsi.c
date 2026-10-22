// (qemu-emulated) lsi53c895a boot support.
//
// Copyright (C) 2012 Red Hat Inc.
//
// Authors:
//  Gerd Hoffmann <kraxel@redhat.com>
//
// based on virtio-scsi.c which is written by:
//  Paolo Bonzini <pbonzini@redhat.com>
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
#അന്തര്ഭാവ "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#അന്തര്ഭാവ "pci_regs.h" // PCI_VENDOR_ID
#അന്തര്ഭാവ "stacks.h" // run_thread
#അന്തര്ഭാവ "std/disk.h" // DISK_RET_SUCCESS
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // usleep

#define LSI_REG_DSTAT     0x0c
#define LSI_REG_ISTAT0    0x14
#define LSI_REG_DSP0      0x2c
#define LSI_REG_DSP1      0x2d
#define LSI_REG_DSP2      0x2e
#define LSI_REG_DSP3      0x2f
#define LSI_REG_SIST0     0x42
#define LSI_REG_SIST1     0x43

#define LSI_ISTAT0_DIP    0x01
#define LSI_ISTAT0_SIP    0x02
#define LSI_ISTAT0_INTF   0x04
#define LSI_ISTAT0_CON    0x08
#define LSI_ISTAT0_SEM    0x10
#define LSI_ISTAT0_SIGP   0x20
#define LSI_ISTAT0_SRST   0x40
#define LSI_ISTAT0_ABRT   0x80

ഘടന lsi_lun_s {
    ഘടന drive_s drive;
    ഘടന pci_device *pci;
    u32 iobase;
    u8 target;
    u8 lun;
};

പൂർണ്ണസംഖ്യ
lsi_scsi_process_op(ഘടന disk_op_s *op)
{
    എങ്കിൽ (!CONFIG_LSI_SCSI)
        തിരികെ DISK_RET_EBADTRACK;
    ഘടന lsi_lun_s *llun_gf =
        container_of(op->drive_fl, ഘടന lsi_lun_s, drive);
    u16 target = GET_GLOBALFLAT(llun_gf->target);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u8 cdbcmd[16];
    പൂർണ്ണസംഖ്യ blocksize = scsi_fill_cmd(op, cdbcmd, വലുപ്പം(cdbcmd));
    എങ്കിൽ (blocksize < 0)
        തിരികെ default_process_op(op);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    u32 dma = ((scsi_is_read(op) ? 0x01000000 : 0x00000000) |
               (op->എണ്ണം * blocksize));
    u8 msgout[] = {
        0x80 | lun,                 // select lun
        0x08,
    };
    u8 status = 0xff;
    u8 msgin_tmp[2];
    u8 msgin = 0xff;

    u32 script[] = {
        /* select target, send scsi command */
        0x40000000 | target << 16,  // select target
        0x00000000,
        0x06000001,                 // msgout
        (u32)MAKE_FLATPTR(GET_SEG(SS), &msgout),
        0x02000010,                 // scsi command
        (u32)MAKE_FLATPTR(GET_SEG(SS), cdbcmd),

        /* handle disconnect */
        0x87820000,                 // phase == msgin ?
        0x00000018,
        0x07000002,                 // msgin
        (u32)MAKE_FLATPTR(GET_SEG(SS), &msgin_tmp),
        0x50000000,                 // re-select
        0x00000000,
        0x07000002,                 // msgin
        (u32)MAKE_FLATPTR(GET_SEG(SS), &msgin_tmp),

        /* dma data, get status, raise irq */
        dma,                        // dma data
        (u32)op->buf_fl,
        0x03000001,                 // status
        (u32)MAKE_FLATPTR(GET_SEG(SS), &status),
        0x07000001,                 // msgin
        (u32)MAKE_FLATPTR(GET_SEG(SS), &msgin),
        0x98080000,                 // dma irq
        0x00000000,
    };
    u32 dsp = (u32)MAKE_FLATPTR(GET_SEG(SS), &script);

    outb(dsp         & 0xff, iobase + LSI_REG_DSP0);
    outb((dsp >>  8) & 0xff, iobase + LSI_REG_DSP1);
    outb((dsp >> 16) & 0xff, iobase + LSI_REG_DSP2);
    outb((dsp >> 24) & 0xff, iobase + LSI_REG_DSP3);

    വേണ്ടി (;;) {
        u8 dstat = inb(iobase + LSI_REG_DSTAT);
        u8 sist0 = inb(iobase + LSI_REG_SIST0);
        u8 sist1 = inb(iobase + LSI_REG_SIST1);
        എങ്കിൽ (sist0 || sist1) {
            പോകുക fail;
        }
        എങ്കിൽ (dstat & 0x04) {
            നിർത്തുക;
        }
        usleep(5);
    }

    എങ്കിൽ (msgin == 0 && status == 0) {
        തിരികെ DISK_RET_SUCCESS;
    }

fail:
    തിരികെ DISK_RET_EBADTRACK;
}

സ്ഥിതം ശൂന്യം
lsi_scsi_init_lun(ഘടന lsi_lun_s *llun, ഘടന pci_device *pci, u32 iobase,
                  u8 target, u8 lun)
{
    memset(llun, 0, വലുപ്പം(*llun));
    llun->drive.type = DTYPE_LSI_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->target = target;
    llun->lun = lun;
    llun->iobase = iobase;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
lsi_scsi_add_lun(u32 lun, ഘടന drive_s *tmpl_drv)
{
    ഘടന lsi_lun_s *tmpl_llun =
        container_of(tmpl_drv, ഘടന lsi_lun_s, drive);
    ഘടന lsi_lun_s *llun = malloc_fseg(വലുപ്പം(*llun));
    എങ്കിൽ (!llun) {
        warn_noalloc();
        തിരികെ -1;
    }
    lsi_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->target, lun);

    boot_lchs_find_scsi_device(llun->pci, llun->target, llun->lun,
                               &(llun->drive.lchs));
    അക്ഷരം *name = znprintf(MAXDESCSIZE, "lsi %pP %d:%d",
                          llun->pci, llun->target, llun->lun);
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
lsi_scsi_scan_target(ഘടന pci_device *pci, u32 iobase, u8 target)
{
    ഘടന lsi_lun_s llun0;

    lsi_scsi_init_lun(&llun0, pci, iobase, target, 0);

    എങ്കിൽ (scsi_rep_luns_scan(&llun0.drive, lsi_scsi_add_lun) < 0)
        scsi_sequential_scan(&llun0.drive, 8, lsi_scsi_add_lun);
}

സ്ഥിതം ശൂന്യം
init_lsi_scsi(ശൂന്യം *data)
{
    ഘടന pci_device *pci = data;
    u32 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
    എങ്കിൽ (!iobase)
        തിരികെ;
    pci_enable_busmaster(pci);

    dprintf(1, "found lsi53c895a at %pP, io @ %x\n", pci, iobase);

    // reset
    outb(LSI_ISTAT0_SRST, iobase + LSI_REG_ISTAT0);

    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i = 0; i < 7; i++)
        lsi_scsi_scan_target(pci, iobase, i);
}

ശൂന്യം
lsi_scsi_setup(ശൂന്യം)
{
    ASSERT32FLAT();
    എങ്കിൽ (!CONFIG_LSI_SCSI || !runningOnQEMU())
        തിരികെ;

    dprintf(3, "init lsi53c895a\n");

    ഘടന pci_device *pci;
    foreachpci(pci) {
        എങ്കിൽ (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC
            || pci->device != PCI_DEVICE_ID_LSI_53C895A)
            തുടരുക;
        run_thread(init_lsi_scsi, pci);
    }
}

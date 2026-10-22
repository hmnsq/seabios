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

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBALFLAT
#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // struct drive_s
#ਅਨ੍ਤਰ੍ਭਾਵ "blockcmd.h" // scsi_drive_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "fw/paravirt.h" // runningOnQEMU
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pcidevice.h" // foreachpci
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_regs.h" // PCI_VENDOR_ID
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // run_thread
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // DISK_RET_SUCCESS
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // usleep

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSI_REG_DSTAT     0x0c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSI_REG_ISTAT0    0x14
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSI_REG_DSP0      0x2c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSI_REG_DSP1      0x2d
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSI_REG_DSP2      0x2e
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSI_REG_DSP3      0x2f
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSI_REG_SIST0     0x42
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSI_REG_SIST1     0x43

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSI_ISTAT0_DIP    0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSI_ISTAT0_SIP    0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSI_ISTAT0_INTF   0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSI_ISTAT0_CON    0x08
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSI_ISTAT0_SEM    0x10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSI_ISTAT0_SIGP   0x20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSI_ISTAT0_SRST   0x40
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LSI_ISTAT0_ABRT   0x80

ਬਣਤਰ lsi_lun_s {
    ਬਣਤਰ drive_s drive;
    ਬਣਤਰ pci_device *pci;
    u32 iobase;
    u8 ਨਿਸ਼ਾਨਾ;
    u8 lun;
};

ਪੂਰਨ
lsi_scsi_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (!CONFIG_LSI_SCSI)
        ਵਾਪਸ DISK_RET_EBADTRACK;
    ਬਣਤਰ lsi_lun_s *llun_gf =
        container_ਦਾ(op->drive_fl, ਬਣਤਰ lsi_lun_s, drive);
    u16 ਨਿਸ਼ਾਨਾ = GET_GLOBALFLAT(llun_gf->ਨਿਸ਼ਾਨਾ);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u8 cdbcmd[16];
    ਪੂਰਨ blocksize = scsi_fill_cmd(op, cdbcmd, ਆਕਾਰ(cdbcmd));
    ਜੇ (blocksize < 0)
        ਵਾਪਸ default_process_op(op);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    u32 dma = ((scsi_is_read(op) ? 0x01000000 : 0x00000000) |
               (op->ਗਿਣਤੀ_2 * blocksize));
    u8 msgout[] = {
        0x80 | lun,                 // select lun
        0x08,
    };
    u8 ਸਥਿਤੀ = 0xff;
    u8 msgin_tmp[2];
    u8 msgin = 0xff;

    u32 script[] = {
        /* select target, send scsi command */
        0x40000000 | ਨਿਸ਼ਾਨਾ << 16,  // select target
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
        0x03000001,                 //ਸਥਿਤੀ
        (u32)MAKE_FLATPTR(GET_SEG(SS), &ਸਥਿਤੀ),
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

    ਲਈ (;;) {
        u8 dstat = inb(iobase + LSI_REG_DSTAT);
        u8 sist0 = inb(iobase + LSI_REG_SIST0);
        u8 sist1 = inb(iobase + LSI_REG_SIST1);
        ਜੇ (sist0 || sist1) {
            ਜਾਓ fail;
        }
        ਜੇ (dstat & 0x04) {
            ਰੋਕੋ;
        }
        usleep(5);
    }

    ਜੇ (msgin == 0 && ਸਥਿਤੀ == 0) {
        ਵਾਪਸ DISK_RET_SUCCESS;
    }

fail:
    ਵਾਪਸ DISK_RET_EBADTRACK;
}

ਸਥਿਰ ਖਾਲੀ
lsi_scsi_init_lun(ਬਣਤਰ lsi_lun_s *llun, ਬਣਤਰ pci_device *pci, u32 iobase,
                  u8 ਨਿਸ਼ਾਨਾ, u8 lun)
{
    memset(llun, 0, ਆਕਾਰ(*llun));
    llun->drive.type = DTYPE_LSI_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->ਨਿਸ਼ਾਨਾ = ਨਿਸ਼ਾਨਾ;
    llun->lun = lun;
    llun->iobase = iobase;
}

ਸਥਿਰ ਪੂਰਨ
lsi_scsi_ਜੋੜੋ_lun(u32 lun, ਬਣਤਰ drive_s *tmpl_drv)
{
    ਬਣਤਰ lsi_lun_s *tmpl_llun =
        container_ਦਾ(tmpl_drv, ਬਣਤਰ lsi_lun_s, drive);
    ਬਣਤਰ lsi_lun_s *llun = malloc_fseg(ਆਕਾਰ(*llun));
    ਜੇ (!llun) {
        warn_noalloc();
        ਵਾਪਸ -1;
    }
    lsi_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->ਨਿਸ਼ਾਨਾ, lun);

    boot_lchs_ਲੱਭੋ_scsi_device(llun->pci, llun->ਨਿਸ਼ਾਨਾ, llun->lun,
                               &(llun->drive.lchs));
    ਅੱਖਰ *name = znprintf(MAXDESCSIZE, "lsi %pP %d:%d",
                          llun->pci, llun->ਨਿਸ਼ਾਨਾ, llun->lun);
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
lsi_scsi_ਜਾਂਚ_ਨਿਸ਼ਾਨਾ(ਬਣਤਰ pci_device *pci, u32 iobase, u8 ਨਿਸ਼ਾਨਾ)
{
    ਬਣਤਰ lsi_lun_s llun0;

    lsi_scsi_init_lun(&llun0, pci, iobase, ਨਿਸ਼ਾਨਾ, 0);

    ਜੇ (scsi_rep_luns_ਜਾਂਚ(&llun0.drive, lsi_scsi_ਜੋੜੋ_lun) < 0)
        scsi_sequential_ਜਾਂਚ(&llun0.drive, 8, lsi_scsi_ਜੋੜੋ_lun);
}

ਸਥਿਰ ਖਾਲੀ
init_lsi_scsi(ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਬਣਤਰ pci_device *pci = ਅੰਕੜੇ;
    u32 iobase = pci_enable_iobar(pci, PCI_ਅਧਾਰ_ਪਤਾ_0);
    ਜੇ (!iobase)
        ਵਾਪਸ;
    pci_enable_busmaster(pci);

    dprintf(1, "found lsi53c895a at %pP, io @ %x\n", pci, iobase);

    // reset
    outb(LSI_ISTAT0_SRST, iobase + LSI_REG_ISTAT0);

    ਪੂਰਨ i;
    ਲਈ (i = 0; i < 7; i++)
        lsi_scsi_ਜਾਂਚ_ਨਿਸ਼ਾਨਾ(pci, iobase, i);
}

ਖਾਲੀ
lsi_scsi_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ASSERT32FLAT();
    ਜੇ (!CONFIG_LSI_SCSI || !runningOnQEMU())
        ਵਾਪਸ;

    dprintf(3, "init lsi53c895a\n");

    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC
            || pci->device != PCI_DEVICE_ID_LSI_53C895A)
            ਜਾਰੀ;
        ਲੜੀਵਾਰ_thread(init_lsi_scsi, pci);
    }
}

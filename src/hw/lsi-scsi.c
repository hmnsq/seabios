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

#అన్తర్భావ "biosvar.h" // GET_GLOBALFLAT
#అన్తర్భావ "block.h" // struct drive_s
#అన్తర్భావ "blockcmd.h" // scsi_drive_setup
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "fw/paravirt.h" // runningOnQEMU
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pcidevice.h" // foreachpci
#అన్తర్భావ "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#అన్తర్భావ "pci_regs.h" // PCI_VENDOR_ID
#అన్తర్భావ "stacks.h" // run_thread
#అన్తర్భావ "std/disk.h" // DISK_RET_SUCCESS
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // usleep

#నిర్వచించు LSI_REG_DSTAT     0x0c
#నిర్వచించు LSI_REG_ISTAT0    0x14
#నిర్వచించు LSI_REG_DSP0      0x2c
#నిర్వచించు LSI_REG_DSP1      0x2d
#నిర్వచించు LSI_REG_DSP2      0x2e
#నిర్వచించు LSI_REG_DSP3      0x2f
#నిర్వచించు LSI_REG_SIST0     0x42
#నిర్వచించు LSI_REG_SIST1     0x43

#నిర్వచించు LSI_ISTAT0_DIP    0x01
#నిర్వచించు LSI_ISTAT0_SIP    0x02
#నిర్వచించు LSI_ISTAT0_INTF   0x04
#నిర్వచించు LSI_ISTAT0_CON    0x08
#నిర్వచించు LSI_ISTAT0_SEM    0x10
#నిర్వచించు LSI_ISTAT0_SIGP   0x20
#నిర్వచించు LSI_ISTAT0_SRST   0x40
#నిర్వచించు LSI_ISTAT0_ABRT   0x80

నిర్మాణం lsi_lun_s {
    నిర్మాణం drive_s drive;
    నిర్మాణం pci_device *pci;
    u32 iobase;
    u8 లక్ష్యం;
    u8 lun;
};

పూర్ణాంకం
lsi_scsi_process_op(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (!CONFIG_LSI_SCSI)
        తిరిగి DISK_RET_EBADTRACK;
    నిర్మాణం lsi_lun_s *llun_gf =
        container_యొక్క(op->drive_fl, నిర్మాణం lsi_lun_s, drive);
    u16 లక్ష్యం = GET_GLOBALFLAT(llun_gf->లక్ష్యం);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u8 cdbcmd[16];
    పూర్ణాంకం blocksize = scsi_fill_cmd(op, cdbcmd, పరిమాణం(cdbcmd));
    ఒకవేళ (blocksize < 0)
        తిరిగి default_process_op(op);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    u32 dma = ((scsi_is_read(op) ? 0x01000000 : 0x00000000) |
               (op->లెక్క * blocksize));
    u8 msgout[] = {
        0x80 | lun,                 // select lun
        0x08,
    };
    u8 స్థితి_2 = 0xff;
    u8 msgin_tmp[2];
    u8 msgin = 0xff;

    u32 script[] = {
        /* select target, send scsi command */
        0x40000000 | లక్ష్యం << 16,  // select target
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
        0x03000001,                 //స్థితి
        (u32)MAKE_FLATPTR(GET_SEG(SS), &స్థితి_2),
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

    కొరకు (;;) {
        u8 dstat = inb(iobase + LSI_REG_DSTAT);
        u8 sist0 = inb(iobase + LSI_REG_SIST0);
        u8 sist1 = inb(iobase + LSI_REG_SIST1);
        ఒకవేళ (sist0 || sist1) {
            వెళ్ళు fail;
        }
        ఒకవేళ (dstat & 0x04) {
            ఆపు;
        }
        usleep(5);
    }

    ఒకవేళ (msgin == 0 && స్థితి_2 == 0) {
        తిరిగి DISK_RET_SUCCESS;
    }

fail:
    తిరిగి DISK_RET_EBADTRACK;
}

స్థిర శూన్య
lsi_scsi_init_lun(నిర్మాణం lsi_lun_s *llun, నిర్మాణం pci_device *pci, u32 iobase,
                  u8 లక్ష్యం, u8 lun)
{
    memset(llun, 0, పరిమాణం(*llun));
    llun->drive.type = DTYPE_LSI_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->లక్ష్యం = లక్ష్యం;
    llun->lun = lun;
    llun->iobase = iobase;
}

స్థిర పూర్ణాంకం
lsi_scsi_కూడు_lun(u32 lun, నిర్మాణం drive_s *tmpl_drv)
{
    నిర్మాణం lsi_lun_s *tmpl_llun =
        container_యొక్క(tmpl_drv, నిర్మాణం lsi_lun_s, drive);
    నిర్మాణం lsi_lun_s *llun = malloc_fseg(పరిమాణం(*llun));
    ఒకవేళ (!llun) {
        warn_noalloc();
        తిరిగి -1;
    }
    lsi_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->లక్ష్యం, lun);

    boot_lchs_కనుగొను_scsi_device(llun->pci, llun->లక్ష్యం, llun->lun,
                               &(llun->drive.lchs));
    అక్షరం *name = znprintf(MAXDESCSIZE, "lsi %pP %d:%d",
                          llun->pci, llun->లక్ష్యం, llun->lun);
    పూర్ణాంకం prio = bootprio_కనుగొను_scsi_device(llun->pci, llun->లక్ష్యం, llun->lun);
    పూర్ణాంకం ret = scsi_drive_అమరిక(&llun->drive, name, prio);
    విడుదల(name);
    ఒకవేళ (ret)
        వెళ్ళు fail;
    తిరిగి 0;

fail:
    విడుదల(llun);
    తిరిగి -1;
}

స్థిర శూన్య
lsi_scsi_పరిశీలన_లక్ష్యం(నిర్మాణం pci_device *pci, u32 iobase, u8 లక్ష్యం)
{
    నిర్మాణం lsi_lun_s llun0;

    lsi_scsi_init_lun(&llun0, pci, iobase, లక్ష్యం, 0);

    ఒకవేళ (scsi_rep_luns_పరిశీలన(&llun0.drive, lsi_scsi_కూడు_lun) < 0)
        scsi_sequential_పరిశీలన(&llun0.drive, 8, lsi_scsi_కూడు_lun);
}

స్థిర శూన్య
init_lsi_scsi(శూన్య *దత్తాంశం)
{
    నిర్మాణం pci_device *pci = దత్తాంశం;
    u32 iobase = pci_enable_iobar(pci, PCI_పునాది_చిరునామా_0);
    ఒకవేళ (!iobase)
        తిరిగి;
    pci_enable_busmaster(pci);

    dprintf(1, "found lsi53c895a at %pP, io @ %x\n", pci, iobase);

    // reset
    outb(LSI_ISTAT0_SRST, iobase + LSI_REG_ISTAT0);

    పూర్ణాంకం i;
    కొరకు (i = 0; i < 7; i++)
        lsi_scsi_పరిశీలన_లక్ష్యం(pci, iobase, i);
}

శూన్య
lsi_scsi_అమరిక(శూన్య)
{
    ASSERT32FLAT();
    ఒకవేళ (!CONFIG_LSI_SCSI || !runningOnQEMU())
        తిరిగి;

    dprintf(3, "init lsi53c895a\n");

    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC
            || pci->device != PCI_DEVICE_ID_LSI_53C895A)
            కొనసాగు;
        నిరంతరం_thread(init_lsi_scsi, pci);
    }
}

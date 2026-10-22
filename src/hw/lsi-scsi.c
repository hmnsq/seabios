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

#অন্তর্ভাব "biosvar.h" // GET_GLOBALFLAT
#অন্তর্ভাব "block.h" // struct drive_s
#অন্তর্ভাব "blockcmd.h" // scsi_drive_setup
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "fw/paravirt.h" // runningOnQEMU
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "pcidevice.h" // foreachpci
#অন্তর্ভাব "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#অন্তর্ভাব "pci_regs.h" // PCI_VENDOR_ID
#অন্তর্ভাব "stacks.h" // run_thread
#অন্তর্ভাব "std/disk.h" // DISK_RET_SUCCESS
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // usleep

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

কাঠামো lsi_lun_s {
    কাঠামো drive_s drive;
    কাঠামো pci_device *pci;
    u32 iobase;
    u8 target;
    u8 lun;
};

পূর্ণসংখ্যা
lsi_scsi_process_op(কাঠামো disk_op_s *op)
{
    যদি (!CONFIG_LSI_SCSI)
        ফেরত DISK_RET_EBADTRACK;
    কাঠামো lsi_lun_s *llun_gf =
        container_of(op->drive_fl, কাঠামো lsi_lun_s, drive);
    u16 target = GET_GLOBALFLAT(llun_gf->target);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u8 cdbcmd[16];
    পূর্ণসংখ্যা blocksize = scsi_fill_cmd(op, cdbcmd, আকার(cdbcmd));
    যদি (blocksize < 0)
        ফেরত default_process_op(op);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    u32 dma = ((scsi_is_read(op) ? 0x01000000 : 0x00000000) |
               (op->গুনতি * blocksize));
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

    জন্য (;;) {
        u8 dstat = inb(iobase + LSI_REG_DSTAT);
        u8 sist0 = inb(iobase + LSI_REG_SIST0);
        u8 sist1 = inb(iobase + LSI_REG_SIST1);
        যদি (sist0 || sist1) {
            যাও fail;
        }
        যদি (dstat & 0x04) {
            বিরতি;
        }
        usleep(5);
    }

    যদি (msgin == 0 && status == 0) {
        ফেরত DISK_RET_SUCCESS;
    }

fail:
    ফেরত DISK_RET_EBADTRACK;
}

স্থির শূন্য
lsi_scsi_init_lun(কাঠামো lsi_lun_s *llun, কাঠামো pci_device *pci, u32 iobase,
                  u8 target, u8 lun)
{
    memset(llun, 0, আকার(*llun));
    llun->drive.type = DTYPE_LSI_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->target = target;
    llun->lun = lun;
    llun->iobase = iobase;
}

স্থির পূর্ণসংখ্যা
lsi_scsi_add_lun(u32 lun, কাঠামো drive_s *tmpl_drv)
{
    কাঠামো lsi_lun_s *tmpl_llun =
        container_of(tmpl_drv, কাঠামো lsi_lun_s, drive);
    কাঠামো lsi_lun_s *llun = malloc_fseg(আকার(*llun));
    যদি (!llun) {
        warn_noalloc();
        ফেরত -1;
    }
    lsi_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->target, lun);

    boot_lchs_find_scsi_device(llun->pci, llun->target, llun->lun,
                               &(llun->drive.lchs));
    অক্ষর *name = znprintf(MAXDESCSIZE, "lsi %pP %d:%d",
                          llun->pci, llun->target, llun->lun);
    পূর্ণসংখ্যা prio = bootprio_find_scsi_device(llun->pci, llun->target, llun->lun);
    পূর্ণসংখ্যা ret = scsi_drive_setup(&llun->drive, name, prio);
    মুক্ত(name);
    যদি (ret)
        যাও fail;
    ফেরত 0;

fail:
    মুক্ত(llun);
    ফেরত -1;
}

স্থির শূন্য
lsi_scsi_scan_target(কাঠামো pci_device *pci, u32 iobase, u8 target)
{
    কাঠামো lsi_lun_s llun0;

    lsi_scsi_init_lun(&llun0, pci, iobase, target, 0);

    যদি (scsi_rep_luns_scan(&llun0.drive, lsi_scsi_add_lun) < 0)
        scsi_sequential_scan(&llun0.drive, 8, lsi_scsi_add_lun);
}

স্থির শূন্য
init_lsi_scsi(শূন্য *তথ্য)
{
    কাঠামো pci_device *pci = তথ্য;
    u32 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
    যদি (!iobase)
        ফেরত;
    pci_enable_busmaster(pci);

    dprintf(1, "found lsi53c895a at %pP, io @ %x\n", pci, iobase);

    // reset
    outb(LSI_ISTAT0_SRST, iobase + LSI_REG_ISTAT0);

    পূর্ণসংখ্যা i;
    জন্য (i = 0; i < 7; i++)
        lsi_scsi_scan_target(pci, iobase, i);
}

শূন্য
lsi_scsi_setup(শূন্য)
{
    ASSERT32FLAT();
    যদি (!CONFIG_LSI_SCSI || !runningOnQEMU())
        ফেরত;

    dprintf(3, "init lsi53c895a\n");

    কাঠামো pci_device *pci;
    foreachpci(pci) {
        যদি (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC
            || pci->device != PCI_DEVICE_ID_LSI_53C895A)
            চালু;
        run_thread(init_lsi_scsi, pci);
    }
}

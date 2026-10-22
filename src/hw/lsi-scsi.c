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

#شامل "biosvar.h" // GET_GLOBALFLAT
#شامل "block.h" // struct drive_s
#شامل "blockcmd.h" // scsi_drive_setup
#شامل "config.h" // CONFIG_*
#شامل "fw/paravirt.h" // runningOnQEMU
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "pcidevice.h" // foreachpci
#شامل "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#شامل "pci_regs.h" // PCI_VENDOR_ID
#شامل "stacks.h" // run_thread
#شامل "std/disk.h" // DISK_RET_SUCCESS
#شامل "string.h" // memset
#شامل "util.h" // usleep

#تعریف_کرو LSI_REG_DSTAT     0x0c
#تعریف_کرو LSI_REG_ISTAT0    0x14
#تعریف_کرو LSI_REG_DSP0      0x2c
#تعریف_کرو LSI_REG_DSP1      0x2d
#تعریف_کرو LSI_REG_DSP2      0x2e
#تعریف_کرو LSI_REG_DSP3      0x2f
#تعریف_کرو LSI_REG_SIST0     0x42
#تعریف_کرو LSI_REG_SIST1     0x43

#تعریف_کرو LSI_ISTAT0_DIP    0x01
#تعریف_کرو LSI_ISTAT0_SIP    0x02
#تعریف_کرو LSI_ISTAT0_INTF   0x04
#تعریف_کرو LSI_ISTAT0_CON    0x08
#تعریف_کرو LSI_ISTAT0_SEM    0x10
#تعریف_کرو LSI_ISTAT0_SIGP   0x20
#تعریف_کرو LSI_ISTAT0_SRST   0x40
#تعریف_کرو LSI_ISTAT0_ABRT   0x80

ساخت lsi_lun_s {
    ساخت drive_s drive;
    ساخت pci_device *pci;
    u32 iobase;
    u8 ہدف;
    u8 lun;
};

صحیح
lsi_scsi_process_op(ساخت disk_op_s *op)
{
    اگر (!CONFIG_LSI_SCSI)
        واپس DISK_RET_EBADTRACK;
    ساخت lsi_lun_s *llun_gf =
        container_کا(op->drive_fl, ساخت lsi_lun_s, drive);
    u16 ہدف = GET_GLOBALFLAT(llun_gf->ہدف);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u8 cdbcmd[16];
    صحیح blocksize = scsi_fill_cmd(op, cdbcmd, حجم(cdbcmd));
    اگر (blocksize < 0)
        واپس default_process_op(op);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    u32 dma = ((scsi_is_read(op) ? 0x01000000 : 0x00000000) |
               (op->گنتی * blocksize));
    u8 msgout[] = {
        0x80 | lun,                 // select lun
        0x08,
    };
    u8 حالت_2 = 0xff;
    u8 msgin_tmp[2];
    u8 msgin = 0xff;

    u32 script[] = {
        /* select target, send scsi command */
        0x40000000 | ہدف << 16,  // select target
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
        0x03000001,                 //حالت
        (u32)MAKE_FLATPTR(GET_SEG(SS), &حالت_2),
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

    برائے (;;) {
        u8 dstat = inb(iobase + LSI_REG_DSTAT);
        u8 sist0 = inb(iobase + LSI_REG_SIST0);
        u8 sist1 = inb(iobase + LSI_REG_SIST1);
        اگر (sist0 || sist1) {
            جاؤ fail;
        }
        اگر (dstat & 0x04) {
            توڑو;
        }
        usleep(5);
    }

    اگر (msgin == 0 && حالت_2 == 0) {
        واپس DISK_RET_SUCCESS;
    }

fail:
    واپس DISK_RET_EBADTRACK;
}

ساکن خالی
lsi_scsi_init_lun(ساخت lsi_lun_s *llun, ساخت pci_device *pci, u32 iobase,
                  u8 ہدف, u8 lun)
{
    memset(llun, 0, حجم(*llun));
    llun->drive.type = DTYPE_LSI_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->ہدف = ہدف;
    llun->lun = lun;
    llun->iobase = iobase;
}

ساکن صحیح
lsi_scsi_جوڑو_lun(u32 lun, ساخت drive_s *tmpl_drv)
{
    ساخت lsi_lun_s *tmpl_llun =
        container_کا(tmpl_drv, ساخت lsi_lun_s, drive);
    ساخت lsi_lun_s *llun = malloc_fseg(حجم(*llun));
    اگر (!llun) {
        warn_noalloc();
        واپس -1;
    }
    lsi_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->ہدف, lun);

    boot_lchs_ڈھونڈو_scsi_device(llun->pci, llun->ہدف, llun->lun,
                               &(llun->drive.lchs));
    حرف *name = znprintf(MAXDESCSIZE, "lsi %pP %d:%d",
                          llun->pci, llun->ہدف, llun->lun);
    صحیح prio = bootprio_ڈھونڈو_scsi_device(llun->pci, llun->ہدف, llun->lun);
    صحیح ret = scsi_drive_تیاری(&llun->drive, name, prio);
    آزاد(name);
    اگر (ret)
        جاؤ fail;
    واپس 0;

fail:
    آزاد(llun);
    واپس -1;
}

ساکن خالی
lsi_scsi_جائزہ_ہدف(ساخت pci_device *pci, u32 iobase, u8 ہدف)
{
    ساخت lsi_lun_s llun0;

    lsi_scsi_init_lun(&llun0, pci, iobase, ہدف, 0);

    اگر (scsi_rep_luns_جائزہ(&llun0.drive, lsi_scsi_جوڑو_lun) < 0)
        scsi_sequential_جائزہ(&llun0.drive, 8, lsi_scsi_جوڑو_lun);
}

ساکن خالی
init_lsi_scsi(خالی *ڈیٹا)
{
    ساخت pci_device *pci = ڈیٹا;
    u32 iobase = pci_enable_iobar(pci, PCI_بنیاد_پتہ_0);
    اگر (!iobase)
        واپس;
    pci_enable_busmaster(pci);

    dprintf(1, "found lsi53c895a at %pP, io @ %x\n", pci, iobase);

    // reset
    outb(LSI_ISTAT0_SRST, iobase + LSI_REG_ISTAT0);

    صحیح i;
    برائے (i = 0; i < 7; i++)
        lsi_scsi_جائزہ_ہدف(pci, iobase, i);
}

خالی
lsi_scsi_تیاری(خالی)
{
    ASSERT32FLAT();
    اگر (!CONFIG_LSI_SCSI || !runningOnQEMU())
        واپس;

    dprintf(3, "init lsi53c895a\n");

    ساخت pci_device *pci;
    foreachpci(pci) {
        اگر (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC
            || pci->device != PCI_DEVICE_ID_LSI_53C895A)
            جاری;
        تسلسل_thread(init_lsi_scsi, pci);
    }
}

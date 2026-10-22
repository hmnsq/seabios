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

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBALFLAT
#ಅನ್ತರ್ಭಾವ "block.h" // struct drive_s
#ಅನ್ತರ್ಭಾವ "blockcmd.h" // scsi_drive_setup
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "fw/paravirt.h" // runningOnQEMU
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_VENDOR_ID
#ಅನ್ತರ್ಭಾವ "stacks.h" // run_thread
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_SUCCESS
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // usleep

#ವ್ಯಾಖ್ಯಾನಿಸು LSI_REG_DSTAT     0x0c
#ವ್ಯಾಖ್ಯಾನಿಸು LSI_REG_ISTAT0    0x14
#ವ್ಯಾಖ್ಯಾನಿಸು LSI_REG_DSP0      0x2c
#ವ್ಯಾಖ್ಯಾನಿಸು LSI_REG_DSP1      0x2d
#ವ್ಯಾಖ್ಯಾನಿಸು LSI_REG_DSP2      0x2e
#ವ್ಯಾಖ್ಯಾನಿಸು LSI_REG_DSP3      0x2f
#ವ್ಯಾಖ್ಯಾನಿಸು LSI_REG_SIST0     0x42
#ವ್ಯಾಖ್ಯಾನಿಸು LSI_REG_SIST1     0x43

#ವ್ಯಾಖ್ಯಾನಿಸು LSI_ISTAT0_DIP    0x01
#ವ್ಯಾಖ್ಯಾನಿಸು LSI_ISTAT0_SIP    0x02
#ವ್ಯಾಖ್ಯಾನಿಸು LSI_ISTAT0_INTF   0x04
#ವ್ಯಾಖ್ಯಾನಿಸು LSI_ISTAT0_CON    0x08
#ವ್ಯಾಖ್ಯಾನಿಸು LSI_ISTAT0_SEM    0x10
#ವ್ಯಾಖ್ಯಾನಿಸು LSI_ISTAT0_SIGP   0x20
#ವ್ಯಾಖ್ಯಾನಿಸು LSI_ISTAT0_SRST   0x40
#ವ್ಯಾಖ್ಯಾನಿಸು LSI_ISTAT0_ABRT   0x80

ರಚನೆ lsi_lun_s {
    ರಚನೆ drive_s drive;
    ರಚನೆ pci_device *pci;
    u32 iobase;
    u8 ಗುರಿ;
    u8 lun;
};

ಪೂರ್ಣಾಂಕ
lsi_scsi_process_op(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (!CONFIG_LSI_SCSI)
        ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
    ರಚನೆ lsi_lun_s *llun_gf =
        container_ದ(op->drive_fl, ರಚನೆ lsi_lun_s, drive);
    u16 ಗುರಿ = GET_GLOBALFLAT(llun_gf->ಗುರಿ);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u8 cdbcmd[16];
    ಪೂರ್ಣಾಂಕ blocksize = scsi_fill_cmd(op, cdbcmd, ಗಾತ್ರ(cdbcmd));
    ಆದರೆ (blocksize < 0)
        ಹಿಂತಿರುಗಿ default_process_op(op);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    u32 dma = ((scsi_is_read(op) ? 0x01000000 : 0x00000000) |
               (op->ಎಣಿಕೆ_2 * blocksize));
    u8 msgout[] = {
        0x80 | lun,                 // select lun
        0x08,
    };
    u8 ಸ್ಥಿತಿ_2 = 0xff;
    u8 msgin_tmp[2];
    u8 msgin = 0xff;

    u32 script[] = {
        /* select target, send scsi command */
        0x40000000 | ಗುರಿ << 16,  // select target
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
        0x03000001,                 //ಸ್ಥಿತಿ
        (u32)MAKE_FLATPTR(GET_SEG(SS), &ಸ್ಥಿತಿ_2),
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

    ಗಾಗಿ (;;) {
        u8 dstat = inb(iobase + LSI_REG_DSTAT);
        u8 sist0 = inb(iobase + LSI_REG_SIST0);
        u8 sist1 = inb(iobase + LSI_REG_SIST1);
        ಆದರೆ (sist0 || sist1) {
            ಹೋಗು fail;
        }
        ಆದರೆ (dstat & 0x04) {
            ನಿಲ್ಲಿಸು;
        }
        usleep(5);
    }

    ಆದರೆ (msgin == 0 && ಸ್ಥಿತಿ_2 == 0) {
        ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
    }

fail:
    ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
}

ಸ್ಥಿರ ಶೂನ್ಯ
lsi_scsi_init_lun(ರಚನೆ lsi_lun_s *llun, ರಚನೆ pci_device *pci, u32 iobase,
                  u8 ಗುರಿ, u8 lun)
{
    memset(llun, 0, ಗಾತ್ರ(*llun));
    llun->drive.type = DTYPE_LSI_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->ಗುರಿ = ಗುರಿ;
    llun->lun = lun;
    llun->iobase = iobase;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
lsi_scsi_ಕೂಡಿಸು_lun(u32 lun, ರಚನೆ drive_s *tmpl_drv)
{
    ರಚನೆ lsi_lun_s *tmpl_llun =
        container_ದ(tmpl_drv, ರಚನೆ lsi_lun_s, drive);
    ರಚನೆ lsi_lun_s *llun = malloc_fseg(ಗಾತ್ರ(*llun));
    ಆದರೆ (!llun) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ -1;
    }
    lsi_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->ಗುರಿ, lun);

    boot_lchs_ಕಂಡುಹಿಡಿ_scsi_device(llun->pci, llun->ಗುರಿ, llun->lun,
                               &(llun->drive.lchs));
    ಅಕ್ಷರ *name = znprintf(MAXDESCSIZE, "lsi %pP %d:%d",
                          llun->pci, llun->ಗುರಿ, llun->lun);
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
lsi_scsi_ಪರಿಶೀಲನೆ_ಗುರಿ(ರಚನೆ pci_device *pci, u32 iobase, u8 ಗುರಿ)
{
    ರಚನೆ lsi_lun_s llun0;

    lsi_scsi_init_lun(&llun0, pci, iobase, ಗುರಿ, 0);

    ಆದರೆ (scsi_rep_luns_ಪರಿಶೀಲನೆ(&llun0.drive, lsi_scsi_ಕೂಡಿಸು_lun) < 0)
        scsi_sequential_ಪರಿಶೀಲನೆ(&llun0.drive, 8, lsi_scsi_ಕೂಡಿಸು_lun);
}

ಸ್ಥಿರ ಶೂನ್ಯ
init_lsi_scsi(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ರಚನೆ pci_device *pci = ದತ್ತಾಂಶ;
    u32 iobase = pci_enable_iobar(pci, PCI_ತಳ_ವಿಳಾಸ_0);
    ಆದರೆ (!iobase)
        ಹಿಂತಿರುಗಿ;
    pci_enable_busmaster(pci);

    dprintf(1, "found lsi53c895a at %pP, io @ %x\n", pci, iobase);

    // reset
    outb(LSI_ISTAT0_SRST, iobase + LSI_REG_ISTAT0);

    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i = 0; i < 7; i++)
        lsi_scsi_ಪರಿಶೀಲನೆ_ಗುರಿ(pci, iobase, i);
}

ಶೂನ್ಯ
lsi_scsi_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಆದರೆ (!CONFIG_LSI_SCSI || !runningOnQEMU())
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "init lsi53c895a\n");

    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC
            || pci->device != PCI_DEVICE_ID_LSI_53C895A)
            ಮುಂದುವರಿ;
        ಸತತ_thread(init_lsi_scsi, pci);
    }
}

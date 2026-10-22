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

#include "biosvar.h" // GET_GLOBALFLAT
#include "block.h" // struct drive_s
#include "blockcmd.h" // scsi_drive_setup
#include "config.h" // CONFIG_*
#include "fw/paravirt.h" // runningOnQEMU
#include "malloc.h" // free
#include "output.h" // dprintf
#include "pcidevice.h" // foreachpci
#include "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#include "pci_regs.h" // PCI_VENDOR_ID
#include "stacks.h" // run_thread
#include "std/disk.h" // DISK_RET_SUCCESS
#include "string.h" // memset
#include "util.h" // usleep

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

struct lsi_lun_s {
    struct drive_s drive;
    struct pci_device *pci;
    u32 iobase;
    u8 target;
    u8 lun;
};

整数
lsi_scsi_process_op(struct disk_op_s *op)
{
    如果 (!CONFIG_LSI_SCSI)
        返回 DISK_RET_EBADTRACK;
    struct lsi_lun_s *llun_gf =
        container_of(op->drive_fl, struct lsi_lun_s, drive);
    u16 target = GET_GLOBALFLAT(llun_gf->target);
    u16 lun = GET_GLOBALFLAT(llun_gf->lun);
    u8 cdbcmd[16];
    整数 blocksize = scsi_fill_cmd(op, cdbcmd, sizeof(cdbcmd));
    如果 (blocksize < 0)
        返回 default_process_op(op);
    u32 iobase = GET_GLOBALFLAT(llun_gf->iobase);
    u32 dma = ((scsi_is_read(op) ? 0x01000000 : 0x00000000) |
               (op->count * blocksize));
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

    为 (;;) {
        u8 dstat = inb(iobase + LSI_REG_DSTAT);
        u8 sist0 = inb(iobase + LSI_REG_SIST0);
        u8 sist1 = inb(iobase + LSI_REG_SIST1);
        如果 (sist0 || sist1) {
            goto fail;
        }
        如果 (dstat & 0x04) {
            break;
        }
        usleep(5);
    }

    如果 (msgin == 0 && status == 0) {
        返回 DISK_RET_SUCCESS;
    }

fail:
    返回 DISK_RET_EBADTRACK;
}

static void
lsi_scsi_init_lun(struct lsi_lun_s *llun, struct pci_device *pci, u32 iobase,
                  u8 target, u8 lun)
{
    memset(llun, 0, sizeof(*llun));
    llun->drive.type = DTYPE_LSI_SCSI;
    llun->drive.cntl_id = pci->bdf;
    llun->pci = pci;
    llun->target = target;
    llun->lun = lun;
    llun->iobase = iobase;
}

static 整数
lsi_scsi_add_lun(u32 lun, struct drive_s *tmpl_drv)
{
    struct lsi_lun_s *tmpl_llun =
        container_of(tmpl_drv, struct lsi_lun_s, drive);
    struct lsi_lun_s *llun = malloc_fseg(sizeof(*llun));
    如果 (!llun) {
        warn_noalloc();
        返回 -1;
    }
    lsi_scsi_init_lun(llun, tmpl_llun->pci, tmpl_llun->iobase,
                      tmpl_llun->target, lun);

    boot_lchs_find_scsi_device(llun->pci, llun->target, llun->lun,
                               &(llun->drive.lchs));
    char *name = znprintf(MAXDESCSIZE, "lsi %pP %d:%d",
                          llun->pci, llun->target, llun->lun);
    整数 prio = bootprio_find_scsi_device(llun->pci, llun->target, llun->lun);
    整数 ret = scsi_drive_setup(&llun->drive, name, prio);
    free(name);
    如果 (ret)
        goto fail;
    返回 0;

fail:
    free(llun);
    返回 -1;
}

static void
lsi_scsi_scan_target(struct pci_device *pci, u32 iobase, u8 target)
{
    struct lsi_lun_s llun0;

    lsi_scsi_init_lun(&llun0, pci, iobase, target, 0);

    如果 (scsi_rep_luns_scan(&llun0.drive, lsi_scsi_add_lun) < 0)
        scsi_sequential_scan(&llun0.drive, 8, lsi_scsi_add_lun);
}

static void
init_lsi_scsi(void *data)
{
    struct pci_device *pci = data;
    u32 iobase = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
    如果 (!iobase)
        返回;
    pci_enable_busmaster(pci);

    dprintf(1, "found lsi53c895a at %pP, io @ %x\n", pci, iobase);

    // reset
    outb(LSI_ISTAT0_SRST, iobase + LSI_REG_ISTAT0);

    整数 i;
    为 (i = 0; i < 7; i++)
        lsi_scsi_scan_target(pci, iobase, i);
}

void
lsi_scsi_setup(void)
{
    ASSERT32FLAT();
    如果 (!CONFIG_LSI_SCSI || !runningOnQEMU())
        返回;

    dprintf(3, "init lsi53c895a\n");

    struct pci_device *pci;
    foreachpci(pci) {
        如果 (pci->vendor != PCI_VENDOR_ID_LSI_LOGIC
            || pci->device != PCI_DEVICE_ID_LSI_53C895A)
            continue;
        run_thread(init_lsi_scsi, pci);
    }
}

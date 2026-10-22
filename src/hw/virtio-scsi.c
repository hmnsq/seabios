// Virtio SCSI boot support.
//
// Copyright (C) 2012 Red Hat Inc.
//
// Authors:
//  Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাৱ "block.h" // struct drive_s
#অন্তর্ভাৱ "blockcmd.h" // scsi_drive_setup
#অন্তর্ভাৱ "config.h" // CONFIG_*
#অন্তর্ভাৱ "malloc.h" // free
#অন্তর্ভাৱ "output.h" // dprintf
#অন্তর্ভাৱ "pcidevice.h" // foreachpci
#অন্তর্ভাৱ "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#অন্তর্ভাৱ "pci_regs.h" // PCI_VENDOR_ID
#অন্তর্ভাৱ "stacks.h" // run_thread
#অন্তর্ভাৱ "std/disk.h" // DISK_RET_SUCCESS
#অন্তর্ভাৱ "string.h" // memset
#অন্তর্ভাৱ "util.h" // usleep, bootprio_find_pci_device, is_bootprio_strict
#অন্তর্ভাৱ "virtio-pci.h"
#অন্তর্ভাৱ "virtio-ring.h"
#অন্তর্ভাৱ "virtio-scsi.h"
#অন্তর্ভাৱ "virtio-mmio.h"

সংরচনা virtio_lun_s {
    সংরচনা drive_s drive;
    সংরচনা pci_device *pci;
    শূন্য *mmio;
    ৱর্ণ name[16];
    সংরচনা vring_virtqueue *vq;
    সংরচনা vp_device *vp;
    u16 target;
    u16 lun;
};

পূর্ণাঙ্ক
virtio_scsi_process_op(সংরচনা disk_op_s *op)
{
    যদি (! CONFIG_VIRTIO_SCSI)
        ঘূৰাই 0;
    সংরচনা virtio_lun_s *vlun =
        container_of(op->drive_fl, সংরচনা virtio_lun_s, drive);
    সংরচনা vp_device *vp = vlun->vp;
    সংরচনা vring_virtqueue *vq = vlun->vq;
    সংরচনা virtio_scsi_req_cmd req;
    সংরচনা virtio_scsi_resp_cmd resp;
    সংরচনা vring_list sg[3];

    memset(&req, 0, পরিমাণ(req));
    পূর্ণাঙ্ক blocksize = scsi_fill_cmd(op, req.cdb, 16);
    যদি (blocksize < 0)
        ঘূৰাই default_process_op(op);
    req.lun[0] = 1;
    req.lun[1] = vlun->target;
    req.lun[2] = (vlun->lun >> 8) | 0x40;
    req.lun[3] = (vlun->lun & 0xff);

    u32 len = op->গুনতি * blocksize;
    পূর্ণাঙ্ক datain = scsi_is_read(op);
    পূর্ণাঙ্ক in_অঙ্ক = (datain ? 2 : 1);
    পূর্ণাঙ্ক out_অঙ্ক = (len ? 3 : 2) - in_অঙ্ক;

    sg[0].addr   = (শূন্য*)(&req);
    sg[0].length = পরিমাণ(req);

    sg[out_অঙ্ক].addr   = (শূন্য*)(&resp);
    sg[out_অঙ্ক].length = পরিমাণ(resp);

    যদি (len) {
        পূর্ণাঙ্ক data_idx = (datain ? 2 : 1);
        sg[data_idx].addr   = op->buf_fl;
        sg[data_idx].length = len;
    }

    /* Add to virtqueue and kick host */
    vring_add_buf(vq, sg, out_অঙ্ক, in_অঙ্ক, 0, 0);
    vring_kick(vp, vq, 1);

    /* Wait for reply */
    যেতিয়ালৈ (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, রিক্ত);

    /* Clear interrupt status register.  Avoid leaving interrupts stuck if
     * VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
     */
    vp_get_isr(vp);

    যদি (resp.response == VIRTIO_SCSI_S_OK && resp.status == 0) {
        ঘূৰাই DISK_RET_SUCCESS;
    }
    ঘূৰাই DISK_RET_EBADTRACK;
}

স্থির শূন্য
virtio_scsi_init_lun(সংরচনা virtio_lun_s *vlun,
                     সংরচনা pci_device *pci, শূন্য *mmio,
                     সংরচনা vp_device *vp, সংরচনা vring_virtqueue *vq,
                     u16 target, u16 lun)
{
    memset(vlun, 0, পরিমাণ(*vlun));
    vlun->drive.type = DTYPE_VIRTIO_SCSI;
    vlun->drive.cntl_id = pci->bdf;
    vlun->pci = pci;
    vlun->mmio = mmio;
    vlun->vp = vp;
    vlun->vq = vq;
    vlun->target = target;
    vlun->lun = lun;
    যদি (vlun->pci)
        snprintf(vlun->name, পরিমাণ(vlun->name), "pci:%pP", vlun->pci);
    যদি (vlun->mmio)
        snprintf(vlun->name, পরিমাণ(vlun->name), "mmio:%08x", (u32)vlun->mmio);
}

স্থির পূর্ণাঙ্ক
virtio_scsi_add_lun(u32 lun, সংরচনা drive_s *tmpl_drv)
{
    u8 skip_nonbootable = is_bootprio_strict();
    সংরচনা virtio_lun_s *tmpl_vlun =
        container_of(tmpl_drv, সংরচনা virtio_lun_s, drive);
    পূর্ণাঙ্ক prio = -1;

    যদি (tmpl_vlun->pci)
        prio = bootprio_find_scsi_device(tmpl_vlun->pci, tmpl_vlun->target, lun);
    যদি (tmpl_vlun->mmio)
        prio = bootprio_find_scsi_mmio_device(tmpl_vlun->mmio, tmpl_vlun->target, lun);

    যদি (skip_nonbootable && prio < 0) {
        dprintf(1, "skipping init of a non-bootable virtio-scsi dev at %s,"
                " target %d, lun %d\n",
                tmpl_vlun->name, tmpl_vlun->target, lun);
        ঘূৰাই -1;
    }

    সংরচনা virtio_lun_s *vlun = malloc_নিচেৰ(পরিমাণ(*vlun));
    যদি (!vlun) {
        warn_noalloc();
        ঘূৰাই -1;
    }
    virtio_scsi_init_lun(vlun, tmpl_vlun->pci, tmpl_vlun->mmio,tmpl_vlun->vp,
                         tmpl_vlun->vq, tmpl_vlun->target, lun);

    যদি (vlun->pci)
        boot_lchs_find_scsi_device(vlun->pci, vlun->target, vlun->lun,
                                   &(vlun->drive.lchs));
    পূর্ণাঙ্ক ret = scsi_drive_setup(&vlun->drive, "virtio-scsi", prio);
    যদি (ret)
        গচ্ছ fail;
    ঘূৰাই 0;

fail:
    মুক্ত(vlun);
    ঘূৰাই -1;
}

স্থির পূর্ণাঙ্ক
virtio_scsi_scan_target(সংরচনা pci_device *pci, শূন্য *mmio, সংরচনা vp_device *vp,
                        সংরচনা vring_virtqueue *vq, u16 target)
{

    সংরচনা virtio_lun_s vlun0;

    virtio_scsi_init_lun(&vlun0, pci, mmio, vp, vq, target, 0);

    পূর্ণাঙ্ক ret = scsi_rep_luns_scan(&vlun0.drive, virtio_scsi_add_lun);
    ঘূৰাই ret < 0 ? 0 : ret;
}

স্থির শূন্য
init_virtio_scsi(শূন্য *data)
{
    সংরচনা pci_device *pci = data;
    dprintf(1, "found virtio-scsi at %pP\n", pci);
    সংরচনা vring_virtqueue *vq = রিক্ত;
    সংরচনা vp_device *vp = malloc_উপৰেৰ(পরিমাণ(*vp));
    যদি (!vp) {
        warn_noalloc();
        ঘূৰাই;
    }
    vp_init_simple(vp, pci);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    যদি (vp->use_modern) {
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        যদি (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            গচ্ছ fail;
        }

        vp_set_features(vp, features & (version1 | iommu_platform));
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        যদি (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            গচ্ছ fail;
        }
    }

    যদি (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi %pP\n", pci);
        গচ্ছ fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    পূর্ণাঙ্ক i, tot;
    বাবে (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(pci, রিক্ত, vp, vq, i);

    যদি (!tot)
        গচ্ছ fail;

    ঘূৰাই;

fail:
    vp_reset(vp);
    মুক্ত(vp);
    মুক্ত(vq);
}

শূন্য
init_virtio_scsi_mmio(শূন্য *mmio)
{
    dprintf(1, "found virtio-scsi-mmio at %p\n", mmio);
    সংরচনা vring_virtqueue *vq = রিক্ত;
    সংরচনা vp_device *vp = malloc_উপৰেৰ(পরিমাণ(*vp));
    যদি (!vp) {
        warn_noalloc();
        ঘূৰাই;
    }
    vp_init_mmio(vp, mmio);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_VERSION_1;
    যদি (features & version1) {
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;

        vp_set_features(vp, features & (version1 | iommu_platform));
        vp_set_status(vp, VIRTIO_CONFIG_S_FEATURES_OK);
        যদি (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", mmio);
            গচ্ছ fail;
        }
    }

    যদি (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi-mmio %p\n", mmio);
        গচ্ছ fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    পূর্ণাঙ্ক i, tot;
    বাবে (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(রিক্ত, mmio, vp, vq, i);

    যদি (!tot)
        গচ্ছ fail;

    ঘূৰাই;

fail:
    vp_reset(vp);
    মুক্ত(vp);
    মুক্ত(vq);
}

শূন্য
virtio_scsi_setup(শূন্য)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    যদি (! CONFIG_VIRTIO_SCSI)
        ঘূৰাই;

    dprintf(3, "init virtio-scsi\n");

    সংরচনা pci_device *pci;
    foreachpci(pci) {
        যদি (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_10))
            অনুৱর্ত;

        যদি (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-scsi at %pP\n",
                    pci);
            অনুৱর্ত;
        }

        run_thread(init_virtio_scsi, pci);
    }
}

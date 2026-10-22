// Virtio SCSI boot support.
//
// Copyright (C) 2012 Red Hat Inc.
//
// Authors:
//  Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "block.h" // struct drive_s
#অন্তর্ভাব "blockcmd.h" // scsi_drive_setup
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "pcidevice.h" // foreachpci
#অন্তর্ভাব "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#অন্তর্ভাব "pci_regs.h" // PCI_VENDOR_ID
#অন্তর্ভাব "stacks.h" // run_thread
#অন্তর্ভাব "std/disk.h" // DISK_RET_SUCCESS
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // usleep, bootprio_find_pci_device, is_bootprio_strict
#অন্তর্ভাব "virtio-pci.h"
#অন্তর্ভাব "virtio-ring.h"
#অন্তর্ভাব "virtio-scsi.h"
#অন্তর্ভাব "virtio-mmio.h"

কাঠামো virtio_lun_s {
    কাঠামো drive_s drive;
    কাঠামো pci_device *pci;
    শূন্য *mmio;
    অক্ষর name[16];
    কাঠামো vring_virtqueue *vq;
    কাঠামো vp_device *vp;
    u16 target;
    u16 lun;
};

পূর্ণসংখ্যা
virtio_scsi_process_op(কাঠামো disk_op_s *op)
{
    যদি (! CONFIG_VIRTIO_SCSI)
        ফেরত 0;
    কাঠামো virtio_lun_s *vlun =
        container_of(op->drive_fl, কাঠামো virtio_lun_s, drive);
    কাঠামো vp_device *vp = vlun->vp;
    কাঠামো vring_virtqueue *vq = vlun->vq;
    কাঠামো virtio_scsi_req_cmd req;
    কাঠামো virtio_scsi_resp_cmd resp;
    কাঠামো vring_list sg[3];

    memset(&req, 0, আকার(req));
    পূর্ণসংখ্যা blocksize = scsi_fill_cmd(op, req.cdb, 16);
    যদি (blocksize < 0)
        ফেরত default_process_op(op);
    req.lun[0] = 1;
    req.lun[1] = vlun->target;
    req.lun[2] = (vlun->lun >> 8) | 0x40;
    req.lun[3] = (vlun->lun & 0xff);

    u32 len = op->গুনতি * blocksize;
    পূর্ণসংখ্যা datain = scsi_is_read(op);
    পূর্ণসংখ্যা in_অঙ্ক = (datain ? 2 : 1);
    পূর্ণসংখ্যা out_অঙ্ক = (len ? 3 : 2) - in_অঙ্ক;

    sg[0].addr   = (শূন্য*)(&req);
    sg[0].দৈর্ঘ্য = আকার(req);

    sg[out_অঙ্ক].addr   = (শূন্য*)(&resp);
    sg[out_অঙ্ক].দৈর্ঘ্য = আকার(resp);

    যদি (len) {
        পূর্ণসংখ্যা তথ্য_idx = (datain ? 2 : 1);
        sg[তথ্য_idx].addr   = op->buf_fl;
        sg[তথ্য_idx].দৈর্ঘ্য = len;
    }

    /* Add to virtqueue and kick host */
    vring_add_buf(vq, sg, out_অঙ্ক, in_অঙ্ক, 0, 0);
    vring_kick(vp, vq, 1);

    /* Wait for reply */
    যখন (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, ফাঁকা);

    /* Clear interrupt status register.  Avoid leaving interrupts stuck if
     * VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
     */
    vp_get_isr(vp);

    যদি (resp.response == VIRTIO_SCSI_S_OK && resp.status == 0) {
        ফেরত DISK_RET_SUCCESS;
    }
    ফেরত DISK_RET_EBADTRACK;
}

স্থির শূন্য
virtio_scsi_init_lun(কাঠামো virtio_lun_s *vlun,
                     কাঠামো pci_device *pci, শূন্য *mmio,
                     কাঠামো vp_device *vp, কাঠামো vring_virtqueue *vq,
                     u16 target, u16 lun)
{
    memset(vlun, 0, আকার(*vlun));
    vlun->drive.type = DTYPE_VIRTIO_SCSI;
    vlun->drive.cntl_id = pci->bdf;
    vlun->pci = pci;
    vlun->mmio = mmio;
    vlun->vp = vp;
    vlun->vq = vq;
    vlun->target = target;
    vlun->lun = lun;
    যদি (vlun->pci)
        snprintf(vlun->name, আকার(vlun->name), "pci:%pP", vlun->pci);
    যদি (vlun->mmio)
        snprintf(vlun->name, আকার(vlun->name), "mmio:%08x", (u32)vlun->mmio);
}

স্থির পূর্ণসংখ্যা
virtio_scsi_add_lun(u32 lun, কাঠামো drive_s *tmpl_drv)
{
    u8 skip_nonbootable = is_bootprio_strict();
    কাঠামো virtio_lun_s *tmpl_vlun =
        container_of(tmpl_drv, কাঠামো virtio_lun_s, drive);
    পূর্ণসংখ্যা prio = -1;

    যদি (tmpl_vlun->pci)
        prio = bootprio_find_scsi_device(tmpl_vlun->pci, tmpl_vlun->target, lun);
    যদি (tmpl_vlun->mmio)
        prio = bootprio_find_scsi_mmio_device(tmpl_vlun->mmio, tmpl_vlun->target, lun);

    যদি (skip_nonbootable && prio < 0) {
        dprintf(1, "skipping init of a non-bootable virtio-scsi dev at %s,"
                " target %d, lun %d\n",
                tmpl_vlun->name, tmpl_vlun->target, lun);
        ফেরত -1;
    }

    কাঠামো virtio_lun_s *vlun = malloc_নিচের(আকার(*vlun));
    যদি (!vlun) {
        warn_noalloc();
        ফেরত -1;
    }
    virtio_scsi_init_lun(vlun, tmpl_vlun->pci, tmpl_vlun->mmio,tmpl_vlun->vp,
                         tmpl_vlun->vq, tmpl_vlun->target, lun);

    যদি (vlun->pci)
        boot_lchs_find_scsi_device(vlun->pci, vlun->target, vlun->lun,
                                   &(vlun->drive.lchs));
    পূর্ণসংখ্যা ret = scsi_drive_setup(&vlun->drive, "virtio-scsi", prio);
    যদি (ret)
        যাও fail;
    ফেরত 0;

fail:
    মুক্ত(vlun);
    ফেরত -1;
}

স্থির পূর্ণসংখ্যা
virtio_scsi_scan_target(কাঠামো pci_device *pci, শূন্য *mmio, কাঠামো vp_device *vp,
                        কাঠামো vring_virtqueue *vq, u16 target)
{

    কাঠামো virtio_lun_s vlun0;

    virtio_scsi_init_lun(&vlun0, pci, mmio, vp, vq, target, 0);

    পূর্ণসংখ্যা ret = scsi_rep_luns_scan(&vlun0.drive, virtio_scsi_add_lun);
    ফেরত ret < 0 ? 0 : ret;
}

স্থির শূন্য
init_virtio_scsi(শূন্য *তথ্য)
{
    কাঠামো pci_device *pci = তথ্য;
    dprintf(1, "found virtio-scsi at %pP\n", pci);
    কাঠামো vring_virtqueue *vq = ফাঁকা;
    কাঠামো vp_device *vp = malloc_উপরের(আকার(*vp));
    যদি (!vp) {
        warn_noalloc();
        ফেরত;
    }
    vp_init_simple(vp, pci);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    যদি (vp->use_modern) {
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        যদি (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            যাও fail;
        }

        vp_set_features(vp, features & (version1 | iommu_platform));
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        যদি (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            যাও fail;
        }
    }

    যদি (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi %pP\n", pci);
        যাও fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    পূর্ণসংখ্যা i, tot;
    জন্য (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(pci, ফাঁকা, vp, vq, i);

    যদি (!tot)
        যাও fail;

    ফেরত;

fail:
    vp_reset(vp);
    মুক্ত(vp);
    মুক্ত(vq);
}

শূন্য
init_virtio_scsi_mmio(শূন্য *mmio)
{
    dprintf(1, "found virtio-scsi-mmio at %p\n", mmio);
    কাঠামো vring_virtqueue *vq = ফাঁকা;
    কাঠামো vp_device *vp = malloc_উপরের(আকার(*vp));
    যদি (!vp) {
        warn_noalloc();
        ফেরত;
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
            যাও fail;
        }
    }

    যদি (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi-mmio %p\n", mmio);
        যাও fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    পূর্ণসংখ্যা i, tot;
    জন্য (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(ফাঁকা, mmio, vp, vq, i);

    যদি (!tot)
        যাও fail;

    ফেরত;

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
        ফেরত;

    dprintf(3, "init virtio-scsi\n");

    কাঠামো pci_device *pci;
    foreachpci(pci) {
        যদি (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_10))
            চালু;

        যদি (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-scsi at %pP\n",
                    pci);
            চালু;
        }

        run_thread(init_virtio_scsi, pci);
    }
}

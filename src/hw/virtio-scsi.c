// Virtio SCSI boot support.
//
// Copyright (C) 2012 Red Hat Inc.
//
// Authors:
//  Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "block.h" // struct drive_s
#شامل "blockcmd.h" // scsi_drive_setup
#شامل "config.h" // CONFIG_*
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "pcidevice.h" // foreachpci
#شامل "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#شامل "pci_regs.h" // PCI_VENDOR_ID
#شامل "stacks.h" // run_thread
#شامل "std/disk.h" // DISK_RET_SUCCESS
#شامل "string.h" // memset
#شامل "util.h" // usleep, bootprio_find_pci_device, is_bootprio_strict
#شامل "virtio-pci.h"
#شامل "virtio-ring.h"
#شامل "virtio-scsi.h"
#شامل "virtio-mmio.h"

ساختار virtio_lun_s {
    ساختار drive_s drive;
    ساختار pci_device *pci;
    تهی *mmio;
    نویسه name[16];
    ساختار vring_virtqueue *vq;
    ساختار vp_device *vp;
    u16 target;
    u16 lun;
};

صحیح
virtio_scsi_process_op(ساختار disk_op_s *op)
{
    اگر (! CONFIG_VIRTIO_SCSI)
        بازگشت 0;
    ساختار virtio_lun_s *vlun =
        container_of(op->drive_fl, ساختار virtio_lun_s, drive);
    ساختار vp_device *vp = vlun->vp;
    ساختار vring_virtqueue *vq = vlun->vq;
    ساختار virtio_scsi_req_cmd req;
    ساختار virtio_scsi_resp_cmd resp;
    ساختار vring_list sg[3];

    memset(&req, 0, اندازه(req));
    صحیح blocksize = scsi_fill_cmd(op, req.cdb, 16);
    اگر (blocksize < 0)
        بازگشت default_process_op(op);
    req.lun[0] = 1;
    req.lun[1] = vlun->target;
    req.lun[2] = (vlun->lun >> 8) | 0x40;
    req.lun[3] = (vlun->lun & 0xff);

    u32 len = op->شمار * blocksize;
    صحیح datain = scsi_is_read(op);
    صحیح in_رقم = (datain ? 2 : 1);
    صحیح out_رقم = (len ? 3 : 2) - in_رقم;

    sg[0].addr   = (تهی*)(&req);
    sg[0].length = اندازه(req);

    sg[out_رقم].addr   = (تهی*)(&resp);
    sg[out_رقم].length = اندازه(resp);

    اگر (len) {
        صحیح data_idx = (datain ? 2 : 1);
        sg[data_idx].addr   = op->buf_fl;
        sg[data_idx].length = len;
    }

    /* Add to virtqueue and kick host */
    vring_add_buf(vq, sg, out_رقم, in_رقم, 0, 0);
    vring_kick(vp, vq, 1);

    /* Wait for reply */
    وقتی (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, پوچ);

    /* Clear interrupt status register.  Avoid leaving interrupts stuck if
     * VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
     */
    vp_get_isr(vp);

    اگر (resp.response == VIRTIO_SCSI_S_OK && resp.status == 0) {
        بازگشت DISK_RET_SUCCESS;
    }
    بازگشت DISK_RET_EBADTRACK;
}

ایستا تهی
virtio_scsi_init_lun(ساختار virtio_lun_s *vlun,
                     ساختار pci_device *pci, تهی *mmio,
                     ساختار vp_device *vp, ساختار vring_virtqueue *vq,
                     u16 target, u16 lun)
{
    memset(vlun, 0, اندازه(*vlun));
    vlun->drive.type = DTYPE_VIRTIO_SCSI;
    vlun->drive.cntl_id = pci->bdf;
    vlun->pci = pci;
    vlun->mmio = mmio;
    vlun->vp = vp;
    vlun->vq = vq;
    vlun->target = target;
    vlun->lun = lun;
    اگر (vlun->pci)
        snprintf(vlun->name, اندازه(vlun->name), "pci:%pP", vlun->pci);
    اگر (vlun->mmio)
        snprintf(vlun->name, اندازه(vlun->name), "mmio:%08x", (u32)vlun->mmio);
}

ایستا صحیح
virtio_scsi_add_lun(u32 lun, ساختار drive_s *tmpl_drv)
{
    u8 skip_nonbootable = is_bootprio_strict();
    ساختار virtio_lun_s *tmpl_vlun =
        container_of(tmpl_drv, ساختار virtio_lun_s, drive);
    صحیح prio = -1;

    اگر (tmpl_vlun->pci)
        prio = bootprio_find_scsi_device(tmpl_vlun->pci, tmpl_vlun->target, lun);
    اگر (tmpl_vlun->mmio)
        prio = bootprio_find_scsi_mmio_device(tmpl_vlun->mmio, tmpl_vlun->target, lun);

    اگر (skip_nonbootable && prio < 0) {
        dprintf(1, "skipping init of a non-bootable virtio-scsi dev at %s,"
                " target %d, lun %d\n",
                tmpl_vlun->name, tmpl_vlun->target, lun);
        بازگشت -1;
    }

    ساختار virtio_lun_s *vlun = malloc_کم(اندازه(*vlun));
    اگر (!vlun) {
        warn_noalloc();
        بازگشت -1;
    }
    virtio_scsi_init_lun(vlun, tmpl_vlun->pci, tmpl_vlun->mmio,tmpl_vlun->vp,
                         tmpl_vlun->vq, tmpl_vlun->target, lun);

    اگر (vlun->pci)
        boot_lchs_find_scsi_device(vlun->pci, vlun->target, vlun->lun,
                                   &(vlun->drive.lchs));
    صحیح ret = scsi_drive_setup(&vlun->drive, "virtio-scsi", prio);
    اگر (ret)
        برو fail;
    بازگشت 0;

fail:
    آزاد(vlun);
    بازگشت -1;
}

ایستا صحیح
virtio_scsi_scan_target(ساختار pci_device *pci, تهی *mmio, ساختار vp_device *vp,
                        ساختار vring_virtqueue *vq, u16 target)
{

    ساختار virtio_lun_s vlun0;

    virtio_scsi_init_lun(&vlun0, pci, mmio, vp, vq, target, 0);

    صحیح ret = scsi_rep_luns_scan(&vlun0.drive, virtio_scsi_add_lun);
    بازگشت ret < 0 ? 0 : ret;
}

ایستا تهی
init_virtio_scsi(تهی *data)
{
    ساختار pci_device *pci = data;
    dprintf(1, "found virtio-scsi at %pP\n", pci);
    ساختار vring_virtqueue *vq = پوچ;
    ساختار vp_device *vp = malloc_بالا(اندازه(*vp));
    اگر (!vp) {
        warn_noalloc();
        بازگشت;
    }
    vp_init_simple(vp, pci);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    اگر (vp->use_modern) {
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        اگر (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            برو fail;
        }

        vp_set_features(vp, features & (version1 | iommu_platform));
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        اگر (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            برو fail;
        }
    }

    اگر (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi %pP\n", pci);
        برو fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    صحیح i, tot;
    برای (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(pci, پوچ, vp, vq, i);

    اگر (!tot)
        برو fail;

    بازگشت;

fail:
    vp_reset(vp);
    آزاد(vp);
    آزاد(vq);
}

تهی
init_virtio_scsi_mmio(تهی *mmio)
{
    dprintf(1, "found virtio-scsi-mmio at %p\n", mmio);
    ساختار vring_virtqueue *vq = پوچ;
    ساختار vp_device *vp = malloc_بالا(اندازه(*vp));
    اگر (!vp) {
        warn_noalloc();
        بازگشت;
    }
    vp_init_mmio(vp, mmio);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_VERSION_1;
    اگر (features & version1) {
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;

        vp_set_features(vp, features & (version1 | iommu_platform));
        vp_set_status(vp, VIRTIO_CONFIG_S_FEATURES_OK);
        اگر (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", mmio);
            برو fail;
        }
    }

    اگر (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi-mmio %p\n", mmio);
        برو fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    صحیح i, tot;
    برای (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(پوچ, mmio, vp, vq, i);

    اگر (!tot)
        برو fail;

    بازگشت;

fail:
    vp_reset(vp);
    آزاد(vp);
    آزاد(vq);
}

تهی
virtio_scsi_setup(تهی)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    اگر (! CONFIG_VIRTIO_SCSI)
        بازگشت;

    dprintf(3, "init virtio-scsi\n");

    ساختار pci_device *pci;
    foreachpci(pci) {
        اگر (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_10))
            ادامه;

        اگر (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-scsi at %pP\n",
                    pci);
            ادامه;
        }

        run_thread(init_virtio_scsi, pci);
    }
}

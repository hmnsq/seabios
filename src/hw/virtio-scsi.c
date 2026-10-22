// Virtio SCSI boot support.
//
// Copyright (C) 2012 Red Hat Inc.
//
// Authors:
//  Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "block.h" // struct drive_s
#ಅನ್ತರ್ಭಾವ "blockcmd.h" // scsi_drive_setup
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_VENDOR_ID
#ಅನ್ತರ್ಭಾವ "stacks.h" // run_thread
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_SUCCESS
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // usleep, bootprio_find_pci_device, is_bootprio_strict
#ಅನ್ತರ್ಭಾವ "virtio-pci.h"
#ಅನ್ತರ್ಭಾವ "virtio-ring.h"
#ಅನ್ತರ್ಭಾವ "virtio-scsi.h"
#ಅನ್ತರ್ಭಾವ "virtio-mmio.h"

ಸಂರಚನಾ virtio_lun_s {
    ಸಂರಚನಾ drive_s drive;
    ಸಂರಚನಾ pci_device *pci;
    ಶೂನ್ಯ *mmio;
    ವರ್ಣ name[16];
    ಸಂರಚನಾ vring_virtqueue *vq;
    ಸಂರಚನಾ vp_device *vp;
    u16 target;
    u16 lun;
};

ಪೂರ್ಣಾಙ್ಕ
virtio_scsi_process_op(ಸಂರಚನಾ disk_op_s *op)
{
    ಯದಿ (! CONFIG_VIRTIO_SCSI)
        ಪ್ರತ್ಯಾಗಮ 0;
    ಸಂರಚನಾ virtio_lun_s *vlun =
        container_of(op->drive_fl, ಸಂರಚನಾ virtio_lun_s, drive);
    ಸಂರಚನಾ vp_device *vp = vlun->vp;
    ಸಂರಚನಾ vring_virtqueue *vq = vlun->vq;
    ಸಂರಚನಾ virtio_scsi_req_cmd req;
    ಸಂರಚನಾ virtio_scsi_resp_cmd resp;
    ಸಂರಚನಾ vring_list sg[3];

    memset(&req, 0, ಪರಿಮಾಣ(req));
    ಪೂರ್ಣಾಙ್ಕ blocksize = scsi_fill_cmd(op, req.cdb, 16);
    ಯದಿ (blocksize < 0)
        ಪ್ರತ್ಯಾಗಮ default_process_op(op);
    req.lun[0] = 1;
    req.lun[1] = vlun->target;
    req.lun[2] = (vlun->lun >> 8) | 0x40;
    req.lun[3] = (vlun->lun & 0xff);

    u32 len = op->ಎಣಿಕೆ * blocksize;
    ಪೂರ್ಣಾಙ್ಕ datain = scsi_is_read(op);
    ಪೂರ್ಣಾಙ್ಕ in_ಅಂಕೆ = (datain ? 2 : 1);
    ಪೂರ್ಣಾಙ್ಕ out_ಅಂಕೆ = (len ? 3 : 2) - in_ಅಂಕೆ;

    sg[0].addr   = (ಶೂನ್ಯ*)(&req);
    sg[0].length = ಪರಿಮಾಣ(req);

    sg[out_ಅಂಕೆ].addr   = (ಶೂನ್ಯ*)(&resp);
    sg[out_ಅಂಕೆ].length = ಪರಿಮಾಣ(resp);

    ಯದಿ (len) {
        ಪೂರ್ಣಾಙ್ಕ data_idx = (datain ? 2 : 1);
        sg[data_idx].addr   = op->buf_fl;
        sg[data_idx].length = len;
    }

    /* Add to virtqueue and kick host */
    vring_add_buf(vq, sg, out_ಅಂಕೆ, in_ಅಂಕೆ, 0, 0);
    vring_kick(vp, vq, 1);

    /* Wait for reply */
    ಯಾವತ್ (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, ರಿಕ್ತ);

    /* Clear interrupt status register.  Avoid leaving interrupts stuck if
     * VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
     */
    vp_get_isr(vp);

    ಯದಿ (resp.response == VIRTIO_SCSI_S_OK && resp.status == 0) {
        ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;
    }
    ಪ್ರತ್ಯಾಗಮ DISK_RET_EBADTRACK;
}

ಸ್ಥಿರ ಶೂನ್ಯ
virtio_scsi_init_lun(ಸಂರಚನಾ virtio_lun_s *vlun,
                     ಸಂರಚನಾ pci_device *pci, ಶೂನ್ಯ *mmio,
                     ಸಂರಚನಾ vp_device *vp, ಸಂರಚನಾ vring_virtqueue *vq,
                     u16 target, u16 lun)
{
    memset(vlun, 0, ಪರಿಮಾಣ(*vlun));
    vlun->drive.type = DTYPE_VIRTIO_SCSI;
    vlun->drive.cntl_id = pci->bdf;
    vlun->pci = pci;
    vlun->mmio = mmio;
    vlun->vp = vp;
    vlun->vq = vq;
    vlun->target = target;
    vlun->lun = lun;
    ಯದಿ (vlun->pci)
        snprintf(vlun->name, ಪರಿಮಾಣ(vlun->name), "pci:%pP", vlun->pci);
    ಯದಿ (vlun->mmio)
        snprintf(vlun->name, ಪರಿಮಾಣ(vlun->name), "mmio:%08x", (u32)vlun->mmio);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
virtio_scsi_add_lun(u32 lun, ಸಂರಚನಾ drive_s *tmpl_drv)
{
    u8 skip_nonbootable = is_bootprio_strict();
    ಸಂರಚನಾ virtio_lun_s *tmpl_vlun =
        container_of(tmpl_drv, ಸಂರಚನಾ virtio_lun_s, drive);
    ಪೂರ್ಣಾಙ್ಕ prio = -1;

    ಯದಿ (tmpl_vlun->pci)
        prio = bootprio_find_scsi_device(tmpl_vlun->pci, tmpl_vlun->target, lun);
    ಯದಿ (tmpl_vlun->mmio)
        prio = bootprio_find_scsi_mmio_device(tmpl_vlun->mmio, tmpl_vlun->target, lun);

    ಯದಿ (skip_nonbootable && prio < 0) {
        dprintf(1, "skipping init of a non-bootable virtio-scsi dev at %s,"
                " target %d, lun %d\n",
                tmpl_vlun->name, tmpl_vlun->target, lun);
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    ಸಂರಚನಾ virtio_lun_s *vlun = malloc_ಕೆಳಗಿನ(ಪರಿಮಾಣ(*vlun));
    ಯದಿ (!vlun) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ -1;
    }
    virtio_scsi_init_lun(vlun, tmpl_vlun->pci, tmpl_vlun->mmio,tmpl_vlun->vp,
                         tmpl_vlun->vq, tmpl_vlun->target, lun);

    ಯದಿ (vlun->pci)
        boot_lchs_find_scsi_device(vlun->pci, vlun->target, vlun->lun,
                                   &(vlun->drive.lchs));
    ಪೂರ್ಣಾಙ್ಕ ret = scsi_drive_setup(&vlun->drive, "virtio-scsi", prio);
    ಯದಿ (ret)
        ಗಚ್ಛ fail;
    ಪ್ರತ್ಯಾಗಮ 0;

fail:
    ಮುಕ್ತ(vlun);
    ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
virtio_scsi_scan_target(ಸಂರಚನಾ pci_device *pci, ಶೂನ್ಯ *mmio, ಸಂರಚನಾ vp_device *vp,
                        ಸಂರಚನಾ vring_virtqueue *vq, u16 target)
{

    ಸಂರಚನಾ virtio_lun_s vlun0;

    virtio_scsi_init_lun(&vlun0, pci, mmio, vp, vq, target, 0);

    ಪೂರ್ಣಾಙ್ಕ ret = scsi_rep_luns_scan(&vlun0.drive, virtio_scsi_add_lun);
    ಪ್ರತ್ಯಾಗಮ ret < 0 ? 0 : ret;
}

ಸ್ಥಿರ ಶೂನ್ಯ
init_virtio_scsi(ಶೂನ್ಯ *data)
{
    ಸಂರಚನಾ pci_device *pci = data;
    dprintf(1, "found virtio-scsi at %pP\n", pci);
    ಸಂರಚನಾ vring_virtqueue *vq = ರಿಕ್ತ;
    ಸಂರಚನಾ vp_device *vp = malloc_ಮೇಲಿನ(ಪರಿಮಾಣ(*vp));
    ಯದಿ (!vp) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    vp_init_simple(vp, pci);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    ಯದಿ (vp->use_modern) {
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        ಯದಿ (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            ಗಚ್ಛ fail;
        }

        vp_set_features(vp, features & (version1 | iommu_platform));
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        ಯದಿ (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            ಗಚ್ಛ fail;
        }
    }

    ಯದಿ (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi %pP\n", pci);
        ಗಚ್ಛ fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    ಪೂರ್ಣಾಙ್ಕ i, tot;
    ಪ್ರತಿ (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(pci, ರಿಕ್ತ, vp, vq, i);

    ಯದಿ (!tot)
        ಗಚ್ಛ fail;

    ಪ್ರತ್ಯಾಗಮ;

fail:
    vp_reset(vp);
    ಮುಕ್ತ(vp);
    ಮುಕ್ತ(vq);
}

ಶೂನ್ಯ
init_virtio_scsi_mmio(ಶೂನ್ಯ *mmio)
{
    dprintf(1, "found virtio-scsi-mmio at %p\n", mmio);
    ಸಂರಚನಾ vring_virtqueue *vq = ರಿಕ್ತ;
    ಸಂರಚನಾ vp_device *vp = malloc_ಮೇಲಿನ(ಪರಿಮಾಣ(*vp));
    ಯದಿ (!vp) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    vp_init_mmio(vp, mmio);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_VERSION_1;
    ಯದಿ (features & version1) {
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;

        vp_set_features(vp, features & (version1 | iommu_platform));
        vp_set_status(vp, VIRTIO_CONFIG_S_FEATURES_OK);
        ಯದಿ (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", mmio);
            ಗಚ್ಛ fail;
        }
    }

    ಯದಿ (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi-mmio %p\n", mmio);
        ಗಚ್ಛ fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    ಪೂರ್ಣಾಙ್ಕ i, tot;
    ಪ್ರತಿ (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(ರಿಕ್ತ, mmio, vp, vq, i);

    ಯದಿ (!tot)
        ಗಚ್ಛ fail;

    ಪ್ರತ್ಯಾಗಮ;

fail:
    vp_reset(vp);
    ಮುಕ್ತ(vp);
    ಮುಕ್ತ(vq);
}

ಶೂನ್ಯ
virtio_scsi_setup(ಶೂನ್ಯ)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    ಯದಿ (! CONFIG_VIRTIO_SCSI)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(3, "init virtio-scsi\n");

    ಸಂರಚನಾ pci_device *pci;
    foreachpci(pci) {
        ಯದಿ (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_10))
            ಅನುವರ್ತ;

        ಯದಿ (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-scsi at %pP\n",
                    pci);
            ಅನುವರ್ತ;
        }

        run_thread(init_virtio_scsi, pci);
    }
}

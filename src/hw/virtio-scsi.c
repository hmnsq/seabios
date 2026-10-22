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

ರಚನೆ virtio_lun_s {
    ರಚನೆ drive_s drive;
    ರಚನೆ pci_device *pci;
    ಶೂನ್ಯ *mmio;
    ಅಕ್ಷರ name[16];
    ರಚನೆ vring_virtqueue *vq;
    ರಚನೆ vp_device *vp;
    u16 ಗುರಿ;
    u16 lun;
};

ಪೂರ್ಣಾಂಕ
virtio_scsi_process_op(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (! CONFIG_VIRTIO_SCSI)
        ಹಿಂತಿರುಗಿ 0;
    ರಚನೆ virtio_lun_s *vlun =
        container_ದ(op->drive_fl, ರಚನೆ virtio_lun_s, drive);
    ರಚನೆ vp_device *vp = vlun->vp;
    ರಚನೆ vring_virtqueue *vq = vlun->vq;
    ರಚನೆ virtio_scsi_req_cmd req;
    ರಚನೆ virtio_scsi_resp_cmd resp;
    ರಚನೆ vring_list sg[3];

    memset(&req, 0, ಗಾತ್ರ(req));
    ಪೂರ್ಣಾಂಕ blocksize = scsi_fill_cmd(op, req.cdb, 16);
    ಆದರೆ (blocksize < 0)
        ಹಿಂತಿರುಗಿ default_process_op(op);
    req.lun[0] = 1;
    req.lun[1] = vlun->ಗುರಿ;
    req.lun[2] = (vlun->lun >> 8) | 0x40;
    req.lun[3] = (vlun->lun & 0xff);

    u32 len = op->ಎಣಿಕೆ_2 * blocksize;
    ಪೂರ್ಣಾಂಕ datain = scsi_is_read(op);
    ಪೂರ್ಣಾಂಕ ಒಳ_ಅಂಕೆ = (datain ? 2 : 1);
    ಪೂರ್ಣಾಂಕ ಹೊರಹೋಗು_ಅಂಕೆ = (len ? 3 : 2) - ಒಳ_ಅಂಕೆ;

    sg[0].addr   = (ಶೂನ್ಯ*)(&req);
    sg[0].ಉದ್ದಳತೆ = ಗಾತ್ರ(req);

    sg[ಹೊರಹೋಗು_ಅಂಕೆ].addr   = (ಶೂನ್ಯ*)(&resp);
    sg[ಹೊರಹೋಗು_ಅಂಕೆ].ಉದ್ದಳತೆ = ಗಾತ್ರ(resp);

    ಆದರೆ (len) {
        ಪೂರ್ಣಾಂಕ ದತ್ತಾಂಶ_idx = (datain ? 2 : 1);
        sg[ದತ್ತಾಂಶ_idx].addr   = op->buf_fl;
        sg[ದತ್ತಾಂಶ_idx].ಉದ್ದಳತೆ = len;
    }

    /* Add to virtqueue and kick host */
    vring_ಕೂಡಿಸು_buf(vq, sg, ಹೊರಹೋಗು_ಅಂಕೆ, ಒಳ_ಅಂಕೆ, 0, 0);
    vring_kick(vp, vq, 1);

    /* Wait for reply */
    ಆಗ (!vring_ಇನ್ನಷ್ಟು_ಬಳಸಿದ(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, ಖಾಲಿ);

    /* Clear interrupt status register.  Avoid leaving interrupts stuck if
     * VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
     */
    vp_get_isr(vp);

    ಆದರೆ (resp.response == VIRTIO_SCSI_S_OK && resp.ಸ್ಥಿತಿ_2 == 0) {
        ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
    }
    ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
}

ಸ್ಥಿರ ಶೂನ್ಯ
virtio_scsi_init_lun(ರಚನೆ virtio_lun_s *vlun,
                     ರಚನೆ pci_device *pci, ಶೂನ್ಯ *mmio,
                     ರಚನೆ vp_device *vp, ರಚನೆ vring_virtqueue *vq,
                     u16 ಗುರಿ, u16 lun)
{
    memset(vlun, 0, ಗಾತ್ರ(*vlun));
    vlun->drive.type = DTYPE_VIRTIO_SCSI;
    vlun->drive.cntl_id = pci->bdf;
    vlun->pci = pci;
    vlun->mmio = mmio;
    vlun->vp = vp;
    vlun->vq = vq;
    vlun->ಗುರಿ = ಗುರಿ;
    vlun->lun = lun;
    ಆದರೆ (vlun->pci)
        snprintf(vlun->name, ಗಾತ್ರ(vlun->name), "pci:%pP", vlun->pci);
    ಆದರೆ (vlun->mmio)
        snprintf(vlun->name, ಗಾತ್ರ(vlun->name), "mmio:%08x", (u32)vlun->mmio);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
virtio_scsi_ಕೂಡಿಸು_lun(u32 lun, ರಚನೆ drive_s *tmpl_drv)
{
    u8 skip_nonbootable = is_bootprio_strict();
    ರಚನೆ virtio_lun_s *tmpl_vlun =
        container_ದ(tmpl_drv, ರಚನೆ virtio_lun_s, drive);
    ಪೂರ್ಣಾಂಕ prio = -1;

    ಆದರೆ (tmpl_vlun->pci)
        prio = bootprio_ಕಂಡುಹಿಡಿ_scsi_device(tmpl_vlun->pci, tmpl_vlun->ಗುರಿ, lun);
    ಆದರೆ (tmpl_vlun->mmio)
        prio = bootprio_ಕಂಡುಹಿಡಿ_scsi_mmio_device(tmpl_vlun->mmio, tmpl_vlun->ಗುರಿ, lun);

    ಆದರೆ (skip_nonbootable && prio < 0) {
        dprintf(1, "skipping init of a non-bootable virtio-scsi dev at %s,"
                " target %d, lun %d\n",
                tmpl_vlun->name, tmpl_vlun->ಗುರಿ, lun);
        ಹಿಂತಿರುಗಿ -1;
    }

    ರಚನೆ virtio_lun_s *vlun = malloc_ಕೆಳಗಿನ(ಗಾತ್ರ(*vlun));
    ಆದರೆ (!vlun) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ -1;
    }
    virtio_scsi_init_lun(vlun, tmpl_vlun->pci, tmpl_vlun->mmio,tmpl_vlun->vp,
                         tmpl_vlun->vq, tmpl_vlun->ಗುರಿ, lun);

    ಆದರೆ (vlun->pci)
        boot_lchs_ಕಂಡುಹಿಡಿ_scsi_device(vlun->pci, vlun->ಗುರಿ, vlun->lun,
                                   &(vlun->drive.lchs));
    ಪೂರ್ಣಾಂಕ ret = scsi_drive_ಸಿದ್ಧತೆ(&vlun->drive, "virtio-scsi", prio);
    ಆದರೆ (ret)
        ಹೋಗು fail;
    ಹಿಂತಿರುಗಿ 0;

fail:
    ಬಿಡುಗಡೆ(vlun);
    ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
virtio_scsi_ಪರಿಶೀಲನೆ_ಗುರಿ(ರಚನೆ pci_device *pci, ಶೂನ್ಯ *mmio, ರಚನೆ vp_device *vp,
                        ರಚನೆ vring_virtqueue *vq, u16 ಗುರಿ)
{

    ರಚನೆ virtio_lun_s vlun0;

    virtio_scsi_init_lun(&vlun0, pci, mmio, vp, vq, ಗುರಿ, 0);

    ಪೂರ್ಣಾಂಕ ret = scsi_rep_luns_ಪರಿಶೀಲನೆ(&vlun0.drive, virtio_scsi_ಕೂಡಿಸು_lun);
    ಹಿಂತಿರುಗಿ ret < 0 ? 0 : ret;
}

ಸ್ಥಿರ ಶೂನ್ಯ
init_virtio_scsi(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ರಚನೆ pci_device *pci = ದತ್ತಾಂಶ;
    dprintf(1, "found virtio-scsi at %pP\n", pci);
    ರಚನೆ vring_virtqueue *vq = ಖಾಲಿ;
    ರಚನೆ vp_device *vp = malloc_ಮೇಲಿನ(ಗಾತ್ರ(*vp));
    ಆದರೆ (!vp) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    vp_init_simple(vp, pci);
    u8 ಸ್ಥಿತಿ_2 = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    ಆದರೆ (vp->use_modern) {
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_ಆವೃತ್ತಿ_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        ಆದರೆ (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            ಹೋಗು fail;
        }

        vp_ಗಣ_features(vp, features & (version1 | iommu_platform));
        ಸ್ಥಿತಿ_2 |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_ಗಣ_ಸ್ಥಿತಿ(vp, ಸ್ಥಿತಿ_2);
        ಆದರೆ (!(vp_get_ಸ್ಥಿತಿ(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            ಹೋಗು fail;
        }
    }

    ಆದರೆ (vp_ಕಂಡುಹಿಡಿ_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi %pP\n", pci);
        ಹೋಗು fail;
    }

    ಸ್ಥಿತಿ_2 |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_ಗಣ_ಸ್ಥಿತಿ(vp, ಸ್ಥಿತಿ_2);

    ಪೂರ್ಣಾಂಕ i, tot;
    ಗಾಗಿ (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_ಪರಿಶೀಲನೆ_ಗುರಿ(pci, ಖಾಲಿ, vp, vq, i);

    ಆದರೆ (!tot)
        ಹೋಗು fail;

    ಹಿಂತಿರುಗಿ;

fail:
    vp_reset(vp);
    ಬಿಡುಗಡೆ(vp);
    ಬಿಡುಗಡೆ(vq);
}

ಶೂನ್ಯ
init_virtio_scsi_mmio(ಶೂನ್ಯ *mmio)
{
    dprintf(1, "found virtio-scsi-mmio at %p\n", mmio);
    ರಚನೆ vring_virtqueue *vq = ಖಾಲಿ;
    ರಚನೆ vp_device *vp = malloc_ಮೇಲಿನ(ಗಾತ್ರ(*vp));
    ಆದರೆ (!vp) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    vp_init_mmio(vp, mmio);
    u8 ಸ್ಥಿತಿ_2 = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_ಆವೃತ್ತಿ_1;
    ಆದರೆ (features & version1) {
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;

        vp_ಗಣ_features(vp, features & (version1 | iommu_platform));
        vp_ಗಣ_ಸ್ಥಿತಿ(vp, VIRTIO_CONFIG_S_FEATURES_OK);
        ಆದರೆ (!(vp_get_ಸ್ಥಿತಿ(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", mmio);
            ಹೋಗು fail;
        }
    }

    ಆದರೆ (vp_ಕಂಡುಹಿಡಿ_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi-mmio %p\n", mmio);
        ಹೋಗು fail;
    }

    ಸ್ಥಿತಿ_2 |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_ಗಣ_ಸ್ಥಿತಿ(vp, ಸ್ಥಿತಿ_2);

    ಪೂರ್ಣಾಂಕ i, tot;
    ಗಾಗಿ (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_ಪರಿಶೀಲನೆ_ಗುರಿ(ಖಾಲಿ, mmio, vp, vq, i);

    ಆದರೆ (!tot)
        ಹೋಗು fail;

    ಹಿಂತಿರುಗಿ;

fail:
    vp_reset(vp);
    ಬಿಡುಗಡೆ(vp);
    ಬಿಡುಗಡೆ(vq);
}

ಶೂನ್ಯ
virtio_scsi_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    ಆದರೆ (! CONFIG_VIRTIO_SCSI)
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "init virtio-scsi\n");

    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_10))
            ಮುಂದುವರಿ;

        ಆದರೆ (skip_nonbootable && bootprio_ಕಂಡುಹಿಡಿ_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-scsi at %pP\n",
                    pci);
            ಮುಂದುವರಿ;
        }

        ಸತತ_thread(init_virtio_scsi, pci);
    }
}

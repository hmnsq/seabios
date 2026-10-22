// Virtio SCSI boot support.
//
// Copyright (C) 2012 Red Hat Inc.
//
// Authors:
//  Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // struct drive_s
#ਅਨ੍ਤਰ੍ਭਾਵ "blockcmd.h" // scsi_drive_setup
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pcidevice.h" // foreachpci
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_regs.h" // PCI_VENDOR_ID
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // run_thread
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // DISK_RET_SUCCESS
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // usleep, bootprio_find_pci_device, is_bootprio_strict
#ਅਨ੍ਤਰ੍ਭਾਵ "virtio-pci.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "virtio-ring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "virtio-scsi.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "virtio-mmio.h"

ਬਣਤਰ virtio_lun_s {
    ਬਣਤਰ drive_s drive;
    ਬਣਤਰ pci_device *pci;
    ਖਾਲੀ *mmio;
    ਅੱਖਰ name[16];
    ਬਣਤਰ vring_virtqueue *vq;
    ਬਣਤਰ vp_device *vp;
    u16 ਨਿਸ਼ਾਨਾ;
    u16 lun;
};

ਪੂਰਨ
virtio_scsi_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (! CONFIG_VIRTIO_SCSI)
        ਵਾਪਸ 0;
    ਬਣਤਰ virtio_lun_s *vlun =
        container_ਦਾ(op->drive_fl, ਬਣਤਰ virtio_lun_s, drive);
    ਬਣਤਰ vp_device *vp = vlun->vp;
    ਬਣਤਰ vring_virtqueue *vq = vlun->vq;
    ਬਣਤਰ virtio_scsi_req_cmd req;
    ਬਣਤਰ virtio_scsi_resp_cmd resp;
    ਬਣਤਰ vring_list sg[3];

    memset(&req, 0, ਆਕਾਰ(req));
    ਪੂਰਨ blocksize = scsi_fill_cmd(op, req.cdb, 16);
    ਜੇ (blocksize < 0)
        ਵਾਪਸ default_process_op(op);
    req.lun[0] = 1;
    req.lun[1] = vlun->ਨਿਸ਼ਾਨਾ;
    req.lun[2] = (vlun->lun >> 8) | 0x40;
    req.lun[3] = (vlun->lun & 0xff);

    u32 len = op->ਗਿਣਤੀ_2 * blocksize;
    ਪੂਰਨ datain = scsi_is_read(op);
    ਪੂਰਨ ਵਿੱਚ_ਅੰਕ = (datain ? 2 : 1);
    ਪੂਰਨ ਬਾਹਰਲਾ_ਅੰਕ = (len ? 3 : 2) - ਵਿੱਚ_ਅੰਕ;

    sg[0].addr   = (ਖਾਲੀ*)(&req);
    sg[0].ਲੰਮਾਈ = ਆਕਾਰ(req);

    sg[ਬਾਹਰਲਾ_ਅੰਕ].addr   = (ਖਾਲੀ*)(&resp);
    sg[ਬਾਹਰਲਾ_ਅੰਕ].ਲੰਮਾਈ = ਆਕਾਰ(resp);

    ਜੇ (len) {
        ਪੂਰਨ ਅੰਕੜੇ_idx = (datain ? 2 : 1);
        sg[ਅੰਕੜੇ_idx].addr   = op->buf_fl;
        sg[ਅੰਕੜੇ_idx].ਲੰਮਾਈ = len;
    }

    /* Add to virtqueue and kick host */
    vring_ਜੋੜੋ_buf(vq, sg, ਬਾਹਰਲਾ_ਅੰਕ, ਵਿੱਚ_ਅੰਕ, 0, 0);
    vring_kick(vp, vq, 1);

    /* Wait for reply */
    ਜਦੋਂ (!vring_ਹੋਰ_ਵਰਤਿਆ(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, ਨਿਰਾ);

    /* Clear interrupt status register.  Avoid leaving interrupts stuck if
     * VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
     */
    vp_get_isr(vp);

    ਜੇ (resp.response == VIRTIO_SCSI_S_OK && resp.ਸਥਿਤੀ == 0) {
        ਵਾਪਸ DISK_RET_SUCCESS;
    }
    ਵਾਪਸ DISK_RET_EBADTRACK;
}

ਸਥਿਰ ਖਾਲੀ
virtio_scsi_init_lun(ਬਣਤਰ virtio_lun_s *vlun,
                     ਬਣਤਰ pci_device *pci, ਖਾਲੀ *mmio,
                     ਬਣਤਰ vp_device *vp, ਬਣਤਰ vring_virtqueue *vq,
                     u16 ਨਿਸ਼ਾਨਾ, u16 lun)
{
    memset(vlun, 0, ਆਕਾਰ(*vlun));
    vlun->drive.type = DTYPE_VIRTIO_SCSI;
    vlun->drive.cntl_id = pci->bdf;
    vlun->pci = pci;
    vlun->mmio = mmio;
    vlun->vp = vp;
    vlun->vq = vq;
    vlun->ਨਿਸ਼ਾਨਾ = ਨਿਸ਼ਾਨਾ;
    vlun->lun = lun;
    ਜੇ (vlun->pci)
        snprintf(vlun->name, ਆਕਾਰ(vlun->name), "pci:%pP", vlun->pci);
    ਜੇ (vlun->mmio)
        snprintf(vlun->name, ਆਕਾਰ(vlun->name), "mmio:%08x", (u32)vlun->mmio);
}

ਸਥਿਰ ਪੂਰਨ
virtio_scsi_ਜੋੜੋ_lun(u32 lun, ਬਣਤਰ drive_s *tmpl_drv)
{
    u8 skip_nonbootable = is_bootprio_strict();
    ਬਣਤਰ virtio_lun_s *tmpl_vlun =
        container_ਦਾ(tmpl_drv, ਬਣਤਰ virtio_lun_s, drive);
    ਪੂਰਨ prio = -1;

    ਜੇ (tmpl_vlun->pci)
        prio = bootprio_ਲੱਭੋ_scsi_device(tmpl_vlun->pci, tmpl_vlun->ਨਿਸ਼ਾਨਾ, lun);
    ਜੇ (tmpl_vlun->mmio)
        prio = bootprio_ਲੱਭੋ_scsi_mmio_device(tmpl_vlun->mmio, tmpl_vlun->ਨਿਸ਼ਾਨਾ, lun);

    ਜੇ (skip_nonbootable && prio < 0) {
        dprintf(1, "skipping init of a non-bootable virtio-scsi dev at %s,"
                " target %d, lun %d\n",
                tmpl_vlun->name, tmpl_vlun->ਨਿਸ਼ਾਨਾ, lun);
        ਵਾਪਸ -1;
    }

    ਬਣਤਰ virtio_lun_s *vlun = malloc_ਨੀਵਾਂ(ਆਕਾਰ(*vlun));
    ਜੇ (!vlun) {
        warn_noalloc();
        ਵਾਪਸ -1;
    }
    virtio_scsi_init_lun(vlun, tmpl_vlun->pci, tmpl_vlun->mmio,tmpl_vlun->vp,
                         tmpl_vlun->vq, tmpl_vlun->ਨਿਸ਼ਾਨਾ, lun);

    ਜੇ (vlun->pci)
        boot_lchs_ਲੱਭੋ_scsi_device(vlun->pci, vlun->ਨਿਸ਼ਾਨਾ, vlun->lun,
                                   &(vlun->drive.lchs));
    ਪੂਰਨ ret = scsi_drive_ਤਿਆਰੀ(&vlun->drive, "virtio-scsi", prio);
    ਜੇ (ret)
        ਜਾਓ fail;
    ਵਾਪਸ 0;

fail:
    ਮੁਕਤ(vlun);
    ਵਾਪਸ -1;
}

ਸਥਿਰ ਪੂਰਨ
virtio_scsi_ਜਾਂਚ_ਨਿਸ਼ਾਨਾ(ਬਣਤਰ pci_device *pci, ਖਾਲੀ *mmio, ਬਣਤਰ vp_device *vp,
                        ਬਣਤਰ vring_virtqueue *vq, u16 ਨਿਸ਼ਾਨਾ)
{

    ਬਣਤਰ virtio_lun_s vlun0;

    virtio_scsi_init_lun(&vlun0, pci, mmio, vp, vq, ਨਿਸ਼ਾਨਾ, 0);

    ਪੂਰਨ ret = scsi_rep_luns_ਜਾਂਚ(&vlun0.drive, virtio_scsi_ਜੋੜੋ_lun);
    ਵਾਪਸ ret < 0 ? 0 : ret;
}

ਸਥਿਰ ਖਾਲੀ
init_virtio_scsi(ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਬਣਤਰ pci_device *pci = ਅੰਕੜੇ;
    dprintf(1, "found virtio-scsi at %pP\n", pci);
    ਬਣਤਰ vring_virtqueue *vq = ਨਿਰਾ;
    ਬਣਤਰ vp_device *vp = malloc_ਉੱਚਾ(ਆਕਾਰ(*vp));
    ਜੇ (!vp) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    vp_init_simple(vp, pci);
    u8 ਸਥਿਤੀ = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    ਜੇ (vp->use_modern) {
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_ਸੰਸਕਰਣ_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        ਜੇ (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            ਜਾਓ fail;
        }

        vp_ਸੈੱਟ_features(vp, features & (version1 | iommu_platform));
        ਸਥਿਤੀ |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_ਸੈੱਟ_ਸਥਿਤੀ(vp, ਸਥਿਤੀ);
        ਜੇ (!(vp_get_ਸਥਿਤੀ(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            ਜਾਓ fail;
        }
    }

    ਜੇ (vp_ਲੱਭੋ_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi %pP\n", pci);
        ਜਾਓ fail;
    }

    ਸਥਿਤੀ |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_ਸੈੱਟ_ਸਥਿਤੀ(vp, ਸਥਿਤੀ);

    ਪੂਰਨ i, tot;
    ਲਈ (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_ਜਾਂਚ_ਨਿਸ਼ਾਨਾ(pci, ਨਿਰਾ, vp, vq, i);

    ਜੇ (!tot)
        ਜਾਓ fail;

    ਵਾਪਸ;

fail:
    vp_reset(vp);
    ਮੁਕਤ(vp);
    ਮੁਕਤ(vq);
}

ਖਾਲੀ
init_virtio_scsi_mmio(ਖਾਲੀ *mmio)
{
    dprintf(1, "found virtio-scsi-mmio at %p\n", mmio);
    ਬਣਤਰ vring_virtqueue *vq = ਨਿਰਾ;
    ਬਣਤਰ vp_device *vp = malloc_ਉੱਚਾ(ਆਕਾਰ(*vp));
    ਜੇ (!vp) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    vp_init_mmio(vp, mmio);
    u8 ਸਥਿਤੀ = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_ਸੰਸਕਰਣ_1;
    ਜੇ (features & version1) {
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;

        vp_ਸੈੱਟ_features(vp, features & (version1 | iommu_platform));
        vp_ਸੈੱਟ_ਸਥਿਤੀ(vp, VIRTIO_CONFIG_S_FEATURES_OK);
        ਜੇ (!(vp_get_ਸਥਿਤੀ(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", mmio);
            ਜਾਓ fail;
        }
    }

    ਜੇ (vp_ਲੱਭੋ_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi-mmio %p\n", mmio);
        ਜਾਓ fail;
    }

    ਸਥਿਤੀ |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_ਸੈੱਟ_ਸਥਿਤੀ(vp, ਸਥਿਤੀ);

    ਪੂਰਨ i, tot;
    ਲਈ (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_ਜਾਂਚ_ਨਿਸ਼ਾਨਾ(ਨਿਰਾ, mmio, vp, vq, i);

    ਜੇ (!tot)
        ਜਾਓ fail;

    ਵਾਪਸ;

fail:
    vp_reset(vp);
    ਮੁਕਤ(vp);
    ਮੁਕਤ(vq);
}

ਖਾਲੀ
virtio_scsi_ਤਿਆਰੀ(ਖਾਲੀ)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    ਜੇ (! CONFIG_VIRTIO_SCSI)
        ਵਾਪਸ;

    dprintf(3, "init virtio-scsi\n");

    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_10))
            ਜਾਰੀ;

        ਜੇ (skip_nonbootable && bootprio_ਲੱਭੋ_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-scsi at %pP\n",
                    pci);
            ਜਾਰੀ;
        }

        ਲੜੀਵਾਰ_thread(init_virtio_scsi, pci);
    }
}

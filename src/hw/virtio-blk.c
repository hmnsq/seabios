// Virtio block boot support.
//
// Copyright (C) 2010 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_GLOBALFLAT
#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_*
#ਅਨ੍ਤਰ੍ਭਾਵ "block.h" // struct drive_s
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
#ਅਨ੍ਤਰ੍ਭਾਵ "virtio-mmio.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "virtio-ring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "virtio-blk.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ min(a, b) ({\
		typeof(a) _a = a;\
		typeof(b) _b = b;\
		_a < _b ? _a : _b; })

ਬਣਤਰ virtiodrive_s {
    ਬਣਤਰ drive_s drive;
    ਬਣਤਰ vring_virtqueue *vq;
    ਬਣਤਰ vp_device vp;
};

ਖਾਲੀ
virtio_blk_op_one_segment(ਬਣਤਰ virtiodrive_s *vdrive,
    ਪੂਰਨ ਲਿਖ, ਬਣਤਰ vring_list sg[])
{
    ਬਣਤਰ vring_virtqueue *vq = vdrive->vq;

    /* Add to virtqueue and kick host */
    ਜੇ (ਲਿਖ)
        vring_ਜੋੜੋ_buf(vq, sg, 2, 1, 0, 0);
    ਨਹੀਂਤਾਂ
        vring_ਜੋੜੋ_buf(vq, sg, 1, 2, 0, 0);
    vring_kick(&vdrive->vp, vq, 1);

    /* Wait for reply */
    ਜਦੋਂ (!vring_ਹੋਰ_ਵਰਤਿਆ(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, ਨਿਰਾ);

    /**
    ** Clear interrupt status register. Avoid leaving interrupts stuck
    ** if VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
    **/
    vp_get_isr(&vdrive->vp);
}

ਸਥਿਰ ਪੂਰਨ
virtio_blk_op(ਬਣਤਰ disk_op_s *op, ਪੂਰਨ ਲਿਖ)
{
    ਬਣਤਰ virtiodrive_s *vdrive =
        container_ਦਾ(op->drive_fl, ਬਣਤਰ virtiodrive_s, drive);
    ਬਣਤਰ virtio_blk_outhdr hdr = {
        .type = ਲਿਖ ? VIRTIO_BLK_T_ਬਾਹਰਲਾ : VIRTIO_BLK_T_ਵਿੱਚ,
        .ioprio = 0,
        .sector = op->lba,
    };
    u8 ਸਥਿਤੀ = VIRTIO_BLK_S_UNSUPP;
    ਬਣਤਰ vring_list sg[] = {
        {
            .addr       = (ਖਾਲੀ*)(&hdr),
            .ਲੰਮਾਈ     = ਆਕਾਰ(hdr),
        },
        {
            .addr       = op->buf_fl,
            .ਲੰਮਾਈ     = vdrive->drive.blksize * op->ਗਿਣਤੀ_2,
        },
        {
            .addr       = (ਖਾਲੀ*)(&ਸਥਿਤੀ),
            .ਲੰਮਾਈ     = ਆਕਾਰ(ਸਥਿਤੀ),
        },
    };
    u32 max_io_ਮਾਪ =
        vdrive->drive.max_segment_ਮਾਪ * vdrive->drive.max_segments;
    u16 blk_ਅੰਕ_max;

    ਜੇ (vdrive->drive.blksize != 0 && max_io_ਮਾਪ != 0)
        blk_ਅੰਕ_max = (u16) min(max_io_ਮਾਪ / vdrive->drive.blksize, 0xffff);
    ਨਹੀਂਤਾਂ
        /* default blk_num_max if hardware doesnot advise a proper value */
        blk_ਅੰਕ_max = 64;

    ਜੇ (op->ਗਿਣਤੀ_2 <= blk_ਅੰਕ_max) {
        virtio_blk_op_one_segment(vdrive, ਲਿਖ, sg);
    } ਨਹੀਂਤਾਂ {
        ਖਾਲੀ *p  = op->buf_fl;
        u16 ਗਿਣਤੀ_2 = op->ਗਿਣਤੀ_2;

        ਜਦੋਂ (ਗਿਣਤੀ_2 > 0) {
            u16 blk_ਅੰਕ = min(ਗਿਣਤੀ_2, blk_ਅੰਕ_max);
            sg[1].ਲੰਮਾਈ = vdrive->drive.blksize * blk_ਅੰਕ;
            sg[1].addr = p;
            virtio_blk_op_one_segment(vdrive, ਲਿਖ, sg);
            ਜੇ (ਸਥਿਤੀ == VIRTIO_BLK_S_OK) {
                hdr.sector += blk_ਅੰਕ;
                p += sg[1].ਲੰਮਾਈ;
                ਗਿਣਤੀ_2 -= blk_ਅੰਕ;
            } ਨਹੀਂਤਾਂ {
                ਰੋਕੋ;
            }
        }
    }
    ਵਾਪਸ ਸਥਿਤੀ == VIRTIO_BLK_S_OK ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

ਪੂਰਨ
virtio_blk_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (! CONFIG_VIRTIO_BLK)
        ਵਾਪਸ 0;
    ਚੋਣ (op->command) {
    ਹਾਲਤ CMD_READ:
        ਵਾਪਸ virtio_blk_op(op, 0);
    ਹਾਲਤ CMD_WRITE:
        ਵਾਪਸ virtio_blk_op(op, 1);
    ਮੂਲ:
        ਵਾਪਸ default_process_op(op);
    }
}

ਸਥਿਰ ਖਾਲੀ
init_virtio_blk(ਖਾਲੀ *ਅੰਕੜੇ)
{
    ਬਣਤਰ pci_device *pci = ਅੰਕੜੇ;
    u8 ਸਥਿਤੀ = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk at %pP\n", pci);
    ਬਣਤਰ virtiodrive_s *vdrive = malloc_ਨੀਵਾਂ(ਆਕਾਰ(*vdrive));
    ਜੇ (!vdrive) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    memset(vdrive, 0, ਆਕਾਰ(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = pci->bdf;

    vp_init_simple(&vdrive->vp, pci);

    ਜੇ (vdrive->vp.use_modern) {
        ਬਣਤਰ vp_device *vp = &vdrive->vp;
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_ਸੰਸਕਰਣ_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        u64 blk_ਮਾਪ = 1ull << VIRTIO_BLK_F_BLK_ਮਾਪ;
        u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
        u64 max_segment_ਮਾਪ = 1ull << VIRTIO_BLK_F_ਮਾਪ_MAX;

        ਜੇ (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            ਜਾਓ fail;
        }

        features = features & (version1 | iommu_platform | blk_ਮਾਪ
                        | max_segments | max_segment_ਮਾਪ);
        vp_ਸੈੱਟ_features(vp, features);
        ਸਥਿਤੀ |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_ਸੈੱਟ_ਸਥਿਤੀ(vp, ਸਥਿਤੀ);
        ਜੇ (!(vp_get_ਸਥਿਤੀ(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            ਜਾਓ fail;
        }

        ਜੇ (features & max_segment_ਮਾਪ)
            vdrive->drive.max_segment_ਮਾਪ =
                vp_read(&vp->device, ਬਣਤਰ virtio_blk_config, ਮਾਪ_max);

        ਜੇ (features & max_segments)
            vdrive->drive.max_segments =
                vp_read(&vp->device, ਬਣਤਰ virtio_blk_config, seg_max);

        vdrive->drive.sectors =
            vp_read(&vp->device, ਬਣਤਰ virtio_blk_config, ਸਮਰੱਥਾ);
        ਜੇ (features & blk_ਮਾਪ) {
            vdrive->drive.blksize =
                vp_read(&vp->device, ਬਣਤਰ virtio_blk_config, blk_ਮਾਪ);
        } ਨਹੀਂਤਾਂ {
            vdrive->drive.blksize = DISK_SECTOR_ਮਾਪ;
        }
        ਜੇ (vdrive->drive.blksize != DISK_SECTOR_ਮਾਪ) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            ਜਾਓ fail;
        }
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_ਮਾਪ,
                vdrive->drive.max_segments);

        vdrive->drive.pchs.cylinder =
            vp_read(&vp->device, ਬਣਤਰ virtio_blk_config, cylinders);
        vdrive->drive.pchs.ਸਿਰ =
            vp_read(&vp->device, ਬਣਤਰ virtio_blk_config, heads);
        vdrive->drive.pchs.sector =
            vp_read(&vp->device, ਬਣਤਰ virtio_blk_config, sectors);
    }

    ਜੇ (vp_ਲੱਭੋ_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk %pP\n", pci);
        ਜਾਓ fail;
    }

    ਜੇ (!vdrive->vp.use_modern) {
        ਬਣਤਰ virtio_blk_config cfg;
        vp_get_legacy(&vdrive->vp, 0, &cfg, ਆਕਾਰ(cfg));

        u64 f = vp_get_features(&vdrive->vp);
        vdrive->drive.blksize = (f & (1 << VIRTIO_BLK_F_BLK_ਮਾਪ)) ?
            cfg.blk_ਮਾਪ : DISK_SECTOR_ਮਾਪ;

        ਜੇ (f & (1 << VIRTIO_BLK_F_ਮਾਪ_MAX))
            vdrive->drive.max_segment_ਮਾਪ = cfg.ਮਾਪ_max;

        ਜੇ (f & (1 << VIRTIO_BLK_F_SEG_MAX))
            vdrive->drive.max_segments = cfg.seg_max;

        vdrive->drive.sectors = cfg.ਸਮਰੱਥਾ;
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_ਮਾਪ,
                vdrive->drive.max_segments);

        ਜੇ (vdrive->drive.blksize != DISK_SECTOR_ਮਾਪ) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            ਜਾਓ fail;
        }
        vdrive->drive.pchs.cylinder = cfg.cylinders;
        vdrive->drive.pchs.ਸਿਰ = cfg.heads;
        vdrive->drive.pchs.sector = cfg.sectors;
    }

    ਅੱਖਰ *desc = znprintf(MAXDESCSIZE, "Virtio disk PCI:%pP", pci);
    boot_ਜੋੜੋ_hd(&vdrive->drive, desc, bootprio_ਲੱਭੋ_pci_device(pci));

    ਸਥਿਤੀ |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_ਸੈੱਟ_ਸਥਿਤੀ(&vdrive->vp, ਸਥਿਤੀ);

    boot_lchs_ਲੱਭੋ_pci_device(pci, &vdrive->drive.lchs);
    ਵਾਪਸ;

fail:
    vp_reset(&vdrive->vp);
    ਮੁਕਤ(vdrive->vq);
    ਮੁਕਤ(vdrive);
}

ਖਾਲੀ
init_virtio_blk_mmio(ਖਾਲੀ *mmio)
{
    u8 ਸਥਿਤੀ = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk-mmio at %p\n", mmio);
    ਬਣਤਰ virtiodrive_s *vdrive = malloc_ਨੀਵਾਂ(ਆਕਾਰ(*vdrive));
    ਜੇ (!vdrive) {
        warn_noalloc();
        ਵਾਪਸ;
    }
    memset(vdrive, 0, ਆਕਾਰ(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = (u32)mmio;

    vp_init_mmio(&vdrive->vp, mmio);

    ਬਣਤਰ vp_device *vp = &vdrive->vp;
    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_ਸੰਸਕਰਣ_1;
    u64 blk_ਮਾਪ = 1ull << VIRTIO_BLK_F_BLK_ਮਾਪ;
    u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
    u64 max_segment_ਮਾਪ = 1ull << VIRTIO_BLK_F_ਮਾਪ_MAX;

    features = features & (version1 | blk_ਮਾਪ
            | max_segments | max_segment_ਮਾਪ);
    vp_ਸੈੱਟ_features(vp, features);
    ਸਥਿਤੀ |= VIRTIO_CONFIG_S_FEATURES_OK;
    vp_ਸੈੱਟ_ਸਥਿਤੀ(vp, ਸਥਿਤੀ);
    ਜੇ (!(vp_get_ਸਥਿਤੀ(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
        dprintf(1, "device didn't accept features: %p\n", mmio);
        ਜਾਓ fail;
    }

    ਜੇ (vp_ਲੱਭੋ_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk-mmio %p\n", mmio);
        ਜਾਓ fail;
    }

    ਜੇ (features & max_segment_ਮਾਪ)
        vdrive->drive.max_segment_ਮਾਪ =
            vp_read(&vp->device, ਬਣਤਰ virtio_blk_config, ਮਾਪ_max);

    ਜੇ (features & max_segments)
        vdrive->drive.max_segments =
            vp_read(&vp->device, ਬਣਤਰ virtio_blk_config, seg_max);

    vdrive->drive.sectors =
        vp_read(&vp->device, ਬਣਤਰ virtio_blk_config, ਸਮਰੱਥਾ);
    ਜੇ (features & blk_ਮਾਪ) {
        vdrive->drive.blksize =
            vp_read(&vp->device, ਬਣਤਰ virtio_blk_config, blk_ਮਾਪ);
    } ਨਹੀਂਤਾਂ {
        vdrive->drive.blksize = DISK_SECTOR_ਮਾਪ;
    }
    ਜੇ (vdrive->drive.blksize != DISK_SECTOR_ਮਾਪ) {
        dprintf(1, "virtio-blk-mmio %p block size %d is unsupported\n",
                mmio, vdrive->drive.blksize);
        ਜਾਓ fail;
    }
    dprintf(1, "virtio-blk-mmio %p blksize=%d sectors=%u size_max=%u "
            "seg_max=%u.\n", mmio, vdrive->drive.blksize,
            (u32)vdrive->drive.sectors, vdrive->drive.max_segment_ਮਾਪ,
            vdrive->drive.max_segments);

    vdrive->drive.pchs.cylinder =
        vp_read(&vp->device, ਬਣਤਰ virtio_blk_config, cylinders);
    vdrive->drive.pchs.ਸਿਰ =
        vp_read(&vp->device, ਬਣਤਰ virtio_blk_config, heads);
    vdrive->drive.pchs.sector =
        vp_read(&vp->device, ਬਣਤਰ virtio_blk_config, sectors);

    ਅੱਖਰ *desc = znprintf(MAXDESCSIZE, "Virtio disk mmio:%p", mmio);
    boot_ਜੋੜੋ_hd(&vdrive->drive, desc, bootprio_ਲੱਭੋ_mmio_device(mmio));

    ਸਥਿਤੀ |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_ਸੈੱਟ_ਸਥਿਤੀ(&vdrive->vp, ਸਥਿਤੀ);
    ਵਾਪਸ;

fail:
    vp_reset(&vdrive->vp);
    ਮੁਕਤ(vdrive->vq);
    ਮੁਕਤ(vdrive);
}

ਖਾਲੀ
virtio_blk_ਤਿਆਰੀ(ਖਾਲੀ)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    ਜੇ (! CONFIG_VIRTIO_BLK)
        ਵਾਪਸ;

    dprintf(3, "init virtio-blk\n");

    ਬਣਤਰ pci_device *pci;
    foreachpci(pci) {
        ਜੇ (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_BLK_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_BLK_10))
            ਜਾਰੀ;

        ਜੇ (skip_nonbootable && bootprio_ਲੱਭੋ_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-blk at %pP\n",
                    pci);
            ਜਾਰੀ;
        }

        ਲੜੀਵਾਰ_thread(init_virtio_blk, pci);
    }
}

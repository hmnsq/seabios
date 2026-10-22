// Virtio block boot support.
//
// Copyright (C) 2010 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_GLOBALFLAT
#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_*
#ಅನ್ತರ್ಭಾವ "block.h" // struct drive_s
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
#ಅನ್ತರ್ಭಾವ "virtio-mmio.h"
#ಅನ್ತರ್ಭಾವ "virtio-ring.h"
#ಅನ್ತರ್ಭಾವ "virtio-blk.h"

#ವ್ಯಾಖ್ಯಾನಿಸು min(a, b) ({\
		typeof(a) _a = a;\
		typeof(b) _b = b;\
		_a < _b ? _a : _b; })

ರಚನೆ virtiodrive_s {
    ರಚನೆ drive_s drive;
    ರಚನೆ vring_virtqueue *vq;
    ರಚನೆ vp_device vp;
};

ಶೂನ್ಯ
virtio_blk_op_one_segment(ರಚನೆ virtiodrive_s *vdrive,
    ಪೂರ್ಣಾಂಕ ಲಿಖ, ರಚನೆ vring_list sg[])
{
    ರಚನೆ vring_virtqueue *vq = vdrive->vq;

    /* Add to virtqueue and kick host */
    ಆದರೆ (ಲಿಖ)
        vring_ಕೂಡಿಸು_buf(vq, sg, 2, 1, 0, 0);
    ಇಲ್ಲದಿದ್ದರೆ
        vring_ಕೂಡಿಸು_buf(vq, sg, 1, 2, 0, 0);
    vring_kick(&vdrive->vp, vq, 1);

    /* Wait for reply */
    ಆಗ (!vring_ಇನ್ನಷ್ಟು_ಬಳಸಿದ(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, ಖಾಲಿ);

    /**
    ** Clear interrupt status register. Avoid leaving interrupts stuck
    ** if VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
    **/
    vp_get_isr(&vdrive->vp);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
virtio_blk_op(ರಚನೆ disk_op_s *op, ಪೂರ್ಣಾಂಕ ಲಿಖ)
{
    ರಚನೆ virtiodrive_s *vdrive =
        container_ದ(op->drive_fl, ರಚನೆ virtiodrive_s, drive);
    ರಚನೆ virtio_blk_outhdr hdr = {
        .type = ಲಿಖ ? VIRTIO_BLK_T_ಹೊರಹೋಗು : VIRTIO_BLK_T_ಒಳ,
        .ioprio = 0,
        .sector = op->lba,
    };
    u8 ಸ್ಥಿತಿ_2 = VIRTIO_BLK_S_UNSUPP;
    ರಚನೆ vring_list sg[] = {
        {
            .addr       = (ಶೂನ್ಯ*)(&hdr),
            .ಉದ್ದಳತೆ     = ಗಾತ್ರ(hdr),
        },
        {
            .addr       = op->buf_fl,
            .ಉದ್ದಳತೆ     = vdrive->drive.blksize * op->ಎಣಿಕೆ_2,
        },
        {
            .addr       = (ಶೂನ್ಯ*)(&ಸ್ಥಿತಿ_2),
            .ಉದ್ದಳತೆ     = ಗಾತ್ರ(ಸ್ಥಿತಿ_2),
        },
    };
    u32 max_io_ಗಾತ್ರ =
        vdrive->drive.max_segment_ಗಾತ್ರ * vdrive->drive.max_segments;
    u16 blk_ಅಂಕೆ_max;

    ಆದರೆ (vdrive->drive.blksize != 0 && max_io_ಗಾತ್ರ != 0)
        blk_ಅಂಕೆ_max = (u16) min(max_io_ಗಾತ್ರ / vdrive->drive.blksize, 0xffff);
    ಇಲ್ಲದಿದ್ದರೆ
        /* default blk_num_max if hardware doesnot advise a proper value */
        blk_ಅಂಕೆ_max = 64;

    ಆದರೆ (op->ಎಣಿಕೆ_2 <= blk_ಅಂಕೆ_max) {
        virtio_blk_op_one_segment(vdrive, ಲಿಖ, sg);
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಶೂನ್ಯ *p  = op->buf_fl;
        u16 ಎಣಿಕೆ_2 = op->ಎಣಿಕೆ_2;

        ಆಗ (ಎಣಿಕೆ_2 > 0) {
            u16 blk_ಅಂಕೆ = min(ಎಣಿಕೆ_2, blk_ಅಂಕೆ_max);
            sg[1].ಉದ್ದಳತೆ = vdrive->drive.blksize * blk_ಅಂಕೆ;
            sg[1].addr = p;
            virtio_blk_op_one_segment(vdrive, ಲಿಖ, sg);
            ಆದರೆ (ಸ್ಥಿತಿ_2 == VIRTIO_BLK_S_OK) {
                hdr.sector += blk_ಅಂಕೆ;
                p += sg[1].ಉದ್ದಳತೆ;
                ಎಣಿಕೆ_2 -= blk_ಅಂಕೆ;
            } ಇಲ್ಲದಿದ್ದರೆ {
                ನಿಲ್ಲಿಸು;
            }
        }
    }
    ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2 == VIRTIO_BLK_S_OK ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

ಪೂರ್ಣಾಂಕ
virtio_blk_process_op(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (! CONFIG_VIRTIO_BLK)
        ಹಿಂತಿರುಗಿ 0;
    ಆಯ್ಕೆ (op->command) {
    ಸ್ಥಿತಿ CMD_READ:
        ಹಿಂತಿರುಗಿ virtio_blk_op(op, 0);
    ಸ್ಥಿತಿ CMD_WRITE:
        ಹಿಂತಿರುಗಿ virtio_blk_op(op, 1);
    ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ default_process_op(op);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
init_virtio_blk(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
    ರಚನೆ pci_device *pci = ದತ್ತಾಂಶ;
    u8 ಸ್ಥಿತಿ_2 = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk at %pP\n", pci);
    ರಚನೆ virtiodrive_s *vdrive = malloc_ಕೆಳಗಿನ(ಗಾತ್ರ(*vdrive));
    ಆದರೆ (!vdrive) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    memset(vdrive, 0, ಗಾತ್ರ(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = pci->bdf;

    vp_init_simple(&vdrive->vp, pci);

    ಆದರೆ (vdrive->vp.use_modern) {
        ರಚನೆ vp_device *vp = &vdrive->vp;
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_ಆವೃತ್ತಿ_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        u64 blk_ಗಾತ್ರ = 1ull << VIRTIO_BLK_F_BLK_ಗಾತ್ರ;
        u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
        u64 max_segment_ಗಾತ್ರ = 1ull << VIRTIO_BLK_F_ಗಾತ್ರ_MAX;

        ಆದರೆ (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            ಹೋಗು fail;
        }

        features = features & (version1 | iommu_platform | blk_ಗಾತ್ರ
                        | max_segments | max_segment_ಗಾತ್ರ);
        vp_ಗಣ_features(vp, features);
        ಸ್ಥಿತಿ_2 |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_ಗಣ_ಸ್ಥಿತಿ(vp, ಸ್ಥಿತಿ_2);
        ಆದರೆ (!(vp_get_ಸ್ಥಿತಿ(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            ಹೋಗು fail;
        }

        ಆದರೆ (features & max_segment_ಗಾತ್ರ)
            vdrive->drive.max_segment_ಗಾತ್ರ =
                vp_read(&vp->device, ರಚನೆ virtio_blk_config, ಗಾತ್ರ_max);

        ಆದರೆ (features & max_segments)
            vdrive->drive.max_segments =
                vp_read(&vp->device, ರಚನೆ virtio_blk_config, seg_max);

        vdrive->drive.sectors =
            vp_read(&vp->device, ರಚನೆ virtio_blk_config, ಸಾಮರ್ಥ್ಯ);
        ಆದರೆ (features & blk_ಗಾತ್ರ) {
            vdrive->drive.blksize =
                vp_read(&vp->device, ರಚನೆ virtio_blk_config, blk_ಗಾತ್ರ);
        } ಇಲ್ಲದಿದ್ದರೆ {
            vdrive->drive.blksize = DISK_SECTOR_ಗಾತ್ರ;
        }
        ಆದರೆ (vdrive->drive.blksize != DISK_SECTOR_ಗಾತ್ರ) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            ಹೋಗು fail;
        }
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_ಗಾತ್ರ,
                vdrive->drive.max_segments);

        vdrive->drive.pchs.cylinder =
            vp_read(&vp->device, ರಚನೆ virtio_blk_config, cylinders);
        vdrive->drive.pchs.ತಲೆ =
            vp_read(&vp->device, ರಚನೆ virtio_blk_config, heads);
        vdrive->drive.pchs.sector =
            vp_read(&vp->device, ರಚನೆ virtio_blk_config, sectors);
    }

    ಆದರೆ (vp_ಕಂಡುಹಿಡಿ_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk %pP\n", pci);
        ಹೋಗು fail;
    }

    ಆದರೆ (!vdrive->vp.use_modern) {
        ರಚನೆ virtio_blk_config cfg;
        vp_get_legacy(&vdrive->vp, 0, &cfg, ಗಾತ್ರ(cfg));

        u64 f = vp_get_features(&vdrive->vp);
        vdrive->drive.blksize = (f & (1 << VIRTIO_BLK_F_BLK_ಗಾತ್ರ)) ?
            cfg.blk_ಗಾತ್ರ : DISK_SECTOR_ಗಾತ್ರ;

        ಆದರೆ (f & (1 << VIRTIO_BLK_F_ಗಾತ್ರ_MAX))
            vdrive->drive.max_segment_ಗಾತ್ರ = cfg.ಗಾತ್ರ_max;

        ಆದರೆ (f & (1 << VIRTIO_BLK_F_SEG_MAX))
            vdrive->drive.max_segments = cfg.seg_max;

        vdrive->drive.sectors = cfg.ಸಾಮರ್ಥ್ಯ;
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_ಗಾತ್ರ,
                vdrive->drive.max_segments);

        ಆದರೆ (vdrive->drive.blksize != DISK_SECTOR_ಗಾತ್ರ) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            ಹೋಗು fail;
        }
        vdrive->drive.pchs.cylinder = cfg.cylinders;
        vdrive->drive.pchs.ತಲೆ = cfg.heads;
        vdrive->drive.pchs.sector = cfg.sectors;
    }

    ಅಕ್ಷರ *desc = znprintf(MAXDESCSIZE, "Virtio disk PCI:%pP", pci);
    boot_ಕೂಡಿಸು_hd(&vdrive->drive, desc, bootprio_ಕಂಡುಹಿಡಿ_pci_device(pci));

    ಸ್ಥಿತಿ_2 |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_ಗಣ_ಸ್ಥಿತಿ(&vdrive->vp, ಸ್ಥಿತಿ_2);

    boot_lchs_ಕಂಡುಹಿಡಿ_pci_device(pci, &vdrive->drive.lchs);
    ಹಿಂತಿರುಗಿ;

fail:
    vp_reset(&vdrive->vp);
    ಬಿಡುಗಡೆ(vdrive->vq);
    ಬಿಡುಗಡೆ(vdrive);
}

ಶೂನ್ಯ
init_virtio_blk_mmio(ಶೂನ್ಯ *mmio)
{
    u8 ಸ್ಥಿತಿ_2 = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk-mmio at %p\n", mmio);
    ರಚನೆ virtiodrive_s *vdrive = malloc_ಕೆಳಗಿನ(ಗಾತ್ರ(*vdrive));
    ಆದರೆ (!vdrive) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ;
    }
    memset(vdrive, 0, ಗಾತ್ರ(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = (u32)mmio;

    vp_init_mmio(&vdrive->vp, mmio);

    ರಚನೆ vp_device *vp = &vdrive->vp;
    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_ಆವೃತ್ತಿ_1;
    u64 blk_ಗಾತ್ರ = 1ull << VIRTIO_BLK_F_BLK_ಗಾತ್ರ;
    u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
    u64 max_segment_ಗಾತ್ರ = 1ull << VIRTIO_BLK_F_ಗಾತ್ರ_MAX;

    features = features & (version1 | blk_ಗಾತ್ರ
            | max_segments | max_segment_ಗಾತ್ರ);
    vp_ಗಣ_features(vp, features);
    ಸ್ಥಿತಿ_2 |= VIRTIO_CONFIG_S_FEATURES_OK;
    vp_ಗಣ_ಸ್ಥಿತಿ(vp, ಸ್ಥಿತಿ_2);
    ಆದರೆ (!(vp_get_ಸ್ಥಿತಿ(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
        dprintf(1, "device didn't accept features: %p\n", mmio);
        ಹೋಗು fail;
    }

    ಆದರೆ (vp_ಕಂಡುಹಿಡಿ_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk-mmio %p\n", mmio);
        ಹೋಗು fail;
    }

    ಆದರೆ (features & max_segment_ಗಾತ್ರ)
        vdrive->drive.max_segment_ಗಾತ್ರ =
            vp_read(&vp->device, ರಚನೆ virtio_blk_config, ಗಾತ್ರ_max);

    ಆದರೆ (features & max_segments)
        vdrive->drive.max_segments =
            vp_read(&vp->device, ರಚನೆ virtio_blk_config, seg_max);

    vdrive->drive.sectors =
        vp_read(&vp->device, ರಚನೆ virtio_blk_config, ಸಾಮರ್ಥ್ಯ);
    ಆದರೆ (features & blk_ಗಾತ್ರ) {
        vdrive->drive.blksize =
            vp_read(&vp->device, ರಚನೆ virtio_blk_config, blk_ಗಾತ್ರ);
    } ಇಲ್ಲದಿದ್ದರೆ {
        vdrive->drive.blksize = DISK_SECTOR_ಗಾತ್ರ;
    }
    ಆದರೆ (vdrive->drive.blksize != DISK_SECTOR_ಗಾತ್ರ) {
        dprintf(1, "virtio-blk-mmio %p block size %d is unsupported\n",
                mmio, vdrive->drive.blksize);
        ಹೋಗು fail;
    }
    dprintf(1, "virtio-blk-mmio %p blksize=%d sectors=%u size_max=%u "
            "seg_max=%u.\n", mmio, vdrive->drive.blksize,
            (u32)vdrive->drive.sectors, vdrive->drive.max_segment_ಗಾತ್ರ,
            vdrive->drive.max_segments);

    vdrive->drive.pchs.cylinder =
        vp_read(&vp->device, ರಚನೆ virtio_blk_config, cylinders);
    vdrive->drive.pchs.ತಲೆ =
        vp_read(&vp->device, ರಚನೆ virtio_blk_config, heads);
    vdrive->drive.pchs.sector =
        vp_read(&vp->device, ರಚನೆ virtio_blk_config, sectors);

    ಅಕ್ಷರ *desc = znprintf(MAXDESCSIZE, "Virtio disk mmio:%p", mmio);
    boot_ಕೂಡಿಸು_hd(&vdrive->drive, desc, bootprio_ಕಂಡುಹಿಡಿ_mmio_device(mmio));

    ಸ್ಥಿತಿ_2 |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_ಗಣ_ಸ್ಥಿತಿ(&vdrive->vp, ಸ್ಥಿತಿ_2);
    ಹಿಂತಿರುಗಿ;

fail:
    vp_reset(&vdrive->vp);
    ಬಿಡುಗಡೆ(vdrive->vq);
    ಬಿಡುಗಡೆ(vdrive);
}

ಶೂನ್ಯ
virtio_blk_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    ಆದರೆ (! CONFIG_VIRTIO_BLK)
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "init virtio-blk\n");

    ರಚನೆ pci_device *pci;
    foreachpci(pci) {
        ಆದರೆ (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_BLK_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_BLK_10))
            ಮುಂದುವರಿ;

        ಆದರೆ (skip_nonbootable && bootprio_ಕಂಡುಹಿಡಿ_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-blk at %pP\n",
                    pci);
            ಮುಂದುವರಿ;
        }

        ಸತತ_thread(init_virtio_blk, pci);
    }
}

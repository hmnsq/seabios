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

#define min(a, b) ({\
		typeof(a) _a = a;\
		typeof(b) _b = b;\
		_a < _b ? _a : _b; })

ಸಂರಚನಾ virtiodrive_s {
    ಸಂರಚನಾ drive_s drive;
    ಸಂರಚನಾ vring_virtqueue *vq;
    ಸಂರಚನಾ vp_device vp;
};

ಶೂನ್ಯ
virtio_blk_op_one_segment(ಸಂರಚನಾ virtiodrive_s *vdrive,
    ಪೂರ್ಣಾಙ್ಕ ಲಿಖ, ಸಂರಚನಾ vring_list sg[])
{
    ಸಂರಚನಾ vring_virtqueue *vq = vdrive->vq;

    /* Add to virtqueue and kick host */
    ಯದಿ (ಲಿಖ)
        vring_add_buf(vq, sg, 2, 1, 0, 0);
    ಅನ್ಯಥಾ
        vring_add_buf(vq, sg, 1, 2, 0, 0);
    vring_kick(&vdrive->vp, vq, 1);

    /* Wait for reply */
    ಯಾವತ್ (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, ರಿಕ್ತ);

    /**
    ** Clear interrupt status register. Avoid leaving interrupts stuck
    ** if VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
    **/
    vp_get_isr(&vdrive->vp);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
virtio_blk_op(ಸಂರಚನಾ disk_op_s *op, ಪೂರ್ಣಾಙ್ಕ ಲಿಖ)
{
    ಸಂರಚನಾ virtiodrive_s *vdrive =
        container_of(op->drive_fl, ಸಂರಚನಾ virtiodrive_s, drive);
    ಸಂರಚನಾ virtio_blk_outhdr hdr = {
        .type = ಲಿಖ ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN,
        .ioprio = 0,
        .sector = op->lba,
    };
    u8 status = VIRTIO_BLK_S_UNSUPP;
    ಸಂರಚನಾ vring_list sg[] = {
        {
            .addr       = (ಶೂನ್ಯ*)(&hdr),
            .length     = ಪರಿಮಾಣ(hdr),
        },
        {
            .addr       = op->buf_fl,
            .length     = vdrive->drive.blksize * op->ಎಣಿಕೆ,
        },
        {
            .addr       = (ಶೂನ್ಯ*)(&status),
            .length     = ಪರಿಮಾಣ(status),
        },
    };
    u32 max_io_size =
        vdrive->drive.max_segment_size * vdrive->drive.max_segments;
    u16 blk_ಅಂಕೆ_max;

    ಯದಿ (vdrive->drive.blksize != 0 && max_io_size != 0)
        blk_ಅಂಕೆ_max = (u16) min(max_io_size / vdrive->drive.blksize, 0xffff);
    ಅನ್ಯಥಾ
        /* default blk_num_max if hardware doesnot advise a proper value */
        blk_ಅಂಕೆ_max = 64;

    ಯದಿ (op->ಎಣಿಕೆ <= blk_ಅಂಕೆ_max) {
        virtio_blk_op_one_segment(vdrive, ಲಿಖ, sg);
    } ಅನ್ಯಥಾ {
        ಶೂನ್ಯ *p  = op->buf_fl;
        u16 ಎಣಿಕೆ = op->ಎಣಿಕೆ;

        ಯಾವತ್ (ಎಣಿಕೆ > 0) {
            u16 blk_ಅಂಕೆ = min(ಎಣಿಕೆ, blk_ಅಂಕೆ_max);
            sg[1].length = vdrive->drive.blksize * blk_ಅಂಕೆ;
            sg[1].addr = p;
            virtio_blk_op_one_segment(vdrive, ಲಿಖ, sg);
            ಯದಿ (status == VIRTIO_BLK_S_OK) {
                hdr.sector += blk_ಅಂಕೆ;
                p += sg[1].length;
                ಎಣಿಕೆ -= blk_ಅಂಕೆ;
            } ಅನ್ಯಥಾ {
                ಭಙ್ಗ;
            }
        }
    }
    ಪ್ರತ್ಯಾಗಮ status == VIRTIO_BLK_S_OK ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

ಪೂರ್ಣಾಙ್ಕ
virtio_blk_process_op(ಸಂರಚನಾ disk_op_s *op)
{
    ಯದಿ (! CONFIG_VIRTIO_BLK)
        ಪ್ರತ್ಯಾಗಮ 0;
    ವಿಕಲ್ಪ (op->command) {
    ಪಕ್ಷ CMD_READ:
        ಪ್ರತ್ಯಾಗಮ virtio_blk_op(op, 0);
    ಪಕ್ಷ CMD_WRITE:
        ಪ್ರತ್ಯಾಗಮ virtio_blk_op(op, 1);
    ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ default_process_op(op);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
init_virtio_blk(ಶೂನ್ಯ *data)
{
    ಸಂರಚನಾ pci_device *pci = data;
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk at %pP\n", pci);
    ಸಂರಚನಾ virtiodrive_s *vdrive = malloc_ಕೆಳಗಿನ(ಪರಿಮಾಣ(*vdrive));
    ಯದಿ (!vdrive) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    memset(vdrive, 0, ಪರಿಮಾಣ(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = pci->bdf;

    vp_init_simple(&vdrive->vp, pci);

    ಯದಿ (vdrive->vp.use_modern) {
        ಸಂರಚನಾ vp_device *vp = &vdrive->vp;
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        u64 blk_size = 1ull << VIRTIO_BLK_F_BLK_SIZE;
        u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
        u64 max_segment_size = 1ull << VIRTIO_BLK_F_SIZE_MAX;

        ಯದಿ (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            ಗಚ್ಛ fail;
        }

        features = features & (version1 | iommu_platform | blk_size
                        | max_segments | max_segment_size);
        vp_set_features(vp, features);
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        ಯದಿ (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            ಗಚ್ಛ fail;
        }

        ಯದಿ (features & max_segment_size)
            vdrive->drive.max_segment_size =
                vp_read(&vp->device, ಸಂರಚನಾ virtio_blk_config, size_max);

        ಯದಿ (features & max_segments)
            vdrive->drive.max_segments =
                vp_read(&vp->device, ಸಂರಚನಾ virtio_blk_config, seg_max);

        vdrive->drive.sectors =
            vp_read(&vp->device, ಸಂರಚನಾ virtio_blk_config, capacity);
        ಯದಿ (features & blk_size) {
            vdrive->drive.blksize =
                vp_read(&vp->device, ಸಂರಚನಾ virtio_blk_config, blk_size);
        } ಅನ್ಯಥಾ {
            vdrive->drive.blksize = DISK_SECTOR_SIZE;
        }
        ಯದಿ (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            ಗಚ್ಛ fail;
        }
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
                vdrive->drive.max_segments);

        vdrive->drive.pchs.cylinder =
            vp_read(&vp->device, ಸಂರಚನಾ virtio_blk_config, cylinders);
        vdrive->drive.pchs.head =
            vp_read(&vp->device, ಸಂರಚನಾ virtio_blk_config, heads);
        vdrive->drive.pchs.sector =
            vp_read(&vp->device, ಸಂರಚನಾ virtio_blk_config, sectors);
    }

    ಯದಿ (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk %pP\n", pci);
        ಗಚ್ಛ fail;
    }

    ಯದಿ (!vdrive->vp.use_modern) {
        ಸಂರಚನಾ virtio_blk_config cfg;
        vp_get_legacy(&vdrive->vp, 0, &cfg, ಪರಿಮಾಣ(cfg));

        u64 f = vp_get_features(&vdrive->vp);
        vdrive->drive.blksize = (f & (1 << VIRTIO_BLK_F_BLK_SIZE)) ?
            cfg.blk_size : DISK_SECTOR_SIZE;

        ಯದಿ (f & (1 << VIRTIO_BLK_F_SIZE_MAX))
            vdrive->drive.max_segment_size = cfg.size_max;

        ಯದಿ (f & (1 << VIRTIO_BLK_F_SEG_MAX))
            vdrive->drive.max_segments = cfg.seg_max;

        vdrive->drive.sectors = cfg.capacity;
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
                vdrive->drive.max_segments);

        ಯದಿ (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            ಗಚ್ಛ fail;
        }
        vdrive->drive.pchs.cylinder = cfg.cylinders;
        vdrive->drive.pchs.head = cfg.heads;
        vdrive->drive.pchs.sector = cfg.sectors;
    }

    ವರ್ಣ *desc = znprintf(MAXDESCSIZE, "Virtio disk PCI:%pP", pci);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_pci_device(pci));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);

    boot_lchs_find_pci_device(pci, &vdrive->drive.lchs);
    ಪ್ರತ್ಯಾಗಮ;

fail:
    vp_reset(&vdrive->vp);
    ಮುಕ್ತ(vdrive->vq);
    ಮುಕ್ತ(vdrive);
}

ಶೂನ್ಯ
init_virtio_blk_mmio(ಶೂನ್ಯ *mmio)
{
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk-mmio at %p\n", mmio);
    ಸಂರಚನಾ virtiodrive_s *vdrive = malloc_ಕೆಳಗಿನ(ಪರಿಮಾಣ(*vdrive));
    ಯದಿ (!vdrive) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ;
    }
    memset(vdrive, 0, ಪರಿಮಾಣ(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = (u32)mmio;

    vp_init_mmio(&vdrive->vp, mmio);

    ಸಂರಚನಾ vp_device *vp = &vdrive->vp;
    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_VERSION_1;
    u64 blk_size = 1ull << VIRTIO_BLK_F_BLK_SIZE;
    u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
    u64 max_segment_size = 1ull << VIRTIO_BLK_F_SIZE_MAX;

    features = features & (version1 | blk_size
            | max_segments | max_segment_size);
    vp_set_features(vp, features);
    status |= VIRTIO_CONFIG_S_FEATURES_OK;
    vp_set_status(vp, status);
    ಯದಿ (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
        dprintf(1, "device didn't accept features: %p\n", mmio);
        ಗಚ್ಛ fail;
    }

    ಯದಿ (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk-mmio %p\n", mmio);
        ಗಚ್ಛ fail;
    }

    ಯದಿ (features & max_segment_size)
        vdrive->drive.max_segment_size =
            vp_read(&vp->device, ಸಂರಚನಾ virtio_blk_config, size_max);

    ಯದಿ (features & max_segments)
        vdrive->drive.max_segments =
            vp_read(&vp->device, ಸಂರಚನಾ virtio_blk_config, seg_max);

    vdrive->drive.sectors =
        vp_read(&vp->device, ಸಂರಚನಾ virtio_blk_config, capacity);
    ಯದಿ (features & blk_size) {
        vdrive->drive.blksize =
            vp_read(&vp->device, ಸಂರಚನಾ virtio_blk_config, blk_size);
    } ಅನ್ಯಥಾ {
        vdrive->drive.blksize = DISK_SECTOR_SIZE;
    }
    ಯದಿ (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
        dprintf(1, "virtio-blk-mmio %p block size %d is unsupported\n",
                mmio, vdrive->drive.blksize);
        ಗಚ್ಛ fail;
    }
    dprintf(1, "virtio-blk-mmio %p blksize=%d sectors=%u size_max=%u "
            "seg_max=%u.\n", mmio, vdrive->drive.blksize,
            (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
            vdrive->drive.max_segments);

    vdrive->drive.pchs.cylinder =
        vp_read(&vp->device, ಸಂರಚನಾ virtio_blk_config, cylinders);
    vdrive->drive.pchs.head =
        vp_read(&vp->device, ಸಂರಚನಾ virtio_blk_config, heads);
    vdrive->drive.pchs.sector =
        vp_read(&vp->device, ಸಂರಚನಾ virtio_blk_config, sectors);

    ವರ್ಣ *desc = znprintf(MAXDESCSIZE, "Virtio disk mmio:%p", mmio);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_mmio_device(mmio));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);
    ಪ್ರತ್ಯಾಗಮ;

fail:
    vp_reset(&vdrive->vp);
    ಮುಕ್ತ(vdrive->vq);
    ಮುಕ್ತ(vdrive);
}

ಶೂನ್ಯ
virtio_blk_setup(ಶೂನ್ಯ)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    ಯದಿ (! CONFIG_VIRTIO_BLK)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(3, "init virtio-blk\n");

    ಸಂರಚನಾ pci_device *pci;
    foreachpci(pci) {
        ಯದಿ (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_BLK_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_BLK_10))
            ಅನುವರ್ತ;

        ಯದಿ (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-blk at %pP\n",
                    pci);
            ಅನುವರ್ತ;
        }

        run_thread(init_virtio_blk, pci);
    }
}

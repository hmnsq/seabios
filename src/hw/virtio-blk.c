// Virtio block boot support.
//
// Copyright (C) 2010 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "biosvar.h" // GET_GLOBALFLAT
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "block.h" // struct drive_s
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pcidevice.h" // foreachpci
#అన్తర్భావ "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#అన్తర్భావ "pci_regs.h" // PCI_VENDOR_ID
#అన్తర్భావ "stacks.h" // run_thread
#అన్తర్భావ "std/disk.h" // DISK_RET_SUCCESS
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // usleep, bootprio_find_pci_device, is_bootprio_strict
#అన్తర్భావ "virtio-pci.h"
#అన్తర్భావ "virtio-mmio.h"
#అన్తర్భావ "virtio-ring.h"
#అన్తర్భావ "virtio-blk.h"

#నిర్వచించు min(a, b) ({\
		typeof(a) _a = a;\
		typeof(b) _b = b;\
		_a < _b ? _a : _b; })

నిర్మాణం virtiodrive_s {
    నిర్మాణం drive_s drive;
    నిర్మాణం vring_virtqueue *vq;
    నిర్మాణం vp_device vp;
};

శూన్య
virtio_blk_op_one_segment(నిర్మాణం virtiodrive_s *vdrive,
    పూర్ణాంకం లిఖ, నిర్మాణం vring_list sg[])
{
    నిర్మాణం vring_virtqueue *vq = vdrive->vq;

    /* Add to virtqueue and kick host */
    ఒకవేళ (లిఖ)
        vring_కూడు_buf(vq, sg, 2, 1, 0, 0);
    లేకపోతే
        vring_కూడు_buf(vq, sg, 1, 2, 0, 0);
    vring_kick(&vdrive->vp, vq, 1);

    /* Wait for reply */
    అయితే (!vring_మరిన్ని_వాడిన(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, ఖాళీ);

    /**
    ** Clear interrupt status register. Avoid leaving interrupts stuck
    ** if VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
    **/
    vp_get_isr(&vdrive->vp);
}

స్థిర పూర్ణాంకం
virtio_blk_op(నిర్మాణం disk_op_s *op, పూర్ణాంకం లిఖ)
{
    నిర్మాణం virtiodrive_s *vdrive =
        container_యొక్క(op->drive_fl, నిర్మాణం virtiodrive_s, drive);
    నిర్మాణం virtio_blk_outhdr hdr = {
        .type = లిఖ ? VIRTIO_BLK_T_బహిర్గతం : VIRTIO_BLK_T_లోపల,
        .ioprio = 0,
        .sector = op->lba,
    };
    u8 స్థితి_2 = VIRTIO_BLK_S_UNSUPP;
    నిర్మాణం vring_list sg[] = {
        {
            .addr       = (శూన్య*)(&hdr),
            .నిడివి     = పరిమాణం(hdr),
        },
        {
            .addr       = op->buf_fl,
            .నిడివి     = vdrive->drive.blksize * op->లెక్క,
        },
        {
            .addr       = (శూన్య*)(&స్థితి_2),
            .నిడివి     = పరిమాణం(స్థితి_2),
        },
    };
    u32 max_io_కొలత =
        vdrive->drive.max_segment_కొలత * vdrive->drive.max_segments;
    u16 blk_అంకె_max;

    ఒకవేళ (vdrive->drive.blksize != 0 && max_io_కొలత != 0)
        blk_అంకె_max = (u16) min(max_io_కొలత / vdrive->drive.blksize, 0xffff);
    లేకపోతే
        /* default blk_num_max if hardware doesnot advise a proper value */
        blk_అంకె_max = 64;

    ఒకవేళ (op->లెక్క <= blk_అంకె_max) {
        virtio_blk_op_one_segment(vdrive, లిఖ, sg);
    } లేకపోతే {
        శూన్య *p  = op->buf_fl;
        u16 లెక్క = op->లెక్క;

        అయితే (లెక్క > 0) {
            u16 blk_అంకె = min(లెక్క, blk_అంకె_max);
            sg[1].నిడివి = vdrive->drive.blksize * blk_అంకె;
            sg[1].addr = p;
            virtio_blk_op_one_segment(vdrive, లిఖ, sg);
            ఒకవేళ (స్థితి_2 == VIRTIO_BLK_S_OK) {
                hdr.sector += blk_అంకె;
                p += sg[1].నిడివి;
                లెక్క -= blk_అంకె;
            } లేకపోతే {
                ఆపు;
            }
        }
    }
    తిరిగి స్థితి_2 == VIRTIO_BLK_S_OK ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

పూర్ణాంకం
virtio_blk_process_op(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (! CONFIG_VIRTIO_BLK)
        తిరిగి 0;
    ఎంపిక (op->command) {
    స్థితి CMD_READ:
        తిరిగి virtio_blk_op(op, 0);
    స్థితి CMD_WRITE:
        తిరిగి virtio_blk_op(op, 1);
    అప్రమేయ:
        తిరిగి default_process_op(op);
    }
}

స్థిర శూన్య
init_virtio_blk(శూన్య *దత్తాంశం)
{
    నిర్మాణం pci_device *pci = దత్తాంశం;
    u8 స్థితి_2 = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk at %pP\n", pci);
    నిర్మాణం virtiodrive_s *vdrive = malloc_దిగువ(పరిమాణం(*vdrive));
    ఒకవేళ (!vdrive) {
        warn_noalloc();
        తిరిగి;
    }
    memset(vdrive, 0, పరిమాణం(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = pci->bdf;

    vp_init_simple(&vdrive->vp, pci);

    ఒకవేళ (vdrive->vp.use_modern) {
        నిర్మాణం vp_device *vp = &vdrive->vp;
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_సంస్కరణ_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        u64 blk_కొలత = 1ull << VIRTIO_BLK_F_BLK_కొలత;
        u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
        u64 max_segment_కొలత = 1ull << VIRTIO_BLK_F_కొలత_MAX;

        ఒకవేళ (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            వెళ్ళు fail;
        }

        features = features & (version1 | iommu_platform | blk_కొలత
                        | max_segments | max_segment_కొలత);
        vp_సమితి_features(vp, features);
        స్థితి_2 |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_సమితి_స్థితి(vp, స్థితి_2);
        ఒకవేళ (!(vp_get_స్థితి(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            వెళ్ళు fail;
        }

        ఒకవేళ (features & max_segment_కొలత)
            vdrive->drive.max_segment_కొలత =
                vp_read(&vp->device, నిర్మాణం virtio_blk_config, కొలత_max);

        ఒకవేళ (features & max_segments)
            vdrive->drive.max_segments =
                vp_read(&vp->device, నిర్మాణం virtio_blk_config, seg_max);

        vdrive->drive.sectors =
            vp_read(&vp->device, నిర్మాణం virtio_blk_config, సామర్థ్యం);
        ఒకవేళ (features & blk_కొలత) {
            vdrive->drive.blksize =
                vp_read(&vp->device, నిర్మాణం virtio_blk_config, blk_కొలత);
        } లేకపోతే {
            vdrive->drive.blksize = DISK_SECTOR_కొలత;
        }
        ఒకవేళ (vdrive->drive.blksize != DISK_SECTOR_కొలత) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            వెళ్ళు fail;
        }
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_కొలత,
                vdrive->drive.max_segments);

        vdrive->drive.pchs.cylinder =
            vp_read(&vp->device, నిర్మాణం virtio_blk_config, cylinders);
        vdrive->drive.pchs.తల =
            vp_read(&vp->device, నిర్మాణం virtio_blk_config, heads);
        vdrive->drive.pchs.sector =
            vp_read(&vp->device, నిర్మాణం virtio_blk_config, sectors);
    }

    ఒకవేళ (vp_కనుగొను_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk %pP\n", pci);
        వెళ్ళు fail;
    }

    ఒకవేళ (!vdrive->vp.use_modern) {
        నిర్మాణం virtio_blk_config cfg;
        vp_get_legacy(&vdrive->vp, 0, &cfg, పరిమాణం(cfg));

        u64 f = vp_get_features(&vdrive->vp);
        vdrive->drive.blksize = (f & (1 << VIRTIO_BLK_F_BLK_కొలత)) ?
            cfg.blk_కొలత : DISK_SECTOR_కొలత;

        ఒకవేళ (f & (1 << VIRTIO_BLK_F_కొలత_MAX))
            vdrive->drive.max_segment_కొలత = cfg.కొలత_max;

        ఒకవేళ (f & (1 << VIRTIO_BLK_F_SEG_MAX))
            vdrive->drive.max_segments = cfg.seg_max;

        vdrive->drive.sectors = cfg.సామర్థ్యం;
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_కొలత,
                vdrive->drive.max_segments);

        ఒకవేళ (vdrive->drive.blksize != DISK_SECTOR_కొలత) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            వెళ్ళు fail;
        }
        vdrive->drive.pchs.cylinder = cfg.cylinders;
        vdrive->drive.pchs.తల = cfg.heads;
        vdrive->drive.pchs.sector = cfg.sectors;
    }

    అక్షరం *desc = znprintf(MAXDESCSIZE, "Virtio disk PCI:%pP", pci);
    boot_కూడు_hd(&vdrive->drive, desc, bootprio_కనుగొను_pci_device(pci));

    స్థితి_2 |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_సమితి_స్థితి(&vdrive->vp, స్థితి_2);

    boot_lchs_కనుగొను_pci_device(pci, &vdrive->drive.lchs);
    తిరిగి;

fail:
    vp_reset(&vdrive->vp);
    విడుదల(vdrive->vq);
    విడుదల(vdrive);
}

శూన్య
init_virtio_blk_mmio(శూన్య *mmio)
{
    u8 స్థితి_2 = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk-mmio at %p\n", mmio);
    నిర్మాణం virtiodrive_s *vdrive = malloc_దిగువ(పరిమాణం(*vdrive));
    ఒకవేళ (!vdrive) {
        warn_noalloc();
        తిరిగి;
    }
    memset(vdrive, 0, పరిమాణం(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = (u32)mmio;

    vp_init_mmio(&vdrive->vp, mmio);

    నిర్మాణం vp_device *vp = &vdrive->vp;
    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_సంస్కరణ_1;
    u64 blk_కొలత = 1ull << VIRTIO_BLK_F_BLK_కొలత;
    u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
    u64 max_segment_కొలత = 1ull << VIRTIO_BLK_F_కొలత_MAX;

    features = features & (version1 | blk_కొలత
            | max_segments | max_segment_కొలత);
    vp_సమితి_features(vp, features);
    స్థితి_2 |= VIRTIO_CONFIG_S_FEATURES_OK;
    vp_సమితి_స్థితి(vp, స్థితి_2);
    ఒకవేళ (!(vp_get_స్థితి(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
        dprintf(1, "device didn't accept features: %p\n", mmio);
        వెళ్ళు fail;
    }

    ఒకవేళ (vp_కనుగొను_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk-mmio %p\n", mmio);
        వెళ్ళు fail;
    }

    ఒకవేళ (features & max_segment_కొలత)
        vdrive->drive.max_segment_కొలత =
            vp_read(&vp->device, నిర్మాణం virtio_blk_config, కొలత_max);

    ఒకవేళ (features & max_segments)
        vdrive->drive.max_segments =
            vp_read(&vp->device, నిర్మాణం virtio_blk_config, seg_max);

    vdrive->drive.sectors =
        vp_read(&vp->device, నిర్మాణం virtio_blk_config, సామర్థ్యం);
    ఒకవేళ (features & blk_కొలత) {
        vdrive->drive.blksize =
            vp_read(&vp->device, నిర్మాణం virtio_blk_config, blk_కొలత);
    } లేకపోతే {
        vdrive->drive.blksize = DISK_SECTOR_కొలత;
    }
    ఒకవేళ (vdrive->drive.blksize != DISK_SECTOR_కొలత) {
        dprintf(1, "virtio-blk-mmio %p block size %d is unsupported\n",
                mmio, vdrive->drive.blksize);
        వెళ్ళు fail;
    }
    dprintf(1, "virtio-blk-mmio %p blksize=%d sectors=%u size_max=%u "
            "seg_max=%u.\n", mmio, vdrive->drive.blksize,
            (u32)vdrive->drive.sectors, vdrive->drive.max_segment_కొలత,
            vdrive->drive.max_segments);

    vdrive->drive.pchs.cylinder =
        vp_read(&vp->device, నిర్మాణం virtio_blk_config, cylinders);
    vdrive->drive.pchs.తల =
        vp_read(&vp->device, నిర్మాణం virtio_blk_config, heads);
    vdrive->drive.pchs.sector =
        vp_read(&vp->device, నిర్మాణం virtio_blk_config, sectors);

    అక్షరం *desc = znprintf(MAXDESCSIZE, "Virtio disk mmio:%p", mmio);
    boot_కూడు_hd(&vdrive->drive, desc, bootprio_కనుగొను_mmio_device(mmio));

    స్థితి_2 |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_సమితి_స్థితి(&vdrive->vp, స్థితి_2);
    తిరిగి;

fail:
    vp_reset(&vdrive->vp);
    విడుదల(vdrive->vq);
    విడుదల(vdrive);
}

శూన్య
virtio_blk_అమరిక(శూన్య)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    ఒకవేళ (! CONFIG_VIRTIO_BLK)
        తిరిగి;

    dprintf(3, "init virtio-blk\n");

    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_BLK_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_BLK_10))
            కొనసాగు;

        ఒకవేళ (skip_nonbootable && bootprio_కనుగొను_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-blk at %pP\n",
                    pci);
            కొనసాగు;
        }

        నిరంతరం_thread(init_virtio_blk, pci);
    }
}

// Virtio block boot support.
//
// Copyright (C) 2010 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "biosvar.h" // GET_GLOBALFLAT
#অন্তর্ভাব "config.h" // CONFIG_*
#অন্তর্ভাব "block.h" // struct drive_s
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
#অন্তর্ভাব "virtio-mmio.h"
#অন্তর্ভাব "virtio-ring.h"
#অন্তর্ভাব "virtio-blk.h"

#define min(a, b) ({\
		typeof(a) _a = a;\
		typeof(b) _b = b;\
		_a < _b ? _a : _b; })

কাঠামো virtiodrive_s {
    কাঠামো drive_s drive;
    কাঠামো vring_virtqueue *vq;
    কাঠামো vp_device vp;
};

শূন্য
virtio_blk_op_one_segment(কাঠামো virtiodrive_s *vdrive,
    পূর্ণসংখ্যা লিখ, কাঠামো vring_list sg[])
{
    কাঠামো vring_virtqueue *vq = vdrive->vq;

    /* Add to virtqueue and kick host */
    যদি (লিখ)
        vring_add_buf(vq, sg, 2, 1, 0, 0);
    অন্যথা
        vring_add_buf(vq, sg, 1, 2, 0, 0);
    vring_kick(&vdrive->vp, vq, 1);

    /* Wait for reply */
    যখন (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, ফাঁকা);

    /**
    ** Clear interrupt status register. Avoid leaving interrupts stuck
    ** if VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
    **/
    vp_get_isr(&vdrive->vp);
}

স্থির পূর্ণসংখ্যা
virtio_blk_op(কাঠামো disk_op_s *op, পূর্ণসংখ্যা লিখ)
{
    কাঠামো virtiodrive_s *vdrive =
        container_of(op->drive_fl, কাঠামো virtiodrive_s, drive);
    কাঠামো virtio_blk_outhdr hdr = {
        .type = লিখ ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN,
        .ioprio = 0,
        .sector = op->lba,
    };
    u8 status = VIRTIO_BLK_S_UNSUPP;
    কাঠামো vring_list sg[] = {
        {
            .addr       = (শূন্য*)(&hdr),
            .দৈর্ঘ্য     = আকার(hdr),
        },
        {
            .addr       = op->buf_fl,
            .দৈর্ঘ্য     = vdrive->drive.blksize * op->গুনতি,
        },
        {
            .addr       = (শূন্য*)(&status),
            .দৈর্ঘ্য     = আকার(status),
        },
    };
    u32 max_io_আকার =
        vdrive->drive.max_segment_আকার * vdrive->drive.max_segments;
    u16 blk_অঙ্ক_max;

    যদি (vdrive->drive.blksize != 0 && max_io_আকার != 0)
        blk_অঙ্ক_max = (u16) min(max_io_আকার / vdrive->drive.blksize, 0xffff);
    অন্যথা
        /* default blk_num_max if hardware doesnot advise a proper value */
        blk_অঙ্ক_max = 64;

    যদি (op->গুনতি <= blk_অঙ্ক_max) {
        virtio_blk_op_one_segment(vdrive, লিখ, sg);
    } অন্যথা {
        শূন্য *p  = op->buf_fl;
        u16 গুনতি = op->গুনতি;

        যখন (গুনতি > 0) {
            u16 blk_অঙ্ক = min(গুনতি, blk_অঙ্ক_max);
            sg[1].দৈর্ঘ্য = vdrive->drive.blksize * blk_অঙ্ক;
            sg[1].addr = p;
            virtio_blk_op_one_segment(vdrive, লিখ, sg);
            যদি (status == VIRTIO_BLK_S_OK) {
                hdr.sector += blk_অঙ্ক;
                p += sg[1].দৈর্ঘ্য;
                গুনতি -= blk_অঙ্ক;
            } অন্যথা {
                বিরতি;
            }
        }
    }
    ফেরত status == VIRTIO_BLK_S_OK ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

পূর্ণসংখ্যা
virtio_blk_process_op(কাঠামো disk_op_s *op)
{
    যদি (! CONFIG_VIRTIO_BLK)
        ফেরত 0;
    নির্বাচন (op->command) {
    ক্ষেত্র CMD_READ:
        ফেরত virtio_blk_op(op, 0);
    ক্ষেত্র CMD_WRITE:
        ফেরত virtio_blk_op(op, 1);
    পূর্বনির্ধারিত:
        ফেরত default_process_op(op);
    }
}

স্থির শূন্য
init_virtio_blk(শূন্য *তথ্য)
{
    কাঠামো pci_device *pci = তথ্য;
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk at %pP\n", pci);
    কাঠামো virtiodrive_s *vdrive = malloc_নিচের(আকার(*vdrive));
    যদি (!vdrive) {
        warn_noalloc();
        ফেরত;
    }
    memset(vdrive, 0, আকার(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = pci->bdf;

    vp_init_simple(&vdrive->vp, pci);

    যদি (vdrive->vp.use_modern) {
        কাঠামো vp_device *vp = &vdrive->vp;
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        u64 blk_আকার = 1ull << VIRTIO_BLK_F_BLK_আকার;
        u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
        u64 max_segment_আকার = 1ull << VIRTIO_BLK_F_আকার_MAX;

        যদি (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            যাও fail;
        }

        features = features & (version1 | iommu_platform | blk_আকার
                        | max_segments | max_segment_আকার);
        vp_set_features(vp, features);
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        যদি (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            যাও fail;
        }

        যদি (features & max_segment_আকার)
            vdrive->drive.max_segment_আকার =
                vp_read(&vp->device, কাঠামো virtio_blk_config, আকার_max);

        যদি (features & max_segments)
            vdrive->drive.max_segments =
                vp_read(&vp->device, কাঠামো virtio_blk_config, seg_max);

        vdrive->drive.sectors =
            vp_read(&vp->device, কাঠামো virtio_blk_config, capacity);
        যদি (features & blk_আকার) {
            vdrive->drive.blksize =
                vp_read(&vp->device, কাঠামো virtio_blk_config, blk_আকার);
        } অন্যথা {
            vdrive->drive.blksize = DISK_SECTOR_আকার;
        }
        যদি (vdrive->drive.blksize != DISK_SECTOR_আকার) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            যাও fail;
        }
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_আকার,
                vdrive->drive.max_segments);

        vdrive->drive.pchs.cylinder =
            vp_read(&vp->device, কাঠামো virtio_blk_config, cylinders);
        vdrive->drive.pchs.শীর্ষ =
            vp_read(&vp->device, কাঠামো virtio_blk_config, heads);
        vdrive->drive.pchs.sector =
            vp_read(&vp->device, কাঠামো virtio_blk_config, sectors);
    }

    যদি (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk %pP\n", pci);
        যাও fail;
    }

    যদি (!vdrive->vp.use_modern) {
        কাঠামো virtio_blk_config cfg;
        vp_get_legacy(&vdrive->vp, 0, &cfg, আকার(cfg));

        u64 f = vp_get_features(&vdrive->vp);
        vdrive->drive.blksize = (f & (1 << VIRTIO_BLK_F_BLK_আকার)) ?
            cfg.blk_আকার : DISK_SECTOR_আকার;

        যদি (f & (1 << VIRTIO_BLK_F_আকার_MAX))
            vdrive->drive.max_segment_আকার = cfg.আকার_max;

        যদি (f & (1 << VIRTIO_BLK_F_SEG_MAX))
            vdrive->drive.max_segments = cfg.seg_max;

        vdrive->drive.sectors = cfg.capacity;
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_আকার,
                vdrive->drive.max_segments);

        যদি (vdrive->drive.blksize != DISK_SECTOR_আকার) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            যাও fail;
        }
        vdrive->drive.pchs.cylinder = cfg.cylinders;
        vdrive->drive.pchs.শীর্ষ = cfg.heads;
        vdrive->drive.pchs.sector = cfg.sectors;
    }

    অক্ষর *desc = znprintf(MAXDESCSIZE, "Virtio disk PCI:%pP", pci);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_pci_device(pci));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);

    boot_lchs_find_pci_device(pci, &vdrive->drive.lchs);
    ফেরত;

fail:
    vp_reset(&vdrive->vp);
    মুক্ত(vdrive->vq);
    মুক্ত(vdrive);
}

শূন্য
init_virtio_blk_mmio(শূন্য *mmio)
{
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk-mmio at %p\n", mmio);
    কাঠামো virtiodrive_s *vdrive = malloc_নিচের(আকার(*vdrive));
    যদি (!vdrive) {
        warn_noalloc();
        ফেরত;
    }
    memset(vdrive, 0, আকার(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = (u32)mmio;

    vp_init_mmio(&vdrive->vp, mmio);

    কাঠামো vp_device *vp = &vdrive->vp;
    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_VERSION_1;
    u64 blk_আকার = 1ull << VIRTIO_BLK_F_BLK_আকার;
    u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
    u64 max_segment_আকার = 1ull << VIRTIO_BLK_F_আকার_MAX;

    features = features & (version1 | blk_আকার
            | max_segments | max_segment_আকার);
    vp_set_features(vp, features);
    status |= VIRTIO_CONFIG_S_FEATURES_OK;
    vp_set_status(vp, status);
    যদি (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
        dprintf(1, "device didn't accept features: %p\n", mmio);
        যাও fail;
    }

    যদি (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk-mmio %p\n", mmio);
        যাও fail;
    }

    যদি (features & max_segment_আকার)
        vdrive->drive.max_segment_আকার =
            vp_read(&vp->device, কাঠামো virtio_blk_config, আকার_max);

    যদি (features & max_segments)
        vdrive->drive.max_segments =
            vp_read(&vp->device, কাঠামো virtio_blk_config, seg_max);

    vdrive->drive.sectors =
        vp_read(&vp->device, কাঠামো virtio_blk_config, capacity);
    যদি (features & blk_আকার) {
        vdrive->drive.blksize =
            vp_read(&vp->device, কাঠামো virtio_blk_config, blk_আকার);
    } অন্যথা {
        vdrive->drive.blksize = DISK_SECTOR_আকার;
    }
    যদি (vdrive->drive.blksize != DISK_SECTOR_আকার) {
        dprintf(1, "virtio-blk-mmio %p block size %d is unsupported\n",
                mmio, vdrive->drive.blksize);
        যাও fail;
    }
    dprintf(1, "virtio-blk-mmio %p blksize=%d sectors=%u size_max=%u "
            "seg_max=%u.\n", mmio, vdrive->drive.blksize,
            (u32)vdrive->drive.sectors, vdrive->drive.max_segment_আকার,
            vdrive->drive.max_segments);

    vdrive->drive.pchs.cylinder =
        vp_read(&vp->device, কাঠামো virtio_blk_config, cylinders);
    vdrive->drive.pchs.শীর্ষ =
        vp_read(&vp->device, কাঠামো virtio_blk_config, heads);
    vdrive->drive.pchs.sector =
        vp_read(&vp->device, কাঠামো virtio_blk_config, sectors);

    অক্ষর *desc = znprintf(MAXDESCSIZE, "Virtio disk mmio:%p", mmio);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_mmio_device(mmio));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);
    ফেরত;

fail:
    vp_reset(&vdrive->vp);
    মুক্ত(vdrive->vq);
    মুক্ত(vdrive);
}

শূন্য
virtio_blk_setup(শূন্য)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    যদি (! CONFIG_VIRTIO_BLK)
        ফেরত;

    dprintf(3, "init virtio-blk\n");

    কাঠামো pci_device *pci;
    foreachpci(pci) {
        যদি (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_BLK_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_BLK_10))
            চালু;

        যদি (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-blk at %pP\n",
                    pci);
            চালু;
        }

        run_thread(init_virtio_blk, pci);
    }
}

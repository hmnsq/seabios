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

সংরচনা virtiodrive_s {
    সংরচনা drive_s drive;
    সংরচনা vring_virtqueue *vq;
    সংরচনা vp_device vp;
};

শূন্য
virtio_blk_op_one_segment(সংরচনা virtiodrive_s *vdrive,
    পূর্ণাঙ্ক লিখ, সংরচনা vring_list sg[])
{
    সংরচনা vring_virtqueue *vq = vdrive->vq;

    /* Add to virtqueue and kick host */
    যদি (লিখ)
        vring_add_buf(vq, sg, 2, 1, 0, 0);
    অন্যথা
        vring_add_buf(vq, sg, 1, 2, 0, 0);
    vring_kick(&vdrive->vp, vq, 1);

    /* Wait for reply */
    যাবত্ (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, রিক্ত);

    /**
    ** Clear interrupt status register. Avoid leaving interrupts stuck
    ** if VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
    **/
    vp_get_isr(&vdrive->vp);
}

স্থির পূর্ণাঙ্ক
virtio_blk_op(সংরচনা disk_op_s *op, পূর্ণাঙ্ক লিখ)
{
    সংরচনা virtiodrive_s *vdrive =
        container_of(op->drive_fl, সংরচনা virtiodrive_s, drive);
    সংরচনা virtio_blk_outhdr hdr = {
        .type = লিখ ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN,
        .ioprio = 0,
        .sector = op->lba,
    };
    u8 status = VIRTIO_BLK_S_UNSUPP;
    সংরচনা vring_list sg[] = {
        {
            .addr       = (শূন্য*)(&hdr),
            .length     = পরিমাণ(hdr),
        },
        {
            .addr       = op->buf_fl,
            .length     = vdrive->drive.blksize * op->গুনতি,
        },
        {
            .addr       = (শূন্য*)(&status),
            .length     = পরিমাণ(status),
        },
    };
    u32 max_io_size =
        vdrive->drive.max_segment_size * vdrive->drive.max_segments;
    u16 blk_অঙ্ক_max;

    যদি (vdrive->drive.blksize != 0 && max_io_size != 0)
        blk_অঙ্ক_max = (u16) min(max_io_size / vdrive->drive.blksize, 0xffff);
    অন্যথা
        /* default blk_num_max if hardware doesnot advise a proper value */
        blk_অঙ্ক_max = 64;

    যদি (op->গুনতি <= blk_অঙ্ক_max) {
        virtio_blk_op_one_segment(vdrive, লিখ, sg);
    } অন্যথা {
        শূন্য *p  = op->buf_fl;
        u16 গুনতি = op->গুনতি;

        যাবত্ (গুনতি > 0) {
            u16 blk_অঙ্ক = min(গুনতি, blk_অঙ্ক_max);
            sg[1].length = vdrive->drive.blksize * blk_অঙ্ক;
            sg[1].addr = p;
            virtio_blk_op_one_segment(vdrive, লিখ, sg);
            যদি (status == VIRTIO_BLK_S_OK) {
                hdr.sector += blk_অঙ্ক;
                p += sg[1].length;
                গুনতি -= blk_অঙ্ক;
            } অন্যথা {
                ভঙ্গ;
            }
        }
    }
    প্রত্যাগম status == VIRTIO_BLK_S_OK ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

পূর্ণাঙ্ক
virtio_blk_process_op(সংরচনা disk_op_s *op)
{
    যদি (! CONFIG_VIRTIO_BLK)
        প্রত্যাগম 0;
    বিকল্প (op->command) {
    পক্ষ CMD_READ:
        প্রত্যাগম virtio_blk_op(op, 0);
    পক্ষ CMD_WRITE:
        প্রত্যাগম virtio_blk_op(op, 1);
    স্বভাব:
        প্রত্যাগম default_process_op(op);
    }
}

স্থির শূন্য
init_virtio_blk(শূন্য *data)
{
    সংরচনা pci_device *pci = data;
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk at %pP\n", pci);
    সংরচনা virtiodrive_s *vdrive = malloc_নিচের(পরিমাণ(*vdrive));
    যদি (!vdrive) {
        warn_noalloc();
        প্রত্যাগম;
    }
    memset(vdrive, 0, পরিমাণ(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = pci->bdf;

    vp_init_simple(&vdrive->vp, pci);

    যদি (vdrive->vp.use_modern) {
        সংরচনা vp_device *vp = &vdrive->vp;
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        u64 blk_size = 1ull << VIRTIO_BLK_F_BLK_SIZE;
        u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
        u64 max_segment_size = 1ull << VIRTIO_BLK_F_SIZE_MAX;

        যদি (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            গচ্ছ fail;
        }

        features = features & (version1 | iommu_platform | blk_size
                        | max_segments | max_segment_size);
        vp_set_features(vp, features);
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        যদি (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            গচ্ছ fail;
        }

        যদি (features & max_segment_size)
            vdrive->drive.max_segment_size =
                vp_read(&vp->device, সংরচনা virtio_blk_config, size_max);

        যদি (features & max_segments)
            vdrive->drive.max_segments =
                vp_read(&vp->device, সংরচনা virtio_blk_config, seg_max);

        vdrive->drive.sectors =
            vp_read(&vp->device, সংরচনা virtio_blk_config, capacity);
        যদি (features & blk_size) {
            vdrive->drive.blksize =
                vp_read(&vp->device, সংরচনা virtio_blk_config, blk_size);
        } অন্যথা {
            vdrive->drive.blksize = DISK_SECTOR_SIZE;
        }
        যদি (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            গচ্ছ fail;
        }
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
                vdrive->drive.max_segments);

        vdrive->drive.pchs.cylinder =
            vp_read(&vp->device, সংরচনা virtio_blk_config, cylinders);
        vdrive->drive.pchs.head =
            vp_read(&vp->device, সংরচনা virtio_blk_config, heads);
        vdrive->drive.pchs.sector =
            vp_read(&vp->device, সংরচনা virtio_blk_config, sectors);
    }

    যদি (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk %pP\n", pci);
        গচ্ছ fail;
    }

    যদি (!vdrive->vp.use_modern) {
        সংরচনা virtio_blk_config cfg;
        vp_get_legacy(&vdrive->vp, 0, &cfg, পরিমাণ(cfg));

        u64 f = vp_get_features(&vdrive->vp);
        vdrive->drive.blksize = (f & (1 << VIRTIO_BLK_F_BLK_SIZE)) ?
            cfg.blk_size : DISK_SECTOR_SIZE;

        যদি (f & (1 << VIRTIO_BLK_F_SIZE_MAX))
            vdrive->drive.max_segment_size = cfg.size_max;

        যদি (f & (1 << VIRTIO_BLK_F_SEG_MAX))
            vdrive->drive.max_segments = cfg.seg_max;

        vdrive->drive.sectors = cfg.capacity;
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
                vdrive->drive.max_segments);

        যদি (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            গচ্ছ fail;
        }
        vdrive->drive.pchs.cylinder = cfg.cylinders;
        vdrive->drive.pchs.head = cfg.heads;
        vdrive->drive.pchs.sector = cfg.sectors;
    }

    বর্ণ *desc = znprintf(MAXDESCSIZE, "Virtio disk PCI:%pP", pci);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_pci_device(pci));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);

    boot_lchs_find_pci_device(pci, &vdrive->drive.lchs);
    প্রত্যাগম;

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
    সংরচনা virtiodrive_s *vdrive = malloc_নিচের(পরিমাণ(*vdrive));
    যদি (!vdrive) {
        warn_noalloc();
        প্রত্যাগম;
    }
    memset(vdrive, 0, পরিমাণ(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = (u32)mmio;

    vp_init_mmio(&vdrive->vp, mmio);

    সংরচনা vp_device *vp = &vdrive->vp;
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
    যদি (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
        dprintf(1, "device didn't accept features: %p\n", mmio);
        গচ্ছ fail;
    }

    যদি (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk-mmio %p\n", mmio);
        গচ্ছ fail;
    }

    যদি (features & max_segment_size)
        vdrive->drive.max_segment_size =
            vp_read(&vp->device, সংরচনা virtio_blk_config, size_max);

    যদি (features & max_segments)
        vdrive->drive.max_segments =
            vp_read(&vp->device, সংরচনা virtio_blk_config, seg_max);

    vdrive->drive.sectors =
        vp_read(&vp->device, সংরচনা virtio_blk_config, capacity);
    যদি (features & blk_size) {
        vdrive->drive.blksize =
            vp_read(&vp->device, সংরচনা virtio_blk_config, blk_size);
    } অন্যথা {
        vdrive->drive.blksize = DISK_SECTOR_SIZE;
    }
    যদি (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
        dprintf(1, "virtio-blk-mmio %p block size %d is unsupported\n",
                mmio, vdrive->drive.blksize);
        গচ্ছ fail;
    }
    dprintf(1, "virtio-blk-mmio %p blksize=%d sectors=%u size_max=%u "
            "seg_max=%u.\n", mmio, vdrive->drive.blksize,
            (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
            vdrive->drive.max_segments);

    vdrive->drive.pchs.cylinder =
        vp_read(&vp->device, সংরচনা virtio_blk_config, cylinders);
    vdrive->drive.pchs.head =
        vp_read(&vp->device, সংরচনা virtio_blk_config, heads);
    vdrive->drive.pchs.sector =
        vp_read(&vp->device, সংরচনা virtio_blk_config, sectors);

    বর্ণ *desc = znprintf(MAXDESCSIZE, "Virtio disk mmio:%p", mmio);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_mmio_device(mmio));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);
    প্রত্যাগম;

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
        প্রত্যাগম;

    dprintf(3, "init virtio-blk\n");

    সংরচনা pci_device *pci;
    foreachpci(pci) {
        যদি (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_BLK_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_BLK_10))
            অনুবর্ত;

        যদি (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-blk at %pP\n",
                    pci);
            অনুবর্ত;
        }

        run_thread(init_virtio_blk, pci);
    }
}

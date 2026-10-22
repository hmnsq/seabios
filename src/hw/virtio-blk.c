// Virtio block boot support.
//
// Copyright (C) 2010 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "biosvar.h" // GET_GLOBALFLAT
#شامل "config.h" // CONFIG_*
#شامل "block.h" // struct drive_s
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
#شامل "virtio-mmio.h"
#شامل "virtio-ring.h"
#شامل "virtio-blk.h"

#define min(a, b) ({\
		typeof(a) _a = a;\
		typeof(b) _b = b;\
		_a < _b ? _a : _b; })

ساختار virtiodrive_s {
    ساختار drive_s drive;
    ساختار vring_virtqueue *vq;
    ساختار vp_device vp;
};

تهی
virtio_blk_op_one_segment(ساختار virtiodrive_s *vdrive,
    صحیح اكتب, ساختار vring_list sg[])
{
    ساختار vring_virtqueue *vq = vdrive->vq;

    /* Add to virtqueue and kick host */
    اگر (اكتب)
        vring_add_buf(vq, sg, 2, 1, 0, 0);
    وگرنه
        vring_add_buf(vq, sg, 1, 2, 0, 0);
    vring_kick(&vdrive->vp, vq, 1);

    /* Wait for reply */
    وقتی (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, پوچ);

    /**
    ** Clear interrupt status register. Avoid leaving interrupts stuck
    ** if VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
    **/
    vp_get_isr(&vdrive->vp);
}

ایستا صحیح
virtio_blk_op(ساختار disk_op_s *op, صحیح اكتب)
{
    ساختار virtiodrive_s *vdrive =
        container_of(op->drive_fl, ساختار virtiodrive_s, drive);
    ساختار virtio_blk_outhdr hdr = {
        .type = اكتب ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN,
        .ioprio = 0,
        .sector = op->lba,
    };
    u8 status = VIRTIO_BLK_S_UNSUPP;
    ساختار vring_list sg[] = {
        {
            .addr       = (تهی*)(&hdr),
            .length     = اندازه(hdr),
        },
        {
            .addr       = op->buf_fl,
            .length     = vdrive->drive.blksize * op->شمار,
        },
        {
            .addr       = (تهی*)(&status),
            .length     = اندازه(status),
        },
    };
    u32 max_io_size =
        vdrive->drive.max_segment_size * vdrive->drive.max_segments;
    u16 blk_رقم_max;

    اگر (vdrive->drive.blksize != 0 && max_io_size != 0)
        blk_رقم_max = (u16) min(max_io_size / vdrive->drive.blksize, 0xffff);
    وگرنه
        /* default blk_num_max if hardware doesnot advise a proper value */
        blk_رقم_max = 64;

    اگر (op->شمار <= blk_رقم_max) {
        virtio_blk_op_one_segment(vdrive, اكتب, sg);
    } وگرنه {
        تهی *p  = op->buf_fl;
        u16 شمار = op->شمار;

        وقتی (شمار > 0) {
            u16 blk_رقم = min(شمار, blk_رقم_max);
            sg[1].length = vdrive->drive.blksize * blk_رقم;
            sg[1].addr = p;
            virtio_blk_op_one_segment(vdrive, اكتب, sg);
            اگر (status == VIRTIO_BLK_S_OK) {
                hdr.sector += blk_رقم;
                p += sg[1].length;
                شمار -= blk_رقم;
            } وگرنه {
                بشکن;
            }
        }
    }
    بازگشت status == VIRTIO_BLK_S_OK ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

صحیح
virtio_blk_process_op(ساختار disk_op_s *op)
{
    اگر (! CONFIG_VIRTIO_BLK)
        بازگشت 0;
    انتخاب (op->command) {
    حالت CMD_READ:
        بازگشت virtio_blk_op(op, 0);
    حالت CMD_WRITE:
        بازگشت virtio_blk_op(op, 1);
    پیشفرض:
        بازگشت default_process_op(op);
    }
}

ایستا تهی
init_virtio_blk(تهی *data)
{
    ساختار pci_device *pci = data;
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk at %pP\n", pci);
    ساختار virtiodrive_s *vdrive = malloc_کم(اندازه(*vdrive));
    اگر (!vdrive) {
        warn_noalloc();
        بازگشت;
    }
    memset(vdrive, 0, اندازه(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = pci->bdf;

    vp_init_simple(&vdrive->vp, pci);

    اگر (vdrive->vp.use_modern) {
        ساختار vp_device *vp = &vdrive->vp;
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        u64 blk_size = 1ull << VIRTIO_BLK_F_BLK_SIZE;
        u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
        u64 max_segment_size = 1ull << VIRTIO_BLK_F_SIZE_MAX;

        اگر (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            برو fail;
        }

        features = features & (version1 | iommu_platform | blk_size
                        | max_segments | max_segment_size);
        vp_set_features(vp, features);
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        اگر (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            برو fail;
        }

        اگر (features & max_segment_size)
            vdrive->drive.max_segment_size =
                vp_read(&vp->device, ساختار virtio_blk_config, size_max);

        اگر (features & max_segments)
            vdrive->drive.max_segments =
                vp_read(&vp->device, ساختار virtio_blk_config, seg_max);

        vdrive->drive.sectors =
            vp_read(&vp->device, ساختار virtio_blk_config, capacity);
        اگر (features & blk_size) {
            vdrive->drive.blksize =
                vp_read(&vp->device, ساختار virtio_blk_config, blk_size);
        } وگرنه {
            vdrive->drive.blksize = DISK_SECTOR_SIZE;
        }
        اگر (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            برو fail;
        }
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
                vdrive->drive.max_segments);

        vdrive->drive.pchs.cylinder =
            vp_read(&vp->device, ساختار virtio_blk_config, cylinders);
        vdrive->drive.pchs.head =
            vp_read(&vp->device, ساختار virtio_blk_config, heads);
        vdrive->drive.pchs.sector =
            vp_read(&vp->device, ساختار virtio_blk_config, sectors);
    }

    اگر (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk %pP\n", pci);
        برو fail;
    }

    اگر (!vdrive->vp.use_modern) {
        ساختار virtio_blk_config cfg;
        vp_get_legacy(&vdrive->vp, 0, &cfg, اندازه(cfg));

        u64 f = vp_get_features(&vdrive->vp);
        vdrive->drive.blksize = (f & (1 << VIRTIO_BLK_F_BLK_SIZE)) ?
            cfg.blk_size : DISK_SECTOR_SIZE;

        اگر (f & (1 << VIRTIO_BLK_F_SIZE_MAX))
            vdrive->drive.max_segment_size = cfg.size_max;

        اگر (f & (1 << VIRTIO_BLK_F_SEG_MAX))
            vdrive->drive.max_segments = cfg.seg_max;

        vdrive->drive.sectors = cfg.capacity;
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
                vdrive->drive.max_segments);

        اگر (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            برو fail;
        }
        vdrive->drive.pchs.cylinder = cfg.cylinders;
        vdrive->drive.pchs.head = cfg.heads;
        vdrive->drive.pchs.sector = cfg.sectors;
    }

    نویسه *desc = znprintf(MAXDESCSIZE, "Virtio disk PCI:%pP", pci);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_pci_device(pci));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);

    boot_lchs_find_pci_device(pci, &vdrive->drive.lchs);
    بازگشت;

fail:
    vp_reset(&vdrive->vp);
    آزاد(vdrive->vq);
    آزاد(vdrive);
}

تهی
init_virtio_blk_mmio(تهی *mmio)
{
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk-mmio at %p\n", mmio);
    ساختار virtiodrive_s *vdrive = malloc_کم(اندازه(*vdrive));
    اگر (!vdrive) {
        warn_noalloc();
        بازگشت;
    }
    memset(vdrive, 0, اندازه(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = (u32)mmio;

    vp_init_mmio(&vdrive->vp, mmio);

    ساختار vp_device *vp = &vdrive->vp;
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
    اگر (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
        dprintf(1, "device didn't accept features: %p\n", mmio);
        برو fail;
    }

    اگر (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk-mmio %p\n", mmio);
        برو fail;
    }

    اگر (features & max_segment_size)
        vdrive->drive.max_segment_size =
            vp_read(&vp->device, ساختار virtio_blk_config, size_max);

    اگر (features & max_segments)
        vdrive->drive.max_segments =
            vp_read(&vp->device, ساختار virtio_blk_config, seg_max);

    vdrive->drive.sectors =
        vp_read(&vp->device, ساختار virtio_blk_config, capacity);
    اگر (features & blk_size) {
        vdrive->drive.blksize =
            vp_read(&vp->device, ساختار virtio_blk_config, blk_size);
    } وگرنه {
        vdrive->drive.blksize = DISK_SECTOR_SIZE;
    }
    اگر (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
        dprintf(1, "virtio-blk-mmio %p block size %d is unsupported\n",
                mmio, vdrive->drive.blksize);
        برو fail;
    }
    dprintf(1, "virtio-blk-mmio %p blksize=%d sectors=%u size_max=%u "
            "seg_max=%u.\n", mmio, vdrive->drive.blksize,
            (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
            vdrive->drive.max_segments);

    vdrive->drive.pchs.cylinder =
        vp_read(&vp->device, ساختار virtio_blk_config, cylinders);
    vdrive->drive.pchs.head =
        vp_read(&vp->device, ساختار virtio_blk_config, heads);
    vdrive->drive.pchs.sector =
        vp_read(&vp->device, ساختار virtio_blk_config, sectors);

    نویسه *desc = znprintf(MAXDESCSIZE, "Virtio disk mmio:%p", mmio);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_mmio_device(mmio));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);
    بازگشت;

fail:
    vp_reset(&vdrive->vp);
    آزاد(vdrive->vq);
    آزاد(vdrive);
}

تهی
virtio_blk_setup(تهی)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    اگر (! CONFIG_VIRTIO_BLK)
        بازگشت;

    dprintf(3, "init virtio-blk\n");

    ساختار pci_device *pci;
    foreachpci(pci) {
        اگر (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_BLK_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_BLK_10))
            ادامه;

        اگر (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-blk at %pP\n",
                    pci);
            ادامه;
        }

        run_thread(init_virtio_blk, pci);
    }
}

// Virtio block boot support.
//
// Copyright (C) 2010 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "biosvar.h" // GET_GLOBALFLAT
#अन्तर्भाव "config.h" // CONFIG_*
#अन्तर्भाव "block.h" // struct drive_s
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "pcidevice.h" // foreachpci
#अन्तर्भाव "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#अन्तर्भाव "pci_regs.h" // PCI_VENDOR_ID
#अन्तर्भाव "stacks.h" // run_thread
#अन्तर्भाव "std/disk.h" // DISK_RET_SUCCESS
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // usleep, bootprio_find_pci_device, is_bootprio_strict
#अन्तर्भाव "virtio-pci.h"
#अन्तर्भाव "virtio-mmio.h"
#अन्तर्भाव "virtio-ring.h"
#अन्तर्भाव "virtio-blk.h"

#define min(a, b) ({\
		typeof(a) _a = a;\
		typeof(b) _b = b;\
		_a < _b ? _a : _b; })

संरचना virtiodrive_s {
    संरचना drive_s drive;
    संरचना vring_virtqueue *vq;
    संरचना vp_device vp;
};

शून्य
virtio_blk_op_one_segment(संरचना virtiodrive_s *vdrive,
    पूर्णाङ्क लिख, संरचना vring_list sg[])
{
    संरचना vring_virtqueue *vq = vdrive->vq;

    /* Add to virtqueue and kick host */
    यदि (लिख)
        vring_add_buf(vq, sg, 2, 1, 0, 0);
    अन्यथा
        vring_add_buf(vq, sg, 1, 2, 0, 0);
    vring_kick(&vdrive->vp, vq, 1);

    /* Wait for reply */
    यावत् (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, रिक्त);

    /**
    ** Clear interrupt status register. Avoid leaving interrupts stuck
    ** if VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
    **/
    vp_get_isr(&vdrive->vp);
}

स्थिर पूर्णाङ्क
virtio_blk_op(संरचना disk_op_s *op, पूर्णाङ्क लिख)
{
    संरचना virtiodrive_s *vdrive =
        container_of(op->drive_fl, संरचना virtiodrive_s, drive);
    संरचना virtio_blk_outhdr hdr = {
        .type = लिख ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN,
        .ioprio = 0,
        .sector = op->lba,
    };
    u8 status = VIRTIO_BLK_S_UNSUPP;
    संरचना vring_list sg[] = {
        {
            .addr       = (शून्य*)(&hdr),
            .length     = परिमाण(hdr),
        },
        {
            .addr       = op->buf_fl,
            .length     = vdrive->drive.blksize * op->गिनती,
        },
        {
            .addr       = (शून्य*)(&status),
            .length     = परिमाण(status),
        },
    };
    u32 max_io_size =
        vdrive->drive.max_segment_size * vdrive->drive.max_segments;
    u16 blk_अंक_max;

    यदि (vdrive->drive.blksize != 0 && max_io_size != 0)
        blk_अंक_max = (u16) min(max_io_size / vdrive->drive.blksize, 0xffff);
    अन्यथा
        /* default blk_num_max if hardware doesnot advise a proper value */
        blk_अंक_max = 64;

    यदि (op->गिनती <= blk_अंक_max) {
        virtio_blk_op_one_segment(vdrive, लिख, sg);
    } अन्यथा {
        शून्य *p  = op->buf_fl;
        u16 गिनती = op->गिनती;

        यावत् (गिनती > 0) {
            u16 blk_अंक = min(गिनती, blk_अंक_max);
            sg[1].length = vdrive->drive.blksize * blk_अंक;
            sg[1].addr = p;
            virtio_blk_op_one_segment(vdrive, लिख, sg);
            यदि (status == VIRTIO_BLK_S_OK) {
                hdr.sector += blk_अंक;
                p += sg[1].length;
                गिनती -= blk_अंक;
            } अन्यथा {
                भङ्ग;
            }
        }
    }
    प्रत्यागम status == VIRTIO_BLK_S_OK ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

पूर्णाङ्क
virtio_blk_process_op(संरचना disk_op_s *op)
{
    यदि (! CONFIG_VIRTIO_BLK)
        प्रत्यागम 0;
    विकल्प (op->command) {
    पक्ष CMD_READ:
        प्रत्यागम virtio_blk_op(op, 0);
    पक्ष CMD_WRITE:
        प्रत्यागम virtio_blk_op(op, 1);
    स्वभाव:
        प्रत्यागम default_process_op(op);
    }
}

स्थिर शून्य
init_virtio_blk(शून्य *data)
{
    संरचना pci_device *pci = data;
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk at %pP\n", pci);
    संरचना virtiodrive_s *vdrive = malloc_निचला(परिमाण(*vdrive));
    यदि (!vdrive) {
        warn_noalloc();
        प्रत्यागम;
    }
    स_रखो(vdrive, 0, परिमाण(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = pci->bdf;

    vp_init_simple(&vdrive->vp, pci);

    यदि (vdrive->vp.use_modern) {
        संरचना vp_device *vp = &vdrive->vp;
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        u64 blk_size = 1ull << VIRTIO_BLK_F_BLK_SIZE;
        u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
        u64 max_segment_size = 1ull << VIRTIO_BLK_F_SIZE_MAX;

        यदि (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            गच्छ fail;
        }

        features = features & (version1 | iommu_platform | blk_size
                        | max_segments | max_segment_size);
        vp_set_features(vp, features);
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        यदि (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            गच्छ fail;
        }

        यदि (features & max_segment_size)
            vdrive->drive.max_segment_size =
                vp_read(&vp->device, संरचना virtio_blk_config, size_max);

        यदि (features & max_segments)
            vdrive->drive.max_segments =
                vp_read(&vp->device, संरचना virtio_blk_config, seg_max);

        vdrive->drive.sectors =
            vp_read(&vp->device, संरचना virtio_blk_config, capacity);
        यदि (features & blk_size) {
            vdrive->drive.blksize =
                vp_read(&vp->device, संरचना virtio_blk_config, blk_size);
        } अन्यथा {
            vdrive->drive.blksize = DISK_SECTOR_SIZE;
        }
        यदि (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            गच्छ fail;
        }
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
                vdrive->drive.max_segments);

        vdrive->drive.pchs.cylinder =
            vp_read(&vp->device, संरचना virtio_blk_config, cylinders);
        vdrive->drive.pchs.head =
            vp_read(&vp->device, संरचना virtio_blk_config, heads);
        vdrive->drive.pchs.sector =
            vp_read(&vp->device, संरचना virtio_blk_config, sectors);
    }

    यदि (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk %pP\n", pci);
        गच्छ fail;
    }

    यदि (!vdrive->vp.use_modern) {
        संरचना virtio_blk_config cfg;
        vp_get_legacy(&vdrive->vp, 0, &cfg, परिमाण(cfg));

        u64 f = vp_get_features(&vdrive->vp);
        vdrive->drive.blksize = (f & (1 << VIRTIO_BLK_F_BLK_SIZE)) ?
            cfg.blk_size : DISK_SECTOR_SIZE;

        यदि (f & (1 << VIRTIO_BLK_F_SIZE_MAX))
            vdrive->drive.max_segment_size = cfg.size_max;

        यदि (f & (1 << VIRTIO_BLK_F_SEG_MAX))
            vdrive->drive.max_segments = cfg.seg_max;

        vdrive->drive.sectors = cfg.capacity;
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
                vdrive->drive.max_segments);

        यदि (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            गच्छ fail;
        }
        vdrive->drive.pchs.cylinder = cfg.cylinders;
        vdrive->drive.pchs.head = cfg.heads;
        vdrive->drive.pchs.sector = cfg.sectors;
    }

    वर्ण *desc = znprintf(MAXDESCSIZE, "Virtio disk PCI:%pP", pci);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_pci_device(pci));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);

    boot_lchs_find_pci_device(pci, &vdrive->drive.lchs);
    प्रत्यागम;

fail:
    vp_reset(&vdrive->vp);
    मुक्त(vdrive->vq);
    मुक्त(vdrive);
}

शून्य
init_virtio_blk_mmio(शून्य *mmio)
{
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk-mmio at %p\n", mmio);
    संरचना virtiodrive_s *vdrive = malloc_निचला(परिमाण(*vdrive));
    यदि (!vdrive) {
        warn_noalloc();
        प्रत्यागम;
    }
    स_रखो(vdrive, 0, परिमाण(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = (u32)mmio;

    vp_init_mmio(&vdrive->vp, mmio);

    संरचना vp_device *vp = &vdrive->vp;
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
    यदि (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
        dprintf(1, "device didn't accept features: %p\n", mmio);
        गच्छ fail;
    }

    यदि (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk-mmio %p\n", mmio);
        गच्छ fail;
    }

    यदि (features & max_segment_size)
        vdrive->drive.max_segment_size =
            vp_read(&vp->device, संरचना virtio_blk_config, size_max);

    यदि (features & max_segments)
        vdrive->drive.max_segments =
            vp_read(&vp->device, संरचना virtio_blk_config, seg_max);

    vdrive->drive.sectors =
        vp_read(&vp->device, संरचना virtio_blk_config, capacity);
    यदि (features & blk_size) {
        vdrive->drive.blksize =
            vp_read(&vp->device, संरचना virtio_blk_config, blk_size);
    } अन्यथा {
        vdrive->drive.blksize = DISK_SECTOR_SIZE;
    }
    यदि (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
        dprintf(1, "virtio-blk-mmio %p block size %d is unsupported\n",
                mmio, vdrive->drive.blksize);
        गच्छ fail;
    }
    dprintf(1, "virtio-blk-mmio %p blksize=%d sectors=%u size_max=%u "
            "seg_max=%u.\n", mmio, vdrive->drive.blksize,
            (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
            vdrive->drive.max_segments);

    vdrive->drive.pchs.cylinder =
        vp_read(&vp->device, संरचना virtio_blk_config, cylinders);
    vdrive->drive.pchs.head =
        vp_read(&vp->device, संरचना virtio_blk_config, heads);
    vdrive->drive.pchs.sector =
        vp_read(&vp->device, संरचना virtio_blk_config, sectors);

    वर्ण *desc = znprintf(MAXDESCSIZE, "Virtio disk mmio:%p", mmio);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_mmio_device(mmio));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);
    प्रत्यागम;

fail:
    vp_reset(&vdrive->vp);
    मुक्त(vdrive->vq);
    मुक्त(vdrive);
}

शून्य
virtio_blk_setup(शून्य)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    यदि (! CONFIG_VIRTIO_BLK)
        प्रत्यागम;

    dprintf(3, "init virtio-blk\n");

    संरचना pci_device *pci;
    foreachpci(pci) {
        यदि (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_BLK_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_BLK_10))
            अनुवर्त;

        यदि (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-blk at %pP\n",
                    pci);
            अनुवर्त;
        }

        run_thread(init_virtio_blk, pci);
    }
}

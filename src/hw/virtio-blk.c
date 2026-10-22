// Virtio block boot support.
//
// Copyright (C) 2010 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "biosvar.h" // GET_GLOBALFLAT
#அந்தர்பாவ "config.h" // CONFIG_*
#அந்தர்பாவ "block.h" // struct drive_s
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "pcidevice.h" // foreachpci
#அந்தர்பாவ "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#அந்தர்பாவ "pci_regs.h" // PCI_VENDOR_ID
#அந்தர்பாவ "stacks.h" // run_thread
#அந்தர்பாவ "std/disk.h" // DISK_RET_SUCCESS
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // usleep, bootprio_find_pci_device, is_bootprio_strict
#அந்தர்பாவ "virtio-pci.h"
#அந்தர்பாவ "virtio-mmio.h"
#அந்தர்பாவ "virtio-ring.h"
#அந்தர்பாவ "virtio-blk.h"

#define min(a, b) ({\
		typeof(a) _a = a;\
		typeof(b) _b = b;\
		_a < _b ? _a : _b; })

கட்டமைப்பு virtiodrive_s {
    கட்டமைப்பு drive_s drive;
    கட்டமைப்பு vring_virtqueue *vq;
    கட்டமைப்பு vp_device vp;
};

வெறுமை
virtio_blk_op_one_segment(கட்டமைப்பு virtiodrive_s *vdrive,
    முழு லிக, கட்டமைப்பு vring_list sg[])
{
    கட்டமைப்பு vring_virtqueue *vq = vdrive->vq;

    /* Add to virtqueue and kick host */
    எனில் (லிக)
        vring_add_buf(vq, sg, 2, 1, 0, 0);
    இல்லையெனில்
        vring_add_buf(vq, sg, 1, 2, 0, 0);
    vring_kick(&vdrive->vp, vq, 1);

    /* Wait for reply */
    போது (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, வெற்று);

    /**
    ** Clear interrupt status register. Avoid leaving interrupts stuck
    ** if VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
    **/
    vp_get_isr(&vdrive->vp);
}

நிலையான முழு
virtio_blk_op(கட்டமைப்பு disk_op_s *op, முழு லிக)
{
    கட்டமைப்பு virtiodrive_s *vdrive =
        container_of(op->drive_fl, கட்டமைப்பு virtiodrive_s, drive);
    கட்டமைப்பு virtio_blk_outhdr hdr = {
        .type = லிக ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN,
        .ioprio = 0,
        .sector = op->lba,
    };
    u8 status = VIRTIO_BLK_S_UNSUPP;
    கட்டமைப்பு vring_list sg[] = {
        {
            .addr       = (வெறுமை*)(&hdr),
            .length     = அளவு(hdr),
        },
        {
            .addr       = op->buf_fl,
            .length     = vdrive->drive.blksize * op->மொத்தம்,
        },
        {
            .addr       = (வெறுமை*)(&status),
            .length     = அளவு(status),
        },
    };
    u32 max_io_size =
        vdrive->drive.max_segment_size * vdrive->drive.max_segments;
    u16 blk_இலக்கம்_max;

    எனில் (vdrive->drive.blksize != 0 && max_io_size != 0)
        blk_இலக்கம்_max = (u16) min(max_io_size / vdrive->drive.blksize, 0xffff);
    இல்லையெனில்
        /* default blk_num_max if hardware doesnot advise a proper value */
        blk_இலக்கம்_max = 64;

    எனில் (op->மொத்தம் <= blk_இலக்கம்_max) {
        virtio_blk_op_one_segment(vdrive, லிக, sg);
    } இல்லையெனில் {
        வெறுமை *p  = op->buf_fl;
        u16 மொத்தம் = op->மொத்தம்;

        போது (மொத்தம் > 0) {
            u16 blk_இலக்கம் = min(மொத்தம், blk_இலக்கம்_max);
            sg[1].length = vdrive->drive.blksize * blk_இலக்கம்;
            sg[1].addr = p;
            virtio_blk_op_one_segment(vdrive, லிக, sg);
            எனில் (status == VIRTIO_BLK_S_OK) {
                hdr.sector += blk_இலக்கம்;
                p += sg[1].length;
                மொத்தம் -= blk_இலக்கம்;
            } இல்லையெனில் {
                நிறுத்து;
            }
        }
    }
    திரும்பு status == VIRTIO_BLK_S_OK ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

முழு
virtio_blk_process_op(கட்டமைப்பு disk_op_s *op)
{
    எனில் (! CONFIG_VIRTIO_BLK)
        திரும்பு 0;
    தேர்வு (op->command) {
    நிலை CMD_READ:
        திரும்பு virtio_blk_op(op, 0);
    நிலை CMD_WRITE:
        திரும்பு virtio_blk_op(op, 1);
    இயல்பு:
        திரும்பு default_process_op(op);
    }
}

நிலையான வெறுமை
init_virtio_blk(வெறுமை *data)
{
    கட்டமைப்பு pci_device *pci = data;
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk at %pP\n", pci);
    கட்டமைப்பு virtiodrive_s *vdrive = malloc_கீழ்(அளவு(*vdrive));
    எனில் (!vdrive) {
        warn_noalloc();
        திரும்பு;
    }
    memset(vdrive, 0, அளவு(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = pci->bdf;

    vp_init_simple(&vdrive->vp, pci);

    எனில் (vdrive->vp.use_modern) {
        கட்டமைப்பு vp_device *vp = &vdrive->vp;
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        u64 blk_size = 1ull << VIRTIO_BLK_F_BLK_SIZE;
        u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
        u64 max_segment_size = 1ull << VIRTIO_BLK_F_SIZE_MAX;

        எனில் (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            செல் fail;
        }

        features = features & (version1 | iommu_platform | blk_size
                        | max_segments | max_segment_size);
        vp_set_features(vp, features);
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        எனில் (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            செல் fail;
        }

        எனில் (features & max_segment_size)
            vdrive->drive.max_segment_size =
                vp_read(&vp->device, கட்டமைப்பு virtio_blk_config, size_max);

        எனில் (features & max_segments)
            vdrive->drive.max_segments =
                vp_read(&vp->device, கட்டமைப்பு virtio_blk_config, seg_max);

        vdrive->drive.sectors =
            vp_read(&vp->device, கட்டமைப்பு virtio_blk_config, capacity);
        எனில் (features & blk_size) {
            vdrive->drive.blksize =
                vp_read(&vp->device, கட்டமைப்பு virtio_blk_config, blk_size);
        } இல்லையெனில் {
            vdrive->drive.blksize = DISK_SECTOR_SIZE;
        }
        எனில் (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            செல் fail;
        }
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
                vdrive->drive.max_segments);

        vdrive->drive.pchs.cylinder =
            vp_read(&vp->device, கட்டமைப்பு virtio_blk_config, cylinders);
        vdrive->drive.pchs.head =
            vp_read(&vp->device, கட்டமைப்பு virtio_blk_config, heads);
        vdrive->drive.pchs.sector =
            vp_read(&vp->device, கட்டமைப்பு virtio_blk_config, sectors);
    }

    எனில் (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk %pP\n", pci);
        செல் fail;
    }

    எனில் (!vdrive->vp.use_modern) {
        கட்டமைப்பு virtio_blk_config cfg;
        vp_get_legacy(&vdrive->vp, 0, &cfg, அளவு(cfg));

        u64 f = vp_get_features(&vdrive->vp);
        vdrive->drive.blksize = (f & (1 << VIRTIO_BLK_F_BLK_SIZE)) ?
            cfg.blk_size : DISK_SECTOR_SIZE;

        எனில் (f & (1 << VIRTIO_BLK_F_SIZE_MAX))
            vdrive->drive.max_segment_size = cfg.size_max;

        எனில் (f & (1 << VIRTIO_BLK_F_SEG_MAX))
            vdrive->drive.max_segments = cfg.seg_max;

        vdrive->drive.sectors = cfg.capacity;
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
                vdrive->drive.max_segments);

        எனில் (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            செல் fail;
        }
        vdrive->drive.pchs.cylinder = cfg.cylinders;
        vdrive->drive.pchs.head = cfg.heads;
        vdrive->drive.pchs.sector = cfg.sectors;
    }

    எழுத்து *desc = znprintf(MAXDESCSIZE, "Virtio disk PCI:%pP", pci);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_pci_device(pci));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);

    boot_lchs_find_pci_device(pci, &vdrive->drive.lchs);
    திரும்பு;

fail:
    vp_reset(&vdrive->vp);
    விடுவி(vdrive->vq);
    விடுவி(vdrive);
}

வெறுமை
init_virtio_blk_mmio(வெறுமை *mmio)
{
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk-mmio at %p\n", mmio);
    கட்டமைப்பு virtiodrive_s *vdrive = malloc_கீழ்(அளவு(*vdrive));
    எனில் (!vdrive) {
        warn_noalloc();
        திரும்பு;
    }
    memset(vdrive, 0, அளவு(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = (u32)mmio;

    vp_init_mmio(&vdrive->vp, mmio);

    கட்டமைப்பு vp_device *vp = &vdrive->vp;
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
    எனில் (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
        dprintf(1, "device didn't accept features: %p\n", mmio);
        செல் fail;
    }

    எனில் (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk-mmio %p\n", mmio);
        செல் fail;
    }

    எனில் (features & max_segment_size)
        vdrive->drive.max_segment_size =
            vp_read(&vp->device, கட்டமைப்பு virtio_blk_config, size_max);

    எனில் (features & max_segments)
        vdrive->drive.max_segments =
            vp_read(&vp->device, கட்டமைப்பு virtio_blk_config, seg_max);

    vdrive->drive.sectors =
        vp_read(&vp->device, கட்டமைப்பு virtio_blk_config, capacity);
    எனில் (features & blk_size) {
        vdrive->drive.blksize =
            vp_read(&vp->device, கட்டமைப்பு virtio_blk_config, blk_size);
    } இல்லையெனில் {
        vdrive->drive.blksize = DISK_SECTOR_SIZE;
    }
    எனில் (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
        dprintf(1, "virtio-blk-mmio %p block size %d is unsupported\n",
                mmio, vdrive->drive.blksize);
        செல் fail;
    }
    dprintf(1, "virtio-blk-mmio %p blksize=%d sectors=%u size_max=%u "
            "seg_max=%u.\n", mmio, vdrive->drive.blksize,
            (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
            vdrive->drive.max_segments);

    vdrive->drive.pchs.cylinder =
        vp_read(&vp->device, கட்டமைப்பு virtio_blk_config, cylinders);
    vdrive->drive.pchs.head =
        vp_read(&vp->device, கட்டமைப்பு virtio_blk_config, heads);
    vdrive->drive.pchs.sector =
        vp_read(&vp->device, கட்டமைப்பு virtio_blk_config, sectors);

    எழுத்து *desc = znprintf(MAXDESCSIZE, "Virtio disk mmio:%p", mmio);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_mmio_device(mmio));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);
    திரும்பு;

fail:
    vp_reset(&vdrive->vp);
    விடுவி(vdrive->vq);
    விடுவி(vdrive);
}

வெறுமை
virtio_blk_setup(வெறுமை)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    எனில் (! CONFIG_VIRTIO_BLK)
        திரும்பு;

    dprintf(3, "init virtio-blk\n");

    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        எனில் (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_BLK_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_BLK_10))
            தொடர்;

        எனில் (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-blk at %pP\n",
                    pci);
            தொடர்;
        }

        run_thread(init_virtio_blk, pci);
    }
}

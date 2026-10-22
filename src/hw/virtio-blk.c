// Virtio block boot support.
//
// Copyright (C) 2010 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "biosvar.h" // GET_GLOBALFLAT
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "block.h" // struct drive_s
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "pcidevice.h" // foreachpci
#ଅନ୍ତର୍ଭାଵ "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#ଅନ୍ତର୍ଭାଵ "pci_regs.h" // PCI_VENDOR_ID
#ଅନ୍ତର୍ଭାଵ "stacks.h" // run_thread
#ଅନ୍ତର୍ଭାଵ "std/disk.h" // DISK_RET_SUCCESS
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // usleep, bootprio_find_pci_device, is_bootprio_strict
#ଅନ୍ତର୍ଭାଵ "virtio-pci.h"
#ଅନ୍ତର୍ଭାଵ "virtio-mmio.h"
#ଅନ୍ତର୍ଭାଵ "virtio-ring.h"
#ଅନ୍ତର୍ଭାଵ "virtio-blk.h"

#define min(a, b) ({\
		typeof(a) _a = a;\
		typeof(b) _b = b;\
		_a < _b ? _a : _b; })

ସଂରଚନା virtiodrive_s {
    ସଂରଚନା drive_s drive;
    ସଂରଚନା vring_virtqueue *vq;
    ସଂରଚନା vp_device vp;
};

ଶୂନ୍ୟ
virtio_blk_op_one_segment(ସଂରଚନା virtiodrive_s *vdrive,
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଲିଖ, ସଂରଚନା vring_list sg[])
{
    ସଂରଚନା vring_virtqueue *vq = vdrive->vq;

    /* Add to virtqueue and kick host */
    ଯଦି (ଲିଖ)
        vring_add_buf(vq, sg, 2, 1, 0, 0);
    ଅନ୍ୟଥା
        vring_add_buf(vq, sg, 1, 2, 0, 0);
    vring_kick(&vdrive->vp, vq, 1);

    /* Wait for reply */
    ଯେତେବେଳେ (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, ଖାଲି);

    /**
    ** Clear interrupt status register. Avoid leaving interrupts stuck
    ** if VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
    **/
    vp_get_isr(&vdrive->vp);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
virtio_blk_op(ସଂରଚନା disk_op_s *op, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଲିଖ)
{
    ସଂରଚନା virtiodrive_s *vdrive =
        container_of(op->drive_fl, ସଂରଚନା virtiodrive_s, drive);
    ସଂରଚନା virtio_blk_outhdr hdr = {
        .type = ଲିଖ ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN,
        .ioprio = 0,
        .sector = op->lba,
    };
    u8 status = VIRTIO_BLK_S_UNSUPP;
    ସଂରଚନା vring_list sg[] = {
        {
            .addr       = (ଶୂନ୍ୟ*)(&hdr),
            .length     = ଆକାର(hdr),
        },
        {
            .addr       = op->buf_fl,
            .length     = vdrive->drive.blksize * op->ଗଣନା_2,
        },
        {
            .addr       = (ଶୂନ୍ୟ*)(&status),
            .length     = ଆକାର(status),
        },
    };
    u32 max_io_size =
        vdrive->drive.max_segment_size * vdrive->drive.max_segments;
    u16 blk_ଅଙ୍କ_max;

    ଯଦି (vdrive->drive.blksize != 0 && max_io_size != 0)
        blk_ଅଙ୍କ_max = (u16) min(max_io_size / vdrive->drive.blksize, 0xffff);
    ଅନ୍ୟଥା
        /* default blk_num_max if hardware doesnot advise a proper value */
        blk_ଅଙ୍କ_max = 64;

    ଯଦି (op->ଗଣନା_2 <= blk_ଅଙ୍କ_max) {
        virtio_blk_op_one_segment(vdrive, ଲିଖ, sg);
    } ଅନ୍ୟଥା {
        ଶୂନ୍ୟ *p  = op->buf_fl;
        u16 ଗଣନା_2 = op->ଗଣନା_2;

        ଯେତେବେଳେ (ଗଣନା_2 > 0) {
            u16 blk_ଅଙ୍କ = min(ଗଣନା_2, blk_ଅଙ୍କ_max);
            sg[1].length = vdrive->drive.blksize * blk_ଅଙ୍କ;
            sg[1].addr = p;
            virtio_blk_op_one_segment(vdrive, ଲିଖ, sg);
            ଯଦି (status == VIRTIO_BLK_S_OK) {
                hdr.sector += blk_ଅଙ୍କ;
                p += sg[1].length;
                ଗଣନା_2 -= blk_ଅଙ୍କ;
            } ଅନ୍ୟଥା {
                ଭାଙ୍ଗ;
            }
        }
    }
    ଫେରନ୍ତୁ status == VIRTIO_BLK_S_OK ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
virtio_blk_process_op(ସଂରଚନା disk_op_s *op)
{
    ଯଦି (! CONFIG_VIRTIO_BLK)
        ଫେରନ୍ତୁ 0;
    ବାଛ (op->command) {
    ସ୍ଥିତି CMD_READ:
        ଫେରନ୍ତୁ virtio_blk_op(op, 0);
    ସ୍ଥିତି CMD_WRITE:
        ଫେରନ୍ତୁ virtio_blk_op(op, 1);
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ଫେରନ୍ତୁ default_process_op(op);
    }
}

ସ୍ଥିର ଶୂନ୍ୟ
init_virtio_blk(ଶୂନ୍ୟ *data)
{
    ସଂରଚନା pci_device *pci = data;
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk at %pP\n", pci);
    ସଂରଚନା virtiodrive_s *vdrive = malloc_ତଳ(ଆକାର(*vdrive));
    ଯଦି (!vdrive) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    memset(vdrive, 0, ଆକାର(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = pci->bdf;

    vp_init_simple(&vdrive->vp, pci);

    ଯଦି (vdrive->vp.use_modern) {
        ସଂରଚନା vp_device *vp = &vdrive->vp;
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        u64 blk_size = 1ull << VIRTIO_BLK_F_BLK_SIZE;
        u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
        u64 max_segment_size = 1ull << VIRTIO_BLK_F_SIZE_MAX;

        ଯଦି (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            ଯାଅ fail;
        }

        features = features & (version1 | iommu_platform | blk_size
                        | max_segments | max_segment_size);
        vp_set_features(vp, features);
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        ଯଦି (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            ଯାଅ fail;
        }

        ଯଦି (features & max_segment_size)
            vdrive->drive.max_segment_size =
                vp_read(&vp->device, ସଂରଚନା virtio_blk_config, size_max);

        ଯଦି (features & max_segments)
            vdrive->drive.max_segments =
                vp_read(&vp->device, ସଂରଚନା virtio_blk_config, seg_max);

        vdrive->drive.sectors =
            vp_read(&vp->device, ସଂରଚନା virtio_blk_config, capacity);
        ଯଦି (features & blk_size) {
            vdrive->drive.blksize =
                vp_read(&vp->device, ସଂରଚନା virtio_blk_config, blk_size);
        } ଅନ୍ୟଥା {
            vdrive->drive.blksize = DISK_SECTOR_SIZE;
        }
        ଯଦି (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            ଯାଅ fail;
        }
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
                vdrive->drive.max_segments);

        vdrive->drive.pchs.cylinder =
            vp_read(&vp->device, ସଂରଚନା virtio_blk_config, cylinders);
        vdrive->drive.pchs.head =
            vp_read(&vp->device, ସଂରଚନା virtio_blk_config, heads);
        vdrive->drive.pchs.sector =
            vp_read(&vp->device, ସଂରଚନା virtio_blk_config, sectors);
    }

    ଯଦି (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk %pP\n", pci);
        ଯାଅ fail;
    }

    ଯଦି (!vdrive->vp.use_modern) {
        ସଂରଚନା virtio_blk_config cfg;
        vp_get_legacy(&vdrive->vp, 0, &cfg, ଆକାର(cfg));

        u64 f = vp_get_features(&vdrive->vp);
        vdrive->drive.blksize = (f & (1 << VIRTIO_BLK_F_BLK_SIZE)) ?
            cfg.blk_size : DISK_SECTOR_SIZE;

        ଯଦି (f & (1 << VIRTIO_BLK_F_SIZE_MAX))
            vdrive->drive.max_segment_size = cfg.size_max;

        ଯଦି (f & (1 << VIRTIO_BLK_F_SEG_MAX))
            vdrive->drive.max_segments = cfg.seg_max;

        vdrive->drive.sectors = cfg.capacity;
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
                vdrive->drive.max_segments);

        ଯଦି (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            ଯାଅ fail;
        }
        vdrive->drive.pchs.cylinder = cfg.cylinders;
        vdrive->drive.pchs.head = cfg.heads;
        vdrive->drive.pchs.sector = cfg.sectors;
    }

    ଅକ୍ଷର *desc = znprintf(MAXDESCSIZE, "Virtio disk PCI:%pP", pci);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_pci_device(pci));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);

    boot_lchs_find_pci_device(pci, &vdrive->drive.lchs);
    ଫେରନ୍ତୁ;

fail:
    vp_reset(&vdrive->vp);
    ମୁକ୍ତ(vdrive->vq);
    ମୁକ୍ତ(vdrive);
}

ଶୂନ୍ୟ
init_virtio_blk_mmio(ଶୂନ୍ୟ *mmio)
{
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk-mmio at %p\n", mmio);
    ସଂରଚନା virtiodrive_s *vdrive = malloc_ତଳ(ଆକାର(*vdrive));
    ଯଦି (!vdrive) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    memset(vdrive, 0, ଆକାର(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = (u32)mmio;

    vp_init_mmio(&vdrive->vp, mmio);

    ସଂରଚନା vp_device *vp = &vdrive->vp;
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
    ଯଦି (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
        dprintf(1, "device didn't accept features: %p\n", mmio);
        ଯାଅ fail;
    }

    ଯଦି (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk-mmio %p\n", mmio);
        ଯାଅ fail;
    }

    ଯଦି (features & max_segment_size)
        vdrive->drive.max_segment_size =
            vp_read(&vp->device, ସଂରଚନା virtio_blk_config, size_max);

    ଯଦି (features & max_segments)
        vdrive->drive.max_segments =
            vp_read(&vp->device, ସଂରଚନା virtio_blk_config, seg_max);

    vdrive->drive.sectors =
        vp_read(&vp->device, ସଂରଚନା virtio_blk_config, capacity);
    ଯଦି (features & blk_size) {
        vdrive->drive.blksize =
            vp_read(&vp->device, ସଂରଚନା virtio_blk_config, blk_size);
    } ଅନ୍ୟଥା {
        vdrive->drive.blksize = DISK_SECTOR_SIZE;
    }
    ଯଦି (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
        dprintf(1, "virtio-blk-mmio %p block size %d is unsupported\n",
                mmio, vdrive->drive.blksize);
        ଯାଅ fail;
    }
    dprintf(1, "virtio-blk-mmio %p blksize=%d sectors=%u size_max=%u "
            "seg_max=%u.\n", mmio, vdrive->drive.blksize,
            (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
            vdrive->drive.max_segments);

    vdrive->drive.pchs.cylinder =
        vp_read(&vp->device, ସଂରଚନା virtio_blk_config, cylinders);
    vdrive->drive.pchs.head =
        vp_read(&vp->device, ସଂରଚନା virtio_blk_config, heads);
    vdrive->drive.pchs.sector =
        vp_read(&vp->device, ସଂରଚନା virtio_blk_config, sectors);

    ଅକ୍ଷର *desc = znprintf(MAXDESCSIZE, "Virtio disk mmio:%p", mmio);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_mmio_device(mmio));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);
    ଫେରନ୍ତୁ;

fail:
    vp_reset(&vdrive->vp);
    ମୁକ୍ତ(vdrive->vq);
    ମୁକ୍ତ(vdrive);
}

ଶୂନ୍ୟ
virtio_blk_setup(ଶୂନ୍ୟ)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    ଯଦି (! CONFIG_VIRTIO_BLK)
        ଫେରନ୍ତୁ;

    dprintf(3, "init virtio-blk\n");

    ସଂରଚନା pci_device *pci;
    foreachpci(pci) {
        ଯଦି (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_BLK_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_BLK_10))
            ଜାରି;

        ଯଦି (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-blk at %pP\n",
                    pci);
            ଜାରି;
        }

        run_thread(init_virtio_blk, pci);
    }
}

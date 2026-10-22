// Virtio block boot support.
//
// Copyright (C) 2010 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "biosvar.h" // GET_GLOBALFLAT
#ضمن "config.h" // CONFIG_*
#ضمن "block.h" // struct drive_s
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "pcidevice.h" // foreachpci
#ضمن "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#ضمن "pci_regs.h" // PCI_VENDOR_ID
#ضمن "stacks.h" // run_thread
#ضمن "std/disk.h" // DISK_RET_SUCCESS
#ضمن "string.h" // memset
#ضمن "util.h" // usleep, bootprio_find_pci_device, is_bootprio_strict
#ضمن "virtio-pci.h"
#ضمن "virtio-mmio.h"
#ضمن "virtio-ring.h"
#ضمن "virtio-blk.h"

#تعریف_کړه min(a, b) ({\
		typeof(a) _a = a;\
		typeof(b) _b = b;\
		_a < _b ? _a : _b; })

بنية virtiodrive_s {
    بنية drive_s drive;
    بنية vring_virtqueue *vq;
    بنية vp_device vp;
};

فراغ
virtio_blk_op_one_segment(بنية virtiodrive_s *vdrive,
    صحيح اكتب, بنية vring_list sg[])
{
    بنية vring_virtqueue *vq = vdrive->vq;

    /* Add to virtqueue and kick host */
    إذا (اكتب)
        vring_جمع_کړه_buf(vq, sg, 2, 1, 0, 0);
    وإلا
        vring_جمع_کړه_buf(vq, sg, 1, 2, 0, 0);
    vring_kick(&vdrive->vp, vq, 1);

    /* Wait for reply */
    طالما (!vring_نور_کارول_شوی(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, عدم);

    /**
    ** Clear interrupt status register. Avoid leaving interrupts stuck
    ** if VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
    **/
    vp_get_isr(&vdrive->vp);
}

ساكن صحيح
virtio_blk_op(بنية disk_op_s *op, صحيح اكتب)
{
    بنية virtiodrive_s *vdrive =
        container_د(op->drive_fl, بنية virtiodrive_s, drive);
    بنية virtio_blk_outhdr hdr = {
        .type = اكتب ? VIRTIO_BLK_T_بهر : VIRTIO_BLK_T_دننه,
        .ioprio = 0,
        .sector = op->lba,
    };
    u8 حالت = VIRTIO_BLK_S_UNSUPP;
    بنية vring_list sg[] = {
        {
            .addr       = (فراغ*)(&hdr),
            .اوږدتیا     = حجم(hdr),
        },
        {
            .addr       = op->buf_fl,
            .اوږدتیا     = vdrive->drive.blksize * op->شمېر,
        },
        {
            .addr       = (فراغ*)(&حالت),
            .اوږدتیا     = حجم(حالت),
        },
    };
    u32 max_io_اندازه =
        vdrive->drive.max_segment_اندازه * vdrive->drive.max_segments;
    u16 blk_عدد_max;

    إذا (vdrive->drive.blksize != 0 && max_io_اندازه != 0)
        blk_عدد_max = (u16) min(max_io_اندازه / vdrive->drive.blksize, 0xffff);
    وإلا
        /* default blk_num_max if hardware doesnot advise a proper value */
        blk_عدد_max = 64;

    إذا (op->شمېر <= blk_عدد_max) {
        virtio_blk_op_one_segment(vdrive, اكتب, sg);
    } وإلا {
        فراغ *p  = op->buf_fl;
        u16 شمېر = op->شمېر;

        طالما (شمېر > 0) {
            u16 blk_عدد = min(شمېر, blk_عدد_max);
            sg[1].اوږدتیا = vdrive->drive.blksize * blk_عدد;
            sg[1].addr = p;
            virtio_blk_op_one_segment(vdrive, اكتب, sg);
            إذا (حالت == VIRTIO_BLK_S_OK) {
                hdr.sector += blk_عدد;
                p += sg[1].اوږدتیا;
                شمېر -= blk_عدد;
            } وإلا {
                اكسر;
            }
        }
    }
    أرجع حالت == VIRTIO_BLK_S_OK ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

صحيح
virtio_blk_process_op(بنية disk_op_s *op)
{
    إذا (! CONFIG_VIRTIO_BLK)
        أرجع 0;
    بدل (op->command) {
    حالة CMD_READ:
        أرجع virtio_blk_op(op, 0);
    حالة CMD_WRITE:
        أرجع virtio_blk_op(op, 1);
    افتراضي:
        أرجع default_process_op(op);
    }
}

ساكن فراغ
init_virtio_blk(فراغ *معلومات)
{
    بنية pci_device *pci = معلومات;
    u8 حالت = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk at %pP\n", pci);
    بنية virtiodrive_s *vdrive = malloc_ټیټ(حجم(*vdrive));
    إذا (!vdrive) {
        warn_noalloc();
        أرجع;
    }
    memset(vdrive, 0, حجم(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = pci->bdf;

    vp_init_simple(&vdrive->vp, pci);

    إذا (vdrive->vp.use_modern) {
        بنية vp_device *vp = &vdrive->vp;
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_نسخه_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        u64 blk_اندازه = 1ull << VIRTIO_BLK_F_BLK_اندازه;
        u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
        u64 max_segment_اندازه = 1ull << VIRTIO_BLK_F_اندازه_MAX;

        إذا (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            اذهب fail;
        }

        features = features & (version1 | iommu_platform | blk_اندازه
                        | max_segments | max_segment_اندازه);
        vp_سېټ_features(vp, features);
        حالت |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_سېټ_حالت(vp, حالت);
        إذا (!(vp_get_حالت(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            اذهب fail;
        }

        إذا (features & max_segment_اندازه)
            vdrive->drive.max_segment_اندازه =
                vp_read(&vp->device, بنية virtio_blk_config, اندازه_max);

        إذا (features & max_segments)
            vdrive->drive.max_segments =
                vp_read(&vp->device, بنية virtio_blk_config, seg_max);

        vdrive->drive.sectors =
            vp_read(&vp->device, بنية virtio_blk_config, ظرفیت);
        إذا (features & blk_اندازه) {
            vdrive->drive.blksize =
                vp_read(&vp->device, بنية virtio_blk_config, blk_اندازه);
        } وإلا {
            vdrive->drive.blksize = DISK_SECTOR_اندازه;
        }
        إذا (vdrive->drive.blksize != DISK_SECTOR_اندازه) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            اذهب fail;
        }
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_اندازه,
                vdrive->drive.max_segments);

        vdrive->drive.pchs.cylinder =
            vp_read(&vp->device, بنية virtio_blk_config, cylinders);
        vdrive->drive.pchs.سر =
            vp_read(&vp->device, بنية virtio_blk_config, heads);
        vdrive->drive.pchs.sector =
            vp_read(&vp->device, بنية virtio_blk_config, sectors);
    }

    إذا (vp_پیدا_کړه_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk %pP\n", pci);
        اذهب fail;
    }

    إذا (!vdrive->vp.use_modern) {
        بنية virtio_blk_config cfg;
        vp_get_legacy(&vdrive->vp, 0, &cfg, حجم(cfg));

        u64 f = vp_get_features(&vdrive->vp);
        vdrive->drive.blksize = (f & (1 << VIRTIO_BLK_F_BLK_اندازه)) ?
            cfg.blk_اندازه : DISK_SECTOR_اندازه;

        إذا (f & (1 << VIRTIO_BLK_F_اندازه_MAX))
            vdrive->drive.max_segment_اندازه = cfg.اندازه_max;

        إذا (f & (1 << VIRTIO_BLK_F_SEG_MAX))
            vdrive->drive.max_segments = cfg.seg_max;

        vdrive->drive.sectors = cfg.ظرفیت;
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_اندازه,
                vdrive->drive.max_segments);

        إذا (vdrive->drive.blksize != DISK_SECTOR_اندازه) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            اذهب fail;
        }
        vdrive->drive.pchs.cylinder = cfg.cylinders;
        vdrive->drive.pchs.سر = cfg.heads;
        vdrive->drive.pchs.sector = cfg.sectors;
    }

    حرف *desc = znprintf(MAXDESCSIZE, "Virtio disk PCI:%pP", pci);
    boot_جمع_کړه_hd(&vdrive->drive, desc, bootprio_پیدا_کړه_pci_device(pci));

    حالت |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_سېټ_حالت(&vdrive->vp, حالت);

    boot_lchs_پیدا_کړه_pci_device(pci, &vdrive->drive.lchs);
    أرجع;

fail:
    vp_reset(&vdrive->vp);
    حرر(vdrive->vq);
    حرر(vdrive);
}

فراغ
init_virtio_blk_mmio(فراغ *mmio)
{
    u8 حالت = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk-mmio at %p\n", mmio);
    بنية virtiodrive_s *vdrive = malloc_ټیټ(حجم(*vdrive));
    إذا (!vdrive) {
        warn_noalloc();
        أرجع;
    }
    memset(vdrive, 0, حجم(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = (u32)mmio;

    vp_init_mmio(&vdrive->vp, mmio);

    بنية vp_device *vp = &vdrive->vp;
    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_نسخه_1;
    u64 blk_اندازه = 1ull << VIRTIO_BLK_F_BLK_اندازه;
    u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
    u64 max_segment_اندازه = 1ull << VIRTIO_BLK_F_اندازه_MAX;

    features = features & (version1 | blk_اندازه
            | max_segments | max_segment_اندازه);
    vp_سېټ_features(vp, features);
    حالت |= VIRTIO_CONFIG_S_FEATURES_OK;
    vp_سېټ_حالت(vp, حالت);
    إذا (!(vp_get_حالت(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
        dprintf(1, "device didn't accept features: %p\n", mmio);
        اذهب fail;
    }

    إذا (vp_پیدا_کړه_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk-mmio %p\n", mmio);
        اذهب fail;
    }

    إذا (features & max_segment_اندازه)
        vdrive->drive.max_segment_اندازه =
            vp_read(&vp->device, بنية virtio_blk_config, اندازه_max);

    إذا (features & max_segments)
        vdrive->drive.max_segments =
            vp_read(&vp->device, بنية virtio_blk_config, seg_max);

    vdrive->drive.sectors =
        vp_read(&vp->device, بنية virtio_blk_config, ظرفیت);
    إذا (features & blk_اندازه) {
        vdrive->drive.blksize =
            vp_read(&vp->device, بنية virtio_blk_config, blk_اندازه);
    } وإلا {
        vdrive->drive.blksize = DISK_SECTOR_اندازه;
    }
    إذا (vdrive->drive.blksize != DISK_SECTOR_اندازه) {
        dprintf(1, "virtio-blk-mmio %p block size %d is unsupported\n",
                mmio, vdrive->drive.blksize);
        اذهب fail;
    }
    dprintf(1, "virtio-blk-mmio %p blksize=%d sectors=%u size_max=%u "
            "seg_max=%u.\n", mmio, vdrive->drive.blksize,
            (u32)vdrive->drive.sectors, vdrive->drive.max_segment_اندازه,
            vdrive->drive.max_segments);

    vdrive->drive.pchs.cylinder =
        vp_read(&vp->device, بنية virtio_blk_config, cylinders);
    vdrive->drive.pchs.سر =
        vp_read(&vp->device, بنية virtio_blk_config, heads);
    vdrive->drive.pchs.sector =
        vp_read(&vp->device, بنية virtio_blk_config, sectors);

    حرف *desc = znprintf(MAXDESCSIZE, "Virtio disk mmio:%p", mmio);
    boot_جمع_کړه_hd(&vdrive->drive, desc, bootprio_پیدا_کړه_mmio_device(mmio));

    حالت |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_سېټ_حالت(&vdrive->vp, حالت);
    أرجع;

fail:
    vp_reset(&vdrive->vp);
    حرر(vdrive->vq);
    حرر(vdrive);
}

فراغ
virtio_blk_تیاری(فراغ)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    إذا (! CONFIG_VIRTIO_BLK)
        أرجع;

    dprintf(3, "init virtio-blk\n");

    بنية pci_device *pci;
    foreachpci(pci) {
        إذا (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_BLK_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_BLK_10))
            استمر;

        إذا (skip_nonbootable && bootprio_پیدا_کړه_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-blk at %pP\n",
                    pci);
            استمر;
        }

        پرلپسې_thread(init_virtio_blk, pci);
    }
}

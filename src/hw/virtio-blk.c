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

#تعریف_کرو min(a, b) ({\
		typeof(a) _a = a;\
		typeof(b) _b = b;\
		_a < _b ? _a : _b; })

ساخت virtiodrive_s {
    ساخت drive_s drive;
    ساخت vring_virtqueue *vq;
    ساخت vp_device vp;
};

خالی
virtio_blk_op_one_segment(ساخت virtiodrive_s *vdrive,
    صحیح اكتب, ساخت vring_list sg[])
{
    ساخت vring_virtqueue *vq = vdrive->vq;

    /* Add to virtqueue and kick host */
    اگر (اكتب)
        vring_جوڑو_buf(vq, sg, 2, 1, 0, 0);
    ورنہ
        vring_جوڑو_buf(vq, sg, 1, 2, 0, 0);
    vring_kick(&vdrive->vp, vq, 1);

    /* Wait for reply */
    جب (!vring_مزید_مستعمل(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, تہی);

    /**
    ** Clear interrupt status register. Avoid leaving interrupts stuck
    ** if VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
    **/
    vp_get_isr(&vdrive->vp);
}

ساکن صحیح
virtio_blk_op(ساخت disk_op_s *op, صحیح اكتب)
{
    ساخت virtiodrive_s *vdrive =
        container_کا(op->drive_fl, ساخت virtiodrive_s, drive);
    ساخت virtio_blk_outhdr hdr = {
        .type = اكتب ? VIRTIO_BLK_T_برآمد : VIRTIO_BLK_T_اندر,
        .ioprio = 0,
        .sector = op->lba,
    };
    u8 حالت_2 = VIRTIO_BLK_S_UNSUPP;
    ساخت vring_list sg[] = {
        {
            .addr       = (خالی*)(&hdr),
            .لمبائی     = حجم(hdr),
        },
        {
            .addr       = op->buf_fl,
            .لمبائی     = vdrive->drive.blksize * op->گنتی,
        },
        {
            .addr       = (خالی*)(&حالت_2),
            .لمبائی     = حجم(حالت_2),
        },
    };
    u32 max_io_سائز =
        vdrive->drive.max_segment_سائز * vdrive->drive.max_segments;
    u16 blk_عدد_max;

    اگر (vdrive->drive.blksize != 0 && max_io_سائز != 0)
        blk_عدد_max = (u16) min(max_io_سائز / vdrive->drive.blksize, 0xffff);
    ورنہ
        /* default blk_num_max if hardware doesnot advise a proper value */
        blk_عدد_max = 64;

    اگر (op->گنتی <= blk_عدد_max) {
        virtio_blk_op_one_segment(vdrive, اكتب, sg);
    } ورنہ {
        خالی *p  = op->buf_fl;
        u16 گنتی = op->گنتی;

        جب (گنتی > 0) {
            u16 blk_عدد = min(گنتی, blk_عدد_max);
            sg[1].لمبائی = vdrive->drive.blksize * blk_عدد;
            sg[1].addr = p;
            virtio_blk_op_one_segment(vdrive, اكتب, sg);
            اگر (حالت_2 == VIRTIO_BLK_S_OK) {
                hdr.sector += blk_عدد;
                p += sg[1].لمبائی;
                گنتی -= blk_عدد;
            } ورنہ {
                توڑو;
            }
        }
    }
    واپس حالت_2 == VIRTIO_BLK_S_OK ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

صحیح
virtio_blk_process_op(ساخت disk_op_s *op)
{
    اگر (! CONFIG_VIRTIO_BLK)
        واپس 0;
    انتخاب (op->command) {
    حالت CMD_READ:
        واپس virtio_blk_op(op, 0);
    حالت CMD_WRITE:
        واپس virtio_blk_op(op, 1);
    طےشدہ:
        واپس default_process_op(op);
    }
}

ساکن خالی
init_virtio_blk(خالی *ڈیٹا)
{
    ساخت pci_device *pci = ڈیٹا;
    u8 حالت_2 = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk at %pP\n", pci);
    ساخت virtiodrive_s *vdrive = malloc_کم(حجم(*vdrive));
    اگر (!vdrive) {
        warn_noalloc();
        واپس;
    }
    memset(vdrive, 0, حجم(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = pci->bdf;

    vp_init_simple(&vdrive->vp, pci);

    اگر (vdrive->vp.use_modern) {
        ساخت vp_device *vp = &vdrive->vp;
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_نسخہ_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        u64 blk_سائز = 1ull << VIRTIO_BLK_F_BLK_سائز;
        u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
        u64 max_segment_سائز = 1ull << VIRTIO_BLK_F_سائز_MAX;

        اگر (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            جاؤ fail;
        }

        features = features & (version1 | iommu_platform | blk_سائز
                        | max_segments | max_segment_سائز);
        vp_سیٹ_features(vp, features);
        حالت_2 |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_سیٹ_حالت(vp, حالت_2);
        اگر (!(vp_get_حالت(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            جاؤ fail;
        }

        اگر (features & max_segment_سائز)
            vdrive->drive.max_segment_سائز =
                vp_read(&vp->device, ساخت virtio_blk_config, سائز_max);

        اگر (features & max_segments)
            vdrive->drive.max_segments =
                vp_read(&vp->device, ساخت virtio_blk_config, seg_max);

        vdrive->drive.sectors =
            vp_read(&vp->device, ساخت virtio_blk_config, استعداد);
        اگر (features & blk_سائز) {
            vdrive->drive.blksize =
                vp_read(&vp->device, ساخت virtio_blk_config, blk_سائز);
        } ورنہ {
            vdrive->drive.blksize = DISK_SECTOR_سائز;
        }
        اگر (vdrive->drive.blksize != DISK_SECTOR_سائز) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            جاؤ fail;
        }
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_سائز,
                vdrive->drive.max_segments);

        vdrive->drive.pchs.cylinder =
            vp_read(&vp->device, ساخت virtio_blk_config, cylinders);
        vdrive->drive.pchs.سر =
            vp_read(&vp->device, ساخت virtio_blk_config, heads);
        vdrive->drive.pchs.sector =
            vp_read(&vp->device, ساخت virtio_blk_config, sectors);
    }

    اگر (vp_ڈھونڈو_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk %pP\n", pci);
        جاؤ fail;
    }

    اگر (!vdrive->vp.use_modern) {
        ساخت virtio_blk_config cfg;
        vp_get_legacy(&vdrive->vp, 0, &cfg, حجم(cfg));

        u64 f = vp_get_features(&vdrive->vp);
        vdrive->drive.blksize = (f & (1 << VIRTIO_BLK_F_BLK_سائز)) ?
            cfg.blk_سائز : DISK_SECTOR_سائز;

        اگر (f & (1 << VIRTIO_BLK_F_سائز_MAX))
            vdrive->drive.max_segment_سائز = cfg.سائز_max;

        اگر (f & (1 << VIRTIO_BLK_F_SEG_MAX))
            vdrive->drive.max_segments = cfg.seg_max;

        vdrive->drive.sectors = cfg.استعداد;
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_سائز,
                vdrive->drive.max_segments);

        اگر (vdrive->drive.blksize != DISK_SECTOR_سائز) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            جاؤ fail;
        }
        vdrive->drive.pchs.cylinder = cfg.cylinders;
        vdrive->drive.pchs.سر = cfg.heads;
        vdrive->drive.pchs.sector = cfg.sectors;
    }

    حرف *desc = znprintf(MAXDESCSIZE, "Virtio disk PCI:%pP", pci);
    boot_جوڑو_hd(&vdrive->drive, desc, bootprio_ڈھونڈو_pci_device(pci));

    حالت_2 |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_سیٹ_حالت(&vdrive->vp, حالت_2);

    boot_lchs_ڈھونڈو_pci_device(pci, &vdrive->drive.lchs);
    واپس;

fail:
    vp_reset(&vdrive->vp);
    آزاد(vdrive->vq);
    آزاد(vdrive);
}

خالی
init_virtio_blk_mmio(خالی *mmio)
{
    u8 حالت_2 = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk-mmio at %p\n", mmio);
    ساخت virtiodrive_s *vdrive = malloc_کم(حجم(*vdrive));
    اگر (!vdrive) {
        warn_noalloc();
        واپس;
    }
    memset(vdrive, 0, حجم(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = (u32)mmio;

    vp_init_mmio(&vdrive->vp, mmio);

    ساخت vp_device *vp = &vdrive->vp;
    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_نسخہ_1;
    u64 blk_سائز = 1ull << VIRTIO_BLK_F_BLK_سائز;
    u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
    u64 max_segment_سائز = 1ull << VIRTIO_BLK_F_سائز_MAX;

    features = features & (version1 | blk_سائز
            | max_segments | max_segment_سائز);
    vp_سیٹ_features(vp, features);
    حالت_2 |= VIRTIO_CONFIG_S_FEATURES_OK;
    vp_سیٹ_حالت(vp, حالت_2);
    اگر (!(vp_get_حالت(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
        dprintf(1, "device didn't accept features: %p\n", mmio);
        جاؤ fail;
    }

    اگر (vp_ڈھونڈو_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk-mmio %p\n", mmio);
        جاؤ fail;
    }

    اگر (features & max_segment_سائز)
        vdrive->drive.max_segment_سائز =
            vp_read(&vp->device, ساخت virtio_blk_config, سائز_max);

    اگر (features & max_segments)
        vdrive->drive.max_segments =
            vp_read(&vp->device, ساخت virtio_blk_config, seg_max);

    vdrive->drive.sectors =
        vp_read(&vp->device, ساخت virtio_blk_config, استعداد);
    اگر (features & blk_سائز) {
        vdrive->drive.blksize =
            vp_read(&vp->device, ساخت virtio_blk_config, blk_سائز);
    } ورنہ {
        vdrive->drive.blksize = DISK_SECTOR_سائز;
    }
    اگر (vdrive->drive.blksize != DISK_SECTOR_سائز) {
        dprintf(1, "virtio-blk-mmio %p block size %d is unsupported\n",
                mmio, vdrive->drive.blksize);
        جاؤ fail;
    }
    dprintf(1, "virtio-blk-mmio %p blksize=%d sectors=%u size_max=%u "
            "seg_max=%u.\n", mmio, vdrive->drive.blksize,
            (u32)vdrive->drive.sectors, vdrive->drive.max_segment_سائز,
            vdrive->drive.max_segments);

    vdrive->drive.pchs.cylinder =
        vp_read(&vp->device, ساخت virtio_blk_config, cylinders);
    vdrive->drive.pchs.سر =
        vp_read(&vp->device, ساخت virtio_blk_config, heads);
    vdrive->drive.pchs.sector =
        vp_read(&vp->device, ساخت virtio_blk_config, sectors);

    حرف *desc = znprintf(MAXDESCSIZE, "Virtio disk mmio:%p", mmio);
    boot_جوڑو_hd(&vdrive->drive, desc, bootprio_ڈھونڈو_mmio_device(mmio));

    حالت_2 |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_سیٹ_حالت(&vdrive->vp, حالت_2);
    واپس;

fail:
    vp_reset(&vdrive->vp);
    آزاد(vdrive->vq);
    آزاد(vdrive);
}

خالی
virtio_blk_تیاری(خالی)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    اگر (! CONFIG_VIRTIO_BLK)
        واپس;

    dprintf(3, "init virtio-blk\n");

    ساخت pci_device *pci;
    foreachpci(pci) {
        اگر (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_BLK_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_BLK_10))
            جاری;

        اگر (skip_nonbootable && bootprio_ڈھونڈو_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-blk at %pP\n",
                    pci);
            جاری;
        }

        تسلسل_thread(init_virtio_blk, pci);
    }
}

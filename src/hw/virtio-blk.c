// Virtio block boot support.
//
// Copyright (C) 2010 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "biosvar.h" // GET_GLOBALFLAT
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "block.h" // struct drive_s
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "pcidevice.h" // foreachpci
#അന്തര്ഭാവ "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#അന്തര്ഭാവ "pci_regs.h" // PCI_VENDOR_ID
#അന്തര്ഭാവ "stacks.h" // run_thread
#അന്തര്ഭാവ "std/disk.h" // DISK_RET_SUCCESS
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // usleep, bootprio_find_pci_device, is_bootprio_strict
#അന്തര്ഭാവ "virtio-pci.h"
#അന്തര്ഭാവ "virtio-mmio.h"
#അന്തര്ഭാവ "virtio-ring.h"
#അന്തര്ഭാവ "virtio-blk.h"

#define min(a, b) ({\
		typeof(a) _a = a;\
		typeof(b) _b = b;\
		_a < _b ? _a : _b; })

ഘടന virtiodrive_s {
    ഘടന drive_s drive;
    ഘടന vring_virtqueue *vq;
    ഘടന vp_device vp;
};

ശൂന്യം
virtio_blk_op_one_segment(ഘടന virtiodrive_s *vdrive,
    പൂർണ്ണസംഖ്യ ലിഖ, ഘടന vring_list sg[])
{
    ഘടന vring_virtqueue *vq = vdrive->vq;

    /* Add to virtqueue and kick host */
    എങ്കിൽ (ലിഖ)
        vring_add_buf(vq, sg, 2, 1, 0, 0);
    അല്ലെങ്കിൽ
        vring_add_buf(vq, sg, 1, 2, 0, 0);
    vring_kick(&vdrive->vp, vq, 1);

    /* Wait for reply */
    ആകുമ്പോൾ (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, ശൂന്യ);

    /**
    ** Clear interrupt status register. Avoid leaving interrupts stuck
    ** if VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
    **/
    vp_get_isr(&vdrive->vp);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
virtio_blk_op(ഘടന disk_op_s *op, പൂർണ്ണസംഖ്യ ലിഖ)
{
    ഘടന virtiodrive_s *vdrive =
        container_of(op->drive_fl, ഘടന virtiodrive_s, drive);
    ഘടന virtio_blk_outhdr hdr = {
        .type = ലിഖ ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN,
        .ioprio = 0,
        .sector = op->lba,
    };
    u8 status = VIRTIO_BLK_S_UNSUPP;
    ഘടന vring_list sg[] = {
        {
            .addr       = (ശൂന്യം*)(&hdr),
            .length     = വലുപ്പം(hdr),
        },
        {
            .addr       = op->buf_fl,
            .length     = vdrive->drive.blksize * op->എണ്ണം,
        },
        {
            .addr       = (ശൂന്യം*)(&status),
            .length     = വലുപ്പം(status),
        },
    };
    u32 max_io_size =
        vdrive->drive.max_segment_size * vdrive->drive.max_segments;
    u16 blk_അക്കം_max;

    എങ്കിൽ (vdrive->drive.blksize != 0 && max_io_size != 0)
        blk_അക്കം_max = (u16) min(max_io_size / vdrive->drive.blksize, 0xffff);
    അല്ലെങ്കിൽ
        /* default blk_num_max if hardware doesnot advise a proper value */
        blk_അക്കം_max = 64;

    എങ്കിൽ (op->എണ്ണം <= blk_അക്കം_max) {
        virtio_blk_op_one_segment(vdrive, ലിഖ, sg);
    } അല്ലെങ്കിൽ {
        ശൂന്യം *p  = op->buf_fl;
        u16 എണ്ണം = op->എണ്ണം;

        ആകുമ്പോൾ (എണ്ണം > 0) {
            u16 blk_അക്കം = min(എണ്ണം, blk_അക്കം_max);
            sg[1].length = vdrive->drive.blksize * blk_അക്കം;
            sg[1].addr = p;
            virtio_blk_op_one_segment(vdrive, ലിഖ, sg);
            എങ്കിൽ (status == VIRTIO_BLK_S_OK) {
                hdr.sector += blk_അക്കം;
                p += sg[1].length;
                എണ്ണം -= blk_അക്കം;
            } അല്ലെങ്കിൽ {
                നിർത്തുക;
            }
        }
    }
    തിരികെ status == VIRTIO_BLK_S_OK ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

പൂർണ്ണസംഖ്യ
virtio_blk_process_op(ഘടന disk_op_s *op)
{
    എങ്കിൽ (! CONFIG_VIRTIO_BLK)
        തിരികെ 0;
    തിരഞ്ഞെടുപ്പ് (op->command) {
    അവസ്ഥ CMD_READ:
        തിരികെ virtio_blk_op(op, 0);
    അവസ്ഥ CMD_WRITE:
        തിരികെ virtio_blk_op(op, 1);
    സ്ഥിരസ്ഥിതി:
        തിരികെ default_process_op(op);
    }
}

സ്ഥിതം ശൂന്യം
init_virtio_blk(ശൂന്യം *data)
{
    ഘടന pci_device *pci = data;
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk at %pP\n", pci);
    ഘടന virtiodrive_s *vdrive = malloc_താഴെയുള്ള(വലുപ്പം(*vdrive));
    എങ്കിൽ (!vdrive) {
        warn_noalloc();
        തിരികെ;
    }
    memset(vdrive, 0, വലുപ്പം(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = pci->bdf;

    vp_init_simple(&vdrive->vp, pci);

    എങ്കിൽ (vdrive->vp.use_modern) {
        ഘടന vp_device *vp = &vdrive->vp;
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        u64 blk_size = 1ull << VIRTIO_BLK_F_BLK_SIZE;
        u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
        u64 max_segment_size = 1ull << VIRTIO_BLK_F_SIZE_MAX;

        എങ്കിൽ (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            പോകുക fail;
        }

        features = features & (version1 | iommu_platform | blk_size
                        | max_segments | max_segment_size);
        vp_set_features(vp, features);
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        എങ്കിൽ (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            പോകുക fail;
        }

        എങ്കിൽ (features & max_segment_size)
            vdrive->drive.max_segment_size =
                vp_read(&vp->device, ഘടന virtio_blk_config, size_max);

        എങ്കിൽ (features & max_segments)
            vdrive->drive.max_segments =
                vp_read(&vp->device, ഘടന virtio_blk_config, seg_max);

        vdrive->drive.sectors =
            vp_read(&vp->device, ഘടന virtio_blk_config, capacity);
        എങ്കിൽ (features & blk_size) {
            vdrive->drive.blksize =
                vp_read(&vp->device, ഘടന virtio_blk_config, blk_size);
        } അല്ലെങ്കിൽ {
            vdrive->drive.blksize = DISK_SECTOR_SIZE;
        }
        എങ്കിൽ (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            പോകുക fail;
        }
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
                vdrive->drive.max_segments);

        vdrive->drive.pchs.cylinder =
            vp_read(&vp->device, ഘടന virtio_blk_config, cylinders);
        vdrive->drive.pchs.head =
            vp_read(&vp->device, ഘടന virtio_blk_config, heads);
        vdrive->drive.pchs.sector =
            vp_read(&vp->device, ഘടന virtio_blk_config, sectors);
    }

    എങ്കിൽ (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk %pP\n", pci);
        പോകുക fail;
    }

    എങ്കിൽ (!vdrive->vp.use_modern) {
        ഘടന virtio_blk_config cfg;
        vp_get_legacy(&vdrive->vp, 0, &cfg, വലുപ്പം(cfg));

        u64 f = vp_get_features(&vdrive->vp);
        vdrive->drive.blksize = (f & (1 << VIRTIO_BLK_F_BLK_SIZE)) ?
            cfg.blk_size : DISK_SECTOR_SIZE;

        എങ്കിൽ (f & (1 << VIRTIO_BLK_F_SIZE_MAX))
            vdrive->drive.max_segment_size = cfg.size_max;

        എങ്കിൽ (f & (1 << VIRTIO_BLK_F_SEG_MAX))
            vdrive->drive.max_segments = cfg.seg_max;

        vdrive->drive.sectors = cfg.capacity;
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
                vdrive->drive.max_segments);

        എങ്കിൽ (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            പോകുക fail;
        }
        vdrive->drive.pchs.cylinder = cfg.cylinders;
        vdrive->drive.pchs.head = cfg.heads;
        vdrive->drive.pchs.sector = cfg.sectors;
    }

    അക്ഷരം *desc = znprintf(MAXDESCSIZE, "Virtio disk PCI:%pP", pci);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_pci_device(pci));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);

    boot_lchs_find_pci_device(pci, &vdrive->drive.lchs);
    തിരികെ;

fail:
    vp_reset(&vdrive->vp);
    സ്വതന്ത്രം(vdrive->vq);
    സ്വതന്ത്രം(vdrive);
}

ശൂന്യം
init_virtio_blk_mmio(ശൂന്യം *mmio)
{
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk-mmio at %p\n", mmio);
    ഘടന virtiodrive_s *vdrive = malloc_താഴെയുള്ള(വലുപ്പം(*vdrive));
    എങ്കിൽ (!vdrive) {
        warn_noalloc();
        തിരികെ;
    }
    memset(vdrive, 0, വലുപ്പം(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = (u32)mmio;

    vp_init_mmio(&vdrive->vp, mmio);

    ഘടന vp_device *vp = &vdrive->vp;
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
    എങ്കിൽ (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
        dprintf(1, "device didn't accept features: %p\n", mmio);
        പോകുക fail;
    }

    എങ്കിൽ (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk-mmio %p\n", mmio);
        പോകുക fail;
    }

    എങ്കിൽ (features & max_segment_size)
        vdrive->drive.max_segment_size =
            vp_read(&vp->device, ഘടന virtio_blk_config, size_max);

    എങ്കിൽ (features & max_segments)
        vdrive->drive.max_segments =
            vp_read(&vp->device, ഘടന virtio_blk_config, seg_max);

    vdrive->drive.sectors =
        vp_read(&vp->device, ഘടന virtio_blk_config, capacity);
    എങ്കിൽ (features & blk_size) {
        vdrive->drive.blksize =
            vp_read(&vp->device, ഘടന virtio_blk_config, blk_size);
    } അല്ലെങ്കിൽ {
        vdrive->drive.blksize = DISK_SECTOR_SIZE;
    }
    എങ്കിൽ (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
        dprintf(1, "virtio-blk-mmio %p block size %d is unsupported\n",
                mmio, vdrive->drive.blksize);
        പോകുക fail;
    }
    dprintf(1, "virtio-blk-mmio %p blksize=%d sectors=%u size_max=%u "
            "seg_max=%u.\n", mmio, vdrive->drive.blksize,
            (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
            vdrive->drive.max_segments);

    vdrive->drive.pchs.cylinder =
        vp_read(&vp->device, ഘടന virtio_blk_config, cylinders);
    vdrive->drive.pchs.head =
        vp_read(&vp->device, ഘടന virtio_blk_config, heads);
    vdrive->drive.pchs.sector =
        vp_read(&vp->device, ഘടന virtio_blk_config, sectors);

    അക്ഷരം *desc = znprintf(MAXDESCSIZE, "Virtio disk mmio:%p", mmio);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_mmio_device(mmio));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);
    തിരികെ;

fail:
    vp_reset(&vdrive->vp);
    സ്വതന്ത്രം(vdrive->vq);
    സ്വതന്ത്രം(vdrive);
}

ശൂന്യം
virtio_blk_setup(ശൂന്യം)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    എങ്കിൽ (! CONFIG_VIRTIO_BLK)
        തിരികെ;

    dprintf(3, "init virtio-blk\n");

    ഘടന pci_device *pci;
    foreachpci(pci) {
        എങ്കിൽ (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_BLK_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_BLK_10))
            തുടരുക;

        എങ്കിൽ (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-blk at %pP\n",
                    pci);
            തുടരുക;
        }

        run_thread(init_virtio_blk, pci);
    }
}

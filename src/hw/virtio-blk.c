// Virtio block boot support.
//
// Copyright (C) 2010 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "biosvar.h" // GET_GLOBALFLAT
#અન્તર્ભાવ "config.h" // CONFIG_*
#અન્તર્ભાવ "block.h" // struct drive_s
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "pcidevice.h" // foreachpci
#અન્તર્ભાવ "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#અન્તર્ભાવ "pci_regs.h" // PCI_VENDOR_ID
#અન્તર્ભાવ "stacks.h" // run_thread
#અન્તર્ભાવ "std/disk.h" // DISK_RET_SUCCESS
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // usleep, bootprio_find_pci_device, is_bootprio_strict
#અન્તર્ભાવ "virtio-pci.h"
#અન્તર્ભાવ "virtio-mmio.h"
#અન્તર્ભાવ "virtio-ring.h"
#અન્તર્ભાવ "virtio-blk.h"

#define min(a, b) ({\
		typeof(a) _a = a;\
		typeof(b) _b = b;\
		_a < _b ? _a : _b; })

સંરચના virtiodrive_s {
    સંરચના drive_s drive;
    સંરચના vring_virtqueue *vq;
    સંરચના vp_device vp;
};

શૂન્ય
virtio_blk_op_one_segment(સંરચના virtiodrive_s *vdrive,
    પૂર્ણાંક લિખ, સંરચના vring_list sg[])
{
    સંરચના vring_virtqueue *vq = vdrive->vq;

    /* Add to virtqueue and kick host */
    જો (લિખ)
        vring_add_buf(vq, sg, 2, 1, 0, 0);
    નહીંતર
        vring_add_buf(vq, sg, 1, 2, 0, 0);
    vring_kick(&vdrive->vp, vq, 1);

    /* Wait for reply */
    જ્યારે (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, ખાલી);

    /**
    ** Clear interrupt status register. Avoid leaving interrupts stuck
    ** if VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
    **/
    vp_get_isr(&vdrive->vp);
}

સ્થિર પૂર્ણાંક
virtio_blk_op(સંરચના disk_op_s *op, પૂર્ણાંક લિખ)
{
    સંરચના virtiodrive_s *vdrive =
        container_of(op->drive_fl, સંરચના virtiodrive_s, drive);
    સંરચના virtio_blk_outhdr hdr = {
        .type = લિખ ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN,
        .ioprio = 0,
        .sector = op->lba,
    };
    u8 status = VIRTIO_BLK_S_UNSUPP;
    સંરચના vring_list sg[] = {
        {
            .addr       = (શૂન્ય*)(&hdr),
            .length     = કદ(hdr),
        },
        {
            .addr       = op->buf_fl,
            .length     = vdrive->drive.blksize * op->ગણતરી,
        },
        {
            .addr       = (શૂન્ય*)(&status),
            .length     = કદ(status),
        },
    };
    u32 max_io_size =
        vdrive->drive.max_segment_size * vdrive->drive.max_segments;
    u16 blk_અંક_max;

    જો (vdrive->drive.blksize != 0 && max_io_size != 0)
        blk_અંક_max = (u16) min(max_io_size / vdrive->drive.blksize, 0xffff);
    નહીંતર
        /* default blk_num_max if hardware doesnot advise a proper value */
        blk_અંક_max = 64;

    જો (op->ગણતરી <= blk_અંક_max) {
        virtio_blk_op_one_segment(vdrive, લિખ, sg);
    } નહીંતર {
        શૂન્ય *p  = op->buf_fl;
        u16 ગણતરી = op->ગણતરી;

        જ્યારે (ગણતરી > 0) {
            u16 blk_અંક = min(ગણતરી, blk_અંક_max);
            sg[1].length = vdrive->drive.blksize * blk_અંક;
            sg[1].addr = p;
            virtio_blk_op_one_segment(vdrive, લિખ, sg);
            જો (status == VIRTIO_BLK_S_OK) {
                hdr.sector += blk_અંક;
                p += sg[1].length;
                ગણતરી -= blk_અંક;
            } નહીંતર {
                તોડો;
            }
        }
    }
    પાછા status == VIRTIO_BLK_S_OK ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

પૂર્ણાંક
virtio_blk_process_op(સંરચના disk_op_s *op)
{
    જો (! CONFIG_VIRTIO_BLK)
        પાછા 0;
    પસંદગી (op->command) {
    સ્થિતિ CMD_READ:
        પાછા virtio_blk_op(op, 0);
    સ્થિતિ CMD_WRITE:
        પાછા virtio_blk_op(op, 1);
    મૂળભૂત:
        પાછા default_process_op(op);
    }
}

સ્થિર શૂન્ય
init_virtio_blk(શૂન્ય *data)
{
    સંરચના pci_device *pci = data;
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk at %pP\n", pci);
    સંરચના virtiodrive_s *vdrive = malloc_નીચલું(કદ(*vdrive));
    જો (!vdrive) {
        warn_noalloc();
        પાછા;
    }
    memset(vdrive, 0, કદ(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = pci->bdf;

    vp_init_simple(&vdrive->vp, pci);

    જો (vdrive->vp.use_modern) {
        સંરચના vp_device *vp = &vdrive->vp;
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        u64 blk_size = 1ull << VIRTIO_BLK_F_BLK_SIZE;
        u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
        u64 max_segment_size = 1ull << VIRTIO_BLK_F_SIZE_MAX;

        જો (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            જાઓ fail;
        }

        features = features & (version1 | iommu_platform | blk_size
                        | max_segments | max_segment_size);
        vp_set_features(vp, features);
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        જો (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            જાઓ fail;
        }

        જો (features & max_segment_size)
            vdrive->drive.max_segment_size =
                vp_read(&vp->device, સંરચના virtio_blk_config, size_max);

        જો (features & max_segments)
            vdrive->drive.max_segments =
                vp_read(&vp->device, સંરચના virtio_blk_config, seg_max);

        vdrive->drive.sectors =
            vp_read(&vp->device, સંરચના virtio_blk_config, capacity);
        જો (features & blk_size) {
            vdrive->drive.blksize =
                vp_read(&vp->device, સંરચના virtio_blk_config, blk_size);
        } નહીંતર {
            vdrive->drive.blksize = DISK_SECTOR_SIZE;
        }
        જો (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            જાઓ fail;
        }
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
                vdrive->drive.max_segments);

        vdrive->drive.pchs.cylinder =
            vp_read(&vp->device, સંરચના virtio_blk_config, cylinders);
        vdrive->drive.pchs.head =
            vp_read(&vp->device, સંરચના virtio_blk_config, heads);
        vdrive->drive.pchs.sector =
            vp_read(&vp->device, સંરચના virtio_blk_config, sectors);
    }

    જો (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk %pP\n", pci);
        જાઓ fail;
    }

    જો (!vdrive->vp.use_modern) {
        સંરચના virtio_blk_config cfg;
        vp_get_legacy(&vdrive->vp, 0, &cfg, કદ(cfg));

        u64 f = vp_get_features(&vdrive->vp);
        vdrive->drive.blksize = (f & (1 << VIRTIO_BLK_F_BLK_SIZE)) ?
            cfg.blk_size : DISK_SECTOR_SIZE;

        જો (f & (1 << VIRTIO_BLK_F_SIZE_MAX))
            vdrive->drive.max_segment_size = cfg.size_max;

        જો (f & (1 << VIRTIO_BLK_F_SEG_MAX))
            vdrive->drive.max_segments = cfg.seg_max;

        vdrive->drive.sectors = cfg.capacity;
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
                vdrive->drive.max_segments);

        જો (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            જાઓ fail;
        }
        vdrive->drive.pchs.cylinder = cfg.cylinders;
        vdrive->drive.pchs.head = cfg.heads;
        vdrive->drive.pchs.sector = cfg.sectors;
    }

    અક્ષર *desc = znprintf(MAXDESCSIZE, "Virtio disk PCI:%pP", pci);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_pci_device(pci));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);

    boot_lchs_find_pci_device(pci, &vdrive->drive.lchs);
    પાછા;

fail:
    vp_reset(&vdrive->vp);
    મુક્ત(vdrive->vq);
    મુક્ત(vdrive);
}

શૂન્ય
init_virtio_blk_mmio(શૂન્ય *mmio)
{
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk-mmio at %p\n", mmio);
    સંરચના virtiodrive_s *vdrive = malloc_નીચલું(કદ(*vdrive));
    જો (!vdrive) {
        warn_noalloc();
        પાછા;
    }
    memset(vdrive, 0, કદ(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = (u32)mmio;

    vp_init_mmio(&vdrive->vp, mmio);

    સંરચના vp_device *vp = &vdrive->vp;
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
    જો (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
        dprintf(1, "device didn't accept features: %p\n", mmio);
        જાઓ fail;
    }

    જો (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk-mmio %p\n", mmio);
        જાઓ fail;
    }

    જો (features & max_segment_size)
        vdrive->drive.max_segment_size =
            vp_read(&vp->device, સંરચના virtio_blk_config, size_max);

    જો (features & max_segments)
        vdrive->drive.max_segments =
            vp_read(&vp->device, સંરચના virtio_blk_config, seg_max);

    vdrive->drive.sectors =
        vp_read(&vp->device, સંરચના virtio_blk_config, capacity);
    જો (features & blk_size) {
        vdrive->drive.blksize =
            vp_read(&vp->device, સંરચના virtio_blk_config, blk_size);
    } નહીંતર {
        vdrive->drive.blksize = DISK_SECTOR_SIZE;
    }
    જો (vdrive->drive.blksize != DISK_SECTOR_SIZE) {
        dprintf(1, "virtio-blk-mmio %p block size %d is unsupported\n",
                mmio, vdrive->drive.blksize);
        જાઓ fail;
    }
    dprintf(1, "virtio-blk-mmio %p blksize=%d sectors=%u size_max=%u "
            "seg_max=%u.\n", mmio, vdrive->drive.blksize,
            (u32)vdrive->drive.sectors, vdrive->drive.max_segment_size,
            vdrive->drive.max_segments);

    vdrive->drive.pchs.cylinder =
        vp_read(&vp->device, સંરચના virtio_blk_config, cylinders);
    vdrive->drive.pchs.head =
        vp_read(&vp->device, સંરચના virtio_blk_config, heads);
    vdrive->drive.pchs.sector =
        vp_read(&vp->device, સંરચના virtio_blk_config, sectors);

    અક્ષર *desc = znprintf(MAXDESCSIZE, "Virtio disk mmio:%p", mmio);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_mmio_device(mmio));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);
    પાછા;

fail:
    vp_reset(&vdrive->vp);
    મુક્ત(vdrive->vq);
    મુક્ત(vdrive);
}

શૂન્ય
virtio_blk_setup(શૂન્ય)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    જો (! CONFIG_VIRTIO_BLK)
        પાછા;

    dprintf(3, "init virtio-blk\n");

    સંરચના pci_device *pci;
    foreachpci(pci) {
        જો (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_BLK_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_BLK_10))
            ચાલુ;

        જો (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-blk at %pP\n",
                    pci);
            ચાલુ;
        }

        run_thread(init_virtio_blk, pci);
    }
}

// Virtio block boot support.
//
// Copyright (C) 2010 Red Hat Inc.
//
// Authors:
//  Gleb Natapov <gnatapov@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "biosvar.h" // GET_GLOBALFLAT
#समावेश "config.h" // CONFIG_*
#समावेश "block.h" // struct drive_s
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "pcidevice.h" // foreachpci
#समावेश "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#समावेश "pci_regs.h" // PCI_VENDOR_ID
#समावेश "stacks.h" // run_thread
#समावेश "std/disk.h" // DISK_RET_SUCCESS
#समावेश "string.h" // memset
#समावेश "util.h" // usleep, bootprio_find_pci_device, is_bootprio_strict
#समावेश "virtio-pci.h"
#समावेश "virtio-mmio.h"
#समावेश "virtio-ring.h"
#समावेश "virtio-blk.h"

#घोषणा min(a, b) ({\
		typeof(a) _a = a;\
		typeof(b) _b = b;\
		_a < _b ? _a : _b; })

काष्ठा virtiodrive_s {
    काष्ठा drive_s drive;
    काष्ठा vring_virtqueue *vq;
    काष्ठा vp_device vp;
};

व्योम
virtio_blk_op_one_segment(काष्ठा virtiodrive_s *vdrive,
    पूर्णांक ग_लिखो, काष्ठा vring_list sg[])
{
    काष्ठा vring_virtqueue *vq = vdrive->vq;

    /* Add to virtqueue and kick host */
    अगर (ग_लिखो)
        vring_add_buf(vq, sg, 2, 1, 0, 0);
    अन्यथा
        vring_add_buf(vq, sg, 1, 2, 0, 0);
    vring_kick(&vdrive->vp, vq, 1);

    /* Wait for reply */
    जबतक (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, शून्य);

    /**
    ** Clear interrupt status register. Avoid leaving interrupts stuck
    ** if VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
    **/
    vp_get_isr(&vdrive->vp);
}

अटल पूर्णांक
virtio_blk_op(काष्ठा disk_op_s *op, पूर्णांक ग_लिखो)
{
    काष्ठा virtiodrive_s *vdrive =
        container_of(op->drive_fl, काष्ठा virtiodrive_s, drive);
    काष्ठा virtio_blk_outhdr hdr = {
        .type = ग_लिखो ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN,
        .ioprio = 0,
        .sector = op->lba,
    };
    u8 status = VIRTIO_BLK_S_UNSUPP;
    काष्ठा vring_list sg[] = {
        {
            .addr       = (व्योम*)(&hdr),
            .लंबाई     = माप(hdr),
        },
        {
            .addr       = op->buf_fl,
            .लंबाई     = vdrive->drive.blksize * op->गिनती,
        },
        {
            .addr       = (व्योम*)(&status),
            .लंबाई     = माप(status),
        },
    };
    u32 max_io_आकार =
        vdrive->drive.max_segment_आकार * vdrive->drive.max_segments;
    u16 blk_अंक_max;

    अगर (vdrive->drive.blksize != 0 && max_io_आकार != 0)
        blk_अंक_max = (u16) min(max_io_आकार / vdrive->drive.blksize, 0xffff);
    अन्यथा
        /* default blk_num_max if hardware doesnot advise a proper value */
        blk_अंक_max = 64;

    अगर (op->गिनती <= blk_अंक_max) {
        virtio_blk_op_one_segment(vdrive, ग_लिखो, sg);
    } अन्यथा {
        व्योम *p  = op->buf_fl;
        u16 गिनती = op->गिनती;

        जबतक (गिनती > 0) {
            u16 blk_अंक = min(गिनती, blk_अंक_max);
            sg[1].लंबाई = vdrive->drive.blksize * blk_अंक;
            sg[1].addr = p;
            virtio_blk_op_one_segment(vdrive, ग_लिखो, sg);
            अगर (status == VIRTIO_BLK_S_OK) {
                hdr.sector += blk_अंक;
                p += sg[1].लंबाई;
                गिनती -= blk_अंक;
            } अन्यथा {
                अवरोध;
            }
        }
    }
    वापस status == VIRTIO_BLK_S_OK ? DISK_RET_SUCCESS : DISK_RET_EBADTRACK;
}

पूर्णांक
virtio_blk_process_op(काष्ठा disk_op_s *op)
{
    अगर (! CONFIG_VIRTIO_BLK)
        वापस 0;
    चयन (op->command) {
    हाल CMD_READ:
        वापस virtio_blk_op(op, 0);
    हाल CMD_WRITE:
        वापस virtio_blk_op(op, 1);
    शेष:
        वापस default_process_op(op);
    }
}

अटल व्योम
init_virtio_blk(व्योम *तथ्य)
{
    काष्ठा pci_device *pci = तथ्य;
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk at %pP\n", pci);
    काष्ठा virtiodrive_s *vdrive = malloc_निचला(माप(*vdrive));
    अगर (!vdrive) {
        warn_noalloc();
        वापस;
    }
    स_रखो(vdrive, 0, माप(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = pci->bdf;

    vp_init_simple(&vdrive->vp, pci);

    अगर (vdrive->vp.use_modern) {
        काष्ठा vp_device *vp = &vdrive->vp;
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        u64 blk_आकार = 1ull << VIRTIO_BLK_F_BLK_आकार;
        u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
        u64 max_segment_आकार = 1ull << VIRTIO_BLK_F_आकार_MAX;

        अगर (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            जाओ fail;
        }

        features = features & (version1 | iommu_platform | blk_आकार
                        | max_segments | max_segment_आकार);
        vp_set_features(vp, features);
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        अगर (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            जाओ fail;
        }

        अगर (features & max_segment_आकार)
            vdrive->drive.max_segment_आकार =
                vp_read(&vp->device, काष्ठा virtio_blk_config, आकार_max);

        अगर (features & max_segments)
            vdrive->drive.max_segments =
                vp_read(&vp->device, काष्ठा virtio_blk_config, seg_max);

        vdrive->drive.sectors =
            vp_read(&vp->device, काष्ठा virtio_blk_config, capacity);
        अगर (features & blk_आकार) {
            vdrive->drive.blksize =
                vp_read(&vp->device, काष्ठा virtio_blk_config, blk_आकार);
        } अन्यथा {
            vdrive->drive.blksize = DISK_SECTOR_आकार;
        }
        अगर (vdrive->drive.blksize != DISK_SECTOR_आकार) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            जाओ fail;
        }
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_आकार,
                vdrive->drive.max_segments);

        vdrive->drive.pchs.cylinder =
            vp_read(&vp->device, काष्ठा virtio_blk_config, cylinders);
        vdrive->drive.pchs.शीर्ष =
            vp_read(&vp->device, काष्ठा virtio_blk_config, heads);
        vdrive->drive.pchs.sector =
            vp_read(&vp->device, काष्ठा virtio_blk_config, sectors);
    }

    अगर (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk %pP\n", pci);
        जाओ fail;
    }

    अगर (!vdrive->vp.use_modern) {
        काष्ठा virtio_blk_config cfg;
        vp_get_legacy(&vdrive->vp, 0, &cfg, माप(cfg));

        u64 f = vp_get_features(&vdrive->vp);
        vdrive->drive.blksize = (f & (1 << VIRTIO_BLK_F_BLK_आकार)) ?
            cfg.blk_आकार : DISK_SECTOR_आकार;

        अगर (f & (1 << VIRTIO_BLK_F_आकार_MAX))
            vdrive->drive.max_segment_आकार = cfg.आकार_max;

        अगर (f & (1 << VIRTIO_BLK_F_SEG_MAX))
            vdrive->drive.max_segments = cfg.seg_max;

        vdrive->drive.sectors = cfg.capacity;
        dprintf(3, "virtio-blk %pP blksize=%d sectors=%u size_max=%u "
                "seg_max=%u.\n", pci, vdrive->drive.blksize,
                (u32)vdrive->drive.sectors, vdrive->drive.max_segment_आकार,
                vdrive->drive.max_segments);

        अगर (vdrive->drive.blksize != DISK_SECTOR_आकार) {
            dprintf(1, "virtio-blk %pP block size %d is unsupported\n",
                    pci, vdrive->drive.blksize);
            जाओ fail;
        }
        vdrive->drive.pchs.cylinder = cfg.cylinders;
        vdrive->drive.pchs.शीर्ष = cfg.heads;
        vdrive->drive.pchs.sector = cfg.sectors;
    }

    अक्षर *desc = znprintf(MAXDESCSIZE, "Virtio disk PCI:%pP", pci);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_pci_device(pci));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);

    boot_lchs_find_pci_device(pci, &vdrive->drive.lchs);
    वापस;

fail:
    vp_reset(&vdrive->vp);
    मुक्त(vdrive->vq);
    मुक्त(vdrive);
}

व्योम
init_virtio_blk_mmio(व्योम *mmio)
{
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
    dprintf(1, "found virtio-blk-mmio at %p\n", mmio);
    काष्ठा virtiodrive_s *vdrive = malloc_निचला(माप(*vdrive));
    अगर (!vdrive) {
        warn_noalloc();
        वापस;
    }
    स_रखो(vdrive, 0, माप(*vdrive));
    vdrive->drive.type = DTYPE_VIRTIO_BLK;
    vdrive->drive.cntl_id = (u32)mmio;

    vp_init_mmio(&vdrive->vp, mmio);

    काष्ठा vp_device *vp = &vdrive->vp;
    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_VERSION_1;
    u64 blk_आकार = 1ull << VIRTIO_BLK_F_BLK_आकार;
    u64 max_segments = 1ull << VIRTIO_BLK_F_SEG_MAX;
    u64 max_segment_आकार = 1ull << VIRTIO_BLK_F_आकार_MAX;

    features = features & (version1 | blk_आकार
            | max_segments | max_segment_आकार);
    vp_set_features(vp, features);
    status |= VIRTIO_CONFIG_S_FEATURES_OK;
    vp_set_status(vp, status);
    अगर (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
        dprintf(1, "device didn't accept features: %p\n", mmio);
        जाओ fail;
    }

    अगर (vp_find_vq(&vdrive->vp, 0, &vdrive->vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-blk-mmio %p\n", mmio);
        जाओ fail;
    }

    अगर (features & max_segment_आकार)
        vdrive->drive.max_segment_आकार =
            vp_read(&vp->device, काष्ठा virtio_blk_config, आकार_max);

    अगर (features & max_segments)
        vdrive->drive.max_segments =
            vp_read(&vp->device, काष्ठा virtio_blk_config, seg_max);

    vdrive->drive.sectors =
        vp_read(&vp->device, काष्ठा virtio_blk_config, capacity);
    अगर (features & blk_आकार) {
        vdrive->drive.blksize =
            vp_read(&vp->device, काष्ठा virtio_blk_config, blk_आकार);
    } अन्यथा {
        vdrive->drive.blksize = DISK_SECTOR_आकार;
    }
    अगर (vdrive->drive.blksize != DISK_SECTOR_आकार) {
        dprintf(1, "virtio-blk-mmio %p block size %d is unsupported\n",
                mmio, vdrive->drive.blksize);
        जाओ fail;
    }
    dprintf(1, "virtio-blk-mmio %p blksize=%d sectors=%u size_max=%u "
            "seg_max=%u.\n", mmio, vdrive->drive.blksize,
            (u32)vdrive->drive.sectors, vdrive->drive.max_segment_आकार,
            vdrive->drive.max_segments);

    vdrive->drive.pchs.cylinder =
        vp_read(&vp->device, काष्ठा virtio_blk_config, cylinders);
    vdrive->drive.pchs.शीर्ष =
        vp_read(&vp->device, काष्ठा virtio_blk_config, heads);
    vdrive->drive.pchs.sector =
        vp_read(&vp->device, काष्ठा virtio_blk_config, sectors);

    अक्षर *desc = znprintf(MAXDESCSIZE, "Virtio disk mmio:%p", mmio);
    boot_add_hd(&vdrive->drive, desc, bootprio_find_mmio_device(mmio));

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(&vdrive->vp, status);
    वापस;

fail:
    vp_reset(&vdrive->vp);
    मुक्त(vdrive->vq);
    मुक्त(vdrive);
}

व्योम
virtio_blk_setup(व्योम)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    अगर (! CONFIG_VIRTIO_BLK)
        वापस;

    dprintf(3, "init virtio-blk\n");

    काष्ठा pci_device *pci;
    foreachpci(pci) {
        अगर (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_BLK_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_BLK_10))
            जारी;

        अगर (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-blk at %pP\n",
                    pci);
            जारी;
        }

        run_thread(init_virtio_blk, pci);
    }
}

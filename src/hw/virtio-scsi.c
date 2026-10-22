// Virtio SCSI boot support.
//
// Copyright (C) 2012 Red Hat Inc.
//
// Authors:
//  Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "block.h" // struct drive_s
#அந்தர்பாவ "blockcmd.h" // scsi_drive_setup
#அந்தர்பாவ "config.h" // CONFIG_*
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
#அந்தர்பாவ "virtio-ring.h"
#அந்தர்பாவ "virtio-scsi.h"
#அந்தர்பாவ "virtio-mmio.h"

கட்டமைப்பு virtio_lun_s {
    கட்டமைப்பு drive_s drive;
    கட்டமைப்பு pci_device *pci;
    வெறுமை *mmio;
    எழுத்து name[16];
    கட்டமைப்பு vring_virtqueue *vq;
    கட்டமைப்பு vp_device *vp;
    u16 target;
    u16 lun;
};

முழு
virtio_scsi_process_op(கட்டமைப்பு disk_op_s *op)
{
    எனில் (! CONFIG_VIRTIO_SCSI)
        திரும்பு 0;
    கட்டமைப்பு virtio_lun_s *vlun =
        container_of(op->drive_fl, கட்டமைப்பு virtio_lun_s, drive);
    கட்டமைப்பு vp_device *vp = vlun->vp;
    கட்டமைப்பு vring_virtqueue *vq = vlun->vq;
    கட்டமைப்பு virtio_scsi_req_cmd req;
    கட்டமைப்பு virtio_scsi_resp_cmd resp;
    கட்டமைப்பு vring_list sg[3];

    memset(&req, 0, அளவு(req));
    முழு blocksize = scsi_fill_cmd(op, req.cdb, 16);
    எனில் (blocksize < 0)
        திரும்பு default_process_op(op);
    req.lun[0] = 1;
    req.lun[1] = vlun->target;
    req.lun[2] = (vlun->lun >> 8) | 0x40;
    req.lun[3] = (vlun->lun & 0xff);

    u32 len = op->மொத்தம் * blocksize;
    முழு datain = scsi_is_read(op);
    முழு in_இலக்கம் = (datain ? 2 : 1);
    முழு out_இலக்கம் = (len ? 3 : 2) - in_இலக்கம்;

    sg[0].addr   = (வெறுமை*)(&req);
    sg[0].length = அளவு(req);

    sg[out_இலக்கம்].addr   = (வெறுமை*)(&resp);
    sg[out_இலக்கம்].length = அளவு(resp);

    எனில் (len) {
        முழு data_idx = (datain ? 2 : 1);
        sg[data_idx].addr   = op->buf_fl;
        sg[data_idx].length = len;
    }

    /* Add to virtqueue and kick host */
    vring_add_buf(vq, sg, out_இலக்கம், in_இலக்கம், 0, 0);
    vring_kick(vp, vq, 1);

    /* Wait for reply */
    போது (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, வெற்று);

    /* Clear interrupt status register.  Avoid leaving interrupts stuck if
     * VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
     */
    vp_get_isr(vp);

    எனில் (resp.response == VIRTIO_SCSI_S_OK && resp.status == 0) {
        திரும்பு DISK_RET_SUCCESS;
    }
    திரும்பு DISK_RET_EBADTRACK;
}

நிலையான வெறுமை
virtio_scsi_init_lun(கட்டமைப்பு virtio_lun_s *vlun,
                     கட்டமைப்பு pci_device *pci, வெறுமை *mmio,
                     கட்டமைப்பு vp_device *vp, கட்டமைப்பு vring_virtqueue *vq,
                     u16 target, u16 lun)
{
    memset(vlun, 0, அளவு(*vlun));
    vlun->drive.type = DTYPE_VIRTIO_SCSI;
    vlun->drive.cntl_id = pci->bdf;
    vlun->pci = pci;
    vlun->mmio = mmio;
    vlun->vp = vp;
    vlun->vq = vq;
    vlun->target = target;
    vlun->lun = lun;
    எனில் (vlun->pci)
        snprintf(vlun->name, அளவு(vlun->name), "pci:%pP", vlun->pci);
    எனில் (vlun->mmio)
        snprintf(vlun->name, அளவு(vlun->name), "mmio:%08x", (u32)vlun->mmio);
}

நிலையான முழு
virtio_scsi_add_lun(u32 lun, கட்டமைப்பு drive_s *tmpl_drv)
{
    u8 skip_nonbootable = is_bootprio_strict();
    கட்டமைப்பு virtio_lun_s *tmpl_vlun =
        container_of(tmpl_drv, கட்டமைப்பு virtio_lun_s, drive);
    முழு prio = -1;

    எனில் (tmpl_vlun->pci)
        prio = bootprio_find_scsi_device(tmpl_vlun->pci, tmpl_vlun->target, lun);
    எனில் (tmpl_vlun->mmio)
        prio = bootprio_find_scsi_mmio_device(tmpl_vlun->mmio, tmpl_vlun->target, lun);

    எனில் (skip_nonbootable && prio < 0) {
        dprintf(1, "skipping init of a non-bootable virtio-scsi dev at %s,"
                " target %d, lun %d\n",
                tmpl_vlun->name, tmpl_vlun->target, lun);
        திரும்பு -1;
    }

    கட்டமைப்பு virtio_lun_s *vlun = malloc_கீழ்(அளவு(*vlun));
    எனில் (!vlun) {
        warn_noalloc();
        திரும்பு -1;
    }
    virtio_scsi_init_lun(vlun, tmpl_vlun->pci, tmpl_vlun->mmio,tmpl_vlun->vp,
                         tmpl_vlun->vq, tmpl_vlun->target, lun);

    எனில் (vlun->pci)
        boot_lchs_find_scsi_device(vlun->pci, vlun->target, vlun->lun,
                                   &(vlun->drive.lchs));
    முழு ret = scsi_drive_setup(&vlun->drive, "virtio-scsi", prio);
    எனில் (ret)
        செல் fail;
    திரும்பு 0;

fail:
    விடுவி(vlun);
    திரும்பு -1;
}

நிலையான முழு
virtio_scsi_scan_target(கட்டமைப்பு pci_device *pci, வெறுமை *mmio, கட்டமைப்பு vp_device *vp,
                        கட்டமைப்பு vring_virtqueue *vq, u16 target)
{

    கட்டமைப்பு virtio_lun_s vlun0;

    virtio_scsi_init_lun(&vlun0, pci, mmio, vp, vq, target, 0);

    முழு ret = scsi_rep_luns_scan(&vlun0.drive, virtio_scsi_add_lun);
    திரும்பு ret < 0 ? 0 : ret;
}

நிலையான வெறுமை
init_virtio_scsi(வெறுமை *data)
{
    கட்டமைப்பு pci_device *pci = data;
    dprintf(1, "found virtio-scsi at %pP\n", pci);
    கட்டமைப்பு vring_virtqueue *vq = வெற்று;
    கட்டமைப்பு vp_device *vp = malloc_உயர்வு(அளவு(*vp));
    எனில் (!vp) {
        warn_noalloc();
        திரும்பு;
    }
    vp_init_simple(vp, pci);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    எனில் (vp->use_modern) {
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        எனில் (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            செல் fail;
        }

        vp_set_features(vp, features & (version1 | iommu_platform));
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        எனில் (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            செல் fail;
        }
    }

    எனில் (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi %pP\n", pci);
        செல் fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    முழு i, tot;
    க்கு (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(pci, வெற்று, vp, vq, i);

    எனில் (!tot)
        செல் fail;

    திரும்பு;

fail:
    vp_reset(vp);
    விடுவி(vp);
    விடுவி(vq);
}

வெறுமை
init_virtio_scsi_mmio(வெறுமை *mmio)
{
    dprintf(1, "found virtio-scsi-mmio at %p\n", mmio);
    கட்டமைப்பு vring_virtqueue *vq = வெற்று;
    கட்டமைப்பு vp_device *vp = malloc_உயர்வு(அளவு(*vp));
    எனில் (!vp) {
        warn_noalloc();
        திரும்பு;
    }
    vp_init_mmio(vp, mmio);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_VERSION_1;
    எனில் (features & version1) {
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;

        vp_set_features(vp, features & (version1 | iommu_platform));
        vp_set_status(vp, VIRTIO_CONFIG_S_FEATURES_OK);
        எனில் (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", mmio);
            செல் fail;
        }
    }

    எனில் (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi-mmio %p\n", mmio);
        செல் fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    முழு i, tot;
    க்கு (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(வெற்று, mmio, vp, vq, i);

    எனில் (!tot)
        செல் fail;

    திரும்பு;

fail:
    vp_reset(vp);
    விடுவி(vp);
    விடுவி(vq);
}

வெறுமை
virtio_scsi_setup(வெறுமை)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    எனில் (! CONFIG_VIRTIO_SCSI)
        திரும்பு;

    dprintf(3, "init virtio-scsi\n");

    கட்டமைப்பு pci_device *pci;
    foreachpci(pci) {
        எனில் (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_10))
            தொடர்;

        எனில் (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-scsi at %pP\n",
                    pci);
            தொடர்;
        }

        run_thread(init_virtio_scsi, pci);
    }
}

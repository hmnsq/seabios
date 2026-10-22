// Virtio SCSI boot support.
//
// Copyright (C) 2012 Red Hat Inc.
//
// Authors:
//  Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "block.h" // struct drive_s
#अन्तर्भाव "blockcmd.h" // scsi_drive_setup
#अन्तर्भाव "config.h" // CONFIG_*
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
#अन्तर्भाव "virtio-ring.h"
#अन्तर्भाव "virtio-scsi.h"
#अन्तर्भाव "virtio-mmio.h"

संरचना virtio_lun_s {
    संरचना drive_s drive;
    संरचना pci_device *pci;
    शून्य *mmio;
    वर्ण name[16];
    संरचना vring_virtqueue *vq;
    संरचना vp_device *vp;
    u16 target;
    u16 lun;
};

पूर्णाङ्क
virtio_scsi_process_op(संरचना disk_op_s *op)
{
    यदि (! CONFIG_VIRTIO_SCSI)
        प्रत्यागम 0;
    संरचना virtio_lun_s *vlun =
        container_सम्बन्धि(op->drive_fl, संरचना virtio_lun_s, drive);
    संरचना vp_device *vp = vlun->vp;
    संरचना vring_virtqueue *vq = vlun->vq;
    संरचना virtio_scsi_req_cmd req;
    संरचना virtio_scsi_resp_cmd resp;
    संरचना vring_list sg[3];

    स्मृतिं_पूरय(&req, 0, परिमाण(req));
    पूर्णाङ्क blocksize = scsi_fill_cmd(op, req.cdb, 16);
    यदि (blocksize < 0)
        प्रत्यागम default_process_op(op);
    req.lun[0] = 1;
    req.lun[1] = vlun->target;
    req.lun[2] = (vlun->lun >> 8) | 0x40;
    req.lun[3] = (vlun->lun & 0xff);

    u32 len = op->गणनम् * blocksize;
    पूर्णाङ्क datain = scsi_is_read(op);
    पूर्णाङ्क अन्तर्_अङ्कः = (datain ? 2 : 1);
    पूर्णाङ्क बहिर्गमनम्_अङ्कः = (len ? 3 : 2) - अन्तर्_अङ्कः;

    sg[0].addr   = (शून्य*)(&req);
    sg[0].दीर्घता = परिमाण(req);

    sg[बहिर्गमनम्_अङ्कः].addr   = (शून्य*)(&resp);
    sg[बहिर्गमनम्_अङ्कः].दीर्घता = परिमाण(resp);

    यदि (len) {
        पूर्णाङ्क दत्त_idx = (datain ? 2 : 1);
        sg[दत्त_idx].addr   = op->buf_fl;
        sg[दत्त_idx].दीर्घता = len;
    }

    /* Add to virtqueue and kick host */
    vring_सङ्कलय_buf(vq, sg, बहिर्गमनम्_अङ्कः, अन्तर्_अङ्कः, 0, 0);
    vring_kick(vp, vq, 1);

    /* Wait for reply */
    यावत् (!vring_अधिकम्_प्रयुक्तम्(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, रिक्त);

    /* Clear interrupt status register.  Avoid leaving interrupts stuck if
     * VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
     */
    vp_get_isr(vp);

    यदि (resp.response == VIRTIO_SCSI_S_OK && resp.स्थितिः == 0) {
        प्रत्यागम DISK_RET_SUCCESS;
    }
    प्रत्यागम DISK_RET_EBADTRACK;
}

स्थिर शून्य
virtio_scsi_init_lun(संरचना virtio_lun_s *vlun,
                     संरचना pci_device *pci, शून्य *mmio,
                     संरचना vp_device *vp, संरचना vring_virtqueue *vq,
                     u16 target, u16 lun)
{
    स्मृतिं_पूरय(vlun, 0, परिमाण(*vlun));
    vlun->drive.type = DTYPE_VIRTIO_SCSI;
    vlun->drive.cntl_id = pci->bdf;
    vlun->pci = pci;
    vlun->mmio = mmio;
    vlun->vp = vp;
    vlun->vq = vq;
    vlun->target = target;
    vlun->lun = lun;
    यदि (vlun->pci)
        snprintf(vlun->name, परिमाण(vlun->name), "pci:%pP", vlun->pci);
    यदि (vlun->mmio)
        snprintf(vlun->name, परिमाण(vlun->name), "mmio:%08x", (u32)vlun->mmio);
}

स्थिर पूर्णाङ्क
virtio_scsi_सङ्कलय_lun(u32 lun, संरचना drive_s *tmpl_drv)
{
    u8 skip_nonbootable = is_bootprio_strict();
    संरचना virtio_lun_s *tmpl_vlun =
        container_सम्बन्धि(tmpl_drv, संरचना virtio_lun_s, drive);
    पूर्णाङ्क prio = -1;

    यदि (tmpl_vlun->pci)
        prio = bootprio_find_scsi_device(tmpl_vlun->pci, tmpl_vlun->target, lun);
    यदि (tmpl_vlun->mmio)
        prio = bootprio_find_scsi_mmio_device(tmpl_vlun->mmio, tmpl_vlun->target, lun);

    यदि (skip_nonbootable && prio < 0) {
        dprintf(1, "skipping init of a non-bootable virtio-scsi dev at %s,"
                " target %d, lun %d\n",
                tmpl_vlun->name, tmpl_vlun->target, lun);
        प्रत्यागम -1;
    }

    संरचना virtio_lun_s *vlun = malloc_नीचः(परिमाण(*vlun));
    यदि (!vlun) {
        warn_noalloc();
        प्रत्यागम -1;
    }
    virtio_scsi_init_lun(vlun, tmpl_vlun->pci, tmpl_vlun->mmio,tmpl_vlun->vp,
                         tmpl_vlun->vq, tmpl_vlun->target, lun);

    यदि (vlun->pci)
        boot_lchs_find_scsi_device(vlun->pci, vlun->target, vlun->lun,
                                   &(vlun->drive.lchs));
    पूर्णाङ्क ret = scsi_drive_सज्जता(&vlun->drive, "virtio-scsi", prio);
    यदि (ret)
        गच्छ fail;
    प्रत्यागम 0;

fail:
    मुक्त(vlun);
    प्रत्यागम -1;
}

स्थिर पूर्णाङ्क
virtio_scsi_अवलोकनम्_target(संरचना pci_device *pci, शून्य *mmio, संरचना vp_device *vp,
                        संरचना vring_virtqueue *vq, u16 target)
{

    संरचना virtio_lun_s vlun0;

    virtio_scsi_init_lun(&vlun0, pci, mmio, vp, vq, target, 0);

    पूर्णाङ्क ret = scsi_rep_luns_अवलोकनम्(&vlun0.drive, virtio_scsi_सङ्कलय_lun);
    प्रत्यागम ret < 0 ? 0 : ret;
}

स्थिर शून्य
init_virtio_scsi(शून्य *दत्त)
{
    संरचना pci_device *pci = दत्त;
    dprintf(1, "found virtio-scsi at %pP\n", pci);
    संरचना vring_virtqueue *vq = रिक्त;
    संरचना vp_device *vp = malloc_उच्चः(परिमाण(*vp));
    यदि (!vp) {
        warn_noalloc();
        प्रत्यागम;
    }
    vp_init_simple(vp, pci);
    u8 स्थितिः = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    यदि (vp->use_modern) {
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_संस्करणम्_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        यदि (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            गच्छ fail;
        }

        vp_समुच्चयः_features(vp, features & (version1 | iommu_platform));
        स्थितिः |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_समुच्चयः_स्थितिः(vp, स्थितिः);
        यदि (!(vp_get_स्थितिः(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            गच्छ fail;
        }
    }

    यदि (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi %pP\n", pci);
        गच्छ fail;
    }

    स्थितिः |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_समुच्चयः_स्थितिः(vp, स्थितिः);

    पूर्णाङ्क i, tot;
    प्रति (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_अवलोकनम्_target(pci, रिक्त, vp, vq, i);

    यदि (!tot)
        गच्छ fail;

    प्रत्यागम;

fail:
    vp_reset(vp);
    मुक्त(vp);
    मुक्त(vq);
}

शून्य
init_virtio_scsi_mmio(शून्य *mmio)
{
    dprintf(1, "found virtio-scsi-mmio at %p\n", mmio);
    संरचना vring_virtqueue *vq = रिक्त;
    संरचना vp_device *vp = malloc_उच्चः(परिमाण(*vp));
    यदि (!vp) {
        warn_noalloc();
        प्रत्यागम;
    }
    vp_init_mmio(vp, mmio);
    u8 स्थितिः = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_संस्करणम्_1;
    यदि (features & version1) {
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;

        vp_समुच्चयः_features(vp, features & (version1 | iommu_platform));
        vp_समुच्चयः_स्थितिः(vp, VIRTIO_CONFIG_S_FEATURES_OK);
        यदि (!(vp_get_स्थितिः(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", mmio);
            गच्छ fail;
        }
    }

    यदि (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi-mmio %p\n", mmio);
        गच्छ fail;
    }

    स्थितिः |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_समुच्चयः_स्थितिः(vp, स्थितिः);

    पूर्णाङ्क i, tot;
    प्रति (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_अवलोकनम्_target(रिक्त, mmio, vp, vq, i);

    यदि (!tot)
        गच्छ fail;

    प्रत्यागम;

fail:
    vp_reset(vp);
    मुक्त(vp);
    मुक्त(vq);
}

शून्य
virtio_scsi_सज्जता(शून्य)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    यदि (! CONFIG_VIRTIO_SCSI)
        प्रत्यागम;

    dprintf(3, "init virtio-scsi\n");

    संरचना pci_device *pci;
    foreachpci(pci) {
        यदि (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_10))
            अनुवर्त;

        यदि (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-scsi at %pP\n",
                    pci);
            अनुवर्त;
        }

        सततम्_thread(init_virtio_scsi, pci);
    }
}

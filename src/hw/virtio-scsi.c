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
    अक्षर name[16];
    संरचना vring_virtqueue *vq;
    संरचना vp_device *vp;
    u16 target;
    u16 lun;
};

पूर्णांक
virtio_scsi_process_op(संरचना disk_op_s *op)
{
    जर (! CONFIG_VIRTIO_SCSI)
        परत 0;
    संरचना virtio_lun_s *vlun =
        container_of(op->drive_fl, संरचना virtio_lun_s, drive);
    संरचना vp_device *vp = vlun->vp;
    संरचना vring_virtqueue *vq = vlun->vq;
    संरचना virtio_scsi_req_cmd req;
    संरचना virtio_scsi_resp_cmd resp;
    संरचना vring_list sg[3];

    स_रखो(&req, 0, आकार(req));
    पूर्णांक blocksize = scsi_fill_cmd(op, req.cdb, 16);
    जर (blocksize < 0)
        परत default_process_op(op);
    req.lun[0] = 1;
    req.lun[1] = vlun->target;
    req.lun[2] = (vlun->lun >> 8) | 0x40;
    req.lun[3] = (vlun->lun & 0xff);

    u32 len = op->मोजणी * blocksize;
    पूर्णांक datain = scsi_is_read(op);
    पूर्णांक in_अंक = (datain ? 2 : 1);
    पूर्णांक out_अंक = (len ? 3 : 2) - in_अंक;

    sg[0].addr   = (शून्य*)(&req);
    sg[0].length = आकार(req);

    sg[out_अंक].addr   = (शून्य*)(&resp);
    sg[out_अंक].length = आकार(resp);

    जर (len) {
        पूर्णांक data_idx = (datain ? 2 : 1);
        sg[data_idx].addr   = op->buf_fl;
        sg[data_idx].length = len;
    }

    /* Add to virtqueue and kick host */
    vring_add_buf(vq, sg, out_अंक, in_अंक, 0, 0);
    vring_kick(vp, vq, 1);

    /* Wait for reply */
    पर्यंत (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, रिक्त);

    /* Clear interrupt status register.  Avoid leaving interrupts stuck if
     * VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
     */
    vp_get_isr(vp);

    जर (resp.response == VIRTIO_SCSI_S_OK && resp.status == 0) {
        परत DISK_RET_SUCCESS;
    }
    परत DISK_RET_EBADTRACK;
}

स्थिर शून्य
virtio_scsi_init_lun(संरचना virtio_lun_s *vlun,
                     संरचना pci_device *pci, शून्य *mmio,
                     संरचना vp_device *vp, संरचना vring_virtqueue *vq,
                     u16 target, u16 lun)
{
    स_रखो(vlun, 0, आकार(*vlun));
    vlun->drive.type = DTYPE_VIRTIO_SCSI;
    vlun->drive.cntl_id = pci->bdf;
    vlun->pci = pci;
    vlun->mmio = mmio;
    vlun->vp = vp;
    vlun->vq = vq;
    vlun->target = target;
    vlun->lun = lun;
    जर (vlun->pci)
        snprintf(vlun->name, आकार(vlun->name), "pci:%pP", vlun->pci);
    जर (vlun->mmio)
        snprintf(vlun->name, आकार(vlun->name), "mmio:%08x", (u32)vlun->mmio);
}

स्थिर पूर्णांक
virtio_scsi_add_lun(u32 lun, संरचना drive_s *tmpl_drv)
{
    u8 skip_nonbootable = is_bootprio_strict();
    संरचना virtio_lun_s *tmpl_vlun =
        container_of(tmpl_drv, संरचना virtio_lun_s, drive);
    पूर्णांक prio = -1;

    जर (tmpl_vlun->pci)
        prio = bootprio_find_scsi_device(tmpl_vlun->pci, tmpl_vlun->target, lun);
    जर (tmpl_vlun->mmio)
        prio = bootprio_find_scsi_mmio_device(tmpl_vlun->mmio, tmpl_vlun->target, lun);

    जर (skip_nonbootable && prio < 0) {
        dprintf(1, "skipping init of a non-bootable virtio-scsi dev at %s,"
                " target %d, lun %d\n",
                tmpl_vlun->name, tmpl_vlun->target, lun);
        परत -1;
    }

    संरचना virtio_lun_s *vlun = malloc_खालचा(आकार(*vlun));
    जर (!vlun) {
        warn_noalloc();
        परत -1;
    }
    virtio_scsi_init_lun(vlun, tmpl_vlun->pci, tmpl_vlun->mmio,tmpl_vlun->vp,
                         tmpl_vlun->vq, tmpl_vlun->target, lun);

    जर (vlun->pci)
        boot_lchs_find_scsi_device(vlun->pci, vlun->target, vlun->lun,
                                   &(vlun->drive.lchs));
    पूर्णांक ret = scsi_drive_setup(&vlun->drive, "virtio-scsi", prio);
    जर (ret)
        जा fail;
    परत 0;

fail:
    मुक्त(vlun);
    परत -1;
}

स्थिर पूर्णांक
virtio_scsi_scan_target(संरचना pci_device *pci, शून्य *mmio, संरचना vp_device *vp,
                        संरचना vring_virtqueue *vq, u16 target)
{

    संरचना virtio_lun_s vlun0;

    virtio_scsi_init_lun(&vlun0, pci, mmio, vp, vq, target, 0);

    पूर्णांक ret = scsi_rep_luns_scan(&vlun0.drive, virtio_scsi_add_lun);
    परत ret < 0 ? 0 : ret;
}

स्थिर शून्य
init_virtio_scsi(शून्य *data)
{
    संरचना pci_device *pci = data;
    dprintf(1, "found virtio-scsi at %pP\n", pci);
    संरचना vring_virtqueue *vq = रिक्त;
    संरचना vp_device *vp = malloc_वरचा(आकार(*vp));
    जर (!vp) {
        warn_noalloc();
        परत;
    }
    vp_init_simple(vp, pci);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    जर (vp->use_modern) {
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        जर (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            जा fail;
        }

        vp_set_features(vp, features & (version1 | iommu_platform));
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        जर (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            जा fail;
        }
    }

    जर (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi %pP\n", pci);
        जा fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    पूर्णांक i, tot;
    साठी (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(pci, रिक्त, vp, vq, i);

    जर (!tot)
        जा fail;

    परत;

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
    संरचना vp_device *vp = malloc_वरचा(आकार(*vp));
    जर (!vp) {
        warn_noalloc();
        परत;
    }
    vp_init_mmio(vp, mmio);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_VERSION_1;
    जर (features & version1) {
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;

        vp_set_features(vp, features & (version1 | iommu_platform));
        vp_set_status(vp, VIRTIO_CONFIG_S_FEATURES_OK);
        जर (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", mmio);
            जा fail;
        }
    }

    जर (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi-mmio %p\n", mmio);
        जा fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    पूर्णांक i, tot;
    साठी (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(रिक्त, mmio, vp, vq, i);

    जर (!tot)
        जा fail;

    परत;

fail:
    vp_reset(vp);
    मुक्त(vp);
    मुक्त(vq);
}

शून्य
virtio_scsi_setup(शून्य)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    जर (! CONFIG_VIRTIO_SCSI)
        परत;

    dprintf(3, "init virtio-scsi\n");

    संरचना pci_device *pci;
    foreachpci(pci) {
        जर (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_10))
            सुरूठेवा;

        जर (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-scsi at %pP\n",
                    pci);
            सुरूठेवा;
        }

        run_thread(init_virtio_scsi, pci);
    }
}

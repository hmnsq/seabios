// Virtio SCSI boot support.
//
// Copyright (C) 2012 Red Hat Inc.
//
// Authors:
//  Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "block.h" // struct drive_s
#समावेश "blockcmd.h" // scsi_drive_setup
#समावेश "config.h" // CONFIG_*
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
#समावेश "virtio-ring.h"
#समावेश "virtio-scsi.h"
#समावेश "virtio-mmio.h"

काष्ठा virtio_lun_s {
    काष्ठा drive_s drive;
    काष्ठा pci_device *pci;
    व्योम *mmio;
    अक्षर name[16];
    काष्ठा vring_virtqueue *vq;
    काष्ठा vp_device *vp;
    u16 target;
    u16 lun;
};

पूर्णांक
virtio_scsi_process_op(काष्ठा disk_op_s *op)
{
    अगर (! CONFIG_VIRTIO_SCSI)
        वापस 0;
    काष्ठा virtio_lun_s *vlun =
        container_of(op->drive_fl, काष्ठा virtio_lun_s, drive);
    काष्ठा vp_device *vp = vlun->vp;
    काष्ठा vring_virtqueue *vq = vlun->vq;
    काष्ठा virtio_scsi_req_cmd req;
    काष्ठा virtio_scsi_resp_cmd resp;
    काष्ठा vring_list sg[3];

    स_रखो(&req, 0, माप(req));
    पूर्णांक blocksize = scsi_fill_cmd(op, req.cdb, 16);
    अगर (blocksize < 0)
        वापस default_process_op(op);
    req.lun[0] = 1;
    req.lun[1] = vlun->target;
    req.lun[2] = (vlun->lun >> 8) | 0x40;
    req.lun[3] = (vlun->lun & 0xff);

    u32 len = op->गिनती * blocksize;
    पूर्णांक datain = scsi_is_read(op);
    पूर्णांक in_अंक = (datain ? 2 : 1);
    पूर्णांक out_अंक = (len ? 3 : 2) - in_अंक;

    sg[0].addr   = (व्योम*)(&req);
    sg[0].लंबाई = माप(req);

    sg[out_अंक].addr   = (व्योम*)(&resp);
    sg[out_अंक].लंबाई = माप(resp);

    अगर (len) {
        पूर्णांक तथ्य_idx = (datain ? 2 : 1);
        sg[तथ्य_idx].addr   = op->buf_fl;
        sg[तथ्य_idx].लंबाई = len;
    }

    /* Add to virtqueue and kick host */
    vring_add_buf(vq, sg, out_अंक, in_अंक, 0, 0);
    vring_kick(vp, vq, 1);

    /* Wait for reply */
    जबतक (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, शून्य);

    /* Clear interrupt status register.  Avoid leaving interrupts stuck if
     * VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
     */
    vp_get_isr(vp);

    अगर (resp.response == VIRTIO_SCSI_S_OK && resp.status == 0) {
        वापस DISK_RET_SUCCESS;
    }
    वापस DISK_RET_EBADTRACK;
}

अटल व्योम
virtio_scsi_init_lun(काष्ठा virtio_lun_s *vlun,
                     काष्ठा pci_device *pci, व्योम *mmio,
                     काष्ठा vp_device *vp, काष्ठा vring_virtqueue *vq,
                     u16 target, u16 lun)
{
    स_रखो(vlun, 0, माप(*vlun));
    vlun->drive.type = DTYPE_VIRTIO_SCSI;
    vlun->drive.cntl_id = pci->bdf;
    vlun->pci = pci;
    vlun->mmio = mmio;
    vlun->vp = vp;
    vlun->vq = vq;
    vlun->target = target;
    vlun->lun = lun;
    अगर (vlun->pci)
        snprintf(vlun->name, माप(vlun->name), "pci:%pP", vlun->pci);
    अगर (vlun->mmio)
        snprintf(vlun->name, माप(vlun->name), "mmio:%08x", (u32)vlun->mmio);
}

अटल पूर्णांक
virtio_scsi_add_lun(u32 lun, काष्ठा drive_s *tmpl_drv)
{
    u8 skip_nonbootable = is_bootprio_strict();
    काष्ठा virtio_lun_s *tmpl_vlun =
        container_of(tmpl_drv, काष्ठा virtio_lun_s, drive);
    पूर्णांक prio = -1;

    अगर (tmpl_vlun->pci)
        prio = bootprio_find_scsi_device(tmpl_vlun->pci, tmpl_vlun->target, lun);
    अगर (tmpl_vlun->mmio)
        prio = bootprio_find_scsi_mmio_device(tmpl_vlun->mmio, tmpl_vlun->target, lun);

    अगर (skip_nonbootable && prio < 0) {
        dprintf(1, "skipping init of a non-bootable virtio-scsi dev at %s,"
                " target %d, lun %d\n",
                tmpl_vlun->name, tmpl_vlun->target, lun);
        वापस -1;
    }

    काष्ठा virtio_lun_s *vlun = malloc_निचला(माप(*vlun));
    अगर (!vlun) {
        warn_noalloc();
        वापस -1;
    }
    virtio_scsi_init_lun(vlun, tmpl_vlun->pci, tmpl_vlun->mmio,tmpl_vlun->vp,
                         tmpl_vlun->vq, tmpl_vlun->target, lun);

    अगर (vlun->pci)
        boot_lchs_find_scsi_device(vlun->pci, vlun->target, vlun->lun,
                                   &(vlun->drive.lchs));
    पूर्णांक ret = scsi_drive_setup(&vlun->drive, "virtio-scsi", prio);
    अगर (ret)
        जाओ fail;
    वापस 0;

fail:
    मुक्त(vlun);
    वापस -1;
}

अटल पूर्णांक
virtio_scsi_scan_target(काष्ठा pci_device *pci, व्योम *mmio, काष्ठा vp_device *vp,
                        काष्ठा vring_virtqueue *vq, u16 target)
{

    काष्ठा virtio_lun_s vlun0;

    virtio_scsi_init_lun(&vlun0, pci, mmio, vp, vq, target, 0);

    पूर्णांक ret = scsi_rep_luns_scan(&vlun0.drive, virtio_scsi_add_lun);
    वापस ret < 0 ? 0 : ret;
}

अटल व्योम
init_virtio_scsi(व्योम *तथ्य)
{
    काष्ठा pci_device *pci = तथ्य;
    dprintf(1, "found virtio-scsi at %pP\n", pci);
    काष्ठा vring_virtqueue *vq = शून्य;
    काष्ठा vp_device *vp = malloc_ऊपरी(माप(*vp));
    अगर (!vp) {
        warn_noalloc();
        वापस;
    }
    vp_init_simple(vp, pci);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    अगर (vp->use_modern) {
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        अगर (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            जाओ fail;
        }

        vp_set_features(vp, features & (version1 | iommu_platform));
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        अगर (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            जाओ fail;
        }
    }

    अगर (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi %pP\n", pci);
        जाओ fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    पूर्णांक i, tot;
    क्रम (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(pci, शून्य, vp, vq, i);

    अगर (!tot)
        जाओ fail;

    वापस;

fail:
    vp_reset(vp);
    मुक्त(vp);
    मुक्त(vq);
}

व्योम
init_virtio_scsi_mmio(व्योम *mmio)
{
    dprintf(1, "found virtio-scsi-mmio at %p\n", mmio);
    काष्ठा vring_virtqueue *vq = शून्य;
    काष्ठा vp_device *vp = malloc_ऊपरी(माप(*vp));
    अगर (!vp) {
        warn_noalloc();
        वापस;
    }
    vp_init_mmio(vp, mmio);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_VERSION_1;
    अगर (features & version1) {
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;

        vp_set_features(vp, features & (version1 | iommu_platform));
        vp_set_status(vp, VIRTIO_CONFIG_S_FEATURES_OK);
        अगर (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", mmio);
            जाओ fail;
        }
    }

    अगर (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi-mmio %p\n", mmio);
        जाओ fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    पूर्णांक i, tot;
    क्रम (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(शून्य, mmio, vp, vq, i);

    अगर (!tot)
        जाओ fail;

    वापस;

fail:
    vp_reset(vp);
    मुक्त(vp);
    मुक्त(vq);
}

व्योम
virtio_scsi_setup(व्योम)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    अगर (! CONFIG_VIRTIO_SCSI)
        वापस;

    dprintf(3, "init virtio-scsi\n");

    काष्ठा pci_device *pci;
    foreachpci(pci) {
        अगर (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_10))
            जारी;

        अगर (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-scsi at %pP\n",
                    pci);
            जारी;
        }

        run_thread(init_virtio_scsi, pci);
    }
}

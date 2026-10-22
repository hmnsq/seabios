// Virtio SCSI boot support.
//
// Copyright (C) 2012 Red Hat Inc.
//
// Authors:
//  Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "block.h" // struct drive_s
#ضمن "blockcmd.h" // scsi_drive_setup
#ضمن "config.h" // CONFIG_*
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
#ضمن "virtio-ring.h"
#ضمن "virtio-scsi.h"
#ضمن "virtio-mmio.h"

بنية virtio_lun_s {
    بنية drive_s drive;
    بنية pci_device *pci;
    فراغ *mmio;
    حرف name[16];
    بنية vring_virtqueue *vq;
    بنية vp_device *vp;
    u16 target;
    u16 lun;
};

صحيح
virtio_scsi_process_op(بنية disk_op_s *op)
{
    אם (! CONFIG_VIRTIO_SCSI)
        החזר 0;
    بنية virtio_lun_s *vlun =
        container_of(op->drive_fl, بنية virtio_lun_s, drive);
    بنية vp_device *vp = vlun->vp;
    بنية vring_virtqueue *vq = vlun->vq;
    بنية virtio_scsi_req_cmd req;
    بنية virtio_scsi_resp_cmd resp;
    بنية vring_list sg[3];

    memset(&req, 0, حجم(req));
    صحيح blocksize = scsi_fill_cmd(op, req.cdb, 16);
    אם (blocksize < 0)
        החזר default_process_op(op);
    req.lun[0] = 1;
    req.lun[1] = vlun->target;
    req.lun[2] = (vlun->lun >> 8) | 0x40;
    req.lun[3] = (vlun->lun & 0xff);

    u32 len = op->count * blocksize;
    صحيح datain = scsi_is_read(op);
    صحيح in_num = (datain ? 2 : 1);
    صحيح out_num = (len ? 3 : 2) - in_num;

    sg[0].addr   = (فراغ*)(&req);
    sg[0].length = حجم(req);

    sg[out_num].addr   = (فراغ*)(&resp);
    sg[out_num].length = حجم(resp);

    אם (len) {
        صحيح data_idx = (datain ? 2 : 1);
        sg[data_idx].addr   = op->buf_fl;
        sg[data_idx].length = len;
    }

    /* Add to virtqueue and kick host */
    vring_add_buf(vq, sg, out_num, in_num, 0, 0);
    vring_kick(vp, vq, 1);

    /* Wait for reply */
    כלעוד (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, عدم);

    /* Clear interrupt status register.  Avoid leaving interrupts stuck if
     * VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
     */
    vp_get_isr(vp);

    אם (resp.response == VIRTIO_SCSI_S_OK && resp.status == 0) {
        החזר DISK_RET_SUCCESS;
    }
    החזר DISK_RET_EBADTRACK;
}

ساكن فراغ
virtio_scsi_init_lun(بنية virtio_lun_s *vlun,
                     بنية pci_device *pci, فراغ *mmio,
                     بنية vp_device *vp, بنية vring_virtqueue *vq,
                     u16 target, u16 lun)
{
    memset(vlun, 0, حجم(*vlun));
    vlun->drive.type = DTYPE_VIRTIO_SCSI;
    vlun->drive.cntl_id = pci->bdf;
    vlun->pci = pci;
    vlun->mmio = mmio;
    vlun->vp = vp;
    vlun->vq = vq;
    vlun->target = target;
    vlun->lun = lun;
    אם (vlun->pci)
        snprintf(vlun->name, حجم(vlun->name), "pci:%pP", vlun->pci);
    אם (vlun->mmio)
        snprintf(vlun->name, حجم(vlun->name), "mmio:%08x", (u32)vlun->mmio);
}

ساكن صحيح
virtio_scsi_add_lun(u32 lun, بنية drive_s *tmpl_drv)
{
    u8 skip_nonbootable = is_bootprio_strict();
    بنية virtio_lun_s *tmpl_vlun =
        container_of(tmpl_drv, بنية virtio_lun_s, drive);
    صحيح prio = -1;

    אם (tmpl_vlun->pci)
        prio = bootprio_find_scsi_device(tmpl_vlun->pci, tmpl_vlun->target, lun);
    אם (tmpl_vlun->mmio)
        prio = bootprio_find_scsi_mmio_device(tmpl_vlun->mmio, tmpl_vlun->target, lun);

    אם (skip_nonbootable && prio < 0) {
        dprintf(1, "skipping init of a non-bootable virtio-scsi dev at %s,"
                " target %d, lun %d\n",
                tmpl_vlun->name, tmpl_vlun->target, lun);
        החזר -1;
    }

    بنية virtio_lun_s *vlun = malloc_low(حجم(*vlun));
    אם (!vlun) {
        warn_noalloc();
        החזר -1;
    }
    virtio_scsi_init_lun(vlun, tmpl_vlun->pci, tmpl_vlun->mmio,tmpl_vlun->vp,
                         tmpl_vlun->vq, tmpl_vlun->target, lun);

    אם (vlun->pci)
        boot_lchs_find_scsi_device(vlun->pci, vlun->target, vlun->lun,
                                   &(vlun->drive.lchs));
    صحيح ret = scsi_drive_setup(&vlun->drive, "virtio-scsi", prio);
    אם (ret)
        اذهب fail;
    החזר 0;

fail:
    حرر(vlun);
    החזר -1;
}

ساكن صحيح
virtio_scsi_scan_target(بنية pci_device *pci, فراغ *mmio, بنية vp_device *vp,
                        بنية vring_virtqueue *vq, u16 target)
{

    بنية virtio_lun_s vlun0;

    virtio_scsi_init_lun(&vlun0, pci, mmio, vp, vq, target, 0);

    صحيح ret = scsi_rep_luns_scan(&vlun0.drive, virtio_scsi_add_lun);
    החזר ret < 0 ? 0 : ret;
}

ساكن فراغ
init_virtio_scsi(فراغ *data)
{
    بنية pci_device *pci = data;
    dprintf(1, "found virtio-scsi at %pP\n", pci);
    بنية vring_virtqueue *vq = عدم;
    بنية vp_device *vp = malloc_high(حجم(*vp));
    אם (!vp) {
        warn_noalloc();
        החזר;
    }
    vp_init_simple(vp, pci);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    אם (vp->use_modern) {
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        אם (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            اذهب fail;
        }

        vp_set_features(vp, features & (version1 | iommu_platform));
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        אם (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            اذهب fail;
        }
    }

    אם (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi %pP\n", pci);
        اذهب fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    صحيح i, tot;
    עבור (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(pci, عدم, vp, vq, i);

    אם (!tot)
        اذهب fail;

    החזר;

fail:
    vp_reset(vp);
    حرر(vp);
    حرر(vq);
}

فراغ
init_virtio_scsi_mmio(فراغ *mmio)
{
    dprintf(1, "found virtio-scsi-mmio at %p\n", mmio);
    بنية vring_virtqueue *vq = عدم;
    بنية vp_device *vp = malloc_high(حجم(*vp));
    אם (!vp) {
        warn_noalloc();
        החזר;
    }
    vp_init_mmio(vp, mmio);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_VERSION_1;
    אם (features & version1) {
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;

        vp_set_features(vp, features & (version1 | iommu_platform));
        vp_set_status(vp, VIRTIO_CONFIG_S_FEATURES_OK);
        אם (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", mmio);
            اذهب fail;
        }
    }

    אם (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi-mmio %p\n", mmio);
        اذهب fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    صحيح i, tot;
    עבור (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(عدم, mmio, vp, vq, i);

    אם (!tot)
        اذهب fail;

    החזר;

fail:
    vp_reset(vp);
    حرر(vp);
    حرر(vq);
}

فراغ
virtio_scsi_setup(فراغ)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    אם (! CONFIG_VIRTIO_SCSI)
        החזר;

    dprintf(3, "init virtio-scsi\n");

    بنية pci_device *pci;
    foreachpci(pci) {
        אם (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_10))
            استمر;

        אם (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-scsi at %pP\n",
                    pci);
            استمر;
        }

        run_thread(init_virtio_scsi, pci);
    }
}

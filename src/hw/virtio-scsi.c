// Virtio SCSI boot support.
//
// Copyright (C) 2012 Red Hat Inc.
//
// Authors:
//  Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "block.h" // struct drive_s
#شامل "blockcmd.h" // scsi_drive_setup
#شامل "config.h" // CONFIG_*
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
#شامل "virtio-ring.h"
#شامل "virtio-scsi.h"
#شامل "virtio-mmio.h"

ساخت virtio_lun_s {
    ساخت drive_s drive;
    ساخت pci_device *pci;
    خالی *mmio;
    حرف name[16];
    ساخت vring_virtqueue *vq;
    ساخت vp_device *vp;
    u16 ہدف;
    u16 lun;
};

صحیح
virtio_scsi_process_op(ساخت disk_op_s *op)
{
    اگر (! CONFIG_VIRTIO_SCSI)
        واپس 0;
    ساخت virtio_lun_s *vlun =
        container_کا(op->drive_fl, ساخت virtio_lun_s, drive);
    ساخت vp_device *vp = vlun->vp;
    ساخت vring_virtqueue *vq = vlun->vq;
    ساخت virtio_scsi_req_cmd req;
    ساخت virtio_scsi_resp_cmd resp;
    ساخت vring_list sg[3];

    memset(&req, 0, حجم(req));
    صحیح blocksize = scsi_fill_cmd(op, req.cdb, 16);
    اگر (blocksize < 0)
        واپس default_process_op(op);
    req.lun[0] = 1;
    req.lun[1] = vlun->ہدف;
    req.lun[2] = (vlun->lun >> 8) | 0x40;
    req.lun[3] = (vlun->lun & 0xff);

    u32 len = op->گنتی * blocksize;
    صحیح datain = scsi_is_read(op);
    صحیح اندر_عدد = (datain ? 2 : 1);
    صحیح برآمد_عدد = (len ? 3 : 2) - اندر_عدد;

    sg[0].addr   = (خالی*)(&req);
    sg[0].لمبائی = حجم(req);

    sg[برآمد_عدد].addr   = (خالی*)(&resp);
    sg[برآمد_عدد].لمبائی = حجم(resp);

    اگر (len) {
        صحیح ڈیٹا_idx = (datain ? 2 : 1);
        sg[ڈیٹا_idx].addr   = op->buf_fl;
        sg[ڈیٹا_idx].لمبائی = len;
    }

    /* Add to virtqueue and kick host */
    vring_جوڑو_buf(vq, sg, برآمد_عدد, اندر_عدد, 0, 0);
    vring_kick(vp, vq, 1);

    /* Wait for reply */
    جب (!vring_مزید_مستعمل(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, تہی);

    /* Clear interrupt status register.  Avoid leaving interrupts stuck if
     * VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
     */
    vp_get_isr(vp);

    اگر (resp.response == VIRTIO_SCSI_S_OK && resp.حالت_2 == 0) {
        واپس DISK_RET_SUCCESS;
    }
    واپس DISK_RET_EBADTRACK;
}

ساکن خالی
virtio_scsi_init_lun(ساخت virtio_lun_s *vlun,
                     ساخت pci_device *pci, خالی *mmio,
                     ساخت vp_device *vp, ساخت vring_virtqueue *vq,
                     u16 ہدف, u16 lun)
{
    memset(vlun, 0, حجم(*vlun));
    vlun->drive.type = DTYPE_VIRTIO_SCSI;
    vlun->drive.cntl_id = pci->bdf;
    vlun->pci = pci;
    vlun->mmio = mmio;
    vlun->vp = vp;
    vlun->vq = vq;
    vlun->ہدف = ہدف;
    vlun->lun = lun;
    اگر (vlun->pci)
        snprintf(vlun->name, حجم(vlun->name), "pci:%pP", vlun->pci);
    اگر (vlun->mmio)
        snprintf(vlun->name, حجم(vlun->name), "mmio:%08x", (u32)vlun->mmio);
}

ساکن صحیح
virtio_scsi_جوڑو_lun(u32 lun, ساخت drive_s *tmpl_drv)
{
    u8 skip_nonbootable = is_bootprio_strict();
    ساخت virtio_lun_s *tmpl_vlun =
        container_کا(tmpl_drv, ساخت virtio_lun_s, drive);
    صحیح prio = -1;

    اگر (tmpl_vlun->pci)
        prio = bootprio_ڈھونڈو_scsi_device(tmpl_vlun->pci, tmpl_vlun->ہدف, lun);
    اگر (tmpl_vlun->mmio)
        prio = bootprio_ڈھونڈو_scsi_mmio_device(tmpl_vlun->mmio, tmpl_vlun->ہدف, lun);

    اگر (skip_nonbootable && prio < 0) {
        dprintf(1, "skipping init of a non-bootable virtio-scsi dev at %s,"
                " target %d, lun %d\n",
                tmpl_vlun->name, tmpl_vlun->ہدف, lun);
        واپس -1;
    }

    ساخت virtio_lun_s *vlun = malloc_کم(حجم(*vlun));
    اگر (!vlun) {
        warn_noalloc();
        واپس -1;
    }
    virtio_scsi_init_lun(vlun, tmpl_vlun->pci, tmpl_vlun->mmio,tmpl_vlun->vp,
                         tmpl_vlun->vq, tmpl_vlun->ہدف, lun);

    اگر (vlun->pci)
        boot_lchs_ڈھونڈو_scsi_device(vlun->pci, vlun->ہدف, vlun->lun,
                                   &(vlun->drive.lchs));
    صحیح ret = scsi_drive_تیاری(&vlun->drive, "virtio-scsi", prio);
    اگر (ret)
        جاؤ fail;
    واپس 0;

fail:
    آزاد(vlun);
    واپس -1;
}

ساکن صحیح
virtio_scsi_جائزہ_ہدف(ساخت pci_device *pci, خالی *mmio, ساخت vp_device *vp,
                        ساخت vring_virtqueue *vq, u16 ہدف)
{

    ساخت virtio_lun_s vlun0;

    virtio_scsi_init_lun(&vlun0, pci, mmio, vp, vq, ہدف, 0);

    صحیح ret = scsi_rep_luns_جائزہ(&vlun0.drive, virtio_scsi_جوڑو_lun);
    واپس ret < 0 ? 0 : ret;
}

ساکن خالی
init_virtio_scsi(خالی *ڈیٹا)
{
    ساخت pci_device *pci = ڈیٹا;
    dprintf(1, "found virtio-scsi at %pP\n", pci);
    ساخت vring_virtqueue *vq = تہی;
    ساخت vp_device *vp = malloc_بلند(حجم(*vp));
    اگر (!vp) {
        warn_noalloc();
        واپس;
    }
    vp_init_simple(vp, pci);
    u8 حالت_2 = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    اگر (vp->use_modern) {
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_نسخہ_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        اگر (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            جاؤ fail;
        }

        vp_سیٹ_features(vp, features & (version1 | iommu_platform));
        حالت_2 |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_سیٹ_حالت(vp, حالت_2);
        اگر (!(vp_get_حالت(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            جاؤ fail;
        }
    }

    اگر (vp_ڈھونڈو_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi %pP\n", pci);
        جاؤ fail;
    }

    حالت_2 |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_سیٹ_حالت(vp, حالت_2);

    صحیح i, tot;
    برائے (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_جائزہ_ہدف(pci, تہی, vp, vq, i);

    اگر (!tot)
        جاؤ fail;

    واپس;

fail:
    vp_reset(vp);
    آزاد(vp);
    آزاد(vq);
}

خالی
init_virtio_scsi_mmio(خالی *mmio)
{
    dprintf(1, "found virtio-scsi-mmio at %p\n", mmio);
    ساخت vring_virtqueue *vq = تہی;
    ساخت vp_device *vp = malloc_بلند(حجم(*vp));
    اگر (!vp) {
        warn_noalloc();
        واپس;
    }
    vp_init_mmio(vp, mmio);
    u8 حالت_2 = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_نسخہ_1;
    اگر (features & version1) {
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;

        vp_سیٹ_features(vp, features & (version1 | iommu_platform));
        vp_سیٹ_حالت(vp, VIRTIO_CONFIG_S_FEATURES_OK);
        اگر (!(vp_get_حالت(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", mmio);
            جاؤ fail;
        }
    }

    اگر (vp_ڈھونڈو_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi-mmio %p\n", mmio);
        جاؤ fail;
    }

    حالت_2 |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_سیٹ_حالت(vp, حالت_2);

    صحیح i, tot;
    برائے (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_جائزہ_ہدف(تہی, mmio, vp, vq, i);

    اگر (!tot)
        جاؤ fail;

    واپس;

fail:
    vp_reset(vp);
    آزاد(vp);
    آزاد(vq);
}

خالی
virtio_scsi_تیاری(خالی)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    اگر (! CONFIG_VIRTIO_SCSI)
        واپس;

    dprintf(3, "init virtio-scsi\n");

    ساخت pci_device *pci;
    foreachpci(pci) {
        اگر (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_10))
            جاری;

        اگر (skip_nonbootable && bootprio_ڈھونڈو_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-scsi at %pP\n",
                    pci);
            جاری;
        }

        تسلسل_thread(init_virtio_scsi, pci);
    }
}

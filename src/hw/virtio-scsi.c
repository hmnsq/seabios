// Virtio SCSI boot support.
//
// Copyright (C) 2012 Red Hat Inc.
//
// Authors:
//  Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "block.h" // struct drive_s
#అన్తర్భావ "blockcmd.h" // scsi_drive_setup
#అన్తర్భావ "config.h" // CONFIG_*
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pcidevice.h" // foreachpci
#అన్తర్భావ "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#అన్తర్భావ "pci_regs.h" // PCI_VENDOR_ID
#అన్తర్భావ "stacks.h" // run_thread
#అన్తర్భావ "std/disk.h" // DISK_RET_SUCCESS
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // usleep, bootprio_find_pci_device, is_bootprio_strict
#అన్తర్భావ "virtio-pci.h"
#అన్తర్భావ "virtio-ring.h"
#అన్తర్భావ "virtio-scsi.h"
#అన్తర్భావ "virtio-mmio.h"

నిర్మాణం virtio_lun_s {
    నిర్మాణం drive_s drive;
    నిర్మాణం pci_device *pci;
    శూన్య *mmio;
    అక్షరం name[16];
    నిర్మాణం vring_virtqueue *vq;
    నిర్మాణం vp_device *vp;
    u16 లక్ష్యం;
    u16 lun;
};

పూర్ణాంకం
virtio_scsi_process_op(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (! CONFIG_VIRTIO_SCSI)
        తిరిగి 0;
    నిర్మాణం virtio_lun_s *vlun =
        container_యొక్క(op->drive_fl, నిర్మాణం virtio_lun_s, drive);
    నిర్మాణం vp_device *vp = vlun->vp;
    నిర్మాణం vring_virtqueue *vq = vlun->vq;
    నిర్మాణం virtio_scsi_req_cmd req;
    నిర్మాణం virtio_scsi_resp_cmd resp;
    నిర్మాణం vring_list sg[3];

    memset(&req, 0, పరిమాణం(req));
    పూర్ణాంకం blocksize = scsi_fill_cmd(op, req.cdb, 16);
    ఒకవేళ (blocksize < 0)
        తిరిగి default_process_op(op);
    req.lun[0] = 1;
    req.lun[1] = vlun->లక్ష్యం;
    req.lun[2] = (vlun->lun >> 8) | 0x40;
    req.lun[3] = (vlun->lun & 0xff);

    u32 len = op->లెక్క * blocksize;
    పూర్ణాంకం datain = scsi_is_read(op);
    పూర్ణాంకం లోపల_అంకె = (datain ? 2 : 1);
    పూర్ణాంకం బహిర్గతం_అంకె = (len ? 3 : 2) - లోపల_అంకె;

    sg[0].addr   = (శూన్య*)(&req);
    sg[0].నిడివి = పరిమాణం(req);

    sg[బహిర్గతం_అంకె].addr   = (శూన్య*)(&resp);
    sg[బహిర్గతం_అంకె].నిడివి = పరిమాణం(resp);

    ఒకవేళ (len) {
        పూర్ణాంకం దత్తాంశం_idx = (datain ? 2 : 1);
        sg[దత్తాంశం_idx].addr   = op->buf_fl;
        sg[దత్తాంశం_idx].నిడివి = len;
    }

    /* Add to virtqueue and kick host */
    vring_కూడు_buf(vq, sg, బహిర్గతం_అంకె, లోపల_అంకె, 0, 0);
    vring_kick(vp, vq, 1);

    /* Wait for reply */
    అయితే (!vring_మరిన్ని_వాడిన(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, ఖాళీ);

    /* Clear interrupt status register.  Avoid leaving interrupts stuck if
     * VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
     */
    vp_get_isr(vp);

    ఒకవేళ (resp.response == VIRTIO_SCSI_S_OK && resp.స్థితి_2 == 0) {
        తిరిగి DISK_RET_SUCCESS;
    }
    తిరిగి DISK_RET_EBADTRACK;
}

స్థిర శూన్య
virtio_scsi_init_lun(నిర్మాణం virtio_lun_s *vlun,
                     నిర్మాణం pci_device *pci, శూన్య *mmio,
                     నిర్మాణం vp_device *vp, నిర్మాణం vring_virtqueue *vq,
                     u16 లక్ష్యం, u16 lun)
{
    memset(vlun, 0, పరిమాణం(*vlun));
    vlun->drive.type = DTYPE_VIRTIO_SCSI;
    vlun->drive.cntl_id = pci->bdf;
    vlun->pci = pci;
    vlun->mmio = mmio;
    vlun->vp = vp;
    vlun->vq = vq;
    vlun->లక్ష్యం = లక్ష్యం;
    vlun->lun = lun;
    ఒకవేళ (vlun->pci)
        snprintf(vlun->name, పరిమాణం(vlun->name), "pci:%pP", vlun->pci);
    ఒకవేళ (vlun->mmio)
        snprintf(vlun->name, పరిమాణం(vlun->name), "mmio:%08x", (u32)vlun->mmio);
}

స్థిర పూర్ణాంకం
virtio_scsi_కూడు_lun(u32 lun, నిర్మాణం drive_s *tmpl_drv)
{
    u8 skip_nonbootable = is_bootprio_strict();
    నిర్మాణం virtio_lun_s *tmpl_vlun =
        container_యొక్క(tmpl_drv, నిర్మాణం virtio_lun_s, drive);
    పూర్ణాంకం prio = -1;

    ఒకవేళ (tmpl_vlun->pci)
        prio = bootprio_కనుగొను_scsi_device(tmpl_vlun->pci, tmpl_vlun->లక్ష్యం, lun);
    ఒకవేళ (tmpl_vlun->mmio)
        prio = bootprio_కనుగొను_scsi_mmio_device(tmpl_vlun->mmio, tmpl_vlun->లక్ష్యం, lun);

    ఒకవేళ (skip_nonbootable && prio < 0) {
        dprintf(1, "skipping init of a non-bootable virtio-scsi dev at %s,"
                " target %d, lun %d\n",
                tmpl_vlun->name, tmpl_vlun->లక్ష్యం, lun);
        తిరిగి -1;
    }

    నిర్మాణం virtio_lun_s *vlun = malloc_దిగువ(పరిమాణం(*vlun));
    ఒకవేళ (!vlun) {
        warn_noalloc();
        తిరిగి -1;
    }
    virtio_scsi_init_lun(vlun, tmpl_vlun->pci, tmpl_vlun->mmio,tmpl_vlun->vp,
                         tmpl_vlun->vq, tmpl_vlun->లక్ష్యం, lun);

    ఒకవేళ (vlun->pci)
        boot_lchs_కనుగొను_scsi_device(vlun->pci, vlun->లక్ష్యం, vlun->lun,
                                   &(vlun->drive.lchs));
    పూర్ణాంకం ret = scsi_drive_అమరిక(&vlun->drive, "virtio-scsi", prio);
    ఒకవేళ (ret)
        వెళ్ళు fail;
    తిరిగి 0;

fail:
    విడుదల(vlun);
    తిరిగి -1;
}

స్థిర పూర్ణాంకం
virtio_scsi_పరిశీలన_లక్ష్యం(నిర్మాణం pci_device *pci, శూన్య *mmio, నిర్మాణం vp_device *vp,
                        నిర్మాణం vring_virtqueue *vq, u16 లక్ష్యం)
{

    నిర్మాణం virtio_lun_s vlun0;

    virtio_scsi_init_lun(&vlun0, pci, mmio, vp, vq, లక్ష్యం, 0);

    పూర్ణాంకం ret = scsi_rep_luns_పరిశీలన(&vlun0.drive, virtio_scsi_కూడు_lun);
    తిరిగి ret < 0 ? 0 : ret;
}

స్థిర శూన్య
init_virtio_scsi(శూన్య *దత్తాంశం)
{
    నిర్మాణం pci_device *pci = దత్తాంశం;
    dprintf(1, "found virtio-scsi at %pP\n", pci);
    నిర్మాణం vring_virtqueue *vq = ఖాళీ;
    నిర్మాణం vp_device *vp = malloc_ఎగువ(పరిమాణం(*vp));
    ఒకవేళ (!vp) {
        warn_noalloc();
        తిరిగి;
    }
    vp_init_simple(vp, pci);
    u8 స్థితి_2 = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    ఒకవేళ (vp->use_modern) {
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_సంస్కరణ_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        ఒకవేళ (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            వెళ్ళు fail;
        }

        vp_సమితి_features(vp, features & (version1 | iommu_platform));
        స్థితి_2 |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_సమితి_స్థితి(vp, స్థితి_2);
        ఒకవేళ (!(vp_get_స్థితి(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            వెళ్ళు fail;
        }
    }

    ఒకవేళ (vp_కనుగొను_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi %pP\n", pci);
        వెళ్ళు fail;
    }

    స్థితి_2 |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_సమితి_స్థితి(vp, స్థితి_2);

    పూర్ణాంకం i, tot;
    కొరకు (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_పరిశీలన_లక్ష్యం(pci, ఖాళీ, vp, vq, i);

    ఒకవేళ (!tot)
        వెళ్ళు fail;

    తిరిగి;

fail:
    vp_reset(vp);
    విడుదల(vp);
    విడుదల(vq);
}

శూన్య
init_virtio_scsi_mmio(శూన్య *mmio)
{
    dprintf(1, "found virtio-scsi-mmio at %p\n", mmio);
    నిర్మాణం vring_virtqueue *vq = ఖాళీ;
    నిర్మాణం vp_device *vp = malloc_ఎగువ(పరిమాణం(*vp));
    ఒకవేళ (!vp) {
        warn_noalloc();
        తిరిగి;
    }
    vp_init_mmio(vp, mmio);
    u8 స్థితి_2 = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_సంస్కరణ_1;
    ఒకవేళ (features & version1) {
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;

        vp_సమితి_features(vp, features & (version1 | iommu_platform));
        vp_సమితి_స్థితి(vp, VIRTIO_CONFIG_S_FEATURES_OK);
        ఒకవేళ (!(vp_get_స్థితి(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", mmio);
            వెళ్ళు fail;
        }
    }

    ఒకవేళ (vp_కనుగొను_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi-mmio %p\n", mmio);
        వెళ్ళు fail;
    }

    స్థితి_2 |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_సమితి_స్థితి(vp, స్థితి_2);

    పూర్ణాంకం i, tot;
    కొరకు (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_పరిశీలన_లక్ష్యం(ఖాళీ, mmio, vp, vq, i);

    ఒకవేళ (!tot)
        వెళ్ళు fail;

    తిరిగి;

fail:
    vp_reset(vp);
    విడుదల(vp);
    విడుదల(vq);
}

శూన్య
virtio_scsi_అమరిక(శూన్య)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    ఒకవేళ (! CONFIG_VIRTIO_SCSI)
        తిరిగి;

    dprintf(3, "init virtio-scsi\n");

    నిర్మాణం pci_device *pci;
    foreachpci(pci) {
        ఒకవేళ (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_10))
            కొనసాగు;

        ఒకవేళ (skip_nonbootable && bootprio_కనుగొను_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-scsi at %pP\n",
                    pci);
            కొనసాగు;
        }

        నిరంతరం_thread(init_virtio_scsi, pci);
    }
}

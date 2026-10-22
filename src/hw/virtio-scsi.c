// Virtio SCSI boot support.
//
// Copyright (C) 2012 Red Hat Inc.
//
// Authors:
//  Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "block.h" // struct drive_s
#ଅନ୍ତର୍ଭାଵ "blockcmd.h" // scsi_drive_setup
#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_*
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "pcidevice.h" // foreachpci
#ଅନ୍ତର୍ଭାଵ "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#ଅନ୍ତର୍ଭାଵ "pci_regs.h" // PCI_VENDOR_ID
#ଅନ୍ତର୍ଭାଵ "stacks.h" // run_thread
#ଅନ୍ତର୍ଭାଵ "std/disk.h" // DISK_RET_SUCCESS
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // usleep, bootprio_find_pci_device, is_bootprio_strict
#ଅନ୍ତର୍ଭାଵ "virtio-pci.h"
#ଅନ୍ତର୍ଭାଵ "virtio-ring.h"
#ଅନ୍ତର୍ଭାଵ "virtio-scsi.h"
#ଅନ୍ତର୍ଭାଵ "virtio-mmio.h"

ସଂରଚନା virtio_lun_s {
    ସଂରଚନା drive_s drive;
    ସଂରଚନା pci_device *pci;
    ଶୂନ୍ୟ *mmio;
    ଅକ୍ଷର name[16];
    ସଂରଚନା vring_virtqueue *vq;
    ସଂରଚନା vp_device *vp;
    u16 target;
    u16 lun;
};

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
virtio_scsi_process_op(ସଂରଚନା disk_op_s *op)
{
    ଯଦି (! CONFIG_VIRTIO_SCSI)
        ଫେରନ୍ତୁ 0;
    ସଂରଚନା virtio_lun_s *vlun =
        container_of(op->drive_fl, ସଂରଚନା virtio_lun_s, drive);
    ସଂରଚନା vp_device *vp = vlun->vp;
    ସଂରଚନା vring_virtqueue *vq = vlun->vq;
    ସଂରଚନା virtio_scsi_req_cmd req;
    ସଂରଚନା virtio_scsi_resp_cmd resp;
    ସଂରଚନା vring_list sg[3];

    memset(&req, 0, ଆକାର(req));
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା blocksize = scsi_fill_cmd(op, req.cdb, 16);
    ଯଦି (blocksize < 0)
        ଫେରନ୍ତୁ default_process_op(op);
    req.lun[0] = 1;
    req.lun[1] = vlun->target;
    req.lun[2] = (vlun->lun >> 8) | 0x40;
    req.lun[3] = (vlun->lun & 0xff);

    u32 len = op->ଗଣନା_2 * blocksize;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା datain = scsi_is_read(op);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା in_ଅଙ୍କ = (datain ? 2 : 1);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା out_ଅଙ୍କ = (len ? 3 : 2) - in_ଅଙ୍କ;

    sg[0].addr   = (ଶୂନ୍ୟ*)(&req);
    sg[0].length = ଆକାର(req);

    sg[out_ଅଙ୍କ].addr   = (ଶୂନ୍ୟ*)(&resp);
    sg[out_ଅଙ୍କ].length = ଆକାର(resp);

    ଯଦି (len) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା data_idx = (datain ? 2 : 1);
        sg[data_idx].addr   = op->buf_fl;
        sg[data_idx].length = len;
    }

    /* Add to virtqueue and kick host */
    vring_add_buf(vq, sg, out_ଅଙ୍କ, in_ଅଙ୍କ, 0, 0);
    vring_kick(vp, vq, 1);

    /* Wait for reply */
    ଯେତେବେଳେ (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, ଖାଲି);

    /* Clear interrupt status register.  Avoid leaving interrupts stuck if
     * VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
     */
    vp_get_isr(vp);

    ଯଦି (resp.response == VIRTIO_SCSI_S_OK && resp.status == 0) {
        ଫେରନ୍ତୁ DISK_RET_SUCCESS;
    }
    ଫେରନ୍ତୁ DISK_RET_EBADTRACK;
}

ସ୍ଥିର ଶୂନ୍ୟ
virtio_scsi_init_lun(ସଂରଚନା virtio_lun_s *vlun,
                     ସଂରଚନା pci_device *pci, ଶୂନ୍ୟ *mmio,
                     ସଂରଚନା vp_device *vp, ସଂରଚନା vring_virtqueue *vq,
                     u16 target, u16 lun)
{
    memset(vlun, 0, ଆକାର(*vlun));
    vlun->drive.type = DTYPE_VIRTIO_SCSI;
    vlun->drive.cntl_id = pci->bdf;
    vlun->pci = pci;
    vlun->mmio = mmio;
    vlun->vp = vp;
    vlun->vq = vq;
    vlun->target = target;
    vlun->lun = lun;
    ଯଦି (vlun->pci)
        snprintf(vlun->name, ଆକାର(vlun->name), "pci:%pP", vlun->pci);
    ଯଦି (vlun->mmio)
        snprintf(vlun->name, ଆକାର(vlun->name), "mmio:%08x", (u32)vlun->mmio);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
virtio_scsi_add_lun(u32 lun, ସଂରଚନା drive_s *tmpl_drv)
{
    u8 skip_nonbootable = is_bootprio_strict();
    ସଂରଚନା virtio_lun_s *tmpl_vlun =
        container_of(tmpl_drv, ସଂରଚନା virtio_lun_s, drive);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio = -1;

    ଯଦି (tmpl_vlun->pci)
        prio = bootprio_find_scsi_device(tmpl_vlun->pci, tmpl_vlun->target, lun);
    ଯଦି (tmpl_vlun->mmio)
        prio = bootprio_find_scsi_mmio_device(tmpl_vlun->mmio, tmpl_vlun->target, lun);

    ଯଦି (skip_nonbootable && prio < 0) {
        dprintf(1, "skipping init of a non-bootable virtio-scsi dev at %s,"
                " target %d, lun %d\n",
                tmpl_vlun->name, tmpl_vlun->target, lun);
        ଫେରନ୍ତୁ -1;
    }

    ସଂରଚନା virtio_lun_s *vlun = malloc_ତଳ(ଆକାର(*vlun));
    ଯଦି (!vlun) {
        warn_noalloc();
        ଫେରନ୍ତୁ -1;
    }
    virtio_scsi_init_lun(vlun, tmpl_vlun->pci, tmpl_vlun->mmio,tmpl_vlun->vp,
                         tmpl_vlun->vq, tmpl_vlun->target, lun);

    ଯଦି (vlun->pci)
        boot_lchs_find_scsi_device(vlun->pci, vlun->target, vlun->lun,
                                   &(vlun->drive.lchs));
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = scsi_drive_setup(&vlun->drive, "virtio-scsi", prio);
    ଯଦି (ret)
        ଯାଅ fail;
    ଫେରନ୍ତୁ 0;

fail:
    ମୁକ୍ତ(vlun);
    ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
virtio_scsi_scan_target(ସଂରଚନା pci_device *pci, ଶୂନ୍ୟ *mmio, ସଂରଚନା vp_device *vp,
                        ସଂରଚନା vring_virtqueue *vq, u16 target)
{

    ସଂରଚନା virtio_lun_s vlun0;

    virtio_scsi_init_lun(&vlun0, pci, mmio, vp, vq, target, 0);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = scsi_rep_luns_scan(&vlun0.drive, virtio_scsi_add_lun);
    ଫେରନ୍ତୁ ret < 0 ? 0 : ret;
}

ସ୍ଥିର ଶୂନ୍ୟ
init_virtio_scsi(ଶୂନ୍ୟ *data)
{
    ସଂରଚନା pci_device *pci = data;
    dprintf(1, "found virtio-scsi at %pP\n", pci);
    ସଂରଚନା vring_virtqueue *vq = ଖାଲି;
    ସଂରଚନା vp_device *vp = malloc_ଉପର(ଆକାର(*vp));
    ଯଦି (!vp) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    vp_init_simple(vp, pci);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    ଯଦି (vp->use_modern) {
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        ଯଦି (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            ଯାଅ fail;
        }

        vp_set_features(vp, features & (version1 | iommu_platform));
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        ଯଦି (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            ଯାଅ fail;
        }
    }

    ଯଦି (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi %pP\n", pci);
        ଯାଅ fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, tot;
    ପାଇଁ (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(pci, ଖାଲି, vp, vq, i);

    ଯଦି (!tot)
        ଯାଅ fail;

    ଫେରନ୍ତୁ;

fail:
    vp_reset(vp);
    ମୁକ୍ତ(vp);
    ମୁକ୍ତ(vq);
}

ଶୂନ୍ୟ
init_virtio_scsi_mmio(ଶୂନ୍ୟ *mmio)
{
    dprintf(1, "found virtio-scsi-mmio at %p\n", mmio);
    ସଂରଚନା vring_virtqueue *vq = ଖାଲି;
    ସଂରଚନା vp_device *vp = malloc_ଉପର(ଆକାର(*vp));
    ଯଦି (!vp) {
        warn_noalloc();
        ଫେରନ୍ତୁ;
    }
    vp_init_mmio(vp, mmio);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_VERSION_1;
    ଯଦି (features & version1) {
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;

        vp_set_features(vp, features & (version1 | iommu_platform));
        vp_set_status(vp, VIRTIO_CONFIG_S_FEATURES_OK);
        ଯଦି (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", mmio);
            ଯାଅ fail;
        }
    }

    ଯଦି (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi-mmio %p\n", mmio);
        ଯାଅ fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, tot;
    ପାଇଁ (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(ଖାଲି, mmio, vp, vq, i);

    ଯଦି (!tot)
        ଯାଅ fail;

    ଫେରନ୍ତୁ;

fail:
    vp_reset(vp);
    ମୁକ୍ତ(vp);
    ମୁକ୍ତ(vq);
}

ଶୂନ୍ୟ
virtio_scsi_setup(ଶୂନ୍ୟ)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    ଯଦି (! CONFIG_VIRTIO_SCSI)
        ଫେରନ୍ତୁ;

    dprintf(3, "init virtio-scsi\n");

    ସଂରଚନା pci_device *pci;
    foreachpci(pci) {
        ଯଦି (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_10))
            ଜାରି;

        ଯଦି (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-scsi at %pP\n",
                    pci);
            ଜାରି;
        }

        run_thread(init_virtio_scsi, pci);
    }
}

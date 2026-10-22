// Virtio SCSI boot support.
//
// Copyright (C) 2012 Red Hat Inc.
//
// Authors:
//  Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "block.h" // struct drive_s
#અન્તર્ભાવ "blockcmd.h" // scsi_drive_setup
#અન્તર્ભાવ "config.h" // CONFIG_*
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
#અન્તર્ભાવ "virtio-ring.h"
#અન્તર્ભાવ "virtio-scsi.h"
#અન્તર્ભાવ "virtio-mmio.h"

સંરચના virtio_lun_s {
    સંરચના drive_s drive;
    સંરચના pci_device *pci;
    શૂન્ય *mmio;
    અક્ષર name[16];
    સંરચના vring_virtqueue *vq;
    સંરચના vp_device *vp;
    u16 target;
    u16 lun;
};

પૂર્ણાંક
virtio_scsi_process_op(સંરચના disk_op_s *op)
{
    જો (! CONFIG_VIRTIO_SCSI)
        પાછા 0;
    સંરચના virtio_lun_s *vlun =
        container_of(op->drive_fl, સંરચના virtio_lun_s, drive);
    સંરચના vp_device *vp = vlun->vp;
    સંરચના vring_virtqueue *vq = vlun->vq;
    સંરચના virtio_scsi_req_cmd req;
    સંરચના virtio_scsi_resp_cmd resp;
    સંરચના vring_list sg[3];

    memset(&req, 0, કદ(req));
    પૂર્ણાંક blocksize = scsi_fill_cmd(op, req.cdb, 16);
    જો (blocksize < 0)
        પાછા default_process_op(op);
    req.lun[0] = 1;
    req.lun[1] = vlun->target;
    req.lun[2] = (vlun->lun >> 8) | 0x40;
    req.lun[3] = (vlun->lun & 0xff);

    u32 len = op->ગણતરી * blocksize;
    પૂર્ણાંક datain = scsi_is_read(op);
    પૂર્ણાંક in_અંક = (datain ? 2 : 1);
    પૂર્ણાંક out_અંક = (len ? 3 : 2) - in_અંક;

    sg[0].addr   = (શૂન્ય*)(&req);
    sg[0].length = કદ(req);

    sg[out_અંક].addr   = (શૂન્ય*)(&resp);
    sg[out_અંક].length = કદ(resp);

    જો (len) {
        પૂર્ણાંક data_idx = (datain ? 2 : 1);
        sg[data_idx].addr   = op->buf_fl;
        sg[data_idx].length = len;
    }

    /* Add to virtqueue and kick host */
    vring_add_buf(vq, sg, out_અંક, in_અંક, 0, 0);
    vring_kick(vp, vq, 1);

    /* Wait for reply */
    જ્યારે (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, ખાલી);

    /* Clear interrupt status register.  Avoid leaving interrupts stuck if
     * VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
     */
    vp_get_isr(vp);

    જો (resp.response == VIRTIO_SCSI_S_OK && resp.status == 0) {
        પાછા DISK_RET_SUCCESS;
    }
    પાછા DISK_RET_EBADTRACK;
}

સ્થિર શૂન્ય
virtio_scsi_init_lun(સંરચના virtio_lun_s *vlun,
                     સંરચના pci_device *pci, શૂન્ય *mmio,
                     સંરચના vp_device *vp, સંરચના vring_virtqueue *vq,
                     u16 target, u16 lun)
{
    memset(vlun, 0, કદ(*vlun));
    vlun->drive.type = DTYPE_VIRTIO_SCSI;
    vlun->drive.cntl_id = pci->bdf;
    vlun->pci = pci;
    vlun->mmio = mmio;
    vlun->vp = vp;
    vlun->vq = vq;
    vlun->target = target;
    vlun->lun = lun;
    જો (vlun->pci)
        snprintf(vlun->name, કદ(vlun->name), "pci:%pP", vlun->pci);
    જો (vlun->mmio)
        snprintf(vlun->name, કદ(vlun->name), "mmio:%08x", (u32)vlun->mmio);
}

સ્થિર પૂર્ણાંક
virtio_scsi_add_lun(u32 lun, સંરચના drive_s *tmpl_drv)
{
    u8 skip_nonbootable = is_bootprio_strict();
    સંરચના virtio_lun_s *tmpl_vlun =
        container_of(tmpl_drv, સંરચના virtio_lun_s, drive);
    પૂર્ણાંક prio = -1;

    જો (tmpl_vlun->pci)
        prio = bootprio_find_scsi_device(tmpl_vlun->pci, tmpl_vlun->target, lun);
    જો (tmpl_vlun->mmio)
        prio = bootprio_find_scsi_mmio_device(tmpl_vlun->mmio, tmpl_vlun->target, lun);

    જો (skip_nonbootable && prio < 0) {
        dprintf(1, "skipping init of a non-bootable virtio-scsi dev at %s,"
                " target %d, lun %d\n",
                tmpl_vlun->name, tmpl_vlun->target, lun);
        પાછા -1;
    }

    સંરચના virtio_lun_s *vlun = malloc_નીચલું(કદ(*vlun));
    જો (!vlun) {
        warn_noalloc();
        પાછા -1;
    }
    virtio_scsi_init_lun(vlun, tmpl_vlun->pci, tmpl_vlun->mmio,tmpl_vlun->vp,
                         tmpl_vlun->vq, tmpl_vlun->target, lun);

    જો (vlun->pci)
        boot_lchs_find_scsi_device(vlun->pci, vlun->target, vlun->lun,
                                   &(vlun->drive.lchs));
    પૂર્ણાંક ret = scsi_drive_setup(&vlun->drive, "virtio-scsi", prio);
    જો (ret)
        જાઓ fail;
    પાછા 0;

fail:
    મુક્ત(vlun);
    પાછા -1;
}

સ્થિર પૂર્ણાંક
virtio_scsi_scan_target(સંરચના pci_device *pci, શૂન્ય *mmio, સંરચના vp_device *vp,
                        સંરચના vring_virtqueue *vq, u16 target)
{

    સંરચના virtio_lun_s vlun0;

    virtio_scsi_init_lun(&vlun0, pci, mmio, vp, vq, target, 0);

    પૂર્ણાંક ret = scsi_rep_luns_scan(&vlun0.drive, virtio_scsi_add_lun);
    પાછા ret < 0 ? 0 : ret;
}

સ્થિર શૂન્ય
init_virtio_scsi(શૂન્ય *data)
{
    સંરચના pci_device *pci = data;
    dprintf(1, "found virtio-scsi at %pP\n", pci);
    સંરચના vring_virtqueue *vq = ખાલી;
    સંરચના vp_device *vp = malloc_ઉપલું(કદ(*vp));
    જો (!vp) {
        warn_noalloc();
        પાછા;
    }
    vp_init_simple(vp, pci);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    જો (vp->use_modern) {
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        જો (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            જાઓ fail;
        }

        vp_set_features(vp, features & (version1 | iommu_platform));
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        જો (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            જાઓ fail;
        }
    }

    જો (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi %pP\n", pci);
        જાઓ fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    પૂર્ણાંક i, tot;
    માટે (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(pci, ખાલી, vp, vq, i);

    જો (!tot)
        જાઓ fail;

    પાછા;

fail:
    vp_reset(vp);
    મુક્ત(vp);
    મુક્ત(vq);
}

શૂન્ય
init_virtio_scsi_mmio(શૂન્ય *mmio)
{
    dprintf(1, "found virtio-scsi-mmio at %p\n", mmio);
    સંરચના vring_virtqueue *vq = ખાલી;
    સંરચના vp_device *vp = malloc_ઉપલું(કદ(*vp));
    જો (!vp) {
        warn_noalloc();
        પાછા;
    }
    vp_init_mmio(vp, mmio);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_VERSION_1;
    જો (features & version1) {
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;

        vp_set_features(vp, features & (version1 | iommu_platform));
        vp_set_status(vp, VIRTIO_CONFIG_S_FEATURES_OK);
        જો (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", mmio);
            જાઓ fail;
        }
    }

    જો (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi-mmio %p\n", mmio);
        જાઓ fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    પૂર્ણાંક i, tot;
    માટે (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(ખાલી, mmio, vp, vq, i);

    જો (!tot)
        જાઓ fail;

    પાછા;

fail:
    vp_reset(vp);
    મુક્ત(vp);
    મુક્ત(vq);
}

શૂન્ય
virtio_scsi_setup(શૂન્ય)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    જો (! CONFIG_VIRTIO_SCSI)
        પાછા;

    dprintf(3, "init virtio-scsi\n");

    સંરચના pci_device *pci;
    foreachpci(pci) {
        જો (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_10))
            ચાલુ;

        જો (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-scsi at %pP\n",
                    pci);
            ચાલુ;
        }

        run_thread(init_virtio_scsi, pci);
    }
}

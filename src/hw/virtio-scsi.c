// Virtio SCSI boot support.
//
// Copyright (C) 2012 Red Hat Inc.
//
// Authors:
//  Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "block.h" // struct drive_s
#include "blockcmd.h" // scsi_drive_setup
#include "config.h" // CONFIG_*
#include "malloc.h" // free
#include "output.h" // dprintf
#include "pcidevice.h" // foreachpci
#include "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#include "pci_regs.h" // PCI_VENDOR_ID
#include "stacks.h" // run_thread
#include "std/disk.h" // DISK_RET_SUCCESS
#include "string.h" // memset
#include "util.h" // usleep, bootprio_find_pci_device, is_bootprio_strict
#include "virtio-pci.h"
#include "virtio-ring.h"
#include "virtio-scsi.h"
#include "virtio-mmio.h"

sa_nracanaa virtio_lun_s {
    sa_nracanaa drive_s drive;
    sa_nracanaa pci_device *pci;
    void *mmio;
    var_na name[16];
    sa_nracanaa vring_virtqueue *vq;
    sa_nracanaa vp_device *vp;
    u16 target;
    u16 lun;
};

ཧྲིལ
virtio_scsi_process_op(sa_nracanaa disk_op_s *op)
{
    གལ་སྲིད (! CONFIG_VIRTIO_SCSI)
        ལོག 0;
    sa_nracanaa virtio_lun_s *vlun =
        container_of(op->drive_fl, sa_nracanaa virtio_lun_s, drive);
    sa_nracanaa vp_device *vp = vlun->vp;
    sa_nracanaa vring_virtqueue *vq = vlun->vq;
    sa_nracanaa virtio_scsi_req_cmd req;
    sa_nracanaa virtio_scsi_resp_cmd resp;
    sa_nracanaa vring_list sg[3];

    memset(&req, 0, parimaa_na(req));
    ཧྲིལ blocksize = scsi_fill_cmd(op, req.cdb, 16);
    གལ་སྲིད (blocksize < 0)
        ལོག default_process_op(op);
    req.lun[0] = 1;
    req.lun[1] = vlun->target;
    req.lun[2] = (vlun->lun >> 8) | 0x40;
    req.lun[3] = (vlun->lun & 0xff);

    u32 len = op->count * blocksize;
    ཧྲིལ datain = scsi_is_read(op);
    ཧྲིལ in_num = (datain ? 2 : 1);
    ཧྲིལ out_num = (len ? 3 : 2) - in_num;

    sg[0].addr   = (void*)(&req);
    sg[0].length = parimaa_na(req);

    sg[out_num].addr   = (void*)(&resp);
    sg[out_num].length = parimaa_na(resp);

    གལ་སྲིད (len) {
        ཧྲིལ data_idx = (datain ? 2 : 1);
        sg[data_idx].addr   = op->buf_fl;
        sg[data_idx].length = len;
    }

    /* Add to virtqueue and kick host */
    vring_add_buf(vq, sg, out_num, in_num, 0, 0);
    vring_kick(vp, vq, 1);

    /* Wait for reply */
    སྐབས (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, NULL);

    /* Clear interrupt status register.  Avoid leaving interrupts stuck if
     * VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
     */
    vp_get_isr(vp);

    གལ་སྲིད (resp.response == VIRTIO_SCSI_S_OK && resp.status == 0) {
        ལོག DISK_RET_SUCCESS;
    }
    ལོག DISK_RET_EBADTRACK;
}

static void
virtio_scsi_init_lun(sa_nracanaa virtio_lun_s *vlun,
                     sa_nracanaa pci_device *pci, void *mmio,
                     sa_nracanaa vp_device *vp, sa_nracanaa vring_virtqueue *vq,
                     u16 target, u16 lun)
{
    memset(vlun, 0, parimaa_na(*vlun));
    vlun->drive.type = DTYPE_VIRTIO_SCSI;
    vlun->drive.cntl_id = pci->bdf;
    vlun->pci = pci;
    vlun->mmio = mmio;
    vlun->vp = vp;
    vlun->vq = vq;
    vlun->target = target;
    vlun->lun = lun;
    གལ་སྲིད (vlun->pci)
        snprintf(vlun->name, parimaa_na(vlun->name), "pci:%pP", vlun->pci);
    གལ་སྲིད (vlun->mmio)
        snprintf(vlun->name, parimaa_na(vlun->name), "mmio:%08x", (u32)vlun->mmio);
}

static ཧྲིལ
virtio_scsi_add_lun(u32 lun, sa_nracanaa drive_s *tmpl_drv)
{
    u8 skip_nonbootable = is_bootprio_strict();
    sa_nracanaa virtio_lun_s *tmpl_vlun =
        container_of(tmpl_drv, sa_nracanaa virtio_lun_s, drive);
    ཧྲིལ prio = -1;

    གལ་སྲིད (tmpl_vlun->pci)
        prio = bootprio_find_scsi_device(tmpl_vlun->pci, tmpl_vlun->target, lun);
    གལ་སྲིད (tmpl_vlun->mmio)
        prio = bootprio_find_scsi_mmio_device(tmpl_vlun->mmio, tmpl_vlun->target, lun);

    གལ་སྲིད (skip_nonbootable && prio < 0) {
        dprintf(1, "skipping init of a non-bootable virtio-scsi dev at %s,"
                " target %d, lun %d\n",
                tmpl_vlun->name, tmpl_vlun->target, lun);
        ལོག -1;
    }

    sa_nracanaa virtio_lun_s *vlun = malloc_low(parimaa_na(*vlun));
    གལ་སྲིད (!vlun) {
        warn_noalloc();
        ལོག -1;
    }
    virtio_scsi_init_lun(vlun, tmpl_vlun->pci, tmpl_vlun->mmio,tmpl_vlun->vp,
                         tmpl_vlun->vq, tmpl_vlun->target, lun);

    གལ་སྲིད (vlun->pci)
        boot_lchs_find_scsi_device(vlun->pci, vlun->target, vlun->lun,
                                   &(vlun->drive.lchs));
    ཧྲིལ ret = scsi_drive_setup(&vlun->drive, "virtio-scsi", prio);
    གལ་སྲིད (ret)
        goto fail;
    ལོག 0;

fail:
    free(vlun);
    ལོག -1;
}

static ཧྲིལ
virtio_scsi_scan_target(sa_nracanaa pci_device *pci, void *mmio, sa_nracanaa vp_device *vp,
                        sa_nracanaa vring_virtqueue *vq, u16 target)
{

    sa_nracanaa virtio_lun_s vlun0;

    virtio_scsi_init_lun(&vlun0, pci, mmio, vp, vq, target, 0);

    ཧྲིལ ret = scsi_rep_luns_scan(&vlun0.drive, virtio_scsi_add_lun);
    ལོག ret < 0 ? 0 : ret;
}

static void
init_virtio_scsi(void *data)
{
    sa_nracanaa pci_device *pci = data;
    dprintf(1, "found virtio-scsi at %pP\n", pci);
    sa_nracanaa vring_virtqueue *vq = NULL;
    sa_nracanaa vp_device *vp = malloc_high(parimaa_na(*vp));
    གལ་སྲིད (!vp) {
        warn_noalloc();
        ལོག;
    }
    vp_init_simple(vp, pci);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    གལ་སྲིད (vp->use_modern) {
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        གལ་སྲིད (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            goto fail;
        }

        vp_set_features(vp, features & (version1 | iommu_platform));
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        གལ་སྲིད (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            goto fail;
        }
    }

    གལ་སྲིད (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi %pP\n", pci);
        goto fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    ཧྲིལ i, tot;
    དོན་ལུ (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(pci, NULL, vp, vq, i);

    གལ་སྲིད (!tot)
        goto fail;

    ལོག;

fail:
    vp_reset(vp);
    free(vp);
    free(vq);
}

void
init_virtio_scsi_mmio(void *mmio)
{
    dprintf(1, "found virtio-scsi-mmio at %p\n", mmio);
    sa_nracanaa vring_virtqueue *vq = NULL;
    sa_nracanaa vp_device *vp = malloc_high(parimaa_na(*vp));
    གལ་སྲིད (!vp) {
        warn_noalloc();
        ལོག;
    }
    vp_init_mmio(vp, mmio);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_VERSION_1;
    གལ་སྲིད (features & version1) {
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;

        vp_set_features(vp, features & (version1 | iommu_platform));
        vp_set_status(vp, VIRTIO_CONFIG_S_FEATURES_OK);
        གལ་སྲིད (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", mmio);
            goto fail;
        }
    }

    གལ་སྲིད (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi-mmio %p\n", mmio);
        goto fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    ཧྲིལ i, tot;
    དོན་ལུ (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(NULL, mmio, vp, vq, i);

    གལ་སྲིད (!tot)
        goto fail;

    ལོག;

fail:
    vp_reset(vp);
    free(vp);
    free(vq);
}

void
virtio_scsi_setup(void)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    གལ་སྲིད (! CONFIG_VIRTIO_SCSI)
        ལོག;

    dprintf(3, "init virtio-scsi\n");

    sa_nracanaa pci_device *pci;
    foreachpci(pci) {
        གལ་སྲིད (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_10))
            continue;

        གལ་སྲིད (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-scsi at %pP\n",
                    pci);
            continue;
        }

        run_thread(init_virtio_scsi, pci);
    }
}

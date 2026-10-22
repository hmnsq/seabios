// Virtio SCSI boot support.
//
// Copyright (C) 2012 Red Hat Inc.
//
// Authors:
//  Paolo Bonzini <pbonzini@redhat.com>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "block.h" // struct drive_s
#അന്തര്ഭാവ "blockcmd.h" // scsi_drive_setup
#അന്തര്ഭാവ "config.h" // CONFIG_*
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "pcidevice.h" // foreachpci
#അന്തര്ഭാവ "pci_ids.h" // PCI_DEVICE_ID_VIRTIO_BLK
#അന്തര്ഭാവ "pci_regs.h" // PCI_VENDOR_ID
#അന്തര്ഭാവ "stacks.h" // run_thread
#അന്തര്ഭാവ "std/disk.h" // DISK_RET_SUCCESS
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // usleep, bootprio_find_pci_device, is_bootprio_strict
#അന്തര്ഭാവ "virtio-pci.h"
#അന്തര്ഭാവ "virtio-ring.h"
#അന്തര്ഭാവ "virtio-scsi.h"
#അന്തര്ഭാവ "virtio-mmio.h"

ഘടന virtio_lun_s {
    ഘടന drive_s drive;
    ഘടന pci_device *pci;
    ശൂന്യം *mmio;
    അക്ഷരം name[16];
    ഘടന vring_virtqueue *vq;
    ഘടന vp_device *vp;
    u16 target;
    u16 lun;
};

പൂർണ്ണസംഖ്യ
virtio_scsi_process_op(ഘടന disk_op_s *op)
{
    എങ്കിൽ (! CONFIG_VIRTIO_SCSI)
        തിരികെ 0;
    ഘടന virtio_lun_s *vlun =
        container_of(op->drive_fl, ഘടന virtio_lun_s, drive);
    ഘടന vp_device *vp = vlun->vp;
    ഘടന vring_virtqueue *vq = vlun->vq;
    ഘടന virtio_scsi_req_cmd req;
    ഘടന virtio_scsi_resp_cmd resp;
    ഘടന vring_list sg[3];

    memset(&req, 0, വലുപ്പം(req));
    പൂർണ്ണസംഖ്യ blocksize = scsi_fill_cmd(op, req.cdb, 16);
    എങ്കിൽ (blocksize < 0)
        തിരികെ default_process_op(op);
    req.lun[0] = 1;
    req.lun[1] = vlun->target;
    req.lun[2] = (vlun->lun >> 8) | 0x40;
    req.lun[3] = (vlun->lun & 0xff);

    u32 len = op->എണ്ണം * blocksize;
    പൂർണ്ണസംഖ്യ datain = scsi_is_read(op);
    പൂർണ്ണസംഖ്യ in_അക്കം = (datain ? 2 : 1);
    പൂർണ്ണസംഖ്യ out_അക്കം = (len ? 3 : 2) - in_അക്കം;

    sg[0].addr   = (ശൂന്യം*)(&req);
    sg[0].length = വലുപ്പം(req);

    sg[out_അക്കം].addr   = (ശൂന്യം*)(&resp);
    sg[out_അക്കം].length = വലുപ്പം(resp);

    എങ്കിൽ (len) {
        പൂർണ്ണസംഖ്യ data_idx = (datain ? 2 : 1);
        sg[data_idx].addr   = op->buf_fl;
        sg[data_idx].length = len;
    }

    /* Add to virtqueue and kick host */
    vring_add_buf(vq, sg, out_അക്കം, in_അക്കം, 0, 0);
    vring_kick(vp, vq, 1);

    /* Wait for reply */
    ആകുമ്പോൾ (!vring_more_used(vq))
        usleep(5);

    /* Reclaim virtqueue element */
    vring_get_buf(vq, ശൂന്യ);

    /* Clear interrupt status register.  Avoid leaving interrupts stuck if
     * VRING_AVAIL_F_NO_INTERRUPT was ignored and interrupts were raised.
     */
    vp_get_isr(vp);

    എങ്കിൽ (resp.response == VIRTIO_SCSI_S_OK && resp.status == 0) {
        തിരികെ DISK_RET_SUCCESS;
    }
    തിരികെ DISK_RET_EBADTRACK;
}

സ്ഥിതം ശൂന്യം
virtio_scsi_init_lun(ഘടന virtio_lun_s *vlun,
                     ഘടന pci_device *pci, ശൂന്യം *mmio,
                     ഘടന vp_device *vp, ഘടന vring_virtqueue *vq,
                     u16 target, u16 lun)
{
    memset(vlun, 0, വലുപ്പം(*vlun));
    vlun->drive.type = DTYPE_VIRTIO_SCSI;
    vlun->drive.cntl_id = pci->bdf;
    vlun->pci = pci;
    vlun->mmio = mmio;
    vlun->vp = vp;
    vlun->vq = vq;
    vlun->target = target;
    vlun->lun = lun;
    എങ്കിൽ (vlun->pci)
        snprintf(vlun->name, വലുപ്പം(vlun->name), "pci:%pP", vlun->pci);
    എങ്കിൽ (vlun->mmio)
        snprintf(vlun->name, വലുപ്പം(vlun->name), "mmio:%08x", (u32)vlun->mmio);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
virtio_scsi_add_lun(u32 lun, ഘടന drive_s *tmpl_drv)
{
    u8 skip_nonbootable = is_bootprio_strict();
    ഘടന virtio_lun_s *tmpl_vlun =
        container_of(tmpl_drv, ഘടന virtio_lun_s, drive);
    പൂർണ്ണസംഖ്യ prio = -1;

    എങ്കിൽ (tmpl_vlun->pci)
        prio = bootprio_find_scsi_device(tmpl_vlun->pci, tmpl_vlun->target, lun);
    എങ്കിൽ (tmpl_vlun->mmio)
        prio = bootprio_find_scsi_mmio_device(tmpl_vlun->mmio, tmpl_vlun->target, lun);

    എങ്കിൽ (skip_nonbootable && prio < 0) {
        dprintf(1, "skipping init of a non-bootable virtio-scsi dev at %s,"
                " target %d, lun %d\n",
                tmpl_vlun->name, tmpl_vlun->target, lun);
        തിരികെ -1;
    }

    ഘടന virtio_lun_s *vlun = malloc_താഴെയുള്ള(വലുപ്പം(*vlun));
    എങ്കിൽ (!vlun) {
        warn_noalloc();
        തിരികെ -1;
    }
    virtio_scsi_init_lun(vlun, tmpl_vlun->pci, tmpl_vlun->mmio,tmpl_vlun->vp,
                         tmpl_vlun->vq, tmpl_vlun->target, lun);

    എങ്കിൽ (vlun->pci)
        boot_lchs_find_scsi_device(vlun->pci, vlun->target, vlun->lun,
                                   &(vlun->drive.lchs));
    പൂർണ്ണസംഖ്യ ret = scsi_drive_setup(&vlun->drive, "virtio-scsi", prio);
    എങ്കിൽ (ret)
        പോകുക fail;
    തിരികെ 0;

fail:
    സ്വതന്ത്രം(vlun);
    തിരികെ -1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
virtio_scsi_scan_target(ഘടന pci_device *pci, ശൂന്യം *mmio, ഘടന vp_device *vp,
                        ഘടന vring_virtqueue *vq, u16 target)
{

    ഘടന virtio_lun_s vlun0;

    virtio_scsi_init_lun(&vlun0, pci, mmio, vp, vq, target, 0);

    പൂർണ്ണസംഖ്യ ret = scsi_rep_luns_scan(&vlun0.drive, virtio_scsi_add_lun);
    തിരികെ ret < 0 ? 0 : ret;
}

സ്ഥിതം ശൂന്യം
init_virtio_scsi(ശൂന്യം *data)
{
    ഘടന pci_device *pci = data;
    dprintf(1, "found virtio-scsi at %pP\n", pci);
    ഘടന vring_virtqueue *vq = ശൂന്യ;
    ഘടന vp_device *vp = malloc_മുകളിലെ(വലുപ്പം(*vp));
    എങ്കിൽ (!vp) {
        warn_noalloc();
        തിരികെ;
    }
    vp_init_simple(vp, pci);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    എങ്കിൽ (vp->use_modern) {
        u64 features = vp_get_features(vp);
        u64 version1 = 1ull << VIRTIO_F_VERSION_1;
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;
        എങ്കിൽ (!(features & version1)) {
            dprintf(1, "modern device without virtio_1 feature bit: %pP\n", pci);
            പോകുക fail;
        }

        vp_set_features(vp, features & (version1 | iommu_platform));
        status |= VIRTIO_CONFIG_S_FEATURES_OK;
        vp_set_status(vp, status);
        എങ്കിൽ (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", pci);
            പോകുക fail;
        }
    }

    എങ്കിൽ (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi %pP\n", pci);
        പോകുക fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    പൂർണ്ണസംഖ്യ i, tot;
    വേണ്ടി (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(pci, ശൂന്യ, vp, vq, i);

    എങ്കിൽ (!tot)
        പോകുക fail;

    തിരികെ;

fail:
    vp_reset(vp);
    സ്വതന്ത്രം(vp);
    സ്വതന്ത്രം(vq);
}

ശൂന്യം
init_virtio_scsi_mmio(ശൂന്യം *mmio)
{
    dprintf(1, "found virtio-scsi-mmio at %p\n", mmio);
    ഘടന vring_virtqueue *vq = ശൂന്യ;
    ഘടന vp_device *vp = malloc_മുകളിലെ(വലുപ്പം(*vp));
    എങ്കിൽ (!vp) {
        warn_noalloc();
        തിരികെ;
    }
    vp_init_mmio(vp, mmio);
    u8 status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

    u64 features = vp_get_features(vp);
    u64 version1 = 1ull << VIRTIO_F_VERSION_1;
    എങ്കിൽ (features & version1) {
        u64 iommu_platform = 1ull << VIRTIO_F_IOMMU_PLATFORM;

        vp_set_features(vp, features & (version1 | iommu_platform));
        vp_set_status(vp, VIRTIO_CONFIG_S_FEATURES_OK);
        എങ്കിൽ (!(vp_get_status(vp) & VIRTIO_CONFIG_S_FEATURES_OK)) {
            dprintf(1, "device didn't accept features: %pP\n", mmio);
            പോകുക fail;
        }
    }

    എങ്കിൽ (vp_find_vq(vp, 2, &vq) < 0 ) {
        dprintf(1, "fail to find vq for virtio-scsi-mmio %p\n", mmio);
        പോകുക fail;
    }

    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    vp_set_status(vp, status);

    പൂർണ്ണസംഖ്യ i, tot;
    വേണ്ടി (tot = 0, i = 0; i < 256; i++)
        tot += virtio_scsi_scan_target(ശൂന്യ, mmio, vp, vq, i);

    എങ്കിൽ (!tot)
        പോകുക fail;

    തിരികെ;

fail:
    vp_reset(vp);
    സ്വതന്ത്രം(vp);
    സ്വതന്ത്രം(vq);
}

ശൂന്യം
virtio_scsi_setup(ശൂന്യം)
{
    u8 skip_nonbootable = is_bootprio_strict();

    ASSERT32FLAT();
    എങ്കിൽ (! CONFIG_VIRTIO_SCSI)
        തിരികെ;

    dprintf(3, "init virtio-scsi\n");

    ഘടന pci_device *pci;
    foreachpci(pci) {
        എങ്കിൽ (pci->vendor != PCI_VENDOR_ID_REDHAT_QUMRANET ||
            (pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_09 &&
             pci->device != PCI_DEVICE_ID_VIRTIO_SCSI_10))
            തുടരുക;

        എങ്കിൽ (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
            dprintf(1, "skipping init of a non-bootable virtio-scsi at %pP\n",
                    pci);
            തുടരുക;
        }

        run_thread(init_virtio_scsi, pci);
    }
}

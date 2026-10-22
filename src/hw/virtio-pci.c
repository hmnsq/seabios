/* virtio-pci.c - pci interface for virtio interface
 *
 * (c) Copyright 2008 Bull S.A.S.
 *
 *  Author: Laurent Vivier <Laurent.Vivier@bull.net>
 *
 * some parts from Linux Virtio PCI driver
 *
 *  Copyright IBM Corp. 2007
 *  Authors: Anthony Liguori  <aliguori@us.ibm.com>
 *
 *  Adopted for Seabios: Gleb Natapov <gleb@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPLv3
 * See the COPYING file in the top-level directory.
 */

#ಅನ್ತರ್ಭಾವ "config.h" // CONFIG_DEBUG_LEVEL
#ಅನ್ತರ್ಭಾವ "malloc.h" // free
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pci.h" // pci_config_readl
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // struct pci_device
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "virtio-pci.h"
#ಅನ್ತರ್ಭಾವ "virtio-mmio.h"
#ಅನ್ತರ್ಭಾವ "virtio-ring.h"

u64 _vp_read(ಸಂರಚನಾ vp_cap *cap, u32 offset, u8 size)
{
    u64 var = 0;

    ವಿಕಲ್ಪ (cap->mode) {
    ಪಕ್ಷ VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        ವಿಕಲ್ಪ (size) {
        ಪಕ್ಷ 8:
            var = inl(addr);
            var |= (u64)inl(addr+4) << 32;
            ಭಙ್ಗ;
        ಪಕ್ಷ 4:
            var = inl(addr);
            ಭಙ್ಗ;
        ಪಕ್ಷ 2:
            var = inw(addr);
            ಭಙ್ಗ;
        ಪಕ್ಷ 1:
            var = inb(addr);
            ಭಙ್ಗ;
        }
        ಭಙ್ಗ;
    }

    ಪಕ್ಷ VP_ACCESS_MMIO:
    {
        ಶೂನ್ಯ *addr = cap->memaddr + offset;
        ವಿಕಲ್ಪ (size) {
        ಪಕ್ಷ 8:
            var = readl(addr);
            var |= (u64)readl(addr+4) << 32;
            ಭಙ್ಗ;
        ಪಕ್ಷ 4:
            var = readl(addr);
            ಭಙ್ಗ;
        ಪಕ್ಷ 2:
            var = readw(addr);
            ಭಙ್ಗ;
        ಪಕ್ಷ 1:
            var = readb(addr);
            ಭಙ್ಗ;
        }
        ಭಙ್ಗ;
    }

    ಪಕ್ಷ VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, cap.length),
                          (size > 4) ? 4 : size);
        ವಿಕಲ್ಪ (size) {
        ಪಕ್ಷ 8:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, pci_cfg_data));
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, cap.offset),
                              addr + 4);
            var |= (u64)pci_config_readl(cap->bdf, cap->cfg +
                                         offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, pci_cfg_data)) << 32;
            ಭಙ್ಗ;
        ಪಕ್ಷ 4:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, pci_cfg_data));
            ಭಙ್ಗ;
        ಪಕ್ಷ 2:
            var = pci_config_readw(cap->bdf, cap->cfg +
                                   offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, pci_cfg_data));
            ಭಙ್ಗ;
        ಪಕ್ಷ 1:
            var = pci_config_readb(cap->bdf, cap->cfg +
                                   offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, pci_cfg_data));
            ಭಙ್ಗ;
        }
    }
    }
    dprintf(9, "vp read   %x (%d) -> 0x%llx\n", cap->ioaddr + offset, size, var);
    ಪ್ರತ್ಯಾಗಮ var;
}

ಶೂನ್ಯ _vp_write(ಸಂರಚನಾ vp_cap *cap, u32 offset, u8 size, u64 var)
{
    dprintf(9, "vp write  %x (%d) <- 0x%llx\n", cap->ioaddr + offset, size, var);

    ವಿಕಲ್ಪ (cap->mode) {
    ಪಕ್ಷ VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        ವಿಕಲ್ಪ (size) {
        ಪಕ್ಷ 4:
            outl(var, addr);
            ಭಙ್ಗ;
        ಪಕ್ಷ 2:
            outw(var, addr);
            ಭಙ್ಗ;
        ಪಕ್ಷ 1:
            outb(var, addr);
            ಭಙ್ಗ;
        }
        ಭಙ್ಗ;
    }

    ಪಕ್ಷ VP_ACCESS_MMIO:
    {
        ಶೂನ್ಯ *addr = cap->memaddr + offset;
        ವಿಕಲ್ಪ (size) {
        ಪಕ್ಷ 4:
            writel(addr, var);
            ಭಙ್ಗ;
        ಪಕ್ಷ 2:
            writew(addr, var);
            ಭಙ್ಗ;
        ಪಕ್ಷ 1:
            writeb(addr, var);
            ಭಙ್ಗ;
        }
        ಭಙ್ಗ;
    }

    ಪಕ್ಷ VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, cap.length),
                          size);
        ವಿಕಲ್ಪ (size) {
        ಪಕ್ಷ 4:
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            ಭಙ್ಗ;
        ಪಕ್ಷ 2:
            pci_config_writew(cap->bdf, cap->cfg +
                              offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            ಭಙ್ಗ;
        ಪಕ್ಷ 1:
            pci_config_writeb(cap->bdf, cap->cfg +
                              offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            ಭಙ್ಗ;
        }
    }
    }
}

u64 vp_get_features(ಸಂರಚನಾ vp_device *vp)
{
    u32 f0, f1;

    ಯದಿ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
    } ಅನ್ಯಥಾ ಯದಿ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
    } ಅನ್ಯಥಾ {
        f0 = vp_read(&vp->legacy, virtio_pci_legacy, host_features);
        f1 = 0;
    }
    ಪ್ರತ್ಯಾಗಮ ((u64)f1 << 32) | f0;
}

ಶೂನ್ಯ vp_set_features(ಸಂರಚನಾ vp_device *vp, u64 features)
{
    u32 f0, f1;

    f0 = features;
    f1 = features >> 32;

    ಯದಿ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f1);
    } ಅನ್ಯಥಾ ಯದಿ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f1);
    } ಅನ್ಯಥಾ {
        vp_write(&vp->legacy, virtio_pci_legacy, guest_features, f0);
    }
}

u8 vp_get_status(ಸಂರಚನಾ vp_device *vp)
{
    ಯದಿ (vp->use_mmio) {
        ಪ್ರತ್ಯಾಗಮ vp_read(&vp->common, virtio_mmio_cfg, device_status);
    } ಅನ್ಯಥಾ ಯದಿ (vp->use_modern) {
        ಪ್ರತ್ಯಾಗಮ vp_read(&vp->common, virtio_pci_common_cfg, device_status);
    } ಅನ್ಯಥಾ {
        ಪ್ರತ್ಯಾಗಮ vp_read(&vp->legacy, virtio_pci_legacy, status);
    }
}

ಶೂನ್ಯ vp_set_status(ಸಂರಚನಾ vp_device *vp, u8 status)
{
    ಯದಿ (status == 0)        /* reset */
        ಪ್ರತ್ಯಾಗಮ;
    ಯದಿ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, status);
    } ಅನ್ಯಥಾ ಯದಿ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, status);
    } ಅನ್ಯಥಾ {
        vp_write(&vp->legacy, virtio_pci_legacy, status, status);
    }
}

u8 vp_get_isr(ಸಂರಚನಾ vp_device *vp)
{
    ಯದಿ (vp->use_mmio) {
        ಪ್ರತ್ಯಾಗಮ vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } ಅನ್ಯಥಾ ಯದಿ (vp->use_modern) {
        ಪ್ರತ್ಯಾಗಮ vp_read(&vp->isr, virtio_pci_isr, isr);
    } ಅನ್ಯಥಾ {
        ಪ್ರತ್ಯಾಗಮ vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

ಶೂನ್ಯ vp_reset(ಸಂರಚನಾ vp_device *vp)
{
    ಯದಿ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, 0);
        vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } ಅನ್ಯಥಾ ಯದಿ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, 0);
        vp_read(&vp->isr, virtio_pci_isr, isr);
    } ಅನ್ಯಥಾ {
        vp_write(&vp->legacy, virtio_pci_legacy, status, 0);
        vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

ಶೂನ್ಯ vp_notify(ಸಂರಚನಾ vp_device *vp, ಸಂರಚನಾ vring_virtqueue *vq)
{
    ಯದಿ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, queue_notify, vq->queue_index);
    } ಅನ್ಯಥಾ ಯದಿ (vp->use_modern) {
        u32 offset = vq->queue_notify_off * vp->notify_off_multiplier;
        ವಿಕಲ್ಪ (vp->notify.mode) {
        ಪಕ್ಷ VP_ACCESS_IO:
            outw(vq->queue_index, vp->notify.ioaddr + offset);
            ಭಙ್ಗ;
        ಪಕ್ಷ VP_ACCESS_MMIO:
            writew(vp->notify.memaddr + offset, vq->queue_index);
            ಭಙ್ಗ;
        ಪಕ್ಷ VP_ACCESS_PCICFG:
            pci_config_writeb(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, cap.bar),
                              vp->notify.bar);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, cap.offset),
                              vp->notify.baroff + offset);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, cap.length),
                              2);
            pci_config_writew(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ಸಂರಚನಾ virtio_pci_cfg_cap, pci_cfg_data),
                              vq->queue_index);
        }
        dprintf(9, "vp notify %x (%d) -- 0x%x\n",
                vp->notify.ioaddr, 2, vq->queue_index);
    } ಅನ್ಯಥಾ {
        vp_write(&vp->legacy, virtio_pci_legacy, queue_notify, vq->queue_index);
    }
}

ಪೂರ್ಣಾಙ್ಕ vp_find_vq(ಸಂರಚನಾ vp_device *vp, ಪೂರ್ಣಾಙ್ಕ queue_index,
               ಸಂರಚನಾ vring_virtqueue **p_vq)
{
   u16 ಅಂಕೆ;

   ASSERT32FLAT();
   ಸಂರಚನಾ vring_virtqueue *vq = *p_vq = memalign_ಮೇಲಿನ(PAGE_SIZE, ಪರಿಮಾಣ(*vq));
   ಯದಿ (!vq) {
       warn_noalloc();
       ಗಚ್ಛ fail;
   }
   memset(vq, 0, ಪರಿಮಾಣ(*vq));


   /* select the queue */
   ಯದಿ (vp->use_mmio) {
       vp_write(&vp->common, virtio_mmio_cfg, queue_select, queue_index);
   } ಅನ್ಯಥಾ ಯದಿ (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, queue_select, queue_index);
   } ಅನ್ಯಥಾ {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_sel, queue_index);
   }

   /* check if the queue is available */
   ಯದಿ (vp->use_mmio) {
       ಅಂಕೆ = vp_read(&vp->common, virtio_mmio_cfg, queue_ಅಂಕೆ_max);
       ಯದಿ (ಅಂಕೆ > MAX_QUEUE_ಅಂಕೆ)
           ಅಂಕೆ = MAX_QUEUE_ಅಂಕೆ;
       vp_write(&vp->common, virtio_mmio_cfg, queue_ಅಂಕೆ, ಅಂಕೆ);
   } ಅನ್ಯಥಾ ಯದಿ (vp->use_modern) {
       ಅಂಕೆ = vp_read(&vp->common, virtio_pci_common_cfg, queue_size);
       ಯದಿ (ಅಂಕೆ > MAX_QUEUE_ಅಂಕೆ) {
           vp_write(&vp->common, virtio_pci_common_cfg, queue_size,
                    MAX_QUEUE_ಅಂಕೆ);
           ಅಂಕೆ = vp_read(&vp->common, virtio_pci_common_cfg, queue_size);
       }
   } ಅನ್ಯಥಾ {
       ಅಂಕೆ = vp_read(&vp->legacy, virtio_pci_legacy, queue_ಅಂಕೆ);
   }
   ಯದಿ (!ಅಂಕೆ) {
       dprintf(1, "ERROR: queue size is 0\n");
       ಗಚ್ಛ fail;
   }
   ಯದಿ (ಅಂಕೆ > MAX_QUEUE_ಅಂಕೆ) {
       dprintf(1, "ERROR: queue size %d > %d\n", ಅಂಕೆ, MAX_QUEUE_ಅಂಕೆ);
       ಗಚ್ಛ fail;
   }

   /* check if the queue is already active */
   ಯದಿ (vp->use_mmio) {
       /* TODO */;
   } ಅನ್ಯಥಾ ಯದಿ (vp->use_modern) {
       ಯದಿ (vp_read(&vp->common, virtio_pci_common_cfg, queue_enable)) {
           dprintf(1, "ERROR: queue already active\n");
           ಗಚ್ಛ fail;
       }
   } ಅನ್ಯಥಾ {
       ಯದಿ (vp_read(&vp->legacy, virtio_pci_legacy, queue_pfn)) {
           dprintf(1, "ERROR: queue already active\n");
           ಗಚ್ಛ fail;
       }
   }
   vq->queue_index = queue_index;

   /* initialize the queue */
   ಸಂರಚನಾ vring * vr = &vq->vring;
   vring_init(vr, ಅಂಕೆ, (unsigned ವರ್ಣ*)&vq->queue);

   /* activate the queue
    *
    * NOTE: vr->desc is initialized by vring_init()
    */

   ಯದಿ (vp->use_mmio) {
       ಯದಿ (vp_read(&vp->common, virtio_mmio_cfg, version) == 2) {
           vp_write(&vp->common, virtio_mmio_cfg, queue_desc_lo,
                    (unsigned long)virt_to_phys(vr->desc));
           vp_write(&vp->common, virtio_mmio_cfg, queue_desc_hi, 0);
           vp_write(&vp->common, virtio_mmio_cfg, queue_driver_lo,
                    (unsigned long)virt_to_phys(vr->avail));
           vp_write(&vp->common, virtio_mmio_cfg, queue_driver_hi, 0);
           vp_write(&vp->common, virtio_mmio_cfg, queue_device_lo,
                    (unsigned long)virt_to_phys(vr->used));
           vp_write(&vp->common, virtio_mmio_cfg, queue_device_hi, 0);
           vp_write(&vp->common, virtio_mmio_cfg, queue_ready, 1);
       } ಅನ್ಯಥಾ {
           vp_write(&vp->common, virtio_mmio_cfg, legacy_guest_page_size,
                    (unsigned long)1 << PAGE_SHIFT);
           vp_write(&vp->common, virtio_mmio_cfg, legacy_queue_pfn,
                    (unsigned long)virt_to_phys(vr->desc) >> PAGE_SHIFT);
       }
   } ಅನ್ಯಥಾ ಯದಿ (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, queue_desc_lo,
                (unsigned long)virt_to_phys(vr->desc));
       vp_write(&vp->common, virtio_pci_common_cfg, queue_desc_hi, 0);
       vp_write(&vp->common, virtio_pci_common_cfg, queue_avail_lo,
                (unsigned long)virt_to_phys(vr->avail));
       vp_write(&vp->common, virtio_pci_common_cfg, queue_avail_hi, 0);
       vp_write(&vp->common, virtio_pci_common_cfg, queue_used_lo,
                (unsigned long)virt_to_phys(vr->used));
       vp_write(&vp->common, virtio_pci_common_cfg, queue_used_hi, 0);
       vp_write(&vp->common, virtio_pci_common_cfg, queue_enable, 1);
       vq->queue_notify_off = vp_read(&vp->common, virtio_pci_common_cfg,
                                      queue_notify_off);
   } ಅನ್ಯಥಾ {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_pfn,
                (unsigned long)virt_to_phys(vr->desc) >> PAGE_SHIFT);
   }
   ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ;

fail:
   ಮುಕ್ತ(vq);
   *p_vq = ರಿಕ್ತ;
   ಪ್ರತ್ಯಾಗಮ -1;
}

ಶೂನ್ಯ vp_init_simple(ಸಂರಚನಾ vp_device *vp, ಸಂರಚನಾ pci_device *pci)
{
    u8 cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, 0);
    ಸಂರಚನಾ vp_cap *vp_cap;
    ಧ್ರುವ ವರ್ಣ *mode;
    u32 offset, base, mul;
    u64 addr;
    u8 type;

    memset(vp, 0, ಪರಿಮಾಣ(*vp));
    ಯಾವತ್ (cap != 0) {
        type = pci_config_readb(pci->bdf, cap +
                                offsetof(ಸಂರಚನಾ virtio_pci_cap, cfg_type));
        ವಿಕಲ್ಪ (type) {
        ಪಕ್ಷ VIRTIO_PCI_CAP_COMMON_CFG:
            vp_cap = &vp->common;
            ಭಙ್ಗ;
        ಪಕ್ಷ VIRTIO_PCI_CAP_NOTIFY_CFG:
            vp_cap = &vp->notify;
            mul = offsetof(ಸಂರಚನಾ virtio_pci_notify_cap, notify_off_multiplier);
            vp->notify_off_multiplier = pci_config_readl(pci->bdf, cap + mul);
            ಭಙ್ಗ;
        ಪಕ್ಷ VIRTIO_PCI_CAP_ISR_CFG:
            vp_cap = &vp->isr;
            ಭಙ್ಗ;
        ಪಕ್ಷ VIRTIO_PCI_CAP_DEVICE_CFG:
            vp_cap = &vp->device;
            ಭಙ್ಗ;
        ಪಕ್ಷ VIRTIO_PCI_CAP_PCI_CFG:
            vp->common.cfg = cap;
            vp->common.bdf = pci->bdf;
            vp->notify.cfg = cap;
            vp->notify.bdf = pci->bdf;
            vp->isr.cfg = cap;
            vp->isr.bdf = pci->bdf;
            vp->device.cfg = cap;
            vp->device.bdf = pci->bdf;
            vp_cap = ರಿಕ್ತ;
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d"
                    " [pci cfg access]\n", pci, cap, type);
            ಭಙ್ಗ;
        ಸ್ವಭಾವ:
            vp_cap = ರಿಕ್ತ;
            ಭಙ್ಗ;
        }
        ಯದಿ (vp_cap && !vp_cap->cap) {
            vp_cap->cap = cap;
            vp_cap->bar = pci_config_readb(pci->bdf, cap +
                                           offsetof(ಸಂರಚನಾ virtio_pci_cap, bar));
            offset = pci_config_readl(pci->bdf, cap +
                                      offsetof(ಸಂರಚನಾ virtio_pci_cap, offset));
            base = PCI_BASE_ADDRESS_0 + 4 * vp_cap->bar;
            addr = pci_config_readl(pci->bdf, base);
            ಯದಿ (addr & PCI_BASE_ADDRESS_SPACE_IO) {
                addr &= PCI_BASE_ADDRESS_IO_MASK;
                vp_cap->mode = VP_ACCESS_IO;
            } ಅನ್ಯಥಾ ಯದಿ ((addr & PCI_BASE_ADDRESS_MEM_TYPE_MASK) ==
                       PCI_BASE_ADDRESS_MEM_TYPE_64) {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                addr |= (u64)pci_config_readl(pci->bdf, base + 4) << 32;
                vp_cap->mode = (addr > 0xffffffffll) ?
                    VP_ACCESS_PCICFG : VP_ACCESS_MMIO;
            } ಅನ್ಯಥಾ {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                vp_cap->mode = VP_ACCESS_MMIO;
            }
            ವಿಕಲ್ಪ (vp_cap->mode) {
            ಪಕ್ಷ VP_ACCESS_IO:
            {
                u32 addr = pci_enable_iobar(pci, base);
                ಯದಿ (!addr)
                    ಪ್ರತ್ಯಾಗಮ;
                vp_cap->ioaddr = addr + offset;
                mode = "io";
                ಭಙ್ಗ;
            }
            ಪಕ್ಷ VP_ACCESS_MMIO:
            {
                ಶೂನ್ಯ *addr = pci_enable_membar(pci, base);
                ಯದಿ (!addr)
                    ಪ್ರತ್ಯಾಗಮ;
                vp_cap->memaddr = addr + offset;
                mode = "mmio";
                ಭಙ್ಗ;
            }
            ಪಕ್ಷ VP_ACCESS_PCICFG:
                mode = "pcicfg";
                vp_cap->baroff = offset;
                ಭಙ್ಗ;
            ಸ್ವಭಾವ:
                mode = "Huh?";
                ಭಙ್ಗ;
            }
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d "
                    "bar %d at 0x%08llx off +0x%04x [%s]\n",
                    pci, vp_cap->cap, type, vp_cap->bar, addr, offset, mode);
        }

        cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, cap);
    }

    ಯದಿ (vp->common.cap && vp->notify.cap && vp->isr.cap && vp->device.cap) {
        dprintf(1, "pci dev %pP using modern (1.0) virtio mode\n", pci);
        vp->use_modern = 1;
    } ಅನ್ಯಥಾ {
        dprintf(1, "pci dev %pP using legacy (0.9.5) virtio mode\n", pci);
        vp->legacy.bar = 0;
        vp->legacy.ioaddr = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        ಯದಿ (!vp->legacy.ioaddr)
            ಪ್ರತ್ಯಾಗಮ;
        vp->legacy.mode = VP_ACCESS_IO;
    }

    vp_reset(vp);
    pci_enable_busmaster(pci);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER );
}

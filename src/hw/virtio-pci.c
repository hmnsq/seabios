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

#ਅਨ੍ਤਰ੍ਭਾਵ "config.h" // CONFIG_DEBUG_LEVEL
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // free
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pci.h" // pci_config_readl
#ਅਨ੍ਤਰ੍ਭਾਵ "pcidevice.h" // struct pci_device
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "virtio-pci.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "virtio-mmio.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "virtio-ring.h"

u64 _vp_read(ਬਣਤਰ vp_cap *cap, u32 ਵਿੱਥ_2, u8 ਮਾਪ)
{
    u64 var = 0;

    ਚੋਣ (cap->ਬਹੁਲਕ) {
    ਹਾਲਤ VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + ਵਿੱਥ_2;
        ਚੋਣ (ਮਾਪ) {
        ਹਾਲਤ 8:
            var = inl(addr);
            var |= (u64)inl(addr+4) << 32;
            ਰੋਕੋ;
        ਹਾਲਤ 4:
            var = inl(addr);
            ਰੋਕੋ;
        ਹਾਲਤ 2:
            var = inw(addr);
            ਰੋਕੋ;
        ਹਾਲਤ 1:
            var = inb(addr);
            ਰੋਕੋ;
        }
        ਰੋਕੋ;
    }

    ਹਾਲਤ VP_ACCESS_MMIO:
    {
        ਖਾਲੀ *addr = cap->memaddr + ਵਿੱਥ_2;
        ਚੋਣ (ਮਾਪ) {
        ਹਾਲਤ 8:
            var = readl(addr);
            var |= (u64)readl(addr+4) << 32;
            ਰੋਕੋ;
        ਹਾਲਤ 4:
            var = readl(addr);
            ਰੋਕੋ;
        ਹਾਲਤ 2:
            var = readw(addr);
            ਰੋਕੋ;
        ਹਾਲਤ 1:
            var = readb(addr);
            ਰੋਕੋ;
        }
        ਰੋਕੋ;
    }

    ਹਾਲਤ VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + ਵਿੱਥ_2;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(ਬਣਤਰ virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ਬਣਤਰ virtio_pci_cfg_cap, cap.ਵਿੱਥ_2),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ਬਣਤਰ virtio_pci_cfg_cap, cap.ਲੰਮਾਈ),
                          (ਮਾਪ > 4) ? 4 : ਮਾਪ);
        ਚੋਣ (ਮਾਪ) {
        ਹਾਲਤ 8:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(ਬਣਤਰ virtio_pci_cfg_cap, pci_cfg_ਅੰਕੜੇ));
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(ਬਣਤਰ virtio_pci_cfg_cap, cap.ਵਿੱਥ_2),
                              addr + 4);
            var |= (u64)pci_config_readl(cap->bdf, cap->cfg +
                                         offsetof(ਬਣਤਰ virtio_pci_cfg_cap, pci_cfg_ਅੰਕੜੇ)) << 32;
            ਰੋਕੋ;
        ਹਾਲਤ 4:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(ਬਣਤਰ virtio_pci_cfg_cap, pci_cfg_ਅੰਕੜੇ));
            ਰੋਕੋ;
        ਹਾਲਤ 2:
            var = pci_config_readw(cap->bdf, cap->cfg +
                                   offsetof(ਬਣਤਰ virtio_pci_cfg_cap, pci_cfg_ਅੰਕੜੇ));
            ਰੋਕੋ;
        ਹਾਲਤ 1:
            var = pci_config_readb(cap->bdf, cap->cfg +
                                   offsetof(ਬਣਤਰ virtio_pci_cfg_cap, pci_cfg_ਅੰਕੜੇ));
            ਰੋਕੋ;
        }
    }
    }
    dprintf(9, "vp read   %x (%d) -> 0x%llx\n", cap->ioaddr + ਵਿੱਥ_2, ਮਾਪ, var);
    ਵਾਪਸ var;
}

ਖਾਲੀ _vp_write(ਬਣਤਰ vp_cap *cap, u32 ਵਿੱਥ_2, u8 ਮਾਪ, u64 var)
{
    dprintf(9, "vp write  %x (%d) <- 0x%llx\n", cap->ioaddr + ਵਿੱਥ_2, ਮਾਪ, var);

    ਚੋਣ (cap->ਬਹੁਲਕ) {
    ਹਾਲਤ VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + ਵਿੱਥ_2;
        ਚੋਣ (ਮਾਪ) {
        ਹਾਲਤ 4:
            outl(var, addr);
            ਰੋਕੋ;
        ਹਾਲਤ 2:
            outw(var, addr);
            ਰੋਕੋ;
        ਹਾਲਤ 1:
            outb(var, addr);
            ਰੋਕੋ;
        }
        ਰੋਕੋ;
    }

    ਹਾਲਤ VP_ACCESS_MMIO:
    {
        ਖਾਲੀ *addr = cap->memaddr + ਵਿੱਥ_2;
        ਚੋਣ (ਮਾਪ) {
        ਹਾਲਤ 4:
            writel(addr, var);
            ਰੋਕੋ;
        ਹਾਲਤ 2:
            writew(addr, var);
            ਰੋਕੋ;
        ਹਾਲਤ 1:
            writeb(addr, var);
            ਰੋਕੋ;
        }
        ਰੋਕੋ;
    }

    ਹਾਲਤ VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + ਵਿੱਥ_2;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(ਬਣਤਰ virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ਬਣਤਰ virtio_pci_cfg_cap, cap.ਵਿੱਥ_2),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ਬਣਤਰ virtio_pci_cfg_cap, cap.ਲੰਮਾਈ),
                          ਮਾਪ);
        ਚੋਣ (ਮਾਪ) {
        ਹਾਲਤ 4:
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(ਬਣਤਰ virtio_pci_cfg_cap, pci_cfg_ਅੰਕੜੇ),
                              var);
            ਰੋਕੋ;
        ਹਾਲਤ 2:
            pci_config_writew(cap->bdf, cap->cfg +
                              offsetof(ਬਣਤਰ virtio_pci_cfg_cap, pci_cfg_ਅੰਕੜੇ),
                              var);
            ਰੋਕੋ;
        ਹਾਲਤ 1:
            pci_config_writeb(cap->bdf, cap->cfg +
                              offsetof(ਬਣਤਰ virtio_pci_cfg_cap, pci_cfg_ਅੰਕੜੇ),
                              var);
            ਰੋਕੋ;
        }
    }
    }
}

u64 vp_get_features(ਬਣਤਰ vp_device *vp)
{
    u32 f0, f1;

    ਜੇ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
    } ਨਹੀਂਤਾਂ ਜੇ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
    } ਨਹੀਂਤਾਂ {
        f0 = vp_read(&vp->legacy, virtio_pci_legacy, host_features);
        f1 = 0;
    }
    ਵਾਪਸ ((u64)f1 << 32) | f0;
}

ਖਾਲੀ vp_ਸੈੱਟ_features(ਬਣਤਰ vp_device *vp, u64 features)
{
    u32 f0, f1;

    f0 = features;
    f1 = features >> 32;

    ਜੇ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f1);
    } ਨਹੀਂਤਾਂ ਜੇ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f1);
    } ਨਹੀਂਤਾਂ {
        vp_write(&vp->legacy, virtio_pci_legacy, guest_features, f0);
    }
}

u8 vp_get_ਸਥਿਤੀ(ਬਣਤਰ vp_device *vp)
{
    ਜੇ (vp->use_mmio) {
        ਵਾਪਸ vp_read(&vp->common, virtio_mmio_cfg, device_ਸਥਿਤੀ);
    } ਨਹੀਂਤਾਂ ਜੇ (vp->use_modern) {
        ਵਾਪਸ vp_read(&vp->common, virtio_pci_common_cfg, device_ਸਥਿਤੀ);
    } ਨਹੀਂਤਾਂ {
        ਵਾਪਸ vp_read(&vp->legacy, virtio_pci_legacy, ਸਥਿਤੀ);
    }
}

ਖਾਲੀ vp_ਸੈੱਟ_ਸਥਿਤੀ(ਬਣਤਰ vp_device *vp, u8 ਸਥਿਤੀ)
{
    ਜੇ (ਸਥਿਤੀ == 0)        /* reset */
        ਵਾਪਸ;
    ਜੇ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_ਸਥਿਤੀ, ਸਥਿਤੀ);
    } ਨਹੀਂਤਾਂ ਜੇ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_ਸਥਿਤੀ, ਸਥਿਤੀ);
    } ਨਹੀਂਤਾਂ {
        vp_write(&vp->legacy, virtio_pci_legacy, ਸਥਿਤੀ, ਸਥਿਤੀ);
    }
}

u8 vp_get_isr(ਬਣਤਰ vp_device *vp)
{
    ਜੇ (vp->use_mmio) {
        ਵਾਪਸ vp_read(&vp->common, virtio_mmio_cfg, irq_ਸਥਿਤੀ);
    } ਨਹੀਂਤਾਂ ਜੇ (vp->use_modern) {
        ਵਾਪਸ vp_read(&vp->isr, virtio_pci_isr, isr);
    } ਨਹੀਂਤਾਂ {
        ਵਾਪਸ vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

ਖਾਲੀ vp_reset(ਬਣਤਰ vp_device *vp)
{
    ਜੇ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_ਸਥਿਤੀ, 0);
        vp_read(&vp->common, virtio_mmio_cfg, irq_ਸਥਿਤੀ);
    } ਨਹੀਂਤਾਂ ਜੇ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_ਸਥਿਤੀ, 0);
        vp_read(&vp->isr, virtio_pci_isr, isr);
    } ਨਹੀਂਤਾਂ {
        vp_write(&vp->legacy, virtio_pci_legacy, ਸਥਿਤੀ, 0);
        vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

ਖਾਲੀ vp_notify(ਬਣਤਰ vp_device *vp, ਬਣਤਰ vring_virtqueue *vq)
{
    ਜੇ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, ਕਤਾਰ_notify, vq->ਕਤਾਰ_ਸੂਚਕਾਂਕ);
    } ਨਹੀਂਤਾਂ ਜੇ (vp->use_modern) {
        u32 ਵਿੱਥ_2 = vq->ਕਤਾਰ_notify_off * vp->notify_off_multiplier;
        ਚੋਣ (vp->notify.ਬਹੁਲਕ) {
        ਹਾਲਤ VP_ACCESS_IO:
            outw(vq->ਕਤਾਰ_ਸੂਚਕਾਂਕ, vp->notify.ioaddr + ਵਿੱਥ_2);
            ਰੋਕੋ;
        ਹਾਲਤ VP_ACCESS_MMIO:
            writew(vp->notify.memaddr + ਵਿੱਥ_2, vq->ਕਤਾਰ_ਸੂਚਕਾਂਕ);
            ਰੋਕੋ;
        ਹਾਲਤ VP_ACCESS_PCICFG:
            pci_config_writeb(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ਬਣਤਰ virtio_pci_cfg_cap, cap.bar),
                              vp->notify.bar);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ਬਣਤਰ virtio_pci_cfg_cap, cap.ਵਿੱਥ_2),
                              vp->notify.baroff + ਵਿੱਥ_2);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ਬਣਤਰ virtio_pci_cfg_cap, cap.ਲੰਮਾਈ),
                              2);
            pci_config_writew(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ਬਣਤਰ virtio_pci_cfg_cap, pci_cfg_ਅੰਕੜੇ),
                              vq->ਕਤਾਰ_ਸੂਚਕਾਂਕ);
        }
        dprintf(9, "vp notify %x (%d) -- 0x%x\n",
                vp->notify.ioaddr, 2, vq->ਕਤਾਰ_ਸੂਚਕਾਂਕ);
    } ਨਹੀਂਤਾਂ {
        vp_write(&vp->legacy, virtio_pci_legacy, ਕਤਾਰ_notify, vq->ਕਤਾਰ_ਸੂਚਕਾਂਕ);
    }
}

ਪੂਰਨ vp_ਲੱਭੋ_vq(ਬਣਤਰ vp_device *vp, ਪੂਰਨ ਕਤਾਰ_ਸੂਚਕਾਂਕ,
               ਬਣਤਰ vring_virtqueue **p_vq)
{
   u16 ਅੰਕ;

   ASSERT32FLAT();
   ਬਣਤਰ vring_virtqueue *vq = *p_vq = memalign_ਉੱਚਾ(ਪੰਨਾ_ਮਾਪ, ਆਕਾਰ(*vq));
   ਜੇ (!vq) {
       warn_noalloc();
       ਜਾਓ fail;
   }
   memset(vq, 0, ਆਕਾਰ(*vq));


   /* select the queue */
   ਜੇ (vp->use_mmio) {
       vp_write(&vp->common, virtio_mmio_cfg, ਕਤਾਰ_select, ਕਤਾਰ_ਸੂਚਕਾਂਕ);
   } ਨਹੀਂਤਾਂ ਜੇ (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, ਕਤਾਰ_select, ਕਤਾਰ_ਸੂਚਕਾਂਕ);
   } ਨਹੀਂਤਾਂ {
       vp_write(&vp->legacy, virtio_pci_legacy, ਕਤਾਰ_sel, ਕਤਾਰ_ਸੂਚਕਾਂਕ);
   }

   /* check if the queue is available */
   ਜੇ (vp->use_mmio) {
       ਅੰਕ = vp_read(&vp->common, virtio_mmio_cfg, ਕਤਾਰ_ਅੰਕ_max);
       ਜੇ (ਅੰਕ > MAX_ਕਤਾਰ_ਅੰਕ)
           ਅੰਕ = MAX_ਕਤਾਰ_ਅੰਕ;
       vp_write(&vp->common, virtio_mmio_cfg, ਕਤਾਰ_ਅੰਕ, ਅੰਕ);
   } ਨਹੀਂਤਾਂ ਜੇ (vp->use_modern) {
       ਅੰਕ = vp_read(&vp->common, virtio_pci_common_cfg, ਕਤਾਰ_ਮਾਪ);
       ਜੇ (ਅੰਕ > MAX_ਕਤਾਰ_ਅੰਕ) {
           vp_write(&vp->common, virtio_pci_common_cfg, ਕਤਾਰ_ਮਾਪ,
                    MAX_ਕਤਾਰ_ਅੰਕ);
           ਅੰਕ = vp_read(&vp->common, virtio_pci_common_cfg, ਕਤਾਰ_ਮਾਪ);
       }
   } ਨਹੀਂਤਾਂ {
       ਅੰਕ = vp_read(&vp->legacy, virtio_pci_legacy, ਕਤਾਰ_ਅੰਕ);
   }
   ਜੇ (!ਅੰਕ) {
       dprintf(1, "ERROR: queue size is 0\n");
       ਜਾਓ fail;
   }
   ਜੇ (ਅੰਕ > MAX_ਕਤਾਰ_ਅੰਕ) {
       dprintf(1, "ERROR: queue size %d > %d\n", ਅੰਕ, MAX_ਕਤਾਰ_ਅੰਕ);
       ਜਾਓ fail;
   }

   /* check if the queue is already active */
   ਜੇ (vp->use_mmio) {
       /* TODO */;
   } ਨਹੀਂਤਾਂ ਜੇ (vp->use_modern) {
       ਜੇ (vp_read(&vp->common, virtio_pci_common_cfg, ਕਤਾਰ_enable)) {
           dprintf(1, "ERROR: queue already active\n");
           ਜਾਓ fail;
       }
   } ਨਹੀਂਤਾਂ {
       ਜੇ (vp_read(&vp->legacy, virtio_pci_legacy, ਕਤਾਰ_pfn)) {
           dprintf(1, "ERROR: queue already active\n");
           ਜਾਓ fail;
       }
   }
   vq->ਕਤਾਰ_ਸੂਚਕਾਂਕ = ਕਤਾਰ_ਸੂਚਕਾਂਕ;

   /* initialize the queue */
   ਬਣਤਰ vring * vr = &vq->vring;
   vring_init(vr, ਅੰਕ, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)&vq->ਕਤਾਰ);

   /* activate the queue
    *
    * NOTE: vr->desc is initialized by vring_init()
    */

   ਜੇ (vp->use_mmio) {
       ਜੇ (vp_read(&vp->common, virtio_mmio_cfg, ਸੰਸਕਰਣ_2) == 2) {
           vp_write(&vp->common, virtio_mmio_cfg, ਕਤਾਰ_desc_lo,
                    (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)virt_ਮੰਜ਼ਿਲ_phys(vr->desc));
           vp_write(&vp->common, virtio_mmio_cfg, ਕਤਾਰ_desc_hi, 0);
           vp_write(&vp->common, virtio_mmio_cfg, ਕਤਾਰ_driver_lo,
                    (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)virt_ਮੰਜ਼ਿਲ_phys(vr->avail));
           vp_write(&vp->common, virtio_mmio_cfg, ਕਤਾਰ_driver_hi, 0);
           vp_write(&vp->common, virtio_mmio_cfg, ਕਤਾਰ_device_lo,
                    (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)virt_ਮੰਜ਼ਿਲ_phys(vr->ਵਰਤਿਆ));
           vp_write(&vp->common, virtio_mmio_cfg, ਕਤਾਰ_device_hi, 0);
           vp_write(&vp->common, virtio_mmio_cfg, ਕਤਾਰ_ready, 1);
       } ਨਹੀਂਤਾਂ {
           vp_write(&vp->common, virtio_mmio_cfg, legacy_guest_ਪੰਨਾ_ਮਾਪ,
                    (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)1 << ਪੰਨਾ_ਖਿਸਕਾਓ);
           vp_write(&vp->common, virtio_mmio_cfg, legacy_ਕਤਾਰ_pfn,
                    (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)virt_ਮੰਜ਼ਿਲ_phys(vr->desc) >> ਪੰਨਾ_ਖਿਸਕਾਓ);
       }
   } ਨਹੀਂਤਾਂ ਜੇ (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, ਕਤਾਰ_desc_lo,
                (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)virt_ਮੰਜ਼ਿਲ_phys(vr->desc));
       vp_write(&vp->common, virtio_pci_common_cfg, ਕਤਾਰ_desc_hi, 0);
       vp_write(&vp->common, virtio_pci_common_cfg, ਕਤਾਰ_avail_lo,
                (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)virt_ਮੰਜ਼ਿਲ_phys(vr->avail));
       vp_write(&vp->common, virtio_pci_common_cfg, ਕਤਾਰ_avail_hi, 0);
       vp_write(&vp->common, virtio_pci_common_cfg, ਕਤਾਰ_ਵਰਤਿਆ_lo,
                (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)virt_ਮੰਜ਼ਿਲ_phys(vr->ਵਰਤਿਆ));
       vp_write(&vp->common, virtio_pci_common_cfg, ਕਤਾਰ_ਵਰਤਿਆ_hi, 0);
       vp_write(&vp->common, virtio_pci_common_cfg, ਕਤਾਰ_enable, 1);
       vq->ਕਤਾਰ_notify_off = vp_read(&vp->common, virtio_pci_common_cfg,
                                      ਕਤਾਰ_notify_off);
   } ਨਹੀਂਤਾਂ {
       vp_write(&vp->legacy, virtio_pci_legacy, ਕਤਾਰ_pfn,
                (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)virt_ਮੰਜ਼ਿਲ_phys(vr->desc) >> ਪੰਨਾ_ਖਿਸਕਾਓ);
   }
   ਵਾਪਸ ਅੰਕ;

fail:
   ਮੁਕਤ(vq);
   *p_vq = ਨਿਰਾ;
   ਵਾਪਸ -1;
}

ਖਾਲੀ vp_init_simple(ਬਣਤਰ vp_device *vp, ਬਣਤਰ pci_device *pci)
{
    u8 cap = pci_ਲੱਭੋ_capability(pci->bdf, PCI_CAP_ID_VNDR, 0);
    ਬਣਤਰ vp_cap *vp_cap;
    ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ;
    u32 ਵਿੱਥ_2, ਅਧਾਰ, mul;
    u64 addr;
    u8 type;

    memset(vp, 0, ਆਕਾਰ(*vp));
    ਜਦੋਂ (cap != 0) {
        type = pci_config_readb(pci->bdf, cap +
                                offsetof(ਬਣਤਰ virtio_pci_cap, cfg_type));
        ਚੋਣ (type) {
        ਹਾਲਤ VIRTIO_PCI_CAP_COMMON_CFG:
            vp_cap = &vp->common;
            ਰੋਕੋ;
        ਹਾਲਤ VIRTIO_PCI_CAP_NOTIFY_CFG:
            vp_cap = &vp->notify;
            mul = offsetof(ਬਣਤਰ virtio_pci_notify_cap, notify_off_multiplier);
            vp->notify_off_multiplier = pci_config_readl(pci->bdf, cap + mul);
            ਰੋਕੋ;
        ਹਾਲਤ VIRTIO_PCI_CAP_ISR_CFG:
            vp_cap = &vp->isr;
            ਰੋਕੋ;
        ਹਾਲਤ VIRTIO_PCI_CAP_DEVICE_CFG:
            vp_cap = &vp->device;
            ਰੋਕੋ;
        ਹਾਲਤ VIRTIO_PCI_CAP_PCI_CFG:
            vp->common.cfg = cap;
            vp->common.bdf = pci->bdf;
            vp->notify.cfg = cap;
            vp->notify.bdf = pci->bdf;
            vp->isr.cfg = cap;
            vp->isr.bdf = pci->bdf;
            vp->device.cfg = cap;
            vp->device.bdf = pci->bdf;
            vp_cap = ਨਿਰਾ;
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d"
                    " [pci cfg access]\n", pci, cap, type);
            ਰੋਕੋ;
        ਮੂਲ:
            vp_cap = ਨਿਰਾ;
            ਰੋਕੋ;
        }
        ਜੇ (vp_cap && !vp_cap->cap) {
            vp_cap->cap = cap;
            vp_cap->bar = pci_config_readb(pci->bdf, cap +
                                           offsetof(ਬਣਤਰ virtio_pci_cap, bar));
            ਵਿੱਥ_2 = pci_config_readl(pci->bdf, cap +
                                      offsetof(ਬਣਤਰ virtio_pci_cap, ਵਿੱਥ_2));
            ਅਧਾਰ = PCI_ਅਧਾਰ_ਪਤਾ_0 + 4 * vp_cap->bar;
            addr = pci_config_readl(pci->bdf, ਅਧਾਰ);
            ਜੇ (addr & PCI_ਅਧਾਰ_ਪਤਾ_SPACE_IO) {
                addr &= PCI_ਅਧਾਰ_ਪਤਾ_IO_ਨਕਾਬ;
                vp_cap->ਬਹੁਲਕ = VP_ACCESS_IO;
            } ਨਹੀਂਤਾਂ ਜੇ ((addr & PCI_ਅਧਾਰ_ਪਤਾ_MEM_TYPE_ਨਕਾਬ) ==
                       PCI_ਅਧਾਰ_ਪਤਾ_MEM_TYPE_64) {
                addr &= PCI_ਅਧਾਰ_ਪਤਾ_MEM_ਨਕਾਬ;
                addr |= (u64)pci_config_readl(pci->bdf, ਅਧਾਰ + 4) << 32;
                vp_cap->ਬਹੁਲਕ = (addr > 0xffffffffll) ?
                    VP_ACCESS_PCICFG : VP_ACCESS_MMIO;
            } ਨਹੀਂਤਾਂ {
                addr &= PCI_ਅਧਾਰ_ਪਤਾ_MEM_ਨਕਾਬ;
                vp_cap->ਬਹੁਲਕ = VP_ACCESS_MMIO;
            }
            ਚੋਣ (vp_cap->ਬਹੁਲਕ) {
            ਹਾਲਤ VP_ACCESS_IO:
            {
                u32 addr = pci_enable_iobar(pci, ਅਧਾਰ);
                ਜੇ (!addr)
                    ਵਾਪਸ;
                vp_cap->ioaddr = addr + ਵਿੱਥ_2;
                ਬਹੁਲਕ = "io";
                ਰੋਕੋ;
            }
            ਹਾਲਤ VP_ACCESS_MMIO:
            {
                ਖਾਲੀ *addr = pci_enable_membar(pci, ਅਧਾਰ);
                ਜੇ (!addr)
                    ਵਾਪਸ;
                vp_cap->memaddr = addr + ਵਿੱਥ_2;
                ਬਹੁਲਕ = "mmio";
                ਰੋਕੋ;
            }
            ਹਾਲਤ VP_ACCESS_PCICFG:
                ਬਹੁਲਕ = "pcicfg";
                vp_cap->baroff = ਵਿੱਥ_2;
                ਰੋਕੋ;
            ਮੂਲ:
                ਬਹੁਲਕ = "Huh?";
                ਰੋਕੋ;
            }
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d "
                    "bar %d at 0x%08llx off +0x%04x [%s]\n",
                    pci, vp_cap->cap, type, vp_cap->bar, addr, ਵਿੱਥ_2, ਬਹੁਲਕ);
        }

        cap = pci_ਲੱਭੋ_capability(pci->bdf, PCI_CAP_ID_VNDR, cap);
    }

    ਜੇ (vp->common.cap && vp->notify.cap && vp->isr.cap && vp->device.cap) {
        dprintf(1, "pci dev %pP using modern (1.0) virtio mode\n", pci);
        vp->use_modern = 1;
    } ਨਹੀਂਤਾਂ {
        dprintf(1, "pci dev %pP using legacy (0.9.5) virtio mode\n", pci);
        vp->legacy.bar = 0;
        vp->legacy.ioaddr = pci_enable_iobar(pci, PCI_ਅਧਾਰ_ਪਤਾ_0);
        ਜੇ (!vp->legacy.ioaddr)
            ਵਾਪਸ;
        vp->legacy.ਬਹੁਲਕ = VP_ACCESS_IO;
    }

    vp_reset(vp);
    pci_enable_busmaster(pci);
    vp_ਸੈੱਟ_ਸਥਿਤੀ(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER );
}

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

u64 _vp_read(ರಚನೆ vp_cap *cap, u32 ಸ್ಥಾನಾಂತರ_2, u8 ಗಾತ್ರ_2)
{
    u64 var = 0;

    ಆಯ್ಕೆ (cap->ಬಹುಲಕ) {
    ಸ್ಥಿತಿ VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + ಸ್ಥಾನಾಂತರ_2;
        ಆಯ್ಕೆ (ಗಾತ್ರ_2) {
        ಸ್ಥಿತಿ 8:
            var = inl(addr);
            var |= (u64)inl(addr+4) << 32;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 4:
            var = inl(addr);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 2:
            var = inw(addr);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 1:
            var = inb(addr);
            ನಿಲ್ಲಿಸು;
        }
        ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ VP_ACCESS_MMIO:
    {
        ಶೂನ್ಯ *addr = cap->memaddr + ಸ್ಥಾನಾಂತರ_2;
        ಆಯ್ಕೆ (ಗಾತ್ರ_2) {
        ಸ್ಥಿತಿ 8:
            var = readl(addr);
            var |= (u64)readl(addr+4) << 32;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 4:
            var = readl(addr);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 2:
            var = readw(addr);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 1:
            var = readb(addr);
            ನಿಲ್ಲಿಸು;
        }
        ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + ಸ್ಥಾನಾಂತರ_2;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(ರಚನೆ virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ರಚನೆ virtio_pci_cfg_cap, cap.ಸ್ಥಾನಾಂತರ_2),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ರಚನೆ virtio_pci_cfg_cap, cap.ಉದ್ದಳತೆ),
                          (ಗಾತ್ರ_2 > 4) ? 4 : ಗಾತ್ರ_2);
        ಆಯ್ಕೆ (ಗಾತ್ರ_2) {
        ಸ್ಥಿತಿ 8:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(ರಚನೆ virtio_pci_cfg_cap, pci_cfg_ದತ್ತಾಂಶ));
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(ರಚನೆ virtio_pci_cfg_cap, cap.ಸ್ಥಾನಾಂತರ_2),
                              addr + 4);
            var |= (u64)pci_config_readl(cap->bdf, cap->cfg +
                                         offsetof(ರಚನೆ virtio_pci_cfg_cap, pci_cfg_ದತ್ತಾಂಶ)) << 32;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 4:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(ರಚನೆ virtio_pci_cfg_cap, pci_cfg_ದತ್ತಾಂಶ));
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 2:
            var = pci_config_readw(cap->bdf, cap->cfg +
                                   offsetof(ರಚನೆ virtio_pci_cfg_cap, pci_cfg_ದತ್ತಾಂಶ));
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 1:
            var = pci_config_readb(cap->bdf, cap->cfg +
                                   offsetof(ರಚನೆ virtio_pci_cfg_cap, pci_cfg_ದತ್ತಾಂಶ));
            ನಿಲ್ಲಿಸು;
        }
    }
    }
    dprintf(9, "vp read   %x (%d) -> 0x%llx\n", cap->ioaddr + ಸ್ಥಾನಾಂತರ_2, ಗಾತ್ರ_2, var);
    ಹಿಂತಿರುಗಿ var;
}

ಶೂನ್ಯ _vp_write(ರಚನೆ vp_cap *cap, u32 ಸ್ಥಾನಾಂತರ_2, u8 ಗಾತ್ರ_2, u64 var)
{
    dprintf(9, "vp write  %x (%d) <- 0x%llx\n", cap->ioaddr + ಸ್ಥಾನಾಂತರ_2, ಗಾತ್ರ_2, var);

    ಆಯ್ಕೆ (cap->ಬಹುಲಕ) {
    ಸ್ಥಿತಿ VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + ಸ್ಥಾನಾಂತರ_2;
        ಆಯ್ಕೆ (ಗಾತ್ರ_2) {
        ಸ್ಥಿತಿ 4:
            outl(var, addr);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 2:
            outw(var, addr);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 1:
            outb(var, addr);
            ನಿಲ್ಲಿಸು;
        }
        ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ VP_ACCESS_MMIO:
    {
        ಶೂನ್ಯ *addr = cap->memaddr + ಸ್ಥಾನಾಂತರ_2;
        ಆಯ್ಕೆ (ಗಾತ್ರ_2) {
        ಸ್ಥಿತಿ 4:
            writel(addr, var);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 2:
            writew(addr, var);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 1:
            writeb(addr, var);
            ನಿಲ್ಲಿಸು;
        }
        ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + ಸ್ಥಾನಾಂತರ_2;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(ರಚನೆ virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ರಚನೆ virtio_pci_cfg_cap, cap.ಸ್ಥಾನಾಂತರ_2),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ರಚನೆ virtio_pci_cfg_cap, cap.ಉದ್ದಳತೆ),
                          ಗಾತ್ರ_2);
        ಆಯ್ಕೆ (ಗಾತ್ರ_2) {
        ಸ್ಥಿತಿ 4:
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(ರಚನೆ virtio_pci_cfg_cap, pci_cfg_ದತ್ತಾಂಶ),
                              var);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 2:
            pci_config_writew(cap->bdf, cap->cfg +
                              offsetof(ರಚನೆ virtio_pci_cfg_cap, pci_cfg_ದತ್ತಾಂಶ),
                              var);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 1:
            pci_config_writeb(cap->bdf, cap->cfg +
                              offsetof(ರಚನೆ virtio_pci_cfg_cap, pci_cfg_ದತ್ತಾಂಶ),
                              var);
            ನಿಲ್ಲಿಸು;
        }
    }
    }
}

u64 vp_get_features(ರಚನೆ vp_device *vp)
{
    u32 f0, f1;

    ಆದರೆ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
    } ಇಲ್ಲದಿದ್ದರೆ {
        f0 = vp_read(&vp->legacy, virtio_pci_legacy, host_features);
        f1 = 0;
    }
    ಹಿಂತಿರುಗಿ ((u64)f1 << 32) | f0;
}

ಶೂನ್ಯ vp_ಗಣ_features(ರಚನೆ vp_device *vp, u64 features)
{
    u32 f0, f1;

    f0 = features;
    f1 = features >> 32;

    ಆದರೆ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f1);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f1);
    } ಇಲ್ಲದಿದ್ದರೆ {
        vp_write(&vp->legacy, virtio_pci_legacy, guest_features, f0);
    }
}

u8 vp_get_ಸ್ಥಿತಿ(ರಚನೆ vp_device *vp)
{
    ಆದರೆ (vp->use_mmio) {
        ಹಿಂತಿರುಗಿ vp_read(&vp->common, virtio_mmio_cfg, device_ಸ್ಥಿತಿ);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (vp->use_modern) {
        ಹಿಂತಿರುಗಿ vp_read(&vp->common, virtio_pci_common_cfg, device_ಸ್ಥಿತಿ);
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಹಿಂತಿರುಗಿ vp_read(&vp->legacy, virtio_pci_legacy, ಸ್ಥಿತಿ_2);
    }
}

ಶೂನ್ಯ vp_ಗಣ_ಸ್ಥಿತಿ(ರಚನೆ vp_device *vp, u8 ಸ್ಥಿತಿ_2)
{
    ಆದರೆ (ಸ್ಥಿತಿ_2 == 0)        /* reset */
        ಹಿಂತಿರುಗಿ;
    ಆದರೆ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_ಸ್ಥಿತಿ, ಸ್ಥಿತಿ_2);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_ಸ್ಥಿತಿ, ಸ್ಥಿತಿ_2);
    } ಇಲ್ಲದಿದ್ದರೆ {
        vp_write(&vp->legacy, virtio_pci_legacy, ಸ್ಥಿತಿ_2, ಸ್ಥಿತಿ_2);
    }
}

u8 vp_get_isr(ರಚನೆ vp_device *vp)
{
    ಆದರೆ (vp->use_mmio) {
        ಹಿಂತಿರುಗಿ vp_read(&vp->common, virtio_mmio_cfg, irq_ಸ್ಥಿತಿ);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (vp->use_modern) {
        ಹಿಂತಿರುಗಿ vp_read(&vp->isr, virtio_pci_isr, isr);
    } ಇಲ್ಲದಿದ್ದರೆ {
        ಹಿಂತಿರುಗಿ vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

ಶೂನ್ಯ vp_reset(ರಚನೆ vp_device *vp)
{
    ಆದರೆ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_ಸ್ಥಿತಿ, 0);
        vp_read(&vp->common, virtio_mmio_cfg, irq_ಸ್ಥಿತಿ);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_ಸ್ಥಿತಿ, 0);
        vp_read(&vp->isr, virtio_pci_isr, isr);
    } ಇಲ್ಲದಿದ್ದರೆ {
        vp_write(&vp->legacy, virtio_pci_legacy, ಸ್ಥಿತಿ_2, 0);
        vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

ಶೂನ್ಯ vp_notify(ರಚನೆ vp_device *vp, ರಚನೆ vring_virtqueue *vq)
{
    ಆದರೆ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, ಸರತಿ_notify, vq->ಸರತಿ_ಸೂಚ್ಯಂಕ);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (vp->use_modern) {
        u32 ಸ್ಥಾನಾಂತರ_2 = vq->ಸರತಿ_notify_off * vp->notify_off_multiplier;
        ಆಯ್ಕೆ (vp->notify.ಬಹುಲಕ) {
        ಸ್ಥಿತಿ VP_ACCESS_IO:
            outw(vq->ಸರತಿ_ಸೂಚ್ಯಂಕ, vp->notify.ioaddr + ಸ್ಥಾನಾಂತರ_2);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ VP_ACCESS_MMIO:
            writew(vp->notify.memaddr + ಸ್ಥಾನಾಂತರ_2, vq->ಸರತಿ_ಸೂಚ್ಯಂಕ);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ VP_ACCESS_PCICFG:
            pci_config_writeb(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ರಚನೆ virtio_pci_cfg_cap, cap.bar),
                              vp->notify.bar);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ರಚನೆ virtio_pci_cfg_cap, cap.ಸ್ಥಾನಾಂತರ_2),
                              vp->notify.baroff + ಸ್ಥಾನಾಂತರ_2);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ರಚನೆ virtio_pci_cfg_cap, cap.ಉದ್ದಳತೆ),
                              2);
            pci_config_writew(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ರಚನೆ virtio_pci_cfg_cap, pci_cfg_ದತ್ತಾಂಶ),
                              vq->ಸರತಿ_ಸೂಚ್ಯಂಕ);
        }
        dprintf(9, "vp notify %x (%d) -- 0x%x\n",
                vp->notify.ioaddr, 2, vq->ಸರತಿ_ಸೂಚ್ಯಂಕ);
    } ಇಲ್ಲದಿದ್ದರೆ {
        vp_write(&vp->legacy, virtio_pci_legacy, ಸರತಿ_notify, vq->ಸರತಿ_ಸೂಚ್ಯಂಕ);
    }
}

ಪೂರ್ಣಾಂಕ vp_ಕಂಡುಹಿಡಿ_vq(ರಚನೆ vp_device *vp, ಪೂರ್ಣಾಂಕ ಸರತಿ_ಸೂಚ್ಯಂಕ,
               ರಚನೆ vring_virtqueue **p_vq)
{
   u16 ಅಂಕೆ;

   ASSERT32FLAT();
   ರಚನೆ vring_virtqueue *vq = *p_vq = memalign_ಮೇಲಿನ(ಪುಟ_ಗಾತ್ರ, ಗಾತ್ರ(*vq));
   ಆದರೆ (!vq) {
       warn_noalloc();
       ಹೋಗು fail;
   }
   memset(vq, 0, ಗಾತ್ರ(*vq));


   /* select the queue */
   ಆದರೆ (vp->use_mmio) {
       vp_write(&vp->common, virtio_mmio_cfg, ಸರತಿ_select, ಸರತಿ_ಸೂಚ್ಯಂಕ);
   } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, ಸರತಿ_select, ಸರತಿ_ಸೂಚ್ಯಂಕ);
   } ಇಲ್ಲದಿದ್ದರೆ {
       vp_write(&vp->legacy, virtio_pci_legacy, ಸರತಿ_sel, ಸರತಿ_ಸೂಚ್ಯಂಕ);
   }

   /* check if the queue is available */
   ಆದರೆ (vp->use_mmio) {
       ಅಂಕೆ = vp_read(&vp->common, virtio_mmio_cfg, ಸರತಿ_ಅಂಕೆ_max);
       ಆದರೆ (ಅಂಕೆ > MAX_ಸರತಿ_ಅಂಕೆ)
           ಅಂಕೆ = MAX_ಸರತಿ_ಅಂಕೆ;
       vp_write(&vp->common, virtio_mmio_cfg, ಸರತಿ_ಅಂಕೆ, ಅಂಕೆ);
   } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (vp->use_modern) {
       ಅಂಕೆ = vp_read(&vp->common, virtio_pci_common_cfg, ಸರತಿ_ಗಾತ್ರ);
       ಆದರೆ (ಅಂಕೆ > MAX_ಸರತಿ_ಅಂಕೆ) {
           vp_write(&vp->common, virtio_pci_common_cfg, ಸರತಿ_ಗಾತ್ರ,
                    MAX_ಸರತಿ_ಅಂಕೆ);
           ಅಂಕೆ = vp_read(&vp->common, virtio_pci_common_cfg, ಸರತಿ_ಗಾತ್ರ);
       }
   } ಇಲ್ಲದಿದ್ದರೆ {
       ಅಂಕೆ = vp_read(&vp->legacy, virtio_pci_legacy, ಸರತಿ_ಅಂಕೆ);
   }
   ಆದರೆ (!ಅಂಕೆ) {
       dprintf(1, "ERROR: queue size is 0\n");
       ಹೋಗು fail;
   }
   ಆದರೆ (ಅಂಕೆ > MAX_ಸರತಿ_ಅಂಕೆ) {
       dprintf(1, "ERROR: queue size %d > %d\n", ಅಂಕೆ, MAX_ಸರತಿ_ಅಂಕೆ);
       ಹೋಗು fail;
   }

   /* check if the queue is already active */
   ಆದರೆ (vp->use_mmio) {
       /* TODO */;
   } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (vp->use_modern) {
       ಆದರೆ (vp_read(&vp->common, virtio_pci_common_cfg, ಸರತಿ_enable)) {
           dprintf(1, "ERROR: queue already active\n");
           ಹೋಗು fail;
       }
   } ಇಲ್ಲದಿದ್ದರೆ {
       ಆದರೆ (vp_read(&vp->legacy, virtio_pci_legacy, ಸರತಿ_pfn)) {
           dprintf(1, "ERROR: queue already active\n");
           ಹೋಗು fail;
       }
   }
   vq->ಸರತಿ_ಸೂಚ್ಯಂಕ = ಸರತಿ_ಸೂಚ್ಯಂಕ;

   /* initialize the queue */
   ರಚನೆ vring * vr = &vq->vring;
   vring_init(vr, ಅಂಕೆ, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)&vq->ಸರತಿ);

   /* activate the queue
    *
    * NOTE: vr->desc is initialized by vring_init()
    */

   ಆದರೆ (vp->use_mmio) {
       ಆದರೆ (vp_read(&vp->common, virtio_mmio_cfg, ಆವೃತ್ತಿ_2) == 2) {
           vp_write(&vp->common, virtio_mmio_cfg, ಸರತಿ_desc_lo,
                    (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)virt_ಗಮ್ಯ_phys(vr->desc));
           vp_write(&vp->common, virtio_mmio_cfg, ಸರತಿ_desc_hi, 0);
           vp_write(&vp->common, virtio_mmio_cfg, ಸರತಿ_driver_lo,
                    (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)virt_ಗಮ್ಯ_phys(vr->avail));
           vp_write(&vp->common, virtio_mmio_cfg, ಸರತಿ_driver_hi, 0);
           vp_write(&vp->common, virtio_mmio_cfg, ಸರತಿ_device_lo,
                    (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)virt_ಗಮ್ಯ_phys(vr->ಬಳಸಿದ));
           vp_write(&vp->common, virtio_mmio_cfg, ಸರತಿ_device_hi, 0);
           vp_write(&vp->common, virtio_mmio_cfg, ಸರತಿ_ready, 1);
       } ಇಲ್ಲದಿದ್ದರೆ {
           vp_write(&vp->common, virtio_mmio_cfg, legacy_guest_ಪುಟ_ಗಾತ್ರ,
                    (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)1 << ಪುಟ_ಸರಿಸು);
           vp_write(&vp->common, virtio_mmio_cfg, legacy_ಸರತಿ_pfn,
                    (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)virt_ಗಮ್ಯ_phys(vr->desc) >> ಪುಟ_ಸರಿಸು);
       }
   } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, ಸರತಿ_desc_lo,
                (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)virt_ಗಮ್ಯ_phys(vr->desc));
       vp_write(&vp->common, virtio_pci_common_cfg, ಸರತಿ_desc_hi, 0);
       vp_write(&vp->common, virtio_pci_common_cfg, ಸರತಿ_avail_lo,
                (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)virt_ಗಮ್ಯ_phys(vr->avail));
       vp_write(&vp->common, virtio_pci_common_cfg, ಸರತಿ_avail_hi, 0);
       vp_write(&vp->common, virtio_pci_common_cfg, ಸರತಿ_ಬಳಸಿದ_lo,
                (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)virt_ಗಮ್ಯ_phys(vr->ಬಳಸಿದ));
       vp_write(&vp->common, virtio_pci_common_cfg, ಸರತಿ_ಬಳಸಿದ_hi, 0);
       vp_write(&vp->common, virtio_pci_common_cfg, ಸರತಿ_enable, 1);
       vq->ಸರತಿ_notify_off = vp_read(&vp->common, virtio_pci_common_cfg,
                                      ಸರತಿ_notify_off);
   } ಇಲ್ಲದಿದ್ದರೆ {
       vp_write(&vp->legacy, virtio_pci_legacy, ಸರತಿ_pfn,
                (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)virt_ಗಮ್ಯ_phys(vr->desc) >> ಪುಟ_ಸರಿಸು);
   }
   ಹಿಂತಿರುಗಿ ಅಂಕೆ;

fail:
   ಬಿಡುಗಡೆ(vq);
   *p_vq = ಖಾಲಿ;
   ಹಿಂತಿರುಗಿ -1;
}

ಶೂನ್ಯ vp_init_simple(ರಚನೆ vp_device *vp, ರಚನೆ pci_device *pci)
{
    u8 cap = pci_ಕಂಡುಹಿಡಿ_capability(pci->bdf, PCI_CAP_ID_VNDR, 0);
    ರಚನೆ vp_cap *vp_cap;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ;
    u32 ಸ್ಥಾನಾಂತರ_2, ತಳ, mul;
    u64 addr;
    u8 type;

    memset(vp, 0, ಗಾತ್ರ(*vp));
    ಆಗ (cap != 0) {
        type = pci_config_readb(pci->bdf, cap +
                                offsetof(ರಚನೆ virtio_pci_cap, cfg_type));
        ಆಯ್ಕೆ (type) {
        ಸ್ಥಿತಿ VIRTIO_PCI_CAP_COMMON_CFG:
            vp_cap = &vp->common;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ VIRTIO_PCI_CAP_NOTIFY_CFG:
            vp_cap = &vp->notify;
            mul = offsetof(ರಚನೆ virtio_pci_notify_cap, notify_off_multiplier);
            vp->notify_off_multiplier = pci_config_readl(pci->bdf, cap + mul);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ VIRTIO_PCI_CAP_ISR_CFG:
            vp_cap = &vp->isr;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ VIRTIO_PCI_CAP_DEVICE_CFG:
            vp_cap = &vp->device;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ VIRTIO_PCI_CAP_PCI_CFG:
            vp->common.cfg = cap;
            vp->common.bdf = pci->bdf;
            vp->notify.cfg = cap;
            vp->notify.bdf = pci->bdf;
            vp->isr.cfg = cap;
            vp->isr.bdf = pci->bdf;
            vp->device.cfg = cap;
            vp->device.bdf = pci->bdf;
            vp_cap = ಖಾಲಿ;
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d"
                    " [pci cfg access]\n", pci, cap, type);
            ನಿಲ್ಲಿಸು;
        ಪೂರ್ವನಿಯೋಜಿತ:
            vp_cap = ಖಾಲಿ;
            ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ (vp_cap && !vp_cap->cap) {
            vp_cap->cap = cap;
            vp_cap->bar = pci_config_readb(pci->bdf, cap +
                                           offsetof(ರಚನೆ virtio_pci_cap, bar));
            ಸ್ಥಾನಾಂತರ_2 = pci_config_readl(pci->bdf, cap +
                                      offsetof(ರಚನೆ virtio_pci_cap, ಸ್ಥಾನಾಂತರ_2));
            ತಳ = PCI_ತಳ_ವಿಳಾಸ_0 + 4 * vp_cap->bar;
            addr = pci_config_readl(pci->bdf, ತಳ);
            ಆದರೆ (addr & PCI_ತಳ_ವಿಳಾಸ_SPACE_IO) {
                addr &= PCI_ತಳ_ವಿಳಾಸ_IO_ಮುಖವಾಡ;
                vp_cap->ಬಹುಲಕ = VP_ACCESS_IO;
            } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((addr & PCI_ತಳ_ವಿಳಾಸ_MEM_TYPE_ಮುಖವಾಡ) ==
                       PCI_ತಳ_ವಿಳಾಸ_MEM_TYPE_64) {
                addr &= PCI_ತಳ_ವಿಳಾಸ_MEM_ಮುಖವಾಡ;
                addr |= (u64)pci_config_readl(pci->bdf, ತಳ + 4) << 32;
                vp_cap->ಬಹುಲಕ = (addr > 0xffffffffll) ?
                    VP_ACCESS_PCICFG : VP_ACCESS_MMIO;
            } ಇಲ್ಲದಿದ್ದರೆ {
                addr &= PCI_ತಳ_ವಿಳಾಸ_MEM_ಮುಖವಾಡ;
                vp_cap->ಬಹುಲಕ = VP_ACCESS_MMIO;
            }
            ಆಯ್ಕೆ (vp_cap->ಬಹುಲಕ) {
            ಸ್ಥಿತಿ VP_ACCESS_IO:
            {
                u32 addr = pci_enable_iobar(pci, ತಳ);
                ಆದರೆ (!addr)
                    ಹಿಂತಿರುಗಿ;
                vp_cap->ioaddr = addr + ಸ್ಥಾನಾಂತರ_2;
                ಬಹುಲಕ = "io";
                ನಿಲ್ಲಿಸು;
            }
            ಸ್ಥಿತಿ VP_ACCESS_MMIO:
            {
                ಶೂನ್ಯ *addr = pci_enable_membar(pci, ತಳ);
                ಆದರೆ (!addr)
                    ಹಿಂತಿರುಗಿ;
                vp_cap->memaddr = addr + ಸ್ಥಾನಾಂತರ_2;
                ಬಹುಲಕ = "mmio";
                ನಿಲ್ಲಿಸು;
            }
            ಸ್ಥಿತಿ VP_ACCESS_PCICFG:
                ಬಹುಲಕ = "pcicfg";
                vp_cap->baroff = ಸ್ಥಾನಾಂತರ_2;
                ನಿಲ್ಲಿಸು;
            ಪೂರ್ವನಿಯೋಜಿತ:
                ಬಹುಲಕ = "Huh?";
                ನಿಲ್ಲಿಸು;
            }
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d "
                    "bar %d at 0x%08llx off +0x%04x [%s]\n",
                    pci, vp_cap->cap, type, vp_cap->bar, addr, ಸ್ಥಾನಾಂತರ_2, ಬಹುಲಕ);
        }

        cap = pci_ಕಂಡುಹಿಡಿ_capability(pci->bdf, PCI_CAP_ID_VNDR, cap);
    }

    ಆದರೆ (vp->common.cap && vp->notify.cap && vp->isr.cap && vp->device.cap) {
        dprintf(1, "pci dev %pP using modern (1.0) virtio mode\n", pci);
        vp->use_modern = 1;
    } ಇಲ್ಲದಿದ್ದರೆ {
        dprintf(1, "pci dev %pP using legacy (0.9.5) virtio mode\n", pci);
        vp->legacy.bar = 0;
        vp->legacy.ioaddr = pci_enable_iobar(pci, PCI_ತಳ_ವಿಳಾಸ_0);
        ಆದರೆ (!vp->legacy.ioaddr)
            ಹಿಂತಿರುಗಿ;
        vp->legacy.ಬಹುಲಕ = VP_ACCESS_IO;
    }

    vp_reset(vp);
    pci_enable_busmaster(pci);
    vp_ಗಣ_ಸ್ಥಿತಿ(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER );
}

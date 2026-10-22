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

#అన్తర్భావ "config.h" // CONFIG_DEBUG_LEVEL
#అన్తర్భావ "malloc.h" // free
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pci.h" // pci_config_readl
#అన్తర్భావ "pcidevice.h" // struct pci_device
#అన్తర్భావ "pci_regs.h" // PCI_BASE_ADDRESS_0
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "virtio-pci.h"
#అన్తర్భావ "virtio-mmio.h"
#అన్తర్భావ "virtio-ring.h"

u64 _vp_read(నిర్మాణం vp_cap *cap, u32 స్థానభ్రంశం_2, u8 కొలత)
{
    u64 var = 0;

    ఎంపిక (cap->బహుళకం) {
    స్థితి VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + స్థానభ్రంశం_2;
        ఎంపిక (కొలత) {
        స్థితి 8:
            var = inl(addr);
            var |= (u64)inl(addr+4) << 32;
            ఆపు;
        స్థితి 4:
            var = inl(addr);
            ఆపు;
        స్థితి 2:
            var = inw(addr);
            ఆపు;
        స్థితి 1:
            var = inb(addr);
            ఆపు;
        }
        ఆపు;
    }

    స్థితి VP_ACCESS_MMIO:
    {
        శూన్య *addr = cap->memaddr + స్థానభ్రంశం_2;
        ఎంపిక (కొలత) {
        స్థితి 8:
            var = readl(addr);
            var |= (u64)readl(addr+4) << 32;
            ఆపు;
        స్థితి 4:
            var = readl(addr);
            ఆపు;
        స్థితి 2:
            var = readw(addr);
            ఆపు;
        స్థితి 1:
            var = readb(addr);
            ఆపు;
        }
        ఆపు;
    }

    స్థితి VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + స్థానభ్రంశం_2;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(నిర్మాణం virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(నిర్మాణం virtio_pci_cfg_cap, cap.స్థానభ్రంశం_2),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(నిర్మాణం virtio_pci_cfg_cap, cap.నిడివి),
                          (కొలత > 4) ? 4 : కొలత);
        ఎంపిక (కొలత) {
        స్థితి 8:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(నిర్మాణం virtio_pci_cfg_cap, pci_cfg_దత్తాంశం));
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(నిర్మాణం virtio_pci_cfg_cap, cap.స్థానభ్రంశం_2),
                              addr + 4);
            var |= (u64)pci_config_readl(cap->bdf, cap->cfg +
                                         offsetof(నిర్మాణం virtio_pci_cfg_cap, pci_cfg_దత్తాంశం)) << 32;
            ఆపు;
        స్థితి 4:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(నిర్మాణం virtio_pci_cfg_cap, pci_cfg_దత్తాంశం));
            ఆపు;
        స్థితి 2:
            var = pci_config_readw(cap->bdf, cap->cfg +
                                   offsetof(నిర్మాణం virtio_pci_cfg_cap, pci_cfg_దత్తాంశం));
            ఆపు;
        స్థితి 1:
            var = pci_config_readb(cap->bdf, cap->cfg +
                                   offsetof(నిర్మాణం virtio_pci_cfg_cap, pci_cfg_దత్తాంశం));
            ఆపు;
        }
    }
    }
    dprintf(9, "vp read   %x (%d) -> 0x%llx\n", cap->ioaddr + స్థానభ్రంశం_2, కొలత, var);
    తిరిగి var;
}

శూన్య _vp_write(నిర్మాణం vp_cap *cap, u32 స్థానభ్రంశం_2, u8 కొలత, u64 var)
{
    dprintf(9, "vp write  %x (%d) <- 0x%llx\n", cap->ioaddr + స్థానభ్రంశం_2, కొలత, var);

    ఎంపిక (cap->బహుళకం) {
    స్థితి VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + స్థానభ్రంశం_2;
        ఎంపిక (కొలత) {
        స్థితి 4:
            outl(var, addr);
            ఆపు;
        స్థితి 2:
            outw(var, addr);
            ఆపు;
        స్థితి 1:
            outb(var, addr);
            ఆపు;
        }
        ఆపు;
    }

    స్థితి VP_ACCESS_MMIO:
    {
        శూన్య *addr = cap->memaddr + స్థానభ్రంశం_2;
        ఎంపిక (కొలత) {
        స్థితి 4:
            writel(addr, var);
            ఆపు;
        స్థితి 2:
            writew(addr, var);
            ఆపు;
        స్థితి 1:
            writeb(addr, var);
            ఆపు;
        }
        ఆపు;
    }

    స్థితి VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + స్థానభ్రంశం_2;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(నిర్మాణం virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(నిర్మాణం virtio_pci_cfg_cap, cap.స్థానభ్రంశం_2),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(నిర్మాణం virtio_pci_cfg_cap, cap.నిడివి),
                          కొలత);
        ఎంపిక (కొలత) {
        స్థితి 4:
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(నిర్మాణం virtio_pci_cfg_cap, pci_cfg_దత్తాంశం),
                              var);
            ఆపు;
        స్థితి 2:
            pci_config_writew(cap->bdf, cap->cfg +
                              offsetof(నిర్మాణం virtio_pci_cfg_cap, pci_cfg_దత్తాంశం),
                              var);
            ఆపు;
        స్థితి 1:
            pci_config_writeb(cap->bdf, cap->cfg +
                              offsetof(నిర్మాణం virtio_pci_cfg_cap, pci_cfg_దత్తాంశం),
                              var);
            ఆపు;
        }
    }
    }
}

u64 vp_get_features(నిర్మాణం vp_device *vp)
{
    u32 f0, f1;

    ఒకవేళ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
    } లేకపోతే ఒకవేళ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
    } లేకపోతే {
        f0 = vp_read(&vp->legacy, virtio_pci_legacy, host_features);
        f1 = 0;
    }
    తిరిగి ((u64)f1 << 32) | f0;
}

శూన్య vp_సమితి_features(నిర్మాణం vp_device *vp, u64 features)
{
    u32 f0, f1;

    f0 = features;
    f1 = features >> 32;

    ఒకవేళ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f1);
    } లేకపోతే ఒకవేళ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f1);
    } లేకపోతే {
        vp_write(&vp->legacy, virtio_pci_legacy, guest_features, f0);
    }
}

u8 vp_get_స్థితి(నిర్మాణం vp_device *vp)
{
    ఒకవేళ (vp->use_mmio) {
        తిరిగి vp_read(&vp->common, virtio_mmio_cfg, device_స్థితి);
    } లేకపోతే ఒకవేళ (vp->use_modern) {
        తిరిగి vp_read(&vp->common, virtio_pci_common_cfg, device_స్థితి);
    } లేకపోతే {
        తిరిగి vp_read(&vp->legacy, virtio_pci_legacy, స్థితి_2);
    }
}

శూన్య vp_సమితి_స్థితి(నిర్మాణం vp_device *vp, u8 స్థితి_2)
{
    ఒకవేళ (స్థితి_2 == 0)        /* reset */
        తిరిగి;
    ఒకవేళ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_స్థితి, స్థితి_2);
    } లేకపోతే ఒకవేళ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_స్థితి, స్థితి_2);
    } లేకపోతే {
        vp_write(&vp->legacy, virtio_pci_legacy, స్థితి_2, స్థితి_2);
    }
}

u8 vp_get_isr(నిర్మాణం vp_device *vp)
{
    ఒకవేళ (vp->use_mmio) {
        తిరిగి vp_read(&vp->common, virtio_mmio_cfg, irq_స్థితి);
    } లేకపోతే ఒకవేళ (vp->use_modern) {
        తిరిగి vp_read(&vp->isr, virtio_pci_isr, isr);
    } లేకపోతే {
        తిరిగి vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

శూన్య vp_reset(నిర్మాణం vp_device *vp)
{
    ఒకవేళ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_స్థితి, 0);
        vp_read(&vp->common, virtio_mmio_cfg, irq_స్థితి);
    } లేకపోతే ఒకవేళ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_స్థితి, 0);
        vp_read(&vp->isr, virtio_pci_isr, isr);
    } లేకపోతే {
        vp_write(&vp->legacy, virtio_pci_legacy, స్థితి_2, 0);
        vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

శూన్య vp_notify(నిర్మాణం vp_device *vp, నిర్మాణం vring_virtqueue *vq)
{
    ఒకవేళ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, వరుస_notify, vq->వరుస_సూచీ);
    } లేకపోతే ఒకవేళ (vp->use_modern) {
        u32 స్థానభ్రంశం_2 = vq->వరుస_notify_off * vp->notify_off_multiplier;
        ఎంపిక (vp->notify.బహుళకం) {
        స్థితి VP_ACCESS_IO:
            outw(vq->వరుస_సూచీ, vp->notify.ioaddr + స్థానభ్రంశం_2);
            ఆపు;
        స్థితి VP_ACCESS_MMIO:
            writew(vp->notify.memaddr + స్థానభ్రంశం_2, vq->వరుస_సూచీ);
            ఆపు;
        స్థితి VP_ACCESS_PCICFG:
            pci_config_writeb(vp->notify.bdf, vp->notify.cfg +
                              offsetof(నిర్మాణం virtio_pci_cfg_cap, cap.bar),
                              vp->notify.bar);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(నిర్మాణం virtio_pci_cfg_cap, cap.స్థానభ్రంశం_2),
                              vp->notify.baroff + స్థానభ్రంశం_2);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(నిర్మాణం virtio_pci_cfg_cap, cap.నిడివి),
                              2);
            pci_config_writew(vp->notify.bdf, vp->notify.cfg +
                              offsetof(నిర్మాణం virtio_pci_cfg_cap, pci_cfg_దత్తాంశం),
                              vq->వరుస_సూచీ);
        }
        dprintf(9, "vp notify %x (%d) -- 0x%x\n",
                vp->notify.ioaddr, 2, vq->వరుస_సూచీ);
    } లేకపోతే {
        vp_write(&vp->legacy, virtio_pci_legacy, వరుస_notify, vq->వరుస_సూచీ);
    }
}

పూర్ణాంకం vp_కనుగొను_vq(నిర్మాణం vp_device *vp, పూర్ణాంకం వరుస_సూచీ,
               నిర్మాణం vring_virtqueue **p_vq)
{
   u16 అంకె;

   ASSERT32FLAT();
   నిర్మాణం vring_virtqueue *vq = *p_vq = memalign_ఎగువ(పుట_కొలత, పరిమాణం(*vq));
   ఒకవేళ (!vq) {
       warn_noalloc();
       వెళ్ళు fail;
   }
   memset(vq, 0, పరిమాణం(*vq));


   /* select the queue */
   ఒకవేళ (vp->use_mmio) {
       vp_write(&vp->common, virtio_mmio_cfg, వరుస_select, వరుస_సూచీ);
   } లేకపోతే ఒకవేళ (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, వరుస_select, వరుస_సూచీ);
   } లేకపోతే {
       vp_write(&vp->legacy, virtio_pci_legacy, వరుస_sel, వరుస_సూచీ);
   }

   /* check if the queue is available */
   ఒకవేళ (vp->use_mmio) {
       అంకె = vp_read(&vp->common, virtio_mmio_cfg, వరుస_అంకె_max);
       ఒకవేళ (అంకె > MAX_వరుస_అంకె)
           అంకె = MAX_వరుస_అంకె;
       vp_write(&vp->common, virtio_mmio_cfg, వరుస_అంకె, అంకె);
   } లేకపోతే ఒకవేళ (vp->use_modern) {
       అంకె = vp_read(&vp->common, virtio_pci_common_cfg, వరుస_కొలత);
       ఒకవేళ (అంకె > MAX_వరుస_అంకె) {
           vp_write(&vp->common, virtio_pci_common_cfg, వరుస_కొలత,
                    MAX_వరుస_అంకె);
           అంకె = vp_read(&vp->common, virtio_pci_common_cfg, వరుస_కొలత);
       }
   } లేకపోతే {
       అంకె = vp_read(&vp->legacy, virtio_pci_legacy, వరుస_అంకె);
   }
   ఒకవేళ (!అంకె) {
       dprintf(1, "ERROR: queue size is 0\n");
       వెళ్ళు fail;
   }
   ఒకవేళ (అంకె > MAX_వరుస_అంకె) {
       dprintf(1, "ERROR: queue size %d > %d\n", అంకె, MAX_వరుస_అంకె);
       వెళ్ళు fail;
   }

   /* check if the queue is already active */
   ఒకవేళ (vp->use_mmio) {
       /* TODO */;
   } లేకపోతే ఒకవేళ (vp->use_modern) {
       ఒకవేళ (vp_read(&vp->common, virtio_pci_common_cfg, వరుస_enable)) {
           dprintf(1, "ERROR: queue already active\n");
           వెళ్ళు fail;
       }
   } లేకపోతే {
       ఒకవేళ (vp_read(&vp->legacy, virtio_pci_legacy, వరుస_pfn)) {
           dprintf(1, "ERROR: queue already active\n");
           వెళ్ళు fail;
       }
   }
   vq->వరుస_సూచీ = వరుస_సూచీ;

   /* initialize the queue */
   నిర్మాణం vring * vr = &vq->vring;
   vring_init(vr, అంకె, (గుర్తులేని అక్షరం*)&vq->వరుస);

   /* activate the queue
    *
    * NOTE: vr->desc is initialized by vring_init()
    */

   ఒకవేళ (vp->use_mmio) {
       ఒకవేళ (vp_read(&vp->common, virtio_mmio_cfg, సంస్కరణ_2) == 2) {
           vp_write(&vp->common, virtio_mmio_cfg, వరుస_desc_lo,
                    (గుర్తులేని దీర్ఘ)virt_గమ్యం_phys(vr->desc));
           vp_write(&vp->common, virtio_mmio_cfg, వరుస_desc_hi, 0);
           vp_write(&vp->common, virtio_mmio_cfg, వరుస_driver_lo,
                    (గుర్తులేని దీర్ఘ)virt_గమ్యం_phys(vr->avail));
           vp_write(&vp->common, virtio_mmio_cfg, వరుస_driver_hi, 0);
           vp_write(&vp->common, virtio_mmio_cfg, వరుస_device_lo,
                    (గుర్తులేని దీర్ఘ)virt_గమ్యం_phys(vr->వాడిన));
           vp_write(&vp->common, virtio_mmio_cfg, వరుస_device_hi, 0);
           vp_write(&vp->common, virtio_mmio_cfg, వరుస_ready, 1);
       } లేకపోతే {
           vp_write(&vp->common, virtio_mmio_cfg, legacy_guest_పుట_కొలత,
                    (గుర్తులేని దీర్ఘ)1 << పుట_జరుపు);
           vp_write(&vp->common, virtio_mmio_cfg, legacy_వరుస_pfn,
                    (గుర్తులేని దీర్ఘ)virt_గమ్యం_phys(vr->desc) >> పుట_జరుపు);
       }
   } లేకపోతే ఒకవేళ (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, వరుస_desc_lo,
                (గుర్తులేని దీర్ఘ)virt_గమ్యం_phys(vr->desc));
       vp_write(&vp->common, virtio_pci_common_cfg, వరుస_desc_hi, 0);
       vp_write(&vp->common, virtio_pci_common_cfg, వరుస_avail_lo,
                (గుర్తులేని దీర్ఘ)virt_గమ్యం_phys(vr->avail));
       vp_write(&vp->common, virtio_pci_common_cfg, వరుస_avail_hi, 0);
       vp_write(&vp->common, virtio_pci_common_cfg, వరుస_వాడిన_lo,
                (గుర్తులేని దీర్ఘ)virt_గమ్యం_phys(vr->వాడిన));
       vp_write(&vp->common, virtio_pci_common_cfg, వరుస_వాడిన_hi, 0);
       vp_write(&vp->common, virtio_pci_common_cfg, వరుస_enable, 1);
       vq->వరుస_notify_off = vp_read(&vp->common, virtio_pci_common_cfg,
                                      వరుస_notify_off);
   } లేకపోతే {
       vp_write(&vp->legacy, virtio_pci_legacy, వరుస_pfn,
                (గుర్తులేని దీర్ఘ)virt_గమ్యం_phys(vr->desc) >> పుట_జరుపు);
   }
   తిరిగి అంకె;

fail:
   విడుదల(vq);
   *p_vq = ఖాళీ;
   తిరిగి -1;
}

శూన్య vp_init_simple(నిర్మాణం vp_device *vp, నిర్మాణం pci_device *pci)
{
    u8 cap = pci_కనుగొను_capability(pci->bdf, PCI_CAP_ID_VNDR, 0);
    నిర్మాణం vp_cap *vp_cap;
    స్థిరరాశి అక్షరం *బహుళకం;
    u32 స్థానభ్రంశం_2, పునాది, mul;
    u64 addr;
    u8 type;

    memset(vp, 0, పరిమాణం(*vp));
    అయితే (cap != 0) {
        type = pci_config_readb(pci->bdf, cap +
                                offsetof(నిర్మాణం virtio_pci_cap, cfg_type));
        ఎంపిక (type) {
        స్థితి VIRTIO_PCI_CAP_COMMON_CFG:
            vp_cap = &vp->common;
            ఆపు;
        స్థితి VIRTIO_PCI_CAP_NOTIFY_CFG:
            vp_cap = &vp->notify;
            mul = offsetof(నిర్మాణం virtio_pci_notify_cap, notify_off_multiplier);
            vp->notify_off_multiplier = pci_config_readl(pci->bdf, cap + mul);
            ఆపు;
        స్థితి VIRTIO_PCI_CAP_ISR_CFG:
            vp_cap = &vp->isr;
            ఆపు;
        స్థితి VIRTIO_PCI_CAP_DEVICE_CFG:
            vp_cap = &vp->device;
            ఆపు;
        స్థితి VIRTIO_PCI_CAP_PCI_CFG:
            vp->common.cfg = cap;
            vp->common.bdf = pci->bdf;
            vp->notify.cfg = cap;
            vp->notify.bdf = pci->bdf;
            vp->isr.cfg = cap;
            vp->isr.bdf = pci->bdf;
            vp->device.cfg = cap;
            vp->device.bdf = pci->bdf;
            vp_cap = ఖాళీ;
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d"
                    " [pci cfg access]\n", pci, cap, type);
            ఆపు;
        అప్రమేయ:
            vp_cap = ఖాళీ;
            ఆపు;
        }
        ఒకవేళ (vp_cap && !vp_cap->cap) {
            vp_cap->cap = cap;
            vp_cap->bar = pci_config_readb(pci->bdf, cap +
                                           offsetof(నిర్మాణం virtio_pci_cap, bar));
            స్థానభ్రంశం_2 = pci_config_readl(pci->bdf, cap +
                                      offsetof(నిర్మాణం virtio_pci_cap, స్థానభ్రంశం_2));
            పునాది = PCI_పునాది_చిరునామా_0 + 4 * vp_cap->bar;
            addr = pci_config_readl(pci->bdf, పునాది);
            ఒకవేళ (addr & PCI_పునాది_చిరునామా_SPACE_IO) {
                addr &= PCI_పునాది_చిరునామా_IO_ముసుగు;
                vp_cap->బహుళకం = VP_ACCESS_IO;
            } లేకపోతే ఒకవేళ ((addr & PCI_పునాది_చిరునామా_MEM_TYPE_ముసుగు) ==
                       PCI_పునాది_చిరునామా_MEM_TYPE_64) {
                addr &= PCI_పునాది_చిరునామా_MEM_ముసుగు;
                addr |= (u64)pci_config_readl(pci->bdf, పునాది + 4) << 32;
                vp_cap->బహుళకం = (addr > 0xffffffffll) ?
                    VP_ACCESS_PCICFG : VP_ACCESS_MMIO;
            } లేకపోతే {
                addr &= PCI_పునాది_చిరునామా_MEM_ముసుగు;
                vp_cap->బహుళకం = VP_ACCESS_MMIO;
            }
            ఎంపిక (vp_cap->బహుళకం) {
            స్థితి VP_ACCESS_IO:
            {
                u32 addr = pci_enable_iobar(pci, పునాది);
                ఒకవేళ (!addr)
                    తిరిగి;
                vp_cap->ioaddr = addr + స్థానభ్రంశం_2;
                బహుళకం = "io";
                ఆపు;
            }
            స్థితి VP_ACCESS_MMIO:
            {
                శూన్య *addr = pci_enable_membar(pci, పునాది);
                ఒకవేళ (!addr)
                    తిరిగి;
                vp_cap->memaddr = addr + స్థానభ్రంశం_2;
                బహుళకం = "mmio";
                ఆపు;
            }
            స్థితి VP_ACCESS_PCICFG:
                బహుళకం = "pcicfg";
                vp_cap->baroff = స్థానభ్రంశం_2;
                ఆపు;
            అప్రమేయ:
                బహుళకం = "Huh?";
                ఆపు;
            }
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d "
                    "bar %d at 0x%08llx off +0x%04x [%s]\n",
                    pci, vp_cap->cap, type, vp_cap->bar, addr, స్థానభ్రంశం_2, బహుళకం);
        }

        cap = pci_కనుగొను_capability(pci->bdf, PCI_CAP_ID_VNDR, cap);
    }

    ఒకవేళ (vp->common.cap && vp->notify.cap && vp->isr.cap && vp->device.cap) {
        dprintf(1, "pci dev %pP using modern (1.0) virtio mode\n", pci);
        vp->use_modern = 1;
    } లేకపోతే {
        dprintf(1, "pci dev %pP using legacy (0.9.5) virtio mode\n", pci);
        vp->legacy.bar = 0;
        vp->legacy.ioaddr = pci_enable_iobar(pci, PCI_పునాది_చిరునామా_0);
        ఒకవేళ (!vp->legacy.ioaddr)
            తిరిగి;
        vp->legacy.బహుళకం = VP_ACCESS_IO;
    }

    vp_reset(vp);
    pci_enable_busmaster(pci);
    vp_సమితి_స్థితి(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER );
}

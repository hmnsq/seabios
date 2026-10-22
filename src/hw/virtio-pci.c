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

#شامل "config.h" // CONFIG_DEBUG_LEVEL
#شامل "malloc.h" // free
#شامل "output.h" // dprintf
#شامل "pci.h" // pci_config_readl
#شامل "pcidevice.h" // struct pci_device
#شامل "pci_regs.h" // PCI_BASE_ADDRESS_0
#شامل "string.h" // memset
#شامل "virtio-pci.h"
#شامل "virtio-mmio.h"
#شامل "virtio-ring.h"

u64 _vp_read(ساخت vp_cap *cap, u32 انحراف_2, u8 سائز)
{
    u64 var = 0;

    انتخاب (cap->منوال) {
    حالت VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + انحراف_2;
        انتخاب (سائز) {
        حالت 8:
            var = inl(addr);
            var |= (u64)inl(addr+4) << 32;
            توڑو;
        حالت 4:
            var = inl(addr);
            توڑو;
        حالت 2:
            var = inw(addr);
            توڑو;
        حالت 1:
            var = inb(addr);
            توڑو;
        }
        توڑو;
    }

    حالت VP_ACCESS_MMIO:
    {
        خالی *addr = cap->memaddr + انحراف_2;
        انتخاب (سائز) {
        حالت 8:
            var = readl(addr);
            var |= (u64)readl(addr+4) << 32;
            توڑو;
        حالت 4:
            var = readl(addr);
            توڑو;
        حالت 2:
            var = readw(addr);
            توڑو;
        حالت 1:
            var = readb(addr);
            توڑو;
        }
        توڑو;
    }

    حالت VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + انحراف_2;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(ساخت virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ساخت virtio_pci_cfg_cap, cap.انحراف_2),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ساخت virtio_pci_cfg_cap, cap.لمبائی),
                          (سائز > 4) ? 4 : سائز);
        انتخاب (سائز) {
        حالت 8:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(ساخت virtio_pci_cfg_cap, pci_cfg_ڈیٹا));
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(ساخت virtio_pci_cfg_cap, cap.انحراف_2),
                              addr + 4);
            var |= (u64)pci_config_readl(cap->bdf, cap->cfg +
                                         offsetof(ساخت virtio_pci_cfg_cap, pci_cfg_ڈیٹا)) << 32;
            توڑو;
        حالت 4:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(ساخت virtio_pci_cfg_cap, pci_cfg_ڈیٹا));
            توڑو;
        حالت 2:
            var = pci_config_readw(cap->bdf, cap->cfg +
                                   offsetof(ساخت virtio_pci_cfg_cap, pci_cfg_ڈیٹا));
            توڑو;
        حالت 1:
            var = pci_config_readb(cap->bdf, cap->cfg +
                                   offsetof(ساخت virtio_pci_cfg_cap, pci_cfg_ڈیٹا));
            توڑو;
        }
    }
    }
    dprintf(9, "vp read   %x (%d) -> 0x%llx\n", cap->ioaddr + انحراف_2, سائز, var);
    واپس var;
}

خالی _vp_write(ساخت vp_cap *cap, u32 انحراف_2, u8 سائز, u64 var)
{
    dprintf(9, "vp write  %x (%d) <- 0x%llx\n", cap->ioaddr + انحراف_2, سائز, var);

    انتخاب (cap->منوال) {
    حالت VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + انحراف_2;
        انتخاب (سائز) {
        حالت 4:
            outl(var, addr);
            توڑو;
        حالت 2:
            outw(var, addr);
            توڑو;
        حالت 1:
            outb(var, addr);
            توڑو;
        }
        توڑو;
    }

    حالت VP_ACCESS_MMIO:
    {
        خالی *addr = cap->memaddr + انحراف_2;
        انتخاب (سائز) {
        حالت 4:
            writel(addr, var);
            توڑو;
        حالت 2:
            writew(addr, var);
            توڑو;
        حالت 1:
            writeb(addr, var);
            توڑو;
        }
        توڑو;
    }

    حالت VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + انحراف_2;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(ساخت virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ساخت virtio_pci_cfg_cap, cap.انحراف_2),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ساخت virtio_pci_cfg_cap, cap.لمبائی),
                          سائز);
        انتخاب (سائز) {
        حالت 4:
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(ساخت virtio_pci_cfg_cap, pci_cfg_ڈیٹا),
                              var);
            توڑو;
        حالت 2:
            pci_config_writew(cap->bdf, cap->cfg +
                              offsetof(ساخت virtio_pci_cfg_cap, pci_cfg_ڈیٹا),
                              var);
            توڑو;
        حالت 1:
            pci_config_writeb(cap->bdf, cap->cfg +
                              offsetof(ساخت virtio_pci_cfg_cap, pci_cfg_ڈیٹا),
                              var);
            توڑو;
        }
    }
    }
}

u64 vp_get_features(ساخت vp_device *vp)
{
    u32 f0, f1;

    اگر (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
    } ورنہ اگر (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
    } ورنہ {
        f0 = vp_read(&vp->legacy, virtio_pci_legacy, host_features);
        f1 = 0;
    }
    واپس ((u64)f1 << 32) | f0;
}

خالی vp_سیٹ_features(ساخت vp_device *vp, u64 features)
{
    u32 f0, f1;

    f0 = features;
    f1 = features >> 32;

    اگر (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f1);
    } ورنہ اگر (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f1);
    } ورنہ {
        vp_write(&vp->legacy, virtio_pci_legacy, guest_features, f0);
    }
}

u8 vp_get_حالت(ساخت vp_device *vp)
{
    اگر (vp->use_mmio) {
        واپس vp_read(&vp->common, virtio_mmio_cfg, device_حالت);
    } ورنہ اگر (vp->use_modern) {
        واپس vp_read(&vp->common, virtio_pci_common_cfg, device_حالت);
    } ورنہ {
        واپس vp_read(&vp->legacy, virtio_pci_legacy, حالت_2);
    }
}

خالی vp_سیٹ_حالت(ساخت vp_device *vp, u8 حالت_2)
{
    اگر (حالت_2 == 0)        /* reset */
        واپس;
    اگر (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_حالت, حالت_2);
    } ورنہ اگر (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_حالت, حالت_2);
    } ورنہ {
        vp_write(&vp->legacy, virtio_pci_legacy, حالت_2, حالت_2);
    }
}

u8 vp_get_isr(ساخت vp_device *vp)
{
    اگر (vp->use_mmio) {
        واپس vp_read(&vp->common, virtio_mmio_cfg, irq_حالت);
    } ورنہ اگر (vp->use_modern) {
        واپس vp_read(&vp->isr, virtio_pci_isr, isr);
    } ورنہ {
        واپس vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

خالی vp_reset(ساخت vp_device *vp)
{
    اگر (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_حالت, 0);
        vp_read(&vp->common, virtio_mmio_cfg, irq_حالت);
    } ورنہ اگر (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_حالت, 0);
        vp_read(&vp->isr, virtio_pci_isr, isr);
    } ورنہ {
        vp_write(&vp->legacy, virtio_pci_legacy, حالت_2, 0);
        vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

خالی vp_notify(ساخت vp_device *vp, ساخت vring_virtqueue *vq)
{
    اگر (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, قطار_notify, vq->قطار_اشاریہ);
    } ورنہ اگر (vp->use_modern) {
        u32 انحراف_2 = vq->قطار_notify_off * vp->notify_off_multiplier;
        انتخاب (vp->notify.منوال) {
        حالت VP_ACCESS_IO:
            outw(vq->قطار_اشاریہ, vp->notify.ioaddr + انحراف_2);
            توڑو;
        حالت VP_ACCESS_MMIO:
            writew(vp->notify.memaddr + انحراف_2, vq->قطار_اشاریہ);
            توڑو;
        حالت VP_ACCESS_PCICFG:
            pci_config_writeb(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ساخت virtio_pci_cfg_cap, cap.bar),
                              vp->notify.bar);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ساخت virtio_pci_cfg_cap, cap.انحراف_2),
                              vp->notify.baroff + انحراف_2);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ساخت virtio_pci_cfg_cap, cap.لمبائی),
                              2);
            pci_config_writew(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ساخت virtio_pci_cfg_cap, pci_cfg_ڈیٹا),
                              vq->قطار_اشاریہ);
        }
        dprintf(9, "vp notify %x (%d) -- 0x%x\n",
                vp->notify.ioaddr, 2, vq->قطار_اشاریہ);
    } ورنہ {
        vp_write(&vp->legacy, virtio_pci_legacy, قطار_notify, vq->قطار_اشاریہ);
    }
}

صحیح vp_ڈھونڈو_vq(ساخت vp_device *vp, صحیح قطار_اشاریہ,
               ساخت vring_virtqueue **p_vq)
{
   u16 عدد;

   ASSERT32FLAT();
   ساخت vring_virtqueue *vq = *p_vq = memalign_بلند(صفحہ_سائز, حجم(*vq));
   اگر (!vq) {
       warn_noalloc();
       جاؤ fail;
   }
   memset(vq, 0, حجم(*vq));


   /* select the queue */
   اگر (vp->use_mmio) {
       vp_write(&vp->common, virtio_mmio_cfg, قطار_select, قطار_اشاریہ);
   } ورنہ اگر (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, قطار_select, قطار_اشاریہ);
   } ورنہ {
       vp_write(&vp->legacy, virtio_pci_legacy, قطار_sel, قطار_اشاریہ);
   }

   /* check if the queue is available */
   اگر (vp->use_mmio) {
       عدد = vp_read(&vp->common, virtio_mmio_cfg, قطار_عدد_max);
       اگر (عدد > MAX_قطار_عدد)
           عدد = MAX_قطار_عدد;
       vp_write(&vp->common, virtio_mmio_cfg, قطار_عدد, عدد);
   } ورنہ اگر (vp->use_modern) {
       عدد = vp_read(&vp->common, virtio_pci_common_cfg, قطار_سائز);
       اگر (عدد > MAX_قطار_عدد) {
           vp_write(&vp->common, virtio_pci_common_cfg, قطار_سائز,
                    MAX_قطار_عدد);
           عدد = vp_read(&vp->common, virtio_pci_common_cfg, قطار_سائز);
       }
   } ورنہ {
       عدد = vp_read(&vp->legacy, virtio_pci_legacy, قطار_عدد);
   }
   اگر (!عدد) {
       dprintf(1, "ERROR: queue size is 0\n");
       جاؤ fail;
   }
   اگر (عدد > MAX_قطار_عدد) {
       dprintf(1, "ERROR: queue size %d > %d\n", عدد, MAX_قطار_عدد);
       جاؤ fail;
   }

   /* check if the queue is already active */
   اگر (vp->use_mmio) {
       /* TODO */;
   } ورنہ اگر (vp->use_modern) {
       اگر (vp_read(&vp->common, virtio_pci_common_cfg, قطار_enable)) {
           dprintf(1, "ERROR: queue already active\n");
           جاؤ fail;
       }
   } ورنہ {
       اگر (vp_read(&vp->legacy, virtio_pci_legacy, قطار_pfn)) {
           dprintf(1, "ERROR: queue already active\n");
           جاؤ fail;
       }
   }
   vq->قطار_اشاریہ = قطار_اشاریہ;

   /* initialize the queue */
   ساخت vring * vr = &vq->vring;
   vring_init(vr, عدد, (غیرعلامتی حرف*)&vq->قطار);

   /* activate the queue
    *
    * NOTE: vr->desc is initialized by vring_init()
    */

   اگر (vp->use_mmio) {
       اگر (vp_read(&vp->common, virtio_mmio_cfg, نسخہ_2) == 2) {
           vp_write(&vp->common, virtio_mmio_cfg, قطار_desc_lo,
                    (غیرعلامتی طویل)virt_مقصد_phys(vr->desc));
           vp_write(&vp->common, virtio_mmio_cfg, قطار_desc_hi, 0);
           vp_write(&vp->common, virtio_mmio_cfg, قطار_driver_lo,
                    (غیرعلامتی طویل)virt_مقصد_phys(vr->avail));
           vp_write(&vp->common, virtio_mmio_cfg, قطار_driver_hi, 0);
           vp_write(&vp->common, virtio_mmio_cfg, قطار_device_lo,
                    (غیرعلامتی طویل)virt_مقصد_phys(vr->مستعمل));
           vp_write(&vp->common, virtio_mmio_cfg, قطار_device_hi, 0);
           vp_write(&vp->common, virtio_mmio_cfg, قطار_ready, 1);
       } ورنہ {
           vp_write(&vp->common, virtio_mmio_cfg, legacy_guest_صفحہ_سائز,
                    (غیرعلامتی طویل)1 << صفحہ_سرکاؤ);
           vp_write(&vp->common, virtio_mmio_cfg, legacy_قطار_pfn,
                    (غیرعلامتی طویل)virt_مقصد_phys(vr->desc) >> صفحہ_سرکاؤ);
       }
   } ورنہ اگر (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, قطار_desc_lo,
                (غیرعلامتی طویل)virt_مقصد_phys(vr->desc));
       vp_write(&vp->common, virtio_pci_common_cfg, قطار_desc_hi, 0);
       vp_write(&vp->common, virtio_pci_common_cfg, قطار_avail_lo,
                (غیرعلامتی طویل)virt_مقصد_phys(vr->avail));
       vp_write(&vp->common, virtio_pci_common_cfg, قطار_avail_hi, 0);
       vp_write(&vp->common, virtio_pci_common_cfg, قطار_مستعمل_lo,
                (غیرعلامتی طویل)virt_مقصد_phys(vr->مستعمل));
       vp_write(&vp->common, virtio_pci_common_cfg, قطار_مستعمل_hi, 0);
       vp_write(&vp->common, virtio_pci_common_cfg, قطار_enable, 1);
       vq->قطار_notify_off = vp_read(&vp->common, virtio_pci_common_cfg,
                                      قطار_notify_off);
   } ورنہ {
       vp_write(&vp->legacy, virtio_pci_legacy, قطار_pfn,
                (غیرعلامتی طویل)virt_مقصد_phys(vr->desc) >> صفحہ_سرکاؤ);
   }
   واپس عدد;

fail:
   آزاد(vq);
   *p_vq = تہی;
   واپس -1;
}

خالی vp_init_simple(ساخت vp_device *vp, ساخت pci_device *pci)
{
    u8 cap = pci_ڈھونڈو_capability(pci->bdf, PCI_CAP_ID_VNDR, 0);
    ساخت vp_cap *vp_cap;
    ثابت حرف *منوال;
    u32 انحراف_2, بنیاد, mul;
    u64 addr;
    u8 type;

    memset(vp, 0, حجم(*vp));
    جب (cap != 0) {
        type = pci_config_readb(pci->bdf, cap +
                                offsetof(ساخت virtio_pci_cap, cfg_type));
        انتخاب (type) {
        حالت VIRTIO_PCI_CAP_COMMON_CFG:
            vp_cap = &vp->common;
            توڑو;
        حالت VIRTIO_PCI_CAP_NOTIFY_CFG:
            vp_cap = &vp->notify;
            mul = offsetof(ساخت virtio_pci_notify_cap, notify_off_multiplier);
            vp->notify_off_multiplier = pci_config_readl(pci->bdf, cap + mul);
            توڑو;
        حالت VIRTIO_PCI_CAP_ISR_CFG:
            vp_cap = &vp->isr;
            توڑو;
        حالت VIRTIO_PCI_CAP_DEVICE_CFG:
            vp_cap = &vp->device;
            توڑو;
        حالت VIRTIO_PCI_CAP_PCI_CFG:
            vp->common.cfg = cap;
            vp->common.bdf = pci->bdf;
            vp->notify.cfg = cap;
            vp->notify.bdf = pci->bdf;
            vp->isr.cfg = cap;
            vp->isr.bdf = pci->bdf;
            vp->device.cfg = cap;
            vp->device.bdf = pci->bdf;
            vp_cap = تہی;
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d"
                    " [pci cfg access]\n", pci, cap, type);
            توڑو;
        طےشدہ:
            vp_cap = تہی;
            توڑو;
        }
        اگر (vp_cap && !vp_cap->cap) {
            vp_cap->cap = cap;
            vp_cap->bar = pci_config_readb(pci->bdf, cap +
                                           offsetof(ساخت virtio_pci_cap, bar));
            انحراف_2 = pci_config_readl(pci->bdf, cap +
                                      offsetof(ساخت virtio_pci_cap, انحراف_2));
            بنیاد = PCI_بنیاد_پتہ_0 + 4 * vp_cap->bar;
            addr = pci_config_readl(pci->bdf, بنیاد);
            اگر (addr & PCI_بنیاد_پتہ_SPACE_IO) {
                addr &= PCI_بنیاد_پتہ_IO_نقاب;
                vp_cap->منوال = VP_ACCESS_IO;
            } ورنہ اگر ((addr & PCI_بنیاد_پتہ_MEM_TYPE_نقاب) ==
                       PCI_بنیاد_پتہ_MEM_TYPE_64) {
                addr &= PCI_بنیاد_پتہ_MEM_نقاب;
                addr |= (u64)pci_config_readl(pci->bdf, بنیاد + 4) << 32;
                vp_cap->منوال = (addr > 0xffffffffll) ?
                    VP_ACCESS_PCICFG : VP_ACCESS_MMIO;
            } ورنہ {
                addr &= PCI_بنیاد_پتہ_MEM_نقاب;
                vp_cap->منوال = VP_ACCESS_MMIO;
            }
            انتخاب (vp_cap->منوال) {
            حالت VP_ACCESS_IO:
            {
                u32 addr = pci_enable_iobar(pci, بنیاد);
                اگر (!addr)
                    واپس;
                vp_cap->ioaddr = addr + انحراف_2;
                منوال = "io";
                توڑو;
            }
            حالت VP_ACCESS_MMIO:
            {
                خالی *addr = pci_enable_membar(pci, بنیاد);
                اگر (!addr)
                    واپس;
                vp_cap->memaddr = addr + انحراف_2;
                منوال = "mmio";
                توڑو;
            }
            حالت VP_ACCESS_PCICFG:
                منوال = "pcicfg";
                vp_cap->baroff = انحراف_2;
                توڑو;
            طےشدہ:
                منوال = "Huh?";
                توڑو;
            }
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d "
                    "bar %d at 0x%08llx off +0x%04x [%s]\n",
                    pci, vp_cap->cap, type, vp_cap->bar, addr, انحراف_2, منوال);
        }

        cap = pci_ڈھونڈو_capability(pci->bdf, PCI_CAP_ID_VNDR, cap);
    }

    اگر (vp->common.cap && vp->notify.cap && vp->isr.cap && vp->device.cap) {
        dprintf(1, "pci dev %pP using modern (1.0) virtio mode\n", pci);
        vp->use_modern = 1;
    } ورنہ {
        dprintf(1, "pci dev %pP using legacy (0.9.5) virtio mode\n", pci);
        vp->legacy.bar = 0;
        vp->legacy.ioaddr = pci_enable_iobar(pci, PCI_بنیاد_پتہ_0);
        اگر (!vp->legacy.ioaddr)
            واپس;
        vp->legacy.منوال = VP_ACCESS_IO;
    }

    vp_reset(vp);
    pci_enable_busmaster(pci);
    vp_سیٹ_حالت(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER );
}

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

#ضمن "config.h" // CONFIG_DEBUG_LEVEL
#ضمن "malloc.h" // free
#ضمن "output.h" // dprintf
#ضمن "pci.h" // pci_config_readl
#ضمن "pcidevice.h" // struct pci_device
#ضمن "pci_regs.h" // PCI_BASE_ADDRESS_0
#ضمن "string.h" // memset
#ضمن "virtio-pci.h"
#ضمن "virtio-mmio.h"
#ضمن "virtio-ring.h"

u64 _vp_read(بنية vp_cap *cap, u32 offset, u8 size)
{
    u64 var = 0;

    بدل (cap->mode) {
    حالة VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        بدل (size) {
        حالة 8:
            var = inl(addr);
            var |= (u64)inl(addr+4) << 32;
            اكسر;
        حالة 4:
            var = inl(addr);
            اكسر;
        حالة 2:
            var = inw(addr);
            اكسر;
        حالة 1:
            var = inb(addr);
            اكسر;
        }
        اكسر;
    }

    حالة VP_ACCESS_MMIO:
    {
        فراغ *addr = cap->memaddr + offset;
        بدل (size) {
        حالة 8:
            var = readl(addr);
            var |= (u64)readl(addr+4) << 32;
            اكسر;
        حالة 4:
            var = readl(addr);
            اكسر;
        حالة 2:
            var = readw(addr);
            اكسر;
        حالة 1:
            var = readb(addr);
            اكسر;
        }
        اكسر;
    }

    حالة VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(بنية virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(بنية virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(بنية virtio_pci_cfg_cap, cap.length),
                          (size > 4) ? 4 : size);
        بدل (size) {
        حالة 8:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(بنية virtio_pci_cfg_cap, pci_cfg_data));
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(بنية virtio_pci_cfg_cap, cap.offset),
                              addr + 4);
            var |= (u64)pci_config_readl(cap->bdf, cap->cfg +
                                         offsetof(بنية virtio_pci_cfg_cap, pci_cfg_data)) << 32;
            اكسر;
        حالة 4:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(بنية virtio_pci_cfg_cap, pci_cfg_data));
            اكسر;
        حالة 2:
            var = pci_config_readw(cap->bdf, cap->cfg +
                                   offsetof(بنية virtio_pci_cfg_cap, pci_cfg_data));
            اكسر;
        حالة 1:
            var = pci_config_readb(cap->bdf, cap->cfg +
                                   offsetof(بنية virtio_pci_cfg_cap, pci_cfg_data));
            اكسر;
        }
    }
    }
    dprintf(9, "vp read   %x (%d) -> 0x%llx\n", cap->ioaddr + offset, size, var);
    أرجع var;
}

فراغ _vp_write(بنية vp_cap *cap, u32 offset, u8 size, u64 var)
{
    dprintf(9, "vp write  %x (%d) <- 0x%llx\n", cap->ioaddr + offset, size, var);

    بدل (cap->mode) {
    حالة VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        بدل (size) {
        حالة 4:
            outl(var, addr);
            اكسر;
        حالة 2:
            outw(var, addr);
            اكسر;
        حالة 1:
            outb(var, addr);
            اكسر;
        }
        اكسر;
    }

    حالة VP_ACCESS_MMIO:
    {
        فراغ *addr = cap->memaddr + offset;
        بدل (size) {
        حالة 4:
            writel(addr, var);
            اكسر;
        حالة 2:
            writew(addr, var);
            اكسر;
        حالة 1:
            writeb(addr, var);
            اكسر;
        }
        اكسر;
    }

    حالة VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(بنية virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(بنية virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(بنية virtio_pci_cfg_cap, cap.length),
                          size);
        بدل (size) {
        حالة 4:
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(بنية virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            اكسر;
        حالة 2:
            pci_config_writew(cap->bdf, cap->cfg +
                              offsetof(بنية virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            اكسر;
        حالة 1:
            pci_config_writeb(cap->bdf, cap->cfg +
                              offsetof(بنية virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            اكسر;
        }
    }
    }
}

u64 vp_get_features(بنية vp_device *vp)
{
    u32 f0, f1;

    إذا (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
    } وإلا إذا (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
    } وإلا {
        f0 = vp_read(&vp->legacy, virtio_pci_legacy, host_features);
        f1 = 0;
    }
    أرجع ((u64)f1 << 32) | f0;
}

فراغ vp_set_features(بنية vp_device *vp, u64 features)
{
    u32 f0, f1;

    f0 = features;
    f1 = features >> 32;

    إذا (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f1);
    } وإلا إذا (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f1);
    } وإلا {
        vp_write(&vp->legacy, virtio_pci_legacy, guest_features, f0);
    }
}

u8 vp_get_status(بنية vp_device *vp)
{
    إذا (vp->use_mmio) {
        أرجع vp_read(&vp->common, virtio_mmio_cfg, device_status);
    } وإلا إذا (vp->use_modern) {
        أرجع vp_read(&vp->common, virtio_pci_common_cfg, device_status);
    } وإلا {
        أرجع vp_read(&vp->legacy, virtio_pci_legacy, status);
    }
}

فراغ vp_set_status(بنية vp_device *vp, u8 status)
{
    إذا (status == 0)        /* reset */
        أرجع;
    إذا (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, status);
    } وإلا إذا (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, status);
    } وإلا {
        vp_write(&vp->legacy, virtio_pci_legacy, status, status);
    }
}

u8 vp_get_isr(بنية vp_device *vp)
{
    إذا (vp->use_mmio) {
        أرجع vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } وإلا إذا (vp->use_modern) {
        أرجع vp_read(&vp->isr, virtio_pci_isr, isr);
    } وإلا {
        أرجع vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

فراغ vp_reset(بنية vp_device *vp)
{
    إذا (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, 0);
        vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } وإلا إذا (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, 0);
        vp_read(&vp->isr, virtio_pci_isr, isr);
    } وإلا {
        vp_write(&vp->legacy, virtio_pci_legacy, status, 0);
        vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

فراغ vp_notify(بنية vp_device *vp, بنية vring_virtqueue *vq)
{
    إذا (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, queue_notify, vq->queue_index);
    } وإلا إذا (vp->use_modern) {
        u32 offset = vq->queue_notify_off * vp->notify_off_multiplier;
        بدل (vp->notify.mode) {
        حالة VP_ACCESS_IO:
            outw(vq->queue_index, vp->notify.ioaddr + offset);
            اكسر;
        حالة VP_ACCESS_MMIO:
            writew(vp->notify.memaddr + offset, vq->queue_index);
            اكسر;
        حالة VP_ACCESS_PCICFG:
            pci_config_writeb(vp->notify.bdf, vp->notify.cfg +
                              offsetof(بنية virtio_pci_cfg_cap, cap.bar),
                              vp->notify.bar);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(بنية virtio_pci_cfg_cap, cap.offset),
                              vp->notify.baroff + offset);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(بنية virtio_pci_cfg_cap, cap.length),
                              2);
            pci_config_writew(vp->notify.bdf, vp->notify.cfg +
                              offsetof(بنية virtio_pci_cfg_cap, pci_cfg_data),
                              vq->queue_index);
        }
        dprintf(9, "vp notify %x (%d) -- 0x%x\n",
                vp->notify.ioaddr, 2, vq->queue_index);
    } وإلا {
        vp_write(&vp->legacy, virtio_pci_legacy, queue_notify, vq->queue_index);
    }
}

صحيح vp_find_vq(بنية vp_device *vp, صحيح queue_index,
               بنية vring_virtqueue **p_vq)
{
   u16 عدد;

   ASSERT32FLAT();
   بنية vring_virtqueue *vq = *p_vq = memalign_بلند(PAGE_SIZE, حجم(*vq));
   إذا (!vq) {
       warn_noalloc();
       اذهب fail;
   }
   memset(vq, 0, حجم(*vq));


   /* select the queue */
   إذا (vp->use_mmio) {
       vp_write(&vp->common, virtio_mmio_cfg, queue_select, queue_index);
   } وإلا إذا (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, queue_select, queue_index);
   } وإلا {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_sel, queue_index);
   }

   /* check if the queue is available */
   إذا (vp->use_mmio) {
       عدد = vp_read(&vp->common, virtio_mmio_cfg, queue_عدد_max);
       إذا (عدد > MAX_QUEUE_عدد)
           عدد = MAX_QUEUE_عدد;
       vp_write(&vp->common, virtio_mmio_cfg, queue_عدد, عدد);
   } وإلا إذا (vp->use_modern) {
       عدد = vp_read(&vp->common, virtio_pci_common_cfg, queue_size);
       إذا (عدد > MAX_QUEUE_عدد) {
           vp_write(&vp->common, virtio_pci_common_cfg, queue_size,
                    MAX_QUEUE_عدد);
           عدد = vp_read(&vp->common, virtio_pci_common_cfg, queue_size);
       }
   } وإلا {
       عدد = vp_read(&vp->legacy, virtio_pci_legacy, queue_عدد);
   }
   إذا (!عدد) {
       dprintf(1, "ERROR: queue size is 0\n");
       اذهب fail;
   }
   إذا (عدد > MAX_QUEUE_عدد) {
       dprintf(1, "ERROR: queue size %d > %d\n", عدد, MAX_QUEUE_عدد);
       اذهب fail;
   }

   /* check if the queue is already active */
   إذا (vp->use_mmio) {
       /* TODO */;
   } وإلا إذا (vp->use_modern) {
       إذا (vp_read(&vp->common, virtio_pci_common_cfg, queue_enable)) {
           dprintf(1, "ERROR: queue already active\n");
           اذهب fail;
       }
   } وإلا {
       إذا (vp_read(&vp->legacy, virtio_pci_legacy, queue_pfn)) {
           dprintf(1, "ERROR: queue already active\n");
           اذهب fail;
       }
   }
   vq->queue_index = queue_index;

   /* initialize the queue */
   بنية vring * vr = &vq->vring;
   vring_init(vr, عدد, (unsigned حرف*)&vq->queue);

   /* activate the queue
    *
    * NOTE: vr->desc is initialized by vring_init()
    */

   إذا (vp->use_mmio) {
       إذا (vp_read(&vp->common, virtio_mmio_cfg, version) == 2) {
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
       } وإلا {
           vp_write(&vp->common, virtio_mmio_cfg, legacy_guest_page_size,
                    (unsigned long)1 << PAGE_SHIFT);
           vp_write(&vp->common, virtio_mmio_cfg, legacy_queue_pfn,
                    (unsigned long)virt_to_phys(vr->desc) >> PAGE_SHIFT);
       }
   } وإلا إذا (vp->use_modern) {
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
   } وإلا {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_pfn,
                (unsigned long)virt_to_phys(vr->desc) >> PAGE_SHIFT);
   }
   أرجع عدد;

fail:
   حرر(vq);
   *p_vq = عدم;
   أرجع -1;
}

فراغ vp_init_simple(بنية vp_device *vp, بنية pci_device *pci)
{
    u8 cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, 0);
    بنية vp_cap *vp_cap;
    ثابت حرف *mode;
    u32 offset, base, mul;
    u64 addr;
    u8 type;

    memset(vp, 0, حجم(*vp));
    طالما (cap != 0) {
        type = pci_config_readb(pci->bdf, cap +
                                offsetof(بنية virtio_pci_cap, cfg_type));
        بدل (type) {
        حالة VIRTIO_PCI_CAP_COMMON_CFG:
            vp_cap = &vp->common;
            اكسر;
        حالة VIRTIO_PCI_CAP_NOTIFY_CFG:
            vp_cap = &vp->notify;
            mul = offsetof(بنية virtio_pci_notify_cap, notify_off_multiplier);
            vp->notify_off_multiplier = pci_config_readl(pci->bdf, cap + mul);
            اكسر;
        حالة VIRTIO_PCI_CAP_ISR_CFG:
            vp_cap = &vp->isr;
            اكسر;
        حالة VIRTIO_PCI_CAP_DEVICE_CFG:
            vp_cap = &vp->device;
            اكسر;
        حالة VIRTIO_PCI_CAP_PCI_CFG:
            vp->common.cfg = cap;
            vp->common.bdf = pci->bdf;
            vp->notify.cfg = cap;
            vp->notify.bdf = pci->bdf;
            vp->isr.cfg = cap;
            vp->isr.bdf = pci->bdf;
            vp->device.cfg = cap;
            vp->device.bdf = pci->bdf;
            vp_cap = عدم;
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d"
                    " [pci cfg access]\n", pci, cap, type);
            اكسر;
        افتراضي:
            vp_cap = عدم;
            اكسر;
        }
        إذا (vp_cap && !vp_cap->cap) {
            vp_cap->cap = cap;
            vp_cap->bar = pci_config_readb(pci->bdf, cap +
                                           offsetof(بنية virtio_pci_cap, bar));
            offset = pci_config_readl(pci->bdf, cap +
                                      offsetof(بنية virtio_pci_cap, offset));
            base = PCI_BASE_ADDRESS_0 + 4 * vp_cap->bar;
            addr = pci_config_readl(pci->bdf, base);
            إذا (addr & PCI_BASE_ADDRESS_SPACE_IO) {
                addr &= PCI_BASE_ADDRESS_IO_MASK;
                vp_cap->mode = VP_ACCESS_IO;
            } وإلا إذا ((addr & PCI_BASE_ADDRESS_MEM_TYPE_MASK) ==
                       PCI_BASE_ADDRESS_MEM_TYPE_64) {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                addr |= (u64)pci_config_readl(pci->bdf, base + 4) << 32;
                vp_cap->mode = (addr > 0xffffffffll) ?
                    VP_ACCESS_PCICFG : VP_ACCESS_MMIO;
            } وإلا {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                vp_cap->mode = VP_ACCESS_MMIO;
            }
            بدل (vp_cap->mode) {
            حالة VP_ACCESS_IO:
            {
                u32 addr = pci_enable_iobar(pci, base);
                إذا (!addr)
                    أرجع;
                vp_cap->ioaddr = addr + offset;
                mode = "io";
                اكسر;
            }
            حالة VP_ACCESS_MMIO:
            {
                فراغ *addr = pci_enable_membar(pci, base);
                إذا (!addr)
                    أرجع;
                vp_cap->memaddr = addr + offset;
                mode = "mmio";
                اكسر;
            }
            حالة VP_ACCESS_PCICFG:
                mode = "pcicfg";
                vp_cap->baroff = offset;
                اكسر;
            افتراضي:
                mode = "Huh?";
                اكسر;
            }
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d "
                    "bar %d at 0x%08llx off +0x%04x [%s]\n",
                    pci, vp_cap->cap, type, vp_cap->bar, addr, offset, mode);
        }

        cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, cap);
    }

    إذا (vp->common.cap && vp->notify.cap && vp->isr.cap && vp->device.cap) {
        dprintf(1, "pci dev %pP using modern (1.0) virtio mode\n", pci);
        vp->use_modern = 1;
    } وإلا {
        dprintf(1, "pci dev %pP using legacy (0.9.5) virtio mode\n", pci);
        vp->legacy.bar = 0;
        vp->legacy.ioaddr = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        إذا (!vp->legacy.ioaddr)
            أرجع;
        vp->legacy.mode = VP_ACCESS_IO;
    }

    vp_reset(vp);
    pci_enable_busmaster(pci);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER );
}

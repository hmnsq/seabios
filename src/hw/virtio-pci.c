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

u64 _vp_read(ساختار vp_cap *cap, u32 offset, u8 size)
{
    u64 var = 0;

    انتخاب (cap->mode) {
    حالت VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        انتخاب (size) {
        حالت 8:
            var = inl(addr);
            var |= (u64)inl(addr+4) << 32;
            بشکن;
        حالت 4:
            var = inl(addr);
            بشکن;
        حالت 2:
            var = inw(addr);
            بشکن;
        حالت 1:
            var = inb(addr);
            بشکن;
        }
        بشکن;
    }

    حالت VP_ACCESS_MMIO:
    {
        تهی *addr = cap->memaddr + offset;
        انتخاب (size) {
        حالت 8:
            var = readl(addr);
            var |= (u64)readl(addr+4) << 32;
            بشکن;
        حالت 4:
            var = readl(addr);
            بشکن;
        حالت 2:
            var = readw(addr);
            بشکن;
        حالت 1:
            var = readb(addr);
            بشکن;
        }
        بشکن;
    }

    حالت VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(ساختار virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ساختار virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ساختار virtio_pci_cfg_cap, cap.length),
                          (size > 4) ? 4 : size);
        انتخاب (size) {
        حالت 8:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(ساختار virtio_pci_cfg_cap, pci_cfg_data));
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(ساختار virtio_pci_cfg_cap, cap.offset),
                              addr + 4);
            var |= (u64)pci_config_readl(cap->bdf, cap->cfg +
                                         offsetof(ساختار virtio_pci_cfg_cap, pci_cfg_data)) << 32;
            بشکن;
        حالت 4:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(ساختار virtio_pci_cfg_cap, pci_cfg_data));
            بشکن;
        حالت 2:
            var = pci_config_readw(cap->bdf, cap->cfg +
                                   offsetof(ساختار virtio_pci_cfg_cap, pci_cfg_data));
            بشکن;
        حالت 1:
            var = pci_config_readb(cap->bdf, cap->cfg +
                                   offsetof(ساختار virtio_pci_cfg_cap, pci_cfg_data));
            بشکن;
        }
    }
    }
    dprintf(9, "vp read   %x (%d) -> 0x%llx\n", cap->ioaddr + offset, size, var);
    بازگشت var;
}

تهی _vp_write(ساختار vp_cap *cap, u32 offset, u8 size, u64 var)
{
    dprintf(9, "vp write  %x (%d) <- 0x%llx\n", cap->ioaddr + offset, size, var);

    انتخاب (cap->mode) {
    حالت VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        انتخاب (size) {
        حالت 4:
            outl(var, addr);
            بشکن;
        حالت 2:
            outw(var, addr);
            بشکن;
        حالت 1:
            outb(var, addr);
            بشکن;
        }
        بشکن;
    }

    حالت VP_ACCESS_MMIO:
    {
        تهی *addr = cap->memaddr + offset;
        انتخاب (size) {
        حالت 4:
            writel(addr, var);
            بشکن;
        حالت 2:
            writew(addr, var);
            بشکن;
        حالت 1:
            writeb(addr, var);
            بشکن;
        }
        بشکن;
    }

    حالت VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(ساختار virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ساختار virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ساختار virtio_pci_cfg_cap, cap.length),
                          size);
        انتخاب (size) {
        حالت 4:
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(ساختار virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            بشکن;
        حالت 2:
            pci_config_writew(cap->bdf, cap->cfg +
                              offsetof(ساختار virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            بشکن;
        حالت 1:
            pci_config_writeb(cap->bdf, cap->cfg +
                              offsetof(ساختار virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            بشکن;
        }
    }
    }
}

u64 vp_get_features(ساختار vp_device *vp)
{
    u32 f0, f1;

    اگر (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
    } وگرنه اگر (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
    } وگرنه {
        f0 = vp_read(&vp->legacy, virtio_pci_legacy, host_features);
        f1 = 0;
    }
    بازگشت ((u64)f1 << 32) | f0;
}

تهی vp_set_features(ساختار vp_device *vp, u64 features)
{
    u32 f0, f1;

    f0 = features;
    f1 = features >> 32;

    اگر (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f1);
    } وگرنه اگر (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f1);
    } وگرنه {
        vp_write(&vp->legacy, virtio_pci_legacy, guest_features, f0);
    }
}

u8 vp_get_status(ساختار vp_device *vp)
{
    اگر (vp->use_mmio) {
        بازگشت vp_read(&vp->common, virtio_mmio_cfg, device_status);
    } وگرنه اگر (vp->use_modern) {
        بازگشت vp_read(&vp->common, virtio_pci_common_cfg, device_status);
    } وگرنه {
        بازگشت vp_read(&vp->legacy, virtio_pci_legacy, status);
    }
}

تهی vp_set_status(ساختار vp_device *vp, u8 status)
{
    اگر (status == 0)        /* reset */
        بازگشت;
    اگر (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, status);
    } وگرنه اگر (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, status);
    } وگرنه {
        vp_write(&vp->legacy, virtio_pci_legacy, status, status);
    }
}

u8 vp_get_isr(ساختار vp_device *vp)
{
    اگر (vp->use_mmio) {
        بازگشت vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } وگرنه اگر (vp->use_modern) {
        بازگشت vp_read(&vp->isr, virtio_pci_isr, isr);
    } وگرنه {
        بازگشت vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

تهی vp_reset(ساختار vp_device *vp)
{
    اگر (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, 0);
        vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } وگرنه اگر (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, 0);
        vp_read(&vp->isr, virtio_pci_isr, isr);
    } وگرنه {
        vp_write(&vp->legacy, virtio_pci_legacy, status, 0);
        vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

تهی vp_notify(ساختار vp_device *vp, ساختار vring_virtqueue *vq)
{
    اگر (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, queue_notify, vq->queue_index);
    } وگرنه اگر (vp->use_modern) {
        u32 offset = vq->queue_notify_off * vp->notify_off_multiplier;
        انتخاب (vp->notify.mode) {
        حالت VP_ACCESS_IO:
            outw(vq->queue_index, vp->notify.ioaddr + offset);
            بشکن;
        حالت VP_ACCESS_MMIO:
            writew(vp->notify.memaddr + offset, vq->queue_index);
            بشکن;
        حالت VP_ACCESS_PCICFG:
            pci_config_writeb(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ساختار virtio_pci_cfg_cap, cap.bar),
                              vp->notify.bar);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ساختار virtio_pci_cfg_cap, cap.offset),
                              vp->notify.baroff + offset);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ساختار virtio_pci_cfg_cap, cap.length),
                              2);
            pci_config_writew(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ساختار virtio_pci_cfg_cap, pci_cfg_data),
                              vq->queue_index);
        }
        dprintf(9, "vp notify %x (%d) -- 0x%x\n",
                vp->notify.ioaddr, 2, vq->queue_index);
    } وگرنه {
        vp_write(&vp->legacy, virtio_pci_legacy, queue_notify, vq->queue_index);
    }
}

صحیح vp_find_vq(ساختار vp_device *vp, صحیح queue_index,
               ساختار vring_virtqueue **p_vq)
{
   u16 رقم;

   ASSERT32FLAT();
   ساختار vring_virtqueue *vq = *p_vq = memalign_بالا(PAGE_SIZE, اندازه(*vq));
   اگر (!vq) {
       warn_noalloc();
       برو fail;
   }
   memset(vq, 0, اندازه(*vq));


   /* select the queue */
   اگر (vp->use_mmio) {
       vp_write(&vp->common, virtio_mmio_cfg, queue_select, queue_index);
   } وگرنه اگر (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, queue_select, queue_index);
   } وگرنه {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_sel, queue_index);
   }

   /* check if the queue is available */
   اگر (vp->use_mmio) {
       رقم = vp_read(&vp->common, virtio_mmio_cfg, queue_رقم_max);
       اگر (رقم > MAX_QUEUE_رقم)
           رقم = MAX_QUEUE_رقم;
       vp_write(&vp->common, virtio_mmio_cfg, queue_رقم, رقم);
   } وگرنه اگر (vp->use_modern) {
       رقم = vp_read(&vp->common, virtio_pci_common_cfg, queue_size);
       اگر (رقم > MAX_QUEUE_رقم) {
           vp_write(&vp->common, virtio_pci_common_cfg, queue_size,
                    MAX_QUEUE_رقم);
           رقم = vp_read(&vp->common, virtio_pci_common_cfg, queue_size);
       }
   } وگرنه {
       رقم = vp_read(&vp->legacy, virtio_pci_legacy, queue_رقم);
   }
   اگر (!رقم) {
       dprintf(1, "ERROR: queue size is 0\n");
       برو fail;
   }
   اگر (رقم > MAX_QUEUE_رقم) {
       dprintf(1, "ERROR: queue size %d > %d\n", رقم, MAX_QUEUE_رقم);
       برو fail;
   }

   /* check if the queue is already active */
   اگر (vp->use_mmio) {
       /* TODO */;
   } وگرنه اگر (vp->use_modern) {
       اگر (vp_read(&vp->common, virtio_pci_common_cfg, queue_enable)) {
           dprintf(1, "ERROR: queue already active\n");
           برو fail;
       }
   } وگرنه {
       اگر (vp_read(&vp->legacy, virtio_pci_legacy, queue_pfn)) {
           dprintf(1, "ERROR: queue already active\n");
           برو fail;
       }
   }
   vq->queue_index = queue_index;

   /* initialize the queue */
   ساختار vring * vr = &vq->vring;
   vring_init(vr, رقم, (unsigned نویسه*)&vq->queue);

   /* activate the queue
    *
    * NOTE: vr->desc is initialized by vring_init()
    */

   اگر (vp->use_mmio) {
       اگر (vp_read(&vp->common, virtio_mmio_cfg, version) == 2) {
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
       } وگرنه {
           vp_write(&vp->common, virtio_mmio_cfg, legacy_guest_page_size,
                    (unsigned long)1 << PAGE_SHIFT);
           vp_write(&vp->common, virtio_mmio_cfg, legacy_queue_pfn,
                    (unsigned long)virt_to_phys(vr->desc) >> PAGE_SHIFT);
       }
   } وگرنه اگر (vp->use_modern) {
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
   } وگرنه {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_pfn,
                (unsigned long)virt_to_phys(vr->desc) >> PAGE_SHIFT);
   }
   بازگشت رقم;

fail:
   آزاد(vq);
   *p_vq = پوچ;
   بازگشت -1;
}

تهی vp_init_simple(ساختار vp_device *vp, ساختار pci_device *pci)
{
    u8 cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, 0);
    ساختار vp_cap *vp_cap;
    ثابت نویسه *mode;
    u32 offset, base, mul;
    u64 addr;
    u8 type;

    memset(vp, 0, اندازه(*vp));
    وقتی (cap != 0) {
        type = pci_config_readb(pci->bdf, cap +
                                offsetof(ساختار virtio_pci_cap, cfg_type));
        انتخاب (type) {
        حالت VIRTIO_PCI_CAP_COMMON_CFG:
            vp_cap = &vp->common;
            بشکن;
        حالت VIRTIO_PCI_CAP_NOTIFY_CFG:
            vp_cap = &vp->notify;
            mul = offsetof(ساختار virtio_pci_notify_cap, notify_off_multiplier);
            vp->notify_off_multiplier = pci_config_readl(pci->bdf, cap + mul);
            بشکن;
        حالت VIRTIO_PCI_CAP_ISR_CFG:
            vp_cap = &vp->isr;
            بشکن;
        حالت VIRTIO_PCI_CAP_DEVICE_CFG:
            vp_cap = &vp->device;
            بشکن;
        حالت VIRTIO_PCI_CAP_PCI_CFG:
            vp->common.cfg = cap;
            vp->common.bdf = pci->bdf;
            vp->notify.cfg = cap;
            vp->notify.bdf = pci->bdf;
            vp->isr.cfg = cap;
            vp->isr.bdf = pci->bdf;
            vp->device.cfg = cap;
            vp->device.bdf = pci->bdf;
            vp_cap = پوچ;
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d"
                    " [pci cfg access]\n", pci, cap, type);
            بشکن;
        پیشفرض:
            vp_cap = پوچ;
            بشکن;
        }
        اگر (vp_cap && !vp_cap->cap) {
            vp_cap->cap = cap;
            vp_cap->bar = pci_config_readb(pci->bdf, cap +
                                           offsetof(ساختار virtio_pci_cap, bar));
            offset = pci_config_readl(pci->bdf, cap +
                                      offsetof(ساختار virtio_pci_cap, offset));
            base = PCI_BASE_ADDRESS_0 + 4 * vp_cap->bar;
            addr = pci_config_readl(pci->bdf, base);
            اگر (addr & PCI_BASE_ADDRESS_SPACE_IO) {
                addr &= PCI_BASE_ADDRESS_IO_MASK;
                vp_cap->mode = VP_ACCESS_IO;
            } وگرنه اگر ((addr & PCI_BASE_ADDRESS_MEM_TYPE_MASK) ==
                       PCI_BASE_ADDRESS_MEM_TYPE_64) {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                addr |= (u64)pci_config_readl(pci->bdf, base + 4) << 32;
                vp_cap->mode = (addr > 0xffffffffll) ?
                    VP_ACCESS_PCICFG : VP_ACCESS_MMIO;
            } وگرنه {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                vp_cap->mode = VP_ACCESS_MMIO;
            }
            انتخاب (vp_cap->mode) {
            حالت VP_ACCESS_IO:
            {
                u32 addr = pci_enable_iobar(pci, base);
                اگر (!addr)
                    بازگشت;
                vp_cap->ioaddr = addr + offset;
                mode = "io";
                بشکن;
            }
            حالت VP_ACCESS_MMIO:
            {
                تهی *addr = pci_enable_membar(pci, base);
                اگر (!addr)
                    بازگشت;
                vp_cap->memaddr = addr + offset;
                mode = "mmio";
                بشکن;
            }
            حالت VP_ACCESS_PCICFG:
                mode = "pcicfg";
                vp_cap->baroff = offset;
                بشکن;
            پیشفرض:
                mode = "Huh?";
                بشکن;
            }
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d "
                    "bar %d at 0x%08llx off +0x%04x [%s]\n",
                    pci, vp_cap->cap, type, vp_cap->bar, addr, offset, mode);
        }

        cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, cap);
    }

    اگر (vp->common.cap && vp->notify.cap && vp->isr.cap && vp->device.cap) {
        dprintf(1, "pci dev %pP using modern (1.0) virtio mode\n", pci);
        vp->use_modern = 1;
    } وگرنه {
        dprintf(1, "pci dev %pP using legacy (0.9.5) virtio mode\n", pci);
        vp->legacy.bar = 0;
        vp->legacy.ioaddr = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        اگر (!vp->legacy.ioaddr)
            بازگشت;
        vp->legacy.mode = VP_ACCESS_IO;
    }

    vp_reset(vp);
    pci_enable_busmaster(pci);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER );
}

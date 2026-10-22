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

#അന്തര്ഭാവ "config.h" // CONFIG_DEBUG_LEVEL
#അന്തര്ഭാവ "malloc.h" // free
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "pci.h" // pci_config_readl
#അന്തര്ഭാവ "pcidevice.h" // struct pci_device
#അന്തര്ഭാവ "pci_regs.h" // PCI_BASE_ADDRESS_0
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "virtio-pci.h"
#അന്തര്ഭാവ "virtio-mmio.h"
#അന്തര്ഭാവ "virtio-ring.h"

u64 _vp_read(ഘടന vp_cap *cap, u32 offset, u8 size)
{
    u64 var = 0;

    തിരഞ്ഞെടുപ്പ് (cap->mode) {
    അവസ്ഥ VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        തിരഞ്ഞെടുപ്പ് (size) {
        അവസ്ഥ 8:
            var = inl(addr);
            var |= (u64)inl(addr+4) << 32;
            നിർത്തുക;
        അവസ്ഥ 4:
            var = inl(addr);
            നിർത്തുക;
        അവസ്ഥ 2:
            var = inw(addr);
            നിർത്തുക;
        അവസ്ഥ 1:
            var = inb(addr);
            നിർത്തുക;
        }
        നിർത്തുക;
    }

    അവസ്ഥ VP_ACCESS_MMIO:
    {
        ശൂന്യം *addr = cap->memaddr + offset;
        തിരഞ്ഞെടുപ്പ് (size) {
        അവസ്ഥ 8:
            var = readl(addr);
            var |= (u64)readl(addr+4) << 32;
            നിർത്തുക;
        അവസ്ഥ 4:
            var = readl(addr);
            നിർത്തുക;
        അവസ്ഥ 2:
            var = readw(addr);
            നിർത്തുക;
        അവസ്ഥ 1:
            var = readb(addr);
            നിർത്തുക;
        }
        നിർത്തുക;
    }

    അവസ്ഥ VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(ഘടന virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ഘടന virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ഘടന virtio_pci_cfg_cap, cap.length),
                          (size > 4) ? 4 : size);
        തിരഞ്ഞെടുപ്പ് (size) {
        അവസ്ഥ 8:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(ഘടന virtio_pci_cfg_cap, pci_cfg_data));
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(ഘടന virtio_pci_cfg_cap, cap.offset),
                              addr + 4);
            var |= (u64)pci_config_readl(cap->bdf, cap->cfg +
                                         offsetof(ഘടന virtio_pci_cfg_cap, pci_cfg_data)) << 32;
            നിർത്തുക;
        അവസ്ഥ 4:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(ഘടന virtio_pci_cfg_cap, pci_cfg_data));
            നിർത്തുക;
        അവസ്ഥ 2:
            var = pci_config_readw(cap->bdf, cap->cfg +
                                   offsetof(ഘടന virtio_pci_cfg_cap, pci_cfg_data));
            നിർത്തുക;
        അവസ്ഥ 1:
            var = pci_config_readb(cap->bdf, cap->cfg +
                                   offsetof(ഘടന virtio_pci_cfg_cap, pci_cfg_data));
            നിർത്തുക;
        }
    }
    }
    dprintf(9, "vp read   %x (%d) -> 0x%llx\n", cap->ioaddr + offset, size, var);
    തിരികെ var;
}

ശൂന്യം _vp_write(ഘടന vp_cap *cap, u32 offset, u8 size, u64 var)
{
    dprintf(9, "vp write  %x (%d) <- 0x%llx\n", cap->ioaddr + offset, size, var);

    തിരഞ്ഞെടുപ്പ് (cap->mode) {
    അവസ്ഥ VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        തിരഞ്ഞെടുപ്പ് (size) {
        അവസ്ഥ 4:
            outl(var, addr);
            നിർത്തുക;
        അവസ്ഥ 2:
            outw(var, addr);
            നിർത്തുക;
        അവസ്ഥ 1:
            outb(var, addr);
            നിർത്തുക;
        }
        നിർത്തുക;
    }

    അവസ്ഥ VP_ACCESS_MMIO:
    {
        ശൂന്യം *addr = cap->memaddr + offset;
        തിരഞ്ഞെടുപ്പ് (size) {
        അവസ്ഥ 4:
            writel(addr, var);
            നിർത്തുക;
        അവസ്ഥ 2:
            writew(addr, var);
            നിർത്തുക;
        അവസ്ഥ 1:
            writeb(addr, var);
            നിർത്തുക;
        }
        നിർത്തുക;
    }

    അവസ്ഥ VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(ഘടന virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ഘടന virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ഘടന virtio_pci_cfg_cap, cap.length),
                          size);
        തിരഞ്ഞെടുപ്പ് (size) {
        അവസ്ഥ 4:
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(ഘടന virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            നിർത്തുക;
        അവസ്ഥ 2:
            pci_config_writew(cap->bdf, cap->cfg +
                              offsetof(ഘടന virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            നിർത്തുക;
        അവസ്ഥ 1:
            pci_config_writeb(cap->bdf, cap->cfg +
                              offsetof(ഘടന virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            നിർത്തുക;
        }
    }
    }
}

u64 vp_get_features(ഘടന vp_device *vp)
{
    u32 f0, f1;

    എങ്കിൽ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
    } അല്ലെങ്കിൽ എങ്കിൽ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
    } അല്ലെങ്കിൽ {
        f0 = vp_read(&vp->legacy, virtio_pci_legacy, host_features);
        f1 = 0;
    }
    തിരികെ ((u64)f1 << 32) | f0;
}

ശൂന്യം vp_set_features(ഘടന vp_device *vp, u64 features)
{
    u32 f0, f1;

    f0 = features;
    f1 = features >> 32;

    എങ്കിൽ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f1);
    } അല്ലെങ്കിൽ എങ്കിൽ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f1);
    } അല്ലെങ്കിൽ {
        vp_write(&vp->legacy, virtio_pci_legacy, guest_features, f0);
    }
}

u8 vp_get_status(ഘടന vp_device *vp)
{
    എങ്കിൽ (vp->use_mmio) {
        തിരികെ vp_read(&vp->common, virtio_mmio_cfg, device_status);
    } അല്ലെങ്കിൽ എങ്കിൽ (vp->use_modern) {
        തിരികെ vp_read(&vp->common, virtio_pci_common_cfg, device_status);
    } അല്ലെങ്കിൽ {
        തിരികെ vp_read(&vp->legacy, virtio_pci_legacy, status);
    }
}

ശൂന്യം vp_set_status(ഘടന vp_device *vp, u8 status)
{
    എങ്കിൽ (status == 0)        /* reset */
        തിരികെ;
    എങ്കിൽ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, status);
    } അല്ലെങ്കിൽ എങ്കിൽ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, status);
    } അല്ലെങ്കിൽ {
        vp_write(&vp->legacy, virtio_pci_legacy, status, status);
    }
}

u8 vp_get_isr(ഘടന vp_device *vp)
{
    എങ്കിൽ (vp->use_mmio) {
        തിരികെ vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } അല്ലെങ്കിൽ എങ്കിൽ (vp->use_modern) {
        തിരികെ vp_read(&vp->isr, virtio_pci_isr, isr);
    } അല്ലെങ്കിൽ {
        തിരികെ vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

ശൂന്യം vp_reset(ഘടന vp_device *vp)
{
    എങ്കിൽ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, 0);
        vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } അല്ലെങ്കിൽ എങ്കിൽ (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, 0);
        vp_read(&vp->isr, virtio_pci_isr, isr);
    } അല്ലെങ്കിൽ {
        vp_write(&vp->legacy, virtio_pci_legacy, status, 0);
        vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

ശൂന്യം vp_notify(ഘടന vp_device *vp, ഘടന vring_virtqueue *vq)
{
    എങ്കിൽ (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, queue_notify, vq->queue_index);
    } അല്ലെങ്കിൽ എങ്കിൽ (vp->use_modern) {
        u32 offset = vq->queue_notify_off * vp->notify_off_multiplier;
        തിരഞ്ഞെടുപ്പ് (vp->notify.mode) {
        അവസ്ഥ VP_ACCESS_IO:
            outw(vq->queue_index, vp->notify.ioaddr + offset);
            നിർത്തുക;
        അവസ്ഥ VP_ACCESS_MMIO:
            writew(vp->notify.memaddr + offset, vq->queue_index);
            നിർത്തുക;
        അവസ്ഥ VP_ACCESS_PCICFG:
            pci_config_writeb(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ഘടന virtio_pci_cfg_cap, cap.bar),
                              vp->notify.bar);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ഘടന virtio_pci_cfg_cap, cap.offset),
                              vp->notify.baroff + offset);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ഘടന virtio_pci_cfg_cap, cap.length),
                              2);
            pci_config_writew(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ഘടന virtio_pci_cfg_cap, pci_cfg_data),
                              vq->queue_index);
        }
        dprintf(9, "vp notify %x (%d) -- 0x%x\n",
                vp->notify.ioaddr, 2, vq->queue_index);
    } അല്ലെങ്കിൽ {
        vp_write(&vp->legacy, virtio_pci_legacy, queue_notify, vq->queue_index);
    }
}

പൂർണ്ണസംഖ്യ vp_find_vq(ഘടന vp_device *vp, പൂർണ്ണസംഖ്യ queue_index,
               ഘടന vring_virtqueue **p_vq)
{
   u16 അക്കം;

   ASSERT32FLAT();
   ഘടന vring_virtqueue *vq = *p_vq = memalign_മുകളിലെ(PAGE_SIZE, വലുപ്പം(*vq));
   എങ്കിൽ (!vq) {
       warn_noalloc();
       പോകുക fail;
   }
   memset(vq, 0, വലുപ്പം(*vq));


   /* select the queue */
   എങ്കിൽ (vp->use_mmio) {
       vp_write(&vp->common, virtio_mmio_cfg, queue_select, queue_index);
   } അല്ലെങ്കിൽ എങ്കിൽ (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, queue_select, queue_index);
   } അല്ലെങ്കിൽ {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_sel, queue_index);
   }

   /* check if the queue is available */
   എങ്കിൽ (vp->use_mmio) {
       അക്കം = vp_read(&vp->common, virtio_mmio_cfg, queue_അക്കം_max);
       എങ്കിൽ (അക്കം > MAX_QUEUE_അക്കം)
           അക്കം = MAX_QUEUE_അക്കം;
       vp_write(&vp->common, virtio_mmio_cfg, queue_അക്കം, അക്കം);
   } അല്ലെങ്കിൽ എങ്കിൽ (vp->use_modern) {
       അക്കം = vp_read(&vp->common, virtio_pci_common_cfg, queue_size);
       എങ്കിൽ (അക്കം > MAX_QUEUE_അക്കം) {
           vp_write(&vp->common, virtio_pci_common_cfg, queue_size,
                    MAX_QUEUE_അക്കം);
           അക്കം = vp_read(&vp->common, virtio_pci_common_cfg, queue_size);
       }
   } അല്ലെങ്കിൽ {
       അക്കം = vp_read(&vp->legacy, virtio_pci_legacy, queue_അക്കം);
   }
   എങ്കിൽ (!അക്കം) {
       dprintf(1, "ERROR: queue size is 0\n");
       പോകുക fail;
   }
   എങ്കിൽ (അക്കം > MAX_QUEUE_അക്കം) {
       dprintf(1, "ERROR: queue size %d > %d\n", അക്കം, MAX_QUEUE_അക്കം);
       പോകുക fail;
   }

   /* check if the queue is already active */
   എങ്കിൽ (vp->use_mmio) {
       /* TODO */;
   } അല്ലെങ്കിൽ എങ്കിൽ (vp->use_modern) {
       എങ്കിൽ (vp_read(&vp->common, virtio_pci_common_cfg, queue_enable)) {
           dprintf(1, "ERROR: queue already active\n");
           പോകുക fail;
       }
   } അല്ലെങ്കിൽ {
       എങ്കിൽ (vp_read(&vp->legacy, virtio_pci_legacy, queue_pfn)) {
           dprintf(1, "ERROR: queue already active\n");
           പോകുക fail;
       }
   }
   vq->queue_index = queue_index;

   /* initialize the queue */
   ഘടന vring * vr = &vq->vring;
   vring_init(vr, അക്കം, (unsigned അക്ഷരം*)&vq->queue);

   /* activate the queue
    *
    * NOTE: vr->desc is initialized by vring_init()
    */

   എങ്കിൽ (vp->use_mmio) {
       എങ്കിൽ (vp_read(&vp->common, virtio_mmio_cfg, version) == 2) {
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
       } അല്ലെങ്കിൽ {
           vp_write(&vp->common, virtio_mmio_cfg, legacy_guest_page_size,
                    (unsigned long)1 << PAGE_SHIFT);
           vp_write(&vp->common, virtio_mmio_cfg, legacy_queue_pfn,
                    (unsigned long)virt_to_phys(vr->desc) >> PAGE_SHIFT);
       }
   } അല്ലെങ്കിൽ എങ്കിൽ (vp->use_modern) {
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
   } അല്ലെങ്കിൽ {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_pfn,
                (unsigned long)virt_to_phys(vr->desc) >> PAGE_SHIFT);
   }
   തിരികെ അക്കം;

fail:
   സ്വതന്ത്രം(vq);
   *p_vq = ശൂന്യ;
   തിരികെ -1;
}

ശൂന്യം vp_init_simple(ഘടന vp_device *vp, ഘടന pci_device *pci)
{
    u8 cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, 0);
    ഘടന vp_cap *vp_cap;
    സ്ഥിരം അക്ഷരം *mode;
    u32 offset, base, mul;
    u64 addr;
    u8 type;

    memset(vp, 0, വലുപ്പം(*vp));
    ആകുമ്പോൾ (cap != 0) {
        type = pci_config_readb(pci->bdf, cap +
                                offsetof(ഘടന virtio_pci_cap, cfg_type));
        തിരഞ്ഞെടുപ്പ് (type) {
        അവസ്ഥ VIRTIO_PCI_CAP_COMMON_CFG:
            vp_cap = &vp->common;
            നിർത്തുക;
        അവസ്ഥ VIRTIO_PCI_CAP_NOTIFY_CFG:
            vp_cap = &vp->notify;
            mul = offsetof(ഘടന virtio_pci_notify_cap, notify_off_multiplier);
            vp->notify_off_multiplier = pci_config_readl(pci->bdf, cap + mul);
            നിർത്തുക;
        അവസ്ഥ VIRTIO_PCI_CAP_ISR_CFG:
            vp_cap = &vp->isr;
            നിർത്തുക;
        അവസ്ഥ VIRTIO_PCI_CAP_DEVICE_CFG:
            vp_cap = &vp->device;
            നിർത്തുക;
        അവസ്ഥ VIRTIO_PCI_CAP_PCI_CFG:
            vp->common.cfg = cap;
            vp->common.bdf = pci->bdf;
            vp->notify.cfg = cap;
            vp->notify.bdf = pci->bdf;
            vp->isr.cfg = cap;
            vp->isr.bdf = pci->bdf;
            vp->device.cfg = cap;
            vp->device.bdf = pci->bdf;
            vp_cap = ശൂന്യ;
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d"
                    " [pci cfg access]\n", pci, cap, type);
            നിർത്തുക;
        സ്ഥിരസ്ഥിതി:
            vp_cap = ശൂന്യ;
            നിർത്തുക;
        }
        എങ്കിൽ (vp_cap && !vp_cap->cap) {
            vp_cap->cap = cap;
            vp_cap->bar = pci_config_readb(pci->bdf, cap +
                                           offsetof(ഘടന virtio_pci_cap, bar));
            offset = pci_config_readl(pci->bdf, cap +
                                      offsetof(ഘടന virtio_pci_cap, offset));
            base = PCI_BASE_ADDRESS_0 + 4 * vp_cap->bar;
            addr = pci_config_readl(pci->bdf, base);
            എങ്കിൽ (addr & PCI_BASE_ADDRESS_SPACE_IO) {
                addr &= PCI_BASE_ADDRESS_IO_MASK;
                vp_cap->mode = VP_ACCESS_IO;
            } അല്ലെങ്കിൽ എങ്കിൽ ((addr & PCI_BASE_ADDRESS_MEM_TYPE_MASK) ==
                       PCI_BASE_ADDRESS_MEM_TYPE_64) {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                addr |= (u64)pci_config_readl(pci->bdf, base + 4) << 32;
                vp_cap->mode = (addr > 0xffffffffll) ?
                    VP_ACCESS_PCICFG : VP_ACCESS_MMIO;
            } അല്ലെങ്കിൽ {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                vp_cap->mode = VP_ACCESS_MMIO;
            }
            തിരഞ്ഞെടുപ്പ് (vp_cap->mode) {
            അവസ്ഥ VP_ACCESS_IO:
            {
                u32 addr = pci_enable_iobar(pci, base);
                എങ്കിൽ (!addr)
                    തിരികെ;
                vp_cap->ioaddr = addr + offset;
                mode = "io";
                നിർത്തുക;
            }
            അവസ്ഥ VP_ACCESS_MMIO:
            {
                ശൂന്യം *addr = pci_enable_membar(pci, base);
                എങ്കിൽ (!addr)
                    തിരികെ;
                vp_cap->memaddr = addr + offset;
                mode = "mmio";
                നിർത്തുക;
            }
            അവസ്ഥ VP_ACCESS_PCICFG:
                mode = "pcicfg";
                vp_cap->baroff = offset;
                നിർത്തുക;
            സ്ഥിരസ്ഥിതി:
                mode = "Huh?";
                നിർത്തുക;
            }
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d "
                    "bar %d at 0x%08llx off +0x%04x [%s]\n",
                    pci, vp_cap->cap, type, vp_cap->bar, addr, offset, mode);
        }

        cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, cap);
    }

    എങ്കിൽ (vp->common.cap && vp->notify.cap && vp->isr.cap && vp->device.cap) {
        dprintf(1, "pci dev %pP using modern (1.0) virtio mode\n", pci);
        vp->use_modern = 1;
    } അല്ലെങ്കിൽ {
        dprintf(1, "pci dev %pP using legacy (0.9.5) virtio mode\n", pci);
        vp->legacy.bar = 0;
        vp->legacy.ioaddr = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        എങ്കിൽ (!vp->legacy.ioaddr)
            തിരികെ;
        vp->legacy.mode = VP_ACCESS_IO;
    }

    vp_reset(vp);
    pci_enable_busmaster(pci);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER );
}

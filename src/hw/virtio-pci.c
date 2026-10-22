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

#அந்தர்பாவ "config.h" // CONFIG_DEBUG_LEVEL
#அந்தர்பாவ "malloc.h" // free
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "pci.h" // pci_config_readl
#அந்தர்பாவ "pcidevice.h" // struct pci_device
#அந்தர்பாவ "pci_regs.h" // PCI_BASE_ADDRESS_0
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "virtio-pci.h"
#அந்தர்பாவ "virtio-mmio.h"
#அந்தர்பாவ "virtio-ring.h"

u64 _vp_read(கட்டமைப்பு vp_cap *cap, u32 offset, u8 size)
{
    u64 var = 0;

    தேர்வு (cap->mode) {
    நிலை VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        தேர்வு (size) {
        நிலை 8:
            var = inl(addr);
            var |= (u64)inl(addr+4) << 32;
            நிறுத்து;
        நிலை 4:
            var = inl(addr);
            நிறுத்து;
        நிலை 2:
            var = inw(addr);
            நிறுத்து;
        நிலை 1:
            var = inb(addr);
            நிறுத்து;
        }
        நிறுத்து;
    }

    நிலை VP_ACCESS_MMIO:
    {
        வெறுமை *addr = cap->memaddr + offset;
        தேர்வு (size) {
        நிலை 8:
            var = readl(addr);
            var |= (u64)readl(addr+4) << 32;
            நிறுத்து;
        நிலை 4:
            var = readl(addr);
            நிறுத்து;
        நிலை 2:
            var = readw(addr);
            நிறுத்து;
        நிலை 1:
            var = readb(addr);
            நிறுத்து;
        }
        நிறுத்து;
    }

    நிலை VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(கட்டமைப்பு virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(கட்டமைப்பு virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(கட்டமைப்பு virtio_pci_cfg_cap, cap.length),
                          (size > 4) ? 4 : size);
        தேர்வு (size) {
        நிலை 8:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(கட்டமைப்பு virtio_pci_cfg_cap, pci_cfg_data));
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(கட்டமைப்பு virtio_pci_cfg_cap, cap.offset),
                              addr + 4);
            var |= (u64)pci_config_readl(cap->bdf, cap->cfg +
                                         offsetof(கட்டமைப்பு virtio_pci_cfg_cap, pci_cfg_data)) << 32;
            நிறுத்து;
        நிலை 4:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(கட்டமைப்பு virtio_pci_cfg_cap, pci_cfg_data));
            நிறுத்து;
        நிலை 2:
            var = pci_config_readw(cap->bdf, cap->cfg +
                                   offsetof(கட்டமைப்பு virtio_pci_cfg_cap, pci_cfg_data));
            நிறுத்து;
        நிலை 1:
            var = pci_config_readb(cap->bdf, cap->cfg +
                                   offsetof(கட்டமைப்பு virtio_pci_cfg_cap, pci_cfg_data));
            நிறுத்து;
        }
    }
    }
    dprintf(9, "vp read   %x (%d) -> 0x%llx\n", cap->ioaddr + offset, size, var);
    திரும்பு var;
}

வெறுமை _vp_write(கட்டமைப்பு vp_cap *cap, u32 offset, u8 size, u64 var)
{
    dprintf(9, "vp write  %x (%d) <- 0x%llx\n", cap->ioaddr + offset, size, var);

    தேர்வு (cap->mode) {
    நிலை VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        தேர்வு (size) {
        நிலை 4:
            outl(var, addr);
            நிறுத்து;
        நிலை 2:
            outw(var, addr);
            நிறுத்து;
        நிலை 1:
            outb(var, addr);
            நிறுத்து;
        }
        நிறுத்து;
    }

    நிலை VP_ACCESS_MMIO:
    {
        வெறுமை *addr = cap->memaddr + offset;
        தேர்வு (size) {
        நிலை 4:
            writel(addr, var);
            நிறுத்து;
        நிலை 2:
            writew(addr, var);
            நிறுத்து;
        நிலை 1:
            writeb(addr, var);
            நிறுத்து;
        }
        நிறுத்து;
    }

    நிலை VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(கட்டமைப்பு virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(கட்டமைப்பு virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(கட்டமைப்பு virtio_pci_cfg_cap, cap.length),
                          size);
        தேர்வு (size) {
        நிலை 4:
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(கட்டமைப்பு virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            நிறுத்து;
        நிலை 2:
            pci_config_writew(cap->bdf, cap->cfg +
                              offsetof(கட்டமைப்பு virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            நிறுத்து;
        நிலை 1:
            pci_config_writeb(cap->bdf, cap->cfg +
                              offsetof(கட்டமைப்பு virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            நிறுத்து;
        }
    }
    }
}

u64 vp_get_features(கட்டமைப்பு vp_device *vp)
{
    u32 f0, f1;

    எனில் (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
    } இல்லையெனில் எனில் (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
    } இல்லையெனில் {
        f0 = vp_read(&vp->legacy, virtio_pci_legacy, host_features);
        f1 = 0;
    }
    திரும்பு ((u64)f1 << 32) | f0;
}

வெறுமை vp_set_features(கட்டமைப்பு vp_device *vp, u64 features)
{
    u32 f0, f1;

    f0 = features;
    f1 = features >> 32;

    எனில் (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f1);
    } இல்லையெனில் எனில் (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f1);
    } இல்லையெனில் {
        vp_write(&vp->legacy, virtio_pci_legacy, guest_features, f0);
    }
}

u8 vp_get_status(கட்டமைப்பு vp_device *vp)
{
    எனில் (vp->use_mmio) {
        திரும்பு vp_read(&vp->common, virtio_mmio_cfg, device_status);
    } இல்லையெனில் எனில் (vp->use_modern) {
        திரும்பு vp_read(&vp->common, virtio_pci_common_cfg, device_status);
    } இல்லையெனில் {
        திரும்பு vp_read(&vp->legacy, virtio_pci_legacy, status);
    }
}

வெறுமை vp_set_status(கட்டமைப்பு vp_device *vp, u8 status)
{
    எனில் (status == 0)        /* reset */
        திரும்பு;
    எனில் (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, status);
    } இல்லையெனில் எனில் (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, status);
    } இல்லையெனில் {
        vp_write(&vp->legacy, virtio_pci_legacy, status, status);
    }
}

u8 vp_get_isr(கட்டமைப்பு vp_device *vp)
{
    எனில் (vp->use_mmio) {
        திரும்பு vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } இல்லையெனில் எனில் (vp->use_modern) {
        திரும்பு vp_read(&vp->isr, virtio_pci_isr, isr);
    } இல்லையெனில் {
        திரும்பு vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

வெறுமை vp_reset(கட்டமைப்பு vp_device *vp)
{
    எனில் (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, 0);
        vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } இல்லையெனில் எனில் (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, 0);
        vp_read(&vp->isr, virtio_pci_isr, isr);
    } இல்லையெனில் {
        vp_write(&vp->legacy, virtio_pci_legacy, status, 0);
        vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

வெறுமை vp_notify(கட்டமைப்பு vp_device *vp, கட்டமைப்பு vring_virtqueue *vq)
{
    எனில் (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, queue_notify, vq->queue_index);
    } இல்லையெனில் எனில் (vp->use_modern) {
        u32 offset = vq->queue_notify_off * vp->notify_off_multiplier;
        தேர்வு (vp->notify.mode) {
        நிலை VP_ACCESS_IO:
            outw(vq->queue_index, vp->notify.ioaddr + offset);
            நிறுத்து;
        நிலை VP_ACCESS_MMIO:
            writew(vp->notify.memaddr + offset, vq->queue_index);
            நிறுத்து;
        நிலை VP_ACCESS_PCICFG:
            pci_config_writeb(vp->notify.bdf, vp->notify.cfg +
                              offsetof(கட்டமைப்பு virtio_pci_cfg_cap, cap.bar),
                              vp->notify.bar);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(கட்டமைப்பு virtio_pci_cfg_cap, cap.offset),
                              vp->notify.baroff + offset);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(கட்டமைப்பு virtio_pci_cfg_cap, cap.length),
                              2);
            pci_config_writew(vp->notify.bdf, vp->notify.cfg +
                              offsetof(கட்டமைப்பு virtio_pci_cfg_cap, pci_cfg_data),
                              vq->queue_index);
        }
        dprintf(9, "vp notify %x (%d) -- 0x%x\n",
                vp->notify.ioaddr, 2, vq->queue_index);
    } இல்லையெனில் {
        vp_write(&vp->legacy, virtio_pci_legacy, queue_notify, vq->queue_index);
    }
}

முழு vp_find_vq(கட்டமைப்பு vp_device *vp, முழு queue_index,
               கட்டமைப்பு vring_virtqueue **p_vq)
{
   u16 இலக்கம்;

   ASSERT32FLAT();
   கட்டமைப்பு vring_virtqueue *vq = *p_vq = memalign_உயர்வு(PAGE_SIZE, அளவு(*vq));
   எனில் (!vq) {
       warn_noalloc();
       செல் fail;
   }
   memset(vq, 0, அளவு(*vq));


   /* select the queue */
   எனில் (vp->use_mmio) {
       vp_write(&vp->common, virtio_mmio_cfg, queue_select, queue_index);
   } இல்லையெனில் எனில் (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, queue_select, queue_index);
   } இல்லையெனில் {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_sel, queue_index);
   }

   /* check if the queue is available */
   எனில் (vp->use_mmio) {
       இலக்கம் = vp_read(&vp->common, virtio_mmio_cfg, queue_இலக்கம்_max);
       எனில் (இலக்கம் > MAX_QUEUE_இலக்கம்)
           இலக்கம் = MAX_QUEUE_இலக்கம்;
       vp_write(&vp->common, virtio_mmio_cfg, queue_இலக்கம், இலக்கம்);
   } இல்லையெனில் எனில் (vp->use_modern) {
       இலக்கம் = vp_read(&vp->common, virtio_pci_common_cfg, queue_size);
       எனில் (இலக்கம் > MAX_QUEUE_இலக்கம்) {
           vp_write(&vp->common, virtio_pci_common_cfg, queue_size,
                    MAX_QUEUE_இலக்கம்);
           இலக்கம் = vp_read(&vp->common, virtio_pci_common_cfg, queue_size);
       }
   } இல்லையெனில் {
       இலக்கம் = vp_read(&vp->legacy, virtio_pci_legacy, queue_இலக்கம்);
   }
   எனில் (!இலக்கம்) {
       dprintf(1, "ERROR: queue size is 0\n");
       செல் fail;
   }
   எனில் (இலக்கம் > MAX_QUEUE_இலக்கம்) {
       dprintf(1, "ERROR: queue size %d > %d\n", இலக்கம், MAX_QUEUE_இலக்கம்);
       செல் fail;
   }

   /* check if the queue is already active */
   எனில் (vp->use_mmio) {
       /* TODO */;
   } இல்லையெனில் எனில் (vp->use_modern) {
       எனில் (vp_read(&vp->common, virtio_pci_common_cfg, queue_enable)) {
           dprintf(1, "ERROR: queue already active\n");
           செல் fail;
       }
   } இல்லையெனில் {
       எனில் (vp_read(&vp->legacy, virtio_pci_legacy, queue_pfn)) {
           dprintf(1, "ERROR: queue already active\n");
           செல் fail;
       }
   }
   vq->queue_index = queue_index;

   /* initialize the queue */
   கட்டமைப்பு vring * vr = &vq->vring;
   vring_init(vr, இலக்கம், (unsigned எழுத்து*)&vq->queue);

   /* activate the queue
    *
    * NOTE: vr->desc is initialized by vring_init()
    */

   எனில் (vp->use_mmio) {
       எனில் (vp_read(&vp->common, virtio_mmio_cfg, version) == 2) {
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
       } இல்லையெனில் {
           vp_write(&vp->common, virtio_mmio_cfg, legacy_guest_page_size,
                    (unsigned long)1 << PAGE_SHIFT);
           vp_write(&vp->common, virtio_mmio_cfg, legacy_queue_pfn,
                    (unsigned long)virt_to_phys(vr->desc) >> PAGE_SHIFT);
       }
   } இல்லையெனில் எனில் (vp->use_modern) {
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
   } இல்லையெனில் {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_pfn,
                (unsigned long)virt_to_phys(vr->desc) >> PAGE_SHIFT);
   }
   திரும்பு இலக்கம்;

fail:
   விடுவி(vq);
   *p_vq = வெற்று;
   திரும்பு -1;
}

வெறுமை vp_init_simple(கட்டமைப்பு vp_device *vp, கட்டமைப்பு pci_device *pci)
{
    u8 cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, 0);
    கட்டமைப்பு vp_cap *vp_cap;
    மாறிலி எழுத்து *mode;
    u32 offset, base, mul;
    u64 addr;
    u8 type;

    memset(vp, 0, அளவு(*vp));
    போது (cap != 0) {
        type = pci_config_readb(pci->bdf, cap +
                                offsetof(கட்டமைப்பு virtio_pci_cap, cfg_type));
        தேர்வு (type) {
        நிலை VIRTIO_PCI_CAP_COMMON_CFG:
            vp_cap = &vp->common;
            நிறுத்து;
        நிலை VIRTIO_PCI_CAP_NOTIFY_CFG:
            vp_cap = &vp->notify;
            mul = offsetof(கட்டமைப்பு virtio_pci_notify_cap, notify_off_multiplier);
            vp->notify_off_multiplier = pci_config_readl(pci->bdf, cap + mul);
            நிறுத்து;
        நிலை VIRTIO_PCI_CAP_ISR_CFG:
            vp_cap = &vp->isr;
            நிறுத்து;
        நிலை VIRTIO_PCI_CAP_DEVICE_CFG:
            vp_cap = &vp->device;
            நிறுத்து;
        நிலை VIRTIO_PCI_CAP_PCI_CFG:
            vp->common.cfg = cap;
            vp->common.bdf = pci->bdf;
            vp->notify.cfg = cap;
            vp->notify.bdf = pci->bdf;
            vp->isr.cfg = cap;
            vp->isr.bdf = pci->bdf;
            vp->device.cfg = cap;
            vp->device.bdf = pci->bdf;
            vp_cap = வெற்று;
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d"
                    " [pci cfg access]\n", pci, cap, type);
            நிறுத்து;
        இயல்பு:
            vp_cap = வெற்று;
            நிறுத்து;
        }
        எனில் (vp_cap && !vp_cap->cap) {
            vp_cap->cap = cap;
            vp_cap->bar = pci_config_readb(pci->bdf, cap +
                                           offsetof(கட்டமைப்பு virtio_pci_cap, bar));
            offset = pci_config_readl(pci->bdf, cap +
                                      offsetof(கட்டமைப்பு virtio_pci_cap, offset));
            base = PCI_BASE_ADDRESS_0 + 4 * vp_cap->bar;
            addr = pci_config_readl(pci->bdf, base);
            எனில் (addr & PCI_BASE_ADDRESS_SPACE_IO) {
                addr &= PCI_BASE_ADDRESS_IO_MASK;
                vp_cap->mode = VP_ACCESS_IO;
            } இல்லையெனில் எனில் ((addr & PCI_BASE_ADDRESS_MEM_TYPE_MASK) ==
                       PCI_BASE_ADDRESS_MEM_TYPE_64) {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                addr |= (u64)pci_config_readl(pci->bdf, base + 4) << 32;
                vp_cap->mode = (addr > 0xffffffffll) ?
                    VP_ACCESS_PCICFG : VP_ACCESS_MMIO;
            } இல்லையெனில் {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                vp_cap->mode = VP_ACCESS_MMIO;
            }
            தேர்வு (vp_cap->mode) {
            நிலை VP_ACCESS_IO:
            {
                u32 addr = pci_enable_iobar(pci, base);
                எனில் (!addr)
                    திரும்பு;
                vp_cap->ioaddr = addr + offset;
                mode = "io";
                நிறுத்து;
            }
            நிலை VP_ACCESS_MMIO:
            {
                வெறுமை *addr = pci_enable_membar(pci, base);
                எனில் (!addr)
                    திரும்பு;
                vp_cap->memaddr = addr + offset;
                mode = "mmio";
                நிறுத்து;
            }
            நிலை VP_ACCESS_PCICFG:
                mode = "pcicfg";
                vp_cap->baroff = offset;
                நிறுத்து;
            இயல்பு:
                mode = "Huh?";
                நிறுத்து;
            }
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d "
                    "bar %d at 0x%08llx off +0x%04x [%s]\n",
                    pci, vp_cap->cap, type, vp_cap->bar, addr, offset, mode);
        }

        cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, cap);
    }

    எனில் (vp->common.cap && vp->notify.cap && vp->isr.cap && vp->device.cap) {
        dprintf(1, "pci dev %pP using modern (1.0) virtio mode\n", pci);
        vp->use_modern = 1;
    } இல்லையெனில் {
        dprintf(1, "pci dev %pP using legacy (0.9.5) virtio mode\n", pci);
        vp->legacy.bar = 0;
        vp->legacy.ioaddr = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        எனில் (!vp->legacy.ioaddr)
            திரும்பு;
        vp->legacy.mode = VP_ACCESS_IO;
    }

    vp_reset(vp);
    pci_enable_busmaster(pci);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER );
}

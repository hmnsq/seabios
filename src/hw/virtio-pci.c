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

#અન્તર્ભાવ "config.h" // CONFIG_DEBUG_LEVEL
#અન્તર્ભાવ "malloc.h" // free
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "pci.h" // pci_config_readl
#અન્તર્ભાવ "pcidevice.h" // struct pci_device
#અન્તર્ભાવ "pci_regs.h" // PCI_BASE_ADDRESS_0
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "virtio-pci.h"
#અન્તર્ભાવ "virtio-mmio.h"
#અન્તર્ભાવ "virtio-ring.h"

u64 _vp_read(સંરચના vp_cap *cap, u32 offset, u8 size)
{
    u64 var = 0;

    પસંદગી (cap->mode) {
    સ્થિતિ VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        પસંદગી (size) {
        સ્થિતિ 8:
            var = inl(addr);
            var |= (u64)inl(addr+4) << 32;
            તોડો;
        સ્થિતિ 4:
            var = inl(addr);
            તોડો;
        સ્થિતિ 2:
            var = inw(addr);
            તોડો;
        સ્થિતિ 1:
            var = inb(addr);
            તોડો;
        }
        તોડો;
    }

    સ્થિતિ VP_ACCESS_MMIO:
    {
        શૂન્ય *addr = cap->memaddr + offset;
        પસંદગી (size) {
        સ્થિતિ 8:
            var = readl(addr);
            var |= (u64)readl(addr+4) << 32;
            તોડો;
        સ્થિતિ 4:
            var = readl(addr);
            તોડો;
        સ્થિતિ 2:
            var = readw(addr);
            તોડો;
        સ્થિતિ 1:
            var = readb(addr);
            તોડો;
        }
        તોડો;
    }

    સ્થિતિ VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(સંરચના virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(સંરચના virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(સંરચના virtio_pci_cfg_cap, cap.length),
                          (size > 4) ? 4 : size);
        પસંદગી (size) {
        સ્થિતિ 8:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(સંરચના virtio_pci_cfg_cap, pci_cfg_data));
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(સંરચના virtio_pci_cfg_cap, cap.offset),
                              addr + 4);
            var |= (u64)pci_config_readl(cap->bdf, cap->cfg +
                                         offsetof(સંરચના virtio_pci_cfg_cap, pci_cfg_data)) << 32;
            તોડો;
        સ્થિતિ 4:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(સંરચના virtio_pci_cfg_cap, pci_cfg_data));
            તોડો;
        સ્થિતિ 2:
            var = pci_config_readw(cap->bdf, cap->cfg +
                                   offsetof(સંરચના virtio_pci_cfg_cap, pci_cfg_data));
            તોડો;
        સ્થિતિ 1:
            var = pci_config_readb(cap->bdf, cap->cfg +
                                   offsetof(સંરચના virtio_pci_cfg_cap, pci_cfg_data));
            તોડો;
        }
    }
    }
    dprintf(9, "vp read   %x (%d) -> 0x%llx\n", cap->ioaddr + offset, size, var);
    પાછા var;
}

શૂન્ય _vp_write(સંરચના vp_cap *cap, u32 offset, u8 size, u64 var)
{
    dprintf(9, "vp write  %x (%d) <- 0x%llx\n", cap->ioaddr + offset, size, var);

    પસંદગી (cap->mode) {
    સ્થિતિ VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        પસંદગી (size) {
        સ્થિતિ 4:
            outl(var, addr);
            તોડો;
        સ્થિતિ 2:
            outw(var, addr);
            તોડો;
        સ્થિતિ 1:
            outb(var, addr);
            તોડો;
        }
        તોડો;
    }

    સ્થિતિ VP_ACCESS_MMIO:
    {
        શૂન્ય *addr = cap->memaddr + offset;
        પસંદગી (size) {
        સ્થિતિ 4:
            writel(addr, var);
            તોડો;
        સ્થિતિ 2:
            writew(addr, var);
            તોડો;
        સ્થિતિ 1:
            writeb(addr, var);
            તોડો;
        }
        તોડો;
    }

    સ્થિતિ VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(સંરચના virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(સંરચના virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(સંરચના virtio_pci_cfg_cap, cap.length),
                          size);
        પસંદગી (size) {
        સ્થિતિ 4:
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(સંરચના virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            તોડો;
        સ્થિતિ 2:
            pci_config_writew(cap->bdf, cap->cfg +
                              offsetof(સંરચના virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            તોડો;
        સ્થિતિ 1:
            pci_config_writeb(cap->bdf, cap->cfg +
                              offsetof(સંરચના virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            તોડો;
        }
    }
    }
}

u64 vp_get_features(સંરચના vp_device *vp)
{
    u32 f0, f1;

    જો (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
    } નહીંતર જો (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
    } નહીંતર {
        f0 = vp_read(&vp->legacy, virtio_pci_legacy, host_features);
        f1 = 0;
    }
    પાછા ((u64)f1 << 32) | f0;
}

શૂન્ય vp_set_features(સંરચના vp_device *vp, u64 features)
{
    u32 f0, f1;

    f0 = features;
    f1 = features >> 32;

    જો (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f1);
    } નહીંતર જો (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f1);
    } નહીંતર {
        vp_write(&vp->legacy, virtio_pci_legacy, guest_features, f0);
    }
}

u8 vp_get_status(સંરચના vp_device *vp)
{
    જો (vp->use_mmio) {
        પાછા vp_read(&vp->common, virtio_mmio_cfg, device_status);
    } નહીંતર જો (vp->use_modern) {
        પાછા vp_read(&vp->common, virtio_pci_common_cfg, device_status);
    } નહીંતર {
        પાછા vp_read(&vp->legacy, virtio_pci_legacy, status);
    }
}

શૂન્ય vp_set_status(સંરચના vp_device *vp, u8 status)
{
    જો (status == 0)        /* reset */
        પાછા;
    જો (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, status);
    } નહીંતર જો (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, status);
    } નહીંતર {
        vp_write(&vp->legacy, virtio_pci_legacy, status, status);
    }
}

u8 vp_get_isr(સંરચના vp_device *vp)
{
    જો (vp->use_mmio) {
        પાછા vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } નહીંતર જો (vp->use_modern) {
        પાછા vp_read(&vp->isr, virtio_pci_isr, isr);
    } નહીંતર {
        પાછા vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

શૂન્ય vp_reset(સંરચના vp_device *vp)
{
    જો (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, 0);
        vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } નહીંતર જો (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, 0);
        vp_read(&vp->isr, virtio_pci_isr, isr);
    } નહીંતર {
        vp_write(&vp->legacy, virtio_pci_legacy, status, 0);
        vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

શૂન્ય vp_notify(સંરચના vp_device *vp, સંરચના vring_virtqueue *vq)
{
    જો (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, queue_notify, vq->queue_index);
    } નહીંતર જો (vp->use_modern) {
        u32 offset = vq->queue_notify_off * vp->notify_off_multiplier;
        પસંદગી (vp->notify.mode) {
        સ્થિતિ VP_ACCESS_IO:
            outw(vq->queue_index, vp->notify.ioaddr + offset);
            તોડો;
        સ્થિતિ VP_ACCESS_MMIO:
            writew(vp->notify.memaddr + offset, vq->queue_index);
            તોડો;
        સ્થિતિ VP_ACCESS_PCICFG:
            pci_config_writeb(vp->notify.bdf, vp->notify.cfg +
                              offsetof(સંરચના virtio_pci_cfg_cap, cap.bar),
                              vp->notify.bar);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(સંરચના virtio_pci_cfg_cap, cap.offset),
                              vp->notify.baroff + offset);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(સંરચના virtio_pci_cfg_cap, cap.length),
                              2);
            pci_config_writew(vp->notify.bdf, vp->notify.cfg +
                              offsetof(સંરચના virtio_pci_cfg_cap, pci_cfg_data),
                              vq->queue_index);
        }
        dprintf(9, "vp notify %x (%d) -- 0x%x\n",
                vp->notify.ioaddr, 2, vq->queue_index);
    } નહીંતર {
        vp_write(&vp->legacy, virtio_pci_legacy, queue_notify, vq->queue_index);
    }
}

પૂર્ણાંક vp_find_vq(સંરચના vp_device *vp, પૂર્ણાંક queue_index,
               સંરચના vring_virtqueue **p_vq)
{
   u16 અંક;

   ASSERT32FLAT();
   સંરચના vring_virtqueue *vq = *p_vq = memalign_ઉપલું(PAGE_SIZE, કદ(*vq));
   જો (!vq) {
       warn_noalloc();
       જાઓ fail;
   }
   memset(vq, 0, કદ(*vq));


   /* select the queue */
   જો (vp->use_mmio) {
       vp_write(&vp->common, virtio_mmio_cfg, queue_select, queue_index);
   } નહીંતર જો (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, queue_select, queue_index);
   } નહીંતર {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_sel, queue_index);
   }

   /* check if the queue is available */
   જો (vp->use_mmio) {
       અંક = vp_read(&vp->common, virtio_mmio_cfg, queue_અંક_max);
       જો (અંક > MAX_QUEUE_અંક)
           અંક = MAX_QUEUE_અંક;
       vp_write(&vp->common, virtio_mmio_cfg, queue_અંક, અંક);
   } નહીંતર જો (vp->use_modern) {
       અંક = vp_read(&vp->common, virtio_pci_common_cfg, queue_size);
       જો (અંક > MAX_QUEUE_અંક) {
           vp_write(&vp->common, virtio_pci_common_cfg, queue_size,
                    MAX_QUEUE_અંક);
           અંક = vp_read(&vp->common, virtio_pci_common_cfg, queue_size);
       }
   } નહીંતર {
       અંક = vp_read(&vp->legacy, virtio_pci_legacy, queue_અંક);
   }
   જો (!અંક) {
       dprintf(1, "ERROR: queue size is 0\n");
       જાઓ fail;
   }
   જો (અંક > MAX_QUEUE_અંક) {
       dprintf(1, "ERROR: queue size %d > %d\n", અંક, MAX_QUEUE_અંક);
       જાઓ fail;
   }

   /* check if the queue is already active */
   જો (vp->use_mmio) {
       /* TODO */;
   } નહીંતર જો (vp->use_modern) {
       જો (vp_read(&vp->common, virtio_pci_common_cfg, queue_enable)) {
           dprintf(1, "ERROR: queue already active\n");
           જાઓ fail;
       }
   } નહીંતર {
       જો (vp_read(&vp->legacy, virtio_pci_legacy, queue_pfn)) {
           dprintf(1, "ERROR: queue already active\n");
           જાઓ fail;
       }
   }
   vq->queue_index = queue_index;

   /* initialize the queue */
   સંરચના vring * vr = &vq->vring;
   vring_init(vr, અંક, (unsigned અક્ષર*)&vq->queue);

   /* activate the queue
    *
    * NOTE: vr->desc is initialized by vring_init()
    */

   જો (vp->use_mmio) {
       જો (vp_read(&vp->common, virtio_mmio_cfg, version) == 2) {
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
       } નહીંતર {
           vp_write(&vp->common, virtio_mmio_cfg, legacy_guest_page_size,
                    (unsigned long)1 << PAGE_SHIFT);
           vp_write(&vp->common, virtio_mmio_cfg, legacy_queue_pfn,
                    (unsigned long)virt_to_phys(vr->desc) >> PAGE_SHIFT);
       }
   } નહીંતર જો (vp->use_modern) {
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
   } નહીંતર {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_pfn,
                (unsigned long)virt_to_phys(vr->desc) >> PAGE_SHIFT);
   }
   પાછા અંક;

fail:
   મુક્ત(vq);
   *p_vq = ખાલી;
   પાછા -1;
}

શૂન્ય vp_init_simple(સંરચના vp_device *vp, સંરચના pci_device *pci)
{
    u8 cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, 0);
    સંરચના vp_cap *vp_cap;
    અચલ અક્ષર *mode;
    u32 offset, base, mul;
    u64 addr;
    u8 type;

    memset(vp, 0, કદ(*vp));
    જ્યારે (cap != 0) {
        type = pci_config_readb(pci->bdf, cap +
                                offsetof(સંરચના virtio_pci_cap, cfg_type));
        પસંદગી (type) {
        સ્થિતિ VIRTIO_PCI_CAP_COMMON_CFG:
            vp_cap = &vp->common;
            તોડો;
        સ્થિતિ VIRTIO_PCI_CAP_NOTIFY_CFG:
            vp_cap = &vp->notify;
            mul = offsetof(સંરચના virtio_pci_notify_cap, notify_off_multiplier);
            vp->notify_off_multiplier = pci_config_readl(pci->bdf, cap + mul);
            તોડો;
        સ્થિતિ VIRTIO_PCI_CAP_ISR_CFG:
            vp_cap = &vp->isr;
            તોડો;
        સ્થિતિ VIRTIO_PCI_CAP_DEVICE_CFG:
            vp_cap = &vp->device;
            તોડો;
        સ્થિતિ VIRTIO_PCI_CAP_PCI_CFG:
            vp->common.cfg = cap;
            vp->common.bdf = pci->bdf;
            vp->notify.cfg = cap;
            vp->notify.bdf = pci->bdf;
            vp->isr.cfg = cap;
            vp->isr.bdf = pci->bdf;
            vp->device.cfg = cap;
            vp->device.bdf = pci->bdf;
            vp_cap = ખાલી;
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d"
                    " [pci cfg access]\n", pci, cap, type);
            તોડો;
        મૂળભૂત:
            vp_cap = ખાલી;
            તોડો;
        }
        જો (vp_cap && !vp_cap->cap) {
            vp_cap->cap = cap;
            vp_cap->bar = pci_config_readb(pci->bdf, cap +
                                           offsetof(સંરચના virtio_pci_cap, bar));
            offset = pci_config_readl(pci->bdf, cap +
                                      offsetof(સંરચના virtio_pci_cap, offset));
            base = PCI_BASE_ADDRESS_0 + 4 * vp_cap->bar;
            addr = pci_config_readl(pci->bdf, base);
            જો (addr & PCI_BASE_ADDRESS_SPACE_IO) {
                addr &= PCI_BASE_ADDRESS_IO_MASK;
                vp_cap->mode = VP_ACCESS_IO;
            } નહીંતર જો ((addr & PCI_BASE_ADDRESS_MEM_TYPE_MASK) ==
                       PCI_BASE_ADDRESS_MEM_TYPE_64) {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                addr |= (u64)pci_config_readl(pci->bdf, base + 4) << 32;
                vp_cap->mode = (addr > 0xffffffffll) ?
                    VP_ACCESS_PCICFG : VP_ACCESS_MMIO;
            } નહીંતર {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                vp_cap->mode = VP_ACCESS_MMIO;
            }
            પસંદગી (vp_cap->mode) {
            સ્થિતિ VP_ACCESS_IO:
            {
                u32 addr = pci_enable_iobar(pci, base);
                જો (!addr)
                    પાછા;
                vp_cap->ioaddr = addr + offset;
                mode = "io";
                તોડો;
            }
            સ્થિતિ VP_ACCESS_MMIO:
            {
                શૂન્ય *addr = pci_enable_membar(pci, base);
                જો (!addr)
                    પાછા;
                vp_cap->memaddr = addr + offset;
                mode = "mmio";
                તોડો;
            }
            સ્થિતિ VP_ACCESS_PCICFG:
                mode = "pcicfg";
                vp_cap->baroff = offset;
                તોડો;
            મૂળભૂત:
                mode = "Huh?";
                તોડો;
            }
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d "
                    "bar %d at 0x%08llx off +0x%04x [%s]\n",
                    pci, vp_cap->cap, type, vp_cap->bar, addr, offset, mode);
        }

        cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, cap);
    }

    જો (vp->common.cap && vp->notify.cap && vp->isr.cap && vp->device.cap) {
        dprintf(1, "pci dev %pP using modern (1.0) virtio mode\n", pci);
        vp->use_modern = 1;
    } નહીંતર {
        dprintf(1, "pci dev %pP using legacy (0.9.5) virtio mode\n", pci);
        vp->legacy.bar = 0;
        vp->legacy.ioaddr = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        જો (!vp->legacy.ioaddr)
            પાછા;
        vp->legacy.mode = VP_ACCESS_IO;
    }

    vp_reset(vp);
    pci_enable_busmaster(pci);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER );
}

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

#ଅନ୍ତର୍ଭାଵ "config.h" // CONFIG_DEBUG_LEVEL
#ଅନ୍ତର୍ଭାଵ "malloc.h" // free
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "pci.h" // pci_config_readl
#ଅନ୍ତର୍ଭାଵ "pcidevice.h" // struct pci_device
#ଅନ୍ତର୍ଭାଵ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "virtio-pci.h"
#ଅନ୍ତର୍ଭାଵ "virtio-mmio.h"
#ଅନ୍ତର୍ଭାଵ "virtio-ring.h"

u64 _vp_read(ସଂରଚନା vp_cap *cap, u32 offset, u8 size)
{
    u64 var = 0;

    ବାଛ (cap->mode) {
    ସ୍ଥିତି VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        ବାଛ (size) {
        ସ୍ଥିତି 8:
            var = inl(addr);
            var |= (u64)inl(addr+4) << 32;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 4:
            var = inl(addr);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 2:
            var = inw(addr);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 1:
            var = inb(addr);
            ଭାଙ୍ଗ;
        }
        ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି VP_ACCESS_MMIO:
    {
        ଶୂନ୍ୟ *addr = cap->memaddr + offset;
        ବାଛ (size) {
        ସ୍ଥିତି 8:
            var = readl(addr);
            var |= (u64)readl(addr+4) << 32;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 4:
            var = readl(addr);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 2:
            var = readw(addr);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 1:
            var = readb(addr);
            ଭାଙ୍ଗ;
        }
        ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(ସଂରଚନା virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ସଂରଚନା virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ସଂରଚନା virtio_pci_cfg_cap, cap.length),
                          (size > 4) ? 4 : size);
        ବାଛ (size) {
        ସ୍ଥିତି 8:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(ସଂରଚନା virtio_pci_cfg_cap, pci_cfg_data));
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(ସଂରଚନା virtio_pci_cfg_cap, cap.offset),
                              addr + 4);
            var |= (u64)pci_config_readl(cap->bdf, cap->cfg +
                                         offsetof(ସଂରଚନା virtio_pci_cfg_cap, pci_cfg_data)) << 32;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 4:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(ସଂରଚନା virtio_pci_cfg_cap, pci_cfg_data));
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 2:
            var = pci_config_readw(cap->bdf, cap->cfg +
                                   offsetof(ସଂରଚନା virtio_pci_cfg_cap, pci_cfg_data));
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 1:
            var = pci_config_readb(cap->bdf, cap->cfg +
                                   offsetof(ସଂରଚନା virtio_pci_cfg_cap, pci_cfg_data));
            ଭାଙ୍ଗ;
        }
    }
    }
    dprintf(9, "vp read   %x (%d) -> 0x%llx\n", cap->ioaddr + offset, size, var);
    ଫେରନ୍ତୁ var;
}

ଶୂନ୍ୟ _vp_write(ସଂରଚନା vp_cap *cap, u32 offset, u8 size, u64 var)
{
    dprintf(9, "vp write  %x (%d) <- 0x%llx\n", cap->ioaddr + offset, size, var);

    ବାଛ (cap->mode) {
    ସ୍ଥିତି VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        ବାଛ (size) {
        ସ୍ଥିତି 4:
            outl(var, addr);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 2:
            outw(var, addr);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 1:
            outb(var, addr);
            ଭାଙ୍ଗ;
        }
        ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି VP_ACCESS_MMIO:
    {
        ଶୂନ୍ୟ *addr = cap->memaddr + offset;
        ବାଛ (size) {
        ସ୍ଥିତି 4:
            writel(addr, var);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 2:
            writew(addr, var);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 1:
            writeb(addr, var);
            ଭାଙ୍ଗ;
        }
        ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(ସଂରଚନା virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ସଂରଚନା virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(ସଂରଚନା virtio_pci_cfg_cap, cap.length),
                          size);
        ବାଛ (size) {
        ସ୍ଥିତି 4:
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(ସଂରଚନା virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 2:
            pci_config_writew(cap->bdf, cap->cfg +
                              offsetof(ସଂରଚନା virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି 1:
            pci_config_writeb(cap->bdf, cap->cfg +
                              offsetof(ସଂରଚନା virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            ଭାଙ୍ଗ;
        }
    }
    }
}

u64 vp_get_features(ସଂରଚନା vp_device *vp)
{
    u32 f0, f1;

    ଯଦି (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
    } ଅନ୍ୟଥା ଯଦି (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
    } ଅନ୍ୟଥା {
        f0 = vp_read(&vp->legacy, virtio_pci_legacy, host_features);
        f1 = 0;
    }
    ଫେରନ୍ତୁ ((u64)f1 << 32) | f0;
}

ଶୂନ୍ୟ vp_set_features(ସଂରଚନା vp_device *vp, u64 features)
{
    u32 f0, f1;

    f0 = features;
    f1 = features >> 32;

    ଯଦି (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f1);
    } ଅନ୍ୟଥା ଯଦି (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f1);
    } ଅନ୍ୟଥା {
        vp_write(&vp->legacy, virtio_pci_legacy, guest_features, f0);
    }
}

u8 vp_get_status(ସଂରଚନା vp_device *vp)
{
    ଯଦି (vp->use_mmio) {
        ଫେରନ୍ତୁ vp_read(&vp->common, virtio_mmio_cfg, device_status);
    } ଅନ୍ୟଥା ଯଦି (vp->use_modern) {
        ଫେରନ୍ତୁ vp_read(&vp->common, virtio_pci_common_cfg, device_status);
    } ଅନ୍ୟଥା {
        ଫେରନ୍ତୁ vp_read(&vp->legacy, virtio_pci_legacy, status);
    }
}

ଶୂନ୍ୟ vp_set_status(ସଂରଚନା vp_device *vp, u8 status)
{
    ଯଦି (status == 0)        /* reset */
        ଫେରନ୍ତୁ;
    ଯଦି (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, status);
    } ଅନ୍ୟଥା ଯଦି (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, status);
    } ଅନ୍ୟଥା {
        vp_write(&vp->legacy, virtio_pci_legacy, status, status);
    }
}

u8 vp_get_isr(ସଂରଚନା vp_device *vp)
{
    ଯଦି (vp->use_mmio) {
        ଫେରନ୍ତୁ vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } ଅନ୍ୟଥା ଯଦି (vp->use_modern) {
        ଫେରନ୍ତୁ vp_read(&vp->isr, virtio_pci_isr, isr);
    } ଅନ୍ୟଥା {
        ଫେରନ୍ତୁ vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

ଶୂନ୍ୟ vp_reset(ସଂରଚନା vp_device *vp)
{
    ଯଦି (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, 0);
        vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } ଅନ୍ୟଥା ଯଦି (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, 0);
        vp_read(&vp->isr, virtio_pci_isr, isr);
    } ଅନ୍ୟଥା {
        vp_write(&vp->legacy, virtio_pci_legacy, status, 0);
        vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

ଶୂନ୍ୟ vp_notify(ସଂରଚନା vp_device *vp, ସଂରଚନା vring_virtqueue *vq)
{
    ଯଦି (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, queue_notify, vq->queue_index);
    } ଅନ୍ୟଥା ଯଦି (vp->use_modern) {
        u32 offset = vq->queue_notify_off * vp->notify_off_multiplier;
        ବାଛ (vp->notify.mode) {
        ସ୍ଥିତି VP_ACCESS_IO:
            outw(vq->queue_index, vp->notify.ioaddr + offset);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି VP_ACCESS_MMIO:
            writew(vp->notify.memaddr + offset, vq->queue_index);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି VP_ACCESS_PCICFG:
            pci_config_writeb(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ସଂରଚନା virtio_pci_cfg_cap, cap.bar),
                              vp->notify.bar);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ସଂରଚନା virtio_pci_cfg_cap, cap.offset),
                              vp->notify.baroff + offset);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ସଂରଚନା virtio_pci_cfg_cap, cap.length),
                              2);
            pci_config_writew(vp->notify.bdf, vp->notify.cfg +
                              offsetof(ସଂରଚନା virtio_pci_cfg_cap, pci_cfg_data),
                              vq->queue_index);
        }
        dprintf(9, "vp notify %x (%d) -- 0x%x\n",
                vp->notify.ioaddr, 2, vq->queue_index);
    } ଅନ୍ୟଥା {
        vp_write(&vp->legacy, virtio_pci_legacy, queue_notify, vq->queue_index);
    }
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା vp_find_vq(ସଂରଚନା vp_device *vp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା queue_index,
               ସଂରଚନା vring_virtqueue **p_vq)
{
   u16 ଅଙ୍କ;

   ASSERT32FLAT();
   ସଂରଚନା vring_virtqueue *vq = *p_vq = memalign_ଉପର(PAGE_SIZE, ଆକାର(*vq));
   ଯଦି (!vq) {
       warn_noalloc();
       ଯାଅ fail;
   }
   memset(vq, 0, ଆକାର(*vq));


   /* select the queue */
   ଯଦି (vp->use_mmio) {
       vp_write(&vp->common, virtio_mmio_cfg, queue_select, queue_index);
   } ଅନ୍ୟଥା ଯଦି (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, queue_select, queue_index);
   } ଅନ୍ୟଥା {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_sel, queue_index);
   }

   /* check if the queue is available */
   ଯଦି (vp->use_mmio) {
       ଅଙ୍କ = vp_read(&vp->common, virtio_mmio_cfg, queue_ଅଙ୍କ_max);
       ଯଦି (ଅଙ୍କ > MAX_QUEUE_ଅଙ୍କ)
           ଅଙ୍କ = MAX_QUEUE_ଅଙ୍କ;
       vp_write(&vp->common, virtio_mmio_cfg, queue_ଅଙ୍କ, ଅଙ୍କ);
   } ଅନ୍ୟଥା ଯଦି (vp->use_modern) {
       ଅଙ୍କ = vp_read(&vp->common, virtio_pci_common_cfg, queue_size);
       ଯଦି (ଅଙ୍କ > MAX_QUEUE_ଅଙ୍କ) {
           vp_write(&vp->common, virtio_pci_common_cfg, queue_size,
                    MAX_QUEUE_ଅଙ୍କ);
           ଅଙ୍କ = vp_read(&vp->common, virtio_pci_common_cfg, queue_size);
       }
   } ଅନ୍ୟଥା {
       ଅଙ୍କ = vp_read(&vp->legacy, virtio_pci_legacy, queue_ଅଙ୍କ);
   }
   ଯଦି (!ଅଙ୍କ) {
       dprintf(1, "ERROR: queue size is 0\n");
       ଯାଅ fail;
   }
   ଯଦି (ଅଙ୍କ > MAX_QUEUE_ଅଙ୍କ) {
       dprintf(1, "ERROR: queue size %d > %d\n", ଅଙ୍କ, MAX_QUEUE_ଅଙ୍କ);
       ଯାଅ fail;
   }

   /* check if the queue is already active */
   ଯଦି (vp->use_mmio) {
       /* TODO */;
   } ଅନ୍ୟଥା ଯଦି (vp->use_modern) {
       ଯଦି (vp_read(&vp->common, virtio_pci_common_cfg, queue_enable)) {
           dprintf(1, "ERROR: queue already active\n");
           ଯାଅ fail;
       }
   } ଅନ୍ୟଥା {
       ଯଦି (vp_read(&vp->legacy, virtio_pci_legacy, queue_pfn)) {
           dprintf(1, "ERROR: queue already active\n");
           ଯାଅ fail;
       }
   }
   vq->queue_index = queue_index;

   /* initialize the queue */
   ସଂରଚନା vring * vr = &vq->vring;
   vring_init(vr, ଅଙ୍କ, (unsigned ଅକ୍ଷର*)&vq->queue);

   /* activate the queue
    *
    * NOTE: vr->desc is initialized by vring_init()
    */

   ଯଦି (vp->use_mmio) {
       ଯଦି (vp_read(&vp->common, virtio_mmio_cfg, version) == 2) {
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
       } ଅନ୍ୟଥା {
           vp_write(&vp->common, virtio_mmio_cfg, legacy_guest_page_size,
                    (unsigned long)1 << PAGE_SHIFT);
           vp_write(&vp->common, virtio_mmio_cfg, legacy_queue_pfn,
                    (unsigned long)virt_to_phys(vr->desc) >> PAGE_SHIFT);
       }
   } ଅନ୍ୟଥା ଯଦି (vp->use_modern) {
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
   } ଅନ୍ୟଥା {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_pfn,
                (unsigned long)virt_to_phys(vr->desc) >> PAGE_SHIFT);
   }
   ଫେରନ୍ତୁ ଅଙ୍କ;

fail:
   ମୁକ୍ତ(vq);
   *p_vq = ଖାଲି;
   ଫେରନ୍ତୁ -1;
}

ଶୂନ୍ୟ vp_init_simple(ସଂରଚନା vp_device *vp, ସଂରଚନା pci_device *pci)
{
    u8 cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, 0);
    ସଂରଚନା vp_cap *vp_cap;
    ଅଚଳ ଅକ୍ଷର *mode;
    u32 offset, base, mul;
    u64 addr;
    u8 type;

    memset(vp, 0, ଆକାର(*vp));
    ଯେତେବେଳେ (cap != 0) {
        type = pci_config_readb(pci->bdf, cap +
                                offsetof(ସଂରଚନା virtio_pci_cap, cfg_type));
        ବାଛ (type) {
        ସ୍ଥିତି VIRTIO_PCI_CAP_COMMON_CFG:
            vp_cap = &vp->common;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି VIRTIO_PCI_CAP_NOTIFY_CFG:
            vp_cap = &vp->notify;
            mul = offsetof(ସଂରଚନା virtio_pci_notify_cap, notify_off_multiplier);
            vp->notify_off_multiplier = pci_config_readl(pci->bdf, cap + mul);
            ଭାଙ୍ଗ;
        ସ୍ଥିତି VIRTIO_PCI_CAP_ISR_CFG:
            vp_cap = &vp->isr;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି VIRTIO_PCI_CAP_DEVICE_CFG:
            vp_cap = &vp->device;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି VIRTIO_PCI_CAP_PCI_CFG:
            vp->common.cfg = cap;
            vp->common.bdf = pci->bdf;
            vp->notify.cfg = cap;
            vp->notify.bdf = pci->bdf;
            vp->isr.cfg = cap;
            vp->isr.bdf = pci->bdf;
            vp->device.cfg = cap;
            vp->device.bdf = pci->bdf;
            vp_cap = ଖାଲି;
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d"
                    " [pci cfg access]\n", pci, cap, type);
            ଭାଙ୍ଗ;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
            vp_cap = ଖାଲି;
            ଭାଙ୍ଗ;
        }
        ଯଦି (vp_cap && !vp_cap->cap) {
            vp_cap->cap = cap;
            vp_cap->bar = pci_config_readb(pci->bdf, cap +
                                           offsetof(ସଂରଚନା virtio_pci_cap, bar));
            offset = pci_config_readl(pci->bdf, cap +
                                      offsetof(ସଂରଚନା virtio_pci_cap, offset));
            base = PCI_BASE_ADDRESS_0 + 4 * vp_cap->bar;
            addr = pci_config_readl(pci->bdf, base);
            ଯଦି (addr & PCI_BASE_ADDRESS_SPACE_IO) {
                addr &= PCI_BASE_ADDRESS_IO_MASK;
                vp_cap->mode = VP_ACCESS_IO;
            } ଅନ୍ୟଥା ଯଦି ((addr & PCI_BASE_ADDRESS_MEM_TYPE_MASK) ==
                       PCI_BASE_ADDRESS_MEM_TYPE_64) {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                addr |= (u64)pci_config_readl(pci->bdf, base + 4) << 32;
                vp_cap->mode = (addr > 0xffffffffll) ?
                    VP_ACCESS_PCICFG : VP_ACCESS_MMIO;
            } ଅନ୍ୟଥା {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                vp_cap->mode = VP_ACCESS_MMIO;
            }
            ବାଛ (vp_cap->mode) {
            ସ୍ଥିତି VP_ACCESS_IO:
            {
                u32 addr = pci_enable_iobar(pci, base);
                ଯଦି (!addr)
                    ଫେରନ୍ତୁ;
                vp_cap->ioaddr = addr + offset;
                mode = "io";
                ଭାଙ୍ଗ;
            }
            ସ୍ଥିତି VP_ACCESS_MMIO:
            {
                ଶୂନ୍ୟ *addr = pci_enable_membar(pci, base);
                ଯଦି (!addr)
                    ଫେରନ୍ତୁ;
                vp_cap->memaddr = addr + offset;
                mode = "mmio";
                ଭାଙ୍ଗ;
            }
            ସ୍ଥିତି VP_ACCESS_PCICFG:
                mode = "pcicfg";
                vp_cap->baroff = offset;
                ଭାଙ୍ଗ;
            ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
                mode = "Huh?";
                ଭାଙ୍ଗ;
            }
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d "
                    "bar %d at 0x%08llx off +0x%04x [%s]\n",
                    pci, vp_cap->cap, type, vp_cap->bar, addr, offset, mode);
        }

        cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, cap);
    }

    ଯଦି (vp->common.cap && vp->notify.cap && vp->isr.cap && vp->device.cap) {
        dprintf(1, "pci dev %pP using modern (1.0) virtio mode\n", pci);
        vp->use_modern = 1;
    } ଅନ୍ୟଥା {
        dprintf(1, "pci dev %pP using legacy (0.9.5) virtio mode\n", pci);
        vp->legacy.bar = 0;
        vp->legacy.ioaddr = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        ଯଦି (!vp->legacy.ioaddr)
            ଫେରନ୍ତୁ;
        vp->legacy.mode = VP_ACCESS_IO;
    }

    vp_reset(vp);
    pci_enable_busmaster(pci);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER );
}

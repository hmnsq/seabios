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

#অন্তর্ভাব "config.h" // CONFIG_DEBUG_LEVEL
#অন্তর্ভাব "malloc.h" // free
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "pci.h" // pci_config_readl
#অন্তর্ভাব "pcidevice.h" // struct pci_device
#অন্তর্ভাব "pci_regs.h" // PCI_BASE_ADDRESS_0
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "virtio-pci.h"
#অন্তর্ভাব "virtio-mmio.h"
#অন্তর্ভাব "virtio-ring.h"

u64 _vp_read(কাঠামো vp_cap *cap, u32 offset, u8 আকার_2)
{
    u64 var = 0;

    নির্বাচন (cap->mode) {
    ক্ষেত্র VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        নির্বাচন (আকার_2) {
        ক্ষেত্র 8:
            var = inl(addr);
            var |= (u64)inl(addr+4) << 32;
            বিরতি;
        ক্ষেত্র 4:
            var = inl(addr);
            বিরতি;
        ক্ষেত্র 2:
            var = inw(addr);
            বিরতি;
        ক্ষেত্র 1:
            var = inb(addr);
            বিরতি;
        }
        বিরতি;
    }

    ক্ষেত্র VP_ACCESS_MMIO:
    {
        শূন্য *addr = cap->memaddr + offset;
        নির্বাচন (আকার_2) {
        ক্ষেত্র 8:
            var = readl(addr);
            var |= (u64)readl(addr+4) << 32;
            বিরতি;
        ক্ষেত্র 4:
            var = readl(addr);
            বিরতি;
        ক্ষেত্র 2:
            var = readw(addr);
            বিরতি;
        ক্ষেত্র 1:
            var = readb(addr);
            বিরতি;
        }
        বিরতি;
    }

    ক্ষেত্র VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(কাঠামো virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(কাঠামো virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(কাঠামো virtio_pci_cfg_cap, cap.দৈর্ঘ্য),
                          (আকার_2 > 4) ? 4 : আকার_2);
        নির্বাচন (আকার_2) {
        ক্ষেত্র 8:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(কাঠামো virtio_pci_cfg_cap, pci_cfg_তথ্য));
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(কাঠামো virtio_pci_cfg_cap, cap.offset),
                              addr + 4);
            var |= (u64)pci_config_readl(cap->bdf, cap->cfg +
                                         offsetof(কাঠামো virtio_pci_cfg_cap, pci_cfg_তথ্য)) << 32;
            বিরতি;
        ক্ষেত্র 4:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(কাঠামো virtio_pci_cfg_cap, pci_cfg_তথ্য));
            বিরতি;
        ক্ষেত্র 2:
            var = pci_config_readw(cap->bdf, cap->cfg +
                                   offsetof(কাঠামো virtio_pci_cfg_cap, pci_cfg_তথ্য));
            বিরতি;
        ক্ষেত্র 1:
            var = pci_config_readb(cap->bdf, cap->cfg +
                                   offsetof(কাঠামো virtio_pci_cfg_cap, pci_cfg_তথ্য));
            বিরতি;
        }
    }
    }
    dprintf(9, "vp read   %x (%d) -> 0x%llx\n", cap->ioaddr + offset, আকার_2, var);
    ফেরত var;
}

শূন্য _vp_write(কাঠামো vp_cap *cap, u32 offset, u8 আকার_2, u64 var)
{
    dprintf(9, "vp write  %x (%d) <- 0x%llx\n", cap->ioaddr + offset, আকার_2, var);

    নির্বাচন (cap->mode) {
    ক্ষেত্র VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        নির্বাচন (আকার_2) {
        ক্ষেত্র 4:
            outl(var, addr);
            বিরতি;
        ক্ষেত্র 2:
            outw(var, addr);
            বিরতি;
        ক্ষেত্র 1:
            outb(var, addr);
            বিরতি;
        }
        বিরতি;
    }

    ক্ষেত্র VP_ACCESS_MMIO:
    {
        শূন্য *addr = cap->memaddr + offset;
        নির্বাচন (আকার_2) {
        ক্ষেত্র 4:
            writel(addr, var);
            বিরতি;
        ক্ষেত্র 2:
            writew(addr, var);
            বিরতি;
        ক্ষেত্র 1:
            writeb(addr, var);
            বিরতি;
        }
        বিরতি;
    }

    ক্ষেত্র VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(কাঠামো virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(কাঠামো virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(কাঠামো virtio_pci_cfg_cap, cap.দৈর্ঘ্য),
                          আকার_2);
        নির্বাচন (আকার_2) {
        ক্ষেত্র 4:
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(কাঠামো virtio_pci_cfg_cap, pci_cfg_তথ্য),
                              var);
            বিরতি;
        ক্ষেত্র 2:
            pci_config_writew(cap->bdf, cap->cfg +
                              offsetof(কাঠামো virtio_pci_cfg_cap, pci_cfg_তথ্য),
                              var);
            বিরতি;
        ক্ষেত্র 1:
            pci_config_writeb(cap->bdf, cap->cfg +
                              offsetof(কাঠামো virtio_pci_cfg_cap, pci_cfg_তথ্য),
                              var);
            বিরতি;
        }
    }
    }
}

u64 vp_get_features(কাঠামো vp_device *vp)
{
    u32 f0, f1;

    যদি (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
    } অন্যথা যদি (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
    } অন্যথা {
        f0 = vp_read(&vp->legacy, virtio_pci_legacy, host_features);
        f1 = 0;
    }
    ফেরত ((u64)f1 << 32) | f0;
}

শূন্য vp_set_features(কাঠামো vp_device *vp, u64 features)
{
    u32 f0, f1;

    f0 = features;
    f1 = features >> 32;

    যদি (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f1);
    } অন্যথা যদি (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f1);
    } অন্যথা {
        vp_write(&vp->legacy, virtio_pci_legacy, guest_features, f0);
    }
}

u8 vp_get_status(কাঠামো vp_device *vp)
{
    যদি (vp->use_mmio) {
        ফেরত vp_read(&vp->common, virtio_mmio_cfg, device_status);
    } অন্যথা যদি (vp->use_modern) {
        ফেরত vp_read(&vp->common, virtio_pci_common_cfg, device_status);
    } অন্যথা {
        ফেরত vp_read(&vp->legacy, virtio_pci_legacy, status);
    }
}

শূন্য vp_set_status(কাঠামো vp_device *vp, u8 status)
{
    যদি (status == 0)        /* reset */
        ফেরত;
    যদি (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, status);
    } অন্যথা যদি (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, status);
    } অন্যথা {
        vp_write(&vp->legacy, virtio_pci_legacy, status, status);
    }
}

u8 vp_get_isr(কাঠামো vp_device *vp)
{
    যদি (vp->use_mmio) {
        ফেরত vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } অন্যথা যদি (vp->use_modern) {
        ফেরত vp_read(&vp->isr, virtio_pci_isr, isr);
    } অন্যথা {
        ফেরত vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

শূন্য vp_reset(কাঠামো vp_device *vp)
{
    যদি (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, 0);
        vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } অন্যথা যদি (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, 0);
        vp_read(&vp->isr, virtio_pci_isr, isr);
    } অন্যথা {
        vp_write(&vp->legacy, virtio_pci_legacy, status, 0);
        vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

শূন্য vp_notify(কাঠামো vp_device *vp, কাঠামো vring_virtqueue *vq)
{
    যদি (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, queue_notify, vq->queue_সূচক);
    } অন্যথা যদি (vp->use_modern) {
        u32 offset = vq->queue_notify_off * vp->notify_off_multiplier;
        নির্বাচন (vp->notify.mode) {
        ক্ষেত্র VP_ACCESS_IO:
            outw(vq->queue_সূচক, vp->notify.ioaddr + offset);
            বিরতি;
        ক্ষেত্র VP_ACCESS_MMIO:
            writew(vp->notify.memaddr + offset, vq->queue_সূচক);
            বিরতি;
        ক্ষেত্র VP_ACCESS_PCICFG:
            pci_config_writeb(vp->notify.bdf, vp->notify.cfg +
                              offsetof(কাঠামো virtio_pci_cfg_cap, cap.bar),
                              vp->notify.bar);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(কাঠামো virtio_pci_cfg_cap, cap.offset),
                              vp->notify.baroff + offset);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(কাঠামো virtio_pci_cfg_cap, cap.দৈর্ঘ্য),
                              2);
            pci_config_writew(vp->notify.bdf, vp->notify.cfg +
                              offsetof(কাঠামো virtio_pci_cfg_cap, pci_cfg_তথ্য),
                              vq->queue_সূচক);
        }
        dprintf(9, "vp notify %x (%d) -- 0x%x\n",
                vp->notify.ioaddr, 2, vq->queue_সূচক);
    } অন্যথা {
        vp_write(&vp->legacy, virtio_pci_legacy, queue_notify, vq->queue_সূচক);
    }
}

পূর্ণসংখ্যা vp_find_vq(কাঠামো vp_device *vp, পূর্ণসংখ্যা queue_সূচক,
               কাঠামো vring_virtqueue **p_vq)
{
   u16 অঙ্ক;

   ASSERT32FLAT();
   কাঠামো vring_virtqueue *vq = *p_vq = memalign_উপরের(PAGE_আকার, আকার(*vq));
   যদি (!vq) {
       warn_noalloc();
       যাও fail;
   }
   memset(vq, 0, আকার(*vq));


   /* select the queue */
   যদি (vp->use_mmio) {
       vp_write(&vp->common, virtio_mmio_cfg, queue_select, queue_সূচক);
   } অন্যথা যদি (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, queue_select, queue_সূচক);
   } অন্যথা {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_sel, queue_সূচক);
   }

   /* check if the queue is available */
   যদি (vp->use_mmio) {
       অঙ্ক = vp_read(&vp->common, virtio_mmio_cfg, queue_অঙ্ক_max);
       যদি (অঙ্ক > MAX_QUEUE_অঙ্ক)
           অঙ্ক = MAX_QUEUE_অঙ্ক;
       vp_write(&vp->common, virtio_mmio_cfg, queue_অঙ্ক, অঙ্ক);
   } অন্যথা যদি (vp->use_modern) {
       অঙ্ক = vp_read(&vp->common, virtio_pci_common_cfg, queue_আকার);
       যদি (অঙ্ক > MAX_QUEUE_অঙ্ক) {
           vp_write(&vp->common, virtio_pci_common_cfg, queue_আকার,
                    MAX_QUEUE_অঙ্ক);
           অঙ্ক = vp_read(&vp->common, virtio_pci_common_cfg, queue_আকার);
       }
   } অন্যথা {
       অঙ্ক = vp_read(&vp->legacy, virtio_pci_legacy, queue_অঙ্ক);
   }
   যদি (!অঙ্ক) {
       dprintf(1, "ERROR: queue size is 0\n");
       যাও fail;
   }
   যদি (অঙ্ক > MAX_QUEUE_অঙ্ক) {
       dprintf(1, "ERROR: queue size %d > %d\n", অঙ্ক, MAX_QUEUE_অঙ্ক);
       যাও fail;
   }

   /* check if the queue is already active */
   যদি (vp->use_mmio) {
       /* TODO */;
   } অন্যথা যদি (vp->use_modern) {
       যদি (vp_read(&vp->common, virtio_pci_common_cfg, queue_enable)) {
           dprintf(1, "ERROR: queue already active\n");
           যাও fail;
       }
   } অন্যথা {
       যদি (vp_read(&vp->legacy, virtio_pci_legacy, queue_pfn)) {
           dprintf(1, "ERROR: queue already active\n");
           যাও fail;
       }
   }
   vq->queue_সূচক = queue_সূচক;

   /* initialize the queue */
   কাঠামো vring * vr = &vq->vring;
   vring_init(vr, অঙ্ক, (unsigned অক্ষর*)&vq->queue);

   /* activate the queue
    *
    * NOTE: vr->desc is initialized by vring_init()
    */

   যদি (vp->use_mmio) {
       যদি (vp_read(&vp->common, virtio_mmio_cfg, version) == 2) {
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
       } অন্যথা {
           vp_write(&vp->common, virtio_mmio_cfg, legacy_guest_page_আকার,
                    (unsigned long)1 << PAGE_SHIFT);
           vp_write(&vp->common, virtio_mmio_cfg, legacy_queue_pfn,
                    (unsigned long)virt_to_phys(vr->desc) >> PAGE_SHIFT);
       }
   } অন্যথা যদি (vp->use_modern) {
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
   } অন্যথা {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_pfn,
                (unsigned long)virt_to_phys(vr->desc) >> PAGE_SHIFT);
   }
   ফেরত অঙ্ক;

fail:
   মুক্ত(vq);
   *p_vq = ফাঁকা;
   ফেরত -1;
}

শূন্য vp_init_simple(কাঠামো vp_device *vp, কাঠামো pci_device *pci)
{
    u8 cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, 0);
    কাঠামো vp_cap *vp_cap;
    ধ্রুবক অক্ষর *mode;
    u32 offset, base, mul;
    u64 addr;
    u8 type;

    memset(vp, 0, আকার(*vp));
    যখন (cap != 0) {
        type = pci_config_readb(pci->bdf, cap +
                                offsetof(কাঠামো virtio_pci_cap, cfg_type));
        নির্বাচন (type) {
        ক্ষেত্র VIRTIO_PCI_CAP_COMMON_CFG:
            vp_cap = &vp->common;
            বিরতি;
        ক্ষেত্র VIRTIO_PCI_CAP_NOTIFY_CFG:
            vp_cap = &vp->notify;
            mul = offsetof(কাঠামো virtio_pci_notify_cap, notify_off_multiplier);
            vp->notify_off_multiplier = pci_config_readl(pci->bdf, cap + mul);
            বিরতি;
        ক্ষেত্র VIRTIO_PCI_CAP_ISR_CFG:
            vp_cap = &vp->isr;
            বিরতি;
        ক্ষেত্র VIRTIO_PCI_CAP_DEVICE_CFG:
            vp_cap = &vp->device;
            বিরতি;
        ক্ষেত্র VIRTIO_PCI_CAP_PCI_CFG:
            vp->common.cfg = cap;
            vp->common.bdf = pci->bdf;
            vp->notify.cfg = cap;
            vp->notify.bdf = pci->bdf;
            vp->isr.cfg = cap;
            vp->isr.bdf = pci->bdf;
            vp->device.cfg = cap;
            vp->device.bdf = pci->bdf;
            vp_cap = ফাঁকা;
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d"
                    " [pci cfg access]\n", pci, cap, type);
            বিরতি;
        পূর্বনির্ধারিত:
            vp_cap = ফাঁকা;
            বিরতি;
        }
        যদি (vp_cap && !vp_cap->cap) {
            vp_cap->cap = cap;
            vp_cap->bar = pci_config_readb(pci->bdf, cap +
                                           offsetof(কাঠামো virtio_pci_cap, bar));
            offset = pci_config_readl(pci->bdf, cap +
                                      offsetof(কাঠামো virtio_pci_cap, offset));
            base = PCI_BASE_ADDRESS_0 + 4 * vp_cap->bar;
            addr = pci_config_readl(pci->bdf, base);
            যদি (addr & PCI_BASE_ADDRESS_SPACE_IO) {
                addr &= PCI_BASE_ADDRESS_IO_MASK;
                vp_cap->mode = VP_ACCESS_IO;
            } অন্যথা যদি ((addr & PCI_BASE_ADDRESS_MEM_TYPE_MASK) ==
                       PCI_BASE_ADDRESS_MEM_TYPE_64) {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                addr |= (u64)pci_config_readl(pci->bdf, base + 4) << 32;
                vp_cap->mode = (addr > 0xffffffffll) ?
                    VP_ACCESS_PCICFG : VP_ACCESS_MMIO;
            } অন্যথা {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                vp_cap->mode = VP_ACCESS_MMIO;
            }
            নির্বাচন (vp_cap->mode) {
            ক্ষেত্র VP_ACCESS_IO:
            {
                u32 addr = pci_enable_iobar(pci, base);
                যদি (!addr)
                    ফেরত;
                vp_cap->ioaddr = addr + offset;
                mode = "io";
                বিরতি;
            }
            ক্ষেত্র VP_ACCESS_MMIO:
            {
                শূন্য *addr = pci_enable_membar(pci, base);
                যদি (!addr)
                    ফেরত;
                vp_cap->memaddr = addr + offset;
                mode = "mmio";
                বিরতি;
            }
            ক্ষেত্র VP_ACCESS_PCICFG:
                mode = "pcicfg";
                vp_cap->baroff = offset;
                বিরতি;
            পূর্বনির্ধারিত:
                mode = "Huh?";
                বিরতি;
            }
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d "
                    "bar %d at 0x%08llx off +0x%04x [%s]\n",
                    pci, vp_cap->cap, type, vp_cap->bar, addr, offset, mode);
        }

        cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, cap);
    }

    যদি (vp->common.cap && vp->notify.cap && vp->isr.cap && vp->device.cap) {
        dprintf(1, "pci dev %pP using modern (1.0) virtio mode\n", pci);
        vp->use_modern = 1;
    } অন্যথা {
        dprintf(1, "pci dev %pP using legacy (0.9.5) virtio mode\n", pci);
        vp->legacy.bar = 0;
        vp->legacy.ioaddr = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        যদি (!vp->legacy.ioaddr)
            ফেরত;
        vp->legacy.mode = VP_ACCESS_IO;
    }

    vp_reset(vp);
    pci_enable_busmaster(pci);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER );
}

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

u64 _vp_read(সংরচনা vp_cap *cap, u32 offset, u8 size)
{
    u64 var = 0;

    বিকল্প (cap->mode) {
    পক্ষ VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        বিকল্প (size) {
        পক্ষ 8:
            var = inl(addr);
            var |= (u64)inl(addr+4) << 32;
            ভঙ্গ;
        পক্ষ 4:
            var = inl(addr);
            ভঙ্গ;
        পক্ষ 2:
            var = inw(addr);
            ভঙ্গ;
        পক্ষ 1:
            var = inb(addr);
            ভঙ্গ;
        }
        ভঙ্গ;
    }

    পক্ষ VP_ACCESS_MMIO:
    {
        শূন্য *addr = cap->memaddr + offset;
        বিকল্প (size) {
        পক্ষ 8:
            var = readl(addr);
            var |= (u64)readl(addr+4) << 32;
            ভঙ্গ;
        পক্ষ 4:
            var = readl(addr);
            ভঙ্গ;
        পক্ষ 2:
            var = readw(addr);
            ভঙ্গ;
        পক্ষ 1:
            var = readb(addr);
            ভঙ্গ;
        }
        ভঙ্গ;
    }

    পক্ষ VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(সংরচনা virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(সংরচনা virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(সংরচনা virtio_pci_cfg_cap, cap.length),
                          (size > 4) ? 4 : size);
        বিকল্প (size) {
        পক্ষ 8:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(সংরচনা virtio_pci_cfg_cap, pci_cfg_data));
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(সংরচনা virtio_pci_cfg_cap, cap.offset),
                              addr + 4);
            var |= (u64)pci_config_readl(cap->bdf, cap->cfg +
                                         offsetof(সংরচনা virtio_pci_cfg_cap, pci_cfg_data)) << 32;
            ভঙ্গ;
        পক্ষ 4:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   offsetof(সংরচনা virtio_pci_cfg_cap, pci_cfg_data));
            ভঙ্গ;
        পক্ষ 2:
            var = pci_config_readw(cap->bdf, cap->cfg +
                                   offsetof(সংরচনা virtio_pci_cfg_cap, pci_cfg_data));
            ভঙ্গ;
        পক্ষ 1:
            var = pci_config_readb(cap->bdf, cap->cfg +
                                   offsetof(সংরচনা virtio_pci_cfg_cap, pci_cfg_data));
            ভঙ্গ;
        }
    }
    }
    dprintf(9, "vp read   %x (%d) -> 0x%llx\n", cap->ioaddr + offset, size, var);
    প্রত্যাগম var;
}

শূন্য _vp_write(সংরচনা vp_cap *cap, u32 offset, u8 size, u64 var)
{
    dprintf(9, "vp write  %x (%d) <- 0x%llx\n", cap->ioaddr + offset, size, var);

    বিকল্প (cap->mode) {
    পক্ষ VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        বিকল্প (size) {
        পক্ষ 4:
            outl(var, addr);
            ভঙ্গ;
        পক্ষ 2:
            outw(var, addr);
            ভঙ্গ;
        পক্ষ 1:
            outb(var, addr);
            ভঙ্গ;
        }
        ভঙ্গ;
    }

    পক্ষ VP_ACCESS_MMIO:
    {
        শূন্য *addr = cap->memaddr + offset;
        বিকল্প (size) {
        পক্ষ 4:
            writel(addr, var);
            ভঙ্গ;
        পক্ষ 2:
            writew(addr, var);
            ভঙ্গ;
        পক্ষ 1:
            writeb(addr, var);
            ভঙ্গ;
        }
        ভঙ্গ;
    }

    পক্ষ VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          offsetof(সংরচনা virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(সংরচনা virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          offsetof(সংরচনা virtio_pci_cfg_cap, cap.length),
                          size);
        বিকল্প (size) {
        পক্ষ 4:
            pci_config_writel(cap->bdf, cap->cfg +
                              offsetof(সংরচনা virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            ভঙ্গ;
        পক্ষ 2:
            pci_config_writew(cap->bdf, cap->cfg +
                              offsetof(সংরচনা virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            ভঙ্গ;
        পক্ষ 1:
            pci_config_writeb(cap->bdf, cap->cfg +
                              offsetof(সংরচনা virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            ভঙ্গ;
        }
    }
    }
}

u64 vp_get_features(সংরচনা vp_device *vp)
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
    প্রত্যাগম ((u64)f1 << 32) | f0;
}

শূন্য vp_set_features(সংরচনা vp_device *vp, u64 features)
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

u8 vp_get_status(সংরচনা vp_device *vp)
{
    যদি (vp->use_mmio) {
        প্রত্যাগম vp_read(&vp->common, virtio_mmio_cfg, device_status);
    } অন্যথা যদি (vp->use_modern) {
        প্রত্যাগম vp_read(&vp->common, virtio_pci_common_cfg, device_status);
    } অন্যথা {
        প্রত্যাগম vp_read(&vp->legacy, virtio_pci_legacy, status);
    }
}

শূন্য vp_set_status(সংরচনা vp_device *vp, u8 status)
{
    যদি (status == 0)        /* reset */
        প্রত্যাগম;
    যদি (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, status);
    } অন্যথা যদি (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, status);
    } অন্যথা {
        vp_write(&vp->legacy, virtio_pci_legacy, status, status);
    }
}

u8 vp_get_isr(সংরচনা vp_device *vp)
{
    যদি (vp->use_mmio) {
        প্রত্যাগম vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } অন্যথা যদি (vp->use_modern) {
        প্রত্যাগম vp_read(&vp->isr, virtio_pci_isr, isr);
    } অন্যথা {
        প্রত্যাগম vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

শূন্য vp_reset(সংরচনা vp_device *vp)
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

শূন্য vp_notify(সংরচনা vp_device *vp, সংরচনা vring_virtqueue *vq)
{
    যদি (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, queue_notify, vq->queue_index);
    } অন্যথা যদি (vp->use_modern) {
        u32 offset = vq->queue_notify_off * vp->notify_off_multiplier;
        বিকল্প (vp->notify.mode) {
        পক্ষ VP_ACCESS_IO:
            outw(vq->queue_index, vp->notify.ioaddr + offset);
            ভঙ্গ;
        পক্ষ VP_ACCESS_MMIO:
            writew(vp->notify.memaddr + offset, vq->queue_index);
            ভঙ্গ;
        পক্ষ VP_ACCESS_PCICFG:
            pci_config_writeb(vp->notify.bdf, vp->notify.cfg +
                              offsetof(সংরচনা virtio_pci_cfg_cap, cap.bar),
                              vp->notify.bar);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(সংরচনা virtio_pci_cfg_cap, cap.offset),
                              vp->notify.baroff + offset);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              offsetof(সংরচনা virtio_pci_cfg_cap, cap.length),
                              2);
            pci_config_writew(vp->notify.bdf, vp->notify.cfg +
                              offsetof(সংরচনা virtio_pci_cfg_cap, pci_cfg_data),
                              vq->queue_index);
        }
        dprintf(9, "vp notify %x (%d) -- 0x%x\n",
                vp->notify.ioaddr, 2, vq->queue_index);
    } অন্যথা {
        vp_write(&vp->legacy, virtio_pci_legacy, queue_notify, vq->queue_index);
    }
}

পূর্ণাঙ্ক vp_find_vq(সংরচনা vp_device *vp, পূর্ণাঙ্ক queue_index,
               সংরচনা vring_virtqueue **p_vq)
{
   u16 অঙ্ক;

   ASSERT32FLAT();
   সংরচনা vring_virtqueue *vq = *p_vq = memalign_উপরের(PAGE_SIZE, পরিমাণ(*vq));
   যদি (!vq) {
       warn_noalloc();
       গচ্ছ fail;
   }
   memset(vq, 0, পরিমাণ(*vq));


   /* select the queue */
   যদি (vp->use_mmio) {
       vp_write(&vp->common, virtio_mmio_cfg, queue_select, queue_index);
   } অন্যথা যদি (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, queue_select, queue_index);
   } অন্যথা {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_sel, queue_index);
   }

   /* check if the queue is available */
   যদি (vp->use_mmio) {
       অঙ্ক = vp_read(&vp->common, virtio_mmio_cfg, queue_অঙ্ক_max);
       যদি (অঙ্ক > MAX_QUEUE_অঙ্ক)
           অঙ্ক = MAX_QUEUE_অঙ্ক;
       vp_write(&vp->common, virtio_mmio_cfg, queue_অঙ্ক, অঙ্ক);
   } অন্যথা যদি (vp->use_modern) {
       অঙ্ক = vp_read(&vp->common, virtio_pci_common_cfg, queue_size);
       যদি (অঙ্ক > MAX_QUEUE_অঙ্ক) {
           vp_write(&vp->common, virtio_pci_common_cfg, queue_size,
                    MAX_QUEUE_অঙ্ক);
           অঙ্ক = vp_read(&vp->common, virtio_pci_common_cfg, queue_size);
       }
   } অন্যথা {
       অঙ্ক = vp_read(&vp->legacy, virtio_pci_legacy, queue_অঙ্ক);
   }
   যদি (!অঙ্ক) {
       dprintf(1, "ERROR: queue size is 0\n");
       গচ্ছ fail;
   }
   যদি (অঙ্ক > MAX_QUEUE_অঙ্ক) {
       dprintf(1, "ERROR: queue size %d > %d\n", অঙ্ক, MAX_QUEUE_অঙ্ক);
       গচ্ছ fail;
   }

   /* check if the queue is already active */
   যদি (vp->use_mmio) {
       /* TODO */;
   } অন্যথা যদি (vp->use_modern) {
       যদি (vp_read(&vp->common, virtio_pci_common_cfg, queue_enable)) {
           dprintf(1, "ERROR: queue already active\n");
           গচ্ছ fail;
       }
   } অন্যথা {
       যদি (vp_read(&vp->legacy, virtio_pci_legacy, queue_pfn)) {
           dprintf(1, "ERROR: queue already active\n");
           গচ্ছ fail;
       }
   }
   vq->queue_index = queue_index;

   /* initialize the queue */
   সংরচনা vring * vr = &vq->vring;
   vring_init(vr, অঙ্ক, (unsigned বর্ণ*)&vq->queue);

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
           vp_write(&vp->common, virtio_mmio_cfg, legacy_guest_page_size,
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
   প্রত্যাগম অঙ্ক;

fail:
   মুক্ত(vq);
   *p_vq = রিক্ত;
   প্রত্যাগম -1;
}

শূন্য vp_init_simple(সংরচনা vp_device *vp, সংরচনা pci_device *pci)
{
    u8 cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, 0);
    সংরচনা vp_cap *vp_cap;
    ধ্রুব বর্ণ *mode;
    u32 offset, base, mul;
    u64 addr;
    u8 type;

    memset(vp, 0, পরিমাণ(*vp));
    যাবত্ (cap != 0) {
        type = pci_config_readb(pci->bdf, cap +
                                offsetof(সংরচনা virtio_pci_cap, cfg_type));
        বিকল্প (type) {
        পক্ষ VIRTIO_PCI_CAP_COMMON_CFG:
            vp_cap = &vp->common;
            ভঙ্গ;
        পক্ষ VIRTIO_PCI_CAP_NOTIFY_CFG:
            vp_cap = &vp->notify;
            mul = offsetof(সংরচনা virtio_pci_notify_cap, notify_off_multiplier);
            vp->notify_off_multiplier = pci_config_readl(pci->bdf, cap + mul);
            ভঙ্গ;
        পক্ষ VIRTIO_PCI_CAP_ISR_CFG:
            vp_cap = &vp->isr;
            ভঙ্গ;
        পক্ষ VIRTIO_PCI_CAP_DEVICE_CFG:
            vp_cap = &vp->device;
            ভঙ্গ;
        পক্ষ VIRTIO_PCI_CAP_PCI_CFG:
            vp->common.cfg = cap;
            vp->common.bdf = pci->bdf;
            vp->notify.cfg = cap;
            vp->notify.bdf = pci->bdf;
            vp->isr.cfg = cap;
            vp->isr.bdf = pci->bdf;
            vp->device.cfg = cap;
            vp->device.bdf = pci->bdf;
            vp_cap = রিক্ত;
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d"
                    " [pci cfg access]\n", pci, cap, type);
            ভঙ্গ;
        স্বভাব:
            vp_cap = রিক্ত;
            ভঙ্গ;
        }
        যদি (vp_cap && !vp_cap->cap) {
            vp_cap->cap = cap;
            vp_cap->bar = pci_config_readb(pci->bdf, cap +
                                           offsetof(সংরচনা virtio_pci_cap, bar));
            offset = pci_config_readl(pci->bdf, cap +
                                      offsetof(সংরচনা virtio_pci_cap, offset));
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
            বিকল্প (vp_cap->mode) {
            পক্ষ VP_ACCESS_IO:
            {
                u32 addr = pci_enable_iobar(pci, base);
                যদি (!addr)
                    প্রত্যাগম;
                vp_cap->ioaddr = addr + offset;
                mode = "io";
                ভঙ্গ;
            }
            পক্ষ VP_ACCESS_MMIO:
            {
                শূন্য *addr = pci_enable_membar(pci, base);
                যদি (!addr)
                    প্রত্যাগম;
                vp_cap->memaddr = addr + offset;
                mode = "mmio";
                ভঙ্গ;
            }
            পক্ষ VP_ACCESS_PCICFG:
                mode = "pcicfg";
                vp_cap->baroff = offset;
                ভঙ্গ;
            স্বভাব:
                mode = "Huh?";
                ভঙ্গ;
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
            প্রত্যাগম;
        vp->legacy.mode = VP_ACCESS_IO;
    }

    vp_reset(vp);
    pci_enable_busmaster(pci);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER );
}

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

#अन्तर्भाव "config.h" // CONFIG_DEBUG_LEVEL
#अन्तर्भाव "malloc.h" // free
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "pci.h" // pci_config_readl
#अन्तर्भाव "pcidevice.h" // struct pci_device
#अन्तर्भाव "pci_regs.h" // PCI_BASE_ADDRESS_0
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "virtio-pci.h"
#अन्तर्भाव "virtio-mmio.h"
#अन्तर्भाव "virtio-ring.h"

u64 _vp_read(संरचना vp_cap *cap, u32 offset, u8 size)
{
    u64 var = 0;

    निवड (cap->mode) {
    स्थिती VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        निवड (size) {
        स्थिती 8:
            var = inl(addr);
            var |= (u64)inl(addr+4) << 32;
            थांबा;
        स्थिती 4:
            var = inl(addr);
            थांबा;
        स्थिती 2:
            var = inw(addr);
            थांबा;
        स्थिती 1:
            var = inb(addr);
            थांबा;
        }
        थांबा;
    }

    स्थिती VP_ACCESS_MMIO:
    {
        शून्य *addr = cap->memaddr + offset;
        निवड (size) {
        स्थिती 8:
            var = readl(addr);
            var |= (u64)readl(addr+4) << 32;
            थांबा;
        स्थिती 4:
            var = readl(addr);
            थांबा;
        स्थिती 2:
            var = readw(addr);
            थांबा;
        स्थिती 1:
            var = readb(addr);
            थांबा;
        }
        थांबा;
    }

    स्थिती VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          दुरत्व(संरचना virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          दुरत्व(संरचना virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          दुरत्व(संरचना virtio_pci_cfg_cap, cap.length),
                          (size > 4) ? 4 : size);
        निवड (size) {
        स्थिती 8:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   दुरत्व(संरचना virtio_pci_cfg_cap, pci_cfg_data));
            pci_config_writel(cap->bdf, cap->cfg +
                              दुरत्व(संरचना virtio_pci_cfg_cap, cap.offset),
                              addr + 4);
            var |= (u64)pci_config_readl(cap->bdf, cap->cfg +
                                         दुरत्व(संरचना virtio_pci_cfg_cap, pci_cfg_data)) << 32;
            थांबा;
        स्थिती 4:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   दुरत्व(संरचना virtio_pci_cfg_cap, pci_cfg_data));
            थांबा;
        स्थिती 2:
            var = pci_config_readw(cap->bdf, cap->cfg +
                                   दुरत्व(संरचना virtio_pci_cfg_cap, pci_cfg_data));
            थांबा;
        स्थिती 1:
            var = pci_config_readb(cap->bdf, cap->cfg +
                                   दुरत्व(संरचना virtio_pci_cfg_cap, pci_cfg_data));
            थांबा;
        }
    }
    }
    dprintf(9, "vp read   %x (%d) -> 0x%llx\n", cap->ioaddr + offset, size, var);
    परत var;
}

शून्य _vp_write(संरचना vp_cap *cap, u32 offset, u8 size, u64 var)
{
    dprintf(9, "vp write  %x (%d) <- 0x%llx\n", cap->ioaddr + offset, size, var);

    निवड (cap->mode) {
    स्थिती VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        निवड (size) {
        स्थिती 4:
            outl(var, addr);
            थांबा;
        स्थिती 2:
            outw(var, addr);
            थांबा;
        स्थिती 1:
            outb(var, addr);
            थांबा;
        }
        थांबा;
    }

    स्थिती VP_ACCESS_MMIO:
    {
        शून्य *addr = cap->memaddr + offset;
        निवड (size) {
        स्थिती 4:
            writel(addr, var);
            थांबा;
        स्थिती 2:
            writew(addr, var);
            थांबा;
        स्थिती 1:
            writeb(addr, var);
            थांबा;
        }
        थांबा;
    }

    स्थिती VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          दुरत्व(संरचना virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          दुरत्व(संरचना virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          दुरत्व(संरचना virtio_pci_cfg_cap, cap.length),
                          size);
        निवड (size) {
        स्थिती 4:
            pci_config_writel(cap->bdf, cap->cfg +
                              दुरत्व(संरचना virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            थांबा;
        स्थिती 2:
            pci_config_writew(cap->bdf, cap->cfg +
                              दुरत्व(संरचना virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            थांबा;
        स्थिती 1:
            pci_config_writeb(cap->bdf, cap->cfg +
                              दुरत्व(संरचना virtio_pci_cfg_cap, pci_cfg_data),
                              var);
            थांबा;
        }
    }
    }
}

u64 vp_get_features(संरचना vp_device *vp)
{
    u32 f0, f1;

    जर (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
    } अन्यथा जर (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
    } अन्यथा {
        f0 = vp_read(&vp->legacy, virtio_pci_legacy, host_features);
        f1 = 0;
    }
    परत ((u64)f1 << 32) | f0;
}

शून्य vp_set_features(संरचना vp_device *vp, u64 features)
{
    u32 f0, f1;

    f0 = features;
    f1 = features >> 32;

    जर (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f1);
    } अन्यथा जर (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f1);
    } अन्यथा {
        vp_write(&vp->legacy, virtio_pci_legacy, guest_features, f0);
    }
}

u8 vp_get_status(संरचना vp_device *vp)
{
    जर (vp->use_mmio) {
        परत vp_read(&vp->common, virtio_mmio_cfg, device_status);
    } अन्यथा जर (vp->use_modern) {
        परत vp_read(&vp->common, virtio_pci_common_cfg, device_status);
    } अन्यथा {
        परत vp_read(&vp->legacy, virtio_pci_legacy, status);
    }
}

शून्य vp_set_status(संरचना vp_device *vp, u8 status)
{
    जर (status == 0)        /* reset */
        परत;
    जर (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, status);
    } अन्यथा जर (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, status);
    } अन्यथा {
        vp_write(&vp->legacy, virtio_pci_legacy, status, status);
    }
}

u8 vp_get_isr(संरचना vp_device *vp)
{
    जर (vp->use_mmio) {
        परत vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } अन्यथा जर (vp->use_modern) {
        परत vp_read(&vp->isr, virtio_pci_isr, isr);
    } अन्यथा {
        परत vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

शून्य vp_reset(संरचना vp_device *vp)
{
    जर (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, 0);
        vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } अन्यथा जर (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, 0);
        vp_read(&vp->isr, virtio_pci_isr, isr);
    } अन्यथा {
        vp_write(&vp->legacy, virtio_pci_legacy, status, 0);
        vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

शून्य vp_notify(संरचना vp_device *vp, संरचना vring_virtqueue *vq)
{
    जर (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, queue_notify, vq->queue_index);
    } अन्यथा जर (vp->use_modern) {
        u32 offset = vq->queue_notify_off * vp->notify_off_multiplier;
        निवड (vp->notify.mode) {
        स्थिती VP_ACCESS_IO:
            outw(vq->queue_index, vp->notify.ioaddr + offset);
            थांबा;
        स्थिती VP_ACCESS_MMIO:
            writew(vp->notify.memaddr + offset, vq->queue_index);
            थांबा;
        स्थिती VP_ACCESS_PCICFG:
            pci_config_writeb(vp->notify.bdf, vp->notify.cfg +
                              दुरत्व(संरचना virtio_pci_cfg_cap, cap.bar),
                              vp->notify.bar);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              दुरत्व(संरचना virtio_pci_cfg_cap, cap.offset),
                              vp->notify.baroff + offset);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              दुरत्व(संरचना virtio_pci_cfg_cap, cap.length),
                              2);
            pci_config_writew(vp->notify.bdf, vp->notify.cfg +
                              दुरत्व(संरचना virtio_pci_cfg_cap, pci_cfg_data),
                              vq->queue_index);
        }
        dprintf(9, "vp notify %x (%d) -- 0x%x\n",
                vp->notify.ioaddr, 2, vq->queue_index);
    } अन्यथा {
        vp_write(&vp->legacy, virtio_pci_legacy, queue_notify, vq->queue_index);
    }
}

पूर्णांक vp_find_vq(संरचना vp_device *vp, पूर्णांक queue_index,
               संरचना vring_virtqueue **p_vq)
{
   u16 अंक;

   ASSERT32FLAT();
   संरचना vring_virtqueue *vq = *p_vq = memalign_वरचा(PAGE_SIZE, आकार(*vq));
   जर (!vq) {
       warn_noalloc();
       जा fail;
   }
   स_रखो(vq, 0, आकार(*vq));


   /* select the queue */
   जर (vp->use_mmio) {
       vp_write(&vp->common, virtio_mmio_cfg, queue_select, queue_index);
   } अन्यथा जर (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, queue_select, queue_index);
   } अन्यथा {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_sel, queue_index);
   }

   /* check if the queue is available */
   जर (vp->use_mmio) {
       अंक = vp_read(&vp->common, virtio_mmio_cfg, queue_अंक_max);
       जर (अंक > MAX_QUEUE_अंक)
           अंक = MAX_QUEUE_अंक;
       vp_write(&vp->common, virtio_mmio_cfg, queue_अंक, अंक);
   } अन्यथा जर (vp->use_modern) {
       अंक = vp_read(&vp->common, virtio_pci_common_cfg, queue_size);
       जर (अंक > MAX_QUEUE_अंक) {
           vp_write(&vp->common, virtio_pci_common_cfg, queue_size,
                    MAX_QUEUE_अंक);
           अंक = vp_read(&vp->common, virtio_pci_common_cfg, queue_size);
       }
   } अन्यथा {
       अंक = vp_read(&vp->legacy, virtio_pci_legacy, queue_अंक);
   }
   जर (!अंक) {
       dprintf(1, "ERROR: queue size is 0\n");
       जा fail;
   }
   जर (अंक > MAX_QUEUE_अंक) {
       dprintf(1, "ERROR: queue size %d > %d\n", अंक, MAX_QUEUE_अंक);
       जा fail;
   }

   /* check if the queue is already active */
   जर (vp->use_mmio) {
       /* TODO */;
   } अन्यथा जर (vp->use_modern) {
       जर (vp_read(&vp->common, virtio_pci_common_cfg, queue_enable)) {
           dprintf(1, "ERROR: queue already active\n");
           जा fail;
       }
   } अन्यथा {
       जर (vp_read(&vp->legacy, virtio_pci_legacy, queue_pfn)) {
           dprintf(1, "ERROR: queue already active\n");
           जा fail;
       }
   }
   vq->queue_index = queue_index;

   /* initialize the queue */
   संरचना vring * vr = &vq->vring;
   vring_init(vr, अंक, (अचिन्हित अक्षर*)&vq->queue);

   /* activate the queue
    *
    * NOTE: vr->desc is initialized by vring_init()
    */

   जर (vp->use_mmio) {
       जर (vp_read(&vp->common, virtio_mmio_cfg, version) == 2) {
           vp_write(&vp->common, virtio_mmio_cfg, queue_desc_lo,
                    (अचिन्हित दीर्घ)virt_to_phys(vr->desc));
           vp_write(&vp->common, virtio_mmio_cfg, queue_desc_hi, 0);
           vp_write(&vp->common, virtio_mmio_cfg, queue_driver_lo,
                    (अचिन्हित दीर्घ)virt_to_phys(vr->avail));
           vp_write(&vp->common, virtio_mmio_cfg, queue_driver_hi, 0);
           vp_write(&vp->common, virtio_mmio_cfg, queue_device_lo,
                    (अचिन्हित दीर्घ)virt_to_phys(vr->used));
           vp_write(&vp->common, virtio_mmio_cfg, queue_device_hi, 0);
           vp_write(&vp->common, virtio_mmio_cfg, queue_ready, 1);
       } अन्यथा {
           vp_write(&vp->common, virtio_mmio_cfg, legacy_guest_page_size,
                    (अचिन्हित दीर्घ)1 << PAGE_SHIFT);
           vp_write(&vp->common, virtio_mmio_cfg, legacy_queue_pfn,
                    (अचिन्हित दीर्घ)virt_to_phys(vr->desc) >> PAGE_SHIFT);
       }
   } अन्यथा जर (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, queue_desc_lo,
                (अचिन्हित दीर्घ)virt_to_phys(vr->desc));
       vp_write(&vp->common, virtio_pci_common_cfg, queue_desc_hi, 0);
       vp_write(&vp->common, virtio_pci_common_cfg, queue_avail_lo,
                (अचिन्हित दीर्घ)virt_to_phys(vr->avail));
       vp_write(&vp->common, virtio_pci_common_cfg, queue_avail_hi, 0);
       vp_write(&vp->common, virtio_pci_common_cfg, queue_used_lo,
                (अचिन्हित दीर्घ)virt_to_phys(vr->used));
       vp_write(&vp->common, virtio_pci_common_cfg, queue_used_hi, 0);
       vp_write(&vp->common, virtio_pci_common_cfg, queue_enable, 1);
       vq->queue_notify_off = vp_read(&vp->common, virtio_pci_common_cfg,
                                      queue_notify_off);
   } अन्यथा {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_pfn,
                (अचिन्हित दीर्घ)virt_to_phys(vr->desc) >> PAGE_SHIFT);
   }
   परत अंक;

fail:
   मुक्त(vq);
   *p_vq = रिक्त;
   परत -1;
}

शून्य vp_init_simple(संरचना vp_device *vp, संरचना pci_device *pci)
{
    u8 cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, 0);
    संरचना vp_cap *vp_cap;
    स्थिरांक अक्षर *mode;
    u32 offset, base, mul;
    u64 addr;
    u8 type;

    स_रखो(vp, 0, आकार(*vp));
    पर्यंत (cap != 0) {
        type = pci_config_readb(pci->bdf, cap +
                                दुरत्व(संरचना virtio_pci_cap, cfg_type));
        निवड (type) {
        स्थिती VIRTIO_PCI_CAP_COMMON_CFG:
            vp_cap = &vp->common;
            थांबा;
        स्थिती VIRTIO_PCI_CAP_NOTIFY_CFG:
            vp_cap = &vp->notify;
            mul = दुरत्व(संरचना virtio_pci_notify_cap, notify_off_multiplier);
            vp->notify_off_multiplier = pci_config_readl(pci->bdf, cap + mul);
            थांबा;
        स्थिती VIRTIO_PCI_CAP_ISR_CFG:
            vp_cap = &vp->isr;
            थांबा;
        स्थिती VIRTIO_PCI_CAP_DEVICE_CFG:
            vp_cap = &vp->device;
            थांबा;
        स्थिती VIRTIO_PCI_CAP_PCI_CFG:
            vp->common.cfg = cap;
            vp->common.bdf = pci->bdf;
            vp->notify.cfg = cap;
            vp->notify.bdf = pci->bdf;
            vp->isr.cfg = cap;
            vp->isr.bdf = pci->bdf;
            vp->device.cfg = cap;
            vp->device.bdf = pci->bdf;
            vp_cap = रिक्त;
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d"
                    " [pci cfg access]\n", pci, cap, type);
            थांबा;
        पूर्वनिर्धारित:
            vp_cap = रिक्त;
            थांबा;
        }
        जर (vp_cap && !vp_cap->cap) {
            vp_cap->cap = cap;
            vp_cap->bar = pci_config_readb(pci->bdf, cap +
                                           दुरत्व(संरचना virtio_pci_cap, bar));
            offset = pci_config_readl(pci->bdf, cap +
                                      दुरत्व(संरचना virtio_pci_cap, offset));
            base = PCI_BASE_ADDRESS_0 + 4 * vp_cap->bar;
            addr = pci_config_readl(pci->bdf, base);
            जर (addr & PCI_BASE_ADDRESS_SPACE_IO) {
                addr &= PCI_BASE_ADDRESS_IO_MASK;
                vp_cap->mode = VP_ACCESS_IO;
            } अन्यथा जर ((addr & PCI_BASE_ADDRESS_MEM_TYPE_MASK) ==
                       PCI_BASE_ADDRESS_MEM_TYPE_64) {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                addr |= (u64)pci_config_readl(pci->bdf, base + 4) << 32;
                vp_cap->mode = (addr > 0xffffffffll) ?
                    VP_ACCESS_PCICFG : VP_ACCESS_MMIO;
            } अन्यथा {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                vp_cap->mode = VP_ACCESS_MMIO;
            }
            निवड (vp_cap->mode) {
            स्थिती VP_ACCESS_IO:
            {
                u32 addr = pci_enable_iobar(pci, base);
                जर (!addr)
                    परत;
                vp_cap->ioaddr = addr + offset;
                mode = "io";
                थांबा;
            }
            स्थिती VP_ACCESS_MMIO:
            {
                शून्य *addr = pci_enable_membar(pci, base);
                जर (!addr)
                    परत;
                vp_cap->memaddr = addr + offset;
                mode = "mmio";
                थांबा;
            }
            स्थिती VP_ACCESS_PCICFG:
                mode = "pcicfg";
                vp_cap->baroff = offset;
                थांबा;
            पूर्वनिर्धारित:
                mode = "Huh?";
                थांबा;
            }
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d "
                    "bar %d at 0x%08llx off +0x%04x [%s]\n",
                    pci, vp_cap->cap, type, vp_cap->bar, addr, offset, mode);
        }

        cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, cap);
    }

    जर (vp->common.cap && vp->notify.cap && vp->isr.cap && vp->device.cap) {
        dprintf(1, "pci dev %pP using modern (1.0) virtio mode\n", pci);
        vp->use_modern = 1;
    } अन्यथा {
        dprintf(1, "pci dev %pP using legacy (0.9.5) virtio mode\n", pci);
        vp->legacy.bar = 0;
        vp->legacy.ioaddr = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        जर (!vp->legacy.ioaddr)
            परत;
        vp->legacy.mode = VP_ACCESS_IO;
    }

    vp_reset(vp);
    pci_enable_busmaster(pci);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER );
}

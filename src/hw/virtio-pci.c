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

#समावेश "config.h" // CONFIG_DEBUG_LEVEL
#समावेश "malloc.h" // free
#समावेश "output.h" // dprintf
#समावेश "pci.h" // pci_config_readl
#समावेश "pcidevice.h" // struct pci_device
#समावेश "pci_regs.h" // PCI_BASE_ADDRESS_0
#समावेश "string.h" // memset
#समावेश "virtio-pci.h"
#समावेश "virtio-mmio.h"
#समावेश "virtio-ring.h"

u64 _vp_read(काष्ठा vp_cap *cap, u32 offset, u8 आकार)
{
    u64 var = 0;

    चयन (cap->mode) {
    हाल VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        चयन (आकार) {
        हाल 8:
            var = inl(addr);
            var |= (u64)inl(addr+4) << 32;
            अवरोध;
        हाल 4:
            var = inl(addr);
            अवरोध;
        हाल 2:
            var = inw(addr);
            अवरोध;
        हाल 1:
            var = inb(addr);
            अवरोध;
        }
        अवरोध;
    }

    हाल VP_ACCESS_MMIO:
    {
        व्योम *addr = cap->memaddr + offset;
        चयन (आकार) {
        हाल 8:
            var = readl(addr);
            var |= (u64)readl(addr+4) << 32;
            अवरोध;
        हाल 4:
            var = readl(addr);
            अवरोध;
        हाल 2:
            var = readw(addr);
            अवरोध;
        हाल 1:
            var = readb(addr);
            अवरोध;
        }
        अवरोध;
    }

    हाल VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          दुरत्व(काष्ठा virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          दुरत्व(काष्ठा virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          दुरत्व(काष्ठा virtio_pci_cfg_cap, cap.लंबाई),
                          (आकार > 4) ? 4 : आकार);
        चयन (आकार) {
        हाल 8:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   दुरत्व(काष्ठा virtio_pci_cfg_cap, pci_cfg_तथ्य));
            pci_config_writel(cap->bdf, cap->cfg +
                              दुरत्व(काष्ठा virtio_pci_cfg_cap, cap.offset),
                              addr + 4);
            var |= (u64)pci_config_readl(cap->bdf, cap->cfg +
                                         दुरत्व(काष्ठा virtio_pci_cfg_cap, pci_cfg_तथ्य)) << 32;
            अवरोध;
        हाल 4:
            var = pci_config_readl(cap->bdf, cap->cfg +
                                   दुरत्व(काष्ठा virtio_pci_cfg_cap, pci_cfg_तथ्य));
            अवरोध;
        हाल 2:
            var = pci_config_readw(cap->bdf, cap->cfg +
                                   दुरत्व(काष्ठा virtio_pci_cfg_cap, pci_cfg_तथ्य));
            अवरोध;
        हाल 1:
            var = pci_config_readb(cap->bdf, cap->cfg +
                                   दुरत्व(काष्ठा virtio_pci_cfg_cap, pci_cfg_तथ्य));
            अवरोध;
        }
    }
    }
    dprintf(9, "vp read   %x (%d) -> 0x%llx\n", cap->ioaddr + offset, आकार, var);
    वापस var;
}

व्योम _vp_write(काष्ठा vp_cap *cap, u32 offset, u8 आकार, u64 var)
{
    dprintf(9, "vp write  %x (%d) <- 0x%llx\n", cap->ioaddr + offset, आकार, var);

    चयन (cap->mode) {
    हाल VP_ACCESS_IO:
    {
        u32 addr = cap->ioaddr + offset;
        चयन (आकार) {
        हाल 4:
            outl(var, addr);
            अवरोध;
        हाल 2:
            outw(var, addr);
            अवरोध;
        हाल 1:
            outb(var, addr);
            अवरोध;
        }
        अवरोध;
    }

    हाल VP_ACCESS_MMIO:
    {
        व्योम *addr = cap->memaddr + offset;
        चयन (आकार) {
        हाल 4:
            writel(addr, var);
            अवरोध;
        हाल 2:
            writew(addr, var);
            अवरोध;
        हाल 1:
            writeb(addr, var);
            अवरोध;
        }
        अवरोध;
    }

    हाल VP_ACCESS_PCICFG:
    {
        u32 addr = cap->baroff + offset;
        pci_config_writeb(cap->bdf, cap->cfg +
                          दुरत्व(काष्ठा virtio_pci_cfg_cap, cap.bar),
                          cap->bar);
        pci_config_writel(cap->bdf, cap->cfg +
                          दुरत्व(काष्ठा virtio_pci_cfg_cap, cap.offset),
                          addr);
        pci_config_writel(cap->bdf, cap->cfg +
                          दुरत्व(काष्ठा virtio_pci_cfg_cap, cap.लंबाई),
                          आकार);
        चयन (आकार) {
        हाल 4:
            pci_config_writel(cap->bdf, cap->cfg +
                              दुरत्व(काष्ठा virtio_pci_cfg_cap, pci_cfg_तथ्य),
                              var);
            अवरोध;
        हाल 2:
            pci_config_writew(cap->bdf, cap->cfg +
                              दुरत्व(काष्ठा virtio_pci_cfg_cap, pci_cfg_तथ्य),
                              var);
            अवरोध;
        हाल 1:
            pci_config_writeb(cap->bdf, cap->cfg +
                              दुरत्व(काष्ठा virtio_pci_cfg_cap, pci_cfg_तथ्य),
                              var);
            अवरोध;
        }
    }
    }
}

u64 vp_get_features(काष्ठा vp_device *vp)
{
    u32 f0, f1;

    अगर (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
        vp_write(&vp->common, virtio_mmio_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_mmio_cfg, device_feature);
    } अन्यथा अगर (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 0);
        f0 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
        vp_write(&vp->common, virtio_pci_common_cfg, device_feature_select, 1);
        f1 = vp_read(&vp->common, virtio_pci_common_cfg, device_feature);
    } अन्यथा {
        f0 = vp_read(&vp->legacy, virtio_pci_legacy, host_features);
        f1 = 0;
    }
    वापस ((u64)f1 << 32) | f0;
}

व्योम vp_set_features(काष्ठा vp_device *vp, u64 features)
{
    u32 f0, f1;

    f0 = features;
    f1 = features >> 32;

    अगर (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_mmio_cfg, guest_feature, f1);
    } अन्यथा अगर (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f0);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature_select, 1);
        vp_write(&vp->common, virtio_pci_common_cfg, guest_feature, f1);
    } अन्यथा {
        vp_write(&vp->legacy, virtio_pci_legacy, guest_features, f0);
    }
}

u8 vp_get_status(काष्ठा vp_device *vp)
{
    अगर (vp->use_mmio) {
        वापस vp_read(&vp->common, virtio_mmio_cfg, device_status);
    } अन्यथा अगर (vp->use_modern) {
        वापस vp_read(&vp->common, virtio_pci_common_cfg, device_status);
    } अन्यथा {
        वापस vp_read(&vp->legacy, virtio_pci_legacy, status);
    }
}

व्योम vp_set_status(काष्ठा vp_device *vp, u8 status)
{
    अगर (status == 0)        /* reset */
        वापस;
    अगर (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, status);
    } अन्यथा अगर (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, status);
    } अन्यथा {
        vp_write(&vp->legacy, virtio_pci_legacy, status, status);
    }
}

u8 vp_get_isr(काष्ठा vp_device *vp)
{
    अगर (vp->use_mmio) {
        वापस vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } अन्यथा अगर (vp->use_modern) {
        वापस vp_read(&vp->isr, virtio_pci_isr, isr);
    } अन्यथा {
        वापस vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

व्योम vp_reset(काष्ठा vp_device *vp)
{
    अगर (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, device_status, 0);
        vp_read(&vp->common, virtio_mmio_cfg, irq_status);
    } अन्यथा अगर (vp->use_modern) {
        vp_write(&vp->common, virtio_pci_common_cfg, device_status, 0);
        vp_read(&vp->isr, virtio_pci_isr, isr);
    } अन्यथा {
        vp_write(&vp->legacy, virtio_pci_legacy, status, 0);
        vp_read(&vp->legacy, virtio_pci_legacy, isr);
    }
}

व्योम vp_notify(काष्ठा vp_device *vp, काष्ठा vring_virtqueue *vq)
{
    अगर (vp->use_mmio) {
        vp_write(&vp->common, virtio_mmio_cfg, queue_notify, vq->queue_सूचकांक);
    } अन्यथा अगर (vp->use_modern) {
        u32 offset = vq->queue_notify_off * vp->notify_off_multiplier;
        चयन (vp->notify.mode) {
        हाल VP_ACCESS_IO:
            outw(vq->queue_सूचकांक, vp->notify.ioaddr + offset);
            अवरोध;
        हाल VP_ACCESS_MMIO:
            writew(vp->notify.memaddr + offset, vq->queue_सूचकांक);
            अवरोध;
        हाल VP_ACCESS_PCICFG:
            pci_config_writeb(vp->notify.bdf, vp->notify.cfg +
                              दुरत्व(काष्ठा virtio_pci_cfg_cap, cap.bar),
                              vp->notify.bar);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              दुरत्व(काष्ठा virtio_pci_cfg_cap, cap.offset),
                              vp->notify.baroff + offset);
            pci_config_writel(vp->notify.bdf, vp->notify.cfg +
                              दुरत्व(काष्ठा virtio_pci_cfg_cap, cap.लंबाई),
                              2);
            pci_config_writew(vp->notify.bdf, vp->notify.cfg +
                              दुरत्व(काष्ठा virtio_pci_cfg_cap, pci_cfg_तथ्य),
                              vq->queue_सूचकांक);
        }
        dprintf(9, "vp notify %x (%d) -- 0x%x\n",
                vp->notify.ioaddr, 2, vq->queue_सूचकांक);
    } अन्यथा {
        vp_write(&vp->legacy, virtio_pci_legacy, queue_notify, vq->queue_सूचकांक);
    }
}

पूर्णांक vp_find_vq(काष्ठा vp_device *vp, पूर्णांक queue_सूचकांक,
               काष्ठा vring_virtqueue **p_vq)
{
   u16 अंक;

   ASSERT32FLAT();
   काष्ठा vring_virtqueue *vq = *p_vq = memalign_ऊपरी(PAGE_आकार, माप(*vq));
   अगर (!vq) {
       warn_noalloc();
       जाओ fail;
   }
   स_रखो(vq, 0, माप(*vq));


   /* select the queue */
   अगर (vp->use_mmio) {
       vp_write(&vp->common, virtio_mmio_cfg, queue_select, queue_सूचकांक);
   } अन्यथा अगर (vp->use_modern) {
       vp_write(&vp->common, virtio_pci_common_cfg, queue_select, queue_सूचकांक);
   } अन्यथा {
       vp_write(&vp->legacy, virtio_pci_legacy, queue_sel, queue_सूचकांक);
   }

   /* check if the queue is available */
   अगर (vp->use_mmio) {
       अंक = vp_read(&vp->common, virtio_mmio_cfg, queue_अंक_max);
       अगर (अंक > MAX_QUEUE_अंक)
           अंक = MAX_QUEUE_अंक;
       vp_write(&vp->common, virtio_mmio_cfg, queue_अंक, अंक);
   } अन्यथा अगर (vp->use_modern) {
       अंक = vp_read(&vp->common, virtio_pci_common_cfg, queue_आकार);
       अगर (अंक > MAX_QUEUE_अंक) {
           vp_write(&vp->common, virtio_pci_common_cfg, queue_आकार,
                    MAX_QUEUE_अंक);
           अंक = vp_read(&vp->common, virtio_pci_common_cfg, queue_आकार);
       }
   } अन्यथा {
       अंक = vp_read(&vp->legacy, virtio_pci_legacy, queue_अंक);
   }
   अगर (!अंक) {
       dprintf(1, "ERROR: queue size is 0\n");
       जाओ fail;
   }
   अगर (अंक > MAX_QUEUE_अंक) {
       dprintf(1, "ERROR: queue size %d > %d\n", अंक, MAX_QUEUE_अंक);
       जाओ fail;
   }

   /* check if the queue is already active */
   अगर (vp->use_mmio) {
       /* TODO */;
   } अन्यथा अगर (vp->use_modern) {
       अगर (vp_read(&vp->common, virtio_pci_common_cfg, queue_enable)) {
           dprintf(1, "ERROR: queue already active\n");
           जाओ fail;
       }
   } अन्यथा {
       अगर (vp_read(&vp->legacy, virtio_pci_legacy, queue_pfn)) {
           dprintf(1, "ERROR: queue already active\n");
           जाओ fail;
       }
   }
   vq->queue_सूचकांक = queue_सूचकांक;

   /* initialize the queue */
   काष्ठा vring * vr = &vq->vring;
   vring_init(vr, अंक, (अचिन्हित अक्षर*)&vq->queue);

   /* activate the queue
    *
    * NOTE: vr->desc is initialized by vring_init()
    */

   अगर (vp->use_mmio) {
       अगर (vp_read(&vp->common, virtio_mmio_cfg, version) == 2) {
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
           vp_write(&vp->common, virtio_mmio_cfg, legacy_guest_page_आकार,
                    (अचिन्हित दीर्घ)1 << PAGE_SHIFT);
           vp_write(&vp->common, virtio_mmio_cfg, legacy_queue_pfn,
                    (अचिन्हित दीर्घ)virt_to_phys(vr->desc) >> PAGE_SHIFT);
       }
   } अन्यथा अगर (vp->use_modern) {
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
   वापस अंक;

fail:
   मुक्त(vq);
   *p_vq = शून्य;
   वापस -1;
}

व्योम vp_init_simple(काष्ठा vp_device *vp, काष्ठा pci_device *pci)
{
    u8 cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, 0);
    काष्ठा vp_cap *vp_cap;
    स्थिर अक्षर *mode;
    u32 offset, base, mul;
    u64 addr;
    u8 type;

    स_रखो(vp, 0, माप(*vp));
    जबतक (cap != 0) {
        type = pci_config_readb(pci->bdf, cap +
                                दुरत्व(काष्ठा virtio_pci_cap, cfg_type));
        चयन (type) {
        हाल VIRTIO_PCI_CAP_COMMON_CFG:
            vp_cap = &vp->common;
            अवरोध;
        हाल VIRTIO_PCI_CAP_NOTIFY_CFG:
            vp_cap = &vp->notify;
            mul = दुरत्व(काष्ठा virtio_pci_notify_cap, notify_off_multiplier);
            vp->notify_off_multiplier = pci_config_readl(pci->bdf, cap + mul);
            अवरोध;
        हाल VIRTIO_PCI_CAP_ISR_CFG:
            vp_cap = &vp->isr;
            अवरोध;
        हाल VIRTIO_PCI_CAP_DEVICE_CFG:
            vp_cap = &vp->device;
            अवरोध;
        हाल VIRTIO_PCI_CAP_PCI_CFG:
            vp->common.cfg = cap;
            vp->common.bdf = pci->bdf;
            vp->notify.cfg = cap;
            vp->notify.bdf = pci->bdf;
            vp->isr.cfg = cap;
            vp->isr.bdf = pci->bdf;
            vp->device.cfg = cap;
            vp->device.bdf = pci->bdf;
            vp_cap = शून्य;
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d"
                    " [pci cfg access]\n", pci, cap, type);
            अवरोध;
        शेष:
            vp_cap = शून्य;
            अवरोध;
        }
        अगर (vp_cap && !vp_cap->cap) {
            vp_cap->cap = cap;
            vp_cap->bar = pci_config_readb(pci->bdf, cap +
                                           दुरत्व(काष्ठा virtio_pci_cap, bar));
            offset = pci_config_readl(pci->bdf, cap +
                                      दुरत्व(काष्ठा virtio_pci_cap, offset));
            base = PCI_BASE_ADDRESS_0 + 4 * vp_cap->bar;
            addr = pci_config_readl(pci->bdf, base);
            अगर (addr & PCI_BASE_ADDRESS_SPACE_IO) {
                addr &= PCI_BASE_ADDRESS_IO_MASK;
                vp_cap->mode = VP_ACCESS_IO;
            } अन्यथा अगर ((addr & PCI_BASE_ADDRESS_MEM_TYPE_MASK) ==
                       PCI_BASE_ADDRESS_MEM_TYPE_64) {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                addr |= (u64)pci_config_readl(pci->bdf, base + 4) << 32;
                vp_cap->mode = (addr > 0xffffffffll) ?
                    VP_ACCESS_PCICFG : VP_ACCESS_MMIO;
            } अन्यथा {
                addr &= PCI_BASE_ADDRESS_MEM_MASK;
                vp_cap->mode = VP_ACCESS_MMIO;
            }
            चयन (vp_cap->mode) {
            हाल VP_ACCESS_IO:
            {
                u32 addr = pci_enable_iobar(pci, base);
                अगर (!addr)
                    वापस;
                vp_cap->ioaddr = addr + offset;
                mode = "io";
                अवरोध;
            }
            हाल VP_ACCESS_MMIO:
            {
                व्योम *addr = pci_enable_membar(pci, base);
                अगर (!addr)
                    वापस;
                vp_cap->memaddr = addr + offset;
                mode = "mmio";
                अवरोध;
            }
            हाल VP_ACCESS_PCICFG:
                mode = "pcicfg";
                vp_cap->baroff = offset;
                अवरोध;
            शेष:
                mode = "Huh?";
                अवरोध;
            }
            dprintf(1, "pci dev %pP virtio cap at 0x%x type %d "
                    "bar %d at 0x%08llx off +0x%04x [%s]\n",
                    pci, vp_cap->cap, type, vp_cap->bar, addr, offset, mode);
        }

        cap = pci_find_capability(pci->bdf, PCI_CAP_ID_VNDR, cap);
    }

    अगर (vp->common.cap && vp->notify.cap && vp->isr.cap && vp->device.cap) {
        dprintf(1, "pci dev %pP using modern (1.0) virtio mode\n", pci);
        vp->use_modern = 1;
    } अन्यथा {
        dprintf(1, "pci dev %pP using legacy (0.9.5) virtio mode\n", pci);
        vp->legacy.bar = 0;
        vp->legacy.ioaddr = pci_enable_iobar(pci, PCI_BASE_ADDRESS_0);
        अगर (!vp->legacy.ioaddr)
            वापस;
        vp->legacy.mode = VP_ACCESS_IO;
    }

    vp_reset(vp);
    pci_enable_busmaster(pci);
    vp_set_status(vp, VIRTIO_CONFIG_S_ACKNOWLEDGE |
                  VIRTIO_CONFIG_S_DRIVER );
}

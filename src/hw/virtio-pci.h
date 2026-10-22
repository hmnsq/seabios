#यदि_अपरिभाषितम् _VIRTIO_PCI_H
#परिभाषय _VIRTIO_PCI_H

#अन्तर्भाव "x86.h" // inl
#अन्तर्भाव "biosvar.h" // GET_LOWFLAT

/* The bit of the ISR which indicates a device configuration change. */
#परिभाषय VIRTIO_PCI_ISR_CONFIG           0x2

/* Virtio ABI version, this must match exactly */
#परिभाषय VIRTIO_PCI_ABI_संस्करणम्          0

/* --- virtio 0.9.5 (legacy) struct --------------------------------- */

प्रकारनिर्देश संरचना virtio_pci_legacy {
    u32 host_features;
    u32 guest_features;
    u32 queue_pfn;
    u16 queue_अङ्कः;
    u16 queue_sel;
    u16 queue_notify;
    u8  स्थितिः;
    u8  isr;
    u8  device[];
} virtio_pci_legacy;

/* --- virtio 1.0 (modern) structs ---------------------------------- */

/* Common configuration */
#परिभाषय VIRTIO_PCI_CAP_COMMON_CFG       1
/* Notifications */
#परिभाषय VIRTIO_PCI_CAP_NOTIFY_CFG       2
/* ISR access */
#परिभाषय VIRTIO_PCI_CAP_ISR_CFG          3
/* Device specific configuration */
#परिभाषय VIRTIO_PCI_CAP_DEVICE_CFG       4
/* PCI configuration access */
#परिभाषय VIRTIO_PCI_CAP_PCI_CFG          5

/* This is the PCI capability header: */
संरचना virtio_pci_cap {
    u8 cap_vndr;          /* Generic PCI field: PCI_CAP_ID_VNDR */
    u8 cap_अग्र;          /* Generic PCI field: next ptr. */
    u8 cap_len;           /* Generic PCI field: capability length */
    u8 cfg_type;          /* Identifies the structure. */
    u8 bar;               /* Where to find it. */
    u8 padding[3];        /* Pad to full dword. */
    u32 विस्थापनम्_2;           /* Offset within bar. */
    u32 दीर्घता;           /* Length of the structure, in bytes. */
};

संरचना virtio_pci_notify_cap {
    संरचना virtio_pci_cap cap;
    u32 notify_off_multiplier;   /* Multiplier for queue_notify_off. */
};

संरचना virtio_pci_cfg_cap {
    संरचना virtio_pci_cap cap;
    u8 pci_cfg_दत्त[4]; /* Data for BAR access. */
};

प्रकारनिर्देश संरचना virtio_pci_common_cfg {
    /* About the whole device. */
    u32 device_feature_select;   /* read-write */
    u32 device_feature;          /* read-only */
    u32 guest_feature_select;    /* read-write */
    u32 guest_feature;           /* read-write */
    u16 msix_config;             /* read-write */
    u16 अङ्कः_queues;              /* read-only */
    u8 device_स्थितिः;            /* read-write */
    u8 config_generation;        /* read-only */

    /* About a specific virtqueue. */
    u16 queue_select;            /* read-write */
    u16 queue_आकार;              /* read-write, power of 2. */
    u16 queue_msix_सदिशम्;       /* read-write */
    u16 queue_enable;            /* read-write */
    u16 queue_notify_off;        /* read-only */
    u32 queue_desc_lo;           /* read-write */
    u32 queue_desc_hi;           /* read-write */
    u32 queue_avail_lo;          /* read-write */
    u32 queue_avail_hi;          /* read-write */
    u32 queue_प्रयुक्तम्_lo;           /* read-write */
    u32 queue_प्रयुक्तम्_hi;           /* read-write */
} virtio_pci_common_cfg;

प्रकारनिर्देश संरचना virtio_pci_isr {
    u8 isr;
} virtio_pci_isr;

/* --- driver structs ----------------------------------------------- */

#परिभाषय VP_ACCESS_IO       1
#परिभाषय VP_ACCESS_MMIO     2
#परिभाषय VP_ACCESS_PCICFG   3

संरचना vp_cap {
    सङ्घ {
        शून्य *memaddr;
        u32 ioaddr;
        u32 baroff;
    };
    u16 bdf;
    u8 cap;
    u8 cfg;
    u8 bar;
    u8 बहुलकम्;
};

संरचना vp_device {
    संरचना vp_cap common, notify, isr, device, legacy;
    u32 notify_off_multiplier;
    u8 use_modern;
    u8 use_mmio;
};

u64 _vp_read(संरचना vp_cap *cap, u32 विस्थापनम्_2, u8 आकार);
शून्य _vp_write(संरचना vp_cap *cap, u32 विस्थापनम्_2, u8 आकार, u64 var);

#परिभाषय vp_read(_cap, _struct, क्षेत्रम्_2)        \
    _vp_read(_cap, offsetof(_struct, क्षेत्रम्_2), \
             परिमाण(((_struct *)0)->क्षेत्रम्_2))

#परिभाषय vp_write(_cap, _struct, क्षेत्रम्_2, _var)           \
    _vp_write(_cap, offsetof(_struct, क्षेत्रम्_2),          \
             परिमाण(((_struct *)0)->क्षेत्रम्_2), _var)

u64 vp_get_features(संरचना vp_device *vp);
शून्य vp_समुच्चयः_features(संरचना vp_device *vp, u64 features);

स्थिर अन्तःस्थ शून्य vp_get_legacy(संरचना vp_device *vp, अचिह्नित विस्थापनम्_2,
                                 शून्य *buf, अचिह्नित len)
{
    u8 *ptr = buf;
    अचिह्नित i;

    प्रति (i = 0; i < len; i++)
        ptr[i] = vp_read(&vp->legacy, virtio_pci_legacy, device[i]);
}

u8 vp_get_स्थितिः(संरचना vp_device *vp);
शून्य vp_समुच्चयः_स्थितिः(संरचना vp_device *vp, u8 स्थितिः);
u8 vp_get_isr(संरचना vp_device *vp);
शून्य vp_reset(संरचना vp_device *vp);

संरचना pci_device;
संरचना vring_virtqueue;
शून्य vp_init_simple(संरचना vp_device *vp, संरचना pci_device *pci);
शून्य vp_notify(संरचना vp_device *vp, संरचना vring_virtqueue *vq);
पूर्णाङ्क vp_find_vq(संरचना vp_device *vp, पूर्णाङ्क queue_सूचकांक,
               संरचना vring_virtqueue **p_vq);
#यदि_अन्तः /* _VIRTIO_PCI_H_ */

#جے_بنا_تعریف _VIRTIO_PCI_H
#تعریف_کرو _VIRTIO_PCI_H

#ضمن "x86.h" // inl
#ضمن "biosvar.h" // GET_LOWFLAT

/* The bit of the ISR which indicates a device configuration change. */
#تعریف_کرو VIRTIO_PCI_ISR_CONFIG           0x2

/* Virtio ABI version, this must match exactly */
#تعریف_کرو VIRTIO_PCI_ABI_نسخہ          0

/* --- virtio 0.9.5 (legacy) struct --------------------------------- */

تعريفنوع بنية virtio_pci_legacy {
    u32 host_features;
    u32 guest_features;
    u32 قطار_pfn;
    u16 قطار_عدد;
    u16 قطار_sel;
    u16 قطار_notify;
    u8  حالت;
    u8  isr;
    u8  device[];
} virtio_pci_legacy;

/* --- virtio 1.0 (modern) structs ---------------------------------- */

/* Common configuration */
#تعریف_کرو VIRTIO_PCI_CAP_COMMON_CFG       1
/* Notifications */
#تعریف_کرو VIRTIO_PCI_CAP_NOTIFY_CFG       2
/* ISR access */
#تعریف_کرو VIRTIO_PCI_CAP_ISR_CFG          3
/* Device specific configuration */
#تعریف_کرو VIRTIO_PCI_CAP_DEVICE_CFG       4
/* PCI configuration access */
#تعریف_کرو VIRTIO_PCI_CAP_PCI_CFG          5

/* This is the PCI capability header: */
بنية virtio_pci_cap {
    u8 cap_vndr;          /* Generic PCI field: PCI_CAP_ID_VNDR */
    u8 cap_اگلا;          /* Generic PCI field: next ptr. */
    u8 cap_len;           /* Generic PCI field: capability length */
    u8 cfg_type;          /* Identifies the structure. */
    u8 bar;               /* Where to find it. */
    u8 padding[3];        /* Pad to full dword. */
    u32 وتھ_2;           /* Offset within bar. */
    u32 لمائی;           /* Length of the structure, in bytes. */
};

بنية virtio_pci_notify_cap {
    بنية virtio_pci_cap cap;
    u32 notify_off_multiplier;   /* Multiplier for queue_notify_off. */
};

بنية virtio_pci_cfg_cap {
    بنية virtio_pci_cap cap;
    u8 pci_cfg_انکڑے[4]; /* Data for BAR access. */
};

تعريفنوع بنية virtio_pci_common_cfg {
    /* About the whole device. */
    u32 device_feature_select;   /* read-write */
    u32 device_feature;          /* read-only */
    u32 guest_feature_select;    /* read-write */
    u32 guest_feature;           /* read-write */
    u16 msix_config;             /* read-write */
    u16 عدد_queues;              /* read-only */
    u8 device_حالت;            /* read-write */
    u8 config_generation;        /* read-only */

    /* About a specific virtqueue. */
    u16 قطار_select;            /* read-write */
    u16 قطار_ناپ;              /* read-write, power of 2. */
    u16 قطار_msix_سمتیہ;       /* read-write */
    u16 قطار_enable;            /* read-write */
    u16 قطار_notify_off;        /* read-only */
    u32 قطار_desc_lo;           /* read-write */
    u32 قطار_desc_hi;           /* read-write */
    u32 قطار_avail_lo;          /* read-write */
    u32 قطار_avail_hi;          /* read-write */
    u32 قطار_ورتیا_lo;           /* read-write */
    u32 قطار_ورتیا_hi;           /* read-write */
} virtio_pci_common_cfg;

تعريفنوع بنية virtio_pci_isr {
    u8 isr;
} virtio_pci_isr;

/* --- driver structs ----------------------------------------------- */

#تعریف_کرو VP_ACCESS_IO       1
#تعریف_کرو VP_ACCESS_MMIO     2
#تعریف_کرو VP_ACCESS_PCICFG   3

بنية vp_cap {
    اتحاد {
        فراغ *memaddr;
        u32 ioaddr;
        u32 baroff;
    };
    u16 bdf;
    u8 cap;
    u8 cfg;
    u8 bar;
    u8 منوال;
};

بنية vp_device {
    بنية vp_cap common, notify, isr, device, legacy;
    u32 notify_off_multiplier;
    u8 use_modern;
    u8 use_mmio;
};

u64 _vp_read(بنية vp_cap *cap, u32 وتھ_2, u8 ناپ);
فراغ _vp_write(بنية vp_cap *cap, u32 وتھ_2, u8 ناپ, u64 var);

#تعریف_کرو vp_read(_cap, _struct, میدان_2)        \
    _vp_read(_cap, offsetof(_struct, میدان_2), \
             حجم(((_struct *)0)->میدان_2))

#تعریف_کرو vp_write(_cap, _struct, میدان_2, _var)           \
    _vp_write(_cap, offsetof(_struct, میدان_2),          \
             حجم(((_struct *)0)->میدان_2), _var)

u64 vp_get_features(بنية vp_device *vp);
فراغ vp_سیٹ_features(بنية vp_device *vp, u64 features);

ساكن مضمن فراغ vp_get_legacy(بنية vp_device *vp, بنانشانی وتھ_2,
                                 فراغ *buf, بنانشانی len)
{
    u8 *ptr = buf;
    بنانشانی i;

    لكل (i = 0; i < len; i++)
        ptr[i] = vp_read(&vp->legacy, virtio_pci_legacy, device[i]);
}

u8 vp_get_حالت(بنية vp_device *vp);
فراغ vp_سیٹ_حالت(بنية vp_device *vp, u8 حالت);
u8 vp_get_isr(بنية vp_device *vp);
فراغ vp_reset(بنية vp_device *vp);

بنية pci_device;
بنية vring_virtqueue;
فراغ vp_init_simple(بنية vp_device *vp, بنية pci_device *pci);
فراغ vp_notify(بنية vp_device *vp, بنية vring_virtqueue *vq);
صحيح vp_لبھو_vq(بنية vp_device *vp, صحيح قطار_اشاریہ,
               بنية vring_virtqueue **p_vq);
#جے_انت /* _VIRTIO_PCI_H_ */

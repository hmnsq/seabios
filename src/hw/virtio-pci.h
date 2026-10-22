#ifndef _VIRTIO_PCI_H
#define _VIRTIO_PCI_H

#অন্তর্ভাৱ "x86.h" // inl
#অন্তর্ভাৱ "biosvar.h" // GET_LOWFLAT

/* The bit of the ISR which indicates a device configuration change. */
#define VIRTIO_PCI_ISR_CONFIG           0x2

/* Virtio ABI version, this must match exactly */
#define VIRTIO_PCI_ABI_VERSION          0

/* --- virtio 0.9.5 (legacy) struct --------------------------------- */

প্রকারনির্দেশ সংরচনা virtio_pci_legacy {
    u32 host_features;
    u32 guest_features;
    u32 queue_pfn;
    u16 queue_অঙ্ক;
    u16 queue_sel;
    u16 queue_notify;
    u8  status;
    u8  isr;
    u8  device[];
} virtio_pci_legacy;

/* --- virtio 1.0 (modern) structs ---------------------------------- */

/* Common configuration */
#define VIRTIO_PCI_CAP_COMMON_CFG       1
/* Notifications */
#define VIRTIO_PCI_CAP_NOTIFY_CFG       2
/* ISR access */
#define VIRTIO_PCI_CAP_ISR_CFG          3
/* Device specific configuration */
#define VIRTIO_PCI_CAP_DEVICE_CFG       4
/* PCI configuration access */
#define VIRTIO_PCI_CAP_PCI_CFG          5

/* This is the PCI capability header: */
সংরচনা virtio_pci_cap {
    u8 cap_vndr;          /* Generic PCI field: PCI_CAP_ID_VNDR */
    u8 cap_next;          /* Generic PCI field: next ptr. */
    u8 cap_len;           /* Generic PCI field: capability length */
    u8 cfg_type;          /* Identifies the structure. */
    u8 bar;               /* Where to find it. */
    u8 padding[3];        /* Pad to full dword. */
    u32 offset;           /* Offset within bar. */
    u32 length;           /* Length of the structure, in bytes. */
};

সংরচনা virtio_pci_notify_cap {
    সংরচনা virtio_pci_cap cap;
    u32 notify_off_multiplier;   /* Multiplier for queue_notify_off. */
};

সংরচনা virtio_pci_cfg_cap {
    সংরচনা virtio_pci_cap cap;
    u8 pci_cfg_data[4]; /* Data for BAR access. */
};

প্রকারনির্দেশ সংরচনা virtio_pci_common_cfg {
    /* About the whole device. */
    u32 device_feature_select;   /* read-write */
    u32 device_feature;          /* read-only */
    u32 guest_feature_select;    /* read-write */
    u32 guest_feature;           /* read-write */
    u16 msix_config;             /* read-write */
    u16 অঙ্ক_queues;              /* read-only */
    u8 device_status;            /* read-write */
    u8 config_generation;        /* read-only */

    /* About a specific virtqueue. */
    u16 queue_select;            /* read-write */
    u16 queue_size;              /* read-write, power of 2. */
    u16 queue_msix_vector;       /* read-write */
    u16 queue_enable;            /* read-write */
    u16 queue_notify_off;        /* read-only */
    u32 queue_desc_lo;           /* read-write */
    u32 queue_desc_hi;           /* read-write */
    u32 queue_avail_lo;          /* read-write */
    u32 queue_avail_hi;          /* read-write */
    u32 queue_used_lo;           /* read-write */
    u32 queue_used_hi;           /* read-write */
} virtio_pci_common_cfg;

প্রকারনির্দেশ সংরচনা virtio_pci_isr {
    u8 isr;
} virtio_pci_isr;

/* --- driver structs ----------------------------------------------- */

#define VP_ACCESS_IO       1
#define VP_ACCESS_MMIO     2
#define VP_ACCESS_PCICFG   3

সংরচনা vp_cap {
    সঙ্ঘ {
        শূন্য *memaddr;
        u32 ioaddr;
        u32 baroff;
    };
    u16 bdf;
    u8 cap;
    u8 cfg;
    u8 bar;
    u8 mode;
};

সংরচনা vp_device {
    সংরচনা vp_cap common, notify, isr, device, legacy;
    u32 notify_off_multiplier;
    u8 use_modern;
    u8 use_mmio;
};

u64 _vp_read(সংরচনা vp_cap *cap, u32 offset, u8 size);
শূন্য _vp_write(সংরচনা vp_cap *cap, u32 offset, u8 size, u64 var);

#define vp_read(_cap, _struct, _field)        \
    _vp_read(_cap, offsetof(_struct, _field), \
             পরিমাণ(((_struct *)0)->_field))

#define vp_write(_cap, _struct, _field, _var)           \
    _vp_write(_cap, offsetof(_struct, _field),          \
             পরিমাণ(((_struct *)0)->_field), _var)

u64 vp_get_features(সংরচনা vp_device *vp);
শূন্য vp_set_features(সংরচনা vp_device *vp, u64 features);

স্থির অন্তঃস্থ শূন্য vp_get_legacy(সংরচনা vp_device *vp, unsigned offset,
                                 শূন্য *buf, unsigned len)
{
    u8 *ptr = buf;
    unsigned i;

    বাবে (i = 0; i < len; i++)
        ptr[i] = vp_read(&vp->legacy, virtio_pci_legacy, device[i]);
}

u8 vp_get_status(সংরচনা vp_device *vp);
শূন্য vp_set_status(সংরচনা vp_device *vp, u8 status);
u8 vp_get_isr(সংরচনা vp_device *vp);
শূন্য vp_reset(সংরচনা vp_device *vp);

সংরচনা pci_device;
সংরচনা vring_virtqueue;
শূন্য vp_init_simple(সংরচনা vp_device *vp, সংরচনা pci_device *pci);
শূন্য vp_notify(সংরচনা vp_device *vp, সংরচনা vring_virtqueue *vq);
পূর্ণাঙ্ক vp_find_vq(সংরচনা vp_device *vp, পূর্ণাঙ্ক queue_index,
               সংরচনা vring_virtqueue **p_vq);
#endif /* _VIRTIO_PCI_H_ */

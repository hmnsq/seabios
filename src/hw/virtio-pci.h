#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _VIRTIO_PCI_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _VIRTIO_PCI_H

#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // inl
#ਅਨ੍ਤਰ੍ਭਾਵ "biosvar.h" // GET_LOWFLAT

/* The bit of the ISR which indicates a device configuration change. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_PCI_ISR_CONFIG           0x2

/* Virtio ABI version, this must match exactly */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_PCI_ABI_ਸੰਸਕਰਣ          0

/* --- virtio 0.9.5 (legacy) struct --------------------------------- */

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ virtio_pci_legacy {
    u32 host_features;
    u32 guest_features;
    u32 ਕਤਾਰ_pfn;
    u16 ਕਤਾਰ_ਅੰਕ;
    u16 ਕਤਾਰ_sel;
    u16 ਕਤਾਰ_notify;
    u8  ਸਥਿਤੀ;
    u8  isr;
    u8  device[];
} virtio_pci_legacy;

/* --- virtio 1.0 (modern) structs ---------------------------------- */

/* Common configuration */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_PCI_CAP_COMMON_CFG       1
/* Notifications */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_PCI_CAP_NOTIFY_CFG       2
/* ISR access */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_PCI_CAP_ISR_CFG          3
/* Device specific configuration */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_PCI_CAP_DEVICE_CFG       4
/* PCI configuration access */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_PCI_CAP_PCI_CFG          5

/* This is the PCI capability header: */
ਬਣਤਰ virtio_pci_cap {
    u8 cap_vndr;          /* Generic PCI field: PCI_CAP_ID_VNDR */
    u8 cap_ਅਗਲਾ;          /* Generic PCI field: next ptr. */
    u8 cap_len;           /* Generic PCI field: capability length */
    u8 cfg_type;          /* Identifies the structure. */
    u8 bar;               /* Where to find it. */
    u8 padding[3];        /* Pad to full dword. */
    u32 ਵਿੱਥ_2;           /* Offset within bar. */
    u32 ਲੰਮਾਈ;           /* Length of the structure, in bytes. */
};

ਬਣਤਰ virtio_pci_notify_cap {
    ਬਣਤਰ virtio_pci_cap cap;
    u32 notify_off_multiplier;   /* Multiplier for queue_notify_off. */
};

ਬਣਤਰ virtio_pci_cfg_cap {
    ਬਣਤਰ virtio_pci_cap cap;
    u8 pci_cfg_ਅੰਕੜੇ[4]; /* Data for BAR access. */
};

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ virtio_pci_common_cfg {
    /* About the whole device. */
    u32 device_feature_select;   /* read-write */
    u32 device_feature;          /* read-only */
    u32 guest_feature_select;    /* read-write */
    u32 guest_feature;           /* read-write */
    u16 msix_config;             /* read-write */
    u16 ਅੰਕ_queues;              /* read-only */
    u8 device_ਸਥਿਤੀ;            /* read-write */
    u8 config_generation;        /* read-only */

    /* About a specific virtqueue. */
    u16 ਕਤਾਰ_select;            /* read-write */
    u16 ਕਤਾਰ_ਮਾਪ;              /* read-write, power of 2. */
    u16 ਕਤਾਰ_msix_ਸਦਿਸ਼;       /* read-write */
    u16 ਕਤਾਰ_enable;            /* read-write */
    u16 ਕਤਾਰ_notify_off;        /* read-only */
    u32 ਕਤਾਰ_desc_lo;           /* read-write */
    u32 ਕਤਾਰ_desc_hi;           /* read-write */
    u32 ਕਤਾਰ_avail_lo;          /* read-write */
    u32 ਕਤਾਰ_avail_hi;          /* read-write */
    u32 ਕਤਾਰ_ਵਰਤਿਆ_lo;           /* read-write */
    u32 ਕਤਾਰ_ਵਰਤਿਆ_hi;           /* read-write */
} virtio_pci_common_cfg;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ virtio_pci_isr {
    u8 isr;
} virtio_pci_isr;

/* --- driver structs ----------------------------------------------- */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VP_ACCESS_IO       1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VP_ACCESS_MMIO     2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VP_ACCESS_PCICFG   3

ਬਣਤਰ vp_cap {
    ਸੰਘ {
        ਖਾਲੀ *memaddr;
        u32 ioaddr;
        u32 baroff;
    };
    u16 bdf;
    u8 cap;
    u8 cfg;
    u8 bar;
    u8 ਬਹੁਲਕ;
};

ਬਣਤਰ vp_device {
    ਬਣਤਰ vp_cap common, notify, isr, device, legacy;
    u32 notify_off_multiplier;
    u8 use_modern;
    u8 use_mmio;
};

u64 _vp_read(ਬਣਤਰ vp_cap *cap, u32 ਵਿੱਥ_2, u8 ਮਾਪ);
ਖਾਲੀ _vp_write(ਬਣਤਰ vp_cap *cap, u32 ਵਿੱਥ_2, u8 ਮਾਪ, u64 var);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ vp_read(_cap, _struct, ਖੇਤਰ_2)        \
    _vp_read(_cap, offsetof(_struct, ਖੇਤਰ_2), \
             ਆਕਾਰ(((_struct *)0)->ਖੇਤਰ_2))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ vp_write(_cap, _struct, ਖੇਤਰ_2, _var)           \
    _vp_write(_cap, offsetof(_struct, ਖੇਤਰ_2),          \
             ਆਕਾਰ(((_struct *)0)->ਖੇਤਰ_2), _var)

u64 vp_get_features(ਬਣਤਰ vp_device *vp);
ਖਾਲੀ vp_ਸੈੱਟ_features(ਬਣਤਰ vp_device *vp, u64 features);

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ vp_get_legacy(ਬਣਤਰ vp_device *vp, ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ_2,
                                 ਖਾਲੀ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ len)
{
    u8 *ptr = buf;
    ਬਿਨਾਨਿਸ਼ਾਨੀ i;

    ਲਈ (i = 0; i < len; i++)
        ptr[i] = vp_read(&vp->legacy, virtio_pci_legacy, device[i]);
}

u8 vp_get_ਸਥਿਤੀ(ਬਣਤਰ vp_device *vp);
ਖਾਲੀ vp_ਸੈੱਟ_ਸਥਿਤੀ(ਬਣਤਰ vp_device *vp, u8 ਸਥਿਤੀ);
u8 vp_get_isr(ਬਣਤਰ vp_device *vp);
ਖਾਲੀ vp_reset(ਬਣਤਰ vp_device *vp);

ਬਣਤਰ pci_device;
ਬਣਤਰ vring_virtqueue;
ਖਾਲੀ vp_init_simple(ਬਣਤਰ vp_device *vp, ਬਣਤਰ pci_device *pci);
ਖਾਲੀ vp_notify(ਬਣਤਰ vp_device *vp, ਬਣਤਰ vring_virtqueue *vq);
ਪੂਰਨ vp_ਲੱਭੋ_vq(ਬਣਤਰ vp_device *vp, ਪੂਰਨ ਕਤਾਰ_ਸੂਚਕਾਂਕ,
               ਬਣਤਰ vring_virtqueue **p_vq);
#ਜੇ_ਅੰਤ /* _VIRTIO_PCI_H_ */

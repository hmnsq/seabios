#ifndef _VIRTIO_PCI_H
#define _VIRTIO_PCI_H

#અન્તર્ભાવ "x86.h" // inl
#અન્તર્ભાવ "biosvar.h" // GET_LOWFLAT

/* The bit of the ISR which indicates a device configuration change. */
#define VIRTIO_PCI_ISR_CONFIG           0x2

/* Virtio ABI version, this must match exactly */
#define VIRTIO_PCI_ABI_VERSION          0

/* --- virtio 0.9.5 (legacy) struct --------------------------------- */

પ્રકારવ્યાખ્યા સંરચના virtio_pci_legacy {
    u32 host_features;
    u32 guest_features;
    u32 queue_pfn;
    u16 queue_અંક;
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
સંરચના virtio_pci_cap {
    u8 cap_vndr;          /* Generic PCI field: PCI_CAP_ID_VNDR */
    u8 cap_next;          /* Generic PCI field: next ptr. */
    u8 cap_len;           /* Generic PCI field: capability length */
    u8 cfg_type;          /* Identifies the structure. */
    u8 bar;               /* Where to find it. */
    u8 padding[3];        /* Pad to full dword. */
    u32 offset;           /* Offset within bar. */
    u32 length;           /* Length of the structure, in bytes. */
};

સંરચના virtio_pci_notify_cap {
    સંરચના virtio_pci_cap cap;
    u32 notify_off_multiplier;   /* Multiplier for queue_notify_off. */
};

સંરચના virtio_pci_cfg_cap {
    સંરચના virtio_pci_cap cap;
    u8 pci_cfg_data[4]; /* Data for BAR access. */
};

પ્રકારવ્યાખ્યા સંરચના virtio_pci_common_cfg {
    /* About the whole device. */
    u32 device_feature_select;   /* read-write */
    u32 device_feature;          /* read-only */
    u32 guest_feature_select;    /* read-write */
    u32 guest_feature;           /* read-write */
    u16 msix_config;             /* read-write */
    u16 અંક_queues;              /* read-only */
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

પ્રકારવ્યાખ્યા સંરચના virtio_pci_isr {
    u8 isr;
} virtio_pci_isr;

/* --- driver structs ----------------------------------------------- */

#define VP_ACCESS_IO       1
#define VP_ACCESS_MMIO     2
#define VP_ACCESS_PCICFG   3

સંરચના vp_cap {
    સંઘ {
        શૂન્ય *memaddr;
        u32 ioaddr;
        u32 baroff;
    };
    u16 bdf;
    u8 cap;
    u8 cfg;
    u8 bar;
    u8 mode;
};

સંરચના vp_device {
    સંરચના vp_cap common, notify, isr, device, legacy;
    u32 notify_off_multiplier;
    u8 use_modern;
    u8 use_mmio;
};

u64 _vp_read(સંરચના vp_cap *cap, u32 offset, u8 size);
શૂન્ય _vp_write(સંરચના vp_cap *cap, u32 offset, u8 size, u64 var);

#define vp_read(_cap, _struct, _field)        \
    _vp_read(_cap, offsetof(_struct, _field), \
             કદ(((_struct *)0)->_field))

#define vp_write(_cap, _struct, _field, _var)           \
    _vp_write(_cap, offsetof(_struct, _field),          \
             કદ(((_struct *)0)->_field), _var)

u64 vp_get_features(સંરચના vp_device *vp);
શૂન્ય vp_set_features(સંરચના vp_device *vp, u64 features);

સ્થિર અંતઃરેખ શૂન્ય vp_get_legacy(સંરચના vp_device *vp, unsigned offset,
                                 શૂન્ય *buf, unsigned len)
{
    u8 *ptr = buf;
    unsigned i;

    માટે (i = 0; i < len; i++)
        ptr[i] = vp_read(&vp->legacy, virtio_pci_legacy, device[i]);
}

u8 vp_get_status(સંરચના vp_device *vp);
શૂન્ય vp_set_status(સંરચના vp_device *vp, u8 status);
u8 vp_get_isr(સંરચના vp_device *vp);
શૂન્ય vp_reset(સંરચના vp_device *vp);

સંરચના pci_device;
સંરચના vring_virtqueue;
શૂન્ય vp_init_simple(સંરચના vp_device *vp, સંરચના pci_device *pci);
શૂન્ય vp_notify(સંરચના vp_device *vp, સંરચના vring_virtqueue *vq);
પૂર્ણાંક vp_find_vq(સંરચના vp_device *vp, પૂર્ણાંક queue_index,
               સંરચના vring_virtqueue **p_vq);
#endif /* _VIRTIO_PCI_H_ */

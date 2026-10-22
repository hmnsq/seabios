#अगर_अघोषित _VIRTIO_PCI_H
#घोषणा _VIRTIO_PCI_H

#समावेश "x86.h" // inl
#समावेश "biosvar.h" // GET_LOWFLAT

/* The bit of the ISR which indicates a device configuration change. */
#घोषणा VIRTIO_PCI_ISR_CONFIG           0x2

/* Virtio ABI version, this must match exactly */
#घोषणा VIRTIO_PCI_ABI_VERSION          0

/* --- virtio 0.9.5 (legacy) struct --------------------------------- */

प्रकार काष्ठा virtio_pci_legacy {
    u32 host_features;
    u32 guest_features;
    u32 queue_pfn;
    u16 queue_अंक;
    u16 queue_sel;
    u16 queue_notify;
    u8  status;
    u8  isr;
    u8  device[];
} virtio_pci_legacy;

/* --- virtio 1.0 (modern) structs ---------------------------------- */

/* Common configuration */
#घोषणा VIRTIO_PCI_CAP_COMMON_CFG       1
/* Notifications */
#घोषणा VIRTIO_PCI_CAP_NOTIFY_CFG       2
/* ISR access */
#घोषणा VIRTIO_PCI_CAP_ISR_CFG          3
/* Device specific configuration */
#घोषणा VIRTIO_PCI_CAP_DEVICE_CFG       4
/* PCI configuration access */
#घोषणा VIRTIO_PCI_CAP_PCI_CFG          5

/* This is the PCI capability header: */
काष्ठा virtio_pci_cap {
    u8 cap_vndr;          /* Generic PCI field: PCI_CAP_ID_VNDR */
    u8 cap_अग्र;          /* Generic PCI field: next ptr. */
    u8 cap_len;           /* Generic PCI field: capability length */
    u8 cfg_type;          /* Identifies the structure. */
    u8 bar;               /* Where to find it. */
    u8 padding[3];        /* Pad to full dword. */
    u32 offset;           /* Offset within bar. */
    u32 लंबाई;           /* Length of the structure, in bytes. */
};

काष्ठा virtio_pci_notify_cap {
    काष्ठा virtio_pci_cap cap;
    u32 notify_off_multiplier;   /* Multiplier for queue_notify_off. */
};

काष्ठा virtio_pci_cfg_cap {
    काष्ठा virtio_pci_cap cap;
    u8 pci_cfg_तथ्य[4]; /* Data for BAR access. */
};

प्रकार काष्ठा virtio_pci_common_cfg {
    /* About the whole device. */
    u32 device_feature_select;   /* read-write */
    u32 device_feature;          /* read-only */
    u32 guest_feature_select;    /* read-write */
    u32 guest_feature;           /* read-write */
    u16 msix_config;             /* read-write */
    u16 अंक_queues;              /* read-only */
    u8 device_status;            /* read-write */
    u8 config_generation;        /* read-only */

    /* About a specific virtqueue. */
    u16 queue_select;            /* read-write */
    u16 queue_आकार;              /* read-write, power of 2. */
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

प्रकार काष्ठा virtio_pci_isr {
    u8 isr;
} virtio_pci_isr;

/* --- driver structs ----------------------------------------------- */

#घोषणा VP_ACCESS_IO       1
#घोषणा VP_ACCESS_MMIO     2
#घोषणा VP_ACCESS_PCICFG   3

काष्ठा vp_cap {
    जोड़ {
        व्योम *memaddr;
        u32 ioaddr;
        u32 baroff;
    };
    u16 bdf;
    u8 cap;
    u8 cfg;
    u8 bar;
    u8 mode;
};

काष्ठा vp_device {
    काष्ठा vp_cap common, notify, isr, device, legacy;
    u32 notify_off_multiplier;
    u8 use_modern;
    u8 use_mmio;
};

u64 _vp_read(काष्ठा vp_cap *cap, u32 offset, u8 आकार);
व्योम _vp_write(काष्ठा vp_cap *cap, u32 offset, u8 आकार, u64 var);

#घोषणा vp_read(_cap, _struct, _field)        \
    _vp_read(_cap, दुरत्व(_struct, _field), \
             माप(((_struct *)0)->_field))

#घोषणा vp_write(_cap, _struct, _field, _var)           \
    _vp_write(_cap, दुरत्व(_struct, _field),          \
             माप(((_struct *)0)->_field), _var)

u64 vp_get_features(काष्ठा vp_device *vp);
व्योम vp_set_features(काष्ठा vp_device *vp, u64 features);

अटल अंतरभूत व्योम vp_get_legacy(काष्ठा vp_device *vp, अचिन्हित offset,
                                 व्योम *buf, अचिन्हित len)
{
    u8 *ptr = buf;
    अचिन्हित i;

    क्रम (i = 0; i < len; i++)
        ptr[i] = vp_read(&vp->legacy, virtio_pci_legacy, device[i]);
}

u8 vp_get_status(काष्ठा vp_device *vp);
व्योम vp_set_status(काष्ठा vp_device *vp, u8 status);
u8 vp_get_isr(काष्ठा vp_device *vp);
व्योम vp_reset(काष्ठा vp_device *vp);

काष्ठा pci_device;
काष्ठा vring_virtqueue;
व्योम vp_init_simple(काष्ठा vp_device *vp, काष्ठा pci_device *pci);
व्योम vp_notify(काष्ठा vp_device *vp, काष्ठा vring_virtqueue *vq);
पूर्णांक vp_find_vq(काष्ठा vp_device *vp, पूर्णांक queue_सूचकांक,
               काष्ठा vring_virtqueue **p_vq);
#पूर्ण_अगर /* _VIRTIO_PCI_H_ */

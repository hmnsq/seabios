#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _VIRTIO_PCI_H
#ವ್ಯಾಖ್ಯಾನಿಸು _VIRTIO_PCI_H

#ಅನ್ತರ್ಭಾವ "x86.h" // inl
#ಅನ್ತರ್ಭಾವ "biosvar.h" // GET_LOWFLAT

/* The bit of the ISR which indicates a device configuration change. */
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_PCI_ISR_CONFIG           0x2

/* Virtio ABI version, this must match exactly */
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_PCI_ABI_ಆವೃತ್ತಿ          0

/* --- virtio 0.9.5 (legacy) struct --------------------------------- */

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ virtio_pci_legacy {
    u32 host_features;
    u32 guest_features;
    u32 ಸರತಿ_pfn;
    u16 ಸರತಿ_ಅಂಕೆ;
    u16 ಸರತಿ_sel;
    u16 ಸರತಿ_notify;
    u8  ಸ್ಥಿತಿ_2;
    u8  isr;
    u8  device[];
} virtio_pci_legacy;

/* --- virtio 1.0 (modern) structs ---------------------------------- */

/* Common configuration */
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_PCI_CAP_COMMON_CFG       1
/* Notifications */
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_PCI_CAP_NOTIFY_CFG       2
/* ISR access */
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_PCI_CAP_ISR_CFG          3
/* Device specific configuration */
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_PCI_CAP_DEVICE_CFG       4
/* PCI configuration access */
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_PCI_CAP_PCI_CFG          5

/* This is the PCI capability header: */
ರಚನೆ virtio_pci_cap {
    u8 cap_vndr;          /* Generic PCI field: PCI_CAP_ID_VNDR */
    u8 cap_ಮುಂದಿನ;          /* Generic PCI field: next ptr. */
    u8 cap_len;           /* Generic PCI field: capability length */
    u8 cfg_type;          /* Identifies the structure. */
    u8 bar;               /* Where to find it. */
    u8 padding[3];        /* Pad to full dword. */
    u32 ಸ್ಥಾನಾಂತರ_2;           /* Offset within bar. */
    u32 ಉದ್ದಳತೆ;           /* Length of the structure, in bytes. */
};

ರಚನೆ virtio_pci_notify_cap {
    ರಚನೆ virtio_pci_cap cap;
    u32 notify_off_multiplier;   /* Multiplier for queue_notify_off. */
};

ರಚನೆ virtio_pci_cfg_cap {
    ರಚನೆ virtio_pci_cap cap;
    u8 pci_cfg_ದತ್ತಾಂಶ[4]; /* Data for BAR access. */
};

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ virtio_pci_common_cfg {
    /* About the whole device. */
    u32 device_feature_select;   /* read-write */
    u32 device_feature;          /* read-only */
    u32 guest_feature_select;    /* read-write */
    u32 guest_feature;           /* read-write */
    u16 msix_config;             /* read-write */
    u16 ಅಂಕೆ_queues;              /* read-only */
    u8 device_ಸ್ಥಿತಿ;            /* read-write */
    u8 config_generation;        /* read-only */

    /* About a specific virtqueue. */
    u16 ಸರತಿ_select;            /* read-write */
    u16 ಸರತಿ_ಗಾತ್ರ;              /* read-write, power of 2. */
    u16 ಸರತಿ_msix_ಸದಿಶ;       /* read-write */
    u16 ಸರತಿ_enable;            /* read-write */
    u16 ಸರತಿ_notify_off;        /* read-only */
    u32 ಸರತಿ_desc_lo;           /* read-write */
    u32 ಸರತಿ_desc_hi;           /* read-write */
    u32 ಸರತಿ_avail_lo;          /* read-write */
    u32 ಸರತಿ_avail_hi;          /* read-write */
    u32 ಸರತಿ_ಬಳಸಿದ_lo;           /* read-write */
    u32 ಸರತಿ_ಬಳಸಿದ_hi;           /* read-write */
} virtio_pci_common_cfg;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ virtio_pci_isr {
    u8 isr;
} virtio_pci_isr;

/* --- driver structs ----------------------------------------------- */

#ವ್ಯಾಖ್ಯಾನಿಸು VP_ACCESS_IO       1
#ವ್ಯಾಖ್ಯಾನಿಸು VP_ACCESS_MMIO     2
#ವ್ಯಾಖ್ಯಾನಿಸು VP_ACCESS_PCICFG   3

ರಚನೆ vp_cap {
    ಒಕ್ಕೂಟ {
        ಶೂನ್ಯ *memaddr;
        u32 ioaddr;
        u32 baroff;
    };
    u16 bdf;
    u8 cap;
    u8 cfg;
    u8 bar;
    u8 ಬಹುಲಕ;
};

ರಚನೆ vp_device {
    ರಚನೆ vp_cap common, notify, isr, device, legacy;
    u32 notify_off_multiplier;
    u8 use_modern;
    u8 use_mmio;
};

u64 _vp_read(ರಚನೆ vp_cap *cap, u32 ಸ್ಥಾನಾಂತರ_2, u8 ಗಾತ್ರ_2);
ಶೂನ್ಯ _vp_write(ರಚನೆ vp_cap *cap, u32 ಸ್ಥಾನಾಂತರ_2, u8 ಗಾತ್ರ_2, u64 var);

#ವ್ಯಾಖ್ಯಾನಿಸು vp_read(_cap, _struct, ಕ್ಷೇತ್ರ_2)        \
    _vp_read(_cap, offsetof(_struct, ಕ್ಷೇತ್ರ_2), \
             ಗಾತ್ರ(((_struct *)0)->ಕ್ಷೇತ್ರ_2))

#ವ್ಯಾಖ್ಯಾನಿಸು vp_write(_cap, _struct, ಕ್ಷೇತ್ರ_2, _var)           \
    _vp_write(_cap, offsetof(_struct, ಕ್ಷೇತ್ರ_2),          \
             ಗಾತ್ರ(((_struct *)0)->ಕ್ಷೇತ್ರ_2), _var)

u64 vp_get_features(ರಚನೆ vp_device *vp);
ಶೂನ್ಯ vp_ಗಣ_features(ರಚನೆ vp_device *vp, u64 features);

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ vp_get_legacy(ರಚನೆ vp_device *vp, ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ_2,
                                 ಶೂನ್ಯ *buf, ಚಿಹ್ನೆರಹಿತ len)
{
    u8 *ptr = buf;
    ಚಿಹ್ನೆರಹಿತ i;

    ಗಾಗಿ (i = 0; i < len; i++)
        ptr[i] = vp_read(&vp->legacy, virtio_pci_legacy, device[i]);
}

u8 vp_get_ಸ್ಥಿತಿ(ರಚನೆ vp_device *vp);
ಶೂನ್ಯ vp_ಗಣ_ಸ್ಥಿತಿ(ರಚನೆ vp_device *vp, u8 ಸ್ಥಿತಿ_2);
u8 vp_get_isr(ರಚನೆ vp_device *vp);
ಶೂನ್ಯ vp_reset(ರಚನೆ vp_device *vp);

ರಚನೆ pci_device;
ರಚನೆ vring_virtqueue;
ಶೂನ್ಯ vp_init_simple(ರಚನೆ vp_device *vp, ರಚನೆ pci_device *pci);
ಶೂನ್ಯ vp_notify(ರಚನೆ vp_device *vp, ರಚನೆ vring_virtqueue *vq);
ಪೂರ್ಣಾಂಕ vp_ಕಂಡುಹಿಡಿ_vq(ರಚನೆ vp_device *vp, ಪೂರ್ಣಾಂಕ ಸರತಿ_ಸೂಚ್ಯಂಕ,
               ರಚನೆ vring_virtqueue **p_vq);
#ಆದರೆ_ಅಂತ್ಯ /* _VIRTIO_PCI_H_ */

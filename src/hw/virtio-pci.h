#నిర్వచితం_కాకపోతే _VIRTIO_PCI_H
#నిర్వచించు _VIRTIO_PCI_H

#అన్తర్భావ "x86.h" // inl
#అన్తర్భావ "biosvar.h" // GET_LOWFLAT

/* The bit of the ISR which indicates a device configuration change. */
#నిర్వచించు VIRTIO_PCI_ISR_CONFIG           0x2

/* Virtio ABI version, this must match exactly */
#నిర్వచించు VIRTIO_PCI_ABI_సంస్కరణ          0

/* --- virtio 0.9.5 (legacy) struct --------------------------------- */

రకనిర్వచనం నిర్మాణం virtio_pci_legacy {
    u32 host_features;
    u32 guest_features;
    u32 వరుస_pfn;
    u16 వరుస_అంకె;
    u16 వరుస_sel;
    u16 వరుస_notify;
    u8  స్థితి_2;
    u8  isr;
    u8  device[];
} virtio_pci_legacy;

/* --- virtio 1.0 (modern) structs ---------------------------------- */

/* Common configuration */
#నిర్వచించు VIRTIO_PCI_CAP_COMMON_CFG       1
/* Notifications */
#నిర్వచించు VIRTIO_PCI_CAP_NOTIFY_CFG       2
/* ISR access */
#నిర్వచించు VIRTIO_PCI_CAP_ISR_CFG          3
/* Device specific configuration */
#నిర్వచించు VIRTIO_PCI_CAP_DEVICE_CFG       4
/* PCI configuration access */
#నిర్వచించు VIRTIO_PCI_CAP_PCI_CFG          5

/* This is the PCI capability header: */
నిర్మాణం virtio_pci_cap {
    u8 cap_vndr;          /* Generic PCI field: PCI_CAP_ID_VNDR */
    u8 cap_తదుపరి;          /* Generic PCI field: next ptr. */
    u8 cap_len;           /* Generic PCI field: capability length */
    u8 cfg_type;          /* Identifies the structure. */
    u8 bar;               /* Where to find it. */
    u8 padding[3];        /* Pad to full dword. */
    u32 స్థానభ్రంశం_2;           /* Offset within bar. */
    u32 నిడివి;           /* Length of the structure, in bytes. */
};

నిర్మాణం virtio_pci_notify_cap {
    నిర్మాణం virtio_pci_cap cap;
    u32 notify_off_multiplier;   /* Multiplier for queue_notify_off. */
};

నిర్మాణం virtio_pci_cfg_cap {
    నిర్మాణం virtio_pci_cap cap;
    u8 pci_cfg_దత్తాంశం[4]; /* Data for BAR access. */
};

రకనిర్వచనం నిర్మాణం virtio_pci_common_cfg {
    /* About the whole device. */
    u32 device_feature_select;   /* read-write */
    u32 device_feature;          /* read-only */
    u32 guest_feature_select;    /* read-write */
    u32 guest_feature;           /* read-write */
    u16 msix_config;             /* read-write */
    u16 అంకె_queues;              /* read-only */
    u8 device_స్థితి;            /* read-write */
    u8 config_generation;        /* read-only */

    /* About a specific virtqueue. */
    u16 వరుస_select;            /* read-write */
    u16 వరుస_కొలత;              /* read-write, power of 2. */
    u16 వరుస_msix_సదిశ;       /* read-write */
    u16 వరుస_enable;            /* read-write */
    u16 వరుస_notify_off;        /* read-only */
    u32 వరుస_desc_lo;           /* read-write */
    u32 వరుస_desc_hi;           /* read-write */
    u32 వరుస_avail_lo;          /* read-write */
    u32 వరుస_avail_hi;          /* read-write */
    u32 వరుస_వాడిన_lo;           /* read-write */
    u32 వరుస_వాడిన_hi;           /* read-write */
} virtio_pci_common_cfg;

రకనిర్వచనం నిర్మాణం virtio_pci_isr {
    u8 isr;
} virtio_pci_isr;

/* --- driver structs ----------------------------------------------- */

#నిర్వచించు VP_ACCESS_IO       1
#నిర్వచించు VP_ACCESS_MMIO     2
#నిర్వచించు VP_ACCESS_PCICFG   3

నిర్మాణం vp_cap {
    సంఘం {
        శూన్య *memaddr;
        u32 ioaddr;
        u32 baroff;
    };
    u16 bdf;
    u8 cap;
    u8 cfg;
    u8 bar;
    u8 బహుళకం;
};

నిర్మాణం vp_device {
    నిర్మాణం vp_cap common, notify, isr, device, legacy;
    u32 notify_off_multiplier;
    u8 use_modern;
    u8 use_mmio;
};

u64 _vp_read(నిర్మాణం vp_cap *cap, u32 స్థానభ్రంశం_2, u8 కొలత);
శూన్య _vp_write(నిర్మాణం vp_cap *cap, u32 స్థానభ్రంశం_2, u8 కొలత, u64 var);

#నిర్వచించు vp_read(_cap, _struct, క్షేత్రం_2)        \
    _vp_read(_cap, offsetof(_struct, క్షేత్రం_2), \
             పరిమాణం(((_struct *)0)->క్షేత్రం_2))

#నిర్వచించు vp_write(_cap, _struct, క్షేత్రం_2, _var)           \
    _vp_write(_cap, offsetof(_struct, క్షేత్రం_2),          \
             పరిమాణం(((_struct *)0)->క్షేత్రం_2), _var)

u64 vp_get_features(నిర్మాణం vp_device *vp);
శూన్య vp_సమితి_features(నిర్మాణం vp_device *vp, u64 features);

స్థిర అంతఃవరుస శూన్య vp_get_legacy(నిర్మాణం vp_device *vp, గుర్తులేని స్థానభ్రంశం_2,
                                 శూన్య *buf, గుర్తులేని len)
{
    u8 *ptr = buf;
    గుర్తులేని i;

    కొరకు (i = 0; i < len; i++)
        ptr[i] = vp_read(&vp->legacy, virtio_pci_legacy, device[i]);
}

u8 vp_get_స్థితి(నిర్మాణం vp_device *vp);
శూన్య vp_సమితి_స్థితి(నిర్మాణం vp_device *vp, u8 స్థితి_2);
u8 vp_get_isr(నిర్మాణం vp_device *vp);
శూన్య vp_reset(నిర్మాణం vp_device *vp);

నిర్మాణం pci_device;
నిర్మాణం vring_virtqueue;
శూన్య vp_init_simple(నిర్మాణం vp_device *vp, నిర్మాణం pci_device *pci);
శూన్య vp_notify(నిర్మాణం vp_device *vp, నిర్మాణం vring_virtqueue *vq);
పూర్ణాంకం vp_కనుగొను_vq(నిర్మాణం vp_device *vp, పూర్ణాంకం వరుస_సూచీ,
               నిర్మాణం vring_virtqueue **p_vq);
#అయితే_ముగింపు /* _VIRTIO_PCI_H_ */

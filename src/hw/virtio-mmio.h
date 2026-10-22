#నిర్వచితం_కాకపోతే _VIRTIO_MMIO_H
#నిర్వచించు _VIRTIO_MMIO_H

నిర్మాణం vp_device;

రకనిర్వచనం నిర్మాణం virtio_mmio_cfg {
    u32 magic;
    u32 సంస్కరణ_2;
    u32 device_id;
    u32 vendor_id;

    u32 device_feature;
    u32 device_feature_select;
    u32 res_18;
    u32 res_1c;

    u32 guest_feature;
    u32 guest_feature_select;
    u32 legacy_guest_పుట_కొలత;
    u32 res_2c;

    u32 వరుస_select;
    u32 వరుస_అంకె_max;
    u32 వరుస_అంకె;
    u32 legacy_వరుస_align;

    u32 legacy_వరుస_pfn;
    u32 వరుస_ready;
    u32 res_48;
    u32 res_4c;

    u32 వరుస_notify;
    u32 res_54;
    u32 res_58;
    u32 res_5c;

    u32 irq_స్థితి;
    u32 irq_ack;
    u32 res_68;
    u32 res_6c;

    u32 device_స్థితి;
    u32 res_74;
    u32 res_78;
    u32 res_7c;

    u32 వరుస_desc_lo;
    u32 వరుస_desc_hi;
    u32 res_88;
    u32 res_8c;

    u32 వరుస_driver_lo;
    u32 వరుస_driver_hi;
    u32 res_98;
    u32 res_9c;

    u32 వరుస_device_lo;
    u32 వరుస_device_hi;
    u32 res_a8;
    u32 shm_sel;

    u32 shmem_len_lo;
    u32 shmem_len_hi;
    u32 shmem_పునాది_lo;
    u32 shmem_పునాది_hi;

    u32 res_c0_f7[14];

    u32 res_f8;
    u32 config_generation;
} virtio_mmio_cfg;

శూన్య virtio_mmio_అమరిక_acpi(శూన్య);
శూన్య virtio_mmio_అమరిక_one(u64 mmio);
శూన్య vp_init_mmio(నిర్మాణం vp_device *vp, శూన్య *mmio);

#అయితే_ముగింపు /* _VIRTIO_MMIO_H */

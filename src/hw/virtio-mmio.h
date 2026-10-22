#جيڪڏهن_وصف_نه_ٿيل _VIRTIO_MMIO_H
#وصف_ڪريو _VIRTIO_MMIO_H

بنية vp_device;

تعريفنوع بنية virtio_mmio_cfg {
    u32 magic;
    u32 نسخو_2;
    u32 device_id;
    u32 vendor_id;

    u32 device_feature;
    u32 device_feature_select;
    u32 res_18;
    u32 res_1c;

    u32 guest_feature;
    u32 guest_feature_select;
    u32 legacy_guest_صفحو_سائيز;
    u32 res_2c;

    u32 قطار_select;
    u32 قطار_انگ_max;
    u32 قطار_انگ;
    u32 legacy_قطار_align;

    u32 legacy_قطار_pfn;
    u32 قطار_ready;
    u32 res_48;
    u32 res_4c;

    u32 قطار_notify;
    u32 res_54;
    u32 res_58;
    u32 res_5c;

    u32 irq_حالت;
    u32 irq_ack;
    u32 res_68;
    u32 res_6c;

    u32 device_حالت;
    u32 res_74;
    u32 res_78;
    u32 res_7c;

    u32 قطار_desc_lo;
    u32 قطار_desc_hi;
    u32 res_88;
    u32 res_8c;

    u32 قطار_driver_lo;
    u32 قطار_driver_hi;
    u32 res_98;
    u32 res_9c;

    u32 قطار_device_lo;
    u32 قطار_device_hi;
    u32 res_a8;
    u32 shm_sel;

    u32 shmem_len_lo;
    u32 shmem_len_hi;
    u32 shmem_بنياد_lo;
    u32 shmem_بنياد_hi;

    u32 res_c0_f7[14];

    u32 res_f8;
    u32 config_generation;
} virtio_mmio_cfg;

فراغ virtio_mmio_تياري_acpi(فراغ);
فراغ virtio_mmio_تياري_one(u64 mmio);
فراغ vp_init_mmio(بنية vp_device *vp, فراغ *mmio);

#جيڪڏهن_پورو /* _VIRTIO_MMIO_H */

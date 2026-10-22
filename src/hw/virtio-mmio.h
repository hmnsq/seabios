#اگر_تعریف_نشده _VIRTIO_MMIO_H
#تعریف_کن _VIRTIO_MMIO_H

بنية vp_device;

تعريفنوع بنية virtio_mmio_cfg {
    u32 magic;
    u32 نسخه_2;
    u32 device_id;
    u32 vendor_id;

    u32 device_feature;
    u32 device_feature_select;
    u32 res_18;
    u32 res_1c;

    u32 guest_feature;
    u32 guest_feature_select;
    u32 legacy_guest_صفحه_اندازه;
    u32 res_2c;

    u32 صف_select;
    u32 صف_رقم_max;
    u32 صف_رقم;
    u32 legacy_صف_align;

    u32 legacy_صف_pfn;
    u32 صف_ready;
    u32 res_48;
    u32 res_4c;

    u32 صف_notify;
    u32 res_54;
    u32 res_58;
    u32 res_5c;

    u32 irq_وضعیت;
    u32 irq_ack;
    u32 res_68;
    u32 res_6c;

    u32 device_وضعیت;
    u32 res_74;
    u32 res_78;
    u32 res_7c;

    u32 صف_desc_lo;
    u32 صف_desc_hi;
    u32 res_88;
    u32 res_8c;

    u32 صف_driver_lo;
    u32 صف_driver_hi;
    u32 res_98;
    u32 res_9c;

    u32 صف_device_lo;
    u32 صف_device_hi;
    u32 res_a8;
    u32 shm_sel;

    u32 shmem_len_lo;
    u32 shmem_len_hi;
    u32 shmem_پایه_lo;
    u32 shmem_پایه_hi;

    u32 res_c0_f7[14];

    u32 res_f8;
    u32 config_generation;
} virtio_mmio_cfg;

فراغ virtio_mmio_راهاندازی_acpi(فراغ);
فراغ virtio_mmio_راهاندازی_one(u64 mmio);
فراغ vp_init_mmio(بنية vp_device *vp, فراغ *mmio);

#پایان_اگر /* _VIRTIO_MMIO_H */

#यदि_अपरिभाषितम् _VIRTIO_MMIO_H
#परिभाषय _VIRTIO_MMIO_H

संरचना vp_device;

प्रकारनिर्देश संरचना virtio_mmio_cfg {
    u32 magic;
    u32 संस्करणम्_2;
    u32 device_id;
    u32 vendor_id;

    u32 device_feature;
    u32 device_feature_select;
    u32 res_18;
    u32 res_1c;

    u32 guest_feature;
    u32 guest_feature_select;
    u32 legacy_guest_page_आकार;
    u32 res_2c;

    u32 queue_select;
    u32 queue_अङ्कः_max;
    u32 queue_अङ्कः;
    u32 legacy_queue_align;

    u32 legacy_queue_pfn;
    u32 queue_ready;
    u32 res_48;
    u32 res_4c;

    u32 queue_notify;
    u32 res_54;
    u32 res_58;
    u32 res_5c;

    u32 irq_स्थितिः;
    u32 irq_ack;
    u32 res_68;
    u32 res_6c;

    u32 device_स्थितिः;
    u32 res_74;
    u32 res_78;
    u32 res_7c;

    u32 queue_desc_lo;
    u32 queue_desc_hi;
    u32 res_88;
    u32 res_8c;

    u32 queue_driver_lo;
    u32 queue_driver_hi;
    u32 res_98;
    u32 res_9c;

    u32 queue_device_lo;
    u32 queue_device_hi;
    u32 res_a8;
    u32 shm_sel;

    u32 shmem_len_lo;
    u32 shmem_len_hi;
    u32 shmem_आधारः_lo;
    u32 shmem_आधारः_hi;

    u32 res_c0_f7[14];

    u32 res_f8;
    u32 config_generation;
} virtio_mmio_cfg;

शून्य virtio_mmio_सज्जता_acpi(शून्य);
शून्य virtio_mmio_सज्जता_one(u64 mmio);
शून्य vp_init_mmio(संरचना vp_device *vp, शून्य *mmio);

#यदि_अन्तः /* _VIRTIO_MMIO_H */

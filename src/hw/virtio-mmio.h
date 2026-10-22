#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _VIRTIO_MMIO_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _VIRTIO_MMIO_H

ਬਣਤਰ vp_device;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ virtio_mmio_cfg {
    u32 magic;
    u32 ਸੰਸਕਰਣ_2;
    u32 device_id;
    u32 vendor_id;

    u32 device_feature;
    u32 device_feature_select;
    u32 res_18;
    u32 res_1c;

    u32 guest_feature;
    u32 guest_feature_select;
    u32 legacy_guest_ਪੰਨਾ_ਮਾਪ;
    u32 res_2c;

    u32 ਕਤਾਰ_select;
    u32 ਕਤਾਰ_ਅੰਕ_max;
    u32 ਕਤਾਰ_ਅੰਕ;
    u32 legacy_ਕਤਾਰ_align;

    u32 legacy_ਕਤਾਰ_pfn;
    u32 ਕਤਾਰ_ready;
    u32 res_48;
    u32 res_4c;

    u32 ਕਤਾਰ_notify;
    u32 res_54;
    u32 res_58;
    u32 res_5c;

    u32 irq_ਸਥਿਤੀ;
    u32 irq_ack;
    u32 res_68;
    u32 res_6c;

    u32 device_ਸਥਿਤੀ;
    u32 res_74;
    u32 res_78;
    u32 res_7c;

    u32 ਕਤਾਰ_desc_lo;
    u32 ਕਤਾਰ_desc_hi;
    u32 res_88;
    u32 res_8c;

    u32 ਕਤਾਰ_driver_lo;
    u32 ਕਤਾਰ_driver_hi;
    u32 res_98;
    u32 res_9c;

    u32 ਕਤਾਰ_device_lo;
    u32 ਕਤਾਰ_device_hi;
    u32 res_a8;
    u32 shm_sel;

    u32 shmem_len_lo;
    u32 shmem_len_hi;
    u32 shmem_ਅਧਾਰ_lo;
    u32 shmem_ਅਧਾਰ_hi;

    u32 res_c0_f7[14];

    u32 res_f8;
    u32 config_generation;
} virtio_mmio_cfg;

ਖਾਲੀ virtio_mmio_ਤਿਆਰੀ_acpi(ਖਾਲੀ);
ਖਾਲੀ virtio_mmio_ਤਿਆਰੀ_one(u64 mmio);
ਖਾਲੀ vp_init_mmio(ਬਣਤਰ vp_device *vp, ਖਾਲੀ *mmio);

#ਜੇ_ਅੰਤ /* _VIRTIO_MMIO_H */

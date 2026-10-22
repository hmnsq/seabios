#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _VIRTIO_MMIO_H
#ವ್ಯಾಖ್ಯಾನಿಸು _VIRTIO_MMIO_H

ರಚನೆ vp_device;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ virtio_mmio_cfg {
    u32 magic;
    u32 ಆವೃತ್ತಿ_2;
    u32 device_id;
    u32 vendor_id;

    u32 device_feature;
    u32 device_feature_select;
    u32 res_18;
    u32 res_1c;

    u32 guest_feature;
    u32 guest_feature_select;
    u32 legacy_guest_ಪುಟ_ಗಾತ್ರ;
    u32 res_2c;

    u32 ಸರತಿ_select;
    u32 ಸರತಿ_ಅಂಕೆ_max;
    u32 ಸರತಿ_ಅಂಕೆ;
    u32 legacy_ಸರತಿ_align;

    u32 legacy_ಸರತಿ_pfn;
    u32 ಸರತಿ_ready;
    u32 res_48;
    u32 res_4c;

    u32 ಸರತಿ_notify;
    u32 res_54;
    u32 res_58;
    u32 res_5c;

    u32 irq_ಸ್ಥಿತಿ;
    u32 irq_ack;
    u32 res_68;
    u32 res_6c;

    u32 device_ಸ್ಥಿತಿ;
    u32 res_74;
    u32 res_78;
    u32 res_7c;

    u32 ಸರತಿ_desc_lo;
    u32 ಸರತಿ_desc_hi;
    u32 res_88;
    u32 res_8c;

    u32 ಸರತಿ_driver_lo;
    u32 ಸರತಿ_driver_hi;
    u32 res_98;
    u32 res_9c;

    u32 ಸರತಿ_device_lo;
    u32 ಸರತಿ_device_hi;
    u32 res_a8;
    u32 shm_sel;

    u32 shmem_len_lo;
    u32 shmem_len_hi;
    u32 shmem_ತಳ_lo;
    u32 shmem_ತಳ_hi;

    u32 res_c0_f7[14];

    u32 res_f8;
    u32 config_generation;
} virtio_mmio_cfg;

ಶೂನ್ಯ virtio_mmio_ಸಿದ್ಧತೆ_acpi(ಶೂನ್ಯ);
ಶೂನ್ಯ virtio_mmio_ಸಿದ್ಧತೆ_one(u64 mmio);
ಶೂನ್ಯ vp_init_mmio(ರಚನೆ vp_device *vp, ಶೂನ್ಯ *mmio);

#ಆದರೆ_ಅಂತ್ಯ /* _VIRTIO_MMIO_H */

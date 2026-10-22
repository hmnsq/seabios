#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _VIRTIO_RING_H
#ವ್ಯಾಖ್ಯಾನಿಸು _VIRTIO_RING_H

#ಅನ್ತರ್ಭಾವ "types.h" // u64
#ಅನ್ತರ್ಭಾವ "memmap.h" // PAGE_SIZE

/* Status byte for guest to report progress, and synchronize features. */
/* We have seen device and processed generic fields (VIRTIO_CONFIG_F_VIRTIO) */
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_CONFIG_S_ACKNOWLEDGE     1
/* We have found a driver for the device. */
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_CONFIG_S_DRIVER          2
/* Driver has used its parts of the config, and is happy */
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_CONFIG_S_DRIVER_OK       4
/* Driver has finished configuring features */
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_CONFIG_S_FEATURES_OK     8
/* We've given up on this device. */
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_CONFIG_S_FAILED          0x80

/* v1.0 compliant. */
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_F_ಆವೃತ್ತಿ_1              32
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_F_IOMMU_PLATFORM         33

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_ಸರತಿ_ಅಂಕೆ      (256)

#ವ್ಯಾಖ್ಯಾನಿಸು VRING_DESC_F_ಮುಂದಿನ  1
#ವ್ಯಾಖ್ಯಾನಿಸು VRING_DESC_F_WRITE 2

#ವ್ಯಾಖ್ಯಾನಿಸು VRING_AVAIL_F_NO_ಅಡಚಣೆ 1

#ವ್ಯಾಖ್ಯಾನಿಸು VRING_ಬಳಸಿದ_F_NO_NOTIFY     1

ರಚನೆ vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 ಮುಂದಿನ;
};

ರಚನೆ vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

ರಚನೆ vring_ಬಳಸಿದ_elem
{
   u32 id;
   u32 len;
};

ರಚನೆ vring_ಬಳಸಿದ
{
   u16 flags;
   u16 idx;
   ರಚನೆ vring_ಬಳಸಿದ_elem ring[];
};

ರಚನೆ vring {
   ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಂಕೆ;
   ರಚನೆ vring_desc *desc;
   ರಚನೆ vring_avail *avail;
   ರಚನೆ vring_ಬಳಸಿದ *ಬಳಸಿದ;
};

#ವ್ಯಾಖ್ಯಾನಿಸು vring_ಗಾತ್ರ(ಅಂಕೆ) \
    (ALIGN(ಗಾತ್ರ(ರಚನೆ vring_desc) * ಅಂಕೆ + ಗಾತ್ರ(ರಚನೆ vring_avail) \
           + ಗಾತ್ರ(u16) * ಅಂಕೆ, ಪುಟ_ಗಾತ್ರ)                              \
     + ಗಾತ್ರ(ರಚನೆ vring_ಬಳಸಿದ) + ಗಾತ್ರ(ರಚನೆ vring_ಬಳಸಿದ_elem) * ಅಂಕೆ)

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ virtio_ಸರತಿ_t[vring_ಗಾತ್ರ(MAX_ಸರತಿ_ಅಂಕೆ)];

ರಚನೆ vring_virtqueue {
   virtio_ಸರತಿ_t ಸರತಿ;
   ರಚನೆ vring vring;
   u16 free_ತಲೆ;
   u16 ಕೊನೆಯ_ಬಳಸಿದ_idx;
   u16 vdata[MAX_ಸರತಿ_ಅಂಕೆ];
   /* PCI */
   ಪೂರ್ಣಾಂಕ ಸರತಿ_ಸೂಚ್ಯಂಕ;
   ಪೂರ್ಣಾಂಕ ಸರತಿ_notify_off;
};

ರಚನೆ vring_list {
  ಅಕ್ಷರ *addr;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ;
};

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
vring_init(ರಚನೆ vring *vr, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಂಕೆ, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಸರತಿ)
{
   ASSERT32FLAT();
   vr->ಅಂಕೆ = ಅಂಕೆ;

   /* physical address of desc must be page aligned */
   vr->desc = (ಶೂನ್ಯ*)ALIGN((u32)ಸರತಿ, ಪುಟ_ಗಾತ್ರ);

   vr->avail = (ರಚನೆ vring_avail *)&vr->desc[ಅಂಕೆ];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_ಅಡಚಣೆ;

   /* physical address of used must be page aligned */
   vr->ಬಳಸಿದ = (ಶೂನ್ಯ*)ALIGN((u32)&vr->avail->ring[ಅಂಕೆ], ಪುಟ_ಗಾತ್ರ);

   ಪೂರ್ಣಾಂಕ i;
   ಗಾಗಿ (i = 0; i < ಅಂಕೆ - 1; i++)
       vr->desc[i].ಮುಂದಿನ = i + 1;
   vr->desc[i].ಮುಂದಿನ = 0;
}

ರಚನೆ vp_device;
ಪೂರ್ಣಾಂಕ vring_ಇನ್ನಷ್ಟು_ಬಳಸಿದ(ರಚನೆ vring_virtqueue *vq);
ಶೂನ್ಯ vring_detach(ರಚನೆ vring_virtqueue *vq, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ತಲೆ);
ಪೂರ್ಣಾಂಕ vring_get_buf(ರಚನೆ vring_virtqueue *vq, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *len);
ಶೂನ್ಯ vring_ಕೂಡಿಸು_buf(ರಚನೆ vring_virtqueue *vq, ರಚನೆ vring_list list[],
                   ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಹೊರಹೋಗು, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಒಳ,
                   ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ, ಪೂರ್ಣಾಂಕ ಅಂಕೆ_added);
ಶೂನ್ಯ vring_kick(ರಚನೆ vp_device *vp, ರಚನೆ vring_virtqueue *vq, ಪೂರ್ಣಾಂಕ ಅಂಕೆ_added);

#ಆದರೆ_ಅಂತ್ಯ /* _VIRTIO_RING_H_ */

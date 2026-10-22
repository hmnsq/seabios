#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _VIRTIO_RING_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _VIRTIO_RING_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u64
#ਅਨ੍ਤਰ੍ਭਾਵ "memmap.h" // PAGE_SIZE

/* Status byte for guest to report progress, and synchronize features. */
/* We have seen device and processed generic fields (VIRTIO_CONFIG_F_VIRTIO) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_CONFIG_S_ACKNOWLEDGE     1
/* We have found a driver for the device. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_CONFIG_S_DRIVER          2
/* Driver has used its parts of the config, and is happy */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_CONFIG_S_DRIVER_OK       4
/* Driver has finished configuring features */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_CONFIG_S_FEATURES_OK     8
/* We've given up on this device. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_CONFIG_S_FAILED          0x80

/* v1.0 compliant. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_F_ਸੰਸਕਰਣ_1              32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_F_IOMMU_PLATFORM         33

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_ਕਤਾਰ_ਅੰਕ      (256)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VRING_DESC_F_ਅਗਲਾ  1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VRING_DESC_F_WRITE 2

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VRING_AVAIL_F_NO_ਰੁਕਾਵਟ 1

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VRING_ਵਰਤਿਆ_F_NO_NOTIFY     1

ਬਣਤਰ vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 ਅਗਲਾ;
};

ਬਣਤਰ vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

ਬਣਤਰ vring_ਵਰਤਿਆ_elem
{
   u32 id;
   u32 len;
};

ਬਣਤਰ vring_ਵਰਤਿਆ
{
   u16 flags;
   u16 idx;
   ਬਣਤਰ vring_ਵਰਤਿਆ_elem ring[];
};

ਬਣਤਰ vring {
   ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਅੰਕ;
   ਬਣਤਰ vring_desc *desc;
   ਬਣਤਰ vring_avail *avail;
   ਬਣਤਰ vring_ਵਰਤਿਆ *ਵਰਤਿਆ;
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ vring_ਮਾਪ(ਅੰਕ) \
    (ALIGN(ਆਕਾਰ(ਬਣਤਰ vring_desc) * ਅੰਕ + ਆਕਾਰ(ਬਣਤਰ vring_avail) \
           + ਆਕਾਰ(u16) * ਅੰਕ, ਪੰਨਾ_ਮਾਪ)                              \
     + ਆਕਾਰ(ਬਣਤਰ vring_ਵਰਤਿਆ) + ਆਕਾਰ(ਬਣਤਰ vring_ਵਰਤਿਆ_elem) * ਅੰਕ)

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ virtio_ਕਤਾਰ_t[vring_ਮਾਪ(MAX_ਕਤਾਰ_ਅੰਕ)];

ਬਣਤਰ vring_virtqueue {
   virtio_ਕਤਾਰ_t ਕਤਾਰ;
   ਬਣਤਰ vring vring;
   u16 free_ਸਿਰ;
   u16 ਆਖਰੀ_ਵਰਤਿਆ_idx;
   u16 vdata[MAX_ਕਤਾਰ_ਅੰਕ];
   /* PCI */
   ਪੂਰਨ ਕਤਾਰ_ਸੂਚਕਾਂਕ;
   ਪੂਰਨ ਕਤਾਰ_notify_off;
};

ਬਣਤਰ vring_list {
  ਅੱਖਰ *addr;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਲੰਮਾਈ;
};

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
vring_init(ਬਣਤਰ vring *vr, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਅੰਕ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਕਤਾਰ)
{
   ASSERT32FLAT();
   vr->ਅੰਕ = ਅੰਕ;

   /* physical address of desc must be page aligned */
   vr->desc = (ਖਾਲੀ*)ALIGN((u32)ਕਤਾਰ, ਪੰਨਾ_ਮਾਪ);

   vr->avail = (ਬਣਤਰ vring_avail *)&vr->desc[ਅੰਕ];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_ਰੁਕਾਵਟ;

   /* physical address of used must be page aligned */
   vr->ਵਰਤਿਆ = (ਖਾਲੀ*)ALIGN((u32)&vr->avail->ring[ਅੰਕ], ਪੰਨਾ_ਮਾਪ);

   ਪੂਰਨ i;
   ਲਈ (i = 0; i < ਅੰਕ - 1; i++)
       vr->desc[i].ਅਗਲਾ = i + 1;
   vr->desc[i].ਅਗਲਾ = 0;
}

ਬਣਤਰ vp_device;
ਪੂਰਨ vring_ਹੋਰ_ਵਰਤਿਆ(ਬਣਤਰ vring_virtqueue *vq);
ਖਾਲੀ vring_detach(ਬਣਤਰ vring_virtqueue *vq, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸਿਰ);
ਪੂਰਨ vring_get_buf(ਬਣਤਰ vring_virtqueue *vq, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *len);
ਖਾਲੀ vring_ਜੋੜੋ_buf(ਬਣਤਰ vring_virtqueue *vq, ਬਣਤਰ vring_list list[],
                   ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਬਾਹਰਲਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਵਿੱਚ_2,
                   ਪੂਰਨ ਸੂਚਕਾਂਕ, ਪੂਰਨ ਅੰਕ_added);
ਖਾਲੀ vring_kick(ਬਣਤਰ vp_device *vp, ਬਣਤਰ vring_virtqueue *vq, ਪੂਰਨ ਅੰਕ_added);

#ਜੇ_ਅੰਤ /* _VIRTIO_RING_H_ */

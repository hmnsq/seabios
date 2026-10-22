#ifndef _VIRTIO_RING_H
#define _VIRTIO_RING_H

#ಅನ್ತರ್ಭಾವ "types.h" // u64
#ಅನ್ತರ್ಭಾವ "memmap.h" // PAGE_SIZE

/* Status byte for guest to report progress, and synchronize features. */
/* We have seen device and processed generic fields (VIRTIO_CONFIG_F_VIRTIO) */
#define VIRTIO_CONFIG_S_ACKNOWLEDGE     1
/* We have found a driver for the device. */
#define VIRTIO_CONFIG_S_DRIVER          2
/* Driver has used its parts of the config, and is happy */
#define VIRTIO_CONFIG_S_DRIVER_OK       4
/* Driver has finished configuring features */
#define VIRTIO_CONFIG_S_FEATURES_OK     8
/* We've given up on this device. */
#define VIRTIO_CONFIG_S_FAILED          0x80

/* v1.0 compliant. */
#define VIRTIO_F_VERSION_1              32
#define VIRTIO_F_IOMMU_PLATFORM         33

#define MAX_QUEUE_ಅಂಕೆ      (256)

#define VRING_DESC_F_NEXT  1
#define VRING_DESC_F_WRITE 2

#define VRING_AVAIL_F_NO_INTERRUPT 1

#define VRING_USED_F_NO_NOTIFY     1

ಸಂರಚನಾ vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 next;
};

ಸಂರಚನಾ vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

ಸಂರಚನಾ vring_used_elem
{
   u32 id;
   u32 len;
};

ಸಂರಚನಾ vring_used
{
   u16 flags;
   u16 idx;
   ಸಂರಚನಾ vring_used_elem ring[];
};

ಸಂರಚನಾ vring {
   unsigned ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ;
   ಸಂರಚನಾ vring_desc *desc;
   ಸಂರಚನಾ vring_avail *avail;
   ಸಂರಚನಾ vring_used *used;
};

#define vring_size(ಅಂಕೆ) \
    (ALIGN(ಪರಿಮಾಣ(ಸಂರಚನಾ vring_desc) * ಅಂಕೆ + ಪರಿಮಾಣ(ಸಂರಚನಾ vring_avail) \
           + ಪರಿಮಾಣ(u16) * ಅಂಕೆ, PAGE_SIZE)                              \
     + ಪರಿಮಾಣ(ಸಂರಚನಾ vring_used) + ಪರಿಮಾಣ(ಸಂರಚನಾ vring_used_elem) * ಅಂಕೆ)

ಪ್ರಕಾರನಿರ್ದೇಶ unsigned ವರ್ಣ virtio_queue_t[vring_size(MAX_QUEUE_ಅಂಕೆ)];

ಸಂರಚನಾ vring_virtqueue {
   virtio_queue_t queue;
   ಸಂರಚನಾ vring vring;
   u16 free_head;
   u16 last_used_idx;
   u16 vdata[MAX_QUEUE_ಅಂಕೆ];
   /* PCI */
   ಪೂರ್ಣಾಙ್ಕ queue_index;
   ಪೂರ್ಣಾಙ್ಕ queue_notify_off;
};

ಸಂರಚನಾ vring_list {
  ವರ್ಣ *addr;
  unsigned ಪೂರ್ಣಾಙ್ಕ length;
};

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
vring_init(ಸಂರಚನಾ vring *vr, unsigned ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ, unsigned ವರ್ಣ *queue)
{
   ASSERT32FLAT();
   vr->ಅಂಕೆ = ಅಂಕೆ;

   /* physical address of desc must be page aligned */
   vr->desc = (ಶೂನ್ಯ*)ALIGN((u32)queue, PAGE_SIZE);

   vr->avail = (ಸಂರಚನಾ vring_avail *)&vr->desc[ಅಂಕೆ];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_INTERRUPT;

   /* physical address of used must be page aligned */
   vr->used = (ಶೂನ್ಯ*)ALIGN((u32)&vr->avail->ring[ಅಂಕೆ], PAGE_SIZE);

   ಪೂರ್ಣಾಙ್ಕ i;
   ಪ್ರತಿ (i = 0; i < ಅಂಕೆ - 1; i++)
       vr->desc[i].next = i + 1;
   vr->desc[i].next = 0;
}

ಸಂರಚನಾ vp_device;
ಪೂರ್ಣಾಙ್ಕ vring_more_used(ಸಂರಚನಾ vring_virtqueue *vq);
ಶೂನ್ಯ vring_detach(ಸಂರಚನಾ vring_virtqueue *vq, unsigned ಪೂರ್ಣಾಙ್ಕ head);
ಪೂರ್ಣಾಙ್ಕ vring_get_buf(ಸಂರಚನಾ vring_virtqueue *vq, unsigned ಪೂರ್ಣಾಙ್ಕ *len);
ಶೂನ್ಯ vring_add_buf(ಸಂರಚನಾ vring_virtqueue *vq, ಸಂರಚನಾ vring_list list[],
                   unsigned ಪೂರ್ಣಾಙ್ಕ out, unsigned ಪೂರ್ಣಾಙ್ಕ in,
                   ಪೂರ್ಣಾಙ್ಕ index, ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_added);
ಶೂನ್ಯ vring_kick(ಸಂರಚನಾ vp_device *vp, ಸಂರಚನಾ vring_virtqueue *vq, ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_added);

#endif /* _VIRTIO_RING_H_ */

#ifndef _VIRTIO_RING_H
#define _VIRTIO_RING_H

#அந்தர்பாவ "types.h" // u64
#அந்தர்பாவ "memmap.h" // PAGE_SIZE

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

#define MAX_QUEUE_இலக்கம்      (256)

#define VRING_DESC_F_NEXT  1
#define VRING_DESC_F_WRITE 2

#define VRING_AVAIL_F_NO_INTERRUPT 1

#define VRING_USED_F_NO_NOTIFY     1

கட்டமைப்பு vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 next;
};

கட்டமைப்பு vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

கட்டமைப்பு vring_used_elem
{
   u32 id;
   u32 len;
};

கட்டமைப்பு vring_used
{
   u16 flags;
   u16 idx;
   கட்டமைப்பு vring_used_elem ring[];
};

கட்டமைப்பு vring {
   unsigned முழு இலக்கம்;
   கட்டமைப்பு vring_desc *desc;
   கட்டமைப்பு vring_avail *avail;
   கட்டமைப்பு vring_used *used;
};

#define vring_size(இலக்கம்) \
    (ALIGN(அளவு(கட்டமைப்பு vring_desc) * இலக்கம் + அளவு(கட்டமைப்பு vring_avail) \
           + அளவு(u16) * இலக்கம், PAGE_SIZE)                              \
     + அளவு(கட்டமைப்பு vring_used) + அளவு(கட்டமைப்பு vring_used_elem) * இலக்கம்)

வகைவரை unsigned எழுத்து virtio_queue_t[vring_size(MAX_QUEUE_இலக்கம்)];

கட்டமைப்பு vring_virtqueue {
   virtio_queue_t queue;
   கட்டமைப்பு vring vring;
   u16 free_head;
   u16 last_used_idx;
   u16 vdata[MAX_QUEUE_இலக்கம்];
   /* PCI */
   முழு queue_index;
   முழு queue_notify_off;
};

கட்டமைப்பு vring_list {
  எழுத்து *addr;
  unsigned முழு length;
};

நிலையான உள்வரி வெறுமை
vring_init(கட்டமைப்பு vring *vr, unsigned முழு இலக்கம், unsigned எழுத்து *queue)
{
   ASSERT32FLAT();
   vr->இலக்கம் = இலக்கம்;

   /* physical address of desc must be page aligned */
   vr->desc = (வெறுமை*)ALIGN((u32)queue, PAGE_SIZE);

   vr->avail = (கட்டமைப்பு vring_avail *)&vr->desc[இலக்கம்];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_INTERRUPT;

   /* physical address of used must be page aligned */
   vr->used = (வெறுமை*)ALIGN((u32)&vr->avail->ring[இலக்கம்], PAGE_SIZE);

   முழு i;
   க்கு (i = 0; i < இலக்கம் - 1; i++)
       vr->desc[i].next = i + 1;
   vr->desc[i].next = 0;
}

கட்டமைப்பு vp_device;
முழு vring_more_used(கட்டமைப்பு vring_virtqueue *vq);
வெறுமை vring_detach(கட்டமைப்பு vring_virtqueue *vq, unsigned முழு head);
முழு vring_get_buf(கட்டமைப்பு vring_virtqueue *vq, unsigned முழு *len);
வெறுமை vring_add_buf(கட்டமைப்பு vring_virtqueue *vq, கட்டமைப்பு vring_list list[],
                   unsigned முழு out, unsigned முழு in,
                   முழு index, முழு இலக்கம்_added);
வெறுமை vring_kick(கட்டமைப்பு vp_device *vp, கட்டமைப்பு vring_virtqueue *vq, முழு இலக்கம்_added);

#endif /* _VIRTIO_RING_H_ */

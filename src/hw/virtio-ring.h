#ifndef _VIRTIO_RING_H
#define _VIRTIO_RING_H

#شامل "types.h" // u64
#شامل "memmap.h" // PAGE_SIZE

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

#define MAX_QUEUE_رقم      (256)

#define VRING_DESC_F_NEXT  1
#define VRING_DESC_F_WRITE 2

#define VRING_AVAIL_F_NO_INTERRUPT 1

#define VRING_USED_F_NO_NOTIFY     1

ساختار vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 next;
};

ساختار vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

ساختار vring_used_elem
{
   u32 id;
   u32 len;
};

ساختار vring_used
{
   u16 flags;
   u16 idx;
   ساختار vring_used_elem ring[];
};

ساختار vring {
   unsigned صحیح رقم;
   ساختار vring_desc *desc;
   ساختار vring_avail *avail;
   ساختار vring_used *used;
};

#define vring_size(رقم) \
    (ALIGN(اندازه(ساختار vring_desc) * رقم + اندازه(ساختار vring_avail) \
           + اندازه(u16) * رقم, PAGE_SIZE)                              \
     + اندازه(ساختار vring_used) + اندازه(ساختار vring_used_elem) * رقم)

تعریفنوع unsigned نویسه virtio_queue_t[vring_size(MAX_QUEUE_رقم)];

ساختار vring_virtqueue {
   virtio_queue_t queue;
   ساختار vring vring;
   u16 free_head;
   u16 last_used_idx;
   u16 vdata[MAX_QUEUE_رقم];
   /* PCI */
   صحیح queue_index;
   صحیح queue_notify_off;
};

ساختار vring_list {
  نویسه *addr;
  unsigned صحیح length;
};

ایستا درونخطی تهی
vring_init(ساختار vring *vr, unsigned صحیح رقم, unsigned نویسه *queue)
{
   ASSERT32FLAT();
   vr->رقم = رقم;

   /* physical address of desc must be page aligned */
   vr->desc = (تهی*)ALIGN((u32)queue, PAGE_SIZE);

   vr->avail = (ساختار vring_avail *)&vr->desc[رقم];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_INTERRUPT;

   /* physical address of used must be page aligned */
   vr->used = (تهی*)ALIGN((u32)&vr->avail->ring[رقم], PAGE_SIZE);

   صحیح i;
   برای (i = 0; i < رقم - 1; i++)
       vr->desc[i].next = i + 1;
   vr->desc[i].next = 0;
}

ساختار vp_device;
صحیح vring_more_used(ساختار vring_virtqueue *vq);
تهی vring_detach(ساختار vring_virtqueue *vq, unsigned صحیح head);
صحیح vring_get_buf(ساختار vring_virtqueue *vq, unsigned صحیح *len);
تهی vring_add_buf(ساختار vring_virtqueue *vq, ساختار vring_list list[],
                   unsigned صحیح out, unsigned صحیح in,
                   صحیح index, صحیح رقم_added);
تهی vring_kick(ساختار vp_device *vp, ساختار vring_virtqueue *vq, صحیح رقم_added);

#endif /* _VIRTIO_RING_H_ */

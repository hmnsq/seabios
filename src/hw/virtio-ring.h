#ifndef _VIRTIO_RING_H
#define _VIRTIO_RING_H

#অন্তর্ভাৱ "types.h" // u64
#অন্তর্ভাৱ "memmap.h" // PAGE_SIZE

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

#define MAX_QUEUE_অঙ্ক      (256)

#define VRING_DESC_F_NEXT  1
#define VRING_DESC_F_WRITE 2

#define VRING_AVAIL_F_NO_INTERRUPT 1

#define VRING_USED_F_NO_NOTIFY     1

সংরচনা vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 next;
};

সংরচনা vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

সংরচনা vring_used_elem
{
   u32 id;
   u32 len;
};

সংরচনা vring_used
{
   u16 flags;
   u16 idx;
   সংরচনা vring_used_elem ring[];
};

সংরচনা vring {
   unsigned পূর্ণাঙ্ক অঙ্ক;
   সংরচনা vring_desc *desc;
   সংরচনা vring_avail *avail;
   সংরচনা vring_used *used;
};

#define vring_size(অঙ্ক) \
    (ALIGN(পরিমাণ(সংরচনা vring_desc) * অঙ্ক + পরিমাণ(সংরচনা vring_avail) \
           + পরিমাণ(u16) * অঙ্ক, PAGE_SIZE)                              \
     + পরিমাণ(সংরচনা vring_used) + পরিমাণ(সংরচনা vring_used_elem) * অঙ্ক)

প্রকারনির্দেশ unsigned ৱর্ণ virtio_queue_t[vring_size(MAX_QUEUE_অঙ্ক)];

সংরচনা vring_virtqueue {
   virtio_queue_t queue;
   সংরচনা vring vring;
   u16 free_head;
   u16 last_used_idx;
   u16 vdata[MAX_QUEUE_অঙ্ক];
   /* PCI */
   পূর্ণাঙ্ক queue_index;
   পূর্ণাঙ্ক queue_notify_off;
};

সংরচনা vring_list {
  ৱর্ণ *addr;
  unsigned পূর্ণাঙ্ক length;
};

স্থির অন্তঃস্থ শূন্য
vring_init(সংরচনা vring *vr, unsigned পূর্ণাঙ্ক অঙ্ক, unsigned ৱর্ণ *queue)
{
   ASSERT32FLAT();
   vr->অঙ্ক = অঙ্ক;

   /* physical address of desc must be page aligned */
   vr->desc = (শূন্য*)ALIGN((u32)queue, PAGE_SIZE);

   vr->avail = (সংরচনা vring_avail *)&vr->desc[অঙ্ক];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_INTERRUPT;

   /* physical address of used must be page aligned */
   vr->used = (শূন্য*)ALIGN((u32)&vr->avail->ring[অঙ্ক], PAGE_SIZE);

   পূর্ণাঙ্ক i;
   বাবে (i = 0; i < অঙ্ক - 1; i++)
       vr->desc[i].next = i + 1;
   vr->desc[i].next = 0;
}

সংরচনা vp_device;
পূর্ণাঙ্ক vring_more_used(সংরচনা vring_virtqueue *vq);
শূন্য vring_detach(সংরচনা vring_virtqueue *vq, unsigned পূর্ণাঙ্ক head);
পূর্ণাঙ্ক vring_get_buf(সংরচনা vring_virtqueue *vq, unsigned পূর্ণাঙ্ক *len);
শূন্য vring_add_buf(সংরচনা vring_virtqueue *vq, সংরচনা vring_list list[],
                   unsigned পূর্ণাঙ্ক out, unsigned পূর্ণাঙ্ক in,
                   পূর্ণাঙ্ক index, পূর্ণাঙ্ক অঙ্ক_added);
শূন্য vring_kick(সংরচনা vp_device *vp, সংরচনা vring_virtqueue *vq, পূর্ণাঙ্ক অঙ্ক_added);

#endif /* _VIRTIO_RING_H_ */

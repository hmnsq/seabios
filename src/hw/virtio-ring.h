#ifndef _VIRTIO_RING_H
#define _VIRTIO_RING_H

#ضمن "types.h" // u64
#ضمن "memmap.h" // PAGE_SIZE

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

#define MAX_QUEUE_NUM      (256)

#define VRING_DESC_F_NEXT  1
#define VRING_DESC_F_WRITE 2

#define VRING_AVAIL_F_NO_INTERRUPT 1

#define VRING_USED_F_NO_NOTIFY     1

بنية vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 next;
};

بنية vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

بنية vring_used_elem
{
   u32 id;
   u32 len;
};

بنية vring_used
{
   u16 flags;
   u16 idx;
   بنية vring_used_elem ring[];
};

بنية vring {
   unsigned صحيح num;
   بنية vring_desc *desc;
   بنية vring_avail *avail;
   بنية vring_used *used;
};

#define vring_size(num) \
    (ALIGN(حجم(بنية vring_desc) * num + حجم(بنية vring_avail) \
           + حجم(u16) * num, PAGE_SIZE)                              \
     + حجم(بنية vring_used) + حجم(بنية vring_used_elem) * num)

تعريفنوع unsigned حرف virtio_queue_t[vring_size(MAX_QUEUE_NUM)];

بنية vring_virtqueue {
   virtio_queue_t queue;
   بنية vring vring;
   u16 free_head;
   u16 last_used_idx;
   u16 vdata[MAX_QUEUE_NUM];
   /* PCI */
   صحيح queue_index;
   صحيح queue_notify_off;
};

بنية vring_list {
  حرف *addr;
  unsigned صحيح length;
};

ساكن مضمن فراغ
vring_init(بنية vring *vr, unsigned صحيح num, unsigned حرف *queue)
{
   ASSERT32FLAT();
   vr->num = num;

   /* physical address of desc must be page aligned */
   vr->desc = (فراغ*)ALIGN((u32)queue, PAGE_SIZE);

   vr->avail = (بنية vring_avail *)&vr->desc[num];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_INTERRUPT;

   /* physical address of used must be page aligned */
   vr->used = (فراغ*)ALIGN((u32)&vr->avail->ring[num], PAGE_SIZE);

   صحيح i;
   لكل (i = 0; i < num - 1; i++)
       vr->desc[i].next = i + 1;
   vr->desc[i].next = 0;
}

بنية vp_device;
صحيح vring_more_used(بنية vring_virtqueue *vq);
فراغ vring_detach(بنية vring_virtqueue *vq, unsigned صحيح head);
صحيح vring_get_buf(بنية vring_virtqueue *vq, unsigned صحيح *len);
فراغ vring_add_buf(بنية vring_virtqueue *vq, بنية vring_list list[],
                   unsigned صحيح out, unsigned صحيح in,
                   صحيح index, صحيح num_added);
فراغ vring_kick(بنية vp_device *vp, بنية vring_virtqueue *vq, صحيح num_added);

#endif /* _VIRTIO_RING_H_ */

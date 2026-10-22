#ifndef _VIRTIO_RING_H
#define _VIRTIO_RING_H

#অন্তর্ভাব "types.h" // u64
#অন্তর্ভাব "memmap.h" // PAGE_SIZE

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

#define VRING_DESC_F_পরবর্তী  1
#define VRING_DESC_F_WRITE 2

#define VRING_AVAIL_F_NO_INTERRUPT 1

#define VRING_USED_F_NO_NOTIFY     1

কাঠামো vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 পরবর্তী;
};

কাঠামো vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

কাঠামো vring_used_elem
{
   u32 id;
   u32 len;
};

কাঠামো vring_used
{
   u16 flags;
   u16 idx;
   কাঠামো vring_used_elem ring[];
};

কাঠামো vring {
   unsigned পূর্ণসংখ্যা অঙ্ক;
   কাঠামো vring_desc *desc;
   কাঠামো vring_avail *avail;
   কাঠামো vring_used *used;
};

#define vring_আকার(অঙ্ক) \
    (ALIGN(আকার(কাঠামো vring_desc) * অঙ্ক + আকার(কাঠামো vring_avail) \
           + আকার(u16) * অঙ্ক, PAGE_আকার)                              \
     + আকার(কাঠামো vring_used) + আকার(কাঠামো vring_used_elem) * অঙ্ক)

প্রকারলেখ unsigned অক্ষর virtio_queue_t[vring_আকার(MAX_QUEUE_অঙ্ক)];

কাঠামো vring_virtqueue {
   virtio_queue_t queue;
   কাঠামো vring vring;
   u16 free_শীর্ষ;
   u16 last_used_idx;
   u16 vdata[MAX_QUEUE_অঙ্ক];
   /* PCI */
   পূর্ণসংখ্যা queue_সূচক;
   পূর্ণসংখ্যা queue_notify_off;
};

কাঠামো vring_list {
  অক্ষর *addr;
  unsigned পূর্ণসংখ্যা দৈর্ঘ্য;
};

স্থির অন্তঃরেখ শূন্য
vring_init(কাঠামো vring *vr, unsigned পূর্ণসংখ্যা অঙ্ক, unsigned অক্ষর *queue)
{
   ASSERT32FLAT();
   vr->অঙ্ক = অঙ্ক;

   /* physical address of desc must be page aligned */
   vr->desc = (শূন্য*)ALIGN((u32)queue, PAGE_আকার);

   vr->avail = (কাঠামো vring_avail *)&vr->desc[অঙ্ক];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_INTERRUPT;

   /* physical address of used must be page aligned */
   vr->used = (শূন্য*)ALIGN((u32)&vr->avail->ring[অঙ্ক], PAGE_আকার);

   পূর্ণসংখ্যা i;
   জন্য (i = 0; i < অঙ্ক - 1; i++)
       vr->desc[i].পরবর্তী = i + 1;
   vr->desc[i].পরবর্তী = 0;
}

কাঠামো vp_device;
পূর্ণসংখ্যা vring_more_used(কাঠামো vring_virtqueue *vq);
শূন্য vring_detach(কাঠামো vring_virtqueue *vq, unsigned পূর্ণসংখ্যা শীর্ষ);
পূর্ণসংখ্যা vring_get_buf(কাঠামো vring_virtqueue *vq, unsigned পূর্ণসংখ্যা *len);
শূন্য vring_add_buf(কাঠামো vring_virtqueue *vq, কাঠামো vring_list list[],
                   unsigned পূর্ণসংখ্যা out, unsigned পূর্ণসংখ্যা in,
                   পূর্ণসংখ্যা সূচক, পূর্ণসংখ্যা অঙ্ক_added);
শূন্য vring_kick(কাঠামো vp_device *vp, কাঠামো vring_virtqueue *vq, পূর্ণসংখ্যা অঙ্ক_added);

#endif /* _VIRTIO_RING_H_ */

#ifndef _VIRTIO_RING_H
#define _VIRTIO_RING_H

#ଅନ୍ତର୍ଭାଵ "types.h" // u64
#ଅନ୍ତର୍ଭାଵ "memmap.h" // PAGE_SIZE

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

#define MAX_QUEUE_ଅଙ୍କ      (256)

#define VRING_DESC_F_NEXT  1
#define VRING_DESC_F_WRITE 2

#define VRING_AVAIL_F_NO_INTERRUPT 1

#define VRING_USED_F_NO_NOTIFY     1

ସଂରଚନା vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 next;
};

ସଂରଚନା vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

ସଂରଚନା vring_used_elem
{
   u32 id;
   u32 len;
};

ସଂରଚନା vring_used
{
   u16 flags;
   u16 idx;
   ସଂରଚନା vring_used_elem ring[];
};

ସଂରଚନା vring {
   unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ;
   ସଂରଚନା vring_desc *desc;
   ସଂରଚନା vring_avail *avail;
   ସଂରଚନା vring_used *used;
};

#define vring_size(ଅଙ୍କ) \
    (ALIGN(ଆକାର(ସଂରଚନା vring_desc) * ଅଙ୍କ + ଆକାର(ସଂରଚନା vring_avail) \
           + ଆକାର(u16) * ଅଙ୍କ, PAGE_SIZE)                              \
     + ଆକାର(ସଂରଚନା vring_used) + ଆକାର(ସଂରଚନା vring_used_elem) * ଅଙ୍କ)

ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned ଅକ୍ଷର virtio_queue_t[vring_size(MAX_QUEUE_ଅଙ୍କ)];

ସଂରଚନା vring_virtqueue {
   virtio_queue_t queue;
   ସଂରଚନା vring vring;
   u16 free_head;
   u16 last_used_idx;
   u16 vdata[MAX_QUEUE_ଅଙ୍କ];
   /* PCI */
   ପୂର୍ଣ୍ଣସଂଖ୍ୟା queue_index;
   ପୂର୍ଣ୍ଣସଂଖ୍ୟା queue_notify_off;
};

ସଂରଚନା vring_list {
  ଅକ୍ଷର *addr;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା length;
};

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
vring_init(ସଂରଚନା vring *vr, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ, unsigned ଅକ୍ଷର *queue)
{
   ASSERT32FLAT();
   vr->ଅଙ୍କ = ଅଙ୍କ;

   /* physical address of desc must be page aligned */
   vr->desc = (ଶୂନ୍ୟ*)ALIGN((u32)queue, PAGE_SIZE);

   vr->avail = (ସଂରଚନା vring_avail *)&vr->desc[ଅଙ୍କ];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_INTERRUPT;

   /* physical address of used must be page aligned */
   vr->used = (ଶୂନ୍ୟ*)ALIGN((u32)&vr->avail->ring[ଅଙ୍କ], PAGE_SIZE);

   ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
   ପାଇଁ (i = 0; i < ଅଙ୍କ - 1; i++)
       vr->desc[i].next = i + 1;
   vr->desc[i].next = 0;
}

ସଂରଚନା vp_device;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା vring_more_used(ସଂରଚନା vring_virtqueue *vq);
ଶୂନ୍ୟ vring_detach(ସଂରଚନା vring_virtqueue *vq, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା head);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା vring_get_buf(ସଂରଚନା vring_virtqueue *vq, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *len);
ଶୂନ୍ୟ vring_add_buf(ସଂରଚନା vring_virtqueue *vq, ସଂରଚନା vring_list list[],
                   unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା out, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା in,
                   ପୂର୍ଣ୍ଣସଂଖ୍ୟା index, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_added);
ଶୂନ୍ୟ vring_kick(ସଂରଚନା vp_device *vp, ସଂରଚନା vring_virtqueue *vq, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_added);

#endif /* _VIRTIO_RING_H_ */

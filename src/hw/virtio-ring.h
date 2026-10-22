#ifndef _VIRTIO_RING_H
#define _VIRTIO_RING_H

#અન્તર્ભાવ "types.h" // u64
#અન્તર્ભાવ "memmap.h" // PAGE_SIZE

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

#define MAX_QUEUE_અંક      (256)

#define VRING_DESC_F_NEXT  1
#define VRING_DESC_F_WRITE 2

#define VRING_AVAIL_F_NO_INTERRUPT 1

#define VRING_USED_F_NO_NOTIFY     1

સંરચના vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 next;
};

સંરચના vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

સંરચના vring_used_elem
{
   u32 id;
   u32 len;
};

સંરચના vring_used
{
   u16 flags;
   u16 idx;
   સંરચના vring_used_elem ring[];
};

સંરચના vring {
   unsigned પૂર્ણાંક અંક;
   સંરચના vring_desc *desc;
   સંરચના vring_avail *avail;
   સંરચના vring_used *used;
};

#define vring_size(અંક) \
    (ALIGN(કદ(સંરચના vring_desc) * અંક + કદ(સંરચના vring_avail) \
           + કદ(u16) * અંક, PAGE_SIZE)                              \
     + કદ(સંરચના vring_used) + કદ(સંરચના vring_used_elem) * અંક)

પ્રકારવ્યાખ્યા unsigned અક્ષર virtio_queue_t[vring_size(MAX_QUEUE_અંક)];

સંરચના vring_virtqueue {
   virtio_queue_t queue;
   સંરચના vring vring;
   u16 free_head;
   u16 last_used_idx;
   u16 vdata[MAX_QUEUE_અંક];
   /* PCI */
   પૂર્ણાંક queue_index;
   પૂર્ણાંક queue_notify_off;
};

સંરચના vring_list {
  અક્ષર *addr;
  unsigned પૂર્ણાંક length;
};

સ્થિર અંતઃરેખ શૂન્ય
vring_init(સંરચના vring *vr, unsigned પૂર્ણાંક અંક, unsigned અક્ષર *queue)
{
   ASSERT32FLAT();
   vr->અંક = અંક;

   /* physical address of desc must be page aligned */
   vr->desc = (શૂન્ય*)ALIGN((u32)queue, PAGE_SIZE);

   vr->avail = (સંરચના vring_avail *)&vr->desc[અંક];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_INTERRUPT;

   /* physical address of used must be page aligned */
   vr->used = (શૂન્ય*)ALIGN((u32)&vr->avail->ring[અંક], PAGE_SIZE);

   પૂર્ણાંક i;
   માટે (i = 0; i < અંક - 1; i++)
       vr->desc[i].next = i + 1;
   vr->desc[i].next = 0;
}

સંરચના vp_device;
પૂર્ણાંક vring_more_used(સંરચના vring_virtqueue *vq);
શૂન્ય vring_detach(સંરચના vring_virtqueue *vq, unsigned પૂર્ણાંક head);
પૂર્ણાંક vring_get_buf(સંરચના vring_virtqueue *vq, unsigned પૂર્ણાંક *len);
શૂન્ય vring_add_buf(સંરચના vring_virtqueue *vq, સંરચના vring_list list[],
                   unsigned પૂર્ણાંક out, unsigned પૂર્ણાંક in,
                   પૂર્ણાંક index, પૂર્ણાંક અંક_added);
શૂન્ય vring_kick(સંરચના vp_device *vp, સંરચના vring_virtqueue *vq, પૂર્ણાંક અંક_added);

#endif /* _VIRTIO_RING_H_ */

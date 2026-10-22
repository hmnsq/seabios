#ifndef _VIRTIO_RING_H
#define _VIRTIO_RING_H

#अन्तर्भाव "types.h" // u64
#अन्तर्भाव "memmap.h" // PAGE_SIZE

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

#define MAX_QUEUE_अंक      (256)

#define VRING_DESC_F_NEXT  1
#define VRING_DESC_F_WRITE 2

#define VRING_AVAIL_F_NO_INTERRUPT 1

#define VRING_USED_F_NO_NOTIFY     1

संरचना vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 next;
};

संरचना vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

संरचना vring_used_elem
{
   u32 id;
   u32 len;
};

संरचना vring_used
{
   u16 flags;
   u16 idx;
   संरचना vring_used_elem ring[];
};

संरचना vring {
   unsigned पूर्णाङ्क अंक;
   संरचना vring_desc *desc;
   संरचना vring_avail *avail;
   संरचना vring_used *used;
};

#define vring_size(अंक) \
    (ALIGN(परिमाण(संरचना vring_desc) * अंक + परिमाण(संरचना vring_avail) \
           + परिमाण(u16) * अंक, PAGE_SIZE)                              \
     + परिमाण(संरचना vring_used) + परिमाण(संरचना vring_used_elem) * अंक)

प्रकारनिर्देश unsigned वर्ण virtio_queue_t[vring_size(MAX_QUEUE_अंक)];

संरचना vring_virtqueue {
   virtio_queue_t queue;
   संरचना vring vring;
   u16 free_head;
   u16 last_used_idx;
   u16 vdata[MAX_QUEUE_अंक];
   /* PCI */
   पूर्णाङ्क queue_index;
   पूर्णाङ्क queue_notify_off;
};

संरचना vring_list {
  वर्ण *addr;
  unsigned पूर्णाङ्क length;
};

स्थिर अन्तःस्थ शून्य
vring_init(संरचना vring *vr, unsigned पूर्णाङ्क अंक, unsigned वर्ण *queue)
{
   ASSERT32FLAT();
   vr->अंक = अंक;

   /* physical address of desc must be page aligned */
   vr->desc = (शून्य*)ALIGN((u32)queue, PAGE_SIZE);

   vr->avail = (संरचना vring_avail *)&vr->desc[अंक];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_INTERRUPT;

   /* physical address of used must be page aligned */
   vr->used = (शून्य*)ALIGN((u32)&vr->avail->ring[अंक], PAGE_SIZE);

   पूर्णाङ्क i;
   प्रति (i = 0; i < अंक - 1; i++)
       vr->desc[i].next = i + 1;
   vr->desc[i].next = 0;
}

संरचना vp_device;
पूर्णाङ्क vring_more_used(संरचना vring_virtqueue *vq);
शून्य vring_detach(संरचना vring_virtqueue *vq, unsigned पूर्णाङ्क head);
पूर्णाङ्क vring_get_buf(संरचना vring_virtqueue *vq, unsigned पूर्णाङ्क *len);
शून्य vring_add_buf(संरचना vring_virtqueue *vq, संरचना vring_list list[],
                   unsigned पूर्णाङ्क out, unsigned पूर्णाङ्क in,
                   पूर्णाङ्क index, पूर्णाङ्क अंक_added);
शून्य vring_kick(संरचना vp_device *vp, संरचना vring_virtqueue *vq, पूर्णाङ्क अंक_added);

#endif /* _VIRTIO_RING_H_ */

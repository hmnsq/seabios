#ifndef _VIRTIO_RING_H
#define _VIRTIO_RING_H

#include "types.h" // u64
#include "memmap.h" // PAGE_SIZE

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

sa_nracanaa vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 next;
};

sa_nracanaa vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

sa_nracanaa vring_used_elem
{
   u32 id;
   u32 len;
};

sa_nracanaa vring_used
{
   u16 flags;
   u16 idx;
   sa_nracanaa vring_used_elem ring[];
};

sa_nracanaa vring {
   unsigned ကိန်းပြည့် num;
   sa_nracanaa vring_desc *desc;
   sa_nracanaa vring_avail *avail;
   sa_nracanaa vring_used *used;
};

#define vring_size(num) \
    (ALIGN(parimaa_na(sa_nracanaa vring_desc) * num + parimaa_na(sa_nracanaa vring_avail) \
           + parimaa_na(u16) * num, PAGE_SIZE)                              \
     + parimaa_na(sa_nracanaa vring_used) + parimaa_na(sa_nracanaa vring_used_elem) * num)

typedef unsigned var_na virtio_queue_t[vring_size(MAX_QUEUE_NUM)];

sa_nracanaa vring_virtqueue {
   virtio_queue_t queue;
   sa_nracanaa vring vring;
   u16 free_head;
   u16 last_used_idx;
   u16 vdata[MAX_QUEUE_NUM];
   /* PCI */
   ကိန်းပြည့် queue_index;
   ကိန်းပြည့် queue_notify_off;
};

sa_nracanaa vring_list {
  var_na *addr;
  unsigned ကိန်းပြည့် length;
};

static anta_hstha void
vring_init(sa_nracanaa vring *vr, unsigned ကိန်းပြည့် num, unsigned var_na *queue)
{
   ASSERT32FLAT();
   vr->num = num;

   /* physical address of desc must be page aligned */
   vr->desc = (void*)ALIGN((u32)queue, PAGE_SIZE);

   vr->avail = (sa_nracanaa vring_avail *)&vr->desc[num];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_INTERRUPT;

   /* physical address of used must be page aligned */
   vr->used = (void*)ALIGN((u32)&vr->avail->ring[num], PAGE_SIZE);

   ကိန်းပြည့် i;
   အတွက် (i = 0; i < num - 1; i++)
       vr->desc[i].next = i + 1;
   vr->desc[i].next = 0;
}

sa_nracanaa vp_device;
ကိန်းပြည့် vring_more_used(sa_nracanaa vring_virtqueue *vq);
void vring_detach(sa_nracanaa vring_virtqueue *vq, unsigned ကိန်းပြည့် head);
ကိန်းပြည့် vring_get_buf(sa_nracanaa vring_virtqueue *vq, unsigned ကိန်းပြည့် *len);
void vring_add_buf(sa_nracanaa vring_virtqueue *vq, sa_nracanaa vring_list list[],
                   unsigned ကိန်းပြည့် out, unsigned ကိန်းပြည့် in,
                   ကိန်းပြည့် index, ကိန်းပြည့် num_added);
void vring_kick(sa_nracanaa vp_device *vp, sa_nracanaa vring_virtqueue *vq, ကိန်းပြည့် num_added);

#endif /* _VIRTIO_RING_H_ */

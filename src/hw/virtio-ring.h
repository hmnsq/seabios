#ifndef _VIRTIO_RING_H
#define _VIRTIO_RING_H

#അന്തര്ഭാവ "types.h" // u64
#അന്തര്ഭാവ "memmap.h" // PAGE_SIZE

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

#define MAX_QUEUE_അക്കം      (256)

#define VRING_DESC_F_NEXT  1
#define VRING_DESC_F_WRITE 2

#define VRING_AVAIL_F_NO_INTERRUPT 1

#define VRING_USED_F_NO_NOTIFY     1

ഘടന vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 next;
};

ഘടന vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

ഘടന vring_used_elem
{
   u32 id;
   u32 len;
};

ഘടന vring_used
{
   u16 flags;
   u16 idx;
   ഘടന vring_used_elem ring[];
};

ഘടന vring {
   unsigned പൂർണ്ണസംഖ്യ അക്കം;
   ഘടന vring_desc *desc;
   ഘടന vring_avail *avail;
   ഘടന vring_used *used;
};

#define vring_size(അക്കം) \
    (ALIGN(വലുപ്പം(ഘടന vring_desc) * അക്കം + വലുപ്പം(ഘടന vring_avail) \
           + വലുപ്പം(u16) * അക്കം, PAGE_SIZE)                              \
     + വലുപ്പം(ഘടന vring_used) + വലുപ്പം(ഘടന vring_used_elem) * അക്കം)

തരനിർവചനം unsigned അക്ഷരം virtio_queue_t[vring_size(MAX_QUEUE_അക്കം)];

ഘടന vring_virtqueue {
   virtio_queue_t queue;
   ഘടന vring vring;
   u16 free_head;
   u16 last_used_idx;
   u16 vdata[MAX_QUEUE_അക്കം];
   /* PCI */
   പൂർണ്ണസംഖ്യ queue_index;
   പൂർണ്ണസംഖ്യ queue_notify_off;
};

ഘടന vring_list {
  അക്ഷരം *addr;
  unsigned പൂർണ്ണസംഖ്യ length;
};

സ്ഥിതം വരിയിൽ ശൂന്യം
vring_init(ഘടന vring *vr, unsigned പൂർണ്ണസംഖ്യ അക്കം, unsigned അക്ഷരം *queue)
{
   ASSERT32FLAT();
   vr->അക്കം = അക്കം;

   /* physical address of desc must be page aligned */
   vr->desc = (ശൂന്യം*)ALIGN((u32)queue, PAGE_SIZE);

   vr->avail = (ഘടന vring_avail *)&vr->desc[അക്കം];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_INTERRUPT;

   /* physical address of used must be page aligned */
   vr->used = (ശൂന്യം*)ALIGN((u32)&vr->avail->ring[അക്കം], PAGE_SIZE);

   പൂർണ്ണസംഖ്യ i;
   വേണ്ടി (i = 0; i < അക്കം - 1; i++)
       vr->desc[i].next = i + 1;
   vr->desc[i].next = 0;
}

ഘടന vp_device;
പൂർണ്ണസംഖ്യ vring_more_used(ഘടന vring_virtqueue *vq);
ശൂന്യം vring_detach(ഘടന vring_virtqueue *vq, unsigned പൂർണ്ണസംഖ്യ head);
പൂർണ്ണസംഖ്യ vring_get_buf(ഘടന vring_virtqueue *vq, unsigned പൂർണ്ണസംഖ്യ *len);
ശൂന്യം vring_add_buf(ഘടന vring_virtqueue *vq, ഘടന vring_list list[],
                   unsigned പൂർണ്ണസംഖ്യ out, unsigned പൂർണ്ണസംഖ്യ in,
                   പൂർണ്ണസംഖ്യ index, പൂർണ്ണസംഖ്യ അക്കം_added);
ശൂന്യം vring_kick(ഘടന vp_device *vp, ഘടന vring_virtqueue *vq, പൂർണ്ണസംഖ്യ അക്കം_added);

#endif /* _VIRTIO_RING_H_ */

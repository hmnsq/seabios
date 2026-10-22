#अगर_अघोषित _VIRTIO_RING_H
#घोषणा _VIRTIO_RING_H

#समावेश "types.h" // u64
#समावेश "memmap.h" // PAGE_SIZE

/* Status byte for guest to report progress, and synchronize features. */
/* We have seen device and processed generic fields (VIRTIO_CONFIG_F_VIRTIO) */
#घोषणा VIRTIO_CONFIG_S_ACKNOWLEDGE     1
/* We have found a driver for the device. */
#घोषणा VIRTIO_CONFIG_S_DRIVER          2
/* Driver has used its parts of the config, and is happy */
#घोषणा VIRTIO_CONFIG_S_DRIVER_OK       4
/* Driver has finished configuring features */
#घोषणा VIRTIO_CONFIG_S_FEATURES_OK     8
/* We've given up on this device. */
#घोषणा VIRTIO_CONFIG_S_FAILED          0x80

/* v1.0 compliant. */
#घोषणा VIRTIO_F_VERSION_1              32
#घोषणा VIRTIO_F_IOMMU_PLATFORM         33

#घोषणा MAX_QUEUE_अंक      (256)

#घोषणा VRING_DESC_F_अग्र  1
#घोषणा VRING_DESC_F_WRITE 2

#घोषणा VRING_AVAIL_F_NO_INTERRUPT 1

#घोषणा VRING_USED_F_NO_NOTIFY     1

काष्ठा vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 अग्र;
};

काष्ठा vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

काष्ठा vring_used_elem
{
   u32 id;
   u32 len;
};

काष्ठा vring_used
{
   u16 flags;
   u16 idx;
   काष्ठा vring_used_elem ring[];
};

काष्ठा vring {
   अचिन्हित पूर्णांक अंक;
   काष्ठा vring_desc *desc;
   काष्ठा vring_avail *avail;
   काष्ठा vring_used *used;
};

#घोषणा vring_आकार(अंक) \
    (ALIGN(माप(काष्ठा vring_desc) * अंक + माप(काष्ठा vring_avail) \
           + माप(u16) * अंक, PAGE_आकार)                              \
     + माप(काष्ठा vring_used) + माप(काष्ठा vring_used_elem) * अंक)

प्रकार अचिन्हित अक्षर virtio_queue_t[vring_आकार(MAX_QUEUE_अंक)];

काष्ठा vring_virtqueue {
   virtio_queue_t queue;
   काष्ठा vring vring;
   u16 free_शीर्ष;
   u16 last_used_idx;
   u16 vdata[MAX_QUEUE_अंक];
   /* PCI */
   पूर्णांक queue_सूचकांक;
   पूर्णांक queue_notify_off;
};

काष्ठा vring_list {
  अक्षर *addr;
  अचिन्हित पूर्णांक लंबाई;
};

अटल अंतरभूत व्योम
vring_init(काष्ठा vring *vr, अचिन्हित पूर्णांक अंक, अचिन्हित अक्षर *queue)
{
   ASSERT32FLAT();
   vr->अंक = अंक;

   /* physical address of desc must be page aligned */
   vr->desc = (व्योम*)ALIGN((u32)queue, PAGE_आकार);

   vr->avail = (काष्ठा vring_avail *)&vr->desc[अंक];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_INTERRUPT;

   /* physical address of used must be page aligned */
   vr->used = (व्योम*)ALIGN((u32)&vr->avail->ring[अंक], PAGE_आकार);

   पूर्णांक i;
   क्रम (i = 0; i < अंक - 1; i++)
       vr->desc[i].अग्र = i + 1;
   vr->desc[i].अग्र = 0;
}

काष्ठा vp_device;
पूर्णांक vring_more_used(काष्ठा vring_virtqueue *vq);
व्योम vring_detach(काष्ठा vring_virtqueue *vq, अचिन्हित पूर्णांक शीर्ष);
पूर्णांक vring_get_buf(काष्ठा vring_virtqueue *vq, अचिन्हित पूर्णांक *len);
व्योम vring_add_buf(काष्ठा vring_virtqueue *vq, काष्ठा vring_list list[],
                   अचिन्हित पूर्णांक out, अचिन्हित पूर्णांक in,
                   पूर्णांक सूचकांक, पूर्णांक अंक_added);
व्योम vring_kick(काष्ठा vp_device *vp, काष्ठा vring_virtqueue *vq, पूर्णांक अंक_added);

#पूर्ण_अगर /* _VIRTIO_RING_H_ */

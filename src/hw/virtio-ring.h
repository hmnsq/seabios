#که_نه_تعریف_شوی _VIRTIO_RING_H
#تعریف_کړه _VIRTIO_RING_H

#ضمن "types.h" // u64
#ضمن "memmap.h" // PAGE_SIZE

/* Status byte for guest to report progress, and synchronize features. */
/* We have seen device and processed generic fields (VIRTIO_CONFIG_F_VIRTIO) */
#تعریف_کړه VIRTIO_CONFIG_S_ACKNOWLEDGE     1
/* We have found a driver for the device. */
#تعریف_کړه VIRTIO_CONFIG_S_DRIVER          2
/* Driver has used its parts of the config, and is happy */
#تعریف_کړه VIRTIO_CONFIG_S_DRIVER_OK       4
/* Driver has finished configuring features */
#تعریف_کړه VIRTIO_CONFIG_S_FEATURES_OK     8
/* We've given up on this device. */
#تعریف_کړه VIRTIO_CONFIG_S_FAILED          0x80

/* v1.0 compliant. */
#تعریف_کړه VIRTIO_F_نسخه_1              32
#تعریف_کړه VIRTIO_F_IOMMU_PLATFORM         33

#تعریف_کړه MAX_کتار_عدد      (256)

#تعریف_کړه VRING_DESC_F_بل  1
#تعریف_کړه VRING_DESC_F_WRITE 2

#تعریف_کړه VRING_AVAIL_F_NO_مداخله 1

#تعریف_کړه VRING_کارول_شوی_F_NO_NOTIFY     1

بنية vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 بل;
};

بنية vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

بنية vring_کارول_شوی_elem
{
   u32 id;
   u32 len;
};

بنية vring_کارول_شوی
{
   u16 flags;
   u16 idx;
   بنية vring_کارول_شوی_elem ring[];
};

بنية vring {
   بینښه صحيح عدد;
   بنية vring_desc *desc;
   بنية vring_avail *avail;
   بنية vring_کارول_شوی *کارول_شوی;
};

#تعریف_کړه vring_اندازه(عدد) \
    (ALIGN(حجم(بنية vring_desc) * عدد + حجم(بنية vring_avail) \
           + حجم(u16) * عدد, پاڼه_اندازه)                              \
     + حجم(بنية vring_کارول_شوی) + حجم(بنية vring_کارول_شوی_elem) * عدد)

تعريفنوع بینښه حرف virtio_کتار_t[vring_اندازه(MAX_کتار_عدد)];

بنية vring_virtqueue {
   virtio_کتار_t کتار;
   بنية vring vring;
   u16 free_سر;
   u16 وروستی_کارول_شوی_idx;
   u16 vdata[MAX_کتار_عدد];
   /* PCI */
   صحيح کتار_شاخص;
   صحيح کتار_notify_off;
};

بنية vring_list {
  حرف *addr;
  بینښه صحيح اوږدتیا;
};

ساكن مضمن فراغ
vring_init(بنية vring *vr, بینښه صحيح عدد, بینښه حرف *کتار)
{
   ASSERT32FLAT();
   vr->عدد = عدد;

   /* physical address of desc must be page aligned */
   vr->desc = (فراغ*)ALIGN((u32)کتار, پاڼه_اندازه);

   vr->avail = (بنية vring_avail *)&vr->desc[عدد];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_مداخله;

   /* physical address of used must be page aligned */
   vr->کارول_شوی = (فراغ*)ALIGN((u32)&vr->avail->ring[عدد], پاڼه_اندازه);

   صحيح i;
   لكل (i = 0; i < عدد - 1; i++)
       vr->desc[i].بل = i + 1;
   vr->desc[i].بل = 0;
}

بنية vp_device;
صحيح vring_نور_کارول_شوی(بنية vring_virtqueue *vq);
فراغ vring_detach(بنية vring_virtqueue *vq, بینښه صحيح سر);
صحيح vring_get_buf(بنية vring_virtqueue *vq, بینښه صحيح *len);
فراغ vring_جمع_کړه_buf(بنية vring_virtqueue *vq, بنية vring_list list[],
                   بینښه صحيح بهر, بینښه صحيح دننه,
                   صحيح شاخص, صحيح عدد_added);
فراغ vring_kick(بنية vp_device *vp, بنية vring_virtqueue *vq, صحيح عدد_added);

#د_که_پای /* _VIRTIO_RING_H_ */

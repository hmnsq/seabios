#جيڪڏهن_وصف_نه_ٿيل _VIRTIO_RING_H
#وصف_ڪريو _VIRTIO_RING_H

#ضمن "types.h" // u64
#ضمن "memmap.h" // PAGE_SIZE

/* Status byte for guest to report progress, and synchronize features. */
/* We have seen device and processed generic fields (VIRTIO_CONFIG_F_VIRTIO) */
#وصف_ڪريو VIRTIO_CONFIG_S_ACKNOWLEDGE     1
/* We have found a driver for the device. */
#وصف_ڪريو VIRTIO_CONFIG_S_DRIVER          2
/* Driver has used its parts of the config, and is happy */
#وصف_ڪريو VIRTIO_CONFIG_S_DRIVER_OK       4
/* Driver has finished configuring features */
#وصف_ڪريو VIRTIO_CONFIG_S_FEATURES_OK     8
/* We've given up on this device. */
#وصف_ڪريو VIRTIO_CONFIG_S_FAILED          0x80

/* v1.0 compliant. */
#وصف_ڪريو VIRTIO_F_نسخو_1              32
#وصف_ڪريو VIRTIO_F_IOMMU_PLATFORM         33

#وصف_ڪريو MAX_قطار_انگ      (256)

#وصف_ڪريو VRING_DESC_F_ايندڙ  1
#وصف_ڪريو VRING_DESC_F_WRITE 2

#وصف_ڪريو VRING_AVAIL_F_NO_مداخلت 1

#وصف_ڪريو VRING_استعمال_ٿيل_F_NO_NOTIFY     1

بنية vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 ايندڙ;
};

بنية vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

بنية vring_استعمال_ٿيل_elem
{
   u32 id;
   u32 len;
};

بنية vring_استعمال_ٿيل
{
   u16 flags;
   u16 idx;
   بنية vring_استعمال_ٿيل_elem ring[];
};

بنية vring {
   بنانشان صحيح انگ;
   بنية vring_desc *desc;
   بنية vring_avail *avail;
   بنية vring_استعمال_ٿيل *استعمال_ٿيل;
};

#وصف_ڪريو vring_سائيز(انگ) \
    (ALIGN(حجم(بنية vring_desc) * انگ + حجم(بنية vring_avail) \
           + حجم(u16) * انگ, صفحو_سائيز)                              \
     + حجم(بنية vring_استعمال_ٿيل) + حجم(بنية vring_استعمال_ٿيل_elem) * انگ)

تعريفنوع بنانشان حرف virtio_قطار_t[vring_سائيز(MAX_قطار_انگ)];

بنية vring_virtqueue {
   virtio_قطار_t قطار;
   بنية vring vring;
   u16 free_سر;
   u16 آخري_استعمال_ٿيل_idx;
   u16 vdata[MAX_قطار_انگ];
   /* PCI */
   صحيح قطار_اشاريو;
   صحيح قطار_notify_off;
};

بنية vring_list {
  حرف *addr;
  بنانشان صحيح ڊيگهائي;
};

ساكن مضمن فراغ
vring_init(بنية vring *vr, بنانشان صحيح انگ, بنانشان حرف *قطار)
{
   ASSERT32FLAT();
   vr->انگ = انگ;

   /* physical address of desc must be page aligned */
   vr->desc = (فراغ*)ALIGN((u32)قطار, صفحو_سائيز);

   vr->avail = (بنية vring_avail *)&vr->desc[انگ];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_مداخلت;

   /* physical address of used must be page aligned */
   vr->استعمال_ٿيل = (فراغ*)ALIGN((u32)&vr->avail->ring[انگ], صفحو_سائيز);

   صحيح i;
   لكل (i = 0; i < انگ - 1; i++)
       vr->desc[i].ايندڙ = i + 1;
   vr->desc[i].ايندڙ = 0;
}

بنية vp_device;
صحيح vring_وڌيڪ_استعمال_ٿيل(بنية vring_virtqueue *vq);
فراغ vring_detach(بنية vring_virtqueue *vq, بنانشان صحيح سر);
صحيح vring_get_buf(بنية vring_virtqueue *vq, بنانشان صحيح *len);
فراغ vring_جوڙيو_buf(بنية vring_virtqueue *vq, بنية vring_list list[],
                   بنانشان صحيح ٻاهريون, بنانشان صحيح اندريون,
                   صحيح اشاريو, صحيح انگ_added);
فراغ vring_kick(بنية vp_device *vp, بنية vring_virtqueue *vq, صحيح انگ_added);

#جيڪڏهن_پورو /* _VIRTIO_RING_H_ */

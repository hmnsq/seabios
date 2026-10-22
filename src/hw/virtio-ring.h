#اگر_تعریف_نشده _VIRTIO_RING_H
#تعریف_کن _VIRTIO_RING_H

#ضمن "types.h" // u64
#ضمن "memmap.h" // PAGE_SIZE

/* Status byte for guest to report progress, and synchronize features. */
/* We have seen device and processed generic fields (VIRTIO_CONFIG_F_VIRTIO) */
#تعریف_کن VIRTIO_CONFIG_S_ACKNOWLEDGE     1
/* We have found a driver for the device. */
#تعریف_کن VIRTIO_CONFIG_S_DRIVER          2
/* Driver has used its parts of the config, and is happy */
#تعریف_کن VIRTIO_CONFIG_S_DRIVER_OK       4
/* Driver has finished configuring features */
#تعریف_کن VIRTIO_CONFIG_S_FEATURES_OK     8
/* We've given up on this device. */
#تعریف_کن VIRTIO_CONFIG_S_FAILED          0x80

/* v1.0 compliant. */
#تعریف_کن VIRTIO_F_نسخه_1              32
#تعریف_کن VIRTIO_F_IOMMU_PLATFORM         33

#تعریف_کن MAX_صف_رقم      (256)

#تعریف_کن VRING_DESC_F_بعدی  1
#تعریف_کن VRING_DESC_F_WRITE 2

#تعریف_کن VRING_AVAIL_F_NO_وقفه 1

#تعریف_کن VRING_استفاده_شده_F_NO_NOTIFY     1

بنية vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 بعدی;
};

بنية vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

بنية vring_استفاده_شده_elem
{
   u32 id;
   u32 len;
};

بنية vring_استفاده_شده
{
   u16 flags;
   u16 idx;
   بنية vring_استفاده_شده_elem ring[];
};

بنية vring {
   بیعلامت صحيح رقم;
   بنية vring_desc *desc;
   بنية vring_avail *avail;
   بنية vring_استفاده_شده *استفاده_شده;
};

#تعریف_کن vring_اندازه(رقم) \
    (ALIGN(حجم(بنية vring_desc) * رقم + حجم(بنية vring_avail) \
           + حجم(u16) * رقم, صفحه_اندازه)                              \
     + حجم(بنية vring_استفاده_شده) + حجم(بنية vring_استفاده_شده_elem) * رقم)

تعريفنوع بیعلامت حرف virtio_صف_t[vring_اندازه(MAX_صف_رقم)];

بنية vring_virtqueue {
   virtio_صف_t صف;
   بنية vring vring;
   u16 free_سر;
   u16 آخرین_استفاده_شده_idx;
   u16 vdata[MAX_صف_رقم];
   /* PCI */
   صحيح صف_شاخص;
   صحيح صف_notify_off;
};

بنية vring_list {
  حرف *addr;
  بیعلامت صحيح درازی;
};

ساكن مضمن فراغ
vring_init(بنية vring *vr, بیعلامت صحيح رقم, بیعلامت حرف *صف)
{
   ASSERT32FLAT();
   vr->رقم = رقم;

   /* physical address of desc must be page aligned */
   vr->desc = (فراغ*)ALIGN((u32)صف, صفحه_اندازه);

   vr->avail = (بنية vring_avail *)&vr->desc[رقم];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_وقفه;

   /* physical address of used must be page aligned */
   vr->استفاده_شده = (فراغ*)ALIGN((u32)&vr->avail->ring[رقم], صفحه_اندازه);

   صحيح i;
   لكل (i = 0; i < رقم - 1; i++)
       vr->desc[i].بعدی = i + 1;
   vr->desc[i].بعدی = 0;
}

بنية vp_device;
صحيح vring_بیشتر_استفاده_شده(بنية vring_virtqueue *vq);
فراغ vring_detach(بنية vring_virtqueue *vq, بیعلامت صحيح سر);
صحيح vring_get_buf(بنية vring_virtqueue *vq, بیعلامت صحيح *len);
فراغ vring_جمع_کن_buf(بنية vring_virtqueue *vq, بنية vring_list list[],
                   بیعلامت صحيح بیرون, بیعلامت صحيح درون,
                   صحيح شاخص, صحيح رقم_added);
فراغ vring_kick(بنية vp_device *vp, بنية vring_virtqueue *vq, صحيح رقم_added);

#پایان_اگر /* _VIRTIO_RING_H_ */

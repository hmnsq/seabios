#جے_بنا_تعریف _VIRTIO_RING_H
#تعریف_کرو _VIRTIO_RING_H

#ضمن "types.h" // u64
#ضمن "memmap.h" // PAGE_SIZE

/* Status byte for guest to report progress, and synchronize features. */
/* We have seen device and processed generic fields (VIRTIO_CONFIG_F_VIRTIO) */
#تعریف_کرو VIRTIO_CONFIG_S_ACKNOWLEDGE     1
/* We have found a driver for the device. */
#تعریف_کرو VIRTIO_CONFIG_S_DRIVER          2
/* Driver has used its parts of the config, and is happy */
#تعریف_کرو VIRTIO_CONFIG_S_DRIVER_OK       4
/* Driver has finished configuring features */
#تعریف_کرو VIRTIO_CONFIG_S_FEATURES_OK     8
/* We've given up on this device. */
#تعریف_کرو VIRTIO_CONFIG_S_FAILED          0x80

/* v1.0 compliant. */
#تعریف_کرو VIRTIO_F_نسخہ_1              32
#تعریف_کرو VIRTIO_F_IOMMU_PLATFORM         33

#تعریف_کرو MAX_قطار_عدد      (256)

#تعریف_کرو VRING_DESC_F_اگلا  1
#تعریف_کرو VRING_DESC_F_WRITE 2

#تعریف_کرو VRING_AVAIL_F_NO_رکاوٹ 1

#تعریف_کرو VRING_ورتیا_F_NO_NOTIFY     1

بنية vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 اگلا;
};

بنية vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

بنية vring_ورتیا_elem
{
   u32 id;
   u32 len;
};

بنية vring_ورتیا
{
   u16 flags;
   u16 idx;
   بنية vring_ورتیا_elem ring[];
};

بنية vring {
   بنانشانی صحيح عدد;
   بنية vring_desc *desc;
   بنية vring_avail *avail;
   بنية vring_ورتیا *ورتیا;
};

#تعریف_کرو vring_ناپ(عدد) \
    (ALIGN(حجم(بنية vring_desc) * عدد + حجم(بنية vring_avail) \
           + حجم(u16) * عدد, ورقہ_ناپ)                              \
     + حجم(بنية vring_ورتیا) + حجم(بنية vring_ورتیا_elem) * عدد)

تعريفنوع بنانشانی حرف virtio_قطار_t[vring_ناپ(MAX_قطار_عدد)];

بنية vring_virtqueue {
   virtio_قطار_t قطار;
   بنية vring vring;
   u16 free_سر;
   u16 آخری_ورتیا_idx;
   u16 vdata[MAX_قطار_عدد];
   /* PCI */
   صحيح قطار_اشاریہ;
   صحيح قطار_notify_off;
};

بنية vring_list {
  حرف *addr;
  بنانشانی صحيح لمائی;
};

ساكن مضمن فراغ
vring_init(بنية vring *vr, بنانشانی صحيح عدد, بنانشانی حرف *قطار)
{
   ASSERT32FLAT();
   vr->عدد = عدد;

   /* physical address of desc must be page aligned */
   vr->desc = (فراغ*)ALIGN((u32)قطار, ورقہ_ناپ);

   vr->avail = (بنية vring_avail *)&vr->desc[عدد];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_رکاوٹ;

   /* physical address of used must be page aligned */
   vr->ورتیا = (فراغ*)ALIGN((u32)&vr->avail->ring[عدد], ورقہ_ناپ);

   صحيح i;
   لكل (i = 0; i < عدد - 1; i++)
       vr->desc[i].اگلا = i + 1;
   vr->desc[i].اگلا = 0;
}

بنية vp_device;
صحيح vring_ہور_ورتیا(بنية vring_virtqueue *vq);
فراغ vring_detach(بنية vring_virtqueue *vq, بنانشانی صحيح سر);
صحيح vring_get_buf(بنية vring_virtqueue *vq, بنانشانی صحيح *len);
فراغ vring_جوڑو_buf(بنية vring_virtqueue *vq, بنية vring_list list[],
                   بنانشانی صحيح باہرلا, بنانشانی صحيح وچ,
                   صحيح اشاریہ, صحيح عدد_added);
فراغ vring_kick(بنية vp_device *vp, بنية vring_virtqueue *vq, صحيح عدد_added);

#جے_انت /* _VIRTIO_RING_H_ */

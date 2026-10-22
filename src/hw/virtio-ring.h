#اگر_غیر_تعریف_شدہ _VIRTIO_RING_H
#تعریف_کرو _VIRTIO_RING_H

#شامل "types.h" // u64
#شامل "memmap.h" // PAGE_SIZE

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

#تعریف_کرو VRING_AVAIL_F_NO_مداخلت 1

#تعریف_کرو VRING_مستعمل_F_NO_NOTIFY     1

ساخت vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 اگلا;
};

ساخت vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

ساخت vring_مستعمل_elem
{
   u32 id;
   u32 len;
};

ساخت vring_مستعمل
{
   u16 flags;
   u16 idx;
   ساخت vring_مستعمل_elem ring[];
};

ساخت vring {
   غیرعلامتی صحیح عدد;
   ساخت vring_desc *desc;
   ساخت vring_avail *avail;
   ساخت vring_مستعمل *مستعمل;
};

#تعریف_کرو vring_سائز(عدد) \
    (ALIGN(حجم(ساخت vring_desc) * عدد + حجم(ساخت vring_avail) \
           + حجم(u16) * عدد, صفحہ_سائز)                              \
     + حجم(ساخت vring_مستعمل) + حجم(ساخت vring_مستعمل_elem) * عدد)

قسمتعریف غیرعلامتی حرف virtio_قطار_t[vring_سائز(MAX_قطار_عدد)];

ساخت vring_virtqueue {
   virtio_قطار_t قطار;
   ساخت vring vring;
   u16 free_سر;
   u16 آخری_مستعمل_idx;
   u16 vdata[MAX_قطار_عدد];
   /* PCI */
   صحیح قطار_اشاریہ;
   صحیح قطار_notify_off;
};

ساخت vring_list {
  حرف *addr;
  غیرعلامتی صحیح لمبائی;
};

ساکن اندرونی خالی
vring_init(ساخت vring *vr, غیرعلامتی صحیح عدد, غیرعلامتی حرف *قطار)
{
   ASSERT32FLAT();
   vr->عدد = عدد;

   /* physical address of desc must be page aligned */
   vr->desc = (خالی*)ALIGN((u32)قطار, صفحہ_سائز);

   vr->avail = (ساخت vring_avail *)&vr->desc[عدد];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_مداخلت;

   /* physical address of used must be page aligned */
   vr->مستعمل = (خالی*)ALIGN((u32)&vr->avail->ring[عدد], صفحہ_سائز);

   صحیح i;
   برائے (i = 0; i < عدد - 1; i++)
       vr->desc[i].اگلا = i + 1;
   vr->desc[i].اگلا = 0;
}

ساخت vp_device;
صحیح vring_مزید_مستعمل(ساخت vring_virtqueue *vq);
خالی vring_detach(ساخت vring_virtqueue *vq, غیرعلامتی صحیح سر);
صحیح vring_get_buf(ساخت vring_virtqueue *vq, غیرعلامتی صحیح *len);
خالی vring_جوڑو_buf(ساخت vring_virtqueue *vq, ساخت vring_list list[],
                   غیرعلامتی صحیح برآمد, غیرعلامتی صحیح اندر,
                   صحیح اشاریہ, صحیح عدد_added);
خالی vring_kick(ساخت vp_device *vp, ساخت vring_virtqueue *vq, صحیح عدد_added);

#اگر_ختم /* _VIRTIO_RING_H_ */

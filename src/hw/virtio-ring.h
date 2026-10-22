#నిర్వచితం_కాకపోతే _VIRTIO_RING_H
#నిర్వచించు _VIRTIO_RING_H

#అన్తర్భావ "types.h" // u64
#అన్తర్భావ "memmap.h" // PAGE_SIZE

/* Status byte for guest to report progress, and synchronize features. */
/* We have seen device and processed generic fields (VIRTIO_CONFIG_F_VIRTIO) */
#నిర్వచించు VIRTIO_CONFIG_S_ACKNOWLEDGE     1
/* We have found a driver for the device. */
#నిర్వచించు VIRTIO_CONFIG_S_DRIVER          2
/* Driver has used its parts of the config, and is happy */
#నిర్వచించు VIRTIO_CONFIG_S_DRIVER_OK       4
/* Driver has finished configuring features */
#నిర్వచించు VIRTIO_CONFIG_S_FEATURES_OK     8
/* We've given up on this device. */
#నిర్వచించు VIRTIO_CONFIG_S_FAILED          0x80

/* v1.0 compliant. */
#నిర్వచించు VIRTIO_F_సంస్కరణ_1              32
#నిర్వచించు VIRTIO_F_IOMMU_PLATFORM         33

#నిర్వచించు MAX_వరుస_అంకె      (256)

#నిర్వచించు VRING_DESC_F_తదుపరి  1
#నిర్వచించు VRING_DESC_F_WRITE 2

#నిర్వచించు VRING_AVAIL_F_NO_అంతరాయం 1

#నిర్వచించు VRING_వాడిన_F_NO_NOTIFY     1

నిర్మాణం vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 తదుపరి;
};

నిర్మాణం vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

నిర్మాణం vring_వాడిన_elem
{
   u32 id;
   u32 len;
};

నిర్మాణం vring_వాడిన
{
   u16 flags;
   u16 idx;
   నిర్మాణం vring_వాడిన_elem ring[];
};

నిర్మాణం vring {
   గుర్తులేని పూర్ణాంకం అంకె;
   నిర్మాణం vring_desc *desc;
   నిర్మాణం vring_avail *avail;
   నిర్మాణం vring_వాడిన *వాడిన;
};

#నిర్వచించు vring_కొలత(అంకె) \
    (ALIGN(పరిమాణం(నిర్మాణం vring_desc) * అంకె + పరిమాణం(నిర్మాణం vring_avail) \
           + పరిమాణం(u16) * అంకె, పుట_కొలత)                              \
     + పరిమాణం(నిర్మాణం vring_వాడిన) + పరిమాణం(నిర్మాణం vring_వాడిన_elem) * అంకె)

రకనిర్వచనం గుర్తులేని అక్షరం virtio_వరుస_t[vring_కొలత(MAX_వరుస_అంకె)];

నిర్మాణం vring_virtqueue {
   virtio_వరుస_t వరుస;
   నిర్మాణం vring vring;
   u16 free_తల;
   u16 చివరి_వాడిన_idx;
   u16 vdata[MAX_వరుస_అంకె];
   /* PCI */
   పూర్ణాంకం వరుస_సూచీ;
   పూర్ణాంకం వరుస_notify_off;
};

నిర్మాణం vring_list {
  అక్షరం *addr;
  గుర్తులేని పూర్ణాంకం నిడివి;
};

స్థిర అంతఃవరుస శూన్య
vring_init(నిర్మాణం vring *vr, గుర్తులేని పూర్ణాంకం అంకె, గుర్తులేని అక్షరం *వరుస)
{
   ASSERT32FLAT();
   vr->అంకె = అంకె;

   /* physical address of desc must be page aligned */
   vr->desc = (శూన్య*)ALIGN((u32)వరుస, పుట_కొలత);

   vr->avail = (నిర్మాణం vring_avail *)&vr->desc[అంకె];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_అంతరాయం;

   /* physical address of used must be page aligned */
   vr->వాడిన = (శూన్య*)ALIGN((u32)&vr->avail->ring[అంకె], పుట_కొలత);

   పూర్ణాంకం i;
   కొరకు (i = 0; i < అంకె - 1; i++)
       vr->desc[i].తదుపరి = i + 1;
   vr->desc[i].తదుపరి = 0;
}

నిర్మాణం vp_device;
పూర్ణాంకం vring_మరిన్ని_వాడిన(నిర్మాణం vring_virtqueue *vq);
శూన్య vring_detach(నిర్మాణం vring_virtqueue *vq, గుర్తులేని పూర్ణాంకం తల);
పూర్ణాంకం vring_get_buf(నిర్మాణం vring_virtqueue *vq, గుర్తులేని పూర్ణాంకం *len);
శూన్య vring_కూడు_buf(నిర్మాణం vring_virtqueue *vq, నిర్మాణం vring_list list[],
                   గుర్తులేని పూర్ణాంకం బహిర్గతం, గుర్తులేని పూర్ణాంకం లోపల,
                   పూర్ణాంకం సూచీ, పూర్ణాంకం అంకె_added);
శూన్య vring_kick(నిర్మాణం vp_device *vp, నిర్మాణం vring_virtqueue *vq, పూర్ణాంకం అంకె_added);

#అయితే_ముగింపు /* _VIRTIO_RING_H_ */

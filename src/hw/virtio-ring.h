#यदि_अपरिभाषितम् _VIRTIO_RING_H
#परिभाषय _VIRTIO_RING_H

#अन्तर्भाव "types.h" // u64
#अन्तर्भाव "memmap.h" // PAGE_SIZE

/* Status byte for guest to report progress, and synchronize features. */
/* We have seen device and processed generic fields (VIRTIO_CONFIG_F_VIRTIO) */
#परिभाषय VIRTIO_CONFIG_S_ACKNOWLEDGE     1
/* We have found a driver for the device. */
#परिभाषय VIRTIO_CONFIG_S_DRIVER          2
/* Driver has used its parts of the config, and is happy */
#परिभाषय VIRTIO_CONFIG_S_DRIVER_OK       4
/* Driver has finished configuring features */
#परिभाषय VIRTIO_CONFIG_S_FEATURES_OK     8
/* We've given up on this device. */
#परिभाषय VIRTIO_CONFIG_S_FAILED          0x80

/* v1.0 compliant. */
#परिभाषय VIRTIO_F_संस्करणम्_1              32
#परिभाषय VIRTIO_F_IOMMU_PLATFORM         33

#परिभाषय MAX_QUEUE_अङ्कः      (256)

#परिभाषय VRING_DESC_F_अग्र  1
#परिभाषय VRING_DESC_F_WRITE 2

#परिभाषय VRING_AVAIL_F_NO_व्यवधानम् 1

#परिभाषय VRING_प्रयुक्तम्_F_NO_NOTIFY     1

संरचना vring_desc
{
   u64 addr;
   u32 len;
   u16 flags;
   u16 अग्र;
};

संरचना vring_avail
{
   u16 flags;
   u16 idx;
   u16 ring[0];
};

संरचना vring_प्रयुक्तम्_elem
{
   u32 id;
   u32 len;
};

संरचना vring_प्रयुक्तम्
{
   u16 flags;
   u16 idx;
   संरचना vring_प्रयुक्तम्_elem ring[];
};

संरचना vring {
   अचिह्नित पूर्णाङ्क अङ्कः;
   संरचना vring_desc *desc;
   संरचना vring_avail *avail;
   संरचना vring_प्रयुक्तम् *प्रयुक्तम्;
};

#परिभाषय vring_आकार(अङ्कः) \
    (ALIGN(परिमाण(संरचना vring_desc) * अङ्कः + परिमाण(संरचना vring_avail) \
           + परिमाण(u16) * अङ्कः, PAGE_आकार)                              \
     + परिमाण(संरचना vring_प्रयुक्तम्) + परिमाण(संरचना vring_प्रयुक्तम्_elem) * अङ्कः)

प्रकारनिर्देश अचिह्नित वर्ण virtio_queue_t[vring_आकार(MAX_QUEUE_अङ्कः)];

संरचना vring_virtqueue {
   virtio_queue_t queue;
   संरचना vring vring;
   u16 free_शीर्ष;
   u16 चरमः_प्रयुक्तम्_idx;
   u16 vdata[MAX_QUEUE_अङ्कः];
   /* PCI */
   पूर्णाङ्क queue_सूचकांक;
   पूर्णाङ्क queue_notify_off;
};

संरचना vring_list {
  वर्ण *addr;
  अचिह्नित पूर्णाङ्क दीर्घता;
};

स्थिर अन्तःस्थ शून्य
vring_init(संरचना vring *vr, अचिह्नित पूर्णाङ्क अङ्कः, अचिह्नित वर्ण *queue)
{
   ASSERT32FLAT();
   vr->अङ्कः = अङ्कः;

   /* physical address of desc must be page aligned */
   vr->desc = (शून्य*)ALIGN((u32)queue, PAGE_आकार);

   vr->avail = (संरचना vring_avail *)&vr->desc[अङ्कः];
   /* disable interrupts */
   vr->avail->flags |= VRING_AVAIL_F_NO_व्यवधानम्;

   /* physical address of used must be page aligned */
   vr->प्रयुक्तम् = (शून्य*)ALIGN((u32)&vr->avail->ring[अङ्कः], PAGE_आकार);

   पूर्णाङ्क i;
   प्रति (i = 0; i < अङ्कः - 1; i++)
       vr->desc[i].अग्र = i + 1;
   vr->desc[i].अग्र = 0;
}

संरचना vp_device;
पूर्णाङ्क vring_अधिकम्_प्रयुक्तम्(संरचना vring_virtqueue *vq);
शून्य vring_detach(संरचना vring_virtqueue *vq, अचिह्नित पूर्णाङ्क शीर्ष);
पूर्णाङ्क vring_get_buf(संरचना vring_virtqueue *vq, अचिह्नित पूर्णाङ्क *len);
शून्य vring_सङ्कलय_buf(संरचना vring_virtqueue *vq, संरचना vring_list list[],
                   अचिह्नित पूर्णाङ्क बहिर्गमनम्, अचिह्नित पूर्णाङ्क अन्तर्,
                   पूर्णाङ्क सूचकांक, पूर्णाङ्क अङ्कः_added);
शून्य vring_kick(संरचना vp_device *vp, संरचना vring_virtqueue *vq, पूर्णाङ्क अङ्कः_added);

#यदि_अन्तः /* _VIRTIO_RING_H_ */

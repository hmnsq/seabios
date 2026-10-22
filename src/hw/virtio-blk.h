#यदि_अपरिभाषितम् _VIRTIO_BLK_H
#परिभाषय _VIRTIO_BLK_H

संरचना virtio_blk_config
{
    u64 क्षमता;
    u32 आकार_max;
    u32 seg_max;
    u16 cylinders;
    u8 heads;
    u8 sectors;
    u32 blk_आकार;
    u8 physical_खण्डम्_exp;
    u8 alignment_विस्थापनम्;
    u16 min_io_आकार;
    u32 opt_io_आकार;
} __attribute__((packed));

/* Feature bits */
#परिभाषय VIRTIO_BLK_F_आकार_MAX 1  /* Maximum size of any single segment */
#परिभाषय VIRTIO_BLK_F_SEG_MAX 2   /* Maximum number of segments in a request */
#परिभाषय VIRTIO_BLK_F_BLK_आकार 6

/* These two define direction. */
#परिभाषय VIRTIO_BLK_T_अन्तर्         0
#परिभाषय VIRTIO_BLK_T_बहिर्गमनम्        1

/* This is the first element of the read scatter-gather list. */
संरचना virtio_blk_outhdr {
    /* VIRTIO_BLK_T* */
    u32 type;
    /* io priority. */
    u32 ioprio;
    /* Sector (ie. 512 byte offset) */
    u64 sector;
};

#परिभाषय VIRTIO_BLK_S_OK         0
#परिभाषय VIRTIO_BLK_S_IOERR      1
#परिभाषय VIRTIO_BLK_S_UNSUPP     2

संरचना disk_op_s;
पूर्णाङ्क virtio_blk_process_op(संरचना disk_op_s *op);
शून्य virtio_blk_सज्जता(शून्य);
शून्य init_virtio_blk_mmio(शून्य *mmio);

#यदि_अन्तः /* _VIRTIO_BLK_H */

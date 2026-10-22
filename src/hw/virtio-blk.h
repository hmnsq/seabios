#جے_بنا_تعریف _VIRTIO_BLK_H
#تعریف_کرو _VIRTIO_BLK_H

بنية virtio_blk_config
{
    u64 سمرتھا;
    u32 ناپ_max;
    u32 seg_max;
    u16 cylinders;
    u8 heads;
    u8 sectors;
    u32 blk_ناپ;
    u8 physical_بلاک_exp;
    u8 alignment_وتھ;
    u16 min_io_ناپ;
    u32 opt_io_ناپ;
} __attribute__((packed));

/* Feature bits */
#تعریف_کرو VIRTIO_BLK_F_ناپ_MAX 1  /* Maximum size of any single segment */
#تعریف_کرو VIRTIO_BLK_F_SEG_MAX 2   /* Maximum number of segments in a request */
#تعریف_کرو VIRTIO_BLK_F_BLK_ناپ 6

/* These two define direction. */
#تعریف_کرو VIRTIO_BLK_T_وچ         0
#تعریف_کرو VIRTIO_BLK_T_باہرلا        1

/* This is the first element of the read scatter-gather list. */
بنية virtio_blk_outhdr {
    /* VIRTIO_BLK_T* */
    u32 type;
    /* io priority. */
    u32 ioprio;
    /* Sector (ie. 512 byte offset) */
    u64 sector;
};

#تعریف_کرو VIRTIO_BLK_S_OK         0
#تعریف_کرو VIRTIO_BLK_S_IOERR      1
#تعریف_کرو VIRTIO_BLK_S_UNSUPP     2

بنية disk_op_s;
صحيح virtio_blk_process_op(بنية disk_op_s *op);
فراغ virtio_blk_تیاری(فراغ);
فراغ init_virtio_blk_mmio(فراغ *mmio);

#جے_انت /* _VIRTIO_BLK_H */

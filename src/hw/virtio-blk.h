#اگر_تعریف_نشده _VIRTIO_BLK_H
#تعریف_کن _VIRTIO_BLK_H

بنية virtio_blk_config
{
    u64 ظرفیت;
    u32 اندازه_max;
    u32 seg_max;
    u16 cylinders;
    u8 heads;
    u8 sectors;
    u32 blk_اندازه;
    u8 physical_بلوک_exp;
    u8 alignment_جابه_جایی;
    u16 min_io_اندازه;
    u32 opt_io_اندازه;
} __attribute__((packed));

/* Feature bits */
#تعریف_کن VIRTIO_BLK_F_اندازه_MAX 1  /* Maximum size of any single segment */
#تعریف_کن VIRTIO_BLK_F_SEG_MAX 2   /* Maximum number of segments in a request */
#تعریف_کن VIRTIO_BLK_F_BLK_اندازه 6

/* These two define direction. */
#تعریف_کن VIRTIO_BLK_T_درون         0
#تعریف_کن VIRTIO_BLK_T_بیرون        1

/* This is the first element of the read scatter-gather list. */
بنية virtio_blk_outhdr {
    /* VIRTIO_BLK_T* */
    u32 type;
    /* io priority. */
    u32 ioprio;
    /* Sector (ie. 512 byte offset) */
    u64 sector;
};

#تعریف_کن VIRTIO_BLK_S_OK         0
#تعریف_کن VIRTIO_BLK_S_IOERR      1
#تعریف_کن VIRTIO_BLK_S_UNSUPP     2

بنية disk_op_s;
صحيح virtio_blk_process_op(بنية disk_op_s *op);
فراغ virtio_blk_راهاندازی(فراغ);
فراغ init_virtio_blk_mmio(فراغ *mmio);

#پایان_اگر /* _VIRTIO_BLK_H */

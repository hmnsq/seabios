#جيڪڏهن_وصف_نه_ٿيل _VIRTIO_BLK_H
#وصف_ڪريو _VIRTIO_BLK_H

بنية virtio_blk_config
{
    u64 صلاحيت;
    u32 سائيز_max;
    u32 seg_max;
    u16 cylinders;
    u8 heads;
    u8 sectors;
    u32 blk_سائيز;
    u8 physical_بلاڪ_exp;
    u8 alignment_وٿي;
    u16 min_io_سائيز;
    u32 opt_io_سائيز;
} __attribute__((packed));

/* Feature bits */
#وصف_ڪريو VIRTIO_BLK_F_سائيز_MAX 1  /* Maximum size of any single segment */
#وصف_ڪريو VIRTIO_BLK_F_SEG_MAX 2   /* Maximum number of segments in a request */
#وصف_ڪريو VIRTIO_BLK_F_BLK_سائيز 6

/* These two define direction. */
#وصف_ڪريو VIRTIO_BLK_T_اندريون         0
#وصف_ڪريو VIRTIO_BLK_T_ٻاهريون        1

/* This is the first element of the read scatter-gather list. */
بنية virtio_blk_outhdr {
    /* VIRTIO_BLK_T* */
    u32 type;
    /* io priority. */
    u32 ioprio;
    /* Sector (ie. 512 byte offset) */
    u64 sector;
};

#وصف_ڪريو VIRTIO_BLK_S_OK         0
#وصف_ڪريو VIRTIO_BLK_S_IOERR      1
#وصف_ڪريو VIRTIO_BLK_S_UNSUPP     2

بنية disk_op_s;
صحيح virtio_blk_process_op(بنية disk_op_s *op);
فراغ virtio_blk_تياري(فراغ);
فراغ init_virtio_blk_mmio(فراغ *mmio);

#جيڪڏهن_پورو /* _VIRTIO_BLK_H */

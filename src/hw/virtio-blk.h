#اگر_غیر_تعریف_شدہ _VIRTIO_BLK_H
#تعریف_کرو _VIRTIO_BLK_H

ساخت virtio_blk_config
{
    u64 استعداد;
    u32 سائز_max;
    u32 seg_max;
    u16 cylinders;
    u8 heads;
    u8 sectors;
    u32 blk_سائز;
    u8 physical_بلاک_exp;
    u8 alignment_انحراف;
    u16 min_io_سائز;
    u32 opt_io_سائز;
} __attribute__((packed));

/* Feature bits */
#تعریف_کرو VIRTIO_BLK_F_سائز_MAX 1  /* Maximum size of any single segment */
#تعریف_کرو VIRTIO_BLK_F_SEG_MAX 2   /* Maximum number of segments in a request */
#تعریف_کرو VIRTIO_BLK_F_BLK_سائز 6

/* These two define direction. */
#تعریف_کرو VIRTIO_BLK_T_اندر         0
#تعریف_کرو VIRTIO_BLK_T_برآمد        1

/* This is the first element of the read scatter-gather list. */
ساخت virtio_blk_outhdr {
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

ساخت disk_op_s;
صحیح virtio_blk_process_op(ساخت disk_op_s *op);
خالی virtio_blk_تیاری(خالی);
خالی init_virtio_blk_mmio(خالی *mmio);

#اگر_ختم /* _VIRTIO_BLK_H */

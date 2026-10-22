#ifndef _VIRTIO_BLK_H
#define _VIRTIO_BLK_H

ساختار virtio_blk_config
{
    u64 capacity;
    u32 size_max;
    u32 seg_max;
    u16 cylinders;
    u8 heads;
    u8 sectors;
    u32 blk_size;
    u8 physical_block_exp;
    u8 alignment_offset;
    u16 min_io_size;
    u32 opt_io_size;
} __attribute__((packed));

/* Feature bits */
#define VIRTIO_BLK_F_SIZE_MAX 1  /* Maximum size of any single segment */
#define VIRTIO_BLK_F_SEG_MAX 2   /* Maximum number of segments in a request */
#define VIRTIO_BLK_F_BLK_SIZE 6

/* These two define direction. */
#define VIRTIO_BLK_T_IN         0
#define VIRTIO_BLK_T_OUT        1

/* This is the first element of the read scatter-gather list. */
ساختار virtio_blk_outhdr {
    /* VIRTIO_BLK_T* */
    u32 type;
    /* io priority. */
    u32 ioprio;
    /* Sector (ie. 512 byte offset) */
    u64 sector;
};

#define VIRTIO_BLK_S_OK         0
#define VIRTIO_BLK_S_IOERR      1
#define VIRTIO_BLK_S_UNSUPP     2

ساختار disk_op_s;
صحیح virtio_blk_process_op(ساختار disk_op_s *op);
تهی virtio_blk_setup(تهی);
تهی init_virtio_blk_mmio(تهی *mmio);

#endif /* _VIRTIO_BLK_H */

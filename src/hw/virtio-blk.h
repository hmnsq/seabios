#अगर_अघोषित _VIRTIO_BLK_H
#घोषणा _VIRTIO_BLK_H

काष्ठा virtio_blk_config
{
    u64 capacity;
    u32 आकार_max;
    u32 seg_max;
    u16 cylinders;
    u8 heads;
    u8 sectors;
    u32 blk_आकार;
    u8 physical_block_exp;
    u8 alignment_offset;
    u16 min_io_आकार;
    u32 opt_io_आकार;
} __attribute__((packed));

/* Feature bits */
#घोषणा VIRTIO_BLK_F_आकार_MAX 1  /* Maximum size of any single segment */
#घोषणा VIRTIO_BLK_F_SEG_MAX 2   /* Maximum number of segments in a request */
#घोषणा VIRTIO_BLK_F_BLK_आकार 6

/* These two define direction. */
#घोषणा VIRTIO_BLK_T_IN         0
#घोषणा VIRTIO_BLK_T_OUT        1

/* This is the first element of the read scatter-gather list. */
काष्ठा virtio_blk_outhdr {
    /* VIRTIO_BLK_T* */
    u32 type;
    /* io priority. */
    u32 ioprio;
    /* Sector (ie. 512 byte offset) */
    u64 sector;
};

#घोषणा VIRTIO_BLK_S_OK         0
#घोषणा VIRTIO_BLK_S_IOERR      1
#घोषणा VIRTIO_BLK_S_UNSUPP     2

काष्ठा disk_op_s;
पूर्णांक virtio_blk_process_op(काष्ठा disk_op_s *op);
व्योम virtio_blk_setup(व्योम);
व्योम init_virtio_blk_mmio(व्योम *mmio);

#पूर्ण_अगर /* _VIRTIO_BLK_H */

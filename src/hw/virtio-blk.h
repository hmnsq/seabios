#నిర్వచితం_కాకపోతే _VIRTIO_BLK_H
#నిర్వచించు _VIRTIO_BLK_H

నిర్మాణం virtio_blk_config
{
    u64 సామర్థ్యం;
    u32 కొలత_max;
    u32 seg_max;
    u16 cylinders;
    u8 heads;
    u8 sectors;
    u32 blk_కొలత;
    u8 physical_ఖండం_exp;
    u8 alignment_స్థానభ్రంశం;
    u16 min_io_కొలత;
    u32 opt_io_కొలత;
} __attribute__((packed));

/* Feature bits */
#నిర్వచించు VIRTIO_BLK_F_కొలత_MAX 1  /* Maximum size of any single segment */
#నిర్వచించు VIRTIO_BLK_F_SEG_MAX 2   /* Maximum number of segments in a request */
#నిర్వచించు VIRTIO_BLK_F_BLK_కొలత 6

/* These two define direction. */
#నిర్వచించు VIRTIO_BLK_T_లోపల         0
#నిర్వచించు VIRTIO_BLK_T_బహిర్గతం        1

/* This is the first element of the read scatter-gather list. */
నిర్మాణం virtio_blk_outhdr {
    /* VIRTIO_BLK_T* */
    u32 type;
    /* io priority. */
    u32 ioprio;
    /* Sector (ie. 512 byte offset) */
    u64 sector;
};

#నిర్వచించు VIRTIO_BLK_S_OK         0
#నిర్వచించు VIRTIO_BLK_S_IOERR      1
#నిర్వచించు VIRTIO_BLK_S_UNSUPP     2

నిర్మాణం disk_op_s;
పూర్ణాంకం virtio_blk_process_op(నిర్మాణం disk_op_s *op);
శూన్య virtio_blk_అమరిక(శూన్య);
శూన్య init_virtio_blk_mmio(శూన్య *mmio);

#అయితే_ముగింపు /* _VIRTIO_BLK_H */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _VIRTIO_BLK_H
#ವ್ಯಾಖ್ಯಾನಿಸು _VIRTIO_BLK_H

ರಚನೆ virtio_blk_config
{
    u64 ಸಾಮರ್ಥ್ಯ;
    u32 ಗಾತ್ರ_max;
    u32 seg_max;
    u16 cylinders;
    u8 heads;
    u8 sectors;
    u32 blk_ಗಾತ್ರ;
    u8 physical_ಖಂಡ_exp;
    u8 alignment_ಸ್ಥಾನಾಂತರ;
    u16 min_io_ಗಾತ್ರ;
    u32 opt_io_ಗಾತ್ರ;
} __attribute__((packed));

/* Feature bits */
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_BLK_F_ಗಾತ್ರ_MAX 1  /* Maximum size of any single segment */
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_BLK_F_SEG_MAX 2   /* Maximum number of segments in a request */
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_BLK_F_BLK_ಗಾತ್ರ 6

/* These two define direction. */
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_BLK_T_ಒಳ         0
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_BLK_T_ಹೊರಹೋಗು        1

/* This is the first element of the read scatter-gather list. */
ರಚನೆ virtio_blk_outhdr {
    /* VIRTIO_BLK_T* */
    u32 type;
    /* io priority. */
    u32 ioprio;
    /* Sector (ie. 512 byte offset) */
    u64 sector;
};

#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_BLK_S_OK         0
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_BLK_S_IOERR      1
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_BLK_S_UNSUPP     2

ರಚನೆ disk_op_s;
ಪೂರ್ಣಾಂಕ virtio_blk_process_op(ರಚನೆ disk_op_s *op);
ಶೂನ್ಯ virtio_blk_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ init_virtio_blk_mmio(ಶೂನ್ಯ *mmio);

#ಆದರೆ_ಅಂತ್ಯ /* _VIRTIO_BLK_H */

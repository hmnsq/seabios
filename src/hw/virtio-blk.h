#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _VIRTIO_BLK_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _VIRTIO_BLK_H

ਬਣਤਰ virtio_blk_config
{
    u64 ਸਮਰੱਥਾ;
    u32 ਮਾਪ_max;
    u32 seg_max;
    u16 cylinders;
    u8 heads;
    u8 sectors;
    u32 blk_ਮਾਪ;
    u8 physical_ਖੰਡ_exp;
    u8 alignment_ਵਿੱਥ;
    u16 min_io_ਮਾਪ;
    u32 opt_io_ਮਾਪ;
} __attribute__((packed));

/* Feature bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_BLK_F_ਮਾਪ_MAX 1  /* Maximum size of any single segment */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_BLK_F_SEG_MAX 2   /* Maximum number of segments in a request */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_BLK_F_BLK_ਮਾਪ 6

/* These two define direction. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_BLK_T_ਵਿੱਚ         0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_BLK_T_ਬਾਹਰਲਾ        1

/* This is the first element of the read scatter-gather list. */
ਬਣਤਰ virtio_blk_outhdr {
    /* VIRTIO_BLK_T* */
    u32 type;
    /* io priority. */
    u32 ioprio;
    /* Sector (ie. 512 byte offset) */
    u64 sector;
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_BLK_S_OK         0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_BLK_S_IOERR      1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_BLK_S_UNSUPP     2

ਬਣਤਰ disk_op_s;
ਪੂਰਨ virtio_blk_process_op(ਬਣਤਰ disk_op_s *op);
ਖਾਲੀ virtio_blk_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ init_virtio_blk_mmio(ਖਾਲੀ *mmio);

#ਜੇ_ਅੰਤ /* _VIRTIO_BLK_H */

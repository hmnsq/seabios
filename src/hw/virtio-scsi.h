#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _VIRTIO_SCSI_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _VIRTIO_SCSI_H

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_SCSI_CDB_ਮਾਪ      32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_SCSI_SENSE_ਮਾਪ    96

ਬਣਤਰ virtio_scsi_config
{
    u32 ਅੰਕ_queues;
    u32 seg_max;
    u32 max_sectors;
    u32 cmd_ਪ੍ਰਤੀ_lun;
    u32 event_info_ਮਾਪ;
    u32 sense_ਮਾਪ;
    u32 cdb_ਮਾਪ;
    u16 max_channel;
    u16 max_ਨਿਸ਼ਾਨਾ;
    u32 max_lun;
} __attribute__((packed));

/* This is the first element of the "out" scatter-gather list. */
ਬਣਤਰ virtio_scsi_req_cmd {
    u8 lun[8];
    u64 id;
    u8 task_attr;
    u8 prio;
    u8 crn;
    ਅੱਖਰ cdb[VIRTIO_SCSI_CDB_ਮਾਪ];
} __attribute__((packed));

/* This is the first element of the "in" scatter-gather list. */
ਬਣਤਰ virtio_scsi_resp_cmd {
    u32 sense_len;
    u32 residual;
    u16 ਸਥਿਤੀ_qualifier;
    u8 ਸਥਿਤੀ;
    u8 response;
    u8 sense[VIRTIO_SCSI_SENSE_ਮਾਪ];
} __attribute__((packed));

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VIRTIO_SCSI_S_OK            0

ਬਣਤਰ disk_op_s;
ਪੂਰਨ virtio_scsi_process_op(ਬਣਤਰ disk_op_s *op);
ਖਾਲੀ virtio_scsi_ਤਿਆਰੀ(ਖਾਲੀ);
ਖਾਲੀ init_virtio_scsi_mmio(ਖਾਲੀ *ਅੰਕੜੇ);

#ਜੇ_ਅੰਤ /* _VIRTIO_SCSI_H */

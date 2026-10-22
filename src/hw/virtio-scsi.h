#यदि_अपरिभाषितम् _VIRTIO_SCSI_H
#परिभाषय _VIRTIO_SCSI_H

#परिभाषय VIRTIO_SCSI_CDB_आकार      32
#परिभाषय VIRTIO_SCSI_SENSE_आकार    96

संरचना virtio_scsi_config
{
    u32 अङ्कः_queues;
    u32 seg_max;
    u32 max_sectors;
    u32 cmd_प्रत्येकम्_lun;
    u32 event_info_आकार;
    u32 sense_आकार;
    u32 cdb_आकार;
    u16 max_channel;
    u16 max_target;
    u32 max_lun;
} __attribute__((packed));

/* This is the first element of the "out" scatter-gather list. */
संरचना virtio_scsi_req_cmd {
    u8 lun[8];
    u64 id;
    u8 task_attr;
    u8 prio;
    u8 crn;
    वर्ण cdb[VIRTIO_SCSI_CDB_आकार];
} __attribute__((packed));

/* This is the first element of the "in" scatter-gather list. */
संरचना virtio_scsi_resp_cmd {
    u32 sense_len;
    u32 residual;
    u16 स्थितिः_qualifier;
    u8 स्थितिः;
    u8 response;
    u8 sense[VIRTIO_SCSI_SENSE_आकार];
} __attribute__((packed));

#परिभाषय VIRTIO_SCSI_S_OK            0

संरचना disk_op_s;
पूर्णाङ्क virtio_scsi_process_op(संरचना disk_op_s *op);
शून्य virtio_scsi_सज्जता(शून्य);
शून्य init_virtio_scsi_mmio(शून्य *दत्त);

#यदि_अन्तः /* _VIRTIO_SCSI_H */

#که_نه_تعریف_شوی _VIRTIO_SCSI_H
#تعریف_کړه _VIRTIO_SCSI_H

#تعریف_کړه VIRTIO_SCSI_CDB_اندازه      32
#تعریف_کړه VIRTIO_SCSI_SENSE_اندازه    96

بنية virtio_scsi_config
{
    u32 عدد_queues;
    u32 seg_max;
    u32 max_sectors;
    u32 cmd_فی_lun;
    u32 event_info_اندازه;
    u32 sense_اندازه;
    u32 cdb_اندازه;
    u16 max_channel;
    u16 max_موخه;
    u32 max_lun;
} __attribute__((packed));

/* This is the first element of the "out" scatter-gather list. */
بنية virtio_scsi_req_cmd {
    u8 lun[8];
    u64 id;
    u8 task_attr;
    u8 prio;
    u8 crn;
    حرف cdb[VIRTIO_SCSI_CDB_اندازه];
} __attribute__((packed));

/* This is the first element of the "in" scatter-gather list. */
بنية virtio_scsi_resp_cmd {
    u32 sense_len;
    u32 residual;
    u16 حالت_qualifier;
    u8 حالت;
    u8 response;
    u8 sense[VIRTIO_SCSI_SENSE_اندازه];
} __attribute__((packed));

#تعریف_کړه VIRTIO_SCSI_S_OK            0

بنية disk_op_s;
صحيح virtio_scsi_process_op(بنية disk_op_s *op);
فراغ virtio_scsi_تیاری(فراغ);
فراغ init_virtio_scsi_mmio(فراغ *معلومات);

#د_که_پای /* _VIRTIO_SCSI_H */

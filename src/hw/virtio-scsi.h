#اگر_غیر_تعریف_شدہ _VIRTIO_SCSI_H
#تعریف_کرو _VIRTIO_SCSI_H

#تعریف_کرو VIRTIO_SCSI_CDB_سائز      32
#تعریف_کرو VIRTIO_SCSI_SENSE_سائز    96

ساخت virtio_scsi_config
{
    u32 عدد_queues;
    u32 seg_max;
    u32 max_sectors;
    u32 cmd_فی_lun;
    u32 event_info_سائز;
    u32 sense_سائز;
    u32 cdb_سائز;
    u16 max_channel;
    u16 max_ہدف;
    u32 max_lun;
} __attribute__((packed));

/* This is the first element of the "out" scatter-gather list. */
ساخت virtio_scsi_req_cmd {
    u8 lun[8];
    u64 id;
    u8 task_attr;
    u8 prio;
    u8 crn;
    حرف cdb[VIRTIO_SCSI_CDB_سائز];
} __attribute__((packed));

/* This is the first element of the "in" scatter-gather list. */
ساخت virtio_scsi_resp_cmd {
    u32 sense_len;
    u32 residual;
    u16 حالت_qualifier;
    u8 حالت_2;
    u8 response;
    u8 sense[VIRTIO_SCSI_SENSE_سائز];
} __attribute__((packed));

#تعریف_کرو VIRTIO_SCSI_S_OK            0

ساخت disk_op_s;
صحیح virtio_scsi_process_op(ساخت disk_op_s *op);
خالی virtio_scsi_تیاری(خالی);
خالی init_virtio_scsi_mmio(خالی *ڈیٹا);

#اگر_ختم /* _VIRTIO_SCSI_H */

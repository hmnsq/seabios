#جے_بنا_تعریف _VIRTIO_SCSI_H
#تعریف_کرو _VIRTIO_SCSI_H

#تعریف_کرو VIRTIO_SCSI_CDB_ناپ      32
#تعریف_کرو VIRTIO_SCSI_SENSE_ناپ    96

بنية virtio_scsi_config
{
    u32 عدد_queues;
    u32 seg_max;
    u32 max_sectors;
    u32 cmd_فی_lun;
    u32 event_info_ناپ;
    u32 sense_ناپ;
    u32 cdb_ناپ;
    u16 max_channel;
    u16 max_نشانہ;
    u32 max_lun;
} __attribute__((packed));

/* This is the first element of the "out" scatter-gather list. */
بنية virtio_scsi_req_cmd {
    u8 lun[8];
    u64 id;
    u8 task_attr;
    u8 prio;
    u8 crn;
    حرف cdb[VIRTIO_SCSI_CDB_ناپ];
} __attribute__((packed));

/* This is the first element of the "in" scatter-gather list. */
بنية virtio_scsi_resp_cmd {
    u32 sense_len;
    u32 residual;
    u16 حالت_qualifier;
    u8 حالت;
    u8 response;
    u8 sense[VIRTIO_SCSI_SENSE_ناپ];
} __attribute__((packed));

#تعریف_کرو VIRTIO_SCSI_S_OK            0

بنية disk_op_s;
صحيح virtio_scsi_process_op(بنية disk_op_s *op);
فراغ virtio_scsi_تیاری(فراغ);
فراغ init_virtio_scsi_mmio(فراغ *انکڑے);

#جے_انت /* _VIRTIO_SCSI_H */

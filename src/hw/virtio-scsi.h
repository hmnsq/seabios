#अगर_अघोषित _VIRTIO_SCSI_H
#घोषणा _VIRTIO_SCSI_H

#घोषणा VIRTIO_SCSI_CDB_आकार      32
#घोषणा VIRTIO_SCSI_SENSE_आकार    96

काष्ठा virtio_scsi_config
{
    u32 अंक_queues;
    u32 seg_max;
    u32 max_sectors;
    u32 cmd_per_lun;
    u32 event_info_आकार;
    u32 sense_आकार;
    u32 cdb_आकार;
    u16 max_channel;
    u16 max_target;
    u32 max_lun;
} __attribute__((packed));

/* This is the first element of the "out" scatter-gather list. */
काष्ठा virtio_scsi_req_cmd {
    u8 lun[8];
    u64 id;
    u8 task_attr;
    u8 prio;
    u8 crn;
    अक्षर cdb[VIRTIO_SCSI_CDB_आकार];
} __attribute__((packed));

/* This is the first element of the "in" scatter-gather list. */
काष्ठा virtio_scsi_resp_cmd {
    u32 sense_len;
    u32 residual;
    u16 status_qualifier;
    u8 status;
    u8 response;
    u8 sense[VIRTIO_SCSI_SENSE_आकार];
} __attribute__((packed));

#घोषणा VIRTIO_SCSI_S_OK            0

काष्ठा disk_op_s;
पूर्णांक virtio_scsi_process_op(काष्ठा disk_op_s *op);
व्योम virtio_scsi_setup(व्योम);
व्योम init_virtio_scsi_mmio(व्योम *तथ्य);

#पूर्ण_अगर /* _VIRTIO_SCSI_H */

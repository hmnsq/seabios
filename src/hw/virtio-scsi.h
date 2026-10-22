#నిర్వచితం_కాకపోతే _VIRTIO_SCSI_H
#నిర్వచించు _VIRTIO_SCSI_H

#నిర్వచించు VIRTIO_SCSI_CDB_కొలత      32
#నిర్వచించు VIRTIO_SCSI_SENSE_కొలత    96

నిర్మాణం virtio_scsi_config
{
    u32 అంకె_queues;
    u32 seg_max;
    u32 max_sectors;
    u32 cmd_చొప్పున_lun;
    u32 event_info_కొలత;
    u32 sense_కొలత;
    u32 cdb_కొలత;
    u16 max_channel;
    u16 max_లక్ష్యం;
    u32 max_lun;
} __attribute__((packed));

/* This is the first element of the "out" scatter-gather list. */
నిర్మాణం virtio_scsi_req_cmd {
    u8 lun[8];
    u64 id;
    u8 task_attr;
    u8 prio;
    u8 crn;
    అక్షరం cdb[VIRTIO_SCSI_CDB_కొలత];
} __attribute__((packed));

/* This is the first element of the "in" scatter-gather list. */
నిర్మాణం virtio_scsi_resp_cmd {
    u32 sense_len;
    u32 residual;
    u16 స్థితి_qualifier;
    u8 స్థితి_2;
    u8 response;
    u8 sense[VIRTIO_SCSI_SENSE_కొలత];
} __attribute__((packed));

#నిర్వచించు VIRTIO_SCSI_S_OK            0

నిర్మాణం disk_op_s;
పూర్ణాంకం virtio_scsi_process_op(నిర్మాణం disk_op_s *op);
శూన్య virtio_scsi_అమరిక(శూన్య);
శూన్య init_virtio_scsi_mmio(శూన్య *దత్తాంశం);

#అయితే_ముగింపు /* _VIRTIO_SCSI_H */

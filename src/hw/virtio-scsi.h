#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _VIRTIO_SCSI_H
#ವ್ಯಾಖ್ಯಾನಿಸು _VIRTIO_SCSI_H

#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_SCSI_CDB_ಗಾತ್ರ      32
#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_SCSI_SENSE_ಗಾತ್ರ    96

ರಚನೆ virtio_scsi_config
{
    u32 ಅಂಕೆ_queues;
    u32 seg_max;
    u32 max_sectors;
    u32 cmd_ಪ್ರತಿ_lun;
    u32 event_info_ಗಾತ್ರ;
    u32 sense_ಗಾತ್ರ;
    u32 cdb_ಗಾತ್ರ;
    u16 max_channel;
    u16 max_ಗುರಿ;
    u32 max_lun;
} __attribute__((packed));

/* This is the first element of the "out" scatter-gather list. */
ರಚನೆ virtio_scsi_req_cmd {
    u8 lun[8];
    u64 id;
    u8 task_attr;
    u8 prio;
    u8 crn;
    ಅಕ್ಷರ cdb[VIRTIO_SCSI_CDB_ಗಾತ್ರ];
} __attribute__((packed));

/* This is the first element of the "in" scatter-gather list. */
ರಚನೆ virtio_scsi_resp_cmd {
    u32 sense_len;
    u32 residual;
    u16 ಸ್ಥಿತಿ_qualifier;
    u8 ಸ್ಥಿತಿ_2;
    u8 response;
    u8 sense[VIRTIO_SCSI_SENSE_ಗಾತ್ರ];
} __attribute__((packed));

#ವ್ಯಾಖ್ಯಾನಿಸು VIRTIO_SCSI_S_OK            0

ರಚನೆ disk_op_s;
ಪೂರ್ಣಾಂಕ virtio_scsi_process_op(ರಚನೆ disk_op_s *op);
ಶೂನ್ಯ virtio_scsi_ಸಿದ್ಧತೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ init_virtio_scsi_mmio(ಶೂನ್ಯ *ದತ್ತಾಂಶ);

#ಆದರೆ_ಅಂತ್ಯ /* _VIRTIO_SCSI_H */

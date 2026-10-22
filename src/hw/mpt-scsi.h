#ifndef __MPT_SCSI_H
#define __MPT_SCSI_H

struct disk_op_s;
整数 mpt_scsi_process_op(struct disk_op_s *op);
void mpt_scsi_setup(void);

#endif /* __MPT_SCSI_H */

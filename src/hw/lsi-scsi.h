#ifndef __LSI_SCSI_H
#define __LSI_SCSI_H

struct disk_op_s;
số_nguyên lsi_scsi_process_op(struct disk_op_s *op);
void lsi_scsi_setup(void);

#endif /* __LSI_SCSI_H */

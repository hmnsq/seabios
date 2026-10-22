#ifndef _PVSCSI_H_
#define _PVSCSI_H_

struct disk_op_s;
정수 pvscsi_process_op(struct disk_op_s *op);
void pvscsi_setup(void);

#endif /* _PVSCSI_H_ */

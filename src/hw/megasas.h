#ifndef __MEGASAS_H
#define __MEGASAS_H

struct disk_op_s;
整数 megasas_process_op(struct disk_op_s *op);
void megasas_setup(void);

#endif /* __MEGASAS_H */

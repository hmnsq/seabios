// External interfaces for low level NVMe support
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#اگر_غیر_تعریف_شدہ __NVME_H
#تعریف_کرو __NVME_H

#شامل "block.h" // struct disk_op_s

خالی nvme_تیاری(خالی);
صحیح nvme_process_op(ساخت disk_op_s *op);

#اگر_ختم

/* EOF */

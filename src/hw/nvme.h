// External interfaces for low level NVMe support
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ifndef __NVME_H
#define __NVME_H

#अन्तर्भाव "block.h" // struct disk_op_s

शून्य nvme_setup(शून्य);
पूर्णांक nvme_process_op(संरचना disk_op_s *op);

#endif

/* EOF */

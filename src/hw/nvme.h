// External interfaces for low level NVMe support
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ifndef __NVME_H
#define __NVME_H

#شامل "block.h" // struct disk_op_s

تهی nvme_setup(تهی);
صحیح nvme_process_op(ساختار disk_op_s *op);

#endif

/* EOF */

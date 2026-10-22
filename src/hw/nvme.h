// External interfaces for low level NVMe support
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ifndef __NVME_H
#define __NVME_H

#অন্তর্ভাব "block.h" // struct disk_op_s

শূন্য nvme_setup(শূন্য);
পূর্ণাঙ্ক nvme_process_op(সংরচনা disk_op_s *op);

#endif

/* EOF */

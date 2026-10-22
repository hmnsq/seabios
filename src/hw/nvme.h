// External interfaces for low level NVMe support
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ifndef __NVME_H
#define __NVME_H

#அந்தர்பாவ "block.h" // struct disk_op_s

வெறுமை nvme_setup(வெறுமை);
முழு nvme_process_op(கட்டமைப்பு disk_op_s *op);

#endif

/* EOF */

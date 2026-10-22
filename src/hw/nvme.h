// External interfaces for low level NVMe support
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ifndef __NVME_H
#define __NVME_H

#અન્તર્ભાવ "block.h" // struct disk_op_s

શૂન્ય nvme_setup(શૂન્ય);
પૂર્ણાંક nvme_process_op(સંરચના disk_op_s *op);

#endif

/* EOF */

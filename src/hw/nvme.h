// External interfaces for low level NVMe support
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ifndef __NVME_H
#define __NVME_H

#ଅନ୍ତର୍ଭାଵ "block.h" // struct disk_op_s

ଶୂନ୍ୟ nvme_setup(ଶୂନ୍ୟ);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା nvme_process_op(ସଂରଚନା disk_op_s *op);

#endif

/* EOF */

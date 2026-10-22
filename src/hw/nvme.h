// External interfaces for low level NVMe support
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ifndef __NVME_H
#define __NVME_H

#ಅನ್ತರ್ಭಾವ "block.h" // struct disk_op_s

ಶೂನ್ಯ nvme_setup(ಶೂನ್ಯ);
ಪೂರ್ಣಾಙ್ಕ nvme_process_op(ಸಂರಚನಾ disk_op_s *op);

#endif

/* EOF */

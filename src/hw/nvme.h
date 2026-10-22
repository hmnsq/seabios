// External interfaces for low level NVMe support
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ifndef __NVME_H
#define __NVME_H

#അന്തര്ഭാവ "block.h" // struct disk_op_s

ശൂന്യം nvme_setup(ശൂന്യം);
പൂർണ്ണസംഖ്യ nvme_process_op(ഘടന disk_op_s *op);

#endif

/* EOF */

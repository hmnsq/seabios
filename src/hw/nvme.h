// External interfaces for low level NVMe support
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#اگر_تعریف_نشده __NVME_H
#تعریف_کن __NVME_H

#ضمن "block.h" // struct disk_op_s

فراغ nvme_راهاندازی(فراغ);
صحيح nvme_process_op(بنية disk_op_s *op);

#پایان_اگر

/* EOF */

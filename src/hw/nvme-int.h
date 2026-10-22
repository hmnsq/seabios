// NVMe datastructures and constants
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#جيڪڏهن_وصف_نه_ٿيل __NVME_INT_H
#وصف_ڪريو __NVME_INT_H

#ضمن "types.h" // u32
#ضمن "pcidevice.h" // struct pci_device

/* Data structures */

/* The register file of a NVMe host controller. This struct follows the naming
   scheme in the NVMe specification. */
بنية nvme_reg {
    u64 cap;                    /* controller capabilities */
    u32 vs;                     /*نسخو*/
    u32 intms;                  /* interrupt mask set */
    u32 intmc;                  /* interrupt mask clear */
    u32 cc;                     /* controller configuration */
    u32 _res0;
    u32 csts;                   /* controller status */
    u32 _res1;
    u32 aqa;                    /* admin queue attributes */
    u64 asq;                    /* admin submission queue base address */
    u64 acq;                    /* admin completion queue base address */
};

/* Submission queue entry */
بنية nvme_sqe {
    اتحاد {
        u32 dword[16];
        بنية {
            u32 cdw0;           /* Command DWORD 0 */
            u32 nsid;           /* Namespace ID */
            u64 _res0;
            u64 mptr;           /* metadata ptr */

            u64 dptr_prp1;
            u64 dptr_prp2;
        };
    };
};

/* Completion queue entry */
بنية nvme_cqe {
    اتحاد {
        u32 dword[4];
        بنية {
            u32 cdw0;
            u32 _res0;
            u16 sq_سر;
            u16 sq_id;
            u16 cid;
            u16 حالت;
        };
    };
};

/* The common part of every submission or completion queue. */
بنية nvme_قطار {
    u32 *dbl;                   /* doorbell */
    u16 نقاب;                   /* length - 1 */
};

بنية nvme_cq {
    بنية nvme_قطار common;
    بنية nvme_cqe *cqe;

    /* We have read upto (but not including) this entry in the queue. */
    u16 سر;

    /* The current phase bit the controller uses to indicate that it has written
       a new entry. This is inverted after each wrap. */
    بنانشان phase : 1;
};

بنية nvme_sq {
    بنية nvme_قطار common;
    بنية nvme_sqe *sqe;

    /* Corresponding completion queue. We only support a single SQ per CQ. */
    بنية nvme_cq *cq;

    /* The last entry the controller has fetched. */
    u16 سر;

    /* The last value we have written to the tail doorbell. */
    u16 پڇ;
};

بنية nvme_ctrl {
    بنية pci_device *pci;
    بنية nvme_reg متطاير *reg;

    u32 doorbell_stride;        /* in bytes */

    بنية nvme_sq admin_sq;
    بنية nvme_cq admin_cq;

    u32 ns_ڳڻپ;

    بنية nvme_sq io_sq;
    بنية nvme_cq io_cq;
};

بنية nvme_namespace {
    بنية drive_s drive;
    بنية nvme_ctrl *ctrl;

    u32 ns_id;

    u64 lba_ڳڻپ;              /* The total amount of sectors. */

    u32 بلاڪ_سائيز;
    u32 metadata_سائيز;
    u32 max_req_سائيز;
};

/* Data structures for NVMe admin identify commands */

بنية nvme_identify_ctrl {
    u16 vid;
    u16 ssvid;
    حرف sn[20];
    حرف mn[40];
    حرف fr[8];

    u8 rab;
    u8 ieee[3];
    u8 cmic;
    u8 mdts;

    حرف _boring[516 - 78];

    u32 nn;                     /* number of namespaces */
};

بنية nvme_identify_ns_list {
    u32 ns_id[1024];
};

بنية nvme_lba_format {
    u16 ms;
    u8  lbads;
    u8  rp;
};

بنية nvme_identify_ns {
    u64 nsze;
    u64 ncap;
    u64 nuse;
    u8  nsfeat;
    u8  nlbaf;
    u8  flbas;

    حرف _boring[128 - 27];

    بنية nvme_lba_format lbaf[16];
};

اتحاد nvme_identify {
    بنية nvme_identify_ns      ns;
    بنية nvme_identify_ctrl    ctrl;
    بنية nvme_identify_ns_list ns_list;
};

/* NVMe constants */

#وصف_ڪريو NVME_CAP_CSS_NVME (1ULL << 37)

#وصف_ڪريو NVME_CSTS_FATAL   (1U <<  1)
#وصف_ڪريو NVME_CSTS_RDY     (1U <<  0)

#وصف_ڪريو NVME_CC_EN        (1U <<  0)

#وصف_ڪريو NVME_SQE_OPC_ADMIN_CREATE_IO_SQ 1U
#وصف_ڪريو NVME_SQE_OPC_ADMIN_CREATE_IO_CQ 5U
#وصف_ڪريو NVME_SQE_OPC_ADMIN_IDENTIFY     6U

#وصف_ڪريو NVME_SQE_OPC_IO_WRITE 1U
#وصف_ڪريو NVME_SQE_OPC_IO_READ  2U

#وصف_ڪريو NVME_ADMIN_IDENTIFY_CNS_ID_NS       0U
#وصف_ڪريو NVME_ADMIN_IDENTIFY_CNS_ID_CTRL     1U
#وصف_ڪريو NVME_ADMIN_IDENTIFY_CNS_GET_NS_LIST 2U

#وصف_ڪريو NVME_CQE_DW3_P (1U << 16)

#وصف_ڪريو NVME_صفحو_سائيز 4096
#وصف_ڪريو NVME_صفحو_نقاب ~(NVME_صفحو_سائيز - 1)

/* Length for the queue entries. */
#وصف_ڪريو NVME_SQE_سائيز_LOG 6
#وصف_ڪريو NVME_CQE_سائيز_LOG 4

#جيڪڏهن_پورو

/* EOF */

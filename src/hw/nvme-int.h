// NVMe datastructures and constants
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#اگر_غیر_تعریف_شدہ __NVME_INT_H
#تعریف_کرو __NVME_INT_H

#شامل "types.h" // u32
#شامل "pcidevice.h" // struct pci_device

/* Data structures */

/* The register file of a NVMe host controller. This struct follows the naming
   scheme in the NVMe specification. */
ساخت nvme_reg {
    u64 cap;                    /* controller capabilities */
    u32 vs;                     /*نسخہ*/
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
ساخت nvme_sqe {
    اتحاد {
        u32 dword[16];
        ساخت {
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
ساخت nvme_cqe {
    اتحاد {
        u32 dword[4];
        ساخت {
            u32 cdw0;
            u32 _res0;
            u16 sq_سر;
            u16 sq_id;
            u16 cid;
            u16 حالت_2;
        };
    };
};

/* The common part of every submission or completion queue. */
ساخت nvme_قطار {
    u32 *dbl;                   /* doorbell */
    u16 نقاب;                   /* length - 1 */
};

ساخت nvme_cq {
    ساخت nvme_قطار common;
    ساخت nvme_cqe *cqe;

    /* We have read upto (but not including) this entry in the queue. */
    u16 سر;

    /* The current phase bit the controller uses to indicate that it has written
       a new entry. This is inverted after each wrap. */
    غیرعلامتی phase : 1;
};

ساخت nvme_sq {
    ساخت nvme_قطار common;
    ساخت nvme_sqe *sqe;

    /* Corresponding completion queue. We only support a single SQ per CQ. */
    ساخت nvme_cq *cq;

    /* The last entry the controller has fetched. */
    u16 سر;

    /* The last value we have written to the tail doorbell. */
    u16 دم;
};

ساخت nvme_ctrl {
    ساخت pci_device *pci;
    ساخت nvme_reg متلون *reg;

    u32 doorbell_stride;        /* in bytes */

    ساخت nvme_sq admin_sq;
    ساخت nvme_cq admin_cq;

    u32 ns_گنتی;

    ساخت nvme_sq io_sq;
    ساخت nvme_cq io_cq;
};

ساخت nvme_namespace {
    ساخت drive_s drive;
    ساخت nvme_ctrl *ctrl;

    u32 ns_id;

    u64 lba_گنتی;              /* The total amount of sectors. */

    u32 بلاک_سائز;
    u32 metadata_سائز;
    u32 max_req_سائز;
};

/* Data structures for NVMe admin identify commands */

ساخت nvme_identify_ctrl {
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

ساخت nvme_identify_ns_list {
    u32 ns_id[1024];
};

ساخت nvme_lba_format {
    u16 ms;
    u8  lbads;
    u8  rp;
};

ساخت nvme_identify_ns {
    u64 nsze;
    u64 ncap;
    u64 nuse;
    u8  nsfeat;
    u8  nlbaf;
    u8  flbas;

    حرف _boring[128 - 27];

    ساخت nvme_lba_format lbaf[16];
};

اتحاد nvme_identify {
    ساخت nvme_identify_ns      ns;
    ساخت nvme_identify_ctrl    ctrl;
    ساخت nvme_identify_ns_list ns_list;
};

/* NVMe constants */

#تعریف_کرو NVME_CAP_CSS_NVME (1ULL << 37)

#تعریف_کرو NVME_CSTS_FATAL   (1U <<  1)
#تعریف_کرو NVME_CSTS_RDY     (1U <<  0)

#تعریف_کرو NVME_CC_EN        (1U <<  0)

#تعریف_کرو NVME_SQE_OPC_ADMIN_CREATE_IO_SQ 1U
#تعریف_کرو NVME_SQE_OPC_ADMIN_CREATE_IO_CQ 5U
#تعریف_کرو NVME_SQE_OPC_ADMIN_IDENTIFY     6U

#تعریف_کرو NVME_SQE_OPC_IO_WRITE 1U
#تعریف_کرو NVME_SQE_OPC_IO_READ  2U

#تعریف_کرو NVME_ADMIN_IDENTIFY_CNS_ID_NS       0U
#تعریف_کرو NVME_ADMIN_IDENTIFY_CNS_ID_CTRL     1U
#تعریف_کرو NVME_ADMIN_IDENTIFY_CNS_GET_NS_LIST 2U

#تعریف_کرو NVME_CQE_DW3_P (1U << 16)

#تعریف_کرو NVME_صفحہ_سائز 4096
#تعریف_کرو NVME_صفحہ_نقاب ~(NVME_صفحہ_سائز - 1)

/* Length for the queue entries. */
#تعریف_کرو NVME_SQE_سائز_LOG 6
#تعریف_کرو NVME_CQE_سائز_LOG 4

#اگر_ختم

/* EOF */

// NVMe datastructures and constants
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ifndef __NVME_INT_H
#define __NVME_INT_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // struct pci_device

/* Data structures */

/* The register file of a NVMe host controller. This struct follows the naming
   scheme in the NVMe specification. */
ಸಂರಚನಾ nvme_reg {
    u64 cap;                    /* controller capabilities */
    u32 vs;                     /* version */
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
ಸಂರಚನಾ nvme_sqe {
    ಸಙ್ಘ {
        u32 dword[16];
        ಸಂರಚನಾ {
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
ಸಂರಚನಾ nvme_cqe {
    ಸಙ್ಘ {
        u32 dword[4];
        ಸಂರಚನಾ {
            u32 cdw0;
            u32 _res0;
            u16 sq_head;
            u16 sq_id;
            u16 cid;
            u16 status;
        };
    };
};

/* The common part of every submission or completion queue. */
ಸಂರಚನಾ nvme_queue {
    u32 *dbl;                   /* doorbell */
    u16 mask;                   /* length - 1 */
};

ಸಂರಚನಾ nvme_cq {
    ಸಂರಚನಾ nvme_queue common;
    ಸಂರಚನಾ nvme_cqe *cqe;

    /* We have read upto (but not including) this entry in the queue. */
    u16 head;

    /* The current phase bit the controller uses to indicate that it has written
       a new entry. This is inverted after each wrap. */
    unsigned phase : 1;
};

ಸಂರಚನಾ nvme_sq {
    ಸಂರಚನಾ nvme_queue common;
    ಸಂರಚನಾ nvme_sqe *sqe;

    /* Corresponding completion queue. We only support a single SQ per CQ. */
    ಸಂರಚನಾ nvme_cq *cq;

    /* The last entry the controller has fetched. */
    u16 head;

    /* The last value we have written to the tail doorbell. */
    u16 tail;
};

ಸಂರಚನಾ nvme_ctrl {
    ಸಂರಚನಾ pci_device *pci;
    ಸಂರಚನಾ nvme_reg ಚಞ್ಚಲ *reg;

    u32 doorbell_stride;        /* in bytes */

    ಸಂರಚನಾ nvme_sq admin_sq;
    ಸಂರಚನಾ nvme_cq admin_cq;

    u32 ns_ಎಣಿಕೆ;

    ಸಂರಚನಾ nvme_sq io_sq;
    ಸಂರಚನಾ nvme_cq io_cq;
};

ಸಂರಚನಾ nvme_namespace {
    ಸಂರಚನಾ drive_s drive;
    ಸಂರಚನಾ nvme_ctrl *ctrl;

    u32 ns_id;

    u64 lba_ಎಣಿಕೆ;              /* The total amount of sectors. */

    u32 block_size;
    u32 metadata_size;
    u32 max_req_size;
};

/* Data structures for NVMe admin identify commands */

ಸಂರಚನಾ nvme_identify_ctrl {
    u16 vid;
    u16 ssvid;
    ವರ್ಣ sn[20];
    ವರ್ಣ mn[40];
    ವರ್ಣ fr[8];

    u8 rab;
    u8 ieee[3];
    u8 cmic;
    u8 mdts;

    ವರ್ಣ _boring[516 - 78];

    u32 nn;                     /* number of namespaces */
};

ಸಂರಚನಾ nvme_identify_ns_list {
    u32 ns_id[1024];
};

ಸಂರಚನಾ nvme_lba_format {
    u16 ms;
    u8  lbads;
    u8  rp;
};

ಸಂರಚನಾ nvme_identify_ns {
    u64 nsze;
    u64 ncap;
    u64 nuse;
    u8  nsfeat;
    u8  nlbaf;
    u8  flbas;

    ವರ್ಣ _boring[128 - 27];

    ಸಂರಚನಾ nvme_lba_format lbaf[16];
};

ಸಙ್ಘ nvme_identify {
    ಸಂರಚನಾ nvme_identify_ns      ns;
    ಸಂರಚನಾ nvme_identify_ctrl    ctrl;
    ಸಂರಚನಾ nvme_identify_ns_list ns_list;
};

/* NVMe constants */

#define NVME_CAP_CSS_NVME (1ULL << 37)

#define NVME_CSTS_FATAL   (1U <<  1)
#define NVME_CSTS_RDY     (1U <<  0)

#define NVME_CC_EN        (1U <<  0)

#define NVME_SQE_OPC_ADMIN_CREATE_IO_SQ 1U
#define NVME_SQE_OPC_ADMIN_CREATE_IO_CQ 5U
#define NVME_SQE_OPC_ADMIN_IDENTIFY     6U

#define NVME_SQE_OPC_IO_WRITE 1U
#define NVME_SQE_OPC_IO_READ  2U

#define NVME_ADMIN_IDENTIFY_CNS_ID_NS       0U
#define NVME_ADMIN_IDENTIFY_CNS_ID_CTRL     1U
#define NVME_ADMIN_IDENTIFY_CNS_GET_NS_LIST 2U

#define NVME_CQE_DW3_P (1U << 16)

#define NVME_PAGE_SIZE 4096
#define NVME_PAGE_MASK ~(NVME_PAGE_SIZE - 1)

/* Length for the queue entries. */
#define NVME_SQE_SIZE_LOG 6
#define NVME_CQE_SIZE_LOG 4

#endif

/* EOF */

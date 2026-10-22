// NVMe datastructures and constants
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __NVME_INT_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __NVME_INT_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // u32
#ਅਨ੍ਤਰ੍ਭਾਵ "pcidevice.h" // struct pci_device

/* Data structures */

/* The register file of a NVMe host controller. This struct follows the naming
   scheme in the NVMe specification. */
ਬਣਤਰ nvme_reg {
    u64 cap;                    /* controller capabilities */
    u32 vs;                     /*ਸੰਸਕਰਣ*/
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
ਬਣਤਰ nvme_sqe {
    ਸੰਘ {
        u32 dword[16];
        ਬਣਤਰ {
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
ਬਣਤਰ nvme_cqe {
    ਸੰਘ {
        u32 dword[4];
        ਬਣਤਰ {
            u32 cdw0;
            u32 _res0;
            u16 sq_ਸਿਰ;
            u16 sq_id;
            u16 cid;
            u16 ਸਥਿਤੀ;
        };
    };
};

/* The common part of every submission or completion queue. */
ਬਣਤਰ nvme_ਕਤਾਰ {
    u32 *dbl;                   /* doorbell */
    u16 ਨਕਾਬ;                   /* length - 1 */
};

ਬਣਤਰ nvme_cq {
    ਬਣਤਰ nvme_ਕਤਾਰ common;
    ਬਣਤਰ nvme_cqe *cqe;

    /* We have read upto (but not including) this entry in the queue. */
    u16 ਸਿਰ;

    /* The current phase bit the controller uses to indicate that it has written
       a new entry. This is inverted after each wrap. */
    ਬਿਨਾਨਿਸ਼ਾਨੀ phase : 1;
};

ਬਣਤਰ nvme_sq {
    ਬਣਤਰ nvme_ਕਤਾਰ common;
    ਬਣਤਰ nvme_sqe *sqe;

    /* Corresponding completion queue. We only support a single SQ per CQ. */
    ਬਣਤਰ nvme_cq *cq;

    /* The last entry the controller has fetched. */
    u16 ਸਿਰ;

    /* The last value we have written to the tail doorbell. */
    u16 ਪੂਛ;
};

ਬਣਤਰ nvme_ctrl {
    ਬਣਤਰ pci_device *pci;
    ਬਣਤਰ nvme_reg ਅਸਥਿਰ *reg;

    u32 doorbell_stride;        /* in bytes */

    ਬਣਤਰ nvme_sq admin_sq;
    ਬਣਤਰ nvme_cq admin_cq;

    u32 ns_ਗਿਣਤੀ;

    ਬਣਤਰ nvme_sq io_sq;
    ਬਣਤਰ nvme_cq io_cq;
};

ਬਣਤਰ nvme_namespace {
    ਬਣਤਰ drive_s drive;
    ਬਣਤਰ nvme_ctrl *ctrl;

    u32 ns_id;

    u64 lba_ਗਿਣਤੀ;              /* The total amount of sectors. */

    u32 ਖੰਡ_ਮਾਪ;
    u32 metadata_ਮਾਪ;
    u32 max_req_ਮਾਪ;
};

/* Data structures for NVMe admin identify commands */

ਬਣਤਰ nvme_identify_ctrl {
    u16 vid;
    u16 ssvid;
    ਅੱਖਰ sn[20];
    ਅੱਖਰ mn[40];
    ਅੱਖਰ fr[8];

    u8 rab;
    u8 ieee[3];
    u8 cmic;
    u8 mdts;

    ਅੱਖਰ _boring[516 - 78];

    u32 nn;                     /* number of namespaces */
};

ਬਣਤਰ nvme_identify_ns_list {
    u32 ns_id[1024];
};

ਬਣਤਰ nvme_lba_format {
    u16 ms;
    u8  lbads;
    u8  rp;
};

ਬਣਤਰ nvme_identify_ns {
    u64 nsze;
    u64 ncap;
    u64 nuse;
    u8  nsfeat;
    u8  nlbaf;
    u8  flbas;

    ਅੱਖਰ _boring[128 - 27];

    ਬਣਤਰ nvme_lba_format lbaf[16];
};

ਸੰਘ nvme_identify {
    ਬਣਤਰ nvme_identify_ns      ns;
    ਬਣਤਰ nvme_identify_ctrl    ctrl;
    ਬਣਤਰ nvme_identify_ns_list ns_list;
};

/* NVMe constants */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NVME_CAP_CSS_NVME (1ULL << 37)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NVME_CSTS_FATAL   (1U <<  1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NVME_CSTS_RDY     (1U <<  0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NVME_CC_EN        (1U <<  0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NVME_SQE_OPC_ADMIN_CREATE_IO_SQ 1U
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NVME_SQE_OPC_ADMIN_CREATE_IO_CQ 5U
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NVME_SQE_OPC_ADMIN_IDENTIFY     6U

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NVME_SQE_OPC_IO_WRITE 1U
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NVME_SQE_OPC_IO_READ  2U

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NVME_ADMIN_IDENTIFY_CNS_ID_NS       0U
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NVME_ADMIN_IDENTIFY_CNS_ID_CTRL     1U
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NVME_ADMIN_IDENTIFY_CNS_GET_NS_LIST 2U

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NVME_CQE_DW3_P (1U << 16)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NVME_ਪੰਨਾ_ਮਾਪ 4096
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NVME_ਪੰਨਾ_ਨਕਾਬ ~(NVME_ਪੰਨਾ_ਮਾਪ - 1)

/* Length for the queue entries. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NVME_SQE_ਮਾਪ_LOG 6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NVME_CQE_ਮਾਪ_LOG 4

#ਜੇ_ਅੰਤ

/* EOF */

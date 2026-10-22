// NVMe datastructures and constants
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अगर_अघोषित __NVME_INT_H
#घोषणा __NVME_INT_H

#समावेश "types.h" // u32
#समावेश "pcidevice.h" // struct pci_device

/* Data structures */

/* The register file of a NVMe host controller. This struct follows the naming
   scheme in the NVMe specification. */
काष्ठा nvme_reg {
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
काष्ठा nvme_sqe {
    जोड़ {
        u32 dword[16];
        काष्ठा {
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
काष्ठा nvme_cqe {
    जोड़ {
        u32 dword[4];
        काष्ठा {
            u32 cdw0;
            u32 _res0;
            u16 sq_शीर्ष;
            u16 sq_id;
            u16 cid;
            u16 status;
        };
    };
};

/* The common part of every submission or completion queue. */
काष्ठा nvme_queue {
    u32 *dbl;                   /* doorbell */
    u16 mask;                   /* length - 1 */
};

काष्ठा nvme_cq {
    काष्ठा nvme_queue common;
    काष्ठा nvme_cqe *cqe;

    /* We have read upto (but not including) this entry in the queue. */
    u16 शीर्ष;

    /* The current phase bit the controller uses to indicate that it has written
       a new entry. This is inverted after each wrap. */
    अचिन्हित phase : 1;
};

काष्ठा nvme_sq {
    काष्ठा nvme_queue common;
    काष्ठा nvme_sqe *sqe;

    /* Corresponding completion queue. We only support a single SQ per CQ. */
    काष्ठा nvme_cq *cq;

    /* The last entry the controller has fetched. */
    u16 शीर्ष;

    /* The last value we have written to the tail doorbell. */
    u16 पुच्छ;
};

काष्ठा nvme_ctrl {
    काष्ठा pci_device *pci;
    काष्ठा nvme_reg अस्थिर *reg;

    u32 doorbell_stride;        /* in bytes */

    काष्ठा nvme_sq admin_sq;
    काष्ठा nvme_cq admin_cq;

    u32 ns_गिनती;

    काष्ठा nvme_sq io_sq;
    काष्ठा nvme_cq io_cq;
};

काष्ठा nvme_namespace {
    काष्ठा drive_s drive;
    काष्ठा nvme_ctrl *ctrl;

    u32 ns_id;

    u64 lba_गिनती;              /* The total amount of sectors. */

    u32 block_आकार;
    u32 metadata_आकार;
    u32 max_req_आकार;
};

/* Data structures for NVMe admin identify commands */

काष्ठा nvme_identify_ctrl {
    u16 vid;
    u16 ssvid;
    अक्षर sn[20];
    अक्षर mn[40];
    अक्षर fr[8];

    u8 rab;
    u8 ieee[3];
    u8 cmic;
    u8 mdts;

    अक्षर _boring[516 - 78];

    u32 nn;                     /* number of namespaces */
};

काष्ठा nvme_identify_ns_list {
    u32 ns_id[1024];
};

काष्ठा nvme_lba_format {
    u16 ms;
    u8  lbads;
    u8  rp;
};

काष्ठा nvme_identify_ns {
    u64 nsze;
    u64 ncap;
    u64 nuse;
    u8  nsfeat;
    u8  nlbaf;
    u8  flbas;

    अक्षर _boring[128 - 27];

    काष्ठा nvme_lba_format lbaf[16];
};

जोड़ nvme_identify {
    काष्ठा nvme_identify_ns      ns;
    काष्ठा nvme_identify_ctrl    ctrl;
    काष्ठा nvme_identify_ns_list ns_list;
};

/* NVMe constants */

#घोषणा NVME_CAP_CSS_NVME (1ULL << 37)

#घोषणा NVME_CSTS_FATAL   (1U <<  1)
#घोषणा NVME_CSTS_RDY     (1U <<  0)

#घोषणा NVME_CC_EN        (1U <<  0)

#घोषणा NVME_SQE_OPC_ADMIN_CREATE_IO_SQ 1U
#घोषणा NVME_SQE_OPC_ADMIN_CREATE_IO_CQ 5U
#घोषणा NVME_SQE_OPC_ADMIN_IDENTIFY     6U

#घोषणा NVME_SQE_OPC_IO_WRITE 1U
#घोषणा NVME_SQE_OPC_IO_READ  2U

#घोषणा NVME_ADMIN_IDENTIFY_CNS_ID_NS       0U
#घोषणा NVME_ADMIN_IDENTIFY_CNS_ID_CTRL     1U
#घोषणा NVME_ADMIN_IDENTIFY_CNS_GET_NS_LIST 2U

#घोषणा NVME_CQE_DW3_P (1U << 16)

#घोषणा NVME_PAGE_आकार 4096
#घोषणा NVME_PAGE_MASK ~(NVME_PAGE_आकार - 1)

/* Length for the queue entries. */
#घोषणा NVME_SQE_आकार_LOG 6
#घोषणा NVME_CQE_आकार_LOG 4

#पूर्ण_अगर

/* EOF */

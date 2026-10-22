// NVMe datastructures and constants
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#यदि_अपरिभाषितम् __NVME_INT_H
#परिभाषय __NVME_INT_H

#अन्तर्भाव "types.h" // u32
#अन्तर्भाव "pcidevice.h" // struct pci_device

/* Data structures */

/* The register file of a NVMe host controller. This struct follows the naming
   scheme in the NVMe specification. */
संरचना nvme_reg {
    u64 cap;                    /* controller capabilities */
    u32 vs;                     /*संस्करणम्*/
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
संरचना nvme_sqe {
    सङ्घ {
        u32 dword[16];
        संरचना {
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
संरचना nvme_cqe {
    सङ्घ {
        u32 dword[4];
        संरचना {
            u32 cdw0;
            u32 _res0;
            u16 sq_शीर्ष;
            u16 sq_id;
            u16 cid;
            u16 स्थितिः;
        };
    };
};

/* The common part of every submission or completion queue. */
संरचना nvme_queue {
    u32 *dbl;                   /* doorbell */
    u16 आवरणम्;                   /* length - 1 */
};

संरचना nvme_cq {
    संरचना nvme_queue common;
    संरचना nvme_cqe *cqe;

    /* We have read upto (but not including) this entry in the queue. */
    u16 शीर्ष;

    /* The current phase bit the controller uses to indicate that it has written
       a new entry. This is inverted after each wrap. */
    अचिह्नित phase : 1;
};

संरचना nvme_sq {
    संरचना nvme_queue common;
    संरचना nvme_sqe *sqe;

    /* Corresponding completion queue. We only support a single SQ per CQ. */
    संरचना nvme_cq *cq;

    /* The last entry the controller has fetched. */
    u16 शीर्ष;

    /* The last value we have written to the tail doorbell. */
    u16 पुच्छ;
};

संरचना nvme_ctrl {
    संरचना pci_device *pci;
    संरचना nvme_reg चञ्चल *reg;

    u32 doorbell_stride;        /* in bytes */

    संरचना nvme_sq admin_sq;
    संरचना nvme_cq admin_cq;

    u32 ns_गणनम्;

    संरचना nvme_sq io_sq;
    संरचना nvme_cq io_cq;
};

संरचना nvme_namespace {
    संरचना drive_s drive;
    संरचना nvme_ctrl *ctrl;

    u32 ns_id;

    u64 lba_गणनम्;              /* The total amount of sectors. */

    u32 खण्डम्_आकार;
    u32 metadata_आकार;
    u32 max_req_आकार;
};

/* Data structures for NVMe admin identify commands */

संरचना nvme_identify_ctrl {
    u16 vid;
    u16 ssvid;
    वर्ण sn[20];
    वर्ण mn[40];
    वर्ण fr[8];

    u8 rab;
    u8 ieee[3];
    u8 cmic;
    u8 mdts;

    वर्ण _boring[516 - 78];

    u32 nn;                     /* number of namespaces */
};

संरचना nvme_identify_ns_list {
    u32 ns_id[1024];
};

संरचना nvme_lba_format {
    u16 ms;
    u8  lbads;
    u8  rp;
};

संरचना nvme_identify_ns {
    u64 nsze;
    u64 ncap;
    u64 nuse;
    u8  nsfeat;
    u8  nlbaf;
    u8  flbas;

    वर्ण _boring[128 - 27];

    संरचना nvme_lba_format lbaf[16];
};

सङ्घ nvme_identify {
    संरचना nvme_identify_ns      ns;
    संरचना nvme_identify_ctrl    ctrl;
    संरचना nvme_identify_ns_list ns_list;
};

/* NVMe constants */

#परिभाषय NVME_CAP_CSS_NVME (1ULL << 37)

#परिभाषय NVME_CSTS_FATAL   (1U <<  1)
#परिभाषय NVME_CSTS_RDY     (1U <<  0)

#परिभाषय NVME_CC_EN        (1U <<  0)

#परिभाषय NVME_SQE_OPC_ADMIN_CREATE_IO_SQ 1U
#परिभाषय NVME_SQE_OPC_ADMIN_CREATE_IO_CQ 5U
#परिभाषय NVME_SQE_OPC_ADMIN_IDENTIFY     6U

#परिभाषय NVME_SQE_OPC_IO_WRITE 1U
#परिभाषय NVME_SQE_OPC_IO_READ  2U

#परिभाषय NVME_ADMIN_IDENTIFY_CNS_ID_NS       0U
#परिभाषय NVME_ADMIN_IDENTIFY_CNS_ID_CTRL     1U
#परिभाषय NVME_ADMIN_IDENTIFY_CNS_GET_NS_LIST 2U

#परिभाषय NVME_CQE_DW3_P (1U << 16)

#परिभाषय NVME_PAGE_आकार 4096
#परिभाषय NVME_PAGE_आवरणम् ~(NVME_PAGE_आकार - 1)

/* Length for the queue entries. */
#परिभाषय NVME_SQE_आकार_LOG 6
#परिभाषय NVME_CQE_आकार_LOG 4

#यदि_अन्तः

/* EOF */

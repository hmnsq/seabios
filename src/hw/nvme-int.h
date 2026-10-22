// NVMe datastructures and constants
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __NVME_INT_H
#ವ್ಯಾಖ್ಯಾನಿಸು __NVME_INT_H

#ಅನ್ತರ್ಭಾವ "types.h" // u32
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // struct pci_device

/* Data structures */

/* The register file of a NVMe host controller. This struct follows the naming
   scheme in the NVMe specification. */
ರಚನೆ nvme_reg {
    u64 cap;                    /* controller capabilities */
    u32 vs;                     /*ಆವೃತ್ತಿ*/
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
ರಚನೆ nvme_sqe {
    ಒಕ್ಕೂಟ {
        u32 dword[16];
        ರಚನೆ {
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
ರಚನೆ nvme_cqe {
    ಒಕ್ಕೂಟ {
        u32 dword[4];
        ರಚನೆ {
            u32 cdw0;
            u32 _res0;
            u16 sq_ತಲೆ;
            u16 sq_id;
            u16 cid;
            u16 ಸ್ಥಿತಿ_2;
        };
    };
};

/* The common part of every submission or completion queue. */
ರಚನೆ nvme_ಸರತಿ {
    u32 *dbl;                   /* doorbell */
    u16 ಮುಖವಾಡ;                   /* length - 1 */
};

ರಚನೆ nvme_cq {
    ರಚನೆ nvme_ಸರತಿ common;
    ರಚನೆ nvme_cqe *cqe;

    /* We have read upto (but not including) this entry in the queue. */
    u16 ತಲೆ;

    /* The current phase bit the controller uses to indicate that it has written
       a new entry. This is inverted after each wrap. */
    ಚಿಹ್ನೆರಹಿತ phase : 1;
};

ರಚನೆ nvme_sq {
    ರಚನೆ nvme_ಸರತಿ common;
    ರಚನೆ nvme_sqe *sqe;

    /* Corresponding completion queue. We only support a single SQ per CQ. */
    ರಚನೆ nvme_cq *cq;

    /* The last entry the controller has fetched. */
    u16 ತಲೆ;

    /* The last value we have written to the tail doorbell. */
    u16 ಬಾಲ;
};

ರಚನೆ nvme_ctrl {
    ರಚನೆ pci_device *pci;
    ರಚನೆ nvme_reg ಅಸ್ಥಿರ *reg;

    u32 doorbell_stride;        /* in bytes */

    ರಚನೆ nvme_sq admin_sq;
    ರಚನೆ nvme_cq admin_cq;

    u32 ns_ಎಣಿಕೆ;

    ರಚನೆ nvme_sq io_sq;
    ರಚನೆ nvme_cq io_cq;
};

ರಚನೆ nvme_namespace {
    ರಚನೆ drive_s drive;
    ರಚನೆ nvme_ctrl *ctrl;

    u32 ns_id;

    u64 lba_ಎಣಿಕೆ;              /* The total amount of sectors. */

    u32 ಖಂಡ_ಗಾತ್ರ;
    u32 metadata_ಗಾತ್ರ;
    u32 max_req_ಗಾತ್ರ;
};

/* Data structures for NVMe admin identify commands */

ರಚನೆ nvme_identify_ctrl {
    u16 vid;
    u16 ssvid;
    ಅಕ್ಷರ sn[20];
    ಅಕ್ಷರ mn[40];
    ಅಕ್ಷರ fr[8];

    u8 rab;
    u8 ieee[3];
    u8 cmic;
    u8 mdts;

    ಅಕ್ಷರ _boring[516 - 78];

    u32 nn;                     /* number of namespaces */
};

ರಚನೆ nvme_identify_ns_list {
    u32 ns_id[1024];
};

ರಚನೆ nvme_lba_format {
    u16 ms;
    u8  lbads;
    u8  rp;
};

ರಚನೆ nvme_identify_ns {
    u64 nsze;
    u64 ncap;
    u64 nuse;
    u8  nsfeat;
    u8  nlbaf;
    u8  flbas;

    ಅಕ್ಷರ _boring[128 - 27];

    ರಚನೆ nvme_lba_format lbaf[16];
};

ಒಕ್ಕೂಟ nvme_identify {
    ರಚನೆ nvme_identify_ns      ns;
    ರಚನೆ nvme_identify_ctrl    ctrl;
    ರಚನೆ nvme_identify_ns_list ns_list;
};

/* NVMe constants */

#ವ್ಯಾಖ್ಯಾನಿಸು NVME_CAP_CSS_NVME (1ULL << 37)

#ವ್ಯಾಖ್ಯಾನಿಸು NVME_CSTS_FATAL   (1U <<  1)
#ವ್ಯಾಖ್ಯಾನಿಸು NVME_CSTS_RDY     (1U <<  0)

#ವ್ಯಾಖ್ಯಾನಿಸು NVME_CC_EN        (1U <<  0)

#ವ್ಯಾಖ್ಯಾನಿಸು NVME_SQE_OPC_ADMIN_CREATE_IO_SQ 1U
#ವ್ಯಾಖ್ಯಾನಿಸು NVME_SQE_OPC_ADMIN_CREATE_IO_CQ 5U
#ವ್ಯಾಖ್ಯಾನಿಸು NVME_SQE_OPC_ADMIN_IDENTIFY     6U

#ವ್ಯಾಖ್ಯಾನಿಸು NVME_SQE_OPC_IO_WRITE 1U
#ವ್ಯಾಖ್ಯಾನಿಸು NVME_SQE_OPC_IO_READ  2U

#ವ್ಯಾಖ್ಯಾನಿಸು NVME_ADMIN_IDENTIFY_CNS_ID_NS       0U
#ವ್ಯಾಖ್ಯಾನಿಸು NVME_ADMIN_IDENTIFY_CNS_ID_CTRL     1U
#ವ್ಯಾಖ್ಯಾನಿಸು NVME_ADMIN_IDENTIFY_CNS_GET_NS_LIST 2U

#ವ್ಯಾಖ್ಯಾನಿಸು NVME_CQE_DW3_P (1U << 16)

#ವ್ಯಾಖ್ಯಾನಿಸು NVME_ಪುಟ_ಗಾತ್ರ 4096
#ವ್ಯಾಖ್ಯಾನಿಸು NVME_ಪುಟ_ಮುಖವಾಡ ~(NVME_ಪುಟ_ಗಾತ್ರ - 1)

/* Length for the queue entries. */
#ವ್ಯಾಖ್ಯಾನಿಸು NVME_SQE_ಗಾತ್ರ_LOG 6
#ವ್ಯಾಖ್ಯಾನಿಸು NVME_CQE_ಗಾತ್ರ_LOG 4

#ಆದರೆ_ಅಂತ್ಯ

/* EOF */

// NVMe datastructures and constants
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#నిర్వచితం_కాకపోతే __NVME_INT_H
#నిర్వచించు __NVME_INT_H

#అన్తర్భావ "types.h" // u32
#అన్తర్భావ "pcidevice.h" // struct pci_device

/* Data structures */

/* The register file of a NVMe host controller. This struct follows the naming
   scheme in the NVMe specification. */
నిర్మాణం nvme_reg {
    u64 cap;                    /* controller capabilities */
    u32 vs;                     /*సంస్కరణ*/
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
నిర్మాణం nvme_sqe {
    సంఘం {
        u32 dword[16];
        నిర్మాణం {
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
నిర్మాణం nvme_cqe {
    సంఘం {
        u32 dword[4];
        నిర్మాణం {
            u32 cdw0;
            u32 _res0;
            u16 sq_తల;
            u16 sq_id;
            u16 cid;
            u16 స్థితి_2;
        };
    };
};

/* The common part of every submission or completion queue. */
నిర్మాణం nvme_వరుస {
    u32 *dbl;                   /* doorbell */
    u16 ముసుగు;                   /* length - 1 */
};

నిర్మాణం nvme_cq {
    నిర్మాణం nvme_వరుస common;
    నిర్మాణం nvme_cqe *cqe;

    /* We have read upto (but not including) this entry in the queue. */
    u16 తల;

    /* The current phase bit the controller uses to indicate that it has written
       a new entry. This is inverted after each wrap. */
    గుర్తులేని phase : 1;
};

నిర్మాణం nvme_sq {
    నిర్మాణం nvme_వరుస common;
    నిర్మాణం nvme_sqe *sqe;

    /* Corresponding completion queue. We only support a single SQ per CQ. */
    నిర్మాణం nvme_cq *cq;

    /* The last entry the controller has fetched. */
    u16 తల;

    /* The last value we have written to the tail doorbell. */
    u16 తోక;
};

నిర్మాణం nvme_ctrl {
    నిర్మాణం pci_device *pci;
    నిర్మాణం nvme_reg అస్థిర *reg;

    u32 doorbell_stride;        /* in bytes */

    నిర్మాణం nvme_sq admin_sq;
    నిర్మాణం nvme_cq admin_cq;

    u32 ns_లెక్క;

    నిర్మాణం nvme_sq io_sq;
    నిర్మాణం nvme_cq io_cq;
};

నిర్మాణం nvme_namespace {
    నిర్మాణం drive_s drive;
    నిర్మాణం nvme_ctrl *ctrl;

    u32 ns_id;

    u64 lba_లెక్క;              /* The total amount of sectors. */

    u32 ఖండం_కొలత;
    u32 metadata_కొలత;
    u32 max_req_కొలత;
};

/* Data structures for NVMe admin identify commands */

నిర్మాణం nvme_identify_ctrl {
    u16 vid;
    u16 ssvid;
    అక్షరం sn[20];
    అక్షరం mn[40];
    అక్షరం fr[8];

    u8 rab;
    u8 ieee[3];
    u8 cmic;
    u8 mdts;

    అక్షరం _boring[516 - 78];

    u32 nn;                     /* number of namespaces */
};

నిర్మాణం nvme_identify_ns_list {
    u32 ns_id[1024];
};

నిర్మాణం nvme_lba_format {
    u16 ms;
    u8  lbads;
    u8  rp;
};

నిర్మాణం nvme_identify_ns {
    u64 nsze;
    u64 ncap;
    u64 nuse;
    u8  nsfeat;
    u8  nlbaf;
    u8  flbas;

    అక్షరం _boring[128 - 27];

    నిర్మాణం nvme_lba_format lbaf[16];
};

సంఘం nvme_identify {
    నిర్మాణం nvme_identify_ns      ns;
    నిర్మాణం nvme_identify_ctrl    ctrl;
    నిర్మాణం nvme_identify_ns_list ns_list;
};

/* NVMe constants */

#నిర్వచించు NVME_CAP_CSS_NVME (1ULL << 37)

#నిర్వచించు NVME_CSTS_FATAL   (1U <<  1)
#నిర్వచించు NVME_CSTS_RDY     (1U <<  0)

#నిర్వచించు NVME_CC_EN        (1U <<  0)

#నిర్వచించు NVME_SQE_OPC_ADMIN_CREATE_IO_SQ 1U
#నిర్వచించు NVME_SQE_OPC_ADMIN_CREATE_IO_CQ 5U
#నిర్వచించు NVME_SQE_OPC_ADMIN_IDENTIFY     6U

#నిర్వచించు NVME_SQE_OPC_IO_WRITE 1U
#నిర్వచించు NVME_SQE_OPC_IO_READ  2U

#నిర్వచించు NVME_ADMIN_IDENTIFY_CNS_ID_NS       0U
#నిర్వచించు NVME_ADMIN_IDENTIFY_CNS_ID_CTRL     1U
#నిర్వచించు NVME_ADMIN_IDENTIFY_CNS_GET_NS_LIST 2U

#నిర్వచించు NVME_CQE_DW3_P (1U << 16)

#నిర్వచించు NVME_పుట_కొలత 4096
#నిర్వచించు NVME_పుట_ముసుగు ~(NVME_పుట_కొలత - 1)

/* Length for the queue entries. */
#నిర్వచించు NVME_SQE_కొలత_LOG 6
#నిర్వచించు NVME_CQE_కొలత_LOG 4

#అయితే_ముగింపు

/* EOF */

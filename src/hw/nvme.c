// Low level NVMe disk access
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "blockcmd.h"
#అన్తర్భావ "malloc.h" // malloc_high
#అన్తర్భావ "output.h" // dprintf
#అన్తర్భావ "pci.h"
#అన్తర్భావ "pci_ids.h" // PCI_CLASS_STORAGE_NVME
#అన్తర్భావ "pci_regs.h" // PCI_BASE_ADDRESS_0
#అన్తర్భావ "pcidevice.h" // foreachpci
#అన్తర్భావ "stacks.h" // yield
#అన్తర్భావ "std/disk.h" // DISK_RET_
#అన్తర్భావ "string.h" // memset
#అన్తర్భావ "util.h" // boot_add_hd
#అన్తర్భావ "x86.h" // readl

#అన్తర్భావ "nvme.h"
#అన్తర్భావ "nvme-int.h"

// Page aligned "dma bounce buffer" of size NVME_PAGE_SIZE in high memory
స్థిర శూన్య *nvme_dma_బఫరు;

స్థిర శూన్య *
zalloc_పుట_aligned(నిర్మాణం zone_s *zone, u32 కొలత)
{
    శూన్య *res = _malloc(zone, కొలత, NVME_పుట_కొలత);
    ఒకవేళ (res) memset(res, 0, కొలత);
    తిరిగి res;
}

స్థిర శూన్య
nvme_init_వరుస_common(నిర్మాణం nvme_ctrl *ctrl, నిర్మాణం nvme_వరుస *q, u16 q_idx,
                       u16 నిడివి)
{
    memset(q, 0, పరిమాణం(*q));
    q->dbl = (u32 *)((అక్షరం *)ctrl->reg + 0x1000 + q_idx * ctrl->doorbell_stride);
    dprintf(3, " q %p q_idx %u dbl %p\n", q, q_idx, q->dbl);
    q->ముసుగు = నిడివి - 1;
}

స్థిర పూర్ణాంకం
nvme_init_sq(నిర్మాణం nvme_ctrl *ctrl, నిర్మాణం nvme_sq *sq, u16 q_idx, u16 నిడివి,
             నిర్మాణం nvme_cq *cq)
{
    nvme_init_వరుస_common(ctrl, &sq->common, q_idx, నిడివి);
    sq->sqe = zalloc_పుట_aligned(&Zone_ఎగువ, పరిమాణం(*sq->sqe) * నిడివి);

    ఒకవేళ (!sq->sqe) {
        warn_noalloc();
        తిరిగి -1;
    }

    dprintf(3, "sq %p q_idx %u sqe %p\n", sq, q_idx, sq->sqe);
    sq->cq   = cq;
    sq->తల = 0;
    sq->తోక = 0;

    తిరిగి 0;
}

స్థిర పూర్ణాంకం
nvme_init_cq(నిర్మాణం nvme_ctrl *ctrl, నిర్మాణం nvme_cq *cq, u16 q_idx, u16 నిడివి)
{
    nvme_init_వరుస_common(ctrl, &cq->common, q_idx, నిడివి);
    cq->cqe = zalloc_పుట_aligned(&Zone_ఎగువ, పరిమాణం(*cq->cqe) * నిడివి);
    ఒకవేళ (!cq->cqe) {
        warn_noalloc();
        తిరిగి -1;
    }

    cq->తల = 0;

    /* All CQE phase bits are initialized to zero. This means initially we wait
       for the host controller to set these to 1. */
    cq->phase = 1;

    తిరిగి 0;
}

స్థిర పూర్ణాంకం
nvme_poll_cq(నిర్మాణం nvme_cq *cq)
{
    u32 dw3 = readl(&cq->cqe[cq->తల].dword[3]);
    తిరిగి (!!(dw3 & NVME_CQE_DW3_P) == cq->phase);
}

స్థిర పూర్ణాంకం
nvme_is_cqe_success(నిర్మాణం nvme_cqe స్థిరరాశి *cqe)
{
    తిరిగి ((cqe->స్థితి_2 >> 1) & 0xFF) == 0;
}

స్థిర నిర్మాణం nvme_cqe
nvme_error_cqe(శూన్య)
{
    నిర్మాణం nvme_cqe r;

    /* 0xFF is a vendor specific status code != success. Should be okay for
       indicating failure. */
    memset(&r, 0xFF, పరిమాణం(r));
    తిరిగి r;
}

స్థిర నిర్మాణం nvme_cqe
nvme_consume_cqe(నిర్మాణం nvme_sq *sq)
{
    నిర్మాణం nvme_cq *cq = sq->cq;

    ఒకవేళ (!nvme_poll_cq(cq)) {
        /* Cannot consume a completion queue entry, if there is none ready. */
        తిరిగి nvme_error_cqe();
    }

    నిర్మాణం nvme_cqe *cqe = &cq->cqe[cq->తల];
    u16 cq_తదుపరి_తల = (cq->తల + 1) & cq->common.ముసుగు;
    dprintf(4, "cq %p head %u -> %u\n", cq, cq->తల, cq_తదుపరి_తల);
    ఒకవేళ (cq_తదుపరి_తల < cq->తల) {
        dprintf(3, "cq %p wrap\n", cq);
        cq->phase = ~cq->phase;
    }
    cq->తల = cq_తదుపరి_తల;

    /* Update the submission queue head. */
    ఒకవేళ (cqe->sq_తల != sq->తల) {
        sq->తల = cqe->sq_తల;
        dprintf(4, "sq %p advanced to %u\n", sq, cqe->sq_తల);
    }

    /* Tell the controller that we consumed the completion. */
    writel(cq->common.dbl, cq->తల);

    తిరిగి *cqe;
}

స్థిర నిర్మాణం nvme_cqe
nvme_wait(నిర్మాణం nvme_sq *sq)
{
    స్థిర స్థిరరాశి గుర్తులేని nvme_timeout = 5000 /* ms */;
    u32 గమ్యం = timer_calc(nvme_timeout);
    అయితే (!nvme_poll_cq(sq->cq)) {
        yield();

        ఒకవేళ (timer_check(గమ్యం)) {
            warn_timeout();
            తిరిగి nvme_error_cqe();
        }
    }

    తిరిగి nvme_consume_cqe(sq);
}

/* Returns the next submission queue entry (or NULL if the queue is full). It
   also fills out Command Dword 0 and clears the rest. */
స్థిర నిర్మాణం nvme_sqe *
nvme_get_తదుపరి_sqe(నిర్మాణం nvme_sq *sq, u8 opc, శూన్య *metadata, శూన్య *దత్తాంశం, శూన్య *data2)
{
    ఒకవేళ (((sq->తల + 1) & sq->common.ముసుగు) == sq->తోక) {
        dprintf(3, "submission queue is full\n");
        తిరిగి ఖాళీ;
    }

    నిర్మాణం nvme_sqe *sqe = &sq->sqe[sq->తోక];
    dprintf(4, "sq %p next_sqe %u\n", sq, sq->తోక);

    memset(sqe, 0, పరిమాణం(*sqe));
    sqe->cdw0 = opc | (sq->తోక << 16 /* CID */);
    sqe->mptr = (u32)metadata;
    sqe->dptr_prp1 = (u32)దత్తాంశం;
    sqe->dptr_prp2 = (u32)data2;

    తిరిగి sqe;
}

/* Call this after you've filled out an sqe that you've got from nvme_get_next_sqe. */
స్థిర శూన్య
nvme_commit_sqe(నిర్మాణం nvme_sq *sq)
{
    dprintf(4, "sq %p commit_sqe %u\n", sq, sq->తోక);
    sq->తోక = (sq->తోక + 1) & sq->common.ముసుగు;
    writel(sq->common.dbl, sq->తోక);
}

/* Perform an identify command on the admin queue and return the resulting
   buffer. This may be a NULL pointer, if something failed. This function
   cannot be used after initialization, because it uses buffers in tmp zone. */
స్థిర సంఘం nvme_identify *
nvme_admin_identify(నిర్మాణం nvme_ctrl *ctrl, u8 cns, u32 nsid)
{
    సంఘం nvme_identify *identify_buf = zalloc_పుట_aligned(&Zone_Tmp_ఎగువ, 4096);
    ఒకవేళ (!identify_buf) {
        /* Could not allocate identify buffer. */
        warn_internalerror();
        తిరిగి ఖాళీ;
    }

    నిర్మాణం nvme_sqe *cmd_identify;
    cmd_identify = nvme_get_తదుపరి_sqe(&ctrl->admin_sq,
                                     NVME_SQE_OPC_ADMIN_IDENTIFY, ఖాళీ,
                                     identify_buf, ఖాళీ);

    ఒకవేళ (!cmd_identify) {
        warn_internalerror();
        వెళ్ళు error;
    }

    cmd_identify->nsid = nsid;
    cmd_identify->dword[10] = cns;

    nvme_commit_sqe(&ctrl->admin_sq);

    నిర్మాణం nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    ఒకవేళ (!nvme_is_cqe_success(&cqe)) {
        వెళ్ళు error;
    }

    తిరిగి identify_buf;
 error:
    విడుదల(identify_buf);
    తిరిగి ఖాళీ;
}

స్థిర నిర్మాణం nvme_identify_ctrl *
nvme_admin_identify_ctrl(నిర్మాణం nvme_ctrl *ctrl)
{
    తిరిగి &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_CTRL, 0)->ctrl;
}

స్థిర నిర్మాణం nvme_identify_ns *
nvme_admin_identify_ns(నిర్మాణం nvme_ctrl *ctrl, u32 ns_id)
{
    తిరిగి &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_NS,
                                ns_id)->ns;
}

స్థిర శూన్య
nvme_probe_ns(నిర్మాణం nvme_ctrl *ctrl, u32 ns_idx, u8 mdts)
{
    u32 ns_id = ns_idx + 1;

    నిర్మాణం nvme_identify_ns *id = nvme_admin_identify_ns(ctrl, ns_id);
    ఒకవేళ (!id) {
        dprintf(2, "NVMe couldn't identify namespace %u.\n", ns_id);
        వెళ్ళు free_బఫరు;
    }

    u8 current_lba_format = id->flbas & 0xF;
    ఒకవేళ (current_lba_format > id->nlbaf) {
        dprintf(2, "NVMe NS %u: current LBA format %u is beyond what the "
                " namespace supports (%u)?\n",
                ns_id, current_lba_format, id->nlbaf + 1);
        వెళ్ళు free_బఫరు;
    }

    ఒకవేళ (!id->nsze) {
        dprintf(2, "NVMe NS %u is inactive.\n", ns_id);
        వెళ్ళు free_బఫరు;
    }

    ఒకవేళ (!nvme_dma_బఫరు) {
        nvme_dma_బఫరు = zalloc_పుట_aligned(&Zone_ఎగువ, NVME_పుట_కొలత);
        ఒకవేళ (!nvme_dma_బఫరు) {
            warn_noalloc();
            వెళ్ళు free_బఫరు;
        }
    }

    నిర్మాణం nvme_namespace *ns = malloc_fseg(పరిమాణం(*ns));
    ఒకవేళ (!ns) {
        warn_noalloc();
        వెళ్ళు free_బఫరు;
    }
    memset(ns, 0, పరిమాణం(*ns));
    ns->ctrl  = ctrl;
    ns->ns_id = ns_id;
    ns->lba_లెక్క = id->nsze;

    నిర్మాణం nvme_lba_format *fmt = &id->lbaf[current_lba_format];

    ns->ఖండం_కొలత    = 1U << fmt->lbads;
    ns->metadata_కొలత = fmt->ms;

    ఒకవేళ (ns->ఖండం_కొలత > NVME_పుట_కొలత) {
        /* If we see devices that trigger this path, we need to increase our
           buffer size. */
        warn_internalerror();
        విడుదల(ns);
        వెళ్ళు free_బఫరు;
    }

    ns->drive.cntl_id   = ns_idx;
    ns->drive.removable = 0;
    ns->drive.type      = DTYPE_NVME;
    ns->drive.blksize   = ns->ఖండం_కొలత;
    ns->drive.sectors   = ns->lba_లెక్క;

    ఒకవేళ (mdts) {
        ns->max_req_కొలత = ((1U << mdts) * NVME_పుట_కొలత) / ns->ఖండం_కొలత;
        dprintf(3, "NVME NS %u max request size: %d sectors\n",
                ns_id, ns->max_req_కొలత);
    } లేకపోతే {
        ns->max_req_కొలత = -1U;
    }

    అక్షరం *desc = znprintf(MAXDESCSIZE, "NVMe NS %u: %llu MiB (%llu %u-byte "
                          "blocks + %u-byte metadata)",
                          ns_id, (ns->lba_లెక్క * ns->ఖండం_కొలత) >> 20,
                          ns->lba_లెక్క, ns->ఖండం_కొలత, ns->metadata_కొలత);

    dprintf(3, "%s\n", desc);
    boot_కూడు_hd(&ns->drive, desc, bootprio_కనుగొను_pci_device(ctrl->pci));

free_బఫరు:
    విడుదల (id);
}


/* Release memory allocated for a completion queue */
స్థిర శూన్య
nvme_destroy_cq(నిర్మాణం nvme_cq *cq)
{
    విడుదల(cq->cqe);
    cq->cqe = ఖాళీ;
}

/* Release memory allocated for a submission queue */
స్థిర శూన్య
nvme_destroy_sq(నిర్మాణం nvme_sq *sq)
{
    విడుదల(sq->sqe);
    sq->sqe = ఖాళీ;
}

/* Returns 0 on success. */
స్థిర పూర్ణాంకం
nvme_create_io_cq(నిర్మాణం nvme_ctrl *ctrl, నిర్మాణం nvme_cq *cq, u16 q_idx)
{
    పూర్ణాంకం rc;
    నిర్మాణం nvme_sqe *cmd_create_cq;
    u32 నిడివి = 1 + (ctrl->reg->cap & 0xffff);
    ఒకవేళ (నిడివి > NVME_పుట_కొలత / పరిమాణం(నిర్మాణం nvme_cqe))
        నిడివి = NVME_పుట_కొలత / పరిమాణం(నిర్మాణం nvme_cqe);

    rc = nvme_init_cq(ctrl, cq, q_idx, నిడివి);
    ఒకవేళ (rc) {
        వెళ్ళు err;
    }

    cmd_create_cq = nvme_get_తదుపరి_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_CQ, ఖాళీ,
                                      cq->cqe, ఖాళీ);
    ఒకవేళ (!cmd_create_cq) {
        వెళ్ళు err_destroy_cq;
    }

    cmd_create_cq->dword[10] = (cq->common.ముసుగు << 16) | (q_idx >> 1);
    cmd_create_cq->dword[11] = 1 /* physically contiguous */;

    nvme_commit_sqe(&ctrl->admin_sq);

    నిర్మాణం nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    ఒకవేళ (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io cq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        వెళ్ళు err_destroy_cq;
    }

    తిరిగి 0;

err_destroy_cq:
    nvme_destroy_cq(cq);
err:
    తిరిగి -1;
}

/* Returns 0 on success. */
స్థిర పూర్ణాంకం
nvme_create_io_sq(నిర్మాణం nvme_ctrl *ctrl, నిర్మాణం nvme_sq *sq, u16 q_idx, నిర్మాణం nvme_cq *cq)
{
    పూర్ణాంకం rc;
    నిర్మాణం nvme_sqe *cmd_create_sq;
    u32 నిడివి = 1 + (ctrl->reg->cap & 0xffff);
    ఒకవేళ (నిడివి > NVME_పుట_కొలత / పరిమాణం(నిర్మాణం nvme_cqe))
        నిడివి = NVME_పుట_కొలత / పరిమాణం(నిర్మాణం nvme_cqe);

    rc = nvme_init_sq(ctrl, sq, q_idx, నిడివి, cq);
    ఒకవేళ (rc) {
        వెళ్ళు err;
    }

    cmd_create_sq = nvme_get_తదుపరి_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_SQ, ఖాళీ,
                                      sq->sqe, ఖాళీ);
    ఒకవేళ (!cmd_create_sq) {
        వెళ్ళు err_destroy_sq;
    }

    cmd_create_sq->dword[10] = (sq->common.ముసుగు << 16) | (q_idx >> 1);
    cmd_create_sq->dword[11] = (q_idx >> 1) << 16 | 1 /* contiguous */;
    dprintf(3, "sq %p create dword10 %08x dword11 %08x\n", sq,
            cmd_create_sq->dword[10], cmd_create_sq->dword[11]);

    nvme_commit_sqe(&ctrl->admin_sq);

    నిర్మాణం nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    ఒకవేళ (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io sq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);
        వెళ్ళు err_destroy_sq;
    }

    తిరిగి 0;

err_destroy_sq:
    nvme_destroy_sq(sq);
err:
    తిరిగి -1;
}

/* Reads count sectors into buf. The buffer cannot cross page boundaries. */
స్థిర పూర్ణాంకం
nvme_io_xfer(నిర్మాణం nvme_namespace *ns, u64 lba, శూన్య *prp1, శూన్య *prp2,
             u16 లెక్క, పూర్ణాంకం లిఖ)
{
    ఒకవేళ (((u32)prp1 & 0x3) || ((u32)prp2 & 0x3)) {
        /* Buffer is misaligned */
        warn_internalerror();
        తిరిగి -1;
    }

    నిర్మాణం nvme_sqe *io_read = nvme_get_తదుపరి_sqe(&ns->ctrl->io_sq,
                                                 లిఖ ? NVME_SQE_OPC_IO_WRITE
                                                       : NVME_SQE_OPC_IO_READ,
                                                 ఖాళీ, prp1, prp2);
    io_read->nsid = ns->ns_id;
    io_read->dword[10] = (u32)lba;
    io_read->dword[11] = (u32)(lba >> 32);
    io_read->dword[12] = (1U << 31 /* limited retry */) | (లెక్క - 1);

    nvme_commit_sqe(&ns->ctrl->io_sq);

    నిర్మాణం nvme_cqe cqe = nvme_wait(&ns->ctrl->io_sq);

    ఒకవేళ (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "read io: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        తిరిగి -1;
    }

    dprintf(5, "ns %u %s lba %llu+%u\n", ns->ns_id, లిఖ ? "write" : "read",
            lba, లెక్క);
    తిరిగి లెక్క;
}

// Transfer up to one page of data using the internal dma bounce buffer
స్థిర పూర్ణాంకం
nvme_bounce_xfer(నిర్మాణం nvme_namespace *ns, u64 lba, శూన్య *buf, u16 లెక్క,
                 పూర్ణాంకం లిఖ)
{
    u16 స్థిరరాశి max_ఖండాలు = NVME_పుట_కొలత / ns->ఖండం_కొలత;
    u16 ఖండాలు = లెక్క < max_ఖండాలు ? లెక్క : max_ఖండాలు;

    ఒకవేళ (లిఖ)
        జ్ఞాపకం_నకలు(nvme_dma_బఫరు, buf, ఖండాలు * ns->ఖండం_కొలత);

    పూర్ణాంకం res = nvme_io_xfer(ns, lba, nvme_dma_బఫరు, ఖాళీ, ఖండాలు, లిఖ);

    ఒకవేళ (!లిఖ && res >= 0)
        జ్ఞాపకం_నకలు(buf, nvme_dma_బఫరు, res * ns->ఖండం_కొలత);

    తిరిగి res;
}

#నిర్వచించు NVME_MAX_PRPL_ENTRIES 15 /* Allows requests up to 64kb */

// Transfer data using page list (if applicable)
స్థిర పూర్ణాంకం
nvme_prpl_xfer(నిర్మాణం nvme_namespace *ns, u64 lba, శూన్య *buf, u16 లెక్క,
               పూర్ణాంకం లిఖ)
{
    u32 పునాది = (దీర్ఘ)buf;
    s32 కొలత;

    ఒకవేళ (లెక్క > ns->max_req_కొలత)
        లెక్క = ns->max_req_కొలత;

    కొలత = లెక్క * ns->ఖండం_కొలత;
    /* Special case for transfers that fit into PRP1, but are unaligned */
    ఒకవేళ (((కొలత + (పునాది & ~NVME_పుట_ముసుగు)) <= NVME_పుట_కొలత))
        వెళ్ళు single;

    /* Every request has to be page aligned */
    ఒకవేళ (పునాది & ~NVME_పుట_ముసుగు)
        వెళ్ళు bounce;

    /* Make sure a full block fits into the last chunk */
    ఒకవేళ (కొలత & (ns->ఖండం_కొలత - 1ULL))
        వెళ్ళు bounce;

    /* Build PRP list if we need to describe more than 2 pages */
    ఒకవేళ ((ns->ఖండం_కొలత * లెక్క) > (NVME_పుట_కొలత * 2)) {
        u32 prpl_len = 0;
        u64 *prpl = nvme_dma_బఫరు;
        పూర్ణాంకం మొదటి_పుట = 1;
        కొరకు (; కొలత > 0; పునాది += NVME_పుట_కొలత, కొలత -= NVME_పుట_కొలత) {
            ఒకవేళ (మొదటి_పుట) {
                /* First page is special */
                మొదటి_పుట = 0;
                కొనసాగు;
            }
            ఒకవేళ (prpl_len >= NVME_MAX_PRPL_ENTRIES)
                వెళ్ళు bounce;
            prpl[prpl_len++] = పునాది;
        }
        తిరిగి nvme_io_xfer(ns, lba, buf, prpl, లెక్క, లిఖ);
    }

    /* Directly embed the 2nd page if we only need 2 pages */
    ఒకవేళ ((ns->ఖండం_కొలత * లెక్క) > NVME_పుట_కొలత)
        తిరిగి nvme_io_xfer(ns, lba, buf, buf + NVME_పుట_కొలత, లెక్క, లిఖ);

single:
    /* One page is enough, don't expose anything else */
    తిరిగి nvme_io_xfer(ns, lba, buf, ఖాళీ, లెక్క, లిఖ);

bounce:
    /* Use bounce buffer to make transfer */
    తిరిగి nvme_bounce_xfer(ns, lba, buf, లెక్క, లిఖ);
}

స్థిర పూర్ణాంకం
nvme_create_io_queues(నిర్మాణం nvme_ctrl *ctrl)
{
    ఒకవేళ (nvme_create_io_cq(ctrl, &ctrl->io_cq, 3))
        వెళ్ళు err;

    ఒకవేళ (nvme_create_io_sq(ctrl, &ctrl->io_sq, 2, &ctrl->io_cq))
        వెళ్ళు err_free_cq;

    తిరిగి 0;

 err_free_cq:
    nvme_destroy_cq(&ctrl->io_cq);
 err:
    తిరిగి -1;
}

/* Waits for CSTS.RDY to match rdy. Returns 0 on success. */
స్థిర పూర్ణాంకం
nvme_wait_csts_rdy(నిర్మాణం nvme_ctrl *ctrl, గుర్తులేని rdy)
{
    u32 స్థిరరాశి max_గమ్యం = 500 /* ms */ * ((ctrl->reg->cap >> 24) & 0xFFU);
    u32 గమ్యం = timer_calc(max_గమ్యం);
    u32 csts;

    అయితే (rdy != ((csts = ctrl->reg->csts) & NVME_CSTS_RDY)) {
        yield();

        ఒకవేళ (csts & NVME_CSTS_FATAL) {
            dprintf(3, "NVMe fatal error during controller shutdown\n");
            తిరిగి -1;
        }

        ఒకవేళ (timer_check(గమ్యం)) {
            warn_timeout();
            తిరిగి -1;
        }
    }

    తిరిగి 0;
}

/* Returns 0 on success. */
స్థిర పూర్ణాంకం
nvme_controller_enable(నిర్మాణం nvme_ctrl *ctrl)
{
    పూర్ణాంకం rc;

    pci_enable_busmaster(ctrl->pci);

    /* Turn the controller off. */
    ctrl->reg->cc = 0;
    ఒకవేళ (nvme_wait_csts_rdy(ctrl, 0)) {
        dprintf(2, "NVMe fatal error during controller shutdown\n");
        తిరిగి -1;
    }

    ctrl->doorbell_stride = 4U << ((ctrl->reg->cap >> 32) & 0xF);

    rc = nvme_init_cq(ctrl, &ctrl->admin_cq, 1,
                      NVME_పుట_కొలత / పరిమాణం(నిర్మాణం nvme_cqe));
    ఒకవేళ (rc) {
        తిరిగి -1;
    }

    rc = nvme_init_sq(ctrl, &ctrl->admin_sq, 0,
                      NVME_పుట_కొలత / పరిమాణం(నిర్మాణం nvme_sqe), &ctrl->admin_cq);
    ఒకవేళ (rc) {
        వెళ్ళు err_destroy_admin_cq;
    }

    ctrl->reg->aqa = ctrl->admin_cq.common.ముసుగు << 16
        | ctrl->admin_sq.common.ముసుగు;

    ctrl->reg->asq = (u32)ctrl->admin_sq.sqe;
    ctrl->reg->acq = (u32)ctrl->admin_cq.cqe;

    dprintf(3, "  admin submission queue: %p\n", ctrl->admin_sq.sqe);
    dprintf(3, "  admin completion queue: %p\n", ctrl->admin_cq.cqe);

    ctrl->reg->cc = NVME_CC_EN | (NVME_CQE_కొలత_LOG << 20)
        | (NVME_SQE_కొలత_LOG << 16 /* IOSQES */);

    ఒకవేళ (nvme_wait_csts_rdy(ctrl, 1)) {
        dprintf(2, "NVMe fatal error while enabling controller\n");
        వెళ్ళు err_destroy_admin_sq;
    }

    /* The admin queue is set up and the controller is ready. Let's figure out
       what namespaces we have. */

    నిర్మాణం nvme_identify_ctrl *identify = nvme_admin_identify_ctrl(ctrl);

    ఒకవేళ (!identify) {
        dprintf(2, "NVMe couldn't identify controller.\n");
        వెళ్ళు err_destroy_admin_sq;
    }

    dprintf(3, "NVMe has %u namespace%s.\n",
            identify->nn, (identify->nn == 1) ? "" : "s");

    ctrl->ns_లెక్క = identify->nn;
    u8 mdts = identify->mdts;
    విడుదల(identify);

    ఒకవేళ ((ctrl->ns_లెక్క == 0) || nvme_create_io_queues(ctrl)) {
        /* No point to continue, if the controller says it doesn't have
           namespaces or we couldn't create I/O queues. */
        వెళ్ళు err_destroy_admin_sq;
    }

    /* Populate namespace IDs */
    పూర్ణాంకం ns_idx;
    కొరకు (ns_idx = 0; ns_idx < ctrl->ns_లెక్క; ns_idx++) {
        nvme_probe_ns(ctrl, ns_idx, mdts);
    }

    dprintf(3, "NVMe initialization complete!\n");
    తిరిగి 0;

 err_destroy_admin_sq:
    nvme_destroy_sq(&ctrl->admin_sq);
 err_destroy_admin_cq:
    nvme_destroy_cq(&ctrl->admin_cq);
    తిరిగి -1;
}

/* Initialize an NVMe controller and detect its drives. */
స్థిర శూన్య
nvme_controller_అమరిక(శూన్య *opaque)
{
    u8 skip_nonbootable = is_bootprio_strict();
    నిర్మాణం pci_device *pci = opaque;

    ఒకవేళ (skip_nonbootable && bootprio_కనుగొను_pci_device(pci) < 0) {
        dprintf(1, "skipping init of a non-bootable NVMe at %pP\n",
                pci);
        వెళ్ళు err;
    }

    నిర్మాణం nvme_reg అస్థిర *reg = pci_enable_membar(pci, PCI_పునాది_చిరునామా_0);
    ఒకవేళ (!reg)
        తిరిగి;

    u32 సంస్కరణ_2 = reg->vs;
    dprintf(3, "Found NVMe controller with version %u.%u.%u.\n",
            సంస్కరణ_2 >> 16, (సంస్కరణ_2 >> 8) & 0xFF, సంస్కరణ_2 & 0xFF);
    dprintf(3, "  Capabilities %016llx\n", reg->cap);

    ఒకవేళ (~reg->cap & NVME_CAP_CSS_NVME) {
        dprintf(3, "Controller doesn't speak NVMe command set. Skipping.\n");
        వెళ్ళు err;
    }

    నిర్మాణం nvme_ctrl *ctrl = malloc_ఎగువ(పరిమాణం(*ctrl));
    ఒకవేళ (!ctrl) {
        warn_noalloc();
        వెళ్ళు err;
    }

    memset(ctrl, 0, పరిమాణం(*ctrl));

    ctrl->reg = reg;
    ctrl->pci = pci;

    ఒకవేళ (nvme_controller_enable(ctrl)) {
        వెళ్ళు err_free_ctrl;
    }

    తిరిగి;

 err_free_ctrl:
    విడుదల(ctrl);
 err:
    dprintf(2, "Failed to enable NVMe controller.\n");
}

// Locate and init NVMe controllers
స్థిర శూన్య
nvme_పరిశీలన(శూన్య)
{
    // Scan PCI bus for NVMe adapters
    నిర్మాణం pci_device *pci;

    foreachpci(pci) {
        ఒకవేళ (pci->class != PCI_CLASS_STORAGE_NVME)
            కొనసాగు;
        ఒకవేళ (pci->prog_if != 2 /* as of NVM 1.0e */) {
            dprintf(3, "Found incompatble NVMe: prog-if=%02x\n", pci->prog_if);
            కొనసాగు;
        }

        నిరంతరం_thread(nvme_controller_అమరిక, pci);
    }
}

స్థిర పూర్ణాంకం
nvme_cmd_readwrite(నిర్మాణం nvme_namespace *ns, నిర్మాణం disk_op_s *op, పూర్ణాంకం లిఖ)
{
    పూర్ణాంకం i;
    కొరకు (i = 0; i < op->లెక్క;) {
        u16 ఖండాలు_మిగిలిన = op->లెక్క - i;
        అక్షరం *op_buf = op->buf_fl + i * ns->ఖండం_కొలత;
        పూర్ణాంకం ఖండాలు = nvme_prpl_xfer(ns, op->lba + i, op_buf,
                                    ఖండాలు_మిగిలిన, లిఖ);
        ఒకవేళ (ఖండాలు < 0)
            తిరిగి DISK_RET_EBADTRACK;
        i += ఖండాలు;
    }

    తిరిగి DISK_RET_SUCCESS;
}

పూర్ణాంకం
nvme_process_op(నిర్మాణం disk_op_s *op)
{
    ఒకవేళ (!CONFIG_NVME)
        తిరిగి DISK_RET_SUCCESS;

    నిర్మాణం nvme_namespace *ns = container_యొక్క(op->drive_fl, నిర్మాణం nvme_namespace,
                                             drive);

    ఎంపిక (op->command) {
    స్థితి CMD_READ:
    స్థితి CMD_WRITE:
        తిరిగి nvme_cmd_readwrite(ns, op, op->command == CMD_WRITE);
    అప్రమేయ:
        తిరిగి default_process_op(op);
    }
}

శూన్య
nvme_అమరిక(శూన్య)
{
    ASSERT32FLAT();
    ఒకవేళ (!CONFIG_NVME)
        తిరిగి;

    dprintf(3, "init nvme\n");
    nvme_పరిశీలన();
}

/* EOF */

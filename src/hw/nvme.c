// Low level NVMe disk access
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "blockcmd.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "malloc.h" // malloc_high
#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // dprintf
#ਅਨ੍ਤਰ੍ਭਾਵ "pci.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_ids.h" // PCI_CLASS_STORAGE_NVME
#ਅਨ੍ਤਰ੍ਭਾਵ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ਅਨ੍ਤਰ੍ਭਾਵ "pcidevice.h" // foreachpci
#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // yield
#ਅਨ੍ਤਰ੍ਭਾਵ "std/disk.h" // DISK_RET_
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memset
#ਅਨ੍ਤਰ੍ਭਾਵ "util.h" // boot_add_hd
#ਅਨ੍ਤਰ੍ਭਾਵ "x86.h" // readl

#ਅਨ੍ਤਰ੍ਭਾਵ "nvme.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "nvme-int.h"

// Page aligned "dma bounce buffer" of size NVME_PAGE_SIZE in high memory
ਸਥਿਰ ਖਾਲੀ *nvme_dma_ਬਫਰ;

ਸਥਿਰ ਖਾਲੀ *
zalloc_ਪੰਨਾ_aligned(ਬਣਤਰ zone_s *zone, u32 ਮਾਪ)
{
    ਖਾਲੀ *res = _malloc(zone, ਮਾਪ, NVME_ਪੰਨਾ_ਮਾਪ);
    ਜੇ (res) memset(res, 0, ਮਾਪ);
    ਵਾਪਸ res;
}

ਸਥਿਰ ਖਾਲੀ
nvme_init_ਕਤਾਰ_common(ਬਣਤਰ nvme_ctrl *ctrl, ਬਣਤਰ nvme_ਕਤਾਰ *q, u16 q_idx,
                       u16 ਲੰਮਾਈ)
{
    memset(q, 0, ਆਕਾਰ(*q));
    q->dbl = (u32 *)((ਅੱਖਰ *)ctrl->reg + 0x1000 + q_idx * ctrl->doorbell_stride);
    dprintf(3, " q %p q_idx %u dbl %p\n", q, q_idx, q->dbl);
    q->ਨਕਾਬ = ਲੰਮਾਈ - 1;
}

ਸਥਿਰ ਪੂਰਨ
nvme_init_sq(ਬਣਤਰ nvme_ctrl *ctrl, ਬਣਤਰ nvme_sq *sq, u16 q_idx, u16 ਲੰਮਾਈ,
             ਬਣਤਰ nvme_cq *cq)
{
    nvme_init_ਕਤਾਰ_common(ctrl, &sq->common, q_idx, ਲੰਮਾਈ);
    sq->sqe = zalloc_ਪੰਨਾ_aligned(&Zone_ਉੱਚਾ, ਆਕਾਰ(*sq->sqe) * ਲੰਮਾਈ);

    ਜੇ (!sq->sqe) {
        warn_noalloc();
        ਵਾਪਸ -1;
    }

    dprintf(3, "sq %p q_idx %u sqe %p\n", sq, q_idx, sq->sqe);
    sq->cq   = cq;
    sq->ਸਿਰ = 0;
    sq->ਪੂਛ = 0;

    ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ
nvme_init_cq(ਬਣਤਰ nvme_ctrl *ctrl, ਬਣਤਰ nvme_cq *cq, u16 q_idx, u16 ਲੰਮਾਈ)
{
    nvme_init_ਕਤਾਰ_common(ctrl, &cq->common, q_idx, ਲੰਮਾਈ);
    cq->cqe = zalloc_ਪੰਨਾ_aligned(&Zone_ਉੱਚਾ, ਆਕਾਰ(*cq->cqe) * ਲੰਮਾਈ);
    ਜੇ (!cq->cqe) {
        warn_noalloc();
        ਵਾਪਸ -1;
    }

    cq->ਸਿਰ = 0;

    /* All CQE phase bits are initialized to zero. This means initially we wait
       for the host controller to set these to 1. */
    cq->phase = 1;

    ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ
nvme_poll_cq(ਬਣਤਰ nvme_cq *cq)
{
    u32 dw3 = readl(&cq->cqe[cq->ਸਿਰ].dword[3]);
    ਵਾਪਸ (!!(dw3 & NVME_CQE_DW3_P) == cq->phase);
}

ਸਥਿਰ ਪੂਰਨ
nvme_is_cqe_success(ਬਣਤਰ nvme_cqe ਅਚਲ *cqe)
{
    ਵਾਪਸ ((cqe->ਸਥਿਤੀ >> 1) & 0xFF) == 0;
}

ਸਥਿਰ ਬਣਤਰ nvme_cqe
nvme_error_cqe(ਖਾਲੀ)
{
    ਬਣਤਰ nvme_cqe r;

    /* 0xFF is a vendor specific status code != success. Should be okay for
       indicating failure. */
    memset(&r, 0xFF, ਆਕਾਰ(r));
    ਵਾਪਸ r;
}

ਸਥਿਰ ਬਣਤਰ nvme_cqe
nvme_consume_cqe(ਬਣਤਰ nvme_sq *sq)
{
    ਬਣਤਰ nvme_cq *cq = sq->cq;

    ਜੇ (!nvme_poll_cq(cq)) {
        /* Cannot consume a completion queue entry, if there is none ready. */
        ਵਾਪਸ nvme_error_cqe();
    }

    ਬਣਤਰ nvme_cqe *cqe = &cq->cqe[cq->ਸਿਰ];
    u16 cq_ਅਗਲਾ_ਸਿਰ = (cq->ਸਿਰ + 1) & cq->common.ਨਕਾਬ;
    dprintf(4, "cq %p head %u -> %u\n", cq, cq->ਸਿਰ, cq_ਅਗਲਾ_ਸਿਰ);
    ਜੇ (cq_ਅਗਲਾ_ਸਿਰ < cq->ਸਿਰ) {
        dprintf(3, "cq %p wrap\n", cq);
        cq->phase = ~cq->phase;
    }
    cq->ਸਿਰ = cq_ਅਗਲਾ_ਸਿਰ;

    /* Update the submission queue head. */
    ਜੇ (cqe->sq_ਸਿਰ != sq->ਸਿਰ) {
        sq->ਸਿਰ = cqe->sq_ਸਿਰ;
        dprintf(4, "sq %p advanced to %u\n", sq, cqe->sq_ਸਿਰ);
    }

    /* Tell the controller that we consumed the completion. */
    writel(cq->common.dbl, cq->ਸਿਰ);

    ਵਾਪਸ *cqe;
}

ਸਥਿਰ ਬਣਤਰ nvme_cqe
nvme_wait(ਬਣਤਰ nvme_sq *sq)
{
    ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ nvme_timeout = 5000 /* ms */;
    u32 ਮੰਜ਼ਿਲ = timer_calc(nvme_timeout);
    ਜਦੋਂ (!nvme_poll_cq(sq->cq)) {
        yield();

        ਜੇ (timer_check(ਮੰਜ਼ਿਲ)) {
            warn_timeout();
            ਵਾਪਸ nvme_error_cqe();
        }
    }

    ਵਾਪਸ nvme_consume_cqe(sq);
}

/* Returns the next submission queue entry (or NULL if the queue is full). It
   also fills out Command Dword 0 and clears the rest. */
ਸਥਿਰ ਬਣਤਰ nvme_sqe *
nvme_get_ਅਗਲਾ_sqe(ਬਣਤਰ nvme_sq *sq, u8 opc, ਖਾਲੀ *metadata, ਖਾਲੀ *ਅੰਕੜੇ, ਖਾਲੀ *data2)
{
    ਜੇ (((sq->ਸਿਰ + 1) & sq->common.ਨਕਾਬ) == sq->ਪੂਛ) {
        dprintf(3, "submission queue is full\n");
        ਵਾਪਸ ਨਿਰਾ;
    }

    ਬਣਤਰ nvme_sqe *sqe = &sq->sqe[sq->ਪੂਛ];
    dprintf(4, "sq %p next_sqe %u\n", sq, sq->ਪੂਛ);

    memset(sqe, 0, ਆਕਾਰ(*sqe));
    sqe->cdw0 = opc | (sq->ਪੂਛ << 16 /* CID */);
    sqe->mptr = (u32)metadata;
    sqe->dptr_prp1 = (u32)ਅੰਕੜੇ;
    sqe->dptr_prp2 = (u32)data2;

    ਵਾਪਸ sqe;
}

/* Call this after you've filled out an sqe that you've got from nvme_get_next_sqe. */
ਸਥਿਰ ਖਾਲੀ
nvme_commit_sqe(ਬਣਤਰ nvme_sq *sq)
{
    dprintf(4, "sq %p commit_sqe %u\n", sq, sq->ਪੂਛ);
    sq->ਪੂਛ = (sq->ਪੂਛ + 1) & sq->common.ਨਕਾਬ;
    writel(sq->common.dbl, sq->ਪੂਛ);
}

/* Perform an identify command on the admin queue and return the resulting
   buffer. This may be a NULL pointer, if something failed. This function
   cannot be used after initialization, because it uses buffers in tmp zone. */
ਸਥਿਰ ਸੰਘ nvme_identify *
nvme_admin_identify(ਬਣਤਰ nvme_ctrl *ctrl, u8 cns, u32 nsid)
{
    ਸੰਘ nvme_identify *identify_buf = zalloc_ਪੰਨਾ_aligned(&Zone_Tmp_ਉੱਚਾ, 4096);
    ਜੇ (!identify_buf) {
        /* Could not allocate identify buffer. */
        warn_internalerror();
        ਵਾਪਸ ਨਿਰਾ;
    }

    ਬਣਤਰ nvme_sqe *cmd_identify;
    cmd_identify = nvme_get_ਅਗਲਾ_sqe(&ctrl->admin_sq,
                                     NVME_SQE_OPC_ADMIN_IDENTIFY, ਨਿਰਾ,
                                     identify_buf, ਨਿਰਾ);

    ਜੇ (!cmd_identify) {
        warn_internalerror();
        ਜਾਓ error;
    }

    cmd_identify->nsid = nsid;
    cmd_identify->dword[10] = cns;

    nvme_commit_sqe(&ctrl->admin_sq);

    ਬਣਤਰ nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    ਜੇ (!nvme_is_cqe_success(&cqe)) {
        ਜਾਓ error;
    }

    ਵਾਪਸ identify_buf;
 error:
    ਮੁਕਤ(identify_buf);
    ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ ਬਣਤਰ nvme_identify_ctrl *
nvme_admin_identify_ctrl(ਬਣਤਰ nvme_ctrl *ctrl)
{
    ਵਾਪਸ &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_CTRL, 0)->ctrl;
}

ਸਥਿਰ ਬਣਤਰ nvme_identify_ns *
nvme_admin_identify_ns(ਬਣਤਰ nvme_ctrl *ctrl, u32 ns_id)
{
    ਵਾਪਸ &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_NS,
                                ns_id)->ns;
}

ਸਥਿਰ ਖਾਲੀ
nvme_probe_ns(ਬਣਤਰ nvme_ctrl *ctrl, u32 ns_idx, u8 mdts)
{
    u32 ns_id = ns_idx + 1;

    ਬਣਤਰ nvme_identify_ns *id = nvme_admin_identify_ns(ctrl, ns_id);
    ਜੇ (!id) {
        dprintf(2, "NVMe couldn't identify namespace %u.\n", ns_id);
        ਜਾਓ free_ਬਫਰ;
    }

    u8 current_lba_format = id->flbas & 0xF;
    ਜੇ (current_lba_format > id->nlbaf) {
        dprintf(2, "NVMe NS %u: current LBA format %u is beyond what the "
                " namespace supports (%u)?\n",
                ns_id, current_lba_format, id->nlbaf + 1);
        ਜਾਓ free_ਬਫਰ;
    }

    ਜੇ (!id->nsze) {
        dprintf(2, "NVMe NS %u is inactive.\n", ns_id);
        ਜਾਓ free_ਬਫਰ;
    }

    ਜੇ (!nvme_dma_ਬਫਰ) {
        nvme_dma_ਬਫਰ = zalloc_ਪੰਨਾ_aligned(&Zone_ਉੱਚਾ, NVME_ਪੰਨਾ_ਮਾਪ);
        ਜੇ (!nvme_dma_ਬਫਰ) {
            warn_noalloc();
            ਜਾਓ free_ਬਫਰ;
        }
    }

    ਬਣਤਰ nvme_namespace *ns = malloc_fseg(ਆਕਾਰ(*ns));
    ਜੇ (!ns) {
        warn_noalloc();
        ਜਾਓ free_ਬਫਰ;
    }
    memset(ns, 0, ਆਕਾਰ(*ns));
    ns->ctrl  = ctrl;
    ns->ns_id = ns_id;
    ns->lba_ਗਿਣਤੀ = id->nsze;

    ਬਣਤਰ nvme_lba_format *fmt = &id->lbaf[current_lba_format];

    ns->ਖੰਡ_ਮਾਪ    = 1U << fmt->lbads;
    ns->metadata_ਮਾਪ = fmt->ms;

    ਜੇ (ns->ਖੰਡ_ਮਾਪ > NVME_ਪੰਨਾ_ਮਾਪ) {
        /* If we see devices that trigger this path, we need to increase our
           buffer size. */
        warn_internalerror();
        ਮੁਕਤ(ns);
        ਜਾਓ free_ਬਫਰ;
    }

    ns->drive.cntl_id   = ns_idx;
    ns->drive.removable = 0;
    ns->drive.type      = DTYPE_NVME;
    ns->drive.blksize   = ns->ਖੰਡ_ਮਾਪ;
    ns->drive.sectors   = ns->lba_ਗਿਣਤੀ;

    ਜੇ (mdts) {
        ns->max_req_ਮਾਪ = ((1U << mdts) * NVME_ਪੰਨਾ_ਮਾਪ) / ns->ਖੰਡ_ਮਾਪ;
        dprintf(3, "NVME NS %u max request size: %d sectors\n",
                ns_id, ns->max_req_ਮਾਪ);
    } ਨਹੀਂਤਾਂ {
        ns->max_req_ਮਾਪ = -1U;
    }

    ਅੱਖਰ *desc = znprintf(MAXDESCSIZE, "NVMe NS %u: %llu MiB (%llu %u-byte "
                          "blocks + %u-byte metadata)",
                          ns_id, (ns->lba_ਗਿਣਤੀ * ns->ਖੰਡ_ਮਾਪ) >> 20,
                          ns->lba_ਗਿਣਤੀ, ns->ਖੰਡ_ਮਾਪ, ns->metadata_ਮਾਪ);

    dprintf(3, "%s\n", desc);
    boot_ਜੋੜੋ_hd(&ns->drive, desc, bootprio_ਲੱਭੋ_pci_device(ctrl->pci));

free_ਬਫਰ:
    ਮੁਕਤ (id);
}


/* Release memory allocated for a completion queue */
ਸਥਿਰ ਖਾਲੀ
nvme_destroy_cq(ਬਣਤਰ nvme_cq *cq)
{
    ਮੁਕਤ(cq->cqe);
    cq->cqe = ਨਿਰਾ;
}

/* Release memory allocated for a submission queue */
ਸਥਿਰ ਖਾਲੀ
nvme_destroy_sq(ਬਣਤਰ nvme_sq *sq)
{
    ਮੁਕਤ(sq->sqe);
    sq->sqe = ਨਿਰਾ;
}

/* Returns 0 on success. */
ਸਥਿਰ ਪੂਰਨ
nvme_create_io_cq(ਬਣਤਰ nvme_ctrl *ctrl, ਬਣਤਰ nvme_cq *cq, u16 q_idx)
{
    ਪੂਰਨ rc;
    ਬਣਤਰ nvme_sqe *cmd_create_cq;
    u32 ਲੰਮਾਈ = 1 + (ctrl->reg->cap & 0xffff);
    ਜੇ (ਲੰਮਾਈ > NVME_ਪੰਨਾ_ਮਾਪ / ਆਕਾਰ(ਬਣਤਰ nvme_cqe))
        ਲੰਮਾਈ = NVME_ਪੰਨਾ_ਮਾਪ / ਆਕਾਰ(ਬਣਤਰ nvme_cqe);

    rc = nvme_init_cq(ctrl, cq, q_idx, ਲੰਮਾਈ);
    ਜੇ (rc) {
        ਜਾਓ err;
    }

    cmd_create_cq = nvme_get_ਅਗਲਾ_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_CQ, ਨਿਰਾ,
                                      cq->cqe, ਨਿਰਾ);
    ਜੇ (!cmd_create_cq) {
        ਜਾਓ err_destroy_cq;
    }

    cmd_create_cq->dword[10] = (cq->common.ਨਕਾਬ << 16) | (q_idx >> 1);
    cmd_create_cq->dword[11] = 1 /* physically contiguous */;

    nvme_commit_sqe(&ctrl->admin_sq);

    ਬਣਤਰ nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    ਜੇ (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io cq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        ਜਾਓ err_destroy_cq;
    }

    ਵਾਪਸ 0;

err_destroy_cq:
    nvme_destroy_cq(cq);
err:
    ਵਾਪਸ -1;
}

/* Returns 0 on success. */
ਸਥਿਰ ਪੂਰਨ
nvme_create_io_sq(ਬਣਤਰ nvme_ctrl *ctrl, ਬਣਤਰ nvme_sq *sq, u16 q_idx, ਬਣਤਰ nvme_cq *cq)
{
    ਪੂਰਨ rc;
    ਬਣਤਰ nvme_sqe *cmd_create_sq;
    u32 ਲੰਮਾਈ = 1 + (ctrl->reg->cap & 0xffff);
    ਜੇ (ਲੰਮਾਈ > NVME_ਪੰਨਾ_ਮਾਪ / ਆਕਾਰ(ਬਣਤਰ nvme_cqe))
        ਲੰਮਾਈ = NVME_ਪੰਨਾ_ਮਾਪ / ਆਕਾਰ(ਬਣਤਰ nvme_cqe);

    rc = nvme_init_sq(ctrl, sq, q_idx, ਲੰਮਾਈ, cq);
    ਜੇ (rc) {
        ਜਾਓ err;
    }

    cmd_create_sq = nvme_get_ਅਗਲਾ_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_SQ, ਨਿਰਾ,
                                      sq->sqe, ਨਿਰਾ);
    ਜੇ (!cmd_create_sq) {
        ਜਾਓ err_destroy_sq;
    }

    cmd_create_sq->dword[10] = (sq->common.ਨਕਾਬ << 16) | (q_idx >> 1);
    cmd_create_sq->dword[11] = (q_idx >> 1) << 16 | 1 /* contiguous */;
    dprintf(3, "sq %p create dword10 %08x dword11 %08x\n", sq,
            cmd_create_sq->dword[10], cmd_create_sq->dword[11]);

    nvme_commit_sqe(&ctrl->admin_sq);

    ਬਣਤਰ nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    ਜੇ (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io sq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);
        ਜਾਓ err_destroy_sq;
    }

    ਵਾਪਸ 0;

err_destroy_sq:
    nvme_destroy_sq(sq);
err:
    ਵਾਪਸ -1;
}

/* Reads count sectors into buf. The buffer cannot cross page boundaries. */
ਸਥਿਰ ਪੂਰਨ
nvme_io_xfer(ਬਣਤਰ nvme_namespace *ns, u64 lba, ਖਾਲੀ *prp1, ਖਾਲੀ *prp2,
             u16 ਗਿਣਤੀ_2, ਪੂਰਨ ਲਿਖ)
{
    ਜੇ (((u32)prp1 & 0x3) || ((u32)prp2 & 0x3)) {
        /* Buffer is misaligned */
        warn_internalerror();
        ਵਾਪਸ -1;
    }

    ਬਣਤਰ nvme_sqe *io_read = nvme_get_ਅਗਲਾ_sqe(&ns->ctrl->io_sq,
                                                 ਲਿਖ ? NVME_SQE_OPC_IO_WRITE
                                                       : NVME_SQE_OPC_IO_READ,
                                                 ਨਿਰਾ, prp1, prp2);
    io_read->nsid = ns->ns_id;
    io_read->dword[10] = (u32)lba;
    io_read->dword[11] = (u32)(lba >> 32);
    io_read->dword[12] = (1U << 31 /* limited retry */) | (ਗਿਣਤੀ_2 - 1);

    nvme_commit_sqe(&ns->ctrl->io_sq);

    ਬਣਤਰ nvme_cqe cqe = nvme_wait(&ns->ctrl->io_sq);

    ਜੇ (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "read io: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        ਵਾਪਸ -1;
    }

    dprintf(5, "ns %u %s lba %llu+%u\n", ns->ns_id, ਲਿਖ ? "write" : "read",
            lba, ਗਿਣਤੀ_2);
    ਵਾਪਸ ਗਿਣਤੀ_2;
}

// Transfer up to one page of data using the internal dma bounce buffer
ਸਥਿਰ ਪੂਰਨ
nvme_bounce_xfer(ਬਣਤਰ nvme_namespace *ns, u64 lba, ਖਾਲੀ *buf, u16 ਗਿਣਤੀ_2,
                 ਪੂਰਨ ਲਿਖ)
{
    u16 ਅਚਲ max_ਖੰਡਾਂ = NVME_ਪੰਨਾ_ਮਾਪ / ns->ਖੰਡ_ਮਾਪ;
    u16 ਖੰਡਾਂ = ਗਿਣਤੀ_2 < max_ਖੰਡਾਂ ? ਗਿਣਤੀ_2 : max_ਖੰਡਾਂ;

    ਜੇ (ਲਿਖ)
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(nvme_dma_ਬਫਰ, buf, ਖੰਡਾਂ * ns->ਖੰਡ_ਮਾਪ);

    ਪੂਰਨ res = nvme_io_xfer(ns, lba, nvme_dma_ਬਫਰ, ਨਿਰਾ, ਖੰਡਾਂ, ਲਿਖ);

    ਜੇ (!ਲਿਖ && res >= 0)
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(buf, nvme_dma_ਬਫਰ, res * ns->ਖੰਡ_ਮਾਪ);

    ਵਾਪਸ res;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NVME_MAX_PRPL_ENTRIES 15 /* Allows requests up to 64kb */

// Transfer data using page list (if applicable)
ਸਥਿਰ ਪੂਰਨ
nvme_prpl_xfer(ਬਣਤਰ nvme_namespace *ns, u64 lba, ਖਾਲੀ *buf, u16 ਗਿਣਤੀ_2,
               ਪੂਰਨ ਲਿਖ)
{
    u32 ਅਧਾਰ = (ਲੰਮਾ)buf;
    s32 ਮਾਪ;

    ਜੇ (ਗਿਣਤੀ_2 > ns->max_req_ਮਾਪ)
        ਗਿਣਤੀ_2 = ns->max_req_ਮਾਪ;

    ਮਾਪ = ਗਿਣਤੀ_2 * ns->ਖੰਡ_ਮਾਪ;
    /* Special case for transfers that fit into PRP1, but are unaligned */
    ਜੇ (((ਮਾਪ + (ਅਧਾਰ & ~NVME_ਪੰਨਾ_ਨਕਾਬ)) <= NVME_ਪੰਨਾ_ਮਾਪ))
        ਜਾਓ single;

    /* Every request has to be page aligned */
    ਜੇ (ਅਧਾਰ & ~NVME_ਪੰਨਾ_ਨਕਾਬ)
        ਜਾਓ bounce;

    /* Make sure a full block fits into the last chunk */
    ਜੇ (ਮਾਪ & (ns->ਖੰਡ_ਮਾਪ - 1ULL))
        ਜਾਓ bounce;

    /* Build PRP list if we need to describe more than 2 pages */
    ਜੇ ((ns->ਖੰਡ_ਮਾਪ * ਗਿਣਤੀ_2) > (NVME_ਪੰਨਾ_ਮਾਪ * 2)) {
        u32 prpl_len = 0;
        u64 *prpl = nvme_dma_ਬਫਰ;
        ਪੂਰਨ ਪਹਿਲਾ_ਪੰਨਾ = 1;
        ਲਈ (; ਮਾਪ > 0; ਅਧਾਰ += NVME_ਪੰਨਾ_ਮਾਪ, ਮਾਪ -= NVME_ਪੰਨਾ_ਮਾਪ) {
            ਜੇ (ਪਹਿਲਾ_ਪੰਨਾ) {
                /* First page is special */
                ਪਹਿਲਾ_ਪੰਨਾ = 0;
                ਜਾਰੀ;
            }
            ਜੇ (prpl_len >= NVME_MAX_PRPL_ENTRIES)
                ਜਾਓ bounce;
            prpl[prpl_len++] = ਅਧਾਰ;
        }
        ਵਾਪਸ nvme_io_xfer(ns, lba, buf, prpl, ਗਿਣਤੀ_2, ਲਿਖ);
    }

    /* Directly embed the 2nd page if we only need 2 pages */
    ਜੇ ((ns->ਖੰਡ_ਮਾਪ * ਗਿਣਤੀ_2) > NVME_ਪੰਨਾ_ਮਾਪ)
        ਵਾਪਸ nvme_io_xfer(ns, lba, buf, buf + NVME_ਪੰਨਾ_ਮਾਪ, ਗਿਣਤੀ_2, ਲਿਖ);

single:
    /* One page is enough, don't expose anything else */
    ਵਾਪਸ nvme_io_xfer(ns, lba, buf, ਨਿਰਾ, ਗਿਣਤੀ_2, ਲਿਖ);

bounce:
    /* Use bounce buffer to make transfer */
    ਵਾਪਸ nvme_bounce_xfer(ns, lba, buf, ਗਿਣਤੀ_2, ਲਿਖ);
}

ਸਥਿਰ ਪੂਰਨ
nvme_create_io_queues(ਬਣਤਰ nvme_ctrl *ctrl)
{
    ਜੇ (nvme_create_io_cq(ctrl, &ctrl->io_cq, 3))
        ਜਾਓ err;

    ਜੇ (nvme_create_io_sq(ctrl, &ctrl->io_sq, 2, &ctrl->io_cq))
        ਜਾਓ err_free_cq;

    ਵਾਪਸ 0;

 err_free_cq:
    nvme_destroy_cq(&ctrl->io_cq);
 err:
    ਵਾਪਸ -1;
}

/* Waits for CSTS.RDY to match rdy. Returns 0 on success. */
ਸਥਿਰ ਪੂਰਨ
nvme_wait_csts_rdy(ਬਣਤਰ nvme_ctrl *ctrl, ਬਿਨਾਨਿਸ਼ਾਨੀ rdy)
{
    u32 ਅਚਲ max_ਮੰਜ਼ਿਲ = 500 /* ms */ * ((ctrl->reg->cap >> 24) & 0xFFU);
    u32 ਮੰਜ਼ਿਲ = timer_calc(max_ਮੰਜ਼ਿਲ);
    u32 csts;

    ਜਦੋਂ (rdy != ((csts = ctrl->reg->csts) & NVME_CSTS_RDY)) {
        yield();

        ਜੇ (csts & NVME_CSTS_FATAL) {
            dprintf(3, "NVMe fatal error during controller shutdown\n");
            ਵਾਪਸ -1;
        }

        ਜੇ (timer_check(ਮੰਜ਼ਿਲ)) {
            warn_timeout();
            ਵਾਪਸ -1;
        }
    }

    ਵਾਪਸ 0;
}

/* Returns 0 on success. */
ਸਥਿਰ ਪੂਰਨ
nvme_controller_enable(ਬਣਤਰ nvme_ctrl *ctrl)
{
    ਪੂਰਨ rc;

    pci_enable_busmaster(ctrl->pci);

    /* Turn the controller off. */
    ctrl->reg->cc = 0;
    ਜੇ (nvme_wait_csts_rdy(ctrl, 0)) {
        dprintf(2, "NVMe fatal error during controller shutdown\n");
        ਵਾਪਸ -1;
    }

    ctrl->doorbell_stride = 4U << ((ctrl->reg->cap >> 32) & 0xF);

    rc = nvme_init_cq(ctrl, &ctrl->admin_cq, 1,
                      NVME_ਪੰਨਾ_ਮਾਪ / ਆਕਾਰ(ਬਣਤਰ nvme_cqe));
    ਜੇ (rc) {
        ਵਾਪਸ -1;
    }

    rc = nvme_init_sq(ctrl, &ctrl->admin_sq, 0,
                      NVME_ਪੰਨਾ_ਮਾਪ / ਆਕਾਰ(ਬਣਤਰ nvme_sqe), &ctrl->admin_cq);
    ਜੇ (rc) {
        ਜਾਓ err_destroy_admin_cq;
    }

    ctrl->reg->aqa = ctrl->admin_cq.common.ਨਕਾਬ << 16
        | ctrl->admin_sq.common.ਨਕਾਬ;

    ctrl->reg->asq = (u32)ctrl->admin_sq.sqe;
    ctrl->reg->acq = (u32)ctrl->admin_cq.cqe;

    dprintf(3, "  admin submission queue: %p\n", ctrl->admin_sq.sqe);
    dprintf(3, "  admin completion queue: %p\n", ctrl->admin_cq.cqe);

    ctrl->reg->cc = NVME_CC_EN | (NVME_CQE_ਮਾਪ_LOG << 20)
        | (NVME_SQE_ਮਾਪ_LOG << 16 /* IOSQES */);

    ਜੇ (nvme_wait_csts_rdy(ctrl, 1)) {
        dprintf(2, "NVMe fatal error while enabling controller\n");
        ਜਾਓ err_destroy_admin_sq;
    }

    /* The admin queue is set up and the controller is ready. Let's figure out
       what namespaces we have. */

    ਬਣਤਰ nvme_identify_ctrl *identify = nvme_admin_identify_ctrl(ctrl);

    ਜੇ (!identify) {
        dprintf(2, "NVMe couldn't identify controller.\n");
        ਜਾਓ err_destroy_admin_sq;
    }

    dprintf(3, "NVMe has %u namespace%s.\n",
            identify->nn, (identify->nn == 1) ? "" : "s");

    ctrl->ns_ਗਿਣਤੀ = identify->nn;
    u8 mdts = identify->mdts;
    ਮੁਕਤ(identify);

    ਜੇ ((ctrl->ns_ਗਿਣਤੀ == 0) || nvme_create_io_queues(ctrl)) {
        /* No point to continue, if the controller says it doesn't have
           namespaces or we couldn't create I/O queues. */
        ਜਾਓ err_destroy_admin_sq;
    }

    /* Populate namespace IDs */
    ਪੂਰਨ ns_idx;
    ਲਈ (ns_idx = 0; ns_idx < ctrl->ns_ਗਿਣਤੀ; ns_idx++) {
        nvme_probe_ns(ctrl, ns_idx, mdts);
    }

    dprintf(3, "NVMe initialization complete!\n");
    ਵਾਪਸ 0;

 err_destroy_admin_sq:
    nvme_destroy_sq(&ctrl->admin_sq);
 err_destroy_admin_cq:
    nvme_destroy_cq(&ctrl->admin_cq);
    ਵਾਪਸ -1;
}

/* Initialize an NVMe controller and detect its drives. */
ਸਥਿਰ ਖਾਲੀ
nvme_controller_ਤਿਆਰੀ(ਖਾਲੀ *opaque)
{
    u8 skip_nonbootable = is_bootprio_strict();
    ਬਣਤਰ pci_device *pci = opaque;

    ਜੇ (skip_nonbootable && bootprio_ਲੱਭੋ_pci_device(pci) < 0) {
        dprintf(1, "skipping init of a non-bootable NVMe at %pP\n",
                pci);
        ਜਾਓ err;
    }

    ਬਣਤਰ nvme_reg ਅਸਥਿਰ *reg = pci_enable_membar(pci, PCI_ਅਧਾਰ_ਪਤਾ_0);
    ਜੇ (!reg)
        ਵਾਪਸ;

    u32 ਸੰਸਕਰਣ_2 = reg->vs;
    dprintf(3, "Found NVMe controller with version %u.%u.%u.\n",
            ਸੰਸਕਰਣ_2 >> 16, (ਸੰਸਕਰਣ_2 >> 8) & 0xFF, ਸੰਸਕਰਣ_2 & 0xFF);
    dprintf(3, "  Capabilities %016llx\n", reg->cap);

    ਜੇ (~reg->cap & NVME_CAP_CSS_NVME) {
        dprintf(3, "Controller doesn't speak NVMe command set. Skipping.\n");
        ਜਾਓ err;
    }

    ਬਣਤਰ nvme_ctrl *ctrl = malloc_ਉੱਚਾ(ਆਕਾਰ(*ctrl));
    ਜੇ (!ctrl) {
        warn_noalloc();
        ਜਾਓ err;
    }

    memset(ctrl, 0, ਆਕਾਰ(*ctrl));

    ctrl->reg = reg;
    ctrl->pci = pci;

    ਜੇ (nvme_controller_enable(ctrl)) {
        ਜਾਓ err_free_ctrl;
    }

    ਵਾਪਸ;

 err_free_ctrl:
    ਮੁਕਤ(ctrl);
 err:
    dprintf(2, "Failed to enable NVMe controller.\n");
}

// Locate and init NVMe controllers
ਸਥਿਰ ਖਾਲੀ
nvme_ਜਾਂਚ(ਖਾਲੀ)
{
    // Scan PCI bus for NVMe adapters
    ਬਣਤਰ pci_device *pci;

    foreachpci(pci) {
        ਜੇ (pci->class != PCI_CLASS_STORAGE_NVME)
            ਜਾਰੀ;
        ਜੇ (pci->prog_if != 2 /* as of NVM 1.0e */) {
            dprintf(3, "Found incompatble NVMe: prog-if=%02x\n", pci->prog_if);
            ਜਾਰੀ;
        }

        ਲੜੀਵਾਰ_thread(nvme_controller_ਤਿਆਰੀ, pci);
    }
}

ਸਥਿਰ ਪੂਰਨ
nvme_cmd_readwrite(ਬਣਤਰ nvme_namespace *ns, ਬਣਤਰ disk_op_s *op, ਪੂਰਨ ਲਿਖ)
{
    ਪੂਰਨ i;
    ਲਈ (i = 0; i < op->ਗਿਣਤੀ_2;) {
        u16 ਖੰਡਾਂ_ਬਚਿਆ = op->ਗਿਣਤੀ_2 - i;
        ਅੱਖਰ *op_buf = op->buf_fl + i * ns->ਖੰਡ_ਮਾਪ;
        ਪੂਰਨ ਖੰਡਾਂ = nvme_prpl_xfer(ns, op->lba + i, op_buf,
                                    ਖੰਡਾਂ_ਬਚਿਆ, ਲਿਖ);
        ਜੇ (ਖੰਡਾਂ < 0)
            ਵਾਪਸ DISK_RET_EBADTRACK;
        i += ਖੰਡਾਂ;
    }

    ਵਾਪਸ DISK_RET_SUCCESS;
}

ਪੂਰਨ
nvme_process_op(ਬਣਤਰ disk_op_s *op)
{
    ਜੇ (!CONFIG_NVME)
        ਵਾਪਸ DISK_RET_SUCCESS;

    ਬਣਤਰ nvme_namespace *ns = container_ਦਾ(op->drive_fl, ਬਣਤਰ nvme_namespace,
                                             drive);

    ਚੋਣ (op->command) {
    ਹਾਲਤ CMD_READ:
    ਹਾਲਤ CMD_WRITE:
        ਵਾਪਸ nvme_cmd_readwrite(ns, op, op->command == CMD_WRITE);
    ਮੂਲ:
        ਵਾਪਸ default_process_op(op);
    }
}

ਖਾਲੀ
nvme_ਤਿਆਰੀ(ਖਾਲੀ)
{
    ASSERT32FLAT();
    ਜੇ (!CONFIG_NVME)
        ਵਾਪਸ;

    dprintf(3, "init nvme\n");
    nvme_ਜਾਂਚ();
}

/* EOF */

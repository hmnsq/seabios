// Low level NVMe disk access
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "blockcmd.h"
#ಅನ್ತರ್ಭಾವ "malloc.h" // malloc_high
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pci.h"
#ಅನ್ತರ್ಭಾವ "pci_ids.h" // PCI_CLASS_STORAGE_NVME
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // boot_add_hd
#ಅನ್ತರ್ಭಾವ "x86.h" // readl

#ಅನ್ತರ್ಭಾವ "nvme.h"
#ಅನ್ತರ್ಭಾವ "nvme-int.h"

// Page aligned "dma bounce buffer" of size NVME_PAGE_SIZE in high memory
ಸ್ಥಿರ ಶೂನ್ಯ *nvme_dma_buffer;

ಸ್ಥಿರ ಶೂನ್ಯ *
zalloc_page_aligned(ಸಂರಚನಾ zone_s *zone, u32 size)
{
    ಶೂನ್ಯ *res = _malloc(zone, size, NVME_PAGE_SIZE);
    ಯದಿ (res) memset(res, 0, size);
    ಪ್ರತ್ಯಾಗಮ res;
}

ಸ್ಥಿರ ಶೂನ್ಯ
nvme_init_queue_common(ಸಂರಚನಾ nvme_ctrl *ctrl, ಸಂರಚನಾ nvme_queue *q, u16 q_idx,
                       u16 length)
{
    memset(q, 0, ಪರಿಮಾಣ(*q));
    q->dbl = (u32 *)((ವರ್ಣ *)ctrl->reg + 0x1000 + q_idx * ctrl->doorbell_stride);
    dprintf(3, " q %p q_idx %u dbl %p\n", q, q_idx, q->dbl);
    q->mask = length - 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
nvme_init_sq(ಸಂರಚನಾ nvme_ctrl *ctrl, ಸಂರಚನಾ nvme_sq *sq, u16 q_idx, u16 length,
             ಸಂರಚನಾ nvme_cq *cq)
{
    nvme_init_queue_common(ctrl, &sq->common, q_idx, length);
    sq->sqe = zalloc_page_aligned(&Zone_ಮೇಲಿನ, ಪರಿಮಾಣ(*sq->sqe) * length);

    ಯದಿ (!sq->sqe) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    dprintf(3, "sq %p q_idx %u sqe %p\n", sq, q_idx, sq->sqe);
    sq->cq   = cq;
    sq->head = 0;
    sq->tail = 0;

    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
nvme_init_cq(ಸಂರಚನಾ nvme_ctrl *ctrl, ಸಂರಚನಾ nvme_cq *cq, u16 q_idx, u16 length)
{
    nvme_init_queue_common(ctrl, &cq->common, q_idx, length);
    cq->cqe = zalloc_page_aligned(&Zone_ಮೇಲಿನ, ಪರಿಮಾಣ(*cq->cqe) * length);
    ಯದಿ (!cq->cqe) {
        warn_noalloc();
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    cq->head = 0;

    /* All CQE phase bits are initialized to zero. This means initially we wait
       for the host controller to set these to 1. */
    cq->phase = 1;

    ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
nvme_poll_cq(ಸಂರಚನಾ nvme_cq *cq)
{
    u32 dw3 = readl(&cq->cqe[cq->head].dword[3]);
    ಪ್ರತ್ಯಾಗಮ (!!(dw3 & NVME_CQE_DW3_P) == cq->phase);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
nvme_is_cqe_success(ಸಂರಚನಾ nvme_cqe ಧ್ರುವ *cqe)
{
    ಪ್ರತ್ಯಾಗಮ ((cqe->status >> 1) & 0xFF) == 0;
}

ಸ್ಥಿರ ಸಂರಚನಾ nvme_cqe
nvme_error_cqe(ಶೂನ್ಯ)
{
    ಸಂರಚನಾ nvme_cqe r;

    /* 0xFF is a vendor specific status code != success. Should be okay for
       indicating failure. */
    memset(&r, 0xFF, ಪರಿಮಾಣ(r));
    ಪ್ರತ್ಯಾಗಮ r;
}

ಸ್ಥಿರ ಸಂರಚನಾ nvme_cqe
nvme_consume_cqe(ಸಂರಚನಾ nvme_sq *sq)
{
    ಸಂರಚನಾ nvme_cq *cq = sq->cq;

    ಯದಿ (!nvme_poll_cq(cq)) {
        /* Cannot consume a completion queue entry, if there is none ready. */
        ಪ್ರತ್ಯಾಗಮ nvme_error_cqe();
    }

    ಸಂರಚನಾ nvme_cqe *cqe = &cq->cqe[cq->head];
    u16 cq_next_head = (cq->head + 1) & cq->common.mask;
    dprintf(4, "cq %p head %u -> %u\n", cq, cq->head, cq_next_head);
    ಯದಿ (cq_next_head < cq->head) {
        dprintf(3, "cq %p wrap\n", cq);
        cq->phase = ~cq->phase;
    }
    cq->head = cq_next_head;

    /* Update the submission queue head. */
    ಯದಿ (cqe->sq_head != sq->head) {
        sq->head = cqe->sq_head;
        dprintf(4, "sq %p advanced to %u\n", sq, cqe->sq_head);
    }

    /* Tell the controller that we consumed the completion. */
    writel(cq->common.dbl, cq->head);

    ಪ್ರತ್ಯಾಗಮ *cqe;
}

ಸ್ಥಿರ ಸಂರಚನಾ nvme_cqe
nvme_wait(ಸಂರಚನಾ nvme_sq *sq)
{
    ಸ್ಥಿರ ಧ್ರುವ unsigned nvme_timeout = 5000 /* ms */;
    u32 to = timer_calc(nvme_timeout);
    ಯಾವತ್ (!nvme_poll_cq(sq->cq)) {
        yield();

        ಯದಿ (timer_check(to)) {
            warn_timeout();
            ಪ್ರತ್ಯಾಗಮ nvme_error_cqe();
        }
    }

    ಪ್ರತ್ಯಾಗಮ nvme_consume_cqe(sq);
}

/* Returns the next submission queue entry (or NULL if the queue is full). It
   also fills out Command Dword 0 and clears the rest. */
ಸ್ಥಿರ ಸಂರಚನಾ nvme_sqe *
nvme_get_next_sqe(ಸಂರಚನಾ nvme_sq *sq, u8 opc, ಶೂನ್ಯ *metadata, ಶೂನ್ಯ *data, ಶೂನ್ಯ *data2)
{
    ಯದಿ (((sq->head + 1) & sq->common.mask) == sq->tail) {
        dprintf(3, "submission queue is full\n");
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }

    ಸಂರಚನಾ nvme_sqe *sqe = &sq->sqe[sq->tail];
    dprintf(4, "sq %p next_sqe %u\n", sq, sq->tail);

    memset(sqe, 0, ಪರಿಮಾಣ(*sqe));
    sqe->cdw0 = opc | (sq->tail << 16 /* CID */);
    sqe->mptr = (u32)metadata;
    sqe->dptr_prp1 = (u32)data;
    sqe->dptr_prp2 = (u32)data2;

    ಪ್ರತ್ಯಾಗಮ sqe;
}

/* Call this after you've filled out an sqe that you've got from nvme_get_next_sqe. */
ಸ್ಥಿರ ಶೂನ್ಯ
nvme_commit_sqe(ಸಂರಚನಾ nvme_sq *sq)
{
    dprintf(4, "sq %p commit_sqe %u\n", sq, sq->tail);
    sq->tail = (sq->tail + 1) & sq->common.mask;
    writel(sq->common.dbl, sq->tail);
}

/* Perform an identify command on the admin queue and return the resulting
   buffer. This may be a NULL pointer, if something failed. This function
   cannot be used after initialization, because it uses buffers in tmp zone. */
ಸ್ಥಿರ ಸಙ್ಘ nvme_identify *
nvme_admin_identify(ಸಂರಚನಾ nvme_ctrl *ctrl, u8 cns, u32 nsid)
{
    ಸಙ್ಘ nvme_identify *identify_buf = zalloc_page_aligned(&Zone_Tmp_ಮೇಲಿನ, 4096);
    ಯದಿ (!identify_buf) {
        /* Could not allocate identify buffer. */
        warn_internalerror();
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }

    ಸಂರಚನಾ nvme_sqe *cmd_identify;
    cmd_identify = nvme_get_next_sqe(&ctrl->admin_sq,
                                     NVME_SQE_OPC_ADMIN_IDENTIFY, ರಿಕ್ತ,
                                     identify_buf, ರಿಕ್ತ);

    ಯದಿ (!cmd_identify) {
        warn_internalerror();
        ಗಚ್ಛ error;
    }

    cmd_identify->nsid = nsid;
    cmd_identify->dword[10] = cns;

    nvme_commit_sqe(&ctrl->admin_sq);

    ಸಂರಚನಾ nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    ಯದಿ (!nvme_is_cqe_success(&cqe)) {
        ಗಚ್ಛ error;
    }

    ಪ್ರತ್ಯಾಗಮ identify_buf;
 error:
    ಮುಕ್ತ(identify_buf);
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸ್ಥಿರ ಸಂರಚನಾ nvme_identify_ctrl *
nvme_admin_identify_ctrl(ಸಂರಚನಾ nvme_ctrl *ctrl)
{
    ಪ್ರತ್ಯಾಗಮ &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_CTRL, 0)->ctrl;
}

ಸ್ಥಿರ ಸಂರಚನಾ nvme_identify_ns *
nvme_admin_identify_ns(ಸಂರಚನಾ nvme_ctrl *ctrl, u32 ns_id)
{
    ಪ್ರತ್ಯಾಗಮ &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_NS,
                                ns_id)->ns;
}

ಸ್ಥಿರ ಶೂನ್ಯ
nvme_probe_ns(ಸಂರಚನಾ nvme_ctrl *ctrl, u32 ns_idx, u8 mdts)
{
    u32 ns_id = ns_idx + 1;

    ಸಂರಚನಾ nvme_identify_ns *id = nvme_admin_identify_ns(ctrl, ns_id);
    ಯದಿ (!id) {
        dprintf(2, "NVMe couldn't identify namespace %u.\n", ns_id);
        ಗಚ್ಛ free_buffer;
    }

    u8 current_lba_format = id->flbas & 0xF;
    ಯದಿ (current_lba_format > id->nlbaf) {
        dprintf(2, "NVMe NS %u: current LBA format %u is beyond what the "
                " namespace supports (%u)?\n",
                ns_id, current_lba_format, id->nlbaf + 1);
        ಗಚ್ಛ free_buffer;
    }

    ಯದಿ (!id->nsze) {
        dprintf(2, "NVMe NS %u is inactive.\n", ns_id);
        ಗಚ್ಛ free_buffer;
    }

    ಯದಿ (!nvme_dma_buffer) {
        nvme_dma_buffer = zalloc_page_aligned(&Zone_ಮೇಲಿನ, NVME_PAGE_SIZE);
        ಯದಿ (!nvme_dma_buffer) {
            warn_noalloc();
            ಗಚ್ಛ free_buffer;
        }
    }

    ಸಂರಚನಾ nvme_namespace *ns = malloc_fseg(ಪರಿಮಾಣ(*ns));
    ಯದಿ (!ns) {
        warn_noalloc();
        ಗಚ್ಛ free_buffer;
    }
    memset(ns, 0, ಪರಿಮಾಣ(*ns));
    ns->ctrl  = ctrl;
    ns->ns_id = ns_id;
    ns->lba_ಎಣಿಕೆ = id->nsze;

    ಸಂರಚನಾ nvme_lba_format *fmt = &id->lbaf[current_lba_format];

    ns->block_size    = 1U << fmt->lbads;
    ns->metadata_size = fmt->ms;

    ಯದಿ (ns->block_size > NVME_PAGE_SIZE) {
        /* If we see devices that trigger this path, we need to increase our
           buffer size. */
        warn_internalerror();
        ಮುಕ್ತ(ns);
        ಗಚ್ಛ free_buffer;
    }

    ns->drive.cntl_id   = ns_idx;
    ns->drive.removable = 0;
    ns->drive.type      = DTYPE_NVME;
    ns->drive.blksize   = ns->block_size;
    ns->drive.sectors   = ns->lba_ಎಣಿಕೆ;

    ಯದಿ (mdts) {
        ns->max_req_size = ((1U << mdts) * NVME_PAGE_SIZE) / ns->block_size;
        dprintf(3, "NVME NS %u max request size: %d sectors\n",
                ns_id, ns->max_req_size);
    } ಅನ್ಯಥಾ {
        ns->max_req_size = -1U;
    }

    ವರ್ಣ *desc = znprintf(MAXDESCSIZE, "NVMe NS %u: %llu MiB (%llu %u-byte "
                          "blocks + %u-byte metadata)",
                          ns_id, (ns->lba_ಎಣಿಕೆ * ns->block_size) >> 20,
                          ns->lba_ಎಣಿಕೆ, ns->block_size, ns->metadata_size);

    dprintf(3, "%s\n", desc);
    boot_add_hd(&ns->drive, desc, bootprio_find_pci_device(ctrl->pci));

free_buffer:
    ಮುಕ್ತ (id);
}


/* Release memory allocated for a completion queue */
ಸ್ಥಿರ ಶೂನ್ಯ
nvme_destroy_cq(ಸಂರಚನಾ nvme_cq *cq)
{
    ಮುಕ್ತ(cq->cqe);
    cq->cqe = ರಿಕ್ತ;
}

/* Release memory allocated for a submission queue */
ಸ್ಥಿರ ಶೂನ್ಯ
nvme_destroy_sq(ಸಂರಚನಾ nvme_sq *sq)
{
    ಮುಕ್ತ(sq->sqe);
    sq->sqe = ರಿಕ್ತ;
}

/* Returns 0 on success. */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
nvme_create_io_cq(ಸಂರಚನಾ nvme_ctrl *ctrl, ಸಂರಚನಾ nvme_cq *cq, u16 q_idx)
{
    ಪೂರ್ಣಾಙ್ಕ rc;
    ಸಂರಚನಾ nvme_sqe *cmd_create_cq;
    u32 length = 1 + (ctrl->reg->cap & 0xffff);
    ಯದಿ (length > NVME_PAGE_SIZE / ಪರಿಮಾಣ(ಸಂರಚನಾ nvme_cqe))
        length = NVME_PAGE_SIZE / ಪರಿಮಾಣ(ಸಂರಚನಾ nvme_cqe);

    rc = nvme_init_cq(ctrl, cq, q_idx, length);
    ಯದಿ (rc) {
        ಗಚ್ಛ err;
    }

    cmd_create_cq = nvme_get_next_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_CQ, ರಿಕ್ತ,
                                      cq->cqe, ರಿಕ್ತ);
    ಯದಿ (!cmd_create_cq) {
        ಗಚ್ಛ err_destroy_cq;
    }

    cmd_create_cq->dword[10] = (cq->common.mask << 16) | (q_idx >> 1);
    cmd_create_cq->dword[11] = 1 /* physically contiguous */;

    nvme_commit_sqe(&ctrl->admin_sq);

    ಸಂರಚನಾ nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    ಯದಿ (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io cq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        ಗಚ್ಛ err_destroy_cq;
    }

    ಪ್ರತ್ಯಾಗಮ 0;

err_destroy_cq:
    nvme_destroy_cq(cq);
err:
    ಪ್ರತ್ಯಾಗಮ -1;
}

/* Returns 0 on success. */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
nvme_create_io_sq(ಸಂರಚನಾ nvme_ctrl *ctrl, ಸಂರಚನಾ nvme_sq *sq, u16 q_idx, ಸಂರಚನಾ nvme_cq *cq)
{
    ಪೂರ್ಣಾಙ್ಕ rc;
    ಸಂರಚನಾ nvme_sqe *cmd_create_sq;
    u32 length = 1 + (ctrl->reg->cap & 0xffff);
    ಯದಿ (length > NVME_PAGE_SIZE / ಪರಿಮಾಣ(ಸಂರಚನಾ nvme_cqe))
        length = NVME_PAGE_SIZE / ಪರಿಮಾಣ(ಸಂರಚನಾ nvme_cqe);

    rc = nvme_init_sq(ctrl, sq, q_idx, length, cq);
    ಯದಿ (rc) {
        ಗಚ್ಛ err;
    }

    cmd_create_sq = nvme_get_next_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_SQ, ರಿಕ್ತ,
                                      sq->sqe, ರಿಕ್ತ);
    ಯದಿ (!cmd_create_sq) {
        ಗಚ್ಛ err_destroy_sq;
    }

    cmd_create_sq->dword[10] = (sq->common.mask << 16) | (q_idx >> 1);
    cmd_create_sq->dword[11] = (q_idx >> 1) << 16 | 1 /* contiguous */;
    dprintf(3, "sq %p create dword10 %08x dword11 %08x\n", sq,
            cmd_create_sq->dword[10], cmd_create_sq->dword[11]);

    nvme_commit_sqe(&ctrl->admin_sq);

    ಸಂರಚನಾ nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    ಯದಿ (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io sq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);
        ಗಚ್ಛ err_destroy_sq;
    }

    ಪ್ರತ್ಯಾಗಮ 0;

err_destroy_sq:
    nvme_destroy_sq(sq);
err:
    ಪ್ರತ್ಯಾಗಮ -1;
}

/* Reads count sectors into buf. The buffer cannot cross page boundaries. */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
nvme_io_xfer(ಸಂರಚನಾ nvme_namespace *ns, u64 lba, ಶೂನ್ಯ *prp1, ಶೂನ್ಯ *prp2,
             u16 ಎಣಿಕೆ, ಪೂರ್ಣಾಙ್ಕ ಲಿಖ)
{
    ಯದಿ (((u32)prp1 & 0x3) || ((u32)prp2 & 0x3)) {
        /* Buffer is misaligned */
        warn_internalerror();
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    ಸಂರಚನಾ nvme_sqe *io_read = nvme_get_next_sqe(&ns->ctrl->io_sq,
                                                 ಲಿಖ ? NVME_SQE_OPC_IO_WRITE
                                                       : NVME_SQE_OPC_IO_READ,
                                                 ರಿಕ್ತ, prp1, prp2);
    io_read->nsid = ns->ns_id;
    io_read->dword[10] = (u32)lba;
    io_read->dword[11] = (u32)(lba >> 32);
    io_read->dword[12] = (1U << 31 /* limited retry */) | (ಎಣಿಕೆ - 1);

    nvme_commit_sqe(&ns->ctrl->io_sq);

    ಸಂರಚನಾ nvme_cqe cqe = nvme_wait(&ns->ctrl->io_sq);

    ಯದಿ (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "read io: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        ಪ್ರತ್ಯಾಗಮ -1;
    }

    dprintf(5, "ns %u %s lba %llu+%u\n", ns->ns_id, ಲಿಖ ? "write" : "read",
            lba, ಎಣಿಕೆ);
    ಪ್ರತ್ಯಾಗಮ ಎಣಿಕೆ;
}

// Transfer up to one page of data using the internal dma bounce buffer
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
nvme_bounce_xfer(ಸಂರಚನಾ nvme_namespace *ns, u64 lba, ಶೂನ್ಯ *buf, u16 ಎಣಿಕೆ,
                 ಪೂರ್ಣಾಙ್ಕ ಲಿಖ)
{
    u16 ಧ್ರುವ max_blocks = NVME_PAGE_SIZE / ns->block_size;
    u16 blocks = ಎಣಿಕೆ < max_blocks ? ಎಣಿಕೆ : max_blocks;

    ಯದಿ (ಲಿಖ)
        memcpy(nvme_dma_buffer, buf, blocks * ns->block_size);

    ಪೂರ್ಣಾಙ್ಕ res = nvme_io_xfer(ns, lba, nvme_dma_buffer, ರಿಕ್ತ, blocks, ಲಿಖ);

    ಯದಿ (!ಲಿಖ && res >= 0)
        memcpy(buf, nvme_dma_buffer, res * ns->block_size);

    ಪ್ರತ್ಯಾಗಮ res;
}

#define NVME_MAX_PRPL_ENTRIES 15 /* Allows requests up to 64kb */

// Transfer data using page list (if applicable)
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
nvme_prpl_xfer(ಸಂರಚನಾ nvme_namespace *ns, u64 lba, ಶೂನ್ಯ *buf, u16 ಎಣಿಕೆ,
               ಪೂರ್ಣಾಙ್ಕ ಲಿಖ)
{
    u32 base = (long)buf;
    s32 size;

    ಯದಿ (ಎಣಿಕೆ > ns->max_req_size)
        ಎಣಿಕೆ = ns->max_req_size;

    size = ಎಣಿಕೆ * ns->block_size;
    /* Special case for transfers that fit into PRP1, but are unaligned */
    ಯದಿ (((size + (base & ~NVME_PAGE_MASK)) <= NVME_PAGE_SIZE))
        ಗಚ್ಛ single;

    /* Every request has to be page aligned */
    ಯದಿ (base & ~NVME_PAGE_MASK)
        ಗಚ್ಛ bounce;

    /* Make sure a full block fits into the last chunk */
    ಯದಿ (size & (ns->block_size - 1ULL))
        ಗಚ್ಛ bounce;

    /* Build PRP list if we need to describe more than 2 pages */
    ಯದಿ ((ns->block_size * ಎಣಿಕೆ) > (NVME_PAGE_SIZE * 2)) {
        u32 prpl_len = 0;
        u64 *prpl = nvme_dma_buffer;
        ಪೂರ್ಣಾಙ್ಕ first_page = 1;
        ಪ್ರತಿ (; size > 0; base += NVME_PAGE_SIZE, size -= NVME_PAGE_SIZE) {
            ಯದಿ (first_page) {
                /* First page is special */
                first_page = 0;
                ಅನುವರ್ತ;
            }
            ಯದಿ (prpl_len >= NVME_MAX_PRPL_ENTRIES)
                ಗಚ್ಛ bounce;
            prpl[prpl_len++] = base;
        }
        ಪ್ರತ್ಯಾಗಮ nvme_io_xfer(ns, lba, buf, prpl, ಎಣಿಕೆ, ಲಿಖ);
    }

    /* Directly embed the 2nd page if we only need 2 pages */
    ಯದಿ ((ns->block_size * ಎಣಿಕೆ) > NVME_PAGE_SIZE)
        ಪ್ರತ್ಯಾಗಮ nvme_io_xfer(ns, lba, buf, buf + NVME_PAGE_SIZE, ಎಣಿಕೆ, ಲಿಖ);

single:
    /* One page is enough, don't expose anything else */
    ಪ್ರತ್ಯಾಗಮ nvme_io_xfer(ns, lba, buf, ರಿಕ್ತ, ಎಣಿಕೆ, ಲಿಖ);

bounce:
    /* Use bounce buffer to make transfer */
    ಪ್ರತ್ಯಾಗಮ nvme_bounce_xfer(ns, lba, buf, ಎಣಿಕೆ, ಲಿಖ);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
nvme_create_io_queues(ಸಂರಚನಾ nvme_ctrl *ctrl)
{
    ಯದಿ (nvme_create_io_cq(ctrl, &ctrl->io_cq, 3))
        ಗಚ್ಛ err;

    ಯದಿ (nvme_create_io_sq(ctrl, &ctrl->io_sq, 2, &ctrl->io_cq))
        ಗಚ್ಛ err_free_cq;

    ಪ್ರತ್ಯಾಗಮ 0;

 err_free_cq:
    nvme_destroy_cq(&ctrl->io_cq);
 err:
    ಪ್ರತ್ಯಾಗಮ -1;
}

/* Waits for CSTS.RDY to match rdy. Returns 0 on success. */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
nvme_wait_csts_rdy(ಸಂರಚನಾ nvme_ctrl *ctrl, unsigned rdy)
{
    u32 ಧ್ರುವ max_to = 500 /* ms */ * ((ctrl->reg->cap >> 24) & 0xFFU);
    u32 to = timer_calc(max_to);
    u32 csts;

    ಯಾವತ್ (rdy != ((csts = ctrl->reg->csts) & NVME_CSTS_RDY)) {
        yield();

        ಯದಿ (csts & NVME_CSTS_FATAL) {
            dprintf(3, "NVMe fatal error during controller shutdown\n");
            ಪ್ರತ್ಯಾಗಮ -1;
        }

        ಯದಿ (timer_check(to)) {
            warn_timeout();
            ಪ್ರತ್ಯಾಗಮ -1;
        }
    }

    ಪ್ರತ್ಯಾಗಮ 0;
}

/* Returns 0 on success. */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
nvme_controller_enable(ಸಂರಚನಾ nvme_ctrl *ctrl)
{
    ಪೂರ್ಣಾಙ್ಕ rc;

    pci_enable_busmaster(ctrl->pci);

    /* Turn the controller off. */
    ctrl->reg->cc = 0;
    ಯದಿ (nvme_wait_csts_rdy(ctrl, 0)) {
        dprintf(2, "NVMe fatal error during controller shutdown\n");
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    ctrl->doorbell_stride = 4U << ((ctrl->reg->cap >> 32) & 0xF);

    rc = nvme_init_cq(ctrl, &ctrl->admin_cq, 1,
                      NVME_PAGE_SIZE / ಪರಿಮಾಣ(ಸಂರಚನಾ nvme_cqe));
    ಯದಿ (rc) {
        ಪ್ರತ್ಯಾಗಮ -1;
    }

    rc = nvme_init_sq(ctrl, &ctrl->admin_sq, 0,
                      NVME_PAGE_SIZE / ಪರಿಮಾಣ(ಸಂರಚನಾ nvme_sqe), &ctrl->admin_cq);
    ಯದಿ (rc) {
        ಗಚ್ಛ err_destroy_admin_cq;
    }

    ctrl->reg->aqa = ctrl->admin_cq.common.mask << 16
        | ctrl->admin_sq.common.mask;

    ctrl->reg->asq = (u32)ctrl->admin_sq.sqe;
    ctrl->reg->acq = (u32)ctrl->admin_cq.cqe;

    dprintf(3, "  admin submission queue: %p\n", ctrl->admin_sq.sqe);
    dprintf(3, "  admin completion queue: %p\n", ctrl->admin_cq.cqe);

    ctrl->reg->cc = NVME_CC_EN | (NVME_CQE_SIZE_LOG << 20)
        | (NVME_SQE_SIZE_LOG << 16 /* IOSQES */);

    ಯದಿ (nvme_wait_csts_rdy(ctrl, 1)) {
        dprintf(2, "NVMe fatal error while enabling controller\n");
        ಗಚ್ಛ err_destroy_admin_sq;
    }

    /* The admin queue is set up and the controller is ready. Let's figure out
       what namespaces we have. */

    ಸಂರಚನಾ nvme_identify_ctrl *identify = nvme_admin_identify_ctrl(ctrl);

    ಯದಿ (!identify) {
        dprintf(2, "NVMe couldn't identify controller.\n");
        ಗಚ್ಛ err_destroy_admin_sq;
    }

    dprintf(3, "NVMe has %u namespace%s.\n",
            identify->nn, (identify->nn == 1) ? "" : "s");

    ctrl->ns_ಎಣಿಕೆ = identify->nn;
    u8 mdts = identify->mdts;
    ಮುಕ್ತ(identify);

    ಯದಿ ((ctrl->ns_ಎಣಿಕೆ == 0) || nvme_create_io_queues(ctrl)) {
        /* No point to continue, if the controller says it doesn't have
           namespaces or we couldn't create I/O queues. */
        ಗಚ್ಛ err_destroy_admin_sq;
    }

    /* Populate namespace IDs */
    ಪೂರ್ಣಾಙ್ಕ ns_idx;
    ಪ್ರತಿ (ns_idx = 0; ns_idx < ctrl->ns_ಎಣಿಕೆ; ns_idx++) {
        nvme_probe_ns(ctrl, ns_idx, mdts);
    }

    dprintf(3, "NVMe initialization complete!\n");
    ಪ್ರತ್ಯಾಗಮ 0;

 err_destroy_admin_sq:
    nvme_destroy_sq(&ctrl->admin_sq);
 err_destroy_admin_cq:
    nvme_destroy_cq(&ctrl->admin_cq);
    ಪ್ರತ್ಯಾಗಮ -1;
}

/* Initialize an NVMe controller and detect its drives. */
ಸ್ಥಿರ ಶೂನ್ಯ
nvme_controller_setup(ಶೂನ್ಯ *opaque)
{
    u8 skip_nonbootable = is_bootprio_strict();
    ಸಂರಚನಾ pci_device *pci = opaque;

    ಯದಿ (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
        dprintf(1, "skipping init of a non-bootable NVMe at %pP\n",
                pci);
        ಗಚ್ಛ err;
    }

    ಸಂರಚನಾ nvme_reg ಚಞ್ಚಲ *reg = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    ಯದಿ (!reg)
        ಪ್ರತ್ಯಾಗಮ;

    u32 version = reg->vs;
    dprintf(3, "Found NVMe controller with version %u.%u.%u.\n",
            version >> 16, (version >> 8) & 0xFF, version & 0xFF);
    dprintf(3, "  Capabilities %016llx\n", reg->cap);

    ಯದಿ (~reg->cap & NVME_CAP_CSS_NVME) {
        dprintf(3, "Controller doesn't speak NVMe command set. Skipping.\n");
        ಗಚ್ಛ err;
    }

    ಸಂರಚನಾ nvme_ctrl *ctrl = malloc_ಮೇಲಿನ(ಪರಿಮಾಣ(*ctrl));
    ಯದಿ (!ctrl) {
        warn_noalloc();
        ಗಚ್ಛ err;
    }

    memset(ctrl, 0, ಪರಿಮಾಣ(*ctrl));

    ctrl->reg = reg;
    ctrl->pci = pci;

    ಯದಿ (nvme_controller_enable(ctrl)) {
        ಗಚ್ಛ err_free_ctrl;
    }

    ಪ್ರತ್ಯಾಗಮ;

 err_free_ctrl:
    ಮುಕ್ತ(ctrl);
 err:
    dprintf(2, "Failed to enable NVMe controller.\n");
}

// Locate and init NVMe controllers
ಸ್ಥಿರ ಶೂನ್ಯ
nvme_scan(ಶೂನ್ಯ)
{
    // Scan PCI bus for NVMe adapters
    ಸಂರಚನಾ pci_device *pci;

    foreachpci(pci) {
        ಯದಿ (pci->class != PCI_CLASS_STORAGE_NVME)
            ಅನುವರ್ತ;
        ಯದಿ (pci->prog_if != 2 /* as of NVM 1.0e */) {
            dprintf(3, "Found incompatble NVMe: prog-if=%02x\n", pci->prog_if);
            ಅನುವರ್ತ;
        }

        run_thread(nvme_controller_setup, pci);
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
nvme_cmd_readwrite(ಸಂರಚನಾ nvme_namespace *ns, ಸಂರಚನಾ disk_op_s *op, ಪೂರ್ಣಾಙ್ಕ ಲಿಖ)
{
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i = 0; i < op->ಎಣಿಕೆ;) {
        u16 blocks_remaining = op->ಎಣಿಕೆ - i;
        ವರ್ಣ *op_buf = op->buf_fl + i * ns->block_size;
        ಪೂರ್ಣಾಙ್ಕ blocks = nvme_prpl_xfer(ns, op->lba + i, op_buf,
                                    blocks_remaining, ಲಿಖ);
        ಯದಿ (blocks < 0)
            ಪ್ರತ್ಯಾಗಮ DISK_RET_EBADTRACK;
        i += blocks;
    }

    ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;
}

ಪೂರ್ಣಾಙ್ಕ
nvme_process_op(ಸಂರಚನಾ disk_op_s *op)
{
    ಯದಿ (!CONFIG_NVME)
        ಪ್ರತ್ಯಾಗಮ DISK_RET_SUCCESS;

    ಸಂರಚನಾ nvme_namespace *ns = container_of(op->drive_fl, ಸಂರಚನಾ nvme_namespace,
                                             drive);

    ವಿಕಲ್ಪ (op->command) {
    ಪಕ್ಷ CMD_READ:
    ಪಕ್ಷ CMD_WRITE:
        ಪ್ರತ್ಯಾಗಮ nvme_cmd_readwrite(ns, op, op->command == CMD_WRITE);
    ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ default_process_op(op);
    }
}

ಶೂನ್ಯ
nvme_setup(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಯದಿ (!CONFIG_NVME)
        ಪ್ರತ್ಯಾಗಮ;

    dprintf(3, "init nvme\n");
    nvme_scan();
}

/* EOF */

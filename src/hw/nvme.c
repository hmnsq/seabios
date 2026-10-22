// Low level NVMe disk access
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "blockcmd.h"
#અન્તર્ભાવ "malloc.h" // malloc_high
#અન્તર્ભાવ "output.h" // dprintf
#અન્તર્ભાવ "pci.h"
#અન્તર્ભાવ "pci_ids.h" // PCI_CLASS_STORAGE_NVME
#અન્તર્ભાવ "pci_regs.h" // PCI_BASE_ADDRESS_0
#અન્તર્ભાવ "pcidevice.h" // foreachpci
#અન્તર્ભાવ "stacks.h" // yield
#અન્તર્ભાવ "std/disk.h" // DISK_RET_
#અન્તર્ભાવ "string.h" // memset
#અન્તર્ભાવ "util.h" // boot_add_hd
#અન્તર્ભાવ "x86.h" // readl

#અન્તર્ભાવ "nvme.h"
#અન્તર્ભાવ "nvme-int.h"

// Page aligned "dma bounce buffer" of size NVME_PAGE_SIZE in high memory
સ્થિર શૂન્ય *nvme_dma_buffer;

સ્થિર શૂન્ય *
zalloc_page_aligned(સંરચના zone_s *zone, u32 size)
{
    શૂન્ય *res = _malloc(zone, size, NVME_PAGE_SIZE);
    જો (res) memset(res, 0, size);
    પાછા res;
}

સ્થિર શૂન્ય
nvme_init_queue_common(સંરચના nvme_ctrl *ctrl, સંરચના nvme_queue *q, u16 q_idx,
                       u16 length)
{
    memset(q, 0, કદ(*q));
    q->dbl = (u32 *)((અક્ષર *)ctrl->reg + 0x1000 + q_idx * ctrl->doorbell_stride);
    dprintf(3, " q %p q_idx %u dbl %p\n", q, q_idx, q->dbl);
    q->mask = length - 1;
}

સ્થિર પૂર્ણાંક
nvme_init_sq(સંરચના nvme_ctrl *ctrl, સંરચના nvme_sq *sq, u16 q_idx, u16 length,
             સંરચના nvme_cq *cq)
{
    nvme_init_queue_common(ctrl, &sq->common, q_idx, length);
    sq->sqe = zalloc_page_aligned(&Zone_ઉપલું, કદ(*sq->sqe) * length);

    જો (!sq->sqe) {
        warn_noalloc();
        પાછા -1;
    }

    dprintf(3, "sq %p q_idx %u sqe %p\n", sq, q_idx, sq->sqe);
    sq->cq   = cq;
    sq->head = 0;
    sq->tail = 0;

    પાછા 0;
}

સ્થિર પૂર્ણાંક
nvme_init_cq(સંરચના nvme_ctrl *ctrl, સંરચના nvme_cq *cq, u16 q_idx, u16 length)
{
    nvme_init_queue_common(ctrl, &cq->common, q_idx, length);
    cq->cqe = zalloc_page_aligned(&Zone_ઉપલું, કદ(*cq->cqe) * length);
    જો (!cq->cqe) {
        warn_noalloc();
        પાછા -1;
    }

    cq->head = 0;

    /* All CQE phase bits are initialized to zero. This means initially we wait
       for the host controller to set these to 1. */
    cq->phase = 1;

    પાછા 0;
}

સ્થિર પૂર્ણાંક
nvme_poll_cq(સંરચના nvme_cq *cq)
{
    u32 dw3 = readl(&cq->cqe[cq->head].dword[3]);
    પાછા (!!(dw3 & NVME_CQE_DW3_P) == cq->phase);
}

સ્થિર પૂર્ણાંક
nvme_is_cqe_success(સંરચના nvme_cqe અચલ *cqe)
{
    પાછા ((cqe->status >> 1) & 0xFF) == 0;
}

સ્થિર સંરચના nvme_cqe
nvme_error_cqe(શૂન્ય)
{
    સંરચના nvme_cqe r;

    /* 0xFF is a vendor specific status code != success. Should be okay for
       indicating failure. */
    memset(&r, 0xFF, કદ(r));
    પાછા r;
}

સ્થિર સંરચના nvme_cqe
nvme_consume_cqe(સંરચના nvme_sq *sq)
{
    સંરચના nvme_cq *cq = sq->cq;

    જો (!nvme_poll_cq(cq)) {
        /* Cannot consume a completion queue entry, if there is none ready. */
        પાછા nvme_error_cqe();
    }

    સંરચના nvme_cqe *cqe = &cq->cqe[cq->head];
    u16 cq_next_head = (cq->head + 1) & cq->common.mask;
    dprintf(4, "cq %p head %u -> %u\n", cq, cq->head, cq_next_head);
    જો (cq_next_head < cq->head) {
        dprintf(3, "cq %p wrap\n", cq);
        cq->phase = ~cq->phase;
    }
    cq->head = cq_next_head;

    /* Update the submission queue head. */
    જો (cqe->sq_head != sq->head) {
        sq->head = cqe->sq_head;
        dprintf(4, "sq %p advanced to %u\n", sq, cqe->sq_head);
    }

    /* Tell the controller that we consumed the completion. */
    writel(cq->common.dbl, cq->head);

    પાછા *cqe;
}

સ્થિર સંરચના nvme_cqe
nvme_wait(સંરચના nvme_sq *sq)
{
    સ્થિર અચલ unsigned nvme_timeout = 5000 /* ms */;
    u32 to = timer_calc(nvme_timeout);
    જ્યારે (!nvme_poll_cq(sq->cq)) {
        yield();

        જો (timer_check(to)) {
            warn_timeout();
            પાછા nvme_error_cqe();
        }
    }

    પાછા nvme_consume_cqe(sq);
}

/* Returns the next submission queue entry (or NULL if the queue is full). It
   also fills out Command Dword 0 and clears the rest. */
સ્થિર સંરચના nvme_sqe *
nvme_get_next_sqe(સંરચના nvme_sq *sq, u8 opc, શૂન્ય *metadata, શૂન્ય *data, શૂન્ય *data2)
{
    જો (((sq->head + 1) & sq->common.mask) == sq->tail) {
        dprintf(3, "submission queue is full\n");
        પાછા ખાલી;
    }

    સંરચના nvme_sqe *sqe = &sq->sqe[sq->tail];
    dprintf(4, "sq %p next_sqe %u\n", sq, sq->tail);

    memset(sqe, 0, કદ(*sqe));
    sqe->cdw0 = opc | (sq->tail << 16 /* CID */);
    sqe->mptr = (u32)metadata;
    sqe->dptr_prp1 = (u32)data;
    sqe->dptr_prp2 = (u32)data2;

    પાછા sqe;
}

/* Call this after you've filled out an sqe that you've got from nvme_get_next_sqe. */
સ્થિર શૂન્ય
nvme_commit_sqe(સંરચના nvme_sq *sq)
{
    dprintf(4, "sq %p commit_sqe %u\n", sq, sq->tail);
    sq->tail = (sq->tail + 1) & sq->common.mask;
    writel(sq->common.dbl, sq->tail);
}

/* Perform an identify command on the admin queue and return the resulting
   buffer. This may be a NULL pointer, if something failed. This function
   cannot be used after initialization, because it uses buffers in tmp zone. */
સ્થિર સંઘ nvme_identify *
nvme_admin_identify(સંરચના nvme_ctrl *ctrl, u8 cns, u32 nsid)
{
    સંઘ nvme_identify *identify_buf = zalloc_page_aligned(&Zone_Tmp_ઉપલું, 4096);
    જો (!identify_buf) {
        /* Could not allocate identify buffer. */
        warn_internalerror();
        પાછા ખાલી;
    }

    સંરચના nvme_sqe *cmd_identify;
    cmd_identify = nvme_get_next_sqe(&ctrl->admin_sq,
                                     NVME_SQE_OPC_ADMIN_IDENTIFY, ખાલી,
                                     identify_buf, ખાલી);

    જો (!cmd_identify) {
        warn_internalerror();
        જાઓ error;
    }

    cmd_identify->nsid = nsid;
    cmd_identify->dword[10] = cns;

    nvme_commit_sqe(&ctrl->admin_sq);

    સંરચના nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    જો (!nvme_is_cqe_success(&cqe)) {
        જાઓ error;
    }

    પાછા identify_buf;
 error:
    મુક્ત(identify_buf);
    પાછા ખાલી;
}

સ્થિર સંરચના nvme_identify_ctrl *
nvme_admin_identify_ctrl(સંરચના nvme_ctrl *ctrl)
{
    પાછા &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_CTRL, 0)->ctrl;
}

સ્થિર સંરચના nvme_identify_ns *
nvme_admin_identify_ns(સંરચના nvme_ctrl *ctrl, u32 ns_id)
{
    પાછા &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_NS,
                                ns_id)->ns;
}

સ્થિર શૂન્ય
nvme_probe_ns(સંરચના nvme_ctrl *ctrl, u32 ns_idx, u8 mdts)
{
    u32 ns_id = ns_idx + 1;

    સંરચના nvme_identify_ns *id = nvme_admin_identify_ns(ctrl, ns_id);
    જો (!id) {
        dprintf(2, "NVMe couldn't identify namespace %u.\n", ns_id);
        જાઓ free_buffer;
    }

    u8 current_lba_format = id->flbas & 0xF;
    જો (current_lba_format > id->nlbaf) {
        dprintf(2, "NVMe NS %u: current LBA format %u is beyond what the "
                " namespace supports (%u)?\n",
                ns_id, current_lba_format, id->nlbaf + 1);
        જાઓ free_buffer;
    }

    જો (!id->nsze) {
        dprintf(2, "NVMe NS %u is inactive.\n", ns_id);
        જાઓ free_buffer;
    }

    જો (!nvme_dma_buffer) {
        nvme_dma_buffer = zalloc_page_aligned(&Zone_ઉપલું, NVME_PAGE_SIZE);
        જો (!nvme_dma_buffer) {
            warn_noalloc();
            જાઓ free_buffer;
        }
    }

    સંરચના nvme_namespace *ns = malloc_fseg(કદ(*ns));
    જો (!ns) {
        warn_noalloc();
        જાઓ free_buffer;
    }
    memset(ns, 0, કદ(*ns));
    ns->ctrl  = ctrl;
    ns->ns_id = ns_id;
    ns->lba_ગણતરી = id->nsze;

    સંરચના nvme_lba_format *fmt = &id->lbaf[current_lba_format];

    ns->block_size    = 1U << fmt->lbads;
    ns->metadata_size = fmt->ms;

    જો (ns->block_size > NVME_PAGE_SIZE) {
        /* If we see devices that trigger this path, we need to increase our
           buffer size. */
        warn_internalerror();
        મુક્ત(ns);
        જાઓ free_buffer;
    }

    ns->drive.cntl_id   = ns_idx;
    ns->drive.removable = 0;
    ns->drive.type      = DTYPE_NVME;
    ns->drive.blksize   = ns->block_size;
    ns->drive.sectors   = ns->lba_ગણતરી;

    જો (mdts) {
        ns->max_req_size = ((1U << mdts) * NVME_PAGE_SIZE) / ns->block_size;
        dprintf(3, "NVME NS %u max request size: %d sectors\n",
                ns_id, ns->max_req_size);
    } નહીંતર {
        ns->max_req_size = -1U;
    }

    અક્ષર *desc = znprintf(MAXDESCSIZE, "NVMe NS %u: %llu MiB (%llu %u-byte "
                          "blocks + %u-byte metadata)",
                          ns_id, (ns->lba_ગણતરી * ns->block_size) >> 20,
                          ns->lba_ગણતરી, ns->block_size, ns->metadata_size);

    dprintf(3, "%s\n", desc);
    boot_add_hd(&ns->drive, desc, bootprio_find_pci_device(ctrl->pci));

free_buffer:
    મુક્ત (id);
}


/* Release memory allocated for a completion queue */
સ્થિર શૂન્ય
nvme_destroy_cq(સંરચના nvme_cq *cq)
{
    મુક્ત(cq->cqe);
    cq->cqe = ખાલી;
}

/* Release memory allocated for a submission queue */
સ્થિર શૂન્ય
nvme_destroy_sq(સંરચના nvme_sq *sq)
{
    મુક્ત(sq->sqe);
    sq->sqe = ખાલી;
}

/* Returns 0 on success. */
સ્થિર પૂર્ણાંક
nvme_create_io_cq(સંરચના nvme_ctrl *ctrl, સંરચના nvme_cq *cq, u16 q_idx)
{
    પૂર્ણાંક rc;
    સંરચના nvme_sqe *cmd_create_cq;
    u32 length = 1 + (ctrl->reg->cap & 0xffff);
    જો (length > NVME_PAGE_SIZE / કદ(સંરચના nvme_cqe))
        length = NVME_PAGE_SIZE / કદ(સંરચના nvme_cqe);

    rc = nvme_init_cq(ctrl, cq, q_idx, length);
    જો (rc) {
        જાઓ err;
    }

    cmd_create_cq = nvme_get_next_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_CQ, ખાલી,
                                      cq->cqe, ખાલી);
    જો (!cmd_create_cq) {
        જાઓ err_destroy_cq;
    }

    cmd_create_cq->dword[10] = (cq->common.mask << 16) | (q_idx >> 1);
    cmd_create_cq->dword[11] = 1 /* physically contiguous */;

    nvme_commit_sqe(&ctrl->admin_sq);

    સંરચના nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    જો (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io cq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        જાઓ err_destroy_cq;
    }

    પાછા 0;

err_destroy_cq:
    nvme_destroy_cq(cq);
err:
    પાછા -1;
}

/* Returns 0 on success. */
સ્થિર પૂર્ણાંક
nvme_create_io_sq(સંરચના nvme_ctrl *ctrl, સંરચના nvme_sq *sq, u16 q_idx, સંરચના nvme_cq *cq)
{
    પૂર્ણાંક rc;
    સંરચના nvme_sqe *cmd_create_sq;
    u32 length = 1 + (ctrl->reg->cap & 0xffff);
    જો (length > NVME_PAGE_SIZE / કદ(સંરચના nvme_cqe))
        length = NVME_PAGE_SIZE / કદ(સંરચના nvme_cqe);

    rc = nvme_init_sq(ctrl, sq, q_idx, length, cq);
    જો (rc) {
        જાઓ err;
    }

    cmd_create_sq = nvme_get_next_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_SQ, ખાલી,
                                      sq->sqe, ખાલી);
    જો (!cmd_create_sq) {
        જાઓ err_destroy_sq;
    }

    cmd_create_sq->dword[10] = (sq->common.mask << 16) | (q_idx >> 1);
    cmd_create_sq->dword[11] = (q_idx >> 1) << 16 | 1 /* contiguous */;
    dprintf(3, "sq %p create dword10 %08x dword11 %08x\n", sq,
            cmd_create_sq->dword[10], cmd_create_sq->dword[11]);

    nvme_commit_sqe(&ctrl->admin_sq);

    સંરચના nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    જો (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io sq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);
        જાઓ err_destroy_sq;
    }

    પાછા 0;

err_destroy_sq:
    nvme_destroy_sq(sq);
err:
    પાછા -1;
}

/* Reads count sectors into buf. The buffer cannot cross page boundaries. */
સ્થિર પૂર્ણાંક
nvme_io_xfer(સંરચના nvme_namespace *ns, u64 lba, શૂન્ય *prp1, શૂન્ય *prp2,
             u16 ગણતરી, પૂર્ણાંક લિખ)
{
    જો (((u32)prp1 & 0x3) || ((u32)prp2 & 0x3)) {
        /* Buffer is misaligned */
        warn_internalerror();
        પાછા -1;
    }

    સંરચના nvme_sqe *io_read = nvme_get_next_sqe(&ns->ctrl->io_sq,
                                                 લિખ ? NVME_SQE_OPC_IO_WRITE
                                                       : NVME_SQE_OPC_IO_READ,
                                                 ખાલી, prp1, prp2);
    io_read->nsid = ns->ns_id;
    io_read->dword[10] = (u32)lba;
    io_read->dword[11] = (u32)(lba >> 32);
    io_read->dword[12] = (1U << 31 /* limited retry */) | (ગણતરી - 1);

    nvme_commit_sqe(&ns->ctrl->io_sq);

    સંરચના nvme_cqe cqe = nvme_wait(&ns->ctrl->io_sq);

    જો (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "read io: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        પાછા -1;
    }

    dprintf(5, "ns %u %s lba %llu+%u\n", ns->ns_id, લિખ ? "write" : "read",
            lba, ગણતરી);
    પાછા ગણતરી;
}

// Transfer up to one page of data using the internal dma bounce buffer
સ્થિર પૂર્ણાંક
nvme_bounce_xfer(સંરચના nvme_namespace *ns, u64 lba, શૂન્ય *buf, u16 ગણતરી,
                 પૂર્ણાંક લિખ)
{
    u16 અચલ max_blocks = NVME_PAGE_SIZE / ns->block_size;
    u16 blocks = ગણતરી < max_blocks ? ગણતરી : max_blocks;

    જો (લિખ)
        memcpy(nvme_dma_buffer, buf, blocks * ns->block_size);

    પૂર્ણાંક res = nvme_io_xfer(ns, lba, nvme_dma_buffer, ખાલી, blocks, લિખ);

    જો (!લિખ && res >= 0)
        memcpy(buf, nvme_dma_buffer, res * ns->block_size);

    પાછા res;
}

#define NVME_MAX_PRPL_ENTRIES 15 /* Allows requests up to 64kb */

// Transfer data using page list (if applicable)
સ્થિર પૂર્ણાંક
nvme_prpl_xfer(સંરચના nvme_namespace *ns, u64 lba, શૂન્ય *buf, u16 ગણતરી,
               પૂર્ણાંક લિખ)
{
    u32 base = (long)buf;
    s32 size;

    જો (ગણતરી > ns->max_req_size)
        ગણતરી = ns->max_req_size;

    size = ગણતરી * ns->block_size;
    /* Special case for transfers that fit into PRP1, but are unaligned */
    જો (((size + (base & ~NVME_PAGE_MASK)) <= NVME_PAGE_SIZE))
        જાઓ single;

    /* Every request has to be page aligned */
    જો (base & ~NVME_PAGE_MASK)
        જાઓ bounce;

    /* Make sure a full block fits into the last chunk */
    જો (size & (ns->block_size - 1ULL))
        જાઓ bounce;

    /* Build PRP list if we need to describe more than 2 pages */
    જો ((ns->block_size * ગણતરી) > (NVME_PAGE_SIZE * 2)) {
        u32 prpl_len = 0;
        u64 *prpl = nvme_dma_buffer;
        પૂર્ણાંક first_page = 1;
        માટે (; size > 0; base += NVME_PAGE_SIZE, size -= NVME_PAGE_SIZE) {
            જો (first_page) {
                /* First page is special */
                first_page = 0;
                ચાલુ;
            }
            જો (prpl_len >= NVME_MAX_PRPL_ENTRIES)
                જાઓ bounce;
            prpl[prpl_len++] = base;
        }
        પાછા nvme_io_xfer(ns, lba, buf, prpl, ગણતરી, લિખ);
    }

    /* Directly embed the 2nd page if we only need 2 pages */
    જો ((ns->block_size * ગણતરી) > NVME_PAGE_SIZE)
        પાછા nvme_io_xfer(ns, lba, buf, buf + NVME_PAGE_SIZE, ગણતરી, લિખ);

single:
    /* One page is enough, don't expose anything else */
    પાછા nvme_io_xfer(ns, lba, buf, ખાલી, ગણતરી, લિખ);

bounce:
    /* Use bounce buffer to make transfer */
    પાછા nvme_bounce_xfer(ns, lba, buf, ગણતરી, લિખ);
}

સ્થિર પૂર્ણાંક
nvme_create_io_queues(સંરચના nvme_ctrl *ctrl)
{
    જો (nvme_create_io_cq(ctrl, &ctrl->io_cq, 3))
        જાઓ err;

    જો (nvme_create_io_sq(ctrl, &ctrl->io_sq, 2, &ctrl->io_cq))
        જાઓ err_free_cq;

    પાછા 0;

 err_free_cq:
    nvme_destroy_cq(&ctrl->io_cq);
 err:
    પાછા -1;
}

/* Waits for CSTS.RDY to match rdy. Returns 0 on success. */
સ્થિર પૂર્ણાંક
nvme_wait_csts_rdy(સંરચના nvme_ctrl *ctrl, unsigned rdy)
{
    u32 અચલ max_to = 500 /* ms */ * ((ctrl->reg->cap >> 24) & 0xFFU);
    u32 to = timer_calc(max_to);
    u32 csts;

    જ્યારે (rdy != ((csts = ctrl->reg->csts) & NVME_CSTS_RDY)) {
        yield();

        જો (csts & NVME_CSTS_FATAL) {
            dprintf(3, "NVMe fatal error during controller shutdown\n");
            પાછા -1;
        }

        જો (timer_check(to)) {
            warn_timeout();
            પાછા -1;
        }
    }

    પાછા 0;
}

/* Returns 0 on success. */
સ્થિર પૂર્ણાંક
nvme_controller_enable(સંરચના nvme_ctrl *ctrl)
{
    પૂર્ણાંક rc;

    pci_enable_busmaster(ctrl->pci);

    /* Turn the controller off. */
    ctrl->reg->cc = 0;
    જો (nvme_wait_csts_rdy(ctrl, 0)) {
        dprintf(2, "NVMe fatal error during controller shutdown\n");
        પાછા -1;
    }

    ctrl->doorbell_stride = 4U << ((ctrl->reg->cap >> 32) & 0xF);

    rc = nvme_init_cq(ctrl, &ctrl->admin_cq, 1,
                      NVME_PAGE_SIZE / કદ(સંરચના nvme_cqe));
    જો (rc) {
        પાછા -1;
    }

    rc = nvme_init_sq(ctrl, &ctrl->admin_sq, 0,
                      NVME_PAGE_SIZE / કદ(સંરચના nvme_sqe), &ctrl->admin_cq);
    જો (rc) {
        જાઓ err_destroy_admin_cq;
    }

    ctrl->reg->aqa = ctrl->admin_cq.common.mask << 16
        | ctrl->admin_sq.common.mask;

    ctrl->reg->asq = (u32)ctrl->admin_sq.sqe;
    ctrl->reg->acq = (u32)ctrl->admin_cq.cqe;

    dprintf(3, "  admin submission queue: %p\n", ctrl->admin_sq.sqe);
    dprintf(3, "  admin completion queue: %p\n", ctrl->admin_cq.cqe);

    ctrl->reg->cc = NVME_CC_EN | (NVME_CQE_SIZE_LOG << 20)
        | (NVME_SQE_SIZE_LOG << 16 /* IOSQES */);

    જો (nvme_wait_csts_rdy(ctrl, 1)) {
        dprintf(2, "NVMe fatal error while enabling controller\n");
        જાઓ err_destroy_admin_sq;
    }

    /* The admin queue is set up and the controller is ready. Let's figure out
       what namespaces we have. */

    સંરચના nvme_identify_ctrl *identify = nvme_admin_identify_ctrl(ctrl);

    જો (!identify) {
        dprintf(2, "NVMe couldn't identify controller.\n");
        જાઓ err_destroy_admin_sq;
    }

    dprintf(3, "NVMe has %u namespace%s.\n",
            identify->nn, (identify->nn == 1) ? "" : "s");

    ctrl->ns_ગણતરી = identify->nn;
    u8 mdts = identify->mdts;
    મુક્ત(identify);

    જો ((ctrl->ns_ગણતરી == 0) || nvme_create_io_queues(ctrl)) {
        /* No point to continue, if the controller says it doesn't have
           namespaces or we couldn't create I/O queues. */
        જાઓ err_destroy_admin_sq;
    }

    /* Populate namespace IDs */
    પૂર્ણાંક ns_idx;
    માટે (ns_idx = 0; ns_idx < ctrl->ns_ગણતરી; ns_idx++) {
        nvme_probe_ns(ctrl, ns_idx, mdts);
    }

    dprintf(3, "NVMe initialization complete!\n");
    પાછા 0;

 err_destroy_admin_sq:
    nvme_destroy_sq(&ctrl->admin_sq);
 err_destroy_admin_cq:
    nvme_destroy_cq(&ctrl->admin_cq);
    પાછા -1;
}

/* Initialize an NVMe controller and detect its drives. */
સ્થિર શૂન્ય
nvme_controller_setup(શૂન્ય *opaque)
{
    u8 skip_nonbootable = is_bootprio_strict();
    સંરચના pci_device *pci = opaque;

    જો (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
        dprintf(1, "skipping init of a non-bootable NVMe at %pP\n",
                pci);
        જાઓ err;
    }

    સંરચના nvme_reg અસ્થિર *reg = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    જો (!reg)
        પાછા;

    u32 version = reg->vs;
    dprintf(3, "Found NVMe controller with version %u.%u.%u.\n",
            version >> 16, (version >> 8) & 0xFF, version & 0xFF);
    dprintf(3, "  Capabilities %016llx\n", reg->cap);

    જો (~reg->cap & NVME_CAP_CSS_NVME) {
        dprintf(3, "Controller doesn't speak NVMe command set. Skipping.\n");
        જાઓ err;
    }

    સંરચના nvme_ctrl *ctrl = malloc_ઉપલું(કદ(*ctrl));
    જો (!ctrl) {
        warn_noalloc();
        જાઓ err;
    }

    memset(ctrl, 0, કદ(*ctrl));

    ctrl->reg = reg;
    ctrl->pci = pci;

    જો (nvme_controller_enable(ctrl)) {
        જાઓ err_free_ctrl;
    }

    પાછા;

 err_free_ctrl:
    મુક્ત(ctrl);
 err:
    dprintf(2, "Failed to enable NVMe controller.\n");
}

// Locate and init NVMe controllers
સ્થિર શૂન્ય
nvme_scan(શૂન્ય)
{
    // Scan PCI bus for NVMe adapters
    સંરચના pci_device *pci;

    foreachpci(pci) {
        જો (pci->class != PCI_CLASS_STORAGE_NVME)
            ચાલુ;
        જો (pci->prog_if != 2 /* as of NVM 1.0e */) {
            dprintf(3, "Found incompatble NVMe: prog-if=%02x\n", pci->prog_if);
            ચાલુ;
        }

        run_thread(nvme_controller_setup, pci);
    }
}

સ્થિર પૂર્ણાંક
nvme_cmd_readwrite(સંરચના nvme_namespace *ns, સંરચના disk_op_s *op, પૂર્ણાંક લિખ)
{
    પૂર્ણાંક i;
    માટે (i = 0; i < op->ગણતરી;) {
        u16 blocks_remaining = op->ગણતરી - i;
        અક્ષર *op_buf = op->buf_fl + i * ns->block_size;
        પૂર્ણાંક blocks = nvme_prpl_xfer(ns, op->lba + i, op_buf,
                                    blocks_remaining, લિખ);
        જો (blocks < 0)
            પાછા DISK_RET_EBADTRACK;
        i += blocks;
    }

    પાછા DISK_RET_SUCCESS;
}

પૂર્ણાંક
nvme_process_op(સંરચના disk_op_s *op)
{
    જો (!CONFIG_NVME)
        પાછા DISK_RET_SUCCESS;

    સંરચના nvme_namespace *ns = container_of(op->drive_fl, સંરચના nvme_namespace,
                                             drive);

    પસંદગી (op->command) {
    સ્થિતિ CMD_READ:
    સ્થિતિ CMD_WRITE:
        પાછા nvme_cmd_readwrite(ns, op, op->command == CMD_WRITE);
    મૂળભૂત:
        પાછા default_process_op(op);
    }
}

શૂન્ય
nvme_setup(શૂન્ય)
{
    ASSERT32FLAT();
    જો (!CONFIG_NVME)
        પાછા;

    dprintf(3, "init nvme\n");
    nvme_scan();
}

/* EOF */

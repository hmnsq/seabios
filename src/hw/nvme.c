// Low level NVMe disk access
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "blockcmd.h"
#ଅନ୍ତର୍ଭାଵ "malloc.h" // malloc_high
#ଅନ୍ତର୍ଭାଵ "output.h" // dprintf
#ଅନ୍ତର୍ଭାଵ "pci.h"
#ଅନ୍ତର୍ଭାଵ "pci_ids.h" // PCI_CLASS_STORAGE_NVME
#ଅନ୍ତର୍ଭାଵ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ଅନ୍ତର୍ଭାଵ "pcidevice.h" // foreachpci
#ଅନ୍ତର୍ଭାଵ "stacks.h" // yield
#ଅନ୍ତର୍ଭାଵ "std/disk.h" // DISK_RET_
#ଅନ୍ତର୍ଭାଵ "string.h" // memset
#ଅନ୍ତର୍ଭାଵ "util.h" // boot_add_hd
#ଅନ୍ତର୍ଭାଵ "x86.h" // readl

#ଅନ୍ତର୍ଭାଵ "nvme.h"
#ଅନ୍ତର୍ଭାଵ "nvme-int.h"

// Page aligned "dma bounce buffer" of size NVME_PAGE_SIZE in high memory
ସ୍ଥିର ଶୂନ୍ୟ *nvme_dma_buffer;

ସ୍ଥିର ଶୂନ୍ୟ *
zalloc_page_aligned(ସଂରଚନା zone_s *zone, u32 size)
{
    ଶୂନ୍ୟ *res = _malloc(zone, size, NVME_PAGE_SIZE);
    ଯଦି (res) memset(res, 0, size);
    ଫେରନ୍ତୁ res;
}

ସ୍ଥିର ଶୂନ୍ୟ
nvme_init_queue_common(ସଂରଚନା nvme_ctrl *ctrl, ସଂରଚନା nvme_queue *q, u16 q_idx,
                       u16 length)
{
    memset(q, 0, ଆକାର(*q));
    q->dbl = (u32 *)((ଅକ୍ଷର *)ctrl->reg + 0x1000 + q_idx * ctrl->doorbell_stride);
    dprintf(3, " q %p q_idx %u dbl %p\n", q, q_idx, q->dbl);
    q->mask = length - 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
nvme_init_sq(ସଂରଚନା nvme_ctrl *ctrl, ସଂରଚନା nvme_sq *sq, u16 q_idx, u16 length,
             ସଂରଚନା nvme_cq *cq)
{
    nvme_init_queue_common(ctrl, &sq->common, q_idx, length);
    sq->sqe = zalloc_page_aligned(&Zone_ଉପର, ଆକାର(*sq->sqe) * length);

    ଯଦି (!sq->sqe) {
        warn_noalloc();
        ଫେରନ୍ତୁ -1;
    }

    dprintf(3, "sq %p q_idx %u sqe %p\n", sq, q_idx, sq->sqe);
    sq->cq   = cq;
    sq->head = 0;
    sq->tail = 0;

    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
nvme_init_cq(ସଂରଚନା nvme_ctrl *ctrl, ସଂରଚନା nvme_cq *cq, u16 q_idx, u16 length)
{
    nvme_init_queue_common(ctrl, &cq->common, q_idx, length);
    cq->cqe = zalloc_page_aligned(&Zone_ଉପର, ଆକାର(*cq->cqe) * length);
    ଯଦି (!cq->cqe) {
        warn_noalloc();
        ଫେରନ୍ତୁ -1;
    }

    cq->head = 0;

    /* All CQE phase bits are initialized to zero. This means initially we wait
       for the host controller to set these to 1. */
    cq->phase = 1;

    ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
nvme_poll_cq(ସଂରଚନା nvme_cq *cq)
{
    u32 dw3 = readl(&cq->cqe[cq->head].dword[3]);
    ଫେରନ୍ତୁ (!!(dw3 & NVME_CQE_DW3_P) == cq->phase);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
nvme_is_cqe_success(ସଂରଚନା nvme_cqe ଅଚଳ *cqe)
{
    ଫେରନ୍ତୁ ((cqe->status >> 1) & 0xFF) == 0;
}

ସ୍ଥିର ସଂରଚନା nvme_cqe
nvme_error_cqe(ଶୂନ୍ୟ)
{
    ସଂରଚନା nvme_cqe r;

    /* 0xFF is a vendor specific status code != success. Should be okay for
       indicating failure. */
    memset(&r, 0xFF, ଆକାର(r));
    ଫେରନ୍ତୁ r;
}

ସ୍ଥିର ସଂରଚନା nvme_cqe
nvme_consume_cqe(ସଂରଚନା nvme_sq *sq)
{
    ସଂରଚନା nvme_cq *cq = sq->cq;

    ଯଦି (!nvme_poll_cq(cq)) {
        /* Cannot consume a completion queue entry, if there is none ready. */
        ଫେରନ୍ତୁ nvme_error_cqe();
    }

    ସଂରଚନା nvme_cqe *cqe = &cq->cqe[cq->head];
    u16 cq_next_head = (cq->head + 1) & cq->common.mask;
    dprintf(4, "cq %p head %u -> %u\n", cq, cq->head, cq_next_head);
    ଯଦି (cq_next_head < cq->head) {
        dprintf(3, "cq %p wrap\n", cq);
        cq->phase = ~cq->phase;
    }
    cq->head = cq_next_head;

    /* Update the submission queue head. */
    ଯଦି (cqe->sq_head != sq->head) {
        sq->head = cqe->sq_head;
        dprintf(4, "sq %p advanced to %u\n", sq, cqe->sq_head);
    }

    /* Tell the controller that we consumed the completion. */
    writel(cq->common.dbl, cq->head);

    ଫେରନ୍ତୁ *cqe;
}

ସ୍ଥିର ସଂରଚନା nvme_cqe
nvme_wait(ସଂରଚନା nvme_sq *sq)
{
    ସ୍ଥିର ଅଚଳ unsigned nvme_timeout = 5000 /* ms */;
    u32 to = timer_calc(nvme_timeout);
    ଯେତେବେଳେ (!nvme_poll_cq(sq->cq)) {
        yield();

        ଯଦି (timer_check(to)) {
            warn_timeout();
            ଫେରନ୍ତୁ nvme_error_cqe();
        }
    }

    ଫେରନ୍ତୁ nvme_consume_cqe(sq);
}

/* Returns the next submission queue entry (or NULL if the queue is full). It
   also fills out Command Dword 0 and clears the rest. */
ସ୍ଥିର ସଂରଚନା nvme_sqe *
nvme_get_next_sqe(ସଂରଚନା nvme_sq *sq, u8 opc, ଶୂନ୍ୟ *metadata, ଶୂନ୍ୟ *data, ଶୂନ୍ୟ *data2)
{
    ଯଦି (((sq->head + 1) & sq->common.mask) == sq->tail) {
        dprintf(3, "submission queue is full\n");
        ଫେରନ୍ତୁ ଖାଲି;
    }

    ସଂରଚନା nvme_sqe *sqe = &sq->sqe[sq->tail];
    dprintf(4, "sq %p next_sqe %u\n", sq, sq->tail);

    memset(sqe, 0, ଆକାର(*sqe));
    sqe->cdw0 = opc | (sq->tail << 16 /* CID */);
    sqe->mptr = (u32)metadata;
    sqe->dptr_prp1 = (u32)data;
    sqe->dptr_prp2 = (u32)data2;

    ଫେରନ୍ତୁ sqe;
}

/* Call this after you've filled out an sqe that you've got from nvme_get_next_sqe. */
ସ୍ଥିର ଶୂନ୍ୟ
nvme_commit_sqe(ସଂରଚନା nvme_sq *sq)
{
    dprintf(4, "sq %p commit_sqe %u\n", sq, sq->tail);
    sq->tail = (sq->tail + 1) & sq->common.mask;
    writel(sq->common.dbl, sq->tail);
}

/* Perform an identify command on the admin queue and return the resulting
   buffer. This may be a NULL pointer, if something failed. This function
   cannot be used after initialization, because it uses buffers in tmp zone. */
ସ୍ଥିର ସଂଘ nvme_identify *
nvme_admin_identify(ସଂରଚନା nvme_ctrl *ctrl, u8 cns, u32 nsid)
{
    ସଂଘ nvme_identify *identify_buf = zalloc_page_aligned(&Zone_Tmp_ଉପର, 4096);
    ଯଦି (!identify_buf) {
        /* Could not allocate identify buffer. */
        warn_internalerror();
        ଫେରନ୍ତୁ ଖାଲି;
    }

    ସଂରଚନା nvme_sqe *cmd_identify;
    cmd_identify = nvme_get_next_sqe(&ctrl->admin_sq,
                                     NVME_SQE_OPC_ADMIN_IDENTIFY, ଖାଲି,
                                     identify_buf, ଖାଲି);

    ଯଦି (!cmd_identify) {
        warn_internalerror();
        ଯାଅ error;
    }

    cmd_identify->nsid = nsid;
    cmd_identify->dword[10] = cns;

    nvme_commit_sqe(&ctrl->admin_sq);

    ସଂରଚନା nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    ଯଦି (!nvme_is_cqe_success(&cqe)) {
        ଯାଅ error;
    }

    ଫେରନ୍ତୁ identify_buf;
 error:
    ମୁକ୍ତ(identify_buf);
    ଫେରନ୍ତୁ ଖାଲି;
}

ସ୍ଥିର ସଂରଚନା nvme_identify_ctrl *
nvme_admin_identify_ctrl(ସଂରଚନା nvme_ctrl *ctrl)
{
    ଫେରନ୍ତୁ &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_CTRL, 0)->ctrl;
}

ସ୍ଥିର ସଂରଚନା nvme_identify_ns *
nvme_admin_identify_ns(ସଂରଚନା nvme_ctrl *ctrl, u32 ns_id)
{
    ଫେରନ୍ତୁ &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_NS,
                                ns_id)->ns;
}

ସ୍ଥିର ଶୂନ୍ୟ
nvme_probe_ns(ସଂରଚନା nvme_ctrl *ctrl, u32 ns_idx, u8 mdts)
{
    u32 ns_id = ns_idx + 1;

    ସଂରଚନା nvme_identify_ns *id = nvme_admin_identify_ns(ctrl, ns_id);
    ଯଦି (!id) {
        dprintf(2, "NVMe couldn't identify namespace %u.\n", ns_id);
        ଯାଅ free_buffer;
    }

    u8 current_lba_format = id->flbas & 0xF;
    ଯଦି (current_lba_format > id->nlbaf) {
        dprintf(2, "NVMe NS %u: current LBA format %u is beyond what the "
                " namespace supports (%u)?\n",
                ns_id, current_lba_format, id->nlbaf + 1);
        ଯାଅ free_buffer;
    }

    ଯଦି (!id->nsze) {
        dprintf(2, "NVMe NS %u is inactive.\n", ns_id);
        ଯାଅ free_buffer;
    }

    ଯଦି (!nvme_dma_buffer) {
        nvme_dma_buffer = zalloc_page_aligned(&Zone_ଉପର, NVME_PAGE_SIZE);
        ଯଦି (!nvme_dma_buffer) {
            warn_noalloc();
            ଯାଅ free_buffer;
        }
    }

    ସଂରଚନା nvme_namespace *ns = malloc_fseg(ଆକାର(*ns));
    ଯଦି (!ns) {
        warn_noalloc();
        ଯାଅ free_buffer;
    }
    memset(ns, 0, ଆକାର(*ns));
    ns->ctrl  = ctrl;
    ns->ns_id = ns_id;
    ns->lba_ଗଣନା = id->nsze;

    ସଂରଚନା nvme_lba_format *fmt = &id->lbaf[current_lba_format];

    ns->block_size    = 1U << fmt->lbads;
    ns->metadata_size = fmt->ms;

    ଯଦି (ns->block_size > NVME_PAGE_SIZE) {
        /* If we see devices that trigger this path, we need to increase our
           buffer size. */
        warn_internalerror();
        ମୁକ୍ତ(ns);
        ଯାଅ free_buffer;
    }

    ns->drive.cntl_id   = ns_idx;
    ns->drive.removable = 0;
    ns->drive.type      = DTYPE_NVME;
    ns->drive.blksize   = ns->block_size;
    ns->drive.sectors   = ns->lba_ଗଣନା;

    ଯଦି (mdts) {
        ns->max_req_size = ((1U << mdts) * NVME_PAGE_SIZE) / ns->block_size;
        dprintf(3, "NVME NS %u max request size: %d sectors\n",
                ns_id, ns->max_req_size);
    } ଅନ୍ୟଥା {
        ns->max_req_size = -1U;
    }

    ଅକ୍ଷର *desc = znprintf(MAXDESCSIZE, "NVMe NS %u: %llu MiB (%llu %u-byte "
                          "blocks + %u-byte metadata)",
                          ns_id, (ns->lba_ଗଣନା * ns->block_size) >> 20,
                          ns->lba_ଗଣନା, ns->block_size, ns->metadata_size);

    dprintf(3, "%s\n", desc);
    boot_add_hd(&ns->drive, desc, bootprio_find_pci_device(ctrl->pci));

free_buffer:
    ମୁକ୍ତ (id);
}


/* Release memory allocated for a completion queue */
ସ୍ଥିର ଶୂନ୍ୟ
nvme_destroy_cq(ସଂରଚନା nvme_cq *cq)
{
    ମୁକ୍ତ(cq->cqe);
    cq->cqe = ଖାଲି;
}

/* Release memory allocated for a submission queue */
ସ୍ଥିର ଶୂନ୍ୟ
nvme_destroy_sq(ସଂରଚନା nvme_sq *sq)
{
    ମୁକ୍ତ(sq->sqe);
    sq->sqe = ଖାଲି;
}

/* Returns 0 on success. */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
nvme_create_io_cq(ସଂରଚନା nvme_ctrl *ctrl, ସଂରଚନା nvme_cq *cq, u16 q_idx)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
    ସଂରଚନା nvme_sqe *cmd_create_cq;
    u32 length = 1 + (ctrl->reg->cap & 0xffff);
    ଯଦି (length > NVME_PAGE_SIZE / ଆକାର(ସଂରଚନା nvme_cqe))
        length = NVME_PAGE_SIZE / ଆକାର(ସଂରଚନା nvme_cqe);

    rc = nvme_init_cq(ctrl, cq, q_idx, length);
    ଯଦି (rc) {
        ଯାଅ err;
    }

    cmd_create_cq = nvme_get_next_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_CQ, ଖାଲି,
                                      cq->cqe, ଖାଲି);
    ଯଦି (!cmd_create_cq) {
        ଯାଅ err_destroy_cq;
    }

    cmd_create_cq->dword[10] = (cq->common.mask << 16) | (q_idx >> 1);
    cmd_create_cq->dword[11] = 1 /* physically contiguous */;

    nvme_commit_sqe(&ctrl->admin_sq);

    ସଂରଚନା nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    ଯଦି (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io cq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        ଯାଅ err_destroy_cq;
    }

    ଫେରନ୍ତୁ 0;

err_destroy_cq:
    nvme_destroy_cq(cq);
err:
    ଫେରନ୍ତୁ -1;
}

/* Returns 0 on success. */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
nvme_create_io_sq(ସଂରଚନା nvme_ctrl *ctrl, ସଂରଚନା nvme_sq *sq, u16 q_idx, ସଂରଚନା nvme_cq *cq)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
    ସଂରଚନା nvme_sqe *cmd_create_sq;
    u32 length = 1 + (ctrl->reg->cap & 0xffff);
    ଯଦି (length > NVME_PAGE_SIZE / ଆକାର(ସଂରଚନା nvme_cqe))
        length = NVME_PAGE_SIZE / ଆକାର(ସଂରଚନା nvme_cqe);

    rc = nvme_init_sq(ctrl, sq, q_idx, length, cq);
    ଯଦି (rc) {
        ଯାଅ err;
    }

    cmd_create_sq = nvme_get_next_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_SQ, ଖାଲି,
                                      sq->sqe, ଖାଲି);
    ଯଦି (!cmd_create_sq) {
        ଯାଅ err_destroy_sq;
    }

    cmd_create_sq->dword[10] = (sq->common.mask << 16) | (q_idx >> 1);
    cmd_create_sq->dword[11] = (q_idx >> 1) << 16 | 1 /* contiguous */;
    dprintf(3, "sq %p create dword10 %08x dword11 %08x\n", sq,
            cmd_create_sq->dword[10], cmd_create_sq->dword[11]);

    nvme_commit_sqe(&ctrl->admin_sq);

    ସଂରଚନା nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    ଯଦି (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io sq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);
        ଯାଅ err_destroy_sq;
    }

    ଫେରନ୍ତୁ 0;

err_destroy_sq:
    nvme_destroy_sq(sq);
err:
    ଫେରନ୍ତୁ -1;
}

/* Reads count sectors into buf. The buffer cannot cross page boundaries. */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
nvme_io_xfer(ସଂରଚନା nvme_namespace *ns, u64 lba, ଶୂନ୍ୟ *prp1, ଶୂନ୍ୟ *prp2,
             u16 ଗଣନା_2, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଲିଖ)
{
    ଯଦି (((u32)prp1 & 0x3) || ((u32)prp2 & 0x3)) {
        /* Buffer is misaligned */
        warn_internalerror();
        ଫେରନ୍ତୁ -1;
    }

    ସଂରଚନା nvme_sqe *io_read = nvme_get_next_sqe(&ns->ctrl->io_sq,
                                                 ଲିଖ ? NVME_SQE_OPC_IO_WRITE
                                                       : NVME_SQE_OPC_IO_READ,
                                                 ଖାଲି, prp1, prp2);
    io_read->nsid = ns->ns_id;
    io_read->dword[10] = (u32)lba;
    io_read->dword[11] = (u32)(lba >> 32);
    io_read->dword[12] = (1U << 31 /* limited retry */) | (ଗଣନା_2 - 1);

    nvme_commit_sqe(&ns->ctrl->io_sq);

    ସଂରଚନା nvme_cqe cqe = nvme_wait(&ns->ctrl->io_sq);

    ଯଦି (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "read io: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        ଫେରନ୍ତୁ -1;
    }

    dprintf(5, "ns %u %s lba %llu+%u\n", ns->ns_id, ଲିଖ ? "write" : "read",
            lba, ଗଣନା_2);
    ଫେରନ୍ତୁ ଗଣନା_2;
}

// Transfer up to one page of data using the internal dma bounce buffer
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
nvme_bounce_xfer(ସଂରଚନା nvme_namespace *ns, u64 lba, ଶୂନ୍ୟ *buf, u16 ଗଣନା_2,
                 ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଲିଖ)
{
    u16 ଅଚଳ max_blocks = NVME_PAGE_SIZE / ns->block_size;
    u16 blocks = ଗଣନା_2 < max_blocks ? ଗଣନା_2 : max_blocks;

    ଯଦି (ଲିଖ)
        memcpy(nvme_dma_buffer, buf, blocks * ns->block_size);

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = nvme_io_xfer(ns, lba, nvme_dma_buffer, ଖାଲି, blocks, ଲିଖ);

    ଯଦି (!ଲିଖ && res >= 0)
        memcpy(buf, nvme_dma_buffer, res * ns->block_size);

    ଫେରନ୍ତୁ res;
}

#define NVME_MAX_PRPL_ENTRIES 15 /* Allows requests up to 64kb */

// Transfer data using page list (if applicable)
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
nvme_prpl_xfer(ସଂରଚନା nvme_namespace *ns, u64 lba, ଶୂନ୍ୟ *buf, u16 ଗଣନା_2,
               ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଲିଖ)
{
    u32 base = (long)buf;
    s32 size;

    ଯଦି (ଗଣନା_2 > ns->max_req_size)
        ଗଣନା_2 = ns->max_req_size;

    size = ଗଣନା_2 * ns->block_size;
    /* Special case for transfers that fit into PRP1, but are unaligned */
    ଯଦି (((size + (base & ~NVME_PAGE_MASK)) <= NVME_PAGE_SIZE))
        ଯାଅ single;

    /* Every request has to be page aligned */
    ଯଦି (base & ~NVME_PAGE_MASK)
        ଯାଅ bounce;

    /* Make sure a full block fits into the last chunk */
    ଯଦି (size & (ns->block_size - 1ULL))
        ଯାଅ bounce;

    /* Build PRP list if we need to describe more than 2 pages */
    ଯଦି ((ns->block_size * ଗଣନା_2) > (NVME_PAGE_SIZE * 2)) {
        u32 prpl_len = 0;
        u64 *prpl = nvme_dma_buffer;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା first_page = 1;
        ପାଇଁ (; size > 0; base += NVME_PAGE_SIZE, size -= NVME_PAGE_SIZE) {
            ଯଦି (first_page) {
                /* First page is special */
                first_page = 0;
                ଜାରି;
            }
            ଯଦି (prpl_len >= NVME_MAX_PRPL_ENTRIES)
                ଯାଅ bounce;
            prpl[prpl_len++] = base;
        }
        ଫେରନ୍ତୁ nvme_io_xfer(ns, lba, buf, prpl, ଗଣନା_2, ଲିଖ);
    }

    /* Directly embed the 2nd page if we only need 2 pages */
    ଯଦି ((ns->block_size * ଗଣନା_2) > NVME_PAGE_SIZE)
        ଫେରନ୍ତୁ nvme_io_xfer(ns, lba, buf, buf + NVME_PAGE_SIZE, ଗଣନା_2, ଲିଖ);

single:
    /* One page is enough, don't expose anything else */
    ଫେରନ୍ତୁ nvme_io_xfer(ns, lba, buf, ଖାଲି, ଗଣନା_2, ଲିଖ);

bounce:
    /* Use bounce buffer to make transfer */
    ଫେରନ୍ତୁ nvme_bounce_xfer(ns, lba, buf, ଗଣନା_2, ଲିଖ);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
nvme_create_io_queues(ସଂରଚନା nvme_ctrl *ctrl)
{
    ଯଦି (nvme_create_io_cq(ctrl, &ctrl->io_cq, 3))
        ଯାଅ err;

    ଯଦି (nvme_create_io_sq(ctrl, &ctrl->io_sq, 2, &ctrl->io_cq))
        ଯାଅ err_free_cq;

    ଫେରନ୍ତୁ 0;

 err_free_cq:
    nvme_destroy_cq(&ctrl->io_cq);
 err:
    ଫେରନ୍ତୁ -1;
}

/* Waits for CSTS.RDY to match rdy. Returns 0 on success. */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
nvme_wait_csts_rdy(ସଂରଚନା nvme_ctrl *ctrl, unsigned rdy)
{
    u32 ଅଚଳ max_to = 500 /* ms */ * ((ctrl->reg->cap >> 24) & 0xFFU);
    u32 to = timer_calc(max_to);
    u32 csts;

    ଯେତେବେଳେ (rdy != ((csts = ctrl->reg->csts) & NVME_CSTS_RDY)) {
        yield();

        ଯଦି (csts & NVME_CSTS_FATAL) {
            dprintf(3, "NVMe fatal error during controller shutdown\n");
            ଫେରନ୍ତୁ -1;
        }

        ଯଦି (timer_check(to)) {
            warn_timeout();
            ଫେରନ୍ତୁ -1;
        }
    }

    ଫେରନ୍ତୁ 0;
}

/* Returns 0 on success. */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
nvme_controller_enable(ସଂରଚନା nvme_ctrl *ctrl)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

    pci_enable_busmaster(ctrl->pci);

    /* Turn the controller off. */
    ctrl->reg->cc = 0;
    ଯଦି (nvme_wait_csts_rdy(ctrl, 0)) {
        dprintf(2, "NVMe fatal error during controller shutdown\n");
        ଫେରନ୍ତୁ -1;
    }

    ctrl->doorbell_stride = 4U << ((ctrl->reg->cap >> 32) & 0xF);

    rc = nvme_init_cq(ctrl, &ctrl->admin_cq, 1,
                      NVME_PAGE_SIZE / ଆକାର(ସଂରଚନା nvme_cqe));
    ଯଦି (rc) {
        ଫେରନ୍ତୁ -1;
    }

    rc = nvme_init_sq(ctrl, &ctrl->admin_sq, 0,
                      NVME_PAGE_SIZE / ଆକାର(ସଂରଚନା nvme_sqe), &ctrl->admin_cq);
    ଯଦି (rc) {
        ଯାଅ err_destroy_admin_cq;
    }

    ctrl->reg->aqa = ctrl->admin_cq.common.mask << 16
        | ctrl->admin_sq.common.mask;

    ctrl->reg->asq = (u32)ctrl->admin_sq.sqe;
    ctrl->reg->acq = (u32)ctrl->admin_cq.cqe;

    dprintf(3, "  admin submission queue: %p\n", ctrl->admin_sq.sqe);
    dprintf(3, "  admin completion queue: %p\n", ctrl->admin_cq.cqe);

    ctrl->reg->cc = NVME_CC_EN | (NVME_CQE_SIZE_LOG << 20)
        | (NVME_SQE_SIZE_LOG << 16 /* IOSQES */);

    ଯଦି (nvme_wait_csts_rdy(ctrl, 1)) {
        dprintf(2, "NVMe fatal error while enabling controller\n");
        ଯାଅ err_destroy_admin_sq;
    }

    /* The admin queue is set up and the controller is ready. Let's figure out
       what namespaces we have. */

    ସଂରଚନା nvme_identify_ctrl *identify = nvme_admin_identify_ctrl(ctrl);

    ଯଦି (!identify) {
        dprintf(2, "NVMe couldn't identify controller.\n");
        ଯାଅ err_destroy_admin_sq;
    }

    dprintf(3, "NVMe has %u namespace%s.\n",
            identify->nn, (identify->nn == 1) ? "" : "s");

    ctrl->ns_ଗଣନା = identify->nn;
    u8 mdts = identify->mdts;
    ମୁକ୍ତ(identify);

    ଯଦି ((ctrl->ns_ଗଣନା == 0) || nvme_create_io_queues(ctrl)) {
        /* No point to continue, if the controller says it doesn't have
           namespaces or we couldn't create I/O queues. */
        ଯାଅ err_destroy_admin_sq;
    }

    /* Populate namespace IDs */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ns_idx;
    ପାଇଁ (ns_idx = 0; ns_idx < ctrl->ns_ଗଣନା; ns_idx++) {
        nvme_probe_ns(ctrl, ns_idx, mdts);
    }

    dprintf(3, "NVMe initialization complete!\n");
    ଫେରନ୍ତୁ 0;

 err_destroy_admin_sq:
    nvme_destroy_sq(&ctrl->admin_sq);
 err_destroy_admin_cq:
    nvme_destroy_cq(&ctrl->admin_cq);
    ଫେରନ୍ତୁ -1;
}

/* Initialize an NVMe controller and detect its drives. */
ସ୍ଥିର ଶୂନ୍ୟ
nvme_controller_setup(ଶୂନ୍ୟ *opaque)
{
    u8 skip_nonbootable = is_bootprio_strict();
    ସଂରଚନା pci_device *pci = opaque;

    ଯଦି (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
        dprintf(1, "skipping init of a non-bootable NVMe at %pP\n",
                pci);
        ଯାଅ err;
    }

    ସଂରଚନା nvme_reg ଅସ୍ଥିର *reg = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    ଯଦି (!reg)
        ଫେରନ୍ତୁ;

    u32 version = reg->vs;
    dprintf(3, "Found NVMe controller with version %u.%u.%u.\n",
            version >> 16, (version >> 8) & 0xFF, version & 0xFF);
    dprintf(3, "  Capabilities %016llx\n", reg->cap);

    ଯଦି (~reg->cap & NVME_CAP_CSS_NVME) {
        dprintf(3, "Controller doesn't speak NVMe command set. Skipping.\n");
        ଯାଅ err;
    }

    ସଂରଚନା nvme_ctrl *ctrl = malloc_ଉପର(ଆକାର(*ctrl));
    ଯଦି (!ctrl) {
        warn_noalloc();
        ଯାଅ err;
    }

    memset(ctrl, 0, ଆକାର(*ctrl));

    ctrl->reg = reg;
    ctrl->pci = pci;

    ଯଦି (nvme_controller_enable(ctrl)) {
        ଯାଅ err_free_ctrl;
    }

    ଫେରନ୍ତୁ;

 err_free_ctrl:
    ମୁକ୍ତ(ctrl);
 err:
    dprintf(2, "Failed to enable NVMe controller.\n");
}

// Locate and init NVMe controllers
ସ୍ଥିର ଶୂନ୍ୟ
nvme_scan(ଶୂନ୍ୟ)
{
    // Scan PCI bus for NVMe adapters
    ସଂରଚନା pci_device *pci;

    foreachpci(pci) {
        ଯଦି (pci->class != PCI_CLASS_STORAGE_NVME)
            ଜାରି;
        ଯଦି (pci->prog_if != 2 /* as of NVM 1.0e */) {
            dprintf(3, "Found incompatble NVMe: prog-if=%02x\n", pci->prog_if);
            ଜାରି;
        }

        run_thread(nvme_controller_setup, pci);
    }
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
nvme_cmd_readwrite(ସଂରଚନା nvme_namespace *ns, ସଂରଚନା disk_op_s *op, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଲିଖ)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i = 0; i < op->ଗଣନା_2;) {
        u16 blocks_remaining = op->ଗଣନା_2 - i;
        ଅକ୍ଷର *op_buf = op->buf_fl + i * ns->block_size;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା blocks = nvme_prpl_xfer(ns, op->lba + i, op_buf,
                                    blocks_remaining, ଲିଖ);
        ଯଦି (blocks < 0)
            ଫେରନ୍ତୁ DISK_RET_EBADTRACK;
        i += blocks;
    }

    ଫେରନ୍ତୁ DISK_RET_SUCCESS;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
nvme_process_op(ସଂରଚନା disk_op_s *op)
{
    ଯଦି (!CONFIG_NVME)
        ଫେରନ୍ତୁ DISK_RET_SUCCESS;

    ସଂରଚନା nvme_namespace *ns = container_of(op->drive_fl, ସଂରଚନା nvme_namespace,
                                             drive);

    ବାଛ (op->command) {
    ସ୍ଥିତି CMD_READ:
    ସ୍ଥିତି CMD_WRITE:
        ଫେରନ୍ତୁ nvme_cmd_readwrite(ns, op, op->command == CMD_WRITE);
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ଫେରନ୍ତୁ default_process_op(op);
    }
}

ଶୂନ୍ୟ
nvme_setup(ଶୂନ୍ୟ)
{
    ASSERT32FLAT();
    ଯଦି (!CONFIG_NVME)
        ଫେରନ୍ତୁ;

    dprintf(3, "init nvme\n");
    nvme_scan();
}

/* EOF */

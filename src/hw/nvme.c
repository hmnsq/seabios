// Low level NVMe disk access
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "blockcmd.h"
#অন্তর্ভাব "malloc.h" // malloc_high
#অন্তর্ভাব "output.h" // dprintf
#অন্তর্ভাব "pci.h"
#অন্তর্ভাব "pci_ids.h" // PCI_CLASS_STORAGE_NVME
#অন্তর্ভাব "pci_regs.h" // PCI_BASE_ADDRESS_0
#অন্তর্ভাব "pcidevice.h" // foreachpci
#অন্তর্ভাব "stacks.h" // yield
#অন্তর্ভাব "std/disk.h" // DISK_RET_
#অন্তর্ভাব "string.h" // memset
#অন্তর্ভাব "util.h" // boot_add_hd
#অন্তর্ভাব "x86.h" // readl

#অন্তর্ভাব "nvme.h"
#অন্তর্ভাব "nvme-int.h"

// Page aligned "dma bounce buffer" of size NVME_PAGE_SIZE in high memory
স্থির শূন্য *nvme_dma_buffer;

স্থির শূন্য *
zalloc_page_aligned(সংরচনা zone_s *zone, u32 size)
{
    শূন্য *res = _malloc(zone, size, NVME_PAGE_SIZE);
    যদি (res) memset(res, 0, size);
    প্রত্যাগম res;
}

স্থির শূন্য
nvme_init_queue_common(সংরচনা nvme_ctrl *ctrl, সংরচনা nvme_queue *q, u16 q_idx,
                       u16 length)
{
    memset(q, 0, পরিমাণ(*q));
    q->dbl = (u32 *)((বর্ণ *)ctrl->reg + 0x1000 + q_idx * ctrl->doorbell_stride);
    dprintf(3, " q %p q_idx %u dbl %p\n", q, q_idx, q->dbl);
    q->mask = length - 1;
}

স্থির পূর্ণাঙ্ক
nvme_init_sq(সংরচনা nvme_ctrl *ctrl, সংরচনা nvme_sq *sq, u16 q_idx, u16 length,
             সংরচনা nvme_cq *cq)
{
    nvme_init_queue_common(ctrl, &sq->common, q_idx, length);
    sq->sqe = zalloc_page_aligned(&Zone_উপরের, পরিমাণ(*sq->sqe) * length);

    যদি (!sq->sqe) {
        warn_noalloc();
        প্রত্যাগম -1;
    }

    dprintf(3, "sq %p q_idx %u sqe %p\n", sq, q_idx, sq->sqe);
    sq->cq   = cq;
    sq->head = 0;
    sq->tail = 0;

    প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক
nvme_init_cq(সংরচনা nvme_ctrl *ctrl, সংরচনা nvme_cq *cq, u16 q_idx, u16 length)
{
    nvme_init_queue_common(ctrl, &cq->common, q_idx, length);
    cq->cqe = zalloc_page_aligned(&Zone_উপরের, পরিমাণ(*cq->cqe) * length);
    যদি (!cq->cqe) {
        warn_noalloc();
        প্রত্যাগম -1;
    }

    cq->head = 0;

    /* All CQE phase bits are initialized to zero. This means initially we wait
       for the host controller to set these to 1. */
    cq->phase = 1;

    প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক
nvme_poll_cq(সংরচনা nvme_cq *cq)
{
    u32 dw3 = readl(&cq->cqe[cq->head].dword[3]);
    প্রত্যাগম (!!(dw3 & NVME_CQE_DW3_P) == cq->phase);
}

স্থির পূর্ণাঙ্ক
nvme_is_cqe_success(সংরচনা nvme_cqe ধ্রুব *cqe)
{
    প্রত্যাগম ((cqe->status >> 1) & 0xFF) == 0;
}

স্থির সংরচনা nvme_cqe
nvme_error_cqe(শূন্য)
{
    সংরচনা nvme_cqe r;

    /* 0xFF is a vendor specific status code != success. Should be okay for
       indicating failure. */
    memset(&r, 0xFF, পরিমাণ(r));
    প্রত্যাগম r;
}

স্থির সংরচনা nvme_cqe
nvme_consume_cqe(সংরচনা nvme_sq *sq)
{
    সংরচনা nvme_cq *cq = sq->cq;

    যদি (!nvme_poll_cq(cq)) {
        /* Cannot consume a completion queue entry, if there is none ready. */
        প্রত্যাগম nvme_error_cqe();
    }

    সংরচনা nvme_cqe *cqe = &cq->cqe[cq->head];
    u16 cq_next_head = (cq->head + 1) & cq->common.mask;
    dprintf(4, "cq %p head %u -> %u\n", cq, cq->head, cq_next_head);
    যদি (cq_next_head < cq->head) {
        dprintf(3, "cq %p wrap\n", cq);
        cq->phase = ~cq->phase;
    }
    cq->head = cq_next_head;

    /* Update the submission queue head. */
    যদি (cqe->sq_head != sq->head) {
        sq->head = cqe->sq_head;
        dprintf(4, "sq %p advanced to %u\n", sq, cqe->sq_head);
    }

    /* Tell the controller that we consumed the completion. */
    writel(cq->common.dbl, cq->head);

    প্রত্যাগম *cqe;
}

স্থির সংরচনা nvme_cqe
nvme_wait(সংরচনা nvme_sq *sq)
{
    স্থির ধ্রুব unsigned nvme_timeout = 5000 /* ms */;
    u32 to = timer_calc(nvme_timeout);
    যাবত্ (!nvme_poll_cq(sq->cq)) {
        yield();

        যদি (timer_check(to)) {
            warn_timeout();
            প্রত্যাগম nvme_error_cqe();
        }
    }

    প্রত্যাগম nvme_consume_cqe(sq);
}

/* Returns the next submission queue entry (or NULL if the queue is full). It
   also fills out Command Dword 0 and clears the rest. */
স্থির সংরচনা nvme_sqe *
nvme_get_next_sqe(সংরচনা nvme_sq *sq, u8 opc, শূন্য *metadata, শূন্য *data, শূন্য *data2)
{
    যদি (((sq->head + 1) & sq->common.mask) == sq->tail) {
        dprintf(3, "submission queue is full\n");
        প্রত্যাগম রিক্ত;
    }

    সংরচনা nvme_sqe *sqe = &sq->sqe[sq->tail];
    dprintf(4, "sq %p next_sqe %u\n", sq, sq->tail);

    memset(sqe, 0, পরিমাণ(*sqe));
    sqe->cdw0 = opc | (sq->tail << 16 /* CID */);
    sqe->mptr = (u32)metadata;
    sqe->dptr_prp1 = (u32)data;
    sqe->dptr_prp2 = (u32)data2;

    প্রত্যাগম sqe;
}

/* Call this after you've filled out an sqe that you've got from nvme_get_next_sqe. */
স্থির শূন্য
nvme_commit_sqe(সংরচনা nvme_sq *sq)
{
    dprintf(4, "sq %p commit_sqe %u\n", sq, sq->tail);
    sq->tail = (sq->tail + 1) & sq->common.mask;
    writel(sq->common.dbl, sq->tail);
}

/* Perform an identify command on the admin queue and return the resulting
   buffer. This may be a NULL pointer, if something failed. This function
   cannot be used after initialization, because it uses buffers in tmp zone. */
স্থির সঙ্ঘ nvme_identify *
nvme_admin_identify(সংরচনা nvme_ctrl *ctrl, u8 cns, u32 nsid)
{
    সঙ্ঘ nvme_identify *identify_buf = zalloc_page_aligned(&Zone_Tmp_উপরের, 4096);
    যদি (!identify_buf) {
        /* Could not allocate identify buffer. */
        warn_internalerror();
        প্রত্যাগম রিক্ত;
    }

    সংরচনা nvme_sqe *cmd_identify;
    cmd_identify = nvme_get_next_sqe(&ctrl->admin_sq,
                                     NVME_SQE_OPC_ADMIN_IDENTIFY, রিক্ত,
                                     identify_buf, রিক্ত);

    যদি (!cmd_identify) {
        warn_internalerror();
        গচ্ছ error;
    }

    cmd_identify->nsid = nsid;
    cmd_identify->dword[10] = cns;

    nvme_commit_sqe(&ctrl->admin_sq);

    সংরচনা nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    যদি (!nvme_is_cqe_success(&cqe)) {
        গচ্ছ error;
    }

    প্রত্যাগম identify_buf;
 error:
    মুক্ত(identify_buf);
    প্রত্যাগম রিক্ত;
}

স্থির সংরচনা nvme_identify_ctrl *
nvme_admin_identify_ctrl(সংরচনা nvme_ctrl *ctrl)
{
    প্রত্যাগম &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_CTRL, 0)->ctrl;
}

স্থির সংরচনা nvme_identify_ns *
nvme_admin_identify_ns(সংরচনা nvme_ctrl *ctrl, u32 ns_id)
{
    প্রত্যাগম &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_NS,
                                ns_id)->ns;
}

স্থির শূন্য
nvme_probe_ns(সংরচনা nvme_ctrl *ctrl, u32 ns_idx, u8 mdts)
{
    u32 ns_id = ns_idx + 1;

    সংরচনা nvme_identify_ns *id = nvme_admin_identify_ns(ctrl, ns_id);
    যদি (!id) {
        dprintf(2, "NVMe couldn't identify namespace %u.\n", ns_id);
        গচ্ছ free_buffer;
    }

    u8 current_lba_format = id->flbas & 0xF;
    যদি (current_lba_format > id->nlbaf) {
        dprintf(2, "NVMe NS %u: current LBA format %u is beyond what the "
                " namespace supports (%u)?\n",
                ns_id, current_lba_format, id->nlbaf + 1);
        গচ্ছ free_buffer;
    }

    যদি (!id->nsze) {
        dprintf(2, "NVMe NS %u is inactive.\n", ns_id);
        গচ্ছ free_buffer;
    }

    যদি (!nvme_dma_buffer) {
        nvme_dma_buffer = zalloc_page_aligned(&Zone_উপরের, NVME_PAGE_SIZE);
        যদি (!nvme_dma_buffer) {
            warn_noalloc();
            গচ্ছ free_buffer;
        }
    }

    সংরচনা nvme_namespace *ns = malloc_fseg(পরিমাণ(*ns));
    যদি (!ns) {
        warn_noalloc();
        গচ্ছ free_buffer;
    }
    memset(ns, 0, পরিমাণ(*ns));
    ns->ctrl  = ctrl;
    ns->ns_id = ns_id;
    ns->lba_গুনতি = id->nsze;

    সংরচনা nvme_lba_format *fmt = &id->lbaf[current_lba_format];

    ns->block_size    = 1U << fmt->lbads;
    ns->metadata_size = fmt->ms;

    যদি (ns->block_size > NVME_PAGE_SIZE) {
        /* If we see devices that trigger this path, we need to increase our
           buffer size. */
        warn_internalerror();
        মুক্ত(ns);
        গচ্ছ free_buffer;
    }

    ns->drive.cntl_id   = ns_idx;
    ns->drive.removable = 0;
    ns->drive.type      = DTYPE_NVME;
    ns->drive.blksize   = ns->block_size;
    ns->drive.sectors   = ns->lba_গুনতি;

    যদি (mdts) {
        ns->max_req_size = ((1U << mdts) * NVME_PAGE_SIZE) / ns->block_size;
        dprintf(3, "NVME NS %u max request size: %d sectors\n",
                ns_id, ns->max_req_size);
    } অন্যথা {
        ns->max_req_size = -1U;
    }

    বর্ণ *desc = znprintf(MAXDESCSIZE, "NVMe NS %u: %llu MiB (%llu %u-byte "
                          "blocks + %u-byte metadata)",
                          ns_id, (ns->lba_গুনতি * ns->block_size) >> 20,
                          ns->lba_গুনতি, ns->block_size, ns->metadata_size);

    dprintf(3, "%s\n", desc);
    boot_add_hd(&ns->drive, desc, bootprio_find_pci_device(ctrl->pci));

free_buffer:
    মুক্ত (id);
}


/* Release memory allocated for a completion queue */
স্থির শূন্য
nvme_destroy_cq(সংরচনা nvme_cq *cq)
{
    মুক্ত(cq->cqe);
    cq->cqe = রিক্ত;
}

/* Release memory allocated for a submission queue */
স্থির শূন্য
nvme_destroy_sq(সংরচনা nvme_sq *sq)
{
    মুক্ত(sq->sqe);
    sq->sqe = রিক্ত;
}

/* Returns 0 on success. */
স্থির পূর্ণাঙ্ক
nvme_create_io_cq(সংরচনা nvme_ctrl *ctrl, সংরচনা nvme_cq *cq, u16 q_idx)
{
    পূর্ণাঙ্ক rc;
    সংরচনা nvme_sqe *cmd_create_cq;
    u32 length = 1 + (ctrl->reg->cap & 0xffff);
    যদি (length > NVME_PAGE_SIZE / পরিমাণ(সংরচনা nvme_cqe))
        length = NVME_PAGE_SIZE / পরিমাণ(সংরচনা nvme_cqe);

    rc = nvme_init_cq(ctrl, cq, q_idx, length);
    যদি (rc) {
        গচ্ছ err;
    }

    cmd_create_cq = nvme_get_next_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_CQ, রিক্ত,
                                      cq->cqe, রিক্ত);
    যদি (!cmd_create_cq) {
        গচ্ছ err_destroy_cq;
    }

    cmd_create_cq->dword[10] = (cq->common.mask << 16) | (q_idx >> 1);
    cmd_create_cq->dword[11] = 1 /* physically contiguous */;

    nvme_commit_sqe(&ctrl->admin_sq);

    সংরচনা nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    যদি (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io cq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        গচ্ছ err_destroy_cq;
    }

    প্রত্যাগম 0;

err_destroy_cq:
    nvme_destroy_cq(cq);
err:
    প্রত্যাগম -1;
}

/* Returns 0 on success. */
স্থির পূর্ণাঙ্ক
nvme_create_io_sq(সংরচনা nvme_ctrl *ctrl, সংরচনা nvme_sq *sq, u16 q_idx, সংরচনা nvme_cq *cq)
{
    পূর্ণাঙ্ক rc;
    সংরচনা nvme_sqe *cmd_create_sq;
    u32 length = 1 + (ctrl->reg->cap & 0xffff);
    যদি (length > NVME_PAGE_SIZE / পরিমাণ(সংরচনা nvme_cqe))
        length = NVME_PAGE_SIZE / পরিমাণ(সংরচনা nvme_cqe);

    rc = nvme_init_sq(ctrl, sq, q_idx, length, cq);
    যদি (rc) {
        গচ্ছ err;
    }

    cmd_create_sq = nvme_get_next_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_SQ, রিক্ত,
                                      sq->sqe, রিক্ত);
    যদি (!cmd_create_sq) {
        গচ্ছ err_destroy_sq;
    }

    cmd_create_sq->dword[10] = (sq->common.mask << 16) | (q_idx >> 1);
    cmd_create_sq->dword[11] = (q_idx >> 1) << 16 | 1 /* contiguous */;
    dprintf(3, "sq %p create dword10 %08x dword11 %08x\n", sq,
            cmd_create_sq->dword[10], cmd_create_sq->dword[11]);

    nvme_commit_sqe(&ctrl->admin_sq);

    সংরচনা nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    যদি (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io sq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);
        গচ্ছ err_destroy_sq;
    }

    প্রত্যাগম 0;

err_destroy_sq:
    nvme_destroy_sq(sq);
err:
    প্রত্যাগম -1;
}

/* Reads count sectors into buf. The buffer cannot cross page boundaries. */
স্থির পূর্ণাঙ্ক
nvme_io_xfer(সংরচনা nvme_namespace *ns, u64 lba, শূন্য *prp1, শূন্য *prp2,
             u16 গুনতি, পূর্ণাঙ্ক লিখ)
{
    যদি (((u32)prp1 & 0x3) || ((u32)prp2 & 0x3)) {
        /* Buffer is misaligned */
        warn_internalerror();
        প্রত্যাগম -1;
    }

    সংরচনা nvme_sqe *io_read = nvme_get_next_sqe(&ns->ctrl->io_sq,
                                                 লিখ ? NVME_SQE_OPC_IO_WRITE
                                                       : NVME_SQE_OPC_IO_READ,
                                                 রিক্ত, prp1, prp2);
    io_read->nsid = ns->ns_id;
    io_read->dword[10] = (u32)lba;
    io_read->dword[11] = (u32)(lba >> 32);
    io_read->dword[12] = (1U << 31 /* limited retry */) | (গুনতি - 1);

    nvme_commit_sqe(&ns->ctrl->io_sq);

    সংরচনা nvme_cqe cqe = nvme_wait(&ns->ctrl->io_sq);

    যদি (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "read io: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        প্রত্যাগম -1;
    }

    dprintf(5, "ns %u %s lba %llu+%u\n", ns->ns_id, লিখ ? "write" : "read",
            lba, গুনতি);
    প্রত্যাগম গুনতি;
}

// Transfer up to one page of data using the internal dma bounce buffer
স্থির পূর্ণাঙ্ক
nvme_bounce_xfer(সংরচনা nvme_namespace *ns, u64 lba, শূন্য *buf, u16 গুনতি,
                 পূর্ণাঙ্ক লিখ)
{
    u16 ধ্রুব max_blocks = NVME_PAGE_SIZE / ns->block_size;
    u16 blocks = গুনতি < max_blocks ? গুনতি : max_blocks;

    যদি (লিখ)
        memcpy(nvme_dma_buffer, buf, blocks * ns->block_size);

    পূর্ণাঙ্ক res = nvme_io_xfer(ns, lba, nvme_dma_buffer, রিক্ত, blocks, লিখ);

    যদি (!লিখ && res >= 0)
        memcpy(buf, nvme_dma_buffer, res * ns->block_size);

    প্রত্যাগম res;
}

#define NVME_MAX_PRPL_ENTRIES 15 /* Allows requests up to 64kb */

// Transfer data using page list (if applicable)
স্থির পূর্ণাঙ্ক
nvme_prpl_xfer(সংরচনা nvme_namespace *ns, u64 lba, শূন্য *buf, u16 গুনতি,
               পূর্ণাঙ্ক লিখ)
{
    u32 base = (long)buf;
    s32 size;

    যদি (গুনতি > ns->max_req_size)
        গুনতি = ns->max_req_size;

    size = গুনতি * ns->block_size;
    /* Special case for transfers that fit into PRP1, but are unaligned */
    যদি (((size + (base & ~NVME_PAGE_MASK)) <= NVME_PAGE_SIZE))
        গচ্ছ single;

    /* Every request has to be page aligned */
    যদি (base & ~NVME_PAGE_MASK)
        গচ্ছ bounce;

    /* Make sure a full block fits into the last chunk */
    যদি (size & (ns->block_size - 1ULL))
        গচ্ছ bounce;

    /* Build PRP list if we need to describe more than 2 pages */
    যদি ((ns->block_size * গুনতি) > (NVME_PAGE_SIZE * 2)) {
        u32 prpl_len = 0;
        u64 *prpl = nvme_dma_buffer;
        পূর্ণাঙ্ক first_page = 1;
        প্রতি (; size > 0; base += NVME_PAGE_SIZE, size -= NVME_PAGE_SIZE) {
            যদি (first_page) {
                /* First page is special */
                first_page = 0;
                অনুবর্ত;
            }
            যদি (prpl_len >= NVME_MAX_PRPL_ENTRIES)
                গচ্ছ bounce;
            prpl[prpl_len++] = base;
        }
        প্রত্যাগম nvme_io_xfer(ns, lba, buf, prpl, গুনতি, লিখ);
    }

    /* Directly embed the 2nd page if we only need 2 pages */
    যদি ((ns->block_size * গুনতি) > NVME_PAGE_SIZE)
        প্রত্যাগম nvme_io_xfer(ns, lba, buf, buf + NVME_PAGE_SIZE, গুনতি, লিখ);

single:
    /* One page is enough, don't expose anything else */
    প্রত্যাগম nvme_io_xfer(ns, lba, buf, রিক্ত, গুনতি, লিখ);

bounce:
    /* Use bounce buffer to make transfer */
    প্রত্যাগম nvme_bounce_xfer(ns, lba, buf, গুনতি, লিখ);
}

স্থির পূর্ণাঙ্ক
nvme_create_io_queues(সংরচনা nvme_ctrl *ctrl)
{
    যদি (nvme_create_io_cq(ctrl, &ctrl->io_cq, 3))
        গচ্ছ err;

    যদি (nvme_create_io_sq(ctrl, &ctrl->io_sq, 2, &ctrl->io_cq))
        গচ্ছ err_free_cq;

    প্রত্যাগম 0;

 err_free_cq:
    nvme_destroy_cq(&ctrl->io_cq);
 err:
    প্রত্যাগম -1;
}

/* Waits for CSTS.RDY to match rdy. Returns 0 on success. */
স্থির পূর্ণাঙ্ক
nvme_wait_csts_rdy(সংরচনা nvme_ctrl *ctrl, unsigned rdy)
{
    u32 ধ্রুব max_to = 500 /* ms */ * ((ctrl->reg->cap >> 24) & 0xFFU);
    u32 to = timer_calc(max_to);
    u32 csts;

    যাবত্ (rdy != ((csts = ctrl->reg->csts) & NVME_CSTS_RDY)) {
        yield();

        যদি (csts & NVME_CSTS_FATAL) {
            dprintf(3, "NVMe fatal error during controller shutdown\n");
            প্রত্যাগম -1;
        }

        যদি (timer_check(to)) {
            warn_timeout();
            প্রত্যাগম -1;
        }
    }

    প্রত্যাগম 0;
}

/* Returns 0 on success. */
স্থির পূর্ণাঙ্ক
nvme_controller_enable(সংরচনা nvme_ctrl *ctrl)
{
    পূর্ণাঙ্ক rc;

    pci_enable_busmaster(ctrl->pci);

    /* Turn the controller off. */
    ctrl->reg->cc = 0;
    যদি (nvme_wait_csts_rdy(ctrl, 0)) {
        dprintf(2, "NVMe fatal error during controller shutdown\n");
        প্রত্যাগম -1;
    }

    ctrl->doorbell_stride = 4U << ((ctrl->reg->cap >> 32) & 0xF);

    rc = nvme_init_cq(ctrl, &ctrl->admin_cq, 1,
                      NVME_PAGE_SIZE / পরিমাণ(সংরচনা nvme_cqe));
    যদি (rc) {
        প্রত্যাগম -1;
    }

    rc = nvme_init_sq(ctrl, &ctrl->admin_sq, 0,
                      NVME_PAGE_SIZE / পরিমাণ(সংরচনা nvme_sqe), &ctrl->admin_cq);
    যদি (rc) {
        গচ্ছ err_destroy_admin_cq;
    }

    ctrl->reg->aqa = ctrl->admin_cq.common.mask << 16
        | ctrl->admin_sq.common.mask;

    ctrl->reg->asq = (u32)ctrl->admin_sq.sqe;
    ctrl->reg->acq = (u32)ctrl->admin_cq.cqe;

    dprintf(3, "  admin submission queue: %p\n", ctrl->admin_sq.sqe);
    dprintf(3, "  admin completion queue: %p\n", ctrl->admin_cq.cqe);

    ctrl->reg->cc = NVME_CC_EN | (NVME_CQE_SIZE_LOG << 20)
        | (NVME_SQE_SIZE_LOG << 16 /* IOSQES */);

    যদি (nvme_wait_csts_rdy(ctrl, 1)) {
        dprintf(2, "NVMe fatal error while enabling controller\n");
        গচ্ছ err_destroy_admin_sq;
    }

    /* The admin queue is set up and the controller is ready. Let's figure out
       what namespaces we have. */

    সংরচনা nvme_identify_ctrl *identify = nvme_admin_identify_ctrl(ctrl);

    যদি (!identify) {
        dprintf(2, "NVMe couldn't identify controller.\n");
        গচ্ছ err_destroy_admin_sq;
    }

    dprintf(3, "NVMe has %u namespace%s.\n",
            identify->nn, (identify->nn == 1) ? "" : "s");

    ctrl->ns_গুনতি = identify->nn;
    u8 mdts = identify->mdts;
    মুক্ত(identify);

    যদি ((ctrl->ns_গুনতি == 0) || nvme_create_io_queues(ctrl)) {
        /* No point to continue, if the controller says it doesn't have
           namespaces or we couldn't create I/O queues. */
        গচ্ছ err_destroy_admin_sq;
    }

    /* Populate namespace IDs */
    পূর্ণাঙ্ক ns_idx;
    প্রতি (ns_idx = 0; ns_idx < ctrl->ns_গুনতি; ns_idx++) {
        nvme_probe_ns(ctrl, ns_idx, mdts);
    }

    dprintf(3, "NVMe initialization complete!\n");
    প্রত্যাগম 0;

 err_destroy_admin_sq:
    nvme_destroy_sq(&ctrl->admin_sq);
 err_destroy_admin_cq:
    nvme_destroy_cq(&ctrl->admin_cq);
    প্রত্যাগম -1;
}

/* Initialize an NVMe controller and detect its drives. */
স্থির শূন্য
nvme_controller_setup(শূন্য *opaque)
{
    u8 skip_nonbootable = is_bootprio_strict();
    সংরচনা pci_device *pci = opaque;

    যদি (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
        dprintf(1, "skipping init of a non-bootable NVMe at %pP\n",
                pci);
        গচ্ছ err;
    }

    সংরচনা nvme_reg চঞ্চল *reg = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    যদি (!reg)
        প্রত্যাগম;

    u32 version = reg->vs;
    dprintf(3, "Found NVMe controller with version %u.%u.%u.\n",
            version >> 16, (version >> 8) & 0xFF, version & 0xFF);
    dprintf(3, "  Capabilities %016llx\n", reg->cap);

    যদি (~reg->cap & NVME_CAP_CSS_NVME) {
        dprintf(3, "Controller doesn't speak NVMe command set. Skipping.\n");
        গচ্ছ err;
    }

    সংরচনা nvme_ctrl *ctrl = malloc_উপরের(পরিমাণ(*ctrl));
    যদি (!ctrl) {
        warn_noalloc();
        গচ্ছ err;
    }

    memset(ctrl, 0, পরিমাণ(*ctrl));

    ctrl->reg = reg;
    ctrl->pci = pci;

    যদি (nvme_controller_enable(ctrl)) {
        গচ্ছ err_free_ctrl;
    }

    প্রত্যাগম;

 err_free_ctrl:
    মুক্ত(ctrl);
 err:
    dprintf(2, "Failed to enable NVMe controller.\n");
}

// Locate and init NVMe controllers
স্থির শূন্য
nvme_scan(শূন্য)
{
    // Scan PCI bus for NVMe adapters
    সংরচনা pci_device *pci;

    foreachpci(pci) {
        যদি (pci->class != PCI_CLASS_STORAGE_NVME)
            অনুবর্ত;
        যদি (pci->prog_if != 2 /* as of NVM 1.0e */) {
            dprintf(3, "Found incompatble NVMe: prog-if=%02x\n", pci->prog_if);
            অনুবর্ত;
        }

        run_thread(nvme_controller_setup, pci);
    }
}

স্থির পূর্ণাঙ্ক
nvme_cmd_readwrite(সংরচনা nvme_namespace *ns, সংরচনা disk_op_s *op, পূর্ণাঙ্ক লিখ)
{
    পূর্ণাঙ্ক i;
    প্রতি (i = 0; i < op->গুনতি;) {
        u16 blocks_remaining = op->গুনতি - i;
        বর্ণ *op_buf = op->buf_fl + i * ns->block_size;
        পূর্ণাঙ্ক blocks = nvme_prpl_xfer(ns, op->lba + i, op_buf,
                                    blocks_remaining, লিখ);
        যদি (blocks < 0)
            প্রত্যাগম DISK_RET_EBADTRACK;
        i += blocks;
    }

    প্রত্যাগম DISK_RET_SUCCESS;
}

পূর্ণাঙ্ক
nvme_process_op(সংরচনা disk_op_s *op)
{
    যদি (!CONFIG_NVME)
        প্রত্যাগম DISK_RET_SUCCESS;

    সংরচনা nvme_namespace *ns = container_of(op->drive_fl, সংরচনা nvme_namespace,
                                             drive);

    বিকল্প (op->command) {
    পক্ষ CMD_READ:
    পক্ষ CMD_WRITE:
        প্রত্যাগম nvme_cmd_readwrite(ns, op, op->command == CMD_WRITE);
    স্বভাব:
        প্রত্যাগম default_process_op(op);
    }
}

শূন্য
nvme_setup(শূন্য)
{
    ASSERT32FLAT();
    যদি (!CONFIG_NVME)
        প্রত্যাগম;

    dprintf(3, "init nvme\n");
    nvme_scan();
}

/* EOF */

// Low level NVMe disk access
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "blockcmd.h"
#அந்தர்பாவ "malloc.h" // malloc_high
#அந்தர்பாவ "output.h" // dprintf
#அந்தர்பாவ "pci.h"
#அந்தர்பாவ "pci_ids.h" // PCI_CLASS_STORAGE_NVME
#அந்தர்பாவ "pci_regs.h" // PCI_BASE_ADDRESS_0
#அந்தர்பாவ "pcidevice.h" // foreachpci
#அந்தர்பாவ "stacks.h" // yield
#அந்தர்பாவ "std/disk.h" // DISK_RET_
#அந்தர்பாவ "string.h" // memset
#அந்தர்பாவ "util.h" // boot_add_hd
#அந்தர்பாவ "x86.h" // readl

#அந்தர்பாவ "nvme.h"
#அந்தர்பாவ "nvme-int.h"

// Page aligned "dma bounce buffer" of size NVME_PAGE_SIZE in high memory
நிலையான வெறுமை *nvme_dma_buffer;

நிலையான வெறுமை *
zalloc_page_aligned(கட்டமைப்பு zone_s *zone, u32 size)
{
    வெறுமை *res = _malloc(zone, size, NVME_PAGE_SIZE);
    எனில் (res) memset(res, 0, size);
    திரும்பு res;
}

நிலையான வெறுமை
nvme_init_queue_common(கட்டமைப்பு nvme_ctrl *ctrl, கட்டமைப்பு nvme_queue *q, u16 q_idx,
                       u16 length)
{
    memset(q, 0, அளவு(*q));
    q->dbl = (u32 *)((எழுத்து *)ctrl->reg + 0x1000 + q_idx * ctrl->doorbell_stride);
    dprintf(3, " q %p q_idx %u dbl %p\n", q, q_idx, q->dbl);
    q->mask = length - 1;
}

நிலையான முழு
nvme_init_sq(கட்டமைப்பு nvme_ctrl *ctrl, கட்டமைப்பு nvme_sq *sq, u16 q_idx, u16 length,
             கட்டமைப்பு nvme_cq *cq)
{
    nvme_init_queue_common(ctrl, &sq->common, q_idx, length);
    sq->sqe = zalloc_page_aligned(&Zone_உயர்வு, அளவு(*sq->sqe) * length);

    எனில் (!sq->sqe) {
        warn_noalloc();
        திரும்பு -1;
    }

    dprintf(3, "sq %p q_idx %u sqe %p\n", sq, q_idx, sq->sqe);
    sq->cq   = cq;
    sq->head = 0;
    sq->tail = 0;

    திரும்பு 0;
}

நிலையான முழு
nvme_init_cq(கட்டமைப்பு nvme_ctrl *ctrl, கட்டமைப்பு nvme_cq *cq, u16 q_idx, u16 length)
{
    nvme_init_queue_common(ctrl, &cq->common, q_idx, length);
    cq->cqe = zalloc_page_aligned(&Zone_உயர்வு, அளவு(*cq->cqe) * length);
    எனில் (!cq->cqe) {
        warn_noalloc();
        திரும்பு -1;
    }

    cq->head = 0;

    /* All CQE phase bits are initialized to zero. This means initially we wait
       for the host controller to set these to 1. */
    cq->phase = 1;

    திரும்பு 0;
}

நிலையான முழு
nvme_poll_cq(கட்டமைப்பு nvme_cq *cq)
{
    u32 dw3 = readl(&cq->cqe[cq->head].dword[3]);
    திரும்பு (!!(dw3 & NVME_CQE_DW3_P) == cq->phase);
}

நிலையான முழு
nvme_is_cqe_success(கட்டமைப்பு nvme_cqe மாறிலி *cqe)
{
    திரும்பு ((cqe->status >> 1) & 0xFF) == 0;
}

நிலையான கட்டமைப்பு nvme_cqe
nvme_error_cqe(வெறுமை)
{
    கட்டமைப்பு nvme_cqe r;

    /* 0xFF is a vendor specific status code != success. Should be okay for
       indicating failure. */
    memset(&r, 0xFF, அளவு(r));
    திரும்பு r;
}

நிலையான கட்டமைப்பு nvme_cqe
nvme_consume_cqe(கட்டமைப்பு nvme_sq *sq)
{
    கட்டமைப்பு nvme_cq *cq = sq->cq;

    எனில் (!nvme_poll_cq(cq)) {
        /* Cannot consume a completion queue entry, if there is none ready. */
        திரும்பு nvme_error_cqe();
    }

    கட்டமைப்பு nvme_cqe *cqe = &cq->cqe[cq->head];
    u16 cq_next_head = (cq->head + 1) & cq->common.mask;
    dprintf(4, "cq %p head %u -> %u\n", cq, cq->head, cq_next_head);
    எனில் (cq_next_head < cq->head) {
        dprintf(3, "cq %p wrap\n", cq);
        cq->phase = ~cq->phase;
    }
    cq->head = cq_next_head;

    /* Update the submission queue head. */
    எனில் (cqe->sq_head != sq->head) {
        sq->head = cqe->sq_head;
        dprintf(4, "sq %p advanced to %u\n", sq, cqe->sq_head);
    }

    /* Tell the controller that we consumed the completion. */
    writel(cq->common.dbl, cq->head);

    திரும்பு *cqe;
}

நிலையான கட்டமைப்பு nvme_cqe
nvme_wait(கட்டமைப்பு nvme_sq *sq)
{
    நிலையான மாறிலி unsigned nvme_timeout = 5000 /* ms */;
    u32 to = timer_calc(nvme_timeout);
    போது (!nvme_poll_cq(sq->cq)) {
        yield();

        எனில் (timer_check(to)) {
            warn_timeout();
            திரும்பு nvme_error_cqe();
        }
    }

    திரும்பு nvme_consume_cqe(sq);
}

/* Returns the next submission queue entry (or NULL if the queue is full). It
   also fills out Command Dword 0 and clears the rest. */
நிலையான கட்டமைப்பு nvme_sqe *
nvme_get_next_sqe(கட்டமைப்பு nvme_sq *sq, u8 opc, வெறுமை *metadata, வெறுமை *data, வெறுமை *data2)
{
    எனில் (((sq->head + 1) & sq->common.mask) == sq->tail) {
        dprintf(3, "submission queue is full\n");
        திரும்பு வெற்று;
    }

    கட்டமைப்பு nvme_sqe *sqe = &sq->sqe[sq->tail];
    dprintf(4, "sq %p next_sqe %u\n", sq, sq->tail);

    memset(sqe, 0, அளவு(*sqe));
    sqe->cdw0 = opc | (sq->tail << 16 /* CID */);
    sqe->mptr = (u32)metadata;
    sqe->dptr_prp1 = (u32)data;
    sqe->dptr_prp2 = (u32)data2;

    திரும்பு sqe;
}

/* Call this after you've filled out an sqe that you've got from nvme_get_next_sqe. */
நிலையான வெறுமை
nvme_commit_sqe(கட்டமைப்பு nvme_sq *sq)
{
    dprintf(4, "sq %p commit_sqe %u\n", sq, sq->tail);
    sq->tail = (sq->tail + 1) & sq->common.mask;
    writel(sq->common.dbl, sq->tail);
}

/* Perform an identify command on the admin queue and return the resulting
   buffer. This may be a NULL pointer, if something failed. This function
   cannot be used after initialization, because it uses buffers in tmp zone. */
நிலையான ஒன்றிப்பு nvme_identify *
nvme_admin_identify(கட்டமைப்பு nvme_ctrl *ctrl, u8 cns, u32 nsid)
{
    ஒன்றிப்பு nvme_identify *identify_buf = zalloc_page_aligned(&Zone_Tmp_உயர்வு, 4096);
    எனில் (!identify_buf) {
        /* Could not allocate identify buffer. */
        warn_internalerror();
        திரும்பு வெற்று;
    }

    கட்டமைப்பு nvme_sqe *cmd_identify;
    cmd_identify = nvme_get_next_sqe(&ctrl->admin_sq,
                                     NVME_SQE_OPC_ADMIN_IDENTIFY, வெற்று,
                                     identify_buf, வெற்று);

    எனில் (!cmd_identify) {
        warn_internalerror();
        செல் error;
    }

    cmd_identify->nsid = nsid;
    cmd_identify->dword[10] = cns;

    nvme_commit_sqe(&ctrl->admin_sq);

    கட்டமைப்பு nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    எனில் (!nvme_is_cqe_success(&cqe)) {
        செல் error;
    }

    திரும்பு identify_buf;
 error:
    விடுவி(identify_buf);
    திரும்பு வெற்று;
}

நிலையான கட்டமைப்பு nvme_identify_ctrl *
nvme_admin_identify_ctrl(கட்டமைப்பு nvme_ctrl *ctrl)
{
    திரும்பு &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_CTRL, 0)->ctrl;
}

நிலையான கட்டமைப்பு nvme_identify_ns *
nvme_admin_identify_ns(கட்டமைப்பு nvme_ctrl *ctrl, u32 ns_id)
{
    திரும்பு &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_NS,
                                ns_id)->ns;
}

நிலையான வெறுமை
nvme_probe_ns(கட்டமைப்பு nvme_ctrl *ctrl, u32 ns_idx, u8 mdts)
{
    u32 ns_id = ns_idx + 1;

    கட்டமைப்பு nvme_identify_ns *id = nvme_admin_identify_ns(ctrl, ns_id);
    எனில் (!id) {
        dprintf(2, "NVMe couldn't identify namespace %u.\n", ns_id);
        செல் free_buffer;
    }

    u8 current_lba_format = id->flbas & 0xF;
    எனில் (current_lba_format > id->nlbaf) {
        dprintf(2, "NVMe NS %u: current LBA format %u is beyond what the "
                " namespace supports (%u)?\n",
                ns_id, current_lba_format, id->nlbaf + 1);
        செல் free_buffer;
    }

    எனில் (!id->nsze) {
        dprintf(2, "NVMe NS %u is inactive.\n", ns_id);
        செல் free_buffer;
    }

    எனில் (!nvme_dma_buffer) {
        nvme_dma_buffer = zalloc_page_aligned(&Zone_உயர்வு, NVME_PAGE_SIZE);
        எனில் (!nvme_dma_buffer) {
            warn_noalloc();
            செல் free_buffer;
        }
    }

    கட்டமைப்பு nvme_namespace *ns = malloc_fseg(அளவு(*ns));
    எனில் (!ns) {
        warn_noalloc();
        செல் free_buffer;
    }
    memset(ns, 0, அளவு(*ns));
    ns->ctrl  = ctrl;
    ns->ns_id = ns_id;
    ns->lba_மொத்தம் = id->nsze;

    கட்டமைப்பு nvme_lba_format *fmt = &id->lbaf[current_lba_format];

    ns->block_size    = 1U << fmt->lbads;
    ns->metadata_size = fmt->ms;

    எனில் (ns->block_size > NVME_PAGE_SIZE) {
        /* If we see devices that trigger this path, we need to increase our
           buffer size. */
        warn_internalerror();
        விடுவி(ns);
        செல் free_buffer;
    }

    ns->drive.cntl_id   = ns_idx;
    ns->drive.removable = 0;
    ns->drive.type      = DTYPE_NVME;
    ns->drive.blksize   = ns->block_size;
    ns->drive.sectors   = ns->lba_மொத்தம்;

    எனில் (mdts) {
        ns->max_req_size = ((1U << mdts) * NVME_PAGE_SIZE) / ns->block_size;
        dprintf(3, "NVME NS %u max request size: %d sectors\n",
                ns_id, ns->max_req_size);
    } இல்லையெனில் {
        ns->max_req_size = -1U;
    }

    எழுத்து *desc = znprintf(MAXDESCSIZE, "NVMe NS %u: %llu MiB (%llu %u-byte "
                          "blocks + %u-byte metadata)",
                          ns_id, (ns->lba_மொத்தம் * ns->block_size) >> 20,
                          ns->lba_மொத்தம், ns->block_size, ns->metadata_size);

    dprintf(3, "%s\n", desc);
    boot_add_hd(&ns->drive, desc, bootprio_find_pci_device(ctrl->pci));

free_buffer:
    விடுவி (id);
}


/* Release memory allocated for a completion queue */
நிலையான வெறுமை
nvme_destroy_cq(கட்டமைப்பு nvme_cq *cq)
{
    விடுவி(cq->cqe);
    cq->cqe = வெற்று;
}

/* Release memory allocated for a submission queue */
நிலையான வெறுமை
nvme_destroy_sq(கட்டமைப்பு nvme_sq *sq)
{
    விடுவி(sq->sqe);
    sq->sqe = வெற்று;
}

/* Returns 0 on success. */
நிலையான முழு
nvme_create_io_cq(கட்டமைப்பு nvme_ctrl *ctrl, கட்டமைப்பு nvme_cq *cq, u16 q_idx)
{
    முழு rc;
    கட்டமைப்பு nvme_sqe *cmd_create_cq;
    u32 length = 1 + (ctrl->reg->cap & 0xffff);
    எனில் (length > NVME_PAGE_SIZE / அளவு(கட்டமைப்பு nvme_cqe))
        length = NVME_PAGE_SIZE / அளவு(கட்டமைப்பு nvme_cqe);

    rc = nvme_init_cq(ctrl, cq, q_idx, length);
    எனில் (rc) {
        செல் err;
    }

    cmd_create_cq = nvme_get_next_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_CQ, வெற்று,
                                      cq->cqe, வெற்று);
    எனில் (!cmd_create_cq) {
        செல் err_destroy_cq;
    }

    cmd_create_cq->dword[10] = (cq->common.mask << 16) | (q_idx >> 1);
    cmd_create_cq->dword[11] = 1 /* physically contiguous */;

    nvme_commit_sqe(&ctrl->admin_sq);

    கட்டமைப்பு nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    எனில் (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io cq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        செல் err_destroy_cq;
    }

    திரும்பு 0;

err_destroy_cq:
    nvme_destroy_cq(cq);
err:
    திரும்பு -1;
}

/* Returns 0 on success. */
நிலையான முழு
nvme_create_io_sq(கட்டமைப்பு nvme_ctrl *ctrl, கட்டமைப்பு nvme_sq *sq, u16 q_idx, கட்டமைப்பு nvme_cq *cq)
{
    முழு rc;
    கட்டமைப்பு nvme_sqe *cmd_create_sq;
    u32 length = 1 + (ctrl->reg->cap & 0xffff);
    எனில் (length > NVME_PAGE_SIZE / அளவு(கட்டமைப்பு nvme_cqe))
        length = NVME_PAGE_SIZE / அளவு(கட்டமைப்பு nvme_cqe);

    rc = nvme_init_sq(ctrl, sq, q_idx, length, cq);
    எனில் (rc) {
        செல் err;
    }

    cmd_create_sq = nvme_get_next_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_SQ, வெற்று,
                                      sq->sqe, வெற்று);
    எனில் (!cmd_create_sq) {
        செல் err_destroy_sq;
    }

    cmd_create_sq->dword[10] = (sq->common.mask << 16) | (q_idx >> 1);
    cmd_create_sq->dword[11] = (q_idx >> 1) << 16 | 1 /* contiguous */;
    dprintf(3, "sq %p create dword10 %08x dword11 %08x\n", sq,
            cmd_create_sq->dword[10], cmd_create_sq->dword[11]);

    nvme_commit_sqe(&ctrl->admin_sq);

    கட்டமைப்பு nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    எனில் (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io sq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);
        செல் err_destroy_sq;
    }

    திரும்பு 0;

err_destroy_sq:
    nvme_destroy_sq(sq);
err:
    திரும்பு -1;
}

/* Reads count sectors into buf. The buffer cannot cross page boundaries. */
நிலையான முழு
nvme_io_xfer(கட்டமைப்பு nvme_namespace *ns, u64 lba, வெறுமை *prp1, வெறுமை *prp2,
             u16 மொத்தம், முழு லிக)
{
    எனில் (((u32)prp1 & 0x3) || ((u32)prp2 & 0x3)) {
        /* Buffer is misaligned */
        warn_internalerror();
        திரும்பு -1;
    }

    கட்டமைப்பு nvme_sqe *io_read = nvme_get_next_sqe(&ns->ctrl->io_sq,
                                                 லிக ? NVME_SQE_OPC_IO_WRITE
                                                       : NVME_SQE_OPC_IO_READ,
                                                 வெற்று, prp1, prp2);
    io_read->nsid = ns->ns_id;
    io_read->dword[10] = (u32)lba;
    io_read->dword[11] = (u32)(lba >> 32);
    io_read->dword[12] = (1U << 31 /* limited retry */) | (மொத்தம் - 1);

    nvme_commit_sqe(&ns->ctrl->io_sq);

    கட்டமைப்பு nvme_cqe cqe = nvme_wait(&ns->ctrl->io_sq);

    எனில் (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "read io: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        திரும்பு -1;
    }

    dprintf(5, "ns %u %s lba %llu+%u\n", ns->ns_id, லிக ? "write" : "read",
            lba, மொத்தம்);
    திரும்பு மொத்தம்;
}

// Transfer up to one page of data using the internal dma bounce buffer
நிலையான முழு
nvme_bounce_xfer(கட்டமைப்பு nvme_namespace *ns, u64 lba, வெறுமை *buf, u16 மொத்தம்,
                 முழு லிக)
{
    u16 மாறிலி max_blocks = NVME_PAGE_SIZE / ns->block_size;
    u16 blocks = மொத்தம் < max_blocks ? மொத்தம் : max_blocks;

    எனில் (லிக)
        memcpy(nvme_dma_buffer, buf, blocks * ns->block_size);

    முழு res = nvme_io_xfer(ns, lba, nvme_dma_buffer, வெற்று, blocks, லிக);

    எனில் (!லிக && res >= 0)
        memcpy(buf, nvme_dma_buffer, res * ns->block_size);

    திரும்பு res;
}

#define NVME_MAX_PRPL_ENTRIES 15 /* Allows requests up to 64kb */

// Transfer data using page list (if applicable)
நிலையான முழு
nvme_prpl_xfer(கட்டமைப்பு nvme_namespace *ns, u64 lba, வெறுமை *buf, u16 மொத்தம்,
               முழு லிக)
{
    u32 base = (long)buf;
    s32 size;

    எனில் (மொத்தம் > ns->max_req_size)
        மொத்தம் = ns->max_req_size;

    size = மொத்தம் * ns->block_size;
    /* Special case for transfers that fit into PRP1, but are unaligned */
    எனில் (((size + (base & ~NVME_PAGE_MASK)) <= NVME_PAGE_SIZE))
        செல் single;

    /* Every request has to be page aligned */
    எனில் (base & ~NVME_PAGE_MASK)
        செல் bounce;

    /* Make sure a full block fits into the last chunk */
    எனில் (size & (ns->block_size - 1ULL))
        செல் bounce;

    /* Build PRP list if we need to describe more than 2 pages */
    எனில் ((ns->block_size * மொத்தம்) > (NVME_PAGE_SIZE * 2)) {
        u32 prpl_len = 0;
        u64 *prpl = nvme_dma_buffer;
        முழு first_page = 1;
        க்கு (; size > 0; base += NVME_PAGE_SIZE, size -= NVME_PAGE_SIZE) {
            எனில் (first_page) {
                /* First page is special */
                first_page = 0;
                தொடர்;
            }
            எனில் (prpl_len >= NVME_MAX_PRPL_ENTRIES)
                செல் bounce;
            prpl[prpl_len++] = base;
        }
        திரும்பு nvme_io_xfer(ns, lba, buf, prpl, மொத்தம், லிக);
    }

    /* Directly embed the 2nd page if we only need 2 pages */
    எனில் ((ns->block_size * மொத்தம்) > NVME_PAGE_SIZE)
        திரும்பு nvme_io_xfer(ns, lba, buf, buf + NVME_PAGE_SIZE, மொத்தம், லிக);

single:
    /* One page is enough, don't expose anything else */
    திரும்பு nvme_io_xfer(ns, lba, buf, வெற்று, மொத்தம், லிக);

bounce:
    /* Use bounce buffer to make transfer */
    திரும்பு nvme_bounce_xfer(ns, lba, buf, மொத்தம், லிக);
}

நிலையான முழு
nvme_create_io_queues(கட்டமைப்பு nvme_ctrl *ctrl)
{
    எனில் (nvme_create_io_cq(ctrl, &ctrl->io_cq, 3))
        செல் err;

    எனில் (nvme_create_io_sq(ctrl, &ctrl->io_sq, 2, &ctrl->io_cq))
        செல் err_free_cq;

    திரும்பு 0;

 err_free_cq:
    nvme_destroy_cq(&ctrl->io_cq);
 err:
    திரும்பு -1;
}

/* Waits for CSTS.RDY to match rdy. Returns 0 on success. */
நிலையான முழு
nvme_wait_csts_rdy(கட்டமைப்பு nvme_ctrl *ctrl, unsigned rdy)
{
    u32 மாறிலி max_to = 500 /* ms */ * ((ctrl->reg->cap >> 24) & 0xFFU);
    u32 to = timer_calc(max_to);
    u32 csts;

    போது (rdy != ((csts = ctrl->reg->csts) & NVME_CSTS_RDY)) {
        yield();

        எனில் (csts & NVME_CSTS_FATAL) {
            dprintf(3, "NVMe fatal error during controller shutdown\n");
            திரும்பு -1;
        }

        எனில் (timer_check(to)) {
            warn_timeout();
            திரும்பு -1;
        }
    }

    திரும்பு 0;
}

/* Returns 0 on success. */
நிலையான முழு
nvme_controller_enable(கட்டமைப்பு nvme_ctrl *ctrl)
{
    முழு rc;

    pci_enable_busmaster(ctrl->pci);

    /* Turn the controller off. */
    ctrl->reg->cc = 0;
    எனில் (nvme_wait_csts_rdy(ctrl, 0)) {
        dprintf(2, "NVMe fatal error during controller shutdown\n");
        திரும்பு -1;
    }

    ctrl->doorbell_stride = 4U << ((ctrl->reg->cap >> 32) & 0xF);

    rc = nvme_init_cq(ctrl, &ctrl->admin_cq, 1,
                      NVME_PAGE_SIZE / அளவு(கட்டமைப்பு nvme_cqe));
    எனில் (rc) {
        திரும்பு -1;
    }

    rc = nvme_init_sq(ctrl, &ctrl->admin_sq, 0,
                      NVME_PAGE_SIZE / அளவு(கட்டமைப்பு nvme_sqe), &ctrl->admin_cq);
    எனில் (rc) {
        செல் err_destroy_admin_cq;
    }

    ctrl->reg->aqa = ctrl->admin_cq.common.mask << 16
        | ctrl->admin_sq.common.mask;

    ctrl->reg->asq = (u32)ctrl->admin_sq.sqe;
    ctrl->reg->acq = (u32)ctrl->admin_cq.cqe;

    dprintf(3, "  admin submission queue: %p\n", ctrl->admin_sq.sqe);
    dprintf(3, "  admin completion queue: %p\n", ctrl->admin_cq.cqe);

    ctrl->reg->cc = NVME_CC_EN | (NVME_CQE_SIZE_LOG << 20)
        | (NVME_SQE_SIZE_LOG << 16 /* IOSQES */);

    எனில் (nvme_wait_csts_rdy(ctrl, 1)) {
        dprintf(2, "NVMe fatal error while enabling controller\n");
        செல் err_destroy_admin_sq;
    }

    /* The admin queue is set up and the controller is ready. Let's figure out
       what namespaces we have. */

    கட்டமைப்பு nvme_identify_ctrl *identify = nvme_admin_identify_ctrl(ctrl);

    எனில் (!identify) {
        dprintf(2, "NVMe couldn't identify controller.\n");
        செல் err_destroy_admin_sq;
    }

    dprintf(3, "NVMe has %u namespace%s.\n",
            identify->nn, (identify->nn == 1) ? "" : "s");

    ctrl->ns_மொத்தம் = identify->nn;
    u8 mdts = identify->mdts;
    விடுவி(identify);

    எனில் ((ctrl->ns_மொத்தம் == 0) || nvme_create_io_queues(ctrl)) {
        /* No point to continue, if the controller says it doesn't have
           namespaces or we couldn't create I/O queues. */
        செல் err_destroy_admin_sq;
    }

    /* Populate namespace IDs */
    முழு ns_idx;
    க்கு (ns_idx = 0; ns_idx < ctrl->ns_மொத்தம்; ns_idx++) {
        nvme_probe_ns(ctrl, ns_idx, mdts);
    }

    dprintf(3, "NVMe initialization complete!\n");
    திரும்பு 0;

 err_destroy_admin_sq:
    nvme_destroy_sq(&ctrl->admin_sq);
 err_destroy_admin_cq:
    nvme_destroy_cq(&ctrl->admin_cq);
    திரும்பு -1;
}

/* Initialize an NVMe controller and detect its drives. */
நிலையான வெறுமை
nvme_controller_setup(வெறுமை *opaque)
{
    u8 skip_nonbootable = is_bootprio_strict();
    கட்டமைப்பு pci_device *pci = opaque;

    எனில் (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
        dprintf(1, "skipping init of a non-bootable NVMe at %pP\n",
                pci);
        செல் err;
    }

    கட்டமைப்பு nvme_reg நிலையற்ற *reg = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    எனில் (!reg)
        திரும்பு;

    u32 version = reg->vs;
    dprintf(3, "Found NVMe controller with version %u.%u.%u.\n",
            version >> 16, (version >> 8) & 0xFF, version & 0xFF);
    dprintf(3, "  Capabilities %016llx\n", reg->cap);

    எனில் (~reg->cap & NVME_CAP_CSS_NVME) {
        dprintf(3, "Controller doesn't speak NVMe command set. Skipping.\n");
        செல் err;
    }

    கட்டமைப்பு nvme_ctrl *ctrl = malloc_உயர்வு(அளவு(*ctrl));
    எனில் (!ctrl) {
        warn_noalloc();
        செல் err;
    }

    memset(ctrl, 0, அளவு(*ctrl));

    ctrl->reg = reg;
    ctrl->pci = pci;

    எனில் (nvme_controller_enable(ctrl)) {
        செல் err_free_ctrl;
    }

    திரும்பு;

 err_free_ctrl:
    விடுவி(ctrl);
 err:
    dprintf(2, "Failed to enable NVMe controller.\n");
}

// Locate and init NVMe controllers
நிலையான வெறுமை
nvme_scan(வெறுமை)
{
    // Scan PCI bus for NVMe adapters
    கட்டமைப்பு pci_device *pci;

    foreachpci(pci) {
        எனில் (pci->class != PCI_CLASS_STORAGE_NVME)
            தொடர்;
        எனில் (pci->prog_if != 2 /* as of NVM 1.0e */) {
            dprintf(3, "Found incompatble NVMe: prog-if=%02x\n", pci->prog_if);
            தொடர்;
        }

        run_thread(nvme_controller_setup, pci);
    }
}

நிலையான முழு
nvme_cmd_readwrite(கட்டமைப்பு nvme_namespace *ns, கட்டமைப்பு disk_op_s *op, முழு லிக)
{
    முழு i;
    க்கு (i = 0; i < op->மொத்தம்;) {
        u16 blocks_remaining = op->மொத்தம் - i;
        எழுத்து *op_buf = op->buf_fl + i * ns->block_size;
        முழு blocks = nvme_prpl_xfer(ns, op->lba + i, op_buf,
                                    blocks_remaining, லிக);
        எனில் (blocks < 0)
            திரும்பு DISK_RET_EBADTRACK;
        i += blocks;
    }

    திரும்பு DISK_RET_SUCCESS;
}

முழு
nvme_process_op(கட்டமைப்பு disk_op_s *op)
{
    எனில் (!CONFIG_NVME)
        திரும்பு DISK_RET_SUCCESS;

    கட்டமைப்பு nvme_namespace *ns = container_of(op->drive_fl, கட்டமைப்பு nvme_namespace,
                                             drive);

    தேர்வு (op->command) {
    நிலை CMD_READ:
    நிலை CMD_WRITE:
        திரும்பு nvme_cmd_readwrite(ns, op, op->command == CMD_WRITE);
    இயல்பு:
        திரும்பு default_process_op(op);
    }
}

வெறுமை
nvme_setup(வெறுமை)
{
    ASSERT32FLAT();
    எனில் (!CONFIG_NVME)
        திரும்பு;

    dprintf(3, "init nvme\n");
    nvme_scan();
}

/* EOF */

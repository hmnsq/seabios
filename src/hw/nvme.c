// Low level NVMe disk access
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "blockcmd.h"
#അന്തര്ഭാവ "malloc.h" // malloc_high
#അന്തര്ഭാവ "output.h" // dprintf
#അന്തര്ഭാവ "pci.h"
#അന്തര്ഭാവ "pci_ids.h" // PCI_CLASS_STORAGE_NVME
#അന്തര്ഭാവ "pci_regs.h" // PCI_BASE_ADDRESS_0
#അന്തര്ഭാവ "pcidevice.h" // foreachpci
#അന്തര്ഭാവ "stacks.h" // yield
#അന്തര്ഭാവ "std/disk.h" // DISK_RET_
#അന്തര്ഭാവ "string.h" // memset
#അന്തര്ഭാവ "util.h" // boot_add_hd
#അന്തര്ഭാവ "x86.h" // readl

#അന്തര്ഭാവ "nvme.h"
#അന്തര്ഭാവ "nvme-int.h"

// Page aligned "dma bounce buffer" of size NVME_PAGE_SIZE in high memory
സ്ഥിതം ശൂന്യം *nvme_dma_buffer;

സ്ഥിതം ശൂന്യം *
zalloc_page_aligned(ഘടന zone_s *zone, u32 size)
{
    ശൂന്യം *res = _malloc(zone, size, NVME_PAGE_SIZE);
    എങ്കിൽ (res) memset(res, 0, size);
    തിരികെ res;
}

സ്ഥിതം ശൂന്യം
nvme_init_queue_common(ഘടന nvme_ctrl *ctrl, ഘടന nvme_queue *q, u16 q_idx,
                       u16 length)
{
    memset(q, 0, വലുപ്പം(*q));
    q->dbl = (u32 *)((അക്ഷരം *)ctrl->reg + 0x1000 + q_idx * ctrl->doorbell_stride);
    dprintf(3, " q %p q_idx %u dbl %p\n", q, q_idx, q->dbl);
    q->mask = length - 1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
nvme_init_sq(ഘടന nvme_ctrl *ctrl, ഘടന nvme_sq *sq, u16 q_idx, u16 length,
             ഘടന nvme_cq *cq)
{
    nvme_init_queue_common(ctrl, &sq->common, q_idx, length);
    sq->sqe = zalloc_page_aligned(&Zone_മുകളിലെ, വലുപ്പം(*sq->sqe) * length);

    എങ്കിൽ (!sq->sqe) {
        warn_noalloc();
        തിരികെ -1;
    }

    dprintf(3, "sq %p q_idx %u sqe %p\n", sq, q_idx, sq->sqe);
    sq->cq   = cq;
    sq->head = 0;
    sq->tail = 0;

    തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
nvme_init_cq(ഘടന nvme_ctrl *ctrl, ഘടന nvme_cq *cq, u16 q_idx, u16 length)
{
    nvme_init_queue_common(ctrl, &cq->common, q_idx, length);
    cq->cqe = zalloc_page_aligned(&Zone_മുകളിലെ, വലുപ്പം(*cq->cqe) * length);
    എങ്കിൽ (!cq->cqe) {
        warn_noalloc();
        തിരികെ -1;
    }

    cq->head = 0;

    /* All CQE phase bits are initialized to zero. This means initially we wait
       for the host controller to set these to 1. */
    cq->phase = 1;

    തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
nvme_poll_cq(ഘടന nvme_cq *cq)
{
    u32 dw3 = readl(&cq->cqe[cq->head].dword[3]);
    തിരികെ (!!(dw3 & NVME_CQE_DW3_P) == cq->phase);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
nvme_is_cqe_success(ഘടന nvme_cqe സ്ഥിരം *cqe)
{
    തിരികെ ((cqe->status >> 1) & 0xFF) == 0;
}

സ്ഥിതം ഘടന nvme_cqe
nvme_error_cqe(ശൂന്യം)
{
    ഘടന nvme_cqe r;

    /* 0xFF is a vendor specific status code != success. Should be okay for
       indicating failure. */
    memset(&r, 0xFF, വലുപ്പം(r));
    തിരികെ r;
}

സ്ഥിതം ഘടന nvme_cqe
nvme_consume_cqe(ഘടന nvme_sq *sq)
{
    ഘടന nvme_cq *cq = sq->cq;

    എങ്കിൽ (!nvme_poll_cq(cq)) {
        /* Cannot consume a completion queue entry, if there is none ready. */
        തിരികെ nvme_error_cqe();
    }

    ഘടന nvme_cqe *cqe = &cq->cqe[cq->head];
    u16 cq_next_head = (cq->head + 1) & cq->common.mask;
    dprintf(4, "cq %p head %u -> %u\n", cq, cq->head, cq_next_head);
    എങ്കിൽ (cq_next_head < cq->head) {
        dprintf(3, "cq %p wrap\n", cq);
        cq->phase = ~cq->phase;
    }
    cq->head = cq_next_head;

    /* Update the submission queue head. */
    എങ്കിൽ (cqe->sq_head != sq->head) {
        sq->head = cqe->sq_head;
        dprintf(4, "sq %p advanced to %u\n", sq, cqe->sq_head);
    }

    /* Tell the controller that we consumed the completion. */
    writel(cq->common.dbl, cq->head);

    തിരികെ *cqe;
}

സ്ഥിതം ഘടന nvme_cqe
nvme_wait(ഘടന nvme_sq *sq)
{
    സ്ഥിതം സ്ഥിരം unsigned nvme_timeout = 5000 /* ms */;
    u32 to = timer_calc(nvme_timeout);
    ആകുമ്പോൾ (!nvme_poll_cq(sq->cq)) {
        yield();

        എങ്കിൽ (timer_check(to)) {
            warn_timeout();
            തിരികെ nvme_error_cqe();
        }
    }

    തിരികെ nvme_consume_cqe(sq);
}

/* Returns the next submission queue entry (or NULL if the queue is full). It
   also fills out Command Dword 0 and clears the rest. */
സ്ഥിതം ഘടന nvme_sqe *
nvme_get_next_sqe(ഘടന nvme_sq *sq, u8 opc, ശൂന്യം *metadata, ശൂന്യം *data, ശൂന്യം *data2)
{
    എങ്കിൽ (((sq->head + 1) & sq->common.mask) == sq->tail) {
        dprintf(3, "submission queue is full\n");
        തിരികെ ശൂന്യ;
    }

    ഘടന nvme_sqe *sqe = &sq->sqe[sq->tail];
    dprintf(4, "sq %p next_sqe %u\n", sq, sq->tail);

    memset(sqe, 0, വലുപ്പം(*sqe));
    sqe->cdw0 = opc | (sq->tail << 16 /* CID */);
    sqe->mptr = (u32)metadata;
    sqe->dptr_prp1 = (u32)data;
    sqe->dptr_prp2 = (u32)data2;

    തിരികെ sqe;
}

/* Call this after you've filled out an sqe that you've got from nvme_get_next_sqe. */
സ്ഥിതം ശൂന്യം
nvme_commit_sqe(ഘടന nvme_sq *sq)
{
    dprintf(4, "sq %p commit_sqe %u\n", sq, sq->tail);
    sq->tail = (sq->tail + 1) & sq->common.mask;
    writel(sq->common.dbl, sq->tail);
}

/* Perform an identify command on the admin queue and return the resulting
   buffer. This may be a NULL pointer, if something failed. This function
   cannot be used after initialization, because it uses buffers in tmp zone. */
സ്ഥിതം സംഘം nvme_identify *
nvme_admin_identify(ഘടന nvme_ctrl *ctrl, u8 cns, u32 nsid)
{
    സംഘം nvme_identify *identify_buf = zalloc_page_aligned(&Zone_Tmp_മുകളിലെ, 4096);
    എങ്കിൽ (!identify_buf) {
        /* Could not allocate identify buffer. */
        warn_internalerror();
        തിരികെ ശൂന്യ;
    }

    ഘടന nvme_sqe *cmd_identify;
    cmd_identify = nvme_get_next_sqe(&ctrl->admin_sq,
                                     NVME_SQE_OPC_ADMIN_IDENTIFY, ശൂന്യ,
                                     identify_buf, ശൂന്യ);

    എങ്കിൽ (!cmd_identify) {
        warn_internalerror();
        പോകുക error;
    }

    cmd_identify->nsid = nsid;
    cmd_identify->dword[10] = cns;

    nvme_commit_sqe(&ctrl->admin_sq);

    ഘടന nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    എങ്കിൽ (!nvme_is_cqe_success(&cqe)) {
        പോകുക error;
    }

    തിരികെ identify_buf;
 error:
    സ്വതന്ത്രം(identify_buf);
    തിരികെ ശൂന്യ;
}

സ്ഥിതം ഘടന nvme_identify_ctrl *
nvme_admin_identify_ctrl(ഘടന nvme_ctrl *ctrl)
{
    തിരികെ &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_CTRL, 0)->ctrl;
}

സ്ഥിതം ഘടന nvme_identify_ns *
nvme_admin_identify_ns(ഘടന nvme_ctrl *ctrl, u32 ns_id)
{
    തിരികെ &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_NS,
                                ns_id)->ns;
}

സ്ഥിതം ശൂന്യം
nvme_probe_ns(ഘടന nvme_ctrl *ctrl, u32 ns_idx, u8 mdts)
{
    u32 ns_id = ns_idx + 1;

    ഘടന nvme_identify_ns *id = nvme_admin_identify_ns(ctrl, ns_id);
    എങ്കിൽ (!id) {
        dprintf(2, "NVMe couldn't identify namespace %u.\n", ns_id);
        പോകുക free_buffer;
    }

    u8 current_lba_format = id->flbas & 0xF;
    എങ്കിൽ (current_lba_format > id->nlbaf) {
        dprintf(2, "NVMe NS %u: current LBA format %u is beyond what the "
                " namespace supports (%u)?\n",
                ns_id, current_lba_format, id->nlbaf + 1);
        പോകുക free_buffer;
    }

    എങ്കിൽ (!id->nsze) {
        dprintf(2, "NVMe NS %u is inactive.\n", ns_id);
        പോകുക free_buffer;
    }

    എങ്കിൽ (!nvme_dma_buffer) {
        nvme_dma_buffer = zalloc_page_aligned(&Zone_മുകളിലെ, NVME_PAGE_SIZE);
        എങ്കിൽ (!nvme_dma_buffer) {
            warn_noalloc();
            പോകുക free_buffer;
        }
    }

    ഘടന nvme_namespace *ns = malloc_fseg(വലുപ്പം(*ns));
    എങ്കിൽ (!ns) {
        warn_noalloc();
        പോകുക free_buffer;
    }
    memset(ns, 0, വലുപ്പം(*ns));
    ns->ctrl  = ctrl;
    ns->ns_id = ns_id;
    ns->lba_എണ്ണം = id->nsze;

    ഘടന nvme_lba_format *fmt = &id->lbaf[current_lba_format];

    ns->block_size    = 1U << fmt->lbads;
    ns->metadata_size = fmt->ms;

    എങ്കിൽ (ns->block_size > NVME_PAGE_SIZE) {
        /* If we see devices that trigger this path, we need to increase our
           buffer size. */
        warn_internalerror();
        സ്വതന്ത്രം(ns);
        പോകുക free_buffer;
    }

    ns->drive.cntl_id   = ns_idx;
    ns->drive.removable = 0;
    ns->drive.type      = DTYPE_NVME;
    ns->drive.blksize   = ns->block_size;
    ns->drive.sectors   = ns->lba_എണ്ണം;

    എങ്കിൽ (mdts) {
        ns->max_req_size = ((1U << mdts) * NVME_PAGE_SIZE) / ns->block_size;
        dprintf(3, "NVME NS %u max request size: %d sectors\n",
                ns_id, ns->max_req_size);
    } അല്ലെങ്കിൽ {
        ns->max_req_size = -1U;
    }

    അക്ഷരം *desc = znprintf(MAXDESCSIZE, "NVMe NS %u: %llu MiB (%llu %u-byte "
                          "blocks + %u-byte metadata)",
                          ns_id, (ns->lba_എണ്ണം * ns->block_size) >> 20,
                          ns->lba_എണ്ണം, ns->block_size, ns->metadata_size);

    dprintf(3, "%s\n", desc);
    boot_add_hd(&ns->drive, desc, bootprio_find_pci_device(ctrl->pci));

free_buffer:
    സ്വതന്ത്രം (id);
}


/* Release memory allocated for a completion queue */
സ്ഥിതം ശൂന്യം
nvme_destroy_cq(ഘടന nvme_cq *cq)
{
    സ്വതന്ത്രം(cq->cqe);
    cq->cqe = ശൂന്യ;
}

/* Release memory allocated for a submission queue */
സ്ഥിതം ശൂന്യം
nvme_destroy_sq(ഘടന nvme_sq *sq)
{
    സ്വതന്ത്രം(sq->sqe);
    sq->sqe = ശൂന്യ;
}

/* Returns 0 on success. */
സ്ഥിതം പൂർണ്ണസംഖ്യ
nvme_create_io_cq(ഘടന nvme_ctrl *ctrl, ഘടന nvme_cq *cq, u16 q_idx)
{
    പൂർണ്ണസംഖ്യ rc;
    ഘടന nvme_sqe *cmd_create_cq;
    u32 length = 1 + (ctrl->reg->cap & 0xffff);
    എങ്കിൽ (length > NVME_PAGE_SIZE / വലുപ്പം(ഘടന nvme_cqe))
        length = NVME_PAGE_SIZE / വലുപ്പം(ഘടന nvme_cqe);

    rc = nvme_init_cq(ctrl, cq, q_idx, length);
    എങ്കിൽ (rc) {
        പോകുക err;
    }

    cmd_create_cq = nvme_get_next_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_CQ, ശൂന്യ,
                                      cq->cqe, ശൂന്യ);
    എങ്കിൽ (!cmd_create_cq) {
        പോകുക err_destroy_cq;
    }

    cmd_create_cq->dword[10] = (cq->common.mask << 16) | (q_idx >> 1);
    cmd_create_cq->dword[11] = 1 /* physically contiguous */;

    nvme_commit_sqe(&ctrl->admin_sq);

    ഘടന nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    എങ്കിൽ (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io cq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        പോകുക err_destroy_cq;
    }

    തിരികെ 0;

err_destroy_cq:
    nvme_destroy_cq(cq);
err:
    തിരികെ -1;
}

/* Returns 0 on success. */
സ്ഥിതം പൂർണ്ണസംഖ്യ
nvme_create_io_sq(ഘടന nvme_ctrl *ctrl, ഘടന nvme_sq *sq, u16 q_idx, ഘടന nvme_cq *cq)
{
    പൂർണ്ണസംഖ്യ rc;
    ഘടന nvme_sqe *cmd_create_sq;
    u32 length = 1 + (ctrl->reg->cap & 0xffff);
    എങ്കിൽ (length > NVME_PAGE_SIZE / വലുപ്പം(ഘടന nvme_cqe))
        length = NVME_PAGE_SIZE / വലുപ്പം(ഘടന nvme_cqe);

    rc = nvme_init_sq(ctrl, sq, q_idx, length, cq);
    എങ്കിൽ (rc) {
        പോകുക err;
    }

    cmd_create_sq = nvme_get_next_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_SQ, ശൂന്യ,
                                      sq->sqe, ശൂന്യ);
    എങ്കിൽ (!cmd_create_sq) {
        പോകുക err_destroy_sq;
    }

    cmd_create_sq->dword[10] = (sq->common.mask << 16) | (q_idx >> 1);
    cmd_create_sq->dword[11] = (q_idx >> 1) << 16 | 1 /* contiguous */;
    dprintf(3, "sq %p create dword10 %08x dword11 %08x\n", sq,
            cmd_create_sq->dword[10], cmd_create_sq->dword[11]);

    nvme_commit_sqe(&ctrl->admin_sq);

    ഘടന nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    എങ്കിൽ (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io sq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);
        പോകുക err_destroy_sq;
    }

    തിരികെ 0;

err_destroy_sq:
    nvme_destroy_sq(sq);
err:
    തിരികെ -1;
}

/* Reads count sectors into buf. The buffer cannot cross page boundaries. */
സ്ഥിതം പൂർണ്ണസംഖ്യ
nvme_io_xfer(ഘടന nvme_namespace *ns, u64 lba, ശൂന്യം *prp1, ശൂന്യം *prp2,
             u16 എണ്ണം, പൂർണ്ണസംഖ്യ ലിഖ)
{
    എങ്കിൽ (((u32)prp1 & 0x3) || ((u32)prp2 & 0x3)) {
        /* Buffer is misaligned */
        warn_internalerror();
        തിരികെ -1;
    }

    ഘടന nvme_sqe *io_read = nvme_get_next_sqe(&ns->ctrl->io_sq,
                                                 ലിഖ ? NVME_SQE_OPC_IO_WRITE
                                                       : NVME_SQE_OPC_IO_READ,
                                                 ശൂന്യ, prp1, prp2);
    io_read->nsid = ns->ns_id;
    io_read->dword[10] = (u32)lba;
    io_read->dword[11] = (u32)(lba >> 32);
    io_read->dword[12] = (1U << 31 /* limited retry */) | (എണ്ണം - 1);

    nvme_commit_sqe(&ns->ctrl->io_sq);

    ഘടന nvme_cqe cqe = nvme_wait(&ns->ctrl->io_sq);

    എങ്കിൽ (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "read io: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        തിരികെ -1;
    }

    dprintf(5, "ns %u %s lba %llu+%u\n", ns->ns_id, ലിഖ ? "write" : "read",
            lba, എണ്ണം);
    തിരികെ എണ്ണം;
}

// Transfer up to one page of data using the internal dma bounce buffer
സ്ഥിതം പൂർണ്ണസംഖ്യ
nvme_bounce_xfer(ഘടന nvme_namespace *ns, u64 lba, ശൂന്യം *buf, u16 എണ്ണം,
                 പൂർണ്ണസംഖ്യ ലിഖ)
{
    u16 സ്ഥിരം max_blocks = NVME_PAGE_SIZE / ns->block_size;
    u16 blocks = എണ്ണം < max_blocks ? എണ്ണം : max_blocks;

    എങ്കിൽ (ലിഖ)
        memcpy(nvme_dma_buffer, buf, blocks * ns->block_size);

    പൂർണ്ണസംഖ്യ res = nvme_io_xfer(ns, lba, nvme_dma_buffer, ശൂന്യ, blocks, ലിഖ);

    എങ്കിൽ (!ലിഖ && res >= 0)
        memcpy(buf, nvme_dma_buffer, res * ns->block_size);

    തിരികെ res;
}

#define NVME_MAX_PRPL_ENTRIES 15 /* Allows requests up to 64kb */

// Transfer data using page list (if applicable)
സ്ഥിതം പൂർണ്ണസംഖ്യ
nvme_prpl_xfer(ഘടന nvme_namespace *ns, u64 lba, ശൂന്യം *buf, u16 എണ്ണം,
               പൂർണ്ണസംഖ്യ ലിഖ)
{
    u32 base = (long)buf;
    s32 size;

    എങ്കിൽ (എണ്ണം > ns->max_req_size)
        എണ്ണം = ns->max_req_size;

    size = എണ്ണം * ns->block_size;
    /* Special case for transfers that fit into PRP1, but are unaligned */
    എങ്കിൽ (((size + (base & ~NVME_PAGE_MASK)) <= NVME_PAGE_SIZE))
        പോകുക single;

    /* Every request has to be page aligned */
    എങ്കിൽ (base & ~NVME_PAGE_MASK)
        പോകുക bounce;

    /* Make sure a full block fits into the last chunk */
    എങ്കിൽ (size & (ns->block_size - 1ULL))
        പോകുക bounce;

    /* Build PRP list if we need to describe more than 2 pages */
    എങ്കിൽ ((ns->block_size * എണ്ണം) > (NVME_PAGE_SIZE * 2)) {
        u32 prpl_len = 0;
        u64 *prpl = nvme_dma_buffer;
        പൂർണ്ണസംഖ്യ first_page = 1;
        വേണ്ടി (; size > 0; base += NVME_PAGE_SIZE, size -= NVME_PAGE_SIZE) {
            എങ്കിൽ (first_page) {
                /* First page is special */
                first_page = 0;
                തുടരുക;
            }
            എങ്കിൽ (prpl_len >= NVME_MAX_PRPL_ENTRIES)
                പോകുക bounce;
            prpl[prpl_len++] = base;
        }
        തിരികെ nvme_io_xfer(ns, lba, buf, prpl, എണ്ണം, ലിഖ);
    }

    /* Directly embed the 2nd page if we only need 2 pages */
    എങ്കിൽ ((ns->block_size * എണ്ണം) > NVME_PAGE_SIZE)
        തിരികെ nvme_io_xfer(ns, lba, buf, buf + NVME_PAGE_SIZE, എണ്ണം, ലിഖ);

single:
    /* One page is enough, don't expose anything else */
    തിരികെ nvme_io_xfer(ns, lba, buf, ശൂന്യ, എണ്ണം, ലിഖ);

bounce:
    /* Use bounce buffer to make transfer */
    തിരികെ nvme_bounce_xfer(ns, lba, buf, എണ്ണം, ലിഖ);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
nvme_create_io_queues(ഘടന nvme_ctrl *ctrl)
{
    എങ്കിൽ (nvme_create_io_cq(ctrl, &ctrl->io_cq, 3))
        പോകുക err;

    എങ്കിൽ (nvme_create_io_sq(ctrl, &ctrl->io_sq, 2, &ctrl->io_cq))
        പോകുക err_free_cq;

    തിരികെ 0;

 err_free_cq:
    nvme_destroy_cq(&ctrl->io_cq);
 err:
    തിരികെ -1;
}

/* Waits for CSTS.RDY to match rdy. Returns 0 on success. */
സ്ഥിതം പൂർണ്ണസംഖ്യ
nvme_wait_csts_rdy(ഘടന nvme_ctrl *ctrl, unsigned rdy)
{
    u32 സ്ഥിരം max_to = 500 /* ms */ * ((ctrl->reg->cap >> 24) & 0xFFU);
    u32 to = timer_calc(max_to);
    u32 csts;

    ആകുമ്പോൾ (rdy != ((csts = ctrl->reg->csts) & NVME_CSTS_RDY)) {
        yield();

        എങ്കിൽ (csts & NVME_CSTS_FATAL) {
            dprintf(3, "NVMe fatal error during controller shutdown\n");
            തിരികെ -1;
        }

        എങ്കിൽ (timer_check(to)) {
            warn_timeout();
            തിരികെ -1;
        }
    }

    തിരികെ 0;
}

/* Returns 0 on success. */
സ്ഥിതം പൂർണ്ണസംഖ്യ
nvme_controller_enable(ഘടന nvme_ctrl *ctrl)
{
    പൂർണ്ണസംഖ്യ rc;

    pci_enable_busmaster(ctrl->pci);

    /* Turn the controller off. */
    ctrl->reg->cc = 0;
    എങ്കിൽ (nvme_wait_csts_rdy(ctrl, 0)) {
        dprintf(2, "NVMe fatal error during controller shutdown\n");
        തിരികെ -1;
    }

    ctrl->doorbell_stride = 4U << ((ctrl->reg->cap >> 32) & 0xF);

    rc = nvme_init_cq(ctrl, &ctrl->admin_cq, 1,
                      NVME_PAGE_SIZE / വലുപ്പം(ഘടന nvme_cqe));
    എങ്കിൽ (rc) {
        തിരികെ -1;
    }

    rc = nvme_init_sq(ctrl, &ctrl->admin_sq, 0,
                      NVME_PAGE_SIZE / വലുപ്പം(ഘടന nvme_sqe), &ctrl->admin_cq);
    എങ്കിൽ (rc) {
        പോകുക err_destroy_admin_cq;
    }

    ctrl->reg->aqa = ctrl->admin_cq.common.mask << 16
        | ctrl->admin_sq.common.mask;

    ctrl->reg->asq = (u32)ctrl->admin_sq.sqe;
    ctrl->reg->acq = (u32)ctrl->admin_cq.cqe;

    dprintf(3, "  admin submission queue: %p\n", ctrl->admin_sq.sqe);
    dprintf(3, "  admin completion queue: %p\n", ctrl->admin_cq.cqe);

    ctrl->reg->cc = NVME_CC_EN | (NVME_CQE_SIZE_LOG << 20)
        | (NVME_SQE_SIZE_LOG << 16 /* IOSQES */);

    എങ്കിൽ (nvme_wait_csts_rdy(ctrl, 1)) {
        dprintf(2, "NVMe fatal error while enabling controller\n");
        പോകുക err_destroy_admin_sq;
    }

    /* The admin queue is set up and the controller is ready. Let's figure out
       what namespaces we have. */

    ഘടന nvme_identify_ctrl *identify = nvme_admin_identify_ctrl(ctrl);

    എങ്കിൽ (!identify) {
        dprintf(2, "NVMe couldn't identify controller.\n");
        പോകുക err_destroy_admin_sq;
    }

    dprintf(3, "NVMe has %u namespace%s.\n",
            identify->nn, (identify->nn == 1) ? "" : "s");

    ctrl->ns_എണ്ണം = identify->nn;
    u8 mdts = identify->mdts;
    സ്വതന്ത്രം(identify);

    എങ്കിൽ ((ctrl->ns_എണ്ണം == 0) || nvme_create_io_queues(ctrl)) {
        /* No point to continue, if the controller says it doesn't have
           namespaces or we couldn't create I/O queues. */
        പോകുക err_destroy_admin_sq;
    }

    /* Populate namespace IDs */
    പൂർണ്ണസംഖ്യ ns_idx;
    വേണ്ടി (ns_idx = 0; ns_idx < ctrl->ns_എണ്ണം; ns_idx++) {
        nvme_probe_ns(ctrl, ns_idx, mdts);
    }

    dprintf(3, "NVMe initialization complete!\n");
    തിരികെ 0;

 err_destroy_admin_sq:
    nvme_destroy_sq(&ctrl->admin_sq);
 err_destroy_admin_cq:
    nvme_destroy_cq(&ctrl->admin_cq);
    തിരികെ -1;
}

/* Initialize an NVMe controller and detect its drives. */
സ്ഥിതം ശൂന്യം
nvme_controller_setup(ശൂന്യം *opaque)
{
    u8 skip_nonbootable = is_bootprio_strict();
    ഘടന pci_device *pci = opaque;

    എങ്കിൽ (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
        dprintf(1, "skipping init of a non-bootable NVMe at %pP\n",
                pci);
        പോകുക err;
    }

    ഘടന nvme_reg അസ്ഥിര *reg = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    എങ്കിൽ (!reg)
        തിരികെ;

    u32 version = reg->vs;
    dprintf(3, "Found NVMe controller with version %u.%u.%u.\n",
            version >> 16, (version >> 8) & 0xFF, version & 0xFF);
    dprintf(3, "  Capabilities %016llx\n", reg->cap);

    എങ്കിൽ (~reg->cap & NVME_CAP_CSS_NVME) {
        dprintf(3, "Controller doesn't speak NVMe command set. Skipping.\n");
        പോകുക err;
    }

    ഘടന nvme_ctrl *ctrl = malloc_മുകളിലെ(വലുപ്പം(*ctrl));
    എങ്കിൽ (!ctrl) {
        warn_noalloc();
        പോകുക err;
    }

    memset(ctrl, 0, വലുപ്പം(*ctrl));

    ctrl->reg = reg;
    ctrl->pci = pci;

    എങ്കിൽ (nvme_controller_enable(ctrl)) {
        പോകുക err_free_ctrl;
    }

    തിരികെ;

 err_free_ctrl:
    സ്വതന്ത്രം(ctrl);
 err:
    dprintf(2, "Failed to enable NVMe controller.\n");
}

// Locate and init NVMe controllers
സ്ഥിതം ശൂന്യം
nvme_scan(ശൂന്യം)
{
    // Scan PCI bus for NVMe adapters
    ഘടന pci_device *pci;

    foreachpci(pci) {
        എങ്കിൽ (pci->class != PCI_CLASS_STORAGE_NVME)
            തുടരുക;
        എങ്കിൽ (pci->prog_if != 2 /* as of NVM 1.0e */) {
            dprintf(3, "Found incompatble NVMe: prog-if=%02x\n", pci->prog_if);
            തുടരുക;
        }

        run_thread(nvme_controller_setup, pci);
    }
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
nvme_cmd_readwrite(ഘടന nvme_namespace *ns, ഘടന disk_op_s *op, പൂർണ്ണസംഖ്യ ലിഖ)
{
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i = 0; i < op->എണ്ണം;) {
        u16 blocks_remaining = op->എണ്ണം - i;
        അക്ഷരം *op_buf = op->buf_fl + i * ns->block_size;
        പൂർണ്ണസംഖ്യ blocks = nvme_prpl_xfer(ns, op->lba + i, op_buf,
                                    blocks_remaining, ലിഖ);
        എങ്കിൽ (blocks < 0)
            തിരികെ DISK_RET_EBADTRACK;
        i += blocks;
    }

    തിരികെ DISK_RET_SUCCESS;
}

പൂർണ്ണസംഖ്യ
nvme_process_op(ഘടന disk_op_s *op)
{
    എങ്കിൽ (!CONFIG_NVME)
        തിരികെ DISK_RET_SUCCESS;

    ഘടന nvme_namespace *ns = container_of(op->drive_fl, ഘടന nvme_namespace,
                                             drive);

    തിരഞ്ഞെടുപ്പ് (op->command) {
    അവസ്ഥ CMD_READ:
    അവസ്ഥ CMD_WRITE:
        തിരികെ nvme_cmd_readwrite(ns, op, op->command == CMD_WRITE);
    സ്ഥിരസ്ഥിതി:
        തിരികെ default_process_op(op);
    }
}

ശൂന്യം
nvme_setup(ശൂന്യം)
{
    ASSERT32FLAT();
    എങ്കിൽ (!CONFIG_NVME)
        തിരികെ;

    dprintf(3, "init nvme\n");
    nvme_scan();
}

/* EOF */

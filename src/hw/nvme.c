// Low level NVMe disk access
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "blockcmd.h"
#شامل "malloc.h" // malloc_high
#شامل "output.h" // dprintf
#شامل "pci.h"
#شامل "pci_ids.h" // PCI_CLASS_STORAGE_NVME
#شامل "pci_regs.h" // PCI_BASE_ADDRESS_0
#شامل "pcidevice.h" // foreachpci
#شامل "stacks.h" // yield
#شامل "std/disk.h" // DISK_RET_
#شامل "string.h" // memset
#شامل "util.h" // boot_add_hd
#شامل "x86.h" // readl

#شامل "nvme.h"
#شامل "nvme-int.h"

// Page aligned "dma bounce buffer" of size NVME_PAGE_SIZE in high memory
ایستا تهی *nvme_dma_buffer;

ایستا تهی *
zalloc_page_aligned(ساختار zone_s *zone, u32 size)
{
    تهی *res = _malloc(zone, size, NVME_PAGE_SIZE);
    اگر (res) memset(res, 0, size);
    بازگشت res;
}

ایستا تهی
nvme_init_queue_common(ساختار nvme_ctrl *ctrl, ساختار nvme_queue *q, u16 q_idx,
                       u16 length)
{
    memset(q, 0, اندازه(*q));
    q->dbl = (u32 *)((نویسه *)ctrl->reg + 0x1000 + q_idx * ctrl->doorbell_stride);
    dprintf(3, " q %p q_idx %u dbl %p\n", q, q_idx, q->dbl);
    q->mask = length - 1;
}

ایستا صحیح
nvme_init_sq(ساختار nvme_ctrl *ctrl, ساختار nvme_sq *sq, u16 q_idx, u16 length,
             ساختار nvme_cq *cq)
{
    nvme_init_queue_common(ctrl, &sq->common, q_idx, length);
    sq->sqe = zalloc_page_aligned(&Zone_بالا, اندازه(*sq->sqe) * length);

    اگر (!sq->sqe) {
        warn_noalloc();
        بازگشت -1;
    }

    dprintf(3, "sq %p q_idx %u sqe %p\n", sq, q_idx, sq->sqe);
    sq->cq   = cq;
    sq->head = 0;
    sq->tail = 0;

    بازگشت 0;
}

ایستا صحیح
nvme_init_cq(ساختار nvme_ctrl *ctrl, ساختار nvme_cq *cq, u16 q_idx, u16 length)
{
    nvme_init_queue_common(ctrl, &cq->common, q_idx, length);
    cq->cqe = zalloc_page_aligned(&Zone_بالا, اندازه(*cq->cqe) * length);
    اگر (!cq->cqe) {
        warn_noalloc();
        بازگشت -1;
    }

    cq->head = 0;

    /* All CQE phase bits are initialized to zero. This means initially we wait
       for the host controller to set these to 1. */
    cq->phase = 1;

    بازگشت 0;
}

ایستا صحیح
nvme_poll_cq(ساختار nvme_cq *cq)
{
    u32 dw3 = readl(&cq->cqe[cq->head].dword[3]);
    بازگشت (!!(dw3 & NVME_CQE_DW3_P) == cq->phase);
}

ایستا صحیح
nvme_is_cqe_success(ساختار nvme_cqe ثابت *cqe)
{
    بازگشت ((cqe->status >> 1) & 0xFF) == 0;
}

ایستا ساختار nvme_cqe
nvme_error_cqe(تهی)
{
    ساختار nvme_cqe r;

    /* 0xFF is a vendor specific status code != success. Should be okay for
       indicating failure. */
    memset(&r, 0xFF, اندازه(r));
    بازگشت r;
}

ایستا ساختار nvme_cqe
nvme_consume_cqe(ساختار nvme_sq *sq)
{
    ساختار nvme_cq *cq = sq->cq;

    اگر (!nvme_poll_cq(cq)) {
        /* Cannot consume a completion queue entry, if there is none ready. */
        بازگشت nvme_error_cqe();
    }

    ساختار nvme_cqe *cqe = &cq->cqe[cq->head];
    u16 cq_next_head = (cq->head + 1) & cq->common.mask;
    dprintf(4, "cq %p head %u -> %u\n", cq, cq->head, cq_next_head);
    اگر (cq_next_head < cq->head) {
        dprintf(3, "cq %p wrap\n", cq);
        cq->phase = ~cq->phase;
    }
    cq->head = cq_next_head;

    /* Update the submission queue head. */
    اگر (cqe->sq_head != sq->head) {
        sq->head = cqe->sq_head;
        dprintf(4, "sq %p advanced to %u\n", sq, cqe->sq_head);
    }

    /* Tell the controller that we consumed the completion. */
    writel(cq->common.dbl, cq->head);

    بازگشت *cqe;
}

ایستا ساختار nvme_cqe
nvme_wait(ساختار nvme_sq *sq)
{
    ایستا ثابت unsigned nvme_timeout = 5000 /* ms */;
    u32 to = timer_calc(nvme_timeout);
    وقتی (!nvme_poll_cq(sq->cq)) {
        yield();

        اگر (timer_check(to)) {
            warn_timeout();
            بازگشت nvme_error_cqe();
        }
    }

    بازگشت nvme_consume_cqe(sq);
}

/* Returns the next submission queue entry (or NULL if the queue is full). It
   also fills out Command Dword 0 and clears the rest. */
ایستا ساختار nvme_sqe *
nvme_get_next_sqe(ساختار nvme_sq *sq, u8 opc, تهی *metadata, تهی *data, تهی *data2)
{
    اگر (((sq->head + 1) & sq->common.mask) == sq->tail) {
        dprintf(3, "submission queue is full\n");
        بازگشت پوچ;
    }

    ساختار nvme_sqe *sqe = &sq->sqe[sq->tail];
    dprintf(4, "sq %p next_sqe %u\n", sq, sq->tail);

    memset(sqe, 0, اندازه(*sqe));
    sqe->cdw0 = opc | (sq->tail << 16 /* CID */);
    sqe->mptr = (u32)metadata;
    sqe->dptr_prp1 = (u32)data;
    sqe->dptr_prp2 = (u32)data2;

    بازگشت sqe;
}

/* Call this after you've filled out an sqe that you've got from nvme_get_next_sqe. */
ایستا تهی
nvme_commit_sqe(ساختار nvme_sq *sq)
{
    dprintf(4, "sq %p commit_sqe %u\n", sq, sq->tail);
    sq->tail = (sq->tail + 1) & sq->common.mask;
    writel(sq->common.dbl, sq->tail);
}

/* Perform an identify command on the admin queue and return the resulting
   buffer. This may be a NULL pointer, if something failed. This function
   cannot be used after initialization, because it uses buffers in tmp zone. */
ایستا اجتماع nvme_identify *
nvme_admin_identify(ساختار nvme_ctrl *ctrl, u8 cns, u32 nsid)
{
    اجتماع nvme_identify *identify_buf = zalloc_page_aligned(&Zone_Tmp_بالا, 4096);
    اگر (!identify_buf) {
        /* Could not allocate identify buffer. */
        warn_internalerror();
        بازگشت پوچ;
    }

    ساختار nvme_sqe *cmd_identify;
    cmd_identify = nvme_get_next_sqe(&ctrl->admin_sq,
                                     NVME_SQE_OPC_ADMIN_IDENTIFY, پوچ,
                                     identify_buf, پوچ);

    اگر (!cmd_identify) {
        warn_internalerror();
        برو error;
    }

    cmd_identify->nsid = nsid;
    cmd_identify->dword[10] = cns;

    nvme_commit_sqe(&ctrl->admin_sq);

    ساختار nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    اگر (!nvme_is_cqe_success(&cqe)) {
        برو error;
    }

    بازگشت identify_buf;
 error:
    آزاد(identify_buf);
    بازگشت پوچ;
}

ایستا ساختار nvme_identify_ctrl *
nvme_admin_identify_ctrl(ساختار nvme_ctrl *ctrl)
{
    بازگشت &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_CTRL, 0)->ctrl;
}

ایستا ساختار nvme_identify_ns *
nvme_admin_identify_ns(ساختار nvme_ctrl *ctrl, u32 ns_id)
{
    بازگشت &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_NS,
                                ns_id)->ns;
}

ایستا تهی
nvme_probe_ns(ساختار nvme_ctrl *ctrl, u32 ns_idx, u8 mdts)
{
    u32 ns_id = ns_idx + 1;

    ساختار nvme_identify_ns *id = nvme_admin_identify_ns(ctrl, ns_id);
    اگر (!id) {
        dprintf(2, "NVMe couldn't identify namespace %u.\n", ns_id);
        برو free_buffer;
    }

    u8 current_lba_format = id->flbas & 0xF;
    اگر (current_lba_format > id->nlbaf) {
        dprintf(2, "NVMe NS %u: current LBA format %u is beyond what the "
                " namespace supports (%u)?\n",
                ns_id, current_lba_format, id->nlbaf + 1);
        برو free_buffer;
    }

    اگر (!id->nsze) {
        dprintf(2, "NVMe NS %u is inactive.\n", ns_id);
        برو free_buffer;
    }

    اگر (!nvme_dma_buffer) {
        nvme_dma_buffer = zalloc_page_aligned(&Zone_بالا, NVME_PAGE_SIZE);
        اگر (!nvme_dma_buffer) {
            warn_noalloc();
            برو free_buffer;
        }
    }

    ساختار nvme_namespace *ns = malloc_fseg(اندازه(*ns));
    اگر (!ns) {
        warn_noalloc();
        برو free_buffer;
    }
    memset(ns, 0, اندازه(*ns));
    ns->ctrl  = ctrl;
    ns->ns_id = ns_id;
    ns->lba_شمار = id->nsze;

    ساختار nvme_lba_format *fmt = &id->lbaf[current_lba_format];

    ns->block_size    = 1U << fmt->lbads;
    ns->metadata_size = fmt->ms;

    اگر (ns->block_size > NVME_PAGE_SIZE) {
        /* If we see devices that trigger this path, we need to increase our
           buffer size. */
        warn_internalerror();
        آزاد(ns);
        برو free_buffer;
    }

    ns->drive.cntl_id   = ns_idx;
    ns->drive.removable = 0;
    ns->drive.type      = DTYPE_NVME;
    ns->drive.blksize   = ns->block_size;
    ns->drive.sectors   = ns->lba_شمار;

    اگر (mdts) {
        ns->max_req_size = ((1U << mdts) * NVME_PAGE_SIZE) / ns->block_size;
        dprintf(3, "NVME NS %u max request size: %d sectors\n",
                ns_id, ns->max_req_size);
    } وگرنه {
        ns->max_req_size = -1U;
    }

    نویسه *desc = znprintf(MAXDESCSIZE, "NVMe NS %u: %llu MiB (%llu %u-byte "
                          "blocks + %u-byte metadata)",
                          ns_id, (ns->lba_شمار * ns->block_size) >> 20,
                          ns->lba_شمار, ns->block_size, ns->metadata_size);

    dprintf(3, "%s\n", desc);
    boot_add_hd(&ns->drive, desc, bootprio_find_pci_device(ctrl->pci));

free_buffer:
    آزاد (id);
}


/* Release memory allocated for a completion queue */
ایستا تهی
nvme_destroy_cq(ساختار nvme_cq *cq)
{
    آزاد(cq->cqe);
    cq->cqe = پوچ;
}

/* Release memory allocated for a submission queue */
ایستا تهی
nvme_destroy_sq(ساختار nvme_sq *sq)
{
    آزاد(sq->sqe);
    sq->sqe = پوچ;
}

/* Returns 0 on success. */
ایستا صحیح
nvme_create_io_cq(ساختار nvme_ctrl *ctrl, ساختار nvme_cq *cq, u16 q_idx)
{
    صحیح rc;
    ساختار nvme_sqe *cmd_create_cq;
    u32 length = 1 + (ctrl->reg->cap & 0xffff);
    اگر (length > NVME_PAGE_SIZE / اندازه(ساختار nvme_cqe))
        length = NVME_PAGE_SIZE / اندازه(ساختار nvme_cqe);

    rc = nvme_init_cq(ctrl, cq, q_idx, length);
    اگر (rc) {
        برو err;
    }

    cmd_create_cq = nvme_get_next_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_CQ, پوچ,
                                      cq->cqe, پوچ);
    اگر (!cmd_create_cq) {
        برو err_destroy_cq;
    }

    cmd_create_cq->dword[10] = (cq->common.mask << 16) | (q_idx >> 1);
    cmd_create_cq->dword[11] = 1 /* physically contiguous */;

    nvme_commit_sqe(&ctrl->admin_sq);

    ساختار nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    اگر (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io cq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        برو err_destroy_cq;
    }

    بازگشت 0;

err_destroy_cq:
    nvme_destroy_cq(cq);
err:
    بازگشت -1;
}

/* Returns 0 on success. */
ایستا صحیح
nvme_create_io_sq(ساختار nvme_ctrl *ctrl, ساختار nvme_sq *sq, u16 q_idx, ساختار nvme_cq *cq)
{
    صحیح rc;
    ساختار nvme_sqe *cmd_create_sq;
    u32 length = 1 + (ctrl->reg->cap & 0xffff);
    اگر (length > NVME_PAGE_SIZE / اندازه(ساختار nvme_cqe))
        length = NVME_PAGE_SIZE / اندازه(ساختار nvme_cqe);

    rc = nvme_init_sq(ctrl, sq, q_idx, length, cq);
    اگر (rc) {
        برو err;
    }

    cmd_create_sq = nvme_get_next_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_SQ, پوچ,
                                      sq->sqe, پوچ);
    اگر (!cmd_create_sq) {
        برو err_destroy_sq;
    }

    cmd_create_sq->dword[10] = (sq->common.mask << 16) | (q_idx >> 1);
    cmd_create_sq->dword[11] = (q_idx >> 1) << 16 | 1 /* contiguous */;
    dprintf(3, "sq %p create dword10 %08x dword11 %08x\n", sq,
            cmd_create_sq->dword[10], cmd_create_sq->dword[11]);

    nvme_commit_sqe(&ctrl->admin_sq);

    ساختار nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    اگر (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io sq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);
        برو err_destroy_sq;
    }

    بازگشت 0;

err_destroy_sq:
    nvme_destroy_sq(sq);
err:
    بازگشت -1;
}

/* Reads count sectors into buf. The buffer cannot cross page boundaries. */
ایستا صحیح
nvme_io_xfer(ساختار nvme_namespace *ns, u64 lba, تهی *prp1, تهی *prp2,
             u16 شمار, صحیح اكتب)
{
    اگر (((u32)prp1 & 0x3) || ((u32)prp2 & 0x3)) {
        /* Buffer is misaligned */
        warn_internalerror();
        بازگشت -1;
    }

    ساختار nvme_sqe *io_read = nvme_get_next_sqe(&ns->ctrl->io_sq,
                                                 اكتب ? NVME_SQE_OPC_IO_WRITE
                                                       : NVME_SQE_OPC_IO_READ,
                                                 پوچ, prp1, prp2);
    io_read->nsid = ns->ns_id;
    io_read->dword[10] = (u32)lba;
    io_read->dword[11] = (u32)(lba >> 32);
    io_read->dword[12] = (1U << 31 /* limited retry */) | (شمار - 1);

    nvme_commit_sqe(&ns->ctrl->io_sq);

    ساختار nvme_cqe cqe = nvme_wait(&ns->ctrl->io_sq);

    اگر (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "read io: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        بازگشت -1;
    }

    dprintf(5, "ns %u %s lba %llu+%u\n", ns->ns_id, اكتب ? "write" : "read",
            lba, شمار);
    بازگشت شمار;
}

// Transfer up to one page of data using the internal dma bounce buffer
ایستا صحیح
nvme_bounce_xfer(ساختار nvme_namespace *ns, u64 lba, تهی *buf, u16 شمار,
                 صحیح اكتب)
{
    u16 ثابت max_blocks = NVME_PAGE_SIZE / ns->block_size;
    u16 blocks = شمار < max_blocks ? شمار : max_blocks;

    اگر (اكتب)
        memcpy(nvme_dma_buffer, buf, blocks * ns->block_size);

    صحیح res = nvme_io_xfer(ns, lba, nvme_dma_buffer, پوچ, blocks, اكتب);

    اگر (!اكتب && res >= 0)
        memcpy(buf, nvme_dma_buffer, res * ns->block_size);

    بازگشت res;
}

#define NVME_MAX_PRPL_ENTRIES 15 /* Allows requests up to 64kb */

// Transfer data using page list (if applicable)
ایستا صحیح
nvme_prpl_xfer(ساختار nvme_namespace *ns, u64 lba, تهی *buf, u16 شمار,
               صحیح اكتب)
{
    u32 base = (long)buf;
    s32 size;

    اگر (شمار > ns->max_req_size)
        شمار = ns->max_req_size;

    size = شمار * ns->block_size;
    /* Special case for transfers that fit into PRP1, but are unaligned */
    اگر (((size + (base & ~NVME_PAGE_MASK)) <= NVME_PAGE_SIZE))
        برو single;

    /* Every request has to be page aligned */
    اگر (base & ~NVME_PAGE_MASK)
        برو bounce;

    /* Make sure a full block fits into the last chunk */
    اگر (size & (ns->block_size - 1ULL))
        برو bounce;

    /* Build PRP list if we need to describe more than 2 pages */
    اگر ((ns->block_size * شمار) > (NVME_PAGE_SIZE * 2)) {
        u32 prpl_len = 0;
        u64 *prpl = nvme_dma_buffer;
        صحیح first_page = 1;
        برای (; size > 0; base += NVME_PAGE_SIZE, size -= NVME_PAGE_SIZE) {
            اگر (first_page) {
                /* First page is special */
                first_page = 0;
                ادامه;
            }
            اگر (prpl_len >= NVME_MAX_PRPL_ENTRIES)
                برو bounce;
            prpl[prpl_len++] = base;
        }
        بازگشت nvme_io_xfer(ns, lba, buf, prpl, شمار, اكتب);
    }

    /* Directly embed the 2nd page if we only need 2 pages */
    اگر ((ns->block_size * شمار) > NVME_PAGE_SIZE)
        بازگشت nvme_io_xfer(ns, lba, buf, buf + NVME_PAGE_SIZE, شمار, اكتب);

single:
    /* One page is enough, don't expose anything else */
    بازگشت nvme_io_xfer(ns, lba, buf, پوچ, شمار, اكتب);

bounce:
    /* Use bounce buffer to make transfer */
    بازگشت nvme_bounce_xfer(ns, lba, buf, شمار, اكتب);
}

ایستا صحیح
nvme_create_io_queues(ساختار nvme_ctrl *ctrl)
{
    اگر (nvme_create_io_cq(ctrl, &ctrl->io_cq, 3))
        برو err;

    اگر (nvme_create_io_sq(ctrl, &ctrl->io_sq, 2, &ctrl->io_cq))
        برو err_free_cq;

    بازگشت 0;

 err_free_cq:
    nvme_destroy_cq(&ctrl->io_cq);
 err:
    بازگشت -1;
}

/* Waits for CSTS.RDY to match rdy. Returns 0 on success. */
ایستا صحیح
nvme_wait_csts_rdy(ساختار nvme_ctrl *ctrl, unsigned rdy)
{
    u32 ثابت max_to = 500 /* ms */ * ((ctrl->reg->cap >> 24) & 0xFFU);
    u32 to = timer_calc(max_to);
    u32 csts;

    وقتی (rdy != ((csts = ctrl->reg->csts) & NVME_CSTS_RDY)) {
        yield();

        اگر (csts & NVME_CSTS_FATAL) {
            dprintf(3, "NVMe fatal error during controller shutdown\n");
            بازگشت -1;
        }

        اگر (timer_check(to)) {
            warn_timeout();
            بازگشت -1;
        }
    }

    بازگشت 0;
}

/* Returns 0 on success. */
ایستا صحیح
nvme_controller_enable(ساختار nvme_ctrl *ctrl)
{
    صحیح rc;

    pci_enable_busmaster(ctrl->pci);

    /* Turn the controller off. */
    ctrl->reg->cc = 0;
    اگر (nvme_wait_csts_rdy(ctrl, 0)) {
        dprintf(2, "NVMe fatal error during controller shutdown\n");
        بازگشت -1;
    }

    ctrl->doorbell_stride = 4U << ((ctrl->reg->cap >> 32) & 0xF);

    rc = nvme_init_cq(ctrl, &ctrl->admin_cq, 1,
                      NVME_PAGE_SIZE / اندازه(ساختار nvme_cqe));
    اگر (rc) {
        بازگشت -1;
    }

    rc = nvme_init_sq(ctrl, &ctrl->admin_sq, 0,
                      NVME_PAGE_SIZE / اندازه(ساختار nvme_sqe), &ctrl->admin_cq);
    اگر (rc) {
        برو err_destroy_admin_cq;
    }

    ctrl->reg->aqa = ctrl->admin_cq.common.mask << 16
        | ctrl->admin_sq.common.mask;

    ctrl->reg->asq = (u32)ctrl->admin_sq.sqe;
    ctrl->reg->acq = (u32)ctrl->admin_cq.cqe;

    dprintf(3, "  admin submission queue: %p\n", ctrl->admin_sq.sqe);
    dprintf(3, "  admin completion queue: %p\n", ctrl->admin_cq.cqe);

    ctrl->reg->cc = NVME_CC_EN | (NVME_CQE_SIZE_LOG << 20)
        | (NVME_SQE_SIZE_LOG << 16 /* IOSQES */);

    اگر (nvme_wait_csts_rdy(ctrl, 1)) {
        dprintf(2, "NVMe fatal error while enabling controller\n");
        برو err_destroy_admin_sq;
    }

    /* The admin queue is set up and the controller is ready. Let's figure out
       what namespaces we have. */

    ساختار nvme_identify_ctrl *identify = nvme_admin_identify_ctrl(ctrl);

    اگر (!identify) {
        dprintf(2, "NVMe couldn't identify controller.\n");
        برو err_destroy_admin_sq;
    }

    dprintf(3, "NVMe has %u namespace%s.\n",
            identify->nn, (identify->nn == 1) ? "" : "s");

    ctrl->ns_شمار = identify->nn;
    u8 mdts = identify->mdts;
    آزاد(identify);

    اگر ((ctrl->ns_شمار == 0) || nvme_create_io_queues(ctrl)) {
        /* No point to continue, if the controller says it doesn't have
           namespaces or we couldn't create I/O queues. */
        برو err_destroy_admin_sq;
    }

    /* Populate namespace IDs */
    صحیح ns_idx;
    برای (ns_idx = 0; ns_idx < ctrl->ns_شمار; ns_idx++) {
        nvme_probe_ns(ctrl, ns_idx, mdts);
    }

    dprintf(3, "NVMe initialization complete!\n");
    بازگشت 0;

 err_destroy_admin_sq:
    nvme_destroy_sq(&ctrl->admin_sq);
 err_destroy_admin_cq:
    nvme_destroy_cq(&ctrl->admin_cq);
    بازگشت -1;
}

/* Initialize an NVMe controller and detect its drives. */
ایستا تهی
nvme_controller_setup(تهی *opaque)
{
    u8 skip_nonbootable = is_bootprio_strict();
    ساختار pci_device *pci = opaque;

    اگر (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
        dprintf(1, "skipping init of a non-bootable NVMe at %pP\n",
                pci);
        برو err;
    }

    ساختار nvme_reg فرار *reg = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    اگر (!reg)
        بازگشت;

    u32 version = reg->vs;
    dprintf(3, "Found NVMe controller with version %u.%u.%u.\n",
            version >> 16, (version >> 8) & 0xFF, version & 0xFF);
    dprintf(3, "  Capabilities %016llx\n", reg->cap);

    اگر (~reg->cap & NVME_CAP_CSS_NVME) {
        dprintf(3, "Controller doesn't speak NVMe command set. Skipping.\n");
        برو err;
    }

    ساختار nvme_ctrl *ctrl = malloc_بالا(اندازه(*ctrl));
    اگر (!ctrl) {
        warn_noalloc();
        برو err;
    }

    memset(ctrl, 0, اندازه(*ctrl));

    ctrl->reg = reg;
    ctrl->pci = pci;

    اگر (nvme_controller_enable(ctrl)) {
        برو err_free_ctrl;
    }

    بازگشت;

 err_free_ctrl:
    آزاد(ctrl);
 err:
    dprintf(2, "Failed to enable NVMe controller.\n");
}

// Locate and init NVMe controllers
ایستا تهی
nvme_scan(تهی)
{
    // Scan PCI bus for NVMe adapters
    ساختار pci_device *pci;

    foreachpci(pci) {
        اگر (pci->class != PCI_CLASS_STORAGE_NVME)
            ادامه;
        اگر (pci->prog_if != 2 /* as of NVM 1.0e */) {
            dprintf(3, "Found incompatble NVMe: prog-if=%02x\n", pci->prog_if);
            ادامه;
        }

        run_thread(nvme_controller_setup, pci);
    }
}

ایستا صحیح
nvme_cmd_readwrite(ساختار nvme_namespace *ns, ساختار disk_op_s *op, صحیح اكتب)
{
    صحیح i;
    برای (i = 0; i < op->شمار;) {
        u16 blocks_remaining = op->شمار - i;
        نویسه *op_buf = op->buf_fl + i * ns->block_size;
        صحیح blocks = nvme_prpl_xfer(ns, op->lba + i, op_buf,
                                    blocks_remaining, اكتب);
        اگر (blocks < 0)
            بازگشت DISK_RET_EBADTRACK;
        i += blocks;
    }

    بازگشت DISK_RET_SUCCESS;
}

صحیح
nvme_process_op(ساختار disk_op_s *op)
{
    اگر (!CONFIG_NVME)
        بازگشت DISK_RET_SUCCESS;

    ساختار nvme_namespace *ns = container_of(op->drive_fl, ساختار nvme_namespace,
                                             drive);

    انتخاب (op->command) {
    حالت CMD_READ:
    حالت CMD_WRITE:
        بازگشت nvme_cmd_readwrite(ns, op, op->command == CMD_WRITE);
    پیشفرض:
        بازگشت default_process_op(op);
    }
}

تهی
nvme_setup(تهی)
{
    ASSERT32FLAT();
    اگر (!CONFIG_NVME)
        بازگشت;

    dprintf(3, "init nvme\n");
    nvme_scan();
}

/* EOF */

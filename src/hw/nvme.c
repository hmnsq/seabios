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
ساکن خالی *nvme_dma_بفر;

ساکن خالی *
zalloc_صفحہ_aligned(ساخت zone_s *zone, u32 سائز)
{
    خالی *res = _malloc(zone, سائز, NVME_صفحہ_سائز);
    اگر (res) memset(res, 0, سائز);
    واپس res;
}

ساکن خالی
nvme_init_قطار_common(ساخت nvme_ctrl *ctrl, ساخت nvme_قطار *q, u16 q_idx,
                       u16 لمبائی)
{
    memset(q, 0, حجم(*q));
    q->dbl = (u32 *)((حرف *)ctrl->reg + 0x1000 + q_idx * ctrl->doorbell_stride);
    dprintf(3, " q %p q_idx %u dbl %p\n", q, q_idx, q->dbl);
    q->نقاب = لمبائی - 1;
}

ساکن صحیح
nvme_init_sq(ساخت nvme_ctrl *ctrl, ساخت nvme_sq *sq, u16 q_idx, u16 لمبائی,
             ساخت nvme_cq *cq)
{
    nvme_init_قطار_common(ctrl, &sq->common, q_idx, لمبائی);
    sq->sqe = zalloc_صفحہ_aligned(&Zone_بلند, حجم(*sq->sqe) * لمبائی);

    اگر (!sq->sqe) {
        warn_noalloc();
        واپس -1;
    }

    dprintf(3, "sq %p q_idx %u sqe %p\n", sq, q_idx, sq->sqe);
    sq->cq   = cq;
    sq->سر = 0;
    sq->دم = 0;

    واپس 0;
}

ساکن صحیح
nvme_init_cq(ساخت nvme_ctrl *ctrl, ساخت nvme_cq *cq, u16 q_idx, u16 لمبائی)
{
    nvme_init_قطار_common(ctrl, &cq->common, q_idx, لمبائی);
    cq->cqe = zalloc_صفحہ_aligned(&Zone_بلند, حجم(*cq->cqe) * لمبائی);
    اگر (!cq->cqe) {
        warn_noalloc();
        واپس -1;
    }

    cq->سر = 0;

    /* All CQE phase bits are initialized to zero. This means initially we wait
       for the host controller to set these to 1. */
    cq->phase = 1;

    واپس 0;
}

ساکن صحیح
nvme_poll_cq(ساخت nvme_cq *cq)
{
    u32 dw3 = readl(&cq->cqe[cq->سر].dword[3]);
    واپس (!!(dw3 & NVME_CQE_DW3_P) == cq->phase);
}

ساکن صحیح
nvme_is_cqe_success(ساخت nvme_cqe ثابت *cqe)
{
    واپس ((cqe->حالت_2 >> 1) & 0xFF) == 0;
}

ساکن ساخت nvme_cqe
nvme_error_cqe(خالی)
{
    ساخت nvme_cqe r;

    /* 0xFF is a vendor specific status code != success. Should be okay for
       indicating failure. */
    memset(&r, 0xFF, حجم(r));
    واپس r;
}

ساکن ساخت nvme_cqe
nvme_consume_cqe(ساخت nvme_sq *sq)
{
    ساخت nvme_cq *cq = sq->cq;

    اگر (!nvme_poll_cq(cq)) {
        /* Cannot consume a completion queue entry, if there is none ready. */
        واپس nvme_error_cqe();
    }

    ساخت nvme_cqe *cqe = &cq->cqe[cq->سر];
    u16 cq_اگلا_سر = (cq->سر + 1) & cq->common.نقاب;
    dprintf(4, "cq %p head %u -> %u\n", cq, cq->سر, cq_اگلا_سر);
    اگر (cq_اگلا_سر < cq->سر) {
        dprintf(3, "cq %p wrap\n", cq);
        cq->phase = ~cq->phase;
    }
    cq->سر = cq_اگلا_سر;

    /* Update the submission queue head. */
    اگر (cqe->sq_سر != sq->سر) {
        sq->سر = cqe->sq_سر;
        dprintf(4, "sq %p advanced to %u\n", sq, cqe->sq_سر);
    }

    /* Tell the controller that we consumed the completion. */
    writel(cq->common.dbl, cq->سر);

    واپس *cqe;
}

ساکن ساخت nvme_cqe
nvme_wait(ساخت nvme_sq *sq)
{
    ساکن ثابت غیرعلامتی nvme_timeout = 5000 /* ms */;
    u32 مقصد = timer_calc(nvme_timeout);
    جب (!nvme_poll_cq(sq->cq)) {
        yield();

        اگر (timer_check(مقصد)) {
            warn_timeout();
            واپس nvme_error_cqe();
        }
    }

    واپس nvme_consume_cqe(sq);
}

/* Returns the next submission queue entry (or NULL if the queue is full). It
   also fills out Command Dword 0 and clears the rest. */
ساکن ساخت nvme_sqe *
nvme_get_اگلا_sqe(ساخت nvme_sq *sq, u8 opc, خالی *metadata, خالی *ڈیٹا, خالی *data2)
{
    اگر (((sq->سر + 1) & sq->common.نقاب) == sq->دم) {
        dprintf(3, "submission queue is full\n");
        واپس تہی;
    }

    ساخت nvme_sqe *sqe = &sq->sqe[sq->دم];
    dprintf(4, "sq %p next_sqe %u\n", sq, sq->دم);

    memset(sqe, 0, حجم(*sqe));
    sqe->cdw0 = opc | (sq->دم << 16 /* CID */);
    sqe->mptr = (u32)metadata;
    sqe->dptr_prp1 = (u32)ڈیٹا;
    sqe->dptr_prp2 = (u32)data2;

    واپس sqe;
}

/* Call this after you've filled out an sqe that you've got from nvme_get_next_sqe. */
ساکن خالی
nvme_commit_sqe(ساخت nvme_sq *sq)
{
    dprintf(4, "sq %p commit_sqe %u\n", sq, sq->دم);
    sq->دم = (sq->دم + 1) & sq->common.نقاب;
    writel(sq->common.dbl, sq->دم);
}

/* Perform an identify command on the admin queue and return the resulting
   buffer. This may be a NULL pointer, if something failed. This function
   cannot be used after initialization, because it uses buffers in tmp zone. */
ساکن اتحاد nvme_identify *
nvme_admin_identify(ساخت nvme_ctrl *ctrl, u8 cns, u32 nsid)
{
    اتحاد nvme_identify *identify_buf = zalloc_صفحہ_aligned(&Zone_Tmp_بلند, 4096);
    اگر (!identify_buf) {
        /* Could not allocate identify buffer. */
        warn_internalerror();
        واپس تہی;
    }

    ساخت nvme_sqe *cmd_identify;
    cmd_identify = nvme_get_اگلا_sqe(&ctrl->admin_sq,
                                     NVME_SQE_OPC_ADMIN_IDENTIFY, تہی,
                                     identify_buf, تہی);

    اگر (!cmd_identify) {
        warn_internalerror();
        جاؤ error;
    }

    cmd_identify->nsid = nsid;
    cmd_identify->dword[10] = cns;

    nvme_commit_sqe(&ctrl->admin_sq);

    ساخت nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    اگر (!nvme_is_cqe_success(&cqe)) {
        جاؤ error;
    }

    واپس identify_buf;
 error:
    آزاد(identify_buf);
    واپس تہی;
}

ساکن ساخت nvme_identify_ctrl *
nvme_admin_identify_ctrl(ساخت nvme_ctrl *ctrl)
{
    واپس &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_CTRL, 0)->ctrl;
}

ساکن ساخت nvme_identify_ns *
nvme_admin_identify_ns(ساخت nvme_ctrl *ctrl, u32 ns_id)
{
    واپس &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_NS,
                                ns_id)->ns;
}

ساکن خالی
nvme_probe_ns(ساخت nvme_ctrl *ctrl, u32 ns_idx, u8 mdts)
{
    u32 ns_id = ns_idx + 1;

    ساخت nvme_identify_ns *id = nvme_admin_identify_ns(ctrl, ns_id);
    اگر (!id) {
        dprintf(2, "NVMe couldn't identify namespace %u.\n", ns_id);
        جاؤ free_بفر;
    }

    u8 current_lba_format = id->flbas & 0xF;
    اگر (current_lba_format > id->nlbaf) {
        dprintf(2, "NVMe NS %u: current LBA format %u is beyond what the "
                " namespace supports (%u)?\n",
                ns_id, current_lba_format, id->nlbaf + 1);
        جاؤ free_بفر;
    }

    اگر (!id->nsze) {
        dprintf(2, "NVMe NS %u is inactive.\n", ns_id);
        جاؤ free_بفر;
    }

    اگر (!nvme_dma_بفر) {
        nvme_dma_بفر = zalloc_صفحہ_aligned(&Zone_بلند, NVME_صفحہ_سائز);
        اگر (!nvme_dma_بفر) {
            warn_noalloc();
            جاؤ free_بفر;
        }
    }

    ساخت nvme_namespace *ns = malloc_fseg(حجم(*ns));
    اگر (!ns) {
        warn_noalloc();
        جاؤ free_بفر;
    }
    memset(ns, 0, حجم(*ns));
    ns->ctrl  = ctrl;
    ns->ns_id = ns_id;
    ns->lba_گنتی = id->nsze;

    ساخت nvme_lba_format *fmt = &id->lbaf[current_lba_format];

    ns->بلاک_سائز    = 1U << fmt->lbads;
    ns->metadata_سائز = fmt->ms;

    اگر (ns->بلاک_سائز > NVME_صفحہ_سائز) {
        /* If we see devices that trigger this path, we need to increase our
           buffer size. */
        warn_internalerror();
        آزاد(ns);
        جاؤ free_بفر;
    }

    ns->drive.cntl_id   = ns_idx;
    ns->drive.removable = 0;
    ns->drive.type      = DTYPE_NVME;
    ns->drive.blksize   = ns->بلاک_سائز;
    ns->drive.sectors   = ns->lba_گنتی;

    اگر (mdts) {
        ns->max_req_سائز = ((1U << mdts) * NVME_صفحہ_سائز) / ns->بلاک_سائز;
        dprintf(3, "NVME NS %u max request size: %d sectors\n",
                ns_id, ns->max_req_سائز);
    } ورنہ {
        ns->max_req_سائز = -1U;
    }

    حرف *desc = znprintf(MAXDESCSIZE, "NVMe NS %u: %llu MiB (%llu %u-byte "
                          "blocks + %u-byte metadata)",
                          ns_id, (ns->lba_گنتی * ns->بلاک_سائز) >> 20,
                          ns->lba_گنتی, ns->بلاک_سائز, ns->metadata_سائز);

    dprintf(3, "%s\n", desc);
    boot_جوڑو_hd(&ns->drive, desc, bootprio_ڈھونڈو_pci_device(ctrl->pci));

free_بفر:
    آزاد (id);
}


/* Release memory allocated for a completion queue */
ساکن خالی
nvme_destroy_cq(ساخت nvme_cq *cq)
{
    آزاد(cq->cqe);
    cq->cqe = تہی;
}

/* Release memory allocated for a submission queue */
ساکن خالی
nvme_destroy_sq(ساخت nvme_sq *sq)
{
    آزاد(sq->sqe);
    sq->sqe = تہی;
}

/* Returns 0 on success. */
ساکن صحیح
nvme_create_io_cq(ساخت nvme_ctrl *ctrl, ساخت nvme_cq *cq, u16 q_idx)
{
    صحیح rc;
    ساخت nvme_sqe *cmd_create_cq;
    u32 لمبائی = 1 + (ctrl->reg->cap & 0xffff);
    اگر (لمبائی > NVME_صفحہ_سائز / حجم(ساخت nvme_cqe))
        لمبائی = NVME_صفحہ_سائز / حجم(ساخت nvme_cqe);

    rc = nvme_init_cq(ctrl, cq, q_idx, لمبائی);
    اگر (rc) {
        جاؤ err;
    }

    cmd_create_cq = nvme_get_اگلا_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_CQ, تہی,
                                      cq->cqe, تہی);
    اگر (!cmd_create_cq) {
        جاؤ err_destroy_cq;
    }

    cmd_create_cq->dword[10] = (cq->common.نقاب << 16) | (q_idx >> 1);
    cmd_create_cq->dword[11] = 1 /* physically contiguous */;

    nvme_commit_sqe(&ctrl->admin_sq);

    ساخت nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    اگر (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io cq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        جاؤ err_destroy_cq;
    }

    واپس 0;

err_destroy_cq:
    nvme_destroy_cq(cq);
err:
    واپس -1;
}

/* Returns 0 on success. */
ساکن صحیح
nvme_create_io_sq(ساخت nvme_ctrl *ctrl, ساخت nvme_sq *sq, u16 q_idx, ساخت nvme_cq *cq)
{
    صحیح rc;
    ساخت nvme_sqe *cmd_create_sq;
    u32 لمبائی = 1 + (ctrl->reg->cap & 0xffff);
    اگر (لمبائی > NVME_صفحہ_سائز / حجم(ساخت nvme_cqe))
        لمبائی = NVME_صفحہ_سائز / حجم(ساخت nvme_cqe);

    rc = nvme_init_sq(ctrl, sq, q_idx, لمبائی, cq);
    اگر (rc) {
        جاؤ err;
    }

    cmd_create_sq = nvme_get_اگلا_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_SQ, تہی,
                                      sq->sqe, تہی);
    اگر (!cmd_create_sq) {
        جاؤ err_destroy_sq;
    }

    cmd_create_sq->dword[10] = (sq->common.نقاب << 16) | (q_idx >> 1);
    cmd_create_sq->dword[11] = (q_idx >> 1) << 16 | 1 /* contiguous */;
    dprintf(3, "sq %p create dword10 %08x dword11 %08x\n", sq,
            cmd_create_sq->dword[10], cmd_create_sq->dword[11]);

    nvme_commit_sqe(&ctrl->admin_sq);

    ساخت nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    اگر (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io sq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);
        جاؤ err_destroy_sq;
    }

    واپس 0;

err_destroy_sq:
    nvme_destroy_sq(sq);
err:
    واپس -1;
}

/* Reads count sectors into buf. The buffer cannot cross page boundaries. */
ساکن صحیح
nvme_io_xfer(ساخت nvme_namespace *ns, u64 lba, خالی *prp1, خالی *prp2,
             u16 گنتی, صحیح اكتب)
{
    اگر (((u32)prp1 & 0x3) || ((u32)prp2 & 0x3)) {
        /* Buffer is misaligned */
        warn_internalerror();
        واپس -1;
    }

    ساخت nvme_sqe *io_read = nvme_get_اگلا_sqe(&ns->ctrl->io_sq,
                                                 اكتب ? NVME_SQE_OPC_IO_WRITE
                                                       : NVME_SQE_OPC_IO_READ,
                                                 تہی, prp1, prp2);
    io_read->nsid = ns->ns_id;
    io_read->dword[10] = (u32)lba;
    io_read->dword[11] = (u32)(lba >> 32);
    io_read->dword[12] = (1U << 31 /* limited retry */) | (گنتی - 1);

    nvme_commit_sqe(&ns->ctrl->io_sq);

    ساخت nvme_cqe cqe = nvme_wait(&ns->ctrl->io_sq);

    اگر (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "read io: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        واپس -1;
    }

    dprintf(5, "ns %u %s lba %llu+%u\n", ns->ns_id, اكتب ? "write" : "read",
            lba, گنتی);
    واپس گنتی;
}

// Transfer up to one page of data using the internal dma bounce buffer
ساکن صحیح
nvme_bounce_xfer(ساخت nvme_namespace *ns, u64 lba, خالی *buf, u16 گنتی,
                 صحیح اكتب)
{
    u16 ثابت max_بلاکیں = NVME_صفحہ_سائز / ns->بلاک_سائز;
    u16 بلاکیں = گنتی < max_بلاکیں ? گنتی : max_بلاکیں;

    اگر (اكتب)
        حافظہ_نقل(nvme_dma_بفر, buf, بلاکیں * ns->بلاک_سائز);

    صحیح res = nvme_io_xfer(ns, lba, nvme_dma_بفر, تہی, بلاکیں, اكتب);

    اگر (!اكتب && res >= 0)
        حافظہ_نقل(buf, nvme_dma_بفر, res * ns->بلاک_سائز);

    واپس res;
}

#تعریف_کرو NVME_MAX_PRPL_ENTRIES 15 /* Allows requests up to 64kb */

// Transfer data using page list (if applicable)
ساکن صحیح
nvme_prpl_xfer(ساخت nvme_namespace *ns, u64 lba, خالی *buf, u16 گنتی,
               صحیح اكتب)
{
    u32 بنیاد = (طویل)buf;
    s32 سائز;

    اگر (گنتی > ns->max_req_سائز)
        گنتی = ns->max_req_سائز;

    سائز = گنتی * ns->بلاک_سائز;
    /* Special case for transfers that fit into PRP1, but are unaligned */
    اگر (((سائز + (بنیاد & ~NVME_صفحہ_نقاب)) <= NVME_صفحہ_سائز))
        جاؤ single;

    /* Every request has to be page aligned */
    اگر (بنیاد & ~NVME_صفحہ_نقاب)
        جاؤ bounce;

    /* Make sure a full block fits into the last chunk */
    اگر (سائز & (ns->بلاک_سائز - 1ULL))
        جاؤ bounce;

    /* Build PRP list if we need to describe more than 2 pages */
    اگر ((ns->بلاک_سائز * گنتی) > (NVME_صفحہ_سائز * 2)) {
        u32 prpl_len = 0;
        u64 *prpl = nvme_dma_بفر;
        صحیح پہلا_صفحہ = 1;
        برائے (; سائز > 0; بنیاد += NVME_صفحہ_سائز, سائز -= NVME_صفحہ_سائز) {
            اگر (پہلا_صفحہ) {
                /* First page is special */
                پہلا_صفحہ = 0;
                جاری;
            }
            اگر (prpl_len >= NVME_MAX_PRPL_ENTRIES)
                جاؤ bounce;
            prpl[prpl_len++] = بنیاد;
        }
        واپس nvme_io_xfer(ns, lba, buf, prpl, گنتی, اكتب);
    }

    /* Directly embed the 2nd page if we only need 2 pages */
    اگر ((ns->بلاک_سائز * گنتی) > NVME_صفحہ_سائز)
        واپس nvme_io_xfer(ns, lba, buf, buf + NVME_صفحہ_سائز, گنتی, اكتب);

single:
    /* One page is enough, don't expose anything else */
    واپس nvme_io_xfer(ns, lba, buf, تہی, گنتی, اكتب);

bounce:
    /* Use bounce buffer to make transfer */
    واپس nvme_bounce_xfer(ns, lba, buf, گنتی, اكتب);
}

ساکن صحیح
nvme_create_io_queues(ساخت nvme_ctrl *ctrl)
{
    اگر (nvme_create_io_cq(ctrl, &ctrl->io_cq, 3))
        جاؤ err;

    اگر (nvme_create_io_sq(ctrl, &ctrl->io_sq, 2, &ctrl->io_cq))
        جاؤ err_free_cq;

    واپس 0;

 err_free_cq:
    nvme_destroy_cq(&ctrl->io_cq);
 err:
    واپس -1;
}

/* Waits for CSTS.RDY to match rdy. Returns 0 on success. */
ساکن صحیح
nvme_wait_csts_rdy(ساخت nvme_ctrl *ctrl, غیرعلامتی rdy)
{
    u32 ثابت max_مقصد = 500 /* ms */ * ((ctrl->reg->cap >> 24) & 0xFFU);
    u32 مقصد = timer_calc(max_مقصد);
    u32 csts;

    جب (rdy != ((csts = ctrl->reg->csts) & NVME_CSTS_RDY)) {
        yield();

        اگر (csts & NVME_CSTS_FATAL) {
            dprintf(3, "NVMe fatal error during controller shutdown\n");
            واپس -1;
        }

        اگر (timer_check(مقصد)) {
            warn_timeout();
            واپس -1;
        }
    }

    واپس 0;
}

/* Returns 0 on success. */
ساکن صحیح
nvme_controller_enable(ساخت nvme_ctrl *ctrl)
{
    صحیح rc;

    pci_enable_busmaster(ctrl->pci);

    /* Turn the controller off. */
    ctrl->reg->cc = 0;
    اگر (nvme_wait_csts_rdy(ctrl, 0)) {
        dprintf(2, "NVMe fatal error during controller shutdown\n");
        واپس -1;
    }

    ctrl->doorbell_stride = 4U << ((ctrl->reg->cap >> 32) & 0xF);

    rc = nvme_init_cq(ctrl, &ctrl->admin_cq, 1,
                      NVME_صفحہ_سائز / حجم(ساخت nvme_cqe));
    اگر (rc) {
        واپس -1;
    }

    rc = nvme_init_sq(ctrl, &ctrl->admin_sq, 0,
                      NVME_صفحہ_سائز / حجم(ساخت nvme_sqe), &ctrl->admin_cq);
    اگر (rc) {
        جاؤ err_destroy_admin_cq;
    }

    ctrl->reg->aqa = ctrl->admin_cq.common.نقاب << 16
        | ctrl->admin_sq.common.نقاب;

    ctrl->reg->asq = (u32)ctrl->admin_sq.sqe;
    ctrl->reg->acq = (u32)ctrl->admin_cq.cqe;

    dprintf(3, "  admin submission queue: %p\n", ctrl->admin_sq.sqe);
    dprintf(3, "  admin completion queue: %p\n", ctrl->admin_cq.cqe);

    ctrl->reg->cc = NVME_CC_EN | (NVME_CQE_سائز_LOG << 20)
        | (NVME_SQE_سائز_LOG << 16 /* IOSQES */);

    اگر (nvme_wait_csts_rdy(ctrl, 1)) {
        dprintf(2, "NVMe fatal error while enabling controller\n");
        جاؤ err_destroy_admin_sq;
    }

    /* The admin queue is set up and the controller is ready. Let's figure out
       what namespaces we have. */

    ساخت nvme_identify_ctrl *identify = nvme_admin_identify_ctrl(ctrl);

    اگر (!identify) {
        dprintf(2, "NVMe couldn't identify controller.\n");
        جاؤ err_destroy_admin_sq;
    }

    dprintf(3, "NVMe has %u namespace%s.\n",
            identify->nn, (identify->nn == 1) ? "" : "s");

    ctrl->ns_گنتی = identify->nn;
    u8 mdts = identify->mdts;
    آزاد(identify);

    اگر ((ctrl->ns_گنتی == 0) || nvme_create_io_queues(ctrl)) {
        /* No point to continue, if the controller says it doesn't have
           namespaces or we couldn't create I/O queues. */
        جاؤ err_destroy_admin_sq;
    }

    /* Populate namespace IDs */
    صحیح ns_idx;
    برائے (ns_idx = 0; ns_idx < ctrl->ns_گنتی; ns_idx++) {
        nvme_probe_ns(ctrl, ns_idx, mdts);
    }

    dprintf(3, "NVMe initialization complete!\n");
    واپس 0;

 err_destroy_admin_sq:
    nvme_destroy_sq(&ctrl->admin_sq);
 err_destroy_admin_cq:
    nvme_destroy_cq(&ctrl->admin_cq);
    واپس -1;
}

/* Initialize an NVMe controller and detect its drives. */
ساکن خالی
nvme_controller_تیاری(خالی *opaque)
{
    u8 skip_nonbootable = is_bootprio_strict();
    ساخت pci_device *pci = opaque;

    اگر (skip_nonbootable && bootprio_ڈھونڈو_pci_device(pci) < 0) {
        dprintf(1, "skipping init of a non-bootable NVMe at %pP\n",
                pci);
        جاؤ err;
    }

    ساخت nvme_reg متلون *reg = pci_enable_membar(pci, PCI_بنیاد_پتہ_0);
    اگر (!reg)
        واپس;

    u32 نسخہ_2 = reg->vs;
    dprintf(3, "Found NVMe controller with version %u.%u.%u.\n",
            نسخہ_2 >> 16, (نسخہ_2 >> 8) & 0xFF, نسخہ_2 & 0xFF);
    dprintf(3, "  Capabilities %016llx\n", reg->cap);

    اگر (~reg->cap & NVME_CAP_CSS_NVME) {
        dprintf(3, "Controller doesn't speak NVMe command set. Skipping.\n");
        جاؤ err;
    }

    ساخت nvme_ctrl *ctrl = malloc_بلند(حجم(*ctrl));
    اگر (!ctrl) {
        warn_noalloc();
        جاؤ err;
    }

    memset(ctrl, 0, حجم(*ctrl));

    ctrl->reg = reg;
    ctrl->pci = pci;

    اگر (nvme_controller_enable(ctrl)) {
        جاؤ err_free_ctrl;
    }

    واپس;

 err_free_ctrl:
    آزاد(ctrl);
 err:
    dprintf(2, "Failed to enable NVMe controller.\n");
}

// Locate and init NVMe controllers
ساکن خالی
nvme_جائزہ(خالی)
{
    // Scan PCI bus for NVMe adapters
    ساخت pci_device *pci;

    foreachpci(pci) {
        اگر (pci->class != PCI_CLASS_STORAGE_NVME)
            جاری;
        اگر (pci->prog_if != 2 /* as of NVM 1.0e */) {
            dprintf(3, "Found incompatble NVMe: prog-if=%02x\n", pci->prog_if);
            جاری;
        }

        تسلسل_thread(nvme_controller_تیاری, pci);
    }
}

ساکن صحیح
nvme_cmd_readwrite(ساخت nvme_namespace *ns, ساخت disk_op_s *op, صحیح اكتب)
{
    صحیح i;
    برائے (i = 0; i < op->گنتی;) {
        u16 بلاکیں_بقایا = op->گنتی - i;
        حرف *op_buf = op->buf_fl + i * ns->بلاک_سائز;
        صحیح بلاکیں = nvme_prpl_xfer(ns, op->lba + i, op_buf,
                                    بلاکیں_بقایا, اكتب);
        اگر (بلاکیں < 0)
            واپس DISK_RET_EBADTRACK;
        i += بلاکیں;
    }

    واپس DISK_RET_SUCCESS;
}

صحیح
nvme_process_op(ساخت disk_op_s *op)
{
    اگر (!CONFIG_NVME)
        واپس DISK_RET_SUCCESS;

    ساخت nvme_namespace *ns = container_کا(op->drive_fl, ساخت nvme_namespace,
                                             drive);

    انتخاب (op->command) {
    حالت CMD_READ:
    حالت CMD_WRITE:
        واپس nvme_cmd_readwrite(ns, op, op->command == CMD_WRITE);
    طےشدہ:
        واپس default_process_op(op);
    }
}

خالی
nvme_تیاری(خالی)
{
    ASSERT32FLAT();
    اگر (!CONFIG_NVME)
        واپس;

    dprintf(3, "init nvme\n");
    nvme_جائزہ();
}

/* EOF */

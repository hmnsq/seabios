// Low level NVMe disk access
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "blockcmd.h"
#ضمن "malloc.h" // malloc_high
#ضمن "output.h" // dprintf
#ضمن "pci.h"
#ضمن "pci_ids.h" // PCI_CLASS_STORAGE_NVME
#ضمن "pci_regs.h" // PCI_BASE_ADDRESS_0
#ضمن "pcidevice.h" // foreachpci
#ضمن "stacks.h" // yield
#ضمن "std/disk.h" // DISK_RET_
#ضمن "string.h" // memset
#ضمن "util.h" // boot_add_hd
#ضمن "x86.h" // readl

#ضمن "nvme.h"
#ضمن "nvme-int.h"

// Page aligned "dma bounce buffer" of size NVME_PAGE_SIZE in high memory
ساكن فراغ *nvme_dma_میانگیر;

ساكن فراغ *
zalloc_صفحه_aligned(بنية zone_s *zone, u32 اندازه)
{
    فراغ *res = _malloc(zone, اندازه, NVME_صفحه_اندازه);
    إذا (res) memset(res, 0, اندازه);
    أرجع res;
}

ساكن فراغ
nvme_init_صف_common(بنية nvme_ctrl *ctrl, بنية nvme_صف *q, u16 q_idx,
                       u16 درازی)
{
    memset(q, 0, حجم(*q));
    q->dbl = (u32 *)((حرف *)ctrl->reg + 0x1000 + q_idx * ctrl->doorbell_stride);
    dprintf(3, " q %p q_idx %u dbl %p\n", q, q_idx, q->dbl);
    q->ماسک = درازی - 1;
}

ساكن صحيح
nvme_init_sq(بنية nvme_ctrl *ctrl, بنية nvme_sq *sq, u16 q_idx, u16 درازی,
             بنية nvme_cq *cq)
{
    nvme_init_صف_common(ctrl, &sq->common, q_idx, درازی);
    sq->sqe = zalloc_صفحه_aligned(&Zone_بالا, حجم(*sq->sqe) * درازی);

    إذا (!sq->sqe) {
        warn_noalloc();
        أرجع -1;
    }

    dprintf(3, "sq %p q_idx %u sqe %p\n", sq, q_idx, sq->sqe);
    sq->cq   = cq;
    sq->سر = 0;
    sq->دم = 0;

    أرجع 0;
}

ساكن صحيح
nvme_init_cq(بنية nvme_ctrl *ctrl, بنية nvme_cq *cq, u16 q_idx, u16 درازی)
{
    nvme_init_صف_common(ctrl, &cq->common, q_idx, درازی);
    cq->cqe = zalloc_صفحه_aligned(&Zone_بالا, حجم(*cq->cqe) * درازی);
    إذا (!cq->cqe) {
        warn_noalloc();
        أرجع -1;
    }

    cq->سر = 0;

    /* All CQE phase bits are initialized to zero. This means initially we wait
       for the host controller to set these to 1. */
    cq->phase = 1;

    أرجع 0;
}

ساكن صحيح
nvme_poll_cq(بنية nvme_cq *cq)
{
    u32 dw3 = readl(&cq->cqe[cq->سر].dword[3]);
    أرجع (!!(dw3 & NVME_CQE_DW3_P) == cq->phase);
}

ساكن صحيح
nvme_is_cqe_success(بنية nvme_cqe ثابت *cqe)
{
    أرجع ((cqe->وضعیت >> 1) & 0xFF) == 0;
}

ساكن بنية nvme_cqe
nvme_error_cqe(فراغ)
{
    بنية nvme_cqe r;

    /* 0xFF is a vendor specific status code != success. Should be okay for
       indicating failure. */
    memset(&r, 0xFF, حجم(r));
    أرجع r;
}

ساكن بنية nvme_cqe
nvme_consume_cqe(بنية nvme_sq *sq)
{
    بنية nvme_cq *cq = sq->cq;

    إذا (!nvme_poll_cq(cq)) {
        /* Cannot consume a completion queue entry, if there is none ready. */
        أرجع nvme_error_cqe();
    }

    بنية nvme_cqe *cqe = &cq->cqe[cq->سر];
    u16 cq_بعدی_سر = (cq->سر + 1) & cq->common.ماسک;
    dprintf(4, "cq %p head %u -> %u\n", cq, cq->سر, cq_بعدی_سر);
    إذا (cq_بعدی_سر < cq->سر) {
        dprintf(3, "cq %p wrap\n", cq);
        cq->phase = ~cq->phase;
    }
    cq->سر = cq_بعدی_سر;

    /* Update the submission queue head. */
    إذا (cqe->sq_سر != sq->سر) {
        sq->سر = cqe->sq_سر;
        dprintf(4, "sq %p advanced to %u\n", sq, cqe->sq_سر);
    }

    /* Tell the controller that we consumed the completion. */
    writel(cq->common.dbl, cq->سر);

    أرجع *cqe;
}

ساكن بنية nvme_cqe
nvme_wait(بنية nvme_sq *sq)
{
    ساكن ثابت بیعلامت nvme_timeout = 5000 /* ms */;
    u32 مقصد = timer_calc(nvme_timeout);
    طالما (!nvme_poll_cq(sq->cq)) {
        yield();

        إذا (timer_check(مقصد)) {
            warn_timeout();
            أرجع nvme_error_cqe();
        }
    }

    أرجع nvme_consume_cqe(sq);
}

/* Returns the next submission queue entry (or NULL if the queue is full). It
   also fills out Command Dword 0 and clears the rest. */
ساكن بنية nvme_sqe *
nvme_get_بعدی_sqe(بنية nvme_sq *sq, u8 opc, فراغ *metadata, فراغ *داده, فراغ *data2)
{
    إذا (((sq->سر + 1) & sq->common.ماسک) == sq->دم) {
        dprintf(3, "submission queue is full\n");
        أرجع عدم;
    }

    بنية nvme_sqe *sqe = &sq->sqe[sq->دم];
    dprintf(4, "sq %p next_sqe %u\n", sq, sq->دم);

    memset(sqe, 0, حجم(*sqe));
    sqe->cdw0 = opc | (sq->دم << 16 /* CID */);
    sqe->mptr = (u32)metadata;
    sqe->dptr_prp1 = (u32)داده;
    sqe->dptr_prp2 = (u32)data2;

    أرجع sqe;
}

/* Call this after you've filled out an sqe that you've got from nvme_get_next_sqe. */
ساكن فراغ
nvme_commit_sqe(بنية nvme_sq *sq)
{
    dprintf(4, "sq %p commit_sqe %u\n", sq, sq->دم);
    sq->دم = (sq->دم + 1) & sq->common.ماسک;
    writel(sq->common.dbl, sq->دم);
}

/* Perform an identify command on the admin queue and return the resulting
   buffer. This may be a NULL pointer, if something failed. This function
   cannot be used after initialization, because it uses buffers in tmp zone. */
ساكن اتحاد nvme_identify *
nvme_admin_identify(بنية nvme_ctrl *ctrl, u8 cns, u32 nsid)
{
    اتحاد nvme_identify *identify_buf = zalloc_صفحه_aligned(&Zone_Tmp_بالا, 4096);
    إذا (!identify_buf) {
        /* Could not allocate identify buffer. */
        warn_internalerror();
        أرجع عدم;
    }

    بنية nvme_sqe *cmd_identify;
    cmd_identify = nvme_get_بعدی_sqe(&ctrl->admin_sq,
                                     NVME_SQE_OPC_ADMIN_IDENTIFY, عدم,
                                     identify_buf, عدم);

    إذا (!cmd_identify) {
        warn_internalerror();
        اذهب error;
    }

    cmd_identify->nsid = nsid;
    cmd_identify->dword[10] = cns;

    nvme_commit_sqe(&ctrl->admin_sq);

    بنية nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    إذا (!nvme_is_cqe_success(&cqe)) {
        اذهب error;
    }

    أرجع identify_buf;
 error:
    حرر(identify_buf);
    أرجع عدم;
}

ساكن بنية nvme_identify_ctrl *
nvme_admin_identify_ctrl(بنية nvme_ctrl *ctrl)
{
    أرجع &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_CTRL, 0)->ctrl;
}

ساكن بنية nvme_identify_ns *
nvme_admin_identify_ns(بنية nvme_ctrl *ctrl, u32 ns_id)
{
    أرجع &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_NS,
                                ns_id)->ns;
}

ساكن فراغ
nvme_probe_ns(بنية nvme_ctrl *ctrl, u32 ns_idx, u8 mdts)
{
    u32 ns_id = ns_idx + 1;

    بنية nvme_identify_ns *id = nvme_admin_identify_ns(ctrl, ns_id);
    إذا (!id) {
        dprintf(2, "NVMe couldn't identify namespace %u.\n", ns_id);
        اذهب free_میانگیر;
    }

    u8 current_lba_format = id->flbas & 0xF;
    إذا (current_lba_format > id->nlbaf) {
        dprintf(2, "NVMe NS %u: current LBA format %u is beyond what the "
                " namespace supports (%u)?\n",
                ns_id, current_lba_format, id->nlbaf + 1);
        اذهب free_میانگیر;
    }

    إذا (!id->nsze) {
        dprintf(2, "NVMe NS %u is inactive.\n", ns_id);
        اذهب free_میانگیر;
    }

    إذا (!nvme_dma_میانگیر) {
        nvme_dma_میانگیر = zalloc_صفحه_aligned(&Zone_بالا, NVME_صفحه_اندازه);
        إذا (!nvme_dma_میانگیر) {
            warn_noalloc();
            اذهب free_میانگیر;
        }
    }

    بنية nvme_namespace *ns = malloc_fseg(حجم(*ns));
    إذا (!ns) {
        warn_noalloc();
        اذهب free_میانگیر;
    }
    memset(ns, 0, حجم(*ns));
    ns->ctrl  = ctrl;
    ns->ns_id = ns_id;
    ns->lba_شمار = id->nsze;

    بنية nvme_lba_format *fmt = &id->lbaf[current_lba_format];

    ns->بلوک_اندازه    = 1U << fmt->lbads;
    ns->metadata_اندازه = fmt->ms;

    إذا (ns->بلوک_اندازه > NVME_صفحه_اندازه) {
        /* If we see devices that trigger this path, we need to increase our
           buffer size. */
        warn_internalerror();
        حرر(ns);
        اذهب free_میانگیر;
    }

    ns->drive.cntl_id   = ns_idx;
    ns->drive.removable = 0;
    ns->drive.type      = DTYPE_NVME;
    ns->drive.blksize   = ns->بلوک_اندازه;
    ns->drive.sectors   = ns->lba_شمار;

    إذا (mdts) {
        ns->max_req_اندازه = ((1U << mdts) * NVME_صفحه_اندازه) / ns->بلوک_اندازه;
        dprintf(3, "NVME NS %u max request size: %d sectors\n",
                ns_id, ns->max_req_اندازه);
    } وإلا {
        ns->max_req_اندازه = -1U;
    }

    حرف *desc = znprintf(MAXDESCSIZE, "NVMe NS %u: %llu MiB (%llu %u-byte "
                          "blocks + %u-byte metadata)",
                          ns_id, (ns->lba_شمار * ns->بلوک_اندازه) >> 20,
                          ns->lba_شمار, ns->بلوک_اندازه, ns->metadata_اندازه);

    dprintf(3, "%s\n", desc);
    boot_جمع_کن_hd(&ns->drive, desc, bootprio_پیدا_کن_pci_device(ctrl->pci));

free_میانگیر:
    حرر (id);
}


/* Release memory allocated for a completion queue */
ساكن فراغ
nvme_destroy_cq(بنية nvme_cq *cq)
{
    حرر(cq->cqe);
    cq->cqe = عدم;
}

/* Release memory allocated for a submission queue */
ساكن فراغ
nvme_destroy_sq(بنية nvme_sq *sq)
{
    حرر(sq->sqe);
    sq->sqe = عدم;
}

/* Returns 0 on success. */
ساكن صحيح
nvme_create_io_cq(بنية nvme_ctrl *ctrl, بنية nvme_cq *cq, u16 q_idx)
{
    صحيح rc;
    بنية nvme_sqe *cmd_create_cq;
    u32 درازی = 1 + (ctrl->reg->cap & 0xffff);
    إذا (درازی > NVME_صفحه_اندازه / حجم(بنية nvme_cqe))
        درازی = NVME_صفحه_اندازه / حجم(بنية nvme_cqe);

    rc = nvme_init_cq(ctrl, cq, q_idx, درازی);
    إذا (rc) {
        اذهب err;
    }

    cmd_create_cq = nvme_get_بعدی_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_CQ, عدم,
                                      cq->cqe, عدم);
    إذا (!cmd_create_cq) {
        اذهب err_destroy_cq;
    }

    cmd_create_cq->dword[10] = (cq->common.ماسک << 16) | (q_idx >> 1);
    cmd_create_cq->dword[11] = 1 /* physically contiguous */;

    nvme_commit_sqe(&ctrl->admin_sq);

    بنية nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    إذا (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io cq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        اذهب err_destroy_cq;
    }

    أرجع 0;

err_destroy_cq:
    nvme_destroy_cq(cq);
err:
    أرجع -1;
}

/* Returns 0 on success. */
ساكن صحيح
nvme_create_io_sq(بنية nvme_ctrl *ctrl, بنية nvme_sq *sq, u16 q_idx, بنية nvme_cq *cq)
{
    صحيح rc;
    بنية nvme_sqe *cmd_create_sq;
    u32 درازی = 1 + (ctrl->reg->cap & 0xffff);
    إذا (درازی > NVME_صفحه_اندازه / حجم(بنية nvme_cqe))
        درازی = NVME_صفحه_اندازه / حجم(بنية nvme_cqe);

    rc = nvme_init_sq(ctrl, sq, q_idx, درازی, cq);
    إذا (rc) {
        اذهب err;
    }

    cmd_create_sq = nvme_get_بعدی_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_SQ, عدم,
                                      sq->sqe, عدم);
    إذا (!cmd_create_sq) {
        اذهب err_destroy_sq;
    }

    cmd_create_sq->dword[10] = (sq->common.ماسک << 16) | (q_idx >> 1);
    cmd_create_sq->dword[11] = (q_idx >> 1) << 16 | 1 /* contiguous */;
    dprintf(3, "sq %p create dword10 %08x dword11 %08x\n", sq,
            cmd_create_sq->dword[10], cmd_create_sq->dword[11]);

    nvme_commit_sqe(&ctrl->admin_sq);

    بنية nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    إذا (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io sq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);
        اذهب err_destroy_sq;
    }

    أرجع 0;

err_destroy_sq:
    nvme_destroy_sq(sq);
err:
    أرجع -1;
}

/* Reads count sectors into buf. The buffer cannot cross page boundaries. */
ساكن صحيح
nvme_io_xfer(بنية nvme_namespace *ns, u64 lba, فراغ *prp1, فراغ *prp2,
             u16 شمار, صحيح اكتب)
{
    إذا (((u32)prp1 & 0x3) || ((u32)prp2 & 0x3)) {
        /* Buffer is misaligned */
        warn_internalerror();
        أرجع -1;
    }

    بنية nvme_sqe *io_read = nvme_get_بعدی_sqe(&ns->ctrl->io_sq,
                                                 اكتب ? NVME_SQE_OPC_IO_WRITE
                                                       : NVME_SQE_OPC_IO_READ,
                                                 عدم, prp1, prp2);
    io_read->nsid = ns->ns_id;
    io_read->dword[10] = (u32)lba;
    io_read->dword[11] = (u32)(lba >> 32);
    io_read->dword[12] = (1U << 31 /* limited retry */) | (شمار - 1);

    nvme_commit_sqe(&ns->ctrl->io_sq);

    بنية nvme_cqe cqe = nvme_wait(&ns->ctrl->io_sq);

    إذا (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "read io: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        أرجع -1;
    }

    dprintf(5, "ns %u %s lba %llu+%u\n", ns->ns_id, اكتب ? "write" : "read",
            lba, شمار);
    أرجع شمار;
}

// Transfer up to one page of data using the internal dma bounce buffer
ساكن صحيح
nvme_bounce_xfer(بنية nvme_namespace *ns, u64 lba, فراغ *buf, u16 شمار,
                 صحيح اكتب)
{
    u16 ثابت max_بلوکها = NVME_صفحه_اندازه / ns->بلوک_اندازه;
    u16 بلوکها = شمار < max_بلوکها ? شمار : max_بلوکها;

    إذا (اكتب)
        کپی_حافظه(nvme_dma_میانگیر, buf, بلوکها * ns->بلوک_اندازه);

    صحيح res = nvme_io_xfer(ns, lba, nvme_dma_میانگیر, عدم, بلوکها, اكتب);

    إذا (!اكتب && res >= 0)
        کپی_حافظه(buf, nvme_dma_میانگیر, res * ns->بلوک_اندازه);

    أرجع res;
}

#تعریف_کن NVME_MAX_PRPL_ENTRIES 15 /* Allows requests up to 64kb */

// Transfer data using page list (if applicable)
ساكن صحيح
nvme_prpl_xfer(بنية nvme_namespace *ns, u64 lba, فراغ *buf, u16 شمار,
               صحيح اكتب)
{
    u32 پایه = (دراز)buf;
    s32 اندازه;

    إذا (شمار > ns->max_req_اندازه)
        شمار = ns->max_req_اندازه;

    اندازه = شمار * ns->بلوک_اندازه;
    /* Special case for transfers that fit into PRP1, but are unaligned */
    إذا (((اندازه + (پایه & ~NVME_صفحه_ماسک)) <= NVME_صفحه_اندازه))
        اذهب single;

    /* Every request has to be page aligned */
    إذا (پایه & ~NVME_صفحه_ماسک)
        اذهب bounce;

    /* Make sure a full block fits into the last chunk */
    إذا (اندازه & (ns->بلوک_اندازه - 1ULL))
        اذهب bounce;

    /* Build PRP list if we need to describe more than 2 pages */
    إذا ((ns->بلوک_اندازه * شمار) > (NVME_صفحه_اندازه * 2)) {
        u32 prpl_len = 0;
        u64 *prpl = nvme_dma_میانگیر;
        صحيح اولی_صفحه = 1;
        لكل (; اندازه > 0; پایه += NVME_صفحه_اندازه, اندازه -= NVME_صفحه_اندازه) {
            إذا (اولی_صفحه) {
                /* First page is special */
                اولی_صفحه = 0;
                استمر;
            }
            إذا (prpl_len >= NVME_MAX_PRPL_ENTRIES)
                اذهب bounce;
            prpl[prpl_len++] = پایه;
        }
        أرجع nvme_io_xfer(ns, lba, buf, prpl, شمار, اكتب);
    }

    /* Directly embed the 2nd page if we only need 2 pages */
    إذا ((ns->بلوک_اندازه * شمار) > NVME_صفحه_اندازه)
        أرجع nvme_io_xfer(ns, lba, buf, buf + NVME_صفحه_اندازه, شمار, اكتب);

single:
    /* One page is enough, don't expose anything else */
    أرجع nvme_io_xfer(ns, lba, buf, عدم, شمار, اكتب);

bounce:
    /* Use bounce buffer to make transfer */
    أرجع nvme_bounce_xfer(ns, lba, buf, شمار, اكتب);
}

ساكن صحيح
nvme_create_io_queues(بنية nvme_ctrl *ctrl)
{
    إذا (nvme_create_io_cq(ctrl, &ctrl->io_cq, 3))
        اذهب err;

    إذا (nvme_create_io_sq(ctrl, &ctrl->io_sq, 2, &ctrl->io_cq))
        اذهب err_free_cq;

    أرجع 0;

 err_free_cq:
    nvme_destroy_cq(&ctrl->io_cq);
 err:
    أرجع -1;
}

/* Waits for CSTS.RDY to match rdy. Returns 0 on success. */
ساكن صحيح
nvme_wait_csts_rdy(بنية nvme_ctrl *ctrl, بیعلامت rdy)
{
    u32 ثابت max_مقصد = 500 /* ms */ * ((ctrl->reg->cap >> 24) & 0xFFU);
    u32 مقصد = timer_calc(max_مقصد);
    u32 csts;

    طالما (rdy != ((csts = ctrl->reg->csts) & NVME_CSTS_RDY)) {
        yield();

        إذا (csts & NVME_CSTS_FATAL) {
            dprintf(3, "NVMe fatal error during controller shutdown\n");
            أرجع -1;
        }

        إذا (timer_check(مقصد)) {
            warn_timeout();
            أرجع -1;
        }
    }

    أرجع 0;
}

/* Returns 0 on success. */
ساكن صحيح
nvme_controller_enable(بنية nvme_ctrl *ctrl)
{
    صحيح rc;

    pci_enable_busmaster(ctrl->pci);

    /* Turn the controller off. */
    ctrl->reg->cc = 0;
    إذا (nvme_wait_csts_rdy(ctrl, 0)) {
        dprintf(2, "NVMe fatal error during controller shutdown\n");
        أرجع -1;
    }

    ctrl->doorbell_stride = 4U << ((ctrl->reg->cap >> 32) & 0xF);

    rc = nvme_init_cq(ctrl, &ctrl->admin_cq, 1,
                      NVME_صفحه_اندازه / حجم(بنية nvme_cqe));
    إذا (rc) {
        أرجع -1;
    }

    rc = nvme_init_sq(ctrl, &ctrl->admin_sq, 0,
                      NVME_صفحه_اندازه / حجم(بنية nvme_sqe), &ctrl->admin_cq);
    إذا (rc) {
        اذهب err_destroy_admin_cq;
    }

    ctrl->reg->aqa = ctrl->admin_cq.common.ماسک << 16
        | ctrl->admin_sq.common.ماسک;

    ctrl->reg->asq = (u32)ctrl->admin_sq.sqe;
    ctrl->reg->acq = (u32)ctrl->admin_cq.cqe;

    dprintf(3, "  admin submission queue: %p\n", ctrl->admin_sq.sqe);
    dprintf(3, "  admin completion queue: %p\n", ctrl->admin_cq.cqe);

    ctrl->reg->cc = NVME_CC_EN | (NVME_CQE_اندازه_LOG << 20)
        | (NVME_SQE_اندازه_LOG << 16 /* IOSQES */);

    إذا (nvme_wait_csts_rdy(ctrl, 1)) {
        dprintf(2, "NVMe fatal error while enabling controller\n");
        اذهب err_destroy_admin_sq;
    }

    /* The admin queue is set up and the controller is ready. Let's figure out
       what namespaces we have. */

    بنية nvme_identify_ctrl *identify = nvme_admin_identify_ctrl(ctrl);

    إذا (!identify) {
        dprintf(2, "NVMe couldn't identify controller.\n");
        اذهب err_destroy_admin_sq;
    }

    dprintf(3, "NVMe has %u namespace%s.\n",
            identify->nn, (identify->nn == 1) ? "" : "s");

    ctrl->ns_شمار = identify->nn;
    u8 mdts = identify->mdts;
    حرر(identify);

    إذا ((ctrl->ns_شمار == 0) || nvme_create_io_queues(ctrl)) {
        /* No point to continue, if the controller says it doesn't have
           namespaces or we couldn't create I/O queues. */
        اذهب err_destroy_admin_sq;
    }

    /* Populate namespace IDs */
    صحيح ns_idx;
    لكل (ns_idx = 0; ns_idx < ctrl->ns_شمار; ns_idx++) {
        nvme_probe_ns(ctrl, ns_idx, mdts);
    }

    dprintf(3, "NVMe initialization complete!\n");
    أرجع 0;

 err_destroy_admin_sq:
    nvme_destroy_sq(&ctrl->admin_sq);
 err_destroy_admin_cq:
    nvme_destroy_cq(&ctrl->admin_cq);
    أرجع -1;
}

/* Initialize an NVMe controller and detect its drives. */
ساكن فراغ
nvme_controller_راهاندازی(فراغ *opaque)
{
    u8 skip_nonbootable = is_bootprio_strict();
    بنية pci_device *pci = opaque;

    إذا (skip_nonbootable && bootprio_پیدا_کن_pci_device(pci) < 0) {
        dprintf(1, "skipping init of a non-bootable NVMe at %pP\n",
                pci);
        اذهب err;
    }

    بنية nvme_reg متطاير *reg = pci_enable_membar(pci, PCI_پایه_آدرس_0);
    إذا (!reg)
        أرجع;

    u32 نسخه_2 = reg->vs;
    dprintf(3, "Found NVMe controller with version %u.%u.%u.\n",
            نسخه_2 >> 16, (نسخه_2 >> 8) & 0xFF, نسخه_2 & 0xFF);
    dprintf(3, "  Capabilities %016llx\n", reg->cap);

    إذا (~reg->cap & NVME_CAP_CSS_NVME) {
        dprintf(3, "Controller doesn't speak NVMe command set. Skipping.\n");
        اذهب err;
    }

    بنية nvme_ctrl *ctrl = malloc_بالا(حجم(*ctrl));
    إذا (!ctrl) {
        warn_noalloc();
        اذهب err;
    }

    memset(ctrl, 0, حجم(*ctrl));

    ctrl->reg = reg;
    ctrl->pci = pci;

    إذا (nvme_controller_enable(ctrl)) {
        اذهب err_free_ctrl;
    }

    أرجع;

 err_free_ctrl:
    حرر(ctrl);
 err:
    dprintf(2, "Failed to enable NVMe controller.\n");
}

// Locate and init NVMe controllers
ساكن فراغ
nvme_پیمایش(فراغ)
{
    // Scan PCI bus for NVMe adapters
    بنية pci_device *pci;

    foreachpci(pci) {
        إذا (pci->class != PCI_CLASS_STORAGE_NVME)
            استمر;
        إذا (pci->prog_if != 2 /* as of NVM 1.0e */) {
            dprintf(3, "Found incompatble NVMe: prog-if=%02x\n", pci->prog_if);
            استمر;
        }

        پیاپی_thread(nvme_controller_راهاندازی, pci);
    }
}

ساكن صحيح
nvme_cmd_readwrite(بنية nvme_namespace *ns, بنية disk_op_s *op, صحيح اكتب)
{
    صحيح i;
    لكل (i = 0; i < op->شمار;) {
        u16 بلوکها_مانده = op->شمار - i;
        حرف *op_buf = op->buf_fl + i * ns->بلوک_اندازه;
        صحيح بلوکها = nvme_prpl_xfer(ns, op->lba + i, op_buf,
                                    بلوکها_مانده, اكتب);
        إذا (بلوکها < 0)
            أرجع DISK_RET_EBADTRACK;
        i += بلوکها;
    }

    أرجع DISK_RET_SUCCESS;
}

صحيح
nvme_process_op(بنية disk_op_s *op)
{
    إذا (!CONFIG_NVME)
        أرجع DISK_RET_SUCCESS;

    بنية nvme_namespace *ns = container_از(op->drive_fl, بنية nvme_namespace,
                                             drive);

    بدل (op->command) {
    حالة CMD_READ:
    حالة CMD_WRITE:
        أرجع nvme_cmd_readwrite(ns, op, op->command == CMD_WRITE);
    افتراضي:
        أرجع default_process_op(op);
    }
}

فراغ
nvme_راهاندازی(فراغ)
{
    ASSERT32FLAT();
    إذا (!CONFIG_NVME)
        أرجع;

    dprintf(3, "init nvme\n");
    nvme_پیمایش();
}

/* EOF */

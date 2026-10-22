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
স্থির শূন্য *nvme_dma_বাফার;

স্থির শূন্য *
zalloc_page_aligned(কাঠামো zone_s *zone, u32 আকার_2)
{
    শূন্য *res = _malloc(zone, আকার_2, NVME_PAGE_আকার);
    যদি (res) memset(res, 0, আকার_2);
    ফেরত res;
}

স্থির শূন্য
nvme_init_queue_common(কাঠামো nvme_ctrl *ctrl, কাঠামো nvme_queue *q, u16 q_idx,
                       u16 দৈর্ঘ্য)
{
    memset(q, 0, আকার(*q));
    q->dbl = (u32 *)((অক্ষর *)ctrl->reg + 0x1000 + q_idx * ctrl->doorbell_stride);
    dprintf(3, " q %p q_idx %u dbl %p\n", q, q_idx, q->dbl);
    q->mask = দৈর্ঘ্য - 1;
}

স্থির পূর্ণসংখ্যা
nvme_init_sq(কাঠামো nvme_ctrl *ctrl, কাঠামো nvme_sq *sq, u16 q_idx, u16 দৈর্ঘ্য,
             কাঠামো nvme_cq *cq)
{
    nvme_init_queue_common(ctrl, &sq->common, q_idx, দৈর্ঘ্য);
    sq->sqe = zalloc_page_aligned(&Zone_উপরের, আকার(*sq->sqe) * দৈর্ঘ্য);

    যদি (!sq->sqe) {
        warn_noalloc();
        ফেরত -1;
    }

    dprintf(3, "sq %p q_idx %u sqe %p\n", sq, q_idx, sq->sqe);
    sq->cq   = cq;
    sq->শীর্ষ = 0;
    sq->লেজ = 0;

    ফেরত 0;
}

স্থির পূর্ণসংখ্যা
nvme_init_cq(কাঠামো nvme_ctrl *ctrl, কাঠামো nvme_cq *cq, u16 q_idx, u16 দৈর্ঘ্য)
{
    nvme_init_queue_common(ctrl, &cq->common, q_idx, দৈর্ঘ্য);
    cq->cqe = zalloc_page_aligned(&Zone_উপরের, আকার(*cq->cqe) * দৈর্ঘ্য);
    যদি (!cq->cqe) {
        warn_noalloc();
        ফেরত -1;
    }

    cq->শীর্ষ = 0;

    /* All CQE phase bits are initialized to zero. This means initially we wait
       for the host controller to set these to 1. */
    cq->phase = 1;

    ফেরত 0;
}

স্থির পূর্ণসংখ্যা
nvme_poll_cq(কাঠামো nvme_cq *cq)
{
    u32 dw3 = readl(&cq->cqe[cq->শীর্ষ].dword[3]);
    ফেরত (!!(dw3 & NVME_CQE_DW3_P) == cq->phase);
}

স্থির পূর্ণসংখ্যা
nvme_is_cqe_success(কাঠামো nvme_cqe ধ্রুবক *cqe)
{
    ফেরত ((cqe->status >> 1) & 0xFF) == 0;
}

স্থির কাঠামো nvme_cqe
nvme_error_cqe(শূন্য)
{
    কাঠামো nvme_cqe r;

    /* 0xFF is a vendor specific status code != success. Should be okay for
       indicating failure. */
    memset(&r, 0xFF, আকার(r));
    ফেরত r;
}

স্থির কাঠামো nvme_cqe
nvme_consume_cqe(কাঠামো nvme_sq *sq)
{
    কাঠামো nvme_cq *cq = sq->cq;

    যদি (!nvme_poll_cq(cq)) {
        /* Cannot consume a completion queue entry, if there is none ready. */
        ফেরত nvme_error_cqe();
    }

    কাঠামো nvme_cqe *cqe = &cq->cqe[cq->শীর্ষ];
    u16 cq_পরবর্তী_শীর্ষ = (cq->শীর্ষ + 1) & cq->common.mask;
    dprintf(4, "cq %p head %u -> %u\n", cq, cq->শীর্ষ, cq_পরবর্তী_শীর্ষ);
    যদি (cq_পরবর্তী_শীর্ষ < cq->শীর্ষ) {
        dprintf(3, "cq %p wrap\n", cq);
        cq->phase = ~cq->phase;
    }
    cq->শীর্ষ = cq_পরবর্তী_শীর্ষ;

    /* Update the submission queue head. */
    যদি (cqe->sq_শীর্ষ != sq->শীর্ষ) {
        sq->শীর্ষ = cqe->sq_শীর্ষ;
        dprintf(4, "sq %p advanced to %u\n", sq, cqe->sq_শীর্ষ);
    }

    /* Tell the controller that we consumed the completion. */
    writel(cq->common.dbl, cq->শীর্ষ);

    ফেরত *cqe;
}

স্থির কাঠামো nvme_cqe
nvme_wait(কাঠামো nvme_sq *sq)
{
    স্থির ধ্রুবক unsigned nvme_timeout = 5000 /* ms */;
    u32 to = timer_calc(nvme_timeout);
    যখন (!nvme_poll_cq(sq->cq)) {
        yield();

        যদি (timer_check(to)) {
            warn_timeout();
            ফেরত nvme_error_cqe();
        }
    }

    ফেরত nvme_consume_cqe(sq);
}

/* Returns the next submission queue entry (or NULL if the queue is full). It
   also fills out Command Dword 0 and clears the rest. */
স্থির কাঠামো nvme_sqe *
nvme_get_পরবর্তী_sqe(কাঠামো nvme_sq *sq, u8 opc, শূন্য *metadata, শূন্য *তথ্য, শূন্য *data2)
{
    যদি (((sq->শীর্ষ + 1) & sq->common.mask) == sq->লেজ) {
        dprintf(3, "submission queue is full\n");
        ফেরত ফাঁকা;
    }

    কাঠামো nvme_sqe *sqe = &sq->sqe[sq->লেজ];
    dprintf(4, "sq %p next_sqe %u\n", sq, sq->লেজ);

    memset(sqe, 0, আকার(*sqe));
    sqe->cdw0 = opc | (sq->লেজ << 16 /* CID */);
    sqe->mptr = (u32)metadata;
    sqe->dptr_prp1 = (u32)তথ্য;
    sqe->dptr_prp2 = (u32)data2;

    ফেরত sqe;
}

/* Call this after you've filled out an sqe that you've got from nvme_get_next_sqe. */
স্থির শূন্য
nvme_commit_sqe(কাঠামো nvme_sq *sq)
{
    dprintf(4, "sq %p commit_sqe %u\n", sq, sq->লেজ);
    sq->লেজ = (sq->লেজ + 1) & sq->common.mask;
    writel(sq->common.dbl, sq->লেজ);
}

/* Perform an identify command on the admin queue and return the resulting
   buffer. This may be a NULL pointer, if something failed. This function
   cannot be used after initialization, because it uses buffers in tmp zone. */
স্থির সংঘ nvme_identify *
nvme_admin_identify(কাঠামো nvme_ctrl *ctrl, u8 cns, u32 nsid)
{
    সংঘ nvme_identify *identify_buf = zalloc_page_aligned(&Zone_Tmp_উপরের, 4096);
    যদি (!identify_buf) {
        /* Could not allocate identify buffer. */
        warn_internalerror();
        ফেরত ফাঁকা;
    }

    কাঠামো nvme_sqe *cmd_identify;
    cmd_identify = nvme_get_পরবর্তী_sqe(&ctrl->admin_sq,
                                     NVME_SQE_OPC_ADMIN_IDENTIFY, ফাঁকা,
                                     identify_buf, ফাঁকা);

    যদি (!cmd_identify) {
        warn_internalerror();
        যাও error;
    }

    cmd_identify->nsid = nsid;
    cmd_identify->dword[10] = cns;

    nvme_commit_sqe(&ctrl->admin_sq);

    কাঠামো nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    যদি (!nvme_is_cqe_success(&cqe)) {
        যাও error;
    }

    ফেরত identify_buf;
 error:
    মুক্ত(identify_buf);
    ফেরত ফাঁকা;
}

স্থির কাঠামো nvme_identify_ctrl *
nvme_admin_identify_ctrl(কাঠামো nvme_ctrl *ctrl)
{
    ফেরত &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_CTRL, 0)->ctrl;
}

স্থির কাঠামো nvme_identify_ns *
nvme_admin_identify_ns(কাঠামো nvme_ctrl *ctrl, u32 ns_id)
{
    ফেরত &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_NS,
                                ns_id)->ns;
}

স্থির শূন্য
nvme_probe_ns(কাঠামো nvme_ctrl *ctrl, u32 ns_idx, u8 mdts)
{
    u32 ns_id = ns_idx + 1;

    কাঠামো nvme_identify_ns *id = nvme_admin_identify_ns(ctrl, ns_id);
    যদি (!id) {
        dprintf(2, "NVMe couldn't identify namespace %u.\n", ns_id);
        যাও free_বাফার;
    }

    u8 current_lba_format = id->flbas & 0xF;
    যদি (current_lba_format > id->nlbaf) {
        dprintf(2, "NVMe NS %u: current LBA format %u is beyond what the "
                " namespace supports (%u)?\n",
                ns_id, current_lba_format, id->nlbaf + 1);
        যাও free_বাফার;
    }

    যদি (!id->nsze) {
        dprintf(2, "NVMe NS %u is inactive.\n", ns_id);
        যাও free_বাফার;
    }

    যদি (!nvme_dma_বাফার) {
        nvme_dma_বাফার = zalloc_page_aligned(&Zone_উপরের, NVME_PAGE_আকার);
        যদি (!nvme_dma_বাফার) {
            warn_noalloc();
            যাও free_বাফার;
        }
    }

    কাঠামো nvme_namespace *ns = malloc_fseg(আকার(*ns));
    যদি (!ns) {
        warn_noalloc();
        যাও free_বাফার;
    }
    memset(ns, 0, আকার(*ns));
    ns->ctrl  = ctrl;
    ns->ns_id = ns_id;
    ns->lba_গুনতি = id->nsze;

    কাঠামো nvme_lba_format *fmt = &id->lbaf[current_lba_format];

    ns->block_আকার    = 1U << fmt->lbads;
    ns->metadata_আকার = fmt->ms;

    যদি (ns->block_আকার > NVME_PAGE_আকার) {
        /* If we see devices that trigger this path, we need to increase our
           buffer size. */
        warn_internalerror();
        মুক্ত(ns);
        যাও free_বাফার;
    }

    ns->drive.cntl_id   = ns_idx;
    ns->drive.removable = 0;
    ns->drive.type      = DTYPE_NVME;
    ns->drive.blksize   = ns->block_আকার;
    ns->drive.sectors   = ns->lba_গুনতি;

    যদি (mdts) {
        ns->max_req_আকার = ((1U << mdts) * NVME_PAGE_আকার) / ns->block_আকার;
        dprintf(3, "NVME NS %u max request size: %d sectors\n",
                ns_id, ns->max_req_আকার);
    } অন্যথা {
        ns->max_req_আকার = -1U;
    }

    অক্ষর *desc = znprintf(MAXDESCSIZE, "NVMe NS %u: %llu MiB (%llu %u-byte "
                          "blocks + %u-byte metadata)",
                          ns_id, (ns->lba_গুনতি * ns->block_আকার) >> 20,
                          ns->lba_গুনতি, ns->block_আকার, ns->metadata_আকার);

    dprintf(3, "%s\n", desc);
    boot_add_hd(&ns->drive, desc, bootprio_find_pci_device(ctrl->pci));

free_বাফার:
    মুক্ত (id);
}


/* Release memory allocated for a completion queue */
স্থির শূন্য
nvme_destroy_cq(কাঠামো nvme_cq *cq)
{
    মুক্ত(cq->cqe);
    cq->cqe = ফাঁকা;
}

/* Release memory allocated for a submission queue */
স্থির শূন্য
nvme_destroy_sq(কাঠামো nvme_sq *sq)
{
    মুক্ত(sq->sqe);
    sq->sqe = ফাঁকা;
}

/* Returns 0 on success. */
স্থির পূর্ণসংখ্যা
nvme_create_io_cq(কাঠামো nvme_ctrl *ctrl, কাঠামো nvme_cq *cq, u16 q_idx)
{
    পূর্ণসংখ্যা rc;
    কাঠামো nvme_sqe *cmd_create_cq;
    u32 দৈর্ঘ্য = 1 + (ctrl->reg->cap & 0xffff);
    যদি (দৈর্ঘ্য > NVME_PAGE_আকার / আকার(কাঠামো nvme_cqe))
        দৈর্ঘ্য = NVME_PAGE_আকার / আকার(কাঠামো nvme_cqe);

    rc = nvme_init_cq(ctrl, cq, q_idx, দৈর্ঘ্য);
    যদি (rc) {
        যাও err;
    }

    cmd_create_cq = nvme_get_পরবর্তী_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_CQ, ফাঁকা,
                                      cq->cqe, ফাঁকা);
    যদি (!cmd_create_cq) {
        যাও err_destroy_cq;
    }

    cmd_create_cq->dword[10] = (cq->common.mask << 16) | (q_idx >> 1);
    cmd_create_cq->dword[11] = 1 /* physically contiguous */;

    nvme_commit_sqe(&ctrl->admin_sq);

    কাঠামো nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    যদি (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io cq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        যাও err_destroy_cq;
    }

    ফেরত 0;

err_destroy_cq:
    nvme_destroy_cq(cq);
err:
    ফেরত -1;
}

/* Returns 0 on success. */
স্থির পূর্ণসংখ্যা
nvme_create_io_sq(কাঠামো nvme_ctrl *ctrl, কাঠামো nvme_sq *sq, u16 q_idx, কাঠামো nvme_cq *cq)
{
    পূর্ণসংখ্যা rc;
    কাঠামো nvme_sqe *cmd_create_sq;
    u32 দৈর্ঘ্য = 1 + (ctrl->reg->cap & 0xffff);
    যদি (দৈর্ঘ্য > NVME_PAGE_আকার / আকার(কাঠামো nvme_cqe))
        দৈর্ঘ্য = NVME_PAGE_আকার / আকার(কাঠামো nvme_cqe);

    rc = nvme_init_sq(ctrl, sq, q_idx, দৈর্ঘ্য, cq);
    যদি (rc) {
        যাও err;
    }

    cmd_create_sq = nvme_get_পরবর্তী_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_SQ, ফাঁকা,
                                      sq->sqe, ফাঁকা);
    যদি (!cmd_create_sq) {
        যাও err_destroy_sq;
    }

    cmd_create_sq->dword[10] = (sq->common.mask << 16) | (q_idx >> 1);
    cmd_create_sq->dword[11] = (q_idx >> 1) << 16 | 1 /* contiguous */;
    dprintf(3, "sq %p create dword10 %08x dword11 %08x\n", sq,
            cmd_create_sq->dword[10], cmd_create_sq->dword[11]);

    nvme_commit_sqe(&ctrl->admin_sq);

    কাঠামো nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    যদি (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io sq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);
        যাও err_destroy_sq;
    }

    ফেরত 0;

err_destroy_sq:
    nvme_destroy_sq(sq);
err:
    ফেরত -1;
}

/* Reads count sectors into buf. The buffer cannot cross page boundaries. */
স্থির পূর্ণসংখ্যা
nvme_io_xfer(কাঠামো nvme_namespace *ns, u64 lba, শূন্য *prp1, শূন্য *prp2,
             u16 গুনতি, পূর্ণসংখ্যা লিখ)
{
    যদি (((u32)prp1 & 0x3) || ((u32)prp2 & 0x3)) {
        /* Buffer is misaligned */
        warn_internalerror();
        ফেরত -1;
    }

    কাঠামো nvme_sqe *io_read = nvme_get_পরবর্তী_sqe(&ns->ctrl->io_sq,
                                                 লিখ ? NVME_SQE_OPC_IO_WRITE
                                                       : NVME_SQE_OPC_IO_READ,
                                                 ফাঁকা, prp1, prp2);
    io_read->nsid = ns->ns_id;
    io_read->dword[10] = (u32)lba;
    io_read->dword[11] = (u32)(lba >> 32);
    io_read->dword[12] = (1U << 31 /* limited retry */) | (গুনতি - 1);

    nvme_commit_sqe(&ns->ctrl->io_sq);

    কাঠামো nvme_cqe cqe = nvme_wait(&ns->ctrl->io_sq);

    যদি (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "read io: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        ফেরত -1;
    }

    dprintf(5, "ns %u %s lba %llu+%u\n", ns->ns_id, লিখ ? "write" : "read",
            lba, গুনতি);
    ফেরত গুনতি;
}

// Transfer up to one page of data using the internal dma bounce buffer
স্থির পূর্ণসংখ্যা
nvme_bounce_xfer(কাঠামো nvme_namespace *ns, u64 lba, শূন্য *buf, u16 গুনতি,
                 পূর্ণসংখ্যা লিখ)
{
    u16 ধ্রুবক max_blocks = NVME_PAGE_আকার / ns->block_আকার;
    u16 blocks = গুনতি < max_blocks ? গুনতি : max_blocks;

    যদি (লিখ)
        memcpy(nvme_dma_বাফার, buf, blocks * ns->block_আকার);

    পূর্ণসংখ্যা res = nvme_io_xfer(ns, lba, nvme_dma_বাফার, ফাঁকা, blocks, লিখ);

    যদি (!লিখ && res >= 0)
        memcpy(buf, nvme_dma_বাফার, res * ns->block_আকার);

    ফেরত res;
}

#define NVME_MAX_PRPL_ENTRIES 15 /* Allows requests up to 64kb */

// Transfer data using page list (if applicable)
স্থির পূর্ণসংখ্যা
nvme_prpl_xfer(কাঠামো nvme_namespace *ns, u64 lba, শূন্য *buf, u16 গুনতি,
               পূর্ণসংখ্যা লিখ)
{
    u32 base = (long)buf;
    s32 আকার_2;

    যদি (গুনতি > ns->max_req_আকার)
        গুনতি = ns->max_req_আকার;

    আকার_2 = গুনতি * ns->block_আকার;
    /* Special case for transfers that fit into PRP1, but are unaligned */
    যদি (((আকার_2 + (base & ~NVME_PAGE_MASK)) <= NVME_PAGE_আকার))
        যাও single;

    /* Every request has to be page aligned */
    যদি (base & ~NVME_PAGE_MASK)
        যাও bounce;

    /* Make sure a full block fits into the last chunk */
    যদি (আকার_2 & (ns->block_আকার - 1ULL))
        যাও bounce;

    /* Build PRP list if we need to describe more than 2 pages */
    যদি ((ns->block_আকার * গুনতি) > (NVME_PAGE_আকার * 2)) {
        u32 prpl_len = 0;
        u64 *prpl = nvme_dma_বাফার;
        পূর্ণসংখ্যা first_page = 1;
        জন্য (; আকার_2 > 0; base += NVME_PAGE_আকার, আকার_2 -= NVME_PAGE_আকার) {
            যদি (first_page) {
                /* First page is special */
                first_page = 0;
                চালু;
            }
            যদি (prpl_len >= NVME_MAX_PRPL_ENTRIES)
                যাও bounce;
            prpl[prpl_len++] = base;
        }
        ফেরত nvme_io_xfer(ns, lba, buf, prpl, গুনতি, লিখ);
    }

    /* Directly embed the 2nd page if we only need 2 pages */
    যদি ((ns->block_আকার * গুনতি) > NVME_PAGE_আকার)
        ফেরত nvme_io_xfer(ns, lba, buf, buf + NVME_PAGE_আকার, গুনতি, লিখ);

single:
    /* One page is enough, don't expose anything else */
    ফেরত nvme_io_xfer(ns, lba, buf, ফাঁকা, গুনতি, লিখ);

bounce:
    /* Use bounce buffer to make transfer */
    ফেরত nvme_bounce_xfer(ns, lba, buf, গুনতি, লিখ);
}

স্থির পূর্ণসংখ্যা
nvme_create_io_queues(কাঠামো nvme_ctrl *ctrl)
{
    যদি (nvme_create_io_cq(ctrl, &ctrl->io_cq, 3))
        যাও err;

    যদি (nvme_create_io_sq(ctrl, &ctrl->io_sq, 2, &ctrl->io_cq))
        যাও err_free_cq;

    ফেরত 0;

 err_free_cq:
    nvme_destroy_cq(&ctrl->io_cq);
 err:
    ফেরত -1;
}

/* Waits for CSTS.RDY to match rdy. Returns 0 on success. */
স্থির পূর্ণসংখ্যা
nvme_wait_csts_rdy(কাঠামো nvme_ctrl *ctrl, unsigned rdy)
{
    u32 ধ্রুবক max_to = 500 /* ms */ * ((ctrl->reg->cap >> 24) & 0xFFU);
    u32 to = timer_calc(max_to);
    u32 csts;

    যখন (rdy != ((csts = ctrl->reg->csts) & NVME_CSTS_RDY)) {
        yield();

        যদি (csts & NVME_CSTS_FATAL) {
            dprintf(3, "NVMe fatal error during controller shutdown\n");
            ফেরত -1;
        }

        যদি (timer_check(to)) {
            warn_timeout();
            ফেরত -1;
        }
    }

    ফেরত 0;
}

/* Returns 0 on success. */
স্থির পূর্ণসংখ্যা
nvme_controller_enable(কাঠামো nvme_ctrl *ctrl)
{
    পূর্ণসংখ্যা rc;

    pci_enable_busmaster(ctrl->pci);

    /* Turn the controller off. */
    ctrl->reg->cc = 0;
    যদি (nvme_wait_csts_rdy(ctrl, 0)) {
        dprintf(2, "NVMe fatal error during controller shutdown\n");
        ফেরত -1;
    }

    ctrl->doorbell_stride = 4U << ((ctrl->reg->cap >> 32) & 0xF);

    rc = nvme_init_cq(ctrl, &ctrl->admin_cq, 1,
                      NVME_PAGE_আকার / আকার(কাঠামো nvme_cqe));
    যদি (rc) {
        ফেরত -1;
    }

    rc = nvme_init_sq(ctrl, &ctrl->admin_sq, 0,
                      NVME_PAGE_আকার / আকার(কাঠামো nvme_sqe), &ctrl->admin_cq);
    যদি (rc) {
        যাও err_destroy_admin_cq;
    }

    ctrl->reg->aqa = ctrl->admin_cq.common.mask << 16
        | ctrl->admin_sq.common.mask;

    ctrl->reg->asq = (u32)ctrl->admin_sq.sqe;
    ctrl->reg->acq = (u32)ctrl->admin_cq.cqe;

    dprintf(3, "  admin submission queue: %p\n", ctrl->admin_sq.sqe);
    dprintf(3, "  admin completion queue: %p\n", ctrl->admin_cq.cqe);

    ctrl->reg->cc = NVME_CC_EN | (NVME_CQE_আকার_LOG << 20)
        | (NVME_SQE_আকার_LOG << 16 /* IOSQES */);

    যদি (nvme_wait_csts_rdy(ctrl, 1)) {
        dprintf(2, "NVMe fatal error while enabling controller\n");
        যাও err_destroy_admin_sq;
    }

    /* The admin queue is set up and the controller is ready. Let's figure out
       what namespaces we have. */

    কাঠামো nvme_identify_ctrl *identify = nvme_admin_identify_ctrl(ctrl);

    যদি (!identify) {
        dprintf(2, "NVMe couldn't identify controller.\n");
        যাও err_destroy_admin_sq;
    }

    dprintf(3, "NVMe has %u namespace%s.\n",
            identify->nn, (identify->nn == 1) ? "" : "s");

    ctrl->ns_গুনতি = identify->nn;
    u8 mdts = identify->mdts;
    মুক্ত(identify);

    যদি ((ctrl->ns_গুনতি == 0) || nvme_create_io_queues(ctrl)) {
        /* No point to continue, if the controller says it doesn't have
           namespaces or we couldn't create I/O queues. */
        যাও err_destroy_admin_sq;
    }

    /* Populate namespace IDs */
    পূর্ণসংখ্যা ns_idx;
    জন্য (ns_idx = 0; ns_idx < ctrl->ns_গুনতি; ns_idx++) {
        nvme_probe_ns(ctrl, ns_idx, mdts);
    }

    dprintf(3, "NVMe initialization complete!\n");
    ফেরত 0;

 err_destroy_admin_sq:
    nvme_destroy_sq(&ctrl->admin_sq);
 err_destroy_admin_cq:
    nvme_destroy_cq(&ctrl->admin_cq);
    ফেরত -1;
}

/* Initialize an NVMe controller and detect its drives. */
স্থির শূন্য
nvme_controller_setup(শূন্য *opaque)
{
    u8 skip_nonbootable = is_bootprio_strict();
    কাঠামো pci_device *pci = opaque;

    যদি (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
        dprintf(1, "skipping init of a non-bootable NVMe at %pP\n",
                pci);
        যাও err;
    }

    কাঠামো nvme_reg অস্থির *reg = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    যদি (!reg)
        ফেরত;

    u32 version = reg->vs;
    dprintf(3, "Found NVMe controller with version %u.%u.%u.\n",
            version >> 16, (version >> 8) & 0xFF, version & 0xFF);
    dprintf(3, "  Capabilities %016llx\n", reg->cap);

    যদি (~reg->cap & NVME_CAP_CSS_NVME) {
        dprintf(3, "Controller doesn't speak NVMe command set. Skipping.\n");
        যাও err;
    }

    কাঠামো nvme_ctrl *ctrl = malloc_উপরের(আকার(*ctrl));
    যদি (!ctrl) {
        warn_noalloc();
        যাও err;
    }

    memset(ctrl, 0, আকার(*ctrl));

    ctrl->reg = reg;
    ctrl->pci = pci;

    যদি (nvme_controller_enable(ctrl)) {
        যাও err_free_ctrl;
    }

    ফেরত;

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
    কাঠামো pci_device *pci;

    foreachpci(pci) {
        যদি (pci->class != PCI_CLASS_STORAGE_NVME)
            চালু;
        যদি (pci->prog_if != 2 /* as of NVM 1.0e */) {
            dprintf(3, "Found incompatble NVMe: prog-if=%02x\n", pci->prog_if);
            চালু;
        }

        run_thread(nvme_controller_setup, pci);
    }
}

স্থির পূর্ণসংখ্যা
nvme_cmd_readwrite(কাঠামো nvme_namespace *ns, কাঠামো disk_op_s *op, পূর্ণসংখ্যা লিখ)
{
    পূর্ণসংখ্যা i;
    জন্য (i = 0; i < op->গুনতি;) {
        u16 blocks_remaining = op->গুনতি - i;
        অক্ষর *op_buf = op->buf_fl + i * ns->block_আকার;
        পূর্ণসংখ্যা blocks = nvme_prpl_xfer(ns, op->lba + i, op_buf,
                                    blocks_remaining, লিখ);
        যদি (blocks < 0)
            ফেরত DISK_RET_EBADTRACK;
        i += blocks;
    }

    ফেরত DISK_RET_SUCCESS;
}

পূর্ণসংখ্যা
nvme_process_op(কাঠামো disk_op_s *op)
{
    যদি (!CONFIG_NVME)
        ফেরত DISK_RET_SUCCESS;

    কাঠামো nvme_namespace *ns = container_of(op->drive_fl, কাঠামো nvme_namespace,
                                             drive);

    নির্বাচন (op->command) {
    ক্ষেত্র CMD_READ:
    ক্ষেত্র CMD_WRITE:
        ফেরত nvme_cmd_readwrite(ns, op, op->command == CMD_WRITE);
    পূর্বনির্ধারিত:
        ফেরত default_process_op(op);
    }
}

শূন্য
nvme_setup(শূন্য)
{
    ASSERT32FLAT();
    যদি (!CONFIG_NVME)
        ফেরত;

    dprintf(3, "init nvme\n");
    nvme_scan();
}

/* EOF */

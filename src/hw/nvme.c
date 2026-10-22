// Low level NVMe disk access
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "blockcmd.h"
#अन्तर्भाव "malloc.h" // malloc_high
#अन्तर्भाव "output.h" // dprintf
#अन्तर्भाव "pci.h"
#अन्तर्भाव "pci_ids.h" // PCI_CLASS_STORAGE_NVME
#अन्तर्भाव "pci_regs.h" // PCI_BASE_ADDRESS_0
#अन्तर्भाव "pcidevice.h" // foreachpci
#अन्तर्भाव "stacks.h" // yield
#अन्तर्भाव "std/disk.h" // DISK_RET_
#अन्तर्भाव "string.h" // memset
#अन्तर्भाव "util.h" // boot_add_hd
#अन्तर्भाव "x86.h" // readl

#अन्तर्भाव "nvme.h"
#अन्तर्भाव "nvme-int.h"

// Page aligned "dma bounce buffer" of size NVME_PAGE_SIZE in high memory
स्थिर शून्य *nvme_dma_buffer;

स्थिर शून्य *
zalloc_page_aligned(संरचना zone_s *zone, u32 size)
{
    शून्य *res = _malloc(zone, size, NVME_PAGE_SIZE);
    यदि (res) memset(res, 0, size);
    प्रत्यागम res;
}

स्थिर शून्य
nvme_init_queue_common(संरचना nvme_ctrl *ctrl, संरचना nvme_queue *q, u16 q_idx,
                       u16 length)
{
    memset(q, 0, परिमाण(*q));
    q->dbl = (u32 *)((वर्ण *)ctrl->reg + 0x1000 + q_idx * ctrl->doorbell_stride);
    dprintf(3, " q %p q_idx %u dbl %p\n", q, q_idx, q->dbl);
    q->mask = length - 1;
}

स्थिर पूर्णाङ्क
nvme_init_sq(संरचना nvme_ctrl *ctrl, संरचना nvme_sq *sq, u16 q_idx, u16 length,
             संरचना nvme_cq *cq)
{
    nvme_init_queue_common(ctrl, &sq->common, q_idx, length);
    sq->sqe = zalloc_page_aligned(&Zone_ऊपरी, परिमाण(*sq->sqe) * length);

    यदि (!sq->sqe) {
        warn_noalloc();
        प्रत्यागम -1;
    }

    dprintf(3, "sq %p q_idx %u sqe %p\n", sq, q_idx, sq->sqe);
    sq->cq   = cq;
    sq->head = 0;
    sq->tail = 0;

    प्रत्यागम 0;
}

स्थिर पूर्णाङ्क
nvme_init_cq(संरचना nvme_ctrl *ctrl, संरचना nvme_cq *cq, u16 q_idx, u16 length)
{
    nvme_init_queue_common(ctrl, &cq->common, q_idx, length);
    cq->cqe = zalloc_page_aligned(&Zone_ऊपरी, परिमाण(*cq->cqe) * length);
    यदि (!cq->cqe) {
        warn_noalloc();
        प्रत्यागम -1;
    }

    cq->head = 0;

    /* All CQE phase bits are initialized to zero. This means initially we wait
       for the host controller to set these to 1. */
    cq->phase = 1;

    प्रत्यागम 0;
}

स्थिर पूर्णाङ्क
nvme_poll_cq(संरचना nvme_cq *cq)
{
    u32 dw3 = readl(&cq->cqe[cq->head].dword[3]);
    प्रत्यागम (!!(dw3 & NVME_CQE_DW3_P) == cq->phase);
}

स्थिर पूर्णाङ्क
nvme_is_cqe_success(संरचना nvme_cqe ध्रुव *cqe)
{
    प्रत्यागम ((cqe->status >> 1) & 0xFF) == 0;
}

स्थिर संरचना nvme_cqe
nvme_error_cqe(शून्य)
{
    संरचना nvme_cqe r;

    /* 0xFF is a vendor specific status code != success. Should be okay for
       indicating failure. */
    memset(&r, 0xFF, परिमाण(r));
    प्रत्यागम r;
}

स्थिर संरचना nvme_cqe
nvme_consume_cqe(संरचना nvme_sq *sq)
{
    संरचना nvme_cq *cq = sq->cq;

    यदि (!nvme_poll_cq(cq)) {
        /* Cannot consume a completion queue entry, if there is none ready. */
        प्रत्यागम nvme_error_cqe();
    }

    संरचना nvme_cqe *cqe = &cq->cqe[cq->head];
    u16 cq_next_head = (cq->head + 1) & cq->common.mask;
    dprintf(4, "cq %p head %u -> %u\n", cq, cq->head, cq_next_head);
    यदि (cq_next_head < cq->head) {
        dprintf(3, "cq %p wrap\n", cq);
        cq->phase = ~cq->phase;
    }
    cq->head = cq_next_head;

    /* Update the submission queue head. */
    यदि (cqe->sq_head != sq->head) {
        sq->head = cqe->sq_head;
        dprintf(4, "sq %p advanced to %u\n", sq, cqe->sq_head);
    }

    /* Tell the controller that we consumed the completion. */
    writel(cq->common.dbl, cq->head);

    प्रत्यागम *cqe;
}

स्थिर संरचना nvme_cqe
nvme_wait(संरचना nvme_sq *sq)
{
    स्थिर ध्रुव unsigned nvme_timeout = 5000 /* ms */;
    u32 to = timer_calc(nvme_timeout);
    यावत् (!nvme_poll_cq(sq->cq)) {
        yield();

        यदि (timer_check(to)) {
            warn_timeout();
            प्रत्यागम nvme_error_cqe();
        }
    }

    प्रत्यागम nvme_consume_cqe(sq);
}

/* Returns the next submission queue entry (or NULL if the queue is full). It
   also fills out Command Dword 0 and clears the rest. */
स्थिर संरचना nvme_sqe *
nvme_get_next_sqe(संरचना nvme_sq *sq, u8 opc, शून्य *metadata, शून्य *data, शून्य *data2)
{
    यदि (((sq->head + 1) & sq->common.mask) == sq->tail) {
        dprintf(3, "submission queue is full\n");
        प्रत्यागम रिक्त;
    }

    संरचना nvme_sqe *sqe = &sq->sqe[sq->tail];
    dprintf(4, "sq %p next_sqe %u\n", sq, sq->tail);

    memset(sqe, 0, परिमाण(*sqe));
    sqe->cdw0 = opc | (sq->tail << 16 /* CID */);
    sqe->mptr = (u32)metadata;
    sqe->dptr_prp1 = (u32)data;
    sqe->dptr_prp2 = (u32)data2;

    प्रत्यागम sqe;
}

/* Call this after you've filled out an sqe that you've got from nvme_get_next_sqe. */
स्थिर शून्य
nvme_commit_sqe(संरचना nvme_sq *sq)
{
    dprintf(4, "sq %p commit_sqe %u\n", sq, sq->tail);
    sq->tail = (sq->tail + 1) & sq->common.mask;
    writel(sq->common.dbl, sq->tail);
}

/* Perform an identify command on the admin queue and return the resulting
   buffer. This may be a NULL pointer, if something failed. This function
   cannot be used after initialization, because it uses buffers in tmp zone. */
स्थिर सङ्घ nvme_identify *
nvme_admin_identify(संरचना nvme_ctrl *ctrl, u8 cns, u32 nsid)
{
    सङ्घ nvme_identify *identify_buf = zalloc_page_aligned(&Zone_Tmp_ऊपरी, 4096);
    यदि (!identify_buf) {
        /* Could not allocate identify buffer. */
        warn_internalerror();
        प्रत्यागम रिक्त;
    }

    संरचना nvme_sqe *cmd_identify;
    cmd_identify = nvme_get_next_sqe(&ctrl->admin_sq,
                                     NVME_SQE_OPC_ADMIN_IDENTIFY, रिक्त,
                                     identify_buf, रिक्त);

    यदि (!cmd_identify) {
        warn_internalerror();
        गच्छ error;
    }

    cmd_identify->nsid = nsid;
    cmd_identify->dword[10] = cns;

    nvme_commit_sqe(&ctrl->admin_sq);

    संरचना nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    यदि (!nvme_is_cqe_success(&cqe)) {
        गच्छ error;
    }

    प्रत्यागम identify_buf;
 error:
    मुक्त(identify_buf);
    प्रत्यागम रिक्त;
}

स्थिर संरचना nvme_identify_ctrl *
nvme_admin_identify_ctrl(संरचना nvme_ctrl *ctrl)
{
    प्रत्यागम &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_CTRL, 0)->ctrl;
}

स्थिर संरचना nvme_identify_ns *
nvme_admin_identify_ns(संरचना nvme_ctrl *ctrl, u32 ns_id)
{
    प्रत्यागम &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_NS,
                                ns_id)->ns;
}

स्थिर शून्य
nvme_probe_ns(संरचना nvme_ctrl *ctrl, u32 ns_idx, u8 mdts)
{
    u32 ns_id = ns_idx + 1;

    संरचना nvme_identify_ns *id = nvme_admin_identify_ns(ctrl, ns_id);
    यदि (!id) {
        dprintf(2, "NVMe couldn't identify namespace %u.\n", ns_id);
        गच्छ free_buffer;
    }

    u8 current_lba_format = id->flbas & 0xF;
    यदि (current_lba_format > id->nlbaf) {
        dprintf(2, "NVMe NS %u: current LBA format %u is beyond what the "
                " namespace supports (%u)?\n",
                ns_id, current_lba_format, id->nlbaf + 1);
        गच्छ free_buffer;
    }

    यदि (!id->nsze) {
        dprintf(2, "NVMe NS %u is inactive.\n", ns_id);
        गच्छ free_buffer;
    }

    यदि (!nvme_dma_buffer) {
        nvme_dma_buffer = zalloc_page_aligned(&Zone_ऊपरी, NVME_PAGE_SIZE);
        यदि (!nvme_dma_buffer) {
            warn_noalloc();
            गच्छ free_buffer;
        }
    }

    संरचना nvme_namespace *ns = malloc_fseg(परिमाण(*ns));
    यदि (!ns) {
        warn_noalloc();
        गच्छ free_buffer;
    }
    memset(ns, 0, परिमाण(*ns));
    ns->ctrl  = ctrl;
    ns->ns_id = ns_id;
    ns->lba_गिनती = id->nsze;

    संरचना nvme_lba_format *fmt = &id->lbaf[current_lba_format];

    ns->block_size    = 1U << fmt->lbads;
    ns->metadata_size = fmt->ms;

    यदि (ns->block_size > NVME_PAGE_SIZE) {
        /* If we see devices that trigger this path, we need to increase our
           buffer size. */
        warn_internalerror();
        मुक्त(ns);
        गच्छ free_buffer;
    }

    ns->drive.cntl_id   = ns_idx;
    ns->drive.removable = 0;
    ns->drive.type      = DTYPE_NVME;
    ns->drive.blksize   = ns->block_size;
    ns->drive.sectors   = ns->lba_गिनती;

    यदि (mdts) {
        ns->max_req_size = ((1U << mdts) * NVME_PAGE_SIZE) / ns->block_size;
        dprintf(3, "NVME NS %u max request size: %d sectors\n",
                ns_id, ns->max_req_size);
    } अन्यथा {
        ns->max_req_size = -1U;
    }

    वर्ण *desc = znprintf(MAXDESCSIZE, "NVMe NS %u: %llu MiB (%llu %u-byte "
                          "blocks + %u-byte metadata)",
                          ns_id, (ns->lba_गिनती * ns->block_size) >> 20,
                          ns->lba_गिनती, ns->block_size, ns->metadata_size);

    dprintf(3, "%s\n", desc);
    boot_add_hd(&ns->drive, desc, bootprio_find_pci_device(ctrl->pci));

free_buffer:
    मुक्त (id);
}


/* Release memory allocated for a completion queue */
स्थिर शून्य
nvme_destroy_cq(संरचना nvme_cq *cq)
{
    मुक्त(cq->cqe);
    cq->cqe = रिक्त;
}

/* Release memory allocated for a submission queue */
स्थिर शून्य
nvme_destroy_sq(संरचना nvme_sq *sq)
{
    मुक्त(sq->sqe);
    sq->sqe = रिक्त;
}

/* Returns 0 on success. */
स्थिर पूर्णाङ्क
nvme_create_io_cq(संरचना nvme_ctrl *ctrl, संरचना nvme_cq *cq, u16 q_idx)
{
    पूर्णाङ्क rc;
    संरचना nvme_sqe *cmd_create_cq;
    u32 length = 1 + (ctrl->reg->cap & 0xffff);
    यदि (length > NVME_PAGE_SIZE / परिमाण(संरचना nvme_cqe))
        length = NVME_PAGE_SIZE / परिमाण(संरचना nvme_cqe);

    rc = nvme_init_cq(ctrl, cq, q_idx, length);
    यदि (rc) {
        गच्छ err;
    }

    cmd_create_cq = nvme_get_next_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_CQ, रिक्त,
                                      cq->cqe, रिक्त);
    यदि (!cmd_create_cq) {
        गच्छ err_destroy_cq;
    }

    cmd_create_cq->dword[10] = (cq->common.mask << 16) | (q_idx >> 1);
    cmd_create_cq->dword[11] = 1 /* physically contiguous */;

    nvme_commit_sqe(&ctrl->admin_sq);

    संरचना nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    यदि (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io cq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        गच्छ err_destroy_cq;
    }

    प्रत्यागम 0;

err_destroy_cq:
    nvme_destroy_cq(cq);
err:
    प्रत्यागम -1;
}

/* Returns 0 on success. */
स्थिर पूर्णाङ्क
nvme_create_io_sq(संरचना nvme_ctrl *ctrl, संरचना nvme_sq *sq, u16 q_idx, संरचना nvme_cq *cq)
{
    पूर्णाङ्क rc;
    संरचना nvme_sqe *cmd_create_sq;
    u32 length = 1 + (ctrl->reg->cap & 0xffff);
    यदि (length > NVME_PAGE_SIZE / परिमाण(संरचना nvme_cqe))
        length = NVME_PAGE_SIZE / परिमाण(संरचना nvme_cqe);

    rc = nvme_init_sq(ctrl, sq, q_idx, length, cq);
    यदि (rc) {
        गच्छ err;
    }

    cmd_create_sq = nvme_get_next_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_SQ, रिक्त,
                                      sq->sqe, रिक्त);
    यदि (!cmd_create_sq) {
        गच्छ err_destroy_sq;
    }

    cmd_create_sq->dword[10] = (sq->common.mask << 16) | (q_idx >> 1);
    cmd_create_sq->dword[11] = (q_idx >> 1) << 16 | 1 /* contiguous */;
    dprintf(3, "sq %p create dword10 %08x dword11 %08x\n", sq,
            cmd_create_sq->dword[10], cmd_create_sq->dword[11]);

    nvme_commit_sqe(&ctrl->admin_sq);

    संरचना nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    यदि (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io sq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);
        गच्छ err_destroy_sq;
    }

    प्रत्यागम 0;

err_destroy_sq:
    nvme_destroy_sq(sq);
err:
    प्रत्यागम -1;
}

/* Reads count sectors into buf. The buffer cannot cross page boundaries. */
स्थिर पूर्णाङ्क
nvme_io_xfer(संरचना nvme_namespace *ns, u64 lba, शून्य *prp1, शून्य *prp2,
             u16 गिनती, पूर्णाङ्क लिख)
{
    यदि (((u32)prp1 & 0x3) || ((u32)prp2 & 0x3)) {
        /* Buffer is misaligned */
        warn_internalerror();
        प्रत्यागम -1;
    }

    संरचना nvme_sqe *io_read = nvme_get_next_sqe(&ns->ctrl->io_sq,
                                                 लिख ? NVME_SQE_OPC_IO_WRITE
                                                       : NVME_SQE_OPC_IO_READ,
                                                 रिक्त, prp1, prp2);
    io_read->nsid = ns->ns_id;
    io_read->dword[10] = (u32)lba;
    io_read->dword[11] = (u32)(lba >> 32);
    io_read->dword[12] = (1U << 31 /* limited retry */) | (गिनती - 1);

    nvme_commit_sqe(&ns->ctrl->io_sq);

    संरचना nvme_cqe cqe = nvme_wait(&ns->ctrl->io_sq);

    यदि (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "read io: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        प्रत्यागम -1;
    }

    dprintf(5, "ns %u %s lba %llu+%u\n", ns->ns_id, लिख ? "write" : "read",
            lba, गिनती);
    प्रत्यागम गिनती;
}

// Transfer up to one page of data using the internal dma bounce buffer
स्थिर पूर्णाङ्क
nvme_bounce_xfer(संरचना nvme_namespace *ns, u64 lba, शून्य *buf, u16 गिनती,
                 पूर्णाङ्क लिख)
{
    u16 ध्रुव max_blocks = NVME_PAGE_SIZE / ns->block_size;
    u16 blocks = गिनती < max_blocks ? गिनती : max_blocks;

    यदि (लिख)
        memcpy(nvme_dma_buffer, buf, blocks * ns->block_size);

    पूर्णाङ्क res = nvme_io_xfer(ns, lba, nvme_dma_buffer, रिक्त, blocks, लिख);

    यदि (!लिख && res >= 0)
        memcpy(buf, nvme_dma_buffer, res * ns->block_size);

    प्रत्यागम res;
}

#define NVME_MAX_PRPL_ENTRIES 15 /* Allows requests up to 64kb */

// Transfer data using page list (if applicable)
स्थिर पूर्णाङ्क
nvme_prpl_xfer(संरचना nvme_namespace *ns, u64 lba, शून्य *buf, u16 गिनती,
               पूर्णाङ्क लिख)
{
    u32 base = (long)buf;
    s32 size;

    यदि (गिनती > ns->max_req_size)
        गिनती = ns->max_req_size;

    size = गिनती * ns->block_size;
    /* Special case for transfers that fit into PRP1, but are unaligned */
    यदि (((size + (base & ~NVME_PAGE_MASK)) <= NVME_PAGE_SIZE))
        गच्छ single;

    /* Every request has to be page aligned */
    यदि (base & ~NVME_PAGE_MASK)
        गच्छ bounce;

    /* Make sure a full block fits into the last chunk */
    यदि (size & (ns->block_size - 1ULL))
        गच्छ bounce;

    /* Build PRP list if we need to describe more than 2 pages */
    यदि ((ns->block_size * गिनती) > (NVME_PAGE_SIZE * 2)) {
        u32 prpl_len = 0;
        u64 *prpl = nvme_dma_buffer;
        पूर्णाङ्क first_page = 1;
        प्रति (; size > 0; base += NVME_PAGE_SIZE, size -= NVME_PAGE_SIZE) {
            यदि (first_page) {
                /* First page is special */
                first_page = 0;
                अनुवर्त;
            }
            यदि (prpl_len >= NVME_MAX_PRPL_ENTRIES)
                गच्छ bounce;
            prpl[prpl_len++] = base;
        }
        प्रत्यागम nvme_io_xfer(ns, lba, buf, prpl, गिनती, लिख);
    }

    /* Directly embed the 2nd page if we only need 2 pages */
    यदि ((ns->block_size * गिनती) > NVME_PAGE_SIZE)
        प्रत्यागम nvme_io_xfer(ns, lba, buf, buf + NVME_PAGE_SIZE, गिनती, लिख);

single:
    /* One page is enough, don't expose anything else */
    प्रत्यागम nvme_io_xfer(ns, lba, buf, रिक्त, गिनती, लिख);

bounce:
    /* Use bounce buffer to make transfer */
    प्रत्यागम nvme_bounce_xfer(ns, lba, buf, गिनती, लिख);
}

स्थिर पूर्णाङ्क
nvme_create_io_queues(संरचना nvme_ctrl *ctrl)
{
    यदि (nvme_create_io_cq(ctrl, &ctrl->io_cq, 3))
        गच्छ err;

    यदि (nvme_create_io_sq(ctrl, &ctrl->io_sq, 2, &ctrl->io_cq))
        गच्छ err_free_cq;

    प्रत्यागम 0;

 err_free_cq:
    nvme_destroy_cq(&ctrl->io_cq);
 err:
    प्रत्यागम -1;
}

/* Waits for CSTS.RDY to match rdy. Returns 0 on success. */
स्थिर पूर्णाङ्क
nvme_wait_csts_rdy(संरचना nvme_ctrl *ctrl, unsigned rdy)
{
    u32 ध्रुव max_to = 500 /* ms */ * ((ctrl->reg->cap >> 24) & 0xFFU);
    u32 to = timer_calc(max_to);
    u32 csts;

    यावत् (rdy != ((csts = ctrl->reg->csts) & NVME_CSTS_RDY)) {
        yield();

        यदि (csts & NVME_CSTS_FATAL) {
            dprintf(3, "NVMe fatal error during controller shutdown\n");
            प्रत्यागम -1;
        }

        यदि (timer_check(to)) {
            warn_timeout();
            प्रत्यागम -1;
        }
    }

    प्रत्यागम 0;
}

/* Returns 0 on success. */
स्थिर पूर्णाङ्क
nvme_controller_enable(संरचना nvme_ctrl *ctrl)
{
    पूर्णाङ्क rc;

    pci_enable_busmaster(ctrl->pci);

    /* Turn the controller off. */
    ctrl->reg->cc = 0;
    यदि (nvme_wait_csts_rdy(ctrl, 0)) {
        dprintf(2, "NVMe fatal error during controller shutdown\n");
        प्रत्यागम -1;
    }

    ctrl->doorbell_stride = 4U << ((ctrl->reg->cap >> 32) & 0xF);

    rc = nvme_init_cq(ctrl, &ctrl->admin_cq, 1,
                      NVME_PAGE_SIZE / परिमाण(संरचना nvme_cqe));
    यदि (rc) {
        प्रत्यागम -1;
    }

    rc = nvme_init_sq(ctrl, &ctrl->admin_sq, 0,
                      NVME_PAGE_SIZE / परिमाण(संरचना nvme_sqe), &ctrl->admin_cq);
    यदि (rc) {
        गच्छ err_destroy_admin_cq;
    }

    ctrl->reg->aqa = ctrl->admin_cq.common.mask << 16
        | ctrl->admin_sq.common.mask;

    ctrl->reg->asq = (u32)ctrl->admin_sq.sqe;
    ctrl->reg->acq = (u32)ctrl->admin_cq.cqe;

    dprintf(3, "  admin submission queue: %p\n", ctrl->admin_sq.sqe);
    dprintf(3, "  admin completion queue: %p\n", ctrl->admin_cq.cqe);

    ctrl->reg->cc = NVME_CC_EN | (NVME_CQE_SIZE_LOG << 20)
        | (NVME_SQE_SIZE_LOG << 16 /* IOSQES */);

    यदि (nvme_wait_csts_rdy(ctrl, 1)) {
        dprintf(2, "NVMe fatal error while enabling controller\n");
        गच्छ err_destroy_admin_sq;
    }

    /* The admin queue is set up and the controller is ready. Let's figure out
       what namespaces we have. */

    संरचना nvme_identify_ctrl *identify = nvme_admin_identify_ctrl(ctrl);

    यदि (!identify) {
        dprintf(2, "NVMe couldn't identify controller.\n");
        गच्छ err_destroy_admin_sq;
    }

    dprintf(3, "NVMe has %u namespace%s.\n",
            identify->nn, (identify->nn == 1) ? "" : "s");

    ctrl->ns_गिनती = identify->nn;
    u8 mdts = identify->mdts;
    मुक्त(identify);

    यदि ((ctrl->ns_गिनती == 0) || nvme_create_io_queues(ctrl)) {
        /* No point to continue, if the controller says it doesn't have
           namespaces or we couldn't create I/O queues. */
        गच्छ err_destroy_admin_sq;
    }

    /* Populate namespace IDs */
    पूर्णाङ्क ns_idx;
    प्रति (ns_idx = 0; ns_idx < ctrl->ns_गिनती; ns_idx++) {
        nvme_probe_ns(ctrl, ns_idx, mdts);
    }

    dprintf(3, "NVMe initialization complete!\n");
    प्रत्यागम 0;

 err_destroy_admin_sq:
    nvme_destroy_sq(&ctrl->admin_sq);
 err_destroy_admin_cq:
    nvme_destroy_cq(&ctrl->admin_cq);
    प्रत्यागम -1;
}

/* Initialize an NVMe controller and detect its drives. */
स्थिर शून्य
nvme_controller_setup(शून्य *opaque)
{
    u8 skip_nonbootable = is_bootprio_strict();
    संरचना pci_device *pci = opaque;

    यदि (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
        dprintf(1, "skipping init of a non-bootable NVMe at %pP\n",
                pci);
        गच्छ err;
    }

    संरचना nvme_reg चञ्चल *reg = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    यदि (!reg)
        प्रत्यागम;

    u32 version = reg->vs;
    dprintf(3, "Found NVMe controller with version %u.%u.%u.\n",
            version >> 16, (version >> 8) & 0xFF, version & 0xFF);
    dprintf(3, "  Capabilities %016llx\n", reg->cap);

    यदि (~reg->cap & NVME_CAP_CSS_NVME) {
        dprintf(3, "Controller doesn't speak NVMe command set. Skipping.\n");
        गच्छ err;
    }

    संरचना nvme_ctrl *ctrl = malloc_ऊपरी(परिमाण(*ctrl));
    यदि (!ctrl) {
        warn_noalloc();
        गच्छ err;
    }

    memset(ctrl, 0, परिमाण(*ctrl));

    ctrl->reg = reg;
    ctrl->pci = pci;

    यदि (nvme_controller_enable(ctrl)) {
        गच्छ err_free_ctrl;
    }

    प्रत्यागम;

 err_free_ctrl:
    मुक्त(ctrl);
 err:
    dprintf(2, "Failed to enable NVMe controller.\n");
}

// Locate and init NVMe controllers
स्थिर शून्य
nvme_scan(शून्य)
{
    // Scan PCI bus for NVMe adapters
    संरचना pci_device *pci;

    foreachpci(pci) {
        यदि (pci->class != PCI_CLASS_STORAGE_NVME)
            अनुवर्त;
        यदि (pci->prog_if != 2 /* as of NVM 1.0e */) {
            dprintf(3, "Found incompatble NVMe: prog-if=%02x\n", pci->prog_if);
            अनुवर्त;
        }

        run_thread(nvme_controller_setup, pci);
    }
}

स्थिर पूर्णाङ्क
nvme_cmd_readwrite(संरचना nvme_namespace *ns, संरचना disk_op_s *op, पूर्णाङ्क लिख)
{
    पूर्णाङ्क i;
    प्रति (i = 0; i < op->गिनती;) {
        u16 blocks_remaining = op->गिनती - i;
        वर्ण *op_buf = op->buf_fl + i * ns->block_size;
        पूर्णाङ्क blocks = nvme_prpl_xfer(ns, op->lba + i, op_buf,
                                    blocks_remaining, लिख);
        यदि (blocks < 0)
            प्रत्यागम DISK_RET_EBADTRACK;
        i += blocks;
    }

    प्रत्यागम DISK_RET_SUCCESS;
}

पूर्णाङ्क
nvme_process_op(संरचना disk_op_s *op)
{
    यदि (!CONFIG_NVME)
        प्रत्यागम DISK_RET_SUCCESS;

    संरचना nvme_namespace *ns = container_of(op->drive_fl, संरचना nvme_namespace,
                                             drive);

    विकल्प (op->command) {
    पक्ष CMD_READ:
    पक्ष CMD_WRITE:
        प्रत्यागम nvme_cmd_readwrite(ns, op, op->command == CMD_WRITE);
    स्वभाव:
        प्रत्यागम default_process_op(op);
    }
}

शून्य
nvme_setup(शून्य)
{
    ASSERT32FLAT();
    यदि (!CONFIG_NVME)
        प्रत्यागम;

    dprintf(3, "init nvme\n");
    nvme_scan();
}

/* EOF */

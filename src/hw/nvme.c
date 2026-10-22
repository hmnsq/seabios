// Low level NVMe disk access
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "blockcmd.h"
#समावेश "malloc.h" // malloc_high
#समावेश "output.h" // dprintf
#समावेश "pci.h"
#समावेश "pci_ids.h" // PCI_CLASS_STORAGE_NVME
#समावेश "pci_regs.h" // PCI_BASE_ADDRESS_0
#समावेश "pcidevice.h" // foreachpci
#समावेश "stacks.h" // yield
#समावेश "std/disk.h" // DISK_RET_
#समावेश "string.h" // memset
#समावेश "util.h" // boot_add_hd
#समावेश "x86.h" // readl

#समावेश "nvme.h"
#समावेश "nvme-int.h"

// Page aligned "dma bounce buffer" of size NVME_PAGE_SIZE in high memory
अटल व्योम *nvme_dma_बफर;

अटल व्योम *
zalloc_page_aligned(काष्ठा zone_s *zone, u32 आकार)
{
    व्योम *res = _malloc(zone, आकार, NVME_PAGE_आकार);
    अगर (res) स_रखो(res, 0, आकार);
    वापस res;
}

अटल व्योम
nvme_init_queue_common(काष्ठा nvme_ctrl *ctrl, काष्ठा nvme_queue *q, u16 q_idx,
                       u16 लंबाई)
{
    स_रखो(q, 0, माप(*q));
    q->dbl = (u32 *)((अक्षर *)ctrl->reg + 0x1000 + q_idx * ctrl->doorbell_stride);
    dprintf(3, " q %p q_idx %u dbl %p\n", q, q_idx, q->dbl);
    q->mask = लंबाई - 1;
}

अटल पूर्णांक
nvme_init_sq(काष्ठा nvme_ctrl *ctrl, काष्ठा nvme_sq *sq, u16 q_idx, u16 लंबाई,
             काष्ठा nvme_cq *cq)
{
    nvme_init_queue_common(ctrl, &sq->common, q_idx, लंबाई);
    sq->sqe = zalloc_page_aligned(&Zone_ऊपरी, माप(*sq->sqe) * लंबाई);

    अगर (!sq->sqe) {
        warn_noalloc();
        वापस -1;
    }

    dprintf(3, "sq %p q_idx %u sqe %p\n", sq, q_idx, sq->sqe);
    sq->cq   = cq;
    sq->शीर्ष = 0;
    sq->पुच्छ = 0;

    वापस 0;
}

अटल पूर्णांक
nvme_init_cq(काष्ठा nvme_ctrl *ctrl, काष्ठा nvme_cq *cq, u16 q_idx, u16 लंबाई)
{
    nvme_init_queue_common(ctrl, &cq->common, q_idx, लंबाई);
    cq->cqe = zalloc_page_aligned(&Zone_ऊपरी, माप(*cq->cqe) * लंबाई);
    अगर (!cq->cqe) {
        warn_noalloc();
        वापस -1;
    }

    cq->शीर्ष = 0;

    /* All CQE phase bits are initialized to zero. This means initially we wait
       for the host controller to set these to 1. */
    cq->phase = 1;

    वापस 0;
}

अटल पूर्णांक
nvme_poll_cq(काष्ठा nvme_cq *cq)
{
    u32 dw3 = readl(&cq->cqe[cq->शीर्ष].dword[3]);
    वापस (!!(dw3 & NVME_CQE_DW3_P) == cq->phase);
}

अटल पूर्णांक
nvme_is_cqe_success(काष्ठा nvme_cqe स्थिर *cqe)
{
    वापस ((cqe->status >> 1) & 0xFF) == 0;
}

अटल काष्ठा nvme_cqe
nvme_error_cqe(व्योम)
{
    काष्ठा nvme_cqe r;

    /* 0xFF is a vendor specific status code != success. Should be okay for
       indicating failure. */
    स_रखो(&r, 0xFF, माप(r));
    वापस r;
}

अटल काष्ठा nvme_cqe
nvme_consume_cqe(काष्ठा nvme_sq *sq)
{
    काष्ठा nvme_cq *cq = sq->cq;

    अगर (!nvme_poll_cq(cq)) {
        /* Cannot consume a completion queue entry, if there is none ready. */
        वापस nvme_error_cqe();
    }

    काष्ठा nvme_cqe *cqe = &cq->cqe[cq->शीर्ष];
    u16 cq_अग्र_शीर्ष = (cq->शीर्ष + 1) & cq->common.mask;
    dprintf(4, "cq %p head %u -> %u\n", cq, cq->शीर्ष, cq_अग्र_शीर्ष);
    अगर (cq_अग्र_शीर्ष < cq->शीर्ष) {
        dprintf(3, "cq %p wrap\n", cq);
        cq->phase = ~cq->phase;
    }
    cq->शीर्ष = cq_अग्र_शीर्ष;

    /* Update the submission queue head. */
    अगर (cqe->sq_शीर्ष != sq->शीर्ष) {
        sq->शीर्ष = cqe->sq_शीर्ष;
        dprintf(4, "sq %p advanced to %u\n", sq, cqe->sq_शीर्ष);
    }

    /* Tell the controller that we consumed the completion. */
    writel(cq->common.dbl, cq->शीर्ष);

    वापस *cqe;
}

अटल काष्ठा nvme_cqe
nvme_wait(काष्ठा nvme_sq *sq)
{
    अटल स्थिर अचिन्हित nvme_timeout = 5000 /* ms */;
    u32 to = timer_calc(nvme_timeout);
    जबतक (!nvme_poll_cq(sq->cq)) {
        yield();

        अगर (timer_check(to)) {
            warn_timeout();
            वापस nvme_error_cqe();
        }
    }

    वापस nvme_consume_cqe(sq);
}

/* Returns the next submission queue entry (or NULL if the queue is full). It
   also fills out Command Dword 0 and clears the rest. */
अटल काष्ठा nvme_sqe *
nvme_get_अग्र_sqe(काष्ठा nvme_sq *sq, u8 opc, व्योम *metadata, व्योम *तथ्य, व्योम *data2)
{
    अगर (((sq->शीर्ष + 1) & sq->common.mask) == sq->पुच्छ) {
        dprintf(3, "submission queue is full\n");
        वापस शून्य;
    }

    काष्ठा nvme_sqe *sqe = &sq->sqe[sq->पुच्छ];
    dprintf(4, "sq %p next_sqe %u\n", sq, sq->पुच्छ);

    स_रखो(sqe, 0, माप(*sqe));
    sqe->cdw0 = opc | (sq->पुच्छ << 16 /* CID */);
    sqe->mptr = (u32)metadata;
    sqe->dptr_prp1 = (u32)तथ्य;
    sqe->dptr_prp2 = (u32)data2;

    वापस sqe;
}

/* Call this after you've filled out an sqe that you've got from nvme_get_next_sqe. */
अटल व्योम
nvme_commit_sqe(काष्ठा nvme_sq *sq)
{
    dprintf(4, "sq %p commit_sqe %u\n", sq, sq->पुच्छ);
    sq->पुच्छ = (sq->पुच्छ + 1) & sq->common.mask;
    writel(sq->common.dbl, sq->पुच्छ);
}

/* Perform an identify command on the admin queue and return the resulting
   buffer. This may be a NULL pointer, if something failed. This function
   cannot be used after initialization, because it uses buffers in tmp zone. */
अटल जोड़ nvme_identify *
nvme_admin_identify(काष्ठा nvme_ctrl *ctrl, u8 cns, u32 nsid)
{
    जोड़ nvme_identify *identify_buf = zalloc_page_aligned(&Zone_Tmp_ऊपरी, 4096);
    अगर (!identify_buf) {
        /* Could not allocate identify buffer. */
        warn_internalerror();
        वापस शून्य;
    }

    काष्ठा nvme_sqe *cmd_identify;
    cmd_identify = nvme_get_अग्र_sqe(&ctrl->admin_sq,
                                     NVME_SQE_OPC_ADMIN_IDENTIFY, शून्य,
                                     identify_buf, शून्य);

    अगर (!cmd_identify) {
        warn_internalerror();
        जाओ error;
    }

    cmd_identify->nsid = nsid;
    cmd_identify->dword[10] = cns;

    nvme_commit_sqe(&ctrl->admin_sq);

    काष्ठा nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    अगर (!nvme_is_cqe_success(&cqe)) {
        जाओ error;
    }

    वापस identify_buf;
 error:
    मुक्त(identify_buf);
    वापस शून्य;
}

अटल काष्ठा nvme_identify_ctrl *
nvme_admin_identify_ctrl(काष्ठा nvme_ctrl *ctrl)
{
    वापस &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_CTRL, 0)->ctrl;
}

अटल काष्ठा nvme_identify_ns *
nvme_admin_identify_ns(काष्ठा nvme_ctrl *ctrl, u32 ns_id)
{
    वापस &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_NS,
                                ns_id)->ns;
}

अटल व्योम
nvme_probe_ns(काष्ठा nvme_ctrl *ctrl, u32 ns_idx, u8 mdts)
{
    u32 ns_id = ns_idx + 1;

    काष्ठा nvme_identify_ns *id = nvme_admin_identify_ns(ctrl, ns_id);
    अगर (!id) {
        dprintf(2, "NVMe couldn't identify namespace %u.\n", ns_id);
        जाओ free_बफर;
    }

    u8 current_lba_format = id->flbas & 0xF;
    अगर (current_lba_format > id->nlbaf) {
        dprintf(2, "NVMe NS %u: current LBA format %u is beyond what the "
                " namespace supports (%u)?\n",
                ns_id, current_lba_format, id->nlbaf + 1);
        जाओ free_बफर;
    }

    अगर (!id->nsze) {
        dprintf(2, "NVMe NS %u is inactive.\n", ns_id);
        जाओ free_बफर;
    }

    अगर (!nvme_dma_बफर) {
        nvme_dma_बफर = zalloc_page_aligned(&Zone_ऊपरी, NVME_PAGE_आकार);
        अगर (!nvme_dma_बफर) {
            warn_noalloc();
            जाओ free_बफर;
        }
    }

    काष्ठा nvme_namespace *ns = malloc_fseg(माप(*ns));
    अगर (!ns) {
        warn_noalloc();
        जाओ free_बफर;
    }
    स_रखो(ns, 0, माप(*ns));
    ns->ctrl  = ctrl;
    ns->ns_id = ns_id;
    ns->lba_गिनती = id->nsze;

    काष्ठा nvme_lba_format *fmt = &id->lbaf[current_lba_format];

    ns->block_आकार    = 1U << fmt->lbads;
    ns->metadata_आकार = fmt->ms;

    अगर (ns->block_आकार > NVME_PAGE_आकार) {
        /* If we see devices that trigger this path, we need to increase our
           buffer size. */
        warn_internalerror();
        मुक्त(ns);
        जाओ free_बफर;
    }

    ns->drive.cntl_id   = ns_idx;
    ns->drive.removable = 0;
    ns->drive.type      = DTYPE_NVME;
    ns->drive.blksize   = ns->block_आकार;
    ns->drive.sectors   = ns->lba_गिनती;

    अगर (mdts) {
        ns->max_req_आकार = ((1U << mdts) * NVME_PAGE_आकार) / ns->block_आकार;
        dprintf(3, "NVME NS %u max request size: %d sectors\n",
                ns_id, ns->max_req_आकार);
    } अन्यथा {
        ns->max_req_आकार = -1U;
    }

    अक्षर *desc = znprintf(MAXDESCSIZE, "NVMe NS %u: %llu MiB (%llu %u-byte "
                          "blocks + %u-byte metadata)",
                          ns_id, (ns->lba_गिनती * ns->block_आकार) >> 20,
                          ns->lba_गिनती, ns->block_आकार, ns->metadata_आकार);

    dprintf(3, "%s\n", desc);
    boot_add_hd(&ns->drive, desc, bootprio_find_pci_device(ctrl->pci));

free_बफर:
    मुक्त (id);
}


/* Release memory allocated for a completion queue */
अटल व्योम
nvme_destroy_cq(काष्ठा nvme_cq *cq)
{
    मुक्त(cq->cqe);
    cq->cqe = शून्य;
}

/* Release memory allocated for a submission queue */
अटल व्योम
nvme_destroy_sq(काष्ठा nvme_sq *sq)
{
    मुक्त(sq->sqe);
    sq->sqe = शून्य;
}

/* Returns 0 on success. */
अटल पूर्णांक
nvme_create_io_cq(काष्ठा nvme_ctrl *ctrl, काष्ठा nvme_cq *cq, u16 q_idx)
{
    पूर्णांक rc;
    काष्ठा nvme_sqe *cmd_create_cq;
    u32 लंबाई = 1 + (ctrl->reg->cap & 0xffff);
    अगर (लंबाई > NVME_PAGE_आकार / माप(काष्ठा nvme_cqe))
        लंबाई = NVME_PAGE_आकार / माप(काष्ठा nvme_cqe);

    rc = nvme_init_cq(ctrl, cq, q_idx, लंबाई);
    अगर (rc) {
        जाओ err;
    }

    cmd_create_cq = nvme_get_अग्र_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_CQ, शून्य,
                                      cq->cqe, शून्य);
    अगर (!cmd_create_cq) {
        जाओ err_destroy_cq;
    }

    cmd_create_cq->dword[10] = (cq->common.mask << 16) | (q_idx >> 1);
    cmd_create_cq->dword[11] = 1 /* physically contiguous */;

    nvme_commit_sqe(&ctrl->admin_sq);

    काष्ठा nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    अगर (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io cq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        जाओ err_destroy_cq;
    }

    वापस 0;

err_destroy_cq:
    nvme_destroy_cq(cq);
err:
    वापस -1;
}

/* Returns 0 on success. */
अटल पूर्णांक
nvme_create_io_sq(काष्ठा nvme_ctrl *ctrl, काष्ठा nvme_sq *sq, u16 q_idx, काष्ठा nvme_cq *cq)
{
    पूर्णांक rc;
    काष्ठा nvme_sqe *cmd_create_sq;
    u32 लंबाई = 1 + (ctrl->reg->cap & 0xffff);
    अगर (लंबाई > NVME_PAGE_आकार / माप(काष्ठा nvme_cqe))
        लंबाई = NVME_PAGE_आकार / माप(काष्ठा nvme_cqe);

    rc = nvme_init_sq(ctrl, sq, q_idx, लंबाई, cq);
    अगर (rc) {
        जाओ err;
    }

    cmd_create_sq = nvme_get_अग्र_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_SQ, शून्य,
                                      sq->sqe, शून्य);
    अगर (!cmd_create_sq) {
        जाओ err_destroy_sq;
    }

    cmd_create_sq->dword[10] = (sq->common.mask << 16) | (q_idx >> 1);
    cmd_create_sq->dword[11] = (q_idx >> 1) << 16 | 1 /* contiguous */;
    dprintf(3, "sq %p create dword10 %08x dword11 %08x\n", sq,
            cmd_create_sq->dword[10], cmd_create_sq->dword[11]);

    nvme_commit_sqe(&ctrl->admin_sq);

    काष्ठा nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    अगर (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io sq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);
        जाओ err_destroy_sq;
    }

    वापस 0;

err_destroy_sq:
    nvme_destroy_sq(sq);
err:
    वापस -1;
}

/* Reads count sectors into buf. The buffer cannot cross page boundaries. */
अटल पूर्णांक
nvme_io_xfer(काष्ठा nvme_namespace *ns, u64 lba, व्योम *prp1, व्योम *prp2,
             u16 गिनती, पूर्णांक ग_लिखो)
{
    अगर (((u32)prp1 & 0x3) || ((u32)prp2 & 0x3)) {
        /* Buffer is misaligned */
        warn_internalerror();
        वापस -1;
    }

    काष्ठा nvme_sqe *io_read = nvme_get_अग्र_sqe(&ns->ctrl->io_sq,
                                                 ग_लिखो ? NVME_SQE_OPC_IO_WRITE
                                                       : NVME_SQE_OPC_IO_READ,
                                                 शून्य, prp1, prp2);
    io_read->nsid = ns->ns_id;
    io_read->dword[10] = (u32)lba;
    io_read->dword[11] = (u32)(lba >> 32);
    io_read->dword[12] = (1U << 31 /* limited retry */) | (गिनती - 1);

    nvme_commit_sqe(&ns->ctrl->io_sq);

    काष्ठा nvme_cqe cqe = nvme_wait(&ns->ctrl->io_sq);

    अगर (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "read io: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        वापस -1;
    }

    dprintf(5, "ns %u %s lba %llu+%u\n", ns->ns_id, ग_लिखो ? "write" : "read",
            lba, गिनती);
    वापस गिनती;
}

// Transfer up to one page of data using the internal dma bounce buffer
अटल पूर्णांक
nvme_bounce_xfer(काष्ठा nvme_namespace *ns, u64 lba, व्योम *buf, u16 गिनती,
                 पूर्णांक ग_लिखो)
{
    u16 स्थिर max_blocks = NVME_PAGE_आकार / ns->block_आकार;
    u16 blocks = गिनती < max_blocks ? गिनती : max_blocks;

    अगर (ग_लिखो)
        स_नकल(nvme_dma_बफर, buf, blocks * ns->block_आकार);

    पूर्णांक res = nvme_io_xfer(ns, lba, nvme_dma_बफर, शून्य, blocks, ग_लिखो);

    अगर (!ग_लिखो && res >= 0)
        स_नकल(buf, nvme_dma_बफर, res * ns->block_आकार);

    वापस res;
}

#घोषणा NVME_MAX_PRPL_ENTRIES 15 /* Allows requests up to 64kb */

// Transfer data using page list (if applicable)
अटल पूर्णांक
nvme_prpl_xfer(काष्ठा nvme_namespace *ns, u64 lba, व्योम *buf, u16 गिनती,
               पूर्णांक ग_लिखो)
{
    u32 base = (दीर्घ)buf;
    s32 आकार;

    अगर (गिनती > ns->max_req_आकार)
        गिनती = ns->max_req_आकार;

    आकार = गिनती * ns->block_आकार;
    /* Special case for transfers that fit into PRP1, but are unaligned */
    अगर (((आकार + (base & ~NVME_PAGE_MASK)) <= NVME_PAGE_आकार))
        जाओ single;

    /* Every request has to be page aligned */
    अगर (base & ~NVME_PAGE_MASK)
        जाओ bounce;

    /* Make sure a full block fits into the last chunk */
    अगर (आकार & (ns->block_आकार - 1ULL))
        जाओ bounce;

    /* Build PRP list if we need to describe more than 2 pages */
    अगर ((ns->block_आकार * गिनती) > (NVME_PAGE_आकार * 2)) {
        u32 prpl_len = 0;
        u64 *prpl = nvme_dma_बफर;
        पूर्णांक first_page = 1;
        क्रम (; आकार > 0; base += NVME_PAGE_आकार, आकार -= NVME_PAGE_आकार) {
            अगर (first_page) {
                /* First page is special */
                first_page = 0;
                जारी;
            }
            अगर (prpl_len >= NVME_MAX_PRPL_ENTRIES)
                जाओ bounce;
            prpl[prpl_len++] = base;
        }
        वापस nvme_io_xfer(ns, lba, buf, prpl, गिनती, ग_लिखो);
    }

    /* Directly embed the 2nd page if we only need 2 pages */
    अगर ((ns->block_आकार * गिनती) > NVME_PAGE_आकार)
        वापस nvme_io_xfer(ns, lba, buf, buf + NVME_PAGE_आकार, गिनती, ग_लिखो);

single:
    /* One page is enough, don't expose anything else */
    वापस nvme_io_xfer(ns, lba, buf, शून्य, गिनती, ग_लिखो);

bounce:
    /* Use bounce buffer to make transfer */
    वापस nvme_bounce_xfer(ns, lba, buf, गिनती, ग_लिखो);
}

अटल पूर्णांक
nvme_create_io_queues(काष्ठा nvme_ctrl *ctrl)
{
    अगर (nvme_create_io_cq(ctrl, &ctrl->io_cq, 3))
        जाओ err;

    अगर (nvme_create_io_sq(ctrl, &ctrl->io_sq, 2, &ctrl->io_cq))
        जाओ err_free_cq;

    वापस 0;

 err_free_cq:
    nvme_destroy_cq(&ctrl->io_cq);
 err:
    वापस -1;
}

/* Waits for CSTS.RDY to match rdy. Returns 0 on success. */
अटल पूर्णांक
nvme_wait_csts_rdy(काष्ठा nvme_ctrl *ctrl, अचिन्हित rdy)
{
    u32 स्थिर max_to = 500 /* ms */ * ((ctrl->reg->cap >> 24) & 0xFFU);
    u32 to = timer_calc(max_to);
    u32 csts;

    जबतक (rdy != ((csts = ctrl->reg->csts) & NVME_CSTS_RDY)) {
        yield();

        अगर (csts & NVME_CSTS_FATAL) {
            dprintf(3, "NVMe fatal error during controller shutdown\n");
            वापस -1;
        }

        अगर (timer_check(to)) {
            warn_timeout();
            वापस -1;
        }
    }

    वापस 0;
}

/* Returns 0 on success. */
अटल पूर्णांक
nvme_controller_enable(काष्ठा nvme_ctrl *ctrl)
{
    पूर्णांक rc;

    pci_enable_busmaster(ctrl->pci);

    /* Turn the controller off. */
    ctrl->reg->cc = 0;
    अगर (nvme_wait_csts_rdy(ctrl, 0)) {
        dprintf(2, "NVMe fatal error during controller shutdown\n");
        वापस -1;
    }

    ctrl->doorbell_stride = 4U << ((ctrl->reg->cap >> 32) & 0xF);

    rc = nvme_init_cq(ctrl, &ctrl->admin_cq, 1,
                      NVME_PAGE_आकार / माप(काष्ठा nvme_cqe));
    अगर (rc) {
        वापस -1;
    }

    rc = nvme_init_sq(ctrl, &ctrl->admin_sq, 0,
                      NVME_PAGE_आकार / माप(काष्ठा nvme_sqe), &ctrl->admin_cq);
    अगर (rc) {
        जाओ err_destroy_admin_cq;
    }

    ctrl->reg->aqa = ctrl->admin_cq.common.mask << 16
        | ctrl->admin_sq.common.mask;

    ctrl->reg->asq = (u32)ctrl->admin_sq.sqe;
    ctrl->reg->acq = (u32)ctrl->admin_cq.cqe;

    dprintf(3, "  admin submission queue: %p\n", ctrl->admin_sq.sqe);
    dprintf(3, "  admin completion queue: %p\n", ctrl->admin_cq.cqe);

    ctrl->reg->cc = NVME_CC_EN | (NVME_CQE_आकार_LOG << 20)
        | (NVME_SQE_आकार_LOG << 16 /* IOSQES */);

    अगर (nvme_wait_csts_rdy(ctrl, 1)) {
        dprintf(2, "NVMe fatal error while enabling controller\n");
        जाओ err_destroy_admin_sq;
    }

    /* The admin queue is set up and the controller is ready. Let's figure out
       what namespaces we have. */

    काष्ठा nvme_identify_ctrl *identify = nvme_admin_identify_ctrl(ctrl);

    अगर (!identify) {
        dprintf(2, "NVMe couldn't identify controller.\n");
        जाओ err_destroy_admin_sq;
    }

    dprintf(3, "NVMe has %u namespace%s.\n",
            identify->nn, (identify->nn == 1) ? "" : "s");

    ctrl->ns_गिनती = identify->nn;
    u8 mdts = identify->mdts;
    मुक्त(identify);

    अगर ((ctrl->ns_गिनती == 0) || nvme_create_io_queues(ctrl)) {
        /* No point to continue, if the controller says it doesn't have
           namespaces or we couldn't create I/O queues. */
        जाओ err_destroy_admin_sq;
    }

    /* Populate namespace IDs */
    पूर्णांक ns_idx;
    क्रम (ns_idx = 0; ns_idx < ctrl->ns_गिनती; ns_idx++) {
        nvme_probe_ns(ctrl, ns_idx, mdts);
    }

    dprintf(3, "NVMe initialization complete!\n");
    वापस 0;

 err_destroy_admin_sq:
    nvme_destroy_sq(&ctrl->admin_sq);
 err_destroy_admin_cq:
    nvme_destroy_cq(&ctrl->admin_cq);
    वापस -1;
}

/* Initialize an NVMe controller and detect its drives. */
अटल व्योम
nvme_controller_setup(व्योम *opaque)
{
    u8 skip_nonbootable = is_bootprio_strict();
    काष्ठा pci_device *pci = opaque;

    अगर (skip_nonbootable && bootprio_find_pci_device(pci) < 0) {
        dprintf(1, "skipping init of a non-bootable NVMe at %pP\n",
                pci);
        जाओ err;
    }

    काष्ठा nvme_reg अस्थिर *reg = pci_enable_membar(pci, PCI_BASE_ADDRESS_0);
    अगर (!reg)
        वापस;

    u32 version = reg->vs;
    dprintf(3, "Found NVMe controller with version %u.%u.%u.\n",
            version >> 16, (version >> 8) & 0xFF, version & 0xFF);
    dprintf(3, "  Capabilities %016llx\n", reg->cap);

    अगर (~reg->cap & NVME_CAP_CSS_NVME) {
        dprintf(3, "Controller doesn't speak NVMe command set. Skipping.\n");
        जाओ err;
    }

    काष्ठा nvme_ctrl *ctrl = malloc_ऊपरी(माप(*ctrl));
    अगर (!ctrl) {
        warn_noalloc();
        जाओ err;
    }

    स_रखो(ctrl, 0, माप(*ctrl));

    ctrl->reg = reg;
    ctrl->pci = pci;

    अगर (nvme_controller_enable(ctrl)) {
        जाओ err_free_ctrl;
    }

    वापस;

 err_free_ctrl:
    मुक्त(ctrl);
 err:
    dprintf(2, "Failed to enable NVMe controller.\n");
}

// Locate and init NVMe controllers
अटल व्योम
nvme_scan(व्योम)
{
    // Scan PCI bus for NVMe adapters
    काष्ठा pci_device *pci;

    foreachpci(pci) {
        अगर (pci->class != PCI_CLASS_STORAGE_NVME)
            जारी;
        अगर (pci->prog_if != 2 /* as of NVM 1.0e */) {
            dprintf(3, "Found incompatble NVMe: prog-if=%02x\n", pci->prog_if);
            जारी;
        }

        run_thread(nvme_controller_setup, pci);
    }
}

अटल पूर्णांक
nvme_cmd_readwrite(काष्ठा nvme_namespace *ns, काष्ठा disk_op_s *op, पूर्णांक ग_लिखो)
{
    पूर्णांक i;
    क्रम (i = 0; i < op->गिनती;) {
        u16 blocks_remaining = op->गिनती - i;
        अक्षर *op_buf = op->buf_fl + i * ns->block_आकार;
        पूर्णांक blocks = nvme_prpl_xfer(ns, op->lba + i, op_buf,
                                    blocks_remaining, ग_लिखो);
        अगर (blocks < 0)
            वापस DISK_RET_EBADTRACK;
        i += blocks;
    }

    वापस DISK_RET_SUCCESS;
}

पूर्णांक
nvme_process_op(काष्ठा disk_op_s *op)
{
    अगर (!CONFIG_NVME)
        वापस DISK_RET_SUCCESS;

    काष्ठा nvme_namespace *ns = container_of(op->drive_fl, काष्ठा nvme_namespace,
                                             drive);

    चयन (op->command) {
    हाल CMD_READ:
    हाल CMD_WRITE:
        वापस nvme_cmd_readwrite(ns, op, op->command == CMD_WRITE);
    शेष:
        वापस default_process_op(op);
    }
}

व्योम
nvme_setup(व्योम)
{
    ASSERT32FLAT();
    अगर (!CONFIG_NVME)
        वापस;

    dprintf(3, "init nvme\n");
    nvme_scan();
}

/* EOF */

// Low level NVMe disk access
//
// Copyright 2017 Amazon.com, Inc. or its affiliates.
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "blockcmd.h"
#ಅನ್ತರ್ಭಾವ "malloc.h" // malloc_high
#ಅನ್ತರ್ಭಾವ "output.h" // dprintf
#ಅನ್ತರ್ಭಾವ "pci.h"
#ಅನ್ತರ್ಭಾವ "pci_ids.h" // PCI_CLASS_STORAGE_NVME
#ಅನ್ತರ್ಭಾವ "pci_regs.h" // PCI_BASE_ADDRESS_0
#ಅನ್ತರ್ಭಾವ "pcidevice.h" // foreachpci
#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "std/disk.h" // DISK_RET_
#ಅನ್ತರ್ಭಾವ "string.h" // memset
#ಅನ್ತರ್ಭಾವ "util.h" // boot_add_hd
#ಅನ್ತರ್ಭಾವ "x86.h" // readl

#ಅನ್ತರ್ಭಾವ "nvme.h"
#ಅನ್ತರ್ಭಾವ "nvme-int.h"

// Page aligned "dma bounce buffer" of size NVME_PAGE_SIZE in high memory
ಸ್ಥಿರ ಶೂನ್ಯ *nvme_dma_ಬಫರ್;

ಸ್ಥಿರ ಶೂನ್ಯ *
zalloc_ಪುಟ_aligned(ರಚನೆ zone_s *zone, u32 ಗಾತ್ರ_2)
{
    ಶೂನ್ಯ *res = _malloc(zone, ಗಾತ್ರ_2, NVME_ಪುಟ_ಗಾತ್ರ);
    ಆದರೆ (res) memset(res, 0, ಗಾತ್ರ_2);
    ಹಿಂತಿರುಗಿ res;
}

ಸ್ಥಿರ ಶೂನ್ಯ
nvme_init_ಸರತಿ_common(ರಚನೆ nvme_ctrl *ctrl, ರಚನೆ nvme_ಸರತಿ *q, u16 q_idx,
                       u16 ಉದ್ದಳತೆ)
{
    memset(q, 0, ಗಾತ್ರ(*q));
    q->dbl = (u32 *)((ಅಕ್ಷರ *)ctrl->reg + 0x1000 + q_idx * ctrl->doorbell_stride);
    dprintf(3, " q %p q_idx %u dbl %p\n", q, q_idx, q->dbl);
    q->ಮುಖವಾಡ = ಉದ್ದಳತೆ - 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
nvme_init_sq(ರಚನೆ nvme_ctrl *ctrl, ರಚನೆ nvme_sq *sq, u16 q_idx, u16 ಉದ್ದಳತೆ,
             ರಚನೆ nvme_cq *cq)
{
    nvme_init_ಸರತಿ_common(ctrl, &sq->common, q_idx, ಉದ್ದಳತೆ);
    sq->sqe = zalloc_ಪುಟ_aligned(&Zone_ಮೇಲಿನ, ಗಾತ್ರ(*sq->sqe) * ಉದ್ದಳತೆ);

    ಆದರೆ (!sq->sqe) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ -1;
    }

    dprintf(3, "sq %p q_idx %u sqe %p\n", sq, q_idx, sq->sqe);
    sq->cq   = cq;
    sq->ತಲೆ = 0;
    sq->ಬಾಲ = 0;

    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
nvme_init_cq(ರಚನೆ nvme_ctrl *ctrl, ರಚನೆ nvme_cq *cq, u16 q_idx, u16 ಉದ್ದಳತೆ)
{
    nvme_init_ಸರತಿ_common(ctrl, &cq->common, q_idx, ಉದ್ದಳತೆ);
    cq->cqe = zalloc_ಪುಟ_aligned(&Zone_ಮೇಲಿನ, ಗಾತ್ರ(*cq->cqe) * ಉದ್ದಳತೆ);
    ಆದರೆ (!cq->cqe) {
        warn_noalloc();
        ಹಿಂತಿರುಗಿ -1;
    }

    cq->ತಲೆ = 0;

    /* All CQE phase bits are initialized to zero. This means initially we wait
       for the host controller to set these to 1. */
    cq->phase = 1;

    ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
nvme_poll_cq(ರಚನೆ nvme_cq *cq)
{
    u32 dw3 = readl(&cq->cqe[cq->ತಲೆ].dword[3]);
    ಹಿಂತಿರುಗಿ (!!(dw3 & NVME_CQE_DW3_P) == cq->phase);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
nvme_is_cqe_success(ರಚನೆ nvme_cqe ಸ್ಥಿರಾಂಕ *cqe)
{
    ಹಿಂತಿರುಗಿ ((cqe->ಸ್ಥಿತಿ_2 >> 1) & 0xFF) == 0;
}

ಸ್ಥಿರ ರಚನೆ nvme_cqe
nvme_error_cqe(ಶೂನ್ಯ)
{
    ರಚನೆ nvme_cqe r;

    /* 0xFF is a vendor specific status code != success. Should be okay for
       indicating failure. */
    memset(&r, 0xFF, ಗಾತ್ರ(r));
    ಹಿಂತಿರುಗಿ r;
}

ಸ್ಥಿರ ರಚನೆ nvme_cqe
nvme_consume_cqe(ರಚನೆ nvme_sq *sq)
{
    ರಚನೆ nvme_cq *cq = sq->cq;

    ಆದರೆ (!nvme_poll_cq(cq)) {
        /* Cannot consume a completion queue entry, if there is none ready. */
        ಹಿಂತಿರುಗಿ nvme_error_cqe();
    }

    ರಚನೆ nvme_cqe *cqe = &cq->cqe[cq->ತಲೆ];
    u16 cq_ಮುಂದಿನ_ತಲೆ = (cq->ತಲೆ + 1) & cq->common.ಮುಖವಾಡ;
    dprintf(4, "cq %p head %u -> %u\n", cq, cq->ತಲೆ, cq_ಮುಂದಿನ_ತಲೆ);
    ಆದರೆ (cq_ಮುಂದಿನ_ತಲೆ < cq->ತಲೆ) {
        dprintf(3, "cq %p wrap\n", cq);
        cq->phase = ~cq->phase;
    }
    cq->ತಲೆ = cq_ಮುಂದಿನ_ತಲೆ;

    /* Update the submission queue head. */
    ಆದರೆ (cqe->sq_ತಲೆ != sq->ತಲೆ) {
        sq->ತಲೆ = cqe->sq_ತಲೆ;
        dprintf(4, "sq %p advanced to %u\n", sq, cqe->sq_ತಲೆ);
    }

    /* Tell the controller that we consumed the completion. */
    writel(cq->common.dbl, cq->ತಲೆ);

    ಹಿಂತಿರುಗಿ *cqe;
}

ಸ್ಥಿರ ರಚನೆ nvme_cqe
nvme_wait(ರಚನೆ nvme_sq *sq)
{
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ nvme_timeout = 5000 /* ms */;
    u32 ಗಮ್ಯ = timer_calc(nvme_timeout);
    ಆಗ (!nvme_poll_cq(sq->cq)) {
        yield();

        ಆದರೆ (timer_check(ಗಮ್ಯ)) {
            warn_timeout();
            ಹಿಂತಿರುಗಿ nvme_error_cqe();
        }
    }

    ಹಿಂತಿರುಗಿ nvme_consume_cqe(sq);
}

/* Returns the next submission queue entry (or NULL if the queue is full). It
   also fills out Command Dword 0 and clears the rest. */
ಸ್ಥಿರ ರಚನೆ nvme_sqe *
nvme_get_ಮುಂದಿನ_sqe(ರಚನೆ nvme_sq *sq, u8 opc, ಶೂನ್ಯ *metadata, ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಶೂನ್ಯ *data2)
{
    ಆದರೆ (((sq->ತಲೆ + 1) & sq->common.ಮುಖವಾಡ) == sq->ಬಾಲ) {
        dprintf(3, "submission queue is full\n");
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }

    ರಚನೆ nvme_sqe *sqe = &sq->sqe[sq->ಬಾಲ];
    dprintf(4, "sq %p next_sqe %u\n", sq, sq->ಬಾಲ);

    memset(sqe, 0, ಗಾತ್ರ(*sqe));
    sqe->cdw0 = opc | (sq->ಬಾಲ << 16 /* CID */);
    sqe->mptr = (u32)metadata;
    sqe->dptr_prp1 = (u32)ದತ್ತಾಂಶ;
    sqe->dptr_prp2 = (u32)data2;

    ಹಿಂತಿರುಗಿ sqe;
}

/* Call this after you've filled out an sqe that you've got from nvme_get_next_sqe. */
ಸ್ಥಿರ ಶೂನ್ಯ
nvme_commit_sqe(ರಚನೆ nvme_sq *sq)
{
    dprintf(4, "sq %p commit_sqe %u\n", sq, sq->ಬಾಲ);
    sq->ಬಾಲ = (sq->ಬಾಲ + 1) & sq->common.ಮುಖವಾಡ;
    writel(sq->common.dbl, sq->ಬಾಲ);
}

/* Perform an identify command on the admin queue and return the resulting
   buffer. This may be a NULL pointer, if something failed. This function
   cannot be used after initialization, because it uses buffers in tmp zone. */
ಸ್ಥಿರ ಒಕ್ಕೂಟ nvme_identify *
nvme_admin_identify(ರಚನೆ nvme_ctrl *ctrl, u8 cns, u32 nsid)
{
    ಒಕ್ಕೂಟ nvme_identify *identify_buf = zalloc_ಪುಟ_aligned(&Zone_Tmp_ಮೇಲಿನ, 4096);
    ಆದರೆ (!identify_buf) {
        /* Could not allocate identify buffer. */
        warn_internalerror();
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }

    ರಚನೆ nvme_sqe *cmd_identify;
    cmd_identify = nvme_get_ಮುಂದಿನ_sqe(&ctrl->admin_sq,
                                     NVME_SQE_OPC_ADMIN_IDENTIFY, ಖಾಲಿ,
                                     identify_buf, ಖಾಲಿ);

    ಆದರೆ (!cmd_identify) {
        warn_internalerror();
        ಹೋಗು error;
    }

    cmd_identify->nsid = nsid;
    cmd_identify->dword[10] = cns;

    nvme_commit_sqe(&ctrl->admin_sq);

    ರಚನೆ nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    ಆದರೆ (!nvme_is_cqe_success(&cqe)) {
        ಹೋಗು error;
    }

    ಹಿಂತಿರುಗಿ identify_buf;
 error:
    ಬಿಡುಗಡೆ(identify_buf);
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರ ರಚನೆ nvme_identify_ctrl *
nvme_admin_identify_ctrl(ರಚನೆ nvme_ctrl *ctrl)
{
    ಹಿಂತಿರುಗಿ &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_CTRL, 0)->ctrl;
}

ಸ್ಥಿರ ರಚನೆ nvme_identify_ns *
nvme_admin_identify_ns(ರಚನೆ nvme_ctrl *ctrl, u32 ns_id)
{
    ಹಿಂತಿರುಗಿ &nvme_admin_identify(ctrl, NVME_ADMIN_IDENTIFY_CNS_ID_NS,
                                ns_id)->ns;
}

ಸ್ಥಿರ ಶೂನ್ಯ
nvme_probe_ns(ರಚನೆ nvme_ctrl *ctrl, u32 ns_idx, u8 mdts)
{
    u32 ns_id = ns_idx + 1;

    ರಚನೆ nvme_identify_ns *id = nvme_admin_identify_ns(ctrl, ns_id);
    ಆದರೆ (!id) {
        dprintf(2, "NVMe couldn't identify namespace %u.\n", ns_id);
        ಹೋಗು free_ಬಫರ್;
    }

    u8 current_lba_format = id->flbas & 0xF;
    ಆದರೆ (current_lba_format > id->nlbaf) {
        dprintf(2, "NVMe NS %u: current LBA format %u is beyond what the "
                " namespace supports (%u)?\n",
                ns_id, current_lba_format, id->nlbaf + 1);
        ಹೋಗು free_ಬಫರ್;
    }

    ಆದರೆ (!id->nsze) {
        dprintf(2, "NVMe NS %u is inactive.\n", ns_id);
        ಹೋಗು free_ಬಫರ್;
    }

    ಆದರೆ (!nvme_dma_ಬಫರ್) {
        nvme_dma_ಬಫರ್ = zalloc_ಪುಟ_aligned(&Zone_ಮೇಲಿನ, NVME_ಪುಟ_ಗಾತ್ರ);
        ಆದರೆ (!nvme_dma_ಬಫರ್) {
            warn_noalloc();
            ಹೋಗು free_ಬಫರ್;
        }
    }

    ರಚನೆ nvme_namespace *ns = malloc_fseg(ಗಾತ್ರ(*ns));
    ಆದರೆ (!ns) {
        warn_noalloc();
        ಹೋಗು free_ಬಫರ್;
    }
    memset(ns, 0, ಗಾತ್ರ(*ns));
    ns->ctrl  = ctrl;
    ns->ns_id = ns_id;
    ns->lba_ಎಣಿಕೆ = id->nsze;

    ರಚನೆ nvme_lba_format *fmt = &id->lbaf[current_lba_format];

    ns->ಖಂಡ_ಗಾತ್ರ    = 1U << fmt->lbads;
    ns->metadata_ಗಾತ್ರ = fmt->ms;

    ಆದರೆ (ns->ಖಂಡ_ಗಾತ್ರ > NVME_ಪುಟ_ಗಾತ್ರ) {
        /* If we see devices that trigger this path, we need to increase our
           buffer size. */
        warn_internalerror();
        ಬಿಡುಗಡೆ(ns);
        ಹೋಗು free_ಬಫರ್;
    }

    ns->drive.cntl_id   = ns_idx;
    ns->drive.removable = 0;
    ns->drive.type      = DTYPE_NVME;
    ns->drive.blksize   = ns->ಖಂಡ_ಗಾತ್ರ;
    ns->drive.sectors   = ns->lba_ಎಣಿಕೆ;

    ಆದರೆ (mdts) {
        ns->max_req_ಗಾತ್ರ = ((1U << mdts) * NVME_ಪುಟ_ಗಾತ್ರ) / ns->ಖಂಡ_ಗಾತ್ರ;
        dprintf(3, "NVME NS %u max request size: %d sectors\n",
                ns_id, ns->max_req_ಗಾತ್ರ);
    } ಇಲ್ಲದಿದ್ದರೆ {
        ns->max_req_ಗಾತ್ರ = -1U;
    }

    ಅಕ್ಷರ *desc = znprintf(MAXDESCSIZE, "NVMe NS %u: %llu MiB (%llu %u-byte "
                          "blocks + %u-byte metadata)",
                          ns_id, (ns->lba_ಎಣಿಕೆ * ns->ಖಂಡ_ಗಾತ್ರ) >> 20,
                          ns->lba_ಎಣಿಕೆ, ns->ಖಂಡ_ಗಾತ್ರ, ns->metadata_ಗಾತ್ರ);

    dprintf(3, "%s\n", desc);
    boot_ಕೂಡಿಸು_hd(&ns->drive, desc, bootprio_ಕಂಡುಹಿಡಿ_pci_device(ctrl->pci));

free_ಬಫರ್:
    ಬಿಡುಗಡೆ (id);
}


/* Release memory allocated for a completion queue */
ಸ್ಥಿರ ಶೂನ್ಯ
nvme_destroy_cq(ರಚನೆ nvme_cq *cq)
{
    ಬಿಡುಗಡೆ(cq->cqe);
    cq->cqe = ಖಾಲಿ;
}

/* Release memory allocated for a submission queue */
ಸ್ಥಿರ ಶೂನ್ಯ
nvme_destroy_sq(ರಚನೆ nvme_sq *sq)
{
    ಬಿಡುಗಡೆ(sq->sqe);
    sq->sqe = ಖಾಲಿ;
}

/* Returns 0 on success. */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
nvme_create_io_cq(ರಚನೆ nvme_ctrl *ctrl, ರಚನೆ nvme_cq *cq, u16 q_idx)
{
    ಪೂರ್ಣಾಂಕ rc;
    ರಚನೆ nvme_sqe *cmd_create_cq;
    u32 ಉದ್ದಳತೆ = 1 + (ctrl->reg->cap & 0xffff);
    ಆದರೆ (ಉದ್ದಳತೆ > NVME_ಪುಟ_ಗಾತ್ರ / ಗಾತ್ರ(ರಚನೆ nvme_cqe))
        ಉದ್ದಳತೆ = NVME_ಪುಟ_ಗಾತ್ರ / ಗಾತ್ರ(ರಚನೆ nvme_cqe);

    rc = nvme_init_cq(ctrl, cq, q_idx, ಉದ್ದಳತೆ);
    ಆದರೆ (rc) {
        ಹೋಗು err;
    }

    cmd_create_cq = nvme_get_ಮುಂದಿನ_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_CQ, ಖಾಲಿ,
                                      cq->cqe, ಖಾಲಿ);
    ಆದರೆ (!cmd_create_cq) {
        ಹೋಗು err_destroy_cq;
    }

    cmd_create_cq->dword[10] = (cq->common.ಮುಖವಾಡ << 16) | (q_idx >> 1);
    cmd_create_cq->dword[11] = 1 /* physically contiguous */;

    nvme_commit_sqe(&ctrl->admin_sq);

    ರಚನೆ nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    ಆದರೆ (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io cq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        ಹೋಗು err_destroy_cq;
    }

    ಹಿಂತಿರುಗಿ 0;

err_destroy_cq:
    nvme_destroy_cq(cq);
err:
    ಹಿಂತಿರುಗಿ -1;
}

/* Returns 0 on success. */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
nvme_create_io_sq(ರಚನೆ nvme_ctrl *ctrl, ರಚನೆ nvme_sq *sq, u16 q_idx, ರಚನೆ nvme_cq *cq)
{
    ಪೂರ್ಣಾಂಕ rc;
    ರಚನೆ nvme_sqe *cmd_create_sq;
    u32 ಉದ್ದಳತೆ = 1 + (ctrl->reg->cap & 0xffff);
    ಆದರೆ (ಉದ್ದಳತೆ > NVME_ಪುಟ_ಗಾತ್ರ / ಗಾತ್ರ(ರಚನೆ nvme_cqe))
        ಉದ್ದಳತೆ = NVME_ಪುಟ_ಗಾತ್ರ / ಗಾತ್ರ(ರಚನೆ nvme_cqe);

    rc = nvme_init_sq(ctrl, sq, q_idx, ಉದ್ದಳತೆ, cq);
    ಆದರೆ (rc) {
        ಹೋಗು err;
    }

    cmd_create_sq = nvme_get_ಮುಂದಿನ_sqe(&ctrl->admin_sq,
                                      NVME_SQE_OPC_ADMIN_CREATE_IO_SQ, ಖಾಲಿ,
                                      sq->sqe, ಖಾಲಿ);
    ಆದರೆ (!cmd_create_sq) {
        ಹೋಗು err_destroy_sq;
    }

    cmd_create_sq->dword[10] = (sq->common.ಮುಖವಾಡ << 16) | (q_idx >> 1);
    cmd_create_sq->dword[11] = (q_idx >> 1) << 16 | 1 /* contiguous */;
    dprintf(3, "sq %p create dword10 %08x dword11 %08x\n", sq,
            cmd_create_sq->dword[10], cmd_create_sq->dword[11]);

    nvme_commit_sqe(&ctrl->admin_sq);

    ರಚನೆ nvme_cqe cqe = nvme_wait(&ctrl->admin_sq);

    ಆದರೆ (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "create io sq failed: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);
        ಹೋಗು err_destroy_sq;
    }

    ಹಿಂತಿರುಗಿ 0;

err_destroy_sq:
    nvme_destroy_sq(sq);
err:
    ಹಿಂತಿರುಗಿ -1;
}

/* Reads count sectors into buf. The buffer cannot cross page boundaries. */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
nvme_io_xfer(ರಚನೆ nvme_namespace *ns, u64 lba, ಶೂನ್ಯ *prp1, ಶೂನ್ಯ *prp2,
             u16 ಎಣಿಕೆ_2, ಪೂರ್ಣಾಂಕ ಲಿಖ)
{
    ಆದರೆ (((u32)prp1 & 0x3) || ((u32)prp2 & 0x3)) {
        /* Buffer is misaligned */
        warn_internalerror();
        ಹಿಂತಿರುಗಿ -1;
    }

    ರಚನೆ nvme_sqe *io_read = nvme_get_ಮುಂದಿನ_sqe(&ns->ctrl->io_sq,
                                                 ಲಿಖ ? NVME_SQE_OPC_IO_WRITE
                                                       : NVME_SQE_OPC_IO_READ,
                                                 ಖಾಲಿ, prp1, prp2);
    io_read->nsid = ns->ns_id;
    io_read->dword[10] = (u32)lba;
    io_read->dword[11] = (u32)(lba >> 32);
    io_read->dword[12] = (1U << 31 /* limited retry */) | (ಎಣಿಕೆ_2 - 1);

    nvme_commit_sqe(&ns->ctrl->io_sq);

    ರಚನೆ nvme_cqe cqe = nvme_wait(&ns->ctrl->io_sq);

    ಆದರೆ (!nvme_is_cqe_success(&cqe)) {
        dprintf(2, "read io: %08x %08x %08x %08x\n",
                cqe.dword[0], cqe.dword[1], cqe.dword[2], cqe.dword[3]);

        ಹಿಂತಿರುಗಿ -1;
    }

    dprintf(5, "ns %u %s lba %llu+%u\n", ns->ns_id, ಲಿಖ ? "write" : "read",
            lba, ಎಣಿಕೆ_2);
    ಹಿಂತಿರುಗಿ ಎಣಿಕೆ_2;
}

// Transfer up to one page of data using the internal dma bounce buffer
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
nvme_bounce_xfer(ರಚನೆ nvme_namespace *ns, u64 lba, ಶೂನ್ಯ *buf, u16 ಎಣಿಕೆ_2,
                 ಪೂರ್ಣಾಂಕ ಲಿಖ)
{
    u16 ಸ್ಥಿರಾಂಕ max_ಖಂಡಗಳು = NVME_ಪುಟ_ಗಾತ್ರ / ns->ಖಂಡ_ಗಾತ್ರ;
    u16 ಖಂಡಗಳು = ಎಣಿಕೆ_2 < max_ಖಂಡಗಳು ? ಎಣಿಕೆ_2 : max_ಖಂಡಗಳು;

    ಆದರೆ (ಲಿಖ)
        ಸ್ಮರಣೆ_ನಕಲು(nvme_dma_ಬಫರ್, buf, ಖಂಡಗಳು * ns->ಖಂಡ_ಗಾತ್ರ);

    ಪೂರ್ಣಾಂಕ res = nvme_io_xfer(ns, lba, nvme_dma_ಬಫರ್, ಖಾಲಿ, ಖಂಡಗಳು, ಲಿಖ);

    ಆದರೆ (!ಲಿಖ && res >= 0)
        ಸ್ಮರಣೆ_ನಕಲು(buf, nvme_dma_ಬಫರ್, res * ns->ಖಂಡ_ಗಾತ್ರ);

    ಹಿಂತಿರುಗಿ res;
}

#ವ್ಯಾಖ್ಯಾನಿಸು NVME_MAX_PRPL_ENTRIES 15 /* Allows requests up to 64kb */

// Transfer data using page list (if applicable)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
nvme_prpl_xfer(ರಚನೆ nvme_namespace *ns, u64 lba, ಶೂನ್ಯ *buf, u16 ಎಣಿಕೆ_2,
               ಪೂರ್ಣಾಂಕ ಲಿಖ)
{
    u32 ತಳ = (ದೀರ್ಘ)buf;
    s32 ಗಾತ್ರ_2;

    ಆದರೆ (ಎಣಿಕೆ_2 > ns->max_req_ಗಾತ್ರ)
        ಎಣಿಕೆ_2 = ns->max_req_ಗಾತ್ರ;

    ಗಾತ್ರ_2 = ಎಣಿಕೆ_2 * ns->ಖಂಡ_ಗಾತ್ರ;
    /* Special case for transfers that fit into PRP1, but are unaligned */
    ಆದರೆ (((ಗಾತ್ರ_2 + (ತಳ & ~NVME_ಪುಟ_ಮುಖವಾಡ)) <= NVME_ಪುಟ_ಗಾತ್ರ))
        ಹೋಗು single;

    /* Every request has to be page aligned */
    ಆದರೆ (ತಳ & ~NVME_ಪುಟ_ಮುಖವಾಡ)
        ಹೋಗು bounce;

    /* Make sure a full block fits into the last chunk */
    ಆದರೆ (ಗಾತ್ರ_2 & (ns->ಖಂಡ_ಗಾತ್ರ - 1ULL))
        ಹೋಗು bounce;

    /* Build PRP list if we need to describe more than 2 pages */
    ಆದರೆ ((ns->ಖಂಡ_ಗಾತ್ರ * ಎಣಿಕೆ_2) > (NVME_ಪುಟ_ಗಾತ್ರ * 2)) {
        u32 prpl_len = 0;
        u64 *prpl = nvme_dma_ಬಫರ್;
        ಪೂರ್ಣಾಂಕ ಮೊದಲ_ಪುಟ = 1;
        ಗಾಗಿ (; ಗಾತ್ರ_2 > 0; ತಳ += NVME_ಪುಟ_ಗಾತ್ರ, ಗಾತ್ರ_2 -= NVME_ಪುಟ_ಗಾತ್ರ) {
            ಆದರೆ (ಮೊದಲ_ಪುಟ) {
                /* First page is special */
                ಮೊದಲ_ಪುಟ = 0;
                ಮುಂದುವರಿ;
            }
            ಆದರೆ (prpl_len >= NVME_MAX_PRPL_ENTRIES)
                ಹೋಗು bounce;
            prpl[prpl_len++] = ತಳ;
        }
        ಹಿಂತಿರುಗಿ nvme_io_xfer(ns, lba, buf, prpl, ಎಣಿಕೆ_2, ಲಿಖ);
    }

    /* Directly embed the 2nd page if we only need 2 pages */
    ಆದರೆ ((ns->ಖಂಡ_ಗಾತ್ರ * ಎಣಿಕೆ_2) > NVME_ಪುಟ_ಗಾತ್ರ)
        ಹಿಂತಿರುಗಿ nvme_io_xfer(ns, lba, buf, buf + NVME_ಪುಟ_ಗಾತ್ರ, ಎಣಿಕೆ_2, ಲಿಖ);

single:
    /* One page is enough, don't expose anything else */
    ಹಿಂತಿರುಗಿ nvme_io_xfer(ns, lba, buf, ಖಾಲಿ, ಎಣಿಕೆ_2, ಲಿಖ);

bounce:
    /* Use bounce buffer to make transfer */
    ಹಿಂತಿರುಗಿ nvme_bounce_xfer(ns, lba, buf, ಎಣಿಕೆ_2, ಲಿಖ);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
nvme_create_io_queues(ರಚನೆ nvme_ctrl *ctrl)
{
    ಆದರೆ (nvme_create_io_cq(ctrl, &ctrl->io_cq, 3))
        ಹೋಗು err;

    ಆದರೆ (nvme_create_io_sq(ctrl, &ctrl->io_sq, 2, &ctrl->io_cq))
        ಹೋಗು err_free_cq;

    ಹಿಂತಿರುಗಿ 0;

 err_free_cq:
    nvme_destroy_cq(&ctrl->io_cq);
 err:
    ಹಿಂತಿರುಗಿ -1;
}

/* Waits for CSTS.RDY to match rdy. Returns 0 on success. */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
nvme_wait_csts_rdy(ರಚನೆ nvme_ctrl *ctrl, ಚಿಹ್ನೆರಹಿತ rdy)
{
    u32 ಸ್ಥಿರಾಂಕ max_ಗಮ್ಯ = 500 /* ms */ * ((ctrl->reg->cap >> 24) & 0xFFU);
    u32 ಗಮ್ಯ = timer_calc(max_ಗಮ್ಯ);
    u32 csts;

    ಆಗ (rdy != ((csts = ctrl->reg->csts) & NVME_CSTS_RDY)) {
        yield();

        ಆದರೆ (csts & NVME_CSTS_FATAL) {
            dprintf(3, "NVMe fatal error during controller shutdown\n");
            ಹಿಂತಿರುಗಿ -1;
        }

        ಆದರೆ (timer_check(ಗಮ್ಯ)) {
            warn_timeout();
            ಹಿಂತಿರುಗಿ -1;
        }
    }

    ಹಿಂತಿರುಗಿ 0;
}

/* Returns 0 on success. */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
nvme_controller_enable(ರಚನೆ nvme_ctrl *ctrl)
{
    ಪೂರ್ಣಾಂಕ rc;

    pci_enable_busmaster(ctrl->pci);

    /* Turn the controller off. */
    ctrl->reg->cc = 0;
    ಆದರೆ (nvme_wait_csts_rdy(ctrl, 0)) {
        dprintf(2, "NVMe fatal error during controller shutdown\n");
        ಹಿಂತಿರುಗಿ -1;
    }

    ctrl->doorbell_stride = 4U << ((ctrl->reg->cap >> 32) & 0xF);

    rc = nvme_init_cq(ctrl, &ctrl->admin_cq, 1,
                      NVME_ಪುಟ_ಗಾತ್ರ / ಗಾತ್ರ(ರಚನೆ nvme_cqe));
    ಆದರೆ (rc) {
        ಹಿಂತಿರುಗಿ -1;
    }

    rc = nvme_init_sq(ctrl, &ctrl->admin_sq, 0,
                      NVME_ಪುಟ_ಗಾತ್ರ / ಗಾತ್ರ(ರಚನೆ nvme_sqe), &ctrl->admin_cq);
    ಆದರೆ (rc) {
        ಹೋಗು err_destroy_admin_cq;
    }

    ctrl->reg->aqa = ctrl->admin_cq.common.ಮುಖವಾಡ << 16
        | ctrl->admin_sq.common.ಮುಖವಾಡ;

    ctrl->reg->asq = (u32)ctrl->admin_sq.sqe;
    ctrl->reg->acq = (u32)ctrl->admin_cq.cqe;

    dprintf(3, "  admin submission queue: %p\n", ctrl->admin_sq.sqe);
    dprintf(3, "  admin completion queue: %p\n", ctrl->admin_cq.cqe);

    ctrl->reg->cc = NVME_CC_EN | (NVME_CQE_ಗಾತ್ರ_LOG << 20)
        | (NVME_SQE_ಗಾತ್ರ_LOG << 16 /* IOSQES */);

    ಆದರೆ (nvme_wait_csts_rdy(ctrl, 1)) {
        dprintf(2, "NVMe fatal error while enabling controller\n");
        ಹೋಗು err_destroy_admin_sq;
    }

    /* The admin queue is set up and the controller is ready. Let's figure out
       what namespaces we have. */

    ರಚನೆ nvme_identify_ctrl *identify = nvme_admin_identify_ctrl(ctrl);

    ಆದರೆ (!identify) {
        dprintf(2, "NVMe couldn't identify controller.\n");
        ಹೋಗು err_destroy_admin_sq;
    }

    dprintf(3, "NVMe has %u namespace%s.\n",
            identify->nn, (identify->nn == 1) ? "" : "s");

    ctrl->ns_ಎಣಿಕೆ = identify->nn;
    u8 mdts = identify->mdts;
    ಬಿಡುಗಡೆ(identify);

    ಆದರೆ ((ctrl->ns_ಎಣಿಕೆ == 0) || nvme_create_io_queues(ctrl)) {
        /* No point to continue, if the controller says it doesn't have
           namespaces or we couldn't create I/O queues. */
        ಹೋಗು err_destroy_admin_sq;
    }

    /* Populate namespace IDs */
    ಪೂರ್ಣಾಂಕ ns_idx;
    ಗಾಗಿ (ns_idx = 0; ns_idx < ctrl->ns_ಎಣಿಕೆ; ns_idx++) {
        nvme_probe_ns(ctrl, ns_idx, mdts);
    }

    dprintf(3, "NVMe initialization complete!\n");
    ಹಿಂತಿರುಗಿ 0;

 err_destroy_admin_sq:
    nvme_destroy_sq(&ctrl->admin_sq);
 err_destroy_admin_cq:
    nvme_destroy_cq(&ctrl->admin_cq);
    ಹಿಂತಿರುಗಿ -1;
}

/* Initialize an NVMe controller and detect its drives. */
ಸ್ಥಿರ ಶೂನ್ಯ
nvme_controller_ಸಿದ್ಧತೆ(ಶೂನ್ಯ *opaque)
{
    u8 skip_nonbootable = is_bootprio_strict();
    ರಚನೆ pci_device *pci = opaque;

    ಆದರೆ (skip_nonbootable && bootprio_ಕಂಡುಹಿಡಿ_pci_device(pci) < 0) {
        dprintf(1, "skipping init of a non-bootable NVMe at %pP\n",
                pci);
        ಹೋಗು err;
    }

    ರಚನೆ nvme_reg ಅಸ್ಥಿರ *reg = pci_enable_membar(pci, PCI_ತಳ_ವಿಳಾಸ_0);
    ಆದರೆ (!reg)
        ಹಿಂತಿರುಗಿ;

    u32 ಆವೃತ್ತಿ_2 = reg->vs;
    dprintf(3, "Found NVMe controller with version %u.%u.%u.\n",
            ಆವೃತ್ತಿ_2 >> 16, (ಆವೃತ್ತಿ_2 >> 8) & 0xFF, ಆವೃತ್ತಿ_2 & 0xFF);
    dprintf(3, "  Capabilities %016llx\n", reg->cap);

    ಆದರೆ (~reg->cap & NVME_CAP_CSS_NVME) {
        dprintf(3, "Controller doesn't speak NVMe command set. Skipping.\n");
        ಹೋಗು err;
    }

    ರಚನೆ nvme_ctrl *ctrl = malloc_ಮೇಲಿನ(ಗಾತ್ರ(*ctrl));
    ಆದರೆ (!ctrl) {
        warn_noalloc();
        ಹೋಗು err;
    }

    memset(ctrl, 0, ಗಾತ್ರ(*ctrl));

    ctrl->reg = reg;
    ctrl->pci = pci;

    ಆದರೆ (nvme_controller_enable(ctrl)) {
        ಹೋಗು err_free_ctrl;
    }

    ಹಿಂತಿರುಗಿ;

 err_free_ctrl:
    ಬಿಡುಗಡೆ(ctrl);
 err:
    dprintf(2, "Failed to enable NVMe controller.\n");
}

// Locate and init NVMe controllers
ಸ್ಥಿರ ಶೂನ್ಯ
nvme_ಪರಿಶೀಲನೆ(ಶೂನ್ಯ)
{
    // Scan PCI bus for NVMe adapters
    ರಚನೆ pci_device *pci;

    foreachpci(pci) {
        ಆದರೆ (pci->class != PCI_CLASS_STORAGE_NVME)
            ಮುಂದುವರಿ;
        ಆದರೆ (pci->prog_if != 2 /* as of NVM 1.0e */) {
            dprintf(3, "Found incompatble NVMe: prog-if=%02x\n", pci->prog_if);
            ಮುಂದುವರಿ;
        }

        ಸತತ_thread(nvme_controller_ಸಿದ್ಧತೆ, pci);
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
nvme_cmd_readwrite(ರಚನೆ nvme_namespace *ns, ರಚನೆ disk_op_s *op, ಪೂರ್ಣಾಂಕ ಲಿಖ)
{
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i = 0; i < op->ಎಣಿಕೆ_2;) {
        u16 ಖಂಡಗಳು_ಉಳಿದ = op->ಎಣಿಕೆ_2 - i;
        ಅಕ್ಷರ *op_buf = op->buf_fl + i * ns->ಖಂಡ_ಗಾತ್ರ;
        ಪೂರ್ಣಾಂಕ ಖಂಡಗಳು = nvme_prpl_xfer(ns, op->lba + i, op_buf,
                                    ಖಂಡಗಳು_ಉಳಿದ, ಲಿಖ);
        ಆದರೆ (ಖಂಡಗಳು < 0)
            ಹಿಂತಿರುಗಿ DISK_RET_EBADTRACK;
        i += ಖಂಡಗಳು;
    }

    ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;
}

ಪೂರ್ಣಾಂಕ
nvme_process_op(ರಚನೆ disk_op_s *op)
{
    ಆದರೆ (!CONFIG_NVME)
        ಹಿಂತಿರುಗಿ DISK_RET_SUCCESS;

    ರಚನೆ nvme_namespace *ns = container_ದ(op->drive_fl, ರಚನೆ nvme_namespace,
                                             drive);

    ಆಯ್ಕೆ (op->command) {
    ಸ್ಥಿತಿ CMD_READ:
    ಸ್ಥಿತಿ CMD_WRITE:
        ಹಿಂತಿರುಗಿ nvme_cmd_readwrite(ns, op, op->command == CMD_WRITE);
    ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ default_process_op(op);
    }
}

ಶೂನ್ಯ
nvme_ಸಿದ್ಧತೆ(ಶೂನ್ಯ)
{
    ASSERT32FLAT();
    ಆದರೆ (!CONFIG_NVME)
        ಹಿಂತಿರುಗಿ;

    dprintf(3, "init nvme\n");
    nvme_ಪರಿಶೀಲನೆ();
}

/* EOF */

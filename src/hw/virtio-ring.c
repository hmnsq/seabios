/* virtio-pci.c - virtio ring management
 *
 * (c) Copyright 2008 Bull S.A.S.
 *
 *  Author: Laurent Vivier <Laurent.Vivier@bull.net>
 *
 *  some parts from Linux Virtio Ring
 *
 *  Copyright Rusty Russell IBM Corporation 2007
 *
 *  Adopted for Seabios: Gleb Natapov <gleb@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPLv3
 * See the COPYING file in the top-level directory.
 *
 *
 */

#include "output.h" // panic
#include "virtio-ring.h"
#include "virtio-pci.h"

#define BUG() do {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } докато (0)
#define BUG_ON(condition) do { ако (condition) BUG(); } докато (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

цяло vring_more_used(struct vring_virtqueue *vq)
{
    struct vring_used *used = vq->vring.used;
    цяло more = vq->last_used_idx != used->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    върни more;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

void vring_detach(struct vring_virtqueue *vq, unsigned цяло head)
{
    struct vring *vr = &vq->vring;
    struct vring_desc *desc = GET_LOWFLAT(vr->desc);
    unsigned цяло i;

    /* find end of given descriptor */

    i = head;
    докато (desc[i].flags & VRING_DESC_F_NEXT)
        i = desc[i].next;

    /* link it with free list and point to it */

    desc[i].next = vq->free_head;
    vq->free_head = head;
}

/*
 * vring_get_buf
 *
 * get a buffer from the used list
 *
 */

цяло vring_get_buf(struct vring_virtqueue *vq, unsigned цяло *len)
{
    struct vring *vr = &vq->vring;
    struct vring_used_elem *elem;
    struct vring_used *used = vq->vring.used;
    u32 id;
    цяло ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &used->ring[vq->last_used_idx % vr->num];
    id = elem->id;
    ако (len != NULL)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->last_used_idx = vq->last_used_idx + 1;

    върни ret;
}

void vring_add_buf(struct vring_virtqueue *vq,
                   struct vring_list list[],
                   unsigned цяло out, unsigned цяло in,
                   цяло index, цяло num_added)
{
    struct vring *vr = &vq->vring;
    цяло i, av, head, prev;
    struct vring_desc *desc = vr->desc;
    struct vring_avail *avail = vr->avail;

    BUG_ON(out + in == 0);

    prev = 0;
    head = vq->free_head;
    за (i = head; out; i = desc[i].next, out--) {
        desc[i].flags = VRING_DESC_F_NEXT;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    за ( ; in; i = desc[i].next, in--) {
        desc[i].flags = VRING_DESC_F_NEXT|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    desc[prev].flags = desc[prev].flags & ~VRING_DESC_F_NEXT;

    vq->free_head = i;

    vq->vdata[head] = index;

    av = (avail->idx + num_added) % vr->num;
    avail->ring[av] = head;
}

void vring_kick(struct vp_device *vp, struct vring_virtqueue *vq, цяло num_added)
{
    struct vring *vr = &vq->vring;
    struct vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + num_added;

    vp_notify(vp, vq);
}

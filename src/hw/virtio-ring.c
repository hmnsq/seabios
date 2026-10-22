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

#ضمن "output.h" // panic
#ضمن "virtio-ring.h"
#ضمن "virtio-pci.h"

#define BUG() افعل {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } طالما (0)
#define BUG_ON(condition) افعل { إذا (condition) BUG(); } طالما (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

صحيح vring_more_used(بنية vring_virtqueue *vq)
{
    بنية vring_used *used = vq->vring.used;
    صحيح more = vq->last_used_idx != used->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    أرجع more;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

فراغ vring_detach(بنية vring_virtqueue *vq, unsigned صحيح head)
{
    بنية vring *vr = &vq->vring;
    بنية vring_desc *desc = GET_LOWFLAT(vr->desc);
    unsigned صحيح i;

    /* find end of given descriptor */

    i = head;
    طالما (desc[i].flags & VRING_DESC_F_NEXT)
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

صحيح vring_get_buf(بنية vring_virtqueue *vq, unsigned صحيح *len)
{
    بنية vring *vr = &vq->vring;
    بنية vring_used_elem *elem;
    بنية vring_used *used = vq->vring.used;
    u32 id;
    صحيح ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &used->ring[vq->last_used_idx % vr->عدد];
    id = elem->id;
    إذا (len != عدم)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->last_used_idx = vq->last_used_idx + 1;

    أرجع ret;
}

فراغ vring_add_buf(بنية vring_virtqueue *vq,
                   بنية vring_list list[],
                   unsigned صحيح out, unsigned صحيح in,
                   صحيح index, صحيح عدد_added)
{
    بنية vring *vr = &vq->vring;
    صحيح i, av, head, prev;
    بنية vring_desc *desc = vr->desc;
    بنية vring_avail *avail = vr->avail;

    BUG_ON(out + in == 0);

    prev = 0;
    head = vq->free_head;
    لكل (i = head; out; i = desc[i].next, out--) {
        desc[i].flags = VRING_DESC_F_NEXT;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    لكل ( ; in; i = desc[i].next, in--) {
        desc[i].flags = VRING_DESC_F_NEXT|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    desc[prev].flags = desc[prev].flags & ~VRING_DESC_F_NEXT;

    vq->free_head = i;

    vq->vdata[head] = index;

    av = (avail->idx + عدد_added) % vr->عدد;
    avail->ring[av] = head;
}

فراغ vring_kick(بنية vp_device *vp, بنية vring_virtqueue *vq, صحيح عدد_added)
{
    بنية vring *vr = &vq->vring;
    بنية vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + عدد_added;

    vp_notify(vp, vq);
}

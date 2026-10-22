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
        } ขณะ (0)
#define BUG_ON(condition) do { ถ้า (condition) BUG(); } ขณะ (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

จำนวนเต็ม vring_more_used(sa_nracanaa vring_virtqueue *vq)
{
    sa_nracanaa vring_used *used = vq->vring.used;
    จำนวนเต็ม more = vq->last_used_idx != used->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    คืน more;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

void vring_detach(sa_nracanaa vring_virtqueue *vq, unsigned จำนวนเต็ม head)
{
    sa_nracanaa vring *vr = &vq->vring;
    sa_nracanaa vring_desc *desc = GET_LOWFLAT(vr->desc);
    unsigned จำนวนเต็ม i;

    /* find end of given descriptor */

    i = head;
    ขณะ (desc[i].flags & VRING_DESC_F_NEXT)
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

จำนวนเต็ม vring_get_buf(sa_nracanaa vring_virtqueue *vq, unsigned จำนวนเต็ม *len)
{
    sa_nracanaa vring *vr = &vq->vring;
    sa_nracanaa vring_used_elem *elem;
    sa_nracanaa vring_used *used = vq->vring.used;
    u32 id;
    จำนวนเต็ม ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &used->ring[vq->last_used_idx % vr->num];
    id = elem->id;
    ถ้า (len != NULL)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->last_used_idx = vq->last_used_idx + 1;

    คืน ret;
}

void vring_add_buf(sa_nracanaa vring_virtqueue *vq,
                   sa_nracanaa vring_list list[],
                   unsigned จำนวนเต็ม out, unsigned จำนวนเต็ม in,
                   จำนวนเต็ม index, จำนวนเต็ม num_added)
{
    sa_nracanaa vring *vr = &vq->vring;
    จำนวนเต็ม i, av, head, prev;
    sa_nracanaa vring_desc *desc = vr->desc;
    sa_nracanaa vring_avail *avail = vr->avail;

    BUG_ON(out + in == 0);

    prev = 0;
    head = vq->free_head;
    สำหรับ (i = head; out; i = desc[i].next, out--) {
        desc[i].flags = VRING_DESC_F_NEXT;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    สำหรับ ( ; in; i = desc[i].next, in--) {
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

void vring_kick(sa_nracanaa vp_device *vp, sa_nracanaa vring_virtqueue *vq, จำนวนเต็ม num_added)
{
    sa_nracanaa vring *vr = &vq->vring;
    sa_nracanaa vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + num_added;

    vp_notify(vp, vq);
}

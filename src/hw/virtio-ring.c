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
        } နေစဉ် (0)
#define BUG_ON(condition) do { အကယ်၍ (condition) BUG(); } နေစဉ် (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

ကိန်းပြည့် vring_more_used(sa_nracanaa vring_virtqueue *vq)
{
    sa_nracanaa vring_used *used = vq->vring.used;
    ကိန်းပြည့် more = vq->last_used_idx != used->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    ပြန်ပေး more;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

void vring_detach(sa_nracanaa vring_virtqueue *vq, unsigned ကိန်းပြည့် head)
{
    sa_nracanaa vring *vr = &vq->vring;
    sa_nracanaa vring_desc *desc = GET_LOWFLAT(vr->desc);
    unsigned ကိန်းပြည့် i;

    /* find end of given descriptor */

    i = head;
    နေစဉ် (desc[i].flags & VRING_DESC_F_NEXT)
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

ကိန်းပြည့် vring_get_buf(sa_nracanaa vring_virtqueue *vq, unsigned ကိန်းပြည့် *len)
{
    sa_nracanaa vring *vr = &vq->vring;
    sa_nracanaa vring_used_elem *elem;
    sa_nracanaa vring_used *used = vq->vring.used;
    u32 id;
    ကိန်းပြည့် ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &used->ring[vq->last_used_idx % vr->num];
    id = elem->id;
    အကယ်၍ (len != NULL)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->last_used_idx = vq->last_used_idx + 1;

    ပြန်ပေး ret;
}

void vring_add_buf(sa_nracanaa vring_virtqueue *vq,
                   sa_nracanaa vring_list list[],
                   unsigned ကိန်းပြည့် out, unsigned ကိန်းပြည့် in,
                   ကိန်းပြည့် index, ကိန်းပြည့် num_added)
{
    sa_nracanaa vring *vr = &vq->vring;
    ကိန်းပြည့် i, av, head, prev;
    sa_nracanaa vring_desc *desc = vr->desc;
    sa_nracanaa vring_avail *avail = vr->avail;

    BUG_ON(out + in == 0);

    prev = 0;
    head = vq->free_head;
    အတွက် (i = head; out; i = desc[i].next, out--) {
        desc[i].flags = VRING_DESC_F_NEXT;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    အတွက် ( ; in; i = desc[i].next, in--) {
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

void vring_kick(sa_nracanaa vp_device *vp, sa_nracanaa vring_virtqueue *vq, ကိန်းပြည့် num_added)
{
    sa_nracanaa vring *vr = &vq->vring;
    sa_nracanaa vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + num_added;

    vp_notify(vp, vq);
}

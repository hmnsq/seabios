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

#অন্তর্ভাৱ "output.h" // panic
#অন্তর্ভাৱ "virtio-ring.h"
#অন্তর্ভাৱ "virtio-pci.h"

#define BUG() কৰা {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } যেতিয়ালৈ (0)
#define BUG_ON(condition) কৰা { যদি (condition) BUG(); } যেতিয়ালৈ (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

পূর্ণাঙ্ক vring_more_used(সংরচনা vring_virtqueue *vq)
{
    সংরচনা vring_used *used = vq->vring.used;
    পূর্ণাঙ্ক more = vq->last_used_idx != used->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    ঘূৰাই more;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

শূন্য vring_detach(সংরচনা vring_virtqueue *vq, unsigned পূর্ণাঙ্ক head)
{
    সংরচনা vring *vr = &vq->vring;
    সংরচনা vring_desc *desc = GET_LOWFLAT(vr->desc);
    unsigned পূর্ণাঙ্ক i;

    /* find end of given descriptor */

    i = head;
    যেতিয়ালৈ (desc[i].flags & VRING_DESC_F_NEXT)
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

পূর্ণাঙ্ক vring_get_buf(সংরচনা vring_virtqueue *vq, unsigned পূর্ণাঙ্ক *len)
{
    সংরচনা vring *vr = &vq->vring;
    সংরচনা vring_used_elem *elem;
    সংরচনা vring_used *used = vq->vring.used;
    u32 id;
    পূর্ণাঙ্ক ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &used->ring[vq->last_used_idx % vr->অঙ্ক];
    id = elem->id;
    যদি (len != রিক্ত)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->last_used_idx = vq->last_used_idx + 1;

    ঘূৰাই ret;
}

শূন্য vring_add_buf(সংরচনা vring_virtqueue *vq,
                   সংরচনা vring_list list[],
                   unsigned পূর্ণাঙ্ক out, unsigned পূর্ণাঙ্ক in,
                   পূর্ণাঙ্ক index, পূর্ণাঙ্ক অঙ্ক_added)
{
    সংরচনা vring *vr = &vq->vring;
    পূর্ণাঙ্ক i, av, head, prev;
    সংরচনা vring_desc *desc = vr->desc;
    সংরচনা vring_avail *avail = vr->avail;

    BUG_ON(out + in == 0);

    prev = 0;
    head = vq->free_head;
    বাবে (i = head; out; i = desc[i].next, out--) {
        desc[i].flags = VRING_DESC_F_NEXT;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    বাবে ( ; in; i = desc[i].next, in--) {
        desc[i].flags = VRING_DESC_F_NEXT|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    desc[prev].flags = desc[prev].flags & ~VRING_DESC_F_NEXT;

    vq->free_head = i;

    vq->vdata[head] = index;

    av = (avail->idx + অঙ্ক_added) % vr->অঙ্ক;
    avail->ring[av] = head;
}

শূন্য vring_kick(সংরচনা vp_device *vp, সংরচনা vring_virtqueue *vq, পূর্ণাঙ্ক অঙ্ক_added)
{
    সংরচনা vring *vr = &vq->vring;
    সংরচনা vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + অঙ্ক_added;

    vp_notify(vp, vq);
}

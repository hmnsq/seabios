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

#شامل "output.h" // panic
#شامل "virtio-ring.h"
#شامل "virtio-pci.h"

#define BUG() انجامبده {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } وقتی (0)
#define BUG_ON(condition) انجامبده { اگر (condition) BUG(); } وقتی (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

صحیح vring_more_used(ساختار vring_virtqueue *vq)
{
    ساختار vring_used *used = vq->vring.used;
    صحیح more = vq->last_used_idx != used->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    بازگشت more;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

تهی vring_detach(ساختار vring_virtqueue *vq, unsigned صحیح head)
{
    ساختار vring *vr = &vq->vring;
    ساختار vring_desc *desc = GET_LOWFLAT(vr->desc);
    unsigned صحیح i;

    /* find end of given descriptor */

    i = head;
    وقتی (desc[i].flags & VRING_DESC_F_NEXT)
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

صحیح vring_get_buf(ساختار vring_virtqueue *vq, unsigned صحیح *len)
{
    ساختار vring *vr = &vq->vring;
    ساختار vring_used_elem *elem;
    ساختار vring_used *used = vq->vring.used;
    u32 id;
    صحیح ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &used->ring[vq->last_used_idx % vr->رقم];
    id = elem->id;
    اگر (len != پوچ)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->last_used_idx = vq->last_used_idx + 1;

    بازگشت ret;
}

تهی vring_add_buf(ساختار vring_virtqueue *vq,
                   ساختار vring_list list[],
                   unsigned صحیح out, unsigned صحیح in,
                   صحیح index, صحیح رقم_added)
{
    ساختار vring *vr = &vq->vring;
    صحیح i, av, head, prev;
    ساختار vring_desc *desc = vr->desc;
    ساختار vring_avail *avail = vr->avail;

    BUG_ON(out + in == 0);

    prev = 0;
    head = vq->free_head;
    برای (i = head; out; i = desc[i].next, out--) {
        desc[i].flags = VRING_DESC_F_NEXT;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    برای ( ; in; i = desc[i].next, in--) {
        desc[i].flags = VRING_DESC_F_NEXT|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    desc[prev].flags = desc[prev].flags & ~VRING_DESC_F_NEXT;

    vq->free_head = i;

    vq->vdata[head] = index;

    av = (avail->idx + رقم_added) % vr->رقم;
    avail->ring[av] = head;
}

تهی vring_kick(ساختار vp_device *vp, ساختار vring_virtqueue *vq, صحیح رقم_added)
{
    ساختار vring *vr = &vq->vring;
    ساختار vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + رقم_added;

    vp_notify(vp, vq);
}

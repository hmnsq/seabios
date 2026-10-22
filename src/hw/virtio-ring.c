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

#அந்தர்பாவ "output.h" // panic
#அந்தர்பாவ "virtio-ring.h"
#அந்தர்பாவ "virtio-pci.h"

#define BUG() செய் {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } போது (0)
#define BUG_ON(condition) செய் { எனில் (condition) BUG(); } போது (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

முழு vring_more_used(கட்டமைப்பு vring_virtqueue *vq)
{
    கட்டமைப்பு vring_used *used = vq->vring.used;
    முழு more = vq->last_used_idx != used->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    திரும்பு more;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

வெறுமை vring_detach(கட்டமைப்பு vring_virtqueue *vq, unsigned முழு head)
{
    கட்டமைப்பு vring *vr = &vq->vring;
    கட்டமைப்பு vring_desc *desc = GET_LOWFLAT(vr->desc);
    unsigned முழு i;

    /* find end of given descriptor */

    i = head;
    போது (desc[i].flags & VRING_DESC_F_NEXT)
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

முழு vring_get_buf(கட்டமைப்பு vring_virtqueue *vq, unsigned முழு *len)
{
    கட்டமைப்பு vring *vr = &vq->vring;
    கட்டமைப்பு vring_used_elem *elem;
    கட்டமைப்பு vring_used *used = vq->vring.used;
    u32 id;
    முழு ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &used->ring[vq->last_used_idx % vr->இலக்கம்];
    id = elem->id;
    எனில் (len != வெற்று)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->last_used_idx = vq->last_used_idx + 1;

    திரும்பு ret;
}

வெறுமை vring_add_buf(கட்டமைப்பு vring_virtqueue *vq,
                   கட்டமைப்பு vring_list list[],
                   unsigned முழு out, unsigned முழு in,
                   முழு index, முழு இலக்கம்_added)
{
    கட்டமைப்பு vring *vr = &vq->vring;
    முழு i, av, head, prev;
    கட்டமைப்பு vring_desc *desc = vr->desc;
    கட்டமைப்பு vring_avail *avail = vr->avail;

    BUG_ON(out + in == 0);

    prev = 0;
    head = vq->free_head;
    க்கு (i = head; out; i = desc[i].next, out--) {
        desc[i].flags = VRING_DESC_F_NEXT;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    க்கு ( ; in; i = desc[i].next, in--) {
        desc[i].flags = VRING_DESC_F_NEXT|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    desc[prev].flags = desc[prev].flags & ~VRING_DESC_F_NEXT;

    vq->free_head = i;

    vq->vdata[head] = index;

    av = (avail->idx + இலக்கம்_added) % vr->இலக்கம்;
    avail->ring[av] = head;
}

வெறுமை vring_kick(கட்டமைப்பு vp_device *vp, கட்டமைப்பு vring_virtqueue *vq, முழு இலக்கம்_added)
{
    கட்டமைப்பு vring *vr = &vq->vring;
    கட்டமைப்பு vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + இலக்கம்_added;

    vp_notify(vp, vq);
}

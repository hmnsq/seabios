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

#અન્તર્ભાવ "output.h" // panic
#અન્તર્ભાવ "virtio-ring.h"
#અન્તર્ભાવ "virtio-pci.h"

#define BUG() કરો {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } જ્યારે (0)
#define BUG_ON(condition) કરો { જો (condition) BUG(); } જ્યારે (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

પૂર્ણાંક vring_more_used(સંરચના vring_virtqueue *vq)
{
    સંરચના vring_used *used = vq->vring.used;
    પૂર્ણાંક more = vq->last_used_idx != used->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    પાછા more;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

શૂન્ય vring_detach(સંરચના vring_virtqueue *vq, unsigned પૂર્ણાંક head)
{
    સંરચના vring *vr = &vq->vring;
    સંરચના vring_desc *desc = GET_LOWFLAT(vr->desc);
    unsigned પૂર્ણાંક i;

    /* find end of given descriptor */

    i = head;
    જ્યારે (desc[i].flags & VRING_DESC_F_NEXT)
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

પૂર્ણાંક vring_get_buf(સંરચના vring_virtqueue *vq, unsigned પૂર્ણાંક *len)
{
    સંરચના vring *vr = &vq->vring;
    સંરચના vring_used_elem *elem;
    સંરચના vring_used *used = vq->vring.used;
    u32 id;
    પૂર્ણાંક ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &used->ring[vq->last_used_idx % vr->અંક];
    id = elem->id;
    જો (len != ખાલી)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->last_used_idx = vq->last_used_idx + 1;

    પાછા ret;
}

શૂન્ય vring_add_buf(સંરચના vring_virtqueue *vq,
                   સંરચના vring_list list[],
                   unsigned પૂર્ણાંક out, unsigned પૂર્ણાંક in,
                   પૂર્ણાંક index, પૂર્ણાંક અંક_added)
{
    સંરચના vring *vr = &vq->vring;
    પૂર્ણાંક i, av, head, prev;
    સંરચના vring_desc *desc = vr->desc;
    સંરચના vring_avail *avail = vr->avail;

    BUG_ON(out + in == 0);

    prev = 0;
    head = vq->free_head;
    માટે (i = head; out; i = desc[i].next, out--) {
        desc[i].flags = VRING_DESC_F_NEXT;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    માટે ( ; in; i = desc[i].next, in--) {
        desc[i].flags = VRING_DESC_F_NEXT|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    desc[prev].flags = desc[prev].flags & ~VRING_DESC_F_NEXT;

    vq->free_head = i;

    vq->vdata[head] = index;

    av = (avail->idx + અંક_added) % vr->અંક;
    avail->ring[av] = head;
}

શૂન્ય vring_kick(સંરચના vp_device *vp, સંરચના vring_virtqueue *vq, પૂર્ણાંક અંક_added)
{
    સંરચના vring *vr = &vq->vring;
    સંરચના vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + અંક_added;

    vp_notify(vp, vq);
}

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

#അന്തര്ഭാവ "output.h" // panic
#അന്തര്ഭാവ "virtio-ring.h"
#അന്തര്ഭാവ "virtio-pci.h"

#define BUG() ചെയ്യുക {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } ആകുമ്പോൾ (0)
#define BUG_ON(condition) ചെയ്യുക { എങ്കിൽ (condition) BUG(); } ആകുമ്പോൾ (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

പൂർണ്ണസംഖ്യ vring_more_used(ഘടന vring_virtqueue *vq)
{
    ഘടന vring_used *used = vq->vring.used;
    പൂർണ്ണസംഖ്യ more = vq->last_used_idx != used->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    തിരികെ more;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

ശൂന്യം vring_detach(ഘടന vring_virtqueue *vq, unsigned പൂർണ്ണസംഖ്യ head)
{
    ഘടന vring *vr = &vq->vring;
    ഘടന vring_desc *desc = GET_LOWFLAT(vr->desc);
    unsigned പൂർണ്ണസംഖ്യ i;

    /* find end of given descriptor */

    i = head;
    ആകുമ്പോൾ (desc[i].flags & VRING_DESC_F_NEXT)
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

പൂർണ്ണസംഖ്യ vring_get_buf(ഘടന vring_virtqueue *vq, unsigned പൂർണ്ണസംഖ്യ *len)
{
    ഘടന vring *vr = &vq->vring;
    ഘടന vring_used_elem *elem;
    ഘടന vring_used *used = vq->vring.used;
    u32 id;
    പൂർണ്ണസംഖ്യ ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &used->ring[vq->last_used_idx % vr->അക്കം];
    id = elem->id;
    എങ്കിൽ (len != ശൂന്യ)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->last_used_idx = vq->last_used_idx + 1;

    തിരികെ ret;
}

ശൂന്യം vring_add_buf(ഘടന vring_virtqueue *vq,
                   ഘടന vring_list list[],
                   unsigned പൂർണ്ണസംഖ്യ out, unsigned പൂർണ്ണസംഖ്യ in,
                   പൂർണ്ണസംഖ്യ index, പൂർണ്ണസംഖ്യ അക്കം_added)
{
    ഘടന vring *vr = &vq->vring;
    പൂർണ്ണസംഖ്യ i, av, head, prev;
    ഘടന vring_desc *desc = vr->desc;
    ഘടന vring_avail *avail = vr->avail;

    BUG_ON(out + in == 0);

    prev = 0;
    head = vq->free_head;
    വേണ്ടി (i = head; out; i = desc[i].next, out--) {
        desc[i].flags = VRING_DESC_F_NEXT;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    വേണ്ടി ( ; in; i = desc[i].next, in--) {
        desc[i].flags = VRING_DESC_F_NEXT|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    desc[prev].flags = desc[prev].flags & ~VRING_DESC_F_NEXT;

    vq->free_head = i;

    vq->vdata[head] = index;

    av = (avail->idx + അക്കം_added) % vr->അക്കം;
    avail->ring[av] = head;
}

ശൂന്യം vring_kick(ഘടന vp_device *vp, ഘടന vring_virtqueue *vq, പൂർണ്ണസംഖ്യ അക്കം_added)
{
    ഘടന vring *vr = &vq->vring;
    ഘടന vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + അക്കം_added;

    vp_notify(vp, vq);
}

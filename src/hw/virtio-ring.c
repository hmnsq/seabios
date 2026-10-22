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

#অন্তর্ভাব "output.h" // panic
#অন্তর্ভাব "virtio-ring.h"
#অন্তর্ভাব "virtio-pci.h"

#define BUG() করো {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } যখন (0)
#define BUG_ON(condition) করো { যদি (condition) BUG(); } যখন (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

পূর্ণসংখ্যা vring_more_used(কাঠামো vring_virtqueue *vq)
{
    কাঠামো vring_used *used = vq->vring.used;
    পূর্ণসংখ্যা more = vq->last_used_idx != used->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    ফেরত more;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

শূন্য vring_detach(কাঠামো vring_virtqueue *vq, unsigned পূর্ণসংখ্যা শীর্ষ)
{
    কাঠামো vring *vr = &vq->vring;
    কাঠামো vring_desc *desc = GET_LOWFLAT(vr->desc);
    unsigned পূর্ণসংখ্যা i;

    /* find end of given descriptor */

    i = শীর্ষ;
    যখন (desc[i].flags & VRING_DESC_F_পরবর্তী)
        i = desc[i].পরবর্তী;

    /* link it with free list and point to it */

    desc[i].পরবর্তী = vq->free_শীর্ষ;
    vq->free_শীর্ষ = শীর্ষ;
}

/*
 * vring_get_buf
 *
 * get a buffer from the used list
 *
 */

পূর্ণসংখ্যা vring_get_buf(কাঠামো vring_virtqueue *vq, unsigned পূর্ণসংখ্যা *len)
{
    কাঠামো vring *vr = &vq->vring;
    কাঠামো vring_used_elem *elem;
    কাঠামো vring_used *used = vq->vring.used;
    u32 id;
    পূর্ণসংখ্যা ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &used->ring[vq->last_used_idx % vr->অঙ্ক];
    id = elem->id;
    যদি (len != ফাঁকা)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->last_used_idx = vq->last_used_idx + 1;

    ফেরত ret;
}

শূন্য vring_add_buf(কাঠামো vring_virtqueue *vq,
                   কাঠামো vring_list list[],
                   unsigned পূর্ণসংখ্যা out, unsigned পূর্ণসংখ্যা in,
                   পূর্ণসংখ্যা সূচক, পূর্ণসংখ্যা অঙ্ক_added)
{
    কাঠামো vring *vr = &vq->vring;
    পূর্ণসংখ্যা i, av, শীর্ষ, পূর্ব;
    কাঠামো vring_desc *desc = vr->desc;
    কাঠামো vring_avail *avail = vr->avail;

    BUG_ON(out + in == 0);

    পূর্ব = 0;
    শীর্ষ = vq->free_শীর্ষ;
    জন্য (i = শীর্ষ; out; i = desc[i].পরবর্তী, out--) {
        desc[i].flags = VRING_DESC_F_পরবর্তী;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->দৈর্ঘ্য;
        পূর্ব = i;
        list++;
    }
    জন্য ( ; in; i = desc[i].পরবর্তী, in--) {
        desc[i].flags = VRING_DESC_F_পরবর্তী|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->দৈর্ঘ্য;
        পূর্ব = i;
        list++;
    }
    desc[পূর্ব].flags = desc[পূর্ব].flags & ~VRING_DESC_F_পরবর্তী;

    vq->free_শীর্ষ = i;

    vq->vdata[শীর্ষ] = সূচক;

    av = (avail->idx + অঙ্ক_added) % vr->অঙ্ক;
    avail->ring[av] = শীর্ষ;
}

শূন্য vring_kick(কাঠামো vp_device *vp, কাঠামো vring_virtqueue *vq, পূর্ণসংখ্যা অঙ্ক_added)
{
    কাঠামো vring *vr = &vq->vring;
    কাঠামো vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + অঙ্ক_added;

    vp_notify(vp, vq);
}

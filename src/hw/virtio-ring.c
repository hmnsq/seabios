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

#تعریف_کړه BUG() افعل {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } طالما (0)
#تعریف_کړه BUG_ON(condition) افعل { إذا (condition) BUG(); } طالما (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

صحيح vring_نور_کارول_شوی(بنية vring_virtqueue *vq)
{
    بنية vring_کارول_شوی *کارول_شوی = vq->vring.کارول_شوی;
    صحيح نور = vq->وروستی_کارول_شوی_idx != کارول_شوی->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    أرجع نور;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

فراغ vring_detach(بنية vring_virtqueue *vq, بینښه صحيح سر)
{
    بنية vring *vr = &vq->vring;
    بنية vring_desc *desc = GET_LOWFLAT(vr->desc);
    بینښه صحيح i;

    /* find end of given descriptor */

    i = سر;
    طالما (desc[i].flags & VRING_DESC_F_بل)
        i = desc[i].بل;

    /* link it with free list and point to it */

    desc[i].بل = vq->free_سر;
    vq->free_سر = سر;
}

/*
 * vring_get_buf
 *
 * get a buffer from the used list
 *
 */

صحيح vring_get_buf(بنية vring_virtqueue *vq, بینښه صحيح *len)
{
    بنية vring *vr = &vq->vring;
    بنية vring_کارول_شوی_elem *elem;
    بنية vring_کارول_شوی *کارول_شوی = vq->vring.کارول_شوی;
    u32 id;
    صحيح ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &کارول_شوی->ring[vq->وروستی_کارول_شوی_idx % vr->عدد];
    id = elem->id;
    إذا (len != عدم)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->وروستی_کارول_شوی_idx = vq->وروستی_کارول_شوی_idx + 1;

    أرجع ret;
}

فراغ vring_جمع_کړه_buf(بنية vring_virtqueue *vq,
                   بنية vring_list list[],
                   بینښه صحيح بهر, بینښه صحيح دننه,
                   صحيح شاخص, صحيح عدد_added)
{
    بنية vring *vr = &vq->vring;
    صحيح i, av, سر, prev;
    بنية vring_desc *desc = vr->desc;
    بنية vring_avail *avail = vr->avail;

    BUG_ON(بهر + دننه == 0);

    prev = 0;
    سر = vq->free_سر;
    لكل (i = سر; بهر; i = desc[i].بل, بهر--) {
        desc[i].flags = VRING_DESC_F_بل;
        desc[i].addr = (u64)virt_منزل_phys(list->addr);
        desc[i].len = list->اوږدتیا;
        prev = i;
        list++;
    }
    لكل ( ; دننه; i = desc[i].بل, دننه--) {
        desc[i].flags = VRING_DESC_F_بل|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_منزل_phys(list->addr);
        desc[i].len = list->اوږدتیا;
        prev = i;
        list++;
    }
    desc[prev].flags = desc[prev].flags & ~VRING_DESC_F_بل;

    vq->free_سر = i;

    vq->vdata[سر] = شاخص;

    av = (avail->idx + عدد_added) % vr->عدد;
    avail->ring[av] = سر;
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

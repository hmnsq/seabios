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

#تعریف_کن BUG() افعل {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } طالما (0)
#تعریف_کن BUG_ON(condition) افعل { إذا (condition) BUG(); } طالما (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

صحيح vring_بیشتر_استفاده_شده(بنية vring_virtqueue *vq)
{
    بنية vring_استفاده_شده *استفاده_شده = vq->vring.استفاده_شده;
    صحيح بیشتر = vq->آخرین_استفاده_شده_idx != استفاده_شده->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    أرجع بیشتر;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

فراغ vring_detach(بنية vring_virtqueue *vq, بیعلامت صحيح سر)
{
    بنية vring *vr = &vq->vring;
    بنية vring_desc *desc = GET_LOWFLAT(vr->desc);
    بیعلامت صحيح i;

    /* find end of given descriptor */

    i = سر;
    طالما (desc[i].flags & VRING_DESC_F_بعدی)
        i = desc[i].بعدی;

    /* link it with free list and point to it */

    desc[i].بعدی = vq->free_سر;
    vq->free_سر = سر;
}

/*
 * vring_get_buf
 *
 * get a buffer from the used list
 *
 */

صحيح vring_get_buf(بنية vring_virtqueue *vq, بیعلامت صحيح *len)
{
    بنية vring *vr = &vq->vring;
    بنية vring_استفاده_شده_elem *elem;
    بنية vring_استفاده_شده *استفاده_شده = vq->vring.استفاده_شده;
    u32 id;
    صحيح ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &استفاده_شده->ring[vq->آخرین_استفاده_شده_idx % vr->رقم];
    id = elem->id;
    إذا (len != عدم)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->آخرین_استفاده_شده_idx = vq->آخرین_استفاده_شده_idx + 1;

    أرجع ret;
}

فراغ vring_جمع_کن_buf(بنية vring_virtqueue *vq,
                   بنية vring_list list[],
                   بیعلامت صحيح بیرون, بیعلامت صحيح درون,
                   صحيح شاخص, صحيح رقم_added)
{
    بنية vring *vr = &vq->vring;
    صحيح i, av, سر, prev;
    بنية vring_desc *desc = vr->desc;
    بنية vring_avail *avail = vr->avail;

    BUG_ON(بیرون + درون == 0);

    prev = 0;
    سر = vq->free_سر;
    لكل (i = سر; بیرون; i = desc[i].بعدی, بیرون--) {
        desc[i].flags = VRING_DESC_F_بعدی;
        desc[i].addr = (u64)virt_مقصد_phys(list->addr);
        desc[i].len = list->درازی;
        prev = i;
        list++;
    }
    لكل ( ; درون; i = desc[i].بعدی, درون--) {
        desc[i].flags = VRING_DESC_F_بعدی|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_مقصد_phys(list->addr);
        desc[i].len = list->درازی;
        prev = i;
        list++;
    }
    desc[prev].flags = desc[prev].flags & ~VRING_DESC_F_بعدی;

    vq->free_سر = i;

    vq->vdata[سر] = شاخص;

    av = (avail->idx + رقم_added) % vr->رقم;
    avail->ring[av] = سر;
}

فراغ vring_kick(بنية vp_device *vp, بنية vring_virtqueue *vq, صحيح رقم_added)
{
    بنية vring *vr = &vq->vring;
    بنية vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + رقم_added;

    vp_notify(vp, vq);
}

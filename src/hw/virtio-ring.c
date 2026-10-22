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

#وصف_ڪريو BUG() افعل {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } طالما (0)
#وصف_ڪريو BUG_ON(condition) افعل { إذا (condition) BUG(); } طالما (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

صحيح vring_وڌيڪ_استعمال_ٿيل(بنية vring_virtqueue *vq)
{
    بنية vring_استعمال_ٿيل *استعمال_ٿيل = vq->vring.استعمال_ٿيل;
    صحيح وڌيڪ = vq->آخري_استعمال_ٿيل_idx != استعمال_ٿيل->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    أرجع وڌيڪ;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

فراغ vring_detach(بنية vring_virtqueue *vq, بنانشان صحيح سر)
{
    بنية vring *vr = &vq->vring;
    بنية vring_desc *desc = GET_LOWFLAT(vr->desc);
    بنانشان صحيح i;

    /* find end of given descriptor */

    i = سر;
    طالما (desc[i].flags & VRING_DESC_F_ايندڙ)
        i = desc[i].ايندڙ;

    /* link it with free list and point to it */

    desc[i].ايندڙ = vq->free_سر;
    vq->free_سر = سر;
}

/*
 * vring_get_buf
 *
 * get a buffer from the used list
 *
 */

صحيح vring_get_buf(بنية vring_virtqueue *vq, بنانشان صحيح *len)
{
    بنية vring *vr = &vq->vring;
    بنية vring_استعمال_ٿيل_elem *elem;
    بنية vring_استعمال_ٿيل *استعمال_ٿيل = vq->vring.استعمال_ٿيل;
    u32 id;
    صحيح ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &استعمال_ٿيل->ring[vq->آخري_استعمال_ٿيل_idx % vr->انگ];
    id = elem->id;
    إذا (len != عدم)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->آخري_استعمال_ٿيل_idx = vq->آخري_استعمال_ٿيل_idx + 1;

    أرجع ret;
}

فراغ vring_جوڙيو_buf(بنية vring_virtqueue *vq,
                   بنية vring_list list[],
                   بنانشان صحيح ٻاهريون, بنانشان صحيح اندريون,
                   صحيح اشاريو, صحيح انگ_added)
{
    بنية vring *vr = &vq->vring;
    صحيح i, av, سر, prev;
    بنية vring_desc *desc = vr->desc;
    بنية vring_avail *avail = vr->avail;

    BUG_ON(ٻاهريون + اندريون == 0);

    prev = 0;
    سر = vq->free_سر;
    لكل (i = سر; ٻاهريون; i = desc[i].ايندڙ, ٻاهريون--) {
        desc[i].flags = VRING_DESC_F_ايندڙ;
        desc[i].addr = (u64)virt_منزل_phys(list->addr);
        desc[i].len = list->ڊيگهائي;
        prev = i;
        list++;
    }
    لكل ( ; اندريون; i = desc[i].ايندڙ, اندريون--) {
        desc[i].flags = VRING_DESC_F_ايندڙ|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_منزل_phys(list->addr);
        desc[i].len = list->ڊيگهائي;
        prev = i;
        list++;
    }
    desc[prev].flags = desc[prev].flags & ~VRING_DESC_F_ايندڙ;

    vq->free_سر = i;

    vq->vdata[سر] = اشاريو;

    av = (avail->idx + انگ_added) % vr->انگ;
    avail->ring[av] = سر;
}

فراغ vring_kick(بنية vp_device *vp, بنية vring_virtqueue *vq, صحيح انگ_added)
{
    بنية vring *vr = &vq->vring;
    بنية vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + انگ_added;

    vp_notify(vp, vq);
}

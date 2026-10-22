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

#ਅਨ੍ਤਰ੍ਭਾਵ "output.h" // panic
#ਅਨ੍ਤਰ੍ਭਾਵ "virtio-ring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "virtio-pci.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUG() ਕਰੋ {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUG_ON(condition) ਕਰੋ { ਜੇ (condition) BUG(); } ਜਦੋਂ (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

ਪੂਰਨ vring_ਹੋਰ_ਵਰਤਿਆ(ਬਣਤਰ vring_virtqueue *vq)
{
    ਬਣਤਰ vring_ਵਰਤਿਆ *ਵਰਤਿਆ = vq->vring.ਵਰਤਿਆ;
    ਪੂਰਨ ਹੋਰ = vq->ਆਖਰੀ_ਵਰਤਿਆ_idx != ਵਰਤਿਆ->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    ਵਾਪਸ ਹੋਰ;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

ਖਾਲੀ vring_detach(ਬਣਤਰ vring_virtqueue *vq, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸਿਰ)
{
    ਬਣਤਰ vring *vr = &vq->vring;
    ਬਣਤਰ vring_desc *desc = GET_LOWFLAT(vr->desc);
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;

    /* find end of given descriptor */

    i = ਸਿਰ;
    ਜਦੋਂ (desc[i].flags & VRING_DESC_F_ਅਗਲਾ)
        i = desc[i].ਅਗਲਾ;

    /* link it with free list and point to it */

    desc[i].ਅਗਲਾ = vq->free_ਸਿਰ;
    vq->free_ਸਿਰ = ਸਿਰ;
}

/*
 * vring_get_buf
 *
 * get a buffer from the used list
 *
 */

ਪੂਰਨ vring_get_buf(ਬਣਤਰ vring_virtqueue *vq, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *len)
{
    ਬਣਤਰ vring *vr = &vq->vring;
    ਬਣਤਰ vring_ਵਰਤਿਆ_elem *elem;
    ਬਣਤਰ vring_ਵਰਤਿਆ *ਵਰਤਿਆ = vq->vring.ਵਰਤਿਆ;
    u32 id;
    ਪੂਰਨ ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &ਵਰਤਿਆ->ring[vq->ਆਖਰੀ_ਵਰਤਿਆ_idx % vr->ਅੰਕ];
    id = elem->id;
    ਜੇ (len != ਨਿਰਾ)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->ਆਖਰੀ_ਵਰਤਿਆ_idx = vq->ਆਖਰੀ_ਵਰਤਿਆ_idx + 1;

    ਵਾਪਸ ret;
}

ਖਾਲੀ vring_ਜੋੜੋ_buf(ਬਣਤਰ vring_virtqueue *vq,
                   ਬਣਤਰ vring_list list[],
                   ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਬਾਹਰਲਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਵਿੱਚ_2,
                   ਪੂਰਨ ਸੂਚਕਾਂਕ, ਪੂਰਨ ਅੰਕ_added)
{
    ਬਣਤਰ vring *vr = &vq->vring;
    ਪੂਰਨ i, av, ਸਿਰ, prev;
    ਬਣਤਰ vring_desc *desc = vr->desc;
    ਬਣਤਰ vring_avail *avail = vr->avail;

    BUG_ON(ਬਾਹਰਲਾ + ਵਿੱਚ_2 == 0);

    prev = 0;
    ਸਿਰ = vq->free_ਸਿਰ;
    ਲਈ (i = ਸਿਰ; ਬਾਹਰਲਾ; i = desc[i].ਅਗਲਾ, ਬਾਹਰਲਾ--) {
        desc[i].flags = VRING_DESC_F_ਅਗਲਾ;
        desc[i].addr = (u64)virt_ਮੰਜ਼ਿਲ_phys(list->addr);
        desc[i].len = list->ਲੰਮਾਈ;
        prev = i;
        list++;
    }
    ਲਈ ( ; ਵਿੱਚ_2; i = desc[i].ਅਗਲਾ, ਵਿੱਚ_2--) {
        desc[i].flags = VRING_DESC_F_ਅਗਲਾ|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_ਮੰਜ਼ਿਲ_phys(list->addr);
        desc[i].len = list->ਲੰਮਾਈ;
        prev = i;
        list++;
    }
    desc[prev].flags = desc[prev].flags & ~VRING_DESC_F_ਅਗਲਾ;

    vq->free_ਸਿਰ = i;

    vq->vdata[ਸਿਰ] = ਸੂਚਕਾਂਕ;

    av = (avail->idx + ਅੰਕ_added) % vr->ਅੰਕ;
    avail->ring[av] = ਸਿਰ;
}

ਖਾਲੀ vring_kick(ਬਣਤਰ vp_device *vp, ਬਣਤਰ vring_virtqueue *vq, ਪੂਰਨ ਅੰਕ_added)
{
    ਬਣਤਰ vring *vr = &vq->vring;
    ਬਣਤਰ vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + ਅੰਕ_added;

    vp_notify(vp, vq);
}

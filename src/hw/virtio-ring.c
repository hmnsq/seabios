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

#تعریف_کرو BUG() کرو {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } جب (0)
#تعریف_کرو BUG_ON(condition) کرو { اگر (condition) BUG(); } جب (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

صحیح vring_مزید_مستعمل(ساخت vring_virtqueue *vq)
{
    ساخت vring_مستعمل *مستعمل = vq->vring.مستعمل;
    صحیح مزید = vq->آخری_مستعمل_idx != مستعمل->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    واپس مزید;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

خالی vring_detach(ساخت vring_virtqueue *vq, غیرعلامتی صحیح سر)
{
    ساخت vring *vr = &vq->vring;
    ساخت vring_desc *desc = GET_LOWFLAT(vr->desc);
    غیرعلامتی صحیح i;

    /* find end of given descriptor */

    i = سر;
    جب (desc[i].flags & VRING_DESC_F_اگلا)
        i = desc[i].اگلا;

    /* link it with free list and point to it */

    desc[i].اگلا = vq->free_سر;
    vq->free_سر = سر;
}

/*
 * vring_get_buf
 *
 * get a buffer from the used list
 *
 */

صحیح vring_get_buf(ساخت vring_virtqueue *vq, غیرعلامتی صحیح *len)
{
    ساخت vring *vr = &vq->vring;
    ساخت vring_مستعمل_elem *elem;
    ساخت vring_مستعمل *مستعمل = vq->vring.مستعمل;
    u32 id;
    صحیح ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &مستعمل->ring[vq->آخری_مستعمل_idx % vr->عدد];
    id = elem->id;
    اگر (len != تہی)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->آخری_مستعمل_idx = vq->آخری_مستعمل_idx + 1;

    واپس ret;
}

خالی vring_جوڑو_buf(ساخت vring_virtqueue *vq,
                   ساخت vring_list list[],
                   غیرعلامتی صحیح برآمد, غیرعلامتی صحیح اندر,
                   صحیح اشاریہ, صحیح عدد_added)
{
    ساخت vring *vr = &vq->vring;
    صحیح i, av, سر, پچھلا;
    ساخت vring_desc *desc = vr->desc;
    ساخت vring_avail *avail = vr->avail;

    BUG_ON(برآمد + اندر == 0);

    پچھلا = 0;
    سر = vq->free_سر;
    برائے (i = سر; برآمد; i = desc[i].اگلا, برآمد--) {
        desc[i].flags = VRING_DESC_F_اگلا;
        desc[i].addr = (u64)virt_مقصد_phys(list->addr);
        desc[i].len = list->لمبائی;
        پچھلا = i;
        list++;
    }
    برائے ( ; اندر; i = desc[i].اگلا, اندر--) {
        desc[i].flags = VRING_DESC_F_اگلا|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_مقصد_phys(list->addr);
        desc[i].len = list->لمبائی;
        پچھلا = i;
        list++;
    }
    desc[پچھلا].flags = desc[پچھلا].flags & ~VRING_DESC_F_اگلا;

    vq->free_سر = i;

    vq->vdata[سر] = اشاریہ;

    av = (avail->idx + عدد_added) % vr->عدد;
    avail->ring[av] = سر;
}

خالی vring_kick(ساخت vp_device *vp, ساخت vring_virtqueue *vq, صحیح عدد_added)
{
    ساخت vring *vr = &vq->vring;
    ساخت vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + عدد_added;

    vp_notify(vp, vq);
}

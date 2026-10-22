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

#అన్తర్భావ "output.h" // panic
#అన్తర్భావ "virtio-ring.h"
#అన్తర్భావ "virtio-pci.h"

#నిర్వచించు BUG() చేయి {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } అయితే (0)
#నిర్వచించు BUG_ON(condition) చేయి { ఒకవేళ (condition) BUG(); } అయితే (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

పూర్ణాంకం vring_మరిన్ని_వాడిన(నిర్మాణం vring_virtqueue *vq)
{
    నిర్మాణం vring_వాడిన *వాడిన = vq->vring.వాడిన;
    పూర్ణాంకం మరిన్ని = vq->చివరి_వాడిన_idx != వాడిన->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    తిరిగి మరిన్ని;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

శూన్య vring_detach(నిర్మాణం vring_virtqueue *vq, గుర్తులేని పూర్ణాంకం తల)
{
    నిర్మాణం vring *vr = &vq->vring;
    నిర్మాణం vring_desc *desc = GET_LOWFLAT(vr->desc);
    గుర్తులేని పూర్ణాంకం i;

    /* find end of given descriptor */

    i = తల;
    అయితే (desc[i].flags & VRING_DESC_F_తదుపరి)
        i = desc[i].తదుపరి;

    /* link it with free list and point to it */

    desc[i].తదుపరి = vq->free_తల;
    vq->free_తల = తల;
}

/*
 * vring_get_buf
 *
 * get a buffer from the used list
 *
 */

పూర్ణాంకం vring_get_buf(నిర్మాణం vring_virtqueue *vq, గుర్తులేని పూర్ణాంకం *len)
{
    నిర్మాణం vring *vr = &vq->vring;
    నిర్మాణం vring_వాడిన_elem *elem;
    నిర్మాణం vring_వాడిన *వాడిన = vq->vring.వాడిన;
    u32 id;
    పూర్ణాంకం ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &వాడిన->ring[vq->చివరి_వాడిన_idx % vr->అంకె];
    id = elem->id;
    ఒకవేళ (len != ఖాళీ)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->చివరి_వాడిన_idx = vq->చివరి_వాడిన_idx + 1;

    తిరిగి ret;
}

శూన్య vring_కూడు_buf(నిర్మాణం vring_virtqueue *vq,
                   నిర్మాణం vring_list list[],
                   గుర్తులేని పూర్ణాంకం బహిర్గతం, గుర్తులేని పూర్ణాంకం లోపల,
                   పూర్ణాంకం సూచీ, పూర్ణాంకం అంకె_added)
{
    నిర్మాణం vring *vr = &vq->vring;
    పూర్ణాంకం i, av, తల, prev;
    నిర్మాణం vring_desc *desc = vr->desc;
    నిర్మాణం vring_avail *avail = vr->avail;

    BUG_ON(బహిర్గతం + లోపల == 0);

    prev = 0;
    తల = vq->free_తల;
    కొరకు (i = తల; బహిర్గతం; i = desc[i].తదుపరి, బహిర్గతం--) {
        desc[i].flags = VRING_DESC_F_తదుపరి;
        desc[i].addr = (u64)virt_గమ్యం_phys(list->addr);
        desc[i].len = list->నిడివి;
        prev = i;
        list++;
    }
    కొరకు ( ; లోపల; i = desc[i].తదుపరి, లోపల--) {
        desc[i].flags = VRING_DESC_F_తదుపరి|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_గమ్యం_phys(list->addr);
        desc[i].len = list->నిడివి;
        prev = i;
        list++;
    }
    desc[prev].flags = desc[prev].flags & ~VRING_DESC_F_తదుపరి;

    vq->free_తల = i;

    vq->vdata[తల] = సూచీ;

    av = (avail->idx + అంకె_added) % vr->అంకె;
    avail->ring[av] = తల;
}

శూన్య vring_kick(నిర్మాణం vp_device *vp, నిర్మాణం vring_virtqueue *vq, పూర్ణాంకం అంకె_added)
{
    నిర్మాణం vring *vr = &vq->vring;
    నిర్మాణం vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + అంకె_added;

    vp_notify(vp, vq);
}

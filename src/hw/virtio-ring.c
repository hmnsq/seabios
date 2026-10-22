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

#ଅନ୍ତର୍ଭାଵ "output.h" // panic
#ଅନ୍ତର୍ଭାଵ "virtio-ring.h"
#ଅନ୍ତର୍ଭାଵ "virtio-pci.h"

#define BUG() କର {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } ଯେତେବେଳେ (0)
#define BUG_ON(condition) କର { ଯଦି (condition) BUG(); } ଯେତେବେଳେ (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା vring_more_used(ସଂରଚନା vring_virtqueue *vq)
{
    ସଂରଚନା vring_used *used = vq->vring.used;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା more = vq->last_used_idx != used->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    ଫେରନ୍ତୁ more;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

ଶୂନ୍ୟ vring_detach(ସଂରଚନା vring_virtqueue *vq, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା head)
{
    ସଂରଚନା vring *vr = &vq->vring;
    ସଂରଚନା vring_desc *desc = GET_LOWFLAT(vr->desc);
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

    /* find end of given descriptor */

    i = head;
    ଯେତେବେଳେ (desc[i].flags & VRING_DESC_F_NEXT)
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

ପୂର୍ଣ୍ଣସଂଖ୍ୟା vring_get_buf(ସଂରଚନା vring_virtqueue *vq, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *len)
{
    ସଂରଚନା vring *vr = &vq->vring;
    ସଂରଚନା vring_used_elem *elem;
    ସଂରଚନା vring_used *used = vq->vring.used;
    u32 id;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &used->ring[vq->last_used_idx % vr->ଅଙ୍କ];
    id = elem->id;
    ଯଦି (len != ଖାଲି)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->last_used_idx = vq->last_used_idx + 1;

    ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ vring_add_buf(ସଂରଚନା vring_virtqueue *vq,
                   ସଂରଚନା vring_list list[],
                   unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା out, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା in,
                   ପୂର୍ଣ୍ଣସଂଖ୍ୟା index, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_added)
{
    ସଂରଚନା vring *vr = &vq->vring;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, av, head, prev;
    ସଂରଚନା vring_desc *desc = vr->desc;
    ସଂରଚନା vring_avail *avail = vr->avail;

    BUG_ON(out + in == 0);

    prev = 0;
    head = vq->free_head;
    ପାଇଁ (i = head; out; i = desc[i].next, out--) {
        desc[i].flags = VRING_DESC_F_NEXT;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    ପାଇଁ ( ; in; i = desc[i].next, in--) {
        desc[i].flags = VRING_DESC_F_NEXT|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    desc[prev].flags = desc[prev].flags & ~VRING_DESC_F_NEXT;

    vq->free_head = i;

    vq->vdata[head] = index;

    av = (avail->idx + ଅଙ୍କ_added) % vr->ଅଙ୍କ;
    avail->ring[av] = head;
}

ଶୂନ୍ୟ vring_kick(ସଂରଚନା vp_device *vp, ସଂରଚନା vring_virtqueue *vq, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_added)
{
    ସଂରଚନା vring *vr = &vq->vring;
    ସଂରଚନା vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + ଅଙ୍କ_added;

    vp_notify(vp, vq);
}

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

#अन्तर्भाव "output.h" // panic
#अन्तर्भाव "virtio-ring.h"
#अन्तर्भाव "virtio-pci.h"

#define BUG() करा {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } पर्यंत (0)
#define BUG_ON(condition) करा { जर (condition) BUG(); } पर्यंत (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

पूर्णांक vring_more_used(संरचना vring_virtqueue *vq)
{
    संरचना vring_used *used = vq->vring.used;
    पूर्णांक more = vq->last_used_idx != used->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    परत more;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

शून्य vring_detach(संरचना vring_virtqueue *vq, अचिन्हित पूर्णांक head)
{
    संरचना vring *vr = &vq->vring;
    संरचना vring_desc *desc = GET_LOWFLAT(vr->desc);
    अचिन्हित पूर्णांक i;

    /* find end of given descriptor */

    i = head;
    पर्यंत (desc[i].flags & VRING_DESC_F_NEXT)
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

पूर्णांक vring_get_buf(संरचना vring_virtqueue *vq, अचिन्हित पूर्णांक *len)
{
    संरचना vring *vr = &vq->vring;
    संरचना vring_used_elem *elem;
    संरचना vring_used *used = vq->vring.used;
    u32 id;
    पूर्णांक ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &used->ring[vq->last_used_idx % vr->अंक];
    id = elem->id;
    जर (len != रिक्त)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->last_used_idx = vq->last_used_idx + 1;

    परत ret;
}

शून्य vring_add_buf(संरचना vring_virtqueue *vq,
                   संरचना vring_list list[],
                   अचिन्हित पूर्णांक out, अचिन्हित पूर्णांक in,
                   पूर्णांक index, पूर्णांक अंक_added)
{
    संरचना vring *vr = &vq->vring;
    पूर्णांक i, av, head, prev;
    संरचना vring_desc *desc = vr->desc;
    संरचना vring_avail *avail = vr->avail;

    BUG_ON(out + in == 0);

    prev = 0;
    head = vq->free_head;
    साठी (i = head; out; i = desc[i].next, out--) {
        desc[i].flags = VRING_DESC_F_NEXT;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    साठी ( ; in; i = desc[i].next, in--) {
        desc[i].flags = VRING_DESC_F_NEXT|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    desc[prev].flags = desc[prev].flags & ~VRING_DESC_F_NEXT;

    vq->free_head = i;

    vq->vdata[head] = index;

    av = (avail->idx + अंक_added) % vr->अंक;
    avail->ring[av] = head;
}

शून्य vring_kick(संरचना vp_device *vp, संरचना vring_virtqueue *vq, पूर्णांक अंक_added)
{
    संरचना vring *vr = &vq->vring;
    संरचना vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + अंक_added;

    vp_notify(vp, vq);
}

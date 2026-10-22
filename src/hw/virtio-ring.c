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

#ಅನ್ತರ್ಭಾವ "output.h" // panic
#ಅನ್ತರ್ಭಾವ "virtio-ring.h"
#ಅನ್ತರ್ಭಾವ "virtio-pci.h"

#define BUG() ಕುರು {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } ಯಾವತ್ (0)
#define BUG_ON(condition) ಕುರು { ಯದಿ (condition) BUG(); } ಯಾವತ್ (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

ಪೂರ್ಣಾಙ್ಕ vring_more_used(ಸಂರಚನಾ vring_virtqueue *vq)
{
    ಸಂರಚನಾ vring_used *used = vq->vring.used;
    ಪೂರ್ಣಾಙ್ಕ more = vq->last_used_idx != used->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    ಪ್ರತ್ಯಾಗಮ more;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

ಶೂನ್ಯ vring_detach(ಸಂರಚನಾ vring_virtqueue *vq, unsigned ಪೂರ್ಣಾಙ್ಕ head)
{
    ಸಂರಚನಾ vring *vr = &vq->vring;
    ಸಂರಚನಾ vring_desc *desc = GET_LOWFLAT(vr->desc);
    unsigned ಪೂರ್ಣಾಙ್ಕ i;

    /* find end of given descriptor */

    i = head;
    ಯಾವತ್ (desc[i].flags & VRING_DESC_F_NEXT)
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

ಪೂರ್ಣಾಙ್ಕ vring_get_buf(ಸಂರಚನಾ vring_virtqueue *vq, unsigned ಪೂರ್ಣಾಙ್ಕ *len)
{
    ಸಂರಚನಾ vring *vr = &vq->vring;
    ಸಂರಚನಾ vring_used_elem *elem;
    ಸಂರಚನಾ vring_used *used = vq->vring.used;
    u32 id;
    ಪೂರ್ಣಾಙ್ಕ ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &used->ring[vq->last_used_idx % vr->ಅಂಕೆ];
    id = elem->id;
    ಯದಿ (len != ರಿಕ್ತ)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->last_used_idx = vq->last_used_idx + 1;

    ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ vring_add_buf(ಸಂರಚನಾ vring_virtqueue *vq,
                   ಸಂರಚನಾ vring_list list[],
                   unsigned ಪೂರ್ಣಾಙ್ಕ out, unsigned ಪೂರ್ಣಾಙ್ಕ in,
                   ಪೂರ್ಣಾಙ್ಕ index, ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_added)
{
    ಸಂರಚನಾ vring *vr = &vq->vring;
    ಪೂರ್ಣಾಙ್ಕ i, av, head, prev;
    ಸಂರಚನಾ vring_desc *desc = vr->desc;
    ಸಂರಚನಾ vring_avail *avail = vr->avail;

    BUG_ON(out + in == 0);

    prev = 0;
    head = vq->free_head;
    ಪ್ರತಿ (i = head; out; i = desc[i].next, out--) {
        desc[i].flags = VRING_DESC_F_NEXT;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    ಪ್ರತಿ ( ; in; i = desc[i].next, in--) {
        desc[i].flags = VRING_DESC_F_NEXT|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->length;
        prev = i;
        list++;
    }
    desc[prev].flags = desc[prev].flags & ~VRING_DESC_F_NEXT;

    vq->free_head = i;

    vq->vdata[head] = index;

    av = (avail->idx + ಅಂಕೆ_added) % vr->ಅಂಕೆ;
    avail->ring[av] = head;
}

ಶೂನ್ಯ vring_kick(ಸಂರಚನಾ vp_device *vp, ಸಂರಚನಾ vring_virtqueue *vq, ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_added)
{
    ಸಂರಚನಾ vring *vr = &vq->vring;
    ಸಂರಚನಾ vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + ಅಂಕೆ_added;

    vp_notify(vp, vq);
}

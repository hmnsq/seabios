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

#ವ್ಯಾಖ್ಯಾನಿಸು BUG() ಮಾಡು {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು BUG_ON(condition) ಮಾಡು { ಆದರೆ (condition) BUG(); } ಆಗ (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

ಪೂರ್ಣಾಂಕ vring_ಇನ್ನಷ್ಟು_ಬಳಸಿದ(ರಚನೆ vring_virtqueue *vq)
{
    ರಚನೆ vring_ಬಳಸಿದ *ಬಳಸಿದ = vq->vring.ಬಳಸಿದ;
    ಪೂರ್ಣಾಂಕ ಇನ್ನಷ್ಟು = vq->ಕೊನೆಯ_ಬಳಸಿದ_idx != ಬಳಸಿದ->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    ಹಿಂತಿರುಗಿ ಇನ್ನಷ್ಟು;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

ಶೂನ್ಯ vring_detach(ರಚನೆ vring_virtqueue *vq, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ತಲೆ)
{
    ರಚನೆ vring *vr = &vq->vring;
    ರಚನೆ vring_desc *desc = GET_LOWFLAT(vr->desc);
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;

    /* find end of given descriptor */

    i = ತಲೆ;
    ಆಗ (desc[i].flags & VRING_DESC_F_ಮುಂದಿನ)
        i = desc[i].ಮುಂದಿನ;

    /* link it with free list and point to it */

    desc[i].ಮುಂದಿನ = vq->free_ತಲೆ;
    vq->free_ತಲೆ = ತಲೆ;
}

/*
 * vring_get_buf
 *
 * get a buffer from the used list
 *
 */

ಪೂರ್ಣಾಂಕ vring_get_buf(ರಚನೆ vring_virtqueue *vq, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *len)
{
    ರಚನೆ vring *vr = &vq->vring;
    ರಚನೆ vring_ಬಳಸಿದ_elem *elem;
    ರಚನೆ vring_ಬಳಸಿದ *ಬಳಸಿದ = vq->vring.ಬಳಸಿದ;
    u32 id;
    ಪೂರ್ಣಾಂಕ ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &ಬಳಸಿದ->ring[vq->ಕೊನೆಯ_ಬಳಸಿದ_idx % vr->ಅಂಕೆ];
    id = elem->id;
    ಆದರೆ (len != ಖಾಲಿ)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->ಕೊನೆಯ_ಬಳಸಿದ_idx = vq->ಕೊನೆಯ_ಬಳಸಿದ_idx + 1;

    ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ vring_ಕೂಡಿಸು_buf(ರಚನೆ vring_virtqueue *vq,
                   ರಚನೆ vring_list list[],
                   ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಹೊರಹೋಗು, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಒಳ,
                   ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ, ಪೂರ್ಣಾಂಕ ಅಂಕೆ_added)
{
    ರಚನೆ vring *vr = &vq->vring;
    ಪೂರ್ಣಾಂಕ i, av, ತಲೆ, prev;
    ರಚನೆ vring_desc *desc = vr->desc;
    ರಚನೆ vring_avail *avail = vr->avail;

    BUG_ON(ಹೊರಹೋಗು + ಒಳ == 0);

    prev = 0;
    ತಲೆ = vq->free_ತಲೆ;
    ಗಾಗಿ (i = ತಲೆ; ಹೊರಹೋಗು; i = desc[i].ಮುಂದಿನ, ಹೊರಹೋಗು--) {
        desc[i].flags = VRING_DESC_F_ಮುಂದಿನ;
        desc[i].addr = (u64)virt_ಗಮ್ಯ_phys(list->addr);
        desc[i].len = list->ಉದ್ದಳತೆ;
        prev = i;
        list++;
    }
    ಗಾಗಿ ( ; ಒಳ; i = desc[i].ಮುಂದಿನ, ಒಳ--) {
        desc[i].flags = VRING_DESC_F_ಮುಂದಿನ|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_ಗಮ್ಯ_phys(list->addr);
        desc[i].len = list->ಉದ್ದಳತೆ;
        prev = i;
        list++;
    }
    desc[prev].flags = desc[prev].flags & ~VRING_DESC_F_ಮುಂದಿನ;

    vq->free_ತಲೆ = i;

    vq->vdata[ತಲೆ] = ಸೂಚ್ಯಂಕ;

    av = (avail->idx + ಅಂಕೆ_added) % vr->ಅಂಕೆ;
    avail->ring[av] = ತಲೆ;
}

ಶೂನ್ಯ vring_kick(ರಚನೆ vp_device *vp, ರಚನೆ vring_virtqueue *vq, ಪೂರ್ಣಾಂಕ ಅಂಕೆ_added)
{
    ರಚನೆ vring *vr = &vq->vring;
    ರಚನೆ vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + ಅಂಕೆ_added;

    vp_notify(vp, vq);
}

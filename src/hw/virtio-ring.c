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

#समावेश "output.h" // panic
#समावेश "virtio-ring.h"
#समावेश "virtio-pci.h"

#घोषणा BUG() करो {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } जबतक (0)
#घोषणा BUG_ON(condition) करो { अगर (condition) BUG(); } जबतक (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

पूर्णांक vring_more_used(काष्ठा vring_virtqueue *vq)
{
    काष्ठा vring_used *used = vq->vring.used;
    पूर्णांक more = vq->last_used_idx != used->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    वापस more;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

व्योम vring_detach(काष्ठा vring_virtqueue *vq, अचिन्हित पूर्णांक शीर्ष)
{
    काष्ठा vring *vr = &vq->vring;
    काष्ठा vring_desc *desc = GET_LOWFLAT(vr->desc);
    अचिन्हित पूर्णांक i;

    /* find end of given descriptor */

    i = शीर्ष;
    जबतक (desc[i].flags & VRING_DESC_F_अग्र)
        i = desc[i].अग्र;

    /* link it with free list and point to it */

    desc[i].अग्र = vq->free_शीर्ष;
    vq->free_शीर्ष = शीर्ष;
}

/*
 * vring_get_buf
 *
 * get a buffer from the used list
 *
 */

पूर्णांक vring_get_buf(काष्ठा vring_virtqueue *vq, अचिन्हित पूर्णांक *len)
{
    काष्ठा vring *vr = &vq->vring;
    काष्ठा vring_used_elem *elem;
    काष्ठा vring_used *used = vq->vring.used;
    u32 id;
    पूर्णांक ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &used->ring[vq->last_used_idx % vr->अंक];
    id = elem->id;
    अगर (len != शून्य)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->last_used_idx = vq->last_used_idx + 1;

    वापस ret;
}

व्योम vring_add_buf(काष्ठा vring_virtqueue *vq,
                   काष्ठा vring_list list[],
                   अचिन्हित पूर्णांक out, अचिन्हित पूर्णांक in,
                   पूर्णांक सूचकांक, पूर्णांक अंक_added)
{
    काष्ठा vring *vr = &vq->vring;
    पूर्णांक i, av, शीर्ष, पूर्व;
    काष्ठा vring_desc *desc = vr->desc;
    काष्ठा vring_avail *avail = vr->avail;

    BUG_ON(out + in == 0);

    पूर्व = 0;
    शीर्ष = vq->free_शीर्ष;
    क्रम (i = शीर्ष; out; i = desc[i].अग्र, out--) {
        desc[i].flags = VRING_DESC_F_अग्र;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->लंबाई;
        पूर्व = i;
        list++;
    }
    क्रम ( ; in; i = desc[i].अग्र, in--) {
        desc[i].flags = VRING_DESC_F_अग्र|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_to_phys(list->addr);
        desc[i].len = list->लंबाई;
        पूर्व = i;
        list++;
    }
    desc[पूर्व].flags = desc[पूर्व].flags & ~VRING_DESC_F_अग्र;

    vq->free_शीर्ष = i;

    vq->vdata[शीर्ष] = सूचकांक;

    av = (avail->idx + अंक_added) % vr->अंक;
    avail->ring[av] = शीर्ष;
}

व्योम vring_kick(काष्ठा vp_device *vp, काष्ठा vring_virtqueue *vq, पूर्णांक अंक_added)
{
    काष्ठा vring *vr = &vq->vring;
    काष्ठा vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + अंक_added;

    vp_notify(vp, vq);
}

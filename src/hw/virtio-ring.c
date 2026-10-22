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

#परिभाषय BUG() कुरु {                                                      \
            panic("BUG: failure at %d/%s()!\n", __LINE__, __func__);    \
        } यावत् (0)
#परिभाषय BUG_ON(condition) कुरु { यदि (condition) BUG(); } यावत् (0)

/*
 * vring_more_used
 *
 * is there some used buffers ?
 *
 */

पूर्णाङ्क vring_अधिकम्_प्रयुक्तम्(संरचना vring_virtqueue *vq)
{
    संरचना vring_प्रयुक्तम् *प्रयुक्तम् = vq->vring.प्रयुक्तम्;
    पूर्णाङ्क अधिकम् = vq->चरमः_प्रयुक्तम्_idx != प्रयुक्तम्->idx;
    /* Make sure ring reads are done after idx read above. */
    smp_rmb();
    प्रत्यागम अधिकम्;
}

/*
 * vring_free
 *
 * put at the begin of the free list the current desc[head]
 */

शून्य vring_detach(संरचना vring_virtqueue *vq, अचिह्नित पूर्णाङ्क शीर्ष)
{
    संरचना vring *vr = &vq->vring;
    संरचना vring_desc *desc = GET_LOWFLAT(vr->desc);
    अचिह्नित पूर्णाङ्क i;

    /* find end of given descriptor */

    i = शीर्ष;
    यावत् (desc[i].flags & VRING_DESC_F_अग्र)
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

पूर्णाङ्क vring_get_buf(संरचना vring_virtqueue *vq, अचिह्नित पूर्णाङ्क *len)
{
    संरचना vring *vr = &vq->vring;
    संरचना vring_प्रयुक्तम्_elem *elem;
    संरचना vring_प्रयुक्तम् *प्रयुक्तम् = vq->vring.प्रयुक्तम्;
    u32 id;
    पूर्णाङ्क ret;

//    BUG_ON(!vring_more_used(vq));

    elem = &प्रयुक्तम्->ring[vq->चरमः_प्रयुक्तम्_idx % vr->अङ्कः];
    id = elem->id;
    यदि (len != रिक्त)
        *len = elem->len;

    ret = vq->vdata[id];

    vring_detach(vq, id);

    vq->चरमः_प्रयुक्तम्_idx = vq->चरमः_प्रयुक्तम्_idx + 1;

    प्रत्यागम ret;
}

शून्य vring_सङ्कलय_buf(संरचना vring_virtqueue *vq,
                   संरचना vring_list list[],
                   अचिह्नित पूर्णाङ्क बहिर्गमनम्, अचिह्नित पूर्णाङ्क अन्तर्,
                   पूर्णाङ्क सूचकांक, पूर्णाङ्क अङ्कः_added)
{
    संरचना vring *vr = &vq->vring;
    पूर्णाङ्क i, av, शीर्ष, पूर्व;
    संरचना vring_desc *desc = vr->desc;
    संरचना vring_avail *avail = vr->avail;

    BUG_ON(बहिर्गमनम् + अन्तर् == 0);

    पूर्व = 0;
    शीर्ष = vq->free_शीर्ष;
    प्रति (i = शीर्ष; बहिर्गमनम्; i = desc[i].अग्र, बहिर्गमनम्--) {
        desc[i].flags = VRING_DESC_F_अग्र;
        desc[i].addr = (u64)virt_गन्तव्यम्_phys(list->addr);
        desc[i].len = list->दीर्घता;
        पूर्व = i;
        list++;
    }
    प्रति ( ; अन्तर्; i = desc[i].अग्र, अन्तर्--) {
        desc[i].flags = VRING_DESC_F_अग्र|VRING_DESC_F_WRITE;
        desc[i].addr = (u64)virt_गन्तव्यम्_phys(list->addr);
        desc[i].len = list->दीर्घता;
        पूर्व = i;
        list++;
    }
    desc[पूर्व].flags = desc[पूर्व].flags & ~VRING_DESC_F_अग्र;

    vq->free_शीर्ष = i;

    vq->vdata[शीर्ष] = सूचकांक;

    av = (avail->idx + अङ्कः_added) % vr->अङ्कः;
    avail->ring[av] = शीर्ष;
}

शून्य vring_kick(संरचना vp_device *vp, संरचना vring_virtqueue *vq, पूर्णाङ्क अङ्कः_added)
{
    संरचना vring *vr = &vq->vring;
    संरचना vring_avail *avail = vr->avail;

    /* Make sure idx update is done after ring write. */
    smp_wmb();
    avail->idx = avail->idx + अङ्कः_added;

    vp_notify(vp, vq);
}

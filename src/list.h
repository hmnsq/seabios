#ifndef __LIST_H
#define __LIST_H

#அந்தர்பாவ "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

கட்டமைப்பு hlist_முனை {
    கட்டமைப்பு hlist_முனை *next, **pprev;
};

கட்டமைப்பு hlist_head {
    கட்டமைப்பு hlist_முனை *first;
};

நிலையான உள்வரி முழு
hlist_empty(மாறிலி கட்டமைப்பு hlist_head *h)
{
    திரும்பு !h->first;
}

நிலையான உள்வரி வெறுமை
hlist_del(கட்டமைப்பு hlist_முனை *n)
{
    கட்டமைப்பு hlist_முனை *next = n->next;
    கட்டமைப்பு hlist_முனை **pprev = n->pprev;
    *pprev = next;
    எனில் (next)
        next->pprev = pprev;
}

நிலையான உள்வரி வெறுமை
hlist_add(கட்டமைப்பு hlist_முனை *n, கட்டமைப்பு hlist_முனை **pprev)
{
    கட்டமைப்பு hlist_முனை *next = *pprev;
    n->pprev = pprev;
    n->next = next;
    எனில் (next)
        next->pprev = &n->next;
    *pprev = n;
}

நிலையான உள்வரி வெறுமை
hlist_add_head(கட்டமைப்பு hlist_முனை *n, கட்டமைப்பு hlist_head *h)
{
    hlist_add(n, &h->first);
}

நிலையான உள்வரி வெறுமை
hlist_add_before(கட்டமைப்பு hlist_முனை *n, கட்டமைப்பு hlist_முனை *next)
{
    hlist_add(n, next->pprev);
}

நிலையான உள்வரி வெறுமை
hlist_add_after(கட்டமைப்பு hlist_முனை *n, கட்டமைப்பு hlist_முனை *prev)
{
    hlist_add(n, &prev->next);
}

நிலையான உள்வரி வெறுமை
hlist_replace(கட்டமைப்பு hlist_முனை *old, கட்டமைப்பு hlist_முனை *new)
{
    new->next = old->next;
    எனில் (new->next)
        new->next->pprev = &new->next;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#define hlist_for_each_entry(pos, head, member)                         \
    க்கு (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(வெற்று, typeof(*pos), member)              \
         ; pos = container_of(pos->member.next, typeof(*pos), member))

#define hlist_for_each_entry_safe(pos, n, head, member)                 \
    க்கு (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(வெற்று, typeof(*pos), member)              \
             && ({ n = pos->member.next; 1; })                          \
         ; pos = container_of(n, typeof(*pos), member))

#define hlist_for_each_entry_pprev(pos, pprev, head, member)            \
    க்கு (pprev = &(head)->first                                         \
         ; *pprev && ({ pos=container_of(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->next)


#endif // list.h

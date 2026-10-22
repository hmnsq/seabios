#ifndef __LIST_H
#define __LIST_H

#অন্তর্ভাব "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

সংরচনা hlist_গ্রন্থি {
    সংরচনা hlist_গ্রন্থি *next, **pprev;
};

সংরচনা hlist_head {
    সংরচনা hlist_গ্রন্থি *first;
};

স্থির অন্তঃস্থ পূর্ণাঙ্ক
hlist_empty(ধ্রুব সংরচনা hlist_head *h)
{
    প্রত্যাগম !h->first;
}

স্থির অন্তঃস্থ শূন্য
hlist_del(সংরচনা hlist_গ্রন্থি *n)
{
    সংরচনা hlist_গ্রন্থি *next = n->next;
    সংরচনা hlist_গ্রন্থি **pprev = n->pprev;
    *pprev = next;
    যদি (next)
        next->pprev = pprev;
}

স্থির অন্তঃস্থ শূন্য
hlist_add(সংরচনা hlist_গ্রন্থি *n, সংরচনা hlist_গ্রন্থি **pprev)
{
    সংরচনা hlist_গ্রন্থি *next = *pprev;
    n->pprev = pprev;
    n->next = next;
    যদি (next)
        next->pprev = &n->next;
    *pprev = n;
}

স্থির অন্তঃস্থ শূন্য
hlist_add_head(সংরচনা hlist_গ্রন্থি *n, সংরচনা hlist_head *h)
{
    hlist_add(n, &h->first);
}

স্থির অন্তঃস্থ শূন্য
hlist_add_before(সংরচনা hlist_গ্রন্থি *n, সংরচনা hlist_গ্রন্থি *next)
{
    hlist_add(n, next->pprev);
}

স্থির অন্তঃস্থ শূন্য
hlist_add_after(সংরচনা hlist_গ্রন্থি *n, সংরচনা hlist_গ্রন্থি *prev)
{
    hlist_add(n, &prev->next);
}

স্থির অন্তঃস্থ শূন্য
hlist_replace(সংরচনা hlist_গ্রন্থি *old, সংরচনা hlist_গ্রন্থি *new)
{
    new->next = old->next;
    যদি (new->next)
        new->next->pprev = &new->next;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#define hlist_for_each_entry(pos, head, member)                         \
    প্রতি (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(রিক্ত, typeof(*pos), member)              \
         ; pos = container_of(pos->member.next, typeof(*pos), member))

#define hlist_for_each_entry_safe(pos, n, head, member)                 \
    প্রতি (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(রিক্ত, typeof(*pos), member)              \
             && ({ n = pos->member.next; 1; })                          \
         ; pos = container_of(n, typeof(*pos), member))

#define hlist_for_each_entry_pprev(pos, pprev, head, member)            \
    প্রতি (pprev = &(head)->first                                         \
         ; *pprev && ({ pos=container_of(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->next)


#endif // list.h

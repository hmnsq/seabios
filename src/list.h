#ifndef __LIST_H
#define __LIST_H

#অন্তর্ভাব "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

কাঠামো hlist_গ্রন্থি {
    কাঠামো hlist_গ্রন্থি *পরবর্তী, **pprev;
};

কাঠামো hlist_শীর্ষ {
    কাঠামো hlist_গ্রন্থি *first;
};

স্থির অন্তঃরেখ পূর্ণসংখ্যা
hlist_empty(ধ্রুবক কাঠামো hlist_শীর্ষ *h)
{
    ফেরত !h->first;
}

স্থির অন্তঃরেখ শূন্য
hlist_del(কাঠামো hlist_গ্রন্থি *n)
{
    কাঠামো hlist_গ্রন্থি *পরবর্তী = n->পরবর্তী;
    কাঠামো hlist_গ্রন্থি **pprev = n->pprev;
    *pprev = পরবর্তী;
    যদি (পরবর্তী)
        পরবর্তী->pprev = pprev;
}

স্থির অন্তঃরেখ শূন্য
hlist_add(কাঠামো hlist_গ্রন্থি *n, কাঠামো hlist_গ্রন্থি **pprev)
{
    কাঠামো hlist_গ্রন্থি *পরবর্তী = *pprev;
    n->pprev = pprev;
    n->পরবর্তী = পরবর্তী;
    যদি (পরবর্তী)
        পরবর্তী->pprev = &n->পরবর্তী;
    *pprev = n;
}

স্থির অন্তঃরেখ শূন্য
hlist_add_শীর্ষ(কাঠামো hlist_গ্রন্থি *n, কাঠামো hlist_শীর্ষ *h)
{
    hlist_add(n, &h->first);
}

স্থির অন্তঃরেখ শূন্য
hlist_add_before(কাঠামো hlist_গ্রন্থি *n, কাঠামো hlist_গ্রন্থি *পরবর্তী)
{
    hlist_add(n, পরবর্তী->pprev);
}

স্থির অন্তঃরেখ শূন্য
hlist_add_after(কাঠামো hlist_গ্রন্থি *n, কাঠামো hlist_গ্রন্থি *পূর্ব)
{
    hlist_add(n, &পূর্ব->পরবর্তী);
}

স্থির অন্তঃরেখ শূন্য
hlist_replace(কাঠামো hlist_গ্রন্থি *old, কাঠামো hlist_গ্রন্থি *new)
{
    new->পরবর্তী = old->পরবর্তী;
    যদি (new->পরবর্তী)
        new->পরবর্তী->pprev = &new->পরবর্তী;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#define hlist_for_each_entry(pos, শীর্ষ, member)                         \
    জন্য (pos = container_of((শীর্ষ)->first, typeof(*pos), member)        \
         ; pos != container_of(ফাঁকা, typeof(*pos), member)              \
         ; pos = container_of(pos->member.পরবর্তী, typeof(*pos), member))

#define hlist_for_each_entry_safe(pos, n, শীর্ষ, member)                 \
    জন্য (pos = container_of((শীর্ষ)->first, typeof(*pos), member)        \
         ; pos != container_of(ফাঁকা, typeof(*pos), member)              \
             && ({ n = pos->member.পরবর্তী; 1; })                          \
         ; pos = container_of(n, typeof(*pos), member))

#define hlist_for_each_entry_pprev(pos, pprev, শীর্ষ, member)            \
    জন্য (pprev = &(শীর্ষ)->first                                         \
         ; *pprev && ({ pos=container_of(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->পরবর্তী)


#endif // list.h

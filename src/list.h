#ifndef __LIST_H
#define __LIST_H

#شامل "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

ساختار hlist_گره {
    ساختار hlist_گره *next, **pprev;
};

ساختار hlist_head {
    ساختار hlist_گره *first;
};

ایستا درونخطی صحیح
hlist_empty(ثابت ساختار hlist_head *h)
{
    بازگشت !h->first;
}

ایستا درونخطی تهی
hlist_del(ساختار hlist_گره *n)
{
    ساختار hlist_گره *next = n->next;
    ساختار hlist_گره **pprev = n->pprev;
    *pprev = next;
    اگر (next)
        next->pprev = pprev;
}

ایستا درونخطی تهی
hlist_add(ساختار hlist_گره *n, ساختار hlist_گره **pprev)
{
    ساختار hlist_گره *next = *pprev;
    n->pprev = pprev;
    n->next = next;
    اگر (next)
        next->pprev = &n->next;
    *pprev = n;
}

ایستا درونخطی تهی
hlist_add_head(ساختار hlist_گره *n, ساختار hlist_head *h)
{
    hlist_add(n, &h->first);
}

ایستا درونخطی تهی
hlist_add_before(ساختار hlist_گره *n, ساختار hlist_گره *next)
{
    hlist_add(n, next->pprev);
}

ایستا درونخطی تهی
hlist_add_after(ساختار hlist_گره *n, ساختار hlist_گره *prev)
{
    hlist_add(n, &prev->next);
}

ایستا درونخطی تهی
hlist_replace(ساختار hlist_گره *old, ساختار hlist_گره *new)
{
    new->next = old->next;
    اگر (new->next)
        new->next->pprev = &new->next;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#define hlist_for_each_entry(pos, head, member)                         \
    برای (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(پوچ, typeof(*pos), member)              \
         ; pos = container_of(pos->member.next, typeof(*pos), member))

#define hlist_for_each_entry_safe(pos, n, head, member)                 \
    برای (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(پوچ, typeof(*pos), member)              \
             && ({ n = pos->member.next; 1; })                          \
         ; pos = container_of(n, typeof(*pos), member))

#define hlist_for_each_entry_pprev(pos, pprev, head, member)            \
    برای (pprev = &(head)->first                                         \
         ; *pprev && ({ pos=container_of(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->next)


#endif // list.h

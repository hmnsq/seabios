#ifndef __LIST_H
#define __LIST_H

#ضمن "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

بنية hlist_عقدة {
    بنية hlist_عقدة *next, **pprev;
};

بنية hlist_head {
    بنية hlist_عقدة *first;
};

ساكن مضمن صحيح
hlist_empty(ثابت بنية hlist_head *h)
{
    أرجع !h->first;
}

ساكن مضمن فراغ
hlist_del(بنية hlist_عقدة *n)
{
    بنية hlist_عقدة *next = n->next;
    بنية hlist_عقدة **pprev = n->pprev;
    *pprev = next;
    إذا (next)
        next->pprev = pprev;
}

ساكن مضمن فراغ
hlist_add(بنية hlist_عقدة *n, بنية hlist_عقدة **pprev)
{
    بنية hlist_عقدة *next = *pprev;
    n->pprev = pprev;
    n->next = next;
    إذا (next)
        next->pprev = &n->next;
    *pprev = n;
}

ساكن مضمن فراغ
hlist_add_head(بنية hlist_عقدة *n, بنية hlist_head *h)
{
    hlist_add(n, &h->first);
}

ساكن مضمن فراغ
hlist_add_before(بنية hlist_عقدة *n, بنية hlist_عقدة *next)
{
    hlist_add(n, next->pprev);
}

ساكن مضمن فراغ
hlist_add_after(بنية hlist_عقدة *n, بنية hlist_عقدة *prev)
{
    hlist_add(n, &prev->next);
}

ساكن مضمن فراغ
hlist_replace(بنية hlist_عقدة *old, بنية hlist_عقدة *new)
{
    new->next = old->next;
    إذا (new->next)
        new->next->pprev = &new->next;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#define hlist_for_each_entry(pos, head, member)                         \
    لكل (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(عدم, typeof(*pos), member)              \
         ; pos = container_of(pos->member.next, typeof(*pos), member))

#define hlist_for_each_entry_safe(pos, n, head, member)                 \
    لكل (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(عدم, typeof(*pos), member)              \
             && ({ n = pos->member.next; 1; })                          \
         ; pos = container_of(n, typeof(*pos), member))

#define hlist_for_each_entry_pprev(pos, pprev, head, member)            \
    لكل (pprev = &(head)->first                                         \
         ; *pprev && ({ pos=container_of(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->next)


#endif // list.h

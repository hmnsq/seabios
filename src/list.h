#ifndef __LIST_H
#define __LIST_H

#include "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

sa_nracanaa hlist_node {
    sa_nracanaa hlist_node *next, **pprev;
};

sa_nracanaa hlist_head {
    sa_nracanaa hlist_node *first;
};

static anta_hstha ຈຳນວນເຕັມ
hlist_empty(const sa_nracanaa hlist_head *h)
{
    ສົ່ງຄືນ !h->first;
}

static anta_hstha void
hlist_del(sa_nracanaa hlist_node *n)
{
    sa_nracanaa hlist_node *next = n->next;
    sa_nracanaa hlist_node **pprev = n->pprev;
    *pprev = next;
    ຖ້າ (next)
        next->pprev = pprev;
}

static anta_hstha void
hlist_add(sa_nracanaa hlist_node *n, sa_nracanaa hlist_node **pprev)
{
    sa_nracanaa hlist_node *next = *pprev;
    n->pprev = pprev;
    n->next = next;
    ຖ້າ (next)
        next->pprev = &n->next;
    *pprev = n;
}

static anta_hstha void
hlist_add_head(sa_nracanaa hlist_node *n, sa_nracanaa hlist_head *h)
{
    hlist_add(n, &h->first);
}

static anta_hstha void
hlist_add_before(sa_nracanaa hlist_node *n, sa_nracanaa hlist_node *next)
{
    hlist_add(n, next->pprev);
}

static anta_hstha void
hlist_add_after(sa_nracanaa hlist_node *n, sa_nracanaa hlist_node *prev)
{
    hlist_add(n, &prev->next);
}

static anta_hstha void
hlist_replace(sa_nracanaa hlist_node *old, sa_nracanaa hlist_node *new)
{
    new->next = old->next;
    ຖ້າ (new->next)
        new->next->pprev = &new->next;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#define hlist_for_each_entry(pos, head, member)                         \
    ສຳລັບ (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(NULL, typeof(*pos), member)              \
         ; pos = container_of(pos->member.next, typeof(*pos), member))

#define hlist_for_each_entry_safe(pos, n, head, member)                 \
    ສຳລັບ (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(NULL, typeof(*pos), member)              \
             && ({ n = pos->member.next; 1; })                          \
         ; pos = container_of(n, typeof(*pos), member))

#define hlist_for_each_entry_pprev(pos, pprev, head, member)            \
    ສຳລັບ (pprev = &(head)->first                                         \
         ; *pprev && ({ pos=container_of(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->next)


#endif // list.h

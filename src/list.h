#ifndef __LIST_H
#define __LIST_H

#ଅନ୍ତର୍ଭାଵ "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

ସଂରଚନା hlist_ଗଣ୍ଠି {
    ସଂରଚନା hlist_ଗଣ୍ଠି *next, **pprev;
};

ସଂରଚନା hlist_head {
    ସଂରଚନା hlist_ଗଣ୍ଠି *first;
};

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା
hlist_empty(ଅଚଳ ସଂରଚନା hlist_head *h)
{
    ଫେରନ୍ତୁ !h->first;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
hlist_del(ସଂରଚନା hlist_ଗଣ୍ଠି *n)
{
    ସଂରଚନା hlist_ଗଣ୍ଠି *next = n->next;
    ସଂରଚନା hlist_ଗଣ୍ଠି **pprev = n->pprev;
    *pprev = next;
    ଯଦି (next)
        next->pprev = pprev;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
hlist_add(ସଂରଚନା hlist_ଗଣ୍ଠି *n, ସଂରଚନା hlist_ଗଣ୍ଠି **pprev)
{
    ସଂରଚନା hlist_ଗଣ୍ଠି *next = *pprev;
    n->pprev = pprev;
    n->next = next;
    ଯଦି (next)
        next->pprev = &n->next;
    *pprev = n;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
hlist_add_head(ସଂରଚନା hlist_ଗଣ୍ଠି *n, ସଂରଚନା hlist_head *h)
{
    hlist_add(n, &h->first);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
hlist_add_before(ସଂରଚନା hlist_ଗଣ୍ଠି *n, ସଂରଚନା hlist_ଗଣ୍ଠି *next)
{
    hlist_add(n, next->pprev);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
hlist_add_after(ସଂରଚନା hlist_ଗଣ୍ଠି *n, ସଂରଚନା hlist_ଗଣ୍ଠି *prev)
{
    hlist_add(n, &prev->next);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
hlist_replace(ସଂରଚନା hlist_ଗଣ୍ଠି *old, ସଂରଚନା hlist_ଗଣ୍ଠି *new)
{
    new->next = old->next;
    ଯଦି (new->next)
        new->next->pprev = &new->next;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#define hlist_for_each_entry(pos, head, member)                         \
    ପାଇଁ (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(ଖାଲି, typeof(*pos), member)              \
         ; pos = container_of(pos->member.next, typeof(*pos), member))

#define hlist_for_each_entry_safe(pos, n, head, member)                 \
    ପାଇଁ (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(ଖାଲି, typeof(*pos), member)              \
             && ({ n = pos->member.next; 1; })                          \
         ; pos = container_of(n, typeof(*pos), member))

#define hlist_for_each_entry_pprev(pos, pprev, head, member)            \
    ପାଇଁ (pprev = &(head)->first                                         \
         ; *pprev && ({ pos=container_of(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->next)


#endif // list.h

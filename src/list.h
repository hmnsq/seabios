#ifndef __LIST_H
#define __LIST_H

#અન્તર્ભાવ "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

સંરચના hlist_ગાંઠ {
    સંરચના hlist_ગાંઠ *next, **pprev;
};

સંરચના hlist_head {
    સંરચના hlist_ગાંઠ *first;
};

સ્થિર અંતઃરેખ પૂર્ણાંક
hlist_empty(અચલ સંરચના hlist_head *h)
{
    પાછા !h->first;
}

સ્થિર અંતઃરેખ શૂન્ય
hlist_del(સંરચના hlist_ગાંઠ *n)
{
    સંરચના hlist_ગાંઠ *next = n->next;
    સંરચના hlist_ગાંઠ **pprev = n->pprev;
    *pprev = next;
    જો (next)
        next->pprev = pprev;
}

સ્થિર અંતઃરેખ શૂન્ય
hlist_add(સંરચના hlist_ગાંઠ *n, સંરચના hlist_ગાંઠ **pprev)
{
    સંરચના hlist_ગાંઠ *next = *pprev;
    n->pprev = pprev;
    n->next = next;
    જો (next)
        next->pprev = &n->next;
    *pprev = n;
}

સ્થિર અંતઃરેખ શૂન્ય
hlist_add_head(સંરચના hlist_ગાંઠ *n, સંરચના hlist_head *h)
{
    hlist_add(n, &h->first);
}

સ્થિર અંતઃરેખ શૂન્ય
hlist_add_before(સંરચના hlist_ગાંઠ *n, સંરચના hlist_ગાંઠ *next)
{
    hlist_add(n, next->pprev);
}

સ્થિર અંતઃરેખ શૂન્ય
hlist_add_after(સંરચના hlist_ગાંઠ *n, સંરચના hlist_ગાંઠ *prev)
{
    hlist_add(n, &prev->next);
}

સ્થિર અંતઃરેખ શૂન્ય
hlist_replace(સંરચના hlist_ગાંઠ *old, સંરચના hlist_ગાંઠ *new)
{
    new->next = old->next;
    જો (new->next)
        new->next->pprev = &new->next;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#define hlist_for_each_entry(pos, head, member)                         \
    માટે (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(ખાલી, typeof(*pos), member)              \
         ; pos = container_of(pos->member.next, typeof(*pos), member))

#define hlist_for_each_entry_safe(pos, n, head, member)                 \
    માટે (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(ખાલી, typeof(*pos), member)              \
             && ({ n = pos->member.next; 1; })                          \
         ; pos = container_of(n, typeof(*pos), member))

#define hlist_for_each_entry_pprev(pos, pprev, head, member)            \
    માટે (pprev = &(head)->first                                         \
         ; *pprev && ({ pos=container_of(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->next)


#endif // list.h

#ifndef __LIST_H
#define __LIST_H

#അന്തര്ഭാവ "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

ഘടന hlist_കണ്ണി {
    ഘടന hlist_കണ്ണി *next, **pprev;
};

ഘടന hlist_head {
    ഘടന hlist_കണ്ണി *first;
};

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ
hlist_empty(സ്ഥിരം ഘടന hlist_head *h)
{
    തിരികെ !h->first;
}

സ്ഥിതം വരിയിൽ ശൂന്യം
hlist_del(ഘടന hlist_കണ്ണി *n)
{
    ഘടന hlist_കണ്ണി *next = n->next;
    ഘടന hlist_കണ്ണി **pprev = n->pprev;
    *pprev = next;
    എങ്കിൽ (next)
        next->pprev = pprev;
}

സ്ഥിതം വരിയിൽ ശൂന്യം
hlist_add(ഘടന hlist_കണ്ണി *n, ഘടന hlist_കണ്ണി **pprev)
{
    ഘടന hlist_കണ്ണി *next = *pprev;
    n->pprev = pprev;
    n->next = next;
    എങ്കിൽ (next)
        next->pprev = &n->next;
    *pprev = n;
}

സ്ഥിതം വരിയിൽ ശൂന്യം
hlist_add_head(ഘടന hlist_കണ്ണി *n, ഘടന hlist_head *h)
{
    hlist_add(n, &h->first);
}

സ്ഥിതം വരിയിൽ ശൂന്യം
hlist_add_before(ഘടന hlist_കണ്ണി *n, ഘടന hlist_കണ്ണി *next)
{
    hlist_add(n, next->pprev);
}

സ്ഥിതം വരിയിൽ ശൂന്യം
hlist_add_after(ഘടന hlist_കണ്ണി *n, ഘടന hlist_കണ്ണി *prev)
{
    hlist_add(n, &prev->next);
}

സ്ഥിതം വരിയിൽ ശൂന്യം
hlist_replace(ഘടന hlist_കണ്ണി *old, ഘടന hlist_കണ്ണി *new)
{
    new->next = old->next;
    എങ്കിൽ (new->next)
        new->next->pprev = &new->next;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#define hlist_for_each_entry(pos, head, member)                         \
    വേണ്ടി (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(ശൂന്യ, typeof(*pos), member)              \
         ; pos = container_of(pos->member.next, typeof(*pos), member))

#define hlist_for_each_entry_safe(pos, n, head, member)                 \
    വേണ്ടി (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(ശൂന്യ, typeof(*pos), member)              \
             && ({ n = pos->member.next; 1; })                          \
         ; pos = container_of(n, typeof(*pos), member))

#define hlist_for_each_entry_pprev(pos, pprev, head, member)            \
    വേണ്ടി (pprev = &(head)->first                                         \
         ; *pprev && ({ pos=container_of(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->next)


#endif // list.h

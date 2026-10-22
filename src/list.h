#ifndef __LIST_H
#define __LIST_H

#ಅನ್ತರ್ಭಾವ "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

ಸಂರಚನಾ hlist_ಗಂಟು {
    ಸಂರಚನಾ hlist_ಗಂಟು *next, **pprev;
};

ಸಂರಚನಾ hlist_head {
    ಸಂರಚನಾ hlist_ಗಂಟು *first;
};

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ
hlist_empty(ಧ್ರುವ ಸಂರಚನಾ hlist_head *h)
{
    ಪ್ರತ್ಯಾಗಮ !h->first;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
hlist_del(ಸಂರಚನಾ hlist_ಗಂಟು *n)
{
    ಸಂರಚನಾ hlist_ಗಂಟು *next = n->next;
    ಸಂರಚನಾ hlist_ಗಂಟು **pprev = n->pprev;
    *pprev = next;
    ಯದಿ (next)
        next->pprev = pprev;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
hlist_add(ಸಂರಚನಾ hlist_ಗಂಟು *n, ಸಂರಚನಾ hlist_ಗಂಟು **pprev)
{
    ಸಂರಚನಾ hlist_ಗಂಟು *next = *pprev;
    n->pprev = pprev;
    n->next = next;
    ಯದಿ (next)
        next->pprev = &n->next;
    *pprev = n;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
hlist_add_head(ಸಂರಚನಾ hlist_ಗಂಟು *n, ಸಂರಚನಾ hlist_head *h)
{
    hlist_add(n, &h->first);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
hlist_add_before(ಸಂರಚನಾ hlist_ಗಂಟು *n, ಸಂರಚನಾ hlist_ಗಂಟು *next)
{
    hlist_add(n, next->pprev);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
hlist_add_after(ಸಂರಚನಾ hlist_ಗಂಟು *n, ಸಂರಚನಾ hlist_ಗಂಟು *prev)
{
    hlist_add(n, &prev->next);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
hlist_replace(ಸಂರಚನಾ hlist_ಗಂಟು *old, ಸಂರಚನಾ hlist_ಗಂಟು *new)
{
    new->next = old->next;
    ಯದಿ (new->next)
        new->next->pprev = &new->next;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#define hlist_for_each_entry(pos, head, member)                         \
    ಪ್ರತಿ (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(ರಿಕ್ತ, typeof(*pos), member)              \
         ; pos = container_of(pos->member.next, typeof(*pos), member))

#define hlist_for_each_entry_safe(pos, n, head, member)                 \
    ಪ್ರತಿ (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(ರಿಕ್ತ, typeof(*pos), member)              \
             && ({ n = pos->member.next; 1; })                          \
         ; pos = container_of(n, typeof(*pos), member))

#define hlist_for_each_entry_pprev(pos, pprev, head, member)            \
    ಪ್ರತಿ (pprev = &(head)->first                                         \
         ; *pprev && ({ pos=container_of(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->next)


#endif // list.h

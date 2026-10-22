#ifndef __LIST_H
#define __LIST_H

#अन्तर्भाव "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

संरचना hlist_गाठ {
    संरचना hlist_गाठ *next, **pprev;
};

संरचना hlist_head {
    संरचना hlist_गाठ *first;
};

स्थिर ओळीत पूर्णांक
hlist_empty(स्थिरांक संरचना hlist_head *h)
{
    परत !h->first;
}

स्थिर ओळीत शून्य
hlist_del(संरचना hlist_गाठ *n)
{
    संरचना hlist_गाठ *next = n->next;
    संरचना hlist_गाठ **pprev = n->pprev;
    *pprev = next;
    जर (next)
        next->pprev = pprev;
}

स्थिर ओळीत शून्य
hlist_add(संरचना hlist_गाठ *n, संरचना hlist_गाठ **pprev)
{
    संरचना hlist_गाठ *next = *pprev;
    n->pprev = pprev;
    n->next = next;
    जर (next)
        next->pprev = &n->next;
    *pprev = n;
}

स्थिर ओळीत शून्य
hlist_add_head(संरचना hlist_गाठ *n, संरचना hlist_head *h)
{
    hlist_add(n, &h->first);
}

स्थिर ओळीत शून्य
hlist_add_before(संरचना hlist_गाठ *n, संरचना hlist_गाठ *next)
{
    hlist_add(n, next->pprev);
}

स्थिर ओळीत शून्य
hlist_add_after(संरचना hlist_गाठ *n, संरचना hlist_गाठ *prev)
{
    hlist_add(n, &prev->next);
}

स्थिर ओळीत शून्य
hlist_replace(संरचना hlist_गाठ *old, संरचना hlist_गाठ *new)
{
    new->next = old->next;
    जर (new->next)
        new->next->pprev = &new->next;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#define hlist_for_each_entry(pos, head, member)                         \
    साठी (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(रिक्त, typeof(*pos), member)              \
         ; pos = container_of(pos->member.next, typeof(*pos), member))

#define hlist_for_each_entry_safe(pos, n, head, member)                 \
    साठी (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(रिक्त, typeof(*pos), member)              \
             && ({ n = pos->member.next; 1; })                          \
         ; pos = container_of(n, typeof(*pos), member))

#define hlist_for_each_entry_pprev(pos, pprev, head, member)            \
    साठी (pprev = &(head)->first                                         \
         ; *pprev && ({ pos=container_of(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->next)


#endif // list.h

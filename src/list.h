#ifndef __LIST_H
#define __LIST_H

#अन्तर्भाव "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

संरचना hlist_गाँठो {
    संरचना hlist_गाँठो *next, **pprev;
};

संरचना hlist_head {
    संरचना hlist_गाँठो *first;
};

स्थिर अन्तःस्थ पूर्णाङ्क
hlist_empty(अचर संरचना hlist_head *h)
{
    फर्काऊ !h->first;
}

स्थिर अन्तःस्थ शून्य
hlist_del(संरचना hlist_गाँठो *n)
{
    संरचना hlist_गाँठो *next = n->next;
    संरचना hlist_गाँठो **pprev = n->pprev;
    *pprev = next;
    यदि (next)
        next->pprev = pprev;
}

स्थिर अन्तःस्थ शून्य
hlist_add(संरचना hlist_गाँठो *n, संरचना hlist_गाँठो **pprev)
{
    संरचना hlist_गाँठो *next = *pprev;
    n->pprev = pprev;
    n->next = next;
    यदि (next)
        next->pprev = &n->next;
    *pprev = n;
}

स्थिर अन्तःस्थ शून्य
hlist_add_head(संरचना hlist_गाँठो *n, संरचना hlist_head *h)
{
    hlist_add(n, &h->first);
}

स्थिर अन्तःस्थ शून्य
hlist_add_before(संरचना hlist_गाँठो *n, संरचना hlist_गाँठो *next)
{
    hlist_add(n, next->pprev);
}

स्थिर अन्तःस्थ शून्य
hlist_add_after(संरचना hlist_गाँठो *n, संरचना hlist_गाँठो *prev)
{
    hlist_add(n, &prev->next);
}

स्थिर अन्तःस्थ शून्य
hlist_replace(संरचना hlist_गाँठो *old, संरचना hlist_गाँठो *new)
{
    new->next = old->next;
    यदि (new->next)
        new->next->pprev = &new->next;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#define hlist_for_each_entry(pos, head, member)                         \
    लागि (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(रिक्त, typeof(*pos), member)              \
         ; pos = container_of(pos->member.next, typeof(*pos), member))

#define hlist_for_each_entry_safe(pos, n, head, member)                 \
    लागि (pos = container_of((head)->first, typeof(*pos), member)        \
         ; pos != container_of(रिक्त, typeof(*pos), member)              \
             && ({ n = pos->member.next; 1; })                          \
         ; pos = container_of(n, typeof(*pos), member))

#define hlist_for_each_entry_pprev(pos, pprev, head, member)            \
    लागि (pprev = &(head)->first                                         \
         ; *pprev && ({ pos=container_of(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->next)


#endif // list.h

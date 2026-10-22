#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __LIST_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __LIST_H

#ਅਨ੍ਤਰ੍ਭਾਵ "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

ਬਣਤਰ hlist_ਗੰਢ {
    ਬਣਤਰ hlist_ਗੰਢ *ਅਗਲਾ, **pprev;
};

ਬਣਤਰ hlist_ਸਿਰ {
    ਬਣਤਰ hlist_ਗੰਢ *ਪਹਿਲਾ;
};

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ
hlist_ਖਾਲੀ_ਥਾਵਾਂ(ਅਚਲ ਬਣਤਰ hlist_ਸਿਰ *h)
{
    ਵਾਪਸ !h->ਪਹਿਲਾ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
hlist_del(ਬਣਤਰ hlist_ਗੰਢ *n)
{
    ਬਣਤਰ hlist_ਗੰਢ *ਅਗਲਾ = n->ਅਗਲਾ;
    ਬਣਤਰ hlist_ਗੰਢ **pprev = n->pprev;
    *pprev = ਅਗਲਾ;
    ਜੇ (ਅਗਲਾ)
        ਅਗਲਾ->pprev = pprev;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
hlist_ਜੋੜੋ(ਬਣਤਰ hlist_ਗੰਢ *n, ਬਣਤਰ hlist_ਗੰਢ **pprev)
{
    ਬਣਤਰ hlist_ਗੰਢ *ਅਗਲਾ = *pprev;
    n->pprev = pprev;
    n->ਅਗਲਾ = ਅਗਲਾ;
    ਜੇ (ਅਗਲਾ)
        ਅਗਲਾ->pprev = &n->ਅਗਲਾ;
    *pprev = n;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
hlist_ਜੋੜੋ_ਸਿਰ(ਬਣਤਰ hlist_ਗੰਢ *n, ਬਣਤਰ hlist_ਸਿਰ *h)
{
    hlist_ਜੋੜੋ(n, &h->ਪਹਿਲਾ);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
hlist_ਜੋੜੋ_before(ਬਣਤਰ hlist_ਗੰਢ *n, ਬਣਤਰ hlist_ਗੰਢ *ਅਗਲਾ)
{
    hlist_ਜੋੜੋ(n, ਅਗਲਾ->pprev);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
hlist_ਜੋੜੋ_after(ਬਣਤਰ hlist_ਗੰਢ *n, ਬਣਤਰ hlist_ਗੰਢ *prev)
{
    hlist_ਜੋੜੋ(n, &prev->ਅਗਲਾ);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
hlist_replace(ਬਣਤਰ hlist_ਗੰਢ *old, ਬਣਤਰ hlist_ਗੰਢ *new)
{
    new->ਅਗਲਾ = old->ਅਗਲਾ;
    ਜੇ (new->ਅਗਲਾ)
        new->ਅਗਲਾ->pprev = &new->ਅਗਲਾ;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ hlist_for_each_ਇੰਦਰਾਜ(pos, ਸਿਰ, member)                         \
    ਲਈ (pos = container_ਦਾ((ਸਿਰ)->ਪਹਿਲਾ, typeof(*pos), member)        \
         ; pos != container_ਦਾ(ਨਿਰਾ, typeof(*pos), member)              \
         ; pos = container_ਦਾ(pos->member.ਅਗਲਾ, typeof(*pos), member))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ hlist_for_each_ਇੰਦਰਾਜ_ਸੁਰੱਖਿਅਤ(pos, n, ਸਿਰ, member)                 \
    ਲਈ (pos = container_ਦਾ((ਸਿਰ)->ਪਹਿਲਾ, typeof(*pos), member)        \
         ; pos != container_ਦਾ(ਨਿਰਾ, typeof(*pos), member)              \
             && ({ n = pos->member.ਅਗਲਾ; 1; })                          \
         ; pos = container_ਦਾ(n, typeof(*pos), member))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ hlist_for_each_ਇੰਦਰਾਜ_pprev(pos, pprev, ਸਿਰ, member)            \
    ਲਈ (pprev = &(ਸਿਰ)->ਪਹਿਲਾ                                         \
         ; *pprev && ({ pos=container_ਦਾ(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->ਅਗਲਾ)


#ਜੇ_ਅੰਤ // list.h

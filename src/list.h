#جيڪڏهن_وصف_نه_ٿيل __LIST_H
#وصف_ڪريو __LIST_H

#ضمن "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

بنية hlist_ڳنڍ {
    بنية hlist_ڳنڍ *ايندڙ, **pprev;
};

بنية hlist_سر {
    بنية hlist_ڳنڍ *پهريون;
};

ساكن مضمن صحيح
hlist_خالي_جايون(ثابت بنية hlist_سر *h)
{
    أرجع !h->پهريون;
}

ساكن مضمن فراغ
hlist_del(بنية hlist_ڳنڍ *n)
{
    بنية hlist_ڳنڍ *ايندڙ = n->ايندڙ;
    بنية hlist_ڳنڍ **pprev = n->pprev;
    *pprev = ايندڙ;
    إذا (ايندڙ)
        ايندڙ->pprev = pprev;
}

ساكن مضمن فراغ
hlist_جوڙيو(بنية hlist_ڳنڍ *n, بنية hlist_ڳنڍ **pprev)
{
    بنية hlist_ڳنڍ *ايندڙ = *pprev;
    n->pprev = pprev;
    n->ايندڙ = ايندڙ;
    إذا (ايندڙ)
        ايندڙ->pprev = &n->ايندڙ;
    *pprev = n;
}

ساكن مضمن فراغ
hlist_جوڙيو_سر(بنية hlist_ڳنڍ *n, بنية hlist_سر *h)
{
    hlist_جوڙيو(n, &h->پهريون);
}

ساكن مضمن فراغ
hlist_جوڙيو_before(بنية hlist_ڳنڍ *n, بنية hlist_ڳنڍ *ايندڙ)
{
    hlist_جوڙيو(n, ايندڙ->pprev);
}

ساكن مضمن فراغ
hlist_جوڙيو_after(بنية hlist_ڳنڍ *n, بنية hlist_ڳنڍ *prev)
{
    hlist_جوڙيو(n, &prev->ايندڙ);
}

ساكن مضمن فراغ
hlist_replace(بنية hlist_ڳنڍ *old, بنية hlist_ڳنڍ *new)
{
    new->ايندڙ = old->ايندڙ;
    إذا (new->ايندڙ)
        new->ايندڙ->pprev = &new->ايندڙ;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#وصف_ڪريو hlist_for_each_داخلا(pos, سر, member)                         \
    لكل (pos = container_جو((سر)->پهريون, typeof(*pos), member)        \
         ; pos != container_جو(عدم, typeof(*pos), member)              \
         ; pos = container_جو(pos->member.ايندڙ, typeof(*pos), member))

#وصف_ڪريو hlist_for_each_داخلا_محفوظ(pos, n, سر, member)                 \
    لكل (pos = container_جو((سر)->پهريون, typeof(*pos), member)        \
         ; pos != container_جو(عدم, typeof(*pos), member)              \
             && ({ n = pos->member.ايندڙ; 1; })                          \
         ; pos = container_جو(n, typeof(*pos), member))

#وصف_ڪريو hlist_for_each_داخلا_pprev(pos, pprev, سر, member)            \
    لكل (pprev = &(سر)->پهريون                                         \
         ; *pprev && ({ pos=container_جو(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->ايندڙ)


#جيڪڏهن_پورو // list.h

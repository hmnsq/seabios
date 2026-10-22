#جے_بنا_تعریف __LIST_H
#تعریف_کرو __LIST_H

#ضمن "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

بنية hlist_گرہ {
    بنية hlist_گرہ *اگلا, **pprev;
};

بنية hlist_سر {
    بنية hlist_گرہ *پہلا;
};

ساكن مضمن صحيح
hlist_خالی_تھاواں(ثابت بنية hlist_سر *h)
{
    أرجع !h->پہلا;
}

ساكن مضمن فراغ
hlist_del(بنية hlist_گرہ *n)
{
    بنية hlist_گرہ *اگلا = n->اگلا;
    بنية hlist_گرہ **pprev = n->pprev;
    *pprev = اگلا;
    إذا (اگلا)
        اگلا->pprev = pprev;
}

ساكن مضمن فراغ
hlist_جوڑو(بنية hlist_گرہ *n, بنية hlist_گرہ **pprev)
{
    بنية hlist_گرہ *اگلا = *pprev;
    n->pprev = pprev;
    n->اگلا = اگلا;
    إذا (اگلا)
        اگلا->pprev = &n->اگلا;
    *pprev = n;
}

ساكن مضمن فراغ
hlist_جوڑو_سر(بنية hlist_گرہ *n, بنية hlist_سر *h)
{
    hlist_جوڑو(n, &h->پہلا);
}

ساكن مضمن فراغ
hlist_جوڑو_before(بنية hlist_گرہ *n, بنية hlist_گرہ *اگلا)
{
    hlist_جوڑو(n, اگلا->pprev);
}

ساكن مضمن فراغ
hlist_جوڑو_after(بنية hlist_گرہ *n, بنية hlist_گرہ *prev)
{
    hlist_جوڑو(n, &prev->اگلا);
}

ساكن مضمن فراغ
hlist_replace(بنية hlist_گرہ *old, بنية hlist_گرہ *new)
{
    new->اگلا = old->اگلا;
    إذا (new->اگلا)
        new->اگلا->pprev = &new->اگلا;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#تعریف_کرو hlist_for_each_اندراج(pos, سر, member)                         \
    لكل (pos = container_دا((سر)->پہلا, typeof(*pos), member)        \
         ; pos != container_دا(عدم, typeof(*pos), member)              \
         ; pos = container_دا(pos->member.اگلا, typeof(*pos), member))

#تعریف_کرو hlist_for_each_اندراج_محفوظ(pos, n, سر, member)                 \
    لكل (pos = container_دا((سر)->پہلا, typeof(*pos), member)        \
         ; pos != container_دا(عدم, typeof(*pos), member)              \
             && ({ n = pos->member.اگلا; 1; })                          \
         ; pos = container_دا(n, typeof(*pos), member))

#تعریف_کرو hlist_for_each_اندراج_pprev(pos, pprev, سر, member)            \
    لكل (pprev = &(سر)->پہلا                                         \
         ; *pprev && ({ pos=container_دا(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->اگلا)


#جے_انت // list.h

#اگر_غیر_تعریف_شدہ __LIST_H
#تعریف_کرو __LIST_H

#شامل "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

ساخت hlist_گرہ {
    ساخت hlist_گرہ *اگلا, **pprev;
};

ساخت hlist_سر {
    ساخت hlist_گرہ *پہلا;
};

ساکن اندرونی صحیح
hlist_خالی_خانے(ثابت ساخت hlist_سر *h)
{
    واپس !h->پہلا;
}

ساکن اندرونی خالی
hlist_del(ساخت hlist_گرہ *n)
{
    ساخت hlist_گرہ *اگلا = n->اگلا;
    ساخت hlist_گرہ **pprev = n->pprev;
    *pprev = اگلا;
    اگر (اگلا)
        اگلا->pprev = pprev;
}

ساکن اندرونی خالی
hlist_جوڑو(ساخت hlist_گرہ *n, ساخت hlist_گرہ **pprev)
{
    ساخت hlist_گرہ *اگلا = *pprev;
    n->pprev = pprev;
    n->اگلا = اگلا;
    اگر (اگلا)
        اگلا->pprev = &n->اگلا;
    *pprev = n;
}

ساکن اندرونی خالی
hlist_جوڑو_سر(ساخت hlist_گرہ *n, ساخت hlist_سر *h)
{
    hlist_جوڑو(n, &h->پہلا);
}

ساکن اندرونی خالی
hlist_جوڑو_before(ساخت hlist_گرہ *n, ساخت hlist_گرہ *اگلا)
{
    hlist_جوڑو(n, اگلا->pprev);
}

ساکن اندرونی خالی
hlist_جوڑو_after(ساخت hlist_گرہ *n, ساخت hlist_گرہ *پچھلا)
{
    hlist_جوڑو(n, &پچھلا->اگلا);
}

ساکن اندرونی خالی
hlist_replace(ساخت hlist_گرہ *old, ساخت hlist_گرہ *new)
{
    new->اگلا = old->اگلا;
    اگر (new->اگلا)
        new->اگلا->pprev = &new->اگلا;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#تعریف_کرو hlist_for_each_اندراج(pos, سر, member)                         \
    برائے (pos = container_کا((سر)->پہلا, typeof(*pos), member)        \
         ; pos != container_کا(تہی, typeof(*pos), member)              \
         ; pos = container_کا(pos->member.اگلا, typeof(*pos), member))

#تعریف_کرو hlist_for_each_اندراج_محفوظ(pos, n, سر, member)                 \
    برائے (pos = container_کا((سر)->پہلا, typeof(*pos), member)        \
         ; pos != container_کا(تہی, typeof(*pos), member)              \
             && ({ n = pos->member.اگلا; 1; })                          \
         ; pos = container_کا(n, typeof(*pos), member))

#تعریف_کرو hlist_for_each_اندراج_pprev(pos, pprev, سر, member)            \
    برائے (pprev = &(سر)->پہلا                                         \
         ; *pprev && ({ pos=container_کا(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->اگلا)


#اگر_ختم // list.h

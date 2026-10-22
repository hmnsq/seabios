#اگر_تعریف_نشده __LIST_H
#تعریف_کن __LIST_H

#ضمن "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

بنية hlist_گره {
    بنية hlist_گره *بعدی, **pprev;
};

بنية hlist_سر {
    بنية hlist_گره *اولی;
};

ساكن مضمن صحيح
hlist_خالی(ثابت بنية hlist_سر *h)
{
    أرجع !h->اولی;
}

ساكن مضمن فراغ
hlist_del(بنية hlist_گره *n)
{
    بنية hlist_گره *بعدی = n->بعدی;
    بنية hlist_گره **pprev = n->pprev;
    *pprev = بعدی;
    إذا (بعدی)
        بعدی->pprev = pprev;
}

ساكن مضمن فراغ
hlist_جمع_کن(بنية hlist_گره *n, بنية hlist_گره **pprev)
{
    بنية hlist_گره *بعدی = *pprev;
    n->pprev = pprev;
    n->بعدی = بعدی;
    إذا (بعدی)
        بعدی->pprev = &n->بعدی;
    *pprev = n;
}

ساكن مضمن فراغ
hlist_جمع_کن_سر(بنية hlist_گره *n, بنية hlist_سر *h)
{
    hlist_جمع_کن(n, &h->اولی);
}

ساكن مضمن فراغ
hlist_جمع_کن_before(بنية hlist_گره *n, بنية hlist_گره *بعدی)
{
    hlist_جمع_کن(n, بعدی->pprev);
}

ساكن مضمن فراغ
hlist_جمع_کن_after(بنية hlist_گره *n, بنية hlist_گره *prev)
{
    hlist_جمع_کن(n, &prev->بعدی);
}

ساكن مضمن فراغ
hlist_replace(بنية hlist_گره *old, بنية hlist_گره *new)
{
    new->بعدی = old->بعدی;
    إذا (new->بعدی)
        new->بعدی->pprev = &new->بعدی;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#تعریف_کن hlist_for_each_مدخل(pos, سر, member)                         \
    لكل (pos = container_از((سر)->اولی, typeof(*pos), member)        \
         ; pos != container_از(عدم, typeof(*pos), member)              \
         ; pos = container_از(pos->member.بعدی, typeof(*pos), member))

#تعریف_کن hlist_for_each_مدخل_امن(pos, n, سر, member)                 \
    لكل (pos = container_از((سر)->اولی, typeof(*pos), member)        \
         ; pos != container_از(عدم, typeof(*pos), member)              \
             && ({ n = pos->member.بعدی; 1; })                          \
         ; pos = container_از(n, typeof(*pos), member))

#تعریف_کن hlist_for_each_مدخل_pprev(pos, pprev, سر, member)            \
    لكل (pprev = &(سر)->اولی                                         \
         ; *pprev && ({ pos=container_از(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->بعدی)


#پایان_اگر // list.h

#که_نه_تعریف_شوی __LIST_H
#تعریف_کړه __LIST_H

#ضمن "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

بنية hlist_غوټه {
    بنية hlist_غوټه *بل, **pprev;
};

بنية hlist_سر {
    بنية hlist_غوټه *لومړی;
};

ساكن مضمن صحيح
hlist_تشه(ثابت بنية hlist_سر *h)
{
    أرجع !h->لومړی;
}

ساكن مضمن فراغ
hlist_del(بنية hlist_غوټه *n)
{
    بنية hlist_غوټه *بل = n->بل;
    بنية hlist_غوټه **pprev = n->pprev;
    *pprev = بل;
    إذا (بل)
        بل->pprev = pprev;
}

ساكن مضمن فراغ
hlist_جمع_کړه(بنية hlist_غوټه *n, بنية hlist_غوټه **pprev)
{
    بنية hlist_غوټه *بل = *pprev;
    n->pprev = pprev;
    n->بل = بل;
    إذا (بل)
        بل->pprev = &n->بل;
    *pprev = n;
}

ساكن مضمن فراغ
hlist_جمع_کړه_سر(بنية hlist_غوټه *n, بنية hlist_سر *h)
{
    hlist_جمع_کړه(n, &h->لومړی);
}

ساكن مضمن فراغ
hlist_جمع_کړه_before(بنية hlist_غوټه *n, بنية hlist_غوټه *بل)
{
    hlist_جمع_کړه(n, بل->pprev);
}

ساكن مضمن فراغ
hlist_جمع_کړه_after(بنية hlist_غوټه *n, بنية hlist_غوټه *prev)
{
    hlist_جمع_کړه(n, &prev->بل);
}

ساكن مضمن فراغ
hlist_replace(بنية hlist_غوټه *old, بنية hlist_غوټه *new)
{
    new->بل = old->بل;
    إذا (new->بل)
        new->بل->pprev = &new->بل;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#تعریف_کړه hlist_for_each_ثبت(pos, سر, member)                         \
    لكل (pos = container_د((سر)->لومړی, typeof(*pos), member)        \
         ; pos != container_د(عدم, typeof(*pos), member)              \
         ; pos = container_د(pos->member.بل, typeof(*pos), member))

#تعریف_کړه hlist_for_each_ثبت_خوندي(pos, n, سر, member)                 \
    لكل (pos = container_د((سر)->لومړی, typeof(*pos), member)        \
         ; pos != container_د(عدم, typeof(*pos), member)              \
             && ({ n = pos->member.بل; 1; })                          \
         ; pos = container_د(n, typeof(*pos), member))

#تعریف_کړه hlist_for_each_ثبت_pprev(pos, pprev, سر, member)            \
    لكل (pprev = &(سر)->لومړی                                         \
         ; *pprev && ({ pos=container_د(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->بل)


#د_که_پای // list.h

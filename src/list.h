#నిర్వచితం_కాకపోతే __LIST_H
#నిర్వచించు __LIST_H

#అన్తర్భావ "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

నిర్మాణం hlist_కణుపు {
    నిర్మాణం hlist_కణుపు *తదుపరి, **pprev;
};

నిర్మాణం hlist_తల {
    నిర్మాణం hlist_కణుపు *మొదటి;
};

స్థిర అంతఃవరుస పూర్ణాంకం
hlist_ఖాళీలు(స్థిరరాశి నిర్మాణం hlist_తల *h)
{
    తిరిగి !h->మొదటి;
}

స్థిర అంతఃవరుస శూన్య
hlist_del(నిర్మాణం hlist_కణుపు *n)
{
    నిర్మాణం hlist_కణుపు *తదుపరి = n->తదుపరి;
    నిర్మాణం hlist_కణుపు **pprev = n->pprev;
    *pprev = తదుపరి;
    ఒకవేళ (తదుపరి)
        తదుపరి->pprev = pprev;
}

స్థిర అంతఃవరుస శూన్య
hlist_కూడు(నిర్మాణం hlist_కణుపు *n, నిర్మాణం hlist_కణుపు **pprev)
{
    నిర్మాణం hlist_కణుపు *తదుపరి = *pprev;
    n->pprev = pprev;
    n->తదుపరి = తదుపరి;
    ఒకవేళ (తదుపరి)
        తదుపరి->pprev = &n->తదుపరి;
    *pprev = n;
}

స్థిర అంతఃవరుస శూన్య
hlist_కూడు_తల(నిర్మాణం hlist_కణుపు *n, నిర్మాణం hlist_తల *h)
{
    hlist_కూడు(n, &h->మొదటి);
}

స్థిర అంతఃవరుస శూన్య
hlist_కూడు_before(నిర్మాణం hlist_కణుపు *n, నిర్మాణం hlist_కణుపు *తదుపరి)
{
    hlist_కూడు(n, తదుపరి->pprev);
}

స్థిర అంతఃవరుస శూన్య
hlist_కూడు_after(నిర్మాణం hlist_కణుపు *n, నిర్మాణం hlist_కణుపు *prev)
{
    hlist_కూడు(n, &prev->తదుపరి);
}

స్థిర అంతఃవరుస శూన్య
hlist_replace(నిర్మాణం hlist_కణుపు *old, నిర్మాణం hlist_కణుపు *new)
{
    new->తదుపరి = old->తదుపరి;
    ఒకవేళ (new->తదుపరి)
        new->తదుపరి->pprev = &new->తదుపరి;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#నిర్వచించు hlist_for_each_నమోదు(pos, తల, member)                         \
    కొరకు (pos = container_యొక్క((తల)->మొదటి, typeof(*pos), member)        \
         ; pos != container_యొక్క(ఖాళీ, typeof(*pos), member)              \
         ; pos = container_యొక్క(pos->member.తదుపరి, typeof(*pos), member))

#నిర్వచించు hlist_for_each_నమోదు_సురక్షిత(pos, n, తల, member)                 \
    కొరకు (pos = container_యొక్క((తల)->మొదటి, typeof(*pos), member)        \
         ; pos != container_యొక్క(ఖాళీ, typeof(*pos), member)              \
             && ({ n = pos->member.తదుపరి; 1; })                          \
         ; pos = container_యొక్క(n, typeof(*pos), member))

#నిర్వచించు hlist_for_each_నమోదు_pprev(pos, pprev, తల, member)            \
    కొరకు (pprev = &(తల)->మొదటి                                         \
         ; *pprev && ({ pos=container_యొక్క(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->తదుపరి)


#అయితే_ముగింపు // list.h

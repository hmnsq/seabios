#यदि_अपरिभाषितम् __LIST_H
#परिभाषय __LIST_H

#अन्तर्भाव "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

संरचना hlist_ग्रन्थिः {
    संरचना hlist_ग्रन्थिः *अग्र, **pprev;
};

संरचना hlist_शीर्ष {
    संरचना hlist_ग्रन्थिः *first;
};

स्थिर अन्तःस्थ पूर्णाङ्क
hlist_रिक्तानि(ध्रुव संरचना hlist_शीर्ष *h)
{
    प्रत्यागम !h->first;
}

स्थिर अन्तःस्थ शून्य
hlist_del(संरचना hlist_ग्रन्थिः *n)
{
    संरचना hlist_ग्रन्थिः *अग्र = n->अग्र;
    संरचना hlist_ग्रन्थिः **pprev = n->pprev;
    *pprev = अग्र;
    यदि (अग्र)
        अग्र->pprev = pprev;
}

स्थिर अन्तःस्थ शून्य
hlist_सङ्कलय(संरचना hlist_ग्रन्थिः *n, संरचना hlist_ग्रन्थिः **pprev)
{
    संरचना hlist_ग्रन्थिः *अग्र = *pprev;
    n->pprev = pprev;
    n->अग्र = अग्र;
    यदि (अग्र)
        अग्र->pprev = &n->अग्र;
    *pprev = n;
}

स्थिर अन्तःस्थ शून्य
hlist_सङ्कलय_शीर्ष(संरचना hlist_ग्रन्थिः *n, संरचना hlist_शीर्ष *h)
{
    hlist_सङ्कलय(n, &h->first);
}

स्थिर अन्तःस्थ शून्य
hlist_सङ्कलय_before(संरचना hlist_ग्रन्थिः *n, संरचना hlist_ग्रन्थिः *अग्र)
{
    hlist_सङ्कलय(n, अग्र->pprev);
}

स्थिर अन्तःस्थ शून्य
hlist_सङ्कलय_after(संरचना hlist_ग्रन्थिः *n, संरचना hlist_ग्रन्थिः *पूर्व)
{
    hlist_सङ्कलय(n, &पूर्व->अग्र);
}

स्थिर अन्तःस्थ शून्य
hlist_replace(संरचना hlist_ग्रन्थिः *old, संरचना hlist_ग्रन्थिः *new)
{
    new->अग्र = old->अग्र;
    यदि (new->अग्र)
        new->अग्र->pprev = &new->अग्र;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#परिभाषय hlist_for_each_प्रविष्टिः(pos, शीर्ष, member)                         \
    प्रति (pos = container_सम्बन्धि((शीर्ष)->first, typeof(*pos), member)        \
         ; pos != container_सम्बन्धि(रिक्त, typeof(*pos), member)              \
         ; pos = container_सम्बन्धि(pos->member.अग्र, typeof(*pos), member))

#परिभाषय hlist_for_each_प्रविष्टिः_safe(pos, n, शीर्ष, member)                 \
    प्रति (pos = container_सम्बन्धि((शीर्ष)->first, typeof(*pos), member)        \
         ; pos != container_सम्बन्धि(रिक्त, typeof(*pos), member)              \
             && ({ n = pos->member.अग्र; 1; })                          \
         ; pos = container_सम्बन्धि(n, typeof(*pos), member))

#परिभाषय hlist_for_each_प्रविष्टिः_pprev(pos, pprev, शीर्ष, member)            \
    प्रति (pprev = &(शीर्ष)->first                                         \
         ; *pprev && ({ pos=container_सम्बन्धि(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->अग्र)


#यदि_अन्तः // list.h

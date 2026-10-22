#अगर_अघोषित __LIST_H
#घोषणा __LIST_H

#समावेश "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

काष्ठा hlist_गाँठ {
    काष्ठा hlist_गाँठ *अग्र, **pprev;
};

काष्ठा hlist_शीर्ष {
    काष्ठा hlist_गाँठ *first;
};

अटल अंतरभूत पूर्णांक
hlist_empty(स्थिर काष्ठा hlist_शीर्ष *h)
{
    वापस !h->first;
}

अटल अंतरभूत व्योम
hlist_del(काष्ठा hlist_गाँठ *n)
{
    काष्ठा hlist_गाँठ *अग्र = n->अग्र;
    काष्ठा hlist_गाँठ **pprev = n->pprev;
    *pprev = अग्र;
    अगर (अग्र)
        अग्र->pprev = pprev;
}

अटल अंतरभूत व्योम
hlist_add(काष्ठा hlist_गाँठ *n, काष्ठा hlist_गाँठ **pprev)
{
    काष्ठा hlist_गाँठ *अग्र = *pprev;
    n->pprev = pprev;
    n->अग्र = अग्र;
    अगर (अग्र)
        अग्र->pprev = &n->अग्र;
    *pprev = n;
}

अटल अंतरभूत व्योम
hlist_add_शीर्ष(काष्ठा hlist_गाँठ *n, काष्ठा hlist_शीर्ष *h)
{
    hlist_add(n, &h->first);
}

अटल अंतरभूत व्योम
hlist_add_before(काष्ठा hlist_गाँठ *n, काष्ठा hlist_गाँठ *अग्र)
{
    hlist_add(n, अग्र->pprev);
}

अटल अंतरभूत व्योम
hlist_add_after(काष्ठा hlist_गाँठ *n, काष्ठा hlist_गाँठ *पूर्व)
{
    hlist_add(n, &पूर्व->अग्र);
}

अटल अंतरभूत व्योम
hlist_replace(काष्ठा hlist_गाँठ *old, काष्ठा hlist_गाँठ *new)
{
    new->अग्र = old->अग्र;
    अगर (new->अग्र)
        new->अग्र->pprev = &new->अग्र;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#घोषणा hlist_for_each_entry(pos, शीर्ष, member)                         \
    क्रम (pos = container_of((शीर्ष)->first, typeof(*pos), member)        \
         ; pos != container_of(शून्य, typeof(*pos), member)              \
         ; pos = container_of(pos->member.अग्र, typeof(*pos), member))

#घोषणा hlist_for_each_entry_safe(pos, n, शीर्ष, member)                 \
    क्रम (pos = container_of((शीर्ष)->first, typeof(*pos), member)        \
         ; pos != container_of(शून्य, typeof(*pos), member)              \
             && ({ n = pos->member.अग्र; 1; })                          \
         ; pos = container_of(n, typeof(*pos), member))

#घोषणा hlist_for_each_entry_pprev(pos, pprev, शीर्ष, member)            \
    क्रम (pprev = &(शीर्ष)->first                                         \
         ; *pprev && ({ pos=container_of(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->अग्र)


#पूर्ण_अगर // list.h

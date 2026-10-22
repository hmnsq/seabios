#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __LIST_H
#ವ್ಯಾಖ್ಯಾನಿಸು __LIST_H

#ಅನ್ತರ್ಭಾವ "types.h" // container_of


/****************************************************************
 * hlist - Double linked lists with a single pointer list head
 ****************************************************************/

ರಚನೆ hlist_ಗಂಟು {
    ರಚನೆ hlist_ಗಂಟು *ಮುಂದಿನ, **pprev;
};

ರಚನೆ hlist_ತಲೆ {
    ರಚನೆ hlist_ಗಂಟು *ಮೊದಲ;
};

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ
hlist_ಖಾಲಿಗಳು(ಸ್ಥಿರಾಂಕ ರಚನೆ hlist_ತಲೆ *h)
{
    ಹಿಂತಿರುಗಿ !h->ಮೊದಲ;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
hlist_del(ರಚನೆ hlist_ಗಂಟು *n)
{
    ರಚನೆ hlist_ಗಂಟು *ಮುಂದಿನ = n->ಮುಂದಿನ;
    ರಚನೆ hlist_ಗಂಟು **pprev = n->pprev;
    *pprev = ಮುಂದಿನ;
    ಆದರೆ (ಮುಂದಿನ)
        ಮುಂದಿನ->pprev = pprev;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
hlist_ಕೂಡಿಸು(ರಚನೆ hlist_ಗಂಟು *n, ರಚನೆ hlist_ಗಂಟು **pprev)
{
    ರಚನೆ hlist_ಗಂಟು *ಮುಂದಿನ = *pprev;
    n->pprev = pprev;
    n->ಮುಂದಿನ = ಮುಂದಿನ;
    ಆದರೆ (ಮುಂದಿನ)
        ಮುಂದಿನ->pprev = &n->ಮುಂದಿನ;
    *pprev = n;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
hlist_ಕೂಡಿಸು_ತಲೆ(ರಚನೆ hlist_ಗಂಟು *n, ರಚನೆ hlist_ತಲೆ *h)
{
    hlist_ಕೂಡಿಸು(n, &h->ಮೊದಲ);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
hlist_ಕೂಡಿಸು_before(ರಚನೆ hlist_ಗಂಟು *n, ರಚನೆ hlist_ಗಂಟು *ಮುಂದಿನ)
{
    hlist_ಕೂಡಿಸು(n, ಮುಂದಿನ->pprev);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
hlist_ಕೂಡಿಸು_after(ರಚನೆ hlist_ಗಂಟು *n, ರಚನೆ hlist_ಗಂಟು *prev)
{
    hlist_ಕೂಡಿಸು(n, &prev->ಮುಂದಿನ);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
hlist_replace(ರಚನೆ hlist_ಗಂಟು *old, ರಚನೆ hlist_ಗಂಟು *new)
{
    new->ಮುಂದಿನ = old->ಮುಂದಿನ;
    ಆದರೆ (new->ಮುಂದಿನ)
        new->ಮುಂದಿನ->pprev = &new->ಮುಂದಿನ;
    new->pprev = old->pprev;
    *new->pprev = new;
}

#ವ್ಯಾಖ್ಯಾನಿಸು hlist_for_each_ನಮೂದು(pos, ತಲೆ, member)                         \
    ಗಾಗಿ (pos = container_ದ((ತಲೆ)->ಮೊದಲ, typeof(*pos), member)        \
         ; pos != container_ದ(ಖಾಲಿ, typeof(*pos), member)              \
         ; pos = container_ದ(pos->member.ಮುಂದಿನ, typeof(*pos), member))

#ವ್ಯಾಖ್ಯಾನಿಸು hlist_for_each_ನಮೂದು_ಸುರಕ್ಷಿತ(pos, n, ತಲೆ, member)                 \
    ಗಾಗಿ (pos = container_ದ((ತಲೆ)->ಮೊದಲ, typeof(*pos), member)        \
         ; pos != container_ದ(ಖಾಲಿ, typeof(*pos), member)              \
             && ({ n = pos->member.ಮುಂದಿನ; 1; })                          \
         ; pos = container_ದ(n, typeof(*pos), member))

#ವ್ಯಾಖ್ಯಾನಿಸು hlist_for_each_ನಮೂದು_pprev(pos, pprev, ತಲೆ, member)            \
    ಗಾಗಿ (pprev = &(ತಲೆ)->ಮೊದಲ                                         \
         ; *pprev && ({ pos=container_ದ(*pprev, typeof(*pos), member); 1; }) \
         ; pprev = &(*pprev)->ಮುಂದಿನ)


#ಆದರೆ_ಅಂತ್ಯ // list.h

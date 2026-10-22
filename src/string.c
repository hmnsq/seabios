// String manipulation functions.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#అన్తర్భావ "stacks.h" // yield
#అన్తర్భావ "string.h" // memcpy
#అన్తర్భావ "farptr.h" // SET_SEG


/****************************************************************
 * String ops
 ****************************************************************/

// Sum the bytes in the specified area.
u8
checksum_far(u16 buf_seg, శూన్య *buf_far, u32 len)
{
    సమితి_SEG(ES, buf_seg);
    u32 i;
    u8 కూడిక = 0;
    కొరకు (i=0; i<len; i++)
        కూడిక += GET_VAR(ES, ((u8*)buf_far)[i]);
    తిరిగి కూడిక;
}

u8
checksum(శూన్య *buf, u32 len)
{
    తిరిగి checksum_far(GET_SEG(SS), buf, len);
}

కొలత_t
అక్షరశ్రేణి_పొడవు(స్థిరరాశి అక్షరం *s)
{
    ఒకవేళ (__builtin_constant_p(s))
        తిరిగి __builtin_strlen(s);
    స్థిరరాశి అక్షరం *p = s;
    అయితే (*p)
        p++;
    తిరిగి p-s;
}

పూర్ణాంకం
memcmp_far(u16 s1seg, స్థిరరాశి శూన్య *s1, u16 s2seg, స్థిరరాశి శూన్య *s2, కొలత_t n)
{
    అయితే (n--) {
        పూర్ణాంకం d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        ఒకవేళ (d)
            తిరిగి d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    తిరిగి 0;
}

// Compare two areas of memory.
పూర్ణాంకం
memcmp(స్థిరరాశి శూన్య *s1, స్థిరరాశి శూన్య *s2, కొలత_t n)
{
    అయితే (n) {
        ఒకవేళ (*(u8*)s1 != *(u8*)s2)
            తిరిగి *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    తిరిగి 0;
}

// Compare two strings.
పూర్ణాంకం
strcmp(స్థిరరాశి అక్షరం *s1, స్థిరరాశి అక్షరం *s2)
{
    కొరకు (;;) {
        ఒకవేళ (*s1 != *s2)
            తిరిగి *s1 < *s2 ? -1 : 1;
        ఒకవేళ (! *s1)
            తిరిగి 0;
        s1++;
        s2++;
    }
}

అంతఃవరుస శూన్య
memset_far(u16 d_seg, శూన్య *d_far, u8 c, కొలత_t len)
{
    సమితి_SEG(ES, d_seg);
    asm అస్థిర(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

అంతఃవరుస శూన్య
memset16_far(u16 d_seg, శూన్య *d_far, u16 c, కొలత_t len)
{
    len /= 2;
    సమితి_SEG(ES, d_seg);
    asm అస్థిర(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

శూన్య *
memset(శూన్య *s, పూర్ణాంకం c, కొలత_t n)
{
    అయితే (n)
        ((అక్షరం *)s)[--n] = c;
    తిరిగి s;
}

శూన్య memset_fl(శూన్య *ptr, u8 val, కొలత_t కొలత)
{
    ఒకవేళ (MODESEGMENT)
        memset_far(FLATPTR_గమ్యం_SEG(ptr), (శూన్య*)(FLATPTR_గమ్యం_స్థానభ్రంశం(ptr)),
                   val, కొలత);
    లేకపోతే
        memset(ptr, val, కొలత);
}

అంతఃవరుస శూన్య
memcpy_far(u16 d_seg, శూన్య *d_far, u16 s_seg, స్థిరరాశి శూన్య *s_far, కొలత_t len)
{
    సమితి_SEG(ES, d_seg);
    u16 bkup_ds;
    asm అస్థిర(
        "movw %%ds, %w0\n"
        "movw %w4, %%ds\n"
        "rep movsb (%%si),%%es:(%%di)\n"
        "movw %w0, %%ds"
        : "=&r"(bkup_ds), "+c"(len), "+S"(s_far), "+D"(d_far)
        : "r"(s_seg), "m" (__segment_ES)
        : "cc", "memory");
}

అంతఃవరుస శూన్య
memcpy_fl(శూన్య *d_fl, స్థిరరాశి శూన్య *s_fl, కొలత_t len)
{
    ఒకవేళ (MODESEGMENT)
        memcpy_far(FLATPTR_గమ్యం_SEG(d_fl), (శూన్య*)FLATPTR_గమ్యం_స్థానభ్రంశం(d_fl)
                   , FLATPTR_గమ్యం_SEG(s_fl), (శూన్య*)FLATPTR_గమ్యం_స్థానభ్రంశం(s_fl)
                   , len);
    లేకపోతే
        జ్ఞాపకం_నకలు(d_fl, s_fl, len);
}

శూన్య *
#నిర్వచనం_రద్దు జ్ఞాపకం_నకలు
జ్ఞాపకం_నకలు(శూన్య *d1, స్థిరరాశి శూన్య *s1, కొలత_t len)
#ఒకవేళ MODESEGMENT == 0
#నిర్వచించు జ్ఞాపకం_నకలు __builtin_memcpy
#అయితే_ముగింపు
{
    సమితి_SEG(ES, GET_SEG(SS));
    శూన్య *d = d1;
    ఒకవేళ (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm అస్థిర(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        తిరిగి d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm అస్థిర(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    తిరిగి d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
శూన్య
iomemcpy(శూన్య *d, స్థిరరాశి శూన్య *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    అయితే (len > 3) {
        u32 copylen = len;
        ఒకవేళ (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm అస్థిర(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    ఒకవేళ (len)
        // Copy any remaining bytes.
        జ్ఞాపకం_నకలు(d, s, len);
}

శూన్య *
memmove(శూన్య *d, స్థిరరాశి శూన్య *s, కొలత_t len)
{
    ఒకవేళ (s >= d)
        తిరిగి జ్ఞాపకం_నకలు(d, s, len);

    d += len-1;
    s += len-1;
    అయితే (len--) {
        *(అక్షరం*)d = *(అక్షరం*)s;
        d--;
        s--;
    }

    తిరిగి d;
}

// Copy a string - truncating it if necessary.
అక్షరం *
strtcpy(అక్షరం *dest, స్థిరరాశి అక్షరం *src, కొలత_t len)
{
    అక్షరం *d = dest;
    అయితే (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    తిరిగి dest;
}

// locate first occurrence of character c in the string s
అక్షరం *
strchr(స్థిరరాశి అక్షరం *s, పూర్ణాంకం c)
{
    కొరకు (; *s; s++)
        ఒకవేళ (*s == c)
            తిరిగి (అక్షరం*)s;
    తిరిగి ఖాళీ;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
అక్షరం *
nullTrailingSpace(అక్షరం *buf)
{
    పూర్ణాంకం len = అక్షరశ్రేణి_పొడవు(buf);
    అక్షరం *చివర = &buf[len-1];
    అయితే (చివర >= buf && *చివర <= ' ')
        *(చివర--) = '\0';
    అయితే (*buf && *buf <= ' ')
        buf++;
    తిరిగి buf;
}

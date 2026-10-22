// String manipulation functions.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "stacks.h" // yield
#अन्तर्भाव "string.h" // memcpy
#अन्तर्भाव "farptr.h" // SET_SEG


/****************************************************************
 * String ops
 ****************************************************************/

// Sum the bytes in the specified area.
u8
checksum_far(u16 buf_seg, शून्य *buf_far, u32 len)
{
    SET_SEG(ES, buf_seg);
    u32 i;
    u8 sum = 0;
    अत्थाय (i=0; i<len; i++)
        sum += GET_VAR(ES, ((u8*)buf_far)[i]);
    पटिनिवत्त sum;
}

u8
checksum(शून्य *buf, u32 len)
{
    पटिनिवत्त checksum_far(GET_SEG(SS), buf, len);
}

size_t
strlen(ध्रुव वर्ण *s)
{
    सचे (__builtin_constant_p(s))
        पटिनिवत्त __builtin_strlen(s);
    ध्रुव वर्ण *p = s;
    याव (*p)
        p++;
    पटिनिवत्त p-s;
}

पूर्णाङ्क
memcmp_far(u16 s1seg, ध्रुव शून्य *s1, u16 s2seg, ध्रुव शून्य *s2, size_t n)
{
    याव (n--) {
        पूर्णाङ्क d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        सचे (d)
            पटिनिवत्त d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    पटिनिवत्त 0;
}

// Compare two areas of memory.
पूर्णाङ्क
memcmp(ध्रुव शून्य *s1, ध्रुव शून्य *s2, size_t n)
{
    याव (n) {
        सचे (*(u8*)s1 != *(u8*)s2)
            पटिनिवत्त *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    पटिनिवत्त 0;
}

// Compare two strings.
पूर्णाङ्क
strcmp(ध्रुव वर्ण *s1, ध्रुव वर्ण *s2)
{
    अत्थाय (;;) {
        सचे (*s1 != *s2)
            पटिनिवत्त *s1 < *s2 ? -1 : 1;
        सचे (! *s1)
            पटिनिवत्त 0;
        s1++;
        s2++;
    }
}

अन्तःस्थ शून्य
memset_far(u16 d_seg, शून्य *d_far, u8 c, size_t len)
{
    SET_SEG(ES, d_seg);
    asm चञ्चल(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

अन्तःस्थ शून्य
memset16_far(u16 d_seg, शून्य *d_far, u16 c, size_t len)
{
    len /= 2;
    SET_SEG(ES, d_seg);
    asm चञ्चल(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

शून्य *
memset(शून्य *s, पूर्णाङ्क c, size_t n)
{
    याव (n)
        ((वर्ण *)s)[--n] = c;
    पटिनिवत्त s;
}

शून्य memset_fl(शून्य *ptr, u8 val, size_t size)
{
    सचे (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (शून्य*)(FLATPTR_TO_OFFSET(ptr)),
                   val, size);
    अञ्ञथा
        memset(ptr, val, size);
}

अन्तःस्थ शून्य
memcpy_far(u16 d_seg, शून्य *d_far, u16 s_seg, ध्रुव शून्य *s_far, size_t len)
{
    SET_SEG(ES, d_seg);
    u16 bkup_ds;
    asm चञ्चल(
        "movw %%ds, %w0\n"
        "movw %w4, %%ds\n"
        "rep movsb (%%si),%%es:(%%di)\n"
        "movw %w0, %%ds"
        : "=&r"(bkup_ds), "+c"(len), "+S"(s_far), "+D"(d_far)
        : "r"(s_seg), "m" (__segment_ES)
        : "cc", "memory");
}

अन्तःस्थ शून्य
memcpy_fl(शून्य *d_fl, ध्रुव शून्य *s_fl, size_t len)
{
    सचे (MODESEGMENT)
        memcpy_far(FLATPTR_TO_SEG(d_fl), (शून्य*)FLATPTR_TO_OFFSET(d_fl)
                   , FLATPTR_TO_SEG(s_fl), (शून्य*)FLATPTR_TO_OFFSET(s_fl)
                   , len);
    अञ्ञथा
        memcpy(d_fl, s_fl, len);
}

शून्य *
#undef memcpy
memcpy(शून्य *d1, ध्रुव शून्य *s1, size_t len)
#सचे MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
{
    SET_SEG(ES, GET_SEG(SS));
    शून्य *d = d1;
    सचे (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm चञ्चल(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        पटिनिवत्त d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm चञ्चल(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    पटिनिवत्त d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
शून्य
iomemcpy(शून्य *d, ध्रुव शून्य *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    याव (len > 3) {
        u32 copylen = len;
        सचे (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm चञ्चल(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    सचे (len)
        // Copy any remaining bytes.
        memcpy(d, s, len);
}

शून्य *
memmove(शून्य *d, ध्रुव शून्य *s, size_t len)
{
    सचे (s >= d)
        पटिनिवत्त memcpy(d, s, len);

    d += len-1;
    s += len-1;
    याव (len--) {
        *(वर्ण*)d = *(वर्ण*)s;
        d--;
        s--;
    }

    पटिनिवत्त d;
}

// Copy a string - truncating it if necessary.
वर्ण *
strtcpy(वर्ण *dest, ध्रुव वर्ण *src, size_t len)
{
    वर्ण *d = dest;
    याव (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    पटिनिवत्त dest;
}

// locate first occurrence of character c in the string s
वर्ण *
strchr(ध्रुव वर्ण *s, पूर्णाङ्क c)
{
    अत्थाय (; *s; s++)
        सचे (*s == c)
            पटिनिवत्त (वर्ण*)s;
    पटिनिवत्त रिक्त;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
वर्ण *
nullTrailingSpace(वर्ण *buf)
{
    पूर्णाङ्क len = strlen(buf);
    वर्ण *end = &buf[len-1];
    याव (end >= buf && *end <= ' ')
        *(end--) = '\0';
    याव (*buf && *buf <= ' ')
        buf++;
    पटिनिवत्त buf;
}

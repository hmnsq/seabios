// String manipulation functions.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "string.h" // memcpy
#ಅನ್ತರ್ಭಾವ "farptr.h" // SET_SEG


/****************************************************************
 * String ops
 ****************************************************************/

// Sum the bytes in the specified area.
u8
checksum_far(u16 buf_seg, ಶೂನ್ಯ *buf_far, u32 len)
{
    SET_SEG(ES, buf_seg);
    u32 i;
    u8 sum = 0;
    ಪ್ರತಿ (i=0; i<len; i++)
        sum += GET_VAR(ES, ((u8*)buf_far)[i]);
    ಪ್ರತ್ಯಾಗಮ sum;
}

u8
checksum(ಶೂನ್ಯ *buf, u32 len)
{
    ಪ್ರತ್ಯಾಗಮ checksum_far(GET_SEG(SS), buf, len);
}

size_t
strlen(ಧ್ರುವ ವರ್ಣ *s)
{
    ಯದಿ (__builtin_constant_p(s))
        ಪ್ರತ್ಯಾಗಮ __builtin_strlen(s);
    ಧ್ರುವ ವರ್ಣ *p = s;
    ಯಾವತ್ (*p)
        p++;
    ಪ್ರತ್ಯಾಗಮ p-s;
}

ಪೂರ್ಣಾಙ್ಕ
memcmp_far(u16 s1seg, ಧ್ರುವ ಶೂನ್ಯ *s1, u16 s2seg, ಧ್ರುವ ಶೂನ್ಯ *s2, size_t n)
{
    ಯಾವತ್ (n--) {
        ಪೂರ್ಣಾಙ್ಕ d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        ಯದಿ (d)
            ಪ್ರತ್ಯಾಗಮ d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}

// Compare two areas of memory.
ಪೂರ್ಣಾಙ್ಕ
memcmp(ಧ್ರುವ ಶೂನ್ಯ *s1, ಧ್ರುವ ಶೂನ್ಯ *s2, size_t n)
{
    ಯಾವತ್ (n) {
        ಯದಿ (*(u8*)s1 != *(u8*)s2)
            ಪ್ರತ್ಯಾಗಮ *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    ಪ್ರತ್ಯಾಗಮ 0;
}

// Compare two strings.
ಪೂರ್ಣಾಙ್ಕ
strcmp(ಧ್ರುವ ವರ್ಣ *s1, ಧ್ರುವ ವರ್ಣ *s2)
{
    ಪ್ರತಿ (;;) {
        ಯದಿ (*s1 != *s2)
            ಪ್ರತ್ಯಾಗಮ *s1 < *s2 ? -1 : 1;
        ಯದಿ (! *s1)
            ಪ್ರತ್ಯಾಗಮ 0;
        s1++;
        s2++;
    }
}

ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
memset_far(u16 d_seg, ಶೂನ್ಯ *d_far, u8 c, size_t len)
{
    SET_SEG(ES, d_seg);
    asm ಚಞ್ಚಲ(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
memset16_far(u16 d_seg, ಶೂನ್ಯ *d_far, u16 c, size_t len)
{
    len /= 2;
    SET_SEG(ES, d_seg);
    asm ಚಞ್ಚಲ(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

ಶೂನ್ಯ *
memset(ಶೂನ್ಯ *s, ಪೂರ್ಣಾಙ್ಕ c, size_t n)
{
    ಯಾವತ್ (n)
        ((ವರ್ಣ *)s)[--n] = c;
    ಪ್ರತ್ಯಾಗಮ s;
}

ಶೂನ್ಯ memset_fl(ಶೂನ್ಯ *ptr, u8 val, size_t size)
{
    ಯದಿ (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (ಶೂನ್ಯ*)(FLATPTR_TO_OFFSET(ptr)),
                   val, size);
    ಅನ್ಯಥಾ
        memset(ptr, val, size);
}

ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
memcpy_far(u16 d_seg, ಶೂನ್ಯ *d_far, u16 s_seg, ಧ್ರುವ ಶೂನ್ಯ *s_far, size_t len)
{
    SET_SEG(ES, d_seg);
    u16 bkup_ds;
    asm ಚಞ್ಚಲ(
        "movw %%ds, %w0\n"
        "movw %w4, %%ds\n"
        "rep movsb (%%si),%%es:(%%di)\n"
        "movw %w0, %%ds"
        : "=&r"(bkup_ds), "+c"(len), "+S"(s_far), "+D"(d_far)
        : "r"(s_seg), "m" (__segment_ES)
        : "cc", "memory");
}

ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
memcpy_fl(ಶೂನ್ಯ *d_fl, ಧ್ರುವ ಶೂನ್ಯ *s_fl, size_t len)
{
    ಯದಿ (MODESEGMENT)
        memcpy_far(FLATPTR_TO_SEG(d_fl), (ಶೂನ್ಯ*)FLATPTR_TO_OFFSET(d_fl)
                   , FLATPTR_TO_SEG(s_fl), (ಶೂನ್ಯ*)FLATPTR_TO_OFFSET(s_fl)
                   , len);
    ಅನ್ಯಥಾ
        memcpy(d_fl, s_fl, len);
}

ಶೂನ್ಯ *
#undef memcpy
memcpy(ಶೂನ್ಯ *d1, ಧ್ರುವ ಶೂನ್ಯ *s1, size_t len)
#ಯದಿ MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
{
    SET_SEG(ES, GET_SEG(SS));
    ಶೂನ್ಯ *d = d1;
    ಯದಿ (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm ಚಞ್ಚಲ(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        ಪ್ರತ್ಯಾಗಮ d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm ಚಞ್ಚಲ(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    ಪ್ರತ್ಯಾಗಮ d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
ಶೂನ್ಯ
iomemcpy(ಶೂನ್ಯ *d, ಧ್ರುವ ಶೂನ್ಯ *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    ಯಾವತ್ (len > 3) {
        u32 copylen = len;
        ಯದಿ (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm ಚಞ್ಚಲ(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    ಯದಿ (len)
        // Copy any remaining bytes.
        memcpy(d, s, len);
}

ಶೂನ್ಯ *
memmove(ಶೂನ್ಯ *d, ಧ್ರುವ ಶೂನ್ಯ *s, size_t len)
{
    ಯದಿ (s >= d)
        ಪ್ರತ್ಯಾಗಮ memcpy(d, s, len);

    d += len-1;
    s += len-1;
    ಯಾವತ್ (len--) {
        *(ವರ್ಣ*)d = *(ವರ್ಣ*)s;
        d--;
        s--;
    }

    ಪ್ರತ್ಯಾಗಮ d;
}

// Copy a string - truncating it if necessary.
ವರ್ಣ *
strtcpy(ವರ್ಣ *dest, ಧ್ರುವ ವರ್ಣ *src, size_t len)
{
    ವರ್ಣ *d = dest;
    ಯಾವತ್ (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    ಪ್ರತ್ಯಾಗಮ dest;
}

// locate first occurrence of character c in the string s
ವರ್ಣ *
strchr(ಧ್ರುವ ವರ್ಣ *s, ಪೂರ್ಣಾಙ್ಕ c)
{
    ಪ್ರತಿ (; *s; s++)
        ಯದಿ (*s == c)
            ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ*)s;
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
ವರ್ಣ *
nullTrailingSpace(ವರ್ಣ *buf)
{
    ಪೂರ್ಣಾಙ್ಕ len = strlen(buf);
    ವರ್ಣ *end = &buf[len-1];
    ಯಾವತ್ (end >= buf && *end <= ' ')
        *(end--) = '\0';
    ಯಾವತ್ (*buf && *buf <= ' ')
        buf++;
    ಪ್ರತ್ಯಾಗಮ buf;
}

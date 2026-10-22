// String manipulation functions.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#include "stacks.h" // yield
#include "string.h" // memcpy
#include "farptr.h" // SET_SEG


/****************************************************************
 * String ops
 ****************************************************************/

// Sum the bytes in the specified area.
u8
checksum_far(u16 buf_seg, void *buf_far, u32 len)
{
    SET_SEG(ES, buf_seg);
    u32 i;
    u8 sum = 0;
    for (i=0; i<len; i++)
        sum += GET_VAR(ES, ((u8*)buf_far)[i]);
    ᯀᯮᯞ sum;
}

u8
checksum(void *buf, u32 len)
{
    ᯀᯮᯞ checksum_far(GET_SEG(SS), buf, len);
}

size_t
strlen(const var_na *s)
{
    ᯀᯮᯝ (__builtin_constant_p(s))
        ᯀᯮᯞ __builtin_strlen(s);
    const var_na *p = s;
    ᯀᯬᯝ (*p)
        p++;
    ᯀᯮᯞ p-s;
}

puur_naa_nka
memcmp_far(u16 s1seg, const void *s1, u16 s2seg, const void *s2, size_t n)
{
    ᯀᯬᯝ (n--) {
        puur_naa_nka d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        ᯀᯮᯝ (d)
            ᯀᯮᯞ d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    ᯀᯮᯞ 0;
}

// Compare two areas of memory.
puur_naa_nka
memcmp(const void *s1, const void *s2, size_t n)
{
    ᯀᯬᯝ (n) {
        ᯀᯮᯝ (*(u8*)s1 != *(u8*)s2)
            ᯀᯮᯞ *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    ᯀᯮᯞ 0;
}

// Compare two strings.
puur_naa_nka
strcmp(const var_na *s1, const var_na *s2)
{
    for (;;) {
        ᯀᯮᯝ (*s1 != *s2)
            ᯀᯮᯞ *s1 < *s2 ? -1 : 1;
        ᯀᯮᯝ (! *s1)
            ᯀᯮᯞ 0;
        s1++;
        s2++;
    }
}

anta_hstha void
memset_far(u16 d_seg, void *d_far, u8 c, size_t len)
{
    SET_SEG(ES, d_seg);
    asm ca_ncala(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

anta_hstha void
memset16_far(u16 d_seg, void *d_far, u16 c, size_t len)
{
    len /= 2;
    SET_SEG(ES, d_seg);
    asm ca_ncala(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

void *
memset(void *s, puur_naa_nka c, size_t n)
{
    ᯀᯬᯝ (n)
        ((var_na *)s)[--n] = c;
    ᯀᯮᯞ s;
}

void memset_fl(void *ptr, u8 val, size_t size)
{
    ᯀᯮᯝ (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (void*)(FLATPTR_TO_OFFSET(ptr)),
                   val, size);
    ᯀᯮᯝᯂᯪ
        memset(ptr, val, size);
}

anta_hstha void
memcpy_far(u16 d_seg, void *d_far, u16 s_seg, const void *s_far, size_t len)
{
    SET_SEG(ES, d_seg);
    u16 bkup_ds;
    asm ca_ncala(
        "movw %%ds, %w0\n"
        "movw %w4, %%ds\n"
        "rep movsb (%%si),%%es:(%%di)\n"
        "movw %w0, %%ds"
        : "=&r"(bkup_ds), "+c"(len), "+S"(s_far), "+D"(d_far)
        : "r"(s_seg), "m" (__segment_ES)
        : "cc", "memory");
}

anta_hstha void
memcpy_fl(void *d_fl, const void *s_fl, size_t len)
{
    ᯀᯮᯝ (MODESEGMENT)
        memcpy_far(FLATPTR_TO_SEG(d_fl), (void*)FLATPTR_TO_OFFSET(d_fl)
                   , FLATPTR_TO_SEG(s_fl), (void*)FLATPTR_TO_OFFSET(s_fl)
                   , len);
    ᯀᯮᯝᯂᯪ
        memcpy(d_fl, s_fl, len);
}

void *
#undef memcpy
memcpy(void *d1, const void *s1, size_t len)
#ᯀᯮᯝ MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
{
    SET_SEG(ES, GET_SEG(SS));
    void *d = d1;
    ᯀᯮᯝ (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm ca_ncala(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        ᯀᯮᯞ d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm ca_ncala(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    ᯀᯮᯞ d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
void
iomemcpy(void *d, const void *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    ᯀᯬᯝ (len > 3) {
        u32 copylen = len;
        ᯀᯮᯝ (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm ca_ncala(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    ᯀᯮᯝ (len)
        // Copy any remaining bytes.
        memcpy(d, s, len);
}

void *
memmove(void *d, const void *s, size_t len)
{
    ᯀᯮᯝ (s >= d)
        ᯀᯮᯞ memcpy(d, s, len);

    d += len-1;
    s += len-1;
    ᯀᯬᯝ (len--) {
        *(var_na*)d = *(var_na*)s;
        d--;
        s--;
    }

    ᯀᯮᯞ d;
}

// Copy a string - truncating it if necessary.
var_na *
strtcpy(var_na *dest, const var_na *src, size_t len)
{
    var_na *d = dest;
    ᯀᯬᯝ (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    ᯀᯮᯞ dest;
}

// locate first occurrence of character c in the string s
var_na *
strchr(const var_na *s, puur_naa_nka c)
{
    for (; *s; s++)
        ᯀᯮᯝ (*s == c)
            ᯀᯮᯞ (var_na*)s;
    ᯀᯮᯞ NULL;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
var_na *
nullTrailingSpace(var_na *buf)
{
    puur_naa_nka len = strlen(buf);
    var_na *end = &buf[len-1];
    ᯀᯬᯝ (end >= buf && *end <= ' ')
        *(end--) = '\0';
    ᯀᯬᯝ (*buf && *buf <= ' ')
        buf++;
    ᯀᯮᯞ buf;
}

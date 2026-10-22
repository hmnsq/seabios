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
    за (i=0; i<len; i++)
        sum += GET_VAR(ES, ((u8*)buf_far)[i]);
    врати sum;
}

u8
checksum(void *buf, u32 len)
{
    врати checksum_far(GET_SEG(SS), buf, len);
}

size_t
strlen(const char *s)
{
    ако (__builtin_constant_p(s))
        врати __builtin_strlen(s);
    const char *p = s;
    док (*p)
        p++;
    врати p-s;
}

цео
memcmp_far(u16 s1seg, const void *s1, u16 s2seg, const void *s2, size_t n)
{
    док (n--) {
        цео d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        ако (d)
            врати d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    врати 0;
}

// Compare two areas of memory.
цео
memcmp(const void *s1, const void *s2, size_t n)
{
    док (n) {
        ако (*(u8*)s1 != *(u8*)s2)
            врати *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    врати 0;
}

// Compare two strings.
цео
strcmp(const char *s1, const char *s2)
{
    за (;;) {
        ако (*s1 != *s2)
            врати *s1 < *s2 ? -1 : 1;
        ако (! *s1)
            врати 0;
        s1++;
        s2++;
    }
}

inline void
memset_far(u16 d_seg, void *d_far, u8 c, size_t len)
{
    SET_SEG(ES, d_seg);
    asm volatile(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

inline void
memset16_far(u16 d_seg, void *d_far, u16 c, size_t len)
{
    len /= 2;
    SET_SEG(ES, d_seg);
    asm volatile(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

void *
memset(void *s, цео c, size_t n)
{
    док (n)
        ((char *)s)[--n] = c;
    врати s;
}

void memset_fl(void *ptr, u8 val, size_t size)
{
    ако (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (void*)(FLATPTR_TO_OFFSET(ptr)),
                   val, size);
    инако
        memset(ptr, val, size);
}

inline void
memcpy_far(u16 d_seg, void *d_far, u16 s_seg, const void *s_far, size_t len)
{
    SET_SEG(ES, d_seg);
    u16 bkup_ds;
    asm volatile(
        "movw %%ds, %w0\n"
        "movw %w4, %%ds\n"
        "rep movsb (%%si),%%es:(%%di)\n"
        "movw %w0, %%ds"
        : "=&r"(bkup_ds), "+c"(len), "+S"(s_far), "+D"(d_far)
        : "r"(s_seg), "m" (__segment_ES)
        : "cc", "memory");
}

inline void
memcpy_fl(void *d_fl, const void *s_fl, size_t len)
{
    ако (MODESEGMENT)
        memcpy_far(FLATPTR_TO_SEG(d_fl), (void*)FLATPTR_TO_OFFSET(d_fl)
                   , FLATPTR_TO_SEG(s_fl), (void*)FLATPTR_TO_OFFSET(s_fl)
                   , len);
    инако
        memcpy(d_fl, s_fl, len);
}

void *
#undef memcpy
memcpy(void *d1, const void *s1, size_t len)
#ако MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
{
    SET_SEG(ES, GET_SEG(SS));
    void *d = d1;
    ако (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm volatile(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        врати d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm volatile(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    врати d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
void
iomemcpy(void *d, const void *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    док (len > 3) {
        u32 copylen = len;
        ако (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm volatile(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    ако (len)
        // Copy any remaining bytes.
        memcpy(d, s, len);
}

void *
memmove(void *d, const void *s, size_t len)
{
    ако (s >= d)
        врати memcpy(d, s, len);

    d += len-1;
    s += len-1;
    док (len--) {
        *(char*)d = *(char*)s;
        d--;
        s--;
    }

    врати d;
}

// Copy a string - truncating it if necessary.
char *
strtcpy(char *dest, const char *src, size_t len)
{
    char *d = dest;
    док (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    врати dest;
}

// locate first occurrence of character c in the string s
char *
strchr(const char *s, цео c)
{
    за (; *s; s++)
        ако (*s == c)
            врати (char*)s;
    врати NULL;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
char *
nullTrailingSpace(char *buf)
{
    цео len = strlen(buf);
    char *end = &buf[len-1];
    док (end >= buf && *end <= ' ')
        *(end--) = '\0';
    док (*buf && *buf <= ' ')
        buf++;
    врати buf;
}

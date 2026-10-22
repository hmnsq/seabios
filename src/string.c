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
    для (i=0; i<len; i++)
        sum += GET_VAR(ES, ((u8*)buf_far)[i]);
    повернути sum;
}

u8
checksum(void *buf, u32 len)
{
    повернути checksum_far(GET_SEG(SS), buf, len);
}

size_t
strlen(const char *s)
{
    якщо (__builtin_constant_p(s))
        повернути __builtin_strlen(s);
    const char *p = s;
    поки (*p)
        p++;
    повернути p-s;
}

ціле
memcmp_far(u16 s1seg, const void *s1, u16 s2seg, const void *s2, size_t n)
{
    поки (n--) {
        ціле d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        якщо (d)
            повернути d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    повернути 0;
}

// Compare two areas of memory.
ціле
memcmp(const void *s1, const void *s2, size_t n)
{
    поки (n) {
        якщо (*(u8*)s1 != *(u8*)s2)
            повернути *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    повернути 0;
}

// Compare two strings.
ціле
strcmp(const char *s1, const char *s2)
{
    для (;;) {
        якщо (*s1 != *s2)
            повернути *s1 < *s2 ? -1 : 1;
        якщо (! *s1)
            повернути 0;
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
memset(void *s, ціле c, size_t n)
{
    поки (n)
        ((char *)s)[--n] = c;
    повернути s;
}

void memset_fl(void *ptr, u8 val, size_t size)
{
    якщо (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (void*)(FLATPTR_TO_OFFSET(ptr)),
                   val, size);
    інакше
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
    якщо (MODESEGMENT)
        memcpy_far(FLATPTR_TO_SEG(d_fl), (void*)FLATPTR_TO_OFFSET(d_fl)
                   , FLATPTR_TO_SEG(s_fl), (void*)FLATPTR_TO_OFFSET(s_fl)
                   , len);
    інакше
        memcpy(d_fl, s_fl, len);
}

void *
#undef memcpy
memcpy(void *d1, const void *s1, size_t len)
#якщо MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
{
    SET_SEG(ES, GET_SEG(SS));
    void *d = d1;
    якщо (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm volatile(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        повернути d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm volatile(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    повернути d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
void
iomemcpy(void *d, const void *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    поки (len > 3) {
        u32 copylen = len;
        якщо (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm volatile(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    якщо (len)
        // Copy any remaining bytes.
        memcpy(d, s, len);
}

void *
memmove(void *d, const void *s, size_t len)
{
    якщо (s >= d)
        повернути memcpy(d, s, len);

    d += len-1;
    s += len-1;
    поки (len--) {
        *(char*)d = *(char*)s;
        d--;
        s--;
    }

    повернути d;
}

// Copy a string - truncating it if necessary.
char *
strtcpy(char *dest, const char *src, size_t len)
{
    char *d = dest;
    поки (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    повернути dest;
}

// locate first occurrence of character c in the string s
char *
strchr(const char *s, ціле c)
{
    для (; *s; s++)
        якщо (*s == c)
            повернути (char*)s;
    повернути NULL;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
char *
nullTrailingSpace(char *buf)
{
    ціле len = strlen(buf);
    char *end = &buf[len-1];
    поки (end >= buf && *end <= ' ')
        *(end--) = '\0';
    поки (*buf && *buf <= ' ')
        buf++;
    повернути buf;
}

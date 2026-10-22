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
    위해 (i=0; i<len; i++)
        sum += GET_VAR(ES, ((u8*)buf_far)[i]);
    반환 sum;
}

u8
checksum(void *buf, u32 len)
{
    반환 checksum_far(GET_SEG(SS), buf, len);
}

size_t
strlen(const char *s)
{
    만약 (__builtin_constant_p(s))
        반환 __builtin_strlen(s);
    const char *p = s;
    동안 (*p)
        p++;
    반환 p-s;
}

정수
memcmp_far(u16 s1seg, const void *s1, u16 s2seg, const void *s2, size_t n)
{
    동안 (n--) {
        정수 d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        만약 (d)
            반환 d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    반환 0;
}

// Compare two areas of memory.
정수
memcmp(const void *s1, const void *s2, size_t n)
{
    동안 (n) {
        만약 (*(u8*)s1 != *(u8*)s2)
            반환 *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    반환 0;
}

// Compare two strings.
정수
strcmp(const char *s1, const char *s2)
{
    위해 (;;) {
        만약 (*s1 != *s2)
            반환 *s1 < *s2 ? -1 : 1;
        만약 (! *s1)
            반환 0;
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
memset(void *s, 정수 c, size_t n)
{
    동안 (n)
        ((char *)s)[--n] = c;
    반환 s;
}

void memset_fl(void *ptr, u8 val, size_t size)
{
    만약 (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (void*)(FLATPTR_TO_OFFSET(ptr)),
                   val, size);
    아니면
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
    만약 (MODESEGMENT)
        memcpy_far(FLATPTR_TO_SEG(d_fl), (void*)FLATPTR_TO_OFFSET(d_fl)
                   , FLATPTR_TO_SEG(s_fl), (void*)FLATPTR_TO_OFFSET(s_fl)
                   , len);
    아니면
        memcpy(d_fl, s_fl, len);
}

void *
#undef memcpy
memcpy(void *d1, const void *s1, size_t len)
#만약 MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
{
    SET_SEG(ES, GET_SEG(SS));
    void *d = d1;
    만약 (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm volatile(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        반환 d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm volatile(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    반환 d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
void
iomemcpy(void *d, const void *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    동안 (len > 3) {
        u32 copylen = len;
        만약 (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm volatile(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    만약 (len)
        // Copy any remaining bytes.
        memcpy(d, s, len);
}

void *
memmove(void *d, const void *s, size_t len)
{
    만약 (s >= d)
        반환 memcpy(d, s, len);

    d += len-1;
    s += len-1;
    동안 (len--) {
        *(char*)d = *(char*)s;
        d--;
        s--;
    }

    반환 d;
}

// Copy a string - truncating it if necessary.
char *
strtcpy(char *dest, const char *src, size_t len)
{
    char *d = dest;
    동안 (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    반환 dest;
}

// locate first occurrence of character c in the string s
char *
strchr(const char *s, 정수 c)
{
    위해 (; *s; s++)
        만약 (*s == c)
            반환 (char*)s;
    반환 NULL;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
char *
nullTrailingSpace(char *buf)
{
    정수 len = strlen(buf);
    char *end = &buf[len-1];
    동안 (end >= buf && *end <= ' ')
        *(end--) = '\0';
    동안 (*buf && *buf <= ' ')
        buf++;
    반환 buf;
}

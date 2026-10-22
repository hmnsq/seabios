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
    вернуть sum;
}

u8
checksum(void *buf, u32 len)
{
    вернуть checksum_far(GET_SEG(SS), buf, len);
}

size_t
strlen(const char *s)
{
    если (__builtin_constant_p(s))
        вернуть __builtin_strlen(s);
    const char *p = s;
    пока (*p)
        p++;
    вернуть p-s;
}

целое
memcmp_far(u16 s1seg, const void *s1, u16 s2seg, const void *s2, size_t n)
{
    пока (n--) {
        целое d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        если (d)
            вернуть d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    вернуть 0;
}

// Compare two areas of memory.
целое
memcmp(const void *s1, const void *s2, size_t n)
{
    пока (n) {
        если (*(u8*)s1 != *(u8*)s2)
            вернуть *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    вернуть 0;
}

// Compare two strings.
целое
strcmp(const char *s1, const char *s2)
{
    для (;;) {
        если (*s1 != *s2)
            вернуть *s1 < *s2 ? -1 : 1;
        если (! *s1)
            вернуть 0;
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
memset(void *s, целое c, size_t n)
{
    пока (n)
        ((char *)s)[--n] = c;
    вернуть s;
}

void memset_fl(void *ptr, u8 val, size_t size)
{
    если (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (void*)(FLATPTR_TO_OFFSET(ptr)),
                   val, size);
    иначе
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
    если (MODESEGMENT)
        memcpy_far(FLATPTR_TO_SEG(d_fl), (void*)FLATPTR_TO_OFFSET(d_fl)
                   , FLATPTR_TO_SEG(s_fl), (void*)FLATPTR_TO_OFFSET(s_fl)
                   , len);
    иначе
        memcpy(d_fl, s_fl, len);
}

void *
#undef memcpy
memcpy(void *d1, const void *s1, size_t len)
#если MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
{
    SET_SEG(ES, GET_SEG(SS));
    void *d = d1;
    если (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm volatile(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        вернуть d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm volatile(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    вернуть d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
void
iomemcpy(void *d, const void *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    пока (len > 3) {
        u32 copylen = len;
        если (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm volatile(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    если (len)
        // Copy any remaining bytes.
        memcpy(d, s, len);
}

void *
memmove(void *d, const void *s, size_t len)
{
    если (s >= d)
        вернуть memcpy(d, s, len);

    d += len-1;
    s += len-1;
    пока (len--) {
        *(char*)d = *(char*)s;
        d--;
        s--;
    }

    вернуть d;
}

// Copy a string - truncating it if necessary.
char *
strtcpy(char *dest, const char *src, size_t len)
{
    char *d = dest;
    пока (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    вернуть dest;
}

// locate first occurrence of character c in the string s
char *
strchr(const char *s, целое c)
{
    для (; *s; s++)
        если (*s == c)
            вернуть (char*)s;
    вернуть NULL;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
char *
nullTrailingSpace(char *buf)
{
    целое len = strlen(buf);
    char *end = &buf[len-1];
    пока (end >= buf && *end <= ' ')
        *(end--) = '\0';
    пока (*buf && *buf <= ' ')
        buf++;
    вернуть buf;
}

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
    အတွက် (i=0; i<len; i++)
        sum += GET_VAR(ES, ((u8*)buf_far)[i]);
    ပြန်ပေး sum;
}

u8
checksum(void *buf, u32 len)
{
    ပြန်ပေး checksum_far(GET_SEG(SS), buf, len);
}

size_t
strlen(const var_na *s)
{
    အကယ်၍ (__builtin_constant_p(s))
        ပြန်ပေး __builtin_strlen(s);
    const var_na *p = s;
    နေစဉ် (*p)
        p++;
    ပြန်ပေး p-s;
}

ကိန်းပြည့်
memcmp_far(u16 s1seg, const void *s1, u16 s2seg, const void *s2, size_t n)
{
    နေစဉ် (n--) {
        ကိန်းပြည့် d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        အကယ်၍ (d)
            ပြန်ပေး d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    ပြန်ပေး 0;
}

// Compare two areas of memory.
ကိန်းပြည့်
memcmp(const void *s1, const void *s2, size_t n)
{
    နေစဉ် (n) {
        အကယ်၍ (*(u8*)s1 != *(u8*)s2)
            ပြန်ပေး *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    ပြန်ပေး 0;
}

// Compare two strings.
ကိန်းပြည့်
strcmp(const var_na *s1, const var_na *s2)
{
    အတွက် (;;) {
        အကယ်၍ (*s1 != *s2)
            ပြန်ပေး *s1 < *s2 ? -1 : 1;
        အကယ်၍ (! *s1)
            ပြန်ပေး 0;
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
memset(void *s, ကိန်းပြည့် c, size_t n)
{
    နေစဉ် (n)
        ((var_na *)s)[--n] = c;
    ပြန်ပေး s;
}

void memset_fl(void *ptr, u8 val, size_t size)
{
    အကယ်၍ (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (void*)(FLATPTR_TO_OFFSET(ptr)),
                   val, size);
    မဟုတ်ပါက
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
    အကယ်၍ (MODESEGMENT)
        memcpy_far(FLATPTR_TO_SEG(d_fl), (void*)FLATPTR_TO_OFFSET(d_fl)
                   , FLATPTR_TO_SEG(s_fl), (void*)FLATPTR_TO_OFFSET(s_fl)
                   , len);
    မဟုတ်ပါက
        memcpy(d_fl, s_fl, len);
}

void *
#undef memcpy
memcpy(void *d1, const void *s1, size_t len)
#အကယ်၍ MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
{
    SET_SEG(ES, GET_SEG(SS));
    void *d = d1;
    အကယ်၍ (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm ca_ncala(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        ပြန်ပေး d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm ca_ncala(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    ပြန်ပေး d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
void
iomemcpy(void *d, const void *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    နေစဉ် (len > 3) {
        u32 copylen = len;
        အကယ်၍ (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm ca_ncala(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    အကယ်၍ (len)
        // Copy any remaining bytes.
        memcpy(d, s, len);
}

void *
memmove(void *d, const void *s, size_t len)
{
    အကယ်၍ (s >= d)
        ပြန်ပေး memcpy(d, s, len);

    d += len-1;
    s += len-1;
    နေစဉ် (len--) {
        *(var_na*)d = *(var_na*)s;
        d--;
        s--;
    }

    ပြန်ပေး d;
}

// Copy a string - truncating it if necessary.
var_na *
strtcpy(var_na *dest, const var_na *src, size_t len)
{
    var_na *d = dest;
    နေစဉ် (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    ပြန်ပေး dest;
}

// locate first occurrence of character c in the string s
var_na *
strchr(const var_na *s, ကိန်းပြည့် c)
{
    အတွက် (; *s; s++)
        အကယ်၍ (*s == c)
            ပြန်ပေး (var_na*)s;
    ပြန်ပေး NULL;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
var_na *
nullTrailingSpace(var_na *buf)
{
    ကိန်းပြည့် len = strlen(buf);
    var_na *end = &buf[len-1];
    နေစဉ် (end >= buf && *end <= ' ')
        *(end--) = '\0';
    နေစဉ် (*buf && *buf <= ' ')
        buf++;
    ပြန်ပေး buf;
}

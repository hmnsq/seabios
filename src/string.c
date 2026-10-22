// String manipulation functions.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#شامل "stacks.h" // yield
#شامل "string.h" // memcpy
#شامل "farptr.h" // SET_SEG


/****************************************************************
 * String ops
 ****************************************************************/

// Sum the bytes in the specified area.
u8
checksum_far(u16 buf_seg, تهی *buf_far, u32 len)
{
    SET_SEG(ES, buf_seg);
    u32 i;
    u8 sum = 0;
    برای (i=0; i<len; i++)
        sum += GET_VAR(ES, ((u8*)buf_far)[i]);
    بازگشت sum;
}

u8
checksum(تهی *buf, u32 len)
{
    بازگشت checksum_far(GET_SEG(SS), buf, len);
}

size_t
strlen(ثابت نویسه *s)
{
    اگر (__builtin_constant_p(s))
        بازگشت __builtin_strlen(s);
    ثابت نویسه *p = s;
    وقتی (*p)
        p++;
    بازگشت p-s;
}

صحیح
memcmp_far(u16 s1seg, ثابت تهی *s1, u16 s2seg, ثابت تهی *s2, size_t n)
{
    وقتی (n--) {
        صحیح d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        اگر (d)
            بازگشت d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    بازگشت 0;
}

// Compare two areas of memory.
صحیح
memcmp(ثابت تهی *s1, ثابت تهی *s2, size_t n)
{
    وقتی (n) {
        اگر (*(u8*)s1 != *(u8*)s2)
            بازگشت *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    بازگشت 0;
}

// Compare two strings.
صحیح
strcmp(ثابت نویسه *s1, ثابت نویسه *s2)
{
    برای (;;) {
        اگر (*s1 != *s2)
            بازگشت *s1 < *s2 ? -1 : 1;
        اگر (! *s1)
            بازگشت 0;
        s1++;
        s2++;
    }
}

درونخطی تهی
memset_far(u16 d_seg, تهی *d_far, u8 c, size_t len)
{
    SET_SEG(ES, d_seg);
    asm فرار(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

درونخطی تهی
memset16_far(u16 d_seg, تهی *d_far, u16 c, size_t len)
{
    len /= 2;
    SET_SEG(ES, d_seg);
    asm فرار(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

تهی *
memset(تهی *s, صحیح c, size_t n)
{
    وقتی (n)
        ((نویسه *)s)[--n] = c;
    بازگشت s;
}

تهی memset_fl(تهی *ptr, u8 val, size_t size)
{
    اگر (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (تهی*)(FLATPTR_TO_OFFSET(ptr)),
                   val, size);
    وگرنه
        memset(ptr, val, size);
}

درونخطی تهی
memcpy_far(u16 d_seg, تهی *d_far, u16 s_seg, ثابت تهی *s_far, size_t len)
{
    SET_SEG(ES, d_seg);
    u16 bkup_ds;
    asm فرار(
        "movw %%ds, %w0\n"
        "movw %w4, %%ds\n"
        "rep movsb (%%si),%%es:(%%di)\n"
        "movw %w0, %%ds"
        : "=&r"(bkup_ds), "+c"(len), "+S"(s_far), "+D"(d_far)
        : "r"(s_seg), "m" (__segment_ES)
        : "cc", "memory");
}

درونخطی تهی
memcpy_fl(تهی *d_fl, ثابت تهی *s_fl, size_t len)
{
    اگر (MODESEGMENT)
        memcpy_far(FLATPTR_TO_SEG(d_fl), (تهی*)FLATPTR_TO_OFFSET(d_fl)
                   , FLATPTR_TO_SEG(s_fl), (تهی*)FLATPTR_TO_OFFSET(s_fl)
                   , len);
    وگرنه
        memcpy(d_fl, s_fl, len);
}

تهی *
#undef memcpy
memcpy(تهی *d1, ثابت تهی *s1, size_t len)
#اگر MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
{
    SET_SEG(ES, GET_SEG(SS));
    تهی *d = d1;
    اگر (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm فرار(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        بازگشت d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm فرار(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    بازگشت d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
تهی
iomemcpy(تهی *d, ثابت تهی *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    وقتی (len > 3) {
        u32 copylen = len;
        اگر (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm فرار(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    اگر (len)
        // Copy any remaining bytes.
        memcpy(d, s, len);
}

تهی *
memmove(تهی *d, ثابت تهی *s, size_t len)
{
    اگر (s >= d)
        بازگشت memcpy(d, s, len);

    d += len-1;
    s += len-1;
    وقتی (len--) {
        *(نویسه*)d = *(نویسه*)s;
        d--;
        s--;
    }

    بازگشت d;
}

// Copy a string - truncating it if necessary.
نویسه *
strtcpy(نویسه *dest, ثابت نویسه *src, size_t len)
{
    نویسه *d = dest;
    وقتی (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    بازگشت dest;
}

// locate first occurrence of character c in the string s
نویسه *
strchr(ثابت نویسه *s, صحیح c)
{
    برای (; *s; s++)
        اگر (*s == c)
            بازگشت (نویسه*)s;
    بازگشت پوچ;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
نویسه *
nullTrailingSpace(نویسه *buf)
{
    صحیح len = strlen(buf);
    نویسه *end = &buf[len-1];
    وقتی (end >= buf && *end <= ' ')
        *(end--) = '\0';
    وقتی (*buf && *buf <= ' ')
        buf++;
    بازگشت buf;
}

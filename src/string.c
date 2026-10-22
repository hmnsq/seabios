// String manipulation functions.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ضمن "stacks.h" // yield
#ضمن "string.h" // memcpy
#ضمن "farptr.h" // SET_SEG


/****************************************************************
 * String ops
 ****************************************************************/

// Sum the bytes in the specified area.
u8
checksum_far(u16 buf_seg, فراغ *buf_far, u32 len)
{
    SET_SEG(ES, buf_seg);
    u32 i;
    u8 sum = 0;
    لكل (i=0; i<len; i++)
        sum += GET_VAR(ES, ((u8*)buf_far)[i]);
    إرجاع sum;
}

u8
checksum(فراغ *buf, u32 len)
{
    إرجاع checksum_far(GET_SEG(SS), buf, len);
}

size_t
strlen(ثابت حرف *s)
{
    إذا (__builtin_constant_p(s))
        إرجاع __builtin_strlen(s);
    ثابت حرف *p = s;
    بينما (*p)
        p++;
    إرجاع p-s;
}

صحيح
memcmp_far(u16 s1seg, ثابت فراغ *s1, u16 s2seg, ثابت فراغ *s2, size_t n)
{
    بينما (n--) {
        صحيح d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        إذا (d)
            إرجاع d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    إرجاع 0;
}

// Compare two areas of memory.
صحيح
memcmp(ثابت فراغ *s1, ثابت فراغ *s2, size_t n)
{
    بينما (n) {
        إذا (*(u8*)s1 != *(u8*)s2)
            إرجاع *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    إرجاع 0;
}

// Compare two strings.
صحيح
strcmp(ثابت حرف *s1, ثابت حرف *s2)
{
    لكل (;;) {
        إذا (*s1 != *s2)
            إرجاع *s1 < *s2 ? -1 : 1;
        إذا (! *s1)
            إرجاع 0;
        s1++;
        s2++;
    }
}

مضمن فراغ
memset_far(u16 d_seg, فراغ *d_far, u8 c, size_t len)
{
    SET_SEG(ES, d_seg);
    asm متطاير(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

مضمن فراغ
memset16_far(u16 d_seg, فراغ *d_far, u16 c, size_t len)
{
    len /= 2;
    SET_SEG(ES, d_seg);
    asm متطاير(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

فراغ *
memset(فراغ *s, صحيح c, size_t n)
{
    بينما (n)
        ((حرف *)s)[--n] = c;
    إرجاع s;
}

فراغ memset_fl(فراغ *ptr, u8 val, size_t size)
{
    إذا (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (فراغ*)(FLATPTR_TO_OFFSET(ptr)),
                   val, size);
    وإلا
        memset(ptr, val, size);
}

مضمن فراغ
memcpy_far(u16 d_seg, فراغ *d_far, u16 s_seg, ثابت فراغ *s_far, size_t len)
{
    SET_SEG(ES, d_seg);
    u16 bkup_ds;
    asm متطاير(
        "movw %%ds, %w0\n"
        "movw %w4, %%ds\n"
        "rep movsb (%%si),%%es:(%%di)\n"
        "movw %w0, %%ds"
        : "=&r"(bkup_ds), "+c"(len), "+S"(s_far), "+D"(d_far)
        : "r"(s_seg), "m" (__segment_ES)
        : "cc", "memory");
}

مضمن فراغ
memcpy_fl(فراغ *d_fl, ثابت فراغ *s_fl, size_t len)
{
    إذا (MODESEGMENT)
        memcpy_far(FLATPTR_TO_SEG(d_fl), (فراغ*)FLATPTR_TO_OFFSET(d_fl)
                   , FLATPTR_TO_SEG(s_fl), (فراغ*)FLATPTR_TO_OFFSET(s_fl)
                   , len);
    وإلا
        memcpy(d_fl, s_fl, len);
}

فراغ *
#undef memcpy
memcpy(فراغ *d1, ثابت فراغ *s1, size_t len)
#إذا MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
{
    SET_SEG(ES, GET_SEG(SS));
    فراغ *d = d1;
    إذا (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm متطاير(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        إرجاع d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm متطاير(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    إرجاع d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
فراغ
iomemcpy(فراغ *d, ثابت فراغ *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    بينما (len > 3) {
        u32 copylen = len;
        إذا (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm متطاير(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    إذا (len)
        // Copy any remaining bytes.
        memcpy(d, s, len);
}

فراغ *
memmove(فراغ *d, ثابت فراغ *s, size_t len)
{
    إذا (s >= d)
        إرجاع memcpy(d, s, len);

    d += len-1;
    s += len-1;
    بينما (len--) {
        *(حرف*)d = *(حرف*)s;
        d--;
        s--;
    }

    إرجاع d;
}

// Copy a string - truncating it if necessary.
حرف *
strtcpy(حرف *dest, ثابت حرف *src, size_t len)
{
    حرف *d = dest;
    بينما (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    إرجاع dest;
}

// locate first occurrence of character c in the string s
حرف *
strchr(ثابت حرف *s, صحيح c)
{
    لكل (; *s; s++)
        إذا (*s == c)
            إرجاع (حرف*)s;
    إرجاع عدم;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
حرف *
nullTrailingSpace(حرف *buf)
{
    صحيح len = strlen(buf);
    حرف *end = &buf[len-1];
    بينما (end >= buf && *end <= ' ')
        *(end--) = '\0';
    بينما (*buf && *buf <= ' ')
        buf++;
    إرجاع buf;
}

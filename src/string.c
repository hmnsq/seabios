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
    سیٹ_SEG(ES, buf_seg);
    u32 i;
    u8 جوڑ = 0;
    لكل (i=0; i<len; i++)
        جوڑ += GET_VAR(ES, ((u8*)buf_far)[i]);
    أرجع جوڑ;
}

u8
checksum(فراغ *buf, u32 len)
{
    أرجع checksum_far(GET_SEG(SS), buf, len);
}

ناپ_t
لڑی_لمبائی(ثابت حرف *s)
{
    إذا (__builtin_constant_p(s))
        أرجع __builtin_strlen(s);
    ثابت حرف *p = s;
    طالما (*p)
        p++;
    أرجع p-s;
}

صحيح
memcmp_far(u16 s1seg, ثابت فراغ *s1, u16 s2seg, ثابت فراغ *s2, ناپ_t n)
{
    طالما (n--) {
        صحيح d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        إذا (d)
            أرجع d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    أرجع 0;
}

// Compare two areas of memory.
صحيح
memcmp(ثابت فراغ *s1, ثابت فراغ *s2, ناپ_t n)
{
    طالما (n) {
        إذا (*(u8*)s1 != *(u8*)s2)
            أرجع *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    أرجع 0;
}

// Compare two strings.
صحيح
strcmp(ثابت حرف *s1, ثابت حرف *s2)
{
    لكل (;;) {
        إذا (*s1 != *s2)
            أرجع *s1 < *s2 ? -1 : 1;
        إذا (! *s1)
            أرجع 0;
        s1++;
        s2++;
    }
}

مضمن فراغ
memset_far(u16 d_seg, فراغ *d_far, u8 c, ناپ_t len)
{
    سیٹ_SEG(ES, d_seg);
    asm متطاير(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

مضمن فراغ
memset16_far(u16 d_seg, فراغ *d_far, u16 c, ناپ_t len)
{
    len /= 2;
    سیٹ_SEG(ES, d_seg);
    asm متطاير(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

فراغ *
memset(فراغ *s, صحيح c, ناپ_t n)
{
    طالما (n)
        ((حرف *)s)[--n] = c;
    أرجع s;
}

فراغ memset_fl(فراغ *ptr, u8 val, ناپ_t ناپ)
{
    إذا (MODESEGMENT)
        memset_far(FLATPTR_منزل_SEG(ptr), (فراغ*)(FLATPTR_منزل_وتھ(ptr)),
                   val, ناپ);
    وإلا
        memset(ptr, val, ناپ);
}

مضمن فراغ
memcpy_far(u16 d_seg, فراغ *d_far, u16 s_seg, ثابت فراغ *s_far, ناپ_t len)
{
    سیٹ_SEG(ES, d_seg);
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
memcpy_fl(فراغ *d_fl, ثابت فراغ *s_fl, ناپ_t len)
{
    إذا (MODESEGMENT)
        memcpy_far(FLATPTR_منزل_SEG(d_fl), (فراغ*)FLATPTR_منزل_وتھ(d_fl)
                   , FLATPTR_منزل_SEG(s_fl), (فراغ*)FLATPTR_منزل_وتھ(s_fl)
                   , len);
    وإلا
        حافظہ_نقل(d_fl, s_fl, len);
}

فراغ *
#تعریف_رد حافظہ_نقل
حافظہ_نقل(فراغ *d1, ثابت فراغ *s1, ناپ_t len)
#إذا MODESEGMENT == 0
#تعریف_کرو حافظہ_نقل __builtin_memcpy
#جے_انت
{
    سیٹ_SEG(ES, GET_SEG(SS));
    فراغ *d = d1;
    إذا (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm متطاير(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        أرجع d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm متطاير(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    أرجع d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
فراغ
iomemcpy(فراغ *d, ثابت فراغ *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    طالما (len > 3) {
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
        حافظہ_نقل(d, s, len);
}

فراغ *
memmove(فراغ *d, ثابت فراغ *s, ناپ_t len)
{
    إذا (s >= d)
        أرجع حافظہ_نقل(d, s, len);

    d += len-1;
    s += len-1;
    طالما (len--) {
        *(حرف*)d = *(حرف*)s;
        d--;
        s--;
    }

    أرجع d;
}

// Copy a string - truncating it if necessary.
حرف *
strtcpy(حرف *dest, ثابت حرف *src, ناپ_t len)
{
    حرف *d = dest;
    طالما (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    أرجع dest;
}

// locate first occurrence of character c in the string s
حرف *
strchr(ثابت حرف *s, صحيح c)
{
    لكل (; *s; s++)
        إذا (*s == c)
            أرجع (حرف*)s;
    أرجع عدم;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
حرف *
nullTrailingSpace(حرف *buf)
{
    صحيح len = لڑی_لمبائی(buf);
    حرف *سرا = &buf[len-1];
    طالما (سرا >= buf && *سرا <= ' ')
        *(سرا--) = '\0';
    طالما (*buf && *buf <= ' ')
        buf++;
    أرجع buf;
}

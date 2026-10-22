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
checksum_far(u16 buf_seg, خالی *buf_far, u32 len)
{
    سیٹ_SEG(ES, buf_seg);
    u32 i;
    u8 مجموع = 0;
    برائے (i=0; i<len; i++)
        مجموع += GET_VAR(ES, ((u8*)buf_far)[i]);
    واپس مجموع;
}

u8
checksum(خالی *buf, u32 len)
{
    واپس checksum_far(GET_SEG(SS), buf, len);
}

سائز_t
لڑی_لمبائی(ثابت حرف *s)
{
    اگر (__builtin_constant_p(s))
        واپس __builtin_strlen(s);
    ثابت حرف *p = s;
    جب (*p)
        p++;
    واپس p-s;
}

صحیح
memcmp_far(u16 s1seg, ثابت خالی *s1, u16 s2seg, ثابت خالی *s2, سائز_t n)
{
    جب (n--) {
        صحیح d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        اگر (d)
            واپس d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    واپس 0;
}

// Compare two areas of memory.
صحیح
memcmp(ثابت خالی *s1, ثابت خالی *s2, سائز_t n)
{
    جب (n) {
        اگر (*(u8*)s1 != *(u8*)s2)
            واپس *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    واپس 0;
}

// Compare two strings.
صحیح
strcmp(ثابت حرف *s1, ثابت حرف *s2)
{
    برائے (;;) {
        اگر (*s1 != *s2)
            واپس *s1 < *s2 ? -1 : 1;
        اگر (! *s1)
            واپس 0;
        s1++;
        s2++;
    }
}

اندرونی خالی
memset_far(u16 d_seg, خالی *d_far, u8 c, سائز_t len)
{
    سیٹ_SEG(ES, d_seg);
    asm متلون(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

اندرونی خالی
memset16_far(u16 d_seg, خالی *d_far, u16 c, سائز_t len)
{
    len /= 2;
    سیٹ_SEG(ES, d_seg);
    asm متلون(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

خالی *
memset(خالی *s, صحیح c, سائز_t n)
{
    جب (n)
        ((حرف *)s)[--n] = c;
    واپس s;
}

خالی memset_fl(خالی *ptr, u8 val, سائز_t سائز)
{
    اگر (MODESEGMENT)
        memset_far(FLATPTR_مقصد_SEG(ptr), (خالی*)(FLATPTR_مقصد_انحراف(ptr)),
                   val, سائز);
    ورنہ
        memset(ptr, val, سائز);
}

اندرونی خالی
memcpy_far(u16 d_seg, خالی *d_far, u16 s_seg, ثابت خالی *s_far, سائز_t len)
{
    سیٹ_SEG(ES, d_seg);
    u16 bkup_ds;
    asm متلون(
        "movw %%ds, %w0\n"
        "movw %w4, %%ds\n"
        "rep movsb (%%si),%%es:(%%di)\n"
        "movw %w0, %%ds"
        : "=&r"(bkup_ds), "+c"(len), "+S"(s_far), "+D"(d_far)
        : "r"(s_seg), "m" (__segment_ES)
        : "cc", "memory");
}

اندرونی خالی
memcpy_fl(خالی *d_fl, ثابت خالی *s_fl, سائز_t len)
{
    اگر (MODESEGMENT)
        memcpy_far(FLATPTR_مقصد_SEG(d_fl), (خالی*)FLATPTR_مقصد_انحراف(d_fl)
                   , FLATPTR_مقصد_SEG(s_fl), (خالی*)FLATPTR_مقصد_انحراف(s_fl)
                   , len);
    ورنہ
        حافظہ_نقل(d_fl, s_fl, len);
}

خالی *
#تعریف_منسوخ حافظہ_نقل
حافظہ_نقل(خالی *d1, ثابت خالی *s1, سائز_t len)
#اگر MODESEGMENT == 0
#تعریف_کرو حافظہ_نقل __builtin_memcpy
#اگر_ختم
{
    سیٹ_SEG(ES, GET_SEG(SS));
    خالی *d = d1;
    اگر (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm متلون(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        واپس d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm متلون(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    واپس d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
خالی
iomemcpy(خالی *d, ثابت خالی *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    جب (len > 3) {
        u32 copylen = len;
        اگر (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm متلون(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    اگر (len)
        // Copy any remaining bytes.
        حافظہ_نقل(d, s, len);
}

خالی *
memmove(خالی *d, ثابت خالی *s, سائز_t len)
{
    اگر (s >= d)
        واپس حافظہ_نقل(d, s, len);

    d += len-1;
    s += len-1;
    جب (len--) {
        *(حرف*)d = *(حرف*)s;
        d--;
        s--;
    }

    واپس d;
}

// Copy a string - truncating it if necessary.
حرف *
strtcpy(حرف *dest, ثابت حرف *src, سائز_t len)
{
    حرف *d = dest;
    جب (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    واپس dest;
}

// locate first occurrence of character c in the string s
حرف *
strchr(ثابت حرف *s, صحیح c)
{
    برائے (; *s; s++)
        اگر (*s == c)
            واپس (حرف*)s;
    واپس تہی;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
حرف *
nullTrailingSpace(حرف *buf)
{
    صحیح len = لڑی_لمبائی(buf);
    حرف *انتہا = &buf[len-1];
    جب (انتہا >= buf && *انتہا <= ' ')
        *(انتہا--) = '\0';
    جب (*buf && *buf <= ' ')
        buf++;
    واپس buf;
}

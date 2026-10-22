// String manipulation functions.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ਅਨ੍ਤਰ੍ਭਾਵ "stacks.h" // yield
#ਅਨ੍ਤਰ੍ਭਾਵ "string.h" // memcpy
#ਅਨ੍ਤਰ੍ਭਾਵ "farptr.h" // SET_SEG


/****************************************************************
 * String ops
 ****************************************************************/

// Sum the bytes in the specified area.
u8
checksum_far(u16 buf_seg, ਖਾਲੀ *buf_far, u32 len)
{
    ਸੈੱਟ_SEG(ES, buf_seg);
    u32 i;
    u8 ਜੋੜ = 0;
    ਲਈ (i=0; i<len; i++)
        ਜੋੜ += GET_VAR(ES, ((u8*)buf_far)[i]);
    ਵਾਪਸ ਜੋੜ;
}

u8
checksum(ਖਾਲੀ *buf, u32 len)
{
    ਵਾਪਸ checksum_far(GET_SEG(SS), buf, len);
}

ਮਾਪ_t
ਲੜੀ_ਲੰਬਾਈ(ਅਚਲ ਅੱਖਰ *s)
{
    ਜੇ (__builtin_constant_p(s))
        ਵਾਪਸ __builtin_strlen(s);
    ਅਚਲ ਅੱਖਰ *p = s;
    ਜਦੋਂ (*p)
        p++;
    ਵਾਪਸ p-s;
}

ਪੂਰਨ
memcmp_far(u16 s1seg, ਅਚਲ ਖਾਲੀ *s1, u16 s2seg, ਅਚਲ ਖਾਲੀ *s2, ਮਾਪ_t n)
{
    ਜਦੋਂ (n--) {
        ਪੂਰਨ d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        ਜੇ (d)
            ਵਾਪਸ d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    ਵਾਪਸ 0;
}

// Compare two areas of memory.
ਪੂਰਨ
memcmp(ਅਚਲ ਖਾਲੀ *s1, ਅਚਲ ਖਾਲੀ *s2, ਮਾਪ_t n)
{
    ਜਦੋਂ (n) {
        ਜੇ (*(u8*)s1 != *(u8*)s2)
            ਵਾਪਸ *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    ਵਾਪਸ 0;
}

// Compare two strings.
ਪੂਰਨ
strcmp(ਅਚਲ ਅੱਖਰ *s1, ਅਚਲ ਅੱਖਰ *s2)
{
    ਲਈ (;;) {
        ਜੇ (*s1 != *s2)
            ਵਾਪਸ *s1 < *s2 ? -1 : 1;
        ਜੇ (! *s1)
            ਵਾਪਸ 0;
        s1++;
        s2++;
    }
}

ਸਤਰਅੰਦਰ ਖਾਲੀ
memset_far(u16 d_seg, ਖਾਲੀ *d_far, u8 c, ਮਾਪ_t len)
{
    ਸੈੱਟ_SEG(ES, d_seg);
    asm ਅਸਥਿਰ(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

ਸਤਰਅੰਦਰ ਖਾਲੀ
memset16_far(u16 d_seg, ਖਾਲੀ *d_far, u16 c, ਮਾਪ_t len)
{
    len /= 2;
    ਸੈੱਟ_SEG(ES, d_seg);
    asm ਅਸਥਿਰ(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

ਖਾਲੀ *
memset(ਖਾਲੀ *s, ਪੂਰਨ c, ਮਾਪ_t n)
{
    ਜਦੋਂ (n)
        ((ਅੱਖਰ *)s)[--n] = c;
    ਵਾਪਸ s;
}

ਖਾਲੀ memset_fl(ਖਾਲੀ *ptr, u8 val, ਮਾਪ_t ਮਾਪ)
{
    ਜੇ (MODESEGMENT)
        memset_far(FLATPTR_ਮੰਜ਼ਿਲ_SEG(ptr), (ਖਾਲੀ*)(FLATPTR_ਮੰਜ਼ਿਲ_ਵਿੱਥ(ptr)),
                   val, ਮਾਪ);
    ਨਹੀਂਤਾਂ
        memset(ptr, val, ਮਾਪ);
}

ਸਤਰਅੰਦਰ ਖਾਲੀ
memcpy_far(u16 d_seg, ਖਾਲੀ *d_far, u16 s_seg, ਅਚਲ ਖਾਲੀ *s_far, ਮਾਪ_t len)
{
    ਸੈੱਟ_SEG(ES, d_seg);
    u16 bkup_ds;
    asm ਅਸਥਿਰ(
        "movw %%ds, %w0\n"
        "movw %w4, %%ds\n"
        "rep movsb (%%si),%%es:(%%di)\n"
        "movw %w0, %%ds"
        : "=&r"(bkup_ds), "+c"(len), "+S"(s_far), "+D"(d_far)
        : "r"(s_seg), "m" (__segment_ES)
        : "cc", "memory");
}

ਸਤਰਅੰਦਰ ਖਾਲੀ
memcpy_fl(ਖਾਲੀ *d_fl, ਅਚਲ ਖਾਲੀ *s_fl, ਮਾਪ_t len)
{
    ਜੇ (MODESEGMENT)
        memcpy_far(FLATPTR_ਮੰਜ਼ਿਲ_SEG(d_fl), (ਖਾਲੀ*)FLATPTR_ਮੰਜ਼ਿਲ_ਵਿੱਥ(d_fl)
                   , FLATPTR_ਮੰਜ਼ਿਲ_SEG(s_fl), (ਖਾਲੀ*)FLATPTR_ਮੰਜ਼ਿਲ_ਵਿੱਥ(s_fl)
                   , len);
    ਨਹੀਂਤਾਂ
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(d_fl, s_fl, len);
}

ਖਾਲੀ *
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ਸਮ੍ਰਿਤੀ_ਨਕਲ
ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਖਾਲੀ *d1, ਅਚਲ ਖਾਲੀ *s1, ਮਾਪ_t len)
#ਜੇ MODESEGMENT == 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਮ੍ਰਿਤੀ_ਨਕਲ __builtin_memcpy
#ਜੇ_ਅੰਤ
{
    ਸੈੱਟ_SEG(ES, GET_SEG(SS));
    ਖਾਲੀ *d = d1;
    ਜੇ (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm ਅਸਥਿਰ(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        ਵਾਪਸ d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm ਅਸਥਿਰ(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    ਵਾਪਸ d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
ਖਾਲੀ
iomemcpy(ਖਾਲੀ *d, ਅਚਲ ਖਾਲੀ *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    ਜਦੋਂ (len > 3) {
        u32 copylen = len;
        ਜੇ (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm ਅਸਥਿਰ(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    ਜੇ (len)
        // Copy any remaining bytes.
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(d, s, len);
}

ਖਾਲੀ *
memmove(ਖਾਲੀ *d, ਅਚਲ ਖਾਲੀ *s, ਮਾਪ_t len)
{
    ਜੇ (s >= d)
        ਵਾਪਸ ਸਮ੍ਰਿਤੀ_ਨਕਲ(d, s, len);

    d += len-1;
    s += len-1;
    ਜਦੋਂ (len--) {
        *(ਅੱਖਰ*)d = *(ਅੱਖਰ*)s;
        d--;
        s--;
    }

    ਵਾਪਸ d;
}

// Copy a string - truncating it if necessary.
ਅੱਖਰ *
strtcpy(ਅੱਖਰ *dest, ਅਚਲ ਅੱਖਰ *src, ਮਾਪ_t len)
{
    ਅੱਖਰ *d = dest;
    ਜਦੋਂ (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    ਵਾਪਸ dest;
}

// locate first occurrence of character c in the string s
ਅੱਖਰ *
strchr(ਅਚਲ ਅੱਖਰ *s, ਪੂਰਨ c)
{
    ਲਈ (; *s; s++)
        ਜੇ (*s == c)
            ਵਾਪਸ (ਅੱਖਰ*)s;
    ਵਾਪਸ ਨਿਰਾ;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
ਅੱਖਰ *
nullTrailingSpace(ਅੱਖਰ *buf)
{
    ਪੂਰਨ len = ਲੜੀ_ਲੰਬਾਈ(buf);
    ਅੱਖਰ *ਸਿਰਾ = &buf[len-1];
    ਜਦੋਂ (ਸਿਰਾ >= buf && *ਸਿਰਾ <= ' ')
        *(ਸਿਰਾ--) = '\0';
    ਜਦੋਂ (*buf && *buf <= ' ')
        buf++;
    ਵਾਪਸ buf;
}

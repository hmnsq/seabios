// String manipulation functions.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ಅನ್ತರ್ಭಾವ "stacks.h" // yield
#ಅನ್ತರ್ಭಾವ "string.h" // memcpy
#ಅನ್ತರ್ಭಾವ "farptr.h" // SET_SEG


/****************************************************************
 * String ops
 ****************************************************************/

// Sum the bytes in the specified area.
u8
checksum_far(u16 buf_seg, ಶೂನ್ಯ *buf_far, u32 len)
{
    ಗಣ_SEG(ES, buf_seg);
    u32 i;
    u8 ಮೊತ್ತ = 0;
    ಗಾಗಿ (i=0; i<len; i++)
        ಮೊತ್ತ += GET_VAR(ES, ((u8*)buf_far)[i]);
    ಹಿಂತಿರುಗಿ ಮೊತ್ತ;
}

u8
checksum(ಶೂನ್ಯ *buf, u32 len)
{
    ಹಿಂತಿರುಗಿ checksum_far(GET_SEG(SS), buf, len);
}

ಗಾತ್ರ_t
ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
    ಆದರೆ (__builtin_constant_p(s))
        ಹಿಂತಿರುಗಿ __builtin_strlen(s);
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = s;
    ಆಗ (*p)
        p++;
    ಹಿಂತಿರುಗಿ p-s;
}

ಪೂರ್ಣಾಂಕ
memcmp_far(u16 s1seg, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s1, u16 s2seg, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s2, ಗಾತ್ರ_t n)
{
    ಆಗ (n--) {
        ಪೂರ್ಣಾಂಕ d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        ಆದರೆ (d)
            ಹಿಂತಿರುಗಿ d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    ಹಿಂತಿರುಗಿ 0;
}

// Compare two areas of memory.
ಪೂರ್ಣಾಂಕ
memcmp(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s1, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s2, ಗಾತ್ರ_t n)
{
    ಆಗ (n) {
        ಆದರೆ (*(u8*)s1 != *(u8*)s2)
            ಹಿಂತಿರುಗಿ *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    ಹಿಂತಿರುಗಿ 0;
}

// Compare two strings.
ಪೂರ್ಣಾಂಕ
strcmp(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s1, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s2)
{
    ಗಾಗಿ (;;) {
        ಆದರೆ (*s1 != *s2)
            ಹಿಂತಿರುಗಿ *s1 < *s2 ? -1 : 1;
        ಆದರೆ (! *s1)
            ಹಿಂತಿರುಗಿ 0;
        s1++;
        s2++;
    }
}

ಸಾಲೊಳಗೆ ಶೂನ್ಯ
memset_far(u16 d_seg, ಶೂನ್ಯ *d_far, u8 c, ಗಾತ್ರ_t len)
{
    ಗಣ_SEG(ES, d_seg);
    asm ಅಸ್ಥಿರ(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

ಸಾಲೊಳಗೆ ಶೂನ್ಯ
memset16_far(u16 d_seg, ಶೂನ್ಯ *d_far, u16 c, ಗಾತ್ರ_t len)
{
    len /= 2;
    ಗಣ_SEG(ES, d_seg);
    asm ಅಸ್ಥಿರ(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

ಶೂನ್ಯ *
memset(ಶೂನ್ಯ *s, ಪೂರ್ಣಾಂಕ c, ಗಾತ್ರ_t n)
{
    ಆಗ (n)
        ((ಅಕ್ಷರ *)s)[--n] = c;
    ಹಿಂತಿರುಗಿ s;
}

ಶೂನ್ಯ memset_fl(ಶೂನ್ಯ *ptr, u8 val, ಗಾತ್ರ_t ಗಾತ್ರ_2)
{
    ಆದರೆ (MODESEGMENT)
        memset_far(FLATPTR_ಗಮ್ಯ_SEG(ptr), (ಶೂನ್ಯ*)(FLATPTR_ಗಮ್ಯ_ಸ್ಥಾನಾಂತರ(ptr)),
                   val, ಗಾತ್ರ_2);
    ಇಲ್ಲದಿದ್ದರೆ
        memset(ptr, val, ಗಾತ್ರ_2);
}

ಸಾಲೊಳಗೆ ಶೂನ್ಯ
memcpy_far(u16 d_seg, ಶೂನ್ಯ *d_far, u16 s_seg, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s_far, ಗಾತ್ರ_t len)
{
    ಗಣ_SEG(ES, d_seg);
    u16 bkup_ds;
    asm ಅಸ್ಥಿರ(
        "movw %%ds, %w0\n"
        "movw %w4, %%ds\n"
        "rep movsb (%%si),%%es:(%%di)\n"
        "movw %w0, %%ds"
        : "=&r"(bkup_ds), "+c"(len), "+S"(s_far), "+D"(d_far)
        : "r"(s_seg), "m" (__segment_ES)
        : "cc", "memory");
}

ಸಾಲೊಳಗೆ ಶೂನ್ಯ
memcpy_fl(ಶೂನ್ಯ *d_fl, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s_fl, ಗಾತ್ರ_t len)
{
    ಆದರೆ (MODESEGMENT)
        memcpy_far(FLATPTR_ಗಮ್ಯ_SEG(d_fl), (ಶೂನ್ಯ*)FLATPTR_ಗಮ್ಯ_ಸ್ಥಾನಾಂತರ(d_fl)
                   , FLATPTR_ಗಮ್ಯ_SEG(s_fl), (ಶೂನ್ಯ*)FLATPTR_ಗಮ್ಯ_ಸ್ಥಾನಾಂತರ(s_fl)
                   , len);
    ಇಲ್ಲದಿದ್ದರೆ
        ಸ್ಮರಣೆ_ನಕಲು(d_fl, s_fl, len);
}

ಶೂನ್ಯ *
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ಸ್ಮರಣೆ_ನಕಲು
ಸ್ಮರಣೆ_ನಕಲು(ಶೂನ್ಯ *d1, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s1, ಗಾತ್ರ_t len)
#ಆದರೆ MODESEGMENT == 0
#ವ್ಯಾಖ್ಯಾನಿಸು ಸ್ಮರಣೆ_ನಕಲು __builtin_memcpy
#ಆದರೆ_ಅಂತ್ಯ
{
    ಗಣ_SEG(ES, GET_SEG(SS));
    ಶೂನ್ಯ *d = d1;
    ಆದರೆ (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm ಅಸ್ಥಿರ(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        ಹಿಂತಿರುಗಿ d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm ಅಸ್ಥಿರ(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    ಹಿಂತಿರುಗಿ d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
ಶೂನ್ಯ
iomemcpy(ಶೂನ್ಯ *d, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    ಆಗ (len > 3) {
        u32 copylen = len;
        ಆದರೆ (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm ಅಸ್ಥಿರ(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    ಆದರೆ (len)
        // Copy any remaining bytes.
        ಸ್ಮರಣೆ_ನಕಲು(d, s, len);
}

ಶೂನ್ಯ *
memmove(ಶೂನ್ಯ *d, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s, ಗಾತ್ರ_t len)
{
    ಆದರೆ (s >= d)
        ಹಿಂತಿರುಗಿ ಸ್ಮರಣೆ_ನಕಲು(d, s, len);

    d += len-1;
    s += len-1;
    ಆಗ (len--) {
        *(ಅಕ್ಷರ*)d = *(ಅಕ್ಷರ*)s;
        d--;
        s--;
    }

    ಹಿಂತಿರುಗಿ d;
}

// Copy a string - truncating it if necessary.
ಅಕ್ಷರ *
strtcpy(ಅಕ್ಷರ *dest, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src, ಗಾತ್ರ_t len)
{
    ಅಕ್ಷರ *d = dest;
    ಆಗ (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    ಹಿಂತಿರುಗಿ dest;
}

// locate first occurrence of character c in the string s
ಅಕ್ಷರ *
strchr(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಪೂರ್ಣಾಂಕ c)
{
    ಗಾಗಿ (; *s; s++)
        ಆದರೆ (*s == c)
            ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ*)s;
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
ಅಕ್ಷರ *
nullTrailingSpace(ಅಕ್ಷರ *buf)
{
    ಪೂರ್ಣಾಂಕ len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(buf);
    ಅಕ್ಷರ *ಕೊನೆ = &buf[len-1];
    ಆಗ (ಕೊನೆ >= buf && *ಕೊನೆ <= ' ')
        *(ಕೊನೆ--) = '\0';
    ಆಗ (*buf && *buf <= ' ')
        buf++;
    ಹಿಂತಿರುಗಿ buf;
}

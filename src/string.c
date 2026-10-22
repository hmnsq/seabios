// String manipulation functions.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#অন্তর্ভাব "stacks.h" // yield
#অন্তর্ভাব "string.h" // memcpy
#অন্তর্ভাব "farptr.h" // SET_SEG


/****************************************************************
 * String ops
 ****************************************************************/

// Sum the bytes in the specified area.
u8
checksum_far(u16 buf_seg, শূন্য *buf_far, u32 len)
{
    SET_SEG(ES, buf_seg);
    u32 i;
    u8 যোগ = 0;
    জন্য (i=0; i<len; i++)
        যোগ += GET_VAR(ES, ((u8*)buf_far)[i]);
    ফেরত যোগ;
}

u8
checksum(শূন্য *buf, u32 len)
{
    ফেরত checksum_far(GET_SEG(SS), buf, len);
}

আকার_t
strlen(ধ্রুবক অক্ষর *s)
{
    যদি (__builtin_constant_p(s))
        ফেরত __builtin_strlen(s);
    ধ্রুবক অক্ষর *p = s;
    যখন (*p)
        p++;
    ফেরত p-s;
}

পূর্ণসংখ্যা
memcmp_far(u16 s1seg, ধ্রুবক শূন্য *s1, u16 s2seg, ধ্রুবক শূন্য *s2, আকার_t n)
{
    যখন (n--) {
        পূর্ণসংখ্যা d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        যদি (d)
            ফেরত d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    ফেরত 0;
}

// Compare two areas of memory.
পূর্ণসংখ্যা
memcmp(ধ্রুবক শূন্য *s1, ধ্রুবক শূন্য *s2, আকার_t n)
{
    যখন (n) {
        যদি (*(u8*)s1 != *(u8*)s2)
            ফেরত *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    ফেরত 0;
}

// Compare two strings.
পূর্ণসংখ্যা
strcmp(ধ্রুবক অক্ষর *s1, ধ্রুবক অক্ষর *s2)
{
    জন্য (;;) {
        যদি (*s1 != *s2)
            ফেরত *s1 < *s2 ? -1 : 1;
        যদি (! *s1)
            ফেরত 0;
        s1++;
        s2++;
    }
}

অন্তঃরেখ শূন্য
memset_far(u16 d_seg, শূন্য *d_far, u8 c, আকার_t len)
{
    SET_SEG(ES, d_seg);
    asm অস্থির(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

অন্তঃরেখ শূন্য
memset16_far(u16 d_seg, শূন্য *d_far, u16 c, আকার_t len)
{
    len /= 2;
    SET_SEG(ES, d_seg);
    asm অস্থির(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

শূন্য *
memset(শূন্য *s, পূর্ণসংখ্যা c, আকার_t n)
{
    যখন (n)
        ((অক্ষর *)s)[--n] = c;
    ফেরত s;
}

শূন্য memset_fl(শূন্য *ptr, u8 val, আকার_t আকার_2)
{
    যদি (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (শূন্য*)(FLATPTR_TO_OFFSET(ptr)),
                   val, আকার_2);
    অন্যথা
        memset(ptr, val, আকার_2);
}

অন্তঃরেখ শূন্য
memcpy_far(u16 d_seg, শূন্য *d_far, u16 s_seg, ধ্রুবক শূন্য *s_far, আকার_t len)
{
    SET_SEG(ES, d_seg);
    u16 bkup_ds;
    asm অস্থির(
        "movw %%ds, %w0\n"
        "movw %w4, %%ds\n"
        "rep movsb (%%si),%%es:(%%di)\n"
        "movw %w0, %%ds"
        : "=&r"(bkup_ds), "+c"(len), "+S"(s_far), "+D"(d_far)
        : "r"(s_seg), "m" (__segment_ES)
        : "cc", "memory");
}

অন্তঃরেখ শূন্য
memcpy_fl(শূন্য *d_fl, ধ্রুবক শূন্য *s_fl, আকার_t len)
{
    যদি (MODESEGMENT)
        memcpy_far(FLATPTR_TO_SEG(d_fl), (শূন্য*)FLATPTR_TO_OFFSET(d_fl)
                   , FLATPTR_TO_SEG(s_fl), (শূন্য*)FLATPTR_TO_OFFSET(s_fl)
                   , len);
    অন্যথা
        memcpy(d_fl, s_fl, len);
}

শূন্য *
#undef memcpy
memcpy(শূন্য *d1, ধ্রুবক শূন্য *s1, আকার_t len)
#যদি MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
{
    SET_SEG(ES, GET_SEG(SS));
    শূন্য *d = d1;
    যদি (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm অস্থির(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        ফেরত d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm অস্থির(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    ফেরত d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
শূন্য
iomemcpy(শূন্য *d, ধ্রুবক শূন্য *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    যখন (len > 3) {
        u32 copylen = len;
        যদি (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm অস্থির(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    যদি (len)
        // Copy any remaining bytes.
        memcpy(d, s, len);
}

শূন্য *
memmove(শূন্য *d, ধ্রুবক শূন্য *s, আকার_t len)
{
    যদি (s >= d)
        ফেরত memcpy(d, s, len);

    d += len-1;
    s += len-1;
    যখন (len--) {
        *(অক্ষর*)d = *(অক্ষর*)s;
        d--;
        s--;
    }

    ফেরত d;
}

// Copy a string - truncating it if necessary.
অক্ষর *
strtcpy(অক্ষর *dest, ধ্রুবক অক্ষর *src, আকার_t len)
{
    অক্ষর *d = dest;
    যখন (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    ফেরত dest;
}

// locate first occurrence of character c in the string s
অক্ষর *
strchr(ধ্রুবক অক্ষর *s, পূর্ণসংখ্যা c)
{
    জন্য (; *s; s++)
        যদি (*s == c)
            ফেরত (অক্ষর*)s;
    ফেরত ফাঁকা;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
অক্ষর *
nullTrailingSpace(অক্ষর *buf)
{
    পূর্ণসংখ্যা len = strlen(buf);
    অক্ষর *end = &buf[len-1];
    যখন (end >= buf && *end <= ' ')
        *(end--) = '\0';
    যখন (*buf && *buf <= ' ')
        buf++;
    ফেরত buf;
}

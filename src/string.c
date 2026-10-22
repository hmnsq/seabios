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
    u8 sum = 0;
    প্রতি (i=0; i<len; i++)
        sum += GET_VAR(ES, ((u8*)buf_far)[i]);
    প্রত্যাগম sum;
}

u8
checksum(শূন্য *buf, u32 len)
{
    প্রত্যাগম checksum_far(GET_SEG(SS), buf, len);
}

size_t
strlen(ধ্রুব বর্ণ *s)
{
    যদি (__builtin_constant_p(s))
        প্রত্যাগম __builtin_strlen(s);
    ধ্রুব বর্ণ *p = s;
    যাবত্ (*p)
        p++;
    প্রত্যাগম p-s;
}

পূর্ণাঙ্ক
memcmp_far(u16 s1seg, ধ্রুব শূন্য *s1, u16 s2seg, ধ্রুব শূন্য *s2, size_t n)
{
    যাবত্ (n--) {
        পূর্ণাঙ্ক d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        যদি (d)
            প্রত্যাগম d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    প্রত্যাগম 0;
}

// Compare two areas of memory.
পূর্ণাঙ্ক
memcmp(ধ্রুব শূন্য *s1, ধ্রুব শূন্য *s2, size_t n)
{
    যাবত্ (n) {
        যদি (*(u8*)s1 != *(u8*)s2)
            প্রত্যাগম *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    প্রত্যাগম 0;
}

// Compare two strings.
পূর্ণাঙ্ক
strcmp(ধ্রুব বর্ণ *s1, ধ্রুব বর্ণ *s2)
{
    প্রতি (;;) {
        যদি (*s1 != *s2)
            প্রত্যাগম *s1 < *s2 ? -1 : 1;
        যদি (! *s1)
            প্রত্যাগম 0;
        s1++;
        s2++;
    }
}

অন্তঃস্থ শূন্য
memset_far(u16 d_seg, শূন্য *d_far, u8 c, size_t len)
{
    SET_SEG(ES, d_seg);
    asm চঞ্চল(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

অন্তঃস্থ শূন্য
memset16_far(u16 d_seg, শূন্য *d_far, u16 c, size_t len)
{
    len /= 2;
    SET_SEG(ES, d_seg);
    asm চঞ্চল(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

শূন্য *
memset(শূন্য *s, পূর্ণাঙ্ক c, size_t n)
{
    যাবত্ (n)
        ((বর্ণ *)s)[--n] = c;
    প্রত্যাগম s;
}

শূন্য memset_fl(শূন্য *ptr, u8 val, size_t size)
{
    যদি (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (শূন্য*)(FLATPTR_TO_OFFSET(ptr)),
                   val, size);
    অন্যথা
        memset(ptr, val, size);
}

অন্তঃস্থ শূন্য
memcpy_far(u16 d_seg, শূন্য *d_far, u16 s_seg, ধ্রুব শূন্য *s_far, size_t len)
{
    SET_SEG(ES, d_seg);
    u16 bkup_ds;
    asm চঞ্চল(
        "movw %%ds, %w0\n"
        "movw %w4, %%ds\n"
        "rep movsb (%%si),%%es:(%%di)\n"
        "movw %w0, %%ds"
        : "=&r"(bkup_ds), "+c"(len), "+S"(s_far), "+D"(d_far)
        : "r"(s_seg), "m" (__segment_ES)
        : "cc", "memory");
}

অন্তঃস্থ শূন্য
memcpy_fl(শূন্য *d_fl, ধ্রুব শূন্য *s_fl, size_t len)
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
memcpy(শূন্য *d1, ধ্রুব শূন্য *s1, size_t len)
#যদি MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
{
    SET_SEG(ES, GET_SEG(SS));
    শূন্য *d = d1;
    যদি (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm চঞ্চল(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        প্রত্যাগম d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm চঞ্চল(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    প্রত্যাগম d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
শূন্য
iomemcpy(শূন্য *d, ধ্রুব শূন্য *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    যাবত্ (len > 3) {
        u32 copylen = len;
        যদি (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm চঞ্চল(
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
memmove(শূন্য *d, ধ্রুব শূন্য *s, size_t len)
{
    যদি (s >= d)
        প্রত্যাগম memcpy(d, s, len);

    d += len-1;
    s += len-1;
    যাবত্ (len--) {
        *(বর্ণ*)d = *(বর্ণ*)s;
        d--;
        s--;
    }

    প্রত্যাগম d;
}

// Copy a string - truncating it if necessary.
বর্ণ *
strtcpy(বর্ণ *dest, ধ্রুব বর্ণ *src, size_t len)
{
    বর্ণ *d = dest;
    যাবত্ (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    প্রত্যাগম dest;
}

// locate first occurrence of character c in the string s
বর্ণ *
strchr(ধ্রুব বর্ণ *s, পূর্ণাঙ্ক c)
{
    প্রতি (; *s; s++)
        যদি (*s == c)
            প্রত্যাগম (বর্ণ*)s;
    প্রত্যাগম রিক্ত;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
বর্ণ *
nullTrailingSpace(বর্ণ *buf)
{
    পূর্ণাঙ্ক len = strlen(buf);
    বর্ণ *end = &buf[len-1];
    যাবত্ (end >= buf && *end <= ' ')
        *(end--) = '\0';
    যাবত্ (*buf && *buf <= ' ')
        buf++;
    প্রত্যাগম buf;
}

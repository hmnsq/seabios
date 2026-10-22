// String manipulation functions.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#ଅନ୍ତର୍ଭାଵ "stacks.h" // yield
#ଅନ୍ତର୍ଭାଵ "string.h" // memcpy
#ଅନ୍ତର୍ଭାଵ "farptr.h" // SET_SEG


/****************************************************************
 * String ops
 ****************************************************************/

// Sum the bytes in the specified area.
u8
checksum_far(u16 buf_seg, ଶୂନ୍ୟ *buf_far, u32 len)
{
    SET_SEG(ES, buf_seg);
    u32 i;
    u8 sum = 0;
    ପାଇଁ (i=0; i<len; i++)
        sum += GET_VAR(ES, ((u8*)buf_far)[i]);
    ଫେରନ୍ତୁ sum;
}

u8
checksum(ଶୂନ୍ୟ *buf, u32 len)
{
    ଫେରନ୍ତୁ checksum_far(GET_SEG(SS), buf, len);
}

size_t
strlen(ଅଚଳ ଅକ୍ଷର *s)
{
    ଯଦି (__builtin_constant_p(s))
        ଫେରନ୍ତୁ __builtin_strlen(s);
    ଅଚଳ ଅକ୍ଷର *p = s;
    ଯେତେବେଳେ (*p)
        p++;
    ଫେରନ୍ତୁ p-s;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
memcmp_far(u16 s1seg, ଅଚଳ ଶୂନ୍ୟ *s1, u16 s2seg, ଅଚଳ ଶୂନ୍ୟ *s2, size_t n)
{
    ଯେତେବେଳେ (n--) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        ଯଦି (d)
            ଫେରନ୍ତୁ d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    ଫେରନ୍ତୁ 0;
}

// Compare two areas of memory.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
memcmp(ଅଚଳ ଶୂନ୍ୟ *s1, ଅଚଳ ଶୂନ୍ୟ *s2, size_t n)
{
    ଯେତେବେଳେ (n) {
        ଯଦି (*(u8*)s1 != *(u8*)s2)
            ଫେରନ୍ତୁ *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    ଫେରନ୍ତୁ 0;
}

// Compare two strings.
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
strcmp(ଅଚଳ ଅକ୍ଷର *s1, ଅଚଳ ଅକ୍ଷର *s2)
{
    ପାଇଁ (;;) {
        ଯଦି (*s1 != *s2)
            ଫେରନ୍ତୁ *s1 < *s2 ? -1 : 1;
        ଯଦି (! *s1)
            ଫେରନ୍ତୁ 0;
        s1++;
        s2++;
    }
}

ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
memset_far(u16 d_seg, ଶୂନ୍ୟ *d_far, u8 c, size_t len)
{
    SET_SEG(ES, d_seg);
    asm ଅସ୍ଥିର(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
memset16_far(u16 d_seg, ଶୂନ୍ୟ *d_far, u16 c, size_t len)
{
    len /= 2;
    SET_SEG(ES, d_seg);
    asm ଅସ୍ଥିର(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

ଶୂନ୍ୟ *
memset(ଶୂନ୍ୟ *s, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c, size_t n)
{
    ଯେତେବେଳେ (n)
        ((ଅକ୍ଷର *)s)[--n] = c;
    ଫେରନ୍ତୁ s;
}

ଶୂନ୍ୟ memset_fl(ଶୂନ୍ୟ *ptr, u8 val, size_t size)
{
    ଯଦି (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (ଶୂନ୍ୟ*)(FLATPTR_TO_OFFSET(ptr)),
                   val, size);
    ଅନ୍ୟଥା
        memset(ptr, val, size);
}

ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
memcpy_far(u16 d_seg, ଶୂନ୍ୟ *d_far, u16 s_seg, ଅଚଳ ଶୂନ୍ୟ *s_far, size_t len)
{
    SET_SEG(ES, d_seg);
    u16 bkup_ds;
    asm ଅସ୍ଥିର(
        "movw %%ds, %w0\n"
        "movw %w4, %%ds\n"
        "rep movsb (%%si),%%es:(%%di)\n"
        "movw %w0, %%ds"
        : "=&r"(bkup_ds), "+c"(len), "+S"(s_far), "+D"(d_far)
        : "r"(s_seg), "m" (__segment_ES)
        : "cc", "memory");
}

ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
memcpy_fl(ଶୂନ୍ୟ *d_fl, ଅଚଳ ଶୂନ୍ୟ *s_fl, size_t len)
{
    ଯଦି (MODESEGMENT)
        memcpy_far(FLATPTR_TO_SEG(d_fl), (ଶୂନ୍ୟ*)FLATPTR_TO_OFFSET(d_fl)
                   , FLATPTR_TO_SEG(s_fl), (ଶୂନ୍ୟ*)FLATPTR_TO_OFFSET(s_fl)
                   , len);
    ଅନ୍ୟଥା
        memcpy(d_fl, s_fl, len);
}

ଶୂନ୍ୟ *
#undef memcpy
memcpy(ଶୂନ୍ୟ *d1, ଅଚଳ ଶୂନ୍ୟ *s1, size_t len)
#ଯଦି MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
{
    SET_SEG(ES, GET_SEG(SS));
    ଶୂନ୍ୟ *d = d1;
    ଯଦି (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm ଅସ୍ଥିର(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        ଫେରନ୍ତୁ d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm ଅସ୍ଥିର(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    ଫେରନ୍ତୁ d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
ଶୂନ୍ୟ
iomemcpy(ଶୂନ୍ୟ *d, ଅଚଳ ଶୂନ୍ୟ *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    ଯେତେବେଳେ (len > 3) {
        u32 copylen = len;
        ଯଦି (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm ଅସ୍ଥିର(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    ଯଦି (len)
        // Copy any remaining bytes.
        memcpy(d, s, len);
}

ଶୂନ୍ୟ *
memmove(ଶୂନ୍ୟ *d, ଅଚଳ ଶୂନ୍ୟ *s, size_t len)
{
    ଯଦି (s >= d)
        ଫେରନ୍ତୁ memcpy(d, s, len);

    d += len-1;
    s += len-1;
    ଯେତେବେଳେ (len--) {
        *(ଅକ୍ଷର*)d = *(ଅକ୍ଷର*)s;
        d--;
        s--;
    }

    ଫେରନ୍ତୁ d;
}

// Copy a string - truncating it if necessary.
ଅକ୍ଷର *
strtcpy(ଅକ୍ଷର *dest, ଅଚଳ ଅକ୍ଷର *src, size_t len)
{
    ଅକ୍ଷର *d = dest;
    ଯେତେବେଳେ (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    ଫେରନ୍ତୁ dest;
}

// locate first occurrence of character c in the string s
ଅକ୍ଷର *
strchr(ଅଚଳ ଅକ୍ଷର *s, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c)
{
    ପାଇଁ (; *s; s++)
        ଯଦି (*s == c)
            ଫେରନ୍ତୁ (ଅକ୍ଷର*)s;
    ଫେରନ୍ତୁ ଖାଲି;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
ଅକ୍ଷର *
nullTrailingSpace(ଅକ୍ଷର *buf)
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = strlen(buf);
    ଅକ୍ଷର *end = &buf[len-1];
    ଯେତେବେଳେ (end >= buf && *end <= ' ')
        *(end--) = '\0';
    ଯେତେବେଳେ (*buf && *buf <= ' ')
        buf++;
    ଫେରନ୍ତୁ buf;
}

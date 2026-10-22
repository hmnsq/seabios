// String manipulation functions.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#અન્તર્ભાવ "stacks.h" // yield
#અન્તર્ભાવ "string.h" // memcpy
#અન્તર્ભાવ "farptr.h" // SET_SEG


/****************************************************************
 * String ops
 ****************************************************************/

// Sum the bytes in the specified area.
u8
checksum_far(u16 buf_seg, શૂન્ય *buf_far, u32 len)
{
    SET_SEG(ES, buf_seg);
    u32 i;
    u8 sum = 0;
    માટે (i=0; i<len; i++)
        sum += GET_VAR(ES, ((u8*)buf_far)[i]);
    પાછા sum;
}

u8
checksum(શૂન્ય *buf, u32 len)
{
    પાછા checksum_far(GET_SEG(SS), buf, len);
}

size_t
strlen(અચલ અક્ષર *s)
{
    જો (__builtin_constant_p(s))
        પાછા __builtin_strlen(s);
    અચલ અક્ષર *p = s;
    જ્યારે (*p)
        p++;
    પાછા p-s;
}

પૂર્ણાંક
memcmp_far(u16 s1seg, અચલ શૂન્ય *s1, u16 s2seg, અચલ શૂન્ય *s2, size_t n)
{
    જ્યારે (n--) {
        પૂર્ણાંક d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        જો (d)
            પાછા d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    પાછા 0;
}

// Compare two areas of memory.
પૂર્ણાંક
memcmp(અચલ શૂન્ય *s1, અચલ શૂન્ય *s2, size_t n)
{
    જ્યારે (n) {
        જો (*(u8*)s1 != *(u8*)s2)
            પાછા *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    પાછા 0;
}

// Compare two strings.
પૂર્ણાંક
strcmp(અચલ અક્ષર *s1, અચલ અક્ષર *s2)
{
    માટે (;;) {
        જો (*s1 != *s2)
            પાછા *s1 < *s2 ? -1 : 1;
        જો (! *s1)
            પાછા 0;
        s1++;
        s2++;
    }
}

અંતઃરેખ શૂન્ય
memset_far(u16 d_seg, શૂન્ય *d_far, u8 c, size_t len)
{
    SET_SEG(ES, d_seg);
    asm અસ્થિર(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

અંતઃરેખ શૂન્ય
memset16_far(u16 d_seg, શૂન્ય *d_far, u16 c, size_t len)
{
    len /= 2;
    SET_SEG(ES, d_seg);
    asm અસ્થિર(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

શૂન્ય *
memset(શૂન્ય *s, પૂર્ણાંક c, size_t n)
{
    જ્યારે (n)
        ((અક્ષર *)s)[--n] = c;
    પાછા s;
}

શૂન્ય memset_fl(શૂન્ય *ptr, u8 val, size_t size)
{
    જો (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (શૂન્ય*)(FLATPTR_TO_OFFSET(ptr)),
                   val, size);
    નહીંતર
        memset(ptr, val, size);
}

અંતઃરેખ શૂન્ય
memcpy_far(u16 d_seg, શૂન્ય *d_far, u16 s_seg, અચલ શૂન્ય *s_far, size_t len)
{
    SET_SEG(ES, d_seg);
    u16 bkup_ds;
    asm અસ્થિર(
        "movw %%ds, %w0\n"
        "movw %w4, %%ds\n"
        "rep movsb (%%si),%%es:(%%di)\n"
        "movw %w0, %%ds"
        : "=&r"(bkup_ds), "+c"(len), "+S"(s_far), "+D"(d_far)
        : "r"(s_seg), "m" (__segment_ES)
        : "cc", "memory");
}

અંતઃરેખ શૂન્ય
memcpy_fl(શૂન્ય *d_fl, અચલ શૂન્ય *s_fl, size_t len)
{
    જો (MODESEGMENT)
        memcpy_far(FLATPTR_TO_SEG(d_fl), (શૂન્ય*)FLATPTR_TO_OFFSET(d_fl)
                   , FLATPTR_TO_SEG(s_fl), (શૂન્ય*)FLATPTR_TO_OFFSET(s_fl)
                   , len);
    નહીંતર
        memcpy(d_fl, s_fl, len);
}

શૂન્ય *
#undef memcpy
memcpy(શૂન્ય *d1, અચલ શૂન્ય *s1, size_t len)
#જો MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
{
    SET_SEG(ES, GET_SEG(SS));
    શૂન્ય *d = d1;
    જો (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm અસ્થિર(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        પાછા d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm અસ્થિર(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    પાછા d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
શૂન્ય
iomemcpy(શૂન્ય *d, અચલ શૂન્ય *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    જ્યારે (len > 3) {
        u32 copylen = len;
        જો (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm અસ્થિર(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    જો (len)
        // Copy any remaining bytes.
        memcpy(d, s, len);
}

શૂન્ય *
memmove(શૂન્ય *d, અચલ શૂન્ય *s, size_t len)
{
    જો (s >= d)
        પાછા memcpy(d, s, len);

    d += len-1;
    s += len-1;
    જ્યારે (len--) {
        *(અક્ષર*)d = *(અક્ષર*)s;
        d--;
        s--;
    }

    પાછા d;
}

// Copy a string - truncating it if necessary.
અક્ષર *
strtcpy(અક્ષર *dest, અચલ અક્ષર *src, size_t len)
{
    અક્ષર *d = dest;
    જ્યારે (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    પાછા dest;
}

// locate first occurrence of character c in the string s
અક્ષર *
strchr(અચલ અક્ષર *s, પૂર્ણાંક c)
{
    માટે (; *s; s++)
        જો (*s == c)
            પાછા (અક્ષર*)s;
    પાછા ખાલી;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
અક્ષર *
nullTrailingSpace(અક્ષર *buf)
{
    પૂર્ણાંક len = strlen(buf);
    અક્ષર *end = &buf[len-1];
    જ્યારે (end >= buf && *end <= ' ')
        *(end--) = '\0';
    જ્યારે (*buf && *buf <= ' ')
        buf++;
    પાછા buf;
}

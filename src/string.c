// String manipulation functions.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#അന്തര്ഭാവ "stacks.h" // yield
#അന്തര്ഭാവ "string.h" // memcpy
#അന്തര്ഭാവ "farptr.h" // SET_SEG


/****************************************************************
 * String ops
 ****************************************************************/

// Sum the bytes in the specified area.
u8
checksum_far(u16 buf_seg, ശൂന്യം *buf_far, u32 len)
{
    SET_SEG(ES, buf_seg);
    u32 i;
    u8 sum = 0;
    വേണ്ടി (i=0; i<len; i++)
        sum += GET_VAR(ES, ((u8*)buf_far)[i]);
    തിരികെ sum;
}

u8
checksum(ശൂന്യം *buf, u32 len)
{
    തിരികെ checksum_far(GET_SEG(SS), buf, len);
}

size_t
strlen(സ്ഥിരം അക്ഷരം *s)
{
    എങ്കിൽ (__builtin_constant_p(s))
        തിരികെ __builtin_strlen(s);
    സ്ഥിരം അക്ഷരം *p = s;
    ആകുമ്പോൾ (*p)
        p++;
    തിരികെ p-s;
}

പൂർണ്ണസംഖ്യ
memcmp_far(u16 s1seg, സ്ഥിരം ശൂന്യം *s1, u16 s2seg, സ്ഥിരം ശൂന്യം *s2, size_t n)
{
    ആകുമ്പോൾ (n--) {
        പൂർണ്ണസംഖ്യ d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        എങ്കിൽ (d)
            തിരികെ d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    തിരികെ 0;
}

// Compare two areas of memory.
പൂർണ്ണസംഖ്യ
memcmp(സ്ഥിരം ശൂന്യം *s1, സ്ഥിരം ശൂന്യം *s2, size_t n)
{
    ആകുമ്പോൾ (n) {
        എങ്കിൽ (*(u8*)s1 != *(u8*)s2)
            തിരികെ *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    തിരികെ 0;
}

// Compare two strings.
പൂർണ്ണസംഖ്യ
strcmp(സ്ഥിരം അക്ഷരം *s1, സ്ഥിരം അക്ഷരം *s2)
{
    വേണ്ടി (;;) {
        എങ്കിൽ (*s1 != *s2)
            തിരികെ *s1 < *s2 ? -1 : 1;
        എങ്കിൽ (! *s1)
            തിരികെ 0;
        s1++;
        s2++;
    }
}

വരിയിൽ ശൂന്യം
memset_far(u16 d_seg, ശൂന്യം *d_far, u8 c, size_t len)
{
    SET_SEG(ES, d_seg);
    asm അസ്ഥിര(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

വരിയിൽ ശൂന്യം
memset16_far(u16 d_seg, ശൂന്യം *d_far, u16 c, size_t len)
{
    len /= 2;
    SET_SEG(ES, d_seg);
    asm അസ്ഥിര(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

ശൂന്യം *
memset(ശൂന്യം *s, പൂർണ്ണസംഖ്യ c, size_t n)
{
    ആകുമ്പോൾ (n)
        ((അക്ഷരം *)s)[--n] = c;
    തിരികെ s;
}

ശൂന്യം memset_fl(ശൂന്യം *ptr, u8 val, size_t size)
{
    എങ്കിൽ (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (ശൂന്യം*)(FLATPTR_TO_OFFSET(ptr)),
                   val, size);
    അല്ലെങ്കിൽ
        memset(ptr, val, size);
}

വരിയിൽ ശൂന്യം
memcpy_far(u16 d_seg, ശൂന്യം *d_far, u16 s_seg, സ്ഥിരം ശൂന്യം *s_far, size_t len)
{
    SET_SEG(ES, d_seg);
    u16 bkup_ds;
    asm അസ്ഥിര(
        "movw %%ds, %w0\n"
        "movw %w4, %%ds\n"
        "rep movsb (%%si),%%es:(%%di)\n"
        "movw %w0, %%ds"
        : "=&r"(bkup_ds), "+c"(len), "+S"(s_far), "+D"(d_far)
        : "r"(s_seg), "m" (__segment_ES)
        : "cc", "memory");
}

വരിയിൽ ശൂന്യം
memcpy_fl(ശൂന്യം *d_fl, സ്ഥിരം ശൂന്യം *s_fl, size_t len)
{
    എങ്കിൽ (MODESEGMENT)
        memcpy_far(FLATPTR_TO_SEG(d_fl), (ശൂന്യം*)FLATPTR_TO_OFFSET(d_fl)
                   , FLATPTR_TO_SEG(s_fl), (ശൂന്യം*)FLATPTR_TO_OFFSET(s_fl)
                   , len);
    അല്ലെങ്കിൽ
        memcpy(d_fl, s_fl, len);
}

ശൂന്യം *
#undef memcpy
memcpy(ശൂന്യം *d1, സ്ഥിരം ശൂന്യം *s1, size_t len)
#എങ്കിൽ MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
{
    SET_SEG(ES, GET_SEG(SS));
    ശൂന്യം *d = d1;
    എങ്കിൽ (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm അസ്ഥിര(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        തിരികെ d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm അസ്ഥിര(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    തിരികെ d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
ശൂന്യം
iomemcpy(ശൂന്യം *d, സ്ഥിരം ശൂന്യം *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    ആകുമ്പോൾ (len > 3) {
        u32 copylen = len;
        എങ്കിൽ (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm അസ്ഥിര(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    എങ്കിൽ (len)
        // Copy any remaining bytes.
        memcpy(d, s, len);
}

ശൂന്യം *
memmove(ശൂന്യം *d, സ്ഥിരം ശൂന്യം *s, size_t len)
{
    എങ്കിൽ (s >= d)
        തിരികെ memcpy(d, s, len);

    d += len-1;
    s += len-1;
    ആകുമ്പോൾ (len--) {
        *(അക്ഷരം*)d = *(അക്ഷരം*)s;
        d--;
        s--;
    }

    തിരികെ d;
}

// Copy a string - truncating it if necessary.
അക്ഷരം *
strtcpy(അക്ഷരം *dest, സ്ഥിരം അക്ഷരം *src, size_t len)
{
    അക്ഷരം *d = dest;
    ആകുമ്പോൾ (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    തിരികെ dest;
}

// locate first occurrence of character c in the string s
അക്ഷരം *
strchr(സ്ഥിരം അക്ഷരം *s, പൂർണ്ണസംഖ്യ c)
{
    വേണ്ടി (; *s; s++)
        എങ്കിൽ (*s == c)
            തിരികെ (അക്ഷരം*)s;
    തിരികെ ശൂന്യ;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
അക്ഷരം *
nullTrailingSpace(അക്ഷരം *buf)
{
    പൂർണ്ണസംഖ്യ len = strlen(buf);
    അക്ഷരം *end = &buf[len-1];
    ആകുമ്പോൾ (end >= buf && *end <= ' ')
        *(end--) = '\0';
    ആകുമ്പോൾ (*buf && *buf <= ' ')
        buf++;
    തിരികെ buf;
}

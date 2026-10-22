// String manipulation functions.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#அந்தர்பாவ "stacks.h" // yield
#அந்தர்பாவ "string.h" // memcpy
#அந்தர்பாவ "farptr.h" // SET_SEG


/****************************************************************
 * String ops
 ****************************************************************/

// Sum the bytes in the specified area.
u8
checksum_far(u16 buf_seg, வெறுமை *buf_far, u32 len)
{
    SET_SEG(ES, buf_seg);
    u32 i;
    u8 sum = 0;
    க்கு (i=0; i<len; i++)
        sum += GET_VAR(ES, ((u8*)buf_far)[i]);
    திரும்பு sum;
}

u8
checksum(வெறுமை *buf, u32 len)
{
    திரும்பு checksum_far(GET_SEG(SS), buf, len);
}

size_t
strlen(மாறிலி எழுத்து *s)
{
    எனில் (__builtin_constant_p(s))
        திரும்பு __builtin_strlen(s);
    மாறிலி எழுத்து *p = s;
    போது (*p)
        p++;
    திரும்பு p-s;
}

முழு
memcmp_far(u16 s1seg, மாறிலி வெறுமை *s1, u16 s2seg, மாறிலி வெறுமை *s2, size_t n)
{
    போது (n--) {
        முழு d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        எனில் (d)
            திரும்பு d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    திரும்பு 0;
}

// Compare two areas of memory.
முழு
memcmp(மாறிலி வெறுமை *s1, மாறிலி வெறுமை *s2, size_t n)
{
    போது (n) {
        எனில் (*(u8*)s1 != *(u8*)s2)
            திரும்பு *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    திரும்பு 0;
}

// Compare two strings.
முழு
strcmp(மாறிலி எழுத்து *s1, மாறிலி எழுத்து *s2)
{
    க்கு (;;) {
        எனில் (*s1 != *s2)
            திரும்பு *s1 < *s2 ? -1 : 1;
        எனில் (! *s1)
            திரும்பு 0;
        s1++;
        s2++;
    }
}

உள்வரி வெறுமை
memset_far(u16 d_seg, வெறுமை *d_far, u8 c, size_t len)
{
    SET_SEG(ES, d_seg);
    asm நிலையற்ற(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

உள்வரி வெறுமை
memset16_far(u16 d_seg, வெறுமை *d_far, u16 c, size_t len)
{
    len /= 2;
    SET_SEG(ES, d_seg);
    asm நிலையற்ற(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

வெறுமை *
memset(வெறுமை *s, முழு c, size_t n)
{
    போது (n)
        ((எழுத்து *)s)[--n] = c;
    திரும்பு s;
}

வெறுமை memset_fl(வெறுமை *ptr, u8 val, size_t size)
{
    எனில் (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (வெறுமை*)(FLATPTR_TO_OFFSET(ptr)),
                   val, size);
    இல்லையெனில்
        memset(ptr, val, size);
}

உள்வரி வெறுமை
memcpy_far(u16 d_seg, வெறுமை *d_far, u16 s_seg, மாறிலி வெறுமை *s_far, size_t len)
{
    SET_SEG(ES, d_seg);
    u16 bkup_ds;
    asm நிலையற்ற(
        "movw %%ds, %w0\n"
        "movw %w4, %%ds\n"
        "rep movsb (%%si),%%es:(%%di)\n"
        "movw %w0, %%ds"
        : "=&r"(bkup_ds), "+c"(len), "+S"(s_far), "+D"(d_far)
        : "r"(s_seg), "m" (__segment_ES)
        : "cc", "memory");
}

உள்வரி வெறுமை
memcpy_fl(வெறுமை *d_fl, மாறிலி வெறுமை *s_fl, size_t len)
{
    எனில் (MODESEGMENT)
        memcpy_far(FLATPTR_TO_SEG(d_fl), (வெறுமை*)FLATPTR_TO_OFFSET(d_fl)
                   , FLATPTR_TO_SEG(s_fl), (வெறுமை*)FLATPTR_TO_OFFSET(s_fl)
                   , len);
    இல்லையெனில்
        memcpy(d_fl, s_fl, len);
}

வெறுமை *
#undef memcpy
memcpy(வெறுமை *d1, மாறிலி வெறுமை *s1, size_t len)
#எனில் MODESEGMENT == 0
#define memcpy __builtin_memcpy
#endif
{
    SET_SEG(ES, GET_SEG(SS));
    வெறுமை *d = d1;
    எனில் (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        asm நிலையற்ற(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        திரும்பு d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    asm நிலையற்ற(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    திரும்பு d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
வெறுமை
iomemcpy(வெறுமை *d, மாறிலி வெறுமை *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    போது (len > 3) {
        u32 copylen = len;
        எனில் (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        asm நிலையற்ற(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    எனில் (len)
        // Copy any remaining bytes.
        memcpy(d, s, len);
}

வெறுமை *
memmove(வெறுமை *d, மாறிலி வெறுமை *s, size_t len)
{
    எனில் (s >= d)
        திரும்பு memcpy(d, s, len);

    d += len-1;
    s += len-1;
    போது (len--) {
        *(எழுத்து*)d = *(எழுத்து*)s;
        d--;
        s--;
    }

    திரும்பு d;
}

// Copy a string - truncating it if necessary.
எழுத்து *
strtcpy(எழுத்து *dest, மாறிலி எழுத்து *src, size_t len)
{
    எழுத்து *d = dest;
    போது (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    திரும்பு dest;
}

// locate first occurrence of character c in the string s
எழுத்து *
strchr(மாறிலி எழுத்து *s, முழு c)
{
    க்கு (; *s; s++)
        எனில் (*s == c)
            திரும்பு (எழுத்து*)s;
    திரும்பு வெற்று;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
எழுத்து *
nullTrailingSpace(எழுத்து *buf)
{
    முழு len = strlen(buf);
    எழுத்து *end = &buf[len-1];
    போது (end >= buf && *end <= ' ')
        *(end--) = '\0';
    போது (*buf && *buf <= ' ')
        buf++;
    திரும்பு buf;
}

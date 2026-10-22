// String manipulation functions.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#अन्तर्भाव "stacks.h" // yield
#अन्तर्भाव "string.h" // memcpy
#अन्तर्भाव "farptr.h" // SET_SEG


/****************************************************************
 * String ops
 ****************************************************************/

// Sum the bytes in the specified area.
u8
checksum_far(u16 buf_seg, शून्य *buf_far, u32 len)
{
    SET_SEG(ES, buf_seg);
    u32 i;
    u8 sum = 0;
    लागि (i=0; i<len; i++)
        sum += GET_VAR(ES, ((u8*)buf_far)[i]);
    फर्काऊ sum;
}

u8
checksum(शून्य *buf, u32 len)
{
    फर्काऊ checksum_far(GET_SEG(SS), buf, len);
}

माप_प्रकार
म_माप(अचर वर्ण *s)
{
    यदि (__builtin_constant_p(s))
        फर्काऊ __builtin_strlen(s);
    अचर वर्ण *p = s;
    जबसम्म (*p)
        p++;
    फर्काऊ p-s;
}

पूर्णाङ्क
memcmp_far(u16 s1seg, अचर शून्य *s1, u16 s2seg, अचर शून्य *s2, माप_प्रकार n)
{
    जबसम्म (n--) {
        पूर्णाङ्क d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        यदि (d)
            फर्काऊ d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    फर्काऊ 0;
}

// Compare two areas of memory.
पूर्णाङ्क
स_भेद(अचर शून्य *s1, अचर शून्य *s2, माप_प्रकार n)
{
    जबसम्म (n) {
        यदि (*(u8*)s1 != *(u8*)s2)
            फर्काऊ *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    फर्काऊ 0;
}

// Compare two strings.
पूर्णाङ्क
म_भेद(अचर वर्ण *s1, अचर वर्ण *s2)
{
    लागि (;;) {
        यदि (*s1 != *s2)
            फर्काऊ *s1 < *s2 ? -1 : 1;
        यदि (! *s1)
            फर्काऊ 0;
        s1++;
        s2++;
    }
}

अन्तःस्थ शून्य
memset_far(u16 d_seg, शून्य *d_far, u8 c, माप_प्रकार len)
{
    SET_SEG(ES, d_seg);
    यंत्र चञ्चल(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

अन्तःस्थ शून्य
memset16_far(u16 d_seg, शून्य *d_far, u16 c, माप_प्रकार len)
{
    len /= 2;
    SET_SEG(ES, d_seg);
    यंत्र चञ्चल(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

शून्य *
स_रखो(शून्य *s, पूर्णाङ्क c, माप_प्रकार n)
{
    जबसम्म (n)
        ((वर्ण *)s)[--n] = c;
    फर्काऊ s;
}

शून्य memset_fl(शून्य *ptr, u8 val, माप_प्रकार size)
{
    यदि (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (शून्य*)(FLATPTR_TO_OFFSET(ptr)),
                   val, size);
    अन्यथा
        स_रखो(ptr, val, size);
}

अन्तःस्थ शून्य
memcpy_far(u16 d_seg, शून्य *d_far, u16 s_seg, अचर शून्य *s_far, माप_प्रकार len)
{
    SET_SEG(ES, d_seg);
    u16 bkup_ds;
    यंत्र चञ्चल(
        "movw %%ds, %w0\n"
        "movw %w4, %%ds\n"
        "rep movsb (%%si),%%es:(%%di)\n"
        "movw %w0, %%ds"
        : "=&r"(bkup_ds), "+c"(len), "+S"(s_far), "+D"(d_far)
        : "r"(s_seg), "m" (__segment_ES)
        : "cc", "memory");
}

अन्तःस्थ शून्य
memcpy_fl(शून्य *d_fl, अचर शून्य *s_fl, माप_प्रकार len)
{
    यदि (MODESEGMENT)
        memcpy_far(FLATPTR_TO_SEG(d_fl), (शून्य*)FLATPTR_TO_OFFSET(d_fl)
                   , FLATPTR_TO_SEG(s_fl), (शून्य*)FLATPTR_TO_OFFSET(s_fl)
                   , len);
    अन्यथा
        स_नकल(d_fl, s_fl, len);
}

शून्य *
#undef स_नकल
स_नकल(शून्य *d1, अचर शून्य *s1, माप_प्रकार len)
#यदि MODESEGMENT == 0
#define स_नकल __builtin_memcpy
#endif
{
    SET_SEG(ES, GET_SEG(SS));
    शून्य *d = d1;
    यदि (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        यंत्र चञ्चल(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        फर्काऊ d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    यंत्र चञ्चल(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    फर्काऊ d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
शून्य
iomemcpy(शून्य *d, अचर शून्य *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    जबसम्म (len > 3) {
        u32 copylen = len;
        यदि (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        यंत्र चञ्चल(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    यदि (len)
        // Copy any remaining bytes.
        स_नकल(d, s, len);
}

शून्य *
स_हटाओ(शून्य *d, अचर शून्य *s, माप_प्रकार len)
{
    यदि (s >= d)
        फर्काऊ स_नकल(d, s, len);

    d += len-1;
    s += len-1;
    जबसम्म (len--) {
        *(वर्ण*)d = *(वर्ण*)s;
        d--;
        s--;
    }

    फर्काऊ d;
}

// Copy a string - truncating it if necessary.
वर्ण *
strtcpy(वर्ण *dest, अचर वर्ण *src, माप_प्रकार len)
{
    वर्ण *d = dest;
    जबसम्म (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    फर्काऊ dest;
}

// locate first occurrence of character c in the string s
वर्ण *
म_अक्षर(अचर वर्ण *s, पूर्णाङ्क c)
{
    लागि (; *s; s++)
        यदि (*s == c)
            फर्काऊ (वर्ण*)s;
    फर्काऊ रिक्त;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
वर्ण *
nullTrailingSpace(वर्ण *buf)
{
    पूर्णाङ्क len = म_माप(buf);
    वर्ण *end = &buf[len-1];
    जबसम्म (end >= buf && *end <= ' ')
        *(end--) = '\0';
    जबसम्म (*buf && *buf <= ' ')
        buf++;
    फर्काऊ buf;
}

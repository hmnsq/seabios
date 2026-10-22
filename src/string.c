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
    साठी (i=0; i<len; i++)
        sum += GET_VAR(ES, ((u8*)buf_far)[i]);
    परत sum;
}

u8
checksum(शून्य *buf, u32 len)
{
    परत checksum_far(GET_SEG(SS), buf, len);
}

माप_प्रकार
म_माप(स्थिरांक अक्षर *s)
{
    जर (__builtin_constant_p(s))
        परत __builtin_strlen(s);
    स्थिरांक अक्षर *p = s;
    पर्यंत (*p)
        p++;
    परत p-s;
}

पूर्णांक
memcmp_far(u16 s1seg, स्थिरांक शून्य *s1, u16 s2seg, स्थिरांक शून्य *s2, माप_प्रकार n)
{
    पर्यंत (n--) {
        पूर्णांक d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        जर (d)
            परत d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    परत 0;
}

// Compare two areas of memory.
पूर्णांक
स_भेद(स्थिरांक शून्य *s1, स्थिरांक शून्य *s2, माप_प्रकार n)
{
    पर्यंत (n) {
        जर (*(u8*)s1 != *(u8*)s2)
            परत *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    परत 0;
}

// Compare two strings.
पूर्णांक
म_भेद(स्थिरांक अक्षर *s1, स्थिरांक अक्षर *s2)
{
    साठी (;;) {
        जर (*s1 != *s2)
            परत *s1 < *s2 ? -1 : 1;
        जर (! *s1)
            परत 0;
        s1++;
        s2++;
    }
}

ओळीत शून्य
memset_far(u16 d_seg, शून्य *d_far, u8 c, माप_प्रकार len)
{
    SET_SEG(ES, d_seg);
    यंत्र अस्थिर(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

ओळीत शून्य
memset16_far(u16 d_seg, शून्य *d_far, u16 c, माप_प्रकार len)
{
    len /= 2;
    SET_SEG(ES, d_seg);
    यंत्र अस्थिर(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

शून्य *
स_रखो(शून्य *s, पूर्णांक c, माप_प्रकार n)
{
    पर्यंत (n)
        ((अक्षर *)s)[--n] = c;
    परत s;
}

शून्य memset_fl(शून्य *ptr, u8 val, माप_प्रकार size)
{
    जर (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (शून्य*)(FLATPTR_TO_OFFSET(ptr)),
                   val, size);
    अन्यथा
        स_रखो(ptr, val, size);
}

ओळीत शून्य
memcpy_far(u16 d_seg, शून्य *d_far, u16 s_seg, स्थिरांक शून्य *s_far, माप_प्रकार len)
{
    SET_SEG(ES, d_seg);
    u16 bkup_ds;
    यंत्र अस्थिर(
        "movw %%ds, %w0\n"
        "movw %w4, %%ds\n"
        "rep movsb (%%si),%%es:(%%di)\n"
        "movw %w0, %%ds"
        : "=&r"(bkup_ds), "+c"(len), "+S"(s_far), "+D"(d_far)
        : "r"(s_seg), "m" (__segment_ES)
        : "cc", "memory");
}

ओळीत शून्य
memcpy_fl(शून्य *d_fl, स्थिरांक शून्य *s_fl, माप_प्रकार len)
{
    जर (MODESEGMENT)
        memcpy_far(FLATPTR_TO_SEG(d_fl), (शून्य*)FLATPTR_TO_OFFSET(d_fl)
                   , FLATPTR_TO_SEG(s_fl), (शून्य*)FLATPTR_TO_OFFSET(s_fl)
                   , len);
    अन्यथा
        स_नकल(d_fl, s_fl, len);
}

शून्य *
#undef स_नकल
स_नकल(शून्य *d1, स्थिरांक शून्य *s1, माप_प्रकार len)
#जर MODESEGMENT == 0
#define स_नकल __builtin_memcpy
#endif
{
    SET_SEG(ES, GET_SEG(SS));
    शून्य *d = d1;
    जर (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        यंत्र अस्थिर(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        परत d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    यंत्र अस्थिर(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    परत d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
शून्य
iomemcpy(शून्य *d, स्थिरांक शून्य *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    पर्यंत (len > 3) {
        u32 copylen = len;
        जर (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        यंत्र अस्थिर(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    जर (len)
        // Copy any remaining bytes.
        स_नकल(d, s, len);
}

शून्य *
स_हटाओ(शून्य *d, स्थिरांक शून्य *s, माप_प्रकार len)
{
    जर (s >= d)
        परत स_नकल(d, s, len);

    d += len-1;
    s += len-1;
    पर्यंत (len--) {
        *(अक्षर*)d = *(अक्षर*)s;
        d--;
        s--;
    }

    परत d;
}

// Copy a string - truncating it if necessary.
अक्षर *
strtcpy(अक्षर *dest, स्थिरांक अक्षर *src, माप_प्रकार len)
{
    अक्षर *d = dest;
    पर्यंत (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    परत dest;
}

// locate first occurrence of character c in the string s
अक्षर *
म_अक्षर(स्थिरांक अक्षर *s, पूर्णांक c)
{
    साठी (; *s; s++)
        जर (*s == c)
            परत (अक्षर*)s;
    परत रिक्त;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
अक्षर *
nullTrailingSpace(अक्षर *buf)
{
    पूर्णांक len = म_माप(buf);
    अक्षर *end = &buf[len-1];
    पर्यंत (end >= buf && *end <= ' ')
        *(end--) = '\0';
    पर्यंत (*buf && *buf <= ' ')
        buf++;
    परत buf;
}

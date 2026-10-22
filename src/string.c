// String manipulation functions.
//
// Copyright (C) 2008-2013  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU LGPLv3 license.

#समावेश "stacks.h" // yield
#समावेश "string.h" // memcpy
#समावेश "farptr.h" // SET_SEG


/****************************************************************
 * String ops
 ****************************************************************/

// Sum the bytes in the specified area.
u8
checksum_far(u16 buf_seg, व्योम *buf_far, u32 len)
{
    SET_SEG(ES, buf_seg);
    u32 i;
    u8 योग = 0;
    क्रम (i=0; i<len; i++)
        योग += GET_VAR(ES, ((u8*)buf_far)[i]);
    वापस योग;
}

u8
checksum(व्योम *buf, u32 len)
{
    वापस checksum_far(GET_SEG(SS), buf, len);
}

माप_प्रकार
म_माप(स्थिर अक्षर *s)
{
    अगर (__builtin_constant_p(s))
        वापस __builtin_strlen(s);
    स्थिर अक्षर *p = s;
    जबतक (*p)
        p++;
    वापस p-s;
}

पूर्णांक
memcmp_far(u16 s1seg, स्थिर व्योम *s1, u16 s2seg, स्थिर व्योम *s2, माप_प्रकार n)
{
    जबतक (n--) {
        पूर्णांक d = GET_FARVAR(s1seg, *(u8*)s1) - GET_FARVAR(s2seg, *(u8*)s2);
        अगर (d)
            वापस d < 0 ? -1 : 1;
        s1++;
        s2++;
    }
    वापस 0;
}

// Compare two areas of memory.
पूर्णांक
स_भेद(स्थिर व्योम *s1, स्थिर व्योम *s2, माप_प्रकार n)
{
    जबतक (n) {
        अगर (*(u8*)s1 != *(u8*)s2)
            वापस *(u8*)s1 < *(u8*)s2 ? -1 : 1;
        s1++;
        s2++;
        n--;
    }
    वापस 0;
}

// Compare two strings.
पूर्णांक
म_भेद(स्थिर अक्षर *s1, स्थिर अक्षर *s2)
{
    क्रम (;;) {
        अगर (*s1 != *s2)
            वापस *s1 < *s2 ? -1 : 1;
        अगर (! *s1)
            वापस 0;
        s1++;
        s2++;
    }
}

अंतरभूत व्योम
memset_far(u16 d_seg, व्योम *d_far, u8 c, माप_प्रकार len)
{
    SET_SEG(ES, d_seg);
    यंत्र अस्थिर(
        "rep stosb %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

अंतरभूत व्योम
memset16_far(u16 d_seg, व्योम *d_far, u16 c, माप_प्रकार len)
{
    len /= 2;
    SET_SEG(ES, d_seg);
    यंत्र अस्थिर(
        "rep stosw %%es:(%%di)"
        : "+c"(len), "+D"(d_far)
        : "a"(c), "m" (__segment_ES)
        : "cc", "memory");
}

व्योम *
स_रखो(व्योम *s, पूर्णांक c, माप_प्रकार n)
{
    जबतक (n)
        ((अक्षर *)s)[--n] = c;
    वापस s;
}

व्योम memset_fl(व्योम *ptr, u8 val, माप_प्रकार आकार)
{
    अगर (MODESEGMENT)
        memset_far(FLATPTR_TO_SEG(ptr), (व्योम*)(FLATPTR_TO_OFFSET(ptr)),
                   val, आकार);
    अन्यथा
        स_रखो(ptr, val, आकार);
}

अंतरभूत व्योम
memcpy_far(u16 d_seg, व्योम *d_far, u16 s_seg, स्थिर व्योम *s_far, माप_प्रकार len)
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

अंतरभूत व्योम
memcpy_fl(व्योम *d_fl, स्थिर व्योम *s_fl, माप_प्रकार len)
{
    अगर (MODESEGMENT)
        memcpy_far(FLATPTR_TO_SEG(d_fl), (व्योम*)FLATPTR_TO_OFFSET(d_fl)
                   , FLATPTR_TO_SEG(s_fl), (व्योम*)FLATPTR_TO_OFFSET(s_fl)
                   , len);
    अन्यथा
        स_नकल(d_fl, s_fl, len);
}

व्योम *
#अघोषित स_नकल
स_नकल(व्योम *d1, स्थिर व्योम *s1, माप_प्रकार len)
#अगर MODESEGMENT == 0
#घोषणा स_नकल __builtin_memcpy
#पूर्ण_अगर
{
    SET_SEG(ES, GET_SEG(SS));
    व्योम *d = d1;
    अगर (((u32)d1 | (u32)s1 | len) & 3) {
        // non-aligned memcpy
        यंत्र अस्थिर(
            "rep movsb (%%esi),%%es:(%%edi)"
            : "+c"(len), "+S"(s1), "+D"(d)
            : "m" (__segment_ES) : "cc", "memory");
        वापस d1;
    }
    // Common case - use 4-byte copy
    len /= 4;
    यंत्र अस्थिर(
        "rep movsl (%%esi),%%es:(%%edi)"
        : "+c"(len), "+S"(s1), "+D"(d)
        : "m" (__segment_ES) : "cc", "memory");
    वापस d1;
}

// Copy to/from memory mapped IO.  IO mem is very slow, so yield
// periodically.
व्योम
iomemcpy(व्योम *d, स्थिर व्योम *s, u32 len)
{
    ASSERT32FLAT();
    yield();
    जबतक (len > 3) {
        u32 copylen = len;
        अगर (copylen > 2048)
            copylen = 2048;
        copylen /= 4;
        len -= copylen * 4;
        यंत्र अस्थिर(
            "rep movsl (%%esi),%%es:(%%edi)"
            : "+c"(copylen), "+S"(s), "+D"(d)
            : : "cc", "memory");
        yield();
    }
    अगर (len)
        // Copy any remaining bytes.
        स_नकल(d, s, len);
}

व्योम *
स_हटाओ(व्योम *d, स्थिर व्योम *s, माप_प्रकार len)
{
    अगर (s >= d)
        वापस स_नकल(d, s, len);

    d += len-1;
    s += len-1;
    जबतक (len--) {
        *(अक्षर*)d = *(अक्षर*)s;
        d--;
        s--;
    }

    वापस d;
}

// Copy a string - truncating it if necessary.
अक्षर *
strtcpy(अक्षर *dest, स्थिर अक्षर *src, माप_प्रकार len)
{
    अक्षर *d = dest;
    जबतक (--len && *src != '\0')
        *d++ = *src++;
    *d = '\0';
    वापस dest;
}

// locate first occurrence of character c in the string s
अक्षर *
म_अक्षर(स्थिर अक्षर *s, पूर्णांक c)
{
    क्रम (; *s; s++)
        अगर (*s == c)
            वापस (अक्षर*)s;
    वापस शून्य;
}

// Remove any trailing blank characters (spaces, new lines, carriage returns)
अक्षर *
nullTrailingSpace(अक्षर *buf)
{
    पूर्णांक len = म_माप(buf);
    अक्षर *end = &buf[len-1];
    जबतक (end >= buf && *end <= ' ')
        *(end--) = '\0';
    जबतक (*buf && *buf <= ' ')
        buf++;
    वापस buf;
}

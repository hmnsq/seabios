// String manipulation function defs.
#ifndef __STRING_H
#define __STRING_H

#अन्तर्भाव "types.h" // u32

// string.c
u8 checksum_far(u16 buf_seg, शून्य *buf_far, u32 len);
u8 checksum(शून्य *buf, u32 len);
माप_प्रकार म_माप(स्थिरांक अक्षर *s);
पूर्णांक memcmp_far(u16 s1seg, स्थिरांक शून्य *s1, u16 s2seg, स्थिरांक शून्य *s2, माप_प्रकार n);
पूर्णांक स_भेद(स्थिरांक शून्य *s1, स्थिरांक शून्य *s2, माप_प्रकार n);
पूर्णांक म_भेद(स्थिरांक अक्षर *s1, स्थिरांक अक्षर *s2);
शून्य memset_far(u16 d_seg, शून्य *d_far, u8 c, माप_प्रकार len);
शून्य memset16_far(u16 d_seg, शून्य *d_far, u16 c, माप_प्रकार len);
शून्य *स_रखो(शून्य *s, पूर्णांक c, माप_प्रकार n);
शून्य memset_fl(शून्य *ptr, u8 val, माप_प्रकार size);
शून्य memcpy_far(u16 d_seg, शून्य *d_far
                , u16 s_seg, स्थिरांक शून्य *s_far, माप_प्रकार len);
शून्य memcpy_fl(शून्य *d_fl, स्थिरांक शून्य *s_fl, माप_प्रकार len);
शून्य *स_नकल(शून्य *d1, स्थिरांक शून्य *s1, माप_प्रकार len);
#जर MODESEGMENT == 0
#define स_नकल __builtin_memcpy
#endif
शून्य iomemcpy(शून्य *d, स्थिरांक शून्य *s, u32 len);
शून्य *स_हटाओ(शून्य *d, स्थिरांक शून्य *s, माप_प्रकार len);
अक्षर *strtcpy(अक्षर *dest, स्थिरांक अक्षर *src, माप_प्रकार len);
अक्षर *म_अक्षर(स्थिरांक अक्षर *s, पूर्णांक c);
अक्षर *nullTrailingSpace(अक्षर *buf);

#endif // string.h
